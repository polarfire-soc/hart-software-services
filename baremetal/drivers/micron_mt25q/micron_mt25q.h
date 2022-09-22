/***************************************************************************//**
 * Copyright 2019 - 2022 Microchip FPGA Embedded Systems Solutions.
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
 * Driver for MICRON N25Q QSPI flash memory.
 * This driver uses the MPFS MSS QSPI driver interface.
 */

#ifndef MSS_MICRON_MT25Q_H_
#define MSS_MICRON_MT25Q_H_

#include <stdint.h>
#include "drivers/mss/mss_qspi/mss_qspi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*//**
  The Flash_init() function initializes the MSS QSPI and the flash memory to
  normal SPI operations. The g_qspi_config.io_format is used for the read/write
  operations to choose the Flash memory commands accordingly.
  This function must be called before calling any other function provided by
  this driver.

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
void
Flash_init
(
    mss_qspi_io_format io_format
);

/*-------------------------------------------------------------------------*//**
  The Flash_readid() function returns first 1 byte of data of the device JEDEC ID.

  @param buf
  The rd_buf parameter provides a pointer to the buffer in which the driver will
  copy the JEDEC ID data. The buffer must be at least 1 bytes long.

  @return
    This function does not returns any value

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
void
Flash_readid
(
    uint8_t* buf
);

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
    This function does not returns any value

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
void
Flash_read
(
    uint8_t* buf,
    uint32_t addr,
    uint32_t len
);

/*-------------------------------------------------------------------------*//**
  The Flash_erase() function erases the complete device.

  @return
    This function returns a non-zero value if there was an error during erase
    operation. A zero return value indicates success.

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
uint8_t
Flash_erase
(
    void
);

/*-------------------------------------------------------------------------*//**
  The Flash_sector_erase() function erases the given sector.

  @return
    This function returns a non-zero value if there was an error during erase
    operation. A zero return value indicates success.

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
uint8_t Flash_sector_erase
(
        uint32_t addr
);

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
uint8_t Flash_program
(
    uint8_t* buf,
    uint32_t addr,
    uint32_t len
);

/*-------------------------------------------------------------------------*//**
  The Flash_read_status_regs() function reads all three status registers

  @param buf
  The buf parameter is a pointer to the buffer in which the driver will
  copy the status register values. The buffer should be large enough to store 6
  8-bit bytes. The sequence is
    - Status register
    - Non-volatile configuration registers (2 bytes)
    - Volatile configuration registers
    - Enhanced volatile configuration register
    - Flag status register

  @return
    This function does not return any value.

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
void
Flash_read_status_regs
(
    uint8_t * buf
);

/*-------------------------------------------------------------------------*//**
  The Flash_enter_xip() function puts the flash memory into the XIP mode. To exit
  XIP, use Flash_exit_xip() function or reset the device.
  After entering into the XIP mode, any AHB access to MSS QSPI register space
  will result into reading a byte from the flash memory over QSPI interface.
  The XIP mode uses 3 byte addressing.

  @return
    This function does not return any value.

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
void
Flash_enter_xip
(
    void
);

/*-------------------------------------------------------------------------*//**
  The Flash_exit_xip() function brings the Flash memory out of the XIP mode.

  @return
    This function does not return any value.

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
void
Flash_exit_xip
(
    void
);

/*-------------------------------------------------------------------------*//**
  The Flash_clr_flagstatusreg() function can be used in case there were errors
  in erase/program operations. This function will clear the error status so that
  subsequent erase/program operations can be initiated.

  @return
    This function does not return any value.

  @example

  ##### Example1

  Example

  @code

  @endcode

*/
void
Flash_clr_flagstatusreg
(
    void
);

#ifdef __cplusplus
}
#endif

#endif /* MSS_MICRON_MT25Q_H_*/
