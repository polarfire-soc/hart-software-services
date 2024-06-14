/*******************************************************************************
 * Copyright 2023 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC Microprocessor Subsystem Inter-Hart Communication bare metal
 * software driver implementation.
 *
 */

#include "mpfs_hal/mss_hal.h"
#include "miv_ihc.h"

IHC_TypeDef g_ihc;

/**
 * \brief IHC configuration
 *
 */

const uint64_t ihc_base_addess[MAX_CHANNELS]= {
        IHC_CH_H0_H1,       // IHC_H0_H1_A,
        IHC_CH_H0_H2,       // IHC_H0_H2_A,
        IHC_CH_H0_H3,       // IHC_H0_H3_A,
        IHC_CH_H0_H4,       // IHC_H0_H4_A,
        IHC_CH_H0_H5,       // IHC_H0_H5_A,
        IHC_CH_H1_H0,       // IHC_H1_H0_B,
        IHC_CH_H1_H2,       // IHC_H1_H2_A,
        IHC_CH_H1_H3,       // IHC_H1_H3_A,
        IHC_CH_H1_H4,       // IHC_H1_H4_A,
        IHC_CH_H1_H5,       // IHC_H1_H5_A,
        IHC_CH_H2_H0,       // IHC_H2_H0_B,
        IHC_CH_H2_H1,       // IHC_H2_H1_B,
        IHC_CH_H2_H3,       // IHC_H2_H3_A,
        IHC_CH_H2_H4,       // IHC_H2_H4_A,
        IHC_CH_H2_H5,       // IHC_H2_H5_A,
        IHC_CH_H3_H0,       // IHC_H3_H0_B,
        IHC_CH_H3_H1,       // IHC_H3_H1_B,
        IHC_CH_H3_H2,       // IHC_H3_H2_B,
        IHC_CH_H3_H4,       // IHC_H3_H4_A,
        IHC_CH_H3_H5,       // IHC_H3_H5_A,
        IHC_CH_H4_H0,       // IHC_H4_H0_B,
        IHC_CH_H4_H1,       // IHC_H4_H1_B,
        IHC_CH_H4_H2,       // IHC_H4_H2_B,
        IHC_CH_H4_H3,       // IHC_H4_H3_B,
        IHC_CH_H4_H5,       // IHC_H4_H5_A,
        IHC_CH_H5_H0,       // IHC_H5_H0_B,
        IHC_CH_H5_H1,       // IHC_H5_H1_B,
        IHC_CH_H5_H2,       // IHC_H5_H2_B,
        IHC_CH_H5_H3,       // IHC_H5_H3_B,
        IHC_CH_H5_H4       // IHC_H5_H4_B
};

/**
 * \brief IHC configuration
 *
 */
const uint64_t IHCIM_base_addess[MAX_MODULE] = {
        IHCIM_H0,
        IHCIM_H1,
        IHCIM_H2,
        IHCIM_H3,
        IHCIM_H4,
        IHCIM_H5
};

#define IHC_ASSERT(x) ASSERT(x)

/*
 * Private Functions
 */

static int8_t rx_message(uint8_t channel, QUEUE_IHC_INCOMING handle_incoming, uint8_t msg_type);
static int8_t message_present_isr(uint8_t channel);
static int8_t message_consumed_isr(uint8_t channel);

static void IHC_enable_mp_interrupt_ch_module(uint8_t channel);
static void IHC_enable_cp_interrupt_ch_module(uint8_t channel);

/*
 * Public API Functions
 */

/*
 * IHC_init()
 *
 * See miv_ihc.h or miv_ihc user guide for details of how to use this function.
 *
 */
