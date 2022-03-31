/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Microchip PolarFire SoC MSS USB Driver Stack
 *      USB Logical Layer (USB-LL)
 *          USBH-MSC class driver.
 *
 *
 *  This file implements Host side MSC class specific initialization
 *  and request handling.
 *
 * V2.4 NAKTIMEOUT error handling.
 * V2.4 Naming convention change, other cosmetic changes.
 * V2.3 Host performance improvement changes
 *
 * SVN $Revision$
 * SVN $Date$
 */

#include "mss_usb_host_msc.h"
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
  Constant values internally used by USBH-MSC driver.
 */
#define MSS_USBH_MSC_CLASS_ID                               0x08u
#define MSS_USBH_MSC_SUBCLASS_ID                            0x06u
#define MSS_USBH_MSC_PROTOCOL_ID                            0x50u

#define MSS_USBH_MSC_DRIVER_ID       (uint32_t)((MSS_USBH_MSC_CLASS_ID << 16) |\
                                               (MSS_USBH_MSC_SUBCLASS_ID << 8)|\
                                               (MSS_USBH_MSC_PROTOCOL_ID) )

#define SCSI_COMMAND_PASSED                                 0x01u
#define SCSI_COMMAND_FAILED                                 0x02u
#define SCSI_COMMAND_PHASE_ERR                              0x03u

#define USBH_MSC_BULK_TX_PIPE                               MSS_USB_TX_EP_1
#define USBH_MSC_BULK_RX_PIPE                               MSS_USB_RX_EP_1

#define USBH_MSC_BULK_TX_PIPE_FIFOADDR                      0x100u
#define USBH_MSC_BULK_RX_PIPE_FIFOADDR                      0x300u

#define USBH_MSC_BULK_TX_PIPE_FIFOSZ                        0x200u
#define USBH_MSC_BULK_RX_PIPE_FIFOSZ                        0x200u

/***************************************************************************//**
  Types internally used by USBH-MSC driver.
 */
typedef enum {
    MSC_BOT_IDLE,
    MSC_BOT_COMMAND_PHASE,
    MSC_BOT_DATA_PHASE,
    MSC_BOT_STATUS_PHASE,
    MSC_BOT_STATUS_WAITCOMPLETE,
    MSC_BOT_ERROR
}g_msc_bot_state_t;

typedef struct {
    uint8_t num;
    uint16_t maxpktsz;
} msd_tdev_ep_t;

typedef struct {
    uint8_t* cbuf; /* always31bytes */
    uint8_t* dbuf;
    uint32_t dbuf_len;
    uint8_t* sbuf; /* statusalways 13bytes */
    uint8_t volatile st;
} scsi_command_t;

/***************************************************************************//**
  Private functions declarations for USBH-MSC driver.
 */
static scsi_command_t g_scsi_command = {0};
static volatile uint8_t g_usbh_msc_alloc_event = 0u;
//static uint8_t g_usbh_msc_release_event = 0u; // unused
static volatile uint8_t g_usbh_msc_cep_event = 0u;
static volatile uint8_t g_usbh_msc_tx_event = 0u;
static volatile uint8_t g_usbh_msc_rx_event = 0u;

#if defined(__GNUC__)
static msd_cbw_t g_bot_cbw __attribute__ ((aligned (4))) = {0};
static uint8_t g_bot_csw[13] __attribute__ ((aligned (4))) = {0};

/* Store Inquiry response */
static uint8_t g_bot_inquiry[36] __attribute__ ((aligned (4))) = {0};

/* Store Read_capacity response */
static uint8_t g_bot_readcap[8] __attribute__ ((aligned (4)))= {0};

#elif defined(__ICCARM__)
#pragma data_alignment = 4
static msd_cbw_t g_bot_cbw = {0};
static uint8_t g_bot_csw[13] = {0};
static uint8_t g_bot_inquiry[36] = {0};
static uint8_t g_bot_readcap[8] = {0};

#elif defined(__CC_ARM)
__align(4) static msd_cbw_t g_bot_cbw = {0};
__align(4) static uint8_t g_bot_csw[13] = {0};
__align(4) static uint8_t g_bot_inquiry[36] = {0};
__align(4) static uint8_t g_bot_readcap[8] = {0};
#endif

static volatile g_msc_bot_state_t g_msc_bot_state = MSC_BOT_IDLE;

static uint8_t g_msd_tdev_addr = 0u;
static mss_usb_state_t msd_tdev_state = MSS_USB_NOT_ATTACHED_STATE;
static uint8_t g_msd_conf_desc[32] = {0};

/* Data type changed from uint8_t since ASSERT in start_control_xfr() */
static uint32_t g_tdev_max_lun_idx = 0u;
static msd_tdev_ep_t g_tdev_in_ep = {0};
static msd_tdev_ep_t g_tdev_out_ep = {0};

static volatile mss_usbh_msc_state_t g_msc_state = USBH_MSC_IDLE;
static mss_usbh_msc_err_code_t g_msch_error_code = USBH_MSC_NO_ERROR;

static mss_usbh_msc_user_cb_t* g_msch_user_cb;

static uint8_t usbh_msc_allocate_cb(uint8_t tdev_addr);
static uint8_t usbh_msc_release_cb(uint8_t tdev_addr);
static uint8_t usbh_msc_cep_done_cb(uint8_t tdev_addr, uint8_t status, uint32_t count);

static uint8_t usbh_msc_tx_complete_cb(uint8_t tdev_addr,
                                       uint8_t status,
                                       uint32_t count);

static uint8_t usbh_msc_rx_cb(uint8_t tdev_addr, uint8_t status, uint32_t count);
static mss_usbh_msc_err_code_t MSS_USBH_MSC_validate_class_desc(uint8_t* p_cd);
static mss_usbh_msc_err_code_t MSS_USBH_MSC_extract_tdev_ep_desc(void);

static void usbh_msc_construct_class_req(uint8_t* buf,
                                         uint8_t req,
                                         uint8_t bInterfaceNumber);

