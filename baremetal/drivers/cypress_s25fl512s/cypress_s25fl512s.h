/***************************************************************************//**
 * Copyright 2019-2021 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 *
 * Driver for CYPRESS_MT25Q QSPI flash memory.
 * This driver uses the MPFS MSS QSPI driver interface.
 *
 *
 * SVN $Revision:  $
 * SVN $Date:  $
 */

#ifndef MSS_CYPRESS_S25Fl512S_H_
#define MSS_CYPRESS_S25Fl512S_H_

#include <stddef.h>
#include <stdint.h>
#include "drivers/mss_qspi/mss_qspi.h"


#ifdef __cplusplus
extern "C" {
#endif

extern mss_qspi_config_t qspi_config;

void Flash_init
(
    mss_qspi_io_format io_format
);

void Flash_readid(uint8_t* rd_buf);

void Flash_erase(void);

void Flash_read
(
    uint8_t* buf,
    uint32_t read_addr,
    uint32_t read_len
);

void Flash_read_statusreg
(
    uint8_t* rd_buf
);

void Flash_read_enh_v_confreg
(
    uint8_t* rd_buf
);

void Flash_write_enh_v_confreg
(
    uint8_t* enh_v_val
);

uint8_t Flash_program
(
    uint8_t* buf,
    uint32_t wr_addr,
    uint32_t wr_len
);

void Flash_sector_erase(uint32_t addr);

void Flash_read_flagstatusreg
(
    uint8_t* rd_buf
);

void Flash_enter_xip
(
    void
);

void Flash_exit_xip
(
    void
);

void Flash_enter_normal_mode
(
    void
);

void Flash_read_nvcfgreg
(
    uint8_t* rd_buf
);

void Flash_force_normal_mode
(
    void
);

mss_qspi_io_format Flash_probe_io_format
(
    void
);

static inline void Flash_init_normal(void)
{
    qspi_config.clk_div =  MSS_QSPI_CLK_DIV_2;
    qspi_config.sample = MSS_QSPI_SAMPLE_POSAGE_SPICLK;
    qspi_config.spi_mode = MSS_QSPI_MODE3;
    qspi_config.xip = MSS_QSPI_DISABLE;
    qspi_config.io_format = MSS_QSPI_NORMAL;
    MSS_QSPI_configure(&qspi_config);
}

static inline void Flash_init_dual(void)
{
    qspi_config.clk_div =  MSS_QSPI_CLK_DIV_2;
    qspi_config.sample = MSS_QSPI_SAMPLE_POSAGE_SPICLK;
    qspi_config.spi_mode = MSS_QSPI_MODE3;
    qspi_config.xip = MSS_QSPI_DISABLE;
    qspi_config.io_format = MSS_QSPI_DUAL_FULL;
    MSS_QSPI_configure(&qspi_config);
}

static inline void Flash_init_quad(void)
{
    qspi_config.clk_div =  MSS_QSPI_CLK_DIV_2;
    qspi_config.sample = MSS_QSPI_SAMPLE_POSAGE_SPICLK;
    qspi_config.spi_mode = MSS_QSPI_MODE3;
    qspi_config.xip = MSS_QSPI_DISABLE;
    qspi_config.io_format = MSS_QSPI_QUAD_FULL;
    MSS_QSPI_configure(&qspi_config);
}

#define S25Fl512S_WRITE_STATUS_REG        0x01
#define S25Fl512S_READ_DATA               0x0C
#define S25Fl512S_WRITE_ENABLE            0x06
#define S25Fl512S_PROGRAM_EXECUTE         0x10
#define S25Fl512S_PAGE_DATA_READ          0x13
#define S25Fl512S_BLOCK_ERASE             0xDC


#define CYPRESS_RESET_ENABLE                   0x66
#define CYPRESS_RESET_MEMORY                   0x99

#define CYPRESS_READ_ID_OPCODE                 0x9F
#define CYPRESS_MIO_READ_ID_OPCODE             0xAF

#define CYPRESS_READ_DISCOVERY                 0x5A

#define CYPRESS_READ                           0x03
#define CYPRESS_FAST_READ                      0x0B
#define CYPRESS_DUALO_FAST_READ                0x3B
#define CYPRESS_DUALIO_FAST_READ               0xBB
#define CYPRESS_QUADO_FAST_READ                0x6B
#define CYPRESS_QUADIO_FAST_READ               0xEB
#define CYPRESS_DTR_FAST_READ                  0x0D
#define CYPRESS_DTR_DUALO_FAST_READ            0x3D
#define CYPRESS_DTR_DUALIO_FAST_READ           0xBD
#define CYPRESS_DTR_QUADO_FAST_READ            0x6D
#define CYPRESS_DTR_QUADIO_FAST_READ           0xED
#define CYPRESS_QUADIO_WORD_READ               0xE7

#define CYPRESS_4BYTE_READ                     0x13
#define CYPRESS_4BYTE_FAST_READ                0x0C
#define CYPRESS_4BYTE_DUALO_FAST_READ          0x3C
#define CYPRESS_4BYTE_DUALIO_FAST_READ         0xBC
#define CYPRESS_4BYTE_QUADO_FAST_READ          0x6C
#define CYPRESS_4BYTE_QUADIO_FAST_READ         0xEC
#define CYPRESS_4BYTE_DTR_FAST_READ            0x0E
#define CYPRESS_4BYTE_DTR_DUALIO_FAST_READ     0xBE
#define CYPRESS_4BYTE_DTR_QUADIO_FAST_READ     0xEE

#define CYPRESS_WRITE_ENABLE                   0x06
#define CYPRESS_WRITE_DISABLE                  0x04

#define CYPRESS_READ_STATUS_REG                0x05
#define CYPRESS_READ_FLAG_STATUS_REG           0x70
#define CYPRESS_READ_NV_CONFIG_REG             0xB5
#define CYPRESS_READ_V_CONFIG_REG              0x85
#define CYPRESS_READ_ENH_V_CONFIG_REG          0x65
#define CYPRESS_READ_EXT_ADDR_REG              0xC8
#define CYPRESS_READ_GEN_PURPOSE_READ_REG      0x96

#define CYPRESS_WR_STATUS_REG                  0x01
#define CYPRESS_WR_NV_CONFIG_REG               0xB1
#define CYPRESS_WR_V_CONFIG_REG                0x81
#define CYPRESS_WR_ENH_V_CONFIG_REG            0x61
#define CYPRESS_WR_EXT_ADDR_REG                0xC5
#define CYPRESS_CLR_FLAG_STATUS_REG            0x50

#define CYPRESS_PAGE_PROGRAM                   0x02
#define CYPRESS_DUAL_INPUT_FAST_PROG           0xA2
#define CYPRESS_EXT_DUAL_INPUT_FAST_PROG       0xD2
#define CYPRESS_QUAD_INPUT_FAST_PROG           0x32
#define CYPRESS_EXT_QUAD_INPUT_FAST_PROG       0x38

#define CYPRESS_4BYTE_PAGE_PROG                0x12
#define CYPRESS_4BYTE_QUAD_INPUT_FAST_PROG     0x34
#define CYPRESS_4BYTE_QUAD_INPUT_EXT_FAST_PROG 0x3E


#define CYPRESS_32KB_SUBSECTOR_ERASE          0x52
#define CYPRESS_4KB_SUBSECTOR_ERASE           0x20
#define CYPRESS_SECTOR_ERASE                  0xD8
#define CYPRESS_DIE_ERASE                     0xC4

#define CYPRESS_4BYTE_SECTOR_ERASE            0xDC
#define CYPRESS_4BYTE_4KB_SUBSECTOR_ERASE     0x21
#define CYPRESS_4BYTE_32KB_SUBSECTOR_ERASE    0x5C

#define CYPRESS_ENTER_4BYTE_ADDR_MODE         0xB7
#define CYPRESS_EXIT_4BYTE_ADDR_MODE          0xE9

#define CYPRESS_ENTER_QUAD_IO_MODE            0x35
#define CYPRESS_RESET_QUAD_IO_MODE            0xF5

#define CYPRESS_READ_SECTOR_PROTECTION        0x2D

#define CYPRESS_READ_V_LOCK_BITS              0xE8
#define CYPRESS_WRITE_V_LOCK_BITS             0xE5
#define CYPRESS_READ_NV_LOCK_BITS             0xE2
#define CYPRESS_READ_GLOBAL_FREEZE_BIT        0xA7
#define CYPRESS_READ_PASSWORD                 0x27

#define CYPRESS_RESET_PROTOCOL                0xFF

#ifdef __cplusplus
}
#endif

#endif /* MSS_CYPRESS_MT25Q_H_*/
