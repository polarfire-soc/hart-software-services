/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Microchip PolarFire SoC MSS USB Driver Stack
 *      USB Core Interface Layer (USB-CIFL)
 *          USBH-CIF
 *
 *  This file provides interfaces to perform register and register bit level
 *  read / write operations in USB Host mode.
 *
 * SVN $Revision$
 * SVN $Date$
 */

#ifndef __MSS_USB_HOST_REG_IO_H_
#define __MSS_USB_HOST_REG_IO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mss_usb_core_regs.h"

/*------------------------------------------------------------------------- *//**
  Host Mode Functions
  ============================
 */

/*------------------------------------------------------------------------- *//**
  CSR0L register related APIs
 */
static __INLINE uint8_t
MSS_USBH_CIF_cep_is_rxstall_err
(
    void
)
{
    return (((USB->ENDPOINT[MSS_USB_CEP].TX_CSR & CSR0L_HOST_STALL_RCVD_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_cep_clr_rxstall_err
(
    void
)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR &= ~CSR0L_HOST_STALL_RCVD_MASK;
}

static __INLINE uint8_t
MSS_USBH_CIF_cep_is_retry_err
(
    void
)
{
    return (((USB->ENDPOINT[MSS_USB_CEP].TX_CSR & CSR0L_HOST_RETRY_ERR_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_cep_clr_retry_err
(
    void
)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR &= ~CSR0L_HOST_RETRY_ERR_MASK;
}

static __INLINE void
MSS_USBH_CIF_cep_clr_csr_reg
(
    void
)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR = 0x0000u;
}

static __INLINE uint8_t
MSS_USBH_CIF_cep_is_naktimeout_err
(
    void
)
{
    return (((USB->ENDPOINT[MSS_USB_CEP].TX_CSR & CSR0L_HOST_NAK_TIMEOUT_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_cep_clr_naktimeout_err
(
    void
)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR &= ~CSR0L_HOST_NAK_TIMEOUT_MASK;
}

/*-------------------------------------------------------------------------*//**
  CSR0H register related APIs
 */
static __INLINE void
MSS_USBH_CIF_cep_flush_fifo
(
    mss_usb_ep_num_t ep_num
)
{
    /* should be set when TxPktRdy/RxPktRdy is set */
    if ((USB->ENDPOINT[ep_num].TX_CSR) & (TxCSRL_HOST_EPN_TX_PKT_RDY_MASK |
                                          CSR0L_HOST_RX_PKT_RDY_MASK))
    {
        USB->ENDPOINT[MSS_USB_CEP].TX_CSR |= CSR0H_HOST_FLUSH_FIFO_MASK;
    }
}

static __INLINE uint8_t
MSS_USBH_CIF_cep_is_data_tog
(
    void
)
{
    return (((USB->ENDPOINT[MSS_USB_CEP].TX_CSR & CSR0H_HOST_DATA_TOG_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_cep_disable_ping
(
    void
)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR |= CSR0H_HOST_DISABLE_PING_MASK;
}

static __INLINE void
MSS_USBH_CIF_cep_enable_ping
(
    void
)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR &= ~CSR0H_HOST_DISABLE_PING_MASK;
}

/*-------------------------------------------------------------------------*//**
  NAKLIMIT0 register related APIs
 */
static __INLINE void
MSS_USBH_CIF_cep_set_naklimit
(
    uint8_t naklimit
)
{
    if((naklimit >= 2u ) && (naklimit <= 16u))
    {
        USB->ENDPOINT[MSS_USB_CEP].TX_INTERVAL = naklimit;
    }
}

/*-------------------------------------------------------------------------*//**
  ENDOF - Host mode functions
 */

/* TXMAXP is common to host and device */
/*-------------------------------------------------------------------------*//**
  TXCSRL register related APIs
 */

static __INLINE void MSS_USBH_CIF_tx_ep_set_txpktrdy(mss_usb_ep_num_t ep_num)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRL_HOST_EPN_TX_PKT_RDY_MASK;
}

static __INLINE uint8_t
MSS_USBH_CIF_tx_ep_is_txpktrdy
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRL_HOST_EPN_TX_PKT_RDY_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE uint8_t
MSS_USBH_CIF_tx_ep_is_fifo_ne
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRL_HOST_EPN_TX_FIFO_NE_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE uint8_t
MSS_USBH_CIF_tx_ep_is_retry_err
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRL_HOST_EPN_RESPONSE_ERR_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_tx_ep_clr_retry_err
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRL_HOST_EPN_RESPONSE_ERR_MASK;
}

static __INLINE void
MSS_USBH_CIF_tx_ep_flush_fifo_reg
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= (TxCSRL_HOST_EPN_FLUSH_FIFO_MASK |
                                    TxCSRL_HOST_EPN_TX_PKT_RDY_MASK);
}

static __INLINE uint8_t
MSS_USBH_CIF_tx_ep_is_rxstall_err
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRL_HOST_EPN_STALL_RCVD_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_tx_ep_clr_rxstall_err
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRL_HOST_EPN_STALL_RCVD_MASK;
}

static __INLINE void
MSS_USBH_CIF_tx_ep_clr_data_tog
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRL_HOST_EPN_CLR_DATA_TOG_MASK;
}


static __INLINE uint8_t
MSS_USBH_CIF_tx_ep_is_naktimeout_err
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRL_HOST_EPN_NAK_TIMEOUT_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_tx_ep_clr_naktimeout_err
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRL_HOST_EPN_NAK_TIMEOUT_MASK ;
}

/*-------------------------------------------------------------------------*//**
  TXCSRH register related APIs
 */

static __INLINE uint8_t
MSS_USBH_CIF_tx_ep_is_data_tog
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRH_HOST_EPN_DATA_TOG_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_tx_ep_write_data_tog
(
    mss_usb_ep_num_t ep_num,
    uint32_t data_tog
)
{
    /* data_tog has to be 0 or 1 */
    USB->ENDPOINT[ep_num].TX_CSR = data_tog & 0x00000001U;
}

static __INLINE void
MSS_USBH_CIF_tx_ep_set_data_tog_we
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRH_HOST_EPN_DATA_TOG_WE_MASK;
}

static __INLINE void
MSS_USBH_CIF_tx_ep_clr_data_tog_we
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRH_HOST_EPN_DATA_TOG_WE_MASK;
}

static __INLINE void
MSS_USBH_CIF_tx_ep_set_dma_mode1
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRH_HOST_EPN_DMA_MODE_MASK;
}

static __INLINE void
MSS_USBH_CIF_tx_ep_set_dma_mode0
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRH_HOST_EPN_DMA_MODE_MASK;
}

