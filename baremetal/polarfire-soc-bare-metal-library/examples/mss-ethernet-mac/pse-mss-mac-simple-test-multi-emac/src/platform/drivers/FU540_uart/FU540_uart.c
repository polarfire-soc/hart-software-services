/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * FU540_uart.c
 *
 *  Created on: Apr 12, 2018
 *      Author: malachy.lynch
 */
#if (PSE==0)

#ifdef __cplusplus
extern "C" {
#endif

#include "FU540_uart.h"
#include "../../mpfs_hal/mss_plic.h"
#include "../../mpfs_hal/mss_util.h"

#include "../../hal/hal.h"


mss_FU540_uart_instance_t g_mss_FU540_uart0;
mss_FU540_uart_instance_t g_mss_FU540_uart1;

#define ASSERT(CHECK)

void
MSS_FU540_UART_init
(
    mss_FU540_uart_instance_t* this_uart,
    uint32_t baud_rate,
    uint8_t line_config
)
{
    static uint8_t uart0buffer[256];
    /* The driver expects g_mss_uart0 and g_mss_uart1 to be the only
     * mss_uart_instance_t instances used to identify UART0 and UART1. */
    ASSERT((this_uart == &g_mss_uart0) || (this_uart == &g_mss_uart1));


    this_uart = &g_mss_FU540_uart0;

    this_uart->hw_reg = (FU540_UART *) BASE32_ADDR_FU540_UART0;

    this_uart->hw_reg->txctrl = ((0x01<<16) | 0x01); /* 0x01 => tx enable, (0x01<<18) => watermark = 1; */
    this_uart->hw_reg->rxctrl = ((0x01<<16) | 0x01);

    this_uart->tx_buffer      =  (uint8_t *)uart0buffer;          /*!< Pointer to transmit buffer. */
    this_uart->tx_buff_size   = sizeof(uint32_t) * 256;           /*!< Transmit buffer size. */

    __enable_irq();

    /* next enable UART0 interrupt on PLIC */

    /* Perform generic initialization */
//    global_init(this_uart, baud_rate, line_config);

    /* set default tx handler for automated TX using interrupt in USART mode */
//    this_uart->tx_handler = default_tx_handler;
}



/***************************************************************************//**
 * See mss_uart.h for details of how to use this function.
 */
uint32_t
MSS_FU540_UART_get_rx
(
    mss_FU540_uart_instance_t * this_uart,
    uint8_t * rx_buff,
    uint32_t buff_size
)
{
    size_t rx_size = 0u;
    uint32_t status = 0u;

    ASSERT(rx_buff != ((uint8_t *)0));
    ASSERT(buff_size > 0u);

    if((rx_buff != (uint8_t *)0) && (buff_size > 0u))
    {
        status = this_uart->hw_reg->rxdata;
        this_uart->status |= status;

        while(((status & RX_EMPTY_MASK) == 0u) && (rx_size < buff_size))
        {
            rx_buff[rx_size] = (uint8_t)status;
            ++rx_size;
            status = this_uart->hw_reg->rxdata;
            this_uart->status |= status;
        }
    }

    return rx_size;
}

void
MSS_FU540_UART_interrupt_tx
(
    mss_FU540_uart_instance_t * this_uart,
    const uint8_t * pbuff,
    uint32_t tx_size
)
{
    MSS_FU540_UART_int_tx ( this_uart, pbuff, tx_size );
}

/***************************************************************************//**
 * See mss_uart.h for details of how to use this function.
 */
void
MSS_FU540_UART_polled_tx
(
    mss_FU540_uart_instance_t * this_uart,
    const uint8_t * pbuff,
    uint32_t tx_size
)
{
    uint32_t char_idx = 0u;
    uint32_t status;

    ASSERT((this_uart == &g_mss_uart0) || (this_uart == &g_mss_uart1));
    ASSERT(pbuff != ( (uint8_t *)0));
    ASSERT(tx_size > 0u);
    //if(this_uart == &g_mss_FU540_uart0)
    //fixme: there is a bug on the stack here, stack gets blown out of the water here */
    if(((this_uart == &g_mss_FU540_uart0) || (this_uart == &g_mss_FU540_uart1)) &&
        (pbuff != ((uint8_t *)0)) && (tx_size > 0u))
    {
         /* Remain in this loop until the entire input buffer
          * has been transferred to the UART.
          */
        do {
            /* Read the Status and update the sticky record */
            status = this_uart->hw_reg->txdata;
            this_uart->status = (status & TX_FULL_MASK);

            /* Check if TX FIFO is empty. */
            if(status == 0)
            {
                this_uart->hw_reg->txdata = pbuff[char_idx++];

                /* Calculate the number of untransmitted bytes remaining. */
                tx_size -= (uint32_t)1;
            }
        } while(tx_size);
    }
}


/**
 * Transmits using an interrupt
 * @param this_uart
 * @param pbuff
 * @param tx_size
 */
void
MSS_FU540_UART_int_tx
(
    mss_FU540_uart_instance_t * this_uart,
    const uint8_t * pbuff,
    uint32_t tx_size
)
{
    uint32_t char_idx = 0u;
    volatile int ix;

    uint32_t status;

    ASSERT((this_uart == &g_mss_uart0) || (this_uart == &g_mss_uart1));
    ASSERT(pbuff != ( (uint8_t *)0));
    ASSERT(tx_size > 0u);

    if(((this_uart == &g_mss_FU540_uart0) || (this_uart == &g_mss_FU540_uart1)) &&
        (pbuff != ((uint8_t *)0)) && (tx_size > 0u))
    {
         /* Remain in this loop until the entire input buffer
          * has been transferred to the UART.
          */


        do {
            /*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
               fixme: had to add in to get print to work properly when using interrupt
               do not know why, needs to be investigated  */
            volatile uint32_t i, dummy_h1 = 0;
            uint32_t num_loops = 100;  /* if this is changed to 10, print does not work correctly */

            for(i = 0; i < num_loops; i++)
            {
                dummy_h1 = i;
            }
            /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
            /* Read the Status and update the sticky record */
            status = this_uart->hw_reg->txdata;
            this_uart->status = (status & TX_FULL_MASK);

            /* Check if TX FIFO is empty. */
            while(this_uart->hw_reg->txdata & TX_FULL_MASK)
                ix++;
            {
#if 0
                this_uart->hw_reg->txdata = pbuff[char_idx];
#else
                this_uart->tx_buffer[0] = pbuff[char_idx++];
                this_uart->hw_reg->ie     = 0x01;                 /* prepare interrupt */
#endif

                /* Calculate the number of untransmitted bytes remaining. */
                tx_size -= (uint32_t)1;
            }
        } while(tx_size);
    }

}



#ifdef __cplusplus
}
#endif


uint8_t USART0_plic_4_IRQHandler(void)
{
    volatile int ix;
    g_mss_FU540_uart0.hw_reg->ie = 0x00;   /* disable UART interrupt */

    /* Check if TX FIFO is empty. */
    while(g_mss_FU540_uart0.hw_reg->txdata & TX_FULL_MASK)
        ix++;
    g_mss_FU540_uart0.hw_reg->txdata = g_mss_FU540_uart0.tx_buffer[0];

    return EXT_IRQ_KEEP_ENABLED;
}

#endif
