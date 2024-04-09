/**************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#include "gpio_ui_service.h"

#include "mss_gpio.h"

#if IS_ENABLED(CONFIG_SERVICE_USBDMSC) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_QSPI))
#    include "usbdmsc_service.h"
#endif

#define COREGPIO_BASE_ADDR  0x40000000

#define REPORT_DDR_INIT_START  MSS_GPIO_0_MASK
#define REPORT_DDR_INIT_END    MSS_GPIO_1_MASK
#define REPORT_GOOD_IMAGE_CRC  MSS_GPIO_2_MASK

#define REPORT_USBDMSC          0xAA0
#define REPORT_USBDMSC_MASK     0xFF0

#define REPORT_IAP_IN_PROGRESS  (MSS_GPIO_0_MASK | MSS_GPIO_2_MASK  | MSS_GPIO_3_MASK | MSS_GPIO_5_MASK | MSS_GPIO_6_MASK | MSS_GPIO_8_MASK | MSS_GPIO_9_MASK | MSS_GPIO_11_MASK)
#define REPORT_IAP_FAILURE      (MSS_GPIO_1_MASK | MSS_GPIO_4_MASK | MSS_GPIO_7_MASK | MSS_GPIO_10_MASK)

static bool g_gpio_configured = false;
static uint32_t g_current_status = 0;

static void configure_leds(void)
{
   /* User button */
    MSS_GPIO_init(GPIO0_LO);
    MSS_GPIO_config(GPIO0_LO, MSS_GPIO_13, MSS_GPIO_INPUT_MODE);

    /* User LEDs */
    MSS_GPIO_init(GPIO2_LO);

    for (int gpioIdx = MSS_GPIO_0; gpioIdx < MSS_GPIO_12; gpioIdx++) {
        MSS_GPIO_config(GPIO2_LO, gpioIdx, MSS_GPIO_OUTPUT_MODE);
    }

    MSS_GPIO_set_outputs(GPIO2_LO, 0);

    g_gpio_configured = true;
}

/**************************************************************************/

void GPIO_UI_Init(void)
{
    if (!g_gpio_configured)
    {
        configure_leds();   
    }
}

bool HSS_GPIO_UI_user_button_pressed(void)
{
    bool result = true;

    uint32_t gpio_inputs;
    gpio_inputs = MSS_GPIO_get_inputs(GPIO0_LO);
    if (gpio_inputs & MSS_GPIO_13_MASK) {
        result = false;
    }
    return result;
}

bool HSS_GPIO_UI_Preboot_Check_Button(void)
{
    bool result = HSS_GPIO_UI_user_button_pressed();

    if (result) {
        uint32_t leds;
        //mHSS_FANCY_PUTS(LOG_NORMAL, "User button pressed\n");
        leds = MSS_GPIO_get_outputs(GPIO2_LO);
        leds = (leds & ~REPORT_USBDMSC_MASK) | REPORT_USBDMSC;
        MSS_GPIO_set_outputs(GPIO2_LO, leds);

        // debounce ...
        uint32_t count=1000000u;
        while (HSS_GPIO_UI_user_button_pressed() & (--count)) {
            ;
        }
    }

    return result;
}

void HSS_GPIO_UI_ShowProgress(size_t totalNumTasks, size_t numTasksRemaining)
{
    static uint32_t toggle = 0x400;
    uint32_t leds;

    if (!g_gpio_configured)
    {
        configure_leds();   
    }

    toggle = ~toggle & 0xC00;
    
    leds = MSS_GPIO_get_outputs(GPIO2_LO);
    leds = (leds & ~0xC00) | toggle;
    MSS_GPIO_set_outputs(GPIO2_LO, leds);
}

void HSS_GPIO_UI_ReportDDRInitStart(void)
{
    if (!g_gpio_configured) configure_leds();   

    g_current_status |= REPORT_DDR_INIT_START;
    MSS_GPIO_set_outputs(GPIO2_LO, g_current_status);
}

void HSS_GPIO_UI_ReportDDRInitEnd(void)
{
    if (!g_gpio_configured) configure_leds();   

    g_current_status |= REPORT_DDR_INIT_END;
    MSS_GPIO_set_outputs(GPIO2_LO, g_current_status);
}

void HSS_GPIO_UI_ReportImageGoodCRC(void)
{
    if (!g_gpio_configured) configure_leds();   

    g_current_status |= REPORT_GOOD_IMAGE_CRC;
    MSS_GPIO_set_outputs(GPIO2_LO, g_current_status);
}

void HSS_GPIO_UI_ReportUSBProgress(uint32_t writeCount, uint32_t readCount)
{
    static uint32_t prev_writeCount;
    static uint32_t prev_readCount;
    uint32_t leds;

    if ((writeCount != prev_writeCount) || (readCount != prev_readCount))
    {
        leds = MSS_GPIO_get_outputs(GPIO2_LO);
        leds = (leds & ~REPORT_USBDMSC_MASK) | (~leds & REPORT_USBDMSC_MASK);
        MSS_GPIO_set_outputs(GPIO2_LO, leds);
    }

    prev_writeCount = writeCount;
    prev_readCount = readCount;
}

void GPIO_UI_Shutdown(void)
{
}

