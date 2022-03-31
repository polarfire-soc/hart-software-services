/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC MSS USB Driver Stack
 *      USB Logical Layer (USB-LL)
 *          USBD driver
 *
 * USBD driver implementation:
 * This source file implements the common functionality of USB device mode,
 * which includes initialization of MSS USB in device mode, USB standard request
 * handling, distribution of requests to specific class, interface or endpoints.
 *
 */
#include "mss_clint.h"
#include "mss_usb_device.h"
#include "mss_usb_config.h"
#include "mss_usb_common_cif.h"
#include "mss_usb_device_cif.h"
#include "mss_usb_std_def.h"
#include "mss_plic.h"
#include "mss_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_DEVICE_ENABLED

#define SETUP_PKT_INIT();                         g_setup_pkt.request_type = 0;\
                                                  g_setup_pkt.request = 0;\
                                                  g_setup_pkt.value = 0;\
                                                  g_setup_pkt.index = 0;\
                                                  g_setup_pkt.length = 0;

/***************************************************************************//**
 Global variables used within this file.
 */
/* This structure must be implemented by user application */
mss_usbd_user_descr_cb_t *g_usbd_user_descr_cb;

/* This structure must be implemented by USBD-Class driver*/
mss_usbd_class_cb_t *g_usbd_class_cb;

/* USB current Speed of operation selected by user*/
static mss_usb_device_speed_t g_usbd_user_speed;

/*Status information for SET_FEATURE,GET_FEATURE requests.*/
static uint16_t g_usbd_status;

/*Device configuration information*/
static mss_usbd_dev_conf_t g_usbd_dev_conf;

/*This structure is used to hold the setup packet received on control endpoint*/
static mss_usbd_setup_pkt_t g_setup_pkt;

/*Structure representing each endpoint on MSS USB*/
static mss_usb_ep_t gd_tx_ep[5u];       /*idx = 0: Control Endpoint*/
static mss_usb_ep_t gd_rx_ep[5u];

/*
 This structure should be implemented for bare testing of the USB transfers,
 when in the test mode, no other class specific components or enumeration
 related APIs are called.
 This mode is only provided for internal customers for testing purpose.
 */
#ifdef MSS_USB_DEVICE_TEST_MODE
extern mss_usbd_user_test_cb_t usbd_test_cb;
#endif

/***************************************************************************//**
 Private function declarations for this file (USBD Driver).
*/
static void mss_usbd_ep_rx_cb(mss_usb_ep_num_t ep_num, uint8_t status);
static void mss_usbd_ep_tx_complete_cb(mss_usb_ep_num_t num, uint8_t status);

static void mss_usbd_cep_setup_cb(uint8_t status);
static void mss_usbd_cep_rx_cb(uint8_t status);
static void mss_usbd_cep_tx_complete_cb(uint8_t status);

static void mss_usbd_reset_cb(void);
static void mss_usbd_sof_cb(uint8_t status);
static void mss_usbd_suspend_cb(void);
static void mss_usbd_resume_cb(void);
static void mss_usbd_disconnect_cb(void);
static void mss_usbd_dma_handler_cb(mss_usb_ep_num_t ep_num,
                                    mss_usb_dma_dir_t dma_dir, uint8_t status,
                                    uint32_t dma_addr_val);

static uint8_t mss_usbd_class_requests(uint8_t** buf_pp, uint32_t* len_p);
static uint8_t mss_usbd_vendor_requests(uint8_t** buf_pp, uint32_t* len_p);
static uint8_t mss_usbd_get_descriptor(uint8_t** buf_pp, uint32_t* len_p);
static uint8_t mss_usbd_set_address(void);
static uint8_t mss_usbd_set_config(void);
static uint8_t mss_usbd_get_config(uint8_t** buf_pp, uint32_t* len_p);
static uint8_t mss_usbd_get_status(uint8_t** buf_pp, uint32_t* len_p);
static uint8_t mss_usbd_set_feature(void);
static uint8_t mss_usbd_clr_feature(void);
static uint8_t mss_usbd_set_descriptor(void);
static uint8_t mss_usbd_std_requests(uint8_t** buf,uint32_t* length);

/*
 This structure implements the callback functions which will be called by the
 USBD-CIFL layer.
 */
mss_usbd_cb_t g_mss_usbd_cb =
{
    mss_usbd_ep_rx_cb,
    mss_usbd_ep_tx_complete_cb,

    mss_usbd_cep_setup_cb,
    mss_usbd_cep_rx_cb,
    mss_usbd_cep_tx_complete_cb,

    mss_usbd_sof_cb,
    mss_usbd_reset_cb,
    mss_usbd_suspend_cb,
    mss_usbd_resume_cb,
    mss_usbd_disconnect_cb,
    mss_usbd_dma_handler_cb
};

/***************************************************************************//**
 Exported functions from this file (USBD Driver)
 */

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
 */
