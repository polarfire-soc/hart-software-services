/*******************************************************************************
 * Copyright 2021-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC Microprocessor Subsystem Inter-Hart Communication bare metal software driver
 * implementation.
 *
 */
#include <string.h>
#include <stdio.h>
#include "mpfs_hal/mss_hal.h"
#include "miv_ihc.h"
#include "assert.h"

/******************************************************************************/
/* configuration           arrays populated from user defines                 */
/******************************************************************************/

IHC_TypeDef             IHC_H0_IP_GROUP ;
IHC_TypeDef             IHC_H1_IP_GROUP ;
IHC_TypeDef             IHC_H2_IP_GROUP ;
IHC_TypeDef             IHC_H3_IP_GROUP ;
IHC_TypeDef             IHC_H4_IP_GROUP ;

IHC_TypeDef * IHC[] = { &IHC_H0_IP_GROUP, &IHC_H1_IP_GROUP, &IHC_H2_IP_GROUP, &IHC_H3_IP_GROUP, &IHC_H4_IP_GROUP};

/**
 * \brief IHC configuration
 *
 */
const uint64_t ihc_base_address[][5U] = {
    /* hart 0 */
    {
        0x0,
        IHC_LOCAL_H0_REMOTE_H1,
        IHC_LOCAL_H0_REMOTE_H2,
        IHC_LOCAL_H0_REMOTE_H3,
        IHC_LOCAL_H0_REMOTE_H4
    },
    /* hart 1 */
    {
        IHC_LOCAL_H1_REMOTE_H0,
        0x0,
        IHC_LOCAL_H1_REMOTE_H2,
        IHC_LOCAL_H1_REMOTE_H3,
        IHC_LOCAL_H1_REMOTE_H4
    },
    /* hart 2 */
    {
        IHC_LOCAL_H2_REMOTE_H0,
        IHC_LOCAL_H2_REMOTE_H1,
        0x0,
        IHC_LOCAL_H2_REMOTE_H3,
        IHC_LOCAL_H2_REMOTE_H4
    },
    /* hart 3 */
    {
        IHC_LOCAL_H3_REMOTE_H0,
        IHC_LOCAL_H3_REMOTE_H1,
        IHC_LOCAL_H3_REMOTE_H2,
        0x0,
        IHC_LOCAL_H3_REMOTE_H4
    },
    /* hart 4 */
    {
        IHC_LOCAL_H4_REMOTE_H0,
        IHC_LOCAL_H4_REMOTE_H1,
        IHC_LOCAL_H4_REMOTE_H2,
        IHC_LOCAL_H4_REMOTE_H3,
        0x0
    },
};

/**
 * \brief IHC configuration
 *
 */
const uint64_t IHCIA_base_address[5U] = {
        IHCIA_LOCAL_H0,
        IHCIA_LOCAL_H1,
        IHCIA_LOCAL_H2,
        IHCIA_LOCAL_H3,
        IHCIA_LOCAL_H4
};

/**
 * \brief Remote harts connected via channel to a local hart
 *
 */
const uint32_t IHCIA_remote_harts[5U] = {
        IHCIA_H0_REMOTE_HARTS,
        IHCIA_H1_REMOTE_HARTS,
        IHCIA_H2_REMOTE_HARTS,
        IHCIA_H3_REMOTE_HARTS,
        IHCIA_H4_REMOTE_HARTS
};


/**
 * \brief Remote harts connected via channel to a local hart
 *
 */
const uint32_t IHCIA_remote_hart_ints[5U] = {
        IHCIA_H0_REMOTE_HARTS_INTS,
        IHCIA_H1_REMOTE_HARTS_INTS,
        IHCIA_H2_REMOTE_HARTS_INTS,
        IHCIA_H3_REMOTE_HARTS_INTS,
        IHCIA_H4_REMOTE_HARTS_INTS
};


/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
static uint32_t parse_incoming_hartid(uint32_t my_hart_id, bool *is_ack, bool polling);
static uint32_t rx_message(IHC_CHANNEL channel, QUEUE_IHC_INCOMING handle_incoming, bool is_ack, uint32_t * message_storage_ptr);
static void message_present_isr(void);

