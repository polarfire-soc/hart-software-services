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
 * Microchip PolarFire SoC MSS USB Driver Stack
 *      USB Logical Layer (USB-LL)
 *          USBH-HID class driver.
 *
 * USBH-HID driver public API.
 *
 * SVN $Revision$
 * SVN $Date$
 */

/*=========================================================================*//**
  @mainpage PolarFire SoC MSS USB driver (USBH-HID)

  ==============================================================================
  Introduction
  ==============================================================================
  The Human Interface Devices host driver implements the USB host controller as
  per the USB HID class specified by the USB-IF. This driver enables easy
  detection and data transfers with the attached USB HID devices.
  The HID Class is used for low bandwidth data exchange.

  This driver implements the HID class using USB Interrupt Transfer. One
  Control and one BULK IN endpoints are used to implement HID class. This driver
  uses the USBH driver interface functions to implement the USB HID host.

  ==============================================================================
  Hardware Flow Dependencies
  ==============================================================================
  The configuration of USB Device mode features of the MSS USB is covered by
  this software stack. There are no dependencies on the hardware flow when
  configuring the PolarFire SoC MSS USB.
  The base address and register addresses are defined in this driver as
  constants. The interrupt number assignment for the MSS USB peripherals are
  defined as constants in the MPFS HAL. You must ensure that the latest MPFS HAL
  is included in the project settings of the SoftConsole tool chain and that it
  is generated into your project.

  ==============================================================================
  Theory of Operation
  ==============================================================================
  The following steps are involved in the operation of USBH-HID driver:
        - Configuration
        - Initialization
        - Application call-back interface
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
  The HID class driver must be initialized using the MSS_USBH_HID_init() function.
  A pointer to the structure of type mss_usbh_hid_user_cb_t must be provided as
  parameter with this function. This pointer is used to call the application
  call-back functions by the USBH-HID driver. These call-back functions can be
  used by the application to provide error/status messages to the user or for
  performing application specific handling of the events.

  --------------------------------
  Application call-back interface
  --------------------------------
  Application call-back interface
  After the USBH-Class driver is assigned to the attached device by the USBH
  driver, as indicated by usbh_class_allocate call-back, this driver can take
  over the communication with the attached device. This driver will then extend
  the enumeration process by requesting the class specific information from the
  device. During this process it will also call the call-back functions to
  indicate the progress of the enumeration process. These call-back functions
  must be implemented by application. These call-back functions can be used to
  take application specific action on specific event or can be used to provide
  error/status messages to the user. A type mss_usbh_hid_user_cb_t is provided
  by this driver which has all these call-back functions as its elements.
  Implementing these call-back functions is optional.

  --------------------------------
  Data transfer
  --------------------------------
  All data is exchanged between the host and the device in the form of reports.
  The format of the report is defined by the report descriptor defined by the
  device based on device need. USB HOST will request the report descriptor
  using interrupt transfers. Every HID device needs to have one input report
  in its report descriptor. An interrupt IN endpoint is required for reading
  reports from USB device. The interrupt OUT endpoint is optional.
  The HID class uses Interrupt Transfer mode, a maximum of 64 bytes can be
  transferred in a single frame (i.e., 64 Kbyte/s per endpoint when operating
  in Full-Speed mode).The USB Host driver needs to make sure that the data is
  polled periodically.

  This driver only needs to perform the USB IN transfers on the Interrupt
  endpoint. The period at which this transfer is repeated is defined by the
  device endpoint descriptor during the enumeration process. The application
  must call the USBH_HID_Handle() function periodically to read the report from
  the HID device. The application must register the call back function which
  will be called by the USBH driver when the report is received.

 *//*=========================================================================*/

#ifndef __MSS_USB_HOST_HID_H_
#define __MSS_USB_HOST_HID_H_

#include <stdint.h>
#include "mss_usb_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_HOST_ENABLED

