/*******************************************************************************
 * Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
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

#if !IS_ENABLED(CONFIG_USE_IHC_V2)
#  error IHC needed for this module
#endif


#include "miv_ihc.h"
#include "opensbi_ecall.h"
#include "opensbi_rproc_ipc_ecall.h"
#include "hss_boot_service.h"

#define RPROC_DETACHED    0x6
#define RPROC_OFFLINE     0x0

static struct RemoteProcMsg rproc_data;

int sbi_ecall_rproc_ipc_handler(unsigned long extid, unsigned long funcid,
    const struct sbi_trap_regs *regs, unsigned long *out_val, struct sbi_trap_info *out_trap)
{
    int result = SBI_ERR_FAILED;
    uint32_t index;
    uint32_t channel = (uint8_t) regs->a0;
    uint32_t target_hart = (channel % CH_PER_MODULE) + 1;
    uint32_t msg;

    switch (funcid) {
#if IS_ENABLED(CONFIG_SERVICE_BOOT)
        case SBI_EXT_RPROC_STATE:
            if (!HSS_SkipBoot_IsSet(target_hart))
                *out_val = RPROC_DETACHED;
            else
                *out_val = RPROC_OFFLINE;
            result = SBI_OK;
            break;
#endif
        case SBI_EXT_RPROC_START:
            rproc_data.target = target_hart;
            if (IPI_MessageAlloc(&index)) {
                if(IPI_MessageDeliver(index, HSS_HART_E51, IPI_MSG_BOOT_REQUEST, RPROC_BOOT, &rproc_data, NULL))
                    result = SBI_OK;
                else
                    IPI_MessageFree(index);
            }
            break;
        case SBI_EXT_RPROC_STOP:
            msg = RPROC_SHUTDOWN_MSG;
            result = IHC_tx_message(channel, &msg, sizeof(msg));
            if (result == 0)
                result = SBI_OK;
            else {
                *out_val = result;
                result = SBI_ERR_FAILED;
            }
            break;
        default:
            result = SBI_ENOTSUPP;
    };

    return result;
}
