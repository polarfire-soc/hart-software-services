/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Microchip PolarFire SoC MSS USB Driver Stack
 *      USB Core Interface Layer (USB-CIFL)
 *          USB-CIF driver
 *
 * USB-CIF driver implementation:
 * This source file implements MSS USB Interrupt handler functions. This file
 * also implements core interface function for the logical layer to control
 * the MSS USB core. These interface functions are independent of the USB mode.
 *
 *
 * SVN $Revision$
 * SVN $Date$
 */

#include "mss_usb_common_cif.h"

#include "mss_assert.h"
#include "mss_usb_common_reg_io.h"
#include "mss_usb_core_regs.h"
#include "mss_plic.h"

#ifdef MSS_USB_HOST_ENABLED
#include "mss_usb_host_cif.h"
#include "mss_usb_host_reg_io.h"
#endif  /* MSS_USB_HOST_ENABLED */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_HOST_ENABLED
static uint8_t MSS_USB_CIF_host_rx_errchk(mss_usb_ep_num_t ep_num);
static uint8_t MSS_USB_CIF_host_tx_errchk(mss_usb_ep_num_t ep_num);
#endif  /* MSS_USB_HOST_ENABLED */

#ifdef MSS_USB_DEVICE_ENABLED
static uint8_t MSS_USB_CIF_device_rx_errchk(mss_usb_ep_num_t ep_num);
static uint8_t MSS_USB_CIF_device_tx_errchk(mss_usb_ep_num_t ep_num);
#endif  /* MSS_USB_HOST_ENABLED */

/***************************************************************************//**
* Global variables shared by mss_usb_device_cif.c and mss_usb_common_cif.c
*/
#ifdef MSS_USB_DEVICE_ENABLED
    extern mss_usbd_cb_t g_mss_usbd_cb;
    extern volatile mss_usbd_cep_state_t cep_state;
#endif  /* MSS_USB_HOST_ENABLED */

#ifdef MSS_USB_HOST_ENABLED
    extern mss_usbh_cb_t g_mss_usbh_cb;
#endif /* MSS_USB_HOST_ENABLED */

/***************************************************************************//**
 * Private functions declarations of USB-CIF.
 ******************************************************************************/
static void
MSS_USB_CIF_handle_cep_irq
(
    void
);

static void
MSS_USB_CIF_handle_tx_ep_irq
(
    uint16_t irq_num
);

static void
MSS_USB_CIF_handle_rx_ep_irq
(
    uint16_t irq_num
);

/***************************************************************************//**
 * Main USB interrupt handler. It checks for TX/RX endpoint interrupts and USB
 * system level interrupts and calls the appropriate routine.
 */
