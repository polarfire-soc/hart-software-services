/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC (MPFS) microprocessor subsystem CAN bare metal software driver
 * implementation.
 *
 * SVN $Revision$
 * SVN $Date$
 */

/*******************************************************************************
 * include files
 */
#include <stddef.h>
#include <stdint.h>
#include "mss_can.h"
#include "mss_sysreg.h"
#include "mss_plic.h"
#include "mss_util.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Macros
 */
#define    CAN_ID_SHIFT                  18u
#define    CAN_ERROR_STATUS_SHIFT        16u
#define    CAN_ERROR_STATUS_MASK         0x03u
#define    CAN_RX_GTE96_SHIFT            19u
#define    CAN_FLAG_MASK                 0x01u
#define    CAN_ERROR_COUNT_SHIFT         8u
#define    CAN_ERROR_COUNT_MASK          0xFFu
#define    CAN_TXGTE96_SHIFT             18u
#define    CAN_INT_MASK                  0xFFFFFFFCu
#define    ENABLE                        1u
#define    DISABLE                       0u
#define    SYSREG_CAN_SOFTRESET_MASK     (uint32_t)(3 << 14u)

/*******************************************************************************
 * Instance definition
 */
mss_can_instance_t g_mss_can_0_lo;
mss_can_instance_t g_mss_can_1_lo;
mss_can_instance_t g_mss_can_0_hi;
mss_can_instance_t g_mss_can_1_hi;

static void global_init
(
    mss_can_instance_t* this_wd
);

/***************************************************************************//**
 * MSS_CAN_init()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_init
(
    mss_can_instance_t* this_can,
    uint32_t bitrate,
    pmss_can_config_reg pcan_config,
    uint8_t basic_can_rx_mb,
    uint8_t basic_can_tx_mb
)
{
    uint32_t temp;
    uint8_t mailbox_number;
    uint8_t ret_value;
    mss_can_rxmsgobject canrxobj;

    global_init(this_can);

    /* Initialize the device structure */
    this_can->basic_can_rx_mb = basic_can_rx_mb;
    this_can->basic_can_tx_mb = basic_can_tx_mb;

    /* Initialize the rx mailbox */
    canrxobj.ID = 0u;
    canrxobj.DATAHIGH = 0u;
    canrxobj.DATALOW = 0u;
    canrxobj.AMR.L = 0u;
    canrxobj.ACR.L = 0u;
    canrxobj.AMR_D = 0u;
    canrxobj.ACR_D = 0u;
    canrxobj.RXB.L = (0u | CAN_RX_WPNH_EBL | CAN_RX_WPNL_EBL);

    for (mailbox_number = 0u; mailbox_number < CAN_RX_MAILBOX; mailbox_number++)
    {
        ret_value = MSS_CAN_config_buffer_n(this_can, mailbox_number,
                                            &canrxobj);
    }

    /* Configure CAN controller */
    if (CAN_SPEED_MANUAL == bitrate)
    {
        /*
         * If user wants to specify registers directly  Check if parameters
         * meet minimums.
         */
        if (pcan_config->CFG_TSEG1 < 2u)
        {
            return (CAN_TSEG1_TOO_SMALL );
        }

        if ((pcan_config->CFG_TSEG2 == 0u) ||
            ((pcan_config->SAMPLING_MODE == 1u) && (pcan_config->CFG_TSEG2
             == 1u)))
        {
            return (CAN_TSEG2_TOO_SMALL);
        }
        temp = pcan_config->CFG_SJW;
        if ((temp > pcan_config->CFG_TSEG1) ||
            (temp > pcan_config->CFG_TSEG2))
        {
            return (CAN_SJW_TOO_BIG);
        }

        this_can->hw_reg->Config.L = pcan_config->L;
    }
    else
    {
        /* User has chosen a default setting. */
        this_can->hw_reg->Config.L = bitrate;
    }

    /* Disable Interrupts */
    this_can->hw_reg->IntEbl.L = DISABLE;

    return (CAN_OK);
}