void IHC_init(uint8_t channel)
{
    IHC_ASSERT(channel < MAX_CHANNELS);

    uint8_t module = (channel / CH_PER_MODULE);

    g_ihc.IHC_Channels[channel].HART_IHCC = (IHCC_IP_TypeDef *)ihc_base_addess[channel];
    g_ihc.IHC_Channels[channel].mp_callback_handler = NULL;
    g_ihc.IHC_Channels[channel].mc_callback_handler = NULL;
    g_ihc.IHC_Channels[channel].HART_IHCC->CTR_REG = 0U;

    if (g_ihc.IHCIM_CONFIG_STATUS[module] != IHCIM_CONFIGURED)
    {
        g_ihc.HART_IHCIM[module] = (IHCIM_IP_TypeDef *)IHCIM_base_addess[module];

        g_ihc.HART_IHCIM[module]->IRQ_MASK = MIV_IHC_REGS_IRQ_DISABLE_MASK;

        g_ihc.IHCIM_CONFIG_STATUS[module] = IHCIM_CONFIGURED;
    }

    static bool ip_version_configured = false;

    if (!ip_version_configured)
    {
        g_ihc.ip_version = (uint32_t *)IHC_IP_VERSION;
        ip_version_configured = true;
    }
}

/*
 * IHC_config_mp_callback_handler()
 *
 * See miv_ihc.h or miv_ihc user guide for details of how to use this function.
 *
 */
void IHC_config_mp_callback_handler(uint8_t channel, QUEUE_IHC_INCOMING handler)
{
    IHC_ASSERT(channel < MAX_CHANNELS);
    IHC_ASSERT(g_ihc.IHC_Channels[channel].HART_IHCC != NULL);

    g_ihc.IHC_Channels[channel].mp_callback_handler = handler;
}

/*
 * IHC_config_mc_callback_handler()
 *
 * See miv_ihc.h or miv_ihc user guide for details of how to use this function.
 *
 */
void IHC_config_mc_callback_handler(uint8_t channel, QUEUE_IHC_INCOMING handler)
{
    IHC_ASSERT(channel < MAX_CHANNELS);
    IHC_ASSERT(g_ihc.IHC_Channels[channel].HART_IHCC != NULL);

    g_ihc.IHC_Channels[channel].mc_callback_handler = handler;
}

/*
 * IHC_enable_mp_interrupt()
 *
 * See miv_ihc.h or miv_ihc user guide for details of how to use this function.
 *
 */
void IHC_enable_mp_interrupt(uint8_t channel)
{
    IHC_ASSERT(channel < MAX_CHANNELS);
    IHC_ASSERT(g_ihc.IHC_Channels[channel].HART_IHCC != NULL);

    g_ihc.IHC_Channels[channel].HART_IHCC->CTR_REG |= MIV_IHC_REGS_CH_CTRL_MPIE_MASK;

    /* enable Interrupt module */
    IHC_enable_mp_interrupt_ch_module(channel);
}

/*
 * IHC_enable_mc_interrupt()
 *
 * See miv_ihc.h or miv_ihc user guide for details of how to use this function.
 *
 */
void IHC_enable_mc_interrupt(uint8_t channel)
{
    IHC_ASSERT(channel < MAX_CHANNELS);
    IHC_ASSERT(g_ihc.IHC_Channels[channel].HART_IHCC != NULL);

    g_ihc.IHC_Channels[channel].HART_IHCC->CTR_REG |= MIV_IHC_REGS_CH_CTRL_ACKIE_MASK;

    /* enable Interrupt module */
    IHC_enable_cp_interrupt_ch_module(channel);
}

/*
 * IHC_disable_mp_interrupt()
 *
 * See miv_ihc.h or miv_ihc user guide for details of how to use this function.
 *
 */
void IHC_disable_mp_interrupt(uint8_t channel)
{
    IHC_ASSERT(channel < MAX_CHANNELS);
    IHC_ASSERT(g_ihc.IHC_Channels[channel].HART_IHCC != NULL);

    g_ihc.IHC_Channels[channel].HART_IHCC->CTR_REG &= ~(MIV_IHC_REGS_CH_CTRL_MPIE_MASK);
}

/*
 * IHC_disable_mc_interrupt()
 *
 * See miv_ihc.h or miv_ihc user guide for details of how to use this function.
 *
 */
