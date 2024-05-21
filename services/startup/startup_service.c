/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file Startup Driver State Machine
 * \brief E51-Assisted Startup
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"
#include "hss_boot_init.h"
#include "hss_boot_pmp.h"
#include "hss_trigger.h"
#include "hss_progress.h"

#include "ssmb_ipi.h"
#include "hss_registry.h"
#include <assert.h>
#include <string.h>

#include "ddr_service.h"
#include "tinycli_service.h"
#include "hss_memcpy_via_pdma.h"
#include "startup_service.h"

#ifndef PRIx64
#  define PRIx64 "llu"
#endif

static void startup_init_handler(struct StateMachine * const pMyMachine);
static void startup_boot_handler(struct StateMachine * const pMyMachine);
static void startup_idle_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief STARTUP Driver States
 */
enum ScrubStatesEnum {
    STARTUP_INITIALIZATION,
    STARTUP_BOOT,
    STARTUP_IDLE,
    STARTUP_NUM_STATES = STARTUP_IDLE+1
};

/*!
 * \brief STARTUP Driver State Descriptors
 */
static const struct StateDesc startup_state_descs[] = {
    { (const stateType_t)STARTUP_INITIALIZATION, (const char *)"init", NULL, NULL, &startup_init_handler },
    { (const stateType_t)STARTUP_BOOT,           (const char *)"boot", NULL, NULL, &startup_boot_handler },
    { (const stateType_t)STARTUP_IDLE,           (const char *)"idle", NULL, NULL, &startup_idle_handler },
};

/*!
 * \brief STARTUP Driver State Machine
 */
struct StateMachine startup_service = {
    .state             = (stateType_t)STARTUP_INITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         = (const uint32_t)STARTUP_NUM_STATES,
    .pMachineName      = (const char *)"startup_service",
    .startTime         = 0u,
    .lastExecutionTime = 0u,
    .executionCount    = 0u,
    .pStateDescs       = startup_state_descs,
    .debugFlag         = true,
    .priority          = 0u,
    .pInstanceData     = NULL
};


size_t i = 0u;
// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void startup_init_handler(struct StateMachine * const pMyMachine)
{
    //if (HSS_Trigger_IsNotified(EVENT_DDR_TRAINED)) { }

    assert(globalInitFunctions[i].handler);

    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Initializing %s\n", globalInitFunctions[i].pName);

    bool result = (globalInitFunctions[i].handler)();
    
    if (!result) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "%s() returned %d\n", globalInitFunctions[i].pName, result);
   
        if (globalInitFunctions[i].haltOnFailure) {
            while (true) { ; } // HALT on failure
        } else if (globalInitFunctions[i].restartOnFailure) {
            uint8_t rcvBuf;
  
            bool keyPressedFlag = HSS_ShowTimeout("Init failed, press a key to prevent restart\n", 5u, &rcvBuf);

            if (IS_ENABLED(CONFIG_SERVICE_TINYCLI) && keyPressedFlag) {
                (void)HSS_TinyCLI_Parser();
            } else {
                extern void _start(void);
                _start();
            }
        }
    }
    
    if (i < (spanOfGlobalInitFunctions-1)) {
        i++;
    } else {
        pMyMachine->state++;
        HSS_Trigger_Notify(EVENT_STARTUP_COMPLETE);
    }
}

static void startup_boot_handler(struct StateMachine * const pMyMachine)
{
#if !IS_ENABLED(CONFIG_SERVICE_TINYCLI) && !IS_ENABLED(CONFIG_SERVICE_GPIO_UI)
    if (!HSS_DDR_IsAddrInDDR(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR) || HSS_Trigger_IsNotified(EVENT_DDR_TRAINED)) {
        HSS_BootHarts();
        pMyMachine->state++;
    }
#endif
}

static void startup_idle_handler(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine; // UNUSED
}