/***************************************************************************//**
 * MSS_CAN_set_config_reg()
 * See "mss_can.h" for details of how to use this function.
 */
void
MSS_CAN_set_config_reg
(
    mss_can_instance_t* this_can,
    uint32_t cfg
)
{
    /* Clear all pending interrupts */
    this_can->hw_reg->IntStatus.L = DISABLE;

    /* Disable CAN Device */
    this_can->hw_reg->Command.RUN_STOP = DISABLE;

    /* Disable receive interrupts. */
    this_can->hw_reg->IntEbl.RX_MSG = DISABLE;

    /* Disable interrupts from CAN device. */
    this_can->hw_reg->IntEbl.INT_EBL = DISABLE;

    /* Sets configuration bits */
    this_can->hw_reg->Config.L = cfg;
    MSS_CAN_start(this_can);
}

/***************************************************************************//**
 * MSS_CAN_set_mode()
 * See "mss_can.h" for details of how to use this function.
 */
void
MSS_CAN_set_mode
(
    mss_can_instance_t* this_can,
    mss_can_mode_t mode
)
{
    this_can->hw_reg->Command.RUN_STOP = DISABLE;
    if (CANOP_SW_RESET == mode)
    {
        SYSREG->SOFT_RESET_CR |= SYSREG_CAN_SOFTRESET_MASK;
        SYSREG->SOFT_RESET_CR &= ~SYSREG_CAN_SOFTRESET_MASK;
    }
    else
    {
        this_can->hw_reg->Command.L = (uint32_t)mode;
    }
}

/***************************************************************************//**
 * MSS_CAN_start()
 * See "mss_can.h" for details of how to use this function.
 */
void
MSS_CAN_start
(
    mss_can_instance_t* this_can
)
{
    /* Clear all pending interrupts*/
    this_can->hw_reg->IntStatus.L = DISABLE;

    /* Enable CAN Device*/
    this_can->hw_reg->Command.RUN_STOP = ENABLE;

    /* Enable CAN Interrupt at NVIC level- if supported */
#ifdef MSS_CAN_ENABLE_INTERRUPTS
    if (if (&g_mss_can_0_lo == this_can) || (&g_mss_can_0_hi == this_can))
    {
        PLIC_DisableIRQ(CAN0_PLIC);
    }
    else
    {
        PLIC_DisableIRQ(CAN1_PLIC);
    }
#endif

    /* Enable receive interrupts. */
    this_can->hw_reg->IntEbl.RX_MSG = ENABLE;

    /* Enable interrupts from CAN device.*/
    this_can->hw_reg->IntEbl.INT_EBL = ENABLE;

}

/***************************************************************************//**
 * MSS_CAN_stop()
 * See "mss_can.h" for details of how to use this function.
 */
void
MSS_CAN_stop
(
    mss_can_instance_t* this_can
)
{
    this_can->hw_reg->Command.RUN_STOP = DISABLE;
}

/***************************************************************************//**
 * MSS_CAN_get_id()
 * See "mss_can.h" for details of how to use this function.
 */
uint32_t
MSS_CAN_get_id
(
    pmss_can_msgobject pmsg
)
{
    if (pmsg->IDE)
    {
        return (pmsg->ID);
    }
    else
    {
        return (pmsg->ID >> CAN_ID_SHIFT);
    }
}

/***************************************************************************//**
 * MSS_CAN_set_id()
 * See "mss_can.h" for details of how to use this function.
 */
uint32_t
MSS_CAN_set_id
(
    pmss_can_msgobject pmsg
)
{
    if (pmsg->IDE)
    {
        return (pmsg->ID);
    }
    else
    {
        return (pmsg->ID << CAN_ID_SHIFT);
    }
}

/***************************************************************************//**
 * MSS_CAN_get_msg_filter_mask()
 * See "mss_can.h" for details of how to use this function.
 */
