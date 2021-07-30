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
 * PolarFire SoC Microprocessor subsystem PDMA bare metal software driver public
 * APIs.
 *
 * SVN $Revision$
 * SVN $Date$
 */

/*=========================================================================*//**
  @mainpage PolarFire SoC MSS PDMA Bare Metal Driver.

  ==============================================================================
  Introduction
  ==============================================================================
  The PolarFire SoC Microprocessor Subsystem (MSS) includes the MSS PDMA
  peripherals for autonomous data transfers. The MSS PDMA has memory-mapped
  control registers accessed over a TileLink slave interface to allow software
  to set up DMA transfers, and has a TileLink bus master port into the TileLink
  bus fabric to allow it to rapidly copy data between two locations in memory.
  The MSS PDMA  can support multiple independent simultaneous DMA transfers
  using different PDMA channels and can generate PLIC interrupts on either a
  transfer has completed, or when a transfer error has occurred.

  ==============================================================================
  Hardware Flow Dependencies
  ==============================================================================
  The configuration of all features of the MSS PDMA driver is covered by this
  driver. There are no dependencies on the hardware flow when configuring MSS
  PDMA.
  The source and destination of the DMA transaction must be enabled, either in
  the hardware or by your application program. The source and destination
  addresses must be located at valid addresses in the PolarFire SoC MSS memory
  map and must be appropriate to the configuration that you specify the PDMA
  channel.
  The base address, register addresses, and interrupt numbers associated with
  MSS PDMA block are defined in the mpfs hal as constants. User must ensure that
  the latest mpfs hal is included in the example project.

  ==============================================================================
  Theory of Operation
  ==============================================================================
  The PDMA driver function are grouped into the following categories:
  - Initialization
  - Configuration
  - Transfer Control
  - Interrupt Control

  --------------------------------
  Initialization
  --------------------------------
  The PolarFire SoC MSS PDMA driver is initialized through the call to the
  MSS_PDMA_init() function. The MSS_PDMA_init() function must be called before
  any other MSS PDMA driver functions.

  --------------------------------
  Configuration
  --------------------------------
  Each PDMA channel is configured through the call to MSS_PDMA_setup_transfer()
  function. Configuration includes :
  -  The PDMA channel for transaction.
  -  Source and destination addresses.
  -  The transfer sizes.
  -  Repeat the transfer
  -  Ordering requirements.
  -  Set / Clear the interrupt for the transfer

  The PolarFire SoC MSS PDMA has four independent DMA channels which operate
  concurrently to support multiple simultaneous transfers. Each channel has an
  independent set of registers and interrupts. The PDMA channels can  be
  configured through the call to MSS_PDMA_setup_transfer() function. Apart from
  selecting the channel the MSS_PDMA_setup_transfer() function will also setup
  the transfer size, source and destination addresses.
  MSS_PDMA_setup_transfer() function will also configure the repeat and force
  order requirements as the PoalrFire SoC MSS PDMA supports multiple
  simultaneous transfers.
  The PDMA has two interrupts per channel which are used to signal when either
  a transfer has completed or when a transfer error has occurred. These
  interrupts are enabled by the application software and are set / cleared
  by MSS_PDMA_setup_transfer() function.

  --------------------------------
  Transfer Control
  --------------------------------
  The PDMA transfers can be initiated by a call to the MSS_PDMA_start_transfer()
  function after a PDMA channel has been configured. The
  MSS_PDMA_start_transfer() function will write the config register in PDMA
  memory map. The write_size and read_size buffers are used to determine the
  size and alignment of individual PDMA transactions as a single PDMA transfers
  may require multiple transactions.
  The configuration of the currently selected PDMA channel can be read by
  calling the MSS_PDMA_get_active_transfer_type() function.

  --------------------------------
  Interrupt Control
  --------------------------------
  The PDMA has two interrupts per channel which are used to signal either a
  successful completion og the transfer, a transfer error. These interrupts are
  enabled by the application software by a call to MSS_PDMA_setup_transfer()
  function. The  MSS_PDMA_get_transfer_complete_status() function will indicate
  the DMA transfer success status for selected DMA channel. The
  MSS_PDMA_get_transfer_error_status() function will indicate the DMA transfer
  failure status for selected DMA channel. The
  MSS_PDMA_clear_transfer_complete_status() function can be used to clear the
  DMA transfer success status.
  The MSS_PDMA_clear_transfer_error_status() function can be used to clear the
  DMA transfer error status.

*//*==========================================================================*/
#ifndef MSS_PDMA_H
#define MSS_PDMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*---------------------------Public Data Structure----------------------------*/
/*----------------------------------PDMA--------------------------------------*/

