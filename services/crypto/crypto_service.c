/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file Crypto Driver State Machine
 * \brief MSS Crypto Driver
 */

#include <stdlib.h>
#include <stdint.h>

#include "ssmb_ipi.h"

#include "hss_state_machine.h"
#include "hss_debug.h"

#include "crypto_types.h"
#include "athena_cal.h"

static void crypto_init_handler(struct StateMachine * const pMyMachine);
static void crypto_state1_handler(struct StateMachine * const pMyMachine);
static void crypto_lastState_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief Crypto Driver States
 */
enum CryptoStatesEnum {
    CRYPTO_INITIALIZATION,
    CRYPTO_STATE1,
    CRYPTO_LAST_STATE,
    CRYPTO_NUM_STATES = CRYPTO_LAST_STATE+1
};

/*!
 * \brief Crypto Driver State Descriptors
 */
static const struct StateDesc crypto_state_descs[] = {
    { (const stateType_t)CRYPTO_INITIALIZATION,           (const char *)"init",      NULL, NULL, &crypto_init_handler },
    { (const stateType_t)CRYPTO_STATE1,                   (const char *)"state1",    NULL, NULL, &crypto_state1_handler },
    { (const stateType_t)CRYPTO_LAST_STATE,               (const char *)"lastState", NULL, NULL, &crypto_lastState_handler }
};

/*!
 * \brief Crypto Driver State Machine
 */
struct StateMachine crypto_service = {
    (stateType_t)CRYPTO_INITIALIZATION, (stateType_t)SM_INVALID_STATE, (const uint32_t)CRYPTO_NUM_STATES, (const char *)"crypto_service", 0u, 0u, 0u, crypto_state_descs, false, 0u, NULL
};

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void crypto_init_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state++;
}

/////////////////

static uint32_t i = HSS_HART_U54_1;
static void crypto_state1_handler(struct StateMachine * const pMyMachine)
{
    // check each core to see if it wants to transmit
    if (IPI_GetQueuePendingCount(i)) {
        IPI_ConsumeIntent(i, IPI_MSG_POWERMODE);
    }
    i = (i + 1u) % HSS_HART_NUM_PEERS;
}


/////////////////

static void crypto_lastState_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state = CRYPTO_INITIALIZATION;
}
