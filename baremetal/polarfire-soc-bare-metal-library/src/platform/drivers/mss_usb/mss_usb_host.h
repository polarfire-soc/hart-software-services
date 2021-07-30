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
 *          USBH driver
 *
 *  USBH driver public API.
 *
 * SVN $Revision$
 * SVN $Date$
 */
/*=========================================================================*//**
  @mainpage PolarFire SoC MSS USB Host driver (USBH)

  ==============================================================================
  Introduction
  ==============================================================================
  The USB Host Driver (USBH) is part of the USB-LL layer of the MSS USB driver
  stack. The USBH driver implements the core functionality of the USB host mode
  operations. The USBH driver is responsible for the following functionalities.
    - Device attach and detach handling
    - Enumeration and USB Standard requests
    - Device Suspend, Resume and Reset handling
    - USB transfers management
    - USBH-Class call-back interface
    - Application call-back interface

  ==============================================================================
  Theory of Operation
  ==============================================================================
  The following steps are involved in the operation of the USBH driver:
    - Configuration
    - Initialization
    - USBH-Class Interface
    - Application Interface
    - Data transfer

  The USBH driver operation depends on user configuration provided in
  mss_usb_config.h.

  --------------------------------
  Configuration
  --------------------------------
  The following parameter must be defined in the mss_usb_config.h file to
  operate the MSS USB hardware block in USB Host mode.

  MSS_USB_HOST_MODE   Configures the MSS USB Driver Stack to operate in USB
  host mode.

  --------------------------------
  Initialization
  --------------------------------
  The USBH driver must be initialized by calling the MSS_USBH_init()
  initialization function with a parameter to provide the application call-back
  interface. Once initialized the USBH driver is ready to detect the attach
  event on the USB port. The application call-back interface is used to provide
  information about the events on the USB bus to the application. The
  MSS_USBH_task() is the main task of the USBH driver which implements the state
  machines for monitoring the USB bus activities. This function must be called
  repeatedly by the application to allow USBH driver to run it's state machines.
  A timer/scheduler can be used to call this function at regular intervals or it
  can be called from the main continuous foreground loop of the application.
  The function MSS_USBH_1ms_tick() must be called to provide a 1milisecond time
  tick to the USBH driver. This time tick is used by the USBH driver to track
  the enumeration process and other housekeeping purposes. The
  MSS_USBH_get_milis() function is provided to get the number of milliseconds
  lapsed from the system reset. The MSS_USBH_test_mode() function is provided to
  force the MSS USB in test modes as required by the USB compliance test
  specification. This function has no use in normal operations with a attached
  USB device.

  --------------------------------
  USBH-Class Call-back Interface
  --------------------------------
  This driver encapsulates the generic USB protocol functionality from the USB
  class functionality. This driver needs the USBH-Class driver to implement a
  set of call-back functions to pass on the control to the USBH-Class driver for
  handling the USB class specific functionality. This driver provides a data
  type mss_usbh_class_cb_t which must be implemented by the USBH-Class driver.
  This data type is the collection of call-back functions which must be
  implemented by the USBH-Class driver. Implementing all the elements of this
  structure may not be necessary for a specific USB class. The USBH-Class driver
  must define a structure of type mss_usbh_class_cb_t and implement its required
  elements. The function MSS_USBH_register_class_driver() must be used to inform
  the USBH driver about this call-back interface and the supported USB Class
  information
  The table below lists the prototypes of the call-back functions which are part
  of the mss_usbh_class_cb_t   type and the event in which they are called.

  | Call-back Element      | Event                                             |
  |------------------------|---------------------------------------------------|
  | usbh_class_allocate    | Called when a match is found between the class    |
  |                        | sub-class code and protocol code of the attached  |
  |                        | device and the corresponding parameters provided  |
  |                        | by the USBH-Class driver in the class_id parameter|
  | usbh_class_release     | Called when the device for which this USBH-Class  |
  |                        | driver is assigned is detached                    |
  | usbh_class_cep_xfr_done| Called when a transfer started by this USBH-Class |
  |                        | driver is complete                                |
  | usbh_class_tx_done     | Called when a transfer started on the OUT pipe by |
  |                        | this USBH-Class driver is complete                |
  | usbh_class_rx_done     | Called when a transfer started on the IN pipe by  |
  |                        | this USBH-Class driver is complete                |
  | usbh_class_sof         | The function pointed by the usbh_class_sof element|
  |                        | will be called when a new (micro) frame starts    |

  --------------------------------
  Application Call-back Interface
  --------------------------------
  The USBH driver uses interrupts to respond to the events on MSS USB. The
  USBH driver handles the enumeration of the attached device internally.
  The enumeration process can take long time (hundreds of milliseconds) to
  complete. The USBH driver uses application call-back interface to inform the
  application about the events on the USB bus as they happen. The application
  may implement these call-back functions to take application specific action
  on each event or to provide error/status messages to the user.

  The table below lists the prototypes of the call-back functions which are
  part of the mss_usbh_user_cb_t type and the event in which they are called.

  | Element                         | Call-back Event                          |
  |---------------------------------|------------------------------------------|
  | usbh_tdev_attached              | Called when a device-attach event is     |
  |                                 | detected by the USBH driver. At this     |
  |                                 | point the device is not enumerated.      |
  | usbh_tdev_dettached             | Called when a device-detach event is     |
  |                                 | detected by the USBH driver              |
  | usbh_tdev_oc_error              | Called when an over-current condition is |
  |                                 | detected by the USBH driver              |
  | usbh_tdev_enumerated            | Called when the attached device is       |
  |                                 | successfully enumerated                  |
  | usbh_tdev_class_driver_assigned | Called when a matching class driver is   |
  |                                 | assigned to the attached device          |
  | usbh_tdev_not_supported         | Called when a device which is not        |
  |                                 | supported by the USBH driver is detected |
  | usbh_tdev_permanent_error       | Called if there was error during data    |
  |                                 | transfer operations with the attached    |
  |                                 | device                                   |

  --------------------------------
  Data transfer
  --------------------------------
  The USB host uses pipe, a representation of logical connection from endpoint
  on MSS USB to the endpoint on the target device. The transmit pipe or the
  receive pipe must first be configured before starting the data transfer on it.
  The functions MSS_USBH_configure_in_pipe() and MSS_USBH_configure_out_pipe()
  must be used to configure an IN(data moving from device to host) and an
  OUT(data moving from host to device) pipe respectively.

  The application must use MSS_USBH_write_out_pipe() function to start data
  transmission on a OUT pipe. This function prepares the MSS USB to transmit
  data to the device. However, actual transmission happens depending on the
  transfer type and the selected interval. This function is non-blocking.
  The USBH-Class driver will be informed about data transmission completion or
  transmission errors by a call to its call-back function. Please refer
  USBH-Class Interface section above for more details.

  To receive data from attached USB device, application must use
  MSS_USBH_read_in_pipe() function. This function prepares the MSS USB to
  receive data from the attached device. However, actual data transfer happens
  depending on the transfer type and the selected interval. This function is
  non-blocking. On receiving data or in case of errors during this operation
  the USBH-Class driver will be informed about it by calling a call-back
  function. Please refer the USBH-Class Interface section above for more
  details.

  This driver supports multi-packet Bulk transfer (both IN and OUT), to make
  sure that it is able to use the full bandwidth offered by USB Bulk transfers.
  When a transfer size of more than wMaxPktSize is provided to this driver using
  MSS_USBH_write_out_pipe() function or MSS_USBH_read_in_pipe() function, this
  driver handles this multi-packet transfer internally by dividing it into
  multiple Bulk transfers of wMaxPktSize each. This way no application
  intervention is required till the total multi-packet Bulk transfer is
  complete.

  The USBH driver supports Bulk transfers using the DMA inside MSS USB or
  without using the internal DMA. It is advised that the USBH driver is
  configured to use the internal DMA in order to free up the application from
  transferring data to/from MSS USB hardware block. However, the internal DMA
  can access only aligned address. Care must be taken that the buffer address
  provided to the USBH driver must be modulo-4. This is not a requirement when
  the MSS USB driver is configured not to use internal DMA.

  All the control transfers happen on the control pipe. The control pipe is
  shared by the USBH driver and the USBH-Class driver. When the USBH-Class
  driver needs to perform control transfer on the attached device, it must first
  configure the control pipe using the MSS_USBH_configure_control_pipe()
  function. The MSS_USBH_start_control_xfr() function can be used to start the
  control transfer After the control pope is configured. The data direction is
  determined by a parameter provided with this function. The USBH-Class driver
  will be informed about the data transfer completion or the error status by
  calling a call-back function. Please refer the USBH-Class Interface section
  above for more details. The control transfer can be started by a USBH-Class
  driver only when the control pipe is free.The MSS_USBH_get_cep_state()
  function can be used to know the current state of the control pipe. At the
  start of the control transfer the USBH driver must send a setup-packet to the
  attached device in the setup phase. The function
  MSS_USBH_construct_get_descr_command() can be used to construct the setup
  packet as per the USB specification for the transfer to be performed. This
  function formats the setup packet depending on the parameters provided with
  this function.

  The application may choose to keep the MSS USB in suspend state to conserve
  power to the USB device, and the MSS USB, In this state the MSS USB does not
  send any frames on the bus and there will be no bus activity. This will also
  ensure that the attached device enters suspend state as specified in the
  USB2.0 specification. The USBH driver provides the MSS_USBH_suspend() function
  to suspend the MSS USB. The  MSS_USBH_resume() function must be used to
  resume the normal operations.

  The table below provides the list of all the functions provided by USBH
  driver.

  | function                     | Description                                 |
  |------------------------------|---------------------------------------------|
  | MSS_USBH_init()              | Initialize the MSS USB to operate in the    |
  |                              | host mode                                   |
  |                              |                                             |
  | MSS_USBH_task()              | Main task of the USBH driver where the state|
  |                              | machines are implemented                    |
  |                              |                                             |
  | MSS_USBH_register_class_     | Used to register the class driver handle    |
  | driver()                     | with the USBH driver                        |
  |                              |                                             |
  | MSS_USBH_configure_control_  | Configures the control pipe for control     |
  | pipe()                       | transfers with the attached device          |
  |                              |                                             |
  | MSS_USBH_configure_in_pipe() | Configures the IN pipe for the USB IN       |
  |                              | transfers with the attached device          |
  |                              |                                             |
  | MSS_USBH_configure_out_pipe()| Configures the OUT pipe for the USB OUT     |
  |                              | transfers with the attached device          |
  |                              |                                             |
  | MSS_USBH_write_out_pipe()    | Writes data on to the attached device using |
  |                              | the OUT pipe                                |
  |                              |                                             |
  | MSS_USBH_read_in_pipe()      | Reads data from the attached device using   |
  |                              | the IN pipe                                 |
  |                              |                                             |
  | MSS_USBH_get_tdev_state()    | Used to find out the current state of the   |
  |                              | attached device                             |
  |                              |                                             |
  | MSS_USBH_suspend()           | Suspends the MSS USB core. No frames are    |
  |                              | transferred                                 |
  |                              |                                             |
  | MSS_USBH_resume()            | Resumes the previously suspended MSS USB    |
  |                              |                                             |
  | MSS_USBH_get_std_dev_descr() | Reads the device descriptor from the        |
  |                              | attached device                             |
  |                              |                                             |
  | MSS_USBH_1ms_tick()          | Time stamping and housekeeping function     |
  |                              |                                             |
  | MSS_USBH_get_milis()         | Used to know the number of milliseconds     |
  |                              | lapsed after reset                          |
  |                              |                                             |
  | MSS_USBH_construct_get_descr | Formats the provided parameters into a USB  |
  | _command()                   | standard setup packet for GET_DESCRIPTOR    |
  |                              | command                                     |
  |                              |                                             |
  | MSS_USBH_start_control_xfr() | Starts a control transfer on previously     |
  |                              | configured control pipe                     |
  |                              |                                             |
  | MSS_USBH_get_cep_state()     | Provides the current state of the control   |
  |                              | pipe                                        |
  |                              |                                             |
  | MSS_USBH_test_mode()         | Initiates compliance test mode as specified |
  |                              | by the parameter.                           |

 *//*=========================================================================*/