uint8_t usb_mc_plic_IRQHandler
(
    void
)
{
    volatile uint8_t usb_irq;
    volatile uint16_t tx_ep_irq;
    volatile uint16_t rx_ep_irq;
    volatile uint8_t role;

    usb_irq = MSS_USB_CIF_read_irq_reg();
    tx_ep_irq = MSS_USB_CIF_read_tx_ep_irq_reg();
    rx_ep_irq = MSS_USB_CIF_read_rx_ep_irq_reg();

    /*
     When operating in Host mode, on detecting Disconnect event, Disconnect
     Interrupt occurs but the HostMode bit in DevCtl is also cleared.
     Hence moving Disconnect handling out of get_mode condition.
     In the event of Disconnection, The decision is made based on the B-Device
     bit(DevCtl-D7).
     */
    if (usb_irq & DISCONNECT_IRQ_MASK)
    {
        role = MSS_USB_CIF_get_role();

#ifdef MSS_USB_DEVICE_ENABLED
        if (MSS_USB_DEVICE_ROLE_DEVICE_B == role)
        {
            MSS_USB_CIF_enable_usbirq(RESET_IRQ_MASK);
            g_mss_usbd_cb.usbd_disconnect();
        }
#endif  /* MSS_USB_DEVICE_ENABLED */

#ifdef MSS_USB_HOST_ENABLED
        if (MSS_USB_DEVICE_ROLE_DEVICE_A == role)
        {
            /*In Host mode, On removing the attached B-device the session bit
              somehow remains set.
              This bit need to be cleared otherwise RESET interrupt is not occurring
              when B-Device is connected after removing A-Device*/
            if (MSS_USB_CIF_is_session_on())
            {
                MSS_USB_CIF_stop_session();
            }

            MSS_USBH_CIF_read_vbus_level();
            g_mss_usbh_cb.usbh_disconnect();
        }
#endif /* MSS_USB_HOST_ENABLED */
    }

#ifdef MSS_USB_DEVICE_ENABLED
    if (MSS_USB_CORE_MODE_DEVICE == MSS_USB_CIF_get_mode())
    {
        if (usb_irq & RESUME_IRQ_MASK)
        {
            g_mss_usbd_cb.usbd_resume();
        }
        if (usb_irq & SUSPEND_IRQ_MASK)
        {
            g_mss_usbd_cb.usbd_suspend();
        }
        if (usb_irq & RESET_IRQ_MASK)
        {
            MSS_USB_CIF_set_index_reg(MSS_USB_CEP);
            MSS_USB_CIF_enable_usbirq(DISCONNECT_IRQ_MASK | SUSPEND_IRQ_MASK);
            cep_state = MSS_USB_CTRL_EP_IDLE;
            MSS_USB_CIF_clr_usb_irq_reg();
            MSS_USB_CIF_cep_clr_setupend();
            MSS_USB_CIF_cep_clr_stall_sent();
            g_mss_usbd_cb.usbd_reset();
        }
    }
#endif /* MSS_USB_DEVICE_ENABLED */

#ifdef MSS_USB_HOST_ENABLED
    if (MSS_USB_CORE_MODE_HOST == MSS_USB_CIF_get_mode())
    {
        role = MSS_USB_CIF_get_role();
        if (usb_irq & RESUME_IRQ_MASK)
        {
            /* Resume interrupt in Host mode means Remote Wakeup request */
        }

        /* Vbus_err and session request interrupts are valid only in A device */
        if (MSS_USB_DEVICE_ROLE_DEVICE_A == role)
        {
            if (usb_irq & SESSION_REQUEST_IRQ_MASK)
            {
                /* This means SRP initiated by Target device. */
            }
            if (usb_irq & VBUS_ERROR_IRQ_MASK)
            {
                /* Power management */
                MSS_USBH_CIF_read_vbus_level();
            }
        }

        if (usb_irq & CONNECT_IRQ_MASK)
        {
            MSS_USBH_CIF_handle_connect_irq();
        }

        if (usb_irq & BABBLE_IRQ_MASK)
        {
            /* Not supported yet */
        }

#if 0   /* SOF interrupt is not processed */
        if (usb_irq & SOF_IRQ_MASK)
        {
            g_mss_usbd_cb.usb_device_sof(0);
        }
#endif /* SOF interrupt is not processed */

    }
#endif    /* MSS_USB_HOST_ENABLED */

    if (tx_ep_irq & 0x0001u)
    {
        /* handle EP0 IRQ */
        MSS_USB_CIF_handle_cep_irq();
    }

    if (tx_ep_irq & 0xFFFEu)    /* EP0 is handled above */
    {
        /* Handle TX EP here, pass on the EP numbers.Mask EP0 bit */
        tx_ep_irq &= 0xFFFEu;
        MSS_USB_CIF_handle_tx_ep_irq(tx_ep_irq);
    }

    if (rx_ep_irq & 0xFFFEu)    /* bit0 is not defined */
    {
        /* Handle RX EP here, pass on the EP numbers */
        MSS_USB_CIF_handle_rx_ep_irq(rx_ep_irq);
    }

    return EXT_IRQ_KEEP_ENABLED;
}

/***************************************************************************//**
 * Routine to handle the interrupt on Control Endpoint.(EP0)
 */
