/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file UART Driver State Machine
 * \brief Virtualised UART Service
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"

#include "ssmb_ipi.h"

static void uart_init_handler(struct StateMachine * const pMyMachine);
static void uart_state1_handler(struct StateMachine * const pMyMachine);
static void uart_lastState_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief UART Driver States
 */
enum UartStatesEnum {
    UART_INITIALIZATION,
    UART_STATE1_DO_SOMETHING,
    UART_LAST_STATE,
    UART_NUM_STATES = UART_LAST_STATE+1
};

/*!
 * \brief UART Driver State Descriptors
 */
static const struct StateDesc uart_state_descs[] = {
    { (const stateType_t)UART_INITIALIZATION,           (const char *)"init",      NULL, NULL, &uart_init_handler },
    { (const stateType_t)UART_STATE1_DO_SOMETHING,      (const char *)"state1",    NULL, NULL, &uart_state1_handler },
    { (const stateType_t)UART_LAST_STATE,               (const char *)"lastState", NULL, NULL, &uart_lastState_handler }
};

/*!
 * \brief UART Driver State Machine
 */
struct StateMachine uart_service = {
    (stateType_t)UART_INITIALIZATION, (stateType_t)SM_INVALID_STATE, (const uint32_t)UART_NUM_STATES, (const char *)"uart_service", 0u, 0u, 0u, uart_state_descs, false, 0u, NULL
};

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void uart_init_handler(struct StateMachine * const pMyMachine)
{
    //mHSS_DEBUG_PRINTF("\tcalled" CRLF);
    pMyMachine->state++;
}

/////////////////
static uint32_t i = HSS_HART_U54_1;
static void uart_state1_handler(struct StateMachine * const pMyMachine)
{
    //mHSS_DEBUG_PRINTF("\tcalled" CRLF);

    // check each core to see if it wants to transmit
    if (IPI_GetQueuePendingCount(i)) {
        IPI_ConsumeIntent(i, IPI_MSG_UART_TX);
    }
    i = (i + 1u) % HSS_HART_NUM_PEERS;
    //pMyMachine->state++;
}


/////////////////

static void uart_lastState_handler(struct StateMachine * const pMyMachine)
{
    //mHSS_DEBUG_PRINTF("\tcalled" CRLF);
    pMyMachine->state = UART_INITIALIZATION;
}

