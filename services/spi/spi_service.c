/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file SPI Driver State Machine
 * \brief SPI Service
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"

static void spi_init_onEntry(struct StateMachine * const pMyMachine);
static void spi_init_onExit(struct StateMachine * const pMyMachine);
static void spi_init_handler(struct StateMachine * const pMyMachine);

static void spi_lastState_onEntry(struct StateMachine * const pMyMachine);
static void spi_lastState_onExit(struct StateMachine * const pMyMachine);
static void spi_lastState_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief SPI Driver States
 *
 */
enum SpiStatesEnum {
    SPI_INITIALIZATION,
    SPI_LAST_STATE,
    SPI_NUM_STATES = SPI_LAST_STATE+1
};

/*!
 * \brief SPI Driver State Descriptors
 *
 */
static const struct StateDesc spi_state_descs[] = {
    { (const stateType_t)SPI_INITIALIZATION,           (const char *)"init",      &spi_init_onEntry,      &spi_init_onExit,      &spi_init_handler },
    { (const stateType_t)SPI_LAST_STATE,               (const char *)"lastState", &spi_lastState_onEntry, &spi_lastState_onExit, &spi_lastState_handler }
};

/*!
 * \brief SPI Driver State Machine
 *
 */
struct StateMachine spi_service = {
    .state             = (stateType_t)SPI_INITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         = (const uint32_t)SPI_NUM_STATES,
    .pMachineName      = (const char *)"spi_service",
    .startTime         = 0u,
    .lastExecutionTime = 0u,
    .executionCount    = 0u,
    .pStateDescs       = spi_state_descs,
    .debugFlag         = false,
    .priority          = 0u,
    .pInstanceData     = NULL
};

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void spi_init_onEntry(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;
}

static void spi_init_onExit(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;
}

static void spi_init_handler(struct StateMachine * const pMyMachine)
{
}

/////////////////

static void spi_lastState_onEntry(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;
}

static void spi_lastState_onExit(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;
}

static void spi_lastState_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state = SPI_INITIALIZATION;
}

