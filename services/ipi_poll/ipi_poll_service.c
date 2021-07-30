/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file IPI Poll Driver State Machine
 * \brief IPI Poll Service
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"

#include "ssmb_ipi.h"
#include <string.h> //memset
#include <assert.h>

#include "mpfs_reg_map.h"
#include "csr_helper.h"

#include "ipi_poll_service.h"


static void ipiPoll_init_handler(struct StateMachine * const pMyMachine);
static void ipiPoll_monitoring_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief UART Driver States
 *
 */
enum UartStatesEnum {
    IPI_POLL_INITIALIZATION,
    IPI_POLL_MONITORING,
    IPI_POLL_NUM_STATES = IPI_POLL_MONITORING+1
};

/*!
 * \brief UART Driver State Descriptors
 *
 */
static const struct StateDesc ipiPoll_state_descs[] = {
    { (const stateType_t)IPI_POLL_INITIALIZATION, (const char *)"Init",       NULL, NULL, &ipiPoll_init_handler },
    { (const stateType_t)IPI_POLL_MONITORING,     (const char *)"Monitoring", NULL, NULL, &ipiPoll_monitoring_handler }
};

/*!
 * \brief UART Driver State Machine
 *
 */
struct StateMachine ipi_poll_service = {
    .state             = (stateType_t)IPI_POLL_INITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         = (const uint32_t)IPI_POLL_NUM_STATES,
    .pMachineName      = (const char *)"ipi_poll_service",
    .startTime         = 0u,
    .lastExecutionTime = 0u,
    .executionCount    = 0u,
    .pStateDescs       = ipiPoll_state_descs,
    .debugFlag         = true,
    .priority          = 0u,
    .pInstanceData     = NULL,
};

// ----------------------------------------------------------------------------------------------------------------------

static union HSSHartBitmask hartBitmask = { .uint = 0 };

static void ipiPoll_init_handler(struct StateMachine * const pMyMachine)
{
    memset(&hartBitmask, 0, sizeof(hartBitmask));
    hartBitmask.uint = mHSS_BITMASK_ALL_U54;
    pMyMachine->state = IPI_POLL_MONITORING;
}

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void ipiPoll_monitoring_handler(struct StateMachine * const pMyMachine)
{
    (void) pMyMachine;

    // poll IPIs each iteration for new messages
    bool const status = IPI_PollReceive(hartBitmask);

    enum HSSHartId const myHartId = current_hartid();
    if (status) {
        unsigned int i;
        for (i = 0u; i < MAX_NUM_HARTS; i++) {
            if (i == myHartId) { continue; } // don't handle messages if to my own hartid

            uint32_t const index = IPI_CalculateQueueIndex(i, myHartId);

            if (IPI_GetQueuePendingCount(index)) {
                IPI_ConsumeIntent(i, IPI_MSG_ACK_COMPLETE); // gobble up any ACK completes
                IPI_ConsumeIntent(i, IPI_MSG_ACK_PENDING);  // gobble up any ACK pendings
            }
        }
    }
}


/////////////////

void HSS_IpiPoll_Enable(enum HSSHartId target)
{
   switch (target) {
   case HSS_HART_U54_1:
       hartBitmask.s.u54_1 = 1;
       break;

   case HSS_HART_U54_2:
       hartBitmask.s.u54_2 = 1;
       break;

   case HSS_HART_U54_3:
       hartBitmask.s.u54_3 = 1;
       break;

   case HSS_HART_U54_4:
       hartBitmask.s.u54_4 = 1;
       break;

   case HSS_HART_ALL:
       hartBitmask.s.u54_1 = 1;
       hartBitmask.s.u54_2 = 1;
       hartBitmask.s.u54_3 = 1;
       hartBitmask.s.u54_4 = 1;
       break;

   default:
       assert(1 == 0); // should never reach here!! LCOV_EXCL_LINE
       break;
   }
   ipi_poll_service.state = IPI_POLL_MONITORING;
}
