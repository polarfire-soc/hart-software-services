/*******************************************************************************
 * Copyright 2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC Microprocessor Subsystem Inter-Hart Communication bare metal
 * software driver implementation.
 *
 */
#include "mpfs_hal/mss_hal.h"
#include "miv_ihc.h"

/******************************************************************************/
/* defines                                                                    */
/******************************************************************************/

#define NO_CONTEXT_INCOMING_ACK_OR_DATA     99U
#define INVALID_HART_ID                   0xFFU

/******************************************************************************/
/* configuration           arrays populated from user defines                 */
/******************************************************************************/

IHC_TypeDef             IHC_H0_IP_GROUP ;
IHC_TypeDef             IHC_H1_IP_GROUP ;
IHC_TypeDef             IHC_H2_IP_GROUP ;
IHC_TypeDef             IHC_H3_IP_GROUP ;
IHC_TypeDef             IHC_H4_IP_GROUP ;

IHC_TypeDef * IHC[] = { &IHC_H0_IP_GROUP , &IHC_H1_IP_GROUP, &IHC_H2_IP_GROUP, &IHC_H3_IP_GROUP, &IHC_H4_IP_GROUP};

/**
 * \brief IHC configuration
 *
 */
const uint64_t ihc_base_address[][5U] = {
        /* hart 0 */
        {0x0,
        IHC_LOCAL_H0_REMOTE_H1,
        IHC_LOCAL_H0_REMOTE_H2,
        IHC_LOCAL_H0_REMOTE_H3,
        IHC_LOCAL_H0_REMOTE_H4},
        /* hart 1 */
        {IHC_LOCAL_H1_REMOTE_H0,
        0x0,
        IHC_LOCAL_H1_REMOTE_H2,
        IHC_LOCAL_H1_REMOTE_H3,
        IHC_LOCAL_H1_REMOTE_H4},
        /* hart 2 */
        {IHC_LOCAL_H2_REMOTE_H0,
        IHC_LOCAL_H2_REMOTE_H1,
        0x0,
        IHC_LOCAL_H2_REMOTE_H3,
        IHC_LOCAL_H2_REMOTE_H4},
        /* hart 3 */
        {IHC_LOCAL_H3_REMOTE_H0,
        IHC_LOCAL_H3_REMOTE_H1,
        IHC_LOCAL_H3_REMOTE_H2,
        0x0,
        IHC_LOCAL_H3_REMOTE_H4},
        /* hart 4 */
        {IHC_LOCAL_H4_REMOTE_H0,
        IHC_LOCAL_H4_REMOTE_H1,
        IHC_LOCAL_H4_REMOTE_H2,
        IHC_LOCAL_H4_REMOTE_H3,
        0x0},
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
static void  message_present_isr(void);
static uint32_t parse_incoming_hartid(uint32_t my_hart_id, bool *is_ack, bool polling);
static uint32_t parse_incoming_context_msg(uint32_t my_hart_id, uint32_t remote_hart_id, bool *is_ack, bool polling);
static uint32_t rx_message(uint32_t my_hart_id, uint32_t remote_hart_id, QUEUE_IHC_INCOMING handle_incoming, bool is_ack, uint32_t * message_storage_ptr);
static uint32_t lowest_hart_in_context(uint32_t mask);
/******************************************************************************/
/* Public API Functions                                                       */
/******************************************************************************/

/***************************************************************************//**
 * IHC_global_init()
 *
 * See miv_ihc.h for details of how to use this
 * function.
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
            IHC[my_hart_id]->HART_IHCC[remote_hart_id] = (IHCC_IP_TypeDef *)ihc_base_address[my_hart_id][remote_hart_id];
            if (NULL != IHC[my_hart_id]->HART_IHCC[remote_hart_id]) {
                IHC[my_hart_id]->HART_IHCC[remote_hart_id]->CTR_REG.CTL_REG = 0U;
            }
            remote_hart_id++;
        }
        /*
         * Configure base addresses
         */
        IHC[my_hart_id]->HART_IHCIA = (IHCIA_IP_TypeDef *)IHCIA_base_address[my_hart_id];
        /*
         * Clear interrupt enables
         */
        IHC[my_hart_id]->HART_IHCIA->INT_EN.INT_EN = 0x0U;
        my_hart_id++;
    }
}

/***************************************************************************//**
 * IHC_local_context_init()
 *
 * See miv_ihc.h or miv_ihc user guide for details of how to use this
 * function.
 */
