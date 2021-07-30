/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
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
 * PolarFire SoC MSS eMMC SD bare metal software driver public API.
 *
 * SVN $Revision: 12579 $
 * SVN $Date: 2019-12-04 16:41:30 +0530 (Wed, 04 Dec 2019) $
 */
/*=========================================================================*//**
  @mainpage PolarFire SoC MSS eMMC SD Bare Metal Driver

  ==============================================================================
  Introduction
  ==============================================================================
  The PolarFire SoC MSS includes an SD host controller and an eMMC/SD PHY.
  The SD Host Controller can support multiple eMMC/SD standards with bus
  widths of 1 bit, 4 bits, and 8 bits at clock rates up to 200 MHz.

  The PolarFire SoC MSS eMMC SD software driver, provided as C source code,
  supports a set of functions for controlling eMMC/SD as part of a bare-metal
  system where no operating system is available. The driver can be adapted for
  use as part of an operating system, but the implementation of the adaptation
  layer between the driver and the operating system's driver model is outside
  the scope of the driver.

  The PolarFire SoC MSS eMMC SD driver provides the following features:
    - Configuring the eMMC/SD/SDIO
    - Single block read and write without DMA.
    - Multiple or single block read and write with DMA (SDMA, ADMA2).
    - eMMC command queue block read and write.
    - eMMC standards LEGACY, SDR, DDR, HS200, HS400 and HS400-ES
    - SD card standards Default Speed(DS), High Speed(HS), UHS-I(SDR12, SDR25,
      SDR50, SDR104, DDR50).
    - Single block read and write operation for SDIO.

  ==============================================================================
  Theory of Operation
  ==============================================================================
  The PolarFire SoC MSS eMMC SD driver functions allow 512-byte blocks of data
  to be written to and read from a eMMC/SD device connected to the host
  controller. The blocks can be read/written singly or using the multi block
  functions.
  There are two variants of the single block functions. One set are blocking,
  the other are non-blocking. The multi block functions are non-blocking.

  Note: The eMMC/SD device connected to the eMMC/SD host hardware must support
  a sector size of 512-bytes. This is the default block size for the
  eMMC/SD device > 2GB.

  The PolarFire SoC MSS eMMC SD driver functions are grouped into the following
  categories:
    - Initialization
    - Block Transfer Control
    - Block Transfer Status
    - Interrupt Handling
    - Command Queue

  --------------------------------
  Initialization
  --------------------------------
  The initialization of the MSS eMMC SD driver involves the following steps:
    - Initialize the mss_mmc_cfg_t data structure.
    - Call the MSS_MMC_init() function.

  The configuration data structure mss_mmc_cfg_t should set eMMC/SD/SDIO
  clock frequency, data width, card type and speed before calling the
  MSS_MMC_init() function.

  The MSS_MMC_init() function takes a pointer to the configuration data
  structure of type mss_mmc_cfg_t.

  --------------------------------
  Block Transfer Control
  --------------------------------
  The following functions are used for block read and write:
    - MSS_MMC_single_block_read()
    - MSS_MMC_single_block_write()
    - MSS_MMC_sdma_read()
    - MSS_MMC_sdma_write()
    - MSS_MMC_adma2_read()
    - MSS_MMC_adma2_write()
    - MSS_MMC_sdio_single_block_read()
    - MSS_MMC_sdio_single_block_write()

  Write Transfer

  To write a single block of data to the eMMC/SD device, a call is made to the
  MSS_MMC_single_block_write() function.

  To write a single block or multiple blocks of data to the eMMC/SD device,
  using DMA, a call is made to the MSS_MMC_sdma_write() or MSS_MMC_adma2_write()
  functions.

  To write a single block of data to the SDIO device, a call is made to the
  MSS_MMC_sdio_single_block_write() function.

  Read Transfer

  To read a single block of data stored within the eMMC/SD device, a call is
  made to the MSS_MMC_single_block_read() function.

  To read a single block or multiple blocks of data stored within the eMMC/SD
  device, using DMA, a call is made to the MSS_MMC_sdma_read() or
  MSS_MMC_adma2_read() functions.

  To read a single block of data stored within the SDIO device, a call is made
  to the MSS_MMC_sdio_single_block_read() function.

  --------------------------------
  Block Transfer Status
  --------------------------------
  The status of the eMMC SD block read or write transfer can be retrieved
  using the MSS_MMC_get_transfer_status() function.

  --------------------------------
  Interrupt Handling
  --------------------------------
  The MSS_MMC_set_handler() function is used to register a handler function
  that will be called by the driver when a block transfer completes. The
  driver passes the outcome of the transfer to the completion handler in
  the form of a status parameter indicating if the transfer was successful
  or the type of error that occurred during the transfer. The application
  must create and register a transfer completion handler function to suit
  the application.

  --------------------------------
  Command Queue
  --------------------------------
  The following functions are used for eMMC command queue operation:
    - MSS_MMC_cq_init()
    - MSS_MMC_cq_write()
    - MSS_MMC_cq_read()

  The MSS_MMC_cq_init() function initializes command queue in the eMMC device
  and the host controller.

  To write a single block or multiple blocks of data to the eMMC device using
  a command queue, a call is made to the MSS_MMC_cq_write() function. This
  function supports up to 32 tasks.

  To read a single block or multiple blocks of data stored within the eMMC
  device using a command queue, a call is made to the MSS_MMC_cq_read()
  function. This function supports up to 32 tasks.

 *//*=========================================================================*/
#ifndef __MSS_MMC_H
#define __MSS_MMC_H

#include "hal/cpu_types.h"

#ifdef __cplusplus
extern "C"
#endif