uint32_t
MSS_CAN_get_msg_filter_mask
(
    uint32_t id,
    uint8_t ide,
    uint8_t rtr
)
{
    if (ide)
    {
        id <<= 3u;
    }
    else
    {
        id <<= 21;

        /* Set unused ID bits to 1! */
        id |= (0x3FFFF << 3u);
    }
    id |= ((uint32_t)(ide << 2u) | (uint32_t)(rtr << 1u));

    return (id);
}

/***************************************************************************//**
 * MSS_CAN_set_int_ebl()
 * See "mss_can.h" for details of how to use this function.
 */
void
MSS_CAN_set_int_ebl
(
    mss_can_instance_t* this_can,
    uint32_t irq_flag
)
{
    this_can->hw_reg->IntEbl.L |= irq_flag;
}

/***************************************************************************//**
 * MSS_CAN_clear_int_ebl()
 * See "mss_can.h" for details of how to use this function.
 */
void
MSS_CAN_clear_int_ebl
(
    mss_can_instance_t* this_can,
    uint32_t irq_flag
)
{
    this_can->hw_reg->IntEbl.L &= ~irq_flag;
}

/***************************************************************************//**
 * MSS_CAN_get_global_int_ebl()
 * See "mss_can.h" for details of how to use this function.
 */
uint32_t
MSS_CAN_get_global_int_ebl
(
    mss_can_instance_t* this_can
)
{
    return (this_can->hw_reg->IntEbl.INT_EBL);
}

/***************************************************************************//**
 * MSS_CAN_get_int_ebl()
 * See "mss_can.h" for details of how to use this function.
 */
uint32_t
MSS_CAN_get_int_ebl
(
    mss_can_instance_t* this_can
)
{
    return (this_can->hw_reg->IntEbl.L & CAN_INT_MASK);
}

/***************************************************************************//**
 * MSS_CAN_clear_int_status()
 * See "mss_can.h" for details of how to use this function.
 */
void
MSS_CAN_clear_int_status
(
    mss_can_instance_t* this_can,
    uint32_t irq_flag
)
{
    this_can->hw_reg->IntStatus.L = irq_flag;
}

/***************************************************************************//**
 * MSS_CAN_get_int_status()
 * See "mss_can.h" for details of how to use this function.
 */
uint32_t
MSS_CAN_get_int_status
(
    mss_can_instance_t* this_can
)
{
    return (this_can->hw_reg->IntStatus.L);
}

/***************************************************************************//**
 * MSS_CAN_set_rtr_message_n()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_set_rtr_message_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number,
    pmss_can_msgobject pmsg
)
{
    /* Is buffer configured for Full CAN? */
    if (mailbox_number >= (CAN_RX_MAILBOX - this_can->basic_can_rx_mb))
    {
        return (CAN_BASIC_CAN_MAILBOX);
    }

    /* Is buffer configured for RTR auto-replay? */
    if (this_can->hw_reg->RxMsg[mailbox_number].RXB.RTRREPLY == 0u)
    {
        return (CAN_NO_RTR_MAILBOX);
    }
    else
    {
        /* Transfer the ID. */
        this_can->hw_reg->RxMsg[mailbox_number].ID = pmsg->ID;
        this_can->hw_reg->RxMsg[mailbox_number].DATALOW = pmsg->DATALOW;
        this_can->hw_reg->RxMsg[mailbox_number].DATAHIGH = pmsg->DATAHIGH;
        return (CAN_OK);
    }
}

/***************************************************************************//**
 * MSS_CAN_get_rtr_message_abort_n()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_get_rtr_message_abort_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number
)
{
    /* Is buffer configured for Full CAN? */
    if (mailbox_number >= (CAN_RX_MAILBOX - this_can->basic_can_rx_mb))
    {
        /* Mailbox is configured for basic CAN */
        return (CAN_BASIC_CAN_MAILBOX);
    }

    /* Set abort request */
    this_can->hw_reg->RxMsg[mailbox_number].RXB.RTRABORT = 1u;

    /* Check the abort is granted */
    if (this_can->hw_reg->RxMsg[mailbox_number].RXB.RTRREPLYPEND == 0u)
    {
        /* If the RX buffer isn't busy. Abort was successful */
        return (CAN_OK);
    }
    else
    {
        /* Message not aborted.*/
        return (CAN_ERR);
    }
}

