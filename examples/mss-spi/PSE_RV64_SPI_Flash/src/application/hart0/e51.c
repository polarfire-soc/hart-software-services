/**************************************************************************//**
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Code running on e51.
 *
 * This example project demonstrates the use of the PolarFire MSS SPI hardware
 * block. It reads and writes the content of an external SPI flash device.
 *
 * SVN $Revision: 11522 $
 * SVN $Date: 2019-06-26 10:14:17 +0530 (Wed, 26 Jun 2019) $
 */

#include <stdint.h>
#include <string.h>
#include "mpfs_hal/mss_hal.h"
#include "drivers/mss_spi/mss_spi.h"
#include "drivers/mt25ql01gbbb/mt25ql01gbbb.h"

#define BUFFER_SIZE                     3000

/*Manufacture and device ID for MICRON MT25QL01GBBB Flash Memory*/
#define FLASH_MANUFACTURER_ID       (uint8_t)0x20
#define FLASH_DEVICE_ID             (uint8_t)0xBB

/*
 * Static Global variables
 */
static uint8_t g_flash_wr_buf[BUFFER_SIZE];
static uint8_t g_flash_rd_buf[BUFFER_SIZE];

/*
 * Local Function
 */
static uint8_t verify_write(uint8_t* write_buff, uint8_t* read_buff, uint16_t size);
static void mss_spi_overflow_handler(uint8_t mss_spi_core);

/**************************************************************************//**
 * Main Function
 */

void e51(void)
{
    volatile uint32_t errors;
    uint32_t loop_count;
    uint8_t manufacturer_id = 0;
    uint8_t device_id = 0;
    uint32_t address = 0;

    SYSREG->SOFT_RESET_CR &= ~(0x01UL << 10);

/**************************************************************************//**
 * Initialize write and read buffers
 */
    for(loop_count = 0; loop_count < (BUFFER_SIZE/2); loop_count++)
    {
        g_flash_wr_buf[loop_count] = 0x44 + loop_count;
        g_flash_rd_buf[loop_count] = 0x00;
    }

    for(loop_count = (BUFFER_SIZE / 2); loop_count < BUFFER_SIZE; loop_count++)
    {
        g_flash_wr_buf[loop_count] = 0x33;
        g_flash_rd_buf[loop_count] = 0x00;
    }

/**************************************************************************//**
     * Flash Driver Initialization
     */

    FLASH_init(&g_mss_spi0_lo, mss_spi_overflow_handler);

    FLASH_global_unprotect(&g_mss_spi0_lo);

    FLASH_erase_64k_block(&g_mss_spi0_lo, 0);

/**************************************************************************//**
 * Check SPI Flash part manufacturer and device ID.
 */
    FLASH_read_device_id(&g_mss_spi0_lo, &manufacturer_id, &device_id);
    if((FLASH_MANUFACTURER_ID != manufacturer_id) || (FLASH_DEVICE_ID != device_id))
    {
        ++errors;
    }

/**************************************************************************//**
 * Write Data to Flash.
 */
    address = 200;
    FLASH_program(&g_mss_spi0_lo, address, g_flash_wr_buf, sizeof(g_flash_wr_buf));

    /*--------------------------------------------------------------------------
     * Read Data From Flash.
     */
    address = 200;
    FLASH_read(&g_mss_spi0_lo, address, g_flash_rd_buf, sizeof(g_flash_wr_buf));

    errors = verify_write(g_flash_rd_buf, g_flash_wr_buf, sizeof(g_flash_wr_buf));

    while(1)
    {
        ;
    }
}

/**************************************************************************//**
 * Read the data from SPI flash and compare the same with write buffer
 */
static uint8_t verify_write(uint8_t* write_buff, uint8_t* read_buff, uint16_t size)
{
    uint8_t error = 0;
    uint16_t index = 0;

    while(size != 0)
    {
        if(write_buff[index] != read_buff[index])
        {
            error = 1;
            break;
        }
        index++;
        size--;
    }
    return error;
}

/**************************************************************************//**
 * SPI buffer overflow handler
 * This function is called by SPI driver in case of buffer overflow.
 */
static void mss_spi_overflow_handler(uint8_t mss_spi_core)
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