/******************************************************************************/
/* Public API Functions                                                       */
/******************************************************************************/

/**
 * Init MiV-IHC
 * This must be called from monitor hart ( needs access to all harts/registers )
 * It function is called before local init functions are called.
 * See miv_ihc.h or miv_ihc user guide for details of how to use this
 * function.
 *
 * @param none
 * @return none
 */
void IHC_global_init(void)
{
    uint32_t remote_hart_id;
    uint32_t my_hart_id = 0;

    while(my_hart_id < 5U)
    {
        remote_hart_id = 0U;

        while(remote_hart_id < 5U)
        {
            IHC[my_hart_id]->local_h_setup.msg_in_handler[remote_hart_id] = NULL;
            /*
             * Configure base addresses
             */
            IHC[my_hart_id]->HART_IHC[remote_hart_id] = (IHC_IP_TypeDef *)ihc_base_address[my_hart_id][remote_hart_id];
            IHC[my_hart_id]->HART_IHC[remote_hart_id]->CTR_REG.CTL_REG = 0U;
            remote_hart_id++;
        }
        /*
         * Configure base addresses
         */
        IHC[my_hart_id]->interrupt_concentrator = (IHCIA_IP_TypeDef *)IHCIA_base_address[my_hart_id];
        /*
         *
         */
        IHC[my_hart_id]->interrupt_concentrator->INT_EN.INT_EN          = 0x0U;
        my_hart_id++;
    }
}

/**
 * This is called from all the local harts using MiV-IHC
 *
 * See miv_ihc.h or miv_ihc user guide for details of how to use this
 * function.
 *
 * @param handler
 * @return
 */
uint8_t IHC_local_context_init(uint32_t hart_to_configure)
{
    uint8_t result = false;

    (void)result;

    {
        /*
         * Configure the base addresses in this hart context
         *
         */
        uint32_t remote_hart_id = 0U;

        while(remote_hart_id < 5U)
        {
            IHC[hart_to_configure]->local_h_setup.msg_in_handler[remote_hart_id] = NULL;
            /*
             * Configure base addresses
             */
            IHC[hart_to_configure]->HART_IHC[remote_hart_id] = (IHC_IP_TypeDef *)ihc_base_address[hart_to_configure][remote_hart_id];
            IHC[hart_to_configure]->HART_IHC[remote_hart_id]->CTR_REG.CTL_REG = 0U;
            remote_hart_id++;
        }
        /*
         * Configure base addresses
         */
        IHC[hart_to_configure]->interrupt_concentrator = (IHCIA_IP_TypeDef *)IHCIA_base_address[hart_to_configure];
        /*
         *
         */
        IHC[hart_to_configure]->interrupt_concentrator->INT_EN.INT_EN = 0x0U;
    }


    IHC[hart_to_configure]->local_h_setup.connected_harts = IHCIA_remote_harts[hart_to_configure];
    IHC[hart_to_configure]->local_h_setup.connected_hart_ints = IHCIA_remote_hart_ints[hart_to_configure];

    return(true);
}

/**
 * See miv_ihc.h or the miv_ihc user guide for details of how to use this
 * function.
 *
 */
uint8_t IHC_local_remote_config(uint32_t hart_to_configure, uint32_t remote_hart_id, QUEUE_IHC_INCOMING  handler, bool set_mpie_en, bool set_ack_en )
{
    uint8_t result = false;

    (void)result;

    /*
     * Set-up enables in concentrator
     */
    IHC[hart_to_configure]->local_h_setup.msg_in_handler[remote_hart_id] = handler;

    if(handler != NULL)
    {
    	IHC[hart_to_configure]->interrupt_concentrator->INT_EN.INT_EN = IHCIA_remote_hart_ints[hart_to_configure];
    }

    {
        if (set_mpie_en == true)
        {
            IHC[hart_to_configure]->HART_IHC[remote_hart_id]->CTR_REG.CTL_REG |= MPIE_EN;
        }
        if(set_ack_en == true)
        {
            IHC[hart_to_configure]->HART_IHC[remote_hart_id]->CTR_REG.CTL_REG |= ACKIE_EN;
        }
    }

    return(true);
}

