/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Microchip PolarFire SoC MSS USB Driver Stack
 *      USB Logical Layer (USB-LL)
 *          USBH driver
 *
 * USBH driver implementation:
 * This source file implements the common functionality of USB host mode
 *
 * SVN $Revision$
 * SVN $Date$
 */

#include "mss_assert.h"
#include <string.h>
#include <stdio.h>
#include "mss_usb_common_cif.h"
#include "mss_usb_common_reg_io.h"
#include "mss_usb_host.h"
#include "mss_usb_host_cif.h"
#include "mss_usb_host_reg_io.h"
#include "mss_usb_std_def.h"
#include "mss_plic.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_HOST_ENABLED

/***************************************************************************//**
  Constant values internally used by USBH driver.
 */
#define MSS_USB_1ms_DIVISION_VAL                        1000u /*In miliSeconds*/
#define MSS_USB_TARGET_RESET_DELAY                      20u   /*In miliSeconds*/
#define MSS_USB_TARGET_VRISE_DELAY                      20u   /*In miliSeconds*/
#define MSS_USB_TARGET_SUSPEND_DELAY                    20u   /*In miliSeconds*/

/***************************************************************************//**
  Types internally used by USBH driver.
 */
typedef enum enum_state {
    ENUM_IDLE,
    ENUM_GET_DEF_DEV_DESC,
    ENUM_WAIT_GET_DEF_DEV_DESC,
    ENUM_RESET_TDEV,
    ENUM_CLR_RESET_TDEV,
    ENUM_SET_TDEV_ADDR,
    ENUM_WAIT_SET_TDEV_ADDR,
    ENUM_GET_FUL_DEV_DESC,
    ENUM_WAIT_GET_FUL_DEV_DESC,
    ENUM_GET_DEF_CONFG_DESC,
    ENUM_WAIT_GET_DEF_CONFG_DESC,
    ENUM_GET_FUL_CONFG_DESC,
    ENUM_WAIT_GET_FUL_CONFG_DESC,
    ENUM_CHECK_TDEV_CLASS_SUPPORT,
    ENUM_SUCCESS,
    ENUM_ERROR,
    ENUM_PET_SET_CONFIG,
    ENUM_PET_WAIT_SET_CONFIG
} mss_usb_enum_state_t;

typedef enum host_state {
    HOST_IDLE,
    HOST_ROOT_TARGET_DETECTED,
    HOST_ROOT_TARGET_RESETING,
    HOST_ROOT_TARGET_ENUMERATING,
    HOST_ROOT_TARGET_ALLOCATE_CLASSD,
    HOST_ROOT_TARGET_SERVICING,
    HOST_ROOT_TARGET_ERROR
} mss_usbh_state_t;

typedef enum {
    TDEV_R,
    TDEV_RHP1,
    TDEV_RHP2
} tdev_id_t;

/***************************************************************************//**
  Data structures internally used by USBH driver.
 */
typedef struct {
    uint8_t alloc_state;
    mss_usbh_class_cb_t* class_handle;
} class_driver_info_t;

/* This structure type is used to read device and config descriptor from pen drive.
   At a time only one descriptor is operated upon.*/
typedef struct {
    union
    {
        uint8_t da[20];
        dev_desc_t ds;
    }dev_desc;
    union
    {
        /* Def_confg + First Interface Desc+AddressAlign */
         uint8_t ca[20];
         def_conf_desc_t cs;
    }conf_desc;
    int8_t desc_err_code;
} tdev_desc_t;

/******************************************************************************
  Global variable declarations for this file (USBH Driver).
 */

/* Flags to indicate the Callback Events */
uint8_t volatile gh_tdev_connect_event = 0u;
uint8_t volatile gh_tdev_discon_event = 0u;
uint8_t volatile gh_cep_cb_event = 0u;

/* Track the physically connected number of target devices */
tdev_id_t tdev_idx = TDEV_R;

/*
 * Information about the connected target devices
 * Index of this array is tdev_idx i.e. physical location on SF2 receptacle.
 */
static mss_usbh_tdev_info_t g_tdev[1];
int8_t g_tdev_error_code = 0;

/*
 * Holds the Target device descriptor information.
 * Currently we support only one device. Still an array is used so that future
 * support for multiple devices would be easy.
 * MPH:Number of connected target Devices.
 */
static tdev_desc_t g_tdev_desc[1];

/* General purpose milliseconds count */
volatile uint32_t ms = 0;

/* Track current state of the Host */
static mss_usbh_state_t g_host_state = HOST_IDLE;

/* Track current state of ENUMERATION FSM */
static mss_usb_enum_state_t g_enum_state = ENUM_IDLE;

/* MSS USB has 4 Transmit, 4 Receive and one control Endpoint */
mss_usb_ep_t gh_tx_ep[5];                               /*[0] ==> Control EP*/
mss_usb_ep_t gh_rx_ep[5];                               /*[0] ==> Empty*/

/* Use this at time of class allocation after device is enumerated
 * Currently we support only one device.Hence one Class allocation, still an
 * array is used so that future support for multiple devices would be easy.*/
/* MPH:Number of registered classes */
static class_driver_info_t g_rcd[1];

static volatile uint8_t g_cep_xfr_result = MSS_USB_EP_XFR_SUCCESS;

/* User application call-back handler */
static mss_usbh_user_cb_t* g_user_cb;

/* Apart from this driver, Class drivers can also do a CEP transfers as per
 * need. This variable is to differentiate between transfers started by this
 * driver and CRP transfers started by Class driver.
 */
static volatile uint8_t g_internal_cep_xfr = 0u;

extern volatile uint8_t txep_irq;

/******************************************************************************
  Private function declarations for this file (USBH driver).
 */
static void mss_usbh_enum_fsm(void);
static void mss_usbh_control_xfr_fsm(void);

/* Call-back functions used by Host CIF layer to communicate with this layer */
static void mss_usbh_ep_tx_complete_cb(uint8_t ep_num, uint8_t status);
static void mss_usbh_ep_rx_cb(uint8_t ep_num, uint8_t status);
static void mss_usbh_cep_cb(uint8_t status);
static void mss_usbh_sof_cb(uint32_t frame_number);
static void mss_usbh_connect_cb(mss_usb_device_speed_t target_speed,
                                mss_usb_vbus_level_t vbus_level);

static void mss_usbh_disconnect_cb(void);
static void mss_usbh_vbus_err_cb(mss_usb_vbus_level_t vbus_level);
static void mss_usbh_babble_err_cb(void);
static void mss_usbh_session_request_cb(void);
static void mss_usbh_dma_handler_cb(mss_usb_ep_num_t ep_num,
                                    mss_usb_dma_dir_t dma_dir, uint8_t status,
                                    uint32_t dma_addr_val);

static int8_t host_enum_check_class_support(tdev_id_t tid);
static int8_t validate_dev_desc(dev_desc_t* p_desc);
static int8_t validate_def_conf_desc(def_conf_desc_t* p_desc);
static void mss_usbh_fsm(void);
static void mss_usbh_reset_enum_fsm(void);
static void mss_usbh_start_enum_fsm(void);
static mss_usb_enum_state_t mss_usbh_get_enum_fsm_state(void);
static void mss_usbh_handle_discon_event(void);

/* Host call-back functions */
mss_usbh_cb_t g_mss_usbh_cb = {
    mss_usbh_ep_tx_complete_cb,
    mss_usbh_ep_rx_cb,
    mss_usbh_cep_cb,
    mss_usbh_sof_cb,
    mss_usbh_connect_cb,
    mss_usbh_disconnect_cb,
    mss_usbh_vbus_err_cb,
    mss_usbh_babble_err_cb,
    mss_usbh_session_request_cb,
    mss_usbh_dma_handler_cb
};

/*******************************************************************************
 * EXPORTED API Functions
 *******************************************************************************/