static void
MSS_USB_CIF_handle_cep_irq
(
    void
)
{
    uint8_t status = 0u;

    MSS_USB_CIF_set_index_reg(MSS_USB_CEP);

#ifdef MSS_USB_DEVICE_ENABLED
    if (MSS_USB_CORE_MODE_DEVICE == MSS_USB_CIF_get_mode())
    {
        if (MSS_USB_CIF_cep_is_stall_sent())
        {
            status |= CTRL_EP_STALL_ERROR;
            MSS_USB_CIF_cep_clr_stall_sent();
            g_mss_usbd_cb.usbd_cep_setup(status);
        }
        else
        {
            if (MSS_USB_CIF_cep_is_setupend())
            {
                MSS_USB_CIF_cep_clr_setupend();

                if (!MSS_USB_CIF_cep_is_rxpktrdy())
                {
                    status |= CTRL_EP_SETUP_END_ERROR;
                }
                else
                {
                    status &= ~CTRL_EP_SETUP_END_ERROR;
                }

                g_mss_usbd_cb.usbd_cep_setup(status);
            }
            else
            {
                if (cep_state == MSS_USB_CTRL_EP_IDLE)
                {
                    if (MSS_USB_CIF_cep_is_rxpktrdy())
                    {
                        g_mss_usbd_cb.usbd_cep_setup(status);
                    }
                }
                else if (cep_state == MSS_USB_CTRL_EP_TX)
                {
                    g_mss_usbd_cb.usbd_cep_tx_complete(status);
                }
                else if (cep_state == MSS_USB_CTRL_EP_RX)
                {
                    if (MSS_USB_CIF_cep_is_rxpktrdy())
                    {
                        g_mss_usbd_cb.usbd_cep_rx(status);
                    }
                }
                else
                {
                    ASSERT(0);
                }
            }
        }
    }
#endif  /* MSS_USB_DEVICE_ENABLED */

#ifdef MSS_USB_HOST_ENABLED
    if (MSS_USB_CORE_MODE_HOST == MSS_USB_CIF_get_mode())
    {
        if (MSS_USBH_CIF_cep_is_rxstall_err())
        {
            status |= MSS_USB_EP_STALL_RCVD;
            MSS_USBH_CIF_cep_clr_rxstall_err();
        }
        if (MSS_USBH_CIF_cep_is_retry_err())
        {
            status |= MSS_USB_EP_NO_RESPONSE;
            MSS_USBH_CIF_cep_clr_retry_err();
        }
        if (MSS_USBH_CIF_cep_is_naktimeout_err())
        {
            status |= MSS_USB_EP_NAK_TOUT;
            MSS_USBH_CIF_cep_clr_naktimeout_err();
        }
        if (status == 0u)    /* No error was found */
        {
            status = MSS_USB_EP_TXN_SUCCESS;
        }
        g_mss_usbh_cb.usbh_cep(status);
    }
#endif /* MSS_USB_HOST_ENABLED */

}

/***************************************************************************//**
 * Routine to handle the interrupt on TX_EP
 */
static void MSS_USB_CIF_handle_tx_ep_irq
(
    uint16_t irq_num
)
{
    mss_usb_ep_num_t ep_num = MSS_USB_TX_EP_1;
    uint8_t status = 0u;

    while (irq_num)
    {
        irq_num >>= 1u;                                  /*EP1 starts from D1*/

        if (irq_num & MSS_USB_WORD_BIT_0_MASK)
        {
            MSS_USB_CIF_tx_ep_disable_irq(ep_num);
#ifdef MSS_USB_DEVICE_ENABLED
            if (MSS_USB_CORE_MODE_DEVICE == MSS_USB_CIF_get_mode())
            {
                status = MSS_USB_CIF_device_tx_errchk(ep_num);
                g_mss_usbd_cb.usbd_ep_tx_complete(ep_num,status);
            }
#endif  /* MSS_USB_DEVICE_ENABLED */

#ifdef MSS_USB_HOST_ENABLED
            if (MSS_USB_CORE_MODE_HOST == MSS_USB_CIF_get_mode())
            {
                status = MSS_USB_CIF_host_tx_errchk(ep_num);
                g_mss_usbh_cb.usbh_tx_complete((uint8_t)ep_num, status);
            }
#endif  /* MSS_USB_HOST_ENABLED */

            MSS_USB_CIF_tx_ep_enable_irq(ep_num);
        }
        status = 0u; /*resetting for next EP status*/
        ++ep_num;
    }
}