/***************************************************************************//**
 * MSS_CAN_config_buffer()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_config_buffer
(
    mss_can_instance_t* this_can,
    pmss_can_filterobject pfilter
)
{
    uint8_t success = CAN_NO_MSG;
    uint8_t mailbox_number;

    /* Is a buffer configured for Basic CAN? */
    if (this_can->basic_can_rx_mb == 0u)
    {
        return (CAN_INVALID_MAILBOX);
    }

    /* Find next BASIC CAN buffer that has a message available */
    for (mailbox_number = CAN_RX_MAILBOX - this_can->basic_can_rx_mb;  \
                              mailbox_number < CAN_RX_MAILBOX; mailbox_number++)
    {
        /* Set filters */
        this_can->hw_reg->RxMsg[mailbox_number].ACR.L = pfilter->ACR.L;
        this_can->hw_reg->RxMsg[mailbox_number].AMR.L = pfilter->AMR.L;
        this_can->hw_reg->RxMsg[mailbox_number].AMR_D = pfilter->AMCR_D.MASK;
        this_can->hw_reg->RxMsg[mailbox_number].ACR_D = pfilter->AMCR_D.CODE;

        /* Configure mailbox */
        if (mailbox_number < (CAN_RX_MAILBOX - 1))
        {
            /* set link flag, if not last buffer */
            this_can->hw_reg->RxMsg[mailbox_number].RXB.L =
                                          (CAN_RX_WPNH_EBL | CAN_RX_WPNL_EBL | \
                                          CAN_RX_BUFFER_EBL | CAN_RX_INT_EBL | \
                                          CAN_RX_LINK_EBL);
        }
        else
        {
            this_can->hw_reg->RxMsg[mailbox_number].RXB.L =
                                          (CAN_RX_WPNH_EBL | CAN_RX_WPNL_EBL | \
                                          CAN_RX_BUFFER_EBL | CAN_RX_INT_EBL);
        }
        success = CAN_OK;
    }
    return (success);
}

/***************************************************************************//**
 * MSS_CAN_config_buffer_n()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_config_buffer_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number,
    pmss_can_rxmsgobject pmsg
)
{
    /* Is buffer configured for Full CAN? */
    if (mailbox_number >= (CAN_RX_MAILBOX - this_can->basic_can_rx_mb))
    {
        return (CAN_BASIC_CAN_MAILBOX);
    }

    /* Configure mailbox */
    this_can->hw_reg->RxMsg[mailbox_number].ID = pmsg->ID;
    this_can->hw_reg->RxMsg[mailbox_number].DATALOW = pmsg->DATALOW;
    this_can->hw_reg->RxMsg[mailbox_number].DATAHIGH = pmsg->DATAHIGH;
    this_can->hw_reg->RxMsg[mailbox_number].ACR.L = pmsg->ACR.L;
    this_can->hw_reg->RxMsg[mailbox_number].AMR.L = pmsg->AMR.L;
    this_can->hw_reg->RxMsg[mailbox_number].AMR_D = pmsg->AMR_D;
    this_can->hw_reg->RxMsg[mailbox_number].ACR_D = pmsg->ACR_D;
    this_can->hw_reg->RxMsg[mailbox_number].RXB.L = (pmsg->RXB.L | \
                                           CAN_RX_WPNH_EBL | CAN_RX_WPNL_EBL | \
                                           CAN_RX_BUFFER_EBL | CAN_RX_INT_EBL);
    return (CAN_OK);
}

