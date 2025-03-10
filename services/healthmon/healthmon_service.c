/*******************************************************************************
 * Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
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

#include "sbi_bitops.h"

#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_trigger.h"

#include <assert.h>
#include <string.h>

#include "hss_clock.h"
#include "healthmon_service.h"

#include "mss_hart_ints.h"

extern int sbi_snprintf(char *out, u32 out_sz, const char *format, ...);

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
    { (const stateType_t)HEALTH_INITIALIZATION, (const char *)"init",       NULL, NULL, &healthmon_init_handler },
    { (const stateType_t)HEALTH_MONITORING,     (const char *)"monitoring", NULL, NULL, &healthmon_monitoring_handler },
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

char const * const checkName[] = {
    "above threshold",
    "below threshold",
    "n/a",
    "equal to",
    "not equal to",
    "changed since last read",
};


void healthmon_nop_trigger(uintptr_t pAddr)
{
     ;
}

extern const struct HealthMonitor monitors[];
extern struct HealthMonitor_Status monitor_status[];
extern const size_t monitors_array_size;

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void healthmon_init_handler(struct StateMachine * const pMyMachine)
{
    if (HSS_Trigger_IsNotified(EVENT_DDR_TRAINED) && HSS_Trigger_IsNotified(EVENT_STARTUP_COMPLETE)
        && HSS_Trigger_IsNotified(EVENT_POST_BOOT)) {
        pMyMachine->state = HEALTH_MONITORING;
    }
}

/////////////////
static void healthmon_monitoring_handler(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;

    // general health monitoring...
    {
        for (size_t i = 0u; i < monitors_array_size; i++) {
            if (HSS_Timer_IsElapsed(monitor_status[i].throttle_startTime, monitors[i].throttleScale * ONE_SEC)) {
                uint32_t value = *(uint32_t volatile *)(monitors[i].pAddr);
                enum HealthMon_CheckType checkType = monitors[i].checkType;
                bool triggered = false;

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

                case CHANGED_SINCE_LAST:
                    if (monitor_status[i].initialized) {
                        if (value != monitor_status[i].lastValue) {
                            triggered = true;
                        }
                    } else {
                        monitor_status[i].initialized = true;
                    }
                    break;

                default:
                    // unexpected check type
                    break;
                }

                if (triggered && value != monitor_status[i].lastValue) {
                    monitor_status[i].count++;

                    mHSS_DEBUG_PRINTF(LOG_ERROR, "%s %s ",
                        monitors[i].pName, checkName[monitors[i].checkType]);
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                    if (checkType != CHANGED_SINCE_LAST) {
                        mHSS_DEBUG_PRINTF_EX("0x%x ", monitors[i].maxValue);
                    }
                    mHSS_DEBUG_PRINTF_EX("(0x%x)\n", value);
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);

                    if (monitors[i].triggerCallback) {
                        monitors[i].triggerCallback(monitors[i].pAddr);
                    }

                    monitor_status[i].throttle_startTime = HSS_GetTime();
                    monitor_status[i].lastValue = value;
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
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "monitors_array_size: %d\n", monitors_array_size);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Health Monitoring Counts per trigger:\n");
    for (size_t i = 0u; i < monitors_array_size; i++) {
        char tmp_buffer[80] = "\0";

        switch (monitors[i].checkType) {
        case BELOW_THRESHOLD:
            sbi_snprintf(tmp_buffer, ARRAY_SIZE(tmp_buffer), "%s => %s 0x%x",
                monitors[i].pName, checkName[monitors[i].checkType],
                monitors[i].minValue);
            break;

        case ABOVE_OR_BELOW_THRESHOLD:
            sbi_snprintf(tmp_buffer, ARRAY_SIZE(tmp_buffer), "%s => %s 0x%x <=> 0x%x",
                monitors[i].pName, checkName[monitors[i].checkType],
                monitors[i].minValue, monitors[i].maxValue);
            break;

        case ABOVE_THRESHOLD:
            __attribute__((fallthrough)); // deliberate fallthrough
        case EQUAL_TO_VALUE:
        case NOT_EQUAL_TO_VALUE:
            sbi_snprintf(tmp_buffer, ARRAY_SIZE(tmp_buffer), "%s => %s 0x%x",
                monitors[i].pName, checkName[monitors[i].checkType],
                monitors[i].maxValue);
            break;

        case CHANGED_SINCE_LAST:
            sbi_snprintf(tmp_buffer, ARRAY_SIZE(tmp_buffer), "%s => %s",
                monitors[i].pName, checkName[monitors[i].checkType]);
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
