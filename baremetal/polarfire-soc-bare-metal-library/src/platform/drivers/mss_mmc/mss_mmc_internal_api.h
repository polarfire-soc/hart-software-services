/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC MSS eMMC SD driver API's for internal use cases.
 *
 * SVN $Revision: 12619 $
 * SVN $Date: 2019-12-09 12:07:14 +0530 (Mon, 09 Dec 2019) $
 */
/*=========================================================================*//**
  Note: The MSS_MMC_pause_sdma_write_hpi(), MSS_MMC_resume_sdma_write_hpi(),
  MSS_MMC_packed_read(), MSS_MMC_packed_write(), MSS_MMC_cq_single_task_write(),
  MSS_MMC_cq_single_task_read()functions provided purely for SVG use cases.

  --------------------------------
  High Priority Interrupt
  --------------------------------
  The following functions are used for eMMC high priority interrupt operation:
    - MSS_MMC_pause_sdma_write_hpi()
    - MSS_MMC_resume_sdma_write_hpi()

  To stop ongoing multiple block write transfers to the eMMC device using the
  high priority interrupt, a call is made to the MSS_MMC_pause_sdma_write_hpi()
  function.

  To resume previously interrupted multiple block transfer of eMMC device,
  a call is made to the MSS_MMC_resume_sdma_write_hpi() function.

  --------------------------------
  Packed Commands
  --------------------------------
  The following functions are used for eMMC packed command read and write:
    - MSS_MMC_packed_read()
    - MSS_MMC_packed_write()

  To read several multiple blocks of data stored within the eMMC device using
  the packed group of read commands, a call is made to the MSS_MMC_packed_read()
  function, specifying the base address of the buffer holding the data of the
  packed command header block and the base address of buffer where the data
  read from the eMMC device will be stored.

  To write several multiple blocks of data to the eMMC device using the packed
  group of write commands, a call is made to the MSS_MMC_packed_write()
  function, specifying the base address of the buffer holding the data of the
  packed command header block and the base address of buffer containing the
  data blocks to be stored into the eMMC device.

  --------------------------------
  Command Queue
  --------------------------------
  The following functions are used for eMMC command queue single task operation:
    - MSS_MMC_cq_single_task_write()
    - MSS_MMC_cq_single_task_read()

  To write a single block or multiple blocks of data to the eMMC device using
  a command queue, a call is made to the MSS_MMC_cq_single_task_write()
  function. This function supports a single task only.

  To read a single block or multiple blocks of data stored within the eMMC
  device using a command queue, a call is made to the
  MSS_MMC_cq_single_task_read() function. This function supports a single task
  only.

 *//*=========================================================================*/
#ifndef __MSS_MMC_INTERNAL_API_H
#define __MSS_MMC_INTERNAL_API_H

#ifdef __cplusplus
extern "C"
#endif

#include "hal/cpu_types.h"

/*-------------------------------------------------------------------------*//**
  The MSS_MMC_pause_sdma_write_hpi() function is used to pause the ongoing SDMA
  write transfer using the eMMC high priority interrupt (HPI).

  Note: This MSS_MMC_pause_sdma_write_hpi()function has parameters provided
  purely for svg use case. if this function in production release then we
  should remove the need for parameters.

  @param src
  This parameter is a pointer to a buffer containing actual the data to be
  stored in the eMMC device. The src parameter must be identical to the src
  parameter of the MSS_MMC_sdma_write() function.

  @param dest
  This parameter specifies the sector address in the eMMC device where the block
  is to be stored. The dest parameter must be identical to the dest parameter of
  the MSS_MMC_sdma_write() function.

  @param size
  The parameter size specifies the size in bytes of the requested transfer.
  The size parameter must be identical to the size parameter of the
  MSS_MMC_sdma_write() function.

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  The following example shows how to initialize the device and perform an HPI

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
        ret_status = MSS_MMC_sdma_write(data_buffer, SECT_1, BUFFER_SIZE);
        if(ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
        {
            ret_status = MSS_MMC_pause_sdma_write_hpi(data_buffer, SECT_1,
                                                                 BUFFER_SIZE);
            if(ret_status == MSS_MMC_TRANSFER_SUCCESS)
            {
                //..
            }
    }
    }
  @endcode
 */