static __INLINE uint8_t
MSS_USBH_CIF_tx_ep_get_dma_mode
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRH_HOST_EPN_DMA_MODE_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_tx_ep_set_force_data_tog
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRH_HOST_EPN_FRC_DATA_TOG_MASK;
}

static __INLINE void
MSS_USBH_CIF_tx_ep_clr_force_data_tog
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRH_HOST_EPN_FRC_DATA_TOG_MASK;
}

static __INLINE void
MSS_USBH_CIF_tx_ep_enable_dma
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRH_HOST_EPN_ENABLE_DMA_MASK;
}

static __INLINE void
MSS_USBH_CIF_tx_ep_disable_dma
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRH_HOST_EPN_ENABLE_DMA_MASK;
}

static __INLINE uint8_t
MSS_USBH_CIF_tx_ep_is_dma_enabled
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRH_HOST_EPN_ENABLE_DMA_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_tx_ep_set_autoset
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRH_HOST_EPN_ENABLE_AUTOSET_MASK;
}
static __INLINE void
MSS_USBH_CIF_tx_ep_clr_autoset
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_CSR &= ~TxCSRH_HOST_EPN_ENABLE_AUTOSET_MASK;
}

static __INLINE uint8_t
MSS_USBH_CIF_tx_ep_is_autoset
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].TX_CSR & TxCSRH_HOST_EPN_ENABLE_AUTOSET_MASK) ?
                                    MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

