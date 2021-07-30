/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file Watchdog Driver State Machine
 * \brief Virtualised Watchdog Service
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"
#include "hss_clock.h"

#include <assert.h>

#include "wdog_service.h"
#include "hss_boot_service.h"

#include "mss_watchdog.h"

static void wdog_init_handler(struct StateMachine * const pMyMachine);
static void wdog_idle_handler(struct StateMachine * const pMyMachine);

static void wdog_monitoring_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief WDOG Driver States
 *
 */
enum UartStatesEnum {
    WDOG_INITIALIZATION,
    WDOG_IDLE,
    WDOG_MONITORING,
    WDOG_NUM_STATES = WDOG_MONITORING+1
};

/*!
 * \brief WDOG Driver State Descriptors
 *
 */
static const struct StateDesc wdog_state_descs[] = {
    { (const stateType_t)WDOG_INITIALIZATION, (const char *)"init",       NULL, NULL, &wdog_init_handler },
    { (const stateType_t)WDOG_IDLE,           (const char *)"idle",       NULL, NULL, &wdog_idle_handler },
    { (const stateType_t)WDOG_MONITORING,     (const char *)"monitoring", NULL, NULL, &wdog_monitoring_handler }
};

/*!
 * \brief WDOG Driver State Machine
 *
 */
struct StateMachine wdog_service = {
    .state             = (stateType_t)WDOG_INITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         = (const uint32_t)WDOG_NUM_STATES,
    .pMachineName      = (const char *)"wdog_service",
    .startTime         = 0u,
    .lastExecutionTime = 0u,
    .executionCount    = 0u,
    .pStateDescs       = wdog_state_descs,
    .debugFlag         = false,
    .priority          = 0u,
    .pInstanceData     = NULL
};

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static union HSSHartBitmask hartBitmask = { .uint = 0u };
static HSSTicks_t wdogInitTime[MAX_NUM_HARTS] = { 0u };

static void wdog_init_handler(struct StateMachine * const pMyMachine)
{
#if IS_ENABLED(CONFIG_SERVICE_WDOG_ENABLE_E51)
    mss_watchdog_config_t wd0lo_config;
    MSS_WD_get_config(MSS_WDOG0_LO, &wd0lo_config);

    wd0lo_config.forbidden_en = MSS_WDOG_ENABLE;
    wd0lo_config.mvrp_val = 0xFFFF000;

    MSS_WD_configure(MSS_WDOG0_LO, &wd0lo_config);
#endif

    pMyMachine->state = WDOG_IDLE;
}

/////////////////

static void wdog_idle_handler(struct StateMachine * const pMyMachine)
{
    (void) pMyMachine;

    if (hartBitmask.uint) {
        if (hartBitmask.s.u54_1) {
            wdogInitTime[HSS_HART_U54_1] = HSS_GetTime();
        }

        if (hartBitmask.s.u54_2) {
            wdogInitTime[HSS_HART_U54_2] = HSS_GetTime();
        }

        if (hartBitmask.s.u54_3) {
            wdogInitTime[HSS_HART_U54_3] = HSS_GetTime();
        }

        if (hartBitmask.s.u54_4) {
            wdogInitTime[HSS_HART_U54_4] = HSS_GetTime();
        }

        //mHSS_DEBUG_PRINTF("watchdog bitmask is 0x%x" CRLF, hartBitmask.uint);
        pMyMachine->state = WDOG_MONITORING;
    }

    // nothing to do in this state
}


/////////////////

