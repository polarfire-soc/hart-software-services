/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file MMC (null) Service
 * \brief Setup MMC
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"
#include "hss_progress.h"
#include "hss_clock.h"

#include <assert.h>
#include <string.h>

#include "mmc_service.h"
#include "encoding.h"
#include "mss_mmc.h"

/*
 * MMC doesn't need a "service" to run every super-loop, but it does need to be
 * initialized early...
 */


#if defined(CONFIG_SERVICE_MMC_MODE_EMMC) && defined(CONFIG_SERVICE_MMC_MODE_SDCARD)
#  error Both MMC Modes defined! These are mutually exclusive.
#endif

#if defined(CONFIG_SERVICE_MMC_BUS_VOLTAGE_1V8) && defined(CONFIG_SERVICE_MMC_BUS_VOLTAGE_3V3)
#  error Both MMC Bus Voltages defined! These are mutually exclusive.
#endif

#include "mpfs_hal/mss_sysreg.h"

bool HSS_MMCInit(void)
{
    SYSREG->SUBBLK_CLOCK_CR |= (uint32_t)SUBBLK_CLOCK_CR_MMC_MASK;
    SYSREG->SOFT_RESET_CR   |= (uint32_t)SOFT_RESET_CR_MMC_MASK;
    SYSREG->SOFT_RESET_CR   &= ~(uint32_t)SOFT_RESET_CR_MMC_MASK;

    //static bool initialized = false;
    mss_mmc_status_t result = MSS_MMC_INIT_SUCCESS;

    //if (!initialized) {
    {
        static mss_mmc_cfg_t g_mmc =
        {
            .clk_rate = MSS_MMC_CLOCK_50MHZ,
#if defined(CONFIG_SERVICE_MMC_MODE_EMMC)
            .card_type = MSS_MMC_CARD_TYPE_MMC,
            .bus_speed_mode = MSS_MMC_MODE_SDR,
#elif defined(CONFIG_SERVICE_MMC_MODE_SDCARD)
            .card_type = MSS_MMC_CARD_TYPE_SD,
            .bus_speed_mode = MSS_SDCARD_MODE_HIGH_SPEED,
#else
#  error Unknown MMC mode (eMMC or SDcard)
#endif
            .data_bus_width = MSS_MMC_DATA_WIDTH_4BIT,
#if defined(CONFIG_SERVICE_MMC_BUS_VOLTAGE_1V8)
            .bus_voltage = MSS_MMC_1_8V_BUS_VOLTAGE
#elif defined(CONFIG_SERVICE_MMC_BUS_VOLTAGE_3V3)
            .bus_voltage = MSS_MMC_3_3V_BUS_VOLTAGE
#else
#  error Unknown MMC Bus Voltage
#endif
        };

        result = MSS_MMC_init(&g_mmc);
        if (result != MSS_MMC_INIT_SUCCESS) {
            SYSREG->SUBBLK_CLOCK_CR |= (uint32_t)SUBBLK_CLOCK_CR_MMC_MASK;
            SYSREG->SOFT_RESET_CR   |= (uint32_t)SOFT_RESET_CR_MMC_MASK;
            SYSREG->SOFT_RESET_CR   &= ~(uint32_t)SOFT_RESET_CR_MMC_MASK;

            HSS_SpinDelay_Secs(1u); // delay for 1 second

            result = MSS_MMC_init(&g_mmc);
        }

        if (result != MSS_MMC_INIT_SUCCESS) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "MSS_MMC_init() returned unexpected %d" CRLF, result);
        }
        //initialized = true;
    }

    return (result == MSS_MMC_INIT_SUCCESS);
}

#define HSS_MMC_SECTOR_SIZE (512u)