/*-------------------------------------------------------------------------*//**
  RXCSRL register related APIs
 */
static __INLINE uint8_t
MSS_USBH_CIF_rx_ep_is_rxpktrdy
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RXCSRL_HOST_EPN_RX_PKT_RDY_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE uint8_t
MSS_USBH_CIF_rx_ep_is_fifo_full
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RXCSRL_HOST_EPN_RX_FIFO_FULL_MASK) ?
                                    MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE uint8_t
MSS_USBH_CIF_rx_ep_is_retry_err
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RXCSRL_HOST_EPN_RESPONSE_ERR_MASK) ?
                                    MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_rx_ep_clr_retry_err
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR &= ~RXCSRL_HOST_EPN_RESPONSE_ERR_MASK;
}

static __INLINE uint8_t
MSS_USBH_CIF_rx_ep_is_naktimeout_err
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RXCSRL_HOST_EPN_NAK_TIMEOUT_ERR_MASK) ?
                                    MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_rx_ep_clr_naktimeout_err
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR &= ~RXCSRL_HOST_EPN_NAK_TIMEOUT_ERR_MASK;
}

static __INLINE void
MSS_USBH_CIF_rx_ep_flush_fifo_reg
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR |= RXCSRL_HOST_EPN_FLUSH_FIFO_MASK;
}

static __INLINE uint8_t
MSS_USBH_CIF_rx_ep_is_rxstall_err
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RXCSRL_HOST_EPN_STALL_RCVD_MASK) ?
                                    MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_rx_ep_clr_rxstall_err
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR &= ~RXCSRL_HOST_EPN_STALL_RCVD_MASK;
}

static __INLINE void
MSS_USBH_CIF_rx_ep_clr_data_tog
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR |= RXCSRL_HOST_EPN_CLR_DATA_TOG_MASK;
}

/*-------------------------------------------------------------------------*//**
  RXCSRH register related APIs
 */
static __INLINE uint8_t
MSS_USBH_CIF_rx_ep_is_iso_incomp
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RXCSRH_HOST_EPN_RX_ISO_INCOMP) ?
                            MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE uint8_t
MSS_USBH_CIF_rx_ep_is_data_tog
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RXCSRH_HOST_EPN_DATA_TOG_MASK) ?
                            MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_rx_ep_write_data_tog
(
    mss_usb_ep_num_t ep_num,
    uint32_t data_tog
)
{
    USB->ENDPOINT[ep_num].RX_CSR = data_tog;
}

static __INLINE void
MSS_USBH_CIF_rx_ep_set_data_tog_we
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR |= RXCSRH_HOST_EPN_DATA_TOG_WE_MASK;
}

static __INLINE void
MSS_USBH_CIF_rx_ep_clr_data_tog_we
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR &= ~RXCSRH_HOST_EPN_DATA_TOG_WE_MASK;
}

static __INLINE mss_usb_dma_mode_t
MSS_USBH_CIF_rx_ep_get_dma_mode
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RXCSRH_HOST_EPN_DMA_MODE_MASK) ?
                                    MSS_USB_DMA_MODE1 : MSS_USB_DMA_MODE0));
}

static __INLINE void
MSS_USBH_CIF_rx_ep_set_dma_mode1
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR |= RXCSRH_HOST_EPN_DMA_MODE_MASK;
}

static __INLINE void
MSS_USBH_CIF_rx_ep_set_dma_mode0
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR &= ~RXCSRH_HOST_EPN_DMA_MODE_MASK;
}

static __INLINE void
MSS_USBH_CIF_rx_ep_enable_dma
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR |= RXCSRH_HOST_EPN_ENABLE_DMA_MASK;
}

