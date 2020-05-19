/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
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

/*
 * QSPI doesn't need a "service" to run every super-loop, but it does need to be
 * initialized early...
 */

bool HSS_QSPIInit(void)
{
#ifdef CONFIG_SERVICE_QSPI_USE_XIP
    static mss_qspi_config_t qspiConfig =
    { 
        .xip = 1,
        .xip_addr = 4, //num_bytes_in_XIP_mode
        .spi_mode = MSS_QSPI_MODE0,
        .clk_div = MSS_QSPI_CLK_DIV_30,
        .io_format = MSS_QSPI_QUAD_FULL,
        .sample = 0
    };

    mHSS_FANCY_PRINTF("Initializing QSPI" CRLF);
    MSS_QSPI_init();

    mHSS_FANCY_PRINTF("Enabling QSPI" CRLF);
    MSS_QSPI_enable();

    mHSS_FANCY_PRINTF("Configuring" CRLF);
    MSS_QSPI_configure(&qspiConfig);
#else
    /* temporary code for Icicle board bringup */
    void e51_qspi_init(void); /* TODO: refactor */
    e51_qspi_init();

    /* read and output Flash ID as a sanity test */
    void Flash_readid(uint8_t *buf);
    uint8_t rd_buf[10] __attribute__ ((aligned(4)));
    Flash_readid(rd_buf);
    mHSS_FANCY_PRINTF("JEDEC Flash ID: %02X%02X%02X" CRLF, rd_buf[0], rd_buf[1], rd_buf[2]);

    if (((rd_buf[0] << 16) | (rd_buf[1] <<8) | (rd_buf[2])) == 0xEFAA21) {
        // EFh => Winbond, AA21h => W25N01GV
        mHSS_FANCY_PRINTF("Winbond W25N01GV detected" CRLF);
    }
#endif

    return true;
}

void *HSS_QSPI_MemCopy(void *pDest, void *pSrc, size_t count)
{
#ifdef CONFIG_SERVICE_QSPI_USE_XIP
    memcpy_via_pdma(pDest, pSrc, count);
#else
    /* temporary code to bring up Icicle board */
    void Flash_read(uint8_t* buf, uint32_t read_addr, uint32_t read_len);

    uint32_t read_addr = (uint32_t)((uint8_t *)pSrc - (uint8_t *)QSPI_BASE);
    Flash_read((uint8_t *)pDest, read_addr, (uint32_t) count);
#endif

    return pDest;
}
