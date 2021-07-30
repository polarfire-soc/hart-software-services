/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Microchip PolarFire SoC MSS USB Driver Stack
 *      USB Logical Layer (USB-LL)
 *          USBH-HID class driver.
 *
 *  This file implements Host side HID class specific initialization
 *  and request handling.
 *
 * SVN $Revision$
 * SVN $Date$
 */

#include "mss_usb_host_hid.h"
#include "mss_usb_host.h"
#include "mss_usb_std_def.h"

#include <string.h>
#include <stdio.h>
#include "mss_assert.h"
#include "mss_plic.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_HOST_ENABLED

/***************************************************************************//**
  Constant values internally used by USBH-HID driver.
 */
#define MSS_USBH_HID_CLASS_ID                               0x03u
#define MSS_USBH_HID_SUBCLASS_ID                            0x01u
#define MSS_USBH_HID_PROTOCOL_ID                            0x02u

#define MSS_USBH_HID_DRIVER_ID       (uint32_t)((MSS_USBH_HID_CLASS_ID << 16) |\
                                               (MSS_USBH_HID_SUBCLASS_ID << 8)|\
                                               (MSS_USBH_HID_PROTOCOL_ID) )


#define USBH_HID_INTR_RX_PIPE                               MSS_USB_RX_EP_1
#define USBH_HID_INTR_TX_PIPE_FIFOADDR                      0x100u
#define USBH_HID_INTR_RX_PIPE_FIFOADDR                      0x300u
#define USBH_HID_INTR_TX_PIPE_FIFOSZ                        0x40u
#define USBH_HID_INTR_RX_PIPE_FIFOSZ                        0x40u
#define USBH_HID_DESC_SIZE                                  9
#define USBH_HID_DESC                                       0x21
#define USBH_HID_REPORT_DESC                                0x22
#define USBH_HID_SET_IDLE                                   0x0A

#define HID_MIN_POLL                           10
#define HC_PID_DATA0                           0
#define HC_PID_DATA2                           1
#define HC_PID_DATA1                           2
#define HC_PID_SETUP                           3

/* States for HID State Machine */
typedef enum
{
  HID_IDLE= 0,
  HID_SEND_DATA,
  HID_BUSY,
  HID_GET_DATA,
  HID_READ_DATA,
  HID_WAIT_READ_DATA,
  HID_SYNC,
  HID_POLL,
  HID_ERROR,
}
HID_State;


typedef struct USBH_HIDDescriptor
{
  uint8_t   bLength;
  uint8_t   bDescriptorType;
  uint16_t  bcdHID;               /* indicates what endpoint this descriptor is
                                   * describing */
  uint8_t   bCountryCode;         /* specifies the transfer type. */
  uint8_t   bNumDescriptors;      /* specifies the transfer type. */
  uint8_t   bReportDescriptorType;/* Maximum Packet Size this endpoint is
                                   * capable of sending or receiving */
  uint16_t  wItemLength;          /* is used to specify the polling interval
                                   * of certain transfers. */
}
USBH_HIDDesc_TypeDef_t;

USBH_HIDDesc_TypeDef_t              USBH_HID_Desc;

#pragma data_alignment = 4
uint8_t g_hid_report[50] = {0};
volatile uint8_t toggle_in = 0;
HID_State HID_Machine_state= HID_IDLE;
volatile uint8_t next = 0;
volatile uint8_t USBH_HID_RX_buffer[10] = {0x00};

/***************************************************************************//**
  Types internally used by USBH-HID driver.
 */
typedef struct {
 uint8_t num;
 uint16_t maxpktsz;
 uint16_t desclen;
} tdev_ep_t;

/***************************************************************************//**
 Private functions declarations for USBH-HID driver.
 */
static volatile uint8_t g_usbh_hid_alloc_event = 0u;
static volatile uint8_t g_usbh_hid_cep_event = 0u;
static volatile uint8_t g_usbh_hid_tx_event = 0u;
static volatile uint8_t g_usbh_hid_rx_event = 0u;

static uint8_t g_hid_tdev_addr = 0u;
static mss_usb_state_t hid_tdev_state = MSS_USB_NOT_ATTACHED_STATE;
static uint8_t g_hid_conf_desc[34] = {0};
static tdev_ep_t g_tdev_in_ep = {0};
static volatile mss_usbh_hid_state_t g_hid_state = USBH_HID_IDLE;
static mss_usbh_hid_err_code_t g_hidh_error_code = USBH_HID_NO_ERROR;
static mss_usbh_hid_user_cb_t* g_hidh_user_cb;

