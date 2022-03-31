/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * PolarFire SoC MSS USB Host logical driver layer Implementation.
 *
 * PolarFire SoC MSS USB Driver Stack
 *      USB Logical Layer (USB-LL)
 *          USBD-MSC class driver.
 *
 *  USBD-MSC class driver public API.
 *
 */

/*=========================================================================*//**
  @mainpage PolarFire SoC MSS USB driver (USBD-MSC)

  ==============================================================================
  Introduction
  ==============================================================================
  The Mass Storage Class device driver implements the USB MSC device as specified
  by the USB-IF. This driver enables easy implementation of mass storage drive
  functionality on the Smarfusion2 devices.

  This driver implements the MSC class using Bulk Only transport (BOT).
  One BULK IN and one BULK OUT endpoints are used to implement BOT. This driver
  is independent of the storage medium used for storing the data and the number
  of Logical Units (LUN) it supports.

  This driver uses the USBD-Class driver template to implement the USB MSC
  device.

  This driver passes the USB-IF USB2.0 compliance test for high speed MSC class
  device.

  ==============================================================================
  Theory of Operation
  ==============================================================================
  The following steps are involved in the operation of the USBD-MSC driver:
    - Configuration
    - Initialization
    - Enumeration
    - Class Specific requests
    - Data transfer

  --------------------------------
  Configuration
  --------------------------------
  To use this driver, the MSS USB driver must first be configured in the USB
  device mode using the MSS_USB_PERIPHERAL_MODE. No other configuration is
  necessary.

  --------------------------------
  Initialization
  --------------------------------
  The MSC class driver must be initialized using the MSS_USBD_MSC_init() function.
  Once initialized, this driver gets configured by the USBD driver during the
  enumeration process. The Call-back function usbd_msc_init_cb() is
  implemented by this driver which will be called by the USBD driver when the
  host configures the this device. The usbd_msc_get_descriptor_cb() function
  is implemented to provide the class specific descriptor table to the USBD driver.

  --------------------------------
  Class Specific requests
  --------------------------------
  This driver defines descriptor table which contains a configuration descriptor,
  an interface descriptor, two endpoint descriptors for successful enumeration
  as a MSC class device.

  Note: For successful enumeration, the device specific descriptors must also be
  provided by the application using the MSS_USBD_set_desc_cb_handler()
  function to the USBD Driver. Since the device descriptor, string descriptors etc.
  are not class specific, they are not part of the MSC class driver.

  Class Specific requests
  The usbd_msc_process_request_cb() call-back function is implemented by
  this driver which processes the MSC class specific requests. This driver may
  not have all the information related to the storage medium e.g. storage
  capacity etc. To process such requests; they are passed on to the application
  by calling appropriate call-back functions. The application must implement
  these call-back functions for the driver to successfully respond to the host
  requests. Refer data structures section for more information.

  --------------------------------
  Data transfer
  --------------------------------
  The MSC class driver performs the data transfers using one BULK IN endpoint
  and one BULK OUT endpoint. The data transfers follow the BoT protocol. This
  driver implements the usbd_msc_tx_complete_cb() and the usbd_msc_tx_complete_cb()
  function to get the information on data transfer events on the USB bus which
  are called by the USBD Driver.

  The BoT read/write operations happen on logical units of fixed block sizes.
  This driver translates these read write operation into memory read/write
  operation and calls the call-back function where application can perform the
  memory read/write operations. This method makes the MSC class driver
  independent of the storage medium being used. The call-back function elements
  media_acquire_write_buf, media_write_ready and media_read (as part of structure
  of type mss_usbd_msc_media_t) must be implemented by the application.

*//*==========================================================================*/

#ifndef __MSS_USB_DEVICE_MSD_H_
#define __MSS_USB_DEVICE_MSD_H_

#include <stdint.h>
#include "mss_usb_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_DEVICE_ENABLED
/*******************************************************************************
 * User Descriptor lengths
 */

/* Full configuration descriptor length*/
#define FULL_CONFIG_DESCR_LENGTH                    (USB_STD_CONFIG_DESCR_LEN + \
                                                     USB_STD_INTERFACE_DESCR_LEN + \
                                                     USB_STD_ENDPOINT_DESCR_LEN + \
                                                     USB_STD_ENDPOINT_DESCR_LEN )

/***************************************************************************//**
 Exported Types from USBD-MSC class driver
 */
/***************************************************************************//**
  mss_usbd_msc_state_t
  The mss_usbd_msc_state_t provides a type to identify the current state of the
  MSC class driver.
    USBD_MSC_NOT_CONFIGURED - The USB MSC class driver is not configured and it
                              cannot perform the data transfers.
    USBD_MSC_CONFIGURED    -  The USB MSC class driver is configured by the host
                              and it can perform the data transfers.

  Note:    The application must not access the flash media (on board SPI Flash
  in this case) while the MSC class driver is in USBD_MSC_CONFIGURED state
  because the USB host might be performing read/write operation on the flash
  media. If the application also attempts to access the flash media at the same
  time, the data might get corrupted and the host will get misleading information
  from the flash media.
*/
typedef enum mss_usbd_msc_state {
    USBD_MSC_NOT_CONFIGURED,
    USBD_MSC_CONFIGURED,
}mss_usbd_msc_state_t;