#if IS_ENABLED(CONFIG_SERVICE_WDOG_ENABLE_E51)
#  if IS_ENABLED(CONFIG_SERVICE_WDOG_DEBUG)
static HSSTicks_t lastEntryTime = 0u;
#  endif
#endif
static void wdog_monitoring_handler(struct StateMachine * const pMyMachine)
{
    (void) pMyMachine;

#if IS_ENABLED(CONFIG_SERVICE_WDOG_ENABLE_E51)
    // tickle the E51 WDog to prevent it from firing...
    if (!MSS_WD_forbidden_status(MSS_WDOG0_LO)) {
        MSS_WD_reload(MSS_WDOG0_LO);
    }
#endif

#if IS_ENABLED(CONFIG_SERVICE_WDOG_DEBUG)
    if ((hartBitmask.uint)
        && (HSS_Timer_IsElapsed(lastEntryTime,
            (HSSTicks_t)CONFIG_SERVICE_WDOG_DEBUG_TIMEOUT_SEC * TICKS_PER_SEC))) {
#  if WDOG_DEBUG
        HSS_Wdog_DumpStats();
#  endif
#endif

    uint32_t status = mHSS_ReadRegU32(SYSREGSCB, MSS_STATUS);
    status = (status >> 4) & mHSS_BITMASK_ALL_U54; // move bits[8:4] to [4:0]

    status &= hartBitmask.uint;

    if (status) {
        // watchdog timer has triggered for a monitored hart..
        mHSS_DEBUG_PRINTF(LOG_ERROR, "Watchdog has triggered - %02x" CRLF, status);

        if (hartBitmask.s.u54_1) {
            HSS_Boot_RestartCore(HSS_HART_U54_1);
            wdogInitTime[HSS_HART_U54_1] = HSS_GetTime();
        }

        if (hartBitmask.s.u54_2) {
            HSS_Boot_RestartCore(HSS_HART_U54_2);
            wdogInitTime[HSS_HART_U54_2] = HSS_GetTime();
        }

        if (hartBitmask.s.u54_3) {
            HSS_Boot_RestartCore(HSS_HART_U54_3);
            wdogInitTime[HSS_HART_U54_3] = HSS_GetTime();
        }

        if (hartBitmask.s.u54_4) {
            HSS_Boot_RestartCore(HSS_HART_U54_4);
            wdogInitTime[HSS_HART_U54_4] = HSS_GetTime();
        }
    }
}


/////////////////

void HSS_Wdog_MonitorHart(enum HSSHartId target)
{
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "monitoring ");

    switch (target) {
    case HSS_HART_U54_1:
	mHSS_DEBUG_PRINTF_EX("U54_1" CRLF);
        hartBitmask.s.u54_1 = 1;
        break;

    case HSS_HART_U54_2:
	mHSS_DEBUG_PRINTF_EX("U54_2" CRLF);
        hartBitmask.s.u54_2 = 1;
        break;

    case HSS_HART_U54_3:
	mHSS_DEBUG_PRINTF_EX("U54_3" CRLF);
        hartBitmask.s.u54_3 = 1;
        break;

    case HSS_HART_U54_4:
	mHSS_DEBUG_PRINTF_EX("U54_4" CRLF);
        hartBitmask.s.u54_4 = 1;
        break;

    case HSS_HART_ALL:
	mHSS_DEBUG_PRINTF_EX("U54_1 U54_2 U54_3 U54_4" CRLF);
        hartBitmask.s.u54_1 = 1;
        hartBitmask.s.u54_2 = 1;
        hartBitmask.s.u54_3 = 1;
        hartBitmask.s.u54_4 = 1;
        break;

    default:
        assert(1 == 0); // should never reach here!! LCOV_EXCL_LINE
        break;
    }
}

void HSS_Wdog_Reboot(enum HSSHartId target)
{
    switch (target) {
    case HSS_HART_E51:
        MSS_WD_force_reset(MSS_WDOG0_LO);
        break;

    case HSS_HART_U54_1:
        MSS_WD_force_reset(MSS_WDOG1_LO);
        break;

    case HSS_HART_U54_2:
        MSS_WD_force_reset(MSS_WDOG2_LO);
        break;

    case HSS_HART_U54_3:
        MSS_WD_force_reset(MSS_WDOG3_LO);
        break;

    case HSS_HART_U54_4:
        MSS_WD_force_reset(MSS_WDOG4_LO);
        break;

    default:
        assert(1 == 0); // should never reach here!! LCOV_EXCL_LINE
        break;
    }
}

void HSS_Wdog_DumpStats(void)
{
#  if IS_ENABLED(CONFIG_SERVICE_WDOG_DEBUG)
    lastEntryTime = HSS_GetTime();

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Watchdog Status Update: " CRLF);

    if (hartBitmask.s.u54_1) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "U54_1: %lu sec uptime" CRLF,
            (lastEntryTime - wdogInitTime[HSS_HART_U54_1]) / TICKS_PER_SEC);
    }

    if (hartBitmask.s.u54_2) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "U54_2: %lu sec uptime" CRLF,
            (lastEntryTime - wdogInitTime[HSS_HART_U54_2]) / TICKS_PER_SEC);
    }

    if (hartBitmask.s.u54_3) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "U54_3: %lu sec uptime " CRLF,
            (lastEntryTime - wdogInitTime[HSS_HART_U54_3]) / TICKS_PER_SEC);
    }

    if (hartBitmask.s.u54_4) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "U54_4: %lu sec uptime" CRLF,
            (lastEntryTime - wdogInitTime[HSS_HART_U54_4]) / TICKS_PER_SEC);
    }

    mHSS_DEBUG_PRINTF_EX(CRLF);
#endif
}