/***************************************************************************//**
 * Routine to handle the interrupt on RX EP
 */
static void MSS_USB_CIF_handle_rx_ep_irq
(
    uint16_t irq_num
)
{
    mss_usb_ep_num_t ep_num = MSS_USB_RX_EP_1;
    uint8_t status = 0u;

    while (irq_num)
    {
        irq_num >>= 1u;  /*EP1 starts from D1*/

        if (irq_num & MSS_USB_WORD_BIT_0_MASK)
        {
            MSS_USB_CIF_rx_ep_disable_irq(ep_num);
#ifdef MSS_USB_DEVICE_ENABLED
            if (MSS_USB_CORE_MODE_DEVICE == MSS_USB_CIF_get_mode())
            {
                status = MSS_USB_CIF_device_rx_errchk(ep_num);
                g_mss_usbd_cb.usbd_ep_rx(ep_num, status);
            }
#endif /* MSS_USB_DEVICE_ENABLED */

#ifdef MSS_USB_HOST_ENABLED
            if (MSS_USB_CORE_MODE_HOST == MSS_USB_CIF_get_mode())
            {
                status = MSS_USB_CIF_host_rx_errchk(ep_num);
                g_mss_usbh_cb.usbh_rx((uint8_t)ep_num, status);
            }
#endif /* MSS_USB_HOST_ENABLED */
            MSS_USB_CIF_rx_ep_enable_irq(ep_num);
        }
        status = 0u; /*resetting for next EP status*/
        ++ep_num;
    }
}

/***************************************************************************//**
 * Handler for DMA interrupt. Checks for the DMA channel on which interrupt has
 * Occurred and corresponding EP number then calls-back to upper layer to indicate
 * the event.
 */
uint8_t usb_dma_plic_IRQHandler(void)
{
    mss_usb_dma_channel_t dma_channel= MSS_USB_DMA_CHANNEL1;
    uint8_t status = 0;
    mss_usb_dma_dir_t dma_dir;
    mss_usb_ep_num_t ep_num;
    uint8_t dma_irq;
    uint32_t increamented_addr=0;

    dma_irq = MSS_USB_CIF_dma_read_irq();

    while (dma_irq)
    {
        if (dma_irq & MSS_USB_BYTE_BIT_0_MASK)
        {
            /* DMA Transfer for this channel is complete.Clear Start_transfer
               bit
             */
            MSS_USB_CIF_dma_stop_xfr(dma_channel);

            ep_num = (mss_usb_ep_num_t)MSS_USB_CIF_dma_get_epnum(dma_channel);
            dma_dir = (mss_usb_dma_dir_t)MSS_USB_CIF_dma_get_dir(dma_channel);

            if (MSS_USB_CIF_dma_is_bus_err(dma_channel))
            {
                status |=DMA_XFR_ERROR;
                MSS_USB_CIF_dma_clr_bus_err(dma_channel);
            }
            else
            {
                increamented_addr = MSS_USB_CIF_dma_read_addr(dma_channel);
                if (MSS_USB_DMA_READ == dma_dir)    /*TX EP*/
                {
                    if (MSS_USB_CIF_tx_ep_is_dma_enabled(ep_num))
                    {
                        MSS_USB_CIF_tx_ep_disable_dma(ep_num);
                    }
                }
#ifdef MSS_USB_HOST_ENABLED
                if (MSS_USB_CORE_MODE_HOST == MSS_USB_CIF_get_mode())
                {
                    /* Call the host mode logical layer driver callback
                       function
                     */
                    g_mss_usbh_cb.usbh_dma_handler(ep_num, dma_dir, status,
                                                             increamented_addr);
                }
#endif /* MSS_USB_HOST_ENABLED */
#ifdef MSS_USB_DEVICE_ENABLED
                if (MSS_USB_CORE_MODE_DEVICE == MSS_USB_CIF_get_mode())
                {
                    /* Call the device mode logical layer driver callback
                       function
                     */
                    g_mss_usbd_cb.usbd_dma_handler(ep_num, dma_dir, status,
                                                             increamented_addr);
                }
#endif /* MSS_USB_DEVICE_ENABLED */

            }
        }
        dma_channel++;
        dma_irq >>= 1u;
    }

    return EXT_IRQ_KEEP_ENABLED;
}

