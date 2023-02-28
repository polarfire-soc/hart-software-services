/*******************************************************************************
 * Copyright 2019-2023 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file Health Monitor State Machine
 * \brief E51-Assisted Health Monitor
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"

#include <assert.h>
#include <string.h>

#include "hss_clock.h"
#include "healthmon_service.h"
#include "sbi_bitops.h"

#include "mss_hart_ints.h"

#ifndef BIT
#  define BIT(nr)			(1UL << (nr))
#endif

static void healthmon_init_handler(struct StateMachine * const pMyMachine);
static void healthmon_monitoring_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief Health Driver States
 */
enum BeuStatesEnum {
    HEALTH_INITIALIZATION,
    HEALTH_MONITORING,
    HEALTH_NUM_STATES = HEALTH_MONITORING+1
};

/*!
 * \brief Health Driver State Descriptors
 */
static const struct StateDesc healthmon_state_descs[] = {
    { (const stateType_t)HEALTH_INITIALIZATION, (const char *)"init",      NULL, NULL, &healthmon_init_handler },
    { (const stateType_t)HEALTH_MONITORING,      (const char *)"monitoring", NULL, NULL, &healthmon_monitoring_handler },
};

/*!
 * \brief Health Driver State Machine
 */
struct StateMachine healthmon_service = {
    .state             = (stateType_t)HEALTH_INITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         = (const uint32_t)HEALTH_NUM_STATES,
    .pMachineName      = (const char *)"healthmon_service",
    .startTime         = 0u,
    .lastExecutionTime = 0u,
    .executionCount    = 0u,
    .pStateDescs       = healthmon_state_descs,
    .debugFlag         = true,
    .priority          = 0u,
    .pInstanceData     = NULL
};

enum CheckTypeEnum
{
    ABOVE_THRESHOLD,
    BELOW_THRESHOLD,
    ABOVE_OR_BELOW_THRESHOLD,
    EQUAL_TO_VALUE,
    NOT_EQUAL_TO_VALUE,
};

char const * const checkName[] = {
    "above threshold",
    "below threshold",
    "n/a",
    "equal to",
    "not equal to",
};


static void nop_trigger(uintptr_t pAddr)
{
     ;
}

static const struct HealthMonitor
{
    char const * const pName;
    uintptr_t pAddr;
    enum CheckTypeEnum checkType;
    uint32_t maxValue;
    uint32_t minValue;
    uint8_t shift; // shift applied first...
    uint64_t mask; // then mask
    void (*triggerCallback)(uintptr_t pAddr);
    uint32_t throttleScale; // times 1sec, to throttle console messages
} monitors[] =
{
    { "IOSCB_PLL_MSS:PLL_CTRL",		0x3E001004, NOT_EQUAL_TO_VALUE, 1u, 0u, 25u, 0x1u, nop_trigger, 1u },
    { "IOSCB_PLL_DDR:PLL_CTRL",		0x3E010004, NOT_EQUAL_TO_VALUE, 1u, 0u, 25u, 0x1u, nop_trigger, 1u },
    { "IOSCB_PLL_SGMII:PLL_CTRL",	0x3E001004, NOT_EQUAL_TO_VALUE, 1u, 0u, 25u, 0x1u, nop_trigger, 1u },
    //{ "IOSCB_PLL:pll_se_0:PLL_CTRL",	0x38010004, NOT_EQUAL_TO_VALUE, 1u, 0u, 25u, 0x1u, nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_se_1:PLL_CTRL",	0x38020004, NOT_EQUAL_TO_VALUE, 1u, 0u, 25u, 0x1u, nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_ne_0:PLL_CTRL",	0x38040004, NOT_EQUAL_TO_VALUE, 1u, 0u, 25u, 0x1u, nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_ne_1:PLL_CTRL",	0x38080004, NOT_EQUAL_TO_VALUE, 1u, 0u, 25u, 0x1u, nop_trigger, 5u },
    { "IOSCB_PLL:pll_nw_0:PLL_CTRL",	0x38100004, NOT_EQUAL_TO_VALUE, 1u, 0u, 25u, 0x1u, nop_trigger, 1u },
    //{ "IOSCB_PLL:pll_nw_1:PLL_CTRL",	0x38200004, NOT_EQUAL_TO_VALUE, 1u, 0u, 25u, 0x1u, nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_sw_0:PLL_CTRL",	0x38400004, NOT_EQUAL_TO_VALUE, 1u, 0u, 25u, 0x1u, nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_sw_1:PLL_CTRL",	0x38800004, NOT_EQUAL_TO_VALUE, 1u, 0u, 25u, 0x1u, nop_trigger, 5u },
};

