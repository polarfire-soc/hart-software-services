/***************************************************************************//**
 * Copyright 2022 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Bare metal driver for the Micron mq25T NAND flash memory.
 * This driver uses the MPFS MSS QSPI driver interface.
 */
#include "mpfs_hal/mss_hal.h"
#include "drivers/micron_mq25t/micron_mq25t.h"
#include "drivers/mss/mss_mmuart/mss_uart.h"

#include <assert.h> //assert
#include <string.h> //memcpy

/* The following constant must be defined if you want to use the interrupt mode
 * transfers provided by the MSS QSPI driver. Comment this out to use the
 * polling mode transfers.
 */
/* #define USE_QSPI_INTERRUPT      1u */

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_PAGES_PER_BLOCK                     64u
#define NUM_BLOCKS_PER_DIE                      1024u
#define PAGE_LENGTH                             2048u
#define BLOCK_LENGTH                            (PAGE_LENGTH * NUM_PAGES_PER_BLOCK)
#define DIE_SIZE                                (BLOCK_LENGTH * NUM_BLOCKS_PER_DIE)
#define NUM_LUTS                                20u

#define MSS_QSPI_CMDBYTES_LIMIT                   (511 - 3)

void Flash_init(mss_qspi_io_format io_format)
{
    (void)io_format;
}

void Flash_readid(uint8_t* buf)
{
    (void)buf;
}

uint8_t Flash_read(uint8_t* pDst, uint32_t srcAddr, uint32_t len)
{
    (void)pDst;
    (void)srcAddr;
    (void)len;

    return 0u;
}

uint8_t Flash_erase(void)
{
    return 0u;
}

uint8_t Flash_erase_block(uint16_t block_index)
{
    uint8_t status = 0xFFu;

    (void)block_index;

    return(status);
}

uint8_t Flash_program(uint8_t* buf, uint32_t addr, uint32_t len)
{
    uint8_t status = 0xFFu;

    (void)buf;
    (void)addr;
    (void)len;

    return status;
}

uint8_t Flash_add_entry_to_bb_lut(uint16_t lba, uint16_t pba)
{
    uint8_t status = 0u;

    (void)lba;
    (void)pba;

    return status;
}

void Flash_read_status_regs(uint8_t * buf)
{
    (void)buf;
}

uint32_t Flash_scan_for_bad_blocks(uint16_t* buf)
{
    uint32_t bad_count = 0u;

    (void)buf;

    return bad_count;
}

#ifdef __cplusplus
}
#endif