#ifndef __MSS_USB_HOST_H_
#define __MSS_USB_HOST_H_

#include "mss_usb_common_cif.h"
#include "mss_usb_common_reg_io.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_HOST_ENABLED

/*-------------------------------------------------------------------------*//**
 Constant values exported by USBH driver
*/

/*-------------------------------------------------------------------------*//**
  The following constants are used to identify the exact reason due to which the
  USBH driver is not supporting the attached device. One of these values is
  passed as a parameter to the call-back function usbh_tdev_not_supported when
  USBH driver detects respective condition.
*/
/*
  Devices operating at the USB full speed are not supported.
 */
#define FS_DEV_NOT_SUPPORTED                            -15

/*
  Devices operating at the USB low speed are not supported.
 */
#define LS_DEV_NOT_SUPPORTED                            -14

/*
  Device belonging to the USB HUB class is detected. The Hub class is not
  supported by the USBH driver.
 */
#define HUB_CLASS_NOT_SUPPORTED                         -13

/*
  Attached device class description does not match the class description
  supported by the registered class driver.
 */
#define CLASS_DRIVER_NOT_MATCHED                        -12

/*
  The registered class driver is already in assigned state when new device
  attachment is detected.
 */
#define CLASS_DRIVER_NOT_FREE                           -11

/*
  The power requirement for the attached device, as described in the bMaxPower
  field of configuration descriptor, can not be supported by the USBH driver.
 */
#define CONF_DESC_POWER_LIM_EXCEED                      -10

/*
  The value received in bDescriptorType field in configuration descriptor is
  invalid.
*/
#define CONF_DESC_WRONG_DESC_TYPE                       -9

/*
  The value received in bLength field of the configuration descriptor does not
  match the standard value (9) as specified in the USB2.0 specification.
 */
#define CONF_DESC_WRONG_LENGTH                          -8

/*
  The value received in bMaxPacketSize0 field of the device descriptor is not 8
  for a LS device.
 */
#define DEV_DESC_LS_MAXPKTSZ0_NOT8                      -7

/*
  The value received in bMaxPacketSize0 field of the device descriptor is not 64
  for a HS device.
 */
#define DEV_DESC_HS_MAXPKTSZ0_NOT64                     -6

/*
  The value received in bcdDevice field of the device descriptor for a HS device
  does not indicate USB2.0
 */
#define DEV_DESC_HS_USBBCD_NOT200                       -5

/*
  The value received in the bMaxPacketSize0 field of the device descriptor is
  invalid. For LS device this value must be 8. For HS device this value must be
  64. For FS device this value must be 8, 16, 32, or 64.
 */
#define DEV_DESC_WRONG_MAXPKTSZ0                        -4

/*
  The value received in the bcdDevice field of the device descriptor is invalid.
  For HS device this value must indicate USB2.0.
 */
#define DEV_DESC_WRONG_USBBCD                           -3

/*
  The value received in the bDescriptorType field in device descriptor is
  invalid.
 */
#define DEV_DESC_WRONG_DESC_TYPE                        -2

/*
  The value received in the bLength field of the device descriptor does not
  match the standard value (18).
 */
#define DEV_DESC_WRONG_LENGTH                           -1