/*----------------------------------------------------------------------------*/
/* Clock rate*/
#define MSS_MMC_CLOCK_400KHZ            400u
#define MSS_MMC_CLOCK_12_5MHZ           12500u
#define MSS_MMC_CLOCK_25MHZ             25000u
#define MSS_MMC_CLOCK_26MHZ             26000u
#define MSS_MMC_CLOCK_50MHZ             50000u
#define MSS_MMC_CLOCK_70MHZ             70000u
#define MSS_MMC_CLOCK_100MHZ            100000u
#define MSS_MMC_CLOCK_200MHZ            200000u
/*  card type */
#define MSS_MMC_CARD_TYPE_NONE          0u
#define MSS_MMC_CARD_TYPE_MMC           1u
#define MSS_MMC_CARD_TYPE_SD            2u
#define MSS_MMC_CARD_TYPE_SDIO          3u
#define MSS_MMC_CARD_TYPE_COMBO         4u

/* Host controller eMMC mode select */

/* High-speed single data rate supports clock frequency up to 52 MHz and data
 * bus width of 1 bit, 4 bits, and 8 bits.
 */
#define MSS_MMC_MODE_SDR                0x2u

/* High speed double data rate supports clock frequency up to 52 MHz and data
 * bus width of 4 bits and 8 bits.
 */
#define MSS_MMC_MODE_DDR                0x3u

/* SDR data sampling supports clock frequency up to 200 MHz and data bus width
 *  of 4 bits and 8 bits.
 */
#define MSS_MMC_MODE_HS200              0x4u

/* DDR data sampling supports clock frequency up to 200 MHz and data bus width
 * of 8 bits.
 */
#define MSS_MMC_MODE_HS400              0x5u

/* HS400 mode with Enhanced Strobe. */
#define MSS_MMC_MODE_HS400_ES           0x6u

/* Backwards compatibility with legacy MMC card supports clock frequency up to
 * 26MHz and data bus width of 1 bit, 4 bits, and 8 bits.
 */
#define MSS_MMC_MODE_LEGACY             0x7u

#define MSS_MMC_MODE_MASK               0x00000007u
#define MSS_MMC_MODE_SDCARD             0x0u

/* Host controller SD/SDIO card mode select */

/* Default speed supports clock frequency up to 25 MHz and data bus width of
 * 4 bits.
 */
#define MSS_SDCARD_MODE_DEFAULT_SPEED   0x8u

/* High-speed supports clock frequency up to 50 MHz and data bus width of
 * 4 bits.
 */
#define MSS_SDCARD_MODE_HIGH_SPEED      0x9u

/* Ultra-High speed-I (UHS-I) single data rate supports clock frequency up to
 * 25 MHz and data bus width of 4 bits.
 */
#define MSS_SDCARD_MODE_SDR12           0xAu

/* Ultra-High speed-I (UHS-I) single data rate supports clock frequency up to
 * 50 MHz and data bus width of 4 bits.
 */
#define MSS_SDCARD_MODE_SDR25           0xBu

/* Ultra-High speed-I (UHS-I) single data rate supports clock frequency up to
 * 100 MHz and data bus width of 4 bits.
 */
#define MSS_SDCARD_MODE_SDR50           0xCu

/* Ultra-High speed-I (UHS-I) single data rate supports clock frequency up to
 * 208 MHz and data bus width of 4 bits.
 */
#define MSS_SDCARD_MODE_SDR104          0xDu

/* Ultra-High speed-I (UHS-I) double data rate supports clock frequency up to
 * 50 MHz and data bus width of 4 bits.
 */
#define MSS_SDCARD_MODE_DDR50           0xEu

/* Host controller data width */
#define MSS_MMC_DATA_WIDTH_1BIT         0x00u
#define MSS_MMC_DATA_WIDTH_4BIT         0x01u
#define MSS_MMC_DATA_WIDTH_8BIT         0x02u

/* eMMC bus voltage */
/* 1.8v */
#define MSS_MMC_1_8V_BUS_VOLTAGE     18u
/* 3.3v */
#define MSS_MMC_3_3V_BUS_VOLTAGE      33u

#define MSS_SDIO_FUNCTION_NUMBER_0      0u
#define MSS_SDIO_FUNCTION_NUMBER_1      1u
#define MSS_SDIO_FUNCTION_NUMBER_2      2u
#define MSS_SDIO_FUNCTION_NUMBER_3      3u
#define MSS_SDIO_FUNCTION_NUMBER_4      4u
#define MSS_SDIO_FUNCTION_NUMBER_5      5u
#define MSS_SDIO_FUNCTION_NUMBER_6      6u
#define MSS_SDIO_FUNCTION_NUMBER_7      7u