static struct HealthMonitor_Status
{
    HSSTicks_t throttle_startTime;
    size_t count;
} monitor_status[ARRAY_SIZE(monitors)] =
{
   { 0u, 0u }
};
// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void healthmon_init_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state++;
}

/////////////////
static void healthmon_monitoring_handler(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;

    // general health monitoring...
    {
        bool triggered = false;

        for (size_t i = 0u; i < ARRAY_SIZE(monitors); i++) {
            uint32_t value = *(uint32_t volatile *)(monitors[i].pAddr);
            enum CheckTypeEnum checkType = monitors[i].checkType;

            if (monitors[i].shift) { value = value >> monitors[i].shift; }
            if (monitors[i].mask) { value = value & monitors[i].mask; }

            switch (checkType) {
            case ABOVE_THRESHOLD:
                if (value > monitors[i].maxValue) { triggered = true; }
                break;

            case BELOW_THRESHOLD:
                if (value < monitors[i].minValue) { triggered = true; }
                break;

            case ABOVE_OR_BELOW_THRESHOLD:
                if (value > monitors[i].maxValue) {
                    triggered = true;
                    checkType = ABOVE_THRESHOLD;
                } else if (value < monitors[i].minValue) {
                    triggered = true;
                    checkType = BELOW_THRESHOLD;
                }
                break;

            case EQUAL_TO_VALUE:
                if (value == monitors[i].maxValue) { triggered = true; }
                break;

            case NOT_EQUAL_TO_VALUE:
                if (value != monitors[i].maxValue) { triggered = true; }
                break;

            default:
                // unexpected check type
                break;
            }

            if (triggered) {
                monitor_status[i].count++;

                if (HSS_Timer_IsElapsed(monitor_status[i].throttle_startTime, monitors[i].throttleScale * ONE_SEC)) {
                    mHSS_DEBUG_PRINTF(LOG_ERROR, "%s %s %" PRIu64 " (%" PRIu64 ")\n",
                        monitors[i].pName, checkName[monitors[i].checkType],
                        monitors[i].maxValue, value);

                    if (monitors[i].triggerCallback) {
                        monitors[i].triggerCallback(monitors[i].pAddr);
                    }

                    monitor_status[i].throttle_startTime = HSS_GetTime();
                } else {
                    continue;
                }
            }
        }

        //if (!triggered) {
        // mHSS_DEBUG_PRINTF(LOG_NORMAL, "No HealthMon triggers fired\n");
        //}
    }
}

/////////////////
void HSS_Health_DumpStats(void)
{
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Health Monitoring Counts per trigger:\n");
    for (size_t i = 0u; i < ARRAY_SIZE(monitors); i++) {
        char tmp_buffer[80] = "\0";

        extern int sbi_snprintf(char *out, u32 out_sz, const char *format, ...);

        switch (monitors[i].checkType) {
        case BELOW_THRESHOLD:
            sbi_snprintf(tmp_buffer, ARRAY_SIZE(tmp_buffer), "%s => %s %u",
                monitors[i].pName, checkName[monitors[i].checkType],
                monitors[i].minValue);
            break;

        case ABOVE_OR_BELOW_THRESHOLD:
            sbi_snprintf(tmp_buffer, ARRAY_SIZE(tmp_buffer), "%s => %s %u <=> %u",
                monitors[i].pName, checkName[monitors[i].checkType],
                monitors[i].minValue, monitors[i].maxValue);
            break;

        case ABOVE_THRESHOLD:
            __attribute__((fallthrough)); // deliberate fallthrough
        case EQUAL_TO_VALUE:
        case NOT_EQUAL_TO_VALUE:
            sbi_snprintf(tmp_buffer, ARRAY_SIZE(tmp_buffer), "%s => %s %u",
                monitors[i].pName, checkName[monitors[i].checkType],
                monitors[i].maxValue);
            break;

        default:
            // unexpected check type
            break;
        }

        if (tmp_buffer[0]) {
            // we have an entry with a valid checkType
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "% 60s: %" PRIu64 "\n",
                tmp_buffer, monitor_status[i].count);
        }
    }
}