mss_mmc_status_t
MSS_MMC_pause_sdma_write_hpi
(
    const uint8_t *src,
    uint32_t dest,
    uint32_t size
);

/*-------------------------------------------------------------------------*//**
  The MSS_MMC_resume_sdma_write_hpi() function is used to resume writing the
  remaining blocks to the target device which was previously interrupted by
  a call to MSS_MMC_pause_sdma_write_hpi().

  @param
    This function has no parameters.

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  The following example shows how to initialize the device and perform a HPI
  interrupt and resume remaining block operation.

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
        ret_status = MSS_MMC_sdma_write(data_buffer, SECT_1, BUFFER_SIZE);
        if(ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
        {
            ret_status = MSS_MMC_pause_sdma_write_hpi(data_buffer, SECT_1,
                                                                 BUFFER_SIZE);
            if(ret_status == MSS_MMC_TRANSFER_SUCCESS)
            {
                ret_status = MSS_MMC_resume_sdma_write_hpi();
                if(ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
                {
                    do
                    {
                        ret_status = MSS_MMC_get_transfer_status();
                    }while(ret_status == MSS_MMC_TRANSFER_IN_PROGRESS);
                }
            }
        }
    }
  @endcode
 */
mss_mmc_status_t MSS_MMC_resume_sdma_write_hpi(void);

/*-------------------------------------------------------------------------*//**
  The MSS_MMC_packed_write() function is used to transmit a packed group of data
  from the host to the eMMC device. The write commands can be packed in a group
  of commands (all write) that transfer the data for all commands in the group
  in one transfer on the bus.
  Note : This function is a non-blocking function and returns immediately after
  initiating the write transfer.

  @param src
  This parameter is a pointer to a buffer containing the data blocks to be
  stored in the eMMC device. The first block containing the packed-command
  header and all data sectors of the individual packed commands are appended
  together after the header.

  @param dest
  Specifies the sector address in the eMMC device where the block is to be
  stored. The dest shall be the same address that is specified by the first
  individual write command in the packed group.

  @param size
  Specifies the size in bytes of the requested transfer. The value of size must
  be a multiple of 512. The size parameter shall be the sum of all block counts
  of the individual writes plus one for the header.

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  This example shows how to initialize the device and perform a packed write
  transfer.
  @code

    #define SECT_9 0x09u
    #define BUFFER_SIZE 4096u

    mss_mmc_cfg_t g_mmc0;
    mss_mmc_status_t ret_status;
    uint8_t data_buffer[BUFFER_SIZE];
    uint32_t loop_count;
    uint8_t packed_write[512];

    g_mmc0.clk_rate = MSS_MMC_CLOCK_25MHZ;
    g_mmc0.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc0.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
    g_mmc0.bus_speed_mode = MSS_MMC_MODE_LEGACY;

    packed_write[0] = 0x01; // version
    packed_write[1] = 0x02; // 0x2 - write
    packed_write[2] = 0x02; // no of entries
    packed_write[3] = 0x00;
    packed_write[4] = 0x00;
    packed_write[5] = 0x00;
    packed_write[6] = 0x00;
    packed_write[7] = 0x00;
    packed_write[8] = 0x04; // CMD23 arg1 - 4 blocks
    packed_write[9] = 0x00;
    packed_write[10] = 0x00;
    packed_write[11] = 0x00;
    packed_write[12] = 0x09; // CMD25 arg1 - sector no 9
    packed_write[13] = 0x00;
    packed_write[14] = 0x00;
    packed_write[15] = 0x00;
    packed_write[16] = 0x04; // CMD23 arg2 - 4-blocks
    packed_write[17] = 0x00;
    packed_write[18] = 0x00;
    packed_write[19] = 0x00;
    packed_write[20] = 0x29; // CMD25 agr2 - sector no 0x29
    packed_write[21] = 0x00;
    packed_write[22] = 0x00;
    packed_write[23] = 0x00;

    for (loop_count = 24; loop_count < 512; loop_count++)
    {
        packed_write[loop_count] = 0;
    }
    for (loop_count = 0; loop_count < (BUFFER_SIZE); loop_count++)
    {
        data_buffer[loop_count] = 0x45 + loop_count;
    }
    // packed header block
    for (loop_count = 0; loop_count < 512; loop_count++)
    {
        data_buffer[loop_count] = packed_write[loop_count];
    }
    resp_reg = MMC_init(&g_mmc0);
    if (MSS_MMC_INIT_SUCCESS == ret_status)
    {
        ret_status = MSS_MMC_packed_write(data_buffer, SECT_9, BUFFER_SIZE);
        if (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
        {
            do
            {
                ret_status = MSS_MMC_get_transfer_status();
            }while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
        }
    }
  @endcode
 */
