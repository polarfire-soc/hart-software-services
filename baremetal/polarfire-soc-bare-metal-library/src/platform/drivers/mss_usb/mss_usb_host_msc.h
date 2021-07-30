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
 * Microchip PolarFire SoC MSS USB Host logical driver layer Implementation.
 *
 * Microchip PolarFire SoC MSS USB Driver Stack
 *      USB Logical Layer (USB-LL)
 *          USBH-MSC class driver.
 *
 * USBH-MSC driver public API.
 *
 *
 * SVN $Revision$
 * SVN $Date$
 */

/*=========================================================================*//**
  @mainpage PolarFire SoC MSS USB driver (USBH-MSC)

  ==============================================================================
  Introduction
  ==============================================================================
  The Mass Storage Class host driver implements the USB host controller as per
  the USB MSC class specified by the USB-IF. This driver enables easy detection
  and data transfers with the attached USB mass storage devices.

  This driver implements the MSC class using Bulk Only transport (BOT). One
  BULK IN and one BULK OUT endpoints are used to implement BOT. This MSC class
  host supports one LUN.

  This driver uses the USBH driver interface functions to implement the USB MSC
  host. This host controller will be able to work with all the USB flash drives
  which fall in "Memory stick" category.

  ==============================================================================
  Theory of Operation
  ==============================================================================
  The following steps are involved in the operation of USBH-MSC driver:
    - Configuration
    - Initialization
    - Enumeration
    - Class Specific requests
    - Data transfer

  --------------------------------
  Configuration
  --------------------------------
  The MSS USB driver must first be configured in the USB Host mode using the
  MSS_USB_HOST_MODE to use this driver. No other configuration is necessary.

  --------------------------------
  Initialization
  --------------------------------
  The MSC class driver must be initialized using the MSS_USBH_MSC_init() function.
  A pointer to the structure of type mss_usbh_msc_user_cb_t must be provided as
  parameter with this function. This pointer is used to call the application
  call-back functions by the USBH-MSC driver. These call-back functions can be
  used by the application to provide error/status messages to the user or for
  performing application specific handling of the events.

  --------------------------------
  Class Specific requests
  --------------------------------
  The class specific requests are handled internally by this driver. Both the
  GET_MAX_LUN and the RESET_RECOVERY requests are supported. The GET_MAX_LUN
  request is executed after the enumeration process to find out the number of
  LUNs on the attached device. This driver supports one LUN (index 0).

  --------------------------------
  Application call-back interface
  --------------------------------
  After the USBH-Class driver is assigned to the attached device by the USBH
  driver, as indicated by usbh_class_allocate call-back, this driver can take
  over the communication with the attached device. This driver will then extend
  the enumeration process by requesting the class specific information from the
  device. During this process it will also call the call-back functions to
  indicate the progress of the enumeration process. These call-back functions
  must be implemented by application. These call-back functions can be used to
  take application specific action on specific event or can be used to provide
  error/status messages to the user. A type mss_usbh_msc_user_cb_t is provided
  by this driver which has all these call-back functions as its elements.
  Implementing these call-back functions is optional.

  |           Element                      |     Call-back Event               |
  |----------------------------------------|-----------------------------------|
  | void (*msch_valid_config)(void)        | Called to indicate that a valid   |
  |                                        | MSC class configuration was found |
  |                                        | on the attached device and the    |
  |                                        | device is configured for this     |
  |                                        | configuration.                    |
  |                                        |                                   |
  | void (*msch_tdev_ready)(void)          | Called when this driver is able   |
  |                                        | to retrieve all the MSC class     |
  |                                        | specific info (including sector   |
  |                                        | size and sector number) from the  |
  |                                        | attached device and is ready to   |
  |                                        | perform data transfers.           |
  |                                        |                                   |
  | void (*msch_driver_released)(void)     | Called to indicate to the         |
  |                                        | application that this driver is   |
  |                                        | released by the USBH driver       |
  |                                        |                                   |
  | void (*msch_error)(int8_t error_code)  | Called to indicate that there was |
  |                                        | an error while retrieving the     |
  |                                        | class specific descriptor         |
  |                                        | information from the attached MSC |
  |                                        | class device.                     |

  --------------------------------
  Data transfer
  --------------------------------
  The MSC class driver performs the data transfers using one BULK IN endpoint
  and one BULK OUT endpoint. The data transfers use transparent SCSI commands
  and follow the BoT specification. The BoT read/write operations happen on
  logical units of fixed block size. During initialization this driver takes
  care of finding all information about the attached device. The
  MSS_USBH_MSC_get_sector_count() and MSS_USBH_MSC_get_sector_size() functions
  can be used to find out the sector count and the sector size on the attached
  MSC class device.

  Most of the times the read (READ_10) and write (WRITE_10) are the only SCSI
  operations that the application needs to do. The MSS_USBH_MSC_read() and
  MSS_USBH_MSC_write() functions are provided for this purpose. The application
  can use the MSS_USBH_MSC_scsi_req() function if it needs to perform any other
  SCSI operation. This function can be used for READ_10 and WRITE_10 commands as
  well. Once the SCSI request is initiated using any of the above functions,
  the MSS_USBH_MSC_is_scsi_req_complete() function can be used to find out when
  the operation is complete.

  The USBH driver supports multi-packet Bulk transfers. The USBH-MSC driver makes
  full use of this feature by passing on the multi-sector transfers from the
  application to the USBH driver. To free up the application from transferring
  data to/from MSS USB FIFO, this driver can configure USBH driver to use the
  MSS USB internal DMA.

  The MSS_USBH_MSC_construct_cbw_cb10byte() and MSS_USBH_MSC_construct_cbw_cb6byte()
  functions are provided so that the user can easily prepare the CBW format
  buffer by providing appropriate parameters instead of manually creating the
  CBW command buffer.

 *//*=========================================================================*/

