/***************************************************************************
 * Copyright 2019 Microchip Corporation.
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
 * FU540-C000 contains two UART instances.
 *
 * SVN $Revision: 9661 $
 * SVN $Date: 2018-01-15 10:43:33 +0000 (Mon, 15 Jan 2018) $
 */

#ifndef FU540_uart_H_
#define FU540_uart_H_

#include <stdint.h>                       /* Include standard types */

#define __IO volatile

#define BASE32_ADDR_FU540_UART0 0x10010000
#define BASE32_ADDR_FU540_UART1    0x10011000

#define TX_FULL_MASK     (0x01 << 31)
#define RX_EMPTY_MASK     (0x01 << 31)

typedef struct _FU540_UART
{
    /* Transmit data register
     * bits 7:0       tx data
     * bit  31       Transmit FIFO full
     */
    __IO unsigned int txdata;

    /* Receive data register
     * bits 7:0       tx data
     * bit  31       Receive FIFO empty
     */
    __IO unsigned int rxdata;

    /* Transmit control register.
     * bits 0       Transmit enable
     * bit  1       Number of stop bits, 0 => 1, 1 => 2
     * bits 18:16   watermark interrupt trigger
     */
    __IO unsigned int txctrl;

    /* Receive control register
     * bits 0       Transmit enable
     * bit  1       Number of stop bits, 0 => 1, 1 => 2
     * bits 18:16   watermark interrupt trigger
     */
    __IO unsigned int rxctrl;

    /* UART interrupt enable
     * bits 0   Transmit watermark interrupt enable
     * bits 1   Receive watermark interrupt enable
     */
    __IO unsigned int ie;

    /* UART Interrupt Pending Register
     * bits 0   Transmit watermark interrupt pending
     * bits 1   Receive watermark interrupt pending
     */
    __IO unsigned int ip;

    /* Baud rate divisor */
    __IO unsigned int div;

}FU540_UART;

/***************************************************************************//**
  mss_uart_instance.
  There is one instance of this structure for each instance of the
  microcontroller subsystem�s UARTs. Instances of this structure are used to
  identify a specific UART. A pointer to an initialized instance of the
  mss_uart_instance_t structure is passed as the first parameter to
  MSS UART driver functions to identify which UART should perform the
  requested operation.
 */
struct mss_FU540_uart_instance{
    /* CMSIS related defines identifying the UART hardware. */
    FU540_UART *              hw_reg;     /*!< Pointer to UART registers. */
//    IRQn_Type               irqn;       /*!< UART's  interrupt number. */
    uint32_t                baudrate;   /*!< Operating baud rate. */
    uint8_t                 lineconfig; /*!< Line configuration parameters. */
    uint8_t                 status;     /*!< Sticky line status. */

    /* transmit related info (used with interrupt driven transmit): */
    volatile uint8_t     *     tx_buffer;          /*!< Pointer to transmit buffer. */
    uint32_t        tx_buff_size;       /*!< Transmit buffer size. */
    uint32_t        tx_idx;             /*!< Index within transmit buffer of next byte to transmit.*/
#if 0
    /* line status interrupt handler:*/
    mss_uart_irq_handler_t linests_handler;   /*!< Pointer to user registered line status handler. */
    /* receive interrupt handler:*/
    mss_uart_irq_handler_t rx_handler;        /*!< Pointer to user registered receiver handler. */
    /* transmit interrupt handler:*/
    mss_uart_irq_handler_t tx_handler;        /*!< Pointer to user registered transmit handler. */
    /* modem status interrupt handler:*/
    mss_uart_irq_handler_t modemsts_handler;  /*!< Pointer to user registered modem status handler. */
    /* receiver timeout interrupt handler */
    mss_uart_irq_handler_t rto_handler;       /*!< Pointer to user registered receiver timeout handler. */
    /* NACK interrupt handler */
    mss_uart_irq_handler_t nack_handler;      /*!< Pointer to user registered NACK handler. */
    /* PID parity prror interrup handler */
    mss_uart_irq_handler_t pid_pei_handler;   /*!< Pointer to user registered PID parity error handler. */
    /* LIN break interrupt handler */
    mss_uart_irq_handler_t break_handler;     /*!< Pointer to user registered LIN break handler. */
    /* LIN sync detection interrupt handler */
    mss_uart_irq_handler_t sync_handler;      /*!< Pointer to user registered LIN sync dectection handler. */
#endif
};

typedef struct  mss_FU540_uart_instance mss_FU540_uart_instance_t;

/***************************************************************************//**
  This instance of mss_uart_instance_t holds all data related to the operations
  performed by UART0. The function MSS_UART_init() initializes this structure.
  A pointer to g_mss_uart0 is passed as the first parameter to MSS UART driver
  functions to indicate that UART0 should perform the requested operation.
 */
extern mss_FU540_uart_instance_t g_mss_FU540_uart0;

/***************************************************************************//**
  This instance of mss_uart_instance_t holds all data related to the operations
  performed by UART1. The function MSS_UART_init() initializes this structure.
  A pointer to g_mss_uart1 is passed as the first parameter to MSS UART driver
  functions to indicate that UART1 should perform the requested operation.
 */
extern mss_FU540_uart_instance_t g_mss_uart1;

#define FU540_UART0_REGS_ADDRESS (*(volatile FU540_UART *) BASE32_ADDR_FU540_UART0)

void
MSS_FU540_UART_init
(
    mss_FU540_uart_instance_t* this_uart,
    uint32_t baud_rate,
    uint8_t line_config
);

uint32_t
MSS_FU540_UART_get_rx
(
    mss_FU540_uart_instance_t * this_uart,
    uint8_t * rx_buff,
    uint32_t buff_size
);

void
MSS_FU540_UART_interrupt_tx
(
    mss_FU540_uart_instance_t * this_uart,
    const uint8_t * pbuff,
    uint32_t tx_size
);

void
MSS_FU540_UART_polled_tx
(
    mss_FU540_uart_instance_t * this_uart,
    const uint8_t * pbuff,
    uint32_t tx_size
);

void
MSS_FU540_UART_int_tx
(
    mss_FU540_uart_instance_t * this_uart,
    const uint8_t * pbuff,
    uint32_t tx_size
);


#endif
