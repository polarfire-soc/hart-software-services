/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * code running on e51
 *
 * PolarFire SoC MSS RTC interrupt example project
 *
 * SVN $Revision: 12632 $
 * SVN $Date: 2019-12-10 11:42:56 +0530 (Tue, 10 Dec 2019) $
 */

#include "drivers/mss_uart/mss_uart.h"
#include "drivers/mss_rtc/mss_rtc.h"
#include "config/hardware/clocks/hw_cfg_clocks.h"

/* Constant used for setting RTC control register.
 */
#define BIT_SET 0x00010000U

/* 1MHz clock is RTC clock source. */
#define RTC_PERIPH_PRESCALER              (1000000u - 1u)

uint64_t uart_lock;

/******************************************************************************
 *  Greeting messages displayed over the UART.
 */
const uint8_t g_greeting_msg[] =
"\r\n\r\n\t  ******* PolarFire SoC RTC Interrupt Example *******\n\n\n\r\
The example project demonstrate the periodic RTC interrupt. The UART\r\n\
message will be printed at each RTC interrupt occurrence. \r\n\n\n\
";

/* RTC wakeup PLIC handler.
 */
uint8_t rtc_wakeup_plic_IRQHandler(void)
{
    /* Trigger the arrow display function. */
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, "RTC interrupt\n\r");

    /* Clear RTC match interrupt. */
    MSS_RTC_clear_irq();

    return EXT_IRQ_KEEP_ENABLED;
}

/* Main function for the HART0(E51 processor).
 * Application code running on HART0 is placed here.
 */
void e51(void)
{
    uint32_t alarm_period = 1u;
    uint32_t temp;
    uint64_t hash[4];

    PLIC_init();
    __enable_irq();
    PLIC_SetPriority(RTC_WAKEUP_PLIC, 2);

    SYSREG->SUBBLK_CLOCK_CR = 0xffffffff;
    SYSREG->SOFT_RESET_CR &= ~( (1u << 0u) | (1u << 4u) | (1u << 5u) |
            (1u << 19u) | (1u << 23u) | (1u << 28u) | (1u << 18u)) ; /* RTC*/

    MSS_UART_init(&g_mss_uart0_lo,
            MSS_UART_115200_BAUD,
            MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT
    );

    MSS_UART_polled_tx_string(&g_mss_uart0_lo, g_greeting_msg);

    temp = BIT_SET;
    SYSREG->RTC_CLOCK_CR &= ~BIT_SET;
    SYSREG->RTC_CLOCK_CR = MSS_RTC_TOGGLE_CLK / 100000UL;
    SYSREG->RTC_CLOCK_CR |= BIT_SET;

    /* Initialize RTC. */
    MSS_RTC_init(MSS_RTC_LO_BASE, MSS_RTC_BINARY_MODE, RTC_PERIPH_PRESCALER / 10u );

    /* Set initial RTC count and match values. */
    MSS_RTC_reset_counter();
    MSS_RTC_set_binary_count_alarm(alarm_period, MSS_RTC_PERIODIC_ALARM);

    /* Enable RTC wakeup interrupt. */
    MSS_RTC_enable_irq();

    /* Enable RTC to start incrementing. */
    MSS_RTC_start();

    /* The RTC periodic alarm is now set. You should be able to see
     * that the rtc_wakeup_plic_IRQHandler is getting called periodically.
     */
    for (;;)
    {

    }

}