void
MSS_USBD_init
(
    mss_usb_device_speed_t speed
)
{
    g_usbd_dev_conf.device_addr = 0x00u;
    g_usbd_dev_conf.device_total_interfaces = 0x00u;
    g_usbd_dev_conf.device_total_ep = 0x00u;
    g_usbd_dev_conf.device_state = MSS_USB_NOT_ATTACHED_STATE;

    /*store this for usage with DevQual, OtherSpeed requests*/
    g_usbd_user_speed = speed;
    g_usbd_dev_conf.device_speed = speed;

    gd_tx_ep[MSS_USB_CEP].state = MSS_USB_CEP_IDLE;

    MSS_USBD_CIF_init(g_usbd_dev_conf.device_speed);

    MSS_USBD_CIF_dev_connect();

    g_usbd_dev_conf.device_state = MSS_USB_POWERED_STATE;
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
 */
void
MSS_USBD_set_descr_cb_handler
(
    mss_usbd_user_descr_cb_t* user_desc_cb
)
{
    g_usbd_user_descr_cb = user_desc_cb;
}
/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
 */
void
MSS_USBD_set_class_cb_handler
(
    mss_usbd_class_cb_t* class_cb
)
{
    g_usbd_class_cb = class_cb;
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
 */
void
MSS_USBD_tx_ep_stall
(
    mss_usb_ep_num_t ep_num
)
{
    gd_tx_ep[ep_num].stall = 1u;
    MSS_USBD_CIF_tx_ep_stall(ep_num);
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
 */
void
MSS_USBD_tx_ep_clr_stall
(
    mss_usb_ep_num_t ep_num
)
{
    MSS_USBD_CIF_tx_ep_clr_stall(ep_num);
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
 */
void
MSS_USBD_tx_ep_flush_fifo
(
    mss_usb_ep_num_t ep_num
)
{
    MSS_USB_CIF_tx_ep_flush_fifo(ep_num);
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
 */
void
MSS_USBD_rx_ep_stall
(
    mss_usb_ep_num_t ep_num
)
{
    gd_rx_ep[ep_num].stall = 1u;
    MSS_USBD_CIF_rx_ep_stall(ep_num);
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
 */
void
MSS_USBD_rx_ep_clr_stall
(
    mss_usb_ep_num_t ep_num
)
{
    MSS_USBD_CIF_rx_ep_clr_stall(ep_num);
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
 */
void
MSS_USBD_rx_ep_flush_fifo
(
    mss_usb_ep_num_t ep_num
)
{
    MSS_USB_CIF_rx_ep_flush_fifo(ep_num);
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
 */
void
MSS_USBD_cep_flush_fifo
(
    void
)
{
    MSS_USB_CIF_cep_flush_fifo();
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
 */
void
MSS_USBD_cep_configure
(
    uint8_t max_pkt_size
)
{
    mss_usb_ep_t* cep_ptr = &gd_tx_ep[MSS_USB_CEP];

    cep_ptr->num = MSS_USB_CEP;
    cep_ptr->stall = 0u;
    cep_ptr->state = MSS_USB_CEP_IDLE;
    cep_ptr->xfr_type = MSS_USB_XFR_CONTROL;

    /*FIFO address */
    cep_ptr->buf_addr = 0u;

    cep_ptr->max_pkt_size = max_pkt_size;
    cep_ptr->txn_length = SETUP_PKT_SIZE;

    MSS_USBD_CIF_cep_configure();
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
 */
void
MSS_USBD_cep_read_prepare
(
    uint8_t * addr,
    uint32_t length
)
{
    mss_usb_ep_t* cep_ptr = &gd_tx_ep[MSS_USB_CEP];

    cep_ptr->buf_addr = addr;
    cep_ptr->xfr_length = length;
    cep_ptr->xfr_count = 0u;
    cep_ptr->txn_count = 0u;

    if(cep_ptr->xfr_length > cep_ptr->max_pkt_size)
    {
        cep_ptr->txn_length = cep_ptr->max_pkt_size;
    }
    else
    {
        cep_ptr->txn_length = length;
    }
    MSS_USBD_CIF_cep_rx_prepare(&gd_tx_ep[MSS_USB_CEP]);
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
 */
void
MSS_USBD_cep_write
(
    uint8_t * addr,
    uint32_t length
)
{
    mss_usb_ep_t* cep_ptr = &gd_tx_ep[MSS_USB_CEP];

    cep_ptr->buf_addr = addr;
    cep_ptr->xfr_length = length;

    cep_ptr->xfr_count = 0u;
    cep_ptr->txn_count = 0u;

    if(cep_ptr->xfr_length > cep_ptr->max_pkt_size)
    {
        cep_ptr->txn_length = cep_ptr->max_pkt_size;
    }
    else
    {
        cep_ptr->txn_length = length;
    }

    MSS_USBD_CIF_cep_write_pkt(&gd_tx_ep[MSS_USB_CEP]);
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
 */
void
MSS_USBD_tx_ep_configure
(
    mss_usb_ep_num_t ep_num,
    uint16_t fifo_addr,
    uint16_t fifo_size,       /*This is actual size of FIFO independent of DPB*/
    uint16_t max_pkt_size,
    uint8_t num_usb_pkt,
    uint8_t dma_enable,
    mss_usb_dma_channel_t dma_channel,
    mss_usb_xfr_type_t xfr_type,
    uint32_t add_zlp
)
{
    uint8_t err_check = USB_SUCCESS;
    uint16_t std_max_pkt_sz = USB_HS_BULK_MAX_PKT_SIZE;
    mss_usb_ep_t* txep_ptr = &gd_tx_ep[ep_num];

    if(MSS_USB_DEVICE_HS == g_usbd_dev_conf.device_speed)
    {
        switch(xfr_type)
        {
            case MSS_USB_XFR_BULK:
                    std_max_pkt_sz = USB_HS_BULK_MAX_PKT_SIZE;
                break;
                case MSS_USB_XFR_INTERRUPT:
                    std_max_pkt_sz = USB_HS_INTERRUPT_MAX_PKT_SIZE;
                break;
                case MSS_USB_XFR_ISO:
                    std_max_pkt_sz = USB_HS_ISO_MAX_PKT_SIZE;
                break;
            default:
                err_check = USB_FAIL;
        }
    }
    else if(MSS_USB_DEVICE_FS == g_usbd_dev_conf.device_speed)
    {
        switch(xfr_type)
        {
            case MSS_USB_XFR_BULK:
                std_max_pkt_sz = USB_FS_BULK_MAX_PKT_SIZE;
                break;
            case MSS_USB_XFR_INTERRUPT:
                std_max_pkt_sz = USB_FS_INTERRUPT_MAX_PKT_SIZE;
                break;
            case MSS_USB_XFR_ISO:
                std_max_pkt_sz = USB_FS_ISO_MAX_PKT_SIZE;
                break;
            default:
                err_check = USB_FAIL;
        }
    }

    if(max_pkt_size > std_max_pkt_sz)
    {
        err_check = USB_FAIL;
    }
    if(fifo_size < max_pkt_size)
    {
        err_check = USB_FAIL;
    }

    ASSERT(err_check == USB_SUCCESS);

    if(USB_SUCCESS == err_check)
    {
        if((max_pkt_size * 2) <= fifo_size)
        {
            txep_ptr->dpb_enable = DPB_ENABLE;
        }
        else
        {
            txep_ptr->dpb_enable = DPB_DISABLE;
        }

        txep_ptr->num = ep_num;
        txep_ptr->fifo_addr = fifo_addr;
        txep_ptr->fifo_size = fifo_size;
        txep_ptr->max_pkt_size = max_pkt_size;
        txep_ptr->num_usb_pkt = num_usb_pkt;
        txep_ptr->dma_enable = dma_enable;
        txep_ptr->dma_channel = dma_channel;
        txep_ptr->stall = 0u;
        txep_ptr->state = MSS_USB_EP_VALID;
        txep_ptr->xfr_type = xfr_type;
        txep_ptr->buf_addr = 0u;
        txep_ptr->add_zlp = add_zlp;
        MSS_USBD_CIF_tx_ep_configure(txep_ptr);
    }
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
void
MSS_USBD_rx_ep_configure
(
    mss_usb_ep_num_t ep_num,
    uint16_t fifo_addr,
    uint16_t fifo_size,      /*This is actual size of FIFO independent of DPB*/
    uint16_t max_pkt_size,
    uint8_t num_usb_pkt,
    uint8_t dma_enable,
    mss_usb_dma_channel_t dma_channel,
    mss_usb_xfr_type_t xfr_type,
    uint32_t add_zlp
)
{
    uint8_t err_check = USB_SUCCESS;

    uint16_t std_max_pkt_sz = USB_HS_BULK_MAX_PKT_SIZE;
    mss_usb_ep_t* rxep_ptr = &gd_rx_ep[ep_num];

    if(MSS_USB_DEVICE_HS == g_usbd_dev_conf.device_speed)
    {
        switch(xfr_type)
        {
            case MSS_USB_XFR_BULK:
                std_max_pkt_sz = USB_HS_BULK_MAX_PKT_SIZE;
                break;
            case MSS_USB_XFR_INTERRUPT:
                std_max_pkt_sz = USB_HS_INTERRUPT_MAX_PKT_SIZE;
                break;
            case MSS_USB_XFR_ISO:
                std_max_pkt_sz = USB_HS_ISO_MAX_PKT_SIZE;
                break;
            default:
                err_check = USB_FAIL;
        }
    }
    else if(MSS_USB_DEVICE_FS == g_usbd_dev_conf.device_speed)
    {
        switch(xfr_type)
        {
            case MSS_USB_XFR_BULK:
                std_max_pkt_sz = USB_FS_BULK_MAX_PKT_SIZE;
                break;
            case MSS_USB_XFR_INTERRUPT:
                std_max_pkt_sz = USB_FS_INTERRUPT_MAX_PKT_SIZE;
                break;
            case MSS_USB_XFR_ISO:
                std_max_pkt_sz = USB_FS_ISO_MAX_PKT_SIZE;
                break;
            default:
                err_check = USB_FAIL;
        }
    }

    if(max_pkt_size > std_max_pkt_sz)
    {
        err_check = USB_FAIL;
    }
    if(fifo_size < max_pkt_size)
    {
        err_check = USB_FAIL;
    }

    ASSERT(err_check == USB_SUCCESS);

    if(USB_SUCCESS == err_check)
    {
        if((max_pkt_size * 2u ) <= fifo_size)
        {
            rxep_ptr->dpb_enable = DPB_ENABLE;
        }
        else
        {
            rxep_ptr->dpb_enable = DPB_DISABLE;
        }
        rxep_ptr->num = ep_num;
        rxep_ptr->fifo_addr = fifo_addr;
        rxep_ptr->fifo_size = fifo_size;
        rxep_ptr->max_pkt_size = max_pkt_size;
        rxep_ptr->num_usb_pkt = num_usb_pkt;
        rxep_ptr->dma_enable = dma_enable;
        rxep_ptr->dma_channel = dma_channel;
        rxep_ptr->stall = 0u;
        rxep_ptr->state = MSS_USB_EP_VALID;
        rxep_ptr->xfr_type = xfr_type;
        rxep_ptr->buf_addr = 0u;
        rxep_ptr->add_zlp = add_zlp;

        MSS_USBD_CIF_rx_ep_configure(rxep_ptr);
    }
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
void
MSS_USBD_rx_ep_read_prepare
(
    mss_usb_ep_num_t ep_num,
    uint8_t * addr,
    uint32_t length
)
{
    mss_usb_ep_t* rxep_ptr = &gd_rx_ep[ep_num];

    ASSERT(ep_num);
    ASSERT(addr != 0);

    rxep_ptr->buf_addr = addr;
    rxep_ptr->xfr_length = length;
    rxep_ptr->xfr_count = 0u;
    rxep_ptr->txn_count = 0u;

    /*HAL_ASSERT when length is 0, address is null or ep number is wrong*/
    if(length > 0u)
    {
        /*
        TODO:    While using DMA, RxMaxP payload value MUST be an even number
        when dealing with Multi packet Bulk transaction for
        proper interrupt generation. This needs to be taken care.
        */
        /*
         section 8.4.2.3 -- No support for HB in Interrupt transfers.
         i.e. maximum one packet(num_usb_pkt=1)can be transferred in one frame.
        */
        if(length >= rxep_ptr->max_pkt_size)
        {
            rxep_ptr->txn_length = rxep_ptr->max_pkt_size;
        }
        else
        {
            rxep_ptr->txn_length = length;
        }
    }

    MSS_USB_CIF_rx_ep_read_prepare(rxep_ptr->num,
                                   rxep_ptr->buf_addr,
                                   rxep_ptr->dma_enable,
                                   rxep_ptr->dma_channel,
                                   rxep_ptr->xfr_type,
                                   rxep_ptr->xfr_length);
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
void
MSS_USBD_tx_ep_write
(
    mss_usb_ep_num_t ep_num,
    uint8_t * addr,
    uint32_t length
)
{
    uint8_t dpb = 1u;
    mss_usb_ep_t* txep_ptr = &gd_tx_ep[ep_num];

    /* HAL_ASSERT if uneven value for BULK transfers */
    ASSERT(ep_num);
    ASSERT(addr != 0);

    if((ep_num) && (addr != 0))
    {
        if(DPB_ENABLE == txep_ptr->dpb_enable)
        {
            dpb = 2u;
        }
        txep_ptr->num = ep_num;
        txep_ptr->buf_addr = addr;
        txep_ptr->xfr_length = length;
        txep_ptr->xfr_count = 0u;
        txep_ptr->txn_count = 0u;

        if(MSS_USB_XFR_BULK == txep_ptr->xfr_type)
        {
            /*
            TODO: TxMaxP payload value MUST be an even number when dealing with
            Multi packet Bulk transaction for proper interrupt generation.
            This needs to be taken care.
            */
            if(length <= txep_ptr->fifo_size)
            {
                txep_ptr->txn_length = length;
            }
            else
            {
                txep_ptr->txn_length= txep_ptr->fifo_size;
            }
        }

        /*
         section 8.4.2.3 -- No support for HB in Interrupt transfers.
         i.e. maximum one packet(num_usb_pkt=1)can be transferred in one frame.
         */
        else if(MSS_USB_XFR_INTERRUPT == txep_ptr->xfr_type)
        {
            if(length >= (txep_ptr->max_pkt_size * dpb))
            {
                txep_ptr->txn_length = (txep_ptr->max_pkt_size * dpb);
            }
            else
            {
                txep_ptr->txn_length = length;
            }
        }
        else if(MSS_USB_XFR_ISO == txep_ptr->xfr_type)
        {
            if(length >= (txep_ptr->max_pkt_size * dpb))
            {
                txep_ptr->txn_length = (txep_ptr->max_pkt_size * dpb);
            }
            else
            {
                txep_ptr->txn_length = length;
            }
        }

        MSS_USB_CIF_ep_write_pkt(txep_ptr->num,
                                 txep_ptr->buf_addr,
                                 txep_ptr->dma_enable,
                                 txep_ptr->dma_channel,
                                 txep_ptr->xfr_type,
                                 txep_ptr->xfr_length,
                                 txep_ptr->txn_length);
    }
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
void
MSS_USBD_get_hwcore_info
(
    mss_usb_core_info_t* hw_info
)
{
    MSS_USBD_CIF_get_hwcore_info(hw_info);
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
uint8_t
MSS_USBD_get_dev_address
(
    void
)
{
    return(MSS_USBD_CIF_get_dev_addr());
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
void
MSS_USBD_set_isoupdate
(
    void
)
{
    MSS_USBD_CIF_set_isoupdate();
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
void
MSS_USBD_clr_isoupdate
(
    void
)
{
    MSS_USBD_CIF_clr_isoupdate();
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
uint8_t
MSS_USBD_tx_ep_is_fifo_notempty
(
    mss_usb_ep_num_t epnum
)
{
    return(MSS_USB_CIF_is_txepfifo_notempty(epnum));
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
uint8_t
MSS_USBD_rx_ep_is_fifo_full
(
    mss_usb_ep_num_t epnum
)
{
    return(MSS_USB_CIF_rx_ep_is_fifo_full(epnum));
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
void
MSS_USBD_tx_ep_disable_irq
(
    mss_usb_ep_num_t epnum
)
{
    MSS_USB_CIF_tx_ep_disable_irq(epnum);
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
void
MSS_USBD_rx_ep_disable_irq
(
    mss_usb_ep_num_t epnum
)
{
    MSS_USB_CIF_rx_ep_disable_irq(epnum);
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
void
MSS_USBD_tx_ep_enable_irq
(
    mss_usb_ep_num_t epnum
)
{
    MSS_USB_CIF_tx_ep_enable_irq(epnum);
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
void
MSS_USBD_rx_ep_enable_irq
(
    mss_usb_ep_num_t epnum
)
{
    MSS_USB_CIF_rx_ep_enable_irq(epnum);
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
void
MSS_USBD_cep_enable_irq
(
    void
)
{
    MSS_USB_CIF_cep_enable_irq();
}

/***************************************************************************//**
 See mss_usb_device.h for details of how to use this function.
*/
void
MSS_USBD_cep_disable_irq
(
    void
)
{
    MSS_USB_CIF_cep_disable_irq();
}

/***************************************************************************//**
 Private functions to this file (USBD Driver)
 */

/***************************************************************************//**
 This call-back function is executed on receiving SOF interrupt in Device mode.
 */
static void
mss_usbd_sof_cb
(
    uint8_t status
)
{
#ifndef MSS_USB_DEVICE_TEST_MODE
#else
    usbd_test_cb.test_sof(status);
#endif

}

/******************************************************************************
 This Callback function is executed when reset interrupt is received when
 the MSS USB is in device mode.
 */
static void
mss_usbd_reset_cb
(
    void
)
{
#ifndef MSS_USB_DEVICE_TEST_MODE
    g_usbd_dev_conf.device_addr = 0x00u;
    g_usbd_dev_conf.device_total_interfaces = 0x00u;
    g_usbd_dev_conf.device_total_ep = 0x00u;
    g_usbd_dev_conf.device_state = MSS_USB_DEFAULT_STATE;

    gd_tx_ep[MSS_USB_CEP].state = MSS_USB_CEP_IDLE;
    g_usbd_dev_conf.remote_wakeup = 0x00u;

    if(MSS_USBD_CIF_is_hs_mode())
    {
        g_usbd_dev_conf.device_speed = MSS_USB_DEVICE_HS;
    }
    else
    {
        g_usbd_dev_conf.device_speed = MSS_USB_DEVICE_FS;
    }

    SETUP_PKT_INIT();

    MSS_USBD_cep_configure(64u);
    MSS_USBD_cep_read_prepare((uint8_t*)&g_setup_pkt, USB_SETUP_PKT_LEN);
#else
    usbd_test_cb.test_reset();
#endif

}

/*******************************************************************************
 This Callback function is executed when Setup packet is received on Control EP
 */
static void
mss_usbd_cep_setup_cb
(
    uint8_t status
)
{
#ifndef MSS_USB_DEVICE_TEST_MODE
    uint8_t* buf = 0;
    uint32_t length = 0u;
    uint8_t result = USB_FAIL;
    mss_usb_ep_t* cep_ptr = &gd_tx_ep[MSS_USB_CEP];

    if(status & (CTRL_EP_SETUP_END_ERROR | CTRL_EP_STALL_ERROR))
    {
        /*
         EP0 was previously stalled, clear the error condition and
         prepare for next transaction
         */
        cep_ptr->state = MSS_USB_CEP_IDLE;

        SETUP_PKT_INIT();

        MSS_USBD_cep_read_prepare((uint8_t*)&g_setup_pkt, USB_SETUP_PKT_LEN);
    }
    else
    {
        MSS_USBD_CIF_cep_read_pkt(cep_ptr);

        if(SETUP_PKT_SIZE == cep_ptr->txn_length)
        {
            cep_ptr->state = MSS_USB_CEP_SETUP;

            if(USB_STANDARD_REQUEST == (g_setup_pkt.request_type &
                                        USB_STD_REQ_TYPE_MASK))
            {
                result = mss_usbd_std_requests(&buf, &length);
            }
            else if(USB_CLASS_REQUEST == (g_setup_pkt.request_type &
                                          USB_STD_REQ_TYPE_MASK))
            {
                result = mss_usbd_class_requests(&buf, &length);
            }
            else if(USB_VENDOR_REQUEST == (g_setup_pkt.request_type &
                                           USB_STD_REQ_TYPE_MASK))
            {
                result = mss_usbd_vendor_requests(&buf, &length);
            }
        }
        else
        {
            ASSERT(0);/*in this transaction the txn_length must be SETUP_PKT_SIZE*/
        }

        if(result)  //USB_SUCCESS
        {
            MSS_USBD_CIF_reset_index_reg();
            if(0u == g_setup_pkt.length)
            {
                volatile uint32_t delay = 0;
                MSS_USBD_CIF_cep_end_zdr();                 //zdl complete

                cep_ptr->state = MSS_USB_CEP_IDLE;
                if(USB_STD_REQ_SET_ADDRESS == g_setup_pkt.request)
                {
                    //special case SetAddress Request
                    for(delay = 0; delay < 5000 ; delay ++) { asm volatile(""); }
                    MSS_USBD_CIF_set_dev_addr(g_usbd_dev_conf.device_addr);
                }

                if((USB_STD_REQ_SET_FEATURE == g_setup_pkt.request) &&
                   (USB_STD_FEATURE_TEST_MODE == g_setup_pkt.value))
                {
                    //let the current request status phase complete.
                    for(delay = 0; delay < 5000 ; delay ++) { asm volatile(""); }
                    switch(g_setup_pkt.index >> USB_WINDEX_HIBITE_SHIFT)
                    {
                        case USB_TEST_MODE_SELECTOR_TEST_J:
                            MSS_USB_CIF_start_testj();
                        break;

                        case USB_TEST_MODE_SELECTOR_TEST_K:
                            MSS_USB_CIF_start_testk();
                        break;

                        case USB_TEST_MODE_SELECTOR_TEST_SE0NAK:
                            MSS_USB_CIF_start_testse0nak();
                        break;

                        case USB_TEST_MODE_SELECTOR_TEST_PACKET:
                            MSS_USB_CIF_start_testpacket();
                        break;
                        default:
                        break;
                    }
                }
                else
                {
                    SETUP_PKT_INIT();
                    MSS_USBD_cep_read_prepare((uint8_t*)&g_setup_pkt,
                                              USB_SETUP_PKT_LEN);
                }
            }
            else
            {
                //end of setup phase for Read/Write Req
                MSS_USBD_CIF_cep_clr_rxpktrdy();

                if((g_setup_pkt.request_type & USB_STD_REQ_DATA_DIR_MASK))
                {
                    cep_ptr->state = MSS_USB_CEP_TX;
                    if((uint8_t*)0 == buf)
                    {
                        ASSERT(0);
                    }

                    if(length > g_setup_pkt.length)
                    {
                        length = g_setup_pkt.length;
                    }

                    MSS_USBD_cep_write(buf,length);
                }
                else
                {
                    cep_ptr->state = MSS_USB_CEP_RX;
                    MSS_USBD_cep_read_prepare(buf,length);
                }
            }
        }
        else
        {
            cep_ptr->state = MSS_USB_CEP_IDLE;
            SETUP_PKT_INIT();
            /*both servicedrxpktrdy and Sendstall should be set in setup phase*/
            MSS_USBD_CIF_cep_stall();
        }
    }

#else /*MSS_USB_DEVICE_TEST_MODE*/
    usbd_test_cb.test_cep_setup(status);
#endif
}

/*******************************************************************************
 This Callback function is called when Data packet is received on Control EP.
 DATAOUT Phase.
 */
static void
mss_usbd_cep_rx_cb
(
    uint8_t status
)
{
#ifndef MSS_USB_DEVICE_TEST_MODE

    uint32_t rem_length;
    mss_usb_ep_t* cep_ptr = &gd_tx_ep[MSS_USB_CEP];

    /*
     xfr_rem_length should have been setup by the Specific request which needed
     data to arrive in the Data phase
     */
    if(status & (CTRL_EP_SETUP_END_ERROR | CTRL_EP_STALL_ERROR))
    {
        SETUP_PKT_INIT();

        if(0 != g_usbd_class_cb->usbd_class_cep_rx_done)
        {
            g_usbd_class_cb->usbd_class_cep_rx_done(status);
        }

        cep_ptr->state = MSS_USB_CEP_IDLE;
        MSS_USBD_cep_read_prepare((uint8_t*)&g_setup_pkt, USB_SETUP_PKT_LEN);
    }
    else
    {
        MSS_USBD_CIF_cep_read_pkt(cep_ptr);

        if(MSS_USB_CEP_RX == cep_ptr->state)
        {
            if(cep_ptr->xfr_count == cep_ptr->xfr_length)
            {
                /*Call USBD-Class indicating that the desired data is arrived*/
                if(0 != g_usbd_class_cb->usbd_class_cep_rx_done)
                {
                    g_usbd_class_cb->usbd_class_cep_rx_done(status);
                }

                MSS_USBD_CIF_reset_index_reg();
                SETUP_PKT_INIT();
                cep_ptr->state = MSS_USB_CEP_IDLE;
                MSS_USBD_CIF_cep_end_wdr();                 //WriteReq complete
                MSS_USBD_cep_read_prepare((uint8_t*)&g_setup_pkt,
                                          USB_SETUP_PKT_LEN);

            }
            else if(cep_ptr->xfr_count < cep_ptr->xfr_length)
            {
                MSS_USBD_CIF_cep_clr_rxpktrdy();       //get more data from host

                /*Continue to read data on the CEP*/
                rem_length = cep_ptr->xfr_length - cep_ptr->xfr_count;

                if(rem_length >= cep_ptr->max_pkt_size)
                {
                    cep_ptr->txn_length = cep_ptr->max_pkt_size;
                }
                else
                {
                    cep_ptr->txn_length = rem_length;
                }

                cep_ptr->txn_count = 0u;
                MSS_USBD_cep_read_prepare((cep_ptr->buf_addr+cep_ptr->xfr_count),
                                          cep_ptr->txn_length);
            }
            else
            {
                SETUP_PKT_INIT();
                cep_ptr->state = MSS_USB_CEP_IDLE;
                MSS_USBD_CIF_cep_stall();               //Send stall
            }
        }
        else
        {
            /*at this stage CEP stage must not be other than MSS_USB_CEP_RX*/
            ASSERT(0);
        }
    }
#else /*MSS_USB_DEVICE_TEST_MODE*/
    usbd_test_cb.test_cep_rx(status);
#endif
}

/*******************************************************************************
 This Callback function is executed when Data packet is sent from control EP.
 DATAIN phase.
 */
static void
mss_usbd_cep_tx_complete_cb
(
    uint8_t status
)
{
#ifndef MSS_USB_DEVICE_TEST_MODE

    mss_usb_ep_t* cep_ptr = &gd_tx_ep[MSS_USB_CEP];

    /*
     xfr_rem_length should have been setup by the Specific request which needed
     data to arrive in the Data phase
     */
    if(status & (CTRL_EP_SETUP_END_ERROR | CTRL_EP_STALL_ERROR))
    {
        SETUP_PKT_INIT();
        if(0 !=  g_usbd_class_cb->usbd_class_cep_tx_done)
        {
             g_usbd_class_cb->usbd_class_cep_tx_done(status);
        }
        cep_ptr->state = MSS_USB_CEP_IDLE;
        MSS_USBD_cep_read_prepare((uint8_t*)&g_setup_pkt, USB_SETUP_PKT_LEN);
    }
    else
    {
        /*Device should be in DATAIN phase.*/
        if(MSS_USB_CEP_TX == cep_ptr->state)
        {
            if(cep_ptr->xfr_count < cep_ptr->xfr_length)
            {
                /*Continue to transmit more data*/
                cep_ptr->buf_addr += cep_ptr->txn_count;

                if((cep_ptr->xfr_length - cep_ptr->xfr_count) >=
                   cep_ptr->max_pkt_size)
                {
                    cep_ptr->txn_length = cep_ptr->max_pkt_size;
                }
                else
                {
                    cep_ptr->txn_length = (cep_ptr->xfr_length - cep_ptr->xfr_count);
                }

                /*
                 Reset the txn_count since one transaction out of the transfer
                 is completed now
                 */
                cep_ptr->txn_count = 0u;
                MSS_USBD_CIF_cep_write_pkt(cep_ptr);
            }
            else
            {
                /*Call USBD-Class indicating that the desired data is sent*/
                if(0 !=  g_usbd_class_cb->usbd_class_cep_tx_done)
                {
                    g_usbd_class_cb->usbd_class_cep_tx_done(status);
                }

                cep_ptr->txn_count = 0u;
                cep_ptr->xfr_count = 0u;
                cep_ptr->xfr_length = 0u;
                cep_ptr->txn_length = 0u;
                cep_ptr->state = MSS_USB_CEP_IDLE;
                SETUP_PKT_INIT();
                MSS_USBD_CIF_reset_index_reg();
                MSS_USBD_cep_read_prepare((uint8_t*)&g_setup_pkt,
                                          USB_SETUP_PKT_LEN);
            }
        }
    }

#else /*MSS_USB_DEVICE_TEST_MODE*/
        usbd_test_cb.test_cep_tx_complete(status);
#endif
}

/*******************************************************************************
 This Call-back function is executed when Data packet is received on RX EP
 */
static void
mss_usbd_ep_rx_cb
(
    mss_usb_ep_num_t ep_num,
    uint8_t status
)
{
    uint8_t transfer_complete = 0u;
    mss_usb_ep_t* rxep_ptr = &gd_rx_ep[ep_num];
    uint32_t received_count = 0u;

    /*
     While receiving data, xfr_length might be known to application or unknown.
     When xfr_length is not known, application is expected to provide
     size of(Receive_buffer) as xfr_length which is big enough to receive
     unknown data. End of data reception in this case is flagged by
     reception of short packet or ZLP(when xfr_length = n*maxpktsz).
     When xfr_length is known(via transport protocol), end of data reception
     is concluded when xfr_count = xfr_length. No zlp is expected to be
     received for the case of xfr_length = n*maxpktsz.
     When DMA-m1 is enabled, the control will return here only once after
     completion of transfer.
     */
    if(status & (RX_EP_OVER_RUN_ERROR | RX_EP_STALL_ERROR |
                 RX_EP_DATA_ERROR | RX_EP_PID_ERROR | RX_EP_ISO_INCOMP_ERROR))
    {
        transfer_complete = 1;
    }
    else
    {
        if(MSS_USB_CIF_rx_ep_is_rxpktrdy(ep_num))
        {
            uint32_t increamented_addr;

            received_count = (uint32_t)MSS_USB_CIF_rx_ep_read_count(ep_num);

            if(DMA_ENABLE == rxep_ptr->dma_enable)
            {
                if(MSS_USB_DMA_MODE1 == (MSS_USB_CIF_rx_ep_get_dma_mode(ep_num)))
                {
                    /*
                    This means we are in BULK transfer with DMA mode1....
                    all the rxmaxP size pkts are received and last short pkt
                    need to be read without DMA or by switching to mode 0.
                    After switching mode to 0, this ISR handler is invoked
                    again. Data packet will be read then.
                    MUSB: section 16
                    */

                    /*read 'short packet' without DMA*/
                    MSS_USB_CIF_dma_stop_xfr(rxep_ptr->dma_channel);
                    MSS_USB_CIF_rx_ep_clr_autoclr(ep_num);

                    /*Count number of bytes read so far,since DMA was
                    operating in m1 with Autoclr.*/
                    increamented_addr = MSS_USB_CIF_dma_read_addr(rxep_ptr->dma_channel);
                    rxep_ptr->xfr_count = (increamented_addr - (ptrdiff_t)(rxep_ptr->buf_addr));

                    if(received_count)
                    {
                        MSS_USB_CIF_read_rx_fifo(ep_num,
                                                 (rxep_ptr->buf_addr+rxep_ptr->xfr_count),
                                                 received_count);

                        rxep_ptr->xfr_count += received_count;
                    }
                    transfer_complete = 1;
                }
                else
                {
                    MSS_USB_CIF_dma_write_count(rxep_ptr->dma_channel, received_count);
                    MSS_USB_CIF_dma_start_xfr(rxep_ptr->dma_channel);
                    transfer_complete = 2u;
                    /*Upper layer cb will be called from DMA ISR*/
                }
            }
            else // no dma
            {
                if(received_count)
                {
                    MSS_USB_CIF_read_rx_fifo(ep_num,
                                             (rxep_ptr->buf_addr+rxep_ptr->xfr_count),
                                             received_count);

                    rxep_ptr->txn_count = received_count;
                    rxep_ptr->xfr_count += received_count;
                }
                if(MSS_USB_XFR_BULK == rxep_ptr->xfr_type)
                {
                    if(ADD_ZLP_TO_XFR == rxep_ptr->add_zlp)
                    {
                        if(rxep_ptr->xfr_count < rxep_ptr->xfr_length)
                        {
                            if(rxep_ptr->txn_count < rxep_ptr->max_pkt_size)
                            {
                                transfer_complete = 1;
                            }
                            else
                            {
                               /*receive short pkt.zlp when xfr_length is multiple of wMaxPktsz*/
                                transfer_complete = 0;
                            }
                        }
                        else if(rxep_ptr->xfr_count == rxep_ptr->xfr_length)
                        {
                            /*buffer is full*/
                            transfer_complete = 1;
                        }
                        else
                        {
                            /*If xfr_count is more than xfr_lenght then something
                            has seriously gone bad.*/
                            ASSERT(0);
                        }
                    }
                    else // no zlp
                    {
                        if(rxep_ptr->xfr_count == rxep_ptr->xfr_length)
                        {
                            transfer_complete = 1;
                        }
                        else if(rxep_ptr->xfr_count < rxep_ptr->xfr_length)
                        {
                            transfer_complete = 0;
                        }
                        else
                        {
                            /*If xfr_count is more than xfr_lenght then something
                            has seriously gone bad.*/
                            ASSERT(0);
                        }
                    }
                }
                else
                {
                    /*
                     For ISO and interrupt transfers, LB-expects only one packet in
                     one transaction HB expects at most 3 packets in one transaction
                     */
                    if(rxep_ptr->txn_count <= rxep_ptr->max_pkt_size)
                    {
                        transfer_complete = 1;
                    }
                    else
                    {
                        ASSERT(0);/*TODO: replace this with stall EP*/
                    }
                }
            }
            MSS_USB_CIF_rx_ep_clr_rxpktrdy(ep_num);
        }
    }

    if(transfer_complete == 0)
    {
        ASSERT(rxep_ptr->xfr_length >= rxep_ptr->xfr_count);

        if((rxep_ptr->xfr_length - rxep_ptr->xfr_count) >= rxep_ptr->max_pkt_size)
        {
            rxep_ptr->txn_length = rxep_ptr->max_pkt_size;
        }
        else
        {
            rxep_ptr->txn_length = (rxep_ptr->xfr_length - rxep_ptr->xfr_count);
        }

        /*
         Reset the txn_count since one transaction out of the
         transfer is completed.
         */
        rxep_ptr->txn_count = 0u;
        MSS_USB_CIF_rx_ep_read_prepare(rxep_ptr->num,
                                       (rxep_ptr->buf_addr +
                                        rxep_ptr->xfr_count),
                                       rxep_ptr->dma_enable,
                                       rxep_ptr->dma_channel,
                                       rxep_ptr->xfr_type,
                                       rxep_ptr->xfr_length);
    }
    else if(transfer_complete == 1)
    {
        #ifndef MSS_USB_DEVICE_TEST_MODE
        if(0 != g_usbd_class_cb->usbd_class_rx_done)
        {
            g_usbd_class_cb->usbd_class_rx_done(ep_num, status, rxep_ptr->xfr_count);
        }
        #else
            usbd_test_cb.test_ep_rx(ep_num, status, rxep_ptr->xfr_count);
        #endif
    }
    else
    {
        /*Do Nothing. DMA m0 will be handled in DMA_Handler*/
    }

}

/******************************************************************************
 This Callback function is executed on transmission complete interrupt event when
 the MSS USB is in device mode.
*/




static void
mss_usbd_ep_tx_complete_cb
(
    mss_usb_ep_num_t num,
    uint8_t status
)
{
    uint8_t transfer_complete = 0u;
    uint32_t increamented_addr = 0u;
    mss_usb_ep_t* txep_ptr = &gd_tx_ep[num];

    if(status & TX_EP_STALL_ERROR)
    {
        transfer_complete = 1u;
    }
    else
    {


        /*
         While transmitting data we always know the xfr_length, use it to
         check if the transfer has ended.

         DMA Enabled:
         Mode-m1 is used for Bulk transfer.In this case all the data
         single or multiple packets is handled by DMA since Autoset bit is set.
         Control will return here only once when complete data is transmitted.
         Mode-m0 is used for ISO/Interrupt transfers. In this case single packet
         should have been given by the application and control will return here
         only once after the single packet is transmitted.

         DMA Disabled:
         For multi-packet bulk transfers, control will reach here after every
         single packet is transferred. Provide next packet for transmission till
         the end of data.
         */
        if(DMA_ENABLE == txep_ptr->dma_enable)
        {
            increamented_addr = MSS_USB_CIF_dma_read_addr(txep_ptr->dma_channel);
            txep_ptr->xfr_count = increamented_addr - ((ptrdiff_t)txep_ptr->buf_addr);

            ASSERT(txep_ptr->xfr_count == txep_ptr->xfr_length);

            /*TODO: Move the decision to transmit ZLP from CIFL to here*/
            transfer_complete = 1u;
        }
        else    /*DMA_DISABLE*/
        {
            txep_ptr->txn_count = txep_ptr->txn_length;
            txep_ptr->xfr_count += txep_ptr->txn_length;

            if(MSS_USB_XFR_BULK == txep_ptr->xfr_type)
            {
                if(txep_ptr->xfr_count < txep_ptr->xfr_length)
                {
                    transfer_complete = 0u;
                }
                else if(txep_ptr->xfr_count == txep_ptr->xfr_length)
                {
                    if(ADD_ZLP_TO_XFR == txep_ptr->add_zlp)
                    {
                        if(0u == txep_ptr->txn_count)
                        {
                            transfer_complete = 1u;
                        }
                        else
                        {
                            if(txep_ptr->txn_count == txep_ptr->max_pkt_size)
                            {
                                transfer_complete = 0u;
                            }
                            else if(txep_ptr->txn_count < txep_ptr->max_pkt_size)
                            {
                                transfer_complete = 1u;
                            }
                        }
                    }
                    else        //no zlp
                    {
                        transfer_complete = 1u;
                    }
                }
                else
                {
                    /*If xfr_count is more than xfr_lenght then something
                      has seriously gone bad.*/
                    ASSERT(0);
                }
            }
            else    /*ISO/INTERRUPT XRF*/
            {
                if(txep_ptr->txn_count != txep_ptr->txn_length)
                {
                    /*The ISO/Interrupt every transfer must be single transaction*/
                    ASSERT(0);
                }
                transfer_complete = 1u;
            }
        }
    }
    if(1u == transfer_complete)
    {
        #ifndef MSS_USB_DEVICE_TEST_MODE
                if(0 != g_usbd_class_cb->usbd_class_tx_done)
                {

                    g_usbd_class_cb->usbd_class_tx_done(num, status);
                }
        #else   /*MSS_USB_DEVICE_TEST_MODE*/
            usbd_test_cb.test_ep_tx_complete(num, status);
        #endif
    }
    else
    {
        /*
         Reset the txn_count since one transaction out of the transfer
         is completed now
         */
        txep_ptr->txn_count = 0u;
        ASSERT(txep_ptr->xfr_length >= txep_ptr->xfr_count);

        if((txep_ptr->xfr_length - txep_ptr->xfr_count) >= txep_ptr->max_pkt_size)
        {
            txep_ptr->txn_length = txep_ptr->max_pkt_size;
        }
        else
        {
            txep_ptr->txn_length = (txep_ptr->xfr_length - txep_ptr->xfr_count);
        }

        while(MSS_USB_CIF_is_txepfifo_notempty(txep_ptr->num));
        MSS_USB_CIF_ep_write_pkt(txep_ptr->num,
                                 (txep_ptr->buf_addr +
                                  txep_ptr->xfr_count),
                                 txep_ptr->dma_enable,
                                 txep_ptr->dma_channel,
                                 txep_ptr->xfr_type,
                                 txep_ptr->xfr_length,
                                 txep_ptr->txn_length);
    }
}

/*******************************************************************************
 This Callback function is executed when suspend interrupt is received when
 the MSS USB is in device mode.
 */
static void
mss_usbd_suspend_cb
(
    void
)
{
    #ifndef MSS_USB_DEVICE_TEST_MODE
        MSS_USB_CIF_enable_usbirq(RESUME_IRQ_MASK);
        g_usbd_dev_conf.device_state_at_suspend = g_usbd_dev_conf.device_state;
        g_usbd_dev_conf.device_state = MSS_USB_SUSPENDED_STATE;
    #else
    /*usbd_test_cb.test_suspend();*/
    #endif
}

/*******************************************************************************
 This Callback function is executed when resume interrupt is received when the
 MSS USB is in device mode.
*/
static void
mss_usbd_resume_cb
(
    void
)
{
    #ifndef MSS_USB_DEVICE_TEST_MODE
        g_usbd_dev_conf.device_state = g_usbd_dev_conf.device_state_at_suspend;
    #else
        usbd_test_cb.test_resume();
    #endif
}

static void mss_usbd_disconnect_cb(void)
{
    #ifndef MSS_USB_DEVICE_TEST_MODE
        g_usbd_dev_conf.device_state = MSS_USB_NOT_ATTACHED_STATE;
        MSS_USB_CIF_rx_ep_disable_irq_all();
        MSS_USB_CIF_tx_ep_disable_irq_all();
        PLIC_EnableIRQ(USB_DMA_PLIC);
        PLIC_EnableIRQ(USB_MC_PLIC);
        if(0 != g_usbd_class_cb->usbd_class_release)
        {
            g_usbd_class_cb->usbd_class_release(0xFF);
        }
    #else
        usbd_test_cb.test_resume();
    #endif
}
/*******************************************************************************
 This function processes standard USB requests.
*/
static uint8_t
mss_usbd_std_requests
(
    uint8_t** buf,
    uint32_t* length
)
{
    uint8_t result = USB_FAIL;
    /*TODO:result should be used for all the functions*/
    switch(g_setup_pkt.request)
    {
        case USB_STD_REQ_GET_DESCRIPTOR:
            result = mss_usbd_get_descriptor(buf, length);
        return result;

        case USB_STD_REQ_SET_ADDRESS:
            mss_usbd_set_address();
        break;

        case USB_STD_REQ_SET_CONFIG:
            mss_usbd_set_config();
        break;

        case USB_STD_REQ_GET_CONFIG:
            mss_usbd_get_config(buf, length);
        break;

        case USB_STD_REQ_GET_STATUS:
            mss_usbd_get_status(buf, length);
        break;

        case USB_STD_REQ_SET_FEATURE:
            result = mss_usbd_set_feature();
        return result;

        case USB_STD_REQ_CLEAR_FEATURE:
            result = mss_usbd_clr_feature();
        return result;

        case USB_STD_REQ_SET_DESCRIPTOR:
            mss_usbd_set_descriptor();
        break;

        case USB_STD_REQ_SET_INTERFACE:
            g_usbd_dev_conf.active_interface_num = (uint8_t)g_setup_pkt.value;
        return USB_SUCCESS;

         case USB_STD_REQ_GET_INTERFACE:
            *buf = &g_usbd_dev_conf.active_interface_num;
            *length = 1u;
        return USB_SUCCESS;

        default:
            return USB_FAIL;
    }

    return USB_SUCCESS;
}

/*******************************************************************************
 This function processes USB class requests.
*/
static uint8_t
mss_usbd_class_requests
(
    uint8_t** buf_pp,
    uint32_t* len_p
)
{
    if((0 != g_usbd_class_cb->usbd_class_process_request))
    {
        if(USB_SUCCESS ==
            g_usbd_class_cb->usbd_class_process_request(&g_setup_pkt,
                                                       buf_pp,
                                                       len_p))
        {
            return USB_SUCCESS;
        }
        else
        {
            return USB_FAIL;
        }
    }
    else
    {
        return USB_FAIL;
    }
}

/*******************************************************************************
 This function processes vendor defined USB requests.
*/
static uint8_t
mss_usbd_vendor_requests
(
    uint8_t** buf_pp,
    uint32_t* len_p
)
{
    if((0 != g_usbd_class_cb->usbd_class_process_request))
    {
        if(USB_SUCCESS ==
            g_usbd_class_cb->usbd_class_process_request(&g_setup_pkt,
                                                       buf_pp,
                                                       len_p))
        {
            return USB_SUCCESS;
        }
        else
        {
            return USB_FAIL;
        }
    }
    else
    {
        return USB_FAIL;
    }
}

static uint8_t
mss_usbd_get_descriptor
(
    uint8_t** buf_pp,
    uint32_t* len_p
)
{
    if((g_usbd_dev_conf.device_state >= MSS_USB_DEFAULT_STATE) &&
        (USB_STD_REQ_DATA_DIR_IN ==
         (g_setup_pkt.request_type & USB_STD_REQ_DATA_DIR_MASK)))
    {
        if(USB_STD_REQ_RECIPIENT_DEVICE ==
           (g_setup_pkt.request_type & USB_STD_REQ_RECIPIENT_MASK))
        {
            switch(g_setup_pkt.value >> USB_WVALUE_HIBITE_SHIFT)
            {
                case USB_DEVICE_DESCRIPTOR_TYPE:

                    /*descriptor Index and Index field should be zero*/
                    if((0u == ((uint8_t)(g_setup_pkt.value))) &&
                        (0u == g_setup_pkt.index) &&
                        (0 != g_usbd_user_descr_cb->usbd_device_descriptor))
                    {
                        *buf_pp = g_usbd_user_descr_cb->usbd_device_descriptor(len_p);
                        return(USB_SUCCESS);
                    }
                    else
                    {
                        return(USB_FAIL);
                    }

                case USB_STRING_DESCRIPTOR_TYPE:

                    /*
                    * When descriptor index is 0, index field must be 0.
                    * When descriptor index is >0, index field indicates Lang ID
                    */
                    if(0 != g_usbd_user_descr_cb->usbd_string_descriptor)
                    {
                        *buf_pp = g_usbd_user_descr_cb->usbd_string_descriptor
                                            ((uint8_t)g_setup_pkt.value, len_p);
                        return(USB_SUCCESS);
                    }
                    else
                    {
                        return(USB_FAIL);
                    }

                case USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE:
                    if(MSS_USB_DEVICE_HS == g_usbd_user_speed)
                    {
                        /*descriptor Index should be zero*/
                        if((0 == ((uint8_t)(g_setup_pkt.value))) &&
                            (0 == g_setup_pkt.index) &&
                            (0 != g_usbd_user_descr_cb->usbd_device_qual_descriptor))
                        {
                            *buf_pp = g_usbd_user_descr_cb->usbd_device_qual_descriptor
                                            (g_usbd_dev_conf.device_speed, len_p);
                            return(USB_SUCCESS);
                        }
                        else
                        {
                            return(USB_FAIL);
                        }
                    }
                    else
                    {
                        /*Since User operates USBD at FS, Stall this request*/
                        return(USB_FAIL);
                    }

                case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                    if(0 != g_usbd_class_cb->usbd_class_get_descriptor)
                    {
                        *buf_pp = g_usbd_class_cb->usbd_class_get_descriptor
                                                (USB_STD_REQ_RECIPIENT_DEVICE,
                                                 USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                                 len_p,
                                                 g_usbd_dev_conf.device_speed);
                        return(USB_SUCCESS);
                    }
                    else
                    {
                        return(USB_FAIL);
                    }

                case USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE:
                    if(MSS_USB_DEVICE_HS == g_usbd_user_speed)
                    {
                        /*descriptor Index should be zero and Index field should be zero*/
                        if((0u == ((uint8_t)(g_setup_pkt.value))) &&
                           (0u == g_setup_pkt.index))
                        {
                            if(0 != g_usbd_class_cb->usbd_class_get_descriptor)
                            {
                                *buf_pp = g_usbd_class_cb->usbd_class_get_descriptor
                                                  (USB_STD_REQ_RECIPIENT_DEVICE,
                                                  USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE,
                                                  len_p,
                                                  g_usbd_dev_conf.device_speed);

                                return(USB_SUCCESS);
                            }
                            else
                            {
                                return(USB_FAIL);
                            }
                        }
                    }
                    else
                    {
                        /*Since User operates USBD at FS, Stall this request*/
                        return(USB_FAIL);
                    }
                break;
                default:
                    return(USB_FAIL);
            }

        }
        else if(USB_STD_REQ_RECIPIENT_INTERFACE ==
                (g_setup_pkt.request_type & USB_STD_REQ_RECIPIENT_MASK))
        {
            if(0 != g_usbd_class_cb->usbd_class_get_descriptor)
            {
                *buf_pp = g_usbd_class_cb->usbd_class_get_descriptor
                                            (USB_STD_REQ_RECIPIENT_INTERFACE,
                                            g_setup_pkt.value >> USB_WVALUE_HIBITE_SHIFT,
                                            len_p,
                                            g_usbd_dev_conf.device_speed);
                return(USB_SUCCESS);
            }
            else
            {
                return(USB_FAIL);
            }
        }
        else if(USB_STD_REQ_RECIPIENT_ENDPOINT ==
                (g_setup_pkt.request_type & USB_STD_REQ_RECIPIENT_MASK))
        {
            if((g_setup_pkt.value >> USB_WVALUE_HIBITE_SHIFT) &&
               (0 != g_usbd_class_cb->usbd_class_get_descriptor))
            {
                *buf_pp = g_usbd_class_cb->usbd_class_get_descriptor
                                            (USB_STD_REQ_RECIPIENT_ENDPOINT,
                                            (g_setup_pkt.value >> USB_WVALUE_HIBITE_SHIFT),
                                            len_p,
                                            g_usbd_dev_conf.device_speed);
                return(USB_SUCCESS);
            }
            else
            {
                return(USB_FAIL);
            }
        }
    }
    return USB_FAIL;
}

/******************************************************************************
 This function sets the Device address sent by host in the MSS USB register.
*/
static uint8_t
mss_usbd_set_address
(
    void
)
{
    uint8_t addr;

    /*USB device address is 7bit only*/
    addr = (uint8_t)(g_setup_pkt.value & 0x7Fu);
    g_usbd_dev_conf.device_addr = addr;

    addr = (uint8_t)(g_setup_pkt.value & 0x7Fu);

    /*USB2.0 section 9.4.6*/
    if(MSS_USB_CONFIGURED_STATE == g_usbd_dev_conf.device_state)
    {
        /*Behaviour not defined by USB2.0, May raise error here*/
        return USB_FAIL;
    }
    else
    {
        if(MSS_USB_DEFAULT_STATE == g_usbd_dev_conf.device_state)
        {
            if(0u != addr)
            {
                g_usbd_dev_conf.device_state = MSS_USB_ADDRESS_STATE;
            }
        }
        else if(MSS_USB_ADDRESS_STATE == g_usbd_dev_conf.device_state)
        {
            if(0u == addr)
            {
                g_usbd_dev_conf.device_state = MSS_USB_DEFAULT_STATE;
            }
        }
        /*FADDR register will be updated after Status phase of this Request*/
    }
    return USB_SUCCESS;
}

/******************************************************************************
 This function processes SET_DESCRIPTOR request.
*/
static uint8_t
mss_usbd_set_descriptor
(
    void
)
{
    return USB_FAIL;
}

/******************************************************************************
 This function processes SET_CONFIGURATION request.
*/
static uint8_t
mss_usbd_set_config
(
    void
)
{
    uint8_t cfgidx;

    cfgidx = (uint8_t)g_setup_pkt.value;

    /*USB2.0 section 9.4.6*/
    if(MSS_USB_DEFAULT_STATE == g_usbd_dev_conf.device_state)
    {
        /*Undefined behaviour*/
        return USB_FAIL;
    }
    else
    {
        /*This value will be returned in Get_config command*/
        g_usbd_dev_conf.active_config_num = cfgidx;
        if(MSS_USB_ADDRESS_STATE == g_usbd_dev_conf.device_state)
        {
            if(cfgidx)
            {
                g_usbd_dev_conf.device_state = MSS_USB_CONFIGURED_STATE;

                if(0 != g_usbd_class_cb->usbd_class_init)
                {
                    g_usbd_class_cb->usbd_class_init(cfgidx,
                                                     g_usbd_dev_conf.device_speed);
                }
            }
        }
        else if(MSS_USB_CONFIGURED_STATE == g_usbd_dev_conf.device_state)
        {
            if(0 == cfgidx)
            {
                g_usbd_dev_conf.device_state = MSS_USB_ADDRESS_STATE;

                if(0 != g_usbd_class_cb->usbd_class_release)
                {
                    g_usbd_class_cb->usbd_class_release(cfgidx);
                }
            }
            else
            {
                if(0 != g_usbd_class_cb->usbd_class_init)
                {
                    g_usbd_class_cb->usbd_class_init(cfgidx,
                                                    g_usbd_dev_conf.device_speed);
                }
            }
        }
    }
    return USB_SUCCESS;
}

/******************************************************************************
 This function processes GET_CONFIGURATION requests
*/
static uint8_t
mss_usbd_get_config
(
    uint8_t** buf_pp,
    uint32_t* len_p
)
{
    /*The field value and index must be 0 and length must be 1*/
    if((0u == g_setup_pkt.value) &&
       (0u == g_setup_pkt.index) &&
       (1u == g_setup_pkt.length))
    {
        /*This value was set in Set_config command*/
        *buf_pp = &g_usbd_dev_conf.active_config_num ;
        *len_p = 1u;
        return USB_SUCCESS;
    }
    else
    {
        return USB_FAIL;
    }
}

/******************************************************************************
 This function processes GET_STATUS request.
*/
static uint8_t
mss_usbd_get_status
(
    uint8_t** buf_pp,
    uint32_t* len_p
)
{
    if(MSS_USB_ADDRESS_STATE == g_usbd_dev_conf.device_state)
    {
        /*The field value and index must be 0 and length must be 2*/
        if((0u == g_setup_pkt.value) &&
           (0u == g_setup_pkt.index) &&
           (2u == g_setup_pkt.length))
        {
            switch (g_setup_pkt.request_type & USB_STD_REQ_RECIPIENT_MASK)
            {
            case USB_STD_REQ_RECIPIENT_DEVICE:
                    /*SF2 device is always self powered. RemoteWakeup NotSupported*/
                    g_usbd_status = 0x0001;
                    *buf_pp = (uint8_t*)&g_usbd_status;
                    break;
            case USB_STD_REQ_RECIPIENT_INTERFACE:        /*Reserved by USB2.0*/
                    g_usbd_status = 0x0000u;
                    *buf_pp = (uint8_t*)&g_usbd_status;
                break;

            case USB_STD_REQ_RECIPIENT_ENDPOINT:/*Endpoint halt (stall) status*/
                    g_usbd_status = ((gd_tx_ep[g_setup_pkt.index].stall) ?
                                     0x0001:0x0000);

                    *buf_pp = (uint8_t*)&g_usbd_status;
                break;
            default:
                return USB_FAIL;
            }
            * len_p = sizeof(g_usbd_status);
        }
        else
        {
            return USB_FAIL;
        }
    }
    else if(MSS_USB_CONFIGURED_STATE == g_usbd_dev_conf.device_state)
    {
        if((0u == g_setup_pkt.value) && (2u == g_setup_pkt.length))
        {
            switch (g_setup_pkt.request_type & USB_STD_REQ_RECIPIENT_MASK)
            {
             case USB_STD_REQ_RECIPIENT_DEVICE: /*SF2 device is self powered*/
                g_usbd_status = 0x0001;
                g_usbd_status |= (g_usbd_dev_conf.remote_wakeup << 0x0001u);
                *buf_pp = (uint8_t*)&g_usbd_status;

             break;

             case USB_STD_REQ_RECIPIENT_INTERFACE:
                if(g_setup_pkt.index <= g_usbd_dev_conf.device_total_interfaces)
                {
                    g_usbd_status = 0x0000u;        /*Reserved by USB2.0*/
                    *buf_pp = (uint8_t*)&g_usbd_status;
                }
                else
                {
                    return USB_FAIL;
                }
            break;
            case USB_STD_REQ_RECIPIENT_ENDPOINT:
                if(((uint8_t)(g_setup_pkt.index) & 0x80u)) /* IN,TX endpoint*/
                {
                    uint8_t idx = (((uint8_t)(g_setup_pkt.index))& 0x7fu);
                    g_usbd_status = ((gd_tx_ep[idx].stall) ? 0x0001:0x0000) ;
                    *buf_pp = (uint8_t*)&g_usbd_status;
                }
                else    /*out, rx endpoint*/
                {
                    g_usbd_status = ((gd_rx_ep[g_setup_pkt.index].stall) ?
                                     0x0001:0x0000) ;
                    *buf_pp = (uint8_t*)&g_usbd_status;
                }
             break;

             default:
                return USB_FAIL;
            }
            * len_p = sizeof(g_usbd_status);
        }
        else
        {
            return USB_FAIL;
        }

    }
    else if(MSS_USB_DEFAULT_STATE == g_usbd_dev_conf.device_state)
    {
        return USB_FAIL;
    }
    return USB_SUCCESS;
}

/******************************************************************************
 This function processes SET_FEATURE request.
*/
static uint8_t
mss_usbd_set_feature
(
    void
)
{
    uint8_t result = USB_SUCCESS;

    if(0u == g_setup_pkt.length)
    {
        switch(g_setup_pkt.value)
        {
        case USB_STD_FEATURE_REMOTE_WAKEUP:
            if((USB_STD_REQ_RECIPIENT_DEVICE ==
                (g_setup_pkt.request_type & USB_STD_REQ_RECIPIENT_MASK)) &&
                (g_usbd_dev_conf.device_state > MSS_USB_DEFAULT_STATE))
            {
                /*Enable Remote wakeup capability for the device*/
                g_usbd_dev_conf.remote_wakeup = 0x01u;
                ASSERT(0);  //RemoteWakeup Not enabled by Config Descr.
            }
            else
            {
                result = USB_FAIL;
            }
            break;
        case USB_STD_FEATURE_EP_HALT:
            if((USB_STD_REQ_RECIPIENT_ENDPOINT ==
                (g_setup_pkt.request_type & USB_STD_REQ_RECIPIENT_MASK)) &&
                ((uint8_t)(g_setup_pkt.index)) &&
                (MSS_USB_CONFIGURED_STATE == g_usbd_dev_conf.device_state))
            {
                /*8.5.3.4 Control EP should not be implementing HALT feature*/
                if(((uint8_t)(g_setup_pkt.index) & 0x80u)) /* IN,TX endpoint*/
                {
                    /*Enable HALT*/
                    gd_tx_ep[(((uint8_t)(g_setup_pkt.index))& 0x7fu)].stall = 0x01;
                    MSS_USBD_CIF_tx_ep_stall((mss_usb_ep_num_t)((((uint8_t)(g_setup_pkt.index)) & 0x7fu)));
                }
                else    /*out, rx endpoint*/
                {
                    /*Enable HALT*/
                    gd_rx_ep[(uint8_t)(g_setup_pkt.index)].stall = 0x01;
                    MSS_USBD_CIF_rx_ep_stall((mss_usb_ep_num_t)(g_setup_pkt.index));
                }
            }
            else
            {
                result = USB_FAIL;
            }
            break;
            case USB_STD_FEATURE_TEST_MODE:
                if((USB_STD_REQ_RECIPIENT_DEVICE ==
                    (g_setup_pkt.request_type & USB_STD_REQ_RECIPIENT_MASK)) &&
                   ((g_setup_pkt.index & 0xFF) == 0x00))
                {
                    result = USB_SUCCESS;
                }
                else
                {
                    result = USB_FAIL;
                }
            break;

        default:
            result = USB_FAIL;
            break;
        }
    }
    else
    {
        result = USB_FAIL;
    }
    return result;
}

/******************************************************************************
 This function processes CLEAR_FEATURE.
*/
static uint8_t
mss_usbd_clr_feature
(
    void
)
{
    uint8_t result = USB_SUCCESS;

    /*TestMode Feature cant be cleared by clr_feature.Device power cycle required*/
    if((0u == g_setup_pkt.length) &&
       (g_usbd_dev_conf.device_state > MSS_USB_DEFAULT_STATE))
    {
        switch(g_setup_pkt.value)
        {
        case USB_STD_FEATURE_REMOTE_WAKEUP:
            if(USB_STD_REQ_RECIPIENT_DEVICE ==
               (g_setup_pkt.request_type & USB_STD_REQ_RECIPIENT_MASK))
            {
                /*Disable Remote wakeup capability for the device*/
                g_usbd_dev_conf.remote_wakeup = 0x00u;
            }
            break;
        case USB_STD_FEATURE_EP_HALT:
            if(USB_STD_REQ_RECIPIENT_ENDPOINT ==
               (g_setup_pkt.request_type & USB_STD_REQ_RECIPIENT_MASK))
            {
                /*8.5.3.4 HALT clear can be performed on CEP and Data EP*/
                if((uint8_t)(g_setup_pkt.index)) /*Not a control endpoint*/
                {
                    if(((uint8_t)(g_setup_pkt.index) & 0x80u)) /* IN,TX endpoint*/
                    {
                        /*Disable HALT*/
                        gd_tx_ep[(((uint8_t)(g_setup_pkt.index))& 0x7fu)].stall = 0x00u;
                        MSS_USBD_CIF_tx_ep_clr_stall((mss_usb_ep_num_t)((((uint8_t)(g_setup_pkt.index)) & 0x7fu)));
                    }
                    else    /* out, rx endpoint */
                    {
                        /*Enable HALT*/
                        gd_rx_ep[(uint8_t)(g_setup_pkt.index)].stall = 0x00u;
                        MSS_USBD_CIF_rx_ep_clr_stall((mss_usb_ep_num_t)(g_setup_pkt.index));
                    }
                }
                else
                {
                    result = USB_SUCCESS;
                }
            }
            break;
        default:
            result = USB_FAIL;
            break;
        }
    }
    else
    {
        result = USB_FAIL;
    }
    return result;
}

static void mss_usbd_dma_handler_cb
(
    mss_usb_ep_num_t ep_num,
    mss_usb_dma_dir_t dma_dir,
    uint8_t status,
    uint32_t dma_addr_val
)
{
    mss_usb_ep_t *ep_ptr = 0;

    if(DMA_XFR_ERROR == status)
    {
        ASSERT(0);
    }
    else
    {

        if(MSS_USB_DMA_READ == dma_dir)    /*TX EP*/
        {
            ep_ptr = &gd_tx_ep[ep_num];

            /*EP interrupt wont happen when */
            if((MSS_USB_DMA_MODE1 == (MSS_USB_CIF_tx_ep_get_dma_mode(ep_num))) &&
               (NO_ZLP_TO_XFR == ep_ptr->add_zlp) &&
               (ep_ptr->xfr_length) &&
               (!(ep_ptr->xfr_length % ep_ptr->max_pkt_size)))
            {
                /* wait till last TxMaxPkt size packet is sent.*/
                while(MSS_USB_CIF_tx_ep_is_txpktrdy(ep_num));
                ep_ptr->xfr_count = dma_addr_val - (ptrdiff_t)ep_ptr->buf_addr;

                if(0 != g_usbd_class_cb->usbd_class_tx_done)
                {
                    /* call-back class driver */
                    g_usbd_class_cb->usbd_class_tx_done(ep_num, status);
                }
            }
            else
            {
                MSS_USB_CIF_tx_ep_set_txpktrdy(ep_num);
            }
        }
        else if(MSS_USB_DMA_WRITE == dma_dir)/*RX EP*/
        {
            ep_ptr = &gd_rx_ep[ep_num];

            if((NO_ZLP_TO_XFR == ep_ptr->add_zlp) &&
               (ep_ptr->xfr_length) &&
               (!(ep_ptr->xfr_length % ep_ptr->max_pkt_size)))
            {
                ep_ptr->xfr_count = dma_addr_val - (ptrdiff_t)ep_ptr->buf_addr;

                if(MSS_USB_DMA_MODE0 == (MSS_USB_CIF_rx_ep_get_dma_mode(ep_num)))
                {
                    MSS_USB_CIF_rx_ep_clr_rxpktrdy(ep_num);
                }
                if(0 != g_usbd_class_cb->usbd_class_rx_done)
                {
                    /* call-back class driver */
                    g_usbd_class_cb->usbd_class_rx_done(ep_num, status,
                                                        ep_ptr->xfr_count);
                }
            }
        }
    }
}

#endif  //MSS_USB_DEVICE_ENABLED

#ifdef __cplusplus
}
#endif