void IHC_disable_mc_interrupt(uint8_t channel)
{
    IHC_ASSERT(channel < MAX_CHANNELS);
    IHC_ASSERT(g_ihc.IHC_Channels[channel].HART_IHCC != NULL);

    g_ihc.IHC_Channels[channel].HART_IHCC->CTR_REG &= ~(MIV_IHC_REGS_CH_CTRL_ACKIE_MASK);
}

static void IHC_enable_mp_interrupt_ch_module(uint8_t channel)
{
    IHC_ASSERT(channel < MAX_CHANNELS);
    IHC_ASSERT(g_ihc.IHC_Channels[channel].HART_IHCC != NULL);

    uint8_t module = channel / CH_PER_MODULE;
    uint8_t module_index = channel % CH_PER_MODULE;

    if (module_index >= module)
    {
        module_index++;
    }

    IHC_ASSERT(g_ihc.HART_IHCIM[module] != NULL);

    g_ihc.HART_IHCIM[module]->IRQ_MASK |= (1U << (module_index * 2U));
}

static void IHC_enable_cp_interrupt_ch_module(uint8_t channel)
{
    IHC_ASSERT(channel < MAX_CHANNELS);
    IHC_ASSERT(g_ihc.IHC_Channels[channel].HART_IHCC != NULL);

    uint8_t module = channel / CH_PER_MODULE;
    uint8_t module_index = channel % CH_PER_MODULE;

    if (module_index >= module)
    {
        module_index++;
    }

    IHC_ASSERT(g_ihc.HART_IHCIM[module] != NULL);

    g_ihc.HART_IHCIM[module]->IRQ_MASK |= (1U << ((module_index * 2U) + 1U));
}

void IHC_enable_mp_interrupt_module(uint8_t module)
{
    IHC_ASSERT(module < MAX_MODULE);
    IHC_ASSERT(g_ihc.HART_IHCIM[module] != NULL);

    g_ihc.HART_IHCIM[module]->IRQ_MASK = MIV_IHC_REGS_IRQ_ENABLE_MASK;
}

void IHC_enable_cp_interrupt_module(uint8_t module)
{
    IHC_ASSERT(module < MAX_MODULE);
    IHC_ASSERT(g_ihc.HART_IHCIM[module] != NULL);

    g_ihc.HART_IHCIM[module]->IRQ_MASK = MIV_IHC_REGS_IRQ_ENABLE_MASK;
}

void IHC_disable_mp_interrupt_module(uint8_t module)
{
    IHC_ASSERT(module < MAX_MODULE);
    IHC_ASSERT(g_ihc.HART_IHCIM[module] != NULL);

    g_ihc.HART_IHCIM[module]->IRQ_MASK = MIV_IHC_REGS_IRQ_DISABLE_MASK;
}

void IHC_disable_cp_interrupt_module(uint8_t module)
{
    IHC_ASSERT(module < MAX_MODULE);
    IHC_ASSERT(g_ihc.HART_IHCIM[module] != NULL);

    g_ihc.HART_IHCIM[module]->IRQ_MASK = MIV_IHC_REGS_IRQ_DISABLE_MASK;
}
/*
 * IHC_tx_message()
 *
 * See miv_ihc.h or miv_ihc user guide for details of how to use this function.
 *
 */
int8_t IHC_tx_message(uint8_t channel, const uint32_t* message, uint16_t msg_size)
{
    IHCC_IP_TypeDef *ihcc = (IHCC_IP_TypeDef *) g_ihc.IHC_Channels[channel].HART_IHCC;

    IHC_ASSERT(channel < MAX_CHANNELS);
    IHC_ASSERT(message != NULL);
    IHC_ASSERT(msg_size <= MAX_MSG_SIZE_IN_BYTES);
    IHC_ASSERT(ihcc != NULL);

    uint32_t ctr_reg = ihcc->CTR_REG;

    if (ctr_reg & MIV_IHC_REGS_CH_CTRL_RMP_MASK)
    {
        return IHC_MSG_BUSY;
    }

    if (ctr_reg & MIV_IHC_REGS_CH_CTRL_ACK_MASK)
    {
        return IHC_MSG_NT_CLR;
    }

    if (msg_size > 0)
    {
        for (uint32_t i = 0; i < msg_size; i++)
        {
            ihcc->MSG_OUT[i] = message[i];
        }

        // Set RMP to set MP at other end
        ihcc->CTR_REG |= MIV_IHC_REGS_CH_CTRL_RMP_MASK;

        return IHC_MSG_SUCCESS;
    }

    return IHC_MSG_INVALID;
}