/*-------------------------------------------------------------------------*//**
The mss_mmc_status_t type is used to indicate the return status of the eMMC/SD
data transfer. A variable of this type is returned by the MSS_MMC_init(),
MSS_MMC_single_block_write(), MSS_MMC_single_block_read(), MSS_MMC_sdma_write(),
MSS_MMC_sdma_read(), MSS_MMC_adma2_write(), MSS_MMC_adma2_read(),
MSS_MMC_cq_init(), MSS_MMC_cq_write(), MSS_MMC_cq_read(),
MSS_MMC_sdio_single_block_read(), MSS_MMC_sdio_single_block_write() functions.
*/
typedef enum
{
    MSS_MMC_INIT_SUCCESS = 0u,
    MSS_MMC_INIT_FAILURE,
    MSS_MMC_NOT_INITIALISED,
    MSS_MMC_TRANSFER_IN_PROGRESS,
    MSS_MMC_TRANSFER_FAIL,
    MSS_MMC_TRANSFER_SUCCESS,
    MSS_MMC_DWIDTH_ERR,
    MSS_MMC_RCA_ERROR,
    MSS_MMC_CID_RESP_ERR,
    MSS_MMC_OP_COND_ERR,
    MSS_MMC_RESET_ERR,
    MSS_MMC_CRC_ERR,
    MSS_MMC_UNSUPPORTED_HW_REVISION,
    MSS_MMC_INVALID_PARAMETER,
    MSS_MMC_NO_ERROR,
    MSS_MMC_BASE_CLK_IS_ZERO_ERR,
    MSS_MMC_CARD_STATE_STABLE_ERR,
    MSS_MMC_CARD_INSERTED_ERR,
    MSS_MMC_MODE_NOT_SUPPORT_DATAWIDTH,
    MSS_MMC_CLK_DIV_ERR,
    MSS_MMC_RESPONSE_ERROR,
    MSS_MMC_ERR_INTERRUPT,
    MSS_MMC_ERR_SWITCH_VOLTAGE_FAILED,
    MSS_MMC_CARD_SELECT_ERROR,
    MSS_MMC_CARD_SELECT_SUCCESS,
    MSS_MMC_DEVICE_NOT_SUPPORT_HS400,
    MSS_MMC_DEVICE_NOT_SUPPORT_HS200,
    MSS_MMC_DEVICE_NOT_SUPPORT_DDR,
    MSS_MMC_DEVICE_NOT_SUPPORT_SDR,
    MSS_MMC_DEVICE_NOT_SUPPORT_LOW_POWER,
    MSS_MMC_HS400_MODE_SETUP_FAILURE,
    MSS_MMC_DEVICE_NOT_SUPPORT_CQ,
    MSS_MMC_CQ_INIT_FAILURE,
    MSS_MMC_CQ_NOT_INITIALISED,
    MSS_MMC_SDCARD_NOT_SUPPORT_SPEED,
    MSS_MMC_SDCARD_NOT_SUPPORT_VOLTAGE,
    MSS_MMC_SDCARD_NOT_SUPPORT_BUS_MODE,
    MSS_MMC_SDCARD_CMD6_SWITCH_ERROR,
    MSS_MMC_SDCARD_TUNING_FAILED,
    MSS_MMC_SDIO_ERR_BUS_SPEED_UNSUPP,
    MSS_MMC_DEVICE_NOT_SUPPORT_HPI,
    MSS_MMC_DEVICE_IS_NOT_IN_HPI_MODE,
    MSS_MMC_DEVICE_HPI_NOT_DISABLED,
    MSS_MMC_DATA_SIZE_IS_NOT_MULTI_BLOCK,
    MSS_MMC_DEVICE_ERROR
} mss_mmc_status_t;

/*-------------------------------------------------------------------------*//**
  The mss_mmc_cfg_t type provides the prototype for the configuration values of
  the MSS eMMC SD driver. The application need to create a record of this type
  to hold the configuration of the eMMC/SD/SDIO. The MSS_MMC_init() function
  initializes the MSS eMMC SD using this structure. A pointer to an initialized
  of this structure should be passed as the first parameter to the
  MSS_MMC_init() function.
 */
typedef struct
{
    /* Specifies the clock frequency of the eMMC/SD/SDIO devices */
     uint32_t clk_rate;
    /* Specifies the card type is the eMMC/SD/SDIO */
     uint8_t card_type;
    /* Specifies the data bus width of the eMMC/SD/SDIO */
     uint8_t data_bus_width;
    /* Specifies the bus speed mode of the eMMC/SD/SDIO */
     uint8_t bus_speed_mode;
    /* Specifies the bus voltage for eMMC */
     uint8_t bus_voltage;
} mss_mmc_cfg_t;

/*-------------------------------------------------------------------------*//**
  This type definition specifies the prototype of a function that can be
  registered with this driver as a eMMC/SD transfer completion handler function
  through a call to MSS_MMC_set_handler(). The eMMC/SD transfer completion
  handler will be called by the driver when an eMMC/SD transfer completes. The
  PolarFire SoC MSS eMMC SD driver passes the outcome of the transfer to the
  completion handler in the form of a status parameter indicating if the
  transfer is successful or the type of error that occurred during the transfer.
*/
typedef void (*mss_mmc_handler_t)(uint32_t status);

/*-----------------------------Public APIs------------------------------------*/

/*-------------------------------------------------------------------------*//**
  The MSS_MMC_init() function initializes the MSS eMMC SD host controller and
  the eMMC/SD/SDIO device. The MSS_MMC_init()function takes a pointer to a
  configuration data structure of type mss_mmc_cfg_t as parameter. This
  configuration data structure contains all the information required to
  configure the MSS eMMC SD.
  The configuration passed to the MSS_MMC_init() function specifies the type
  of interface used to connect the MSS eMMC SD host controller and the
  eMMC/SD/SDIO device. It also specifies the allowed clock frequency, data bus
  width and bus speed mode. The MSS_MMC_init() function must be called prior
  to any MSS eMMC SD data transfer functions being called.

  @param cfg
  This parameter is a pointer to a data structure of type mss_mmc_cfg_t
  containing the MSS eMMC SD desired configuration. The application must fill
  the configuration data structure parameters before passing it as parameter to
  the call to the MSS_MMC_init() function.

  @return
  This function returns the initialization status of the eMMC/SD/SDIO device as
  a value of type mss_mmc_status_t.

  Example:
  The following example shows how to initialize the eMMC device and configure
  the data rate 25Mhz.
  @code

    mss_mmc_cfg_t g_mmc0;
    mss_mmc_status_t ret_status;

    g_mmc0.clk_rate = MSS_MMC_CLOCK_25MHZ;
    g_mmc0.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc0.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
    g_mmc0.bus_speed_mode = MSS_MMC_MODE_LEGACY;
    g_mmc.bus_voltage = MSS_MMC_3_3V_BUS_VOLTAGE;

    ret_status = MSS_MMC_init(&g_mmc0);
    if (MSS_MMC_INIT_SUCCESS == ret_status)
    {
    //...
    }
  @endcode
 */