/*----------------------------------------------------------------------------*//*
  The mss_pdma_channel_id_t enumeration is used to identify peripheral DMA channels.
  It is used as function parameter to specify the PDMA channel used.
 */
typedef enum __pdma_channel_id
{
    MSS_PDMA_CHANNEL_0 = 0,
    MSS_PDMA_CHANNEL_1,
    MSS_PDMA_CHANNEL_2,
    MSS_PDMA_CHANNEL_3,
    MSS_PDMA_lAST_CHANNEL,
} mss_pdma_channel_id_t;

/*-------------------------------------------------------------------------*//**
  The mss_pdma_channel_config_t structure is used to configure the desired
  Platform DMA channel. PDMA channel configuration includes configuration of
  source, destination address and the number of bytes for the DMA transfer.
  Single DMA transfer may require multiple DMA transactions, the size and
  alignment of the individual DMA transaction can be forced. The PDMA can be
  programmed to automatically repeat a transfer.
  The MSS PDMA have two interrupts per channel, transfer complete interrupt and
  transfer error interrupt. The two interrupts can be enabled by configuring
  respective bits in the mss_pdma_channel_config_t structure.
 */
typedef struct _pdmachannelconfig
{
    volatile uint64_t src_addr;            /* source address */
    volatile uint64_t dest_addr;           /* destination address */
    volatile uint64_t num_bytes;           /* Number of bytes to be transfered.
                                            * Base 2 Logarithm */
    volatile uint8_t enable_done_int;      /* enable transfer complete interrupt */
    volatile uint8_t enable_err_int;       /* enable transfer error interrupt*/
    volatile uint8_t repeat;               /* repeat the transaction */
    volatile uint8_t force_order;          /* Enforces strict ordering by only
                                              allowing one of each transfer type
                                              in-flight at a time */
} mss_pdma_channel_config_t;

/*------------------------Private data structures-----------------------------*/
/*----------------------------------- PDMA -----------------------------------*/

/*------------------------------------------------------------------------*//**
 * The mss_pdma_error_id_t enumeration is used to specify the error status from
 * MSS PDMA transfer / transaction functions.
 */
typedef enum __pdma_error_id_t
{
    MSS_PDMA_OK = 0,                       //!< PDMA_OK
    MSS_PDMA_ERROR_INVALID_SRC_ADDR,       //!< ERROR_INVALID_SRC_ADDR
    MSS_PDMA_ERROR_INVALID_DEST_ADDR,      //!< ERROR_INVALID_DEST_ADDR
    MSS_PDMA_ERROR_TRANSACTION_IN_PROGRESS,//!< ERROR_TRANSACTION_IN_PROGRESS
    MSS_PDMA_ERROR_INVALID_CHANNEL_ID,     //!< ERROR_INVALID_CHANNEL_ID
    MSS_PDMA_ERROR_INVALID_NEXTCFG_WSIZE,  //!< ERROR_INVALID_NEXTCFG_WSIZE
    MSS_PDMA_ERROR_INVALID_NEXTCFG_RSIZE,  //!< ERROR_INVALID_NEXTCFG_RSIZE
    MSS_PDMA_ERROR_LAST_ID,                //!< ERROR_LAST_ID
} mss_pdma_error_id_t;

