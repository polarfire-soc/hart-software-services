/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file Goto State Machine
 * \brief U54 goto
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"

#include "ssmb_ipi.h"

#include <assert.h>

#include "csr_helper.h"

#include "ssmb_ipi.h"

#include "goto_service.h"

#if IS_ENABLED(CONFIG_SERVICE_BOOT)
#  include "hss_boot_pmp.h"
#endif

#include "mpfs_reg_map.h"
#include "csr_helper.h"

#include "hss_atomic.h"

static void goto_init_handler(struct StateMachine * const pMyMachine);
static void goto_idle_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief GOTO Driver States
 */
enum GotoStatesEnum {
    GOTO_INITIALIZATION,
    GOTO_IDLE,
    GOTO_NUM_STATES = GOTO_IDLE+1
};

/*!
 * \brief GOTO Driver State Descriptors
 */
static const struct StateDesc goto_state_descs[] = {
    { (const stateType_t)GOTO_INITIALIZATION, (const char *)"init",         NULL, NULL, &goto_init_handler },
    { (const stateType_t)GOTO_IDLE,           (const char *)"idle",         NULL, NULL, &goto_idle_handler },
};

/*!
 * \brief GOTO Driver State Machine
 */
struct StateMachine goto_service = {
    .state             = (stateType_t)GOTO_INITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         = (const uint32_t)GOTO_NUM_STATES,
    .pMachineName      = (const char *)"goto_service",
    .startTime         = 0u,
    .lastExecutionTime = 0u,
    .executionCount    = 0u,
    .pStateDescs       = goto_state_descs,
    .debugFlag         = false,
    .priority          = 0u,
    .pInstanceData     = NULL,
};


// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void goto_init_handler(struct StateMachine * const pMyMachine)
{
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "called" CRLF);
    pMyMachine->state++;
}

/////////////////
static void goto_idle_handler(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine; // unused
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "called" CRLF);
}


/////////////////

enum IPIStatusCode HSS_GOTO_IPIHandler(TxId_t transaction_id, enum HSSHartId source, uint32_t immediate_arg, void *p_extended_buffer, void *p_ancilliary_buffer_in_ddr)
{
    enum IPIStatusCode result = IPI_FAIL;
    (void)p_ancilliary_buffer_in_ddr;

    int hartid = current_hartid();

    if (source != HSS_HART_E51) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "security policy prevented GOTO request from hart %d" CRLF, source);
    } else if (hartid == HSS_HART_E51) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "hart %d: request prohibited by policy" CRLF, HSS_HART_E51);
    } else {
        // the following should always be true if we have consumed intents for GOTO...
        assert(p_extended_buffer != NULL);

        // we ain't coming back from the GOTO, so need to ACK here...
        IPI_Send(source, IPI_MSG_ACK_COMPLETE, transaction_id, IPI_SUCCESS, NULL, NULL);
        IPI_MessageUpdateStatus(transaction_id, IPI_IDLE); // free the IPI

        struct IPI_Outbox_Msg *pMsg = IPI_DirectionToFirstMsgInQueue(source, current_hartid());
        size_t i;

        for (i = 0u; i < IPI_MAX_NUM_QUEUE_MESSAGES; i++) {
            if (pMsg->transaction_id == transaction_id) { break; }
            pMsg++;
        }

        // if message found process it...
        if (pMsg->transaction_id == transaction_id) {
            pMsg->msg_type = IPI_MSG_NO_MESSAGE;

            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Address to execute is %p" CRLF, (void *)p_extended_buffer);
            CSR_ClearMSIP();

            uint32_t mstatus_val = mHSS_CSR_READ(CSR_MSTATUS);
            mstatus_val = EXTRACT_FIELD(mstatus_val, MSTATUS_MPIE);
            mHSS_CSR_WRITE(CSR_MSTATUS, mstatus_val);
            mHSS_CSR_WRITE(CSR_MIE, 0u);

            result = IPI_SUCCESS;
        }

        if (result != IPI_FAIL) {
            // From the v1.10 RISC-V Priileged Spec:
            // The MRET, SRET, or URET instructions are used to return from traps in M-mode, S-mode,
            // or U-mode respectively. When executing an xRET instruction, supposing x PP holds the
            // value y, x IE is set to x PIE; the privilege mode is changed to y; x PIE is set to 1;
            // and x PP is set to U (or M if user-mode is not supported).
            const uint32_t next_mode = immediate_arg;

#if IS_ENABLED(CONFIG_OPENSBI)
            sbi_hart_switch_mode(0u, 0u, (unsigned long)p_extended_buffer, next_mode, false /*next_virt -> required hypervisor */);
#else
            // set MSTATUS.MPP to Supervisor mode, and set MSTATUS.MPIE to 1
            uint32_t mstatus_val = mHSS_CSR_READ(mstatus);

            // next_mode stores the desired privilege mode to return to..
            // typically PRV_S
            //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Setting priv mode to %d" CRLF, next_mode);
            mstatus_val = INSERT_FIELD(mstatus_val, MSTATUS_MPP, next_mode);

            if (next_mode == PRV_M) {
                mHSS_DEBUG_PRINTF(LOG_NORMAL, "Booting into M-Mode so clearing MSTATUS:MIE" CRLF);
                mstatus_val = INSERT_FIELD(mstatus_val, MSTATUS_MPIE, 0);
                mstatus_val = INSERT_FIELD(mstatus_val, MSTATUS_MIE, 0);
            } else {
                mstatus_val = INSERT_FIELD(mstatus_val, MSTATUS_MPIE, 1);
                mstatus_val = INSERT_FIELD(mstatus_val, MSTATUS_MIE, 1);
            }

            mstatus_val = INSERT_FIELD(mstatus_val, MSTATUS_SIE, 0);
            mstatus_val = INSERT_FIELD(mstatus_val, MSTATUS_SPIE, 0);
            mHSS_CSR_WRITE(mstatus, mstatus_val);

            // set MEPC to function address (smuggled in p_extended_buffer argument)
            mHSS_CSR_WRITE(mepc, *((void **)p_extended_buffer));

            // execute MRET, causing MIE <= MPIE, new priv mode <= PRV_S, MPIE <= 1, MPP <= U
            asm("mret");
            __builtin_unreachable();
#endif

            // state machine doesn't want outside framework to send a separate complete
            // message, as we have jumped somewhere else
            //
            // for tidyness/test code, we'll return IPI_IDLE in this scenario if all is okay, or
            // IPI_FAIL otherwise
            result = IPI_IDLE;
        }

    }

    return result;
}