#ifndef __MSS_USB_HOST_MSC_H_
#define __MSS_USB_HOST_MSC_H_

#include <stdint.h>
#include "mss_usb_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_HOST_ENABLED

/*-------------------------------------------------------------------------*//**
 Types exported from USBH-MSC driver
  ============================
 */
/*-------------------------------------------------------------------------*//**
  The mss_usbh_msc_err_code_t provides a type to identify the error occurred
  during retrieving configuration and other class specific descriptors from the
  attached MSC class device. This type can be used with msch_error  call-back
  function element of mss_usbh_msc_user_cb_t type to identify the exact cause
  of the error. The meaning of the constants is as described below

  |   Value                      | Description                                  |
  |----------------------------- |----------------------------------------------|
  | USBH_MSC_EP_NOT_VALID        | Indicates that the endpoint information      |
  |                              | retrieved from the attached device was not   |
  |                              | consistent with the MSC class.               |
  |                              |                                              |
  | USBH_MSC_CLR_CEP_STALL_ERROR | Indicates that the host controller was not   |
  |                              | able to clear the stall condition on the     |
  |                              | attached device even after multiple retries. |
  |                              |                                              |
  | USBH_MSC_SECTOR_SIZE_NOT_    | Indicates that the attached device sector    |
  | SUPPORTED                    | size is not supported by this driver.        |

 */
typedef enum {
    USBH_MSC_NO_ERROR = 0,
    USBH_MSC_EP_NOT_VALID = -1,
    USBH_MSC_CLR_CEP_STALL_ERROR = -2,
    USBH_MSC_SECTOR_SIZE_NOT_SUPPORTED = -3,
    USBH_MSC_WRONG_DESCR = -4,
} mss_usbh_msc_err_code_t;

/*-------------------------------------------------------------------------*//**
  The mss_usbh_msc_state_t provides a type for the states of operation of the
  USBH-MSC driver. Most of the states are internally used by the USBH-MSC driver
  during the enumeration process. The USBH-MSC driver is ready to perform data
  transfers with the attached device when the driver is in state
  USBH_MSC_DEVICE_READY. The USBH_MSC_ERROR state indicates that the error was
  detected either during enumeration or during the normal data transfer
  operations with the attached device even after retries.
 */
