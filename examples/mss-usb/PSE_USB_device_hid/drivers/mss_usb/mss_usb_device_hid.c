/*******************************************************************************
 * (c) Copyright 2018 Microsemi SoC Products Group. All rights reserved.
 *
 * Microsemi Smartfusion2 MSS USB Driver Stack
 *      USB Logical Layer (USB-LL)
 *      USBD-HID class driver
 *
 * This file implements HID class functionality.
 * HID Mouse report is supported.
 *
 * 
 * SVN $Revision: 10280 $
 * SVN $Date: 2018-08-01 16:36:37 +0530 (Wed, 01 Aug 2018) $
 */

#include "mss_usb_device_hid.h"
#include "mss_usb_device.h"
#include "mss_usb_std_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_DEVICE_ENABLED

#define HID_CONF_DESCR_DESCTYPE_IDX             1u
#define HID_CONF_DESCR_HIDDESCRTYPE_IDX         18u

/***************************************************************************//**
 * Private Functions
 */
static uint8_t* usbd_hid_get_descriptor_cb(uint8_t recepient,
                                           uint8_t type,
                                           uint32_t* length,
                                           mss_usb_device_speed_t musb_speed);

static uint8_t usbd_hid_init_cb(uint8_t cfgidx, mss_usb_device_speed_t musb_speed);
static uint8_t usbd_hid_release_cb(uint8_t cfgidx);

static uint8_t usbd_hid_process_request_cb(mss_usbd_setup_pkt_t * setup_pkt,
                                           uint8_t** buf_pp,
                                           uint32_t* length);

static uint8_t usbd_hid_tx_complete_cb(mss_usb_ep_num_t num, uint8_t status);
static uint8_t usbd_hid_rx_cb(mss_usb_ep_num_t num, uint8_t status, uint32_t rx_count);
static uint8_t usbd_hid_cep_tx_done_cb(uint8_t status);
static uint8_t usbd_hid_cep_rx_done_cb(uint8_t status);

/* Call-back function structure definition needed by USB Device Core Driver */
mss_usbd_class_cb_t usbd_hid_cb = {
    usbd_hid_init_cb,
    usbd_hid_release_cb,
    usbd_hid_get_descriptor_cb,
    usbd_hid_process_request_cb,
    usbd_hid_tx_complete_cb,
    usbd_hid_rx_cb,
    usbd_hid_cep_tx_done_cb,
    usbd_hid_cep_rx_done_cb
};

volatile uint32_t g_tx_complete_status = 1u;

/* USB current Speed of operation selected by user*/
mss_usb_device_speed_t g_usbd_hid_user_speed;

mss_usbd_hid_state_t g_hid_state = USBD_HID_NOT_CONFIGURED;

uint8_t hid_conf_descr[HID_CONFIG_DESCR_LENGTH] = {
    /*----------------------- Configuration Descriptor -----------------------*/
    USB_STD_CONFIG_DESCR_LEN,                           /* bLength */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,                  /* bDescriptorType */
    0x22u,                                              /* wTotalLength LSB */
    0x00u,                                              /* wTotalLength MSB */
    0x01u,                                              /* bNumInterfaces */
    0x01u,                                              /* bConfigurationValue */
    0x04u,                                              /* iConfiguration */
    0xC0u,                                              /* bmAttributes */
    0x32u,                                              /* bMaxPower */
    /*------------------------- Interface Descriptor -------------------------*/
    USB_STD_INTERFACE_DESCR_LEN,                        /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE,                      /* bDescriptorType */
    0x00u,                                              /* bInterfaceNumber */
    0x00u,                                              /* bAlternateSetting */
    0x01u,                                              /* bNumEndpoints */
    0x03u,                                              /* bInterfaceClass */
    0x00u,                                              /* bInterfaceSubClass */
    0x02u,                                              /* bInterfaceProtocol */
    0x00u,                                              /* bInterface */
    /*---------------------------- HID Descriptor ----------------------------*/
    USB_HID_DESCR_LENGTH,                               /* bLength */
    0x21u,                                              /* bDescriptorType */
    0x10u,                                              /* bcdHID */
    0x01u,
    0x00u,                                              /* bCountryCode */
    0x01u,                                              /* bNumDescriptor */
    0x22u,                                              /* bDescriptorType */
    0x34u,                                              /* wDescriptorLength */
    0x00u,
    /*------------------------- Endpoint Descriptor --------------------------*/
    USB_STD_ENDPOINT_DESCR_LEN,                         /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                       /* bDescriptorType */
    (0x80u | HID_INTR_TX_EP),                           /* bEndpointAddress */
    0x03u,                                              /* bmAttributes */
    0x08u,                                              /* wMaxPacketSize LSB */
    0x00u,                                              /* wMaxPacketSize MSB */
    0x06u                                               /* bInterval */
};

