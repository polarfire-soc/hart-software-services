/**************************************************************************//**
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Code running on e51.
 * Example project demonstrate the Master Slave communication using internal
 * loopback mechanism.
 *
 * SVN $Revision: 11522 $
 * SVN $Date: 2019-06-26 10:14:17 +0530 (Wed, 26 Jun 2019) $
 */

#include <stdint.h>
#include <string.h>
#include "mpfs_hal/mss_hal.h"
#include "drivers/mss_spi/mss_spi.h"

/**************************************************************************//**
 * Macro definitions
 */
#define COMMAND_BYTE_SIZE       1
#define NB_OF_TURNAROUND_BYTES  4
#define SLAVE_NB_OF_COMMANDS    4
#define SLAVE_PACKET_LENGTH     8
#define MSS_SPI_LOOPBACK_MASK   2

/**************************************************************************//**
 * Private function declaration.
 */
static void spi1_slave_cmd_handler(uint8_t *, uint32_t);
static void spi1_block_rx_handler(uint8_t *, uint32_t);
static void mss_spi_overflow_handler(uint8_t mss_spi_core);

/**************************************************************************//**
 * Data returned by SPI slave based on value of received command byte.
 */
uint8_t g_slave_tx_buffer[SLAVE_NB_OF_COMMANDS][SLAVE_PACKET_LENGTH] =
{
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18},
    {0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28},
    {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38}
};

static uint8_t g_master_tx_buffer[5] = { 0x00, 0x01, 0x02, 0x03, 0xAA};
static uint8_t g_master_rx_buffer[8] = { 0u };
static uint8_t g_slave_rx_buffer[13] = { 0u };


/**************************************************************************//**
 * Main function for the HART0(E51 processor).
 * Application code running on HART0 is placed here.
 * U54 HARTs are not used and they are kept in WFI
 */
void e51 (void)
{
    uint8_t cmd_idx = 0;

    /*Reset SPI0 and SPI1*/
    SYSREG->SOFT_RESET_CR &= ~(0x01UL << 10);
    SYSREG->SOFT_RESET_CR &= ~(0x01UL << 11);

    /* all clocks on */
    SYSREG->SUBBLK_CLOCK_CR = 0xffffffff;

    PLIC_init();
    __enable_irq();
    PLIC_SetPriority(SPI0_PLIC, 2);
    PLIC_SetPriority(SPI1_PLIC, 2);

    MSS_SPI_init(&g_mss_spi0_lo);

    MSS_SPI_configure_master_mode(&g_mss_spi0_lo,
                                  MSS_SPI_SLAVE_1,
                                  MSS_SPI_MODE1,
                                  256u,
                                  MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE,
                                  mss_spi_overflow_handler);

/**************************************************************************//**
 * Initialize and configure SPI1 as slave
 */
    MSS_SPI_init(&g_mss_spi1_lo);

    MSS_SPI_configure_slave_mode(&g_mss_spi1_lo,
                                 MSS_SPI_MODE1,
                                 MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE,
                                 mss_spi_overflow_handler);

    MSS_SPI_set_slave_block_buffers(&g_mss_spi1_lo,
                                    &g_slave_tx_buffer[0][0],
                                    COMMAND_BYTE_SIZE + NB_OF_TURNAROUND_BYTES,
                                    g_slave_rx_buffer,
                                    sizeof(g_slave_rx_buffer),
                                    spi1_block_rx_handler);

    MSS_SPI_set_cmd_handler(&g_mss_spi1_lo,
                            spi1_slave_cmd_handler,
                            COMMAND_BYTE_SIZE);

    MSS_SPI_set_slave_select(&g_mss_spi0_lo, MSS_SPI_SLAVE_1);

    for (;;)
    {
        /* Assert slave select. */
        MSS_SPI_set_slave_select(&g_mss_spi0_lo, MSS_SPI_SLAVE_1);

        /* Perform block transfer */
        MSS_SPI_transfer_block(&g_mss_spi0_lo,
                               g_master_tx_buffer,
                               COMMAND_BYTE_SIZE + NB_OF_TURNAROUND_BYTES,
                               g_master_rx_buffer,
                               sizeof(g_master_rx_buffer));

        /* De-assert slave select. */
        MSS_SPI_clear_slave_select(&g_mss_spi0_lo, MSS_SPI_SLAVE_1);

        /* Issue a different command each time to the slave.*/
        if (3u == cmd_idx)
        {
          cmd_idx = 0u;
        }
        else
        {
          ++cmd_idx;
        }
        g_master_tx_buffer[0] = cmd_idx;
    }
}

/**************************************************************************//**
 * SPI slave receive completion handler.
 * This function is called by the SPI slave on the completion of each SPI
 * transaction after the SPI chip select signal is de-asserted.
 */
static void spi1_block_rx_handler
(
    uint8_t * rx_buff,
    uint32_t rx_size
)
{
}

/**************************************************************************//**
 * SPI slave command handler.
 * This function is called by the SPI slave driver once the command byte is
 * received.
 */
static void spi1_slave_cmd_handler
(
    uint8_t * rx_buff,
    uint32_t rx_size
)
{
    uint8_t index;
    const uint8_t * p_response;

    index = rx_buff[0];

    if (index < SLAVE_NB_OF_COMMANDS)
    {
        p_response = &g_slave_tx_buffer[index][0];
    }
    else
    {
        p_response = &g_slave_tx_buffer[0][0];
    }
    MSS_SPI_set_cmd_response(&g_mss_spi1_lo, p_response, SLAVE_PACKET_LENGTH);
}

/**************************************************************************//**
 * SPI buffer overflow handler
 * This function is called by SPI driver in case of buffer overflow.
 */
static void mss_spi_overflow_handler
(
    uint8_t mss_spi_core
)
{
    if (mss_spi_core)
    {
        /* reset SPI1 */
        SYSREG->SOFT_RESET_CR |= (0x01UL << 11);
       /* Take SPI1 out of reset. */
        SYSREG->SOFT_RESET_CR &= ~(0x01UL << 11);
    }
    else
    {
        /* reset SPI0 */
         SYSREG->SOFT_RESET_CR |= (0x01UL << 10);
        /* Take SPI0 out of reset. */
         SYSREG->SOFT_RESET_CR &= ~(0x01UL << 10);
    }
}