static uint8_t usbh_hid_allocate_cb(uint8_t tdev_addr);
static uint8_t usbh_hid_release_cb(uint8_t tdev_addr);
static uint8_t usbh_hid_cep_done_cb(uint8_t tdev_addr,
                                    uint8_t status, uint32_t count);
static uint8_t usbh_hid_tx_complete_cb(uint8_t tdev_addr,
                                       uint8_t status,
                                       uint32_t count);
static uint8_t usbh_hid_rx_cb(uint8_t tdev_addr, uint8_t status, uint32_t count);
static mss_usbh_hid_err_code_t MSS_USBH_HID_validate_class_desc(uint8_t* p_cd);
static mss_usbh_hid_err_code_t MSS_USBH_HID_extract_tdev_ep_desc(void);
static void USBH_HID_Handle(void);

/***************************************************************************//**
  Definition of Class call-back functions used by USBH driver.
 */
mss_usbh_class_cb_t hid_class =
{
    MSS_USBH_HID_DRIVER_ID,
    usbh_hid_allocate_cb,
    usbh_hid_release_cb,
    usbh_hid_cep_done_cb,
    usbh_hid_tx_complete_cb,
    usbh_hid_rx_cb,
    0
};

/*******************************************************************************
 * EXPORTED API Functions
 ******************************************************************************/

/******************************************************************************
 * See mss_usb_host_hid.h for details of how to use this function.
 */
void
MSS_USBH_HID_init
(
    mss_usbh_hid_user_cb_t* user_sb
)
{
    g_hid_state = USBH_HID_IDLE;
    memset(g_hid_conf_desc, 0u, sizeof(g_hid_conf_desc));
    g_tdev_in_ep.maxpktsz = 0u;
    g_tdev_in_ep.num = 0u;
    g_hid_tdev_addr = 0u;
    g_hidh_user_cb = user_sb;
    HID_Machine_state= HID_IDLE;
}

/******************************************************************************
 * See mss_usb_host_hid.h for details of how to use this function.
 */
void* MSS_USBH_HID_get_handle
(
    void
)
{
    return((void*)&hid_class);
}

/******************************************************************************
 * See mss_usb_host_hid.h for details of how to use this function.
 */
