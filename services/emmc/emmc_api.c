/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file EMMC (null) Service
 * \brief Setup EMMC
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"
#include "hss_progress.h"

#include <assert.h>
#include <string.h>

#include "emmc_service.h"
#include "encoding.h"
#include "mss_mmc.h"

/*
 * EMMC doesn't need a "service" to run every super-loop, but it does need to be
 * initialized early...
 */

#ifndef MMC_CARD
#  define MMC_CARD
#endif

#define MMC_CARD
bool HSS_EMMCInit(void)
{
    //static bool initialized = false;
    mss_mmc_status_t result = MSS_MMC_INIT_SUCCESS;

    //if (!initialized) {
    {
        static mss_mmc_cfg_t g_mmc =
        {
#if defined(MMC_CARD)
            .clk_rate = MSS_MMC_CLOCK_50MHZ,
            .card_type = MSS_MMC_CARD_TYPE_MMC,
            .bus_speed_mode = MSS_MMC_MODE_SDR,
            .data_bus_width = MSS_MMC_DATA_WIDTH_4BIT,
            .bus_voltage = MSS_MMC_1_8V_BUS_VOLTAGE
#elif defined(SD_CARD)
            .clk_rate = MSS_MMC_CLOCK_50MHZ,
            .card_type = MSS_MMC_CARD_TYPE_SD,
            .bus_speed_mode = MSS_SDCARD_MODE_HIGH_SPEED,
            .data_bus_width = MSS_MMC_DATA_WIDTH_4BIT,
            .bus_voltage = MSS_MMC_1_8V_BUS_VOLTAGE
#endif
        };

        result = MSS_MMC_init(&g_mmc);

        if (result != MSS_MMC_INIT_SUCCESS) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "MSS_MMC_init() returned unexpected %d" CRLF, result);
        }
        //initialized = true;
    }

    return (result == MSS_MMC_INIT_SUCCESS);
}

#define HSS_EMMC_SECTOR_SIZE (512u)

//
// HSS_EMMC_ReadBlock will handle reads less than a multiple of the sector
// size by doing the last transfer into a sector buffer
//
static char runtBuffer[HSS_EMMC_SECTOR_SIZE] __attribute__((aligned(sizeof(uint32_t))));
bool HSS_EMMC_ReadBlock(void *pDest, size_t srcOffset, size_t byteCount)
{
    // temporary code to bring up Icicle board
    char *pCDest = (char *)pDest;

    // source and byteCount must be multiples of the sector size
    //
    // The MSS MMC driver uses uint32_t* as its pointer type
    // To ensure alignment, would rather tramp through void* and
    // assert check here
    assert(((size_t)srcOffset & (HSS_EMMC_SECTOR_SIZE-1)) == 0u);
    assert(((size_t)pCDest & (sizeof(uint32_t)-1)) == 0u);

    uint32_t src_sector_num = (uint32_t)srcOffset / HSS_EMMC_SECTOR_SIZE;
    mss_mmc_status_t result = MSS_MMC_TRANSFER_SUCCESS;

    while ((result == MSS_MMC_TRANSFER_SUCCESS) && (byteCount >= HSS_EMMC_SECTOR_SIZE)) {
        //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Calling MSS_MMC_single_block_read(%lu, 0x%p) "
        //    "(%lu bytes remaining)" CRLF, src_sector_num, pCDest, byteCount);

        result = MSS_MMC_single_block_read(src_sector_num, (uint32_t *)pCDest);

        if (result != MSS_MMC_TRANSFER_SUCCESS) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "MSS_MMC_single_block_read() unexpectedly returned %d" CRLF,
                result);
        }

        if (result == MSS_MMC_TRANSFER_SUCCESS) {
            src_sector_num++;
            pCDest = pCDest + HSS_EMMC_SECTOR_SIZE;

            if (byteCount < HSS_EMMC_SECTOR_SIZE) {
                ;
            } else {
                byteCount = byteCount - HSS_EMMC_SECTOR_SIZE;
            }
        }
    }

    // handle remainder
    if ((result == MSS_MMC_TRANSFER_SUCCESS) && byteCount) {
        assert(byteCount < HSS_EMMC_SECTOR_SIZE);

        //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Dealing with remainder (less that full sector)" CRLF);
        //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Calling MSS_MMC_single_block_read(%lu, 0x%p) "
        //    "(%lu bytes remaining)" CRLF, src_sector_num, runtBuffer, byteCount);

        result = MSS_MMC_single_block_read(src_sector_num, (uint32_t *)runtBuffer);

        if (result != MSS_MMC_TRANSFER_SUCCESS) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "MSS_MMC_single_block_read() unexpectedly returned %d" CRLF,
                result);
        }

        if (result == MSS_MMC_TRANSFER_SUCCESS) {
            memcpy(pCDest, runtBuffer, byteCount);
        }
    }

    return (result == MSS_MMC_TRANSFER_SUCCESS);
}

//
// HSS_EMMC_WriteBlock will handle requested writes of less than a multiple of the sector
// size by rounding up to the next full sector worth
//
bool HSS_EMMC_WriteBlock(size_t dstOffset, void *pSrc, size_t byteCount)
{
    // temporary code to bring up Icicle board
    char *pCSrc = (char *)pSrc;

    // TODO: for now, if byte count is not a multiple of the sector size, round it up...
    if (byteCount & (HSS_EMMC_SECTOR_SIZE-1)) {
        byteCount = byteCount + HSS_EMMC_SECTOR_SIZE;
        byteCount &= ~(HSS_EMMC_SECTOR_SIZE-1);
    }

    // source and byteCount must be multiples of the sector size
    //
    // The MSS MMC driver uses uint32_t* as its pointer type
    // To ensure alignment, would rather tramp through void* and
    // assert check here
    assert(((size_t)dstOffset & (HSS_EMMC_SECTOR_SIZE-1)) == 0u);
    assert(((size_t)pCSrc & (sizeof(uint32_t)-1)) == 0u);
    assert((byteCount & (HSS_EMMC_SECTOR_SIZE-1)) == 0u);

    uint32_t dst_sector_num = (uint32_t)dstOffset / HSS_EMMC_SECTOR_SIZE;
    mss_mmc_status_t result = MSS_MMC_TRANSFER_SUCCESS;

    size_t origByteCount = byteCount;
    while ((result == MSS_MMC_TRANSFER_SUCCESS) && (byteCount)) {
        //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Calling MSS_MMC_single_block_write(0x%p, %lu) "
        //  "(%lu bytes remaining)" CRLF, pCSrc, dst_sector_num, byteCount);
        HSS_ShowProgress(origByteCount, byteCount);

        result = MSS_MMC_single_block_write((uint32_t *)pCSrc, dst_sector_num);

        if (result != MSS_MMC_TRANSFER_SUCCESS) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "MSS_MMC_single_block_write() unexpectedly returned %d" CRLF,
                result);
        }

        dst_sector_num++;
        byteCount = byteCount - HSS_EMMC_SECTOR_SIZE;
        pCSrc = pCSrc + HSS_EMMC_SECTOR_SIZE;
    }

    HSS_ShowProgress(origByteCount, 0u);

    return (result == MSS_MMC_TRANSFER_SUCCESS);
}
