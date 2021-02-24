/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC MSS USB Driver Stack
 *      USB Core Interface Layer (USB-CIFL)
 *          USB-CIF.
 *
 * This file provides interfaces to perform register bit level
 * I\O operations which are independent of USB Device/host mode.
 *
 */

#ifndef __MSS_USB_COMMON_REG_IO_H_
#define __MSS_USB_COMMON_REG_IO_H_

#include <stdint.h>
#include "mss_usb_common_cif.h"
#include "mss_usb_core_regs.h"

#define __INLINE        inline

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*//**
* Common function for DEVICE and HOST mode.
*/

/*-------------------------------------------------------------------------*//**
  SOFT_RST register related APIs
 */
static __INLINE void
MSS_USB_CIF_soft_reset
(
    void
)
{
    volatile uint8_t soft_reset;

    USB->SOFT_RST = SOFT_RESET_REG_MASK;
    do {
        soft_reset = USB->SOFT_RST;
    } while (soft_reset != 0x00u);
}

/*-------------------------------------------------------------------------*//**
  POWER register related APIs
 */
static __INLINE void
MSS_USB_CIF_enable_hs_mode
(
    void
)
{
    USB->POWER |= POWER_REG_ENABLE_HS_MASK;
}

static __INLINE void
MSS_USB_CIF_disable_hs_mode
(
    void
)
{
    USB->POWER &= ~POWER_REG_ENABLE_HS_MASK;
}

/*-------------------------------------------------------------------------*//**
  Read USB interrupt register to know which interrupt has occurred.
 */
static __INLINE uint8_t
MSS_USB_CIF_read_irq_reg
(
    void
)
{
    return (USB->USB_IRQ);
}

/*-------------------------------------------------------------------------*//**
  EP IRQ related APIs
 */
/* TX IRQ related APIs */
static __INLINE uint16_t
MSS_USB_CIF_read_tx_ep_irq_reg
(
    void
)
{
    return (USB->TX_IRQ);
}

static __INLINE uint16_t
MSS_USB_CIF_read_rx_ep_irq_reg
(
    void
)
{
    return (USB->RX_IRQ);
}

static __INLINE void
MSS_USB_CIF_clr_usb_irq_reg
(
    void
)
{
    USB->USB_IRQ = 0u;
}

static __INLINE void
MSS_USB_CIF_tx_ep_enable_irq
(
    mss_usb_ep_num_t ep_num
)
{
    USB->TX_IRQ_ENABLE |= (uint16_t)(MSS_USB_WORD_BIT_0_MASK << (uint8_t)ep_num);
}


static __INLINE void
MSS_USB_CIF_tx_ep_disable_irq
(
    mss_usb_ep_num_t ep_num
)
{
    USB->TX_IRQ_ENABLE &= (uint16_t)(~(MSS_USB_WORD_BIT_0_MASK << (uint8_t)ep_num));
}

static __INLINE void
MSS_USB_CIF_tx_ep_disable_irq_all
(
    void
)
{
    /* Keep D0, CEP interrupt bit unaltered.*/
    USB->TX_IRQ_ENABLE = (USB->TX_IRQ_ENABLE & TX_IRQ_ENABLE_REG_CEP_MASK);
}

static __INLINE void
MSS_USB_CIF_rx_ep_enable_irq
(
    mss_usb_ep_num_t ep_num
)
{
    USB->RX_IRQ_ENABLE |= (uint16_t)(MSS_USB_WORD_BIT_0_MASK << (uint8_t)ep_num);
}

static __INLINE void
MSS_USB_CIF_rx_ep_disable_irq
(
    mss_usb_ep_num_t ep_num
)
{
    USB->RX_IRQ_ENABLE &= (uint16_t)(~(MSS_USB_WORD_BIT_0_MASK << (uint8_t)ep_num));
}

static __INLINE void
MSS_USB_CIF_rx_ep_disable_irq_all
(
    void
)
{
    USB->RX_IRQ_ENABLE = 0u ;
}

/*-------------------------------------------------------------------------*//**
  FRAME register related APIs
 */
static __INLINE uint16_t
MSS_USB_CIF_get_last_frame_nbr
(
    void
)
{
    return (USB->FRAME);
}

/*-------------------------------------------------------------------------*//**
  DEVCTL register related APIs
 */
static __INLINE mss_usb_core_mode_t
MSS_USB_CIF_get_mode
(
    void
)
{
    return (((USB->DEV_CTRL & DEV_CTRL_HOST_MODE_MASK) ?
                            MSS_USB_CORE_MODE_HOST : MSS_USB_CORE_MODE_DEVICE));
}