/*
 * IHC_poll_msg_present_status()
 *
 * See miv_ihc.h or miv_ihc user guide for details of how to use this function.
 *
 */
int8_t IHC_poll_msg_present(uint8_t channel)
{
    IHCC_IP_TypeDef *ihcc = (IHCC_IP_TypeDef *) g_ihc.IHC_Channels[channel].HART_IHCC;

    IHC_ASSERT(channel < MAX_CHANNELS);
    IHC_ASSERT(ihcc != NULL);

    uint32_t ctr_reg = ihcc->CTR_REG;

    if (ihcc->CTR_REG & MIV_IHC_REGS_CH_CTRL_MP_MASK)
    {
        if (NULL != g_ihc.IHC_Channels[channel].mp_callback_handler)
        {
            return rx_message(channel,
                              g_ihc.IHC_Channels[channel].mp_callback_handler,
                              IHC_MSG_PRESENT);
        }
        else
        {
            /* Clear interrupt & Send Ack*/
            ihcc->CTR_REG =
                (ctr_reg & ~MIV_IHC_REGS_CH_CTRL_MP_MASK) | MIV_IHC_REGS_CH_CTRL_ACK_MASK;
            return IHC_CALLBACK_NOT_CONFIG;
        }
    }

    return IHC_NO_MSG;
}

/*
 * IHC_poll_msg_consumed_status()
 *
 * See miv_ihc.h or miv_ihc user guide for details of how to use this function.
 *
 */
int8_t IHC_poll_msg_consumed(uint8_t channel)
{
    IHCC_IP_TypeDef *ihcc = (IHCC_IP_TypeDef *) g_ihc.IHC_Channels[channel].HART_IHCC;

    IHC_ASSERT(channel < MAX_CHANNELS);
    IHC_ASSERT(ihcc != NULL);

    if (ihcc->CTR_REG & MIV_IHC_REGS_CH_CTRL_ACKCLR_MASK)
    {
        if (NULL != g_ihc.IHC_Channels[channel].mc_callback_handler)
        {
            return rx_message(channel,
                              g_ihc.IHC_Channels[channel].mc_callback_handler,
                              IHC_MSG_CONSUMED);
        }
        else
        {
            /* Clear interrupt */
            ihcc->CTR_REG &= ~(MIV_IHC_REGS_CH_CTRL_ACKCLR_MASK);

            return IHC_CALLBACK_NOT_CONFIG;
        }
    }

    return IHC_NO_MSG;
}

uint8_t IHC_APP_X_H0_IRQHandler(void)
{
    IHC_app_irq_handler(MIV_IHCIM_H0);
    return (EXT_IRQ_KEEP_ENABLED);
}

uint8_t IHC_APP_X_H1_IRQHandler(void)
{
    IHC_app_irq_handler(MIV_IHCIM_H1);
    return (EXT_IRQ_KEEP_ENABLED);
}

uint8_t IHC_APP_X_H2_IRQHandler(void)
{
    IHC_app_irq_handler(MIV_IHCIM_H2);
    return (EXT_IRQ_KEEP_ENABLED);
}

uint8_t IHC_APP_X_H3_IRQHandler(void)
{
    IHC_app_irq_handler(MIV_IHCIM_H3);
    return (EXT_IRQ_KEEP_ENABLED);
}

uint8_t IHC_APP_X_H4_IRQHandler(void)
{
    IHC_app_irq_handler(MIV_IHCIM_H4);
    return (EXT_IRQ_KEEP_ENABLED);
}