/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
void
MSS_USBH_init
(
     mss_usbh_user_cb_t* app_cb
)
{
    /*
     * By Default Prepare CEP for HS target.
     * Actual speed will be found in Enumeration FSM
     */
    g_tdev[TDEV_R].addr = USB_DEFAULT_TARGET_ADDR;
    g_tdev[TDEV_R].speed = MSS_USB_DEVICE_HS;
    g_tdev[TDEV_R].state = MSS_USB_NOT_ATTACHED_STATE;
    g_tdev[TDEV_R].hub_addr = 0u;
    g_tdev[TDEV_R].hub_port = 0u;
    g_tdev[TDEV_R].hub_mtt = 0u;
    g_tdev[TDEV_R].tdev_maxpktsz0 = CEP_MAX_PKT_SIZE;
    g_tdev[TDEV_R].class_handle = 0u;
    g_user_cb = app_cb;

    /* Initialize host core interface layer */
    MSS_USBH_CIF_init();
    MSS_USBH_configure_control_pipe(TDEV_R);
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
int8_t
MSS_USBH_register_class_driver
(
    void* class_handle
)
{
    volatile int8_t value = 0;

    g_rcd[0].alloc_state = 0u;
    if ((mss_usbh_class_cb_t*)0 != class_handle)
    {
        g_rcd[0].class_handle = (mss_usbh_class_cb_t*)class_handle;
        value = 0;
    }
    else
    {
        value = 1;
    }

    return(value);
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
int8_t
MSS_USBH_configure_control_pipe
(
    uint8_t target_addr
)
{
    tdev_id_t tid = (tdev_id_t)0u;
    mss_usb_ep_t* cep_ptr = &gh_tx_ep[MSS_USB_CEP];

    /*
     * Find the physical location of Target Device from Assigned address.
     * This will be used to call the correct call-back function assigned to this
     * address.
     * For multiple devices support using a HUB will be must. we intent to assign
     * non-zero device addresses starting with 1 (This will be HUB).This will be
     * done by this driver in mss_usbh_enum_fsm() function. This way array indexing
     * will be easier. But all this only when Multiple devices are supported.
     * Currently we assign 0x10 as the non-zero address for single connected device
     * and this will evaluate tdex_idx to 0.
     */
    tid = (tdev_id_t)(target_addr & 0x03u);
    cep_ptr->tdev_idx = tid;

    cep_ptr->num = MSS_USB_CEP;
    cep_ptr->fifo_size = 64u;
    cep_ptr->max_pkt_size = g_tdev[tid].tdev_maxpktsz0;
    cep_ptr->num_usb_pkt = 1u;
    cep_ptr->state = MSS_USB_CEP_IDLE;

    /* Control Endpoint Config is fixed. NakLimit = MaxVal.*/
    cep_ptr->interval = 32768u;

    MSS_USBH_CIF_cep_configure(cep_ptr);

    /* Type0:Default target speed */
    MSS_USBH_CIF_cep_set_type0_reg(g_tdev[tid].speed);
    MSS_USBH_CIF_tx_ep_set_target_func_addr(MSS_USB_CEP,target_addr);

    return (0);
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
int8_t
MSS_USBH_configure_out_pipe
(
    uint8_t target_addr,
    uint8_t outpipe_num,
    uint8_t target_ep,
    uint16_t fifo_addr,
    uint16_t fifo_size,
    uint16_t max_pkt_size,
    uint8_t num_usb_pkt,
    uint8_t dma_enable,
    mss_usb_dma_channel_t dma_channel,
    mss_usb_xfr_type_t xfr_type,
    uint32_t add_zlp,
    uint32_t interval
)
{
    mss_usb_ep_t* txep_ptr = &gh_tx_ep[outpipe_num];

    /* TODO: Error check for all the parameters. */
    tdev_id_t tid = (tdev_id_t)0u;

    tid = (tdev_id_t)(target_addr & 0x03u);
    txep_ptr->tdev_idx = tid;

    txep_ptr->num = (mss_usb_ep_num_t)outpipe_num;
    txep_ptr->dpb_enable = DPB_DISABLE;
    txep_ptr->fifo_size = fifo_size;
    txep_ptr->fifo_addr =fifo_addr;
    txep_ptr->dma_enable =dma_enable;
    txep_ptr->dma_channel =dma_channel;
    txep_ptr->max_pkt_size = max_pkt_size;
    txep_ptr->stall = 0u;
    txep_ptr->state = MSS_USB_EP_VALID;
    txep_ptr->xfr_type = xfr_type;
    txep_ptr->add_zlp = add_zlp;
    txep_ptr->num_usb_pkt = 1u;
    txep_ptr->buf_addr = 0u;

    txep_ptr->xfr_length = 0u;
    txep_ptr->xfr_count = 0u;
    txep_ptr->txn_length = 0u;
    txep_ptr->txn_count = 0u;
    txep_ptr->disable_ping = 1u;
    txep_ptr->req_pkt_n = 0u;
    txep_ptr->interval = interval;

    /* Configure MP Registers with Target Device informations */
    MSS_USBH_CIF_tx_ep_mp_configure(outpipe_num,
                                    target_ep,
                                    target_addr,
                                    g_tdev[tid].hub_addr,
                                    g_tdev[tid].hub_port,
                                    g_tdev[tid].hub_mtt,
                                    g_tdev[tid].speed,
                                    interval,
                                    xfr_type
                                    );

    MSS_USBH_CIF_tx_ep_configure(txep_ptr);

    return (0);
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
int8_t
MSS_USBH_configure_in_pipe
(
    uint8_t target_addr,
    uint8_t inpipe_num,
    uint8_t target_ep,
    uint16_t fifo_addr,
    uint16_t fifo_size,
    uint16_t max_pkt_size,
    uint8_t num_usb_pkt,
    uint8_t dma_enable,
    mss_usb_dma_channel_t dma_channel,
    mss_usb_xfr_type_t xfr_type,
    uint32_t add_zlp,
    uint32_t interval
)
{
    /* TODO: Error check for all the parameters.*/
    tdev_id_t tid = (tdev_id_t)0u;
    mss_usb_ep_t* rxep_ptr = &gh_rx_ep[inpipe_num];

    tid = (tdev_id_t)(target_addr & 0x03u);
    rxep_ptr->tdev_idx = tid;

    rxep_ptr->num = (mss_usb_ep_num_t)inpipe_num;
    rxep_ptr->dpb_enable = DPB_DISABLE;
    rxep_ptr->fifo_size = fifo_size;
    rxep_ptr->fifo_addr = fifo_addr;
    rxep_ptr->dma_enable = dma_enable;
    rxep_ptr->dma_channel = dma_channel;
    rxep_ptr->max_pkt_size = max_pkt_size;
    rxep_ptr->stall = 0u;
    rxep_ptr->state = MSS_USB_EP_VALID;
    rxep_ptr->xfr_type = xfr_type;
    rxep_ptr->add_zlp = add_zlp;
    rxep_ptr->num_usb_pkt = 1u;
    rxep_ptr->buf_addr = 0u;

    rxep_ptr->xfr_length = 0u;
    rxep_ptr->xfr_count = 0u;
    rxep_ptr->txn_length = 0u;
    rxep_ptr->txn_count = 0u;

    rxep_ptr->disable_ping = 1u;
    rxep_ptr->req_pkt_n = 0u;

    rxep_ptr->interval = interval;

    /* Configure MP Registers with Target Device informations */
    MSS_USBH_CIF_rx_ep_mp_configure(inpipe_num,
                                    target_ep,
                                    target_addr,
                                    g_tdev[tid].hub_addr,
                                    g_tdev[tid].hub_port,
                                    g_tdev[tid].hub_mtt,
                                    g_tdev[tid].speed,
                                    interval,
                                    xfr_type);

    MSS_USBH_CIF_rx_ep_configure(rxep_ptr);

    return (0);
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
int8_t
MSS_USBH_write_out_pipe
(
    uint8_t target_addr,
    uint8_t outpipe_num,
    uint8_t tdev_ep_num,
    uint16_t maxpktsz,
    uint8_t* buf,
    uint32_t length
)
{
    mss_usb_ep_t* txep_ptr = &gh_tx_ep[outpipe_num];

    /* TODO: Error check for all the parameters. */
    tdev_id_t tid = (tdev_id_t)0u;

    ASSERT(target_addr);
    ASSERT(outpipe_num);
    ASSERT(tdev_ep_num);
    ASSERT(maxpktsz);
    ASSERT(buf);

    if ((target_addr != 0) && (outpipe_num != 0) && (tdev_ep_num != 0) &&
       (maxpktsz != 0u) && (buf != NULL) )
    {
        tid = (tdev_id_t)(target_addr & 0x03u);
        txep_ptr->tdev_idx = tid;

        txep_ptr->xfr_length = length;

        if (length > maxpktsz)
        {
            txep_ptr->txn_length = maxpktsz;
        }
        else
        {
            txep_ptr->txn_length = length;
        }

        txep_ptr->xfr_count = 0u;
        txep_ptr->txn_count = 0u;

        txep_ptr->buf_addr = buf;
        txep_ptr->state = MSS_USB_EP_VALID;
        MSS_USB_CIF_ep_write_pkt(txep_ptr->num,
                                 txep_ptr->buf_addr,
                                 txep_ptr->dma_enable,
                                 txep_ptr->dma_channel,
                                 txep_ptr->xfr_type,
                                 txep_ptr->xfr_length,
                                 txep_ptr->txn_length);

        return (0);
    }
    else
    {
        return (1);
    }
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
int8_t
MSS_USBH_read_in_pipe
(
    uint8_t target_addr,
    uint8_t inpipe_num,
    uint8_t tdev_ep_num,
    uint16_t tdev_ep_maxpktsz,
    uint8_t* buf,
    uint32_t length
)
{
    mss_usb_ep_t* rxep_ptr = &gh_rx_ep[inpipe_num];

    /* TODO: Error check for all the parameters. */
    tdev_id_t tid = (tdev_id_t)0u;

    ASSERT(target_addr);
    ASSERT(inpipe_num);
    ASSERT(tdev_ep_num);
    ASSERT(tdev_ep_maxpktsz);
    ASSERT(buf);

    if ((target_addr != 0) && (inpipe_num != 0) && (tdev_ep_num != 0) &&
       (tdev_ep_maxpktsz != 0u) && (buf != NULL))
    {
        tid = (tdev_id_t)(target_addr & 0x03u);
        rxep_ptr->tdev_idx = tid;

        rxep_ptr->xfr_length = length;

        if (length > tdev_ep_maxpktsz)
        {
            rxep_ptr->txn_length = tdev_ep_maxpktsz;
        }
        else
        {
            rxep_ptr->txn_length = length;
        }

        rxep_ptr->xfr_count = 0u;
        rxep_ptr->txn_count = 0u;
        rxep_ptr->buf_addr = buf;

        if ((DMA_ENABLE == rxep_ptr->dma_enable) &&
                                       (MSS_USB_XFR_BULK == rxep_ptr->xfr_type))
        {
            if (rxep_ptr->xfr_length >= rxep_ptr->max_pkt_size)
            {
                MSS_USBH_CIF_rx_ep_set_reqpkt_count(rxep_ptr->num,
                                                     (rxep_ptr->xfr_length/
                                                      rxep_ptr->max_pkt_size));
            }
            MSS_USBH_CIF_rx_ep_set_autoreq(rxep_ptr->num);
        }

        if ((rxep_ptr->max_pkt_size <= rxep_ptr->fifo_size) &&
               (rxep_ptr->txn_length <= rxep_ptr->fifo_size))
        {
            MSS_USB_CIF_rx_ep_read_prepare(rxep_ptr->num,
                                           rxep_ptr->buf_addr,
                                           rxep_ptr->dma_enable,
                                           rxep_ptr->dma_channel,
                                           rxep_ptr->xfr_type,
                                           rxep_ptr->xfr_length);
            MSS_USBH_CIF_rx_ep_set_reqpkt((mss_usb_ep_num_t)inpipe_num);
        }
        return (0);
    }
    else
    {
        return (1);
    }
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
mss_usb_state_t
MSS_USBH_get_tdev_state
(
    uint8_t target_addr
)
{
    tdev_id_t tid = (tdev_id_t)0;

    if (target_addr)
    {
        tid = (tdev_id_t)(target_addr & 0x03u);
    }

    ASSERT(g_tdev[tid].addr == target_addr);

    return (g_tdev[tid].state);
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
void
MSS_USBH_task
(
    void
)
{
    mss_usbh_control_xfr_fsm();
    mss_usbh_enum_fsm();
    mss_usbh_fsm();
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
uint8_t
MSS_USBH_start_control_xfr
(
    uint8_t* cbuf_addr,
    uint8_t* dbuf_addr,
    uint8_t  data_dir,
    uint32_t data_len
    /*Add tdev_addr, speed and tdevmaxpktsz as the parameter*/
)
{
    mss_usb_ep_t* cep_ptr = &gh_tx_ep[MSS_USB_CEP];

    ASSERT(cbuf_addr != (uint8_t*)0);
    ASSERT(dbuf_addr != (uint8_t*)0);
    ASSERT(!(((ptrdiff_t)cbuf_addr) & 0x00000002U));
    ASSERT(!(((ptrdiff_t)dbuf_addr) & 0x00000002U));
    ASSERT((data_dir == USB_STD_REQ_DATA_DIR_IN) ||
           (data_dir == USB_STD_REQ_DATA_DIR_OUT));

    cep_ptr->buf_addr = dbuf_addr;
    cep_ptr->cep_cmd_addr = cbuf_addr;

    cep_ptr->xfr_length = data_len;

    /* Set the logical MaxPktSize of Host CEP to the MazPktsz of the associated
     * Target CEP
     */
    /* TODO: Copy from Parameter*/
    /* cep_ptr->max_pkt_size = cep_ptr->tdev_addr */

    cep_ptr->cep_data_dir = data_dir;
    cep_ptr->xfr_count = 0u;
    cep_ptr->txn_count = 0u;
    cep_ptr->xfr_type = MSS_USB_XFR_CONTROL;

    g_cep_xfr_result = 0u;

    MSS_USBH_CIF_load_tx_fifo(MSS_USB_CEP,
                              cbuf_addr,
                              USB_SETUP_PKT_LEN);

    cep_ptr->state = MSS_USB_CEP_SETUP;
    MSS_USBH_CIF_cep_set_setuppktrdy();

    return (0);
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
void
MSS_USBH_construct_get_descr_command
(
    uint8_t* buf,
    uint8_t xfr_dir,
    uint8_t req_type,
    uint8_t recip_type,
    uint8_t request,
    uint8_t desc_type,
    uint8_t strng_idx,
    uint16_t length
)
{
    /* bmRequestType */
    buf[0] = (xfr_dir| req_type| recip_type);

    /* bRequest */
    buf[1] = request;

    /* wValue-MSB = desc_type */
    buf[3] = desc_type;

    if ((request == USB_STD_REQ_GET_DESCRIPTOR) &&
        (desc_type == USB_STRING_DESCRIPTOR_TYPE))
    {
        /* We support only one LANGID 0x0409.
         * Refer USB standards for list of all supported LangIDs */
        buf[2] = strng_idx;                      /* wValue-LSB = string_idx */
        buf[4] = (uint8_t)0x09u;                 /* wIndex-LSB = LangID for string Desc */
        buf[5] = (uint8_t)0x04u;                 /* wIndex-MSB */
    }
    else
    {
        /*
         * wValue-LSB = Conf_idx, Field should be used only for Conf_desc or
         * String Desc.
         * Since we support only one configuration, we set it to zero for
         * Conf_desc. For all other descriptors this field must be zero
         */
        buf[2] = 0x00U;

        /* wIndex-LSB and MSB => other than String Desc, these values must be
         * zero */
        buf[4] = 0x00U;
        buf[5] = 0x00U;
    }

    /* wLength-LSB and MSB, Length of data to be received */
    buf[6] = (uint8_t)length;
    buf[7] = (uint8_t)(length >> 8u);
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
mss_usb_ep_state_t
MSS_USBH_get_cep_state
(
    void
)
{
    return (gh_tx_ep[MSS_USB_CEP].state);
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
void
MSS_USBH_1ms_tick
(
    void
)
{
    ms++;
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
uint32_t
MSS_USBH_get_milis
(
    void
)
{
    return (ms);
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
int8_t
MSS_USBH_get_std_dev_descr
(
    uint8_t* buffer
)
{
    uint8_t command_buf[USB_SETUP_PKT_LEN] = {0};

    g_internal_cep_xfr = 1u;

    /* 0x10 is the fixed non-zero device address we are using */
    MSS_USBH_configure_control_pipe(0x10u);

    memset(command_buf, 0u, USB_SETUP_PKT_LEN *(sizeof(uint8_t)));

    /* Read first 8 bytes of the Device descriptor */
    MSS_USBH_construct_get_descr_command(command_buf,
                                         USB_STD_REQ_DATA_DIR_IN,
                                         USB_STANDARD_REQUEST,
                                         USB_STD_REQ_RECIPIENT_DEVICE,
                                         USB_STD_REQ_GET_DESCRIPTOR,
                                         USB_DEVICE_DESCRIPTOR_TYPE,
                                         0u,
                                         8u);

    MSS_USBH_start_control_xfr(command_buf,
                               buffer,
                               USB_STD_REQ_DATA_DIR_IN,
                               8u);

    /* Wait for internal CEP transfer to complete */
    while ((0u == g_cep_xfr_result) && (g_internal_cep_xfr == 1u))
    {
        ;
    }

    if (MSS_USB_EP_XFR_SUCCESS == g_cep_xfr_result)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
void
MSS_USBH_test_mode
(
    uint8_t test_case
)
{
    switch (test_case)
    {
        case USB_TEST_MODE_SELECTOR_TEST_PACKET:
            MSS_USB_CIF_start_testpacket();
        break;

        case USB_TEST_MODE_SELECTOR_TEST_J:
            MSS_USB_CIF_start_testj();
        break;

        case USB_TEST_MODE_SELECTOR_TEST_K:
            MSS_USB_CIF_start_testk();
        break;

        case USB_TEST_MODE_SELECTOR_TEST_SE0NAK:
            MSS_USB_CIF_start_testse0nak();
        break;

        case USB_TEST_MODE_SELECTOR_TEST_FORCE_ENA:
            MSS_USB_CIF_start_forcehost_ena();
        break;

        default:
            /* Empty default */
        break;
    }
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
void
MSS_USBH_suspend
(
    void
)
{
    /* suspendM enabled. RemoteWakeup not supported. */
    MSS_USBH_CIF_bus_suspend(0u);
}

/******************************************************************************
 * See mss_usb_host.h for details of how to use this function.
 */
void
MSS_USBH_resume
(
    void
)
{
    static volatile uint32_t resume_milis = 0u;

    resume_milis = MSS_USBH_get_milis();

    MSS_USBH_CIF_clr_suspend_bus();

    MSS_USBH_CIF_assert_bus_resume();

    /* This delay should be at lease 20ms */
    while ((MSS_USBH_get_milis() - resume_milis) <= 40u)
    {
        asm volatile(" ");
    }

    MSS_USBH_CIF_clr_bus_resume();
}

void
MSS_USBH_abort_in_pipe
(
    mss_usb_ep_num_t inpipe_num
)
{
    MSS_USBH_CIF_rx_ep_clr_reqpkt(inpipe_num);
    MSS_USBH_CIF_rx_ep_clr_naktimeout_err(inpipe_num);
    MSS_USBH_CIF_rx_ep_clr_rxstall_err(inpipe_num);
    MSS_USBH_CIF_rx_ep_clr_retry_err(inpipe_num);
    if (MSS_USBH_CIF_rx_ep_is_rxpktrdy(inpipe_num))
    {
        MSS_USBH_CIF_rx_ep_flush_fifo_reg(inpipe_num);
        MSS_USB_CIF_rx_ep_clr_rxpktrdy(inpipe_num);
    }
}

void
MSS_USBH_abort_out_pipe
(
    mss_usb_ep_num_t outpipe_num
)
{
    gh_tx_ep[outpipe_num].state = MSS_USB_EP_ABORTED;

    MSS_USBH_CIF_tx_ep_clr_autoset(outpipe_num);
    MSS_USBH_CIF_tx_ep_flush_fifo_reg(outpipe_num);
    MSS_USB_CIF_tx_ep_disable_dma(outpipe_num);
    MSS_USBH_CIF_tx_ep_clr_retry_err(outpipe_num);
    MSS_USBH_CIF_tx_ep_clr_rxstall_err(outpipe_num);
    MSS_USBH_CIF_tx_ep_clr_naktimeout_err(outpipe_num);
}

/*******************************************************************************
 * Internal Functions
 *******************************************************************************/
/*
 * This function is the main FSM of the USB Host. It handles connect/disconnect
 * events, initiate enumeration FSM when device is detected and handles allocation
 * of class driver to the device.
 */
static void
mss_usbh_fsm
(
    void
)
{
    static uint32_t reset_milis = 0u;
    mss_usb_enum_state_t enum_st;

    if (1u == gh_tdev_discon_event)
    {
        mss_usbh_handle_discon_event();
        gh_tdev_discon_event = 0u;
        g_host_state = HOST_IDLE;
    }
    switch (g_host_state)
    {
        case HOST_IDLE:
            /*
             * Repeatedly enable session bit so that MUSB sample Idig pin to
             * detect device connection
             */
            MSS_USB_CIF_start_session();
            if (gh_tdev_connect_event)
            {
                gh_tdev_connect_event = 0u;
                g_tdev[tdev_idx].state = MSS_USB_ATTACHED_STATE;
                reset_milis = MSS_USBH_get_milis();
                g_host_state = HOST_ROOT_TARGET_DETECTED;
            }
        break;

        case HOST_ROOT_TARGET_DETECTED:
            /* Wait for at least 100ms for target power de-bounce as per spec */
            if ((MSS_USBH_get_milis() - reset_milis) >= 100u)
            {
                MSS_USB_CIF_enable_usbirq(VBUS_ERROR_IRQ_MASK);
                g_tdev[TDEV_R].state = MSS_USB_POWERED_STATE;
                reset_milis = MSS_USBH_get_milis();
                MSS_USBH_CIF_assert_bus_reset();           /*Reset Root device*/
                g_host_state = HOST_ROOT_TARGET_RESETING;
            }
        break;

        case HOST_ROOT_TARGET_RESETING:
            /* Keep target device in reset, at least for 20ms as per spec */
            if ((MSS_USBH_get_milis() - reset_milis) == 20u)
            {
                MSS_USBH_CIF_clr_bus_reset();
            }
            else if ((MSS_USBH_get_milis() - reset_milis) >= 40u)
            {
                /* If target was operating at FS, check if can work with HS as well */
                if ((g_tdev[TDEV_R].speed == MSS_USB_DEVICE_FS) &&
                    (MSS_USBH_CIF_is_hs_mode()))
                {
                    g_tdev[TDEV_R].speed = MSS_USB_DEVICE_HS;
                }
                if (0 != g_user_cb->usbh_tdev_attached)
                {
                    g_user_cb->usbh_tdev_attached(g_tdev[tdev_idx].speed);
                }

                /* Configure CEP for the detected speed */
                MSS_USBH_configure_control_pipe(TDEV_R);

                g_tdev[TDEV_R].state = MSS_USB_DEFAULT_STATE;
                g_host_state = HOST_ROOT_TARGET_ENUMERATING;
            }
        break;

        case HOST_ROOT_TARGET_ENUMERATING:
            enum_st = mss_usbh_get_enum_fsm_state();
            if (ENUM_IDLE == enum_st)
            {
                mss_usbh_start_enum_fsm();
            }
            else if (ENUM_SUCCESS == enum_st)
            {
                g_tdev[TDEV_R].state = MSS_USB_ADDRESS_STATE;

                mss_usbh_reset_enum_fsm();
                g_host_state = HOST_ROOT_TARGET_ALLOCATE_CLASSD;
            }
            else if (ENUM_ERROR == enum_st)
            {
                mss_usbh_reset_enum_fsm();
                g_tdev[TDEV_R].state = MSS_USB_ATTACHED_STATE;
                g_host_state = HOST_ROOT_TARGET_ERROR;
            }
            else
            {
                /* EnumFSM in progress, Do nothing */
            }
        break;

        case HOST_ROOT_TARGET_ALLOCATE_CLASSD:
            g_rcd[TDEV_R].class_handle->usbh_class_allocate(g_tdev[TDEV_R].addr);
            g_host_state = HOST_ROOT_TARGET_SERVICING;

            if (0 != g_user_cb->usbh_tdev_class_driver_assigned)
            {
                g_user_cb->usbh_tdev_class_driver_assigned();
            }

        break;

        case HOST_ROOT_TARGET_SERVICING:
            /* Class driver FSM is active now */
        break;

        case HOST_ROOT_TARGET_ERROR:
            if (0 != g_user_cb->usbh_tdev_not_supported)
            {
                g_user_cb->usbh_tdev_not_supported(g_tdev_error_code);
            }
            g_host_state = HOST_IDLE;

            /*
             * Clear Enum-FSM
             * Clear Target_info for all targets.
             * Clear EP0 config
             * Give Visual Err Indication and remain in this state,
             * till Disconnect Interrupt occurs or USER wants to give a retry
             * command.
             */
        break;

        default:
            /* Empty default */
        break;
    }
}

/*
 * This function handles the enumeration process which includes, device reset,
 * speed detection, and standard USB request for retrieving Device and Config
 * Descriptor from the device.
*/
static void
mss_usbh_enum_fsm
(
    void
)
{
    uint8_t command_buf[USB_SETUP_PKT_LEN] = {0};
    static uint32_t reset_milis = 0u;
    int8_t cd_idx = 0x04;
    uint8_t dummy[8];
    /*
     * We do not do any assert checks here.
     * Before Calling this FSM, Calling function should make sure that This FSM
     * can be executed
     */
    switch(g_enum_state)
    {
        case ENUM_IDLE:
            /* Free running Enum FSM hits here when doing nothing */
        break;

        case ENUM_GET_DEF_DEV_DESC:
            MSS_USBH_construct_get_descr_command(command_buf,
                                                 USB_STD_REQ_DATA_DIR_IN,
                                                 USB_STANDARD_REQUEST,
                                                 USB_STD_REQ_RECIPIENT_DEVICE,
                                                 USB_STD_REQ_GET_DESCRIPTOR,
                                                 USB_DEVICE_DESCRIPTOR_TYPE,
                                                 0u,
                                                 USB_SETUP_PKT_LEN);

            g_enum_state = ENUM_WAIT_GET_DEF_DEV_DESC;

            MSS_USBH_start_control_xfr(command_buf,
                                      (uint8_t*)&g_tdev_desc[tdev_idx].dev_desc,
                                      USB_STD_REQ_DATA_DIR_IN,
                                      USB_SETUP_PKT_LEN);
        break;

        case ENUM_WAIT_GET_DEF_DEV_DESC:
            if (g_cep_xfr_result)
            {
                if (MSS_USB_EP_XFR_SUCCESS == g_cep_xfr_result)
                {
                    g_tdev[TDEV_R].tdev_maxpktsz0 =
                                g_tdev_desc[TDEV_R].dev_desc.ds.bMaxPacketSize0;

                    g_enum_state = ENUM_RESET_TDEV;
                }
                else
                {
                    g_enum_state = ENUM_ERROR;
                }
            }
        break;

        case ENUM_RESET_TDEV:
            reset_milis = MSS_USBH_get_milis();
            MSS_USBH_CIF_assert_bus_reset();
            g_enum_state = ENUM_CLR_RESET_TDEV;
        break;

        case ENUM_CLR_RESET_TDEV:
            if ((MSS_USBH_get_milis() - reset_milis) == 20u)
            {
                MSS_USBH_CIF_clr_bus_reset();
            }
            else if ((MSS_USBH_get_milis() - reset_milis) >= 40u)
            {
                /* Wait for 20ms to let the Target settle down */
                g_enum_state = ENUM_SET_TDEV_ADDR;
            }
        break;

        case ENUM_SET_TDEV_ADDR:
            /* Configure CEP Since Maxpkt0 might have changed. */
            MSS_USBH_configure_control_pipe(TDEV_R);

            memset(command_buf, 0, USB_SETUP_PKT_LEN*(sizeof(uint8_t)));
            command_buf[1] = USB_STD_REQ_SET_ADDRESS;
            command_buf[2] = 0x10U;                         /*New non-zero ADDR*/
            reset_milis = MSS_USBH_get_milis();
            g_enum_state = ENUM_WAIT_SET_TDEV_ADDR;
            MSS_USBH_start_control_xfr(command_buf,
                                      command_buf,     /*Dummy Buf for zld req*/
                                      USB_STD_REQ_DATA_DIR_OUT,
                                      0u);
        break;

        case ENUM_WAIT_SET_TDEV_ADDR:
            if (g_cep_xfr_result)
            {
                if (MSS_USB_EP_XFR_SUCCESS == g_cep_xfr_result)
                {
                    int8_t res;

                    /* SetAddrDelay at least 2ms */
                    if ((MSS_USBH_get_milis() - reset_milis) >= 5u)
                    {
                        g_tdev[TDEV_R].addr = 0x10U;         /* New non-zero ADDR */
                        MSS_USBH_CIF_tx_ep_set_target_func_addr(gh_tx_ep[MSS_USB_CEP].num,
                                                                g_tdev[TDEV_R].addr);

                        /* Validate DevDescriptor. Take exception for FS LS
                         * devices */
                        res = validate_dev_desc((dev_desc_t*)&g_tdev_desc[tdev_idx].dev_desc);
                        g_tdev_desc[tdev_idx].desc_err_code = res;
                        if (0u == res)
                        {
                            g_enum_state = ENUM_GET_FUL_DEV_DESC;
                        }
                        else
                        {
                            g_enum_state = ENUM_ERROR;
                            g_tdev_error_code = res;
                        }
                    }
                }
                else
                {
                    g_tdev[TDEV_R].addr = 0x00u;
                    g_enum_state = ENUM_ERROR;
                }
            }
        break;

        case ENUM_GET_FUL_DEV_DESC:
            MSS_USBH_construct_get_descr_command(command_buf,
                                                 USB_STD_REQ_DATA_DIR_IN,
                                                 USB_STANDARD_REQUEST,
                                                 USB_STD_REQ_RECIPIENT_DEVICE,
                                                 USB_STD_REQ_GET_DESCRIPTOR,
                                                 USB_DEVICE_DESCRIPTOR_TYPE,
                                                 0u,
                                                 USB_STD_DEVICE_DESCR_LEN);

            g_enum_state = ENUM_WAIT_GET_FUL_DEV_DESC;

            MSS_USBH_start_control_xfr(command_buf,
                                       (uint8_t*)&g_tdev_desc[tdev_idx].dev_desc,
                                       USB_STD_REQ_DATA_DIR_IN,
                                       USB_STD_DEVICE_DESCR_LEN);
        break;

        case ENUM_WAIT_GET_FUL_DEV_DESC:
            if (g_cep_xfr_result)
            {
                if (MSS_USB_EP_XFR_SUCCESS == g_cep_xfr_result)
                {
                    /* Support for PET device with VID = 0x1A0A PID = PID=0x0200*/
                    if ((g_tdev_desc[tdev_idx].dev_desc.ds.idVendor == 0x1A0AU) &&
                        (g_tdev_desc[tdev_idx].dev_desc.ds.idProduct == 0x0200U))
                    {
                        g_enum_state = ENUM_PET_SET_CONFIG;
                    }
                    else
                    {
                        int8_t res;

                        /* Validate DevDescriptor */
                        res = validate_dev_desc((dev_desc_t*)&g_tdev_desc[tdev_idx].dev_desc);
                        g_tdev_desc[tdev_idx].desc_err_code = res;

                        if (0u == res)
                        {
                            g_enum_state = ENUM_GET_DEF_CONFG_DESC;
                        }
                        else
                        {
                            g_enum_state = ENUM_ERROR;
                            g_tdev_error_code = res;
                        }
                    }
                }
                else
                {
                    g_enum_state = ENUM_ERROR;
                }
            }
        break;

        case ENUM_GET_DEF_CONFG_DESC:
            memset(command_buf, 0u, USB_SETUP_PKT_LEN*(sizeof(uint8_t)));
            MSS_USBH_construct_get_descr_command(command_buf,
                                                 USB_STD_REQ_DATA_DIR_IN,
                                                 USB_STANDARD_REQUEST,
                                                 USB_STD_REQ_RECIPIENT_DEVICE,
                                                 USB_STD_REQ_GET_DESCRIPTOR,
                                                 USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                                 0x0U, /*stringID*/
                                                 USB_STD_CONFIG_DESCR_LEN);

            g_enum_state = ENUM_WAIT_GET_DEF_CONFG_DESC;

            MSS_USBH_start_control_xfr(command_buf,
                                      (uint8_t*)&g_tdev_desc[tdev_idx].conf_desc,
                                      USB_STD_REQ_DATA_DIR_IN,
                                      USB_STD_CONFIG_DESCR_LEN);
        break;

        case ENUM_WAIT_GET_DEF_CONFG_DESC:
            if (g_cep_xfr_result)
            {
                if (MSS_USB_EP_XFR_SUCCESS == g_cep_xfr_result)
                {
                    int8_t res;
                    res = validate_def_conf_desc((def_conf_desc_t*)&g_tdev_desc[tdev_idx].conf_desc);
                    g_tdev_desc[tdev_idx].desc_err_code = res;
                    if (0u == res)
                    {
                        g_enum_state = ENUM_GET_FUL_CONFG_DESC;
                    }
                    else
                    {
                        g_enum_state = ENUM_ERROR;
                        g_tdev_error_code = res;
                    }
                }
                else
                {
                    g_enum_state = ENUM_ERROR;
                }
            }
        break;

        case ENUM_GET_FUL_CONFG_DESC:
            /* We already read the Standard Config desc of size 9. Now read
             * additional MSC class specific descriptor data. reading first
             * 18 bytes is sufficient for supporting MSC class */
            memset(command_buf, 0u, USB_SETUP_PKT_LEN*(sizeof(uint8_t)));
            MSS_USBH_construct_get_descr_command(command_buf,
                                                 USB_STD_REQ_DATA_DIR_IN,
                                                 USB_STANDARD_REQUEST,
                                                 USB_STD_REQ_RECIPIENT_DEVICE,
                                                 USB_STD_REQ_GET_DESCRIPTOR,
                                                 USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                                 0u, /*stringID*/
                                                 18u);

            g_enum_state = ENUM_WAIT_GET_FUL_CONFG_DESC;

            MSS_USBH_start_control_xfr(command_buf,
                                      (uint8_t*)&g_tdev_desc[tdev_idx].conf_desc,
                                      USB_STD_REQ_DATA_DIR_IN,
                                      18u);

        break;

        case ENUM_WAIT_GET_FUL_CONFG_DESC:
            if (g_cep_xfr_result)
            {
                if (MSS_USB_EP_XFR_SUCCESS == g_cep_xfr_result)
                {
                    g_enum_state = ENUM_CHECK_TDEV_CLASS_SUPPORT;
                }
                else
                {
                    g_enum_state = ENUM_ERROR;
                }
            }
        break;

        case ENUM_CHECK_TDEV_CLASS_SUPPORT:
            cd_idx =  host_enum_check_class_support(TDEV_R);

            if (cd_idx < (int8_t)0u)
            {
               g_enum_state = ENUM_ERROR;
               g_tdev_error_code = cd_idx;
            }
            /* Max No of classDriver is 3 */
            else if (cd_idx < 3u)
            {
                g_rcd[cd_idx].alloc_state = 1u;
                g_tdev[TDEV_R].class_handle = g_rcd[cd_idx].class_handle;
                g_enum_state = ENUM_SUCCESS;
            }
        break;

        case ENUM_PET_SET_CONFIG:
            memset(command_buf, 0u, USB_SETUP_PKT_LEN*(sizeof(uint8_t)));
            command_buf[1] = USB_STD_REQ_SET_CONFIG;
            command_buf[2] = 0x01u;                   /* ConfigNum for PET Device */
            g_enum_state = ENUM_PET_WAIT_SET_CONFIG;

            MSS_USBH_start_control_xfr(command_buf,
                                       (uint8_t*)dummy,
                                       USB_STD_REQ_DATA_DIR_IN,
                                       0u);
        break;

        case ENUM_PET_WAIT_SET_CONFIG:
            if (g_cep_xfr_result)
            {
                if (MSS_USB_EP_XFR_SUCCESS == g_cep_xfr_result)
                {
                    g_enum_state = ENUM_CHECK_TDEV_CLASS_SUPPORT;
                }
                else
                {
                    g_enum_state = ENUM_ERROR;
                }
            }
        break;

        case ENUM_SUCCESS:
        break;

        case ENUM_ERROR:
        default:
            ASSERT(0);          /* Should never happen */
        break;
    }
}

/*
 * This function is the FSM for the control transfer. Once the control transfer
 * is initiated, this function will monitor it's progress in all the stages and
 * record the status at the end.
*/
static void
mss_usbh_control_xfr_fsm
(
    void
)
{
    volatile mss_usb_ep_state_t cep_event_st = MSS_USB_EP_VALID;
    uint32_t rem_length = 0u;

    mss_usb_ep_t* cep_ptr = &gh_tx_ep[MSS_USB_CEP];

    MSS_USB_CIF_cep_disable_irq();

    /* gh_cep_cb_event is critical */
    cep_event_st = (mss_usb_ep_state_t)gh_cep_cb_event;
    gh_cep_cb_event = 0u;
    MSS_USB_CIF_cep_enable_irq();
    MSS_USB_CIF_set_index_reg(MSS_USB_CEP);

    switch (cep_ptr->state)
    {
        case MSS_USB_CEP_IDLE:
            /* do nothing. Free running CEP-FSM hits here when doing no transfers */
        break;

        case MSS_USB_CEP_SETUP:
            if (cep_event_st)
            {
                if (MSS_USB_EP_TXN_SUCCESS == cep_event_st)
                {
                    if (0u == cep_ptr->xfr_length)
                    {
                        /* zdl-request is sent. Get the status now */
                        cep_ptr->state = MSS_USB_CEP_STATUS_AFTER_OUT;
                        MSS_USBH_CIF_cep_set_statuspktrdy_after_out();
                    }
                    else
                    {
                        rem_length = cep_ptr->xfr_length - cep_ptr->xfr_count;

                        if (rem_length > cep_ptr->max_pkt_size)
                        {
                            cep_ptr->txn_length = cep_ptr->max_pkt_size;
                        }
                        else
                        {
                            cep_ptr->txn_length = rem_length;
                        }

                        if (USB_STD_REQ_DATA_DIR_IN == cep_ptr->cep_data_dir)
                        {
                            cep_ptr->state = MSS_USB_CEP_RX;
                            MSS_USBH_CIF_cep_set_request_in_pkt();  /* One Packet at a time */
                        }
                        else if (USB_STD_REQ_DATA_DIR_OUT == cep_ptr->cep_data_dir)
                        {
                            MSS_USBH_CIF_cep_write_pkt(cep_ptr);
                            cep_ptr->state = MSS_USB_CEP_TX;
                            MSS_USBH_CIF_cep_set_txpktrdy();
                        }
                        else
                        {
                            ASSERT(0);                  /* DataDir not valid */
                        }
                    }
                }
                else
                {
                    cep_ptr->state = cep_event_st;
                }
            }
        break;

        case MSS_USB_CEP_RX:

            if (MSS_USB_EP_TXN_SUCCESS == cep_event_st)
            {
                MSS_USBH_CIF_cep_read_pkt(cep_ptr);

                if (cep_ptr->xfr_count == cep_ptr->xfr_length)
                {
                    cep_ptr->state = MSS_USB_CEP_STATUS_AFTER_IN;
                    MSS_USBH_CIF_cep_set_statuspktrdy_after_in();
                }
                else
                {
                    rem_length = cep_ptr->xfr_length - cep_ptr->xfr_count;

                    if (rem_length > cep_ptr->max_pkt_size)
                    {
                        cep_ptr->txn_length = cep_ptr->max_pkt_size;
                    }
                    else
                    {
                        cep_ptr->txn_length = rem_length;
                    }

                    cep_ptr->state = MSS_USB_CEP_RX;
                    MSS_USBH_CIF_cep_set_request_in_pkt();  /*One Packet at a time*/
                }
            }
            else
            {
                cep_ptr->state = cep_event_st;
                ASSERT(0);/* Flush fifo, if RxpktRdy is set.MUSB:21.2.2 */
            }
        break;

        case MSS_USB_CEP_TX:
            if (MSS_USB_EP_TXN_SUCCESS == cep_event_st)
            {
                if (cep_ptr->xfr_count == cep_ptr->xfr_length)
                {
                    cep_ptr->state = MSS_USB_CEP_STATUS_AFTER_OUT;
                    MSS_USBH_CIF_cep_set_statuspktrdy_after_out();
                }
                else
                {
                    rem_length = cep_ptr->xfr_length - cep_ptr->xfr_count;

                    if (rem_length > cep_ptr->max_pkt_size)
                    {
                        cep_ptr->txn_length = cep_ptr->max_pkt_size;
                    }
                    else
                    {
                        cep_ptr->txn_length = rem_length;
                    }

                    MSS_USBH_CIF_cep_write_pkt(cep_ptr);
                }
            }
            else
            {
                cep_ptr->state = cep_event_st;
                ASSERT(0); /* Error response received in INTR */
            }
            /* TODO: Check if more data needs to Transmitted */
            /* Initiate IN status phase. */
        break;

        case MSS_USB_CEP_STATUS_AFTER_IN:
            if (MSS_USB_EP_TXN_SUCCESS == cep_event_st)
            {
                uint8_t this_tdev = cep_ptr->tdev_idx;
                cep_ptr->state = MSS_USB_EP_XFR_SUCCESS;

                if ((NULL == g_tdev[(cep_ptr->tdev_idx)].class_handle) ||
                    (1u == g_internal_cep_xfr))
                {
                    g_cep_xfr_result = MSS_USB_EP_XFR_SUCCESS;
                    g_internal_cep_xfr = 0u;
                }
                else
                {
                    if (0 != g_tdev[this_tdev].class_handle->usbh_class_cep_xfr_done)
                    {
                        g_tdev[this_tdev].class_handle->
                            usbh_class_cep_xfr_done(g_tdev[this_tdev].addr,
                                                    cep_ptr->state,
                                                    cep_ptr->xfr_count);
                    }
                    else
                    {
                        ASSERT(0);/* invalid pointer */
                    }
                }

                cep_ptr->state = MSS_USB_CEP_IDLE;
            }
            else
            {
                cep_ptr->state = cep_event_st;
                ASSERT(0);
            }
        break;

        case MSS_USB_CEP_STATUS_AFTER_OUT:
            if (MSS_USB_EP_TXN_SUCCESS == cep_event_st)
            {
                uint8_t this_tdev = cep_ptr->tdev_idx;
                MSS_USBH_CIF_cep_clr_statusRxpktrdy();
                cep_ptr->state = MSS_USB_EP_XFR_SUCCESS;

                if ((0 == g_tdev[(cep_ptr->tdev_idx)].class_handle) ||
                    (1u == g_internal_cep_xfr))
                {
                    g_cep_xfr_result = MSS_USB_EP_XFR_SUCCESS;
                }
                else
                {
                    if (0 != g_tdev[this_tdev].class_handle->usbh_class_cep_xfr_done)
                    {
                        g_tdev[this_tdev].class_handle->
                            usbh_class_cep_xfr_done(g_tdev[this_tdev].addr,
                                                    cep_ptr->state,
                                                    cep_ptr->xfr_count);
                    }
                    else
                    {
                        ASSERT(0); /* invalid pointer */
                    }
                }

                cep_ptr->state = MSS_USB_CEP_IDLE;
            }
            else
            {
                cep_ptr->state = cep_event_st;
                ASSERT(0);
            }
        break;

        case MSS_USB_EP_NAK_TOUT:
        case MSS_USB_EP_NO_RESPONSE:
        case MSS_USB_EP_STALL_RCVD:
        {
            uint8_t this_tdev = cep_ptr->tdev_idx;
            if ((0 == g_tdev[(cep_ptr->tdev_idx)].class_handle) ||
                (1u == g_internal_cep_xfr))
            {
                g_cep_xfr_result = cep_ptr->state;
                g_internal_cep_xfr = 0u;
            }
            else
            {
                if (0 != g_tdev[this_tdev].class_handle->usbh_class_cep_xfr_done)
                {
                    g_tdev[this_tdev].class_handle->
                        usbh_class_cep_xfr_done(g_tdev[this_tdev].addr,
                                                cep_ptr->state,
                                                cep_ptr->xfr_count);
                }
                else
                {
                    ASSERT(0); /* invalid pointer */
                }

                cep_ptr->state = MSS_USB_CEP_IDLE;
            }
        }
        break;

        default:
        {
            ASSERT(0); /* Invalid CEP state */
            break;
        }
    }
}

/*
 * This function is used to start the enumeration FSM
 */
static void
mss_usbh_start_enum_fsm
(
    void
)
{
    g_enum_state = ENUM_GET_DEF_DEV_DESC;
}

/*
 * This function is used to bring the enum FSM to it's initial IDLE state.
 */
static void
mss_usbh_reset_enum_fsm
(
    void
)
{
    g_enum_state = ENUM_IDLE;
}

/*
 * This function is used to find the current status of the enumeration FSM. The
 * state information is used to know if the enumeration was successful or not.
 */
static mss_usb_enum_state_t
mss_usbh_get_enum_fsm_state
(
    void
)
{
    return (g_enum_state);
}

/*
 * This Call-back function is executed on completion of current OUT transfer.
 */
static void
mss_usbh_ep_tx_complete_cb
(
    uint8_t ep_num,
    uint8_t status
)
{
    uint8_t this_tdev = 0u;
    uint8_t transfer_complete = 0u;

    mss_usb_ep_t* txep_ptr = &gh_tx_ep[ep_num];

    /* retrieve tdev_idx which transmitted data. */
    this_tdev = gh_tx_ep[ep_num].tdev_idx;

    if (MSS_USB_EP_ABORTED != txep_ptr->state)
    {
        txep_ptr->txn_count = txep_ptr->txn_length;
        txep_ptr->xfr_count += txep_ptr->txn_length;

        if (0u == status)
        {
            if (DMA_ENABLE == txep_ptr->dma_enable)
            {
                txep_ptr->state = MSS_USB_EP_XFR_SUCCESS;

                txep_ptr->xfr_count = MSS_USB_CIF_dma_read_addr(txep_ptr->dma_channel) -
                                              ((ptrdiff_t)txep_ptr->buf_addr);
                ASSERT(txep_ptr->xfr_count == txep_ptr->xfr_length);
                transfer_complete = 1u;
            }
            else
            {
                if (MSS_USB_XFR_BULK == txep_ptr->xfr_type)
                {
                    if (txep_ptr->xfr_count < txep_ptr->xfr_length)
                    {
                        transfer_complete = 0u;
                    }
                    else if (txep_ptr->xfr_count == txep_ptr->xfr_length)
                    {
                        if (ADD_ZLP_TO_XFR == txep_ptr->add_zlp)
                        {
                            if (0u == txep_ptr->txn_count)
                            {
                                transfer_complete = 1u;
                            }
                            else
                            {
                                if (txep_ptr->txn_count == txep_ptr->max_pkt_size)
                                {
                                    transfer_complete = 0u;
                                }
                                else if (txep_ptr->txn_count < txep_ptr->max_pkt_size)
                                {
                                    transfer_complete = 1u;
                                }
                            }
                        }
                        else        /* no zlp */
                        {
                            transfer_complete = 1u;
                        }
                    }
                    else
                    {
                        /* If xfr_count is more than xfr_lenght then something
                         * has seriously gone bad.*/
                        ASSERT(0);
                    }
                }
            }
        }
        else
        {
            txep_ptr->state = (mss_usb_ep_state_t)status;
            transfer_complete = 1u;
        }
        if (1u == transfer_complete)
        {
            if (0 != g_tdev[this_tdev].class_handle->usbh_class_tx_done)
            {
                /* call-back class driver */
                if (0u == txep_ptr->xfr_count)
                {
                    ASSERT(0);
                }
                else
                {
                    g_tdev[this_tdev].class_handle->usbh_class_tx_done(g_tdev[this_tdev].addr,
                                                                       status,
                                                                       txep_ptr->xfr_count);

                }
            }
            else
            {
                ASSERT(0); /* Invalid function pointer */
            }
        }
        else
        {
            txep_ptr->txn_count = 0u;/* reset txn_count for next txn */
            ASSERT(txep_ptr->xfr_length >= txep_ptr->xfr_count);

            if ((txep_ptr->xfr_length - txep_ptr->xfr_count) >= txep_ptr->max_pkt_size)
            {
                txep_ptr->txn_length = txep_ptr->max_pkt_size;
            }
            else
            {
                txep_ptr->txn_length = (txep_ptr->xfr_length - txep_ptr->xfr_count);
            }

            while (MSS_USB_CIF_is_txepfifo_notempty(txep_ptr->num))
            {
                ;
            }

            if ((txep_ptr->max_pkt_size <= txep_ptr->fifo_size) &&
                (txep_ptr->txn_length <= txep_ptr->fifo_size))
            {
                txep_ptr->state = MSS_USB_EP_VALID;
                MSS_USB_CIF_ep_write_pkt(txep_ptr->num,
                                        (txep_ptr->buf_addr +
                                         txep_ptr->xfr_count),
                                         txep_ptr->dma_enable,
                                         txep_ptr->dma_channel,
                                         txep_ptr->xfr_type,
                                         txep_ptr->xfr_length,
                                         txep_ptr->txn_length);
            }
            else
            {
                ASSERT(0);
            }
        }
    }
    else
    {
        /* The Transmit endpoint generates an endpoint interrupt when previously
         * initiated TX transfer is aborted.Class driver doesn't need to know this.
         * Also this driver doesn't need to do anything in this case.*/
    }
}

/*
 * This Call-back function is executed on completion of current IN transfer.
 */
static void
mss_usbh_ep_rx_cb
(
    uint8_t ep_num,
    uint8_t status
)
{
    uint8_t this_tdev = 0u;
    uint8_t transfer_complete = 0u;
    uint32_t received_count = 0u;

    mss_usb_ep_t* rxep_ptr = &gh_rx_ep[ep_num];

    /* Retrieve tdev_idx which received data. */
    this_tdev = gh_rx_ep[ep_num].tdev_idx;

    if (status & (RX_EP_OVER_RUN_ERROR | RX_EP_STALL_ERROR |
                 RX_EP_DATA_ERROR | RX_EP_PID_ERROR | RX_EP_ISO_INCOMP_ERROR))
    {
        transfer_complete = 1u;
    }
    else
    {
        if (MSS_USB_CIF_rx_ep_is_rxpktrdy((mss_usb_ep_num_t)ep_num))
        {
            received_count = (uint32_t)MSS_USB_CIF_rx_ep_read_count((mss_usb_ep_num_t)ep_num);
            if (DMA_ENABLE == rxep_ptr->dma_enable)
            {
                if (MSS_USB_DMA_MODE1 == (MSS_USB_CIF_rx_ep_get_dma_mode((mss_usb_ep_num_t)ep_num)))
                {
                    uint32_t increamented_addr;

                    /*
                     * This means we are in BULK transfer with DMA mode1.
                     * all the rxmaxP size pkts are received and last short pkt
                     * need to be read without DMA or by switching to mode 0.
                     * After switching mode to 0, this ISR handler is invoked
                     * again. Data packet will be read then.
                     */

                    /* Read 'short packet' without DMA */
                    MSS_USB_CIF_dma_stop_xfr(rxep_ptr->dma_channel);
                    MSS_USB_CIF_rx_ep_clr_autoclr((mss_usb_ep_num_t)ep_num);

                    increamented_addr = MSS_USB_CIF_dma_read_addr(rxep_ptr->dma_channel);

                    /* Count number of bytes read so far,since DMA was operating
                     * in m1 with Autoclr.*/
                    rxep_ptr->xfr_count = (increamented_addr - ((ptrdiff_t)(rxep_ptr->buf_addr)));

                    if (received_count)
                    {
                        MSS_USB_CIF_read_rx_fifo((mss_usb_ep_num_t)ep_num,
                                                 (rxep_ptr->buf_addr+rxep_ptr->xfr_count),
                                                 received_count);

                        rxep_ptr->xfr_count += received_count;
                        MSS_USB_CIF_rx_ep_clr_rxpktrdy((mss_usb_ep_num_t)ep_num);
                    }
                    transfer_complete = 1u;
                }
                else
                {
                    MSS_USB_CIF_dma_write_count(rxep_ptr->dma_channel, received_count);
                    MSS_USB_CIF_dma_start_xfr(rxep_ptr->dma_channel);
                    transfer_complete = 2u;
                    /*Upper layer cb will be called from DMA ISR*/
                }
            }
            else /* dma disable */
            {
                if (received_count)
                {
                    MSS_USB_CIF_read_rx_fifo((mss_usb_ep_num_t)ep_num,
                                             (rxep_ptr->buf_addr+rxep_ptr->xfr_count),
                                             received_count);
                    rxep_ptr->txn_count = received_count;
                    rxep_ptr->xfr_count += received_count;
                    MSS_USB_CIF_rx_ep_clr_rxpktrdy((mss_usb_ep_num_t)ep_num);
                }

                if (rxep_ptr->xfr_count == rxep_ptr->xfr_length)
                {
                    transfer_complete = 1u;
                }
                else if (rxep_ptr->xfr_count < rxep_ptr->xfr_length)
                {
                    /* Reset txn_count for next txn */
                    rxep_ptr->txn_count = 0u;

                    if ((rxep_ptr->xfr_length - rxep_ptr->xfr_count) > rxep_ptr->max_pkt_size)
                    {
                        rxep_ptr->txn_length = rxep_ptr->max_pkt_size;
                    }
                    else
                    {
                        rxep_ptr->txn_length = (rxep_ptr->xfr_length - rxep_ptr->xfr_count);
                    }

                    MSS_USBH_CIF_rx_ep_set_reqpkt((mss_usb_ep_num_t)ep_num);
                }
                else
                {
                    ASSERT(0);
                }
            }
        }
    }

    if (1u == transfer_complete)
    {
        rxep_ptr->state = (mss_usb_ep_state_t)status;
        if (0 != g_tdev[this_tdev].class_handle->usbh_class_rx_done)
        {
            /* call-back class driver */ /* count = 0 since there was error */
            g_tdev[this_tdev].class_handle->usbh_class_rx_done(g_tdev[this_tdev].addr,
                                                               status,
                                                               rxep_ptr->xfr_count);
        }
        else
        {
            ASSERT(0); /* Invalid pointer */
        }

    }
}

/*
 * This Call-back function is executed on completion of current CONTROL transfer.
 */
static void
mss_usbh_cep_cb
(
    uint8_t status
)
{
    gh_cep_cb_event = status;
}

/*
 * This Call-back function is executed on receiving SOF interrupt in HOST mode
 */
static void
mss_usbh_sof_cb
(
    uint32_t frame_number
)
{
}

/*
 * This Call-back function is executed on detecting the device attach event
 */
static void
mss_usbh_connect_cb
(
    mss_usb_device_speed_t target_speed,
    mss_usb_vbus_level_t vbus_level
)
{
    g_tdev[TDEV_R].speed = target_speed;
    gh_tdev_connect_event = 1u;
}

/*
 * This Call-back function is executed on detecting the device detach event
 */
static void
mss_usbh_disconnect_cb
(
    void
)
{
    gh_tdev_discon_event = 1u;
}

/*
 * This Call-back function is executed on detecting a Vbus level error event
 */
static void
mss_usbh_vbus_err_cb
(
    mss_usb_vbus_level_t vbus_level
)
{

}

/*
 * This Call-back function is executed on detecting the babble error event
 */
static void
mss_usbh_babble_err_cb
(
    void
)
{
    /* Not supported yet */
}

/*
 * This Call-back function is executed on detecting the session request event
 * when the attached device is also a OTG device.
 */
static void
mss_usbh_session_request_cb
(
    void
)
{
    /* Not supported yet */
}

static void mss_usbh_dma_handler_cb
(
    mss_usb_ep_num_t ep_num,
    mss_usb_dma_dir_t dma_dir,
    uint8_t status,
    uint32_t dma_addr_val
)
{
    mss_usb_ep_t* ep_ptr = 0;
    uint8_t this_tdev = 0u;

    if (DMA_XFR_ERROR == status)
    {
        ASSERT(0);
    }
    else
    {
        if (MSS_USB_DMA_READ == dma_dir)    /*TX EP*/
        {
            ep_ptr = &gh_tx_ep[ep_num];

            /* Retrieve tdev_idx which received data.*/
            this_tdev = gh_tx_ep[ep_num].tdev_idx;

            /* EP interrupt won't happen when short packet is not received */
            if ((NO_ZLP_TO_XFR == ep_ptr->add_zlp) &&
                (ep_ptr->xfr_length) &&
                (!(ep_ptr->xfr_length % ep_ptr->max_pkt_size)))
            {
                /* Wait till last TxMaxPkt size packet is sent.*/
                while (MSS_USB_CIF_tx_ep_is_txpktrdy(ep_num))
                {
                    ;
                }
                ep_ptr->xfr_count = dma_addr_val - (ptrdiff_t)ep_ptr->buf_addr;
                ep_ptr->state = MSS_USB_EP_XFR_SUCCESS;
                if (0 != g_tdev[this_tdev].class_handle->usbh_class_tx_done)
                {
                    /* Call-back class driver */
                    g_tdev[this_tdev].class_handle->usbh_class_tx_done(g_tdev[this_tdev].addr,
                                                                       status,
                                                                       ep_ptr->xfr_count);
                }
            }
            else
            {
                MSS_USB_CIF_tx_ep_set_txpktrdy(ep_num);
            }
        }
        /* RX EP */
        else if (MSS_USB_DMA_WRITE == dma_dir)
        {
            ep_ptr = &gh_rx_ep[ep_num];

            /* Retrieve tdev_idx which transmitted data.*/
            this_tdev = gh_rx_ep[ep_num].tdev_idx;

            if ((NO_ZLP_TO_XFR == ep_ptr->add_zlp) &&
                (ep_ptr->xfr_length) &&
                (!(ep_ptr->xfr_length % ep_ptr->max_pkt_size)))
            {
                MSS_USB_CIF_dma_stop_xfr(ep_ptr->dma_channel);
                MSS_USBH_CIF_rx_ep_clr_reqpkt((mss_usb_ep_num_t)ep_num);
                MSS_USB_CIF_rx_ep_clr_autoclr((mss_usb_ep_num_t)ep_num);

                ep_ptr->xfr_count = dma_addr_val - (ptrdiff_t)ep_ptr->buf_addr;
                ep_ptr->state = MSS_USB_EP_XFR_SUCCESS;
                if (MSS_USB_DMA_MODE0 == (MSS_USB_CIF_rx_ep_get_dma_mode(ep_num)))
                {
                    MSS_USB_CIF_rx_ep_clr_rxpktrdy(ep_num);
                }
                if (0 != g_tdev[this_tdev].class_handle->usbh_class_rx_done)
                {
                    /* Call-back class driver */
                    g_tdev[this_tdev].class_handle->
                        usbh_class_rx_done(g_tdev[this_tdev].addr,
                                           status,
                                           ep_ptr->xfr_count);
                }
            }
        }
    }
}

/*
 * This function checks errors in the received device descriptor.
 */
static int8_t
validate_dev_desc
(
    dev_desc_t* p_desc
)
{
    int8_t result = 0;

    if (p_desc->bLength != USB_STD_DEVICE_DESCR_LEN)
    {
        result = DEV_DESC_WRONG_LENGTH;
    }
    if (p_desc->bDescriptorType != USB_DEVICE_DESCRIPTOR_TYPE)
    {
        result = DEV_DESC_WRONG_DESC_TYPE;
    }

    if (!((p_desc->bcdUSB != USB_BCD_VERSION_2_0) ||
          (p_desc->bcdUSB != USB_BCD_VERSION_2_1) ||
          (p_desc->bcdUSB != USB_BCD_VERSION_1_1) ||
          (p_desc->bcdUSB != USB_BCD_VERSION_1_0)))
    {
        result = DEV_DESC_WRONG_USBBCD;
    }

    if (!((p_desc->bMaxPacketSize0 != 8u) ||
          (p_desc->bMaxPacketSize0 != 16u) ||
          (p_desc->bMaxPacketSize0 != 32u) ||
          (p_desc->bMaxPacketSize0 != 64u)))

    {
        result = DEV_DESC_WRONG_MAXPKTSZ0;
    }

    if ((MSS_USB_DEVICE_HS == g_tdev[TDEV_R].speed))
    {
        if ((p_desc->bcdUSB != USB_BCD_VERSION_2_0) &&
            (p_desc->bcdUSB != USB_BCD_VERSION_2_1))
        {
            result = DEV_DESC_HS_USBBCD_NOT200;
        }
        if (p_desc->bMaxPacketSize0 != 64u)
        {
            result = DEV_DESC_HS_MAXPKTSZ0_NOT64;
        }
    }

    if ((MSS_USB_DEVICE_LS == g_tdev[TDEV_R].speed))
    {
        if (p_desc->bMaxPacketSize0 != 8u)
        {
            result = DEV_DESC_LS_MAXPKTSZ0_NOT8;
        }
    }

    if ((MSS_USB_DEVICE_FS == g_tdev[TDEV_R].speed))
    {
        result = FS_DEV_NOT_SUPPORTED;
    }

    /* Need to comment the below section for keyboard and mouse are low speed
     * device
     */
#if 0
    if ((MSS_USB_DEVICE_LS == g_tdev[TDEV_R].speed))
    {
        result = LS_DEV_NOT_SUPPORTED;
    }
#endif

    /*0 return value indicates success*/
    return (result);
}

/*
 * This function checks errors in the received configuration descriptor (9bytes)
 */
static int8_t
validate_def_conf_desc
(
    def_conf_desc_t* p_desc
)
{
    int8_t result = 0u;
    if (p_desc->bLength != USB_STD_CONFIG_DESCR_LEN)
    {
        result = CONF_DESC_WRONG_LENGTH;
    }
    if (p_desc->bDescriptorType != USB_CONFIGURATION_DESCRIPTOR_TYPE)
    {
        result = CONF_DESC_WRONG_DESC_TYPE;
    }
    if (p_desc->bMaxPower > USB_MAX_BUS_POWER)
    {
        result = CONF_DESC_POWER_LIM_EXCEED;
    }

    /*0 return value indicates success*/
    return (result);
}

/*
 * This function executes the sequence of actions when device is disconnected.
 */
static void
mss_usbh_handle_discon_event
(
    void
)
{
    gh_tdev_discon_event = 0u;
    tdev_idx = TDEV_R;

    if (0 != g_user_cb->usbh_tdev_dettached)
    {
        g_user_cb->usbh_tdev_dettached();
    }

    if (1u == g_rcd[TDEV_R].alloc_state)
    {
        g_rcd[TDEV_R].class_handle->usbh_class_release(g_tdev[TDEV_R].addr);
    }

    g_tdev[TDEV_R].addr = USB_DEFAULT_TARGET_ADDR;
    g_tdev[TDEV_R].speed = MSS_USB_DEVICE_HS;
    g_tdev[TDEV_R].state = MSS_USB_NOT_ATTACHED_STATE;
    g_tdev[TDEV_R].hub_addr = 0u;
    g_tdev[TDEV_R].hub_port = 0u;
    g_tdev[TDEV_R].hub_mtt = 0u;
    g_tdev[TDEV_R].tdev_maxpktsz0 = CEP_MAX_PKT_SIZE;
    g_tdev[TDEV_R].class_handle = 0u;
    g_tdev_error_code = 0;

    memset(&g_tdev_desc[TDEV_R], 0u, sizeof(tdev_desc_t));

    /* Un-register Class driver */
    g_rcd[TDEV_R].alloc_state = 0u;

    mss_usbh_reset_enum_fsm();

    MSS_USB_CIF_enable_usbirq(CONNECT_IRQ_MASK |
                              DISCONNECT_IRQ_MASK);

    MSS_USBH_configure_control_pipe(USB_DEFAULT_TARGET_ADDR);

    MSS_USBH_CIF_tx_ep_set_target_func_addr(MSS_USB_CEP,0u);
    MSS_USBH_CIF_cep_set_type0_reg((mss_usb_device_speed_t)0u);
}

static int8_t
host_enum_check_class_support
(
    tdev_id_t tid
)
{
    uint8_t i = 0u;

    /* Max number of registered Class drivers supported is 1 */
    for (i = 0u;i < 1u;i++)
    {
        /* Value 9 in interface descriptor's bInterfaceClass field indicates
         * HUB Class device
         */
        if (0x09u == g_tdev_desc[tid].conf_desc.cs.upended_desc[5])
        {
            return (HUB_CLASS_NOT_SUPPORTED);
        }

        /* Comparing assigned class driver with the interface descriptor's
         * bInterfaceClass, bInterfaceSubClass  and bInterfaceProtocol
         * fields*/
        if (((uint8_t)(g_rcd[i].class_handle->class_id>>16u) ==
             g_tdev_desc[tid].conf_desc.cs.upended_desc[5]) &&
            ((uint8_t)(g_rcd[i].class_handle->class_id>>8u) ==
             g_tdev_desc[tid].conf_desc.cs.upended_desc[6]) &&
            ((uint8_t)(g_rcd[i].class_handle->class_id) ==
             g_tdev_desc[tid].conf_desc.cs.upended_desc[7]))
        {
            /* Class driver is matched and available. */
            if (g_rcd[i].alloc_state == 0u)
            {
                /* Allocation successful */
                return (i);
            }
            else
            {
                /* Match found but Driver is already assigned */
                return (CLASS_DRIVER_NOT_FREE);
            }
        }
    }

    /* No matching class driver found. */
    return (CLASS_DRIVER_NOT_MATCHED);
}

#endif /* MSS_USB_HOST_ENABLED */

#ifdef __cplusplus
}
#endif