void IHC_local_context_init(uint32_t hart_to_configure)
{

	ASSERT(hart_to_configure < 5U);

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
            IHC[hart_to_configure]->HART_IHCC[remote_hart_id] = (IHCC_IP_TypeDef *)ihc_base_address[hart_to_configure][remote_hart_id];
            if (NULL != IHC[hart_to_configure]->HART_IHCC[remote_hart_id]) {
                IHC[hart_to_configure]->HART_IHCC[remote_hart_id]->CTR_REG.CTL_REG = 0U;
            }
            remote_hart_id++;
        }
        /*
         * Configure base addresses
         */
        IHC[hart_to_configure]->HART_IHCIA = (IHCIA_IP_TypeDef *)IHCIA_base_address[hart_to_configure];
        /*
         *
         */
        IHC[hart_to_configure]->HART_IHCIA->INT_EN.INT_EN = 0x0U;
    }

    IHC[hart_to_configure]->local_h_setup.connected_harts = IHCIA_remote_harts[hart_to_configure];
    IHC[hart_to_configure]->local_h_setup.connected_hart_ints = IHCIA_remote_hart_ints[hart_to_configure];
}

/***************************************************************************//**
 * IHC_local_remote_config()
 *
 * See miv_ihc.h or miv_ihc user guide for details of how to use this
 * function.
 */
void IHC_local_remote_config(uint32_t hart_to_configure, uint32_t remote_hart_id, QUEUE_IHC_INCOMING  handler, bool set_mpie_en, bool set_ack_en )
{
    /*
     * Set-up enables in aggregator
     */
    IHC[hart_to_configure]->local_h_setup.msg_in_handler[remote_hart_id] = handler;

    if(handler != NULL)
    {
    	IHC[hart_to_configure]->HART_IHCIA->INT_EN.INT_EN = IHCIA_remote_hart_ints[hart_to_configure];
    }

	if (set_mpie_en == true)
	{
		IHC[hart_to_configure]->HART_IHCC[remote_hart_id]->CTR_REG.CTL_REG |= MPIE_EN;
	}
	if(set_ack_en == true)
	{
		IHC[hart_to_configure]->HART_IHCC[remote_hart_id]->CTR_REG.CTL_REG |= ACKIE_EN;
	}
}

/***************************************************************************//**
 * IHC_tx_message_from_context()
 *
 * See miv_ihc.h for details of how to use this
 * function.
 */
uint32_t IHC_tx_message_from_context(IHC_CHANNEL remote_channel, uint32_t *message)
{
    uint32_t i, ret_value = MESSAGE_SENT;

    uint32_t my_hart_id = IHC_partner_context_hart_id(remote_channel);
    uint32_t remote_hart_id = IHC_context_to_context_hart_id(remote_channel);
    uint32_t message_size = IHC[my_hart_id]->HART_IHCC[remote_hart_id]->size_msg;

    ASSERT(my_hart_id != INVALID_HART_ID);
    ASSERT(remote_hart_id != INVALID_HART_ID);

    /*
     * return if RMP bit 1 indicating busy
     */
    if (RMP_MESSAGE_PRESENT == (IHC[my_hart_id]->HART_IHCC[remote_hart_id]->CTR_REG.CTL_REG & RMP_MASK))
    {
        ret_value = MP_BUSY;
    }
    else if (ACK_INT_MASK == (IHC[my_hart_id]->HART_IHCC[remote_hart_id]->CTR_REG.CTL_REG & ACK_INT_MASK))
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
            IHC[my_hart_id]->HART_IHCC[remote_hart_id]->mesg_out[i] = message[i];
        }
        /*
         * set the MP bit. This will notify other of incoming hart message
         */
        IHC[my_hart_id]->HART_IHCC[remote_hart_id]->CTR_REG.CTL_REG |= RMP_MESSAGE_PRESENT;
        /*
         * report status
         */
        ret_value = MESSAGE_SENT;
    }

    return (ret_value);
}

/***************************************************************************//**
 * IHC_tx_message_from_hart()
 *
 * See miv_ihc.h for details of how to use this
 * function.
 */
