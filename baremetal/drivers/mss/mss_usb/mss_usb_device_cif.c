/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC MSS USB Driver Stack
 *      USB Core Interface Layer (USB-CIFL)
 *          USBD-CIF driver
 *
 * USBD-CIF driver implementation:
 * This file implements MSS USB core initialization in device mode and
 * implements core interface function for the logical layer to control the
 * MSS USB core in USB Device mode.
 *
 */

#include "mss_usb_device_cif.h"
#include "mss_usb_core_regs.h"
#include "mss_usb_common_cif.h"
#include "mss_usb_device_reg_io.h"
#include "mss_usb_common_reg_io.h"
#include "mss_plic.h"
#include "mss_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_DEVICE_ENABLED

/***************************************************************************//**
 These pointers are used for holding the Receive transfer parameters (Buffer
 address, length) etc. This pointer will be updated in
 MSS_USBD_CIF_rx_ep_read_prepare() API and used in
 MSS_USB_CIF_handle_rx_ep_irq().
 */
volatile mss_usbd_cep_state_t cep_state;

/***************************************************************************//**
 EXPORTED APIs
 ******************************************************************************/

/***************************************************************************//**
 This function initializes the MSS USB core in Device mode.
 */
volatile uint32_t reg=0;

void MSS_USBD_CIF_init(mss_usb_device_speed_t speed)
{
    cep_state = MSS_USB_CTRL_EP_IDLE;

    /*TODO: Avoid resetting twice in DualRole mode*/
    MSS_USB_CIF_soft_reset();
    MSS_USB_CIF_clr_usb_irq_reg();

    /*Reset and Resume are by default enabled in INTRUSBE reg after soft reset*/
    PLIC_EnableIRQ(USB_DMA_PLIC);
    PLIC_EnableIRQ(USB_MC_PLIC);

    MSS_USB_CIF_rx_ep_disable_irq_all();
    MSS_USB_CIF_tx_ep_disable_irq_all();

    if(MSS_USB_DEVICE_FS == speed)
    {
        MSS_USB_CIF_disable_hs_mode();
    }

    /*This was added during Compliance testing. Refer MUSB section 3.8.5*/
    USB->C_T_HSBT = 0x01u;
}

/***************************************************************************//**
 Provides the information about the MSS USB core configuration
 */
void MSS_USBD_CIF_get_hwcore_info(mss_usb_core_info_t* hw_core)
{
    MSS_USB_CIF_set_index_reg(MSS_USB_CEP);
    hw_core->core_max_nbr_of_tx_ep = (USB->EP_INFO & 0x0Fu); /*lower nibble for txep*/
    hw_core->core_max_nbr_of_rx_ep = ((USB->EP_INFO & 0xF0u) >> 4u);/*higher nibble for txep*/
    hw_core->core_ram_bus_width = (USB->RAM_INFO & 0x0Fu);/*lower nibble for bus width*/
    hw_core->core_max_nbr_of_dma_chan = ((USB->RAM_INFO & 0xF0u) >> 4u);/*higher nibble for dma channels*/
    hw_core->core_WTCON = ((USB->LINK_INFO & 0xF0u) >> 4u);/*refer musb section 3 . 7 . 3*/
    hw_core->core_WTID = (USB->LINK_INFO & 0x0Fu);/*refer musb section 3 . 7 . 3*/
    hw_core->core_VPLEN = USB->VP_LEN;
    hw_core->core_HS_EOF1 = USB->HS_EOF1;
    hw_core->core_FS_EOF1 = USB->FS_EOF1;
    hw_core->core_LS_EOF1 = USB->LS_EOF1;
    hw_core->core_configdata = USB->INDEXED_CSR.DEVICE_EP0.CONFIG_DATA;
}

/***************************************************************************//**
 Configures the registers related to TX EP for data transfer operations as
 per the parameters provided by the upper layer.
 */
