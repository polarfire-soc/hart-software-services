/* w25n01gv_apb.h
 * APIs for the Winbond W25N01GV NAND flash driver using APB QSPI interface
 * on PolarFire SoC FPGA.
 * Copyright 2023-2025 Microchip FPGA Embedded Systems Solutions.
 * SPDX-License-Identifier: MIT
 */

#ifndef W25N01GV_APB_H
#define W25N01GV_APB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// QSPI base address
#define QSPI_BASE_ADDR 0x70000300 // BNOR_BASE_ADDRESS

// QSPI register offsets
#define QSPI_WR_DATA   0x00 // Write data
#define QSPI_WR_CTRL1  0x04 // Control register 1
#define QSPI_RD_DATA   0x00 // Read data
#define QSPI_RD_STATUS 0x04 // Status register

// QSPI control register 1 bits
#define QSPI_CTRL1_ENABLE     0x00000001 // Enable QSPI
#define QSPI_CTRL1_CE         0x00000002 // Chip Enable
#define QSPI_CTRL1_nRESET     0x00000008 // Reset
#define QSPI_CTRL1_START_OP   0x00000800 // Start operation
#define QSPI_CTRL1_QT         0x003FC000 // TX quantity
#define QSPI_CTRL1_QR         0xFFC00000 // RX quantity
#define QSPI_CTRL1_S_QT       14         // TX quantity shift
#define QSPI_CTRL1_S_QR       22         // RX quantity shift
#define QSPI_CTRL1_S_DIV      7          // Clock divisor shift

// QSPI status register bits
#define QSPI_STATUS_IDLE      0x00000001 // QSPI idle
#define QSPI_STATUS_FT_FULL   0x00000010 // TX FIFO full
#define QSPI_STATUS_FR_EMPTY  0x00000002 // RX FIFO empty

// Timer for delays
#define TMR_MQSPI 3

// IO format for flash operations
typedef enum {
    W25_NORMAL,
    W25_DUAL_EX_RO,
    W25_QUAD_EX_RO
} w25_io_format;

// Bad block LUT entry structure
typedef struct w25_bb_lut_entry {
    uint8_t enable;
    uint8_t invalid;
    uint16_t lba;
    uint16_t pba;
} w25_bb_lut_entry_t;

// Function prototypes
void Flash_init(w25_io_format io_format);
void Flash_readid(uint8_t* buf);
uint8_t Flash_read(uint8_t* buf, uint32_t addr, uint32_t len);
uint8_t Flash_erase(void);
uint8_t Flash_erase_block(uint16_t block_nb);
uint8_t Flash_program(uint8_t* buf, uint32_t addr, uint32_t len);
uint32_t Flash_scan_for_bad_blocks(uint16_t* buf);
void Flash_read_status_regs(uint8_t * buf);
uint8_t Flash_read_bb_lut(w25_bb_lut_entry_t* lut_ptr);
uint8_t Flash_add_entry_to_bb_lut(uint16_t lba, uint16_t pba);
void Flash_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* W25N01GV_APB_H */