/***************************************************************************//**
 mss_usbd_msc_media_t
  The mss_usbd_msc_media_t provides the prototype for all the call-back
  functions which must be implemented by the user application. The user
  application must define and initialize a structure of this type and provide
  the address of that structure as parameter to the MSS_USBD_MSC_init() function.

  media_init
  The function pointed by the media_init function pointer will be called to
  indicate that the MSC driver is configured by the USB host. The application
  can use this function to prepare itself for data exchange with the host.
  This driver cannot perform data transfers unless it is configured by the USB
  host to do so.

  media_get_capacity
  The function pointed to by the media_get_capacity function pointer is called
  to find out the capacity of the LUN on the storage medium. The parameter lun
  indicates the logical unit on which the current operation is being performed.
  The application must return address of the last logical block and the size of
  the logical block in the parameters last_block_lba and block_size respectively.

  media_is_ready
  The function pointed by the media_is_ready function pointer is called to find
  out if the LUN on the media is ready for the data transfers. The parameter lun
  indicates the logical unit on which the current operation is being performed.

  media_is_write_protected
  The function pointed by the media_is_write_protected function pointer is called
  to find out if the LUN on the media is write-protected, before a write operation
  can be performed on the LUN. The parameter lun indicates the logical unit on
  which the current operation is being performed.

  media_read
  The function pointed by the media_read function pointer is called when the
  host wants to read the data from the storage medium. The application must
  provide a buffer and its length which can be sent to the host. The parameter
  lun indicates the logical unit on which the current operation is being performed.
  The blk_addr parameter provides the logical block address and the len
  parameter provides the number of bytes to be read. The application must provide
  a buffer in return parameter buf and the length of this buffer must be provided
  as the return value of this function.

  media_acquire_write_buffer
  The function pointed by the msd_media_acquire_write_buffer function pointer is
  called when the host wants to write data on to the storage medium. The
  application must provide a buffer where the data sent by the host can be stored.
  The parameter lun indicates the logical unit on which the current operation is
  being performed. The blk_addr parameter provides the logical block address
  where the data is to be written. The application must provide a buffer as a
  return value of this function and the length of the buffer must be provided
  in the return parameter length.

  media_write_ready
  The function pointed by the media_write_ready function pointer is called when
  the data to be written is received from the host and is ready to be written on
  the storage medium. The data is stored in the previously provided write buffer
  using media_acquire_write_buffer. The parameter lun indicates the logical unit
  on which the current operation is being performed. The blk_addr parameter
  provides the logical block address where data is to be written. The parameter
  length provides the number of bytes to be written.

  media_get_max_lun
  The function pointed by the media_get_max_lun function pointer is called to
  find out the maximum number of logical units supported by the application or
  the storage medium.

  media_inquiry
  The function pointed by the media_inquiry function pointer is called to get
  the response for the INQUIRY request from the MSC class host. The parameter
  lun indicates the logical unit on which the current operation is being performed.
  The application must provide a buffer as a return value of this function and
  must provide the length of this buffer in the parameter length.

  media_release
  The function pointed by the media_inquiry function pointer is called to
  indicate that the either the MSC class device has been un-configured or is
  disconnected from the host. The MSC class device is un-configured when it
  receives SET_CONFIGURATION request from USB host with a cfgidx = 0. This value
  is passed as a parameter. In case when the disconnect event is detected by the
  USBD driver a value of cfgidx = 0xFF is passed. The application can use this
  call-back function and its parameter to take appropriate action as required.
 */

typedef struct mss_usbd_msc_media {
    uint8_t (*media_init)(uint8_t lun);
    uint8_t (*media_get_capacity)(uint8_t lun,
                                  uint32_t *last_block_lba,
                                  uint32_t *block_size);

    uint8_t (*media_is_ready)(uint8_t lun);
    uint8_t (*media_is_write_protected)(uint8_t lun);
    uint32_t(*media_read)(uint8_t lun,
                          uint8_t **buf,
                          uint64_t blk_addr,
                          uint32_t len);

    uint8_t*(*media_acquire_write_buf)(uint8_t lun,
                                       uint64_t blk_addr,
                                       uint32_t *len);

    uint32_t(*media_write_ready)(uint8_t lun,
                                 uint64_t blk_addr,
                                 uint32_t len);

    uint8_t (*media_get_max_lun)(void);
    uint8_t*(*media_inquiry)(uint8_t lun, uint32_t *len);
    uint8_t (*media_release)(uint8_t cfgidx);

} mss_usbd_msc_media_t;

/***************************************************************************//**
 Types Internal to the USBD-MSC class driver
 */
typedef enum mss_usbd_msc_bot_events {
    BOT_EVENT_IDLE,
    BOT_EVENT_TX,
    BOT_EVENT_RX,
    BOT_EVENT_TX_ERROR,
    BOT_EVENT_RX_ERROR
} mss_usbd_msc_bot_events_t;