/***************************************************************************//**
  Definition of Class call-back functions used by USBH driver.
 */
mss_usbh_class_cb_t msd_class =
{
    MSS_USBH_MSC_DRIVER_ID,
    usbh_msc_allocate_cb,
    usbh_msc_release_cb,
    usbh_msc_cep_done_cb,
    usbh_msc_tx_complete_cb,
    usbh_msc_rx_cb,
    0
};

/*******************************************************************************
 * EXPORTED API Functions
 ******************************************************************************/

/******************************************************************************
 * See mss_usb_host_msc.h for details of how to use this function.
 */
void
MSS_USBH_MSC_init
(
    mss_usbh_msc_user_cb_t* user_sb
)
{
    g_msc_state = USBH_MSC_IDLE;
    g_msc_bot_state = MSC_BOT_IDLE;
    g_tdev_max_lun_idx = 0u;
    memset(g_msd_conf_desc, 0u, sizeof(g_msd_conf_desc));
    memset(g_bot_inquiry, 0u, sizeof(g_bot_inquiry));
    memset(&g_bot_csw, 0u, sizeof(g_bot_csw));
    g_tdev_in_ep.maxpktsz = 0u;
    g_tdev_in_ep.num = 0u;
    g_tdev_out_ep.maxpktsz = 0u;
    g_tdev_out_ep.num = 0u;
    g_msd_tdev_addr = 0u;
    g_msch_user_cb = user_sb;

    g_scsi_command.cbuf = (uint8_t*)0;
    g_scsi_command.dbuf = (uint8_t*)0;
    g_scsi_command.sbuf = (uint8_t*)0;
    g_scsi_command.dbuf_len = 0u;
    g_scsi_command.st = 0u;
    memset(g_bot_readcap, 0u, sizeof(g_bot_readcap));
}

/******************************************************************************
 * See mss_usb_host_msc.h for details of how to use this function.
 */
void* MSS_USBH_MSC_get_handle
(
    void
)
{
    return ((void*)&msd_class);
}

/******************************************************************************
 * See mss_usb_host_msc.h for details of how to use this function.
 */
