/*******************************************************************************
 * (c) Copyright 2018 Microsemi SoC Products Group. All rights reserved.
 *
 * Smartfusion2 MSS USB Driver Stack
 *
 * MSS USB Driver stack configuration parameters.
 * User must choose the constant definitions in this file to select the mode of
 * operation.
 * The constants defined in this file are used by MSS USB driver stack to
 * function as per configuration.
 *
 * SVN $Revision: 10280 $
 * SVN $Date: 2018-08-01 16:36:37 +0530 (Wed, 01 Aug 2018) $
 */

#ifndef __MSS_USB_CONFIG_H_
#define __MSS_USB_CONFIG_H_

/***************************************************************************//**
  User should choose the Mode in which Smartfusion2 MSS USB should operate
*/
/* #define MSS_USB_OTG_DUAL_ROLE_MODE */
/* #define MSS_USB_OTG_PERIPHERAL_MODE*/

/* Configures the MSS USB Driver Stack to operate in USB Host mode.*/
//   #define MSS_USB_OTG_HOST_MODE

/* Configures the MSS USB Driver Stack to operate in USB Device mode. */
 #define MSS_USB_PERIPHERAL_MODE

/*Used for internal testing of the driver. Not for Application use*/
/* #define MSS_USB_DEVICE_TEST_MODE */

/***************************************************************************//**
   Definitions Internally generated for use in the Core and logical layer.
*/
#ifdef MSS_USB_OTG_DUAL_ROLE_MODE
    #define MSS_USB_HOST_ENABLED
    #define MSS_USB_DEVICE_ENABLED
    #define MSS_USB_OTG_SRP_ENABLED
    #define MSS_USB_OTG_HNP_ENABLED
#endif

#ifdef MSS_USB_OTG_PERIPHERAL_MODE
    #define MSS_USB_DEVICE_ENABLED
    #define MSS_USB_OTG_SRP_ENABLED
#endif

#ifdef MSS_USB_PERIPHERAL_MODE
    #define MSS_USB_DEVICE_ENABLED
    #define MSS_USB_DEVICE_PRINTER
#endif

#ifdef MSS_USB_OTG_HOST_MODE
    #define MSS_USB_HOST_ENABLED
    #define MSS_USB_OTG_SRP_ENABLED
#endif

#endif  /* __MSS_USB_CONFIG_H_ */
