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

enum CheckTypeEnum
{
    ABOVE_THRESHOLD,
    BELOW_THRESHOLD,
    ABOVE_OR_BELOW_THRESHOLD,
    EQUAL_TO_VALUE,
    NOT_EQUAL_TO_VALUE,
    CHANGED_SINCE_LAST,
};

char const * const checkName[] = {
    "above threshold",
    "below threshold",
    "n/a",
    "equal to",
    "not equal to",
    "changed since last read",
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
    { "IOSCB_PLL_MSS:PLL_CTRL",		0x3E001004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       nop_trigger, 1u },
    { "IOSCB_PLL_DDR:PLL_CTRL",		0x3E010004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       nop_trigger, 1u },
    { "IOSCB_PLL_SGMII:PLL_CTRL",	0x3E001004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       nop_trigger, 1u },
    { "IOSCB_PLL:pll_nw_0:PLL_CTRL",	0x38100004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       nop_trigger, 1u },
    //{ "IOSCB_PLL:pll_se_0:PLL_CTRL",	0x38010004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_se_1:PLL_CTRL",	0x38020004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_ne_0:PLL_CTRL",	0x38040004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_ne_1:PLL_CTRL",	0x38080004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_nw_1:PLL_CTRL",	0x38200004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_sw_0:PLL_CTRL",	0x38400004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       nop_trigger, 5u },
    //{ "IOSCB_PLL:pll_sw_1:PLL_CTRL",	0x38800004, NOT_EQUAL_TO_VALUE, 1u,      0u, 25u, 1u,       nop_trigger, 5u },

    { "SYSREG:BOOT_FAIL_CR",            0x20002014, NOT_EQUAL_TO_VALUE, 0u,      0u, 0u,  1u,       nop_trigger, 5u },
    { "SYSREG:DEVICE_STATUS",           0x20002024, NOT_EQUAL_TO_VALUE, 0x1F09u, 0u, 0u,  0x1FFF,   nop_trigger, 5u },
    { "SYSREG:MPU_VIOLATION_SR",        0x200020F0, NOT_EQUAL_TO_VALUE, 0u,      0u, 0u,  1u,       nop_trigger, 5u },
    { "SYSREG:EDAC_SR",                 0x20002100, NOT_EQUAL_TO_VALUE, 0u,      0u, 0u,  0x3FFF,   nop_trigger, 5u },

    { "SYSREG:EDAC_CNT_MMC",            0x20002108, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1F,     nop_trigger, 5u },
    { "SYSREG:EDAC_CNT_DDRC",           0x2000210C, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1F,     nop_trigger, 5u },
    { "SYSREG:EDAC_CNT_MAC0",           0x20002110, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1F,     nop_trigger, 5u },
    { "SYSREG:EDAC_CNT_MAC1",           0x20002114, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1F,     nop_trigger, 5u },
    { "SYSREG:EDAC_CNT_USB",            0x20002118, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1F,     nop_trigger, 5u },
    { "SYSREG:EDAC_CNT_CAN0",           0x2000211c, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1F,     nop_trigger, 5u },
    { "SYSREG:EDAC_CNT_CAN1",           0x20002120, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1F,     nop_trigger, 5u },

    { "SYSREG:MAINTENANCE_INT_SR",      0x20002148, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1FFFFF, nop_trigger, 5u },// [20:0] == some cleared by writing 1, some y writing to PLL_STATUS
    { "SYSREG:PLL_STATUS_SR",           0x2000214c, NOT_EQUAL_TO_VALUE, 0x707u,  0u, 0u,  0x7FF,    nop_trigger, 5u },
    { "SYSREG:MISC_SR",                 0x20002154, NOT_EQUAL_TO_VALUE, 0u,      0u, 0u,  2u,       nop_trigger, 5u },
    { "SYSREG:DLL_STATUS_SR",           0x2000215c, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0x1FFFFF, nop_trigger, 5u },

    { "IOSCBCFG:STATUS",                0x37080004, NOT_EQUAL_TO_VALUE, 0u,      0u, 0u,  0xEu,     nop_trigger, 5u },// [3:1] => scb_buserr, timeout, scb_error

    // unknown what the following should be...
    { "IOSCB_PLL:pll_se_0:PLL_CTRL",	0x38010004, CHANGED_SINCE_LAST, 0u,      0u, 25u, 1u,       nop_trigger, 5u },
    { "IOSCB_PLL:pll_se_1:PLL_CTRL",	0x38020004, CHANGED_SINCE_LAST, 0u,      0u, 25u, 1u,       nop_trigger, 5u },
    { "IOSCB_PLL:pll_ne_0:PLL_CTRL",	0x38040004, CHANGED_SINCE_LAST, 0u,      0u, 25u, 1u,       nop_trigger, 5u },
    { "IOSCB_PLL:pll_ne_1:PLL_CTRL",	0x38080004, CHANGED_SINCE_LAST, 0u,      0u, 25u, 1u,       nop_trigger, 5u },
    { "IOSCB_PLL:pll_nw_1:PLL_CTRL",	0x38200004, CHANGED_SINCE_LAST, 0u,      0u, 25u, 1u,       nop_trigger, 5u },
    { "IOSCB_PLL:pll_sw_0:PLL_CTRL",	0x38400004, CHANGED_SINCE_LAST, 0u,      0u, 25u, 1u,       nop_trigger, 5u },
    { "IOSCB_PLL:pll_sw_1:PLL_CTRL",	0x38800004, CHANGED_SINCE_LAST, 0u,      0u, 25u, 1u,       nop_trigger, 5u },

    { "L2:Config:ECCDirFixCount",       0x02010108, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0xFFFFFFFFFFFFFFFFu, nop_trigger, 1u },
    { "L2:Config:ECCDataFixCount",      0x02010148, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0xFFFFFFFFFFFFFFFFu, nop_trigger, 1u },
    { "L2:Config:ECCDataFailCount",     0x02010168, CHANGED_SINCE_LAST, 0u,      0u, 0u,  0xFFFFFFFFFFFFFFFFu, nop_trigger, 1u },
};

static struct HealthMonitor_Status
{
    HSSTicks_t throttle_startTime;
    uint32_t lastValue;
    size_t count;
    bool initialized;
} monitor_status[ARRAY_SIZE(monitors)] =
{
   { 0u, 0u, 0u, false }
};
// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void healthmon_init_handler(struct StateMachine * const pMyMachine)
{
    if (HSS_Trigger_IsNotified(EVENT_DDR_TRAINED) && HSS_Trigger_IsNotified(EVENT_STARTUP_COMPLETE)) {
        pMyMachine->state = HEALTH_MONITORING;
    }
}

/////////////////
static void healthmon_monitoring_handler(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;

    // general health monitoring...
    {
        for (size_t i = 0u; i < ARRAY_SIZE(monitors); i++) {
            if (HSS_Timer_IsElapsed(monitor_status[i].throttle_startTime, monitors[i].throttleScale * ONE_SEC)) {
                uint32_t value = *(uint32_t volatile *)(monitors[i].pAddr);
                enum CheckTypeEnum checkType = monitors[i].checkType;
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
                    monitor_status[i].lastValue = value;
                    break;

                default:
                    // unexpected check type
                    break;
                }

                if (triggered) {
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
