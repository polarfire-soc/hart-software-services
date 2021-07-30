/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file PowerMode Driver State Machine
 * \brief Clock Switching / MSS Power Mode Selection
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"

#include "ssmb_ipi.h"


static void powermode_init_handler(struct StateMachine * const pMyMachine);
static void powermode_state1_handler(struct StateMachine * const pMyMachine);
static void powermode_lastState_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief PowerMode Driver States
 */
enum PowerModeStatesEnum {
    POWER_MODE_INITIALIZATION,
    POWER_MODE_STATE1_DO_SOMETHING,
    POWER_MODE_LAST_STATE,
    POWER_MODE_NUM_STATES = POWER_MODE_LAST_STATE+1
};

/*!
 * \brief PowerMode Driver State Descriptors
 */
static const struct StateDesc powermode_state_descs[] = {
    { (const stateType_t)POWER_MODE_INITIALIZATION,           (const char *)"init",      NULL, NULL, &powermode_init_handler },
    { (const stateType_t)POWER_MODE_STATE1_DO_SOMETHING,      (const char *)"state1",    NULL, NULL, &powermode_state1_handler },
    { (const stateType_t)POWER_MODE_LAST_STATE,               (const char *)"lastState", NULL, NULL, &powermode_lastState_handler }
};

/*!
 * \brief PowerMode Driver State Machine
 */
struct StateMachine powermode_service = {
    .state             = (stateType_t)POWER_MODE_INITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         =  (const uint32_t)POWER_MODE_NUM_STATES,
    .pMachineName          =  (const char *)"powermode_service",
    .startTime         =  0u,
    .lastExecutionTime =  0u,
    .executionCount    =  0u,
    .pStateDescs       =  powermode_state_descs,
    .debugFlag         =  false,
    .priority          =  0u,
    .pInstanceData     =  NULL
};

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void powermode_init_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state++;
}

/////////////////

static uint32_t i = HSS_HART_U54_1;
static void powermode_state1_handler(struct StateMachine * const pMyMachine)
{
    // check each core to see if it wants to transmit
    if (IPI_GetQueuePendingCount(i)) {
        IPI_ConsumeIntent(i, IPI_MSG_POWERMODE);
    }
    i = (i + 1u) % HSS_HART_NUM_PEERS;
}


/////////////////

static void powermode_lastState_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state = POWER_MODE_INITIALIZATION;
}