/*-------------------------------------------------------------------------*//**
  Types exported from USBH-HID driver
  ============================
 */

/*-------------------------------------------------------------------------*//**
  The mss_usbh_HID_err_code_t provides a type to identify the error occurred
  during retrieving configuration and other class specific descriptors from the
  attached HID class device. This type can be used with hidh_error  call-back
  function element of mss_usbh_hid_user_cb_t type to identify the exact cause
  of the error. The meaning of the constants is as described below

  |          Constant             |                Description                   |
  |-------------------------------|----------------------------------------------|
  | USBH_HID_EP_NOT_VALID         | Indicates that the endpoint information      |
  |                               | retrieved from the attached device was not   |
  |                               | consistent with the HID class.               |
  |                               |                                              |
  | USBH_HID_CLR_CEP_STALL_ERROR  | Indicates that the host controller was not   |
  |                               | able to clear the stall condition on the     |
  |                               | attached device even after multiple retries. |

 */
typedef enum mss_usbh_hid_err_code
{
    USBH_HID_NO_ERROR = 0,
    USBH_HID_EP_NOT_VALID = -1,
    USBH_HID_CLR_CEP_STALL_ERROR = -2,
    USBH_HID_WRONG_DESCR = -3,

} mss_usbh_hid_err_code_t;

/*-------------------------------------------------------------------------*//**
  The mss_usbh_HID_state_t provides a type for the states of operation of the
  USBH-HID driver. Most of the states are internally used by the USBH-HID driver
  during the enumeration process. The USBH-HID driver is ready to perform data
  transfers with the attached device when the driver is in state
  USBH_HID_DEVICE_READY. The USBH_HID_ERROR state indicates that the error was
  detected either during enumeration or during the normal data transfer
  operations with the attached device even after retries.
 */
typedef enum mss_usbh_hid_state
{
    USBH_HID_IDLE,
    USBH_HID_GET_CLASS_DESCR,
    USBH_HID_WAIT_GET_CLASS_DESCR,

    USBH_HID_SET_CONFIG,
    USBH_HID_WAIT_SET_CONFIG,
    USBH_HID_WAIT_DEV_SETTLE,

    USBH_HID_REQ_GET_REPORT_DESC,
    USBH_HID_WAIT_REQ_GET_REPORT_DESC,
    USBH_HID_REQ_SET_IDLE,
    USBH_HID_WAIT_REQ_SET_IDLE,

    USBH_HID_REQ_GET_HID_DESC,
    USBH_HID_WAIT_GET_HID_DESC,

    USBH_HID_DEVICE_READY,

    USBH_HID_REQ_SET_PROTOCOL,
    USBH_HID_WAIT_REQ_SET_PROTOCOL,
    USBH_HID_DEVICE_RETRY,
    USBH_HID_ERROR

} mss_usbh_hid_state_t;

/*------------------------Public data structures-----------------------------*/
/*-------------------------------- USBH-HID----------------------------------*/
/*----------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*//**
  The mss_usbh_hid_user_cb_t provides the prototype for all the call-back
  functions which must be implemented by the user application. The user
  application must define and initialize a structure of this type and provide
  the address of that structure as parameter to the MSS_USBH_HID_init() function.

  hidh_valid_config
  The function pointed by the hidh_valid_config function pointer will be called
  to indicate that a valid HID class configuration was found on the attached
  device and the device is configured for this configuration.

  hidh_tdev_ready
  The function pointed by the hidh_tdev_ready function pointer is called when
  this driver is able to retrieve all the HID class specific information
  (including sector size and sector number) from the attached device and is
  ready to perform the data transfers.

  hidh_driver_released
  The function pointed by the hidh_driver_released function pointer is called to
  indicate to the application that this driver is released by the USBH driver.
  This could be either because the HID class device is now detached or there is
  permanent error with the USBH driver.

  hidh_error
  The function pointed by the hidh_error function pointer is called to indicate
  that there was an error while retrieving the class specific descriptor
  information from the attached HID class device. The error_code parameter
  indicates the exact cause of the error.

  hidh_decode
  The function pointed by the hidh_decode function pointer is called to indicate
  that there is a new report received from the attached HID class device. The
  data parameter indicates the location of received data present in the internal
  buffer.

 */
