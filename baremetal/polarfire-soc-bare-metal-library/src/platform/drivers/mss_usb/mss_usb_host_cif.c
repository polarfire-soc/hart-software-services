/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Microchip PolarFire SoC MSS USB Driver Stack
 *      USB Core Interface Layer (USB-CIFL)
 *          USBH-CIF driver
 *
 * USBH-CIF driver implementation:
 * This file implements MSS USB core initialization in host mode and
 * implements core interface function for the logical layer to control the
 * MSS USB core in USB Host mode.
 *
 * SVN $Revision$
 * SVN $Date$
 */

#include "mss_assert.h"
#include "mss_plic.h"
#include "mss_usb_host_cif.h"
#include "mss_usb_common_cif.h"
#include "mss_usb_common_reg_io.h"
#include "mss_usb_core_regs.h"
#include "mss_usb_host_reg_io.h"
#include "mss_usb_std_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_HOST_ENABLED

extern mss_usbh_cb_t g_mss_usbh_cb;

/***************************************************************************//**
 *
 */
void
MSS_USBH_CIF_init
(
    void
)
{
    MSS_USB_CIF_enable_hs_mode();
    MSS_USB_CIF_clr_usb_irq_reg();

    PLIC_EnableIRQ(USB_DMA_PLIC);
    MSS_USB_CIF_rx_ep_disable_irq_all();
    MSS_USB_CIF_tx_ep_disable_irq_all();

    PLIC_EnableIRQ(USB_MC_PLIC);

    /* This was added during Compliance testing */
    USB->C_T_HSBT = 0x01u;

    MSS_USB_CIF_enable_usbirq(CONNECT_IRQ_MASK | DISCONNECT_IRQ_MASK);
    MSS_USB_CIF_start_session();
}

/***************************************************************************//**
 *
 */
void
MSS_USBH_CIF_cep_configure
(
    mss_usb_ep_t* cep
)
{
    /* Control transfers will be handled without DMA */
    MSS_USBH_CIF_cep_clr_csr_reg();

    /* TODO:Ping should be decided based on TDev Speed. */
    if (cep->disable_ping)
    {
        MSS_USBH_CIF_cep_disable_ping();
    }
    else
    {
        MSS_USBH_CIF_cep_enable_ping();
    }

    ASSERT(cep->interval <= 32768u);

    /* Value 0 or 1 disables the NAKTIMEOUT functionality.*/
    if (cep->interval <= 32768u)
    {
        /*Value must be true and power of 2*/
        ASSERT(((cep->interval != 0U) &&
                (!(cep->interval & (cep->interval- 1)))));

        /*2 pow (m-1)*/
        MSS_USBH_CIF_cep_set_naklimit((MSS_USBH_CIF_ctz(cep->interval) + 1u));
    }

    /* Not flushing FIFO..since Tx/Rxpktrdy bit is not set. */
    MSS_USB_CIF_cep_enable_irq();
}

/*******************************************************************************
 * Configures the registers related to TX EP for data transfer operations as
 * per the parameters provided by the upper layer.
 */
void
MSS_USBH_CIF_tx_ep_configure
(
    mss_usb_ep_t* host_ep
)
{
    MSS_USB_CIF_tx_ep_clr_csrreg(host_ep->num);

    /* Perform host mode configurations here */
    /* TODO: Add AutoSet DMA etc */
    MSS_USBH_CIF_tx_ep_clr_retry_err(host_ep->num);
    MSS_USBH_CIF_tx_ep_clr_naktimeout_err(host_ep->num);
    MSS_USBH_CIF_tx_ep_clr_rxstall_err(host_ep->num);
    MSS_USBH_CIF_tx_ep_clr_data_tog_we(host_ep->num);
    MSS_USBH_CIF_tx_ep_clr_force_data_tog(host_ep->num);
    if (DMA_ENABLE == host_ep->dma_enable)
    {
        MSS_USB_CIF_tx_ep_set_autoset(host_ep->num);
    }
    else
    {
        MSS_USB_CIF_tx_ep_clr_autoset(host_ep->num);
    }

    /* Do the common configuration for TX EP */
    MSS_USB_CIF_tx_ep_configure(host_ep);
}

/*******************************************************************************
 * Configures the registers related to RX EP for data transfer operations as
 * per the parameters provided by the upper layer.
 */
void
MSS_USBH_CIF_rx_ep_configure
(
    mss_usb_ep_t* host_ep
)
{
    MSS_USB_CIF_rx_ep_clr_csrreg(host_ep->num);

    /* Perform host mode configurations here */
    /* TODO: Add AutoSet DMA etc */
    MSS_USBH_CIF_rx_ep_clr_retry_err(host_ep->num);
    MSS_USBH_CIF_rx_ep_clr_naktimeout_err(host_ep->num);
    MSS_USBH_CIF_rx_ep_clr_rxstall_err(host_ep->num);
    MSS_USBH_CIF_rx_ep_clr_data_tog_we(host_ep->num);
    MSS_USBH_CIF_rx_ep_clr_data_tog(host_ep->num);
    if (DMA_ENABLE == host_ep->dma_enable)
    {
        MSS_USBH_CIF_rx_ep_set_autoreq(host_ep->num);
    }
    else
    {
        MSS_USBH_CIF_rx_ep_clr_autoreq(host_ep->num);
    }

    /* Do the common configuration for RX EP */
    MSS_USB_CIF_rx_ep_configure(host_ep);
}