static __INLINE mss_usb_vbus_level_t
MSS_USB_CIF_get_vbus_level
(
    void
)
{
    uint8_t vbus;
    mss_usb_vbus_level_t ret_val = VBUS_BLOW_SESSIONEND;

    vbus = (USB->DEV_CTRL & DEV_CTRL_VBUS_MASK);

    switch (vbus)
    {
        case VBUS_BELOW_SESSION_END:
                ret_val = VBUS_BLOW_SESSIONEND;
                break;
        case VBUS_ABOVE_SESSION_END:
                ret_val = VBUS_ABV_SESSIONEND_BLOW_AVALID;
                break;
        case VBUS_ABOVE_AVALID:
                ret_val = VBUS_ABV_AVALID_BLOW_VB_VALID;
                break;
        case VBUS_ABOVE_VBUS_VALID:
                ret_val = VBUS_ABV_VB_VALID;
                break;
        default:
                /* Empty */
                break;

    }
    return ret_val;
}

static __INLINE mss_usb_device_role_t
MSS_USB_CIF_get_role
(
    void
)
{
    /* TODO:only valid when session bit is set */
    return (((USB->DEV_CTRL & DEV_CTRL_B_DEVICE_MASK) ?
                MSS_USB_DEVICE_ROLE_DEVICE_B : MSS_USB_DEVICE_ROLE_DEVICE_A));
}

