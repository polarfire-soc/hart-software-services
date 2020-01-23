/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * code running on e51
 *
 * PolarFire SoC MSS RTC time example project
 *
 * SVN $Revision: 12632 $
 * SVN $Date: 2019-12-10 11:42:56 +0530 (Tue, 10 Dec 2019) $
 */

#include <stdio.h>
#include "drivers/mss_uart/mss_uart.h"
#include "drivers/mss_rtc/mss_rtc.h"
#include "config/hardware/clocks/hw_cfg_clocks.h"

/* Constant used for setting RTC control register.
 */
#define BIT_SET 0x00010000U

/* 1MHz clock is RTC clock source. */
#define RTC_PERIPH_PRESCALER              (1000000u - 1u)

/* Constant used for setting RTC control register.
 */
#define BIT_SET 0x00010000U

/* 1MHz clock is RTC clock source. */
#define RTC_PERIPH_PRESCALER              (1000000u - 1u)

uint64_t uart_lock;
uint8_t display_buffer[100];

/******************************************************************************
 *  Greeting messages displayed over the UART.
 */
const uint8_t g_greeting_msg[] =
        "\r\n\r\n\t  ******* PolarFire SoC RTC Time Example *******\n\n\n\r\
The example project demonstrate the RTC time mode. The UART\r\n\
message will be displayed at each second. \r\n\n\n\
";

/* Main function for the HART0(E51 processor).
 * Application code running on HART0 is placed here.
 */
void e51(void)
{
    mss_rtc_calender_t calendar_count;

    /* RTC */
    SYSREG->SOFT_RESET_CR &= ~((1u << 0u) | (1u << 4u) | (1u << 5u) |
                        (1u << 19u) | (1u << 23u) | (1u << 28u) | (1u << 18u)) ;

    MSS_UART_init(&g_mss_uart0_lo,
            MSS_UART_115200_BAUD,
            MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT
    );

    PLIC_init();
    __enable_irq();
    PLIC_SetPriority(RTC_WAKEUP_PLIC, 2);

    MSS_UART_polled_tx_string(&g_mss_uart0_lo, g_greeting_msg);

    SYSREG->RTC_CLOCK_CR &= ~BIT_SET;
    SYSREG->RTC_CLOCK_CR = MSS_RTC_TOGGLE_CLK / 100000UL;
    SYSREG->RTC_CLOCK_CR |= BIT_SET;

    /* Initialize RTC. */
    MSS_RTC_init(MSS_RTC_LO_BASE, MSS_RTC_CALENDAR_MODE, RTC_PERIPH_PRESCALER );

    /* Enable RTC to start incrementing. */
    MSS_RTC_start();

    for (;;)
    {
        volatile uint32_t rtc_count_updated;
        /* Update displayed time if value read from RTC changed since last read.*/
        rtc_count_updated = MSS_RTC_get_update_flag();
        if(rtc_count_updated)
        {
            MSS_RTC_get_calendar_count(&calendar_count);
            snprintf((char *)display_buffer, sizeof(display_buffer),
                      "Seconds: %02d",(int)(calendar_count.second));

            MSS_UART_polled_tx_string (&g_mss_uart0_lo, display_buffer);
            MSS_UART_polled_tx_string (&g_mss_uart0_lo, "\r\n");
            MSS_RTC_clear_update_flag();
        }
    }
    /* never return*/
}