/***************************************************************************//**
 *
 */
void
MSS_USBH_CIF_bus_suspend
(
    uint32_t enable_suspendm
)
{
    if (SUSPENDM_DISABLE == enable_suspendm)
    {
        MSS_USBH_CIF_disable_suspendm_out();
    }
    else
    {
        MSS_USBH_CIF_enable_suspendm_out();
    }

    MSS_USBH_CIF_assert_suspend_bus();
}


/***************************************************************************//**
 *
 */
void
MSS_USBH_CIF_bus_resume
(
    void
)
{
    MSS_USBH_CIF_clr_suspend_bus();
    MSS_USBH_CIF_assert_bus_resume();

    /* TODO:This delay should be 20ms */
    MSS_USBH_CIF_clr_bus_resume();
}


/***************************************************************************//**
 *
 */
mss_usb_vbus_level_t
MSS_USBH_CIF_read_vbus_level
(
    void
)
{
    return (MSS_USB_CIF_get_vbus_level());
}


/***************************************************************************//**
 *
 */
uint32_t
MSS_USBH_CIF_tx_ep_mp_configure
(
    uint8_t outpipe_num,
    uint8_t tdev_ep_num,
    uint8_t tdev_addr,
    uint8_t tdev_hub_addr,
    uint8_t tdev_hub_port,
    uint8_t tdev_hub_mtt,
    mss_usb_device_speed_t tdev_speed,
    uint32_t tdev_interval,
    mss_usb_xfr_type_t tdev_xfr_type
)
{
    MSS_USBH_CIF_tx_ep_clr_txtypereg((mss_usb_ep_num_t)outpipe_num);

    /*
     * Since the core has Multipoint option enabled, following settings need to
     * be done even though there is single device connected
     */
    MSS_USBH_CIF_tx_ep_set_target_func_addr((mss_usb_ep_num_t)outpipe_num, tdev_addr);

#if 0
    /* Hub and multiple devices are not supported yet */
    MSS_USBH_CIF_tx_ep_set_target_hub_addr(outpipe_num, tdev_hub_addr, tdev_hub_mtt);
    MSS_USBH_CIF_tx_ep_set_target_hub_port(outpipe_num, tdev_hub_port);
#endif

    if ((MSS_USB_XFR_INTERRUPT == tdev_xfr_type) &&
        (MSS_USB_DEVICE_HS != tdev_speed))
    {
        MSS_USBH_CIF_tx_ep_set_target_interval((mss_usb_ep_num_t)outpipe_num, tdev_interval);
    }
    else
    {
        /*2 pow (m-1)*/
        MSS_USBH_CIF_tx_ep_set_target_interval((mss_usb_ep_num_t)outpipe_num,
                                               ((MSS_USBH_CIF_ctz(tdev_interval)) + 1u));
    }

    MSS_USBH_CIF_tx_ep_set_target_ep_no((mss_usb_ep_num_t)outpipe_num, tdev_ep_num);
    MSS_USBH_CIF_tx_ep_set_target_protocol((mss_usb_ep_num_t)outpipe_num, tdev_xfr_type);
    MSS_USBH_CIF_tx_ep_set_target_speed((mss_usb_ep_num_t)outpipe_num, tdev_speed);

    return (0);
}

/***************************************************************************//**
 *
 */
