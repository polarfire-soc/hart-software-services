/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
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
#include "hss_perfctr.h"

#include <assert.h>
#include <string.h>

#include "mmc_service.h"
#include "encoding.h"
#include "mss_mmc.h"
#include "hal/hw_macros.h"
#ifndef __IO
#define __IO volatile
#endif
#include "mss_io_config.h"
#include "hss_memcpy_via_pdma.h"

/*
 * MMC doesn't need a "service" to run every super-loop, but it does need to be
 * initialized early...
 */


#if !defined(CONFIG_SERVICE_MMC_MODE_EMMC) && !defined(CONFIG_SERVICE_MMC_MODE_SDCARD)
#  error Unknown MMC mode (eMMC or SDcard)
#endif

#if defined(CONFIG_SERVICE_MMC_BUS_VOLTAGE_1V8) && defined(CONFIG_SERVICE_MMC_BUS_VOLTAGE_3V3)
#  error Both MMC Bus Voltages defined! These are mutually exclusive.
#endif

// ---------------------------------------------------------------------------------------------

/*
 * We need MSSIO settings for eMMC...
 */
#define LIBERO_SETTING_IOMUX1_CR_eMMC			0x11111111UL
#define LIBERO_SETTING_IOMUX2_CR_eMMC			0x00FF1111UL
#define LIBERO_SETTING_IOMUX6_CR_eMMC			0x00000000UL
#define LIBERO_SETTING_MSSIO_BANK4_CFG_CR_eMMC		0x00040A0DUL
#define LIBERO_SETTING_MSSIO_BANK4_IO_CFG_0_1_CR_eMMC	0x09280928UL
#define LIBERO_SETTING_MSSIO_BANK4_IO_CFG_2_3_CR_eMMC	0x09280928UL
#define LIBERO_SETTING_MSSIO_BANK4_IO_CFG_4_5_CR_eMMC	0x09280928UL
#define LIBERO_SETTING_MSSIO_BANK4_IO_CFG_6_7_CR_eMMC	0x09280928UL
#define LIBERO_SETTING_MSSIO_BANK4_IO_CFG_8_9_CR_eMMC	0x09280928UL
#define LIBERO_SETTING_MSSIO_BANK4_IO_CFG_10_11_CR_eMMC	0x09280928UL
#define LIBERO_SETTING_MSSIO_BANK4_IO_CFG_12_13_CR_eMMC	0x09280928UL

/*
 * ... and for SDCard
 */
#define LIBERO_SETTING_IOMUX1_CR_SD			0x00000000UL
#define LIBERO_SETTING_IOMUX2_CR_SD			0x00000000UL
#define LIBERO_SETTING_IOMUX6_CR_SD			0x0000001DUL
#define LIBERO_SETTING_MSSIO_BANK4_CFG_CR_SD		0x00080907UL
#define LIBERO_SETTING_MSSIO_BANK4_IO_CFG_0_1_CR_SD	0x08290829UL
#define LIBERO_SETTING_MSSIO_BANK4_IO_CFG_2_3_CR_SD	0x08290829UL
#define LIBERO_SETTING_MSSIO_BANK4_IO_CFG_4_5_CR_SD	0x08290829UL
#define LIBERO_SETTING_MSSIO_BANK4_IO_CFG_6_7_CR_SD	0x08290829UL
#define LIBERO_SETTING_MSSIO_BANK4_IO_CFG_8_9_CR_SD	0x08290829UL
#define LIBERO_SETTING_MSSIO_BANK4_IO_CFG_10_11_CR_SD	0x08290829UL
#define LIBERO_SETTING_MSSIO_BANK4_IO_CFG_12_13_CR_SD	0x08290829UL

// ---------------------------------------------------------------------------------------------

#include "mss_sysreg.h"

static void mmc_reset_block(void)
{
    SYSREG->SUBBLK_CLOCK_CR |=
        (uint32_t)(SUBBLK_CLOCK_CR_MMC_MASK);
    SYSREG->SOFT_RESET_CR |=
        (uint32_t)(SOFT_RESET_CR_MMC_MASK);
    SYSREG->SOFT_RESET_CR &=
        ~(uint32_t)(SOFT_RESET_CR_MMC_MASK);
}

static bool mmc_init_common(mss_mmc_cfg_t *p_mmcConfig)
{
    bool result = false;
    mss_mmc_status_t retval = MSS_MMC_init(p_mmcConfig);

    if (retval != MSS_MMC_INIT_SUCCESS) {
        mmc_reset_block();

        HSS_SpinDelay_Secs(1u); // delay for 1 second
        retval = MSS_MMC_init(p_mmcConfig);
    }

    if (retval != MSS_MMC_INIT_SUCCESS) {
        //mHSS_DEBUG_PRINTF(LOG_ERROR, "MSS_MMC_init() returned unexpected %d" CRLF, retval);
    } else {
        result = true;
    }

    return result;
}