/***************************************************************************//**
 * Prepares the RX EP for receiving data as per parameters provided by upper layer
 */
void
MSS_USB_CIF_rx_ep_read_prepare
(
    mss_usb_ep_num_t ep_num,
    uint8_t* buf_addr,
    uint8_t dma_enable,
    mss_usb_dma_channel_t dma_channel,
    mss_usb_xfr_type_t xfr_type,
    uint32_t xfr_length
)
{
    /*
     * Fixed Buffer overwriting issue found with printer driver and issue with
     *  interrupt transfer with DMA by moving the location of interrupt enable
     *  function
     */
    if (DMA_ENABLE == dma_enable)
    {
        /*Make sure that address is Modulo-4.Bits D0-D1 are read only.*/
        ASSERT(!(((ptrdiff_t)buf_addr) & 0x00000002U));

        MSS_USB_CIF_dma_write_addr(dma_channel, (ptrdiff_t)buf_addr);

        /*
         * DMA Count register will be loaded after receive interrupt occurs.
         * Mode need to be set every time since M1 to M0 transition might have
         * happened for "short packet".
         */
         if (MSS_USB_XFR_BULK == xfr_type)
        {
            MSS_USB_CIF_rx_ep_set_dma_mode1(ep_num);
            MSS_USB_CIF_rx_ep_set_autoclr(ep_num);
            MSS_USB_CIF_rx_ep_enable_dma(ep_num);

            MSS_USB_CIF_dma_write_count(dma_channel,
                                        xfr_length);

            /* Handling single NULL packet reception */
            if (0u != xfr_length )
            {
                MSS_USB_CIF_dma_start_xfr(dma_channel);
            }
        }
        else
        {
            MSS_USB_CIF_rx_ep_clr_autoclr(ep_num);
            MSS_USB_CIF_rx_ep_set_dma_mode0(ep_num);
            MSS_USB_CIF_rx_ep_disable_dma(ep_num);
        }
    }
}

/***************************************************************************//**
 * Writes packet on TX EP
 */
void
MSS_USB_CIF_ep_write_pkt
(
    mss_usb_ep_num_t ep_num,
    uint8_t* buf_addr,
    uint8_t dma_enable,
    mss_usb_dma_channel_t dma_channel,
    mss_usb_xfr_type_t xfr_type,
    uint32_t xfr_length,
    uint32_t txn_length
)
{
    if (ep_num && (buf_addr != 0))
    {
        if (DMA_ENABLE == dma_enable)
        {
            /* Make sure that address is Modulo-4.Bits D0-D1 are read only.*/
            ASSERT(!(((ptrdiff_t)buf_addr) & 0x00000002u));

            MSS_USB_CIF_dma_write_addr(dma_channel,(ptrdiff_t)(buf_addr));

            if (MSS_USB_XFR_BULK == xfr_type)
            {
                MSS_USB_CIF_tx_ep_enable_dma(ep_num);

                 /*
                  * DMA-m1 will take care of transferring 'xfr_length' data
                  * as IN packets arrive.
                  * DMA interrupt will occur when all TxMaxPkt size packets
                  * are transferred.
                  */
                MSS_USB_CIF_dma_write_count(dma_channel, xfr_length);
            }
            else
            {
                /*
                 * DMA Enable bit in TxCSR is not needed. If set,TX EP
                 * Interrupt will not occur.
                 */
                MSS_USB_CIF_tx_ep_disable_dma(ep_num);

                /* Transfer only one packet with DMA-m0 */
                MSS_USB_CIF_dma_write_count(dma_channel, txn_length);
           }

             /*
              * This will start DMA transfer.
              * TODO: For Null transfer DMA is not needed, but not setting
              * TxPktRdy bit here, is not invoking EP interrupt.
              * EP interrupt does get called when Null DMA transfer is done.
              */
            MSS_USB_CIF_dma_start_xfr(dma_channel);


            /*
             * DMA interrupt will occur when all bytes are written to FIFO
             * TxPktRdy should be set when the DMA interrupt occurs.
             */
        }
        else    /* no DMA */
        {
            MSS_USB_CIF_load_tx_fifo(ep_num,
                                     buf_addr,
                                     txn_length);
            MSS_USB_CIF_tx_ep_set_txpktrdy(ep_num);
        }
    }
}

