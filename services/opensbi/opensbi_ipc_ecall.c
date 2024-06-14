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
#include "opensbi_service.h"

#if !IS_ENABLED(CONFIG_OPENSBI)
#  error OPENSBI needed for this module
#endif

#if !IS_ENABLED(CONFIG_USE_IHC)
#  error IHC needed for this module
#endif

#include <sbi/sbi_ecall.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_error.h>

#include "miv_ihc.h"
#include "opensbi_ecall.h"
#include "opensbi_ipc_ecall.h"

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
    uint64_t addr;
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
            probe_msg->hw_type = MIV_IHC;
            probe_msg->num_channels = MAX_CHANNELS;
            result = SBI_OK;
            break;
        case SBI_EXT_IPC_CH_INIT:
            channel = (uint32_t) regs->a0;
            ipc_init_msg_ptr = (struct mchp_ipc_init *) regs->a1;
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
