/***********************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Code running on e51. MPFS MSS GPIO example. Please see README.txt for details
 *
 * SVN $Revision: 10516 $
 * SVN $Date: 2018-11-08 18:09:23 +0000 (Thu, 08 Nov 2018) $
 */

#include <stdio.h>
#include <string.h>
#include "mpfs_hal/mss_hal.h"
#include "drivers/mss_uart/mss_uart.h"
#include "drivers/mss_gpio/mss_gpio.h"

/******************************************************************************
 * Instruction message. This message will be transmitted over the UART to
 * HyperTerminal when the program starts.
 *****************************************************************************/
const uint8_t g_message1[] =
"\r\n\r\n\r\n **** PolarFire SoC MSS GPIO example ****\r\n\r\n\r\n";

uint8_t g_message2[] =
"This program is run from E51.\r\n\r\n\
The GPIO0_0, GPIO0_1, GPIO0_2, GPIO0_3 and GPIO2_0, are configured as INOUT\r\n\
and the outputs are connected to the LEDs.\r\n\
Observe the LEDS blinking. LEDs toggle every time the SYSTICK timer expires\r\n\
\r\n\
GPIO0_0, GPIO0_1 and GPIO2_0 inputs are connected to switches.\r\n\
A message is generated when a respective GPIO input interrupt is asserted when \
\r\n the switch is pressed.\r\n";

#define RX_BUFF_SIZE    64
uint8_t g_rx_buff[RX_BUFF_SIZE] = {0};
volatile uint32_t count_sw_ints_h0 = 0U;
volatile uint8_t g_rx_size = 0;
uint64_t uart_lock;

static volatile uint32_t irq_cnt = 0;

/*Created for convenience. Uses Polled method of UART TX*/
static void uart_tx_with_mutex (mss_uart_instance_t * this_uart,
                                uint64_t mutex_addr,
                                const uint8_t * pbuff,
                                uint32_t tx_size)
{
    mss_take_mutex(mutex_addr);
    MSS_UART_polled_tx(this_uart, pbuff, tx_size);
    mss_release_mutex(mutex_addr);
}

/* This is the handler function for the UART RX interrupt over PLIC.
 * In this example project UART0 PLIC interrupt is enabled on hart0.
 */
void uart0_rx_handler (mss_uart_instance_t * this_uart)
{
    uint32_t hart_id = read_csr(mhartid);
    int8_t info_string[50];

    irq_cnt++;
    sprintf(info_string,"UART0 irq_cnt = %x \r\n\r\n", irq_cnt);

    /*This will execute when interrupt from HART 0 is raised */
    mss_take_mutex((uint64_t)&uart_lock);
    g_rx_size = MSS_UART_get_rx(this_uart, g_rx_buff, sizeof(g_rx_buff));
    mss_release_mutex((uint64_t)&uart_lock);

    uart_tx_with_mutex(&g_mss_uart0_lo, (uint64_t)&uart_lock,
                        info_string, strlen(info_string));
}

/* Main function for the HART0(E51 processor).
 * Application code running on HART0 is placed here.
 * UART0 PLIC interrupt is enabled on hart0.*/
void e51(void)
{
    int8_t info_string[100];
    uint64_t mcycle_start = 0U;
    uint64_t mcycle_end = 0U;
    uint64_t delta_mcycle = 0U;
    uint64_t hartid = read_csr(mhartid);
    uint8_t cnt, int_num;

    /*Bring the UART0, GPIO0, GPIO1 and GPIO2 out of Reset*/
    SYSREG->SOFT_RESET_CR &= ~( (1u << 0u) | (1u << 5u) | (1u << 20u) |
                                (1u << 21u) | (1u << 22u)) ;

    PLIC_init();
    PLIC_SetPriority(MMUART0_PLIC_77, 2);

    __enable_irq();

    /* All HARTs use MMUART0 to display messages on Terminal. This mutex helps
     * serializing MMUART0 accesses from multiple HARTs.*/
    mss_init_mutex((uint64_t)&uart_lock);

    MSS_UART_init( &g_mss_uart0_lo,
            MSS_UART_115200_BAUD,
            MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    /*Receive interrupt is enabled now. The interrupt will be received on the
     * PLIC. Please see uart0_rx_handler() for more details */
    MSS_UART_set_rx_handler(&g_mss_uart0_lo,
                            uart0_rx_handler,
                            MSS_UART_FIFO_SINGLE_BYTE);

    uart_tx_with_mutex(&g_mss_uart0_lo, (uint64_t)&uart_lock,
                       g_message1, sizeof(g_message1));

    uart_tx_with_mutex(&g_mss_uart0_lo, (uint64_t)&uart_lock,
                       g_message2, sizeof(g_message2));


    mcycle_start = readmcycle();

    /*Configure Systick. The tick rate is configured in mss_sw_config.h*/
    SysTick_Config();

    /*Making sure that the default GPIO0 and GPIO1 are used on interrupts.
     * No Non-direct interrupts enabled of GPIO0 and GPIO1.
     * Please see the mss_gpio.h for more description on how GPIO interrupts
     * are routed to the PLIC*/
    SYSREG->GPIO_INTERRUPT_FAB_CR = 0x00000000UL;

    PLIC_SetPriority_Threshold(0);

    for (int_num = 0; int_num <= GPIO2_NON_DIRECT_PLIC; int_num++)
    {
        PLIC_SetPriority(GPIO0_BIT0_or_GPIO2_BIT0_PLIC_0 + int_num, 2);
    }

    MSS_GPIO_init(GPIO0_LO);

    for (cnt = 0; cnt< 2; cnt++)
    {
        MSS_GPIO_config(GPIO0_LO,
                        cnt,
                        MSS_GPIO_INOUT_MODE | MSS_GPIO_IRQ_EDGE_POSITIVE);

        MSS_GPIO_enable_irq(GPIO0_LO, cnt);
    }

    MSS_GPIO_init(GPIO2_LO);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_0, MSS_GPIO_INOUT_MODE |
                              MSS_GPIO_IRQ_EDGE_POSITIVE);

    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_0, 0x0);

    /*since GPIO0_0 is previously enabled, GPIO2_0 will be routed to
     * GPIO2 non-direct interrupt.*/
    MSS_GPIO_enable_irq(GPIO2_LO, MSS_GPIO_0);
    MSS_GPIO_enable_nondirect_irq(GPIO2_LO);

    while (1u)
    {
        if (g_rx_size > 0u)
        {
            switch (g_rx_buff[0u])
            {

            case '0':
                mcycle_end      = readmcycle();
                delta_mcycle    = mcycle_end - mcycle_start;
                sprintf(info_string,"Hart %ld, %ld delta_mcycle \r\n", hartid,
                        delta_mcycle);
                uart_tx_with_mutex(&g_mss_uart0_lo, (uint64_t)&uart_lock,
                                    info_string, strlen(info_string));
                break;
            default:
                /*Echo the characters received from the terminal*/
                uart_tx_with_mutex(&g_mss_uart0_lo, (uint64_t)&uart_lock,
                                    g_rx_buff, g_rx_size);
                break;
            }

            g_rx_size = 0;
        }
    }
}