static __INLINE uint8_t
MSS_USB_CIF_is_session_on
(
    void
)
{
    /* TODO:only valid when session bit is set */
    return (((USB->DEV_CTRL & DEV_CTRL_SESSION_MASK) ?
                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void MSS_USB_CIF_start_session(void)
{
    USB->DEV_CTRL |= DEV_CTRL_SESSION_MASK;
}
static __INLINE void MSS_USB_CIF_stop_session(void)
{
    USB->DEV_CTRL &= ~DEV_CTRL_SESSION_MASK;
}

/*-------------------------------------------------------------------------*//**
  INDEXED registers
 */

/*-------------------------------------------------------------------------*//**
  CSR0L register related APIs
 */
static __INLINE uint8_t
MSS_USB_CIF_cep_is_rxpktrdy
(
    void
)
{
    return (((USB->INDEXED_CSR.DEVICE_EP0.CSR0 & CSR0L_DEV_RX_PKT_RDY_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_cep_set_txpktrdy
(
    void
)
{
    USB->INDEXED_CSR.DEVICE_EP0.CSR0 |= CSR0L_DEV_TX_PKT_RDY_MASK;
}

static __INLINE void
MSS_USB_CIF_cep_clr_stall_sent
(
    void
)
{
    USB->INDEXED_CSR.DEVICE_EP0.CSR0 &= ~CSR0L_DEV_STALL_SENT_MASK;
}

static __INLINE uint8_t
MSS_USB_CIF_cep_is_stall_sent
(
    void
)
{
    return (((USB->INDEXED_CSR.DEVICE_EP0.CSR0 & CSR0L_DEV_STALL_SENT_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_cep_clr_setupend
(
    void
)
{
    /* Setting SERVICED_SETUP_END bit clears SetupEnd bit */
    USB->INDEXED_CSR.DEVICE_EP0.CSR0 |= CSR0L_DEV_SERVICED_SETUP_END_MASK;
}
static __INLINE uint8_t
MSS_USB_CIF_cep_is_setupend
(
    void
)
{
    return (((USB->INDEXED_CSR.DEVICE_EP0.CSR0 & CSR0L_DEV_SETUP_END_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_cep_reset_csr0_reg
(
    void
)
{
    /* load the reset value for the register */
    USB->INDEXED_CSR.DEVICE_EP0.CSR0 = 0u;
}

/*-------------------------------------------------------------------------*//**
  CNTRH0 register related APIs
 */
static __INLINE uint8_t
MSS_USB_CIF_cep_rx_byte_count
(
    void
)
{
    /* TODO:confirm CSR0.D0 bit before returning */
    return (USB->INDEXED_CSR.DEVICE_EP0.COUNT0 & COUNT0_REG_MASK);
}

/*-------------------------------------------------------------------------*//**
  TXMAXP register related APIs
 */
static __INLINE void
MSS_USB_CIF_tx_ep_set_max_pkt
(
    mss_usb_ep_num_t ep_num,
    mss_usb_xfr_type_t xfr_type,
    uint16_t max_pkt_size,
    uint8_t num_usb_pkt
)
{
    /* TODO:make sure that there is no data in FIFO before writing into the maxP
       reg
     */
    if ((ep_num > MSS_USB_CEP) && ((max_pkt_size % 8) == 0u) && (num_usb_pkt > 0u))
    {
        if((num_usb_pkt > 31u))
        {
            /* not allowed */
        }
        else
        {
            USB->ENDPOINT[ep_num].TX_MAX_P = 0u;
            USB->ENDPOINT[ep_num].TX_MAX_P = num_usb_pkt - 1u;
            USB->ENDPOINT[ep_num].TX_MAX_P <<= TX_MAX_P_REG_NUM_USB_PKT_SHIFT;
            USB->ENDPOINT[ep_num].TX_MAX_P |= (max_pkt_size);
        }
    }
}

/*-------------------------------------------------------------------------*//**
  TXCSRL register related APIs
 */
static __INLINE void
MSS_USB_CIF_tx_ep_set_txpktrdy
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRL_REG_EPN_TX_PKT_RDY_MASK;
}

static __INLINE uint8_t
MSS_USB_CIF_tx_ep_is_txpktrdy
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRL_REG_EPN_TX_PKT_RDY_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_tx_ep_set_send_stall_bit
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRL_REG_EPN_SEND_STALL_MASK;
}

static __INLINE void
MSS_USB_CIF_tx_ep_clr_send_stall_bit
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRL_REG_EPN_SEND_STALL_MASK;
}

static __INLINE void
MSS_USB_CIF_tx_ep_clr_stall_sent_bit
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRL_REG_EPN_STALL_SENT_MASK;
}

static __INLINE uint8_t
MSS_USB_CIF_tx_ep_is_stall_sent_bit
(
    mss_usb_ep_num_t ep_num
)
{
    return (uint8_t)(((USB->ENDPOINT[ep_num].TX_CSR & TxCSRL_REG_EPN_STALL_SENT_MASK) ?
                                        MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_tx_ep_clr_data_tog
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRL_REG_EPN_CLR_DATA_TOG_MASK;
}

static __INLINE uint8_t
MSS_USB_CIF_tx_ep_is_isoincomp
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRL_REG_EPN_ISO_INCOMP_TX_MASK) ?
                                    MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

/*-------------------------------------------------------------------------*//**
  TXCSRH register related APIs
 */
static __INLINE void
MSS_USB_CIF_tx_ep_set_dma_mode1
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRH_REG_EPN_DMA_MODE_MASK;
}

static __INLINE void
MSS_USB_CIF_tx_ep_set_dma_mode0
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRH_REG_EPN_DMA_MODE_MASK;
}

static __INLINE uint8_t
MSS_USB_CIF_tx_ep_get_dma_mode
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRH_REG_EPN_DMA_MODE_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_tx_ep_set_force_data_tog
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRH_REG_EPN_FRC_DAT_TOG_MASK;
}

static __INLINE void
MSS_USB_CIF_tx_ep_clr_force_data_tog
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRH_REG_EPN_FRC_DAT_TOG_MASK;
}

static __INLINE void
MSS_USB_CIF_tx_ep_enable_dma
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRH_REG_EPN_ENABLE_DMA_MASK;
}

static __INLINE void
MSS_USB_CIF_tx_ep_disable_dma
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRH_REG_EPN_ENABLE_DMA_MASK;
}

static __INLINE uint8_t
MSS_USB_CIF_tx_ep_is_dma_enabled
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRH_REG_EPN_ENABLE_DMA_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_tx_ep_set_tx_mode
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRH_REG_EPN_TXRX_MODE_MASK;
}
static __INLINE void
MSS_USB_CIF_tx_ep_set_rx_mode
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRH_REG_EPN_TXRX_MODE_MASK;
}

static __INLINE void
MSS_USB_CIF_tx_ep_enable_iso
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRH_REG_EPN_ENABLE_ISO_MASK;
}

static __INLINE void
MSS_USB_CIF_tx_ep_disable_iso
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRH_REG_EPN_ENABLE_ISO_MASK;
}

static __INLINE void
MSS_USB_CIF_tx_ep_set_autoset
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRH_REG_EPN_ENABLE_AUTOSET_MASK;
}
static __INLINE void
MSS_USB_CIF_tx_ep_clr_autoset
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRH_REG_EPN_ENABLE_AUTOSET_MASK;
}

static __INLINE uint8_t
MSS_USB_CIF_tx_ep_is_autoset
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRH_REG_EPN_ENABLE_AUTOSET_MASK) ?
                                    MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE uint8_t
MSS_USB_CIF_tx_ep_is_underrun
(
    mss_usb_ep_num_t ep_num
)
{
   return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRL_REG_EPN_UNDERRUN_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_tx_ep_clr_underrun
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRL_REG_EPN_UNDERRUN_MASK;
}