/***************************************************************************//**
 * Configures DMA for data transfer operations.
 */
void
MSS_USB_CIF_configure_ep_dma
(
    mss_usb_dma_channel_t dma_channel,
    mss_usb_dma_dir_t dma_dir,
    mss_usb_dma_mode_t dma_mode,
    mss_usb_dma_burst_mode_t burst_mode,
    mss_usb_ep_num_t ep_num,
    uint32_t buf_addr
)
{
    MSS_USB_CIF_dma_assign_to_epnum(dma_channel, ep_num);
    MSS_USB_CIF_dma_set_dir(dma_channel, dma_dir);
    MSS_USB_CIF_dma_set_mode(dma_channel, dma_mode);
    MSS_USB_CIF_dma_set_burst_mode(dma_channel, burst_mode);
    MSS_USB_CIF_dma_write_addr(dma_channel, buf_addr);
    MSS_USB_CIF_dma_enable_irq(dma_channel);
}

/***************************************************************************//**
 * Configures the TX EP for data transfer operations as per the parameters
 * provided by upper layer.
 */
void
MSS_USB_CIF_tx_ep_configure

(
    mss_usb_ep_t* core_ep
)
{
    uint8_t dpb = 1u;
    mss_usb_dma_mode_t mode;

    if (DPB_ENABLE == core_ep->dpb_enable)
    {
        dpb = 2u;
    }

    MSS_USB_CIF_tx_ep_set_fifo_size(core_ep->num,
                                    ((core_ep->fifo_size) / dpb),
                                    core_ep->dpb_enable);

    MSS_USB_CIF_tx_ep_set_fifo_addr(core_ep->num, core_ep->fifo_addr);

    if (DPB_ENABLE == core_ep->dpb_enable)
    {
        MSS_USB_enable_tx_ep_dpb(core_ep->num);
    }
    else if (DPB_DISABLE == core_ep->dpb_enable)
    {
        MSS_USB_disable_tx_ep_dpb(core_ep->num);
    }
    else
    {
        ASSERT(0);
    }

    MSS_USB_CIF_tx_ep_set_max_pkt(core_ep->num,
                                  core_ep->xfr_type,
                                  core_ep->max_pkt_size,
                                  core_ep->num_usb_pkt);

    MSS_USB_CIF_tx_ep_clr_data_tog(core_ep->num);

    if (DMA_ENABLE == core_ep->dma_enable)
    {
        if (MSS_USB_XFR_BULK == core_ep->xfr_type )
        {
            MSS_USB_CIF_tx_ep_set_dma_mode1(core_ep->num);
            MSS_USB_CIF_tx_ep_enable_dma(core_ep->num);
            mode = MSS_USB_DMA_MODE1;
        }
        else
        {
            /*
             * DMA_ENable bit in TXCSRL is not required to be set for m0. if it
             * is set TX interrupt would not occur.
             */
            MSS_USB_CIF_tx_ep_set_dma_mode0(core_ep->num);
            MSS_USB_CIF_tx_ep_disable_dma(core_ep->num);
            mode = MSS_USB_DMA_MODE0;
        }

        MSS_USB_CIF_configure_ep_dma(core_ep->dma_channel,
                                     MSS_USB_DMA_READ,
                                     mode,
                                     MSS_USB_DMA_BURST_MODE3,
                                     core_ep->num,
                                     (ptrdiff_t)(core_ep->buf_addr));
    }

    MSS_USB_CIF_tx_ep_enable_irq(core_ep->num);
}

/***************************************************************************//**
 * Configures the RX EP for data transfer operations as per the parameters
 * provided by upper layer.
 */
void
MSS_USB_CIF_rx_ep_configure