void
MSS_USBH_HID_task
(
    void
)
{
    uint8_t std_req_buf[USB_SETUP_PKT_LEN] = {0};
    static volatile uint32_t wait_mili = 0u;

    switch (g_hid_state)
    {
        case USBH_HID_IDLE:
            if (g_usbh_hid_alloc_event)
            {
                g_usbh_hid_alloc_event = 0;
                g_hid_state = USBH_HID_GET_CLASS_DESCR;
            }
        break;

        case USBH_HID_GET_CLASS_DESCR:
            /* Read all the User Configuration descripter, HID descripter,
             * Interface and Endpoint  descriptor in one go instaed of reading
             * each descripter individually.
             * May seperated it after completing implementation
             *       #define USB_CONFIGURATION_DESC_SIZE                      9
             *       #define USB_HID_DESC_SIZE                                9
             *       #define USB_INTERFACE_DESC_SIZE                          9
             *       #define USB_ENDPOINT_DESC_SIZE                           7
             */
            hid_tdev_state = MSS_USBH_get_tdev_state(g_hid_tdev_addr);
            if (MSS_USB_ADDRESS_STATE == hid_tdev_state)
            {
                mss_usb_ep_state_t cep_st;
                cep_st = MSS_USBH_get_cep_state();
                if (MSS_USB_CEP_IDLE == cep_st)
                {
                    MSS_USBH_configure_control_pipe(g_hid_tdev_addr);
                    memset(std_req_buf, 0u, 8*(sizeof(uint8_t)));
                    MSS_USBH_construct_get_descr_command(std_req_buf,
                                             USB_STD_REQ_DATA_DIR_IN,
                                             USB_STANDARD_REQUEST,
                                             USB_STD_REQ_RECIPIENT_DEVICE,
                                             USB_STD_REQ_GET_DESCRIPTOR,
                                             USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                             0, /*stringID*/
                                             34);

                    g_hid_state = USBH_HID_WAIT_GET_CLASS_DESCR;
                    MSS_USBH_start_control_xfr(std_req_buf,
                                               (uint8_t*)&g_hid_conf_desc,
                                               USB_STD_REQ_DATA_DIR_IN,
                                               34);
                }
            }
        break;

        case USBH_HID_WAIT_GET_CLASS_DESCR:
            if (g_usbh_hid_cep_event)
            {
                mss_usbh_hid_err_code_t res = USBH_HID_NO_ERROR;
                g_usbh_hid_cep_event = 0u;
                res = MSS_USBH_HID_validate_class_desc(0);

                if (res == 0u)
                {
                    g_hid_state = USBH_HID_SET_CONFIG;
                }
                else
                {
                    g_hid_state = USBH_HID_ERROR;
                    g_hidh_error_code = res;
                }

                res = MSS_USBH_HID_extract_tdev_ep_desc();

                if (res == 0u)
                {
                    g_hid_state = USBH_HID_SET_CONFIG;
                }
                else
                {
                    g_hid_state = USBH_HID_ERROR;
                    g_hidh_error_code = res;
                }
            }
        break;

        case USBH_HID_SET_CONFIG:
            if (0 != g_hidh_user_cb->hidh_valid_config)
            {
                g_hidh_user_cb->hidh_valid_config();
            }

            memset(std_req_buf, 0u, 8*(sizeof(uint8_t)));
            std_req_buf[1] = USB_STD_REQ_SET_CONFIG;
            std_req_buf[2] = g_hid_conf_desc[5];
            g_hid_state = USBH_HID_WAIT_SET_CONFIG;

            MSS_USBH_start_control_xfr(std_req_buf,
                                       (uint8_t*)&g_hid_conf_desc,
                                       USB_STD_REQ_DATA_DIR_IN,
                                       0u);
        break;

        case USBH_HID_WAIT_SET_CONFIG:
            if (g_usbh_hid_cep_event)
            {
                g_usbh_hid_cep_event = 0;
                wait_mili = MSS_USBH_get_milis();
                g_hid_state = USBH_HID_WAIT_DEV_SETTLE;
            }
        break;

        case USBH_HID_WAIT_DEV_SETTLE:
            /* After SET_CONFIG command, we must give time for device to settle
             * down as per spec*/
            if ((MSS_USBH_get_milis() - wait_mili) > 60)
            {
                g_hid_state = USBH_HID_REQ_GET_HID_DESC;
            }
        break;

        case USBH_HID_REQ_GET_HID_DESC:
            {
              for (uint32_t i = 0; i < sizeof(g_hid_conf_desc); i++)
              {
                  g_hid_conf_desc[i] = 0;
              }

              memset(std_req_buf, 0u, 8*(sizeof(uint8_t)));
              MSS_USBH_construct_get_descr_command(std_req_buf,
                                               USB_STD_REQ_DATA_DIR_IN,
                                               USB_STANDARD_REQUEST,
                                               USB_STD_REQ_RECIPIENT_INTERFACE,
                                               USB_STD_REQ_GET_DESCRIPTOR,
                                               USBH_HID_DESC,
                                               0, /*stringID*/
                                               USBH_HID_DESC_SIZE);

              g_hid_state = USBH_HID_WAIT_GET_HID_DESC;
              MSS_USBH_start_control_xfr(std_req_buf,(uint8_t*)&USBH_HID_Desc,
                                            USB_STD_REQ_DATA_DIR_IN,
                                            9);

            }
        break;

        case USBH_HID_WAIT_GET_HID_DESC:
            if (g_usbh_hid_cep_event)
            {
                mss_usbh_hid_err_code_t res = USBH_HID_NO_ERROR;
                g_usbh_hid_cep_event = 0u;
                g_hid_state = USBH_HID_REQ_GET_REPORT_DESC;
            }
        break;

        case USBH_HID_REQ_GET_REPORT_DESC:
             MSS_USBH_configure_control_pipe(g_hid_tdev_addr);
             memset(std_req_buf, 0u, 8*(sizeof(uint8_t)));
             MSS_USBH_construct_get_descr_command(std_req_buf,
                                 USB_STD_REQ_DATA_DIR_IN,
                                 USB_STANDARD_REQUEST,
                                 USB_STD_REQ_RECIPIENT_INTERFACE,
                                 USB_STD_REQ_GET_DESCRIPTOR,
                                 USBH_HID_REPORT_DESC,
                                 0, /*stringID*/
                                 USBH_HID_Desc.wItemLength);

             g_hid_state = USBH_HID_WAIT_REQ_GET_REPORT_DESC;
             MSS_USBH_start_control_xfr(std_req_buf,
                                       (uint8_t*)&g_hid_report,
                                        USB_STD_REQ_DATA_DIR_IN,
                                        USBH_HID_Desc.wItemLength);
        break;

        case USBH_HID_WAIT_REQ_GET_REPORT_DESC:
            if (g_usbh_hid_cep_event)
            {
                mss_usbh_hid_err_code_t res = USBH_HID_NO_ERROR;
                g_usbh_hid_cep_event = 0u;
                g_hid_state = USBH_HID_REQ_SET_IDLE;
            }
        break;

        case USBH_HID_REQ_SET_IDLE:
            memset(std_req_buf, 0u, 8*(sizeof(uint8_t)));
            MSS_USBH_construct_get_descr_command(std_req_buf,
                                               USB_STD_REQ_DATA_DIR_OUT,
                                               USB_CLASS_REQUEST,
                                               USB_STD_REQ_RECIPIENT_INTERFACE,
                                               USBH_HID_SET_IDLE,
                                               0,
                                               0, /*stringID*/
                                               0);

            g_hid_state = USBH_HID_WAIT_REQ_SET_IDLE;
            MSS_USBH_start_control_xfr(std_req_buf,
                               (uint8_t*)&g_hid_report,
                                USB_STD_REQ_DATA_DIR_IN,
                                0);
        break;

        case USBH_HID_WAIT_REQ_SET_IDLE:
            if (g_usbh_hid_cep_event)
            {
                mss_usbh_hid_err_code_t res = USBH_HID_NO_ERROR;
                g_usbh_hid_cep_event = 0u;
                g_hid_state = USBH_HID_DEVICE_READY;
                if (0 != g_hidh_user_cb->hidh_tdev_ready)
                {
                    g_hidh_user_cb->hidh_tdev_ready();
                }
            }

        break;

        case USBH_HID_REQ_SET_PROTOCOL:

            memset(std_req_buf, 0u, 8*(sizeof(uint8_t)));
            std_req_buf[0] = 0x21;
            std_req_buf[1] = 0x0B;
            std_req_buf[2] = 0x00;
            std_req_buf[3] = 0x01;
            std_req_buf[4] = 0x00;
            std_req_buf[5] = 0x00;
            std_req_buf[6] = 0x00;
            std_req_buf[7] = 0x00;

            g_hid_state = USBH_HID_WAIT_REQ_SET_PROTOCOL;
            MSS_USBH_start_control_xfr(std_req_buf,
                               (uint8_t*)&g_hid_report,
                                USB_STD_REQ_DATA_DIR_IN,
                                0);
        break;

        case USBH_HID_WAIT_REQ_SET_PROTOCOL:
            if (g_usbh_hid_cep_event)
            {
                mss_usbh_hid_err_code_t res = USBH_HID_NO_ERROR;
                g_usbh_hid_cep_event = 0u;
                g_hid_state = USBH_HID_REQ_SET_PROTOCOL;
            }
        break;

        case USBH_HID_DEVICE_READY:
            USBH_HID_Handle();
        break;

        default:
        {
            ASSERT(0);  /*Reset recovery should be tried.*/
        }
        break;
    }
}