void
MSS_USBH_MSC_task
(
    void
)
{
    uint8_t std_req_buf[USB_SETUP_PKT_LEN] = {0};
    static volatile uint32_t wait_mili = 0u;

    switch (g_msc_state)
    {
        case USBH_MSC_IDLE:
            if (g_usbh_msc_alloc_event)
            {
                g_usbh_msc_alloc_event = 0u;
                g_msc_state = USBH_MSC_GET_CLASS_DESCR;
            }
        break;

        case USBH_MSC_GET_CLASS_DESCR:
            msd_tdev_state = MSS_USBH_get_tdev_state(g_msd_tdev_addr);
            if (MSS_USB_ADDRESS_STATE == msd_tdev_state)
            {
                mss_usb_ep_state_t cep_st;
                cep_st = MSS_USBH_get_cep_state();
                if (MSS_USB_CEP_IDLE == cep_st)
                {
                    MSS_USBH_configure_control_pipe(g_msd_tdev_addr);
                    memset(std_req_buf, 0u, 8*(sizeof(uint8_t)));
                    MSS_USBH_construct_get_descr_command(std_req_buf,
                                                         USB_STD_REQ_DATA_DIR_IN,
                                                         USB_STANDARD_REQUEST,
                                                         USB_STD_REQ_RECIPIENT_DEVICE,
                                                         USB_STD_REQ_GET_DESCRIPTOR,
                                                         USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                                         0u, /*stringID*/
                                                         32u);/* config_Desc_length */

                    g_msc_state = USBH_MSC_WAIT_GET_CLASS_DESCR;
                    MSS_USBH_start_control_xfr(std_req_buf,
                                               (uint8_t*)&g_msd_conf_desc,
                                               USB_STD_REQ_DATA_DIR_IN,
                                               32u);        /* config_Desc_length */
                }
            }
        break;

        case USBH_MSC_WAIT_GET_CLASS_DESCR:
            if (g_usbh_msc_cep_event)
            {
                mss_usbh_msc_err_code_t res = USBH_MSC_NO_ERROR;
                g_usbh_msc_cep_event = 0u;
                res = MSS_USBH_MSC_validate_class_desc(0);

                if (res == 0u)
                {
                    g_msc_state = USBH_MSC_SET_CONFIG;
                }
                else
                {
                    g_msc_state = USBH_MSC_ERROR;
                    g_msch_error_code = res;
                }

                res = MSS_USBH_MSC_extract_tdev_ep_desc();

                if (res == 0u)
                {
                    g_msc_state = USBH_MSC_SET_CONFIG;
                }
                else
                {
                    g_msc_state = USBH_MSC_ERROR;
                    g_msch_error_code = res;
                }
            }
        break;

        case USBH_MSC_SET_CONFIG:
            if (0 != g_msch_user_cb->msch_valid_config)
            {
                g_msch_user_cb->msch_valid_config();
            }

            memset(std_req_buf, 0u, 8*(sizeof(uint8_t)));
            std_req_buf[1] = USB_STD_REQ_SET_CONFIG;

            /* bConfigurationValue*/
            std_req_buf[2] = g_msd_conf_desc[5];
            g_msc_state = USBH_MSC_WAIT_SET_CONFIG;

            MSS_USBH_start_control_xfr(std_req_buf,
                                       (uint8_t*)&g_msd_conf_desc,
                                       USB_STD_REQ_DATA_DIR_IN,
                                       0u);
        break;
        case USBH_MSC_WAIT_SET_CONFIG:
            if (g_usbh_msc_cep_event)
            {
                g_usbh_msc_cep_event = 0u;
                wait_mili = MSS_USBH_get_milis();
                g_msc_state = USBH_MSC_WAIT_DEV_SETTLE;
            }
        break;

        case USBH_MSC_WAIT_DEV_SETTLE:
            /* After SET_CONFIG command, we must give time for device to settle
             * down as per spec
             */
            if ((MSS_USBH_get_milis() - wait_mili) > 60u)
            {
                g_msc_state = USBH_MSC_GET_MAX_LUN;
            }
        break;

        case USBH_MSC_GET_MAX_LUN:
            usbh_msc_construct_class_req(std_req_buf,
                                         USB_MSC_BOT_REQ_GET_MAX_LUN,
                                         g_msd_conf_desc[11]);/* bInterfaceNum */

            MSS_USBH_configure_control_pipe(g_msd_tdev_addr);

            g_msc_state = USBH_MSC_WAIT_GET_MAX_LUN;

            MSS_USBH_start_control_xfr(std_req_buf,
                                       (uint8_t*)&g_tdev_max_lun_idx,
                                       USB_STD_REQ_DATA_DIR_IN,
                                       1u);
        break;

        case USBH_MSC_WAIT_GET_MAX_LUN:
            if (g_usbh_msc_cep_event)
            {
                if (MSS_USB_EP_XFR_SUCCESS == g_usbh_msc_cep_event)
                {
                    g_msc_state = USBH_MSC_CONFIG_BULK_ENDPOINTS;
                }
                else if (MSS_USB_EP_STALL_RCVD == g_usbh_msc_cep_event)
                {
                    /* Stall for this command means single LUN support by Target */
                    g_tdev_max_lun_idx = 0u;
                    g_msc_state = USBH_MSC_CLR_CEP_STALL;

                    /* Clear Stall using CLR_FEATURE Command */
                }
                else
                {
                    ASSERT(0); /* Invalid CEP event */
                }

                g_usbh_msc_cep_event = 0u;
            }
        break;

        case USBH_MSC_CLR_CEP_STALL:
        {
            /* Std clear CEP STALL command */
            uint8_t temp_buf[8u] = {0x02u,
                                    0x01u,
                                    0x00u, 0x00u,
                                    0x00u, 0x00u,
                                    0x00u, 0x00u};

            g_msc_state = USBH_MSC_WAIT_CLR_CEP_STALL;

            /* clear feature Endpoint halt on Tdev Out EP */
            MSS_USBH_start_control_xfr(temp_buf,
                                       (uint8_t*)&g_tdev_max_lun_idx,
                                       USB_STD_REQ_DATA_DIR_IN,
                                       0u);
            break;
        }
        case USBH_MSC_WAIT_CLR_CEP_STALL:
            if (g_usbh_msc_cep_event)
            {
                if (MSS_USB_EP_XFR_SUCCESS == g_usbh_msc_cep_event)
                {
                    g_msc_state = USBH_MSC_CONFIG_BULK_ENDPOINTS;
                }
                else if (MSS_USB_EP_STALL_RCVD == g_usbh_msc_cep_event)
                {
                    g_msc_state = USBH_MSC_ERROR;
                    g_msch_error_code = USBH_MSC_CLR_CEP_STALL_ERROR;
                }
                else
                {
                    ASSERT(0);/* Invalid CEP event */
                }
                g_usbh_msc_cep_event = 0u;
            }
        break;

        case USBH_MSC_CONFIG_BULK_ENDPOINTS:
            /* Configure BULK TX/RX Endpoints */
            MSS_USBH_configure_out_pipe(g_msd_tdev_addr,
                                        USBH_MSC_BULK_TX_PIPE,
                                        g_tdev_out_ep.num,  /* Targeted OUTEP Num */
                                        USBH_MSC_BULK_TX_PIPE_FIFOADDR,
                                        USBH_MSC_BULK_TX_PIPE_FIFOSZ,
                                        g_tdev_out_ep.maxpktsz,
                                        1,
                                        DMA_DISABLE,
                                        MSS_USB_DMA_CHANNEL1,
                                        MSS_USB_XFR_BULK,
                                        NO_ZLP_TO_XFR,
                                        32768);/* Max NAKLimit value */

            MSS_USBH_configure_in_pipe(g_msd_tdev_addr,
                                       USBH_MSC_BULK_RX_PIPE,
                                       g_tdev_in_ep.num,    /* Targeted OutEP Num */
                                       USBH_MSC_BULK_RX_PIPE_FIFOADDR,
                                       USBH_MSC_BULK_RX_PIPE_FIFOSZ,
                                       g_tdev_in_ep.maxpktsz,
                                       1,
                                       DMA_DISABLE,
                                       MSS_USB_DMA_CHANNEL2,
                                       MSS_USB_XFR_BULK,
                                       NO_ZLP_TO_XFR,
                                       32768);/* Max NAKLimit value */

            g_msc_state = USBH_MSC_TEST_UNIT_READY_CPHASE;
        break;


        case USBH_MSC_TEST_UNIT_READY_CPHASE:
            MSS_USBH_MSC_construct_cbw_cb6byte(USB_MSC_SCSI_TEST_UNIT_READY,
                                               0u,
                                               &g_bot_cbw);

            MSS_USBH_write_out_pipe(g_msd_tdev_addr,
                                    USBH_MSC_BULK_TX_PIPE,
                                    g_tdev_out_ep.num,
                                    g_tdev_out_ep.maxpktsz,
                                    (uint8_t*)&g_bot_cbw,
                                    31u); /* std_cbw_len */

            g_msc_state = USBH_MSC_TEST_UNIT_READY_SPHASE;
        break;

        case USBH_MSC_TEST_UNIT_READY_SPHASE:
            if (g_usbh_msc_tx_event)
            {
                g_usbh_msc_tx_event = 0u;
                g_msc_state = USBH_MSC_TEST_UNIT_READY_WAITCOMPLETE;
                MSS_USBH_read_in_pipe(g_msd_tdev_addr,
                                      USBH_MSC_BULK_RX_PIPE,
                                      g_tdev_in_ep.num,
                                      g_tdev_in_ep.maxpktsz,
                                      (uint8_t*)&g_bot_csw,
                                      13u);/* standard status msg */
            }
        break;

        case USBH_MSC_TEST_UNIT_READY_WAITCOMPLETE:
            if (g_usbh_msc_rx_event)
            {
                g_usbh_msc_rx_event = 0u;

                /* decode the received status */
                if (g_bot_csw[12] == 0x00u)       /* PASSED */
                {
                    g_msc_state = USBH_MSC_SCSI_INQUIRY_CPHASE;
                }
                else if (g_bot_csw[12] == 0x01u)  /* FAILED */
                {
                    g_msc_state = USBH_MSC_SCSI_REQSENSE_CPHASE;
                }
                else if (g_bot_csw[12] == 0x02u)
                {
                    ASSERT(0);            /* phase error, reset recovery required */
                }
            }

        break;
        case USBH_MSC_SCSI_INQUIRY_CPHASE:
            MSS_USBH_MSC_construct_cbw_cb6byte(USB_MSC_SCSI_INQUIRY,
                                               36u, /*standard INQUIRY response size */
                                               &g_bot_cbw);

            g_msc_state = USBH_MSC_SCSI_INQUIRY_DPHASE;

            MSS_USBH_write_out_pipe(g_msd_tdev_addr,
                                    USBH_MSC_BULK_TX_PIPE,
                                    g_tdev_out_ep.num,
                                    g_tdev_out_ep.maxpktsz,
                                    (uint8_t*)&g_bot_cbw,
                                    31u); /* std_cbw_len */
        break;

        case USBH_MSC_SCSI_INQUIRY_DPHASE:
            if (g_usbh_msc_tx_event)
            {
                g_usbh_msc_tx_event = 0u;
                g_msc_state = USBH_MSC_SCSI_INQUIRY_SPHASE;

                MSS_USBH_read_in_pipe(g_msd_tdev_addr,
                                      USBH_MSC_BULK_RX_PIPE,
                                      g_tdev_in_ep.num,
                                      g_tdev_in_ep.maxpktsz,
                                      g_bot_inquiry,
                                      36u);/* standard INQUIRY response size */
            }
        break;

        case USBH_MSC_SCSI_INQUIRY_SPHASE:
            if (g_usbh_msc_rx_event)
            {
                g_usbh_msc_rx_event = 0u;
                g_msc_state = USBH_MSC_SCSI_INQUIRY_WAITCOMPLETE;

                MSS_USBH_read_in_pipe(g_msd_tdev_addr,
                                      USBH_MSC_BULK_RX_PIPE,
                                      g_tdev_in_ep.num,
                                      g_tdev_in_ep.maxpktsz,
                                      (uint8_t*)&g_bot_csw,
                                      13u);/* standard status msg */
            }
        break;

        case USBH_MSC_SCSI_INQUIRY_WAITCOMPLETE:
            if (g_usbh_msc_rx_event)
            {
                g_usbh_msc_rx_event = 0x0U;
                g_msc_state = USBH_MSC_SCSI_READ_CAPACITY_CPHASE;
            }
        break;

        case USBH_MSC_SCSI_REQSENSE_CPHASE:
        {
            /* This is a standard command buffer for REQUEST SENSE command */
            uint8_t cbuf[] = {0x55U, 0x53U, 0x42U, 0x43U, 0x28U, 0x2EU, 0x59U, 0xAFU,
                              0x12U, 0x00U, 0x00U, 0x00U, 0x80U, 0x00U, 0x0CU, 0x03U,
                              0x00U, 0x00U, 0x00U, 0x12U, 0x00U, 0x00U, 0x00U, 0x00U,
                              0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

            g_msc_state = USBH_MSC_SCSI_REQSENSE_DPHASE;

            MSS_USBH_write_out_pipe(g_msd_tdev_addr,
                                    USBH_MSC_BULK_TX_PIPE,
                                    g_tdev_out_ep.num,
                                    g_tdev_out_ep.maxpktsz,
                                    (uint8_t*)&cbuf,
                                    31u);                          /*std_cbw_len */

        }
        break;
        case USBH_MSC_SCSI_REQSENSE_DPHASE:
        if (g_usbh_msc_tx_event)
        {
            /* standard response from device for REQUEST sense command is 18 bytes */
            static uint8_t reqsense_buf[18];
            g_usbh_msc_tx_event = 0u;
            g_msc_state = USBH_MSC_SCSI_REQSENSE_SPHASE;

            MSS_USBH_read_in_pipe(g_msd_tdev_addr,
                                  USBH_MSC_BULK_RX_PIPE,
                                  g_tdev_in_ep.num,
                                  g_tdev_in_ep.maxpktsz,
                                  reqsense_buf,
                                  18u);
        }
        break;
        case USBH_MSC_SCSI_REQSENSE_SPHASE:
            if (g_usbh_msc_rx_event)
            {
                g_usbh_msc_rx_event = 0u;
                g_msc_state = USBH_MSC_SCSI_REQSENSE_WAITCOMPLETE;

                MSS_USBH_read_in_pipe(g_msd_tdev_addr,
                                      USBH_MSC_BULK_RX_PIPE,
                                      g_tdev_in_ep.num,
                                      g_tdev_in_ep.maxpktsz,
                                      (uint8_t*)&g_bot_csw,
                                      13u);/* standard status msg */
            }

        break;
        case USBH_MSC_SCSI_REQSENSE_WAITCOMPLETE:
            if (g_usbh_msc_rx_event)
            {
                g_usbh_msc_rx_event = 0u;
                g_msc_state = USBH_MSC_TEST_UNIT_READY_CPHASE;
            }
        break;

        case USBH_MSC_SCSI_READ_CAPACITY_CPHASE:

            MSS_USBH_MSC_construct_cbw_cb10byte(USB_MSC_SCSI_READ_CAPACITY_10,
                                                0u,
                                                0u,
                                                0u,
                                                0u,
                                                &g_bot_cbw);

            g_msc_state = USBH_MSC_SCSI_READ_CAPACITY_DPHASE;

            MSS_USBH_write_out_pipe(g_msd_tdev_addr,
                                    USBH_MSC_BULK_TX_PIPE,
                                    g_tdev_out_ep.num,
                                    g_tdev_out_ep.maxpktsz,
                                    (uint8_t*)&g_bot_cbw,
                                    31u);                          /* std_cbw_len */
        break;

        case USBH_MSC_SCSI_READ_CAPACITY_DPHASE:
            if (g_usbh_msc_tx_event)
            {
                g_usbh_msc_rx_event = 0u;
                g_msc_state = USBH_MSC_SCSI_READ_CAPACITY_SPHASE;

                /* standard Read Capacity response size is 8bytes.
                 * We need this info later so keep it in g_bot_readcap
                 */
                MSS_USBH_read_in_pipe(g_msd_tdev_addr,
                                      USBH_MSC_BULK_RX_PIPE,
                                      g_tdev_in_ep.num,
                                      g_tdev_in_ep.maxpktsz,
                                      g_bot_readcap,
                                      8u);
            }

        break;

        case USBH_MSC_SCSI_READ_CAPACITY_SPHASE:
            if (g_usbh_msc_rx_event)
            {
                g_usbh_msc_rx_event = 0u;
                g_msc_state = USBH_MSC_SCSI_READ_CAPACITY_WAITCOMPLETE;

                MSS_USBH_read_in_pipe(g_msd_tdev_addr,
                                      USBH_MSC_BULK_RX_PIPE,
                                      g_tdev_in_ep.num,
                                      g_tdev_in_ep.maxpktsz,
                                      (uint8_t*)&g_bot_csw,
                                      13u);/* standard status msg */
            }
        break;

        case USBH_MSC_SCSI_READ_CAPACITY_WAITCOMPLETE:
            if (g_usbh_msc_rx_event)
            {
                uint32_t sector_size = 0u;
                g_usbh_msc_rx_event = 0u;

                sector_size = ((g_bot_readcap[4u] << 24u) |          /*sector size */
                               (g_bot_readcap[5u] << 16u) |
                               (g_bot_readcap[6u] << 8u) |
                               g_bot_readcap[7u]);

                if (sector_size != 512u)
                {
                    g_msc_state = USBH_MSC_ERROR;
                    g_msch_error_code = USBH_MSC_SECTOR_SIZE_NOT_SUPPORTED;
                }
                else
                {
                    g_msc_state = USBH_MSC_DEVICE_READY;

                    if (0 != g_msch_user_cb->msch_tdev_ready)
                    {
                        g_msch_user_cb->msch_tdev_ready();
                    }
                }
            }
        break;

        case USBH_MSC_DEVICE_READY:
            /*
             * The USBH-MSC driver will stay in this state.
             * Being in this state, the SCSI requests will be executed as
             * user the user makes those requests through the APIs of this driver.
             */
        break;

        case USBH_MSC_ERROR:
        {
            static uint8_t error = 0u;
            if (0u == error)
            {
                error = 1u;
                if (0 != g_msch_user_cb->msch_error)
                {
                    g_msch_user_cb->msch_error(g_msch_error_code);
                }
            }
        }
        break;

        case USBH_MSC_BOT_RETRY:
        {
            static uint32_t first_mili = 0;
            static uint32_t crrent_mili = 0;

            if (0u == first_mili)
            {
                first_mili = MSS_USBH_get_milis();
            }

            crrent_mili = MSS_USBH_get_milis();

            /* Found that the Sandisc devices are reporting NAKTIMEOUT error.
             * This is mostly happening when moving from DATA phase to Status
             * phase of the SCSI read/write command. At this stage restarting
             * the status phase is able to get the device to respond properly.
             * The NAKLIMIT is device specific. The WIndows hosts allow up to
             * 5Sec before declaring NAKTIMOUT. MSS USB allows to wait up to
             * ~4Sec. Hence waiting for another 1 Sec here hoping that the
             * device is recovered by then.
             */
            if (MSC_BOT_STATUS_WAITCOMPLETE == g_msc_bot_state)
            {
                if ((crrent_mili - first_mili) >= 1000u)
                {
                    first_mili = 0u;
                    crrent_mili = 0u;
                    MSS_USBH_abort_in_pipe(USBH_MSC_BULK_RX_PIPE);

                    MSS_USBH_read_in_pipe(g_msd_tdev_addr,
                                          USBH_MSC_BULK_RX_PIPE,
                                          g_tdev_in_ep.num,
                                          g_tdev_in_ep.maxpktsz,
                                          g_scsi_command.sbuf,
                                          13u);
                    g_msc_state = USBH_MSC_DEVICE_READY;
                }
            }
        }
        break;

        default:
        {
            ASSERT(0);  /*Reset recovery should be tried.*/
        }
        break;
    }
}

/*******************************************************************************
 * See mss_usb_host_msc.h for details of how to use this function.
 */
static void
usbh_msc_construct_class_req
(
    uint8_t* buf,
    uint8_t req,
    uint8_t bInterfaceNumber
)
{
    /* This implementation is as per MSC class spec */
    buf[0] = 0x21u;                                     /* bmRequestType */
    buf[1] = req;                                       /* bmRequest */

    buf[2] = 0x00u;
    buf[3] = 0x00u;                                      /* wValue */

    buf[4] = bInterfaceNumber;
    buf[5] = 0x00u;                                      /* wIndex */

    if (req == USB_MSC_BOT_REQ_GET_MAX_LUN)
    {
        buf[6] = 0x01u;
        buf[0] = 0xA1u;                                     /* bmRequestType */
    }
    else if (req == USB_MSC_BOT_REQ_BMS_RESET)
    {
        buf[6] = 0x00u;
        buf[0] = 0x21u;                                     /* bmRequestType */
    }
    else
    {
        ASSERT(0);/* invalid MSC class class request */
    }

    buf[7] = 0x00u;
}

/*******************************************************************************
 * See mss_usb_host_msc.h for details of how to use this function.
 */
void
MSS_USBH_MSC_construct_cbw_cb6byte
(
    uint8_t command_opcode,
    uint32_t data_xfr_len,
    msd_cbw_t* buf
)
{
    /* This implementation is as per MSC class spec*/

    /* Inquiry, Request_sense, Test_unit_ready commands (cb10 byte commands) */
    memset(buf, 0u, 31*(sizeof(uint8_t)));
    buf->dCBWSignature = USB_MSC_BOT_CBW_SIGNATURE;
    buf->dCBWTag = 0x20304050U;

    buf->dCBWDataTransferLength = data_xfr_len;

    if (USB_MSC_SCSI_TEST_UNIT_READY == command_opcode)
    {
        buf->bCBWFlags = 0x00u;
    }
    else if ((USB_MSC_SCSI_INQUIRY == command_opcode) ||
             (USB_MSC_SCSI_REQUEST_SENSE == command_opcode))
    {
        buf->bCBWFlags = 0x80u;
    }
    else
    {
        ASSERT(0);/* invalid cb6byte command */
    }

    buf->bCBWCBLength = 0x06u;
    buf->CBWCB[0] = command_opcode;
    buf->CBWCB[4] = data_xfr_len;
}

/*******************************************************************************
 * See mss_usb_host_msc.h for details of how to use this function.
 */
void
MSS_USBH_MSC_construct_cbw_cb10byte
(
    uint8_t command_opcode,
    uint8_t lun,
    uint32_t lb_addr,
    uint16_t num_of_lb,
    uint16_t lb_size,
    msd_cbw_t* buf
)
{
    /* This implementation is as per MSC class spec */

    /* Read_capacity, Read10, Write10 commands (cb10 byte commands) */
    memset(buf, 0u, 31*(sizeof(uint8_t)));
    buf->dCBWSignature = USB_MSC_BOT_CBW_SIGNATURE;
    buf->dCBWTag = 0x20304050U;

    if (USB_MSC_SCSI_WRITE_10 == command_opcode)
    {
        buf->bCBWFlags = 0x00u;     /* H2D */
    }
    else if ((USB_MSC_SCSI_READ_10 == command_opcode) ||
             (USB_MSC_SCSI_READ_CAPACITY_10 == command_opcode))
    {
        buf->bCBWFlags = 0x80u;     /* D2H */
    }

    buf->bCBWCBLength = 0x0Au;
    buf->CBWCB[0] = command_opcode;

    if ((USB_MSC_SCSI_READ_10 == command_opcode) ||
        (USB_MSC_SCSI_WRITE_10 == command_opcode))
    {
        buf->dCBWDataTransferLength = (num_of_lb * lb_size);    /* Transfer length */
        buf->CBWCB[1] = lun;                                    /* Lun Number */
        buf->CBWCB[2] = (uint8_t)((lb_addr >> 24) & 0xFFU);     /* MSB first */
        buf->CBWCB[3] = (uint8_t)((lb_addr >> 16) & 0xFFU);
        buf->CBWCB[4] = (uint8_t)((lb_addr >> 8) & 0xFFU);
        buf->CBWCB[5] = (uint8_t)(lb_addr & 0xFFU);

        buf->CBWCB[7] = (uint8_t)((num_of_lb >> 8) & 0xFFU);    /* MSB first */
        buf->CBWCB[8] = (uint8_t)(num_of_lb & 0xFFU);
    }
    else if ((USB_MSC_SCSI_READ_CAPACITY_10 == command_opcode))
    {
        buf->dCBWDataTransferLength = 0x08u;    /* Read Capacity Transfer length */
    }
    else
    {
        ASSERT(0);/* invalid cb10byte command */
    }
}

/*******************************************************************************
 * See mss_usb_host_msc.h for details of how to use this function.
 */
mss_usbh_msc_state_t
MSS_USBH_MSC_get_state
(
    void
)
{
   return(g_msc_state);
}

uint8_t
MSS_USBH_MSC_scsi_req
(
    uint8_t* command_buf, /* always31bytes */
    uint8_t* data_buf,
    uint32_t data_buf_len,
    uint8_t* status_buf /* status always 13bytes */
)
{
    g_scsi_command.cbuf = command_buf;
    g_scsi_command.dbuf = data_buf;
    g_scsi_command.dbuf_len = data_buf_len;
    g_scsi_command.sbuf = status_buf;

    g_msc_bot_state = MSC_BOT_COMMAND_PHASE;

    MSS_USBH_write_out_pipe(g_msd_tdev_addr,
                            USBH_MSC_BULK_TX_PIPE,
                            g_tdev_out_ep.num,
                            g_tdev_out_ep.maxpktsz,
                            g_scsi_command.cbuf,
                            31u); /* std_cbw_len */

    return(0);
}

/*******************************************************************************
 * See mss_usb_host_msc.h for details of how to use this function.
 */
uint8_t
MSS_USBH_MSC_is_scsi_req_complete
(
    void
)
{
    return (g_scsi_command.st);
}

/*******************************************************************************
 * See mss_usb_host_msc.h for details of how to use this function.
 */
uint32_t
MSS_USBH_MSC_get_sector_count
(
    void
)
{
    /*
     * TODO:Check this
     * Read_capacity_10 command returns the Last LAB i.e. Address of the
     * last Logical block. Hence Number of logical blocks(sectors) is (Last LBA+1)
     */
    return ((g_bot_readcap[0u] << 24u) |
            (g_bot_readcap[1u] << 16u) |
            (g_bot_readcap[2u] << 8u) |
            g_bot_readcap[3u]);
}

/*******************************************************************************
 * See mss_usb_host_msc.h for details of how to use this function.
 */
uint32_t
MSS_USBH_MSC_get_sector_size
(
    void
)
{
    /*
     * Return the logical block(sector) size in bytes.
     */
    return ((g_bot_readcap[4u] << 24u) |
            (g_bot_readcap[5u] << 16u) |
            (g_bot_readcap[6u] << 8u) |
             g_bot_readcap[7u]);
}

/*******************************************************************************
 * See mss_usb_host_msc.h for details of how to use this function.
 */
int8_t
MSS_USBH_MSC_read
(
    uint8_t* buf,
    uint32_t sector,
    uint32_t count
)
{
    if (0u == g_scsi_command.st)
    {
        g_scsi_command.st = 1u;
        MSS_USBH_MSC_construct_cbw_cb10byte(USB_MSC_SCSI_READ_10,
                                            0u,
                                            sector,
                                            count,
                                            512u,
                                            &g_bot_cbw);

        MSS_USBH_MSC_scsi_req((uint8_t*)&g_bot_cbw,
                              buf,
                              (count*512u),
                              (uint8_t*)&g_bot_csw);
        return (0);
    }
    else
    {
        return (-1); /* previous command is in progress */
    }
}

/*******************************************************************************
 * See mss_usb_host_msc.h for details of how to use this function.
 */
int8_t
MSS_USBH_MSC_write
(
    uint8_t* buf,
    uint32_t sector,
    uint32_t count
)
{
    if (0u == g_scsi_command.st)
    {
        g_scsi_command.st = 1u;
        MSS_USBH_MSC_construct_cbw_cb10byte(USB_MSC_SCSI_WRITE_10,
                                            0u,
                                            sector,
                                            count,
                                            512u,
                                            &g_bot_cbw);

        MSS_USBH_MSC_scsi_req((uint8_t*)&g_bot_cbw,
                              buf,
                              (count*512u),
                              (uint8_t*)&g_bot_csw);
        return (0);
    }
    else
    {
        return (-1); /* Previous command is in progress */
    }
}

/*******************************************************************************
 * Internal Functions
 ******************************************************************************/

/*
 * This Call-back function is executed when the USBH-MSC driver is allocated
 * to the attached device by USBH driver.
 */
uint8_t
usbh_msc_allocate_cb
(
    uint8_t tdev_addr
)
{
    g_msd_tdev_addr = tdev_addr;
    g_usbh_msc_alloc_event = 1u;
    return(USB_SUCCESS);
}

/*
 * This Call-back function is executed when the USBH-MSC driver is released
 * from the attached device by USBH driver.
 */
uint8_t
usbh_msc_release_cb
(
    uint8_t tdev_addr
)
{
    g_msc_state = USBH_MSC_IDLE;
    g_msc_bot_state = MSC_BOT_IDLE;
    g_tdev_max_lun_idx = 0u;
    memset(g_msd_conf_desc, 0u, sizeof(g_msd_conf_desc));
    memset(g_bot_inquiry, 0u, sizeof(g_bot_inquiry));
    memset(&g_bot_csw, 0u, sizeof(g_bot_csw));
    g_tdev_in_ep.maxpktsz = 0u;
    g_tdev_in_ep.num = 0u;
    g_tdev_out_ep.maxpktsz = 0u;
    g_tdev_out_ep.num = 0u;
    g_msd_tdev_addr = 0u;

    g_scsi_command.cbuf = (uint8_t*)0;
    g_scsi_command.dbuf = (uint8_t*)0;
    g_scsi_command.sbuf = (uint8_t*)0;
    g_scsi_command.dbuf_len = 0u;
    g_scsi_command.st = 0u;
    memset(g_bot_readcap, 0u, sizeof(g_bot_readcap));

    MSS_USB_CIF_tx_ep_disable_irq(USBH_MSC_BULK_RX_PIPE);
    MSS_USB_CIF_tx_ep_clr_csrreg(USBH_MSC_BULK_RX_PIPE);
    MSS_USB_CIF_dma_clr_ctrlreg(MSS_USB_DMA_CHANNEL2);

    MSS_USB_CIF_rx_ep_disable_irq(USBH_MSC_BULK_TX_PIPE);
    MSS_USB_CIF_rx_ep_clr_csrreg(USBH_MSC_BULK_TX_PIPE);
    MSS_USB_CIF_dma_clr_ctrlreg(MSS_USB_DMA_CHANNEL1);

    if (0 != g_msch_user_cb->msch_driver_released)
    {
        g_msch_user_cb->msch_driver_released();
    }

    return(USB_SUCCESS);
}

/*
 * This Call-back function is executed when the control transfer initiated by this
 * driver is complete.
 */
uint8_t
usbh_msc_cep_done_cb
(
    uint8_t tdev_addr,
    uint8_t status,
    uint32_t count
)
{
    g_usbh_msc_cep_event = status;

    return (USB_SUCCESS);
}

/*
 * This Call-back function is executed when the data OUT transfer initiated by
 * this driver is complete.
 */
uint8_t
usbh_msc_tx_complete_cb
(
    uint8_t tdev_addr,
    uint8_t status,
    uint32_t count
)
{
    if (g_msc_state < USBH_MSC_DEVICE_READY)
    {
        g_usbh_msc_tx_event = 1u;
    }
    else
    {
        if (0u == status)
        {
            switch (g_msc_bot_state)
            {
                case MSC_BOT_COMMAND_PHASE:
                if (0u == g_scsi_command.dbuf_len)/* zdl request */
                {
                    g_msc_bot_state = MSC_BOT_STATUS_WAITCOMPLETE;
                    MSS_USBH_read_in_pipe(g_msd_tdev_addr,
                                          USBH_MSC_BULK_RX_PIPE,
                                          g_tdev_in_ep.num,
                                          g_tdev_in_ep.maxpktsz,
                                          g_scsi_command.sbuf,
                                          13u);
                }
                else
                {
                    g_msc_bot_state = MSC_BOT_DATA_PHASE;
                    if (g_scsi_command.cbuf[12] & 0x80U)  /* bmCBWFLags field -- Read Command */
                    {
                        MSS_USBH_read_in_pipe(g_msd_tdev_addr,
                                              USBH_MSC_BULK_RX_PIPE,
                                              g_tdev_in_ep.num,
                                              g_tdev_in_ep.maxpktsz,
                                              g_scsi_command.dbuf,
                                              g_scsi_command.dbuf_len);
                    }
                    else /* write command */
                    {
                        MSS_USBH_write_out_pipe(g_msd_tdev_addr,
                                                USBH_MSC_BULK_TX_PIPE,
                                                g_tdev_out_ep.num,
                                                g_tdev_out_ep.maxpktsz,
                                                g_scsi_command.dbuf,
                                                g_scsi_command.dbuf_len); /* data_length */
                    }
                }
                break;

                case MSC_BOT_DATA_PHASE:
                if (count == g_scsi_command.dbuf_len)
                {
                    g_msc_bot_state = MSC_BOT_STATUS_WAITCOMPLETE;

                    MSS_USBH_read_in_pipe(g_msd_tdev_addr,
                                          USBH_MSC_BULK_RX_PIPE,
                                          g_tdev_in_ep.num,
                                          g_tdev_in_ep.maxpktsz,
                                          g_scsi_command.sbuf,
                                          13u);
                }
                else
                {
                    ASSERT(0);  /* at this point all data must be transfered */
                }
                break;

                case MSC_BOT_STATUS_WAITCOMPLETE:
                break;

                default:
                    ASSERT(0);  /* g_msc_bot_state must not be in any other state */
                break;
            }
        }
        else if (MSS_USB_EP_NAK_TOUT & status)
        {
            /* Device responding with NAKs. Retry */
            g_msc_state = USBH_MSC_BOT_RETRY;
        }
        else
        {
            ASSERT(0);/* Handling any other error. Not yet supported */
        }

    }
    return (USB_SUCCESS);
}

/*
 * This Call-back function is executed when the data IN transfer initiated by
 * this driver is complete.
*/
uint8_t
usbh_msc_rx_cb
(
    uint8_t tdev_addr,
    uint8_t status,
    uint32_t count
)
{
    if (g_msc_state < USBH_MSC_DEVICE_READY)
    {
        g_usbh_msc_rx_event = 1u;
    }
    else
    {
        if (0x0U == status)
        {
            switch (g_msc_bot_state)
            {
            case MSC_BOT_DATA_PHASE:
                if (count == g_scsi_command.dbuf_len)
                {
                    g_msc_bot_state = MSC_BOT_STATUS_WAITCOMPLETE;

                    MSS_USBH_read_in_pipe(g_msd_tdev_addr,
                                          USBH_MSC_BULK_RX_PIPE,
                                          g_tdev_in_ep.num,
                                          g_tdev_in_ep.maxpktsz,
                                          g_scsi_command.sbuf,
                                          13u);
                }
                else
                {
                    ASSERT(0);/* at this point all data must be transferred */
                }
            break;

            case MSC_BOT_STATUS_WAITCOMPLETE:
                g_usbh_msc_rx_event = 0u;
                g_scsi_command.st = 0u;
                g_msc_bot_state = MSC_BOT_IDLE;
            break;

            default:
                ASSERT(0);/*g_msc_bot_state must not be in any other state */
            break;
            }
        }
        else if (MSS_USB_EP_NAK_TOUT & status)
        {
            /* Device responding with NAKs. Retry */
            g_msc_state = USBH_MSC_BOT_RETRY;
        }
        else
        {
            ASSERT(0);/* Handling any other error. Not yet supported */
        }
    }

    return (USB_SUCCESS);
}

/*
 * This function validates the MSC class descriptors.
 */
mss_usbh_msc_err_code_t
MSS_USBH_MSC_validate_class_desc
(
    uint8_t* p_cd
)
{
    return(USBH_MSC_NO_ERROR);
}

/*
 * This function extract the endpoint information from the Config Descriptor.
 */
mss_usbh_msc_err_code_t
MSS_USBH_MSC_extract_tdev_ep_desc
(
    void
)
{
    mss_usbh_msc_err_code_t error = USBH_MSC_NO_ERROR;

    if (!(g_msd_conf_desc[21u] & USB_EP_DESCR_ATTR_BULK))      /* FirstEP Attributes Not BulkEP */
    {
        error = USBH_MSC_WRONG_DESCR;
    }
    if (!(g_msd_conf_desc[28u] & USB_EP_DESCR_ATTR_BULK))      /*SecondEP Attributes Not BulkEP */
    {
        error = USBH_MSC_WRONG_DESCR;
    }

    if (g_msd_conf_desc[20u] & USB_STD_REQ_DATA_DIR_MASK)     /* TdevEP is IN type */
    {
        g_tdev_in_ep.num = (g_msd_conf_desc[20u] & 0x7fu);
        g_tdev_in_ep.maxpktsz = (uint16_t)((g_msd_conf_desc[23u] << 8u) |
                                           (g_msd_conf_desc[22u]));

        g_tdev_out_ep.num = (g_msd_conf_desc[27u] & 0x7fu);
        g_tdev_out_ep.maxpktsz = (uint16_t)((g_msd_conf_desc[30u] << 8u) |
                                            (g_msd_conf_desc[29u]));

    }
    else if (g_msd_conf_desc[27u] & USB_STD_REQ_DATA_DIR_MASK)
    {
        g_tdev_in_ep.num = (g_msd_conf_desc[27u] & 0x7fu);
        g_tdev_in_ep.maxpktsz = (uint16_t)((g_msd_conf_desc[30u] << 8u) |
                                           (g_msd_conf_desc[29u]));

        g_tdev_out_ep.num = (g_msd_conf_desc[20u] & 0x7fu);
        g_tdev_out_ep.maxpktsz = (uint16_t)((g_msd_conf_desc[23u] << 8u) |
                                            (g_msd_conf_desc[22u]));
    }
    else
    {
        error = USBH_MSC_WRONG_DESCR;
    }

    return(error);
}

#endif /* MSS_USB_HOST_ENABLED */

#ifdef __cplusplus
}
#endif
