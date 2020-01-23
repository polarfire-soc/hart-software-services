/*******************************************************************************
 * (c) Copyright 2012 Microsemi Corporation.  All rights reserved.
 *
 * USB HID Class Mouse Device example aplication to demonstrate the SmartFusion2
 * MSS USB operations in device mode.
 *
 * This file uses MSS USB Driver stack (incusive of USBD-HID class driver) to
 * connect to USB Host as USB HID Mouse device.
 *
 * SVN $Revision: 7858 $
 * SVN $Date: 2015-09-23 18:12:35 +0530 (Wed, 23 Sep 2015) $
 */

#include "mouse_app.h"
#include "mss_gpio.h"
#include "mss_usb_device.h"
#include "mss_usb_device_hid.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 Switch Mapping - SmartFusion2-DevKit
 SW5 - Up
 SW1 - Down
 SW2 - Right
 SW4 - Left
 SW3 - Select buttongo
*/
#define EAST_BUTTON_MASK                                0x00000004u
#define SOUTH_BUTTON_MASK                               0x00000100u

#define CENTER_BUTTON_MASK                              0x00000008u

#define NORTH_BUTTON_MASK                               0x00000002u
#define WEST_BUTTON_MASK                                0x00000010u


#define MAX_REPORT_VALUE                                127
#define MIN_REPORT_VALUE                                -127
#define DELAY_BETWEEN_SENSES                            50000u

/*This buffer is passed to the USB driver. When USB drivers are configured to
use internal DMA, the address of this buffer must be modulo-4.Otherwise DMA
Transfer will fail.*/
#if defined(__GNUC__)
mss_usbd_hid_report_t report __attribute__ ((aligned (4))) = {0};
#elif defined(__ICCARM__)
#pragma data_alignment = 4
mss_usbd_hid_report_t report = {0};
#elif defined(__CC_ARM)
__align(4) mss_usbd_hid_report_t report = {0};
#endif

/* USB descriptors for HID class mouse enumeration. */
extern mss_usbd_user_descr_cb_t hid_mouse_descriptors_cb;
volatile uint32_t gpio_in;

void
MOUSE_task
(
    void
)
{
    volatile uint32_t delay;
    uint32_t debounce = 0u;

    if(1 == MSS_USBD_HID_tx_done())
    {
        MSS_USBD_HID_tx_report ((uint8_t*)&report, sizeof(report));
//        report.buttons = 0;
//        report.x_move = 0;
//        report.y_move = 0;
//        report.wheel = 0;
    }
    else
    {
        for(delay = 0; delay < DELAY_BETWEEN_SENSES; ++delay)
        {
            ;
        }
//
//        gpio_in = MSS_GPIO_get_inputs(GPIO0_LO);
//
//        if(((~gpio_in) & CENTER_BUTTON_MASK))
//        {
//            if(0u == debounce)
//            {
//                debounce = 1;
//                report.buttons = 1;
//            }
//        }
//        else
//        {
//            debounce = 0;
//        }
//
//        if(((~gpio_in) & WEST_BUTTON_MASK))
//        {
//            if(report.x_move > MIN_REPORT_VALUE)
//            {
//                --report.x_move;
//            }
//        }
//
//        if(((~gpio_in) & EAST_BUTTON_MASK))
//        {
//            if(report.x_move < MAX_REPORT_VALUE)
//            {
//                ++report.x_move;
//            }
//        }
//
//        if(((~gpio_in) & SOUTH_BUTTON_MASK))
//        {
//            if(report.y_move > MIN_REPORT_VALUE)
//            {
//                --report.y_move;
//            }
//        }
//
//        if(((~gpio_in) & NORTH_BUTTON_MASK))
//        {
//            if( report.y_move < MAX_REPORT_VALUE)
//            {
//                ++report.y_move;
//            }
//        }

//        if( report.y_move < 30)
//        {
//            ++report.y_move;
//        }
//        else
//            report.y_move = -30;

        if(report.x_move < (int8_t)30)
        {
            ++report.x_move;
        }
        else
            report.x_move = -30;
    }
}

void
MOUSE_init
(
void
)
{
    MSS_GPIO_init(GPIO0_LO);
//    MSS_GPIO_config(GPIO0_LO, MSS_GPIO_0 , MSS_GPIO_OUTPUT_MODE);

    MSS_GPIO_config(GPIO0_LO, MSS_GPIO_1 , MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO0_LO, MSS_GPIO_2 , MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO0_LO, MSS_GPIO_3 , MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO0_LO, MSS_GPIO_4 , MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO0_LO, MSS_GPIO_8 , MSS_GPIO_INPUT_MODE);

    /*Keep USB PHY out of Reset*/
//    MSS_GPIO_set_output(GPIO0_LO, MSS_GPIO_0 , 0u);

    /*
    Assign call-back function handler structure needed by USB Device Core driver
    */
    MSS_USBD_set_descr_cb_handler(&hid_mouse_descriptors_cb);

    /*Initialize HID Class driver.*/
    MSS_USBD_HID_init(MSS_USB_DEVICE_HS);

    /*Initialize USB Device Core driver.*/
    MSS_USBD_init(MSS_USB_DEVICE_HS);
}

#ifdef __cplusplus
}
#endif