/**
 * Send a message to another hart using MiV-IHC
 *
 * See miv_ihc.h or the miv_ihc user guide for details of how to use this
 * function.
 *
 * @param remote_hart_id
 * @param message
 * @return MP_BUSY / MESSAGE_SENT
 */
uint32_t IHC_tx_message(IHC_CHANNEL channel, uint32_t *message)
{
    uint32_t i, ret_value = MESSAGE_SENT;

    uint32_t my_hart_id = IHC_context_to_local_hart_id(channel);
    uint32_t remote_hart_id = IHC_context_to_remote_hart_id(channel);
    uint32_t message_size = IHC[my_hart_id]->HART_IHC[remote_hart_id]->size_msg;

    /*
     * return if RMP bit 1 indicating busy
     */
    if (RMP_MESSAGE_PRESENT == (IHC[my_hart_id]->HART_IHC[remote_hart_id]->CTR_REG.CTL_REG & RMP_MASK))
    {
        ret_value = MP_BUSY;
    }
    else if (ACK_INT_MASK == (IHC[my_hart_id]->HART_IHC[remote_hart_id]->CTR_REG.CTL_REG & ACK_INT_MASK))
    {
        ret_value = MP_BUSY;
    }
    else
    {
        /*
         * Fill the buffer
        */
        for(i = 0;i < message_size; i++)
        {
            IHC[my_hart_id]->HART_IHC[remote_hart_id]->mesg_out[i] = message[i];
        }
        /*
         * set the MP bit. This will notify other of incoming hart message
         */
        IHC[my_hart_id]->HART_IHC[remote_hart_id]->CTR_REG.CTL_REG |= RMP_MESSAGE_PRESENT;
        /*
         * report status
         */
        ret_value = MESSAGE_SENT;
    }

    return (ret_value);
}


/**
 * See miv_ihc.h or the miv_ihc user guide for details of how to use this
 * function.
 */
void IHC_message_present_poll(void)
{
    bool is_ack;
    uint64_t my_hart_id = read_csr(mhartid);
    /*
     * Check all our channels
     */
    uint32_t origin_hart = parse_incoming_hartid((uint32_t)my_hart_id, &is_ack, true);
    if(origin_hart != 99U)
    {
        /*
         * process incoming packet
         */
        rx_message(origin_hart, IHC[my_hart_id]->local_h_setup.msg_in_handler[origin_hart], is_ack, NULL );
        if(is_ack == true)
        {
            /* clear the ack */
            IHC[my_hart_id]->HART_IHC[origin_hart]->CTR_REG.CTL_REG &= ~ACK_CLR;
        }
    }
}

/**
 * See miv_ihc.h or the miv_ihc user guide for details of how to use this
 * function.
 */
void IHC_message_present_indirect_isr(uint32_t my_hart_id, uint32_t remote_channel, uint32_t * message_storage_ptr)
{
    bool is_ack;

    (void)remote_channel;

    /*
     * Check all our channels
     */
    uint32_t origin_hart = parse_incoming_hartid((uint32_t)my_hart_id, &is_ack, false);
    if(origin_hart != 99U)
    {
        /*
         * process incoming packet
         */
        rx_message(origin_hart, IHC[my_hart_id]->local_h_setup.msg_in_handler[origin_hart], is_ack, message_storage_ptr);
        if(is_ack == true)
        {
            /* clear the ack */
            IHC[my_hart_id]->HART_IHC[origin_hart]->CTR_REG.CTL_REG &= ~ACK_CLR;
        }
    }
}

/**
 * Returns local hart ID. If the channel is a context, it returns to lowest
 * hartID in the context.
 *
 * @param channel
 * @return
 */