/*******************************************************************************
 * See mss_usb_host_hid.h for details of how to use this function.
 */
mss_usbh_hid_state_t
MSS_USBH_HID_get_state
(
    void
)
{
    return (g_hid_state);
}

/*******************************************************************************
 * Internal Functions
 ******************************************************************************/
/*
  This Call-back function is executed when the USBH-HID driver is allocated
  to the attached device by USBH driver.
 */
uint8_t
usbh_hid_allocate_cb
(
    uint8_t tdev_addr
)
{
    g_hid_tdev_addr = tdev_addr;
    g_usbh_hid_alloc_event = 1u;

    return (USB_SUCCESS);
}

/*
  This Call-back function is executed when the USBH-HID driver is released
  from the attached device by USBH driver.
 */
uint8_t
usbh_hid_release_cb
(
    uint8_t tdev_addr
)
{
    g_hid_state = USBH_HID_IDLE;
    memset(g_hid_conf_desc, 0u, sizeof(g_hid_conf_desc));
    g_tdev_in_ep.maxpktsz = 0u;
    g_tdev_in_ep.num = 0u;
    g_hid_tdev_addr = 0u;

    MSS_USB_CIF_dma_clr_ctrlreg(MSS_USB_DMA_CHANNEL2);

    MSS_USB_CIF_dma_clr_ctrlreg(MSS_USB_DMA_CHANNEL1);

    if (0 != g_hidh_user_cb->hidh_driver_released)
    {
        g_hidh_user_cb->hidh_driver_released();
    }

    return (USB_SUCCESS);
}

/*
  This Call-back function is executed when the control transfer initiated by this
  driver is complete.
 */
uint8_t
usbh_hid_cep_done_cb
(
    uint8_t tdev_addr,
    uint8_t status,
    uint32_t count
)
{
    g_usbh_hid_cep_event = status;

    return (USB_SUCCESS);
}