static __INLINE void
MSS_USBH_CIF_rx_ep_disable_dma
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR &= ~RXCSRH_HOST_EPN_ENABLE_DMA_MASK;
}

static __INLINE uint8_t
MSS_USBH_CIF_rx_ep_is_dma_enabled
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RXCSRH_HOST_EPN_ENABLE_DMA_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USBH_CIF_rx_ep_set_autoreq
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR |= RXCSRH_HOST_EPN_ENABLE_AUTOREQ_MASK;
}

static __INLINE void
MSS_USBH_CIF_rx_ep_clr_autoreq
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR &= ~RXCSRH_HOST_EPN_ENABLE_AUTOREQ_MASK;
}

static __INLINE void
MSS_USBH_CIF_rx_ep_set_autoclr
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR |= RXCSRH_HOST_EPN_ENABLE_AUTOCLR_MASK;
}

static __INLINE void
MSS_USBH_CIF_rx_ep_clr_autoclr
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR &= ~RXCSRH_HOST_EPN_ENABLE_AUTOCLR_MASK;
}

static __INLINE uint8_t
MSS_USBH_CIF_rx_ep_is_autoclr
(
    mss_usb_ep_num_t ep_num
)
{
    return (((USB->ENDPOINT[ep_num].RX_CSR & RXCSRH_HOST_EPN_ENABLE_AUTOCLR_MASK) ?
                                    MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

/*-------------------------------------------------------------------------*//**
  TXTYPE register related APIs
 */
static __INLINE void
MSS_USBH_CIF_tx_ep_set_target_speed
(
    mss_usb_ep_num_t ep_num,
    mss_usb_device_speed_t speed
)
{
    USB->ENDPOINT[ep_num].TX_TYPE &= ~TXTYPE_HOST_TARGET_EP_SPEED_MASK;
    USB->ENDPOINT[ep_num].TX_TYPE |= (speed <<
                                            TXTYPE_HOST_TARGET_EP_SPEED_SHIFT);
}

static __INLINE mss_usb_device_speed_t
MSS_USBH_CIF_tx_ep_get_target_speed
(
    mss_usb_ep_num_t ep_num
)
{
    return ((mss_usb_device_speed_t)((USB->ENDPOINT[ep_num].TX_TYPE &
      TXTYPE_HOST_TARGET_EP_SPEED_MASK) >> TXTYPE_HOST_TARGET_EP_SPEED_SHIFT));
}

static __INLINE void
MSS_USBH_CIF_tx_ep_set_target_protocol
(
    mss_usb_ep_num_t ep_num,
    mss_usb_xfr_type_t xfr_type
)
{
    USB->ENDPOINT[ep_num].TX_TYPE &= ~TXTYPE_HOST_TARGET_EP_PROTOCOL_MASK;
    USB->ENDPOINT[ep_num].TX_TYPE |= (xfr_type <<
                                         TXTYPE_HOST_TARGET_EP_PROTOCOL_SHIFT);
}

static __INLINE mss_usb_xfr_type_t
MSS_USBH_CIF_tx_ep_get_target_protocol
(
    mss_usb_ep_num_t ep_num
)
{
    return ((mss_usb_xfr_type_t)((USB->ENDPOINT[ep_num].TX_TYPE &
    TXTYPE_HOST_TARGET_EP_PROTOCOL_MASK) >> TXTYPE_HOST_TARGET_EP_PROTOCOL_SHIFT));
}

static __INLINE void
MSS_USBH_CIF_tx_ep_set_target_ep_no
(
    mss_usb_ep_num_t ep_num,
    uint8_t tdev_ep_num
)
{
    USB->ENDPOINT[ep_num].TX_TYPE &= ~TXTYPE_HOST_TARGET_EP_NUM_MASK;
    USB->ENDPOINT[ep_num].TX_TYPE |= (tdev_ep_num <<
                                              TXTYPE_HOST_TARGET_EP_NUM_SHIFT);
}

static __INLINE uint8_t
MSS_USBH_CIF_tx_ep_get_target_ep_no
(
    mss_usb_ep_num_t ep_num
)
{
    return ((mss_usb_ep_num_t)((USB->ENDPOINT[ep_num].TX_TYPE &
          TXTYPE_HOST_TARGET_EP_NUM_MASK) >> TXTYPE_HOST_TARGET_EP_NUM_SHIFT));
}

static __INLINE void
MSS_USBH_CIF_tx_ep_clr_txtypereg
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].TX_TYPE = 0x0000U;
}
/*-------------------------------------------------------------------------*//**
  TXINTERVAL register related APIs
 */
static __INLINE void
MSS_USBH_CIF_tx_ep_set_target_interval
(
    mss_usb_ep_num_t ep_num,
    uint8_t interval
)
{
    USB->ENDPOINT[ep_num].TX_INTERVAL = interval;
}

/*-------------------------------------------------------------------------*//**
  RXTYPE register related APIs
 */
static __INLINE void
MSS_USBH_CIF_rx_ep_set_target_speed
(
    mss_usb_ep_num_t ep_num,
    mss_usb_device_speed_t speed
)
{
    USB->ENDPOINT[ep_num].RX_TYPE &= ~RXTYPE_HOST_TARGET_EP_SPEED_MASK;
    USB->ENDPOINT[ep_num].RX_TYPE |= (speed << RXTYPE_HOST_TARGET_EP_SPEED_SHIFT);
}

static __INLINE mss_usb_device_speed_t
MSS_USBH_CIF_rx_ep_get_target_speed
(
    mss_usb_ep_num_t ep_num
)
{
    return ((mss_usb_device_speed_t)((USB->ENDPOINT[ep_num].RX_TYPE &
     RXTYPE_HOST_TARGET_EP_SPEED_MASK) >> RXTYPE_HOST_TARGET_EP_SPEED_SHIFT));
}

static __INLINE void
MSS_USBH_CIF_rx_ep_set_target_protocol
(
    mss_usb_ep_num_t ep_num,
    mss_usb_xfr_type_t xfr_type
)
{
    USB->ENDPOINT[ep_num].RX_TYPE &= ~RXTYPE_HOST_TARGET_EP_PROTOCOL_MASK;
    USB->ENDPOINT[ep_num].RX_TYPE |= (xfr_type << RXTYPE_HOST_TARGET_EP_PROTOCOL_SHIFT);
}

static __INLINE mss_usb_xfr_type_t
MSS_USBH_CIF_rx_ep_get_target_protocol
(
    mss_usb_ep_num_t ep_num
)
{
    return ((mss_usb_xfr_type_t)((USB->ENDPOINT[ep_num].RX_TYPE &
    RXTYPE_HOST_TARGET_EP_PROTOCOL_MASK) >> RXTYPE_HOST_TARGET_EP_PROTOCOL_SHIFT));
}

static __INLINE void
MSS_USBH_CIF_rx_ep_set_target_ep_no
(
    mss_usb_ep_num_t ep_num,
    uint8_t tdev_ep_num
)
{
    USB->ENDPOINT[ep_num].RX_TYPE &= ~RXTYPE_HOST_TARGET_EP_NUM_MASK;
    USB->ENDPOINT[ep_num].RX_TYPE |= (tdev_ep_num << RXTYPE_HOST_TARGET_EP_NUM_SHIFT);
}

static __INLINE uint8_t
MSS_USBH_CIF_rx_ep_get_target_ep_no
(
    mss_usb_ep_num_t ep_num
)
{
    return ((mss_usb_ep_num_t)((USB->ENDPOINT[ep_num].RX_TYPE &
    RXTYPE_HOST_TARGET_EP_NUM_MASK) >> RXTYPE_HOST_TARGET_EP_NUM_SHIFT));
}

static __INLINE void
MSS_USBH_CIF_rx_ep_clr_rxtypereg
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_TYPE = 0x0000U;
}
/*-------------------------------------------------------------------------*//**
  RXINTERVAL register related APIs
 */