uint32_t IHC_tx_message_from_hart(IHC_CHANNEL remote_channel, uint32_t *message)
{
    uint32_t i, ret_value = MESSAGE_SENT;

    uint64_t my_hart_id = read_csr(mhartid);
    uint32_t remote_hart_id = IHC_hart_to_context_or_hart_id(remote_channel);
    uint32_t message_size = IHC[my_hart_id]->HART_IHCC[remote_hart_id]->size_msg;

    /*
     * return if RMP bit 1 indicating busy
     */
    if (RMP_MESSAGE_PRESENT == (IHC[my_hart_id]->HART_IHCC[remote_hart_id]->CTR_REG.CTL_REG & RMP_MASK))
    {
        ret_value = MP_BUSY;
    }
    else if (ACK_INT_MASK == (IHC[my_hart_id]->HART_IHCC[remote_hart_id]->CTR_REG.CTL_REG & ACK_INT_MASK))
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
            IHC[my_hart_id]->HART_IHCC[remote_hart_id]->mesg_out[i] = message[i];
        }
        /*
         * set the MP bit. This will notify other of incoming hart message
         */
        IHC[my_hart_id]->HART_IHCC[remote_hart_id]->CTR_REG.CTL_REG |= RMP_MESSAGE_PRESENT;
        /*
         * report status
         */
        ret_value = MESSAGE_SENT;
    }

    return (ret_value);
}

/***************************************************************************//**
 * IHC_message_present_poll()
 *
 * See miv_ihc.h for details of how to use this
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
    if(origin_hart != NO_CONTEXT_INCOMING_ACK_OR_DATA)
    {
        /*
         * process incoming packet
         */
        rx_message((uint32_t)my_hart_id, origin_hart, IHC[my_hart_id]->local_h_setup.msg_in_handler[origin_hart], is_ack, NULL );
        if(is_ack == true)
        {
            /* clear the ack */
            IHC[my_hart_id]->HART_IHCC[origin_hart]->CTR_REG.CTL_REG &= ~ACK_CLR;
        }
    }
}

/***************************************************************************//**
 * IHC_context_indirect_isr()
 *
 * See miv_ihc.h for details of how to use this
 * function.
 */
void  IHC_context_indirect_isr(uint32_t * message_storage_ptr)
{
    bool is_ack;
    uint32_t my_context_hart_id;
    uint64_t my_hart_id = read_csr(mhartid);
    IHC_CHANNEL remote_channel;

    /*
     * Get the receiving context hart
     */
    my_context_hart_id = IHC_context_to_context_hart_id((uint32_t)my_hart_id);
    remote_channel = IHC_partner_context_hart_id(my_context_hart_id);

    if(IHC[my_context_hart_id]->HART_IHCC[0]->version <= 5U)
    {
        /* clear the ack and message present if HSS has not cleared */
        IHC[my_context_hart_id]->HART_IHCC[0]->CTR_REG.CTL_REG &= ~(ACK_CLR | MP_MASK) ;
    }

    if((my_context_hart_id == INVALID_HART_ID) || (remote_channel == INVALID_HART_ID))
    {
        /* HSS hart or no contexts setup */
        return;
    }

    /*
     * We know this routine is only called from a context receive, not the HSS.
     * So we can assume it is the other context is sending us something
     */
    uint32_t origin_hart = parse_incoming_context_msg((uint32_t)my_context_hart_id, remote_channel, &is_ack, false);
    if(origin_hart != NO_CONTEXT_INCOMING_ACK_OR_DATA)
    {
        /*
         * process incoming packet
         */
        rx_message(my_context_hart_id, origin_hart, IHC[my_context_hart_id]->local_h_setup.msg_in_handler[origin_hart], is_ack, message_storage_ptr );
        if(is_ack == true)
        {
            /* clear the ack */
            IHC[my_context_hart_id]->HART_IHCC[origin_hart]->CTR_REG.CTL_REG &= ~ACK_CLR;
        }
    }
    else
    {
        /*
         * nothing to do
         */
    }
}

/***************************************************************************//**
 * parse_incoming_context_msg()
 *
 * See miv_ihc.h for details of how to use this
 * function.
 */
static uint32_t parse_incoming_context_msg(uint32_t my_hart_id, uint32_t remote_hart_id, bool *is_ack, bool polling)
{
    uint32_t return_hart_id = NO_CONTEXT_INCOMING_ACK_OR_DATA;

    if(my_hart_id > 5U)
        return(return_hart_id);

    if (IHC[my_hart_id]->local_h_setup.connected_harts & (0x01U << remote_hart_id))
    {
        uint32_t test_int = (0x01U << ((remote_hart_id * 2) + 1));

        if(IHC[my_hart_id]->HART_IHCIA->MSG_AVAIL_STAT.MSG_AVAIL & test_int)
        {
            if (polling == true)
            {
                return_hart_id = remote_hart_id;
                *is_ack = true;
                return(return_hart_id);
            }
            else if(IHC[my_hart_id]->local_h_setup.connected_hart_ints & test_int)
            {
                return_hart_id = remote_hart_id;
                *is_ack = true;
                return(return_hart_id);
            }
        }
        test_int = (0x01U << (remote_hart_id * 2));
        if(IHC[my_hart_id]->HART_IHCIA->MSG_AVAIL_STAT.MSG_AVAIL & test_int)
        {
            if (polling == true)
            {
                return_hart_id = remote_hart_id;
                *is_ack = false;
                return(return_hart_id);
            }
            else if(((IHC[my_hart_id]->local_h_setup.connected_hart_ints & test_int) == test_int ) )
            {
                return_hart_id = remote_hart_id;
                *is_ack = false;
                return(return_hart_id);
            }
        }
    }

    return(return_hart_id);
}

