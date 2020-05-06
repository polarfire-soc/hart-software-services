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
#include "hss_atomic.h"
#include "hss_clock.h"

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
#  error OPENSBI needed for this module
#endif


extern const struct sbi_platform platform;

extern unsigned long _hartid_to_scratch(int hartid); // crt.S

static unsigned long l_hartid_to_scratch(int hartid);

//
// OpenSBI needs a scratch structure per hart, plus some ancilliary data space
// after this...
//
// "External firmware must create per-HART non-overlapping:
// 1. Program Stack
// 2.OpenSBI scratch space (i.e. struct sbi_scratch instance with extra space above)"

union {
    struct sbi_scratch scratch;
    unsigned long buffer[SBI_SCRATCH_SIZE+16];
} scratches[MAX_NUM_HARTS];

static void opensbi_scratch_setup(int hartid)
{
    scratches[hartid].scratch.options = SBI_SCRATCH_DEBUG_PRINTS;
    scratches[hartid].scratch.hartid_to_scratch = (unsigned long)l_hartid_to_scratch;
    scratches[hartid].scratch.platform_addr = (unsigned long)&platform;
}

static unsigned long l_hartid_to_scratch(int hartid)
{
    assert(hartid < MAX_NUM_HARTS);
    return (unsigned long)&(scratches[hartid].scratch);
}

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
    { (const stateType_t)OPENSBI_INITIALIZATION, (const char *)"Init",     NULL, NULL, &opensbi_init_handler },
    { (const stateType_t)OPENSBI_IDLE,       (const char *)"Idle",     NULL, NULL, &opensbi_idle_handler },
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

extern unsigned long _trap_handler;
void HSS_OpenSBI_Setup(enum HSSHartId hartid)
{
    assert(current_hartid() == hartid);

    opensbi_scratch_setup(hartid);

    if (hartid == HSS_HART_E51) {
        sbi_hss_e51_init(&(scratches[hartid].scratch), false);
    } else {
        //sbi_printf("%s(): _trap_handler is %lx\n", __func__, (unsigned long)&_trap_handler);
	while (mHSS_CSR_READ(CSR_MTVEC) != (unsigned long)&_trap_handler) {
            mHSS_CSR_WRITE(CSR_MTVEC, (unsigned long)&_trap_handler);
	}

        mb();
        mb_i();
        sbi_init(&(scratches[hartid].scratch));
    }
}

void HSS_OpenSBI_DoBoot(enum HSSHartId hartid);

void HSS_OpenSBI_DoBoot(enum HSSHartId hartid)
{
    uint32_t mstatus_val = mHSS_CSR_READ(CSR_MSTATUS);
    mstatus_val = EXTRACT_FIELD(mstatus_val, MSTATUS_MPIE);
    mHSS_CSR_WRITE(CSR_MSTATUS, mstatus_val);
    mHSS_CSR_WRITE(CSR_MIE, 0u);

    HSS_OpenSBI_Setup(hartid);

    struct sbi_scratch *scratch = &(scratches[hartid].scratch);
    sbi_hart_switch_mode(hartid, scratch->next_arg1, scratch->next_addr,
         scratch->next_mode, FALSE);
}

enum IPIStatusCode HSS_OpenSBI_IPIHandler(TxId_t transaction_id, enum HSSHartId source, uint32_t immediate_arg, void *p_extended_buffer)
{   
    enum IPIStatusCode result = IPI_FAIL;

    result = IPI_SUCCESS;
    int hartid = current_hartid();

    if (source != HSS_HART_E51) { // prohibited by policy
        mHSS_DEBUG_PRINTF("request from source %d prohibited by policy" CRLF, source);
    } else if (hartid == HSS_HART_E51) { // prohibited by policy
        mHSS_DEBUG_PRINTF("request to %d prohibited by policy" CRLF, HSS_HART_E51);
    } else {
        IPI_Send(source, IPI_MSG_ACK_COMPLETE, transaction_id, IPI_SUCCESS, NULL);
        IPI_MessageUpdateStatus(transaction_id, IPI_IDLE); // free the IPI

        struct IPI_Outbox_Msg *pMsg = IPI_DirectionToFirstMsgInQueue(source, current_hartid());
        pMsg->msg_type = IPI_MSG_NO_MESSAGE;

        result = IPI_SUCCESS;

        csr_write(mscratch, &(scratches[hartid].scratch));


        extern unsigned long _hss_start, _hss_end;
        scratches[hartid].scratch.fw_start = (unsigned long)&_hss_start;
        scratches[hartid].scratch.fw_size = (unsigned long)&_hss_end - (unsigned long)&_hss_start;

        scratches[hartid].scratch.next_addr = *(unsigned long*)p_extended_buffer; 
        scratches[hartid].scratch.next_mode = (unsigned long)immediate_arg;

        // set arg1 (A1) to point to device tree blob
#ifdef CONFIG_PROVIDE_DTB
#  if defined(CONFIG_PLATFORM_POLARFIRESOC)
        extern unsigned long _binary_services_opensbi_mpfs_dtb_start;
        scratches[hartid].scratch.next_arg1 = (unsigned long)&_binary_services_opensbi_mpfs_dtb_start;
#  elif defined(CONFIG_PLATFORM_MPFS)
        extern unsigned long _binary_hifive_unleashed_a00_dtb_start;
        scratches[hartid].scratch.next_arg1 = (unsigned long)&_binary_hifive_unleashed_a00_dtb_start;
#  else
#    error Unknown PLATFORM settings
#  endif
#else
        scratches[hartid].scratch.next_arg1 = 0u;
#endif

        HSS_OpenSBI_DoBoot(hartid);
    }

    return result;
}

#include <sbi/sbi_ecall.h>
#include <sbi/sbi_error.h>
#include "hss_boot_service.h"

static int sbi_ecall_hss_handler(struct sbi_scratch *scratch,
    unsigned long extid, unsigned long funcid,
    unsigned long *args, unsigned long *out_val, struct sbi_trap_info *out_trap)
{
    int ret = 0;
    //struct sbi_scratch *scratch = sbi_scratch_thishart_ptr();

    (void)scratch;
    (void)args;
    uint32_t index;

    switch (funcid) {
    case SBI_EXT_HSS_REBOOT:
        IPI_MessageAlloc(&index);
        IPI_MessageDeliver(index, HSS_HART_E51, IPI_MSG_BOOT_REQUEST, 0u, 0);
        ret = 0; //ret = hss_reboot_req(scratch, args[0], args[1], args[2]);
        break;

    default:
        ret = SBI_ENOTSUPP;
    };

    if (ret >= 0) {
        *out_val = ret;
        ret = 0;
    }

    return ret;
}

struct sbi_ecall_extension ecall_hss = {
    .extid_start = SBI_EXT_HSS,
    .extid_end = SBI_EXT_HSS,
    .handle = sbi_ecall_hss_handler
};

void HSS_SBI_Ecall_Register(void)
{
    int result = sbi_ecall_register_extension(&ecall_hss);
    //if (result)
    //    sbi_hart_hang();
    (void)result; 
}
