/*******************************************************************************
 * (c) Copyright 2018 Microsemi SoC Products Group. All rights reserved.
 *
 * Microsemi Smartfusion2 MSS USB Driver Stack.
 *      USB Logical Layer (USB-LL)
 *      USBD-HID class driver
 *
 * Header file for mss_usb_device_hid.c
 *
 *
 * SVN $Revision: 10280 $
 * SVN $Date: 2018-08-01 16:36:37 +0530 (Wed, 01 Aug 2018) $
 */
/*=========================================================================*//**
  @mainpage Smartfusion2 MSS USB driver
                    USBD-HID class driver

  @section intro_sec Introduction
  The Human Interface Device class driver implements the USB HID class
  functionality specified by the USB-IF. This driver supports the HID mouse
  functionality. The USB control transfers and Interrupt transfers are used to
  implement this functionality. With this driver, the Smartfusion2 device
  appears as a Human interface class Mouse device when connected to USB host,
  This driver can be extended to be used for other HID class functionalities.

  This driver uses the USBD-Class driver template to implement the HID class
  functionality.

  @section theory_op Theory of Operation
  The following steps are involved in the operation of the USBD-HID driver:
        •    Configuration
        •    Initialization
        •    Enumeration
        •    Class Specific requests
        •    data transfer
  Configuration
  The MSS USB driver stack must first be configured in the USB device mode using the
  MSS_USB_PERIPHERAL_MODE constant to use this driver. No other configuration is
  necessary.

Initialization
  The HID class driver must be initialized using the MSS_USBD_HID_init() function.
  Once initialized, this driver will get configured by the USBD driver during
  the enumeration process. The call-back function usbd_hid_init_cb() is
  implemented by this driver which will be called by the USBD driver when the
  host configures this device. The USBD_HID_get_descriptor() function is
  implemented to provide class specific descriptor table to the USBD driver.

  This driver defines descriptor table which contains a configuration descriptor,
  an Interface descriptor, a HID class descriptor and an Interrupt IN endpoint
  descriptor for successful enumeration as a HID class device.

  Note: For successful enumeration, device specific descriptors must be provided
  by application using MSS_USBD_set_desc_cb_handler() function to the
  USBD driver. Since device, string descriptors etc. are not specific to a USB
  class they are not part of HID Class driver.

  Class Specific requests
  The requests specific to HID Class are handled internally by this driver. The
  call-back function usbd_hid_process_request_cb(), is implemented by this
  driver which processes the HID class specific requests.

  Data transfer
  This driver only needs to perform the USB IN transfers on the Interrupt
  endpoint. The period at which this transfer is repeated is defined by the
  endpoint descriptor during enumeration process. The application must use the
  MSS_USBD_HID_tx_report() function to provide the report to be transmitted.
  The application can use the MSS_USBD_HID_tx_done() function to know if the
  previously provided report is transmitted over USB. This driver implements
  the usbd_hid_tx_complete_cb()  function which will be called by the USBD
  driver when the data transfers is complete.

 *//*=========================================================================*/

#ifndef __MSS_USB_DEVICE_HID_H_
#define __MSS_USB_DEVICE_HID_H_

#include <stdint.h>
#include "mss_usb_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_DEVICE_ENABLED
/*******************************************************************************
 * Public constants and data structures from HID class driver.
 */
/*******************************************************************************
 USBD-HID configuration definitions: These values will be used by HID driver
 to configure the MSS USB core endpoints. These values are also used for
 generating the descriptor tables which are sent to the USB host during
 enumeration.

 Note1:
 You are allowed to modify these values according to requirement.
 Make sure that the values here are in accordance with the user descriptors
 device descriptor etc), USB 2.0 standard and USB Core Driver operation speed
 (MSS_USB_DEVICE_HS or MSS_USB_DEVICE_FS)

 Note2:
 If you change these constants make sure that the speed dependent descriptor are
 updated accordingly.
 */

#define HID_INTR_TX_EP                                  MSS_USB_TX_EP_1
#define HID_INTR_TX_EP_FIFO_ADDR                        0x200u
#define HID_INTR_TX_EP_FIFO_SIZE                        64u
#define HID_INTR_TX_EP_MAX_PKT_SIZE                     64u