/*
Report descriptor: sent to the host during enumeration process.
This descriptors defines the format of the HID report.
*/
uint8_t report_descr[] = {
    0x05u, 0x01u,                                   /* USAGE_PAGE (Generic Desktop) */
    0x09u, 0x02u,                                   /* USAGE (Mouse) */
    0xA1u, 0x01u,                                   /* COLLECTION (Application) */
    0x09u, 0x01u,                                   /* USAGE (Pointer) */
    0xA1u, 0x00u,                                   /* COLLECTION (Physical) */
    0x05u, 0x09u,                                   /* USAGE_PAGE (Button) */
    0x19u, 0x01u,                                   /* USAGE_MINIMUM (Button 1) */
    0x29u, 0x03u,                                   /* USAGE_MAXIMUM (Button 3) */
    0x15u, 0x00u,                                   /* LOGICAL_MINIMUM (0) */
    0x25u, 0x01u,                                   /* LOGICAL_MAXIMUM (1) */
    0x75u, 0x01u,                                   /* REPORT_SIZE (1) */
    0x95u, 0x03u,                                   /* REPORT_COUNT (3) */
    0x81u, 0x02u,                                   /* INPUT (Data,Var,Abs) */
    0x75u, 0x05u,                                   /* REPORT_SIZE (5) */
    0x95u, 0x01u,                                   /* REPORT_COUNT (1) */
    0x81u, 0x01u,                                   /* INPUT (Const,Array,Abs) */
    0x05u, 0x01u,                                   /* USAGE_PAGE (Generic Desktop) */
    0x09u, 0x30u,                                   /* USAGE (X) */
    0x09u, 0x31u,                                   /* USAGE (Y) */
    0x09u, 0x38u,                                   /* USAGE(Wheel) */
    0x15u, 0x81u,                                   /* LOGICAL_MINIMUM (-127) */
    0x25u, 0x7Fu,                                   /* LOGICAL_MAXIMUM (127) */
    0x75u, 0x08u,                                   /* REPORT_SIZE (8) */
    0x95u, 0x03u,                                   /* REPORT_COUNT (3) */
    0x81u, 0x06u,                                   /* INPUT (Data,Var,Rel) */
    0xC0u, 0xC0u                                    /* END_COLLECTION */
};

/***************************************************************************//**
* Exported function from HID Class driver.
*******************************************************************************/

/***************************************************************************//**
* See mss_usb_device_hid.h for details of how to use this function.
*/
void
MSS_USBD_HID_init
(
    mss_usb_device_speed_t speed
)
{
    g_usbd_hid_user_speed = speed;
    MSS_USBD_set_class_cb_handler(&usbd_hid_cb);
}

/***************************************************************************//**
* See mss_usb_device_hid.h for details of how to use this function.
*/
uint32_t
MSS_USBD_HID_tx_report
(
    uint8_t * buf,
    uint32_t length
)
{
    if(USBD_HID_CONFIGURED == g_hid_state)
    {
        g_tx_complete_status = 0u;
        MSS_USBD_tx_ep_write(HID_INTR_TX_EP, buf, length);
        return (USB_SUCCESS);
    }
    else
    {
        return (USB_FAIL);
    }
}

/***************************************************************************//**
* See mss_usb_device_hid.h for details of how to use this function.
*/
uint8_t
MSS_USBD_HID_tx_done
(
    void
)
{
    return(g_tx_complete_status);
}

/***************************************************************************//**
 Private functions to USBD_HID class driver.
 ******************************************************************************/