typedef enum mss_usbd_msc_csw_status {
    SCSI_COMMAND_PASS,
    SCSI_COMMAND_FAIL,
    SCSI_COMMAND_PHASE_ERR,
    SCSI_COMMAND_LESSDATAPASS,
} mss_usbd_msc_csw_status_t;

typedef enum mss_usbd_msc_bot_state {
    BOT_IDLE,
    BOT_DATA_RX,
    BOT_DATA_TX,
    BOT_SEND_STATUS,
    BOT_ABORTED
} mss_usbd_msc_bot_state_t;

/*  Command Block Wrapper (CBW) */
typedef struct mss_usb_msc_cbw {
    uint32_t signature;
    uint32_t tag;
    uint32_t xfr_length;
    uint8_t flags;
    uint8_t lun;
    uint8_t cmd_length;
    uint8_t cmd_block[16];
} mss_usb_msc_cbw_t;

/* Command Status Wrapper (CSW) */
typedef struct mss_usbd_msc_csw {
    uint32_t signature;
    uint32_t tag;
    uint32_t data_residue;
    mss_usbd_msc_csw_status_t status;
} mss_usbd_msc_csw_t;

/* SCSI inquiry response */
typedef struct mss_usbd_msc_scsi_inq_resp {
    uint8_t peripheral;
    uint8_t removable;
    uint8_t version;
    uint8_t resp_data_format;
    uint8_t additional_length;
    uint8_t sccstp;
    uint8_t bqueetc;
    uint8_t cmd_que;
    uint8_t vendor_id[8];
    uint8_t product_id[16];
    uint8_t product_rev[4];
} mss_usbd_msc_scsi_inq_resp_t;

typedef struct mss_usbd_msc_scsi_sense_resp {
    const uint8_t response_code;    /*0x70 - spc3-table12*/
    const uint8_t obsolete;
    uint8_t sense_key;              /*Table 27 - spc3-4.5.6*/
    const uint8_t info[4];
    const uint8_t additional_length;
    const uint8_t command_info[4];
    uint8_t asc;                    /*Table28 - spc3-4.5.6*/
    const uint8_t unused[5];
} mss_usbd_msc_scsi_sense_resp_t;

typedef struct mss_usbd_msc_lun_capacity {
    uint8_t last_lba_msb;
    uint8_t last_lba_2;
    uint8_t last_lba_1;
    uint8_t last_lba_lsb;
    uint8_t block_size_msb;
    uint8_t block_size_2;
    uint8_t block_size_1;
    uint8_t block_size_lsb;
    uint32_t blk_sz_len;
} mss_usbd_msc_lun_capacity_t;

/***************************************************************************//**
 Exported functions from USBD-MSC class driver
 */

/***************************************************************************//**
  @brief MSS_USBD_MSC_init()
  The MSS_USBD_MSC_init() function must be used by the application to initialize
  the MSC class driver. A pointer to the structure of type mss_usbd_msc_media_t
  must be passed as a parameter to this function.

  @param media_ops
  The media_ops parameter is a pointer to the structure of type
  mss_usbd_msc_media_t. This is used by the MSC class driver to call the
  call-back functions implemented by the application.

  @param speed
  The speed parameter indicates the USB speed at which this class driver must
  operate.

  @return
    This function does not return a value.

  Example:
  @code
        //Assign call-back function handler structure needed by MSD class driver
        MSS_USBD_MSC_init(&usb_flash_media, MSS_USB_DEVICE_HS);

        //Assign call-back function handler structure needed by USB Device Core driver
        MSS_USBD_set_desc_cb_handler(&flash_drive_descr_cb);

        //Initialize USB driver HS device mode
        MSS_USBD_init(MSS_USB_DEVICE_HS);

  @endcode
*/
void
MSS_USBD_MSC_init
(
    mss_usbd_msc_media_t* media_ops,
    mss_usb_device_speed_t speed
);

/***************************************************************************//**
  @brief USBD_get_state()
  The MSS_USBD_MSC_get_state() function can be used by the application to find
  out the current state of the MSC class driver.

  @param media_ops
    This function does not take a parameter.

  @return
    This function returns a value of type mss_usbd_msc_state_t indicating the
    current state of the MSC class driver.

  Example:
  @code
        uint8_t
        usb_flash_media_get_capacity
        (
            uint8_t lun,
            uint32_t *no_of_blocks,
            uint32_t *block_size
        )
        {
           if(USBD_MSC_CONFIGURED == MSS_USBD_MSC_get_state())
           {
              *no_of_blocks = lun_data[lun].number_of_blocks;
              *block_size = lun_data[lun].lba_block_size;
              return 1;
           }
           Else
           {
              Return 0;
           }
        }

  @endcode
*/
mss_usbd_msc_state_t
MSS_USBD_MSC_get_state
(
    void
);

#endif  //MSS_USB_DEVICE_ENABLED

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_DEVICE_MSD_H_ */