//
// HSS_MMC_ReadBlock will handle reads less than a multiple of the sector
// size by doing the last transfer into a sector buffer
//
static char runtBuffer[HSS_MMC_SECTOR_SIZE] __attribute__((aligned(sizeof(uint32_t))));
bool HSS_MMC_ReadBlock(void *pDest, size_t srcOffset, size_t byteCount)
{
    // temporary code to bring up Icicle board
    char *pCDest = (char *)pDest;

    // source and byteCount must be multiples of the sector size
    //
    // The MSS MMC driver uses uint32_t* as its pointer type
    // To ensure alignment, would rather tramp through void* and
    // assert check here
    assert(((size_t)srcOffset & (HSS_MMC_SECTOR_SIZE-1)) == 0u);
    assert(((size_t)pCDest & (sizeof(uint32_t)-1)) == 0u);

    uint32_t src_sector_num = (uint32_t)srcOffset / HSS_MMC_SECTOR_SIZE;
    mss_mmc_status_t result = MSS_MMC_TRANSFER_SUCCESS;

    while ((result == MSS_MMC_TRANSFER_SUCCESS) && (byteCount >= HSS_MMC_SECTOR_SIZE)) {
        //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Calling MSS_MMC_single_block_read(%lu, 0x%p) "
        //    "(%lu bytes remaining)" CRLF, src_sector_num, pCDest, byteCount);

        result = MSS_MMC_single_block_read(src_sector_num, (uint32_t *)pCDest);

        if (result != MSS_MMC_TRANSFER_SUCCESS) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "MSS_MMC_single_block_read() unexpectedly returned %d" CRLF,
                result);
        }

        if (result == MSS_MMC_TRANSFER_SUCCESS) {
            src_sector_num++;
            pCDest = pCDest + HSS_MMC_SECTOR_SIZE;

            if (byteCount < HSS_MMC_SECTOR_SIZE) {
                ;
            } else {
                byteCount = byteCount - HSS_MMC_SECTOR_SIZE;
            }
        }
    }

    // handle remainder
    if ((result == MSS_MMC_TRANSFER_SUCCESS) && byteCount) {
        assert(byteCount < HSS_MMC_SECTOR_SIZE);

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
// HSS_MMC_WriteBlock will handle requested writes of less than a multiple of the sector
// size by rounding up to the next full sector worth
//
bool HSS_MMC_WriteBlock(size_t dstOffset, void *pSrc, size_t byteCount)
{
    // temporary code to bring up Icicle board
    char *pCSrc = (char *)pSrc;

    // TODO: for now, if byte count is not a multiple of the sector size, round it up...
    if (byteCount & (HSS_MMC_SECTOR_SIZE-1)) {
        byteCount = byteCount + HSS_MMC_SECTOR_SIZE;
        byteCount &= ~(HSS_MMC_SECTOR_SIZE-1);
    }

    // source and byteCount must be multiples of the sector size
    //
    // The MSS MMC driver uses uint32_t* as its pointer type
    // To ensure alignment, would rather tramp through void* and
    // assert check here
    assert(((size_t)dstOffset & (HSS_MMC_SECTOR_SIZE-1)) == 0u);
    assert(((size_t)pCSrc & (sizeof(uint32_t)-1)) == 0u);
    assert((byteCount & (HSS_MMC_SECTOR_SIZE-1)) == 0u);

    uint32_t dst_sector_num = (uint32_t)dstOffset / HSS_MMC_SECTOR_SIZE;
    mss_mmc_status_t result = MSS_MMC_TRANSFER_SUCCESS;

    while ((result == MSS_MMC_TRANSFER_SUCCESS) && (byteCount)) {
        result = MSS_MMC_single_block_write((uint32_t *)pCSrc, dst_sector_num);

        if (result != MSS_MMC_TRANSFER_SUCCESS) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "MSS_MMC_single_block_write() unexpectedly returned %d" CRLF,
                result);
        }

        dst_sector_num++;
        byteCount = byteCount - HSS_MMC_SECTOR_SIZE;
        pCSrc = pCSrc + HSS_MMC_SECTOR_SIZE;
    }

    return (result == MSS_MMC_TRANSFER_SUCCESS);
}