void IHC_app_irq_handler(uint8_t module_num)
{
    IHC_ASSERT(module_num < MAX_MODULE);
    IHC_ASSERT(g_ihc.HART_IHCIM[module_num] != NULL);

    uint32_t local_irq_status =
        g_ihc.HART_IHCIM[module_num]->IRQ_STATUS & MIV_IHC_REGS_IRQ_STATUS_NS_MASK;

    if (local_irq_status)
    {
        uint8_t channel_parse = 0u, channel_num = 0u;

        for (channel_parse = 0; channel_parse < MAX_MODULE; channel_parse++)
        {
            uint32_t msg_mask = 0x01u << (channel_parse * 2u);

            channel_num = (CH_PER_MODULE * module_num) + channel_parse;
            if (module_num <= channel_parse)
            {
                channel_num -= 1u;
            }

            if (local_irq_status & msg_mask)
            {
                // message present Interrupt
                message_present_isr(channel_num);
            }

            msg_mask <<= 1u;

            if (local_irq_status & msg_mask)
            {
                // message clear Interrupt
                message_consumed_isr(channel_num);
            }
        }
    }
    else
    {
        // False Interrupt
    }

}

int8_t IHC_indirect_irq_handler(uint8_t module_num, uint32_t *msg_buffer)
{
    IHC_ASSERT(module_num < MAX_MODULE);
    IHC_ASSERT(g_ihc.HART_IHCIM[module_num] != NULL);

    uint32_t local_irq_status =
        g_ihc.HART_IHCIM[module_num]->IRQ_STATUS & MIV_IHC_REGS_IRQ_STATUS_NS_MASK;

    int8_t msg_status = IHC_NO_MSG;

    if (local_irq_status)
    {
        uint8_t channel_parse = 0u, channel_num = 0u;

        for (channel_parse = 0; channel_parse < MAX_MODULE; channel_parse++)
        {
            uint32_t msg_mask = 0x01u << (channel_parse * 2u);

            channel_num = (CH_PER_MODULE * module_num) + channel_parse;
            if (module_num <= channel_parse)
            {
                channel_num -= 1u;
            }

            if (local_irq_status & msg_mask)
            {
                // message present Interrupt
                g_ihc.IHC_Channels[channel_num].ext_msg_ptr = msg_buffer;
                msg_status = message_present_isr(channel_num);
                

            }

            msg_mask <<= 1u;

            if (local_irq_status & msg_mask)
            {
                // message clear Interrupt
                g_ihc.IHC_Channels[channel_num].ext_msg_ptr = msg_buffer;
                msg_status = message_consumed_isr(channel_num);
            }
        }
    }
    else
    {
        // False Interrupt
        msg_status = IHC_NO_MSG;
    }

    return msg_status;
}

/******************************************************************************/

void IHC_CTRL_H0_H1_IRQHandler(void)
{
    IHC_ctlr_irqhandler(MIV_IHCIM_H1);
}

void IHC_CTRL_H0_H2_IRQHandler(void)
{
    IHC_ctlr_irqhandler(MIV_IHCIM_H2);
}

void IHC_CTRL_H0_H3_IRQHandler(void)
{
    IHC_ctlr_irqhandler(MIV_IHCIM_H3);
}

void IHC_CTRL_H0_H4_IRQHandler(void)
{
    IHC_ctlr_irqhandler(MIV_IHCIM_H4);
}


void IHC_ctlr_irqhandler(uint8_t module_num)
{
    IHC_ASSERT(module_num < MAX_MODULE);
    IHC_ASSERT(g_ihc.HART_IHCIM[module_num] != NULL);

    uint8_t channel_num = CH_PER_MODULE * module_num;
    uint32_t local_irq_status =
        g_ihc.HART_IHCIM[module_num]->IRQ_STATUS & MIV_IHC_REGS_IRQ_STATUS_NS_MASK;

    // Add loop to determine sender
    if (local_irq_status & MIV_IHC_REGS_MP_IRQ_STATUS_MASK)
    {
        message_present_isr(channel_num);
    }
    else if (local_irq_status & MIV_IHC_REGS_MC_IRQ_STATUS_MASK)
    {
        message_consumed_isr(channel_num);
    }
}