mss_mmc_status_t
MSS_MMC_init
(
    const mss_mmc_cfg_t * cfg
);

/*-------------------------------------------------------------------------*//**
  The MSS_MMC_single_block_write() function is used to transmit a single block
  of data from the host controller to the eMMC/SD device. The size of the block
  of data transferred by this function is always 512 bytes, which is the
  standard sector size for all eMMC/SD devices with a capacity of greater than
  2 GB.

  Note: This function is a blocking function and will not return until the
  write operation is successful or an error occurs.

  @param src_addr
  This parameter is a pointer to a buffer containing the data to be stored in
  the eMMC/SD device. The buffer to which this parameter points should be
  declared with a minimum size of 512 bytes.

  @param dst_addr
  Specifies the sector address in the eMMC/SD device where the data is to be
  stored.
  Note: For eMMC/SD devices of greater than 2 GB in size, this address refers
  to a 512-byte sector.

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  The following example shows how to initialize the device, perform a single
  block transfer.
  @code

    #define SECT_1 0x01u
    #define BUFFER_SIZE 512u

    mss_mmc_cfg_t g_mmc0;
    mss_mmc_status_t ret_status;
    uint8_t tx_data_buffer[BUFFER_SIZE] = {0u};

    g_mmc0.clk_rate = MSS_MMC_CLOCK_25MHZ;
    g_mmc0.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc0.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
    g_mmc0.bus_speed_mode = MSS_MMC_MODE_LEGACY;
    g_mmc.bus_voltage = MSS_MMC_3_3V_BUS_VOLTAGE;

    for (loop_count = 0; loop_count < (BUFFER_SIZE); loop_count++)
    {
        tx_data_buffer[loop_count] = 0x45 + loop_count;
    }

    ret_status = MSS_MMC_init(&g_mmc0);
    if (MSS_MMC_INIT_SUCCESS == ret_status)
    {
        ret_status = MSS_MMC_single_block_write(tx_data_buffer, SECT_1);
        if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
        {
            //..
        }
    }
  @endcode
 */
mss_mmc_status_t
MSS_MMC_single_block_write
(
    const uint32_t * src_addr,
    uint32_t dst_addr
);

/*-------------------------------------------------------------------------*//**
  The MSS_MMC_single_block_read() function is used to read a single block of
  data from the eMMC/SD device to the host controller. The size of the block
  of data read by this function is always 512 bytes, which is the standard
  sector size for all eMMC/SD devices with a capacity of greater than 2 GB.

  Note: This function is a blocking function and will not return until the
  read operation is successful or an error occurs.

  @param src_addr
  Specifies the sector address in the eMMC/SD device from where the data is
  to be read.
  Note: For eMMC/SD devices of greater than 2 GB in size, this address refers
  to a 512-byte sector.

  @param dst_addr
  This parameter is a pointer to a buffer where the data to read from the
  eMMC/SD device will be stored. The buffer to which this parameter points
  should be declared with a minimum size of 512 bytes.

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  The following example shows how to initialize the device, perform a single
  block transfer and read back the data from the sector written to within
  the eMMC device.
  @code

    #define SECT_1 0x01u
    #define BUFFER_SIZE 512u

    mss_mmc_cfg_t g_mmc0;
    mss_mmc_status_t ret_status;
    uint8_t tx_data_buffer[BUFFER_SIZE] = {0u};
    uint8_t rx_data_buffer[BUFFER_SIZE] = {0u};

    g_mmc0.clk_rate = MSS_MMC_CLOCK_25MHZ;
    g_mmc0.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc0.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
    g_mmc0.bus_speed_mode = MSS_MMC_MODE_LEGACY;
    g_mmc.bus_voltage = MSS_MMC_3_3V_BUS_VOLTAGE;

    for (loop_count = 0; loop_count < (BUFFER_SIZE); loop_count++)
    {
        tx_data_buffer[loop_count] = 0x45 + loop_count;
    }

    ret_status = MSS_MMC_init(&g_mmc0);
    if (MSS_MMC_INIT_SUCCESS == ret_status)
    {
        ret_status = MSS_MMC_single_block_write(tx_data_buffer, SECT_1);
        if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
        {
            ret_status = MSS_MMC_single_block_read(SECT_1, rx_data_buffer);
            if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
            {
                //..
            }
        }
    }
  @endcode
 */
mss_mmc_status_t
MSS_MMC_single_block_read
(
    uint32_t src_addr,
    uint32_t * dst_addr
);

