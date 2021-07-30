/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file FlashFreeze Driver State Machine
 * \brief FlashFreeze Service
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"

#include "ssmb_ipi.h"

static void flashfreeze_init_handler(struct StateMachine * const pMyMachine);
static void flashfreeze_state1_handler(struct StateMachine * const pMyMachine);
static void flashfreeze_lastState_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief FLASHFREEZE Driver States
 */
enum FlashFreezeStatesEnum {
    FLASHFREEZE_INITIALIZATION,
    FLASHFREEZE_STATE1_DO_SOMETHING,
    FLASHFREEZE_LAST_STATE,
    FLASHFREEZE_NUM_STATES = FLASHFREEZE_LAST_STATE+1
};

/*!
 * \brief FLASHFREEZE Driver State Descriptors
 */
static const struct StateDesc flashfreeze_state_descs[] = {
    { (const stateType_t)FLASHFREEZE_INITIALIZATION,           (const char *)"init",      NULL, NULL, &flashfreeze_init_handler },
    { (const stateType_t)FLASHFREEZE_STATE1_DO_SOMETHING,      (const char *)"state1",    NULL, NULL, &flashfreeze_state1_handler },
    { (const stateType_t)FLASHFREEZE_LAST_STATE,               (const char *)"lastState", NULL, NULL, &flashfreeze_lastState_handler }
};

/*!
 * \brief FLASHFREEZE Driver State Machine
 */
struct StateMachine flashfreeze_service = {
    .state             = (stateType_t)FLASHFREEZEINITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         = (const uint32_t)FLASHFREEZE_NUM_STATES,
    .pMachineName      = (const char *)"flashfreeze_service",
    .startTime         = 0u,
    .lastExecutionTime = 0u,
    .executionCount    = 0u,
    .pStateDescs       = flashfreeze_state_descs,
    .debugFlag         = false,
    .priority          = 0u,
    .pInstanceData     = NULL,
};
};

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void flashfreeze_init_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state++;
}

/////////////////
static uint32_t i = HSS_HART_U54_1;
static void flashfreeze_state1_handler(struct StateMachine * const pMyMachine)
{
    // check each core to see if it wants to transmit
    if (IPI_GetQueuePendingCount(i)) {
        IPI_ConsumeIntent(i, IPI_MSG_FLASHFREEZE);
    }
    i = (i + 1u) % HSS_HART_NUM_PEERS;
}


/////////////////

static void flashfreeze_lastState_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state = FLASHFREEZE_INITIALIZATION;
}