static __INLINE void
MSS_USBH_CIF_rx_ep_set_target_interval
(
    mss_usb_ep_num_t ep_num,
    uint8_t interval
)
{
    USB->ENDPOINT[ep_num].RX_INTERVAL = interval;
}

/*-------------------------------------------------------------------------*//**
  REQPKTCOUNT register related APIs
 */
static __INLINE void
MSS_USBH_CIF_rx_ep_set_reqpkt_count
(
    mss_usb_ep_num_t ep_num,
    uint32_t count
)
{
    /*
     * Used with AUTOREQ option.
     * Multiple packets combined into a single bulk  packet within the FIFO count
     * as one packet.
     */
    USB->RQ_PKT_CNT[ep_num] = count;
}

/*-------------------------------------------------------------------------*//**
  RXFUNCADDR register related APIs
 */
static __INLINE void
MSS_USBH_CIF_rx_ep_set_target_func_addr
(
    mss_usb_ep_num_t ep_num,
    uint32_t addr
)
{
    /* Device number of the target - initially zero, then determined by
     * enumeration process.
     */
    USB->TAR[ep_num].RX_FUNC_ADDR = (addr & 0x7Fu);
}

/*-------------------------------------------------------------------------*//**
 * TXHUBADDR register related APIs
 */
static __INLINE void
MSS_USBH_CIF_tx_ep_set_target_hub_addr
(
    mss_usb_ep_num_t ep_num,
    uint32_t addr,
    uint32_t mtt
)
{
    /* Device number of the target - initially zero, then determined by
     * enumeration process.
     */
    USB->TAR[ep_num].TX_HUB_ADDR = ((uint8_t)(mtt << TARGET_DEVICE_HUB_MT_SHIFT) |
                                    (addr & 0x7Fu));
}