/***************************************************************************//**
 * MSS_CAN_get_message_n()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_get_message_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number,
    pmss_can_msgobject pmsg
)
{
    /* Is buffer configured for Full CAN? */
    if (mailbox_number >= (CAN_RX_MAILBOX - this_can->basic_can_rx_mb))
    {
        return (CAN_BASIC_CAN_MAILBOX);
    }

    /* Check that a new message is available and get it */
    if ((ENABLE == this_can->hw_reg->Command.RUN_STOP) &&
        (this_can->hw_reg->RxMsg[mailbox_number].RXB.MSGAV))
    {
        /* Copy ID */
        pmsg->ID = this_can->hw_reg->RxMsg[mailbox_number].ID;

        /* Copy 4 of the data bytes */
        pmsg->DATALOW = this_can->hw_reg->RxMsg[mailbox_number].DATALOW;

        /* Copy the other 4 data bytes. */
        pmsg->DATAHIGH = this_can->hw_reg->RxMsg[mailbox_number].DATAHIGH;

        /* Get DLC, IDE and RTR and time stamp. */
        pmsg->L = this_can->hw_reg->RxMsg[mailbox_number].RXB.L;

        /* Ack that it's been removed from the FIFO */
        this_can->hw_reg->RxMsg[mailbox_number].RXB.MSGAV = ENABLE;

        /* And let app know there is a message. */
        return (CAN_VALID_MSG);
    }
    else
    {
        return (CAN_NO_MSG);
    }
}

/*******************************************************************************
 * MSS_CAN_get_message()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_get_message
(
    mss_can_instance_t* this_can,
    pmss_can_msgobject pmsg
)
{
    uint8_t success = CAN_NO_MSG;
    uint8_t mailbox_number;

    /* Is a buffer configured for Basic CAN? */
    if (this_can->basic_can_rx_mb == 0u)
    {
        return (CAN_INVALID_MAILBOX);
    }

    /* Find next BASIC CAN buffer that has a message available */
    for (mailbox_number = CAN_RX_MAILBOX-this_can->basic_can_rx_mb;  \
                             mailbox_number < CAN_RX_MAILBOX; mailbox_number++)
    {
        /* Check that if there is a valid message */
        if (this_can->hw_reg->RxMsg[mailbox_number].RXB.MSGAV)
        {
            /* Copy ID */
            pmsg->ID = this_can->hw_reg->RxMsg[mailbox_number].ID;

            /* Copy 4 of the data bytes */
            pmsg->DATALOW = this_can->hw_reg->RxMsg[mailbox_number].DATALOW;

            /* Copy the other 4 data bytes.*/
            pmsg->DATAHIGH = this_can->hw_reg->RxMsg[mailbox_number].DATAHIGH;

            /* Get DLC, IDE and RTR and time stamp.*/
            pmsg->L = this_can->hw_reg->RxMsg[mailbox_number].RXB.L;

            /* Ack that it's been removed from the FIFO */
            this_can->hw_reg->RxMsg[mailbox_number].RXB.MSGAV = ENABLE;
            success = CAN_VALID_MSG;
            break;
        }
    }
    return (success);
}

/***************************************************************************//**
 * MSS_CAN_get_message_av()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_get_message_av
(
    mss_can_instance_t* this_can
)
{
    uint8_t success = CAN_NO_MSG;
    uint8_t mailbox_number;

    /* Is a buffer configured for Basic CAN? */
    if (this_can->basic_can_rx_mb == 0u)
    {
        return (CAN_INVALID_MAILBOX);
    }

    /* Find next BASIC CAN buffer that has a message available */
    for (mailbox_number = CAN_RX_MAILBOX-this_can->basic_can_rx_mb;  \
                             mailbox_number < CAN_RX_MAILBOX; mailbox_number++)
    {
        /* Check that buffer is enabled and contains a message */
        if (this_can->hw_reg->RxMsg[mailbox_number].RXB.MSGAV)
        {
            success = CAN_VALID_MSG;
            break;
        }
    }
    return (success);
}