uint32_t IHC_context_to_local_hart_id(IHC_CHANNEL channel)
{
    uint32_t hart = 0xFFU;
    uint32_t hart_idx = 0U;
    uint32_t harts_in_context = LIBERO_SETTING_CONTEXT_B_HART_EN;
    uint64_t my_hart_id = read_csr(mhartid);

    /*
     * If we are sending to a Context, assume we are a Context.
     * i.e. HSS will not send directly to a context
     */
    if(channel <= IHC_CHANNEL_TO_HART4)
    {
        hart = (uint32_t)my_hart_id;
    }
    else
    {
        if(channel == IHC_CHANNEL_TO_CONTEXTA)
        {
            /* we are context B */
            harts_in_context = LIBERO_SETTING_CONTEXT_B_HART_EN;
        }
        else
        {
            /* we are context A */
            harts_in_context = LIBERO_SETTING_CONTEXT_A_HART_EN;
        }

        hart_idx = 0U;
        while(hart_idx < 5U)
        {
            if  (harts_in_context & (1U<<hart_idx))
            {
                hart = hart_idx;
                break;
            }
            hart_idx++;
        }
    }

    return (hart);
}

/**
 * Converts a remote context to a hartID. If the channel is a context, it
 * returns to lowest hartID in the context.
 *
 * @param channel The remote channel we want to get hartID of.
 * @return
 */
uint32_t IHC_context_to_remote_hart_id(IHC_CHANNEL channel)
{
    uint32_t hart = 0xFFU;
    uint32_t hart_idx = 0U;
    uint32_t harts_in_context = LIBERO_SETTING_CONTEXT_B_HART_EN;

    if(channel <= IHC_CHANNEL_TO_HART4)
    {
        hart = channel;
    }
    else
    {
#ifndef LIBERO_SETTING_CONTEXT_A_HART_EN
#error "Use newer mss configurator to configure"
#else
        ASSERT(LIBERO_SETTING_CONTEXT_A_HART_EN > 0U);
        ASSERT(LIBERO_SETTING_CONTEXT_B_HART_EN > 0U);
        /* Determine context we are in */

        if(channel == IHC_CHANNEL_TO_CONTEXTA)
        {

           harts_in_context = LIBERO_SETTING_CONTEXT_A_HART_EN;
        }
        else
        {
            harts_in_context = LIBERO_SETTING_CONTEXT_B_HART_EN;
        }

        hart_idx = 0U;
        while(hart_idx < 5U)
        {
            if  (harts_in_context & (1U<<hart_idx))
            {
                hart = hart_idx;
                break;
            }
            hart_idx++;
        }
#endif
    }
    return (hart);
}

/**
 * See miv_ihc.h or miv_ihc user guide for details on this function.
 *
 * @return
 */
uint8_t IHCIA_hart0_IRQHandler(void)
{
    message_present_isr();
    return(EXT_IRQ_KEEP_ENABLED);
}

/**
 * See miv_ihc.h or miv_ihc user guide for details on this function.
 *
 * @return
 */
uint8_t IHCIA_hart1_IRQHandler(void)
{
    message_present_isr();
    return(EXT_IRQ_KEEP_ENABLED);
}

/**
 * See miv_ihc.h or miv_ihc user guide for details on this function.
 *
 * @return
 */
uint8_t IHCIA_hart2_IRQHandler(void)
{
    message_present_isr();
    return(EXT_IRQ_KEEP_ENABLED);
}

/**
 * See miv_ihc.h or miv_ihc user guide for details on this function.
 *
 * @return
 */
uint8_t IHCIA_hart3_IRQHandler(void)
{
    message_present_isr();
    return(EXT_IRQ_KEEP_ENABLED);
}

/**
 * See miv_ihc.h or miv_ihc user guide for details on this function.
 *
 * @return
 */
uint8_t IHCIA_hart4_IRQHandler(void)
{
    message_present_isr();
    return(EXT_IRQ_KEEP_ENABLED);
}


/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/

/**
 * This function is called on receipt of a MiV-IHCIA interrupt in a bare metal
 * system. It parses the incoming message and calls the processing function
 * which ends up calling the registered application handler.
 * user registered function:
 * IHC[my_hart_id]->local_h_setup.msg_in_handler[origin_hart]
 */