typedef struct mss_usbh_hid_user_cb
{
  void (*hidh_valid_config)(void);
  void (*hidh_tdev_ready)(void);
  void (*hidh_driver_released)(void);
  void (*hidh_error)(int8_t error_code);
  void (*hidh_decode)(uint8_t *data);

} mss_usbh_hid_user_cb_t;


/*----------------------------------------------------------------------------*/
/*----------------------MSS USBH-HID Public APIs------------------------------*/
/*----------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*//**
  The MSS_USBH_HID_init() function must be used by the application to initialize
  the USBH-HID driver. This function must be called before any other function of
  the USBH-HID driver.

  @param user_cb
    The user_cb parameter provides a pointer to the structure of type
    mss_usbh_hid_user_cb_t. This pointer is used to call the application
    call-back functions by the USBH-HID driver. These call-back functions can
    be used by the application to provide error/status messages to the user or
    for performing the application specific handling of the events.

  @return
    This function does not return a value.

  Example:
  @code
      MSS_USBH_init(&MSS_USBH_user_cb);
      MSS_USBH_HID_init(&MSS_USBH_HID_user_cb);
      MSS_USBH_register_class_driver(MSS_USBH_HID_get_handle());
  @endcode
*/
void
MSS_USBH_HID_init
(
    mss_usbh_hid_user_cb_t* user_sb
);

/***************************************************************************//**
  The MSS_USBH_HID_task() function is the main task of the USBH-HID driver.
  This function monitors the events from the USBH driver as well as the user
  application and makes decisions. This function must be called repeatedly by
  the application to allow the USBH-HID driver to perform the housekeeping
  tasks.A timer/scheduler can be used to call this function at regular intervals
  or it can be called from the main continuous foreground loop of the
  application.

  @param
    This function does not take any parameters.

  @return
    This function does not return a value.

  Example:
  @code
  @endcode
 */
void
MSS_USBH_HID_task
(
    void
);

/***************************************************************************//**
  The MSS_USBH_HID_get_handle() function must be used by the application to get
  the handle from the USBH-HID driver. This handle must then be used to register
  this driver with the USBH driver.

  @param
    This function does not take any parameters.

  @return
    This function returns a pointer to the class-handle structure.

  Example:
  @code
      MSS_USBH_init(&MSS_USBH_user_cb);
      MSS_USBH_HID_init(&MSS_USBH_HID_user_cb);
      MSS_USBH_register_class_driver(MSS_USBH_HID_get_handle());
  @endcode
 */
void*
MSS_USBH_HID_get_handle
(
    void
);

/***************************************************************************//**
  The MSS_USBH_HID_get_state() function can be used to find out the current
  state of the USBH-HID driver. This information can be used by the application
  to check the readiness of the USBH-HID driver to start the data transfers. The
  USBH-HID driver can perform data transfers only when it is in the
  USBH_HID_DEVICE_READY state.

  @param
    This function does not take any parameters.

  @return
    This function returns a value of type mss_usbh_hid_state_t indicating the
    current state of the USBH-HID driver.

  Example:
  @code
      if (USBH_HID_DEVICE_READY == MSS_USBH_HID_get_state())
      {
          *result = MSS_USBH_HID_get_sector_count();
          return RES_OK;
      }
      else if (USBH_HID_DEVICE_READY < MSS_USBH_HID_get_state())
      {
          *result = 0u;
          return RES_NOTRDY;
      }
  @endcode
 */
mss_usbh_hid_state_t
MSS_USBH_HID_get_state
(
    void
);

#endif  /* MSS_USB_HOST_ENABLED */

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_HOST_HID_H_ */