/***************************************************************************//**
 * MSS_CAN_send_message_n()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_send_message_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number,
    pmss_can_msgobject pmsg
)
{
    /* Can't send if device is disabled */
    if (DISABLE == this_can->hw_reg->Command.RUN_STOP)
    {
        /* Message not sent. */
        return (CAN_NO_MSG);
    }

    /* Is buffer configured for Full CAN? */
    if (mailbox_number >= (CAN_TX_MAILBOX - this_can->basic_can_tx_mb))
    {
        /* mailbox is configured for basic CAN */
        return (CAN_BASIC_CAN_MAILBOX);
    }

    if (this_can->hw_reg->TxMsg[mailbox_number].TXB.TXREQ == 0u)
    {
        /* If the Tx buffer isn't busy.... */
        this_can->hw_reg->TxMsg[mailbox_number].ID = pmsg->ID;
        this_can->hw_reg->TxMsg[mailbox_number].DATALOW = pmsg->DATALOW;
        this_can->hw_reg->TxMsg[mailbox_number].DATAHIGH = pmsg->DATAHIGH;
        this_can->hw_reg->TxMsg[mailbox_number].TXB.L = (pmsg->L | \
                                                   CAN_TX_WPNH_EBL | \
                                                   CAN_TX_REQ);
        return (CAN_VALID_MSG);
    }
    else
    {
        /* Message not sent. */
        return (CAN_NO_MSG);
    }
}

/***************************************************************************//**
 * MSS_CAN_send_message_abort_n()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_send_message_abort_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number
)
{
    /* Is buffer configured for Full CAN? */
    if (mailbox_number >= (CAN_TX_MAILBOX - this_can->basic_can_tx_mb))
    {
        /* mailbox is configured for basic CAN */
        return (CAN_BASIC_CAN_MAILBOX);
    }

    /* Set abort request */
    this_can->hw_reg->TxMsg[mailbox_number].TXB.L =
    ((this_can->hw_reg->TxMsg[mailbox_number].TXB.L & ~CAN_TX_REQ) | \
     CAN_TX_ABORT);

    /* Check the abort is granted */
    if (this_can->hw_reg->TxMsg[mailbox_number].TXB.TXABORT == 0u)
    {
        /* If the Tx buffer isn't busy, Abort was successful */
        return (CAN_OK);
    }
    else
    {
        /* Message not aborted. */
        return (CAN_ERR);
    }
}

/***************************************************************************//**
 * MSS_CAN_send_message_ready()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_send_message_ready
(
    mss_can_instance_t* this_can
)
{
    uint8_t success = CAN_ERR;
    uint8_t mailbox_number;

    /* Is a buffer configured for Basic CAN? */
    if (this_can->basic_can_tx_mb == 0u)
    {
       return (CAN_INVALID_MAILBOX);
    }

    /* Find next BASIC CAN buffer that is available */
    for (mailbox_number = CAN_TX_MAILBOX-this_can->basic_can_tx_mb; \
                             mailbox_number < CAN_TX_MAILBOX; mailbox_number++)
    {
        if (this_can->hw_reg->TxMsg[mailbox_number].TXB.TXREQ == 0u)
        {
            /* Tx buffer isn't busy */
            success = CAN_OK;
            break;
        }
    }

    return (success);
}

