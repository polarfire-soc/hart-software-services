/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file QSPI (null) Service
 * \brief Setup QSPI
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"

#include <assert.h>

#include "qspi_service.h"
#include "encoding.h"
#include "mss_qspi.h"
#include "mss_sys_services.h"

#include "winbond_w25n01gv.h"

/*
 * QSPI doesn't need a "service" to run every super-loop, but it does need to be
 * initialized early...
 */

bool HSS_QSPIInit(void)
{
#if IS_ENABLED(CONFIG_SERVICE_QSPI_USE_XIP)
    static mss_qspi_config_t qspiConfig =
    {
        .xip = 1,
        .xip_addr = 4, //num_bytes_in_XIP_mode
        .spi_mode = MSS_QSPI_MODE0,
        .clk_div = MSS_QSPI_CLK_DIV_30,
        .io_format = MSS_QSPI_QUAD_FULL,
        .sample = 0
    };

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Initializing QSPI" CRLF);
    MSS_QSPI_init();

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Enabling QSPI" CRLF);
    MSS_QSPI_enable();

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Configuring" CRLF);
    MSS_QSPI_configure(&qspiConfig);
#else
    /* temporary code for Icicle board bringup */

    MSS_SYS_select_service_mode(MSS_SYS_SERVICE_POLLING_MODE, NULL);
    MSS_QSPI_init();
    MSS_QSPI_enable();

    /* read and output Flash ID as a sanity test */
    uint8_t rd_buf[10] __attribute__ ((aligned(4)));

    Flash_init(MSS_QSPI_NORMAL);
    Flash_readid(rd_buf);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "JEDEC Flash ID: %02X%02X%02X" CRLF, rd_buf[0], rd_buf[1], rd_buf[2]);

    if (((rd_buf[0] << 16) | (rd_buf[1] <<8) | (rd_buf[2])) == 0xEFAA21) {
        // EFh => Winbond, AA21h => W25N01GV
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Winbond W25N01GV detected" CRLF);
    }
#endif

    return true;
}

#define HSS_QSPI_PAGE_SIZE 512u
bool HSS_QSPI_ReadBlock(void *pDest, size_t srcOffset, size_t byteCount)
{
    bool result = true;

    // Temporary code for ICICLE Bringup
    if (byteCount < HSS_QSPI_PAGE_SIZE) { byteCount = HSS_QSPI_PAGE_SIZE; } // TODO

#if IS_ENABLED(CONFIG_SERVICE_QSPI_USE_XIP)
    memcpy_via_pdma(pDest, srcOffset + QSPI_BASE, byteCount);
#else
    /* temporary code to bring up Icicle board */
    uint32_t read_addr = (uint32_t)srcOffset;
    Flash_read((uint8_t *)pDest, read_addr, (uint32_t) byteCount);
#endif

    return result;
}

bool HSS_QSPI_WriteBlock(size_t dstOffset, void *pSrc, size_t byteCount)
{
    bool result = true;

    // Temporary code for ICICLE Bringup
    if (byteCount < HSS_QSPI_PAGE_SIZE) { byteCount = HSS_QSPI_PAGE_SIZE; } // TODO

    Flash_program((uint8_t *)pSrc, (uint32_t)dstOffset, (uint32_t)byteCount);
    return result;
}

