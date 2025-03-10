/***************************************************************************//**
 * Copyright 2023-2025 Microchip FPGA Embedded Systems Solutions.
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
 * APIs for the Winbond w25n01gv flash driver.
 * This driver uses the MPFS MSS QSPI driver interface.
 *=========================================================================*/
#ifndef MSS_WINBOND_MT25Q_H_
#define MSS_WINBOND_MT25Q_H_

#include <stdint.h>
#include "drivers/mss/mss_qspi/mss_qspi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*//**
 * The w25_bb_lut_entry_t defines the lookup table intry in the Winbond NAD
 * flash memory device.
 */
typedef struct w25_bb_lut_entry {
    uint8_t enable;
    uint8_t invalid;
    uint16_t lba;
    uint16_t pba;
} w25_bb_lut_entry_t;

/*-------------------------------------------------------------------------*//**
  The Flash_init() function initializes the MSS QSPI and the flash memory to
  normal SPI operations. The g_qspi_config.io_format is used for the read/write
  operations to choose the Flash memory commands accordingly.
  This function must be called before calling any other function provided by
  this driver.

  The Winbondw25n01gv device expects the command-address bytes on DQ0 always.
  Hence the only MSS QSPI IO formats that can be used are MSS_QSPI_NORMAL,
  MSS_QSPI_DUAL_EX_RO, MSS_QSPI_QUAD_EX_RO.

  Furthermore, the DUAL operations are not supported by the flash program
  commands (Flash read supports dual and quad IOs).
  When dual operations are selected the programming operations will fall-back
  to normal mode. Reads will happen on quad IO.

  It is recommended that QUAD mode is selected for faster operations.

  @param io_format
  The io_format parameter provides the SPI IO format that needs to be used for
  read/write operations.

  @return
    This function does not returns any value

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
void Flash_init(mss_qspi_io_format io_format);

/*-------------------------------------------------------------------------*//**
  The Flash_readid() function returns first 3 bytes of data of the device JEDEC ID.

  @param buf
  The rd_buf parameter provides a pointer to the buffer in which the driver will
  copy the JEDEC ID data. The buffer must be at least 3 bytes long.

  @return
    This function does not returns any value

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
void Flash_readid(uint8_t* buf);

/*-------------------------------------------------------------------------*//**
  The Flash_read() function reads data from the flash memory.

  @param buf
  The buf parameter is a pointer to the buffer in which the driver will
  copy the data read from the flash memory.

  @param addr
  The addr parameter is the address in the flash memory from which the driver
  will read the data.

  @param len
  The len parameter is the number of 8-bit bytes that will be read from the flash
  memory starting with the address indicated by the addr parameter.

  @return
    This function returns a non-zero value if there was an error during the read
    operation. A zero return value indicates success.

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
uint8_t Flash_read(uint8_t* buf, uint32_t addr, uint32_t len);

/*-------------------------------------------------------------------------*//**
  The Flash_erase() function erases the complete device.

  @return
    This function returns a non-zero value if there was an error during the erase
    operation. A zero return value indicates success.

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
uint8_t Flash_erase(void);

/*-------------------------------------------------------------------------*//**
  The Flash_erase_block() function erases the complete device.

  @return
    This function returns a non-zero value if there was an error during the erase
    operation. A zero return value indicates success.

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
uint8_t Flash_erase_block(uint16_t block_nb);

/*-------------------------------------------------------------------------*//**
  The Flash_program() function writes data into the flash memory.

  @param buf
  The rd_buf parameter provides a pointer to the buffer from which the data
  needs to be written into the flash memory.

  @param addr
  The addr parameter is an address in the flash memory to which the data will be
  written to.

  @param len
  The len parameter indicates the number of 8-bit bytes that will be written to
  the flash memory starting from the address indicated by the addr parameter.

  @return
    This function returns a non-zero value if there was an error during program
    operation. A zero return value indicates success.

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
uint8_t Flash_program(uint8_t* buf, uint32_t addr, uint32_t len);

/*-------------------------------------------------------------------------*//**
  The Flash_scan_for_bad_blocks() function scans for bad blocks within the flash
  memory. The NAND flash devices are allowed to be shipped with certain number of
  bad blocks. In such cases, the flash device is shipped with bad blocks markers
  written into the respective bad blocks. The markers will be permanently lost
  if the block is written or erased. This function can be called on a unused
  device to know the factory marked bad blocks before erasing or writing on those
  block. The bad blocks can then be re-mapped to good blocks by adding a bad
  block (lba) to good block (pba) mapping in the lookup table (LUT) within the
  flash memory using the Flash_add_entry_to_bb_lut() function. Maximum 20 entries
  are allowed in the lookup table in the W25N01 devices. Only the first block of
  the shipped device is guaranteed to be a good block.

  Bad blocks can also be developed during the regular usage of the flash memory
  device. These should be handled as and when errors happen during program/erase
  operations and the LUT should be updated accordingly.

  @param buf
  The buf parameter is a pointer to the buffer in which the driver will
  write the block numbers of the blocks containing the bad-block marker. The
  buffer must be larg enough to contain 20 block numbers.

  @return
    This function returns the total number of bad blocks found during the scan.

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
uint32_t Flash_scan_for_bad_blocks(uint16_t* buf);

/*-------------------------------------------------------------------------*//**
  The Flash_read_status_regs() function reads all three status registers

  @param buf
  The buf parameter is a pointer to the buffer in which the driver will
  copy the status register values. The buffer should be large enough to store 3
  8-bit bytes.

  @return
    This function does not return any value.

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
void Flash_read_status_regs(uint8_t * buf);

/*-------------------------------------------------------------------------*//**
  The Flash_read_bb_lut() function reads the look up table (LUT) in the flash
  memory that contains the bad block (lba) to good block (pba) mapping.

  @param lut_ptr
  The lut_ptr parameter is a pointer to the buffer in which the driver will
  copy the LUT data read from the flash memory. The buffer must be large enough
  to hold 20 LUT entries of type w25_bb_lut_entry_t.

  @return
    This function returns the number of valid bad block mappings in the LUT.

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
uint8_t Flash_read_bb_lut(w25_bb_lut_entry_t* lut_ptr);



/*-------------------------------------------------------------------------*//**
  The Flash_add_entry_to_bb_lut() function adds an entry to the look up table
  (LUT) in the flash memory that contains the bad block (lba) to good block
  (pba) mapping. When the bad blocks are found during the initial bad block scan
  ( using Flash_scan_for_bad_blocks() function) or when a bad block is found
  during regular operation (indicated by errors during programming or erase
  operation), this function can be used to map these bad block to a good block
  where no errors are seen so far.

  After the LUT entry is added, any access to the bad block will be directed to
  the mapped good block. The bad block will not be accessed.

  @param lba
  The lba parameter is the block number of the bad block.

  @param pba
  The pba parameter is the block number of a good block.

  @return
    This function returns 1 if successful, or 0 otherwise

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
uint8_t Flash_add_entry_to_bb_lut(uint16_t lba, uint16_t pba);

/*
*/
void Flash_flush(void);
#ifdef __cplusplus
}
#endif

#endif /* MSS_WINBOND_MT25Q_H_*/