/***************************************************************************//**
 * MSS_CAN_send_message()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_send_message
(
    mss_can_instance_t* this_can,
    pmss_can_msgobject pmsg
)
{
    uint8_t success = CAN_NO_MSG;
    uint8_t mailbox_number;

    /* Is a buffer configured for Basic CAN? */
    if (this_can->basic_can_tx_mb == 0u)
    {
        return (CAN_INVALID_MAILBOX);
    }

    /* Find next BASIC CAN buffer that is available */
    for (mailbox_number = CAN_TX_MAILBOX-this_can->basic_can_tx_mb;  \
                             mailbox_number < CAN_TX_MAILBOX; mailbox_number++)
    {
        /* Check which transmit mailbox is not busy and use it. */
        if ((MSS_CAN_get_tx_buffer_status(this_can) & (1u << mailbox_number))
            == 0)
        {
            /* If the Tx buffer isn't busy.... */
            this_can->hw_reg->TxMsg[mailbox_number].ID = pmsg->ID;
            this_can->hw_reg->TxMsg[mailbox_number].DATALOW = pmsg->DATALOW;
            this_can->hw_reg->TxMsg[mailbox_number].DATAHIGH = pmsg->DATAHIGH;
            this_can->hw_reg->TxMsg[mailbox_number].TXB.L = (pmsg->L | \
                                                   CAN_TX_WPNH_EBL | \
                                                   CAN_TX_REQ);
            success = CAN_VALID_MSG;
            break;
        }
    }

    return (success);
}

/***************************************************************************//**
 * MSS_CAN_get_mask_n()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_get_mask_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number,
    uint32_t *pamr,
    uint32_t *pacr,
    uint16_t *pdta_amr,
    uint16_t *pdta_acr
)
{
    if (mailbox_number >= CAN_RX_MAILBOX)
    {
        return (CAN_BASIC_CAN_MAILBOX);
    }

    *pamr = this_can->hw_reg->RxMsg[mailbox_number].AMR.L;
    *pacr = this_can->hw_reg->RxMsg[mailbox_number].ACR.L;
    *pdta_acr = this_can->hw_reg->RxMsg[mailbox_number].ACR_D;
    *pdta_amr = this_can->hw_reg->RxMsg[mailbox_number].AMR_D;

    return (CAN_OK);
}

/***************************************************************************//**
 * MSS_CAN_set_mask_n()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_set_mask_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number,
    uint32_t amr,
    uint32_t acr,
    uint16_t dta_amr,
    uint16_t dta_acr
)
{
    if (mailbox_number >= CAN_RX_MAILBOX)
    {
        return (CAN_BASIC_CAN_MAILBOX);
    }

    this_can->hw_reg->RxMsg[mailbox_number].AMR.L = amr;
    this_can->hw_reg->RxMsg[mailbox_number].ACR.L = acr;
    this_can->hw_reg->RxMsg[mailbox_number].AMR_D = (uint32_t)dta_amr;
    this_can->hw_reg->RxMsg[mailbox_number].ACR_D = (uint32_t)dta_acr;

    return (CAN_OK);
}

/***************************************************************************//**
 * MSS_CAN_get_rx_buffer_status()
 * See "mss_can.h" for details of how to use this function.
 */
uint32_t
MSS_CAN_get_rx_buffer_status
(
    mss_can_instance_t* this_can
)
{
#ifdef CANMOD3
    return (this_can->hw_reg->BufferStatus.L & 0x0000FFFF);
#else
    return (this_can->hw_reg->BufferStatus.RXMSGAV);
#endif
}

/***************************************************************************//**
 * MSS_CAN_get_tx_buffer_status()
 * See "mss_can.h" for details of how to use this function.
 */
uint32_t
MSS_CAN_get_tx_buffer_status
(
    mss_can_instance_t* this_can
)
{
#ifdef CANMOD3
    return ((this_can->hw_reg->BufferStatus.L >> 16u) & 0x00FF);
#else
    return (this_can->hw_reg->BufferStatus.TXREQ);
#endif
}

/***************************************************************************//**
 * MSS_CAN_get_error_status()
 * See "mss_can.h" for details of how to use this function.
 */
uint8_t
MSS_CAN_get_error_status
(
    mss_can_instance_t* this_can,
    uint32_t *status
)
{
    /* Supply error register info if user wants. */
    *status = this_can->hw_reg->ErrorStatus.L;

    /* 00 Error Active, 01 Error Passive, 1x Bus Off */
    return ((uint8_t)(((*status) >> CAN_ERROR_STATUS_SHIFT) &
                      CAN_ERROR_STATUS_MASK));
}