uint32_t
MSS_USBH_CIF_rx_ep_mp_configure
(
    uint8_t inpipe_num,
    uint8_t tdev_ep_num,
    uint8_t tdev_addr,
    uint8_t tdev_hub_addr,
    uint8_t tdev_hub_port,
    uint8_t tdev_hub_mtt,
    mss_usb_device_speed_t tdev_speed,
    uint32_t tdev_interval,
    mss_usb_xfr_type_t tdev_xfr_type
)
{
    MSS_USBH_CIF_rx_ep_clr_rxtypereg((mss_usb_ep_num_t)inpipe_num);

    /*
     * Since the core has Multipoint option enabled, following settings need to
     * be done even though there is single device connected
     */
    MSS_USBH_CIF_rx_ep_set_target_func_addr((mss_usb_ep_num_t)inpipe_num,
                                            tdev_addr);

#if 0
    /* Hub and multiple devices are not supported yet */
    MSS_USBH_CIF_rx_ep_set_target_hub_addr((mss_usb_ep_num_t)inpipe_num,
                                           tdev_hub_addr,
                                           tdev_hub_mtt);

    MSS_USBH_CIF_rx_ep_set_target_hub_port((mss_usb_ep_num_t)inpipe_num,
                                           tdev_hub_port);
#endif

    if((MSS_USB_XFR_INTERRUPT == tdev_xfr_type) &&
        (MSS_USB_DEVICE_HS != tdev_speed))
    {


        MSS_USBH_CIF_rx_ep_set_target_interval((mss_usb_ep_num_t)inpipe_num,
                                               tdev_interval);
    }
    else
    {

        MSS_USBH_CIF_rx_ep_set_target_interval((mss_usb_ep_num_t)inpipe_num,
                                               ((MSS_USBH_CIF_ctz(tdev_interval)) + 1u));
    }

    MSS_USBH_CIF_rx_ep_set_target_ep_no((mss_usb_ep_num_t)inpipe_num, tdev_ep_num);
    MSS_USBH_CIF_rx_ep_set_target_protocol((mss_usb_ep_num_t)inpipe_num, tdev_xfr_type);
    MSS_USBH_CIF_rx_ep_set_target_speed((mss_usb_ep_num_t)inpipe_num, tdev_speed);

    return(0);
}

/*******************************************************************************
 * Writes a data packet on EP0 in host mode(control endpoint).
 */
void
MSS_USBH_CIF_cep_write_pkt
(
    mss_usb_ep_t* hcep
)
{
    ASSERT((hcep->num == MSS_USB_CEP) &&
           (hcep->buf_addr != 0) &&
           (hcep->xfr_type == MSS_USB_XFR_CONTROL));

    /* null buffer, xfr type, transaction type */
    if ((hcep->num == MSS_USB_CEP) &&
        (hcep->buf_addr != 0) &&
        (hcep->xfr_type == MSS_USB_XFR_CONTROL))
    {
        MSS_USB_CIF_load_tx_fifo(hcep->num,
                                 hcep->buf_addr,
                                 hcep->txn_length);

        hcep->txn_count = hcep->txn_length;
        hcep->xfr_count += hcep->txn_length;
    }
}

/*******************************************************************************
 * Reads data packet arrived on EP0 in host mode(control endpoint).
 */
void
MSS_USBH_CIF_cep_read_pkt
(
    mss_usb_ep_t* hcep
)
{
       uint16_t received_count = 0u;

    ASSERT((hcep->num == MSS_USB_CEP) &&
           (hcep->buf_addr != 0) &&
           (hcep->xfr_type == MSS_USB_XFR_CONTROL));

    /* TODO: check stalled, null buffer, xfr type, transaction type */
    received_count = MSS_USB_CIF_cep_rx_byte_count();
    ASSERT(received_count <= hcep->txn_length);
    MSS_USB_CIF_read_rx_fifo(MSS_USB_CEP, hcep->buf_addr, received_count);
    hcep->buf_addr += received_count;
    MSS_USBH_CIF_cep_clr_rxpktrdy();
    hcep->xfr_count += received_count;
    hcep->txn_count = received_count;
}

/***************************************************************************//**
 *
 */
void
MSS_USBH_CIF_cep_abort_xfr
(
    void
)
{
    /* Flush the FIFO and reset all values */
    MSS_USB_CIF_cep_disable_irq();
    MSS_USB_CIF_cep_set_txpktrdy();
    MSS_USB_CIF_cep_flush_fifo();
    MSS_USB_CIF_cep_enable_irq();

    /* clear all bits but retain the values of Data_tog_we and Disable_ping */
    MSS_USBH_CIF_cep_clr_naktimeout_err();
    MSS_USBH_CIF_cep_clr_rxstall_err();
    MSS_USBH_CIF_cep_clr_rxpktrdy();

    USB->ENDPOINT[MSS_USB_CEP].TX_CSR &= ~ (CSR0L_HOST_IN_PKT_REQ_MASK |
                                            CSR0L_HOST_SETUP_PKT_MASK  |
                                            CSR0L_HOST_STATUS_PKT_MASK |
                                            CSR0L_HOST_RX_PKT_RDY_MASK |
                                            CSR0H_HOST_DATA_TOG_MASK);
}

/***************************************************************************//**
 *
 */
void
MSS_USBH_CIF_handle_connect_irq
(
    void
)
{
    mss_usb_device_speed_t speed = MSS_USB_DEVICE_FS;

    if (MSS_USBH_CIF_is_target_ls())
    {
        speed = MSS_USB_DEVICE_LS;
    }
    else if (MSS_USBH_CIF_is_target_fs())
    {
        speed = (MSS_USB_DEVICE_FS);
    }

    if (MSS_USBH_CIF_is_host_suspended())
    {
        MSS_USBH_CIF_clr_suspend_bus();
    }

    g_mss_usbh_cb.usbh_connect(speed , MSS_USB_CIF_get_vbus_level());
}

#endif /* MSS_USB_HOST_ENABLED */

#ifdef __cplusplus
}
#endif