static __INLINE void
MSS_USB_CIF_tx_ep_clr_csrreg
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR = 0x0000U;
}

/*-------------------------------------------------------------------------*//**
  RXMAXP register related APIs
 */
static __INLINE void
MSS_USB_CIF_rx_ep_set_max_pkt
(
    mss_usb_ep_num_t ep_num,
    mss_usb_xfr_type_t xfr_type,
    uint16_t max_pkt_size,
    uint8_t num_usb_pkt
)
{
    /* TODO:make sure that there is no data in FIFO before writing into the
       maxP reg
     */
    if ((ep_num > MSS_USB_CEP) && (max_pkt_size > 0u) && ((max_pkt_size % 8) == 0u))
    {
        if ((num_usb_pkt > 31u))
        {
            /*not allowed*/
        }
        else
        {
            USB->ENDPOINT[ep_num].RX_MAX_P = 0u;
            USB->ENDPOINT[ep_num].RX_MAX_P = num_usb_pkt - 1u;
            USB->ENDPOINT[ep_num].RX_MAX_P <<= RX_MAX_P_REG_NUM_USB_PKT_SHIFT;
            USB->ENDPOINT[ep_num].RX_MAX_P |= (max_pkt_size);
        }
    }
}

/*-------------------------------------------------------------------------*//**
  RXCSRL register related APIs
 */
static __INLINE uint8_t
MSS_USB_CIF_rx_ep_is_rxpktrdy
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RxCSRL_REG_EPN_RX_PKT_RDY_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_rx_ep_clr_rxpktrdy
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR &= ~RxCSRL_REG_EPN_RX_PKT_RDY_MASK;
}

static __INLINE uint8_t
MSS_USB_CIF_rx_ep_is_overrun
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RxCSRL_REG_EPN_OVERRUN_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_rx_ep_clr_overrun
(
    mss_usb_ep_num_t ep_num
)
{
    uint16_t reg_val = USB->ENDPOINT[ep_num].RX_CSR;

    reg_val &= ~RxCSRL_REG_EPN_OVERRUN_MASK;
    reg_val |= RxCSRL_REG_EPN_RX_PKT_RDY_MASK;
    USB->ENDPOINT[ep_num].RX_CSR = reg_val;
}

static __INLINE uint8_t
MSS_USB_CIF_rx_ep_is_dataerr
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RxCSRL_REG_EPN_DATA_ERR_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_rx_ep_set_send_stall_bit
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR |= (RxCSRL_REG_EPN_SEND_STALL_MASK |
                                               RxCSRL_REG_EPN_RX_PKT_RDY_MASK);
}

static __INLINE void
MSS_USB_CIF_rx_ep_clr_send_stall_bit
(
    mss_usb_ep_num_t ep_num
)
{
    uint16_t reg_val = USB->ENDPOINT[ep_num].RX_CSR;

    reg_val &= ~RxCSRL_REG_EPN_SEND_STALL_MASK;
    reg_val |= RxCSRL_REG_EPN_RX_PKT_RDY_MASK;
    USB->ENDPOINT[ep_num].RX_CSR = reg_val;
}

static __INLINE uint8_t
MSS_USB_CIF_rx_ep_is_stall_sent_bit
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RxCSRL_REG_EPN_STALL_SENT_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_rx_ep_clr_stall_sent_bit
(
    mss_usb_ep_num_t ep_num
)
{
    uint16_t reg_val = USB->ENDPOINT[ep_num].RX_CSR;

    reg_val &= ~RxCSRL_REG_EPN_STALL_SENT_MASK;
    reg_val |= RxCSRL_REG_EPN_RX_PKT_RDY_MASK;
    USB->ENDPOINT[ep_num].RX_CSR = reg_val;
}

static __INLINE void
MSS_USB_CIF_rx_ep_clr_data_tog
(
    mss_usb_ep_num_t ep_num
)
{
    /* setting CLR_DAT_TOG bit clears USB Data toggle bit */
    USB->ENDPOINT[ep_num].RX_CSR |= (RxCSRL_REG_EPN_CLR_DAT_TOG_MASK |
                                               RxCSRL_REG_EPN_RX_PKT_RDY_MASK);
}

/*-------------------------------------------------------------------------*//**
  RXCSRH register related APIs
 */
static __INLINE uint8_t
MSS_USB_CIF_rx_ep_is_isoincomp
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RxCSRL_REG_EPN_RX_ISO_INCOMP) ?
                            MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE mss_usb_dma_mode_t
MSS_USB_CIF_rx_ep_get_dma_mode
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RxCSRL_REG_EPN_DMA_MODE_MASK) ?
                                    MSS_USB_DMA_MODE1 : MSS_USB_DMA_MODE0));
}