/***************************************************************************//**
 * MSS_CAN_get_rx_error_count()
 * See "mss_can.h" for details of how to use this function.
 */
uint32_t
MSS_CAN_get_rx_error_count
(
    mss_can_instance_t* this_can
)
{
    return ((this_can->hw_reg->ErrorStatus.L >> CAN_ERROR_COUNT_SHIFT) & \
                                                          CAN_ERROR_COUNT_MASK);
}

/***************************************************************************//**
 * MSS_CAN_get_rx_gte96()
 * See "mss_can.h" for details of how to use this function.
 */
uint32_t
MSS_CAN_get_rx_gte96
(
    mss_can_instance_t* this_can
)
{
    return ((this_can->hw_reg->ErrorStatus.L >> CAN_RX_GTE96_SHIFT) & \
                                                                CAN_FLAG_MASK);
}

/***************************************************************************//**
 * MSS_CAN_get_tx_error_count()
 * See "mss_can.h" for details of how to use this function.
 */
uint32_t
MSS_CAN_get_tx_error_count
(
    mss_can_instance_t* this_can
)
{
    return (this_can->hw_reg->ErrorStatus.L & CAN_ERROR_COUNT_MASK);
}

/***************************************************************************//**
 * MSS_CAN_get_tx_gte96 ()
 * See "mss_can.h" for details of how to use this function.
 */
uint32_t
MSS_CAN_get_tx_gte96
(
    mss_can_instance_t* this_can
)
{
    return ((this_can->hw_reg->ErrorStatus.L >> CAN_TXGTE96_SHIFT) & \
                                                                CAN_FLAG_MASK);
}

/*******************************************************************************
 * Global initialization for all modes
 */
static void global_init
(
    mss_can_instance_t* this_wd
)
{
    if (&g_mss_can_0_lo == this_wd)
    {
        this_wd->hw_reg = MSS_CAN_0_LO_BASE;
        this_wd->irqn = CAN0_PLIC;
        this_wd->int_type = 0;
    }
    else if (&g_mss_can_1_lo == this_wd)
    {
        this_wd->hw_reg = MSS_CAN_1_LO_BASE;
        this_wd->irqn = CAN1_PLIC;
        this_wd->int_type = 0;
    }
    else if (&g_mss_can_0_hi == this_wd)
    {
        this_wd->hw_reg = MSS_CAN_0_HI_BASE;
        this_wd->irqn = CAN0_PLIC;
        this_wd->int_type = 0;
    }
    else if (&g_mss_can_1_hi == this_wd)
    {
        this_wd->hw_reg = MSS_CAN_1_HI_BASE;
        this_wd->irqn = CAN1_PLIC;
        this_wd->int_type = 0;
    }
    else
    {
        ;/* LDRA Warning */
    }
}

#ifndef MSS_CAN_USER_ISR
/***************************************************************************//**
 * CAN interrupt service routine.
 * CAN_IRQHandler is included within the RISC-V vector table as part of the
 * MPFS HAL.
 */
uint8_t External_can0_plic_IRQHandler(void)
{
#ifdef MSS_CAN_ENABLE_INTERRUPTS
    /* User provided code is required here to handle interrupts from the MSS CAN
     * peripheral. Remove the assert once this is in place.*/
    ASSERT(!"An ISR is required here if interrupts are enabled");
#else
    ASSERT(!"Unexpected MSS CAN interrupt - MSS CAN NVIC Interrupts should be \
           disabled");
#endif
    return 0;
}

uint8_t can1_IRQHandler(void)
{
#ifdef MSS_CAN_ENABLE_INTERRUPTS
    /* User provided code is required here to handle interrupts from the MSS CAN
     * peripheral. Remove the assert once this is in place.*/
    ASSERT(!"An ISR is required here if interrupts are enabled");
#else
    ASSERT(!"Unexpected MSS CAN interrupt - MSS CAN NVIC Interrupts should be \
           disabled");
#endif
    return 0;
}
#endif

#ifdef __cplusplus
}
#endif