/*-------------------------------------------------------------------------*//**
  The MSS_MMC_sdma_write() function is used to transfer a single block or multi
  blocks of data from the host controller to the eMMC/SD device using SDMA.
  The size of the block of data transferred by this function must be set to 512
  bytes or a multiple of 512 bytes. The 512 bytes is the standard sector
  size for all eMMC/SD devices with a capacity of greater than 2 GB.

  Note: A call to MSS_MMC_sdma_write() while a transfer is in progress will not
  initiate a new transfer. Use the MSS_MMC_get_transfer_status() function
  or a completion handler registered by the MSS_MMC_set_handler() function
  to check the status of the current transfer before calling the
  MSS_MMC_sdma_write() function again.

  Note: This function is a non-blocking function and returns immediately after
  initiating the write transfer.

  @param src
  This parameter is a pointer to a buffer containing the data to be stored in
  the eMMC/SD device. The buffer to which this parameter points must be
  declared with a minimum size of 512 bytes.

  @param dest
  Specifies the sector address in the eMMC/SD device where the data is to be
  stored.
  Note: For eMMC/SD devices of greater than 2 GB in size, this address refers
  to a 512-byte sector.

  @param size
  Specifies the size in bytes of the requested transfer. The value of size must
  be a multiple of 512 but not greater than (32MB - 512).

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  The following example shows how to initialize the device, perform a multi
  block write transfer using SDMA.
  @code

    #define SECT_1 0x01u
    #define BUFFER_SIZE 4096u

    mss_mmc_cfg_t g_mmc0;
    mss_mmc_status_t ret_status;
    uint8_t data_buffer[BUFFER_SIZE];
    uint32_t loop_count;

    g_mmc0.clk_rate = MSS_MMC_CLOCK_25MHZ;
    g_mmc0.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc0.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
    g_mmc0.bus_speed_mode = MSS_MMC_MODE_LEGACY;
    g_mmc.bus_voltage = MSS_MMC_3_3V_BUS_VOLTAGE;

    for (loop_count = 0; loop_count < (BUFFER_SIZE); loop_count++)
    {
        data_buffer[loop_count] = 0x45 + loop_count;
    }

    ret_status = MSS_MMC_init(&g_mmc0);
    if (MSS_MMC_INIT_SUCCESS == ret_status)
    {
        ret_status = MSS_MMC_sdma_write(data_buffer, SECT_1, BUFFER_SIZE);
        do
        {
            ret_status = MSS_MMC_get_transfer_status();
        }while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
    }
  @endcode
 */
mss_mmc_status_t
MSS_MMC_sdma_write
(
    const uint8_t *src,
    uint32_t dest,
    uint32_t size
);
/*-------------------------------------------------------------------------*//**
  The MSS_MMC_sdma_read() function is used to read a single block or multiple
  blocks of data from the eMMC/SD device to the host controller using SDMA. The
  size of the block of data read by this function must be set to 512 bytes or
  a multiple of 512 bytes. The 512 bytes is the standard sector size for all
  eMMC/SD devices with a capacity of greater than 2 GB.

  Note: A call to MSS_MMC_sdma_read() while a transfer is in progress will not
  initiate a new transfer. Use the MSS_MMC_get_transfer_status() function
  or a completion handler registered by the MSS_MMC_set_handler() function
  to check the status of the current transfer before calling the
  MSS_MMC_sdma_read() function again.

  Note: This function is a non-blocking function and will return immediately
  after initiating the read transfer.

  @param src
  Specifies the sector address in the eMMC/SD device from where the data is
  to be read.
  Note: For eMMC/SD devices of greater than 2 GB in size, this address refers
  to a 512-byte sector.

  @param dest
  This parameter is a pointer to a buffer where the data to read from the
  eMMC/SD device will be stored. The buffer to which this parameter points
  must be declared with a minimum size of 512 bytes.

  @param size
  Specifies the size in bytes of the requested transfer. The value of size
  must be a multiple of 512 but not greater than (32MB - 512).

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  The following example shows how to initialize the device, perform a multi
  block read transfer using SDMA.
  @code

    #define SECT_1 0x01u
    #define BUFFER_SIZE 4096u

    mss_mmc_cfg_t g_mmc0;
    mss_mmc_status_t ret_status;
    uint8_t data_buffer[BUFFER_SIZE];
    uint32_t loop_count;

    g_mmc0.clk_rate = MSS_MMC_CLOCK_25MHZ;
    g_mmc0.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc0.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
    g_mmc0.bus_speed_mode = MSS_MMC_MODE_LEGACY;
    g_mmc.bus_voltage = MSS_MMC_3_3V_BUS_VOLTAGE;

    for (loop_count = 0; loop_count < (BUFFER_SIZE); loop_count++)
    {
        data_buffer[loop_count] = 0x45 + loop_count;
    }
    ret_status = MSS_MMC_init(&g_mmc0);
    if (MSS_MMC_INIT_SUCCESS == ret_status)
    {
        ret_status = MSS_MMC_sdma_read(SECT_1, data_buffer, BUFFER_SIZE);
        do
        {
            ret_status = MSS_MMC_get_transfer_status();
        }while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
    }
  @endcode
 */
mss_mmc_status_t
MSS_MMC_sdma_read
(
    uint32_t src,
    uint8_t *dest,
    uint32_t size
);

/*-------------------------------------------------------------------------*//**
  The MSS_MMC_adma2_write() function is used to transfer a single block or
  multiple blocks of data from the host controller to the eMMC/SD device using
  ADMA2. The size of the block of data transferred by this function must be set
  to 512 bytes or a multiple of 512 bytes. The 512 bytes is the standard sector
  size for all eMMC/SD devices with a capacity of greater than 2 GB.

  Note: A call to MSS_MMC_adma2_write() while a transfer is in progress will not
  initiate a new transfer. Use the MSS_MMC_get_transfer_status() function
  or a completion handler registered by the MSS_MMC_set_handler() function
  to check the status of the current transfer before calling the
  MSS_MMC_adma2_write() function again.

  Note: This function is a non-blocking function and returns immediately after
  initiating the write transfer.

  @param src
  This parameter is a pointer to a buffer containing the data to be stored in
  the eMMC/SD device. The buffer to which this parameter points must be
  declared with a minimum size of 512 bytes.

  @param dest
  Specifies the sector address in the eMMC/SD device where the data is
  to be stored.
  Note: For eMMC/SD devices of greater than 2 GB in size, this address refers
  to a 512-byte sector.

  @param size
  Specifies the size in bytes of the requested transfer. The value of size must
  be a multiple of 512 but not greater than (32MB - 512).

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  The following example shows how to initialize the device, perform a multi
  block transfer using ADMA2.

  @code

    #define SECT_1 0x01u
    #define BUFFER_SIZE 4096u

    mss_mmc_cfg_t g_mmc0;
    mss_mmc_status_t ret_status;
    uint8_t data_buffer[BUFFER_SIZE];
    uint32_t loop_count;

    g_mmc0.clk_rate = MSS_MMC_CLOCK_25MHZ;
    g_mmc0.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc0.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
    g_mmc0.bus_speed_mode = MSS_MMC_MODE_LEGACY;
    g_mmc.bus_voltage = MSS_MMC_3_3V_BUS_VOLTAGE;

    for (loop_count = 0; loop_count < (BUFFER_SIZE); loop_count++)
    {
        data_buffer[loop_count] = 0x45 + loop_count;
    }

    ret_status = MSS_MMC_init(&g_mmc0);
    if (MSS_MMC_INIT_SUCCESS == ret_status)
    {
        ret_status = MSS_MMC_adma2_write(data_buffer, SECT_1, BUFFER_SIZE);
        do
        {
            ret_status = MSS_MMC_get_transfer_status();
        }while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
    }
  @endcode
 */
