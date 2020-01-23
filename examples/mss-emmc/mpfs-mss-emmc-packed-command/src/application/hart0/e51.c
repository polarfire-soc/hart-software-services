/***********************************************************************************
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
#include "drivers/mss_mmc/mss_mmc_internal_api.h"

#define ERROR_INTERRUPT         0x8000u
#define TRANSFER_COMPLETE       0x2u

#define SECT_NO                 100u
#define BUFFER_A_SIZE           4096 + 512u

uint8_t g_mmc_rx_buff[BUFFER_A_SIZE];
uint8_t g_mmc_tx_buff[BUFFER_A_SIZE];

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
    uint8_t packed_read[512u];
    uint8_t packed_write[512u];

    SYSREG->SUBBLK_CLOCK_CR = 0xffffffff;        /* all clocks on */
    SYSREG->SOFT_RESET_CR &= ~( (1u << 0u) | (1u << 3u) | (1u << 4u) | (1u << 5u)
                                    | (1u << 19u) | (1u << 23u) | (1u << 28u));

    /*--------------------------------------------------------------------------
     * Initialize the write and read Buffers
     */
    for (loop_count = 0u; loop_count < (BUFFER_A_SIZE); loop_count++)
    {
        g_mmc_tx_buff[loop_count] = 0x45u + loop_count;
        g_mmc_rx_buff[loop_count] = 0x00u;
    }

    packed_write[0u] = 0x01u; /* version */
    packed_write[1u] = 0x02u; /* 0x2 - write */
    packed_write[2u] = 0x02u; /* no of entries */
    packed_write[3u] = 0x00u;
    packed_write[4u] = 0x00u;
    packed_write[5u] = 0x00u;
    packed_write[6u] = 0x00u;
    packed_write[7u] = 0x00u;
    packed_write[8u] = 0x04u; /* CMD23 arg1 - 4 blocks */
    packed_write[9u] = 0x00u;
    packed_write[10u] = 0x00u;
    packed_write[11u] = 0x00u;
    packed_write[12u] = 0x09u; /* CMD25 arg1 - sector no 9 */
    packed_write[13u] = 0x00u;
    packed_write[14u] = 0x00u;
    packed_write[15u] = 0x00u;
    packed_write[16u] = 0x04u; /* CMD23 arg2 - 4-blocks */
    packed_write[17u] = 0x00u;
    packed_write[18u] = 0x00u;
    packed_write[19u] = 0x00u;
    packed_write[20u] = 0x29u; /* CMD25 agr2 - sector no 25 */
    packed_write[21u] = 0x00u;
    packed_write[22u] = 0x00u;
    packed_write[23u] = 0x00u;
    for(loop_count = 24u; loop_count < 512u; loop_count++)
    {
        packed_write[loop_count] = 0u;
    }

    packed_read[0] = 0x01u; /* version */
    packed_read[1] = 0x01u; /* 0x1- read */
    packed_read[2] = 0x02u; /* no of entries */
    packed_read[3] = 0x00u;
    packed_read[4] = 0x00u;
    packed_read[5] = 0x00u;
    packed_read[6] = 0x00u;
    packed_read[7] = 0x00u;
    packed_read[8] = 0x04u; /* CMD23 arg1 - 4 blocks */
    packed_read[9] = 0x00u;
    packed_read[10] = 0x00u;
    packed_read[11] = 0x00u;
    packed_read[12] = 0x09u; /* CMD18 arg1 - sector no 9 */
    packed_read[13] = 0x00u;
    packed_read[14] = 0x00u;
    packed_read[15] = 0x00u;
    packed_read[16] = 0x04u; /* CMD23 arg2 - 4-blocks */
    packed_read[17] = 0x00u;
    packed_read[18] = 0x00u;
    packed_read[19] = 0x00u;
    packed_read[20] = 0x29u; /* CMD18 agr2 - sector no 0x29 */
    packed_read[21] = 0x00u;
    packed_read[22] = 0x00u;
    packed_read[23] = 0x00u;
    for(loop_count = 24u; loop_count < 512u; loop_count++)
    {
        packed_read[loop_count] = 0u;
    }

      /*--------------------------------------------------------------------------
       * Initialize the write and read Buffers
      */
    for (loop_count = 0u; loop_count < (BUFFER_A_SIZE); loop_count++)
    {
        g_mmc_tx_buff[loop_count] = 0x45u + loop_count;
        g_mmc_rx_buff[loop_count] = 0x00u;
    }
    for (loop_count = 0u; loop_count < 512u; loop_count++)
    {
        g_mmc_tx_buff[loop_count] = packed_write[loop_count];
    }

    PLIC_init();
    __enable_irq();
    PLIC_SetPriority(MMC_main_PLIC, 2u);
    PLIC_SetPriority(MMC_wakeup_PLIC, 2u);
    /* DMA init for eMMC */
    loop_count = MSS_MPU_configure(MSS_MPU_MMC,
                                   MSS_MPU_PMP_REGION3,
                                   0x08000000u,
                                   0x200000u,
                                   MPU_MODE_READ_ACCESS|MPU_MODE_WRITE_ACCESS|MPU_MODE_EXEC_ACCESS,
                                   MSS_MPU_AM_NAPOT,
                                   0u);

    /* Configure eMMC */
    g_mmc.clk_rate = MSS_MMC_CLOCK_50MHZ;
    g_mmc.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc.bus_speed_mode = MSS_MMC_MODE_SDR;
    g_mmc.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;

    ret_status = MSS_MMC_init(&g_mmc);
    if (ret_status == MSS_MMC_INIT_SUCCESS)
    {
        MSS_MMC_set_handler(transfer_complete_handler);
        ret_status = MSS_MMC_packed_write(g_mmc_tx_buff, 0x09u, BUFFER_A_SIZE);
        if (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
        {
            do
            {
                ret_status = MSS_MMC_get_transfer_status();
            }while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS);
        }
        if (ret_status == MSS_MMC_TRANSFER_SUCCESS)
        {
            ret_status = MSS_MMC_packed_read(0x09u, g_mmc_rx_buff, (uint32_t *)packed_read, BUFFER_A_SIZE - 512u);
            if (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
            {
                do
                {
                    ret_status = MSS_MMC_get_transfer_status();
                }while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS);
            }

            if (ret_status == MSS_MMC_TRANSFER_SUCCESS)
            {
                error = verify_write(g_mmc_tx_buff + 512u, g_mmc_rx_buff, BUFFER_A_SIZE - 512u);
            }
        }
    }
    while(1);
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