static uint8_t*
usbd_hid_get_descriptor_cb
(
    uint8_t recepient,
    uint8_t type,
    uint32_t* length,
    mss_usb_device_speed_t musb_speed
)
{
    /*User Selected FS:
            Operate only in FS
      User Selected HS:
        Device connected to 2.0 Host(musb_speed = HS):Operate in HS
        Device connected to 1.x Host(musb_speed = FS):Operate in FS
    */

    /*
    Since Endpoint Size is wMaxpacketSize is 8, which is valid for both
    FS and HS, no need to make decision based on musb_speed.

    bInterval value is 6.
    For FS it results in 6mSec polling period
    For HS it results in 4mSec polling period
    Since this is OK for mouse app, we will return same configuration for
    Other Speed Config as well.
    */

    if(USB_STD_REQ_RECIPIENT_DEVICE == recepient )
    {
        if(USB_CONFIGURATION_DESCRIPTOR_TYPE == type)
        {
            hid_conf_descr[HID_CONF_DESCR_DESCTYPE_IDX] = USB_CONFIGURATION_DESCRIPTOR_TYPE;
            *length = sizeof(hid_conf_descr);
            return(hid_conf_descr);
        }
        else if(USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE == type)
        {
            hid_conf_descr[HID_CONF_DESCR_DESCTYPE_IDX] = USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE;
            *length = sizeof(hid_conf_descr);
            return(hid_conf_descr);
        }
    }
    else if( USB_STD_REQ_RECIPIENT_ENDPOINT == recepient )/*Need index(EP Num)*/
    {
        /*Do nothing*/
    }
    else if(USB_STD_REQ_RECIPIENT_INTERFACE == recepient)/*Need index(interface number)*/
    {
        if(USB_REPORT_DESCRIPTOR_TYPE == type)
        {
            *length = sizeof(report_descr);
            return(report_descr);
        }
        else if (USB_HID_DESCRIPTOR_TYPE == type)
        {
            *length = USB_HID_DESCR_LENGTH;
            return(&hid_conf_descr[HID_CONF_DESCR_HIDDESCRTYPE_IDX]);
        }
    }
    else
    {
        /*Do nothing*/
    }

    return USB_FAIL;
}

static uint8_t
usbd_hid_init_cb
(
    uint8_t cfgidx,
    mss_usb_device_speed_t musb_speed
)
{
    /*
    Since Endpoint Size is wMaxpacketSize is 8, which is valid for both
    FS and HS, we don't need to make decision based on musb_speed
    */
    g_tx_complete_status = 1u;
    MSS_USBD_tx_ep_configure(HID_INTR_TX_EP,
                             HID_INTR_TX_EP_FIFO_ADDR,
                             HID_INTR_TX_EP_MAX_PKT_SIZE,
                             HID_INTR_TX_EP_MAX_PKT_SIZE,
                             1u,
                             DMA_DISABLE,
                             MSS_USB_DMA_CHANNEL1,
                             MSS_USB_XFR_INTERRUPT,
                             NO_ZLP_TO_XFR);

    g_hid_state = USBD_HID_CONFIGURED;
    return USB_SUCCESS;
}

static uint8_t
usbd_hid_release_cb
(
    uint8_t cfgidx
)
{
    g_hid_state = USBD_HID_NOT_CONFIGURED;
    return USB_SUCCESS;
}

static uint8_t
usbd_hid_process_request_cb
(
    mss_usbd_setup_pkt_t * setup_pkt,
    uint8_t** buf_pp,
    uint32_t* length
)
{
    return USB_FAIL;
}

static uint8_t
usbd_hid_tx_complete_cb
(
    mss_usb_ep_num_t num,
    uint8_t status
)
{
    if(status & (TX_EP_UNDER_RUN_ERROR|TX_EP_STALL_ERROR) )
    {
        MSS_USBD_tx_ep_flush_fifo(HID_INTR_TX_EP);
    }
     g_tx_complete_status = 1u;


    return USB_SUCCESS;
}

static uint8_t
usbd_hid_rx_cb
(
    mss_usb_ep_num_t num,
    uint8_t status,
    uint32_t rx_count
)
{
    return USB_SUCCESS;
}

static uint8_t
usbd_hid_cep_tx_done_cb
(
    uint8_t status
)
{
    return USB_SUCCESS;
}
static uint8_t
usbd_hid_cep_rx_done_cb
(
    uint8_t status
)
{
    return USB_SUCCESS;
}

#endif  //MSS_USB_DEVICE_ENABLED

#ifdef __cplusplus
}
#endif