/*mss_mmc_status_t
MSS_MMC_adma2_write
(
    const uint8_t *src,
    uint32_t dest,
    uint32_t size
); */
/*-------------------------------------------------------------------------*//**
  The MSS_MMC_adma2_read() function is used to read a single or multiple blocks
  of data from the eMMC/SD device to the host controller using ADMA2. The size
  of the block of data read by this function must be set to 512 bytes or a
  multiple of 512 bytes. The 512 bytes is the standard sector size for all
  eMMC/SD devices with a capacity of greater than 2 GB.

  Note: A call to MSS_MMC_adma2_read() while a transfer is in progress will not
  initiate a new transfer. Use the MSS_MMC_get_transfer_status() function
  or a completion handler registered by the MSS_MMC_set_handler() function
  to check the status of the current transfer before calling the
  MSS_MMC_adma2_read() function again.

  Note: This function is a non-blocking function and returns immediately after
  initiating the read transfer.

  @param src
  Specifies the sector address in the eMMC/SD device from where the data is
  to be read.
  Note: For eMMC/SD devices of greater than 2 GB in size, this address refers
  to a 512-byte sector.

  @param dest
  This parameter is a pointer to a buffer where the data to read from the
  eMMC/SD device will be stored. The buffer to which this parameter points
  must be declared with a minimum size of 512 bytes.

  @param size
  Specifies the size in bytes of the requested transfer. The value of size must
  be a multiple of 512 but not greater than (32MB -512).

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  The following example shows how to initialize the device, perform a multi
  block read transfer using ADMA2.
  @code

    #define SECT_1 0x01u
    #define BUFFER_SIZE 4096u

    mss_mmc_cfg_t g_mmc0;
    mss_mmc_status_t ret_status;
    uint8_t data_buffer[BUFFER_SIZE];
    uint32_t loop_count;

    g_mmc0.clk_rate = MSS_MMC_CLOCK_25MHZ;
    g_mmc0.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc0.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
    g_mmc0.bus_speed_mode = MSS_MMC_MODE_LEGACY;
    g_mmc.bus_voltage = MSS_MMC_3_3V_BUS_VOLTAGE;

    for (loop_count = 0; loop_count < (BUFFER_SIZE); loop_count++)
    {
        data_buffer[loop_count] = 0x45 + loop_count;
    }

    ret_status = MSS_MMC_init(&g_mmc0);
    if (MSS_MMC_INIT_SUCCESS == ret_status)
    {
        ret_status = MSS_MMC_adma2_read(SECT_1, data_buffer, BUFFER_SIZE);
        do
        {
            ret_status = MSS_MMC_get_transfer_status();
        }while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
    }
  @endcode
 */
/*mss_mmc_status_t
MSS_MMC_adma2_read
(
    uint32_t src,
    uint8_t *dest,
    uint32_t size
); */
/*-------------------------------------------------------------------------*//**
  The MSS_MMC_sdio_single_block_write() function is used to transfer a single
  block of data from the host controller to the SDIO device. The size of the
  block of data transferred by this function is 512 bytes.

  Note: This function is a blocking function and will not return until the
  write operation is successful or an error occurs.

  @param function_num
  Specifies the SDIO standard function number.

  @param src_addr
  This parameter is a pointer to a buffer containing the data to be stored in
  the SDIO device. The buffer to which this parameter points must be declared
  with a minimum size of 512 bytes.

  @param dst_addr
  Specifies the function register address in the SDIO device where the data is
  to be stored.

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  The following example shows how to initialize the SDIO device and perform
  single block transfer.

  @code

    #define BUFFER_SIZE 512u
    #define REG_NUM 0x00000001u
    #define SDIO_FUN_NUM 0x00000001u

    mss_mmc_cfg_t g_mmc0;
    mss_mmc_status_t ret_status;

    uint8_t data_buffer[BUFFER_SIZE];
    uint32_t loop_count;

    g_mmc0.clk_rate = MSS_MMC_CLOCK_12MHZ;
    g_mmc0.card_type = MSS_MMC_CARD_TYPE_SDIO;
    g_mmc0.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
    g_mmc0.bus_speed_mode = MSS_SDCARD_MODE_DEFAULT_SPEED;

    for (loop_count = 0; loop_count < (BUFFER_SIZE); loop_count++)
    {
        data_buffer[loop_count] = 0x45 + loop_count;
    }

    ret_status = MSS_MMC_init(&g_mmc0);
    if (MSS_MMC_INIT_SUCCESS == ret_status)
    {
        ret_status = MSS_MMC_sdio_single_block_write(SDIO_FUN_NUM, data_buffer,
                                                                     REG_NUM);
        if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
        {
            //...
        }
    }
  @endcode
 */
