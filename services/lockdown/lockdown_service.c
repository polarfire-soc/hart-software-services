/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file Lockdown Driver State Machine
 * \brief E51-Assisted Lockdown
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"
#include "hss_trigger.h"
#include "hss_progress.h"

#include "ssmb_ipi.h"
#include "hss_registry.h"
#include <assert.h>
#include <string.h>

#include "lockdown_service.h"

#if IS_ENABLED(CONFIG_SERVICE_WDOG)
#  include "wdog_service.h"
#endif

static void lockdown_init_handler(struct StateMachine * const pMyMachine);
static void lockdown_active_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief LOCKDOWN Driver States
 */
enum ScrubStatesEnum {
    LOCKDOWN_INITIALIZATION,
    LOCKDOWN_LOCKED_DOWN,
    LOCKDOWN_NUM_STATES = LOCKDOWN_LOCKED_DOWN+1
};

/*!
 * \brief LOCKDOWN Driver State Descriptors
 */
static const struct StateDesc lockdown_state_descs[] = {
    { (const stateType_t)LOCKDOWN_INITIALIZATION, (const char *)"init",   NULL, NULL, &lockdown_init_handler },
    { (const stateType_t)LOCKDOWN_LOCKED_DOWN,    (const char *)"active", NULL, NULL, &lockdown_active_handler },
};

/*!
 * \brief LOCKDOWN Driver State Machine
 */
struct StateMachine lockdown_service = {
    .state             = (stateType_t)LOCKDOWN_INITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         = (const uint32_t)LOCKDOWN_NUM_STATES,
    .pMachineName      = (const char *)"lockdown_service",
    .startTime         = 0u,
    .lastExecutionTime = 0u,
    .executionCount    = 0u,
    .pStateDescs       = lockdown_state_descs,
    .debugFlag         = true,
    .priority          = 0u,
    .pInstanceData     = NULL
};


// --------------------------------------------------------------------------------------------------
// Weak=binding functions whose functionality can be changed here, or can be overridden in 
// per-board BSP code to implement customer-specific behavior
//

__attribute__((weak)) void e51_pmp_lockdown(void);
__attribute__((weak)) void e51_pmp_lockdown(void)
{
    // 
    // setup E51 PMPs, limited to L2Scratch and Watchdog only... (and system controller?)
    //
    mHSS_DEBUG_PRINTF(LOG_STATE_TRANSITION, "lockdown:: E51 PMPs should be configured here...\n");
}

__attribute__((weak,__noreturn__)) void e51_lockdown(void);
__attribute__((weak,__noreturn__)) void e51_lockdown(void)
{

    mHSS_DEBUG_PRINTF_EX(
        " _____ ____  _    _               _            _    ____\n"
        "| ____| ___|/ |  | |    ___   ___| | _____  __| |  |  _ \\  _____      ___ __\n"
        "|  _| |___ \\| |  | |   / _ \\ / __| |/ / _ \\/ _` |  | | | |/ _ \\ \\ /\\ / /  _ \\ \n"
        "| |___ ___) | |  | |__| (_) | (__|   <  __/ (_| |  | |_| | (_) \\ V  V /| | | |\n"
        "|_____|____/|_|  |_____\\___/ \\___|_|\\_\\___|\\__,_|  |____/ \\___/ \\_/\\_/ |_| |_|\n");

    // when we get here, we're post boot and we have locked down the PMPs ...
#if IS_ENABLED(CONFIG_SERVICE_WDOG)
    mHSS_DEBUG_PRINTF(LOG_STATE_TRANSITION, "lockdown:: from this point, only servicing watchdogs\n");
#endif

    while (1) {
        // ... so we'll now just spin servicing the watchdogs...
#if IS_ENABLED(CONFIG_SERVICE_WDOG)
        RunStateMachine(&wdog_service); // TODO: should we refactor wdog to guarantee using fewer regs?
#endif
    }
}

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//

static void lockdown_init_handler(struct StateMachine * const pMyMachine)
{
    assert(pMyMachine);

    if (HSS_Trigger_IsNotified(EVENT_BOOT_COMPLETE)) {
        // delay one superloop to lock down, to give other state machines an
        // opportunity to reach acquiescence

        pMyMachine->state = LOCKDOWN_LOCKED_DOWN;
    }
}

__attribute__ ((__noreturn__)) static void lockdown_active_handler(struct StateMachine * const pMyMachine)
{
    assert(pMyMachine);

    e51_pmp_lockdown();
    e51_lockdown();
}