/*
 * message_present_isr()
 * This function is called on receipt of a mp interrupt in a bare metal system.
 * It parses the incoming message and calls the processing function
 * which ends up calling the registered application handler.
 * user registered function:
 * g_ihc[channel].msg_in_handler
 */
static int8_t message_present_isr(uint8_t channel)
{
    IHC_ASSERT(channel < MAX_CHANNELS);
    IHC_ASSERT(g_ihc.IHC_Channels[channel].HART_IHCC != NULL);

    if (NULL != g_ihc.IHC_Channels[channel].mp_callback_handler)
    {
        return rx_message(channel, g_ihc.IHC_Channels[channel].mp_callback_handler, IHC_MSG_PRESENT);
    }
    else
    {
        /* Clear interrupt */
        g_ihc.IHC_Channels[channel].HART_IHCC->CTR_REG &= ~(MIV_IHC_REGS_CH_CTRL_MP_MASK);
        return IHC_CALLBACK_NOT_CONFIG;
    }
}

/*
 * message_consumed_isr()
 * This function is called on receipt of a mc interrupt in a bare metal system.
 * It parses the incoming message and calls the processing function
 * which ends up calling the registered application handler.
 * user registered function:
 * g_ihc[channel].msg_in_handler
 */
static int8_t message_consumed_isr(uint8_t channel)
{
    IHC_ASSERT(channel < MAX_CHANNELS);
    IHC_ASSERT(g_ihc.IHC_Channels[channel].HART_IHCC != NULL);

    if (NULL != g_ihc.IHC_Channels[channel].mc_callback_handler)
    {
        return rx_message(channel, g_ihc.IHC_Channels[channel].mc_callback_handler, IHC_MSG_CONSUMED);
    }
    else
    {
        /* Clear interrupt */
        g_ihc.IHC_Channels[channel].HART_IHCC->CTR_REG &= ~(MIV_IHC_REGS_CH_CTRL_ACKCLR_MASK);
        return IHC_CALLBACK_NOT_CONFIG;
    }
}

/**
 * rx_message()
 * Called on receipt of message
 * @param channel
 * @param handle_incoming This is a point to a function that is provided by
 * upper layer. It will read/copy the incoming message.
 * @param msg_type This is inform if its message present or message consumed msg
 * @return
 */
static int8_t rx_message(uint8_t channel, QUEUE_IHC_INCOMING handle_incoming, uint8_t msg_type)
{
    IHCC_IP_TypeDef *ihcc = (IHCC_IP_TypeDef *) g_ihc.IHC_Channels[channel].HART_IHCC;
    //uint32_t *local_rx_msg = g_ihc.IHC_Channels[channel].rx_msg;
    uint32_t msg_size = (g_ihc.IHC_Channels[channel].HART_IHCC->MESSAGE_SIZE &
                         MIV_IHC_REGS_MESSAGE_SIZE_MESSAGE_IN_MASK);

    msg_size = (msg_size >> MIV_IHC_REGS_MESSAGE_SIZE_MESSAGE_IN_SHIFT);

    if (msg_type != IHC_MSG_CONSUMED && msg_type != IHC_MSG_PRESENT)
    {
        return IHC_MSG_INVALID;
    }

/*     for (uint32_t i = 0; i < msg_size; i++)
    {
        local_rx_msg[i] = ihcc->MSG_IN[i];
    } */

    handle_incoming(channel, (uint32_t *)&ihcc->MSG_IN[0], msg_size, g_ihc.IHC_Channels[channel].ext_msg_ptr);

    if (msg_type == IHC_MSG_CONSUMED)
    {
        ihcc->CTR_REG &= ~(MIV_IHC_REGS_CH_CTRL_ACKCLR_MASK); // clear msg

        return IHC_MSG_CLR;
    }
    else // IHC_MSG_PRESENT
    {
        // clear MP you should consume msg
        ihcc->CTR_REG &= ~(MIV_IHC_REGS_CH_CTRL_MP_MASK);

        // Send Ack
        ihcc->CTR_REG |= (MIV_IHC_REGS_CH_CTRL_ACK_MASK);

        return IHC_MSG_RECEIVED;
    }
}
