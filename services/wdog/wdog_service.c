/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
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

#include <string.h> //memset
#include <assert.h>

#include "wdog_service.h"
#include "mpfs_reg_map.h"
#include "hss_boot_service.h"

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
    (stateType_t)WDOG_INITIALIZATION, (stateType_t)SM_INVALID_STATE, (const uint32_t)WDOG_NUM_STATES, (const char *)"wdog_service", 0u, 0u, 0u, wdog_state_descs, false, 0u, NULL
};

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static union HSSHartBitmask hartBitmask = { .uint = 0u };

static HSSTicks_t wdogInitTime[MAX_NUM_HARTS] = { 0u };

static void wdog_init_handler(struct StateMachine * const pMyMachine)
{
    //memset(&hartBitmask, 0, sizeof(hartBitmask));
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

        //mHSS_DEBUG_PRINTF("watchdog bitmask is 0x%X" CRLF, hartBitmask.uint);
        wdog_service.state = WDOG_MONITORING;
    }

    // nothing to do in this state
}


/////////////////

#define WDOG_OUTPUT_TIMEOUT_SEC  240u
static HSSTicks_t lastEntryTime = 0u;
static void wdog_monitoring_handler(struct StateMachine * const pMyMachine)
{
    (void) pMyMachine;

    if ((hartBitmask.uint)
        && (HSS_Timer_IsElapsed(lastEntryTime, WDOG_OUTPUT_TIMEOUT_SEC * TICKS_PER_SEC))) {
        lastEntryTime = HSS_GetTime();

        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Watchdog Status Update: ");

        if (hartBitmask.s.u54_1) {
            mHSS_DEBUG_PRINTF_EX("U54_1 (%lu sec) ",
                (lastEntryTime - wdogInitTime[HSS_HART_U54_1]) / TICKS_PER_SEC);
        }

        if (hartBitmask.s.u54_2) {
            mHSS_DEBUG_PRINTF_EX("U54_2 (%lu sec) ",
                (lastEntryTime - wdogInitTime[HSS_HART_U54_2]) / TICKS_PER_SEC);
        }

        if (hartBitmask.s.u54_3) {
            mHSS_DEBUG_PRINTF_EX("U54_3 (%lu sec) ",
                (lastEntryTime - wdogInitTime[HSS_HART_U54_3]) / TICKS_PER_SEC);
        }

        if (hartBitmask.s.u54_4) {
            mHSS_DEBUG_PRINTF_EX("U54_4 (%lu sec) ",
                (lastEntryTime - wdogInitTime[HSS_HART_U54_4]) / TICKS_PER_SEC);
        }
        mHSS_DEBUG_PRINTF_EX(CRLF);

    }

    uint32_t status = mHSS_ReadRegU32(SYSREGSCB, MSS_STATUS);
    status = (status >> 4) & mHSS_BITMASK_ALL_U54; // move bits[8:4] to [4:0]

    status &= hartBitmask.uint;

    if (status) {
        // watchdog timer has triggered for a monitored hart..
        mHSS_DEBUG_PRINTF(LOG_ERROR, "Watchdog has triggered - %02x" CRLF, status);

        if (hartBitmask.s.u54_1) {
            HSS_Boot_RestartCore(HSS_HART_U54_1);
        }

        if (hartBitmask.s.u54_2) {
            HSS_Boot_RestartCore(HSS_HART_U54_2);
        }

        if (hartBitmask.s.u54_3) {
            HSS_Boot_RestartCore(HSS_HART_U54_3);
        }

        if (hartBitmask.s.u54_4) {
            HSS_Boot_RestartCore(HSS_HART_U54_4);
        }
    }
}


/////////////////

void HSS_Wdog_MonitorHart(enum HSSHartId target)
{
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "called" CRLF);

    switch (target) {
    case HSS_HART_U54_1:
        hartBitmask.s.u54_1 = 1;
        break;

    case HSS_HART_U54_2:
        hartBitmask.s.u54_2 = 1;
        break;

    case HSS_HART_U54_3:
        hartBitmask.s.u54_3 = 1;
        break;

    case HSS_HART_U54_4:
        hartBitmask.s.u54_4 = 1;
        break;

    case HSS_HART_ALL:
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

#if 0
#include "mss_watchdog.h"
void HSS_Wdog_Configure(void)
{
    mss_watchdog_config_t wd0lo_config;
    MSS_WD_get_config(MSS_WDOG0_LO, &wd0lo_config);

    wd0l0_config.forbidden_en = WDOG_ENABLE;
    wd0l0_config.mvrp_intr_en = WDOG_DISABLE;
    wd0l0_config.mvrp_intr_in_sleep = WDOG_DISABLE;
    wd0l0_config.mvrp_val = 0xFFFF000;

    MSS_WD_configure(MSS_WDOG0_LO, &wd0lo_config);
}
#endif
