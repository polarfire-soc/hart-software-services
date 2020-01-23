/******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Code running on e51
 *
 * SVN $Revision$
 * SVN $Date$
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mpfs_hal/mss_hal.h"
#include "mpfs_hal/mss_mpu.h"
#include "drivers/mss_mmc/mss_mmc.h"

/* Single block buffer size */
#define BUFFER_A_SIZE           512u

#define SECT_NO                 175u

uint8_t g_mmc_rx_buff[BUFFER_A_SIZE];
uint8_t g_mmc_tx_buff[BUFFER_A_SIZE];

static uint8_t verify_write(uint8_t* write_buff, uint8_t* read_buff, uint32_t size);

/******************************************************************************
 *
 */
void e51(void)
{
    mss_mmc_cfg_t g_mmc;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    uint32_t loop_count;
    uint8_t error = 0u;
    uint32_t sector_number = SECT_NO;

    SYSREG->SUBBLK_CLOCK_CR = 0xffffffff;        /* all clocks on */
    SYSREG->SOFT_RESET_CR &= ~( (1u << 0u) |  (1u << 3u) | (1u << 4u) |
                      (1u << 5u) | (1u << 19u) | (1u << 23u) | (1u << 28u));

    /*--------------------------------------------------------------------------
     * Initialize the write and read Buffers
     */
    for (loop_count = 0u; loop_count < (BUFFER_A_SIZE); loop_count++)
    {
        g_mmc_tx_buff[loop_count] = 0x45u + loop_count;
        g_mmc_rx_buff[loop_count] = 0x00u;
    }

#ifdef MMC_CARD
    g_mmc.clk_rate = MSS_MMC_CLOCK_50MHZ;
    g_mmc.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc.bus_speed_mode = MSS_MMC_MODE_SDR;
    g_mmc.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
#endif

#ifdef SD_CARD
    g_mmc.clk_rate = MSS_MMC_CLOCK_50MHZ;
    g_mmc.card_type = MSS_MMC_CARD_TYPE_SD;
    g_mmc.bus_speed_mode = MSS_SDCARD_MODE_HIGH_SPEED;
    g_mmc.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
#endif
    /* Initialize eMMC/SD */
    ret_status = MSS_MMC_init(&g_mmc);
    if (ret_status == MSS_MMC_INIT_SUCCESS)
    {
        /* Single Block - write */
        ret_status = MSS_MMC_single_block_write((uint32_t *)g_mmc_tx_buff, sector_number);
        if (ret_status == MSS_MMC_TRANSFER_SUCCESS)
        {
            /* Single Block - read */
            ret_status = MSS_MMC_single_block_read(sector_number, (uint32_t *)g_mmc_rx_buff);
            if (ret_status == MSS_MMC_TRANSFER_SUCCESS)
            {
                error = verify_write(g_mmc_tx_buff, g_mmc_rx_buff, BUFFER_A_SIZE);
            }
        }
    }
    while(1u);
}
/******************************************************************************/
static uint8_t verify_write(uint8_t* write_buff, uint8_t* read_buff, uint32_t size)
{
    uint8_t error = 0u;
    uint32_t index = 0u;

    while (size != 0u)
    {
        if (write_buff[index] != read_buff[index])
        {
            error = 1u;
            break;
        }
        index++;
        size--;
    }

    return error;
}
/******************************************************************************/