/*Full configuration descriptor length*/
#define HID_CONFIG_DESCR_LENGTH                     (USB_STD_CONFIG_DESCR_LEN + \
                                                    USB_STD_INTERFACE_DESCR_LEN + \
                                                    USB_HID_DESCR_LENGTH + \
                                                    USB_STD_ENDPOINT_DESCR_LEN)

/***************************************************************************//**
 The mss_usbd_hid_state_t is used to provide the information of the current
 state of the HID class driver.This type is intended to be used by the application
 to know if this driver is been configured by the USB host. i.e. the enumeration
 is complete and data transfers can be started.

  USBD_HID_NOT_CONFIGURED -- The USB HID class driver is not configured and it
                             cannot perform the data transfers.
  USBD_HID_CONFIGURED     -- The USB HID class driver is configured by the host
                             and it can perform the data transfers.
*/
typedef enum mss_usbd_hid_state {
    USBD_HID_NOT_CONFIGURED,
    USBD_HID_CONFIGURED
}mss_usbd_hid_state_t;

/***************************************************************************//**
 mss_usbd_hid_report_t
  The mss_usbd_hid_report_t type provides the prototype for the report to be
  sent to the host. The application must provide the report data using this type.

 buttons
 The buttons parameter provides the current state of the Button0 to Button5 on
 mouse in bit0 to bit5 of this parameter.

 x_move
 The x_move parameter provides the current x co-ordinate of the mouse.

 y_move
 The y-move parameter provides the current y co-ordinate of the mouse.

 wheel
 The wheel parameter provides the current wheel position of the mouse.
*/
typedef struct mss_usbd_hid_report {
    int8_t  buttons;
    int8_t  x_move;
    int8_t  y_move;
    int8_t  wheel;
} mss_usbd_hid_report_t;

/***************************************************************************//**
* Exported function from HID Class driver.
*******************************************************************************/
/***************************************************************************//**
  @brief MSS_USBD_HID_init()
  The MSS_USBD_HID_init() function must be used by the application to initialize
  the USBD-HID driver before transmitting the report data.

  @param speed
  The speed parameter indicates the USB speed at which the USBD-HID driver must
  operate.

  @return
   This function does not return any value.

  Example:
  @code
    // Assign call-back function handler structure needed by USBD driver
    MSS_USBD_set_desc_cb_handler(&hid_mouse_descr_cb);

    //Initialize USB Device Core driver
    MSS_USBD_init(MSS_USB_DEVICE_FS);

    //Initialize HID Class driver.
    MSS_USBD_HID_init(MSS_USB_DEVICE_FS);

  @endcode
*/
void
MSS_USBD_HID_init
(
    mss_usb_device_speed_t speed
);

/***************************************************************************//**
  @brief MSS_USBD_HID_tx_report()
  The MSS_USBD_HID_tx_report() function can be used by the application to provide
  the buffer containing the report data to the HID class driver. The report data
  will be loaded into the TX endpoint and will be transmitted when the next IN
  packet arrives from the host.

  @param  buf
  The buf parameter indicates the address of the buffer containing the HID
  report data.

  @param  length
  The length parameter indicates the number of bytes to be transmitted.

  @return
  This function return a non-zero value if the function was successfully
  executed. A zero return value indicates that the transmission was not started.

  Example:
  @code
  @endcode
*/
uint32_t
MSS_USBD_HID_tx_report
(
    uint8_t* buf,
    uint32_t length
);

/***************************************************************************//**
  @brief MSS_USBD_HID_tx_done()
  The MSS_USBD_HID_tx_done() function can be used by the application to find out
  if the transmission of the previously provided report is complete.

  @param
    This function takes no parameters.

  @return
  This function returns a non-zero value to indicate the previously provided
  report is successfully transmitted. A zero value indicates that the
  transmission is not yet complete.

  Example:
  @code
        if(MSS_USBD_HID_tx_done())
        {
            MSS_USBD_HID_tx_report ((uint8_t*)&report, sizeof(report));
        }

  @endcode
*/
uint8_t
MSS_USBD_HID_tx_done
(
    void
);

#endif  //MSS_USB_DEVICE_ENABLED

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_DEVICE_HID_H_ */