static __INLINE void
MSS_USB_CIF_rx_ep_set_dma_mode1
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR |= (RxCSRL_REG_EPN_DMA_MODE_MASK |
                                               RxCSRL_REG_EPN_RX_PKT_RDY_MASK);
}

static __INLINE void
MSS_USB_CIF_rx_ep_set_dma_mode0
(
    mss_usb_ep_num_t ep_num
)
{
    uint16_t reg_val = USB->ENDPOINT[ep_num].RX_CSR;

    reg_val &= ~RxCSRL_REG_EPN_DMA_MODE_MASK;
    reg_val |= RxCSRL_REG_EPN_RX_PKT_RDY_MASK;
    USB->ENDPOINT[ep_num].RX_CSR = reg_val;
}

static __INLINE void
MSS_USB_CIF_rx_ep_enable_dma
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR |= (RxCSRL_REG_EPN_ENABLE_DMA_MASK |
                                               RxCSRL_REG_EPN_RX_PKT_RDY_MASK);
}

static __INLINE void
MSS_USB_CIF_rx_ep_disable_dma
(
    mss_usb_ep_num_t ep_num
)
{
    uint16_t reg_val = USB->ENDPOINT[ep_num].RX_CSR;

    reg_val &= ~RxCSRL_REG_EPN_ENABLE_DMA_MASK;
    reg_val |= RxCSRL_REG_EPN_RX_PKT_RDY_MASK;
    USB->ENDPOINT[ep_num].RX_CSR = reg_val;
}

static __INLINE uint8_t
MSS_USB_CIF_rx_ep_is_dma_enabled
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RxCSRL_REG_EPN_ENABLE_DMA_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE uint8_t
MSS_USB_CIF_rx_ep_is_piderr
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RxCSRL_REG_EPN_ISO_PID_ERR_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_rx_ep_disable_nyet
(
    mss_usb_ep_num_t ep_num
)
{
    /* Setting BI_DIS_NYET mask disables NYET */
    USB->ENDPOINT[ep_num].RX_CSR |= (RxCSRL_REG_EPN_BI_DIS_NYET_MASK |
                                               RxCSRL_REG_EPN_RX_PKT_RDY_MASK);
}

static __INLINE void
MSS_USB_CIF_rx_ep_enable_nyet
(
    mss_usb_ep_num_t ep_num
)
{
    uint16_t reg_val = USB->ENDPOINT[ep_num].RX_CSR;

    reg_val &= ~RxCSRL_REG_EPN_BI_DIS_NYET_MASK;
    reg_val |= RxCSRL_REG_EPN_RX_PKT_RDY_MASK;
    USB->ENDPOINT[ep_num].RX_CSR = reg_val;
}

static __INLINE void
MSS_USB_CIF_rx_ep_disable_iso
(
    mss_usb_ep_num_t ep_num
)
{
    uint16_t reg_val = USB->ENDPOINT[ep_num].RX_CSR;

    reg_val &= ~RxCSRL_REG_EPN_ENABLE_ISO_MASK;
    reg_val |= RxCSRL_REG_EPN_RX_PKT_RDY_MASK;
    USB->ENDPOINT[ep_num].RX_CSR = reg_val;
}

static __INLINE void
MSS_USB_CIF_rx_ep_enable_iso
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR |= (RxCSRL_REG_EPN_ENABLE_ISO_MASK |
                                               RxCSRL_REG_EPN_RX_PKT_RDY_MASK);
}

static __INLINE void
MSS_USB_CIF_rx_ep_set_autoclr
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR |= (RxCSRL_REG_EPN_ENABLE_AUTOCLR_MASK |
                                               RxCSRL_REG_EPN_RX_PKT_RDY_MASK);
}

static __INLINE void
MSS_USB_CIF_rx_ep_clr_autoclr
(
    mss_usb_ep_num_t ep_num
)
{
    uint16_t reg_val = USB->ENDPOINT[ep_num].RX_CSR;

    reg_val &= ~RxCSRL_REG_EPN_ENABLE_AUTOCLR_MASK;
    reg_val |= RxCSRL_REG_EPN_RX_PKT_RDY_MASK;
    USB->ENDPOINT[ep_num].RX_CSR = reg_val;
}

static __INLINE uint8_t
MSS_USB_CIF_rx_ep_is_autoclr
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RxCSRL_REG_EPN_ENABLE_AUTOCLR_MASK) ?
                                  MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_rx_ep_clr_csrreg
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR = 0x0000U;
}

/*-------------------------------------------------------------------------*//**
  RXCOUNT register related APIs
 */