typedef enum {
    USBH_MSC_IDLE,
    USBH_MSC_GET_CLASS_DESCR,
    USBH_MSC_WAIT_GET_CLASS_DESCR,
    USBH_MSC_SET_CONFIG,
    USBH_MSC_WAIT_SET_CONFIG,
    USBH_MSC_WAIT_DEV_SETTLE,
    USBH_MSC_GET_MAX_LUN,
    USBH_MSC_WAIT_GET_MAX_LUN,
    USBH_MSC_CLR_CEP_STALL,
    USBH_MSC_WAIT_CLR_CEP_STALL,
    USBH_MSC_CONFIG_BULK_ENDPOINTS,

    USBH_MSC_TEST_UNIT_READY_CPHASE,
    USBH_MSC_TEST_UNIT_READY_SPHASE,
    USBH_MSC_TEST_UNIT_READY_WAITCOMPLETE,

    USBH_MSC_SCSI_INQUIRY_CPHASE,
    USBH_MSC_SCSI_INQUIRY_DPHASE,
    USBH_MSC_SCSI_INQUIRY_SPHASE,
    USBH_MSC_SCSI_INQUIRY_WAITCOMPLETE,

    USBH_MSC_SCSI_REQSENSE_CPHASE,
    USBH_MSC_SCSI_REQSENSE_DPHASE,
    USBH_MSC_SCSI_REQSENSE_SPHASE,
    USBH_MSC_SCSI_REQSENSE_WAITCOMPLETE,

    USBH_MSC_SCSI_READ_CAPACITY_CPHASE,
    USBH_MSC_SCSI_READ_CAPACITY_DPHASE,
    USBH_MSC_SCSI_READ_CAPACITY_SPHASE,
    USBH_MSC_SCSI_READ_CAPACITY_WAITCOMPLETE,

    USBH_MSC_DEVICE_READY,
    USBH_MSC_BOT_RETRY,
    USBH_MSC_ERROR
} mss_usbh_msc_state_t;

/*----------------------------------------------------------------------------*/
/*---------------Data structures exported by USBH-MSC driver   ---------------*/
/*----------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*//**
  The mss_usbh_msc_user_cb_t provides the prototype for all the call-back
  functions which must be implemented by the user application. The user
  application must define and initialize a structure of this type and provide
  the address of that structure as parameter to the MSS_USBH_MSC_init() function.

  msch_valid_config
  The function pointed by the msch_valid_config function pointer will be called
  to indicate that a valid MSC class configuration was found on the attached
  device and the device is configured for this configuration.

  msch_tdev_ready
  The function pointed by the msch_tdev_ready function pointer is called when
  this driver is able to retrieve all the MSC class specific information
  (including sector size and sector number) from the attached device and is
  ready to perform the data transfers.

  msch_driver_released
  The function pointed by the msch_driver_released function pointer is called to
  indicate to the application that this driver is released by the USBH driver.
  This could be either because the MSC class device is now detached or there is
  permanent error with the USBH driver.

  msch_error
  The function pointed by the msch_error function pointer is called to indicate
  that there was an error while retrieving the class specific descriptor
  information from the attached MSC class device. The error_code parameter
  indicates the exact cause of the error.
*/
typedef struct mss_usbh_msc_user_cb
{
  void (*msch_valid_config)(void);
  void (*msch_tdev_ready)(void);
  void (*msch_driver_released)(void);
  void (*msch_error)(int8_t error_code);
} mss_usbh_msc_user_cb_t;

/*-------------------------------------------------------------------------*//**
  The msd_cbw_t type provides the prototype for the Command Block Wrapper (CBW)
  as defined in Universal Serial Bus Mass Storage Class Bulk-Only Transport
  Revision 1.0. This type can be used by the application to create buffer of
  this type for creating the CBW which can then be passed on to functions
  provided by this driver as parameter.

  dCBWSignature
  Signature that helps identify this data packet as a CBW. The signature field
  shall contain the value 43425355h (little endian), indicating a CBW.

  dCBWTag
  A Command Block Tag sent by the host. The device shall echo the contents of
  this field back to the host in the dCSWTagfield of the associated CSW. The
  dCSWTagpositively associates a CSW with the corresponding CBW.

  dCBWDataTransferLength
  The number of bytes of data that the host expects to transfer on the Bulk-In
  or Bulk-Out endpoint (as indicated by the Direction-bit) during the execution
  of this command. If this field is zero, the device and the host shall transfer
  no data between the CBW and the associated CSW, and the device shall ignore
  the value of the Direction-bit in bmCBWFlags.

  bCBWFlags
  The bits of this field are defined as follows:

    | Bit        | Description                                                 |
    |------------|-------------------------------------------------------------|
    | Bit 7      | Direction- the device shall ignore this bit if the          |
    |            | dCBWDataTransferLengthfield is zero, otherwise:             |
    |            | 0 = Data-Out from host to the device,                       |
    |            | 1 = Data-In from the device to the host.                    |
    |            |                                                             |
    | Bit 6      | Obsolete. The host shall set this bit to zero.              |
    | Bits 5..0  | Reserved - the host shall set these bits to zero.           |

  bCBWLUN
  The device Logical Unit Number (LUN) to which the command block is being sent.
  For devices that support multiple LUNs, the host shall place into this field
  the LUN to which this command block is addressed. Otherwise, the host shall
  set this field to zero.

  bCBWCBLength
  The valid length of the CBWCBin bytes. This defines the valid length of the
  command block. The only legal values are 1 through 16 (01h through 10h).
  All other values are reserved.

  CBWCB[16]
  The command block to be executed by the device. The device shall interpret the
  first bCBWCBLength bytes in this field as a command block as defined by the
  command set identified by bInterfaceSubClass. If the command set supported by
  the device uses command blocks of fewer than 16 (10h) bytes in length, the
  significant bytes shall be transferred first, beginning with the byte at
  offset 15 (0Fh). The device shall ignore the content of the CBWCBfield past
  the byte at offset (15 + bCBWCBLength- 1)
 */