mss_mmc_status_t
MSS_MMC_packed_write
(
    const uint8_t *src,
    uint32_t dest,
    uint32_t size
);

/*-------------------------------------------------------------------------*//**
  The MSS_MMC_packed_read() function is used to read several multiple block of
  data from the device to the host using packed group. The read commands can be
  packed in group of commands (all read) that transfer the data for all commands
  in the group in one transfer on the bus.
  Note : This function is a non-blocking function and returns immediately after
  initiating the block transfer.

  @param src
  Specifies the sector address in the eMMC device where the block is to be read.
  The src shall be the same address that is specified by the first individual
  read command in the packed group.

  @param dest
  This parameter is a pointer to a buffer where the data read from the eMMC
  device will be stored. The buffer to which this parameter points should be
  declared with a minimum size of 512 bytes.

  @param packed_header
  This parameter is a pointer to a buffer containing the packed-command header
  block.

  @param size
  Specifies the size in bytes of the requested transfer. The value of size must
  be a multiple of 512. The size parameter shall be the sum of all block counts
  of the individual read.

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  This example shows how to initialize the device and perform a packed read
  transfer.

  @code

    #define SECT_9 0x09u
    #define BUFFER_SIZE 4096u

    mss_mmc_cfg_t g_mmc0;
    mss_mmc_status_t ret_status;
    uint8_t data_buffer[BUFFER_SIZE];
    uint32_t loop_count;
    uint8_t packed_read[512];

    g_mmc0.clk_rate = MSS_MMC_CLOCK_25MHZ;
    g_mmc0.card_type = MSS_MMC_CARD_TYPE_MMC;
    g_mmc0.data_bus_width = MSS_MMC_DATA_WIDTH_4BIT;
    g_mmc0.bus_speed_mode = MSS_MMC_MODE_LEGACY;

    packed_read[0] = 0x01; // version
    packed_read[1] = 0x02; // 0x1 - read
    packed_read[2] = 0x02; // no of entries
    packed_read[3] = 0x00;
    packed_read[4] = 0x00;
    packed_read[5] = 0x00;
    packed_read[6] = 0x00;
    packed_read[7] = 0x00;
    packed_read[8] = 0x04; // CMD23 arg1 - 4 blocks
    packed_read[9] = 0x00;
    packed_read[10] = 0x00;
    packed_read[11] = 0x00;
    packed_read[12] = 0x09; // CMD18 arg1 - sector no 9
    packed_read[13] = 0x00;
    packed_read[14] = 0x00;
    packed_read[15] = 0x00;
    packed_read[16] = 0x04; // CMD23 arg2 - 4-blocks
    packed_read[17] = 0x00;
    packed_read[18] = 0x00;
    packed_read[19] = 0x00;
    packed_read[20] = 0x29; // CMD18 agr2 - sector no 0x29
    packed_read[21] = 0x00;
    packed_read[22] = 0x00;
    packed_read[23] = 0x00;
    for(loop_count = 24; loop_count < 512; loop_count++)
    {
        packed_read[loop_count] = 0;
    }
    for(loop_count = 0; loop_count < (BUFFER_SIZE); loop_count++)
    {
        data_buffer[loop_count] = = 0x00;
    }
    resp_reg = MMC_init(&g_mmc0);
    if (MSS_MMC_INIT_SUCCESS == ret_status)
    {
        ret_status = MSS_MMC_packed_read(SECT_9, data_buffer,
                                (uint32 *)packed_read, BUFFER_SIZE);
        if (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
        {
            do
            {
                ret_status = MSS_MMC_get_transfer_status();
            }while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
        }
    }

  @endcode
 */
