/***********************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * code running on e51
 *
 * SVN $Revision$
 * SVN $Date$
 */

#include <stdio.h>
#include <string.h>
#include "mpfs_hal/mss_hal.h"
#include "drivers/mss_timer/mss_timer.h"
#include "drivers/mss_uart/mss_uart.h"

uint64_t uart_lock;

/******************************************************************************
 * Instruction message. This message will be transmitted over the UART to
 * HyperTerminal when the program starts.
 *****************************************************************************/
uint8_t g_message[] =
        "\r\n\r\n\r\n **** PolarFire SoC MSS TIMER example ****\r\n\n\n\
        \r\nThis example project configures PolarFire SoC MSS Timer\r\n\
        in 64 bit periodic mode.\r\n\n";

uint8_t g_message2[] =
    "\rObserve the messages on UART terminal at periodic programmed delays.\
    \r\nThe messages are displayed when the timer interrupt occurs \r\n\n";

/* Main function for the HART0(E51 processor).
 * Application code running on HART0 is placed here.
 */
void e51(void)
{
    uint32_t timer64_load_value;
    uint32_t hartid = read_csr(mhartid);
    SYSREG->SUBBLK_CLOCK_CR = 0xffffffff;

    SYSREG->SOFT_RESET_CR &= ~( (1u << 0u) | (1u << 4u) | (1u << 5u) |
                                    (1u << 19u) | (1u << 23u) | (1u << 28u));

    /*This mutex is used to serialize accesses to UART0 when all harts want to
     * TX/RX on UART0. This mutex is shared across all harts.*/
    mss_init_mutex((uint64_t)&uart_lock);

    MSS_UART_init( &g_mss_uart0_lo,
            MSS_UART_115200_BAUD,
            MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    PLIC_init();
    PLIC_SetPriority(MMUART0_PLIC_77, 2);
    PLIC_SetPriority(TIMER1_PLIC, 2);
   __enable_irq();

    mss_take_mutex((uint64_t)&uart_lock);
    MSS_UART_polled_tx(&g_mss_uart0_lo, g_message,strlen(g_message));
    MSS_UART_polled_tx(&g_mss_uart0_lo, g_message2,strlen(g_message2));
    mss_release_mutex((uint64_t)&uart_lock);

     /*--------------------------------------------------------------------------
      * Configure Timer64
      */
     MSS_TIM64_init(TIMER_LO, MSS_TIMER_PERIODIC_MODE);
     /*
      * Use the timer input frequency as load value to achieve a one second
      * periodic interrupt.
      */
     timer64_load_value = 25000000;
     MSS_TIM64_load_immediate(TIMER_LO, 0, timer64_load_value);
     MSS_TIM64_start(TIMER_LO);
     MSS_TIM64_enable_irq(TIMER_LO);

    for(;;)
    {
        ;
    }
}

uint8_t timer1_plic_IRQHandler()
{

    /*Print informative message on UART terminal for each interrupt occurrence*/
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,
                              "PSE_Timer_64 interrupt example\r\n");

    /* Clear TIM64 interrupt */
    MSS_TIM64_clear_irq(TIMER_LO);

    return EXT_IRQ_KEEP_ENABLED;
}