(
    mss_usb_ep_t* core_ep
)
{
    uint8_t dpb = 1u;
    mss_usb_dma_mode_t mode;
    if (DPB_ENABLE == core_ep->dpb_enable)
    {
        dpb = 2u;
    }

    MSS_USB_CIF_rx_ep_set_fifo_size(core_ep->num,
                                     ((core_ep->fifo_size) / dpb),
                                     core_ep->dpb_enable);

    MSS_USB_CIF_rx_ep_set_fifo_addr(core_ep->num,
                                     core_ep->fifo_addr);

    if (DPB_ENABLE == core_ep->dpb_enable)
    {
        MSS_USB_CIF_enable_rx_ep_dpb(core_ep->num);
    }
    else if (DPB_DISABLE == core_ep->dpb_enable)
    {
        MSS_USB_CIF_disable_rx_ep_dpb(core_ep->num);
    }
    else
    {
        ASSERT(0);
    }

    MSS_USB_CIF_rx_ep_set_max_pkt(core_ep->num,
                                  core_ep->xfr_type,
                                  core_ep->max_pkt_size,
                                  core_ep->num_usb_pkt);

    MSS_USB_CIF_rx_ep_clr_data_tog(core_ep->num);
    MSS_USB_CIF_rx_ep_clr_rxpktrdy(core_ep->num);

    if (DMA_ENABLE == core_ep->dma_enable)
    {
        if (MSS_USB_XFR_BULK == core_ep->xfr_type)
        {
            MSS_USB_CIF_rx_ep_set_dma_mode1(core_ep->num);
            MSS_USB_CIF_rx_ep_enable_dma(core_ep->num);
            mode = MSS_USB_DMA_MODE1;
        }
        else
        {
            /*
             * DMA_ENable bit in RXCSRL is not required to be set in m0. if it is
             * set RX interrupt would not occur.
             */
            MSS_USB_CIF_rx_ep_set_dma_mode0(core_ep->num);
            MSS_USB_CIF_rx_ep_disable_dma(core_ep->num);
            mode = MSS_USB_DMA_MODE0;
        }

        MSS_USB_CIF_configure_ep_dma(core_ep->dma_channel,
                                     MSS_USB_DMA_WRITE,
                                     mode,
                                     MSS_USB_DMA_BURST_MODE3,
                                     core_ep->num,
                                     (ptrdiff_t)(core_ep->buf_addr));
    }

    MSS_USB_CIF_rx_ep_enable_irq(core_ep->num);
}

/***************************************************************************//**
 * Starts sending Test packet as specified in the USB2.0
 * This is USB-IF certification requirement.
 */
void MSS_USB_CIF_start_testpacket(void)
{
    uint8_t test_pkt[53] =
    {
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0xAAU, 0xAAU, 0xAAU, 0xAAU, 0xAAU, 0xAAU, 0xAAU, 0xAAU, 0xEEU,
    0xEEU, 0xEEU, 0xEEU, 0xEEU, 0xEEU, 0xEEU, 0xEEU, 0xFEU, 0xFFU,
    0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU,
    0xFFU, 0x7FU, 0xBFU, 0xDFU, 0xEFU, 0xF7U, 0xFBU, 0xFDU, 0xFCU,
    0x7EU, 0xBFU, 0xDFU, 0xEFU, 0xF7U, 0xFBU, 0xFDU, 0x7EU
    };
    MSS_USB_CIF_load_tx_fifo(MSS_USB_CEP, test_pkt, 53u);
    MSS_USB_CIF_start_testpacket_bit();
    MSS_USB_CIF_cep_set_txpktrdy();
    MSS_USB_CIF_cep_disable_irq();
}

#ifdef MSS_USB_HOST_ENABLED
static uint8_t MSS_USB_CIF_host_rx_errchk(mss_usb_ep_num_t ep_num)
{
    uint8_t status = 0u;

    if (MSS_USBH_CIF_rx_ep_is_rxpktrdy(ep_num))
    {
        status = 0u;
    }
    if (MSS_USBH_CIF_rx_ep_is_rxstall_err(ep_num))
    {
        status |= MSS_USB_EP_STALL_RCVD;
        MSS_USBH_CIF_rx_ep_clr_rxstall_err(ep_num);
    }
    if (MSS_USBH_CIF_rx_ep_is_naktimeout_err(ep_num))
    {
        status |= MSS_USB_EP_NAK_TOUT;
        /* Not clearing NAKTIMEOUT error here. Application may want to abort
         * transfer. Clearing it here makes Scheduler keep trying the transfer
         */
    }
    if (MSS_USBH_CIF_rx_ep_is_retry_err(ep_num))
    {
        status |= MSS_USB_EP_NO_RESPONSE;
        MSS_USBH_CIF_rx_ep_clr_retry_err(ep_num);
    }

    return (status);
}

