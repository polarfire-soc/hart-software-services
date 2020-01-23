/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * code running on e51
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

#define ERROR_INTERRUPT         0x8000u
#define TRANSFER_COMPLETE       0x1u

#define LIM_BASE_ADDRESS        0x08000000u
#define LIM_SIZE                0x200000u

#define SECT_NO                 1000u
/* multiple block data size */
#define BUFFER_SIZE             4096u

uint8_t g_mmc_rx_buff[BUFFER_SIZE];
uint8_t g_mmc_tx_buff[BUFFER_SIZE];

static uint8_t verify_write(uint8_t* write_buff, uint8_t* read_buff, uint32_t size);
void transfer_complete_handler(uint32_t status);

/******************************************************************************
 *
 */
void e51(void)
{
    mss_mmc_cfg_t g_mmc;
    mss_mmc_status_t ret_status;
    uint32_t loop_count;
    uint8_t error = 0u;
    uint32_t sector_number = SECT_NO;

    SYSREG->SUBBLK_CLOCK_CR = 0xffffffff;        // all clocks on
    SYSREG->SOFT_RESET_CR &= ~( (1u << 0u) | (1u << 3u) | (1u << 4u) | (1u << 5u)
                                    | (1u << 19u) | (1u << 23u) | (1u << 28u));

    /*--------------------------------------------------------------------------
     * Initialize the write and read Buffers
     */
    for (loop_count = 0; loop_count < (BUFFER_SIZE); loop_count++)
    {
        g_mmc_tx_buff[loop_count] = 0x45u + loop_count;
        g_mmc_rx_buff[loop_count] = 0x00u;
    }

    PLIC_init();
    __enable_irq();
    PLIC_SetPriority(MMC_main_PLIC, 2u);
    PLIC_SetPriority(MMC_wakeup_PLIC, 2u);

    /* DMA init for eMMC */
    loop_count = MSS_MPU_configure(MSS_MPU_MMC,
                                   MSS_MPU_PMP_REGION3,
                                   LIM_BASE_ADDRESS,
                                   LIM_SIZE,
                                   MPU_MODE_READ_ACCESS|MPU_MODE_WRITE_ACCESS|MPU_MODE_EXEC_ACCESS,
                                   MSS_MPU_AM_NAPOT,
                                   0u);
#ifdef MMC_CARD
    /* Configure eMMC */
    g_mmc.clk_rate = MSS_MMC_CLOCK_50MHZ;
    g_mmc.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc.bus_speed_mode = MSS_MMC_MODE_SDR;
    g_mmc.data_bus_width = MSS_MMC_DATA_WIDTH_8BIT;
#endif

#ifdef SD_CARD
    /* Configure SD card */
    g_mmc.clk_rate = MSS_MMC_CLOCK_50MHZ;
    g_mmc.card_type = MSS_MMC_CARD_TYPE_SD;
    g_mmc.bus_speed_mode = MSS_SDCARD_MODE_HIGH_SPEED;
    g_mmc.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
#endif

    ret_status = MSS_MMC_init(&g_mmc);
    if (ret_status == MSS_MMC_INIT_SUCCESS)
    {
        MSS_MMC_set_handler(transfer_complete_handler);
#ifdef MSS_MMC_ADMA2
        ret_status = MSS_MMC_adma2_write(g_mmc_tx_buff, sector_number, BUFFER_SIZE);
#else
        ret_status = MSS_MMC_sdma_write(g_mmc_tx_buff, sector_number, BUFFER_SIZE);
#endif
        if (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
        {
            do
            {
                ret_status = MSS_MMC_get_transfer_status();
            }while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS);
        }
    }
    if (ret_status == MSS_MMC_TRANSFER_SUCCESS)
    {

#ifdef MSS_MMC_ADMA2
        ret_status = MSS_MMC_adma2_read(sector_number, g_mmc_rx_buff, BUFFER_SIZE);
#else
        ret_status = MSS_MMC_sdma_read(sector_number, g_mmc_rx_buff, BUFFER_SIZE);
#endif
        if (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
        {
            do
            {
                ret_status = MSS_MMC_get_transfer_status();
            }while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS);
        }

        if (ret_status == MSS_MMC_TRANSFER_SUCCESS)
        {
            error = verify_write(g_mmc_tx_buff, g_mmc_rx_buff, BUFFER_SIZE);
        }
    }
    while (1)
    {

    }
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
void transfer_complete_handler(uint32_t status)
{
    uint32_t isr_err;

    if (ERROR_INTERRUPT & status)
    {
        isr_err = status >> 16u;
    }
    else if (TRANSFER_COMPLETE & status)
    {
        isr_err = 0u;
    }
    else
    {

    }

}
/******************************************************************************/