/***************************************************************************//**
 * IHC_context_to_context_hart_id()
 *
 * See miv_ihc.h for details of how to use this
 * function.
 */
uint32_t IHC_context_to_context_hart_id(IHC_CHANNEL channel)
{
    uint32_t hart = INVALID_HART_ID;

    if(channel <= IHC_CHANNEL_TO_HART4)
    {
        if ( (1U<<channel) & LIBERO_SETTING_CONTEXT_A_HART_EN )
        {
            hart = lowest_hart_in_context(LIBERO_SETTING_CONTEXT_A_HART_EN);
        }
        else if ( (1U<<channel) & LIBERO_SETTING_CONTEXT_B_HART_EN )
        {
            hart = lowest_hart_in_context(LIBERO_SETTING_CONTEXT_B_HART_EN);
        }
    }
    else
    {
        /* Returns the lowest hart in a context */
        if(channel == IHC_CHANNEL_TO_CONTEXTA)
        {
            hart = lowest_hart_in_context(LIBERO_SETTING_CONTEXT_A_HART_EN);
        }
        else
        {
            hart = lowest_hart_in_context(LIBERO_SETTING_CONTEXT_B_HART_EN);
        }
    }
    return (hart);
}

/***************************************************************************//**
 * IHC_hart_to_context_or_hart_id()
 *
 * See miv_ihc.h for details of how to use this
 * function.
 */
uint32_t IHC_hart_to_context_or_hart_id(IHC_CHANNEL channel)
{
    uint32_t hart = INVALID_HART_ID;

    if(channel <= IHC_CHANNEL_TO_HART4)
    {
        hart = channel;
    }
    else
    {
        /* Returns the lowest hart in a context */
        if(channel == IHC_CHANNEL_TO_CONTEXTA)
        {
            hart = lowest_hart_in_context(LIBERO_SETTING_CONTEXT_A_HART_EN);
        }
        else
        {
            hart = lowest_hart_in_context(LIBERO_SETTING_CONTEXT_B_HART_EN);
        }
    }
    return (hart);
}

/**
 * IHC_partner_context_hart_id()
 * @param channel channel ID
 * @return hart ID of context ( the lowest hart ID in the associated context )
 */
uint32_t IHC_partner_context_hart_id(IHC_CHANNEL channel)
{
    uint32_t hart = INVALID_HART_ID;

    if(channel <= IHC_CHANNEL_TO_HART4)
    {
        if ((1U<<channel) & LIBERO_SETTING_CONTEXT_A_HART_EN)
        {
            hart = lowest_hart_in_context(LIBERO_SETTING_CONTEXT_B_HART_EN);
        }
        else if ((1U<<channel) & LIBERO_SETTING_CONTEXT_B_HART_EN)
        {
            hart = lowest_hart_in_context(LIBERO_SETTING_CONTEXT_A_HART_EN);
        }
    }
    else /* in case called using context ID */
    {
        if(channel == IHC_CHANNEL_TO_CONTEXTA)
        {
            hart = lowest_hart_in_context(LIBERO_SETTING_CONTEXT_B_HART_EN);
        }
        else
        {
            hart = lowest_hart_in_context(LIBERO_SETTING_CONTEXT_A_HART_EN);
        }
    }

    return (hart);
}

/***************************************************************************//**
 * IHCIA_hart0_IRQHandler()
 *
 * See miv_ihc.h for details of how to use this
 * function.
 */
uint8_t IHCIA_hart0_IRQHandler(void)
{
    message_present_isr();
    return(EXT_IRQ_KEEP_ENABLED);
}

/***************************************************************************//**
 * IHCIA_hart1_IRQHandler()
 *
 * See miv_ihc.h for details of how to use this
 * function.
 */
uint8_t IHCIA_hart1_IRQHandler(void)
{
    message_present_isr();
    return(EXT_IRQ_KEEP_ENABLED);
}

/***************************************************************************//**
 * IHCIA_hart2_IRQHandler()
 *
 * See miv_ihc.h for details of how to use this
 * function.
 */
uint8_t IHCIA_hart2_IRQHandler(void)
{
    message_present_isr();
    return(EXT_IRQ_KEEP_ENABLED);
}