static uint8_t MSS_USB_CIF_host_tx_errchk(mss_usb_ep_num_t ep_num)
{
    uint8_t status = 0;

    if (MSS_USBH_CIF_tx_ep_is_retry_err(ep_num))
    {
        status |= MSS_USB_EP_NO_RESPONSE;
        MSS_USBH_CIF_tx_ep_clr_retry_err(ep_num);
    }
    if (MSS_USBH_CIF_tx_ep_is_rxstall_err(ep_num))
    {
        status |= MSS_USB_EP_STALL_RCVD;
        MSS_USBH_CIF_tx_ep_clr_rxstall_err(ep_num);
    }
    if (MSS_USBH_CIF_tx_ep_is_naktimeout_err(ep_num))
    {
        status |= MSS_USB_EP_NAK_TOUT;
        /* Not clearing NAKTIMEOUT error here. Application may want to abort
         * transfer. Clearing it here makes Scheduler keep trying the transfer
         */
    }

    return(status);
}
#endif /* MSS_USB_HOST_ENABLED */

#ifdef MSS_USB_DEVICE_ENABLED
static uint8_t MSS_USB_CIF_device_rx_errchk(mss_usb_ep_num_t ep_num)
{
    uint8_t status = 0u;

    if (MSS_USB_CIF_rx_ep_is_overrun(ep_num))
    {
        status |= RX_EP_OVER_RUN_ERROR;
        MSS_USB_CIF_rx_ep_clr_overrun(ep_num);
    }
    if (MSS_USB_CIF_rx_ep_is_stall_sent_bit(ep_num))
    {
        status |= RX_EP_STALL_ERROR;
        /*
         * "sent stall" bit should be cleared."Send Stall" bit is still set.
         * it should be cleared via Clear feature command or reset"
         */
        MSS_USB_CIF_rx_ep_clr_stall_sent_bit(ep_num);
    }
    if (MSS_USB_CIF_rx_ep_is_dataerr(ep_num))
    {
        /* This error will be cleared when RxPktRdy bit is cleared.
         */
        status |= RX_EP_DATA_ERROR;

    }
#if 0
    /*
     * PID error and INCOMP error should be checked only in ISO transfers.
     * This should be moved to logical layer
     */
    if (MSS_USB_CIF_rx_ep_is_piderr(ep_num))
    {
        status |= RX_EP_PID_ERROR;
        /* Data sheet doesn't mention about how this error bit is cleared
         * Assuming that this will be cleared when RxPKTRdy is cleared.*/
    }
    if (MSS_USB_CIF_rx_ep_is_isoincomp(ep_num))
    {
        status |= RX_EP_ISO_INCOMP_ERROR;
        /* This error will be cleared when RxPktRdy bit is cleared.*/
    }
#endif /* if 0 */

    return(status);
}

static uint8_t MSS_USB_CIF_device_tx_errchk(mss_usb_ep_num_t ep_num)
{
    uint8_t status = 0u;

    if (MSS_USB_CIF_tx_ep_is_underrun(ep_num))
    {
        /* Under-run errors should happen only for ISO endpoints.*/
        status |= TX_EP_UNDER_RUN_ERROR;
        MSS_USB_CIF_tx_ep_clr_underrun(ep_num);
    }
    if (MSS_USB_CIF_tx_ep_is_stall_sent_bit(ep_num))
    {
        status |= TX_EP_STALL_ERROR;
        MSS_USB_CIF_tx_ep_clr_stall_sent_bit(ep_num);
    }

    return(status);
}
#endif /* MSS_USB_DEVICE_ENABLED */

#ifdef __cplusplus
}
#endif
