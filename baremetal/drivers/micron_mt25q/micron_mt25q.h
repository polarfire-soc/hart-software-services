/***************************************************************************//**
 * (c) Copyright 2018 Microsemi SoC Products Group.  All rights reserved.
 *
 * Driver for MICRON_MT25Q QSPI flash memory on Xilinx Emulation board.
 * This dirver uses the PSE MSS QSPI driver interface.
 *
 *
 * SVN $Revision:  $
 * SVN $Date:  $
 */

#ifndef MSS_MICRON_MT25Q_H_
#define MSS_MICRON_MT25Q_H_

#include <stddef.h>
#include <stdint.h>
#include "mss_qspi.h"

#ifdef __cplusplus
extern "C" {
#endif

extern mss_qspi_config_t qspi_config;

void Flash_init
(
    mss_qspi_io_format io_format
);

void Flash_readid(uint8_t* rd_buf);

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
void Flash_die_erase(void);
#define Flash_erase Flash_die_erase

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


//mss_qspi_io_format Flash_descover_ioformat
//(
//    void
//);

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


#define MICRON_RESET_ENABLE           		0x66
#define MICRON_RESET_MEMORY           		0x99

#define MICRON_READ_ID_OPCODE               0x9F
#define MICRON_MIO_READ_ID_OPCODE           0xAF

#define MICRON_READ_DISCOVERY           	0x5A

#define MICRON_READ			           		0x03
#define MICRON_FAST_READ	           		0x0B
#define MICRON_DUALO_FAST_READ        		0x3B
#define MICRON_DUALIO_FAST_READ        		0xBB
#define MICRON_QUADO_FAST_READ        		0x6B
#define MICRON_QUADIO_FAST_READ        		0xEB
#define MICRON_DTR_FAST_READ        		0x0D
#define MICRON_DTR_DUALO_FAST_READ        	0x3D
#define MICRON_DTR_DUALIO_FAST_READ        	0xBD
#define MICRON_DTR_QUADO_FAST_READ        	0x6D
#define MICRON_DTR_QUADIO_FAST_READ        	0xED
#define MICRON_QUADIO_WORD_READ        		0xE7

#define MICRON_4BYTE_READ 					0x13
#define MICRON_4BYTE_FAST_READ 				0x0C
#define MICRON_4BYTE_DUALO_FAST_READ 		0x3C
#define MICRON_4BYTE_DUALIO_FAST_READ 		0xBC
#define MICRON_4BYTE_QUADO_FAST_READ 		0x6C
#define MICRON_4BYTE_QUADIO_FAST_READ 		0xEC
#define MICRON_4BYTE_DTR_FAST_READ 			0x0E
#define MICRON_4BYTE_DTR_DUALIO_FAST_READ 	0xBE
#define MICRON_4BYTE_DTR_QUADIO_FAST_READ 	0xEE

#define MICRON_WRITE_ENABLE					0x06
#define MICRON_WRITE_DISABLE				0x04

#define MICRON_READ_STATUS_REG 				0x05
#define MICRON_READ_FLAG_STATUS_REG			0x70
#define MICRON_READ_NV_CONFIG_REG			0xB5
#define MICRON_READ_V_CONFIG_REG			0x85
#define MICRON_READ_ENH_V_CONFIG_REG		0x65
#define MICRON_READ_EXT_ADDR_REG			0xC8
#define MICRON_READ_GEN_PURPOSE_READ_REG	0x96

#define MICRON_WR_STATUS_REG				0x01
#define MICRON_WR_NV_CONFIG_REG				0xB1
#define MICRON_WR_V_CONFIG_REG				0x81
#define MICRON_WR_ENH_V_CONFIG_REG			0x61
#define MICRON_WR_EXT_ADDR_REG				0xC5
#define MICRON_CLR_FLAG_STATUS_REG			0x50

#define MICRON_PAGE_PROGRAM					0x02
#define MICRON_DUAL_INPUT_FAST_PROG			0xA2
#define MICRON_EXT_DUAL_INPUT_FAST_PROG		0xD2
#define MICRON_QUAD_INPUT_FAST_PROG			0x32
#define MICRON_EXT_QUAD_INPUT_FAST_PROG		0x38

#define MICRON_4BYTE_PAGE_PROG				0x12
#define MICRON_4BYTE_QUAD_INPUT_FAST_PROG	0x34
#define MICRON_4BYTE_QUAD_INPUT_EXT_FAST_PROG	0x3E


#define MICRON_32KB_SUBSECTOR_ERASE			0x52
#define MICRON_4KB_SUBSECTOR_ERASE			0x20
#define MICRON_SECTOR_ERASE					0xD8
#define MICRON_DIE_ERASE					0xC4

#define MICRON_4BYTE_SECTOR_ERASE			0xDC
#define MICRON_4BYTE_4KB_SUBSECTOR_ERASE 	0x21
#define MICRON_4BYTE_32KB_SUBSECTOR_ERASE 	0x5C

#define MICRON_ENTER_4BYTE_ADDR_MODE		0xB7
#define MICRON_EXIT_4BYTE_ADDR_MODE 		0xE9

#define MICRON_ENTER_QUAD_IO_MODE			0x35
#define MICRON_RESET_QUAD_IO_MODE			0xF5

#define MICRON_READ_SECTOR_PROTECTION 		0x2D

#define MICRON_READ_V_LOCK_BITS 			0xE8
#define MICRON_WRITE_V_LOCK_BITS 			0xE5
#define MICRON_READ_NV_LOCK_BITS 			0xE2
#define MICRON_READ_GLOBAL_FREEZE_BIT 		0xA7
#define MICRON_READ_PASSWORD 				0x27

#define MICRON_RESET_PROTOCOL               0xFF

#ifdef __cplusplus
}
#endif

#endif /* MSS_MICRON_MT25Q_H_*/