/*-------------------------------------------------------------------------*//**
  RXHUBADDR register related APIs
 */
static __INLINE void
MSS_USBH_CIF_rx_ep_set_target_hub_addr
(
    mss_usb_ep_num_t ep_num,
    uint32_t addr,
    uint32_t mtt
)
{
    /* Device number of the target - initially zero, then determined by
     * enumeration process.
     */
    USB->TAR[ep_num].RX_HUB_ADDR = ((uint8_t)(mtt << TARGET_DEVICE_HUB_MT_SHIFT) |
                                    (addr & 0x7Fu));
}

/*-------------------------------------------------------------------------*//**
  TXHUBADDR register related APIs
 */
static __INLINE void
MSS_USBH_CIF_tx_ep_set_target_hub_port
(
    mss_usb_ep_num_t ep_num,
    uint32_t port_no
)
{
    /* for FS/LS devices only */
    USB->TAR[ep_num].TX_HUB_PORT = (port_no & 0xFFu);
}

/*-------------------------------------------------------------------------*//**
  RXHUBADDR register related APIs
 */
static __INLINE void
MSS_USBH_CIF_rx_ep_set_target_hub_port
(
    mss_usb_ep_num_t ep_num,
    uint32_t port_no
)
{
    /* for FS/LS devices only */
    USB->TAR[ep_num].RX_HUB_PORT = (port_no & 0xFFu);
}

/* Trailing zeros */
static __INLINE uint32_t
MSS_USBH_CIF_ctz
(
    uint32_t number
)
{
    uint32_t c = 32u;
    number &= -(int32_t)(number);
    if (number)
    {
        c--;
    }
    if (number & 0x0000FFFFU)
    {
        c -= 16;
    }
    if (number & 0x00FF00FFU)
    {
        c -= 8;
    }
    if (number & 0x0F0F0F0FU)
    {
        c -= 4;
    }
    if (number & 0x33333333U)
    {
        c -= 2;
    }
    if (number & 0x55555555U)
    {
        c -= 1;
    }

    return c;
}


#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_HOST_REG_IO_H_ */