/*mss_mmc_status_t
MSS_MMC_sdio_single_block_write
(
    uint8_t function_num,
    const uint32_t * src_addr,
    uint32_t dst_addr
);*/

/*-------------------------------------------------------------------------*//**
  The MSS_MMC_sdio_single_block_read() function is used to read a single block
  of data from the the SDIO device to host controller. The size of the block of
  data transferred by this function is set to 512 bytes.
  Note: This function is a blocking function and will not return until the
  write operation is successful or an error occurs.

  @param function_num
  Specifies the SDIO standard function number.

  @param src_addr
  Specifies the SDIO function number space register address in the SDIO device
  from where the 512 bytes block of data will be read.

  @param dst_addr
  This parameter is a pointer to a buffer where the data read from the SDIO
  device will be stored. The buffer to which this parameter points must be
  declared with a minimum size of 512 bytes.

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  The following example shows how to initialize the SDIO device and perform
  single block transfer.

  @code

    #define BUFFER_SIZE 512u
    #define REG_NUM 0x00000001u
    #define SDIO_FUN_NUM 0x00000001u

    mss_mmc_cfg_t g_mmc0;
    mss_mmc_status_t ret_status;
    uint8_t data_buffer[BUFFER_SIZE];

    g_mmc0.clk_rate = MSS_MMC_CLOCK_12MHZ;
    g_mmc0.card_type = MSS_MMC_CARD_TYPE_SDIO;
    g_mmc0.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
    g_mmc0.bus_speed_mode = MSS_SDCARD_MODE_DEFAULT_SPEED;

    ret_status = MSS_MMC_init(&g_mmc0);
    if (MSS_MMC_INIT_SUCCESS == ret_status)
    {
        ret_status = MSS_MMC_sdio_single_block_read(SDIO_FUN_NUM, REG_NUM,
                                                                 data_buffer);
        if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
        {
            //...
        }
    }
  @endcode
 */
/*mss_mmc_status_t
MSS_MMC_sdio_single_block_read
(
    uint8_t function_num,
    uint32_t src_addr,
    uint8_t *dst_addr
);*/

/*-------------------------------------------------------------------------*//**
  The MSS_MMC_get_transfer_status() function returns the status of the MMC
  transfer initiated by a call to MSS_MMC_sdma_write(), MSS_MMC_sdma_read(),
  MSS_MMC_adma2_write(), MSS_MMC_adma2_read(), MSS_MMC_cq_write(),
  MSS_MMC_cq_read() functions.

  @param
    This function has no parameters.

  @return
  This function returns a value of type mss_mmc_status_t. The possible return
  values are:
        - MSS_MMC_TRANSFER_IN_PROGRESS
        - MSS_MMC_TRANSFER_SUCCESS
        - MSS_MMC_TRANSFER_FAIL
        - MSS_MMC_RESPONSE_ERROR

  Example:
  The following example shows the use of MSS_MMC_get_transfer_status() function.

  @code

        mss_mmc_status_t ret_status;
        ret_status = MSS_MMC_write(data_buffer, SECT_1, BUFFER_SIZE);
        do
        {
            ret_status = MSS_MMC_get_transfer_status();
        }while(ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)

  @endcode
 */
mss_mmc_status_t MSS_MMC_get_transfer_status(void);

/*-------------------------------------------------------------------------*//**
  The MSS_MMC_set_handler() function registers a handler function that will be
  called by the driver when a read o write transfer completes. The application
  must create and register a transfer completion handler function. The MSS
  eMMC SD driver passes the outcome of the transfer to the completion handler
  in the form of a status (SRS12 register) parameter indicating if the transfer
  is successful or the type of error that occurred during the transfer if the
  transfer failed.

  @param handler
  The handler parameter is a pointer to a handler function provided by the
  application. This handler is of type mss_mmc_handler_t. The handler function
  must take one parameter of type uint32_t and must not return a value.

  @return
    This function does not return a value.

  Example:
  The following example shows the use of MSS_MMC_set_handler() function.

  @code

    #define BLOCK_1 0x00000001u
    #define BUFFER_SIZE 1024
    #define ERROR_INTERRUPT 0x8000
    #define TRANSFER_COMPLETE 0x1

    void transfer_complete_handler(uint32_t srs12_status);
    volatile uint32_t g_xfer_in_progress = 0;

    mss_mmc_cfg_t g_mmc0;
    mss_mmc_status_t ret_status;
    uint8_t data_buffer[BUFFER_SIZE];
    uint32_t loop_count;

    g_mmc0.clk_rate = MSS_MMC_CLOCK_25MHZ;
    g_mmc0.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc0.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
    g_mmc0.bus_speed_mode = MSS_MMC_MODE_LEGACY;

    for (loop_count = 0; loop_count < (BUFFER_SIZE); loop_count++)
    {
        data_buffer[loop_count] = 0x45 + loop_count;
    }

    ret_status = MMC_init(&g_mmc0);
    if (MSS_MMC_INIT_SUCCESS == ret_status)
    {
        MSS_MMC_set_handler(transfer_complete_handler);
        ret_status = MSS_MMC_adma2_write(data_buffer, BLOCK_1, BUFFER_SIZE);
        if (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
        {
            while(g_xfer_in_progress)
            {
                ;
            }
        }
    }

    void transfer_complete_handler(uint32_t srs12_status)
    {
        g_xfer_in_progress = 0;
        uint32_t isr_err;
        if(ERROR_INTERRUPT & srs12_status)
        {
            isr_err = srs12_status >> 16;
        }
        else if(TRANSFER_COMPLETE & srs12_status)
        {
            isr_err = 0;
        }
        else
        {
        }
    }
  @endcode
 */
void MSS_MMC_set_handler(mss_mmc_handler_t handler);

