/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC MSS USB Driver Stack
 *      USB Core Interface Layer (USB-CIFL)
 *          USBD-CIF
 *
 *  This file provides interfaces to perform register and register bit level
 *  read / write operations in USB Device mode.
 *
 */

#ifndef __MSS_USB_DEVICE_REG_IO_H_
#define __MSS_USB_DEVICE_REG_IO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mss_usb_core_regs.h"


/*******************************************************************************
* Device mode functions
*/

/*******************************************************************************
 * POWER register related APIs
 */
/*lint -e20 -e522 -e10 -e40 -e63 -e35 -e26 -e78*/
static __INLINE uint8_t
MSS_USBD_CIF_is_suspend_mode
(
    void
)
{
    return(((USB->POWER & POWER_REG_SUSPEND_MODE_MASK) ?
            MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

/*To force out of the Suspend while in Device mode.*/
static __INLINE void
MSS_USBD_CIF_resume_gen
(
    void
)
{
    USB->POWER |= POWER_REG_RESUME_SIGNAL_MASK;
    /*clear after 10ms*/
}

/*******************************************************************************
 * DEVCTL register related APIs
 */
static __INLINE uint8_t
MSS_USB_OTG_is_session_on
(
    void
)
{
    return (((USB->DEV_CTRL & DEV_CTRL_SESSION_MASK) ?
            MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void
MSS_USB_OTG_initiate_srp
(
    void
)
{
    /*
    TODO:make sure that the device is suspended and is a B-device when this
    function is called
    */
    USB->DEV_CTRL |= DEV_CTRL_SESSION_MASK;
}

static __INLINE void
MSS_USB_OTG_initiate_hnp
(
    void
)
{
    /*TODO:make sure that the device is B-device when this function is called*/
    USB->DEV_CTRL |= DEV_CTRL_HOST_REQ_MASK;
}

/*lint -restore */
#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_DEVICE_REG_IO_H_ */
