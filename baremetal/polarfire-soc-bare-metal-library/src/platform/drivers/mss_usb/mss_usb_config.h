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
 *
 * MSS USB Driver stack configuration parameters.
 * User must choose the constant definitions in this file to select the mode of
 * operation.
 * The constants defined in this file are used by MSS USB driver stack to
 * function as per configuration.
 *
 * SVN $Revision$
 * SVN $Date$
 */

#ifndef __MSS_USB_CONFIG_H_
#define __MSS_USB_CONFIG_H_

/*-------------------------------------------------------------------------*//**
  User should choose the Mode in which PolarFire SoC MSS USB should operate
*/
/* #define MSS_USB_OTG_DUAL_ROLE_MODE */
/* #define MSS_USB_OTG_PERIPHERAL_MODE*/

/* Configures the MSS USB Driver Stack to operate in USB Host mode. */
   #define MSS_USB_OTG_HOST_MODE

/* Configures the MSS USB Driver Stack to operate in USB Device mode. */
/* #define MSS_USB_PERIPHERAL_MODE */

/* Used for internal testing of the driver. Not for Application use. */
/* #define MSS_USB_DEVICE_TEST_MODE */

/*-------------------------------------------------------------------------*//**
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
