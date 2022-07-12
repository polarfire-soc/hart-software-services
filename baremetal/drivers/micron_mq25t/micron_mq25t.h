/***************************************************************************//**
 * Copyright 2022 Microchip Corporation.
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
 * APIs for the Micron mq25t flash driver.
 * This driver uses the MPFS MSS QSPI driver interface.
 *=========================================================================*/
#ifndef MSS_MICRON_MQ25T_H_
#define MSS_MICRON_MQ25T_H_

#include <stdint.h>
#include "drivers/mss/mss_qspi/mss_qspi.h"

#ifdef __cplusplus
extern "C" {
#endif

void Flash_init(mss_qspi_io_format io_format);
void Flash_readid(uint8_t* buf);
uint8_t Flash_read(uint8_t* buf, uint32_t addr, uint32_t len);
uint8_t Flash_erase(void);
uint8_t Flash_erase_block(uint16_t block_nb);
uint8_t Flash_program(uint8_t* buf, uint32_t addr, uint32_t len);
uint32_t Flash_scan_for_bad_blocks(uint16_t* buf);
void Flash_read_status_regs(uint8_t * buf);
uint8_t Flash_add_entry_to_bb_lut(uint16_t lba, uint16_t pba);

#endif /* MSS_MICRON_MQ25T_H_ */
