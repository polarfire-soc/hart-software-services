/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS Embedded Software
 *
 */


#include "config.h"
#include "hss_types.h"
#include "csr_helper.h"
#include "opensbi_service.h"

#include <sbi/riscv_encoding.h>
#include <sbi/sbi_bitops.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_scratch.h>
#include <sbi/sbi_trap.h>
#include <sbi/sbi_unpriv.h>

#if !IS_ENABLED(CONFIG_OPENSBI)
#  error OPENSBI needed for this module
#endif

#if !IS_ENABLED(CONFIG_USE_IHC)
#  error IHC needed for this module
#endif


#include "miv_ihc.h"
#include "opensbi_ecall.h"
#include "opensbi_rproc_ecall.h"
#include "hss_boot_service.h"

static struct RemoteProcMsg rproc_data;

int sbi_ecall_rproc_handler(unsigned long extid, unsigned long funcid,
    const struct sbi_trap_regs *regs, unsigned long *out_val, struct sbi_trap_info *out_trap)
{
    int result = SBI_ERR_FAILED;
    uint32_t index, remote_hart_id;
    uint32_t remote_channel = (uint8_t) regs->a0;
    uint32_t ihc_tx_message[IHC_MAX_MESSAGE_SIZE];

    if ((remote_channel < IHC_CHANNEL_TO_CONTEXTA) || (remote_channel > IHC_CHANNEL_TO_CONTEXTB)) {
            result = SBI_EINVAL;
            goto exit;
    }

    remote_hart_id = IHC_context_to_context_hart_id(remote_channel);

    switch (funcid) {
#if IS_ENABLED(CONFIG_SERVICE_BOOT)
        case SBI_EXT_RPROC_STATE:
            if (!HSS_SkipBoot_IsSet(remote_hart_id))
                result = CONTEXT_RUNNING;
            else
                result = CONTEXT_OFFLINE;
            break;
#endif
        case SBI_EXT_RPROC_START:
            rproc_data.target = remote_hart_id;
            if (IPI_MessageAlloc(&index)) {
                if(IPI_MessageDeliver(index, HSS_HART_E51, IPI_MSG_BOOT_REQUEST, RPROC_BOOT, &rproc_data, NULL))
                    result = SBI_OK;
                else
                    IPI_MessageFree(index);
            }
            break;
        case SBI_EXT_RPROC_STOP:
            ihc_tx_message[0] = RPROC_SHUTDOWN_MSG;
            if (IHC_tx_message_from_context(remote_channel, (uint32_t *) ihc_tx_message))
                result = SBI_ERR_FAILED;
            else
                result = SBI_OK;
            break;
        default:
            result = SBI_ENOTSUPP;
    };

exit:
    if (result >= 0) {
        *out_val = result;
        result = 0;
    }
    return result;
}