static __INLINE uint16_t
MSS_USB_CIF_rx_ep_read_count
(
    mss_usb_ep_num_t ep_num
)
{
    /* only valid when rxpktrdy is set */
    return (USB->ENDPOINT[ep_num].RX_COUNT);
}

/*-------------------------------------------------------------------------*//**
  FIFOx register related APIs
 */
static __INLINE void
MSS_USB_CIF_load_tx_fifo
(
    mss_usb_ep_num_t ep_num,
    void * in_data,
    uint32_t length
)
{
    uint32_t idx;
    uint32_t* temp;
    uint8_t* temp_8bit;

    uint16_t words = length / 4;
    temp = in_data;
    temp_8bit = in_data;

    for(idx = 0u; idx < words; ++idx)
    {
        USB->FIFO[ep_num].WORD.VALUE = (uint32_t)temp[idx];
    }

    for(idx = (length - (length % 4)); idx < length; ++idx)
    {
        USB->FIFO[ep_num].BYTE.VALUE = (uint8_t)temp_8bit[idx];
    }
}

static __INLINE void
MSS_USB_CIF_read_rx_fifo
(
    mss_usb_ep_num_t ep_num,
    void * out_data,
    uint32_t length
)
{
    uint32_t idx;
    volatile uint32_t* temp;
    uint8_t* temp_8bit;

    uint16_t words = length / 4;
    temp = out_data;
    temp_8bit = out_data;

    for(idx = 0u; idx < words; ++idx)
    {
        temp[idx] = USB->FIFO[ep_num].WORD.VALUE;
    }

    for(idx = (length - (length % 4u)); idx < length; ++idx)
    {
        temp_8bit[idx] = USB->FIFO[ep_num].BYTE.VALUE;
    }
}

static __INLINE uint8_t
MSS_USB_CIF_read_rx_fifo_byte
(
    mss_usb_ep_num_t ep_num
)
{
    return (uint8_t)(USB->FIFO[ep_num].BYTE.VALUE);
}

static __INLINE uint16_t
MSS_USB_CIF_read_rx_fifo_halfword
(
    mss_usb_ep_num_t ep_num
)
{
    return (uint16_t)(USB->FIFO[ep_num].HALFWORD.VALUE);
}

static __INLINE uint32_t
MSS_USB_CIF_read_rx_fifo_word
(
    mss_usb_ep_num_t ep_num
)
{
    return (uint32_t)(USB->FIFO[ep_num].WORD.VALUE);
}

/*-------------------------------------------------------------------------*//**
  DynFIFOSIZE register related APIs
 */
static __INLINE void
MSS_USB_CIF_tx_ep_set_fifo_size
(
    mss_usb_ep_num_t ep_num,
    uint16_t fifo_size,
    uint8_t dpb
)
{
    uint16_t temp;
    uint8_t i = 0;

    /* Valid FIFO sizes are 8,16,32,64,128,512,1024,2048,4096 */
    if ((ep_num > MSS_USB_CEP) && (fifo_size >= MIN_EP_FIFO_SZ))
    {
        USB->INDEX = ep_num;
        temp = (fifo_size / MIN_EP_FIFO_SZ);
        while (!(temp & MSS_USB_WORD_BIT_0_MASK))
        {
            temp >>= 1u;
            i++;
        }
        USB->TX_FIFO_SIZE = ((dpb << TXFIFOSZ_REG_DPB_SHIFT) | i);
    }
}

static __INLINE void
MSS_USB_CIF_rx_ep_set_fifo_size
(
    mss_usb_ep_num_t ep_num,
    uint16_t fifo_size,
    uint8_t dpb
)
{
    /* fifo_size is the size in terms of number of bytes.*/
    uint16_t temp;
    uint8_t i = 0u;

    /* Valid FIFO sizes are 8,16,32,64,128,512,1024,2048,4096 */
    if ((ep_num > MSS_USB_CEP) && (fifo_size >= MIN_EP_FIFO_SZ))
    {
        USB->INDEX = ep_num;
        temp= (fifo_size / MIN_EP_FIFO_SZ);
        while (!(temp & MSS_USB_WORD_BIT_0_MASK))
        {
            temp >>= 1u;
            i++;
        }
        USB->RX_FIFO_SIZE = ((dpb << RXFIFOSZ_REG_DPB_SHIFT) | i);
    }
}

/*-------------------------------------------------------------------------*//**
  DynFIFOAD register related APIs
 */