mss_mmc_status_t
MSS_MMC_packed_read
(
    uint32_t src,
    uint8_t *dest,
    uint32_t *packed_header,
    uint32_t size
);
/*-------------------------------------------------------------------------*//**
  The MSS_MMC_cq_single_task_write() function is used to transmit a single block
  or multiple blocks of data from the host controller to the eMMC device using
  command queue with single task based on the data size. The size of the block
  of data transferred by this function must be set to 512 bytes or a multiple of
  512 bytes. The 512 bytes is the standard sector size for all eMMC
  devices with a capacity of greater than 2 GB.

  Note: A call to MSS_MMC_cq_single_task_write() while a transfer is in
  progress will not initiate a new transfer. Use the MSS_MMC_get_transfer_status()
  function or a completion handler registered by the MSS_MMC_set_handler()
  function to check the status of the current transfer before calling the
  MSS_MMC_cq_single_task_write() function again.

  Note: This function is a non-blocking function and returns immediately after
  initiating the write transfer.

  Note: This MSS_MMC_cq_single_task_write() and MSS_MMC_cq_single_task_read()
  functions are provided purely for svg use case. For production release we
  will remove.

  @param src
  This parameter is a pointer to a buffer containing the data to be stored
  in the eMMC device. The buffer to which this parameter points must be
  declared with a minimum size of 512 bytes.

  @param dest
  Specifies the sector address in the eMMC device where the data is
  to be stored.
  Note: For eMMC devices of greater than 2 GB in size, this address refers to a
  512 byte sector.

  @param task_id
  Specifies the eMMC command queue task id number 0 to 31

  @param size
  Specifies the size in bytes of the requested transfer. The value of size must
  be a multiple of 512 but not greater than (32MB - 512).

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  The following example shows how to initialize the device and perform a multi
  block transfer..
  @code

    #define SECT_1 0x01u
    #define BUFFER_SIZE 4096u
    #define TASK_ID 0x01

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
            ret_status = MSS_MMC_cq_single_task_write(data_buffer, SECT_1,
                                                         TASK_ID, BUFFER_SIZE);
            do
            {
                ret_status = MSS_MMC_get_transfer_status();
            }while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
        }
    }
  @endcode
 */
mss_mmc_status_t
MSS_MMC_cq_single_task_write
(
    const uint8_t *src,
    uint32_t dest,
    uint8_t task_id,
    uint32_t size
);
/*-------------------------------------------------------------------------*//**
  The MSS_MMC_cq_single_task_read() function is used to read a single or
  multiple blocks of data from the eMMC device to the host controller using
  command queue with single task based on the data size. The size of the block
  of data read by this function must be set to 512 bytes or a multiple of
  512 bytes. The 512 bytes is the standard sector size for all eMMC devices
  with a capacity of greater than 2 GB.

  Note: A call to MSS_MMC_cq_single_task_read() while a transfer is in progress
  will not initiate a new transfer. Use the MSS_MMC_get_transfer_status()
  function or a completion handler registered by the MSS_MMC_set_handler()
  function to check the status of the current transfer before calling the
  MSS_MMC_cq_single_task_read() function again.

  Note: This function is a non-blocking function and returns immediately after
  initiating the write transfer.

  @param src
  Specifies the sector address in the eMMC device from where the data is
  to be read.
  Note: For eMMC devices of greater than 2 GB in size, this address refers
  to a 512-byte sector.

  @param dest
  This parameter is a pointer to a buffer where the data to read from the eMMC
  device will be stored. The buffer to which this parameter points must be
  declared with a minimum size of 512 bytes.

  @param task_id
  Specifies the eMMC command queue task id number 0 to 31

  @param size
  Specifies the size in bytes of the requested transfer. The value of size must
  be a multiple of 512 but not greater than (32MB - 512).

  @return
  This function returns a value of type mss_mmc_status_t which specifies the
  transfer status of the operation.

  Example:
  The following example shows how to initialize the device and perform a multi
  block transfer.

  @code

    #define SECT_1 0x01u
    #define BUFFER_SIZE 4096u
    #define TASK_ID 0x02

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
            ret_status = MSS_MMC_cq_single_task_read(SECT_1, data_buffer,
                                                         TASK_ID, BUFFER_SIZE);
            do
            {
                ret_status = MSS_MMC_get_transfer_status();
            }while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS)
        }
    }
  @endcode
 */
mss_mmc_status_t
MSS_MMC_cq_single_task_read
(
    uint32_t src,
    uint8_t *dest,
    uint8_t task_id,
    uint32_t size
);

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_MMC_INTERNAL_API_H */
