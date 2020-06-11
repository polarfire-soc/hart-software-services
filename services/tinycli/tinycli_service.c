/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file tinycli Driver State Machine
 * \brief Virtualised tinycli Service
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"
#include "hss_clock.h"

#include <string.h> //memset
#include <assert.h>

#include "tinycli_service.h"
#include "mpfs_reg_map.h"
#include "hss_boot_service.h"

static void tinycli_init_handler(struct StateMachine * const pMyMachine);
static void tinycli_idle_handler(struct StateMachine * const pMyMachine);

static void tinycli_monitoring_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief WDOG Driver States
 *
 */
enum UartStatesEnum {
    WDOG_INITIALIZATION,
    WDOG_IDLE,
    WDOG_MONITORING,
    WDOG_NUM_STATES = WDOG_MONITORING+1
};

/*!
 * \brief WDOG Driver State Descriptors
 *
 */
static const struct StateDesc tinycli_state_descs[] = {
    { (const stateType_t)WDOG_INITIALIZATION, (const char *)"init",       NULL, NULL, &tinycli_init_handler },
    { (const stateType_t)WDOG_IDLE,           (const char *)"idle",       NULL, NULL, &tinycli_idle_handler },
    { (const stateType_t)WDOG_MONITORING,     (const char *)"monitoring", NULL, NULL, &tinycli_monitoring_handler }
};

/*!
 * \brief WDOG Driver State Machine
 *
 */
struct StateMachine tinycli_service = {
    (stateType_t)WDOG_INITIALIZATION, (stateType_t)SM_INVALID_STATE, (const uint32_t)WDOG_NUM_STATES, (const char *)"tinycli_service", 0u, 0u, 0u, tinycli_state_descs, false, 0u, NULL
};

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//

static void tinycli_init_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state = WDOG_IDLE;
}

/////////////////

static void tinycli_idle_handler(struct StateMachine * const pMyMachine)
{
    (void) pMyMachine;

    // nothing to do in this state
}


/////////////////

static void tinycli_monitoring_handler(struct StateMachine * const pMyMachine)
{
    (void) pMyMachine;
}
