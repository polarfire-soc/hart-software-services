/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS Embedded Software
 *
 */


#include "config.h"
#include "hss_types.h"

#include <sbi/sbi_ecall.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_error.h>
//#include <sbi/sbi_trap.h>
//#include <sbi/sbi_version.h>
//#include <sbi/riscv_asm.h>
//#include <sbi/riscv_barrier.h>


#include "opensbi_service.h"
#include "opensbi_ecall.h"

#if !IS_ENABLED(CONFIG_OPENSBI)
#  error OPENSBI needed for this module
#endif

#if IS_ENABLED(CONFIG_USE_IHC) && IS_ENABLED(CONFIG_SERVICE_OPENSBI_IHC)
#  include "miv_ihc.h"
#  include "opensbi_ihc_ecall.h"
#endif

#include "hss_boot_service.h"

int HSS_SBI_ECALL_Handler(long extid, long funcid,
    const struct sbi_trap_regs *regs, unsigned long *out_val, struct sbi_trap_info *out_trap)
{
    int result = 0;
    uint32_t index;

    switch (funcid) {
        //
        // MiV IHC functions
        case SBI_EXT_IHC_CTX_INIT:
            __attribute__((fallthrough)); // deliberate fallthrough
        case SBI_EXT_IHC_SEND:
            __attribute__((fallthrough)); // deliberate fallthrough
        case SBI_EXT_IHC_RECEIVE:
#if IS_ENABLED(CONFIG_USE_IHC) && IS_ENABLED(CONFIG_SERVICE_OPENSBI_IHC)
            result = sbi_ecall_ihc_handler(extid, funcid, regs, out_val, out_trap);
#endif
            break;

        //
        // HSS functions
        case SBI_EXT_HSS_REBOOT:
            IPI_MessageAlloc(&index);
            IPI_MessageDeliver(index, HSS_HART_E51, IPI_MSG_BOOT_REQUEST, 0u, NULL, NULL);
            result = 0;
            break;

        default:
            result = SBI_ENOTSUPP;
    };

//exit:
    if (result >= 0) {
        *out_val = result;
        result = 0;
    }
    return result;
}

int HSS_SBI_Vendor_Ext_Check(long extid)
{
    return (SBI_EXT_MICROCHIP_TECHNOLOGY == extid);
}
