/*******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
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
 * PolarFire SoC MSS eNVM bare metal software driver public APIs.
 *
 */

#ifndef MSS_ENVM_H
#define MSS_ENVM_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Base address for C_BUS set by Microsemi. All other REG_IF registers are
 * offset from this address - C_Bus address base
 */
#define FM_SYS_START_ADDR        0x20200000
#define MSS_ENVM_DATA_BASE       ((uint64_t) 0x0020220000llu)
#define MSS_ENVM_DATA_SECTOR2    (MSS_ENVM_DATA_BASE + 0x00000)
#define MSS_ENVM_DATA_SECTOR0    (MSS_ENVM_DATA_BASE + 0x02000)
#define MSS_ENVM_DATA_SECTOR1    (MSS_ENVM_DATA_BASE + 0x10000)
#define MSS_ENVM_DATA_SECTOR3    (MSS_ENVM_DATA_BASE + 0x1E000)

#define NUM_FM_PAGES    224u
#define NUM_SM_PAGES    32u

typedef struct {
uint8_t sector;
uint8_t page;
} envm_addr_t;


/**
* envm_set_clock function which setups the clock and timing for the eNVM
*
* Must be called after envm_init()
*
* \param[in] mss_frequency is the frequency the MSS is running at in MHz
*/
void     envm_set_clock(uint32_t mss_frequency);

/**
* envm_init function which sets up the programming and read parameters.
*
* Must be called first in order to use eNVM
*
* \pre eNVM powered up by controller
*
* \param[in] p_envm_params specifies a pointer to an array containing the eNVM
*             parameters retrieved through system services.
*
* \retval 0 passed
* \retval 1 failed
*
*/
uint8_t  envm_init(const uint8_t * p_envm_params);

/**
* Top level envm_program_page function which takes in absolute address
* performs a full program and verify cycle of the eNVM
* performs a preprogram - erase - write cycle and then a verify
*
* Erased is 0 and Program is 1
*
* \pre pageLatchData.pageData correctly setup with data to be programmed
*
* \param[in] absolute_address specifies the absolute address
*
* \retval 0 eNVM was programmed successfully
* \retval 1 eNVM verify error occurred during programming
* \retval 2 page is outside boundary error
* \retval 3 page latch load error
* \retval 4 startup previously failed
*/
uint_fast8_t envm_program_page_address(uint64_t absolute_address);

/**
* Top level envm_erase_sector function which takes in absolute address
* performs a preprogram - erase cycle of a sector
*
* \param[in] absolute_address specifies the absolute address
*
* \retval 0 eNVM was erased successfully
* \retval 2 page is outside boundary error
* \retval 4 startup previously failed
*/
uint_fast8_t envm_erase_sector_address(uint64_t absolute_address);

/**
* performs a full program and verify cycle of the eNVM
* performs a preprogram - erase - write cycle and then a verify
*
* Erased is 0 and Program is 1
*
* \pre pageLatchData.pageData correctly setup with data to be programmed
*
* \param[in] sector specifies one of the four sectors
* \param[in] page specifies one of the pages within a sector
*
* \retval 0 eNVM was programmed successfully
* \retval 1 eNVM verify error occurred during programming
* \retval 2 page is outside boundary error
* \retval 3 page latch load error
* \retval 4 startup previously failed
* \retval 5 sector outside bounds error
*/
uint_fast8_t envm_program_page(uint_fast8_t sector, uint_fast8_t page);

/**
* performs an erase of a full sector of the eNVM
* performs a preprogram - erase cycle
*
* \param[in] sector specifies one of the four sectors
*
* \retval 0 eNVM was programmed successfully
* \retval 4 startup previously failed
* \retval 5 sector outside bounds error
*/
uint_fast8_t envm_erase_sector(uint_fast8_t sector);

/**
* Top level envm_erase_bulk function
* performs a preprogram - erase cycle of all sectors
*
* \param[in] absolute_address specifies the absolute address
*
* \retval 0 eNVM was erased successfully
* \retval 4 startup previously failed
*/
uint_fast8_t envm_erase_bulk(void);

/**
* Returns the eNVM Sector & Page information derived from eNVM absolute address
* Returns 0 if Sector & Page are valid
*
*/
uint_fast8_t envm_address(uint64_t absolute_address, envm_addr_t * envm_address);

/**
* load block of data into pageData
*
*/
void envm_set_page_data(uint32_t *source);

/**
* clears pageData contents to 0
*
*/
void envm_clear_page_data(void);



#ifdef __cplusplus
}
#endif

#endif