/*
  The following constants are used as the return values for the call-back
  functions, implemented by the USBH-Class driver as part of mss_usbh_class_cb_t
  type, which are called by the USBH driver. The USBH driver uses these values
  to infer whether or not the call-back function was successfully executed. The
  current control transfer is stalled if the return value is USB_FAIL.
 */
#define USB_FAIL                                        0u
#define USB_SUCCESS                                     1u

/*-------------------------------------------------------------------------*//**
  Data structures exported by USBH driver
 */

/*-------------------------------------------------------------------------*//**
  The mss_usbh_class_cb_t provides the prototype of the structure that must be
  implemented by the USBH-Class driver to provide the call-back functions which
  will be called by the USBH driver to communicate events on the target device.

  class_id
  The class_id parameter is the place-holder for identifying the USBH-Class
  driver which is implementing this call-back interface. This parameter is used
  by the USBH driver to identify and assign the class driver to the attached
  device.The MSS_USBH_register_class_driver() function must be used to register
  a class driver with the the USBH driver. This parameter must be of the form:

  | Class id bit  |                  Description                               |
  |-------------- |------------------------------------------------------------|
  |  D7 - D0      |                  USB Std Protocol code                     |
  |  D15 - D8     |                  USB Std sub-class code                    |
  |  D23 - D16    |                  USB Std class code                        |
  |  D31 - D24    |                            0x00                            |

  usbh_class_allocate
  The function pointed by the usbh_class_allocate element will be called when a
  match is found between the class code, sub-class code and protocol code of the
  attached device and the corresponding parameters provided by the USBH-Class
  driver in class_id parameter. The tdev_addr parameter provides the non-zero
  device address assigned to the attached device by the USBH driver.

  usbh_class_release
  The function pointed by the usbh_class_release element will be called when the
  device for which this USBH-Class driver is assigned is detached. The tdev_addr
  parameter provides the non-zero device address assigned to the attached device
  by the USBH driver.

  usbh_class_cep_xfr_done
  The function pointed by the usbh_class_cep_xfr_done element will be called
  when a control transfer started by this USBH-Class driver is complete. The
  tdev_addr parameter provides the non-zero device address assigned to the
  attached device by the USBH driver. The status parameter indicates whether or
  not the current control transfer was completed successfully. The count
  parameter provides the number of bytes transferred in the current control
  transfer. The status parameter provides the error status of current control
  transfer. A non-zero value indicates that there was an error.

  usbh_class_tx_done
  The function pointed by the usbh_class_tx_done element will be called
  when a transfer started on OUT pipe by this USBH-Class driver is complete.
  The tdev_addr parameter provides the non-zero device address assigned to the
  attached device by USBH driver. The status parameter indicates whether or not
  the current transfer was completed successfully. The count parameter provides
  the number of bytes transferred in the current control transfer. The status
  parameter provides the error status of the current control transfer. A
  non-zero value indicates that there was an error.

  usbh_class_rx_done
  The function pointed by the usbh_class_rx_done element will be called
  when a transfer started on the IN pipe by this USBH-Class driver is complete.
  The tdev_addr parameter provides the non-zero device address assigned to the
  attached device by the USBH driver. The status parameter indicates whether or
  not the current transfer was completed successfully. The count parameter
  provides the number of bytes transferred in the current control transfer.
  The status parameter provides the error status of current control transfer.
  A non-zero value indicates that there was an error.

  usbh_class_sof
  The function pointed by the usbh_class_sof element will be called when a new
  micro frame starts. The frame_number provides the number of the current
  micro frame.

*/
typedef struct mss_usbh_class_cb {
    uint32_t class_id;
    uint8_t (*usbh_class_allocate)(uint8_t tdev_addr);
    uint8_t (*usbh_class_release)(uint8_t tdev_addr);
    uint8_t (*usbh_class_cep_xfr_done)(uint8_t tdev_addr,
                                       uint8_t status,
                                       uint32_t count);

    uint8_t (*usbh_class_tx_done)(uint8_t tdev_addr,
                                  uint8_t status,
                                  uint32_t count);

    uint8_t (*usbh_class_rx_done)(uint8_t tdev_addr,
                                  uint8_t status,
                                  uint32_t count);

    uint8_t (*usbh_class_sof)(uint32_t frame_number);
}mss_usbh_class_cb_t;

/*-------------------------------------------------------------------------*//**
  The mss_usbh_tdev_info_t type provides the prototype for the detailed
  information of the attached device. Some of the parameters are only applicable
  when multi-point support is enabled on MSS USB. Multi-point support is
  currently not enabled in the USBH driver.

  addr
  The addr parameter provides the device address assigned to the attached device
  by the USBH driver.

  speed
  The speed parameter indicates the USB speed of operation at which the USBH
  driver is communicating with the attached device.

  state
  The state parameter provides the current state of the attached device. E.g.
  address, Configured etc.

  hub_addr
  The hub_addr parameter provides the address of the hub to which the device is
  attached. This parameter is meaningful only when Multi-Point support is
  enabled. Multi-point support is currently not enabled in the USBH driver.

  hub_port
  The hub_port parameter provides the port number of the hub to which the device
  is attached. This parameter is meaningful only when Multi-Point support is
  enabled. Multi-point support is currently not enabled in the USBH driver.

  hub_mtt
  The hub_mtt parameter indicates whether or not the hub to which the device is
  attached supports multiple transaction translators. This parameter is
  meaningful only when Multi-Point support is enabled. Multi-point support is
  currently not enabled in the USBH driver.

  maxpktsz0
  The maxpktsz0 parameter provides the maximum packet size of the control
  endpoint on the attached device.

  state
  The state parameter provides the current state of the attached device.
  E.g. address, Configured etc.

  class_handle
  The class_handle parameter is the class handle registered with the USBH driver
  using MSS_USBH_register_class_driver() function. This handle is used by the
  USBH driver to call the call-back functions implemented by the USBH-Class
  driver.

 */
typedef struct mss_usbh_target_info {
    uint8_t addr;
    mss_usb_device_speed_t speed;
    mss_usb_state_t state;
    uint8_t hub_addr;
    uint8_t hub_port;
    uint8_t hub_mtt;
    uint16_t tdev_maxpktsz0;
    mss_usbh_class_cb_t* class_handle;
} mss_usbh_tdev_info_t;

/*-------------------------------------------------------------------------*//**
  The mss_usbh_user_cb_t provides the prototype of the structure that must be
  implemented by the application to provide the call-back functions which will
  be called by the USBH driver to communicate events on the target device. These
  call-back functions can be used by the application to know the USBH driver
  state and information about the attached device from the USBH driver.
  These events are not specific to any USB class.

  usbh_tdev_attached
  The function pointed by the usbh_tdev_attached element will be called when a
  device attach even is detected by the USBH driver. At this point the device is
  not enumerated. The speed parameter indicates the USB speed of operation at
  which the USBH driver is going to communicate with the attached device.

  usbh_tdev_dettached
  The function pointed by the usbh_tdev_dettached element will be called when
  a device detach even is detected by the USBH driver.

  usbh_tdev_oc_error
  The function pointed by the usbh_tdev_oc_error element will be called when an
  over-current condition is detected by the USBH driver.

  usbh_tdev_enumerated
  The function pointed by the usbh_tdev_enumerated element will be called when
  the attached device is successfully enumerated. At this point the device
  descriptor is successfully read from the attached device. No class specific
  information is read from the device yet. The tdev_info parameter gives the
  information about the attached device.

  usbh_tdev_class_driver_assigned
  The function pointed by the usbh_tdev_class_driver_assigned element will be
  called when a matching class driver is assigned to the attached device.
  The class driver must be previously registered with the USBH driver using
  MSS_USBH_register_class_driver() function.

  usbh_tdev_not_supported
  The function pointed by the usbh_tdev_not_supported element will be called
  when a device which is not supported by the USBH driver is attached. The
  error_code parameter provides the exact reason why the device is not
  supported.
  E.g. devices belonging to Hub class are not supported.

  usbh_tdev_ permanent_error
  The function pointed by the usbh_tdev_permanent_error element will be called
  if there is error during data transfer operations with the attached device
  persists after retries. On detecting an error in data transfer with the
  attached device, the USBH driver retries for maximum three times.
*/
typedef struct mss_usbh_user_cb
{
    void (*usbh_tdev_attached)(mss_usb_device_speed_t speed);
    void (*usbh_tdev_dettached)(void);
    void (*usbh_tdev_oc_error)(void);
    void (*usbh_tdev_enumerated)(mss_usbh_tdev_info_t tdev_info);
    void (*usbh_tdev_class_driver_assigned) (void);
    void (*usbh_tdev_not_supported)(int8_t error_code);
    void (*usbh_tdev_permanent_error)(void);
} mss_usbh_user_cb_t;