/*-------------------------------------------------------------------------*//**
  The MSS_MMC_cq_init() function enables command queue in the eMMC device and
  in the host controller. The command queue allows the application to queue
  multiple read or write tasks.

  Note: The MSS_MMC_init() must be configured for eMMC mode before using the
  MSS_MMC_cq_init() function.

  @param
    This function has no parameters.

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.
 */
/*mss_mmc_status_t MSS_MMC_cq_init(void); */

/*-------------------------------------------------------------------------*//**
  The MSS_MMC_cq_write() function is used to transmit a single block or multiple
  blocks of data from the host controller to the eMMC device using command queue
  with single or multiple tasks based on the data size. The size of the block of
  data transferred by this function must be set to 512 bytes or a multiple of
  512 bytes. The 512 bytes is the standard sector size for all eMMC
  devices with a capacity of greater than 2 GB.

  Note: A call to MSS_MMC_cq_write() while a transfer is in progress will not
  initiate a new transfer. Use the MSS_MMC_get_transfer_status() function
  or a completion handler registered by the MSS_MMC_set_handler() function
  to check the status of the current transfer before calling the
  MSS_MMC_cq_write() function again.

  Note: This function is a non-blocking function and returns immediately after
  initiating the write transfer.

  @param src
  This parameter is a pointer to a buffer containing the data to be stored
  in the eMMC device. The buffer to which this parameter points must be
  declared with a minimum size of 512 bytes.

  @param dest
  Specifies the sector address in the eMMC device where the data is
  to be stored.
  Note: For eMMC devices of greater than 2 GB in size, this address refers to a
  512 byte sector.

  @param size
  Specifies the size in bytes of the requested transfer. The value of size must
  be a multiple of 512 but not greater than (1GB - 16KB).

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  The following example shows how to initialize the device and perform a multi
  block transfer..
  @code

    #define SECT_1 0x01u
    #define BUFFER_SIZE 4096u

    mss_mmc_cfg_t g_mmc0;
    mss_mmc_status_t ret_status;
    uint8_t data_buffer[BUFFER_SIZE];
    uint32_t loop_count;

    g_mmc0.clk_rate = MSS_MMC_CLOCK_25MHZ;
    g_mmc0.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc0.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
    g_mmc0.bus_speed_mode = MSS_MMC_MODE_LEGACY;

    for (loop_count = 0; loop_count < (BUFFER_SIZE); loop_count++)
    {
        data_buffer[loop_count] = 0x45 + loop_count;
    }

    ret_status = MSS_MMC_init(&g_mmc0);
    if (MSS_MMC_INIT_SUCCESS == ret_status)
    {
        ret_status = MSS_MMC_cq_init();
        if ( MSS_MMC_INIT_SUCCESS == ret_status)
        {
            ret_status = MSS_MMC_cq_write(data_buffer, SECT_1, BUFFER_SIZE);
            do
            {
                ret_status = MSS_MMC_get_transfer_status();
            }while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
        }
    }
  @endcode
 */
/*mss_mmc_status_t
MSS_MMC_cq_write
(
    const uint8_t *src,
    uint32_t dest,
    uint32_t size
);*/
/*-------------------------------------------------------------------------*//**
  The MSS_MMC_cq_read() function is used to read a single block or multiple
  blocks of data from the eMMC device to the host controller using command queue
  with single or multiple tasks based on the data size. The size of the block of
  data read by this function must be set to 512 bytes or a multiple of 512 bytes.
  The 512 bytes is the standard sector size for all eMMC devices with a capacity
  of greater than 2 GB.

  Note: A call to MSS_MMC_cq_read() while a transfer is in progress will not
  initiate a new transfer. Use the MSS_MMC_get_transfer_status() function
  or a completion handler registered by the MSS_MMC_set_handler() function
  to check the status of the current transfer before calling the
  MSS_MMC_cq_read() function again.

  Note: This function is a non-blocking function and returns immediately after
  initiating the write transfer.

  @param src_addr
  Specifies the sector address in the eMMC device from where the datato is
  to be read.

  Note: For eMMC devices of greater than 2 GB in size, this address refers
  to a 512-byte sector.

  @param dst_addr
  This parameter is a pointer to a buffer where the data to read from the eMMC
  device will be stored. The buffer to which this parameter points must be
  declared with a minimum size of 512 bytes.

  @param size
  Specifies the size in bytes of the requested transfer. The value of size must
  be a multiple of 512 but not greater than (1GB - 16KB).

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  The following example shows how to initialize the device and perform a multi
  block transfer.

  @code

    #define SECT_1 0x01u
    #define BUFFER_SIZE 4096u

    mss_mmc_cfg_t g_mmc0;
    mss_mmc_status_t ret_status;
    uint8_t data_buffer[BUFFER_SIZE];
    uint32_t loop_count;

    g_mmc0.clk_rate = MSS_MMC_CLOCK_25MHZ;
    g_mmc0.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc0.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
    g_mmc0.bus_speed_mode = MSS_MMC_MODE_LEGACY;

    for (loop_count = 0; loop_count < (BUFFER_SIZE); loop_count++)
    {
        data_buffer[loop_count] = 0x45 + loop_count;
    }

    ret_status = MSS_MMC_init(&g_mmc0);
    if (MSS_MMC_INIT_SUCCESS == ret_status)
    {
        ret_status = MSS_MMC_cq_init();
        if (MSS_MMC_INIT_SUCCESS == ret_status)
        {
            ret_status = MSS_MMC_cq_read(SECT_1, data_buffer, BUFFER_SIZE);
            do
            {
                ret_status = MSS_MMC_get_transfer_status();
            }while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
        }
    }
  @endcode
 */
/*mss_mmc_status_t
MSS_MMC_cq_read
(
    uint32_t src,
    uint8_t *dest,
    uint32_t size
);*/

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_MMC_H */