#if defined(CONFIG_SERVICE_MMC_MODE_EMMC)
static bool mmc_init_emmc(void)
{
    SYSREG->IOMUX1_CR = LIBERO_SETTING_IOMUX1_CR_eMMC;
    SYSREG->IOMUX2_CR = LIBERO_SETTING_IOMUX2_CR_eMMC;
    SYSREG->IOMUX6_CR = LIBERO_SETTING_IOMUX6_CR_eMMC;
    SYSREG->MSSIO_BANK4_CFG_CR = LIBERO_SETTING_MSSIO_BANK4_CFG_CR_eMMC;
    SYSREG->MSSIO_BANK4_IO_CFG_0_1_CR = LIBERO_SETTING_MSSIO_BANK4_IO_CFG_0_1_CR_eMMC;
    SYSREG->MSSIO_BANK4_IO_CFG_2_3_CR = LIBERO_SETTING_MSSIO_BANK4_IO_CFG_2_3_CR_eMMC;
    SYSREG->MSSIO_BANK4_IO_CFG_4_5_CR = LIBERO_SETTING_MSSIO_BANK4_IO_CFG_4_5_CR_eMMC;
    SYSREG->MSSIO_BANK4_IO_CFG_6_7_CR = LIBERO_SETTING_MSSIO_BANK4_IO_CFG_6_7_CR_eMMC;
    SYSREG->MSSIO_BANK4_IO_CFG_8_9_CR = LIBERO_SETTING_MSSIO_BANK4_IO_CFG_8_9_CR_eMMC;
    SYSREG->MSSIO_BANK4_IO_CFG_10_11_CR = LIBERO_SETTING_MSSIO_BANK4_IO_CFG_10_11_CR_eMMC;
    SYSREG->MSSIO_BANK4_IO_CFG_12_13_CR = LIBERO_SETTING_MSSIO_BANK4_IO_CFG_12_13_CR_eMMC;

#if defined(CONFIG_SERVICE_SDIO_REGISTER_PRESENT)
    HW_set_uint32(CONFIG_SERVICE_SDIO_REGISTER_ADDRESS,  0);
#endif

    static mss_mmc_cfg_t emmcConfig =
    {
        .card_type = MSS_MMC_CARD_TYPE_MMC,
        .data_bus_width = MSS_MMC_DATA_WIDTH_8BIT,
#if defined(CONFIG_SERVICE_MMC_BUS_VOLTAGE_1V8)
        .bus_voltage = MSS_MMC_1_8V_BUS_VOLTAGE,
        .clk_rate = MSS_MMC_CLOCK_200MHZ,
        .bus_speed_mode = MSS_MMC_MODE_HS200,
#elif defined(CONFIG_SERVICE_MMC_BUS_VOLTAGE_3V3)
        .bus_voltage = MSS_MMC_3_3V_BUS_VOLTAGE,
        .clk_rate = MSS_MMC_CLOCK_50MHZ,
        .bus_speed_mode = MSS_MMC_MODE_SDR,
#endif
    };

    return mmc_init_common(&emmcConfig);
}
#endif

#if defined(CONFIG_SERVICE_MMC_MODE_SDCARD)
static bool mmc_init_sdcard(void)
{
    SYSREG->IOMUX1_CR = LIBERO_SETTING_IOMUX1_CR_SD;
    SYSREG->IOMUX2_CR = LIBERO_SETTING_IOMUX2_CR_SD;
    SYSREG->IOMUX6_CR = LIBERO_SETTING_IOMUX6_CR_SD;
    SYSREG->MSSIO_BANK4_CFG_CR = LIBERO_SETTING_MSSIO_BANK4_CFG_CR_SD;
    SYSREG->MSSIO_BANK4_IO_CFG_0_1_CR = LIBERO_SETTING_MSSIO_BANK4_IO_CFG_0_1_CR_SD;
    SYSREG->MSSIO_BANK4_IO_CFG_2_3_CR = LIBERO_SETTING_MSSIO_BANK4_IO_CFG_2_3_CR_SD;
    SYSREG->MSSIO_BANK4_IO_CFG_4_5_CR = LIBERO_SETTING_MSSIO_BANK4_IO_CFG_4_5_CR_SD;
    SYSREG->MSSIO_BANK4_IO_CFG_6_7_CR = LIBERO_SETTING_MSSIO_BANK4_IO_CFG_6_7_CR_SD;
    SYSREG->MSSIO_BANK4_IO_CFG_8_9_CR = LIBERO_SETTING_MSSIO_BANK4_IO_CFG_8_9_CR_SD;
    SYSREG->MSSIO_BANK4_IO_CFG_10_11_CR = LIBERO_SETTING_MSSIO_BANK4_IO_CFG_10_11_CR_SD;
    SYSREG->MSSIO_BANK4_IO_CFG_12_13_CR = LIBERO_SETTING_MSSIO_BANK4_IO_CFG_12_13_CR_SD;

#if defined(CONFIG_SERVICE_SDIO_REGISTER_PRESENT)
    HW_set_uint32(CONFIG_SERVICE_SDIO_REGISTER_ADDRESS,  1);
#endif

    static mss_mmc_cfg_t sdcardConfig =
    {
        .card_type = MSS_MMC_CARD_TYPE_SD,
        .data_bus_width = MSS_MMC_DATA_WIDTH_4BIT,
        .bus_speed_mode = MSS_SDCARD_MODE_HIGH_SPEED,
        .clk_rate = MSS_MMC_CLOCK_50MHZ,
    };

    return mmc_init_common(&sdcardConfig);
}
#endif