typedef struct {
 uint32_t dCBWSignature;
 uint32_t dCBWTag;
 uint32_t dCBWDataTransferLength;
 uint8_t bCBWFlags;
 uint8_t bCBWLUN;
 uint8_t bCBWCBLength;
 uint8_t CBWCB[16];
} msd_cbw_t;

/*-------------------------------------------------------------------------*//**
  The msd_csw_t type provides the prototype for the Command Status Wrapper (CSW)
  as defined in Universal Serial Bus Mass Storage Class Bulk-Only Transport
  Revision 1.0. This type can be used by application to create buffer of this
  type for creating CSW which can then be passed on to APIs provided by this
  driver as parameter.

  dCSWSignature
    Signature that helps identify this data packet as a CSW. The signature field
    shall contain the value 53425355h (little endian), indicating CSW.

  dCSWTag
    The device shall set this field to the value received in the dCBWTag of the
    associated CBW.

  dCSWDataResidue
    For Data-Out the device shall report in the dCSWDataResiduethe difference
    between the amount of data expected as stated in the dCBWDataTransferLength,
    and the actual amount of data processed by the device. For Data-In the device
    shall report in the dCSWDataResiduethe difference between the amount of data
    expected as stated in the dCBWDataTransferLengthand the actual amount of
    relevant data sent by the device. The dCSWDataResidueshall not exceed the
    value sent in the dCBWDataTransferLength.

  dCSWStatus
    bCSWStatusindicates the success or failure of the command. The device shall
    set this byte to zero if the command completed successfully. A non-zero value
    shall indicate a failure during command execution according to the following
    table:
    |    Value             |  Description                          |
    |----------------------|---------------------------------------|
    |   00h                | Command Passed ("good status")        |
    |   01h                | Command Failed                        |
    |   02h                | Phase Error                           |
    |   03h and 04h        | Reserved (Obsolete)                   |
    |   05h to FFh         | Reserved
 */
typedef struct {
 uint32_t dCSWSignature;
 uint32_t dCSWTag;
 uint32_t dCSWDataResidue;
 uint32_t dCSWStatus;
} msd_csw_t;

/*-------------------------------------------------------------------------*//**
  EXPORTED APIs from USBH-MSC driver
  ============================
 */
/*-------------------------------------------------------------------------*//**
  The MSS_USBH_MSC_init() function must be used by the application to initialize
  the USBH-MSC driver. This function must be called before any other function of
  the USBH-MSC driver.

  @param user_cb
    The user_cb parameter provides a pointer to the structure of type
    mss_usbh_msc_user_cb_t. This pointer is used to call the application
    call-back functions by the USBH-MSC driver. These call-back functions can be
    used by the application to provide error/status messages to the user or for
    performing the application specific handling of the events.

  @return
    This function does not return a value.

  Example:
  @code
      Initialize the USBH driver
      MSS_USBH_init(&MSS_USBH_user_cb);

      Initialize the USBH-MSC driver
      MSS_USBH_MSC_init(&MSS_USBH_MSC_user_cb);


      Get Class driver handle from the USBH-MSC class driver and register it with
      the USBH driver.
      On Device attachment, USBH driver will look for Device information through
      Descriptors and if match it with the Class driver using this Handle.
      MSS_USBH_register_class_driver(MSS_USBH_MSC_get_handle());
    @endcode
 */