/*-------------------------------------------------------------------------*//**
 * The mss_pdma_t structure will describe the functionality of the memory mapped
 * registers in the Platform DMA Engine.
 */
typedef struct _pdmaregs
{
    volatile uint32_t control_reg;           /* Channel Control Register */
    volatile uint32_t next_config;           /* Next transfer type */
    volatile uint64_t next_bytes;            /* Number of bytes to be transfered.
                                              * Base 2 Logarithm
                                              */
    volatile uint64_t next_destination;       /* Destination Start Address */
    volatile uint64_t next_source;            /* Source start Address*/
    const volatile  uint32_t exec_config;     /* Active transfer type */
    const volatile  uint64_t exec_bytes;      /* Number of bytes remaining. */
    const volatile  uint64_t exec_destination;/* Destination current address. */
    const volatile  uint64_t exec_source;     /* Source current address. */
} mss_pdma_t;

/* PDMA Register base address. */
#define PDMA_REG_BASE                                  0x03000000U

/* PDMA Channel Register offset.*/
#define PDMA_CHL_REG_OFFSET                            0x1000U

#define MASK_PDMA_CONTROL_RUN                          0x02U

#define MASK_CLAIM_PDMA_CHANNEL                        0x01U

#define MASK_PDMA_ENABLE_DONE_INT                      0x00004000U
#define MASK_PDMA_ENABLE_ERR_INT                       0x00008000U

#define SHIFT_CH_CONFIG_WSIZE                          24U
#define SHIFT_CH_CONFIG_RSIZE                          28U

#define MASK_MAXIUM_WSIZE                              0x0F000000U
#define MASK_MAXIUM_RSIZE                              0xF0000000U

#define MASK_REPEAT_TRANSCTION                         0x04U
#define MASK_FORCE_ORDERING                            0x08U

#define MASK_PDMA_TRANSFER_ERROR                       0x80000000U
#define MASK_PDMA_TRANSFER_DONE                        0x40000000U

/*--------------------------------Public APIs---------------------------------*/

/*-------------------------------------------------------------------------*//**
  The MSS_PDMA_init() function is used to initialize the PolarFire SoC MSS
  Platform DMA engine. It initializes the basic data structures required for
  driver functionality.
   @param
           This function does not need any parameters.

   @return
           This function does not return value.
  @endcode
 */
void
MSS_PDMA_init
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_PDMA_setup_transfer() function is used to configure an individual
  DMA channel. Apart from selecting the DMA channel the MSS_PDMA_setup_transfer()
  function will also setup the transfer size, source and destination addresses.
  This function will also configure the repeat and force order requirements as
  the PolarFire SoC MSS Peripheral DMA supports multiple simultaneous transfers.
  Once transfer is setup, it can be started.

  @param channel_id
           The channel_id parameter specifies the Platform DMA channel selected
           for DMA transaction.

  @param channel_config
           The channel_config parameter structure contains the data needed for
           a DMA transfer.
           - Source Address
           - Destination address
           - Number of Bytes
           - Enable the Done Interrupt
           - Enable the ErrorInterrupt
           - Set the active transfer type, single or repeat.
           - Force Order.

   @return pdma_error_id_t
           The function returns error signals of type mss_pdma_error_id_t.

  Example:
  The following call will configure channel 0
  @code
                g_pdma_error_code = MSS_PDMA_setup_transfer(PDMA_CHANNEL_0,
                                                          &pdma_config_ch0);
  @endcode
 */
mss_pdma_error_id_t
MSS_PDMA_setup_transfer
(
    mss_pdma_channel_id_t channel_id,
    mss_pdma_channel_config_t *channel_config
);