static __INLINE void
MSS_USB_CIF_tx_ep_set_fifo_addr
(
    mss_usb_ep_num_t ep_num,
    uint16_t addr
)
{
    /* Valid address values are from 0 to FFF8 in steps of 8 */
    if (ep_num > MSS_USB_CEP)
    {
        USB->INDEX = ep_num;
        USB->TX_FIFO_ADDR = (addr / EP_FIFO_ADDR_STEP);
    }
}

static __INLINE void
MSS_USB_CIF_rx_ep_set_fifo_addr
(
    mss_usb_ep_num_t ep_num,
    uint16_t addr
)
{
    /* Valid address values are from 0 to FFF8 in steps of 8 */
    if (ep_num > MSS_USB_CEP)
    {
        USB->INDEX = ep_num;
        USB->RX_FIFO_ADDR = (addr / EP_FIFO_ADDR_STEP);
    }
}

/*-------------------------------------------------------------------------*//**
  RX_DPKTBUFDIS register related APIs
 */
static __INLINE void
MSS_USB_CIF_disable_rx_ep_dpb
(
    mss_usb_ep_num_t ep_num
)
{
    USB->RX_DPBUF_DIS |= (uint16_t)(MSS_USB_WORD_BIT_0_MASK << ep_num);
}

static __INLINE void
MSS_USB_CIF_enable_rx_ep_dpb
(
    mss_usb_ep_num_t ep_num
)
{
    USB->RX_DPBUF_DIS &= (uint16_t)(~(MSS_USB_WORD_BIT_0_MASK << ep_num));
}

/*-------------------------------------------------------------------------*//**
  TX_DPKTBUFDIS register related APIs
 */
static __INLINE void
MSS_USB_disable_tx_ep_dpb
(
    mss_usb_ep_num_t ep_num
)
{
    USB->TX_DPBUF_DIS |= (uint16_t)(MSS_USB_WORD_BIT_0_MASK << ep_num);
}

static __INLINE void
MSS_USB_enable_tx_ep_dpb
(
    mss_usb_ep_num_t ep_num
)
{
    USB->TX_DPBUF_DIS &= (uint16_t)(~(MSS_USB_WORD_BIT_0_MASK << ep_num));
}

/*-------------------------------------------------------------------------*//**
  DMA_INTR register related APIs
 */
static __INLINE uint32_t
MSS_USB_CIF_dma_read_irq
(
    void
)
{
    return (USB->DMA_CHANNEL[0].IRQ);
}

/*-------------------------------------------------------------------------*//**
 * DMA_CNTL register related APIs
 */
static __INLINE void
MSS_USB_CIF_dma_start_xfr
(
    mss_usb_dma_channel_t dma_channel
)
{
    USB->DMA_CHANNEL[dma_channel].CNTL |= DMA_CNTL_REG_START_XFR_MASK;
}

static __INLINE void
MSS_USB_CIF_dma_stop_xfr
(
    mss_usb_dma_channel_t dma_channel
)
{
    USB->DMA_CHANNEL[dma_channel].CNTL &= ~DMA_CNTL_REG_START_XFR_MASK;
}

static __INLINE void
MSS_USB_CIF_dma_set_dir
(
    mss_usb_dma_channel_t dma_channel,
    mss_usb_dma_dir_t dir
)
{
    /*
      dir = 1 => DMA read (TX EP)
      dir = 0 => DMA write (RX EP)
     */
    USB->DMA_CHANNEL[dma_channel].CNTL |= (dir << DMA_CNTL_REG_DMA_DIR_SHIFT);
}

static __INLINE mss_usb_dma_dir_t
MSS_USB_CIF_dma_get_dir
(
    mss_usb_dma_channel_t dma_channel
)
{
    return(((USB->DMA_CHANNEL[dma_channel].CNTL & DMA_CNTL_REG_DMA_DIR_MASK) ?
                                        MSS_USB_DMA_READ : MSS_USB_DMA_WRITE));
}

static __INLINE void
MSS_USB_CIF_dma_set_mode
(
    mss_usb_dma_channel_t dma_channel,
    mss_usb_dma_mode_t dma_mode
)
{
    USB->DMA_CHANNEL[dma_channel].CNTL |= (dma_mode << DMA_CNTL_REG_DMA_MODE_SHIFT);
}

static __INLINE mss_usb_dma_mode_t
MSS_USB_CIF_dma_get_mode
(
    mss_usb_dma_channel_t dma_channel
)
{
    return (((USB->DMA_CHANNEL[dma_channel].CNTL & DMA_CNTL_REG_DMA_MODE_MASK) ?
                                         MSS_USB_DMA_MODE1: MSS_USB_DMA_MODE0));
}

