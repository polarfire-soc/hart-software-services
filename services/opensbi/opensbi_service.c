/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * 
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file Goto State Machine
 * \brief U54 opensbi
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"

#include "ssmb_ipi.h"

#include <assert.h>

#include "csr_helper.h"

#include "ssmb_ipi.h"

#include "opensbi_service.h"

#ifdef CONFIG_SERVICE_BOOT
#  include "hss_boot_pmp.h"
#endif

#include "mpfs_reg_map.h"

#ifndef CONFIG_OPENSBI
#error OPENSBI needed for this module
#endif

struct sbi_scratch scratches[MAX_NUM_HARTS];

static void opensbi_init_handler(struct StateMachine * const pMyMachine);
static void opensbi_idle_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief OPENSBI Driver States
 */
enum GotoStatesEnum {
    OPENSBI_INITIALIZATION,
    OPENSBI_IDLE,
    OPENSBI_NUM_STATES = OPENSBI_IDLE+1
};

/*!
 * \brief OPENSBI Driver State Descriptors
 */
static const struct StateDesc opensbi_state_descs[] = {
    { (const stateType_t)OPENSBI_INITIALIZATION, (const char *)"init",         NULL, NULL, &opensbi_init_handler },
    { (const stateType_t)OPENSBI_IDLE,           (const char *)"idle",         NULL, NULL, &opensbi_idle_handler },
};

/*!
 * \brief OPENSBI Driver State Machine
 */
struct StateMachine opensbi_service = {
    (stateType_t)OPENSBI_INITIALIZATION, (stateType_t)SM_INVALID_STATE, (const uint32_t)OPENSBI_NUM_STATES, (const char *)"opensbi_service", 0u, 0u, 0u, opensbi_state_descs, false, 0u, NULL
};


// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void opensbi_init_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state++;
}

/////////////////
static void opensbi_idle_handler(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine; // unused
}


/////////////////

enum IPIStatusCode HSS_OpenSBI_IPIHandler(TxId_t transaction_id, enum HSSHartId source, uint32_t immediate_arg, void *p_extended_buffer)
{   
    enum IPIStatusCode result = IPI_FAIL;

    (void)immediate_arg;
    (void)p_extended_buffer;

    mHSS_DEBUG_PRINTF("%s() called" CRLF);

    result = IPI_SUCCESS;
    if (source != HSS_HART_E51) { // prohibited by policy
         mHSS_DEBUG_PRINTF("%s(): request from source %d prohibited by policy" CRLF, __func__, source);
    } else if (CSR_GetHartId() == HSS_HART_E51) { // prohibited by policy
         mHSS_DEBUG_PRINTF("%s(): request to %d prohibited by policy" CRLF, __func__, HSS_HART_E51);
    } else {
        mHSS_DEBUG_PRINTF("%s(): I am %d, called by %d, about to invoke sbi_init()..." CRLF, __func__, CSR_GetHartId(), source);
        IPI_Send(source, IPI_MSG_ACK_COMPLETE, transaction_id, IPI_SUCCESS, NULL);
        IPI_MessageUpdateStatus(transaction_id, IPI_IDLE); // free the IPI

        struct IPI_Outbox_Msg *pMsg = IPI_DirectionToFirstMsgInQueue(source, CSR_GetHartId());
        pMsg->msg_type = IPI_MSG_NO_MESSAGE;

        // SBI Scratch must be appropriately setup by this point...
        sbi_init(&(scratches[CSR_GetHartId()]));
    }

    // NOTE: sbi_init should not return...
    return result;
}