void
MSS_USBH_MSC_init
(
    mss_usbh_msc_user_cb_t* user_sb
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_MSC_task() function is the main task of the USBH-MSC driver.
  This function monitors the events from the USBH driver as well as the user
  application and makes decisions. This function must be called repeatedly by
  the application to allow the USBH-MSC driver to perform the housekeeping tasks.
  A timer/scheduler can be used to call this function at regular intervals or
  it can be called from the main continuous foreground loop of the application.

  @param
    This function does not take any parameters.

  @return
    This function does not return a value.

  Example:
  @code
      #define SYS_TICK_LOAD_VALUE                             48000u
      int main()
      {
          Initialize SysTick
          SysTick_Config(SYS_TICK_LOAD_VALUE);
          NVIC_EnableIRQ(SysTick_IRQn);
      }
      void SysTick_Handler(void)
      {
          MSS_USBH_MSC_task();
      }
  @endcode
 */
void
MSS_USBH_MSC_task
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_MSC_get_handle() function must be used by the application to get
  the handle from the USBH-MSC driver. This handle must then be used to register
  this driver with the USBH driver.

  @param
    This function does not take any parameters.

  @return
    This function returns a pointer to the class-handle structure.

  Example:
  @code
      MSS_USBH_init(&MSS_USBH_user_cb);
      MSS_USBH_MSC_init(&MSS_USBH_MSC_user_cb);
      MSS_USBH_register_class_driver(MSS_USBH_MSC_get_handle());
    @endcode
*/
void*
MSS_USBH_MSC_get_handle
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_MSC_get_state() function can be used to find out the current state
  of the USBH-MSC driver. This information can be used by the application to check
  the readiness of the USBH-MSC driver to start the data transfers. The USBH-MSC
  driver can perform data transfers only when it is in the USBH_MSC_DEVICE_READY
  state.

  @param
    This function does not take any parameters.

  @return
    This function returns a value of type mss_usbh_msc_state_t indicating the
    current state of the USBH-MSC driver.

  Example:
  @code
      if(USBH_MSC_DEVICE_READY == MSS_USBH_MSC_get_state())
      {
          *result = MSS_USBH_MSC_get_sector_count();
          return RES_OK;
      }
      else if(USBH_MSC_DEVICE_READY < MSS_USBH_MSC_get_state())
      {
          *result = 0u;
          return RES_NOTRDY;
      }
  @endcode
 */
mss_usbh_msc_state_t
MSS_USBH_MSC_get_state
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_MSC_read() function can be used to read data from the attached
  mass storage device. This is a non-blocking function. The function prepares
  the MSS USB for the USB IN transfer. Once prepared, the MSS USB will start
  this IN transfer depending on the configuration of the IN pipe on which this
  transfer is occurring. The USBH-MSC driver takes care of the allocation and
  configuration of the IN pipe during enumeration process depending on the
  attached mass storage device. After preparing the IN pipe for the IN transfer,
  the MSS_USBH_MSC_is_scsi_req_complete() function can be used to find out the
  completion of the transfer.

  @param buf
    The buf parameter is a pointer to the buffer where the data received in the
    IN transfer from the attached MSC class device is stored.

  @param sector
    The sector parameter indicates the sector number (logical block address) on
    the mass storage device starting from which the data is to be read.

  @param count
    The count parameter indicates the number of sectors to be read.

  @return
    This function returns a zero value when execution was successful.

  Example:
  @code
      DRESULT disk_read (BYTE drv,
          BYTE *buff,
          DWORD sector,
          BYTE count)
      {
          if (0u != drv)
              return(RES_ERROR);
          if (USBH_MSC_DEVICE_READY < MSS_USBH_MSC_get_state())
          {
              return(RES_NOTRDY);
          }
          else if (USBH_MSC_DEVICE_READY == MSS_USBH_MSC_get_state())
          {
              MSS_USBH_MSC_read(buff, sector, count);
              while (MSS_USBH_MSC_is_scsi_req_complete());
              return (RES_OK);
          }
          else
              return (RES_ERROR);
      }
  @endcode
*/
int8_t
MSS_USBH_MSC_read
(
    uint8_t* buf,
    uint32_t sector,
    uint32_t count
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_MSC_write() function can be used to write data to the attached
  mass storage device. This is a non-blocking function. The function readies the
  MSS USB for the USB OUT transfer. Once ready, the MSS USB will start this OUT
  transfer depending on the configuration of the OUT pipe on which this transfer
  is occurring. The USBH-MSC driver takes care of the allocation and
  configuration of the OUT pipe during enumeration process depending on the
  attached mass storage device. After starting the OUT transfer on the specified
  OUT pipe, the MSS_USBH_MSC_is_scsi_req_complete() function can be used to find
  out the completion of the transfer.

  @param buf
    The buf parameter is a pointer to the buffer from which data need to be
    transmitted to the attached MSC class device.

  @param sector
    The sector parameter indicates the sector number (logical block address) on
    the mass storage device starting from which the data is to be written.

  @param count
    The count parameter indicates the number of sectors to be written.

  @return
    This function returns a zero value when execution was successful.

  Example:
  @code
      DRESULT disk_write (BYTE pdrv,
          const BYTE *buff,
          DWORD sector,
          BYTE count)
      {
          if (0u != pdrv)
              return(RES_ERROR);

          if (USBH_MSC_DEVICE_READY < MSS_USBH_MSC_get_state())
          {
              return(RES_NOTRDY);
          }
          else if (USBH_MSC_DEVICE_READY == MSS_USBH_MSC_get_state())
          {
              MSS_USBH_MSC_write((uint8_t*)buff, sector, count);
              while (MSS_USBH_MSC_is_scsi_req_complete());
              return (RES_OK);
          }
          else
              return (RES_ERROR);
      }
  @endcode
*/
int8_t
MSS_USBH_MSC_write
(
    uint8_t* buf,
    uint32_t sector,
    uint32_t count
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_MSC_get_sector_count() function can be used to find out the
  number of sectors (logical blocks) available on the attached MSC class device.

  @param
    This function does not take any parameters.

  @return
    This function returns a value of type uint32_t indicating the number of
    sectors (logical blocks) available on the attached MSC class device.

  Example:
  @code
      DRESULT disk_ioctl (
          BYTE pdrv,
          BYTE ctrl,
          void *buff)
      )
      {
          UINT *result = (UINT *)buff;

          switch (ctrl) {
          case GET_SECTOR_COUNT:
              if (USBH_MSC_DEVICE_READY == MSS_USBH_MSC_get_state())
              {
                  *result = MSS_USBH_MSC_get_sector_count();
                  return RES_OK;
              }
              else if (USBH_MSC_DEVICE_READY < MSS_USBH_MSC_get_state())
              {
                  *result = 0u;
                  return RES_NOTRDY;
              }
              else
              {
                  *result = 0u;
                  return RES_ERROR;
              }
              break;

          default:
              return RES_NOTRDY;
          }
      }
  @endcode
 */
uint32_t
MSS_USBH_MSC_get_sector_count
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_MSC_get_sector_size() function can be used to find out the size
  of a sector (in bytes) on the attached MSC class device.

  @param
    This function does not take any parameters.

  @return
    This function returns a value of type uint32_t indicating the sector size
    (in bytes) on the attached MSC class device.

  Example:
  @code
      DRESULT disk_ioctl (
      BYTE pdrv,
      BYTE ctrl,
      void *buff
      )
      {
          UINT *result = (UINT *)buff;

          switch (ctrl) {
          case GET_SECTOR_SIZE:
              if(USBH_MSC_DEVICE_READY == MSS_USBH_MSC_get_state())
              {
                  *result = MSS_USBH_MSC_get_sector_size();
                  return RES_OK;
              }
              else if(USBH_MSC_DEVICE_READY < MSS_USBH_MSC_get_state())
              {
                  *result = 0u;
                  return RES_NOTRDY;
              }
              else
              {
                  *result = 0u;
                  return RES_ERROR;
              }
          break;

          default:
              return RES_NOTRDY;
          }
      }
  @endcode
 */
uint32_t
MSS_USBH_MSC_get_sector_size
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_MSC_construct_cbw_cb10byte() function can be used to create the
  SCSI request command block wrapper (CBW) as per MSC class which has command
  block(CB) of length 10bytes.

  @param command_opcode
    The command_opcode parameter provides the transparent SCSI command code.

  @param lun
    The lun parameter indicates the logical unit number on the attached MSC
    class device.

  @param lb_addr
    The lb_addr parameter provides the logical block address on which the
    operation indicated by the command_opcode parameter is applicable. For the
    commands where logical block address is not applicable, a zero value must
    be provided.

  @param num_of_lb
    The num_of_addr parameter provides the number of logical blocks on which
    the operation indicated by the command_opcode parameter is applicable. For
    the commands where logical block address is not applicable, a zero value
    must be provided.

  @param lb_size
    The lb_size parameter provides the size of the logical block on the attached
    MSC class device.

  @param buf
    The buf parameter provides the pointer to the buffer where the formatted SCSI
    command is to be stored.

  @return
    This function does not return any value.

  Example:
  @code
      int8_t
      MSS_USBH_MSC_read(uint8_t* buf, uint32_t sector, uint32_t count)
      {
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
          return(0);
      }
  @endcode
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
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_MSC_construct_cbw_cb6byte() function can be used to create the
  SCSI request command block wrapper (CBW) as per MSC class which has command
  block(CB) of length 6 bytes.

  @param command_opcode
    The command_opcode parameter provides the transparent SCSI command code.

  @param xfr_length
    The xfr_length parameter provides the number of bytes to be transferred in
    the data phase of the command.

  @param buf
    The buf parameter provides the pointer to the buffer where the formatted
    SCSI command is to be stored.

  @return
    This function does not return any value.

  Example:
  @code
      MSS_USBH_MSC_construct_cbw_cb6byte(USB_MSC_SCSI_TEST_UNIT_READY,
                                         0u,
                                         &g_bot_cbw);

      MSS_USBH_write_out_pipe(g_msd_tdev_addr,
                              USBH_MSC_BULK_TX_PIPE,
                              g_tdev_out_ep.num,
                              g_tdev_out_ep.maxpktsz,
                              (uint8_t*)&g_bot_cbw,
                              31u);
  @endcode
 */
void
MSS_USBH_MSC_construct_cbw_cb6byte
(
    uint8_t command_opcode,
    uint32_t data_xfr_len,
    msd_cbw_t* buf
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_MSC_scsi_req() function can be used to execute any SCSI command
  required by the MSC class  on the attached MSC device. In most cases using
  the MSS_USBH_MSC_write() and MSS_USBH_MSC_read() functions is enough for the
  application. However, if the application wants to execute other SCSI commands
  it can use the MSS_USBH_MSC_scsi_req() function. This function can be used as
  alternative to MSS_USBH_MSC_write() and MSS_USBH_MSC_read() functions.

  The MSS_USBH_MSC_is_scsi_req_complete() function can be used to find out when
  the transfer started using this function is complete.

  @param command_buf
    The command_buf parameter provides the pointer to the buffer where the SCSI
    command (CBW format) to be executed is stored.

  @param data_buf
    The data_buf parameter provides the pointer to the data buffer which is to
    be used in the data phase of the command. This parameter is the source of
    the data when the data direction is from the host to the device. This
    parameter is the destination for the data when the data direction is from
    the device to the host. This function extracts the data direction from the
    CBW format command provided using command_buf parameter.

  @param data_buf_len
    The data_buf_len parameter indicates the number of bytes to be transferred
    in the data phase of the current command.

  @param status_buf
    The status_buf parameter provides the pointer to the buffer where the status
    (CSW format) received from the attached MSC device for the current SCSI
    operation is to be stored.

  @return
    This function returns zero value when successfully executed.

  Example:
  @code
      int8_t
      MSS_USBH_MSC_read
      (
          uint8_t* buf,
          uint32_t sector,
          uint32_t count
      )
      {
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
          return(0);
      }
  @endcode
 */
uint8_t
MSS_USBH_MSC_scsi_req
(
    uint8_t* command_buf,   /* always31bytes */
    uint8_t* data_buf,
    uint32_t data_buf_len,
    uint8_t* status_buf     /* status always 13bytes */
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_MSC_is_scsi_req_complete() function must be used to find out
  whether the SCSI request initiated using MSS_USBH_MSC_scsi_req() or
  MSS_USBH_MSC_read() or MSS_USBH_MSC_write() function is complete.

  @param
    This function does not take any parameters.

  @return
    This function returns zero value when the current command is completed.

  Example:
  @code
      MSS_USBH_MSC_write((uint8_t*)buff, sector, count);
      while (MSS_USBH_MSC_is_scsi_req_complete());
      return (RES_OK);
  @endcode
 */
uint8_t
MSS_USBH_MSC_is_scsi_req_complete
(
    void
);

#endif  /* MSS_USB_HOST_ENABLED */

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_HOST_MSC_H_ */