void MSS_USBD_CIF_tx_ep_configure(mss_usb_ep_t* device_ep)
{
    MSS_USB_CIF_tx_ep_clr_csrreg(device_ep->num); //clear previous config, if any

    MSS_USB_CIF_tx_ep_clr_underrun(device_ep->num);
    MSS_USB_CIF_tx_ep_clr_send_stall_bit(device_ep->num);
    MSS_USB_CIF_tx_ep_clr_stall_sent_bit(device_ep->num);

    switch(device_ep->xfr_type)
    {
        case MSS_USB_XFR_INTERRUPT:
            MSS_USB_CIF_tx_ep_disable_iso(device_ep->num);
            MSS_USB_CIF_tx_ep_clr_autoset(device_ep->num);
        break;

        case MSS_USB_XFR_BULK:
            MSS_USB_CIF_tx_ep_disable_iso(device_ep->num);
            if(device_ep->dma_enable == DMA_ENABLE)
                MSS_USB_CIF_tx_ep_set_autoset(device_ep->num);
        break;

        case MSS_USB_XFR_ISO:
            /*Error check and Data toggle is ignored in ISO transfers*/
            MSS_USB_CIF_tx_ep_enable_iso(device_ep->num);
            MSS_USB_CIF_tx_ep_clr_autoset(device_ep->num);
        break;

        default:
            ASSERT(0);
        break;
    }

    /*Do the common configuration for TX EP*/
    MSS_USB_CIF_tx_ep_configure(device_ep);
}

/***************************************************************************//**
 Configures the RX EP for data transfer operations as per the parameters
 provided by upper layer.
 */
void MSS_USBD_CIF_rx_ep_configure(mss_usb_ep_t* device_ep)
{
    MSS_USB_CIF_rx_ep_clr_csrreg(device_ep->num); //clear previous config, if any

    MSS_USB_CIF_rx_ep_clr_overrun(device_ep->num);
    MSS_USB_CIF_rx_ep_clr_stall_sent_bit(device_ep->num);
    MSS_USB_CIF_rx_ep_clr_send_stall_bit(device_ep->num);

    switch(device_ep->xfr_type)
    {
        case MSS_USB_XFR_INTERRUPT:
            MSS_USB_CIF_rx_ep_disable_iso(device_ep->num);
            MSS_USB_CIF_rx_ep_disable_nyet(device_ep->num);
            MSS_USB_CIF_rx_ep_clr_autoclr(device_ep->num);
        break;

        case MSS_USB_XFR_BULK:
            MSS_USB_CIF_rx_ep_disable_iso(device_ep->num);
            MSS_USB_CIF_rx_ep_enable_nyet(device_ep->num);
            MSS_USB_CIF_rx_ep_set_autoclr(device_ep->num);
        break;

        case MSS_USB_XFR_ISO:
            MSS_USB_CIF_rx_ep_clr_autoclr(device_ep->num);
            MSS_USB_CIF_rx_ep_enable_iso(device_ep->num);
            MSS_USB_CIF_rx_ep_disable_nyet(device_ep->num);
        break;

        default:
            ASSERT(0);
        break;
    }

    /*Do the common configuration for RX EP*/
    MSS_USB_CIF_rx_ep_configure(device_ep);
}

/***************************************************************************//**
 Configures the Control EP for data transfer operations.
 */
void MSS_USBD_CIF_cep_configure(void)
{
    /*Control transfers will be handled without DMA*/
    MSS_USB_CIF_set_index_reg(MSS_USB_CEP);
    MSS_USB_CIF_cep_reset_csr0_reg();
    MSS_USB_CIF_cep_enable_irq();
}

/***************************************************************************//**
 Prepares the Control EP for receiving data as per the parameters provided by
 upper layer
 */
void MSS_USBD_CIF_cep_rx_prepare(mss_usb_ep_t* device_ep)
{
    if(MSS_USB_CEP_IDLE == device_ep[MSS_USB_CEP].state)
    {
        cep_state = MSS_USB_CTRL_EP_IDLE;
    }
    else if((MSS_USB_CEP_SETUP == device_ep[MSS_USB_CEP].state) ||
           (MSS_USB_CEP_RX == device_ep[MSS_USB_CEP].state))
    {
        cep_state = MSS_USB_CTRL_EP_RX;
    }
}

/***************************************************************************//**
 Prepares the RX EP for receiving data as per parameters provided by upper layer
 */
