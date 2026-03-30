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
#include "opensbi_service.h"

#if !IS_ENABLED(CONFIG_OPENSBI)
#  error OPENSBI needed for this module
#endif

#if !IS_ENABLED(CONFIG_USE_IHC_V2)
#  error IHC needed for this module
#endif

#include <sbi/sbi_ecall.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_error.h>

#include "miv_ihc.h"
#include "opensbi_ecall.h"
#include "opensbi_ipc_ecall.h"
#include "ddr_service.h"

// validate a user-supplied pointer by ensuring it is non-null, and that the start and end of
// the struct are in DDR
//

#define mIPC_PTR_VALID(ptr, type) \
    ((ptr) != NULL \
     && HSS_DDR_IsAddrInDDR((uintptr_t)(ptr)) \
     && HSS_DDR_IsAddrInDDR((uintptr_t)(ptr) + sizeof(type)))

enum ipc_hw {
    MIV_IHC,
};

enum ipc_irq_type {
    IPC_OPS_NOT_SUPPORTED   = 1,
    IPC_MP_IRQ              = 2,
    IPC_MC_IRQ              = 4,
};

struct mchp_ipc_probe {
    enum ipc_hw hw_type;
    uint8_t num_channels;
};

struct mchp_ipc_init {
    uint16_t max_msg_size;
};

struct mchp_ipc_status {
    uint32_t status;
    uint8_t module;
};

struct mchp_ipc_sbi_message {
    uintptr_t addr;
    uint16_t size;
    uint8_t irq_type;
};

static uint32_t ihc_mp_isr_callback(uint8_t channel,
                                    const uint32_t *message,
                                    uint32_t message_size,
                                    uint32_t *ext_msg_ptr)
{
    struct mchp_ipc_sbi_message * sbi_msg;

    sbi_msg = ( struct mchp_ipc_sbi_message *) ext_msg_ptr;
    sbi_msg->irq_type = IPC_MP_IRQ;
    sbi_msg->size = message_size * (sizeof(uint32_t));

    // validate the destination before writing as sbi_msg->addr is U-mode-supplied
    // and could otherwise redirect this memcpy to arbitrary M-mode memory
    //
    if (!mIPC_PTR_VALID((void *)sbi_msg->addr, uint8_t)
            || !HSS_DDR_IsAddrInDDR(sbi_msg->addr + sbi_msg->size)) {
        return 1u;
    }

    memcpy((uint32_t *) sbi_msg->addr, message, message_size * (sizeof(uint32_t)));

    return 0u;
}

int sbi_ecall_ipc_handler(unsigned long extid, unsigned long funcid,
    const struct sbi_trap_regs *regs, unsigned long *out_val, struct sbi_trap_info *out_trap)
{
    int result;
    uint32_t channel;
    struct mchp_ipc_init *ipc_init_msg_ptr;
    struct mchp_ipc_probe * probe_msg;
    struct mchp_ipc_status * status_msg_ptr;
    struct mchp_ipc_sbi_message * sbi_msg;

    switch (funcid) {
        case SBI_EXT_IPC_PROBE:
            probe_msg = (struct mchp_ipc_probe *)regs->a0;
            if (!mIPC_PTR_VALID(probe_msg, struct mchp_ipc_probe)) {
                result = SBI_EINVAL;
                break;
            }
            probe_msg->hw_type = MIV_IHC;
            probe_msg->num_channels = MAX_CHANNELS;
            result = SBI_OK;
            break;
        case SBI_EXT_IPC_CH_INIT:
            channel = (uint32_t) regs->a0;
            ipc_init_msg_ptr = (struct mchp_ipc_init *) regs->a1;
            if (!mIPC_PTR_VALID(ipc_init_msg_ptr, struct mchp_ipc_init)) {
                result = SBI_EINVAL;
                break;
            }
            IHC_init(channel);
            IHC_config_mp_callback_handler(channel,ihc_mp_isr_callback);
            IHC_enable_mp_interrupt(channel);
            IHC_enable_mc_interrupt(channel);
            ipc_init_msg_ptr->max_msg_size = MAX_MSG_SIZE_IN_BYTES;
            result = SBI_OK;
            break;
        case SBI_EXT_IPC_SEND:
            channel = (uint32_t) regs->a0;
            sbi_msg = (struct mchp_ipc_sbi_message *) regs->a1;
            if (!mIPC_PTR_VALID(sbi_msg, struct mchp_ipc_sbi_message)) {
                result = SBI_EINVAL;
                break;
            }
            if (!mIPC_PTR_VALID((void *)sbi_msg->addr, uint8_t)
                    || !HSS_DDR_IsAddrInDDR(sbi_msg->addr + sbi_msg->size)) {
                result = SBI_EINVAL;
                break;
            }
            result = IHC_tx_message(channel, (uint32_t *) sbi_msg->addr, sbi_msg->size);
            if(result == 0)
                result = SBI_OK;
            else {
                *out_val = result;
                result = SBI_ERR_FAILED;
            }
            break;
        case SBI_EXT_IPC_RECEIVE:
            channel = (uint32_t) regs->a0;
            uint32_t module = (uint32_t)channel / CH_PER_MODULE;
            if (!mIPC_PTR_VALID((void *)regs->a1, uint32_t)) {
                result = SBI_EINVAL;
                break;
            }
            result = IHC_indirect_irq_handler(module, (uint32_t *) regs->a1);
            if(result == 0)
                result = SBI_OK;
            else {
                *out_val = result;
                result = SBI_ERR_FAILED;
            }
            break;
        case SBI_EXT_IPC_STATUS:
            status_msg_ptr = ( struct mchp_ipc_status *) regs->a0;
            if (!mIPC_PTR_VALID(status_msg_ptr, struct mchp_ipc_status)) {
                result = SBI_EINVAL;
                break;
            }
            uint32_t status = IHC_get_module_intr_status(status_msg_ptr->module);
            status_msg_ptr->status = status;
            status_msg_ptr->module = status_msg_ptr->module;
            result = SBI_OK;
            break;
        default:
            result = SBI_ENOTSUPP;
    };

    return result;
}