/* HART0 Software interrupt handler */
void Software_h0_IRQHandler (void)
{
    uint32_t hart_id = read_csr(mhartid);
    count_sw_ints_h0++;
}

uint8_t gpio0_bit0_or_gpio2_bit13_plic_0_IRQHandler(void)
{
    uart_tx_with_mutex(&g_mss_uart0_lo, (uint64_t)&uart_lock,
            "GPIO0_LO generating an interrupt_0\r\n",
            sizeof("GPIO0_LO generating an interrupt_0\r\n"));

    MSS_GPIO_clear_irq(GPIO0_LO, MSS_GPIO_0);

    return EXT_IRQ_KEEP_ENABLED;
}

uint8_t gpio0_bit1_or_gpio2_bit13_plic_1_IRQHandler(void)
{
    uart_tx_with_mutex(&g_mss_uart0_lo, (uint64_t)&uart_lock,
            "GPIO0_LO generating an interrupt_1\r\n",
            sizeof("GPIO0_LO generating an interrupt_1\r\n"));

    MSS_GPIO_clear_irq(GPIO0_LO, MSS_GPIO_1);

    return EXT_IRQ_KEEP_ENABLED;
}

uint8_t gpio1_bit0_or_gpio2_bit14_plic_14_IRQHandler(void)
{
    uart_tx_with_mutex(&g_mss_uart0_lo, (uint64_t)&uart_lock,
            "GPIO1_LO generating an interrupt_1\r\n",
            sizeof("GPIO1_LO generating an interrupt_0\r\n"));

    MSS_GPIO_clear_irq(GPIO1_LO, MSS_GPIO_0);

    return EXT_IRQ_KEEP_ENABLED;
}
uint8_t gpio0_non_direct_plic_IRQHandler(void)
{
    uint32_t intr_num = 0;
    intr_num = MSS_GPIO_get_irq(GPIO0_LO);

    for(int cnt=0; cnt<14; cnt++)
    {
        if (1u == (intr_num & 0x00000001U))
        {
            uart_tx_with_mutex(&g_mss_uart0_lo, (uint64_t)&uart_lock,
                  "NON_DIRECT_GPIO0_LO\r\n", sizeof("NON_DIRECT_GPIO0_LO\r\n"));

            MSS_GPIO_clear_irq(GPIO0_LO, (mss_gpio_id_t)cnt);
        }

        intr_num >>= 1u;
    }

    return EXT_IRQ_KEEP_ENABLED;
}

uint8_t gpio1_non_direct_plic_IRQHandler(void)
{
    uint32_t intr_num = 0;
    intr_num = MSS_GPIO_get_irq(GPIO1_LO);

    for(int cnt=0; cnt<24; cnt++)
    {
        if (1u == (intr_num & 0x00000001U))
        {
            uart_tx_with_mutex(&g_mss_uart0_lo, (uint64_t)&uart_lock,
                  "NON_DIRECT_GPIO1_LO\r\n", sizeof("NON_DIRECT_GPIO1_LO\r\n"));

            MSS_GPIO_clear_irq(GPIO1_LO, (mss_gpio_id_t)cnt);
        }

        intr_num >>= 1u;
    }

    return EXT_IRQ_KEEP_ENABLED;
}

uint8_t gpio2_non_direct_plic_IRQHandler(void)
{
    uint32_t intr_num = 0;
    intr_num = MSS_GPIO_get_irq(GPIO2_LO);

    for(int cnt=0; cnt<32; cnt++)
    {
        if (1u == (intr_num & 0x00000001U))
        {
            uart_tx_with_mutex(&g_mss_uart0_lo, (uint64_t)&uart_lock,
                  "NON_DIRECT_GPIO2_LO\r\n", sizeof("NON_DIRECT_GPIO2_LO\r\n"));

            MSS_GPIO_clear_irq(GPIO2_LO, (mss_gpio_id_t)cnt);
        }

        intr_num >>= 1u;
    }

    return EXT_IRQ_KEEP_ENABLED;
}

void SysTick_Handler(uint32_t hard_id)
{
    static volatile uint8_t value = 0u;
    if (0u == hard_id)
    {
        if(0 == value)
        {
            value = 0x0f;
        }
        else
        {
            value = 0x00;
        }
        MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_0, value);
        MSS_GPIO_set_outputs(GPIO0_LO, value);
    }
}