/*-------------------------------------------------------------------------*//**
 Data structures internally used by USBH driver
*/

/*-------------------------------------------------------------------------*//**
  The dev_desc_t provides the prototype of the structure for the standard USB
  device descriptor. All the parameters of this structure are as per the
  standard setup packet defined in the USB2.0 specification.

  bLength
  The bLength parameter indicates the length of the device descriptor in bytes.
  The meaning of this parameter is exactly as defined by the USB2.0
  specification.

  bDescriptorType
  The bDescriptorType parameter indicates the standard descriptor type of this
  descriptor. The meaning of this parameter is exactly as defined by the USB2.0
  specification.

  bcdUSB
  The bcdUSB parameter indicates the USB standard (BCD format) supported by the
  device. The meaning of this parameter is exactly as defined by the USB2.0
  specification.

  bDeviceClass
  The bDeviceClass parameter indicates the standard USB class supported by the
  device. The meaning of this parameter is exactly as defined by the USB2.0
  specification.

  bDeviceSubClass
  The bDeviceSubClass parameter indicates the standard USB sub-class supported
  by the device. The meaning of this parameter is exactly as defined by the
  USB2.0 specification.

  bDeviceProtocol
  The bDeviceProtocol parameter indicates the standard USB data transfer
  protocol supported by the device. The meaning of this parameter is exactly as
  defined by the USB2.0 specification.

  bMaxPacketSize0
  The bMaxPacketSize0 parameter indicates the Maximum packet size of the
  Endpoint0 on the device. The meaning of this parameter is exactly as defined
  by the USB2.0 specification.

  idVendor
  The idVendor parameter indicates Vendor ID of the connected device. The
  meaning of this parameter is exactly as defined by the USB2.0 specification.

  idProduct
  The idProduct parameter indicates Product ID of the connected device. The
  meaning of this parameter is exactly as defined by the USB2.0 specification.

  bcdDevice
  The bcdDevice parameter indicates release number (BCD format) of the connected
  device. The meaning of this parameter is exactly as defined by the USB2.0
  specification.

  iManufacturer
  The iManufacturer parameter indicates the index of the string descriptor
  describing the manufacture name of the connected device. The meaning of this
  parameter is exactly as defined by the USB2.0 specification.

  iProduct
  The iProduct parameter indicates index of string descriptor describing the
  product name of the connected device. The meaning of this parameter is exactly
  as defined by the USB2.0 specification.

  iSerialNumber
  The iSerialNumber parameter indicates the index of the string descriptor
  describing the serial number of the connected device. The meaning of this
  parameter is exactly as defined by the USB2.0 specification.

  bNumConfigurations
  The bNumConfigurations parameter indicates number of configurations supported
  by the connected device. The meaning of this parameter is exactly as defined
  by the USB2.0 specification.

  reserved
  The reserved parameter does not have a specific meaning. It is provided to
  make the structure word aligned.
*/
typedef struct
{
    uint8_t   bLength;
    uint8_t   bDescriptorType;
    uint16_t  bcdUSB;
    uint8_t   bDeviceClass;
    uint8_t   bDeviceSubClass;
    uint8_t   bDeviceProtocol;
    uint8_t   bMaxPacketSize0;
    uint16_t  idVendor;
    uint16_t  idProduct;
    uint16_t  bcdDevice;
    uint8_t   iManufacturer;
    uint8_t   iProduct;
    uint8_t   iSerialNumber;
    uint8_t   bNumConfigurations;
    uint16_t  reserved; /*Word alignment*/
}dev_desc_t;

/*-------------------------------------------------------------------------*//**
  The def_conf_desc_t provides the prototype of the structure for the standard
  USB configuration descriptor. All the parameters of this structure are as per
  the standard setup packet defined in the USB2.0 specification.

  bLength
  The bLength parameter indicates the length of the configuration descriptor in
  bytes. The meaning of this parameter is exactly as defined by the USB2.0
  specification.

  bDescriptorType
  The bDescriptorType parameter indicates the standard descriptor type of this
  descriptor. The meaning of this parameter is exactly as defined by the USB2.0
  specification.

  bTotalLength
  The bTotalLength parameter indicates the total length (in bytes) of the
  configuration descriptor including the class specific subordinate descriptors.
  The meaning of this parameter is exactly as defined by the USB2.0
  specification.

  bNumInterface
  The bNumInterfaces parameter indicates the number of interfaces supported by
  this configuration. The meaning of this parameter is exactly as defined by
  the USB2.0 specification.

  bConfigurationValue
  The bConfigurationValue parameter indicates the value to be use as an argument
  to SET_CONFIGURATION request to select this configuration. The meaning of this
  parameter is exactly as defined by the USB2.0 specification.

  iConfiguration
  The iConfiguration parameter indicates the index of the string descriptor
  describing this configuration. The meaning of this parameter is exactly as
  defined by the USB2.0 specification.

  bmAttributes
  The bmAttributes parameter provides configuration characteristics of this
  configuration. The meaning of this parameter is exactly as defined by the
  USB2.0 specification.

  bMaxPower
  The bMaxPower parameter indicates the Maximum power consumption of the USB
  device from the bus in this specific configuration when the device is fully
  operational. The meaning of this parameter is exactly as defined by the
  USB2.0 specification.

  upended_desc
  The upended_desc parameter can be used to store the class specific subordinate
  descriptor information. This might be useful to retrieve some basic
  information about the selected configuration.
*/
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
    uint8_t upended_desc[11];
}def_conf_desc_t;


/*---------------------------USBH Public APIs---------------------------------*/

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_init() function initializes the MSS USB driver in the USB host
  mode. This function initializes the internal data structures and readies the
  MSS USB hardware block to detect the device attachment event. This function
  also configures the control endpoint to keep it ready for communicating with
  the detected device on the USB port. The parameter app_cb provided with this
  function must be implemented by the application. This driver uses the app_cb
  to inform the application about the events on the USB bus by calling the
  appropriate callback element.

  The MSS_USBH_init() function must be called before any other function to
  initialize the USBH driver in the USB host mode.

  @param app_cb
    The app_cb parameter provides the pointer to the structure of type
    mss_usbh_user_cb_t. This structure is used to call the user call-back
    functions by the USBH driver. These call-back functions can be used to
    provide error/status messages to the user for the application specific
    handling of the events.

  @return
    This function does not return any value.

  Example:
  @code
      mss_usbh_user_cb_t MSS_USBH_user_cb =
      {
          USB_DEV_attached,
          USB_DEV_dettached,
          USB_DEV_oc_error,
          USB_DEV_enumerated,
          USB_DEV_class_driver_assigned,
          USB_DEV_not_supported,
          USB_DEV_permanent_erro
      };

      MSS_USBH_init(&MSS_USBH_user_cb);
      MSS_USBH_register_class_driver(MSS_USBH_HID_get_handle());
  @endcode