/*
  This Call-back function is executed when the data OUT transfer initiated by
  this driver is complete.
 */
uint8_t
usbh_hid_tx_complete_cb
(
    uint8_t tdev_addr,
    uint8_t status,
    uint32_t count
)
{
    g_usbh_hid_tx_event = 1;

    return (USB_SUCCESS);
}

/*
 This Call-back function is executed when the data IN transfer initiated by
 this driver is complete.
*/
uint8_t
usbh_hid_rx_cb
(
    uint8_t tdev_addr,
    uint8_t status,
    uint32_t count
)
{
    if (0 == status)
    {
        g_usbh_hid_rx_event = 1u;
    }
    else
    {
        if (MSS_USB_EP_NAK_TOUT & status)
        {
            /* Device responding with NAKs. Retry*/
            g_hid_state = USBH_HID_DEVICE_RETRY;
        }
        else
        {
            ASSERT(0);/* Handling any other error. Not yet supported */
        }
    }

    return (USB_SUCCESS);
}

/*
  This function validates the HID class descriptors.
 */
mss_usbh_hid_err_code_t
MSS_USBH_HID_validate_class_desc
(
    uint8_t* p_cd
)
{
    return (USBH_HID_NO_ERROR);
}

/*
  This function extract the endpoint information from the Config Descriptor.
 */
mss_usbh_hid_err_code_t
MSS_USBH_HID_extract_tdev_ep_desc
(
    void
)
{
    /* FirstEP Attributes Not INTR. bInterfaceclass For HID value should be 0x03 */
    if (!(g_hid_conf_desc[30u] & USB_EP_DESCR_ATTR_INTR))
    {
        return (USBH_HID_WRONG_DESCR);
    }

     /* TdevEP is IN type for HID class */
    if (g_hid_conf_desc[29u] & USB_STD_REQ_DATA_DIR_MASK)
    {
        g_tdev_in_ep.num = (g_hid_conf_desc[29u] & 0x7fu);
        g_tdev_in_ep.maxpktsz = (uint16_t)((g_hid_conf_desc[32u] << 8u) |
                                           (g_hid_conf_desc[31u]));

        g_tdev_in_ep.desclen = (uint16_t)((g_hid_conf_desc[26u] << 8u) |
                                           (g_hid_conf_desc[25u]));

    }
    else
    {
        return (USBH_HID_WRONG_DESCR);
    }

    return (USBH_HID_NO_ERROR);
}

/*
  This function read the report from the hid device.
 */
static void USBH_HID_Handle(void)
{
    static uint32_t wait_mili = 0;

    switch (HID_Machine_state)
    {
        case HID_IDLE:
            HID_Machine_state = HID_GET_DATA;

            MSS_USBH_configure_in_pipe(g_hid_tdev_addr,
            USBH_HID_INTR_RX_PIPE,
            g_tdev_in_ep.num,
            USBH_HID_INTR_RX_PIPE_FIFOADDR,
            USBH_HID_INTR_RX_PIPE_FIFOSZ,
            g_tdev_in_ep.maxpktsz,
            1,
            DMA_DISABLE,
            MSS_USB_DMA_CHANNEL2,
            MSS_USB_XFR_INTERRUPT,
            ADD_ZLP_TO_XFR,
            32768);

            HID_Machine_state = HID_READ_DATA;
        break;

        case HID_READ_DATA:
            MSS_USBH_read_in_pipe(g_hid_tdev_addr,
            USBH_HID_INTR_RX_PIPE,
            g_tdev_in_ep.num,
            g_tdev_in_ep.maxpktsz,
            (uint8_t*)&USBH_HID_RX_buffer,
            g_tdev_in_ep.maxpktsz);

            HID_Machine_state = HID_POLL;

            wait_mili = MSS_USBH_get_milis();

        break;

        case HID_POLL:
            if ((MSS_USBH_get_milis() - wait_mili) > HID_MIN_POLL)
            {
                HID_Machine_state = HID_WAIT_READ_DATA;
            }
        break;

        case HID_WAIT_READ_DATA:
            if (g_usbh_hid_rx_event)
            {
                g_usbh_hid_rx_event = 0;
                HID_Machine_state = HID_READ_DATA;
                if (0 != g_hidh_user_cb->hidh_decode)
                {
                    g_hidh_user_cb->hidh_decode((uint8_t*)&USBH_HID_RX_buffer);
                }
            }
        break;

        default:
        break;
    }

}

#endif /* MSS_USB_HOST_ENABLED */

#ifdef __cplusplus
}
#endif