/*-------------------------------------------------------------------------*//**
  The MSS_PDMA_start_transfer() function is used to initiate an individual
  transfer on selected   DMA channel . The source and destination address of
  the transfer and the number of bytes to be transferred must be configured
  before calling this function.

  @param channel_id
           The channel_id parameter specifies the Platform DMA channel selected
           for DMA transaction.

   @return
           The function returns error signals of type mss_pdma_error_id_t.

  Example:
  The following call will configure channel 0
  @code
                // Setup the PDMA channel for transfer
                g_pdma_error_code = MSS_PDMA_setup_transfer(PDMA_CHANNEL_0,
                                                          &pdma_config_ch0);

                // Initiate the transfer for channel 0.
                MSS_PDMA_start_transfer(PDMA_CHANNEL_0);
  @endcode
 */
mss_pdma_error_id_t
MSS_PDMA_start_transfer
(
    mss_pdma_channel_id_t channel_id
);

/*-------------------------------------------------------------------------*//**
  The MSS_PDMA_set_transction_size() function is used to set a channel
  DMA transaction size. The write_size and read_size buffers are used to
  determine the size and alignment of individual PDMA transaction as a single
  PDMA transfer may require multiple transaction.

  @param channel_id
           The channel_id parameter specifies the Platform DMA channel selected
           for DMA transaction.

  @param write_size
           The write_size parameter specifies the base 2 logarithm of PDMA
           transaction.
           e.g. 0 is 1 byte, 3 is 8 bytes, 5 is 32 bytes

  @param read_size
           The read_size parameter specifies the base 2 logarithm of PDMA
           transaction.
           e.g. 0 is 1 byte, 3 is 8 bytes, 5 is 32 bytes

   @return
           The function returns error signals of type mss_pdma_error_id_t.

  Example:
  The following call will configure channel 0 transaction to 32bytes.
  @code
                MSS_PDMA_set_transction_size(PDMA_CHANNEL_0,
                                             write_size,
                                             read_size)
  @endcode
 */
mss_pdma_error_id_t
MSS_PDMA_set_transction_size
(
    mss_pdma_channel_id_t channel_id,
    uint8_t write_size,
    uint8_t read_size
);

/*-------------------------------------------------------------------------*//**
  The MSS_PDMA_get_active_transfer_type() function is used to request active
  transfer type for selected DMA channel.
  The transfer type for the DMA channel was configured during the channel
  configuration process, calling this function will read the transfer type
  from the configured DMA channel.

  @param channel_id
           The channel_id parameter specifies the Platform DMA channel selected
           for DMA transaction.

   @return
           This function returns a 32-bit value indicating the active transfer
           type of the DMA channel.

  Example:
  The following call will return the Channel (0) active transfer type.

  @code
                MSS_PDMA_get_active_transfer_type(PDMA_CHANNEL_0);
  @endcode
 */
uint32_t
MSS_PDMA_get_active_transfer_type
(
    mss_pdma_channel_id_t channel_id
);

/*-------------------------------------------------------------------------*//**
  The MSS_PDMA_get_number_bytes_remaining() function is used to request number
  of bytes remaining to be transfered.

  @param channel_id
           The channel_id parameter specifies the Platform DMA channel selected
           for DMA transaction.

  @return
           This function return 64-bit value indicating number of bytes
           remaining to be transferred.

  Example:
  The following call will return the number of bytes remaining Channel (0).

  @code
                MSS_PDMA_get_number_bytes_remaining(PDMA_CHANNEL_0);
  @endcode
 */
uint64_t
MSS_PDMA_get_number_bytes_remaining
(
    mss_pdma_channel_id_t channel_id
);

/*-------------------------------------------------------------------------*//**
  The MSS_PDMA_get_destination_current_addr() function is used to request the
  channel Destination address.

  @param channel_id
           The channel_id parameter specifies the Platform DMA channel selected
           for DMA transaction.

  @return
           This function return 64-bit value indicating destination address of
           the selected DMA channel.

  Example:
  The following call will return the Channel (0) Destination address register
  value.

  @code
                MSS_PDMA_get_destination_current_addr(PDMA_CHANNEL_0);
  @endcode
 */