static __INLINE void
MSS_USB_CIF_dma_enable_irq
(
    mss_usb_dma_channel_t dma_channel
)
{
    USB->DMA_CHANNEL[dma_channel].CNTL |= DMA_CNTL_REG_ENABLE_DMA_IRQ_MASK;
}

static __INLINE void
MSS_USB_CIF_dma_disable_irq
(
    mss_usb_dma_channel_t dma_channel
)
{
    USB->DMA_CHANNEL[dma_channel].CNTL &= ~DMA_CNTL_REG_ENABLE_DMA_IRQ_MASK;
}

static __INLINE void
MSS_USB_CIF_dma_assign_to_epnum
(
    mss_usb_dma_channel_t dma_channel,
    mss_usb_ep_num_t ep_num
)
{
    USB->DMA_CHANNEL[dma_channel].CNTL |=
                                  ((ep_num << DMA_CNTL_REG_DMA_EP_NUM_SHIFT) &
                                    DMA_CNTL_REG_DMA_EP_NUM_MASK);
}

static __INLINE mss_usb_ep_num_t
MSS_USB_CIF_dma_get_epnum
(
    mss_usb_dma_channel_t dma_channel
)
{
    /*
      This API will return numbers from 0 to 15, mss_usb_ep_num_t maps it to TX
      EP numbers. Using DMA DIR, CIF driver should correctly map it as TX EP or
      RX EP.
     */
    volatile uint8_t ep_num;

    ep_num = (USB->DMA_CHANNEL[dma_channel].CNTL  & DMA_CNTL_REG_DMA_EP_NUM_MASK);

    return (mss_usb_ep_num_t)(ep_num >> DMA_CNTL_REG_DMA_EP_NUM_SHIFT);
}

static __INLINE uint8_t
MSS_USB_CIF_dma_is_bus_err
(
    mss_usb_dma_channel_t dma_channel
)
{
    return (((USB->DMA_CHANNEL[dma_channel].CNTL  & DMA_CNTL_REG_DMA_BUS_ERR_MASK) ?
                                    MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_CIF_dma_clr_bus_err
(
    mss_usb_dma_channel_t dma_channel
)
{
    USB->DMA_CHANNEL[dma_channel].CNTL  &= ~DMA_CNTL_REG_DMA_BUS_ERR_MASK;
}

static __INLINE void
MSS_USB_CIF_dma_set_burst_mode
(
    mss_usb_dma_channel_t dma_channel,
    mss_usb_dma_burst_mode_t
    burst_mode
)
{
    USB->DMA_CHANNEL[dma_channel].CNTL |=
                       ((burst_mode << DMA_CNTL_REG_DMA_BURST_MODE_SHIFT) &
                       DMA_CNTL_REG_DMA_BURST_MODE_MASK);
}

static __INLINE mss_usb_dma_burst_mode_t
MSS_USB_CIF_dma_get_burst_mode
(
    mss_usb_dma_channel_t dma_channel,
    mss_usb_dma_burst_mode_t burst_mode
)
{
    uint8_t mode;
    mode = (USB->DMA_CHANNEL[dma_channel].CNTL  &
                                             DMA_CNTL_REG_DMA_BURST_MODE_MASK);

    return (mss_usb_dma_burst_mode_t)(mode >> DMA_CNTL_REG_DMA_BURST_MODE_SHIFT);
}

static __INLINE void
MSS_USB_CIF_dma_clr_ctrlreg
(
    mss_usb_dma_channel_t dma_channel
)
{
    USB->DMA_CHANNEL[dma_channel].CNTL = 0x0000U;
}

/*-------------------------------------------------------------------------*//**
  DMA_ADDR register related APIs
 */
static __INLINE uint32_t
MSS_USB_CIF_dma_read_addr
(
    mss_usb_dma_channel_t dma_channel
)
{
    return (USB->DMA_CHANNEL[dma_channel].ADDR);
}

static __INLINE void
MSS_USB_CIF_dma_write_addr
(
    mss_usb_dma_channel_t dma_channel,
    uint32_t addr
)
{
    USB->DMA_CHANNEL[dma_channel].ADDR = addr;
}

/*-------------------------------------------------------------------------*//**
  DMA_COUNT register related APIs
 */
static __INLINE uint32_t
MSS_USB_CIF_dma_read_count
(
    mss_usb_dma_channel_t dma_channel
)
{
    return (USB->DMA_CHANNEL[dma_channel].COUNT);
}

static __INLINE void
MSS_USB_CIF_dma_write_count
(
    mss_usb_dma_channel_t dma_channel,
    uint32_t count
)
{
    USB->DMA_CHANNEL[dma_channel].COUNT = count;
}

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_COMMON_REG_IO_H_ */