static bool mmc_initialized = false;
bool HSS_MMCInit(void)
{
    bool result = false;

    if (!mmc_initialized) {
        int perf_ctr_index = PERF_CTR_UNINITIALIZED;
        HSS_PerfCtr_Allocate(&perf_ctr_index, "MMC Init");
        mmc_reset_block();

#if defined(CONFIG_SERVICE_MMC_MODE_SDCARD)
        mHSS_DEBUG_PRINTF(LOG_STATUS, "Attempting to select SDCARD ... ");
        mmc_initialized = mmc_init_sdcard();
        mHSS_DEBUG_PRINTF_EX("%s" CRLF, mmc_initialized ? "Passed" : "Failed");

#endif
#if defined(CONFIG_SERVICE_MMC_MODE_EMMC)
        if (!mmc_initialized) {
            mHSS_DEBUG_PRINTF(LOG_STATUS, "Attempting to select eMMC ... ");
            mmc_initialized = mmc_init_emmc();
            mHSS_DEBUG_PRINTF_EX("%s" CRLF, mmc_initialized ? "Passed" : "Failed");
        }
#endif
        HSS_PerfCtr_Lap(perf_ctr_index);
    }

    result = mmc_initialized;

    return result;
}

#define HSS_MMC_SECTOR_SIZE (512u)

//
// HSS_MMC_ReadBlock will handle reads less than a multiple of the sector
// size by doing the last transfer into a sector buffer
//
static char runtBuffer[HSS_MMC_SECTOR_SIZE] __attribute__((aligned(sizeof(uint32_t))));

bool HSS_MMC_ReadBlock(void *pDest, size_t srcOffset, size_t byteCount)
{
    char *pCDest = (char *)pDest;
    assert(((size_t)srcOffset & (HSS_MMC_SECTOR_SIZE-1)) == 0u);
    assert(((size_t)pCDest & (sizeof(uint32_t)-1)) == 0u);

    uint32_t src_sector_num = (uint32_t)(srcOffset / HSS_MMC_SECTOR_SIZE);
    mss_mmc_status_t result = MSS_MMC_TRANSFER_SUCCESS;
    uint32_t sectorByteCount = byteCount - (byteCount % HSS_MMC_SECTOR_SIZE);

    uint8_t mmc_main_plic_IRQHandler(void);

    do {
        result = mmc_main_plic_IRQHandler();
    } while (MSS_MMC_TRANSFER_IN_PROGRESS == result);

    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Calling MSS_MMC_sdma_read(%lu, %p) "
    //    "(%lu bytes remaining)" CRLF, src_sector_num, pCDest, sectorByteCount);
    result = MSS_MMC_sdma_read(src_sector_num, (uint8_t *)pCDest, sectorByteCount);

    while (result == MSS_MMC_TRANSFER_IN_PROGRESS) {
        result = mmc_main_plic_IRQHandler();
    }

    byteCount = byteCount - sectorByteCount;
    // handle remainder
    if ((result == MSS_MMC_TRANSFER_SUCCESS) && byteCount) {
        assert(byteCount < HSS_MMC_SECTOR_SIZE);

        src_sector_num += (uint32_t)(sectorByteCount / HSS_MMC_SECTOR_SIZE);

        //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Dealing with remainder (less that full sector)" CRLF);
        //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Calling MSS_MMC_single_block_read(%lu, %p) "
        //    "(%lu bytes remaining)" CRLF, src_sector_num, runtBuffer, byteCount);
        result = MSS_MMC_sdma_read(src_sector_num, (uint8_t *)runtBuffer, HSS_MMC_SECTOR_SIZE);

        while (result == MSS_MMC_TRANSFER_IN_PROGRESS) {
            result = mmc_main_plic_IRQHandler();
        }

        if (result != MSS_MMC_TRANSFER_SUCCESS) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "MSS_MMC_single_block_read() unexpectedly returned %d" CRLF,
                result);
        }

        if (result == MSS_MMC_TRANSFER_SUCCESS) {
            memcpy_via_pdma(pCDest + sectorByteCount, runtBuffer, byteCount);
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

    uint32_t dst_sector_num = (uint32_t)(dstOffset / HSS_MMC_SECTOR_SIZE);
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
