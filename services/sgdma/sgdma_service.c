/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file SGDMA Driver State Machine
 * \brief E51-Assisted ScatterGather DMA Service
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"
#include "hss_boot_pmp.h"

#include "ssmb_ipi.h"
#include <assert.h>
#include <string.h>

#include "hss_memcpy_via_pdma.h"
#include "sgdma_service.h"
#include "sgdma_types.h"

static void sgdma_init_handler(struct StateMachine * const pMyMachine);
static void sgdma_idle_handler(struct StateMachine * const pMyMachine);
static void sgdma_transferring_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief SGDMA Driver States
 */
enum SgdmaStatesEnum {
    SGDMA_INITIALIZATION,
    SGDMA_IDLE,
    SGDMA_TRANSFERRING,
    SGDMA_NUM_STATES = SGDMA_TRANSFERRING+1
};

/*!
 * \brief SGDMA Driver State Descriptors
 */
static const struct StateDesc sgdma_state_descs[] = {
    { (const stateType_t)SGDMA_INITIALIZATION, (const char *)"init",         NULL, NULL, &sgdma_init_handler },
    { (const stateType_t)SGDMA_IDLE,           (const char *)"idle",         NULL, NULL, &sgdma_idle_handler },
    { (const stateType_t)SGDMA_TRANSFERRING,   (const char *)"transferring", NULL, NULL, &sgdma_transferring_handler }
};

/*!
 * \brief SGDMA Driver State Machine
 */
struct StateMachine sgdma_service = {
    .state             = (stateType_t)SGDMA_INITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         = (const uint32_t)SGDMA_NUM_STATES,
    .pMachineName      = (const char *)"sgdma_service",
    .startTime         = 0u,
    .lastExecutionTime = 0u,
    .executionCount    = 0u,
    .pStateDescs       = sgdma_state_descs,
    .debugFlag         = false,
    .priority          = 0u,
    .pInstanceData     = NULL
};


static struct HSS_SGDMA_BlockDesc *pBlockDesc = NULL;
static enum HSSHartId activeHart = HSS_HART_E51; // set to signify no U54 active...

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void sgdma_init_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state++;
}

/////////////////
static uint32_t i = HSS_HART_U54_1;
static void sgdma_idle_handler(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;

    // check each core to see if it wants to transmit
    if (IPI_GetQueuePendingCount(i)) {
        IPI_ConsumeIntent(i, IPI_MSG_SCATTERGATHER_DMA);
    }
    i = (i + 1u) % HSS_HART_NUM_PEERS;
}


/////////////////

#define MAX_SGDMA_SIZE_PER_LOOP_ITER (4096u)
static size_t remaining_in_current_block = 0u;
static void sgdma_transferring_handler(struct StateMachine * const pMyMachine)
{
    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "called" CRLF);

    assert(pBlockDesc != NULL);

    if (pBlockDesc->ext) {
       // for now, memcpy...
       // TODO: replace this with PDMA, and x blocks per superloop??
       // should a max pBlockDesc->size be permitted per loop iteration? I guess so...
       //
       size_t chunk_size = 0u;

       if (remaining_in_current_block == 0u) {
           remaining_in_current_block = pBlockDesc->size;
           chunk_size = pBlockDesc->size;
       } else {
           chunk_size = remaining_in_current_block;
       }

       if (chunk_size > MAX_SGDMA_SIZE_PER_LOOP_ITER) {
           chunk_size = MAX_SGDMA_SIZE_PER_LOOP_ITER;
       }

       // check PMPs - todo - check MPRs also
       if (HSS_PMP_CheckWrite(activeHart, (ptrdiff_t)pBlockDesc->dest_phys_addr, chunk_size)
           && HSS_PMP_CheckRead(activeHart, (ptrdiff_t)pBlockDesc->src_phys_addr, chunk_size)) {
           memcpy_via_pdma(pBlockDesc->dest_phys_addr, pBlockDesc->src_phys_addr, chunk_size);
       }

       assert(remaining_in_current_block >= chunk_size);
       remaining_in_current_block -= chunk_size;

       if (remaining_in_current_block == 0u) {
           // finished current block, move to next
           pBlockDesc++;
       }
    } else {
        pMyMachine->state = SGDMA_IDLE;
        pBlockDesc = NULL;
        activeHart = HSS_HART_E51; // set to signify no U54 active...
    }
}

enum IPIStatusCode HSS_SGDMA_IPIHandler(TxId_t transaction_id, enum HSSHartId source, uint32_t immediate_arg, void *p_extended_buffer_in_ddr, void *p_ancilliary_buffer_in_ddr)
{
    (void)transaction_id;
    (void)immediate_arg;
    (void)p_ancilliary_buffer_in_ddr;

    // scatter gather DMA IPI received from one of the U54s...
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "called (sgdma_service.state is %u)" CRLF, sgdma_service.state);


    // the following should always be true if we have consumed intents for SGDMA...
    assert(p_extended_buffer_in_ddr != NULL);
    assert(sgdma_service.state == SGDMA_IDLE);

    // setup the transfer -- the state machine will execute it in chunks
    pBlockDesc = (struct HSS_SGDMA_BlockDesc *)p_extended_buffer_in_ddr;
    activeHart = source;
    sgdma_service.state = SGDMA_TRANSFERRING;

    return IPI_SUCCESS;
}