uint64_t
MSS_PDMA_get_destination_current_addr
(
    mss_pdma_channel_id_t channel_id
);

/*-------------------------------------------------------------------------*//**
  The MSS_PDMA_get_source_current_addr() function is used to request the
  channel Source address.

  @param channel_id
           The channel_id parameter specifies the Platform DMA channel selected
           for DMA transaction.

  @return
           This function return 64-bit value indicating source address of
           the selected DMA channel.

  Example:
  The following call will return the Channel (0) Source address register
  value.

  @code
                MSS_PDMA_get_source_current_addr(PDMA_CHANNEL_0);
  @endcode
 */
uint64_t
MSS_PDMA_get_source_current_addr
(
    mss_pdma_channel_id_t channel_id
);

/*-------------------------------------------------------------------------*//**
  The MSS_PDMA_get_transfer_complete_status() function is used to request the
  completion status of last DMA transfer.

  @param channel_id
           The channel_id parameter specifies the Platform DMA channel selected
           for DMA transaction.

  @return
           This function return 8-bit value indicating completion status of the
           last DMA transfer.

  Example:
  The following call will return the Channel (0) transfer status.

  @code
                MSS_PDMA_get_transfer_complete_status(PDMA_CHANNEL_0);
  @endcode

 */
uint8_t
MSS_PDMA_get_transfer_complete_status
(
    mss_pdma_channel_id_t channel_id
);

/*-------------------------------------------------------------------------*//**
  The MSS_PDMA_get_transfer_error_status() function is used to request the
  error status of last DMA transfer.

  @param channel_id
           The channel_id parameter specifies the Platform DMA channel selected
           for DMA transaction.

  @return
           This function return 8-bit value indicating the error status of the
           last DMA transfer.

  Example:
  The following call will return the Channel (0) transfer error status.

  @code
                MSS_PDMA_get_transfer_error_status(PDMA_CHANNEL_0);
  @endcode

 */
uint8_t
MSS_PDMA_get_transfer_error_status
(
    mss_pdma_channel_id_t channel_id
);

/*-------------------------------------------------------------------------*//**
  The MSS_PDMA_clear_transfer_complete_status() function is used to request the
  transfer complete interrupt status. If the function returns one, that indicates
  that the transfer is complete and the transfer complete interrupt is cleared.

  @param channel_id
           The channel_id parameter specifies the Platform DMA channel selected
           for DMA transaction.

  @return
           This function return 8-bit value indicating the transfer complete
           interrupt status of the last DMA transfer.

  Example:
           The following call will return the Channel (0) transfer complete
           interrupt status of the last DMA transfer

  @code
                MSS_PDMA_clear_transfer_complete_status(PDMA_CHANNEL_0);
  @endcode
 */
uint8_t
MSS_PDMA_clear_transfer_complete_status
(
    mss_pdma_channel_id_t channel_id
);

/*-------------------------------------------------------------------------*//**
  The MSS_PDMA_clear_transfer_error_status() function is used to request the
  transfer error interrupt status. If the function returns one, that indicates
  that the transfer error has occurred and error interrupt is cleared.

  @param channel_id
           The channel_id parameter specifies the Platform DMA channel selected
           for DMA transaction.

  @return
           This function return 8-bit value indicating the transfer error
           interrupt status of the last DMA transfer.

  Example:
           The following call will return the Channel (0) transfer error
           interrupt status of the last DMA transfer

  @code
                MSS_PDMA_clear_transfer_complete_status(PDMA_CHANNEL_0);
  @endcode

 */
uint8_t
MSS_PDMA_clear_transfer_error_status
(
    mss_pdma_channel_id_t channel_id
);

#endif  /* MSS_PDMA_H */