void MSS_USBD_CIF_rx_ep_read_prepare(mss_usb_ep_t* device_ep)
{
    /*Fixed Buffer overwriting issue found with printer driver and
      issue with interrupt transfer with DMA by moving the location
      of interrupt enable function*/
    if(DMA_ENABLE == device_ep->dma_enable)
    {
        /*Make sure that address is Modulo-4.Bits D0-D1 are read only.*/
        ASSERT(!(((ptrdiff_t)device_ep->buf_addr) & 0x00000002));

        MSS_USB_CIF_dma_write_addr(device_ep->dma_channel,
                                   (ptrdiff_t)device_ep->buf_addr);

        /*
         DMA Count register will be loaded after receive interrupt occurs.
         Mode need to be set every time since M1 to M0 transition might have
         happened for "short packet".
         */
         if(MSS_USB_XFR_BULK == device_ep->xfr_type)
        {
            MSS_USB_CIF_rx_ep_set_dma_mode1(device_ep->num);
            MSS_USB_CIF_rx_ep_set_autoclr(device_ep->num);
            MSS_USB_CIF_rx_ep_enable_dma(device_ep->num);

            MSS_USB_CIF_dma_write_count(device_ep->dma_channel,
                                        device_ep->xfr_length);

            MSS_USB_CIF_rx_ep_enable_irq(device_ep->num);

            /*Handling single NULL packet reception*/
            if(0u != device_ep->xfr_length )
            {
                MSS_USB_CIF_dma_start_xfr(device_ep->dma_channel);
            }
        }
        else
        {
            MSS_USB_CIF_rx_ep_clr_autoclr(device_ep->num);
            MSS_USB_CIF_rx_ep_set_dma_mode0(device_ep->num);

            /*MUSB section 17.4.1*/
            MSS_USB_CIF_rx_ep_disable_dma(device_ep->num);
            MSS_USB_CIF_rx_ep_enable_irq(device_ep->num);
        }
    }
    else
    {
        MSS_USB_CIF_rx_ep_enable_irq(device_ep->num);
    }
}

/***************************************************************************//**
 Writes a data packet on EP0 in device mode(control endpoint).
 */
void MSS_USBD_CIF_cep_write_pkt(mss_usb_ep_t* device_ep)
{
    if(device_ep->num == MSS_USB_CEP)
    {
        /* null buffer, xfr type, transaction type */
        if((0 != device_ep->buf_addr) && (MSS_USB_XFR_CONTROL == device_ep->xfr_type))
        {
            MSS_USB_CIF_load_tx_fifo(device_ep->num,
                                     device_ep->buf_addr,
                                     device_ep->txn_length);

            device_ep->txn_count = device_ep->txn_length;
            device_ep->xfr_count += device_ep->txn_length;
        }

        if(MSS_USB_CEP_TX == device_ep->state)
        {
            cep_state = MSS_USB_CTRL_EP_TX;
        }

        if(device_ep->xfr_count < device_ep->xfr_length)
        {
            MSS_USB_CIF_cep_set_txpktrdy();
        }
        else if(device_ep->xfr_count >= device_ep->xfr_length)
        {
            MSS_USBD_CIF_cep_end_rdr();
        }
    }
}

/***************************************************************************//**
 Reads data packet arrived on EP0 in device mode(control endpoint).
 */
void MSS_USBD_CIF_cep_read_pkt(mss_usb_ep_t* device_ep)
{
    volatile uint16_t received_count = 0u;

    MSS_USB_CIF_set_index_reg(MSS_USB_CEP);

    if((MSS_USB_CEP == device_ep->num) && (0 != device_ep->buf_addr))
    {
        /*TODO: check stalled, null buffer, transfer type, transaction type */
        received_count = MSS_USB_CIF_cep_rx_byte_count();

        MSS_USB_CIF_read_rx_fifo(MSS_USB_CEP,
                                 device_ep->buf_addr,
                                 received_count);

        device_ep->xfr_count += received_count;
        device_ep->txn_count = received_count;
    }
}

#endif  //MSS_USB_DEVICE_ENABLED

#ifdef __cplusplus
}
#endif