static void message_present_isr(void)
{
    bool is_ack;
    uint64_t my_hart_id = read_csr(mhartid);
    /*
     * Check all our channels
     */
    uint32_t origin_hart = parse_incoming_hartid((uint32_t)my_hart_id, &is_ack, false);
    if(origin_hart != 99U)
    {
        /*
         * process incoming packet
         */
        rx_message(origin_hart, IHC[my_hart_id]->local_h_setup.msg_in_handler[origin_hart], is_ack, NULL );
        if(is_ack == true)
        {
            /* clear the ack */
            IHC[my_hart_id]->HART_IHC[origin_hart]->CTR_REG.CTL_REG &= ~ACK_CLR;
        }
    }
    return;
}

/**
 * Called on receipt of message
 *
 * @param remote_hart_id
 * @param handle_incoming This is a point to a function that is provided by
 * upper layer. It will read/copy the incoming message.
 * @return
 */
static uint32_t rx_message(IHC_CHANNEL channel, QUEUE_IHC_INCOMING handle_incoming, bool is_ack, uint32_t * message_storage_ptr)
{

    uint32_t ret_value = NO_MESSAGE_RX;
    uint64_t my_hart_id = read_csr(mhartid);
    uint32_t remote_hart_id = IHC_context_to_remote_hart_id(channel);
    uint32_t message_size = IHC[my_hart_id]->HART_IHC[remote_hart_id]->size_msg;

    assert(handle_incoming);

    if (is_ack == true)
    {
        handle_incoming(remote_hart_id, (uint32_t *)&IHC[my_hart_id]->HART_IHC[remote_hart_id]->mesg_in[0U], message_size, is_ack, message_storage_ptr);
    }
    else if (MP_MESSAGE_PRESENT == (IHC[my_hart_id]->HART_IHC[remote_hart_id]->CTR_REG.CTL_REG & MP_MASK))
    {
        /*
         * check if we have a message
         */
        handle_incoming(remote_hart_id, (uint32_t *)&IHC[my_hart_id]->HART_IHC[remote_hart_id]->mesg_in[0U], message_size, is_ack, message_storage_ptr);

        {
            /*
             * set MP to 0
             * Note this generates an interrupt on the other hart if it has MPIE
             * bit set in the control register
             */

            volatile uint32_t temp = IHC[my_hart_id]->HART_IHC[remote_hart_id]->CTR_REG.CTL_REG & ~MP_MASK;
            /* Check if ACKIE_EN is set*/
            if(temp & ACKIE_EN)
            {
                temp |= ACK_INT;
            }
            IHC[my_hart_id]->HART_IHC[remote_hart_id]->CTR_REG.CTL_REG = temp;

            ret_value = MESSAGE_RX;
        }
    }
    else
    {
        /*
         * report status
         */
        ret_value = NO_MESSAGE_RX;
    }

    return (ret_value);
}

/**
 * Check where the message is coming from
 * @return returns hart ID of incoming message
 */
static uint32_t parse_incoming_hartid(uint32_t my_hart_id, bool *is_ack, bool polling)
{
    uint32_t hart_id = 0U;
    uint32_t return_hart_id = 99U;

    while(hart_id < 5U)
    {
        if (IHC[my_hart_id]->local_h_setup.connected_harts & (0x01U << hart_id))
        {
            uint32_t test_int = (0x01U << ((hart_id * 2) + 1));
            if(IHC[my_hart_id]->interrupt_concentrator->MSG_AVAIL_STAT.MSG_AVAIL & test_int)
            {
                if (polling == true)
                {
                    return_hart_id = hart_id;
                    *is_ack = true;
                    break;
                }
                else if(IHC[my_hart_id]->local_h_setup.connected_hart_ints & test_int)
                {
                    return_hart_id = hart_id;
                    *is_ack = true;
                    break;
                }
            }
            test_int = (0x01U << (hart_id * 2));
            if(IHC[my_hart_id]->interrupt_concentrator->MSG_AVAIL_STAT.MSG_AVAIL & test_int)
            {
                if (polling == true)
                {
                    return_hart_id = hart_id;
                    *is_ack = false;
                    break;
                }
                else if(((IHC[my_hart_id]->local_h_setup.connected_hart_ints & test_int) == test_int ) )
                {
                    return_hart_id = hart_id;
                    *is_ack = false;
                    break;
                }
            }
        }
        hart_id++;
    }
    return(return_hart_id);
}