/***************************************************************************//**
 * IHCIA_hart3_IRQHandler()
 *
 * See miv_ihc.h for details of how to use this
 * function.
 */
uint8_t IHCIA_hart3_IRQHandler(void)
{
    message_present_isr();
    return(EXT_IRQ_KEEP_ENABLED);
}

/***************************************************************************//**
 * IHCIA_hart4_IRQHandler()
 *
 * See miv_ihc.h for details of how to use this
 * function.
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
 * message_present_isr()
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
    if(origin_hart != NO_CONTEXT_INCOMING_ACK_OR_DATA)
    {
        /*
         * process incoming packet
         */
        rx_message((uint32_t)my_hart_id, origin_hart, IHC[my_hart_id]->local_h_setup.msg_in_handler[origin_hart], is_ack, NULL );
        if(is_ack == true)
        {
            /* clear the ack */
            IHC[my_hart_id]->HART_IHCC[origin_hart]->CTR_REG.CTL_REG &= ~ACK_CLR;
        }
    }
}

/**
 * rx_message()
 * Called on receipt of message
 * @param remote_hart_id
 * @param handle_incoming This is a point to a function that is provided by
 * upper layer. It will read/copy the incoming message.
 * @return
 */
static uint32_t rx_message(uint32_t my_hart_id, uint32_t remote_hart_id, QUEUE_IHC_INCOMING handle_incoming, bool is_ack, uint32_t * message_storage_ptr)
{
    uint32_t ret_value = NO_MESSAGE_RX;
    uint32_t message_size = IHC[my_hart_id]->HART_IHCC[remote_hart_id]->size_msg;

    if (is_ack == true)
    {
        handle_incoming(remote_hart_id, (uint32_t *)&IHC[my_hart_id]->HART_IHCC[remote_hart_id]->mesg_in[0U], message_size, is_ack, message_storage_ptr);
    }
    else if (MP_MESSAGE_PRESENT == (IHC[my_hart_id]->HART_IHCC[remote_hart_id]->CTR_REG.CTL_REG & MP_MASK))
    {
        /*
         * check if we have a message
         */
        handle_incoming(remote_hart_id, (uint32_t *)&IHC[my_hart_id]->HART_IHCC[remote_hart_id]->mesg_in[0U], message_size, is_ack, message_storage_ptr);
        {
            /*
             * set MP to 0
             * Note this generates an interrupt on the other hart if it has RMPIE
             * bit set in the control register
             */

            volatile uint32_t temp = IHC[my_hart_id]->HART_IHCC[remote_hart_id]->CTR_REG.CTL_REG & ~MP_MASK;
            /* Check if ACKIE_EN is set*/
            if(temp & ACKIE_EN)
            {
                temp |= ACK_INT;
            }
            IHC[my_hart_id]->HART_IHCC[remote_hart_id]->CTR_REG.CTL_REG = temp;

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
 * parse_incoming_hartid()
 * determine origin hartID
 * @param my_hart_id my hart id
 * @param is_ack Are we an ack?
 * @param polling Are we polling true/false
 * @return returns hart ID of incoming message
 */
static uint32_t parse_incoming_hartid(uint32_t my_hart_id, bool *is_ack, bool polling)
{
    uint32_t hart_id = 0U;
    uint32_t return_hart_id = NO_CONTEXT_INCOMING_ACK_OR_DATA;

    while(hart_id < 5U)
    {
        if (IHC[my_hart_id]->local_h_setup.connected_harts & (0x01U << hart_id))
        {
            uint32_t test_int = (0x01U << ((hart_id * 2) + 1));

            if(IHC[my_hart_id]->HART_IHCIA->MSG_AVAIL_STAT.MSG_AVAIL & test_int)
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
            if(IHC[my_hart_id]->HART_IHCIA->MSG_AVAIL_STAT.MSG_AVAIL & test_int)
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

/***************************************************************************//**
 * lowest_hart_in_context based on Libero settings()
 *
 * See miv_ihc.h for details of how to use this
 * function.
 */
static uint32_t lowest_hart_in_context(uint32_t mask)
{
    uint32_t lowest_hart = 0u;

    if(mask == LIBERO_SETTING_CONTEXT_A_HART_EN )
    {
        lowest_hart = (uint32_t) __builtin_ffs(LIBERO_SETTING_CONTEXT_A_HART_EN);
    }
    else
    {
        lowest_hart = (uint32_t) __builtin_ffs(LIBERO_SETTING_CONTEXT_B_HART_EN);
    }

    if(lowest_hart)
        return lowest_hart - 1u;
    else
        return 0u;
}