*/
void
MSS_USBH_init
(
    mss_usbh_user_cb_t* app_cb
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_task() function is the main task of the USBH driver. This
  function must be called repeatedly by the application to allow the USBH driver
  to perform the housekeeping tasks. A timer/scheduler can be used to call this
  function at regular intervals or it can be called from the main continuous
  foreground loop of the application.

  @param
    This function does not take any parameters.

  @return
    This function does not return any value.

  Example:
  @code
      void
      Host_controller_task
      (
          void
      )
      {
          MSS_USBH_task();
          MSS_USBH_HID_task();
          switch (c_state)
          {
              case CONTROLLER_IDLE:
                  if(USBH_MSD_DEVICE_READY == MSS_USBH_MSC_get_state())
                  {
                       c_state = CONTROLLER_INQUIRY;
                  }
                  break;
                .
              default:
                  break;
          }
      }
  @endcode
*/
void
MSS_USBH_task
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_register_class_driver() function must be used by the application
  to register a USBH-Class driver with the USBH driver. The application must
  first get the handle from the USBH-Class driver before assigning it to the
  USBH driver. Refer the USBH-Class section to know how to get a handle from the
  USBH-Class driver. This handle is used by the USBH driver to communicate with
  the USBH-Class driver.

  @param class_handle
    The class_handle parameter specifies the class driver handle to be
    registered with the USBH driver.

  @return
    This function returns zero when execution was successful.

  Example:
  @code
      mss_usbh_user_cb_t MSS_USBH_user_cb =
      {
          USB_DEV_attached,
          USB_DEV_dettached,
          USB_DEV_oc_error,
          USB_DEV_enumerated,
          USB_DEV_class_driver_assigned,
          USB_DEV_not_supported,
          USB_DEV_permanent_erro
      };

      MSS_USBH_init(&MSS_USBH_user_cb);
      MSS_USBH_register_class_driver(MSS_USBH_HID_get_handle());
  @endcode
*/
int8_t
MSS_USBH_register_class_driver
(
    void* class_handle
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_configure_control_pipe() function is used to configure the
  control pipe for the control transactions with attached device with the
  address provided in target_addr parameter. This function also enables the
  control pipe interrupt.

  This function must be called before any other function for the control pipe.

  @param target_addr
    The target_addr parameter is address of the attached device with which the
    MSS USB needs to communicate.

  @return
    This function returns zero when execution was successful.

  Example:
  @code
      typedef enum {
          TDEV_R,
          TDEV_RHP1,
          TDEV_RHP2
      } tdev_id_t;

      MSS_USBH_configure_control_pipe(TDEV_R);
  @endcode
*/
int8_t
MSS_USBH_configure_control_pipe
(
    uint8_t target_addr
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_configure_in_pipe() function is used to configure the IN pipe
  for the IN transactions with the attached device with the address provided in
  the target_addr parameter. This function also enables the IN pipe interrupt.

  This function must be called before any other function for the IN pipe.

  @param target_addr
    The target_addr parameter is the address of the attached device with which
    MSS USB needs to communicate.

  @param inpipe_num
    The inpipe_num parameter indicates the IN pipe number (endpoint) on MSS USB
    which must be used for IN transfers with the attached device.

  @param target_ep
    The target_ep parameter indicates the IN endpoint number on the attached
    device with which MSS USB needs to communicate.

  @param fifo_addr
    The fifo_addr parameter is the address of the FIFO in the MSS USB internal
    RAM. The valid FIFO address values are from 0x00 to 0xFFF8. The FIFO Address
    must be a multiple of 8. If the value provided is not a multiple of 8, then
    the immediate lower value which is a multiple of 8 is taken as the FIFO
    address.
    E.g. If the provided value is 0x09, the actual value taken by the driver is
    0x08. If the provided value is less than 0x08 then the FIFO address is taken
    as 0x00.

  @param fifo_size
    The fifo_size parameter provides the endpoint FIFO size in the USB core
    internal RAM. The valid FIFO size values are 8, 16, 32, 64, 128, 512, 1024,
    2048, 4096. The MSS USB assigns 8 byte FIFO by default if the FIFO size is
    not configured.

  @param max_pkt_size
    The max_pkt_size parameter provides the maximum packet size of the USB
    transfer. This value must be equal to the maximum packet size as mentioned
    in the endpoint descriptor which is used during enumeration process.

    Note: This value must be less than or equal to the FIFO size value.


  @param num_usb_pkt
    The num_usb_pkt parameter has different meanings for different types of
    transfers.

    Low bandwidth ISO/interrupt transfers - This parameter must always be 1u.
    This parameter represents the number of packets transferred in one (micro)
    frame.

    High bandwidth ISO transfers - This parameter represents the number of
    packets transferred in one (Micro) frame. In this case, this parameter can
    have a value of 1 2 or 3. High bandwidth ISO transfers are not yet
    implemented.

    Bulk transfers. - For Bulk transfer this value must always be 1u. This
    parameter is  be used with the auto-amalgamation/auto-split feature where it
    indicates number of bulk packets to be auto-amalgamated/auto-split in bulk
    transfer. The auto-amalgamation/auto-split feature is implemented but not
    yet tested.

  @param dma_enable
    The dma_enable parameter specifies whether or not the internal DMA must be
    used for the data transfer from the provided buffer to the USB FIFO.

  @param dma_channel
    The dma_channel parameter specifies the internal DMA channel to be used for
    the data transfers. DMA channel will be associated with selected endpoint.
    A unique DMA channel must be selected to transfer data on individual
    endpoints. This parameter is ignored when dma_enable parameter indicates
    that the DMA must not be used.

  @param xfr_type
    The xfr_type parameter specifies the type of transfer to be performed on the
    specified endpoint. All other types of transfers (Interrupt, Isochronous,
    and Bulk) can be selected except the control transfer.

  @param add_zlp
    The add_zlp parameter indicates whether to expect a zero length packet (ZLP)
    if the transfer size is exact a multiple of wMaxPacketSize. This parameter
    is only applicable for bulk transfers. For all other transfer types this
    parameter is ignored.

  @param interval
    The interval parameter for interrupt and isochronous transfers defines the
    polling interval for the currently-selected IN pipe. For bulk transfers,
    this parameter indicates the number of frames/microframes after which the IN
    pipe must timeout on receiving a stream of NAK responses. The valid values
    for this parameter are as mentioned in the table below.

    | Transfer Type    |  Speed     | Valid values                               |
    |------------------|------------|--------------------------------------------|
    |  Interrupt       | LS or FS   | 1 - 255                                    |
    |  Interrupt       |    HS      | 1,2,4,8,16,32,64...32768 microframes       |
    |                  |            |                                            |
    |  Isochronous     | FS or HS   | 1,2,4,8,16,32,64...32768 frames/microframes|
    |                  |            |                                            |
    |  Bulk            |    HS      | 0,2,4,8,16,32,64...32768 microframes       |
    |                  |            | Note:Value 0 disables the NAK Timeout      |
    |                  |            | detection on Bulk endpoint                 |

  @return
    This function returns zero when execution was successful.

  Example:
  @code
      case USBH_MSC_CONFIG_BULK_ENDPOINTS:
          MSS_USBH_configure_out_pipe(g_msd_tdev_addr,
                                      USBH_MSC_BULK_TX_PIPE,
                                      g_tdev_out_ep.num,
                                      USBH_MSC_BULK_TX_PIPE_FIFOADDR,
                                      USBH_MSC_BULK_TX_PIPE_FIFOSZ,
                                      g_tdev_out_ep.maxpktsz,
                                      1,
                                      DMA_DISABLE,
                                      MSS_USB_DMA_CHANNEL_NA,
                                      MSS_USB_XFR_BULK,
                                      NO_ZLP_TO_XFR,
                                      32768u);

          MSS_USBH_configure_in_pipe(g_msd_tdev_addr,
                                     USBH_MSC_BULK_RX_PIPE,
                                     g_tdev_in_ep.num,
                                     USBH_MSC_BULK_RX_PIPE_FIFOADDR,
                                     USBH_MSC_BULK_RX_PIPE_FIFOSZ,
                                     g_tdev_in_ep.maxpktsz,
                                     1,
                                     DMA_DISABLE,
                                     MSS_USB_DMA_CHANNEL_NA,
                                     MSS_USB_XFR_BULK,
                                     NO_ZLP_TO_XFR,
                                     32768u);

          g_msc_state = USBH_MSC_TEST_UNIT_READY_CPHASE;
      break;
  @endcode
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
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_configure_out_pipe() function is used to configure the OUT pipe
  for the OUT transactions with attached device with the address provided in the
  target_addr parameter. This function also enables the OUT pipe interrupt.

  This function must be called before any other function for the OUT pipe.

  @param target_addr
    The target_addr parameter is the address of the attached device with which
    MSS USB needs to communicate.

  @param outpipe_num
    The outpipe_num parameter indicates the OUT pipe number (endpoint) on MSS
    USB which must be used for OUT transfers with the attached device.

  @param target_ep
    The target_ep parameter indicates the OUT endpoint number on the attached
    device with which MSS USB needs to communicate.

  @param fifo_addr
    The fifo_addr parameter is the address of the FIFO in the MSS USB internal
    ram. The valid FIFO address values are from 0x00 to 0xFFF8. The FIFO address
    must be a multiple of 8. If the value provided is not a multiple of 8, then
    the immediate lower value which is a multiple of 8 is taken as the FIFO
    address.
    E.g. If the provided value is 0x09, the actual value taken by the driver is
    0x08. If the provided value is less than 0x08 then the  FIFO address is
    taken as 0x00.

  @param fifo_size
    The fifo_size parameter provides the endpoint FIFO size in the USB core
    internal RAM. The valid FIFO size values are 8, 16, 32, 64, 128, 512, 1024,
    2048, 4096. The MSS USB assigns 8 byte FIFO by default if the FIFO size is
    not configured.

  @param max_pkt_size
    The max_pkt_size parameter provides the maximum packet size of the USB
    transfer. This value must be equal to the maximum packet size as mentioned
    in the endpoint descriptor which is used during enumeration process.

    Note: This value must be less than or equal to the FIFO size value.

  @param num_usb_pkt
    The num_usb_pkt parameter has different meanings for different types of
    transfers.

    Low bandwidth ISO/interrupt transfers - This parameter must always be '1u'.
    This parameter represents the number of packets transferred in one (micro)
    frame.

    High bandwidth ISO transfers - This parameter represents the number of
    packets transferred in one (Micro) frame. In this case, this parameter can
    have a value of 1 2 or 3. High bandwidth ISO transfers are not yet
    implemented.

    Bulk transfers. - For Bulk transfer this value must always be '1u'. This
    parameter will be used with the auto-amalgamation/auto-split feature where
    it indicates the number of bulk packets to be auto-amalgamated/auto-split
    in bulk transfer. The auto-amalgamation/auto-split feature is implemented
    but not yet tested.

  @param dma_enable
    The dma_enable parameter specifies whether or not the internal DMA must be
    used for the data transfer from the provided buffer to the USB FIFO.

  @param dma_channel
    The dma_channel parameter specifies the internal DMA channel to be used for
    the data transfers. The DMA channel will be associated with the specified
    endpoint. A unique DMA channel must be selected to transfer data on
    individual endpoints. This parameter is ignored when the dma_enable
    parameter indicates that the DMA must not be used.

  @param xfr_type
    The xfr_type parameter specifies the type of transfer to be performed on the
    specified endpoint. All other types of transfers (Interrupt, Isochronous,
    and Bulk) can be selected except control transfer.

  @param add_zlp
    The add_zlp parameter indicates whether to send a zero length packet (ZLP)
    if the transfer size is a exact multiple of wMaxPacketSize. This parameter
    is only applicable for Bulk transfers. For all other transfer types this
    parameter is ignored.

  @param interval
    The interval parameter for interrupt and isochronous transfers, defines the
    polling interval for the currently-selected OUT pipe. For the bulk
    transfers, this parameter indicates the number of frames/microframes after
    which the OUT Pipe must time out on receiving a stream of NAK responses.
    The valid values for this parameter are as mentioned in the table below.

    | Transfer Type    |  Speed    | Valid values                                |
    |------------------|-----------|---------------------------------------------|
    |  Interrupt       | LS or FS  | 1 - 255                                     |
    |  Interrupt       |    HS     | 1,2,4,8,16,32,64...32768 microframes        |
    |                  |           |                                             |
    |  Isochronous     | FS or HS  | 1,2,4,8,16,32,64...32768 frames/microframes |
    |                  |           |                                             |
    |  Bulk            |    HS     | 0,2,4,8,16,32,64...32768 microframes        |
    |                  |           | Note:Value 0 disables the NAK Timeout       |
    |                  |           | detection on Bulk endpoint                  |

  @return
    This function returns zero when execution was successful.

  Example:
  @code
      case USBH_MSC_CONFIG_BULK_ENDPOINTS:
          MSS_USBH_configure_out_pipe(g_msd_tdev_addr,
                                      USBH_MSC_BULK_TX_PIPE,
                                      g_tdev_out_ep.num,
                                      USBH_MSC_BULK_TX_PIPE_FIFOADDR,
                                      USBH_MSC_BULK_TX_PIPE_FIFOSZ,
                                      g_tdev_out_ep.maxpktsz,
                                      1,
                                      DMA_DISABLE,
                                      MSS_USB_DMA_CHANNEL_NA,
                                      MSS_USB_XFR_BULK,
                                      NO_ZLP_TO_XFR,
                                      32768u);

          MSS_USBH_configure_in_pipe(g_msd_tdev_addr,
                                     USBH_MSC_BULK_RX_PIPE,
                                     g_tdev_in_ep.num,
                                     USBH_MSC_BULK_RX_PIPE_FIFOADDR,
                                     USBH_MSC_BULK_RX_PIPE_FIFOSZ,
                                     g_tdev_in_ep.maxpktsz,
                                     1,
                                     DMA_DISABLE,
                                     MSS_USB_DMA_CHANNEL_NA,
                                     MSS_USB_XFR_BULK,
                                     NO_ZLP_TO_XFR,
                                     32768u);

          g_msc_state = USBH_MSC_TEST_UNIT_READY_CPHASE;
      break;
  @endcode
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
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_write_out_pipe() function writes the data provided by the user
  into the previously configured out pipe FIFO. The data is ready to be
  transferred after calling this function. The Data will be transferred by the
  MSS USB according to the transfer type and interval parameter set for this
  OUT pipe using MSS_USBH_configure_out_pipe() function. This function is
  non-blocking.A call-back function will be called to indicate the status of
  this transfer after the status phase of the current OUT transaction is
  complete.

  @param target_addr
    The target_addr parameter is the address of the attached device with which
    the MSS USB needs to communicate.

  @param outpipe_num
    The outpipe_num parameter indicates the OUT pipe number (endpoint) on the
    MSS USB which must be used for the OUT transfers with the attached device.

  @param target_ep
    The target_ep parameter indicates the OUT endpoint number on the attached
    device with which MSS USB needs to communicate.

  @param max_pkt_size
    The max_pkt_size parameter provides the maximum packet size of the USB
    transfer. This value must be equal to the maximum packet size as mentioned
    in the endpoint descriptor which is used during enumeration process.

  @param buf
    The buf parameter is the address of the buffer provided by the user from
    which the data is copied to the selected pipe buffer.

  @param length
    The length parameter specifies the length of the data buffer in bytes.

  @return
    This function does not return a value.

  Example:
  @code
      case USBH_MSC_TEST_UNIT_READY_CPHASE:
          MSS_USBH_MSC_construct_cbw_cb6byte(USB_MSC_SCSI_TEST_UNIT_READY,
                                             0u,
                                             &g_bot_cbw);

          MSS_USBH_write_out_pipe(g_msd_tdev_addr,
                                  USBH_MSC_BULK_TX_PIPE,
                                  g_tdev_out_ep.num,
                                  g_tdev_out_ep.maxpktsz,
                                  (uint8_t*)&g_bot_cbw,
                                  31u);

          g_msc_state = USBH_MSC_TEST_UNIT_READY_SPHASE;
      break;
  @endcode
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
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_read_in_pipe() function writes the data provided by the user into
  the previously configured out pipe FIFO. The data is ready to be transferred
  after calling this function. The data will be transferred by the MSS USB
  according to the transfer type and the interval parameter set for this OUT
  pipe using theMSS_USBH_configure_in_pipe() function.  This function is
  non-blocking. A call-back function will be called to indicate the status of
  this transfer after the status phase of the current IN transaction is
  complete.

  @param target_addr
    The target_addr parameter is the address of the attached device with which
    the MSS USB needs to communicate.

  @param outpipe_num
    The outpipe_num parameter indicates the OUT pipe number (endpoint) on MSS
    USB which must be used for the OUT transfers with the attached device.

  @param target_ep
    The target_ep parameter indicates the OUT endpoint number on the attached
    device with which the MSS USB needs to communicate.

  @param max_pkt_size
    The max_pkt_size parameter provides the maximum packet size of the USB
    transfer. This value must be equal to the maximum packet size as mentioned
    in the endpoint descriptor which is used during enumeration process.

  @param buf
    The buf parameter is the address of the buffer provided by the user from
    which the data is copied to the selected pipe buffer.

  @param length
    The length parameter specifies the length of the data buffer in bytes.

  @return
    This function does not return a value.

  Example:
  @code
      case USBH_MSC_TEST_UNIT_READY_SPHASE:
              if(g_usbh_msc_tx_event)
              {
                  g_usbh_msc_tx_event = 0;
                  g_msc_state = USBH_MSC_TEST_UNIT_READY_WAITCOMPLETE;
                  MSS_USBH_read_in_pipe(g_msd_tdev_addr,
                                        USBH_MSC_BULK_RX_PIPE,
                                        g_tdev_in_ep.num,
                                        g_tdev_in_ep.maxpktsz,
                                        (uint8_t*)&g_bot_csw,
                                        13u);
              }
          break;
  @endcode
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
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_get_tdev_state() function can be used to find out the current
  state of the attached device. E.g. ADDRESS, CONFIGURED etc. (as defined by
  USB2.0).

  @param target_addr
    The target_addr parameter is the address of the attached device with which
    the MSS USB needs to communicate.

  @return
    This function returns a value of type mss_usb_state_t indicating the current
    state of the attached device.

  Example:
  @code
      static mss_usb_state_t msd_tdev_state = MSS_USB_NOT_ATTACHED_STATE;
      msd_tdev_state = MSS_USBH_get_tdev_state(g_msd_tdev_addr);
  @endcode

 */
mss_usb_state_t
MSS_USBH_get_tdev_state
(
    uint8_t target_addr
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_suspend() function can be used to suspend the MSS USB. The MSS
  USB will complete the current transaction then stop the transaction scheduler
  and frame counter. No further transactions will be started and no SOF packets
  will be generated.

  @param
    This function does not take any parameters.

  @return
    This function does not return any value.

  Example:
  @code
  @endcode
*/
void
MSS_USBH_suspend
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_resume() function can be used to resume the previously suspended
  MSS USB. If the MSS USB was in suspend mode then it will start the transaction
  scheduler and frame counter after calling this function.

  @param
    This function does not take any parameters.

  @return
    This function does not return any value.

  Example:
  @code
  @endcode
*/
void
MSS_USBH_resume
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_1ms_tick() function must be used to provide the 1ms time tick to
  the USBH driver. This time reference is used by the USBH driver to keep track
  of different time delays required during enumeration process. This way the
  application need not wait for the enumeration to complete which may take long
  time.

  @param
    This function does not take any parameters.

  @return
    This function does not return any value.

  Example:
  @code
      void SysTick_Handler(void)
      {
          MSS_USBH_task();
          MSS_USBH_HID_task();
          MSS_USBH_1ms_tick();
      }
  @endcode
*/
void
MSS_USBH_1ms_tick
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_get_milis() function is used to find out the number of
  milliseconds elapsed from the time when last reset occurred.

  @param
    This function does not take any parameters.

  @return
    This function returns a 32-bit unsigned integer value. This value indicates
    the number of milliseconds elapsed from the time when the last reset
    occurred. This integer value will overflow after approximately 49Hrs after
    reset.

  Example:
  @code
      case USBH_MSC_WAIT_SET_CONFIG:
          if(g_usbh_msc_cep_event)
          {
              g_usbh_msc_cep_event = 0;
              wait_mili = MSS_USBH_get_milis();
              g_msc_state = USBH_MSC_WAIT_DEV_SETTLE;
          }
      break;

      case USBH_MSC_WAIT_DEV_SETTLE:
          if((MSS_USBH_get_milis() - wait_mili) > 60)
          {
              g_msc_state = USBH_MSC_GET_MAX_LUN;
          }
      break;
  @endcode
*/
uint32_t
MSS_USBH_get_milis
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_construct_get_descr_command() function is be used to construct
  the setup packet which is to be sent on the USB bus. This function formats the
  provided parameters into a USB standard format GET_DESCRIPTOR request and
  stores it in the location provided by the buf parameter.

  @param buf
    The buf parameter is the address of the buffer where the formatted
    GET_DESCRIPTOR request is stored.

  @param xfr_dir
    The xfr_dir parameter indicates the direction of the data transfer on the
    USB bus. The value USB_STD_REQ_DATA_DIR_IN indicates that the direction of
    the data transfer is from the USB device to USB host (USB IN transaction).
    The value USB_STD_REQ_DATA_DIR_OUT indicates that the direction of the data
    transfer is from the USB host to the USB device (USB OUT transaction).

  @param req_type
    The req_type parameter indicates the request type. The request type can be
    one of the following: standard request, class request or vendor request.

  @param recip_type
    The recip_type parameter indicates the recipient type on the USB device. The
    request recipient on the attached device can be one of the following:
    device, endpoint or interface.

  @param request
    The request parameter indicates the request that needs to be sent to the
    attached device.

  @param desc_type
    The desc_type parameter indicates the descriptor type that needs to be
    requested from the attached device.

  @param strng_idx
    The strng_idx parameter indicates the index of the string descriptor when
    the desc_type parameter indicates STRING TYPE.

  @param length
    The length parameter indicates the number of bytes to be received from
    attached device as part of the requested descriptor.

  @return
    This function does not return any value.

  Example:
  @code
      MSS_USBH_configure_control_pipe(g_msd_tdev_addr);
      memset(std_req_buf, 0u, 8*(sizeof(uint8_t)));
      MSS_USBH_construct_get_descr_command(std_req_buf,
                                           USB_STD_REQ_DATA_DIR_IN,
                                           USB_STANDARD_REQUEST,
                                           USB_STD_REQ_RECIPIENT_DEVICE,
                                           USB_STD_REQ_GET_DESCRIPTOR,
                                           USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                           0,
                                           32u);

      g_msc_state = USBH_MSC_WAIT_GET_CLASS_DESCR;
      MSS_USBH_start_control_xfr(std_req_buf,
                                 (uint8_t*)&g_msd_conf_desc,
                                 USB_STD_REQ_DATA_DIR_IN,
                                 32u);
  @endcode
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
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_start_control_xfr() function must be used to start a control
  transfer with the attached USB device. This is a non-blocking function.
  This function prepares the control pipe for the control transfer. A call-back
  function will be called to indicate the status of the transfer after the
  status phase of the control transfer is complete.

  @param cbuf_addr
    The cbuf_addf parameter is the address of the buffer where the USB request
    setup packet is available.

  @param dbuf_addr
    The dbuf_addf parameter is the address of the buffer for the data phase of
    the control transfer. The USBD driver will put the data received from
    attached device in this buffer for the USB IN transfers. The data from this
    buffer will be sent to the attached device for the USB OUT transfers.

  @param data_dir
    The data_dir parameter indicates the direction of the data flow in the data
    phase of the control transfer. The value USB_STD_REQ_DATA_DIR_IN indicates
    that the direction of the data transfer is from the USB device to the USB
    host (USB IN transaction). The value USB_STD_REQ_DATA_DIR_OUT indicates
    that the direction of the data transfer is from the USB host to the USB
    device (USB OUT transaction).

  @param data_len
    The data_len parameter indicates the number of bytes to be transferred
    during the data phase of the control transfer.

  @return
    This function returns zero when execution was successful.

  Example:
  @code
      MSS_USBH_configure_control_pipe(g_msd_tdev_addr);
      memset(std_req_buf, 0u, 8*(sizeof(uint8_t)));
      MSS_USBH_construct_get_descr_command(std_req_buf,
                                           USB_STD_REQ_DATA_DIR_IN,
                                           USB_STANDARD_REQUEST,
                                           USB_STD_REQ_RECIPIENT_DEVICE,
                                           USB_STD_REQ_GET_DESCRIPTOR,
                                           USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                           0,
                                           32u);

      g_msc_state = USBH_MSC_WAIT_GET_CLASS_DESCR;
      MSS_USBH_start_control_xfr(std_req_buf,
                                 (uint8_t*)&g_msd_conf_desc,
                                 USB_STD_REQ_DATA_DIR_IN,
                                 32u);
@endcode
 */
uint8_t
MSS_USBH_start_control_xfr
(
    uint8_t* cbuf_addr,
    uint8_t* dbuf_addr,
    uint8_t  data_dir,
    uint32_t data_len
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_get_cep_state() function can be used to find out the current
  state of the control pipe. A new control transfer can be started when the
  control pipe is in the MSS_USB_CEP_IDLE state.

  @param
    This function does not take any parameters.

  @return
    This function returns a value of type mss_usb_ep_state_t indicating the
    current state of the control pipe.

  Example:
  @code
      mss_usb_ep_state_t cep_st;
      cep_st = MSS_USBH_get_cep_state();
      if(MSS_USB_CEP_IDLE == cep_st)
      {
          MSS_USBH_configure_control_pipe(g_msd_tdev_addr);
          memset(std_req_buf, 0u, 8*(sizeof(uint8_t)));
          MSS_USBH_construct_get_descr_command(std_req_buf,
                                              USB_STD_REQ_DATA_DIR_IN,
                                              USB_STANDARD_REQUEST,
                                              USB_STD_REQ_RECIPIENT_DEVICE,
                                              USB_STD_REQ_GET_DESCRIPTOR,
                                              USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                              0,
                                              32u);

          g_msc_state = USBH_MSC_WAIT_GET_CLASS_DESCR;
          MSS_USBH_start_control_xfr(std_req_buf,
                                     (uint8_t*)&g_msd_conf_desc,
                                     USB_STD_REQ_DATA_DIR_IN,
                                     32u);
      }
  @endcode
*/
mss_usb_ep_state_t
MSS_USBH_get_cep_state
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_test_mode() function can be used to initiate one of the
  compliance test modes in the MSS USB. This function is useful only for
  compliance test procedures. The MSS USB goes into the specified test mode once
  this function is executed. A hard reset is required to resume normal
  operations.

  @param test_case
    The test_case parameter indicates the compliance test that need to be
    executed.The compliance tests could be one of the following:
        Test-J (USB_TEST_MODE_SELECTOR_TEST_J),
        Test-K (USB_TEST_MODE_SELECTOR_TEST_K),
        SE0-NAK (USB_TEST_MODE_SELECTOR_TEST_SE0NAK),
        Test-Packet (USB_TEST_MODE_SELECTOR_TEST_PACKET)

  @return
    This function does not return any value.

  Example:
  @code
      case '1':
          MSS_USBH_test_mode(USB_TEST_MODE_SELECTOR_TEST_PACKET);
          MSS_UART_polled_tx_string(&g_mss_uart1,
                                    (uint8_t *)"\n\n\r Test-Packet started...");
      break;

      case '2':
          MSS_USBH_test_mode(USB_TEST_MODE_SELECTOR_TEST_J);
          MSS_UART_polled_tx_string(&g_mss_uart1,
                                         (uint8_t *)"\n\n\r Test-J started...");
      break;

      case '3':
          MSS_USBH_test_mode(USB_TEST_MODE_SELECTOR_TEST_K);
          MSS_UART_polled_tx_string(&g_mss_uart1,
                                         (uint8_t *)"\n\n\r Test-K started...");
      break;

      case '4':
          MSS_USBH_test_mode(USB_TEST_MODE_SELECTOR_TEST_SE0NAK);
          MSS_UART_polled_tx_string(&g_mss_uart1,
                                   (uint8_t *)"\n\n\r Test-SE0-NAK started...");
      break;

      case '5':
          MSS_USBH_test_mode(USB_TEST_MODE_SELECTOR_TEST_FORCE_ENA);
          MSS_UART_polled_tx_string(&g_mss_uart1,
                                (uint8_t *)"\n\n\r Test-Force-Host started...");
      break;

      default:
          invalid_selection_menu();
      break;
  @endcode
 */
void
MSS_USBH_test_mode
(
    uint8_t test_case
);

/*-------------------------------------------------------------------------*//**
  The MSS_USBH_get_std_dev_descr() function can be used to get the 8 byte USB
  standard device descriptor from the attached device. This function is provided
  specifically for the Standard Descriptor test case of USB-IF compliance
  procedure. This function might not be useful In general scenarios.

  @param buffer
    The buffer parameter is the pointer to the buffer where the USB standard
    device descriptor received from the attached device will be stored.

  @return
    This function returns zero on successful execution.

  Example:
  @code
      mss_usb_ep_state_t cep_st;
      cep_st = MSS_USBH_get_cep_state();
      if(MSS_USB_CEP_IDLE == cep_st)
      {
          MSS_USBH_configure_control_pipe(g_msd_tdev_addr);
          memset(std_req_buf, 0u, 8*(sizeof(uint8_t)));
          MSS_USBH_construct_get_descr_command(std_req_buf,
                                              USB_STD_REQ_DATA_DIR_IN,
                                              USB_STANDARD_REQUEST,
                                              USB_STD_REQ_RECIPIENT_DEVICE,
                                              USB_STD_REQ_GET_DESCRIPTOR,
                                              USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                              0,
                                              32u);

          g_msc_state = USBH_MSC_WAIT_GET_CLASS_DESCR;
          MSS_USBH_start_control_xfr(std_req_buf,
                                     (uint8_t*)&g_msd_conf_desc,
                                     USB_STD_REQ_DATA_DIR_IN,
                                     32u);
      }
  @endcode
 */
int8_t
MSS_USBH_get_std_dev_descr
(
    uint8_t* buffer
);

void
MSS_USBH_abort_in_pipe
(
    mss_usb_ep_num_t inpipe_num
);

void
MSS_USBH_abort_out_pipe
(
    mss_usb_ep_num_t outpipe_num
);

#endif /* MSS_USB_HOST_ENABLED */

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_HOST_H_ */
