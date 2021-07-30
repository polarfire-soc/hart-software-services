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
 * PolarFire SoC microprocessor subsystem CAN bare metal software driver public
 * API.
 *
 * SVN $Revision$
 * SVN $Date$
 */
/*=========================================================================*//**
  @mainpage PolarFire MSS CAN Bare Metal Driver.

  ==============================================================================
  Introduction
  ==============================================================================
  The PolarFire SoC microprocessor subsystem (MSS) includes two CAN controller.
  The CAN controller is configurable to provide support for up to 32 transmit
  and 32 receive mailboxes.

  This PolarFire SoC MSS CAN driver provides a set of functions for accessing
  and controlling the MSS CAN as part of a bare metal system where no operating
  system is available. The driver can be adapted for use as part of an
  operating system, but the implementation of the adaptation layer between the
  driver and the operating system's driver model is outside the scope of the
  driver.

  --------------------------------
  Features
  --------------------------------
  The MSS CAN driver provides support for the following features:
    - Basic CAN APIs if application needs support for Basic CAN operation.
     (Configure as FIFO by linking several mailboxes together, one message
      filter for entire FIFO)
    - Full CAN APIs (each mail box has its own message filter)
    - Support for 11 bit and 29 bit message identifiers
    - Support for Data frame and Remote frames
    - Error detection mechanism

  ==============================================================================
  Hardware Flow Dependencies
  ==============================================================================
  The configuration of all features of the PolarFire MSS CAN is covered by
  this driver, with the exception of the PolarFire SoC IOMUX configuration.
  The PolarFire SoC allows multiple non-concurrent uses of few external pins
  through IOMUX configuration. This feature allows optimization of external pin
  usage by assigning external pins for use by either the microprocessor
  subsystem or the FPGA fabric. The MSS CAN serial signals are routed through
  IOMUXs to the PolarFire SoC device external pins. The MSS CAN serial
  signals can also be routed through IOMUXs to the PolarFire SoC FPGA fabric.
  For more information on IOMUX, see the IOMUX section of the PolarFire SoC
  Microprocessor Subsystem (MSS) User's Guide.

  The IOMUXs are configured using the PolarFire SoC MSS configurator tool. You
  must ensure that the MSS CAN peripherals are enabled and configured in the
  PolarFire SoC MSS configurator if you wish to use them. For more information
  on IOMUXs, refer to the IOMUX section of the PolarFire SoC microprocessor
  Subsystem (MSS) User's Guide.

  On PolarFire SoC an AXI switch forms a bus matrix interconnect among multiple
  masters and multiple slaves. Five RISC-V CPUs connect to the Master ports M10
  to M14 of the AXI switch. By default, all the APB peripherals are accessible
  on AXI-Slave 5 of the AXI switch via the AXI to AHB and AHB to APB bridges
  (referred as main APB bus). However, to support logical separation in the
  Asymmetric Multi-Processing (AMP) mode of operation, the APB peripherals can
  alternatively be accessed on the AXI-Slave 6 via the AXI to AHB and AHB to APB
  bridges (referred as the AMP APB bus).
  Application must make sure that the desired CAN instance is appropriately
  configured on one of the APB bus described above by configuring the PolarFire
  SoC system registers (SYSREG) as per the application need and that the
  appropriate data structures are provided to this driver as parameter to the
  functions provided by this driver.

  The base address and the register addresses are defined in this driver as
  constants. The interrupt number assignment for the MSS CAN peripherals is
  defined as constants in the MPFS HAL. You must ensure that the latest MPFS HAL
  is included in the project settings of the SoftConsole toolchain and that it
  is generated into your project.

  ==============================================================================
  Theory of Operation
  ==============================================================================
  The MSS CAN driver uses one instance of the mss_can_instance_t structure per
  port. This instance is used to identify the target port and a pointer to this
  instance is passed as the first argument to all CAN driver functions.

  The PolarFire SoC MSS CAN driver operations can be divided in following
  sub-sections:
    - CAN Controller Configuration
    - Operation Status
    - Interrupt Support
    - Helper Functions
    - Basic CAN Message Handling
    - Full CAN Message Handling

  --------------------------------
  Configuration
  --------------------------------
  The MSS CAN driver must first be initialized and the mode of operation must
  be selected before performing data transfers on CAN Bus. The MSS_CAN_init()
  function is used to initialize the CAN controller and driver. Set CAN
  controller operation mode as normal operation using MSS_CAN_set_mode()
  function. The operating mode of operation is selected using
  MSS_CAN_set_mode() function. The actual data transfer can be started using
  start the CAN controller by using MSS_CAN_start() function, with this actual
  data transmission or reception shall start. MSS_CAN_stop() function is used
  to stops the CAN controller. Once initialized, during normal mode of
  operation, the MSS CAN driver configuration can be changed using
  MSS_CAN_set_config_reg() function is used to change the CAN controllers
  configuration during normal operation.

  --------------------------------
  Operation Status
  --------------------------------
  MSS_CAN_get_error_status() function  returns the current CAN error state
  (error active, error passive, and bus off). The MSS_CAN_get_rx_error_count()
  and MSS_CAN_get_tx_error_count() functions return the actual
  receive and transmit  error counter values while MSS_CAN_get_rx_gte96()
  function and MSS_CAN_get_tx_gte96() function  show if the error counters are
  greater or equal to 96, which indicates a heavily disturbed bus.

  --------------------------------
  Interrupt Support
  --------------------------------
  The interrupt service routines are not part of the CAN driver. But access
  functions for the interrupt registers are provided. The individual
  interrupt enable bits can be set using MSS_CAN_set_int_ebl() function  and
  individual interrupt enable bits can be cleared using MSS_CAN_clear_int_ebl
  while MSS_CAN_get_int_ebl() function  returns their actual state.
  MSS_CAN_get_global_int_ebl() function indicates if interrupt
  generation is enabled at all. MSS_CAN_get_int_status() function shows the
  current state of the different interrupt status bits. Each interrupt status
  bit can be individually cleared using MSS_CAN_clear_int_status() function.

  The interrupt service routines are not part of the MSS CAN driver and the
  driver ships with the MSS_CAN_ENABLE_INTERRUPTS macro disabled which stops
  the MSS CAN interrupt being enabled at the PLIC, however a stub ISR is
  present in the mss_can.c file to show the format of the function and catch
  any unexpected interrupts to aid in debugging.

  --------------------------------
  Helper Functions
  --------------------------------
  The MSS CAN peripheral expects all ID bits to be left aligned. This makes
  setting the ID cumbersome. Using MSS_CAN_set_id(), a given right-aligned
  ID field is modified according to the ID size which is indicated by the
  IDE bit. MSS_CAN_get_id() provides the reverse operation: It returns the
  ID right aligned. MSS_CAN_get_msg_filter_mask() packs the ID, IDE, and RTR
  bits together as they are used in the mask registers. MSS_CAN_get_mask_n()
  returns the message filter settings of the selected receive mailbox.
  MSS_CAN_set_mask_n() configures the message filter settings for the
  selected receive mailbox.

  --------------------------------
  Basic CAN Message Handling
  --------------------------------
  A Basic CAN type controller contains one or more message filter and one
  common message buffer or FIFO. The CAN driver contains some functions to
  emulate Basic CAN operation by linking several buffers together to form a
  buffer array that shares one message filter. Since this buffer array is not
  a real FIFO, message inversion might happen (eg, a newer message might be
  pulled from the receive buffer prior to an older message).
  Before using the Basic CAN API, the CAN controller has to be configured
  first with a MSS_CAN_config_buffer() function  call. This sets up the
  message array and configures the message filter. MSS_CAN_send_message()
  function  and MSS_CAN_get_message() function  are used to send and receive
  a message from transmit or receive buffers. MSS_CAN_send_message_ready()
  function  indicates if a new message can be sent. MSS_CAN_get_message_av()
  function  shows if a new message is available.

  --------------------------------
  Full CAN Message Handling
  --------------------------------
  In Full CAN operation, each message mailbox has its own message filter.
  This reduces the number of receive interrupts as the host CPU only gets an
  interrupt when a message of interest has arrived. Further, software based
  message filtering overhead is reduced and there is less message to
  be checked. Before a buffer can be used for Full CAN operation, it needs to
  be configured using MSS_CAN_config_buffer_n() function. An error is
  generated if this buffer is already reserved for Basic CAN operation.
  The MSS_CAN_get_rx_buffer_status() and MSS_CAN_get_tx_buffer_status()
  functions indicate the current state of the receive and transmit buffers
  respectively. With MSS_CAN_send_message_n() function  a message can be sent
  using buffer. A  pending message transfer can be aborted with
  MSS_CAN_send_message_abort_n() function  and a message can be read with
  MSS_CAN_get_message_n() function. If a buffer is set for automatic RTR reply,
  MSS_CAN_set_rtr_message_n() function sets the CAN message that is returned
  upon reception of the RTR message.  MSS_CAN_get_rtr_message_abort_n()
  function aborts a RTR message transmit request.

  NOTE:
  1. User has to set the RTR message filter to match with
     MSS_CAN_rtr_message_abort_n() function a pending RTR auto-reply can be
     aborted.
  2. An error is generated if buffer is already reserved for Basic CAN
     operation and is trying to use the same buffer for Full CAN functionality.
  3. Special case of Full CAN where several mailboxes are linked together to
     create FIFOs that share an identical message filter configuration, can
     be built upon the available Full CAN functions.

 *//*=========================================================================*/

#ifndef MSS_CAN_H_
#define MSS_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#define __I  const volatile
#define __IO volatile
#define __O volatile

/* The following macro MSS_CAN_ENABLE_INTERRUPTS must be defined to allow the
 * enabling of the MSS CAN peripheral interrupts at the PLIC level.
 * This version of the MSS CAN driver does not provide any support for MSS CAN
 * interrupts and so this MACRO should be disabled unless there is a user
 * supplied ISR.
 */

#if 0
#define MSS_CAN_ENABLE_INTERRUPTS
#endif

/**
 * Define CAN target device
 *
 * CANMOD3: Device with 16 Rx and 8 Tx mailboxes
 * CANMOD3X: Device with 32 Rx and 32 Tx mailboxes
 */

#define CANMOD3X

#ifdef CANMOD3
  #define CAN_RX_MAILBOX 16u
  #define CAN_TX_MAILBOX 8u
#else
  #define CAN_RX_MAILBOX 32u
  #define CAN_TX_MAILBOX 32u
#endif


/* Configuration and Speed definitions */
#define CAN_PRESET                  (mss_can_config_reg.L)0
#define CAN_SAMPLE_BOTH_EDGES       0x00000001u
#define CAN_THREE_SAMPLES           0x00000002u
#define CAN_SET_SJW(_sjw)           (_sjw<<2u)
#define CAN_AUTO_RESTART            0x00000010u
#define CAN_SET_TSEG2(_tseg2)       (_tseg2<<5u)
#define CAN_SET_TSEG1(_tseg1)       (_tseg1<<8u)
#define CAN_SET_BITRATE(_bitrate)   (_bitrate<<16u)
#define CAN_ARB_ROUNDROBIN          0x00000000u
#define CAN_ARB_FIXED_PRIO          0x00001000u
#define CAN_BIG_ENDIAN              0x00000000u
#define CAN_LITTLE_ENDIAN           0x00002000u

/* Manual setting with specified fields  */
#define CAN_SPEED_MANUAL  0u

/*-------------------------------------------------------------------------*//**
  The following constants are used in the PolarFire SoC MSS CAN driver for
  bitrate definitions:

  | Constants          |  Description                                        |
  |--------------------|-----------------------------------------------------|
  | CAN_SPEED_8M_5K    | Indicates CAN controller shall be configured with   |
  |                    | 5Kbps baud rate if the input clock is 8MHz.         |
  | CAN_SPEED_16M_5K   | Indicates CAN controller shall be configured with   |
  |                    | 5Kbps baud rate if the input clock is 16MHz.        |
  | CAN_SPEED_32M_5K   | Indicates CAN controller shall be configured with   |
  |                    | 5Kbps baud rate if the input clock is 32MHz.        |
  | CAN_SPEED_8M_10K   | Indicates CAN controller shall be configured with   |
  |                    | 10Kbps baud rate if the input clock is 8MHz.        |
  | CAN_SPEED_16M_10K  | Indicates CAN controller shall be configured with   |
  |                    | 10Kbps baud rate if the input clock is 16MHz.       |
  | CAN_SPEED_32M_10K  | Indicates CAN controller shall be configured with   |
  |                    | 10Kbps baud rate if the input clock is 32MHz.       |
  | CAN_SPEED_8M_20K   | Indicates CAN controller shall be configured with   |
  |                    | 20Kbps baud rate if the input clock is 8MHz.        |
  | CAN_SPEED_16M_20K  | Indicates CAN controller shall be configured with   |
  |                    | 20Kbps baud rate if the input clock is 16MHz.       |
  | CAN_SPEED_32M_20K  | Indicates CAN controller shall be configured with   |
  |                    | 20Kbps baud rate if the input clock is 32MHz.       |
  | CAN_SPEED_8M_50K   | Indicates CAN controller shall be configured with   |
  |                    | 50Kbps baud rate if the input clock is 8MHz.        |
  | CAN_SPEED_16M_50K  | Indicates CAN controller shall be configured with   |
  |                    | 50Kbps baud rate if the input clock is 16MHz.       |
  | CAN_SPEED_32M_50K  | Indicates CAN controller shall be configured with   |
  |                    | 50Kbps baud rate if the input clock is 32MHz.       |
  | CAN_SPEED_8M_100K  | Indicates CAN controller shall be configured with   |
  |                    | 100Kbps baud rate if the input clock is 8MHz.       |
  | CAN_SPEED_16M_100K | Indicates CAN controller shall be configured with   |
  |                    | 100Kbps baud rate if the input clock is 16MHz.      |
  | CAN_SPEED_32M_100K | Indicates CAN controller shall be configured with   |
  |                    | 100Kbps baud rate if the input clock is 32MHz.      |
  | CAN_SPEED_8M_125K  | Indicates CAN controller shall be configured with   |
  |                    | 125Kbps baud rate if the input clock is 8MHz.       |
  | CAN_SPEED_16M_125K | Indicates CAN controller shall be configured with   |
  |                    | 125Kbps baud rate if the input clock is 16MHz.      |
  | CAN_SPEED_32M_125K | Indicates CAN controller shall be configured with   |
  |                    | 125Kbps baud rate if the input clock is 32MHz.      |
  | AN_SPEED_8M_250K   | Indicates CAN controller shall be configured with   |
  |                    | 250Kbps baud rate if the input clock is 8MHz.       |
  | CAN_SPEED_16M_250K | Indicates CAN controller shall be configured with   |
  |                    | 250Kbps baud rate if the input clock is 16MHz.      |
  | CAN_SPEED_32M_250K | Indicates CAN controller shall be configured with   |
  |                    | 250Kbps baud rate if the input clock is 32MHz.      |
  | CAN_SPEED_8M_500K  | Indicates CAN controller shall be configured with   |
  |                    | 500Kbps baud rate if the input clock is 8MHz.       |
  | CAN_SPEED_16M_500K | Indicates CAN controller shall be configured with   |
  |                    | 500Kbps baud rate if the input clock is 16MHz.      |
  | CAN_SPEED_32M_500K | Indicates CAN controller shall be configured with   |
  |                    | 500Kbps baud rate if the input clock is 32MHz.      |
  | CAN_SPEED_8M_1M    | Indicates CAN controller shall be configured with   |
  |                    | 1MBPS baud rate if the input clock is 8MHz.         |
  | CAN_SPEED_16M_1M   | Indicates CAN controller shall be configured with   |
  |                    | 1MBPS baud rate if the input clock is 16MHz.        |
  | CAN_SPEED_32M_1M   | Indicates CAN controller shall be configured with   |
  |                    | 1MBPS baud rate if the input clock is 32MHz.        |

 */
/* 5000m       81%  Sample bit three times  */
#define CAN_SPEED_8M_5K      CAN_SET_BITRATE(99)|CAN_SET_TSEG1(11)|CAN_SET_TSEG2(2)|CAN_THREE_SAMPLES
#define CAN_SPEED_16M_5K     CAN_SET_BITRATE(199)|CAN_SET_TSEG1(11)|CAN_SET_TSEG2(2)|CAN_THREE_SAMPLES
#define CAN_SPEED_32M_5K     CAN_SET_BITRATE(399)|CAN_SET_TSEG1(11)|CAN_SET_TSEG2(2)|CAN_THREE_SAMPLES

/* 5000m       81%  Sample bit three times */
#define CAN_SPEED_8M_10K     CAN_SET_BITRATE(49)|CAN_SET_TSEG1(11)|CAN_SET_TSEG2(2)|CAN_THREE_SAMPLES
#define CAN_SPEED_16M_10K    CAN_SET_BITRATE(99)|CAN_SET_TSEG1(11)|CAN_SET_TSEG2(2)|CAN_THREE_SAMPLES
#define CAN_SPEED_32M_10K    CAN_SET_BITRATE(199)|CAN_SET_TSEG1(11)|CAN_SET_TSEG2(2)|CAN_THREE_SAMPLES

/* 2500m       81%  Sample bit three times */
#define CAN_SPEED_8M_20K     CAN_SET_BITRATE(24)|CAN_SET_TSEG1(11)|CAN_SET_TSEG2(2)|CAN_THREE_SAMPLES
#define CAN_SPEED_16M_20K    CAN_SET_BITRATE(49)|CAN_SET_TSEG1(11)|CAN_SET_TSEG2(2)|CAN_THREE_SAMPLES
#define CAN_SPEED_32M_20K    CAN_SET_BITRATE(99)|CAN_SET_TSEG1(11)|CAN_SET_TSEG2(2)|CAN_THREE_SAMPLES

/* 1000m       87% */
#define CAN_SPEED_8M_50K     CAN_SET_BITRATE(9)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)
#define CAN_SPEED_16M_50K    CAN_SET_BITRATE(19)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)
#define CAN_SPEED_32M_50K    CAN_SET_BITRATE(39)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)

/* 600m        87% */
#define CAN_SPEED_8M_100K    CAN_SET_BITRATE(4)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)
#define CAN_SPEED_16M_100K   CAN_SET_BITRATE(9)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)
#define CAN_SPEED_32M_100K   CAN_SET_BITRATE(19)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)

/*  500m        87% */
#define CAN_SPEED_8M_125K    CAN_SET_BITRATE(3)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)
#define CAN_SPEED_16M_125K   CAN_SET_BITRATE(7)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)
#define CAN_SPEED_32M_125K   CAN_SET_BITRATE(15)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)

/* 250m        87% */
#define CAN_SPEED_8M_250K    CAN_SET_BITRATE(1)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)
#define CAN_SPEED_16M_250K   CAN_SET_BITRATE(3)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)
#define CAN_SPEED_32M_250K   CAN_SET_BITRATE(7)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)

/* 100m        75% @ 8M, 87% @ 16M */
#define CAN_SPEED_8M_500K    CAN_SET_BITRATE(1)|CAN_SET_TSEG1(4)|CAN_SET_TSEG2(1)
#define CAN_SPEED_16M_500K   CAN_SET_BITRATE(1)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)
#define CAN_SPEED_32M_500K   CAN_SET_BITRATE(3)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)

/* 25m         75% */
#define CAN_SPEED_8M_1M      CAN_SET_BITRATE(0)|CAN_SET_TSEG1(4)|CAN_SET_TSEG2(1)
#define CAN_SPEED_16M_1M     CAN_SET_BITRATE(1)|CAN_SET_TSEG1(4)|CAN_SET_TSEG2(1)
#define CAN_SPEED_32M_1M     CAN_SET_BITRATE(1)|CAN_SET_TSEG1(12)|CAN_SET_TSEG2(1)

/*-------------------------------------------------------------------------*//**
  The following constants are used for error codes:

  |  Constants            |  Description                                |
  |-----------------------|---------------------------------------------|
  | CAN_OK                | Indicates there is no error                 |
  | CAN_ERR               | Indicates error condition                   |
  | CAN_TSEG1_TOO_SMALL   | Value provided to configure TSEG1 is too    |
  |                       | small                                       |
  | CAN_TSEG2_TOO_SMALL   | Value provided to configure TSEG2 is too    |
  |                       | small                                       |
  | CAN_SJW_TOO_BIG       | Value provided to configure synchronous jump|
  |                       | width (SJW) is too big.                     |
  | CAN_BASIC_CAN_MAILBOX | Indicates that mailbox is configured for    |
  |                       | Basic CAN operation                         |
  | CAN_NO_RTR_MAILBOX    | Indicates that there is no mailbox for      |
  |                       | remote transmit request (RTR) frame         |
  | CAN_INVALID_MAILBOX   | Indicates invalid mailbox number            |

 */
#define CAN_OK                 0u
#define CAN_ERR                1u
#define CAN_TSEG1_TOO_SMALL    2u
#define CAN_TSEG2_TOO_SMALL    3u
#define CAN_SJW_TOO_BIG        4u
#define CAN_BASIC_CAN_MAILBOX  5u
#define CAN_NO_RTR_MAILBOX     6u
#define CAN_INVALID_MAILBOX    7u

/*  Flag bits */
#define CAN_NO_MSG         0x00u
#define CAN_VALID_MSG      0x01u

/*
 * A couple of definitions just to make the code more readable so we know
 * what a 1 and 0 mean.
#define CAN_RTR           1<<21
#define CAN_EXT_IDE       1<<20

/*-------------------------------------------------------------------------*//**
  The following constants are used in the MSS CAN driver for Interrupt Bit
  Definitions

  |  Constants               |  Description                                   |
  |--------------------------|------------------------------------------------|
  | CAN_INT_GLOBAL           | Indicates to enable global interrupts          |
  | CAN_INT_ARB_LOSS         | Indicates arbitration loss interrupt           |
  | CAN_INT_OVR_LOAD         | Indicates overload message detected interrupt  |
  | CAN_INT_BIT_ERR          | Indicates bit error interrupt                  |
  | CAN_INT_STUFF_ERR        | Indicates bit stuffing error interrupt         |
  | CAN_INT_ACK_ERR          | Indicates acknowledge error interrupt          |
  | CAN_INT_FORM_ERR         | Indicates format error interrupt               |
  | CAN_INT_CRC_ERR          | Indicates CRC error interrupt                  |
  | CAN_INT_BUS_OFF          | Indicates bus off interrupt                    |
  | CAN_INT_RX_MSG_LOST      | Indicates received message lost interrupt      |
  | CAN_INT_TX_MSG           | Indicates message transmit interrupt           |
  | CAN_INT_RX_MSG           | Indicates receive message available interrupt  |
  | CAN_INT_RTR_MSG          | Indicates RTR auto-reply message sent interrupt|
  | CAN_INT_STUCK_AT_0       | Indicates stuck at dominant error interrupt    |
  | CAN_INT_SST_FAILURE      | Indicates single shot transmission failure     |
  |                          | interrupt                                      |

 */
#define CAN_INT_GLOBAL        1<<0    /* Global interrupt  */
#define CAN_INT_ARB_LOSS      1<<2    /* Arbitration loss interrupt  */
#define CAN_INT_OVR_LOAD      1<<3    /*Overload interrupt  */
#define CAN_INT_BIT_ERR       1<<4    /* Bit error interrupt  */
#define CAN_INT_STUFF_ERR     1<<5    /* Bit stuffing error interrupt  */
#define CAN_INT_ACK_ERR       1<<6    /* Acknowledgement error interrupt  */
#define CAN_INT_FORM_ERR      1<<7    /* Format error interrupt  */
#define CAN_INT_CRC_ERR       1<<8    /* CRC error interrupt  */
#define CAN_INT_BUS_OFF       1<<9    /* Bus-off interrupt  */
#define CAN_INT_RX_MSG_LOST   1<<10   /* Rx message lost interrupt  */
#define CAN_INT_TX_MSG        1<<11   /* Tx message interupt  */
#define CAN_INT_RX_MSG        1<<12   /* Rx message interrupt  */
#define CAN_INT_RTR_MSG       1<<13   /* RTR message interrupt  */
#define CAN_INT_STUCK_AT_0    1<<14   /* Stuck-at-0 error interrupt  */
#define CAN_INT_SST_FAILURE   1<<15   /* Single-shot transmission error interrupt*/

/*-------------------------------------------------------------------------*//**
  The following constants are used for transmit message buffer control bit
  definitions:

  |  Constants               |  Description                                   |
  |--------------------------|------------------------------------------------|
  | CAN_TX_WPNH_EBL          | Indicates "WPNH" bit mask                      |
  | CAN_TX_WPNL_EBL          | Indicates WPNL bit mask                        |
  | CAN_TX_REQ               | Indicates transmit request flag bit position   |
  | CAN_TX_INT_EBL           | Indicates transmit Interrupt enable bit mask   |
  | CAN_TX_ABORT             | Indicates Transmit abort mask                  |

 */
#define CAN_TX_WPNH_EBL      1<<23
#define CAN_TX_WPNL_EBL      1<<3
#define CAN_TX_INT_EBL       1<<2
#define CAN_TX_ABORT         1<<1
#define CAN_TX_REQ           0x01u

/*-------------------------------------------------------------------------*//**
  The following constants are used for receive message buffer control bit
  definitions:

  |  Constants               |  Description                                   |
  |--------------------------|------------------------------------------------|
  | CAN_RX_WPNH_EBL          | Indicates WPNH bit mask.                       |
  | CAN_RX_WPNL_EBL          | Indicates WPNL bit mask                        |
  | CAN_RX_LINK_EBL          | Indicates link flag bit mask                   |
  | CAN_RX_INT_EBL           | Indicates receive interrupt enable bit mask    |
  | CAN_RX_RTR_REPLY_EBL     | Indicates RTR reply bit mask                   |
  | CAN_RX_BUFFER_EBL        | Indicates Transaction buffer enable bit mask   |
  | CAN_RX_RTR_ABORT         | Indicates RTR abort request mask               |
  | CAN_RX_RTRP              | Indicates RTReply pending status mask          |
  | CAN_RX_MSGAV             | Indicates receive message available status mask|

 */
#define CAN_RX_WPNH_EBL      1<<23
#define CAN_RX_WPNL_EBL      1<<7
#define CAN_RX_LINK_EBL      1<<6
#define CAN_RX_INT_EBL       1<<5
#define CAN_RX_RTR_REPLY_EBL 1<<4
#define CAN_RX_BUFFER_EBL    1<<3
#define CAN_RX_RTR_ABORT     1<<2
#define CAN_RX_RTRP          1<<1
#define CAN_RX_MSGAV         0x01

/*-------------------------------------------------------------------------*//**
  The mss_can_mode_t enumeration specifies the possible operating modes of CAN
  controller. The meaning of the constants is as described below

  |  Modes                     |  Description                             |
  |----------------------------|------------------------------------------|
  | CANOP_MODE_NORMAL          | Indicates CAN controller is in normal    |
  |                            | operational mode.                        |
  | CANOP_MODE_LISTEN_ONLY     | Indicates CAN controller is in listen    |
  |                            | only mode.                               |
  | CANOP_MODE_EXT_LOOPBACK    | Indicates CAN controller is in external  |
  |                            | loop back mode.                          |
  | CANOP_MODE_INT_LOOPBACK    | Indicates CAN controller is in internal  |
  |                            | loop back mode.                          |
  | CANOP_SRAM_TEST_MODE       | Indicates CAN controller is in test mode.|
  | CANOP_SW_RESET             | Indicates CAN controller is in stop mode.|

 */
typedef enum mss_can_mode
{
    CANOP_MODE_NORMAL       = 0x01u,
    CANOP_MODE_LISTEN_ONLY  = 0x03u,
    CANOP_MODE_EXT_LOOPBACK = 0x05u,
    CANOP_MODE_INT_LOOPBACK = 0x07u,
    CANOP_SRAM_TEST_MODE    = 0x08u,
    CANOP_SW_RESET          = 0x10u
} mss_can_mode_t;

typedef struct _mss_can_msgobject
{
    /* CAN Message ID. */
    struct
    {
        __IO uint32_t N_ID:3;
        __IO uint32_t ID:29;
    };

    /* CAN Message Data organized as two 32 bit words or 8 data bytes */
    union
    {
       struct
       {
           __IO uint32_t DATAHIGH;
           __IO uint32_t DATALOW;
        };
        __IO int8_t DATA[8];
    };

    /* CAN Message flags and smaller values organized as one single 32 bit word
       or a number of bit fields. */
    union
    {
        __IO uint32_t L;   /* 32 bit flag */
        struct
        {
            /* Flags structure. */
            __IO uint32_t NA0:16;
            __IO uint32_t DLC:4;
            __IO uint32_t IDE:1;
            __IO uint32_t RTR:1;
            __IO uint32_t NA1:10;
        };
    };
} mss_can_msgobject;

typedef mss_can_msgobject * pmss_can_msgobject;

/* _CAN_filterobject */

typedef struct _CAN_filterobject
{
    /* Acceptance mask settings */
    union
    {
        __IO    uint32_t L;
        struct
        {
            __IO uint32_t N_A:1;
            __IO uint32_t RTR:1;
            __IO uint32_t IDE:1;
            __IO uint32_t ID:29;
        };
    } AMR;

    /* Acceptance code settings */
    union
    {
        __IO uint32_t L;
        struct
        {
            __IO uint32_t N_A:1;
            __IO uint32_t RTR:1;
            __IO uint32_t IDE:1;
            __IO uint32_t ID:29;
        };
    } ACR;

    /* Acceptance mask and code settings for first two data bytes */
    union
    {
        __IO uint32_t L;
        struct
        {
            __IO uint32_t MASK:16;
            __IO uint32_t CODE:16;
        };
    } AMCR_D;
} mss_can_filterobject;

typedef mss_can_filterobject * pmss_can_filterobject;

/*_CAN_txmsgobject */

typedef struct _CAN_txmsgobject
{
    /* CAN Message flags and smaller values organized as one single 32 bit word
       or a number of bit fields.*/
    union
    {
        __IO uint32_t L;

        /* Tx Flags structure. */
        struct
        {
            __IO uint32_t TXREQ:1;
            __IO uint32_t TXABORT:1;
            __IO uint32_t TXINTEBL:1;
            __IO uint32_t WPNL:1;
            __IO uint32_t NA0:12;
            __IO uint32_t DLC:4;
            __IO uint32_t IDE:1;
            __IO uint32_t RTR:1;
            __IO uint32_t NA1:1;
            __IO uint32_t WPNH:1;
            __IO uint32_t NA2:8;
        };
    } TXB;

    /* CAN Message ID. */
    struct
    {
        __IO uint32_t N_ID:3;
        __IO uint32_t ID:29;
    };

    /* CAN Message Data organized as two 32 bit words or 8 data bytes */
    union
    {
        struct
        {
            __IO uint32_t DATAHIGH;
            __IO uint32_t DATALOW;
        };
        __IO int8_t DATA[8];
    };

} mss_can_txmsgobject;

/* _mss_can_rxmsgobject */

typedef struct _mss_can_rxmsgobject
{
    /* CAN Message flags and smaller values organized as one single 32 bit word
       or a number of bit fields. */
    union
    {
        __IO uint32_t L;                 /* 32 bit flag */

        /* Tx Flags structure. */
        struct
        {
            __IO uint32_t MSGAV:1;
            __IO uint32_t RTRREPLYPEND:1;
            __IO uint32_t RTRABORT:1;
            __IO uint32_t BUFFEREBL:1;
            __IO uint32_t RTRREPLY:1;
            __IO uint32_t RXINTEBL:1;
            __IO uint32_t LINKFLAG:1;
            __IO uint32_t WPNL:1;
            __IO uint32_t NA0:8;
            __IO uint32_t DLC:4;
            __IO uint32_t IDE:1;
            __IO uint32_t RTR:1;
            __IO uint32_t NA1:1;
            __IO uint32_t WPNH:1;
            __IO uint32_t NA2:8;
        };
    } RXB;

    /* CAN Message ID.  */
    struct
    {
        __IO uint32_t N_ID:3;
        __IO uint32_t ID:29;
    };

    /* CAN Message Data organized as two 32 bit words or 8 data bytes */
    union
    {
        struct
        {
            __IO uint32_t DATAHIGH;
            __IO uint32_t DATALOW;
        };
        __IO int8_t DATA[8];
    };

    /* CAN Message Filter: Acceptance mask register */
    union
    {
        __IO uint32_t L;
        struct
        {
            __IO uint32_t N_A:1;
            __IO uint32_t RTR:1;
            __IO uint32_t IDE:1;
            __IO uint32_t ID:29;
       };
    } AMR;

    /* CAN Message Filter: Acceptance code register */
    union
    {
        __IO uint32_t L;
        struct
        {
            __IO uint32_t N_A:1;
            __IO uint32_t RTR:1;
            __IO uint32_t IDE:1;
            __IO uint32_t ID:29;
        };
    } ACR;

    __IO uint32_t AMR_D;
    __IO uint32_t ACR_D;

} mss_can_rxmsgobject;
typedef mss_can_rxmsgobject * pmss_can_rxmsgobject;

/* Error status register */
typedef union error_status
{
    __IO  uint32_t L;
    struct
    {
        __IO uint32_t TX_ERR_CNT:8;
        __IO uint32_t RX_ERR_CNT:8;
        __IO uint32_t ERROR_STAT:2;
        __IO uint32_t TXGTE96:1;
        __IO uint32_t RXGTE96:1;
        __IO uint32_t N_A:12;
   };
} mss_can_error_status;

/*
 * Buffer status register For CANMOD3X,
 * this are two 32-bit registers, for can, it is only one 32-bit register.
 */
#ifdef CANMOD3
typedef union buffer_status
{
    __I uint32_t L;
    struct
    {
        __I uint32_t RXMSGAV:16;
        __I uint32_t TXREQ:8;
        __I uint32_t N_A:8;
    };
#else
typedef struct buffer_status
{
    __I uint32_t RXMSGAV;
    __I uint32_t TXREQ;
#endif
} mss_can_buffer_status;

/* Interrupt enable register */
typedef union int_enable
{
    __IO  uint32_t L;
    struct
    {
        __IO uint32_t INT_EBL:1;
        __IO uint32_t N_A0:1;
        __IO uint32_t ARB_LOSS:1;
        __IO uint32_t OVR_LOAD:1;
        __IO uint32_t BIT_ERR:1;
        __IO uint32_t STUFF_ERR:1;
        __IO uint32_t ACK_ERR:1;
        __IO uint32_t FORM_ERR:1;
        __IO uint32_t CRC_ERR:1;
        __IO uint32_t BUS_OFF:1;
        __IO uint32_t RX_MSG_LOSS:1;
        __IO uint32_t TX_MSG:1;
        __IO uint32_t RX_MSG:1;
        __IO uint32_t RTR_MSG:1;
        __IO uint32_t STUCK_AT_0:1;
        __IO uint32_t SST_FAILURE:1;
        __IO uint32_t N_A1:16;
    };
} mss_can_int_enable;

typedef mss_can_int_enable * pmss_can_int_enable;

/* Interrupt status register */
typedef union int_status
{
    __IO uint32_t L;
    struct
    {
        __IO uint32_t N_A0:2;
        __IO uint32_t ARB_LOSS:1;
        __IO uint32_t OVR_LOAD:1;
        __IO uint32_t BIT_ERR:1;
        __IO uint32_t STUFF_ERR:1;
        __IO uint32_t ACK_ERR:1;
        __IO uint32_t FORM_ERR:1;
        __IO uint32_t CRC_ERR:1;
        __IO uint32_t BUS_OFF:1;
        __IO uint32_t RX_MSG_LOSS:1;
        __IO uint32_t TX_MSG:1;
        __IO uint32_t RX_MSG:1;
        __IO uint32_t RTR_MSG:1;
        __IO uint32_t STUCK_AT_0:1;
        __IO uint32_t SST_FAILURE:1;
        __IO uint32_t N_A1:16;
   };
} mss_can_int_status;
typedef mss_can_int_status * pmss_can_int_status;

/* Command register */
typedef union command_reg
{
    __IO uint32_t L;
    struct
    {
        __IO uint32_t RUN_STOP:1;
        __IO uint32_t LISTEN_ONLY:1;
        __IO uint32_t LOOP_BACK:1;
        __IO uint32_t SRAM_TEST:1;
        __IO uint32_t SW_RESET:1;
        __IO uint32_t N_A:27;
    };
} mss_can_command_reg;

/* Configuration register */
typedef union can_config_reg
{
    __IO uint32_t L;
    struct
    {
        __IO uint32_t EDGE_MODE:1;
        __IO uint32_t SAMPLING_MODE:1;
        __IO uint32_t CFG_SJW:2;
        __IO uint32_t AUTO_RESTART:1;
        __IO uint32_t CFG_TSEG2:3;
        __IO uint32_t CFG_TSEG1:4;
        __IO uint32_t CFG_ARBITER:1;
        __IO uint32_t ENDIAN:1;
        __IO uint32_t ECR_MODE:1;
        __IO uint32_t N_A0:1;
        __IO uint32_t CFG_BITRATE:15;
        __IO uint32_t N_A1:1;
    };
} mss_can_config_reg;
typedef mss_can_config_reg * pmss_can_config_reg ;

/* Register mapping of CAN controller */
typedef struct CAN_device
{
    mss_can_int_status    IntStatus;      /* Interrupt status register */
    mss_can_int_enable    IntEbl;         /* Interrupt enable register */
    mss_can_buffer_status BufferStatus;   /* Buffer status indicators */
    mss_can_error_status  ErrorStatus;    /* Error status */
    mss_can_command_reg   Command;        /* CAN operating mode */
    mss_can_config_reg    Config;         /* Configuration register */
#ifdef CANMOD3
    uint32_t              NA[2];
#else
    uint32_t              NA;
#endif
    mss_can_txmsgobject   TxMsg[CAN_TX_MAILBOX];   /* Tx message buffers */
    mss_can_rxmsgobject   RxMsg[CAN_RX_MAILBOX];   /* Rx message buffers */

} CAN_DEVICE;

typedef CAN_DEVICE * PCAN_DEVICE;

#define MSS_CAN_0_LO_BASE           (CAN_DEVICE*)0x2010C000
#define MSS_CAN_1_LO_BASE           (CAN_DEVICE*)0x2010D000
#define MSS_CAN_0_HI_BASE           (CAN_DEVICE*)0x2810C000
#define MSS_CAN_1_HI_BASE           (CAN_DEVICE*)0x2810D000

#define SYSREG_CAN_A_SOFTRESET_MASK           ( (uint32_t)0x01u << 14u )
#define SYSREG_CAN_B_SOFTRESET_MASK           ( (uint32_t)0x01u << 15u )

/*-------------------------------------------------------------------------*//**
  The structure mss_can_instance_t is used by the driver to manage the
  configuration and operation of each MSS CAN peripheral. The instance content
  should only be accessed by using the respective API functions.

  Each API function has a pointer to this instance as first argument.

 */
typedef struct can_instance
{
    /* Hardware related entries (pointer to device, interrupt number etc) */
    CAN_DEVICE * hw_reg;   /* Pointer to CAN registers. */
    uint8_t            irqn;      /* refer to local or PLIC */
    uint8_t            int_type;  /*!< 0 => local, 1 => PLIC */
    /* Local data (eg pointer to local FIFO, irq number etc) */
    uint8_t  basic_can_rx_mb; /* number of rx mailboxes */
    uint8_t  basic_can_tx_mb; /* number of tx mailboxes */
 } mss_can_instance_t;

 /*------------------------------------------------------------------------*//**
  This instances of mss_can_instance_t holds all data related to the operations
  performed by CAN. A pointer to instance is passed as the first parameter
  to CAN driver functions to indicate that which CAN instance should perform the
  requested operation.
 */
extern mss_can_instance_t g_mss_can_0_lo;
extern mss_can_instance_t g_mss_can_1_lo;
extern mss_can_instance_t g_mss_can_0_hi;
extern mss_can_instance_t g_mss_can_1_hi;

/*----------------------------------------------------------------------------*/
/*-----------------------MSS CAN Public APIs ---------------------------------*/
/*----------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_init() function initializes the CAN driver as well as the CAN
  controller. The basic_can_rx_mb and basic_can_tx_mb are used to configure the
  number of receive and transmit mailboxes in basic CAN operation. This function
  configures the CAN channel speed as per the "bitrate" parameter. It
  initializes all receive mailboxes and make it ready for configuration. This is
  the first function to be called before using any other function.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param bitrate
    The bitRate parameter is used to configure CAN speed. The following standard
    preset definitions are provided for systems with a PCLK1 of 8MHz, 16MHz or
    32MHz:
        +-------------------+--------------------+--------------------+
        | 8MHz PCLK1        | 16MHz PCLK1        | 32MHz PCLK1        |
        +-------------------+--------------------+--------------------+
        | CAN_SPEED_8M_5K   | CAN_SPEED_16M_5K   | CAN_SPEED_32M_5K   |
        | CAN_SPEED_8M_10K  | CAN_SPEED_16M_10K  | CAN_SPEED_32M_10K  |
        | CAN_SPEED_8M_20K  | CAN_SPEED_16M_20K  | CAN_SPEED_32M_20K  |
        | CAN_SPEED_8M_50K  | CAN_SPEED_16M_50K  | CAN_SPEED_32M_50K  |
        | CAN_SPEED_8M_100K | CAN_SPEED_16M_100K | CAN_SPEED_32M_100K |
        | CAN_SPEED_8M_125K | CAN_SPEED_16M_125K | CAN_SPEED_32M_125K |
        | CAN_SPEED_8M_250K | CAN_SPEED_16M_250K | CAN_SPEED_32M_250K |
        | CAN_SPEED_8M_500K | CAN_SPEED_16M_500K | CAN_SPEED_32M_500K |
        | CAN_SPEED_8M_1M   | CAN_SPEED_16M_1M   | CAN_SPEED_32M_1M   |
        +-------------------+--------------------+--------------------+

    For custom settings, use CAN_SPEED_MANUAL and configure the settings via
    pcan_config.

    The default configurations can be altered by the addition of 0 or more of
    the following:
        - CAN_AUTO_RESTART
        - CAN_ARB_FIXED_PRIO
        - CAN_LITTLE_ENDIAN

  @param pcan_config
    The pcan_config parameter is a pointer to a mss_can_config_reg structure. This
    structure is only used when bitrate is configured as CAN_SPEED_MANUAL.

    When populating the mss_can_config_reg structure, the following should be noted:

      1. CFG_BITRATE defines the length of a CAN time quantum in terms of PCLK1
         with 0 = 1 PCLK1, 1 = 2 PCLK1s and so on.
      2. A CAN bit time is made up of between 8 and 25 time quanta and the bit
         rate is PCLK1 / ((CFG_BITRATE + 1) * number of time quanta per bit).
      3. There is a fixed overhead of 1 time quantum for synchronization at the
         start of every CAN bit and the remaining time quanta in the bit are
         allocated with CFG_TSEG1 and CFG_TSEG2.
      4. CFG_TSEG1 can have a value between 2 and 15 which represents between 3
         and 16 time quanta.
      5. If SAMPLING_MODE is 0, CFG_TSEG2 can have a value between 1 and 7 which
         represents between 2 and 8 time quanta and if SAMPLING_MODE is 1,
         CFG_TSEG2 can have a value between 2 and 7 which represents between 3
         and 8 time quanta.
      6. Receive sampling takes place at the end of the segment defined by
         CFG_TSEG1.

    For example, if CFG_TSEG1 = 3 and CFG_TSEG2 = 2 we get:

          |<------------ 1 CAN bit time (8 time quanta)------------>|
           /------+------+------+------+------+------+------+------\
         -+ Synch |        CFG_TSEG1 + 1      | CFG_TSEG2 + 1       +-
           \------+------+------+------+------+------+------+------/
                                              |
                Receiver samples date here -->|

  @param basic_can_rx_mb
    The basic_can_rx_mb parameter is the number of receive mailboxes used in
    basic CAN mode.

  @param basic_can_tx_mb
     The basic_can_tx_mb parameter is the number of transmit mailboxes used in
     basic CAN mode.

  @return
    This function returns CAN_OK on successful execution, otherwise it will
    returns following error codes:

    |  Constants            |  Description                                |
    |-----------------------|---------------------------------------------|
    | CAN_ERR               | Indicates error condition                   |
    | CAN_TSEG1_TOO_SMALL   | Value provided to configure TSEG1 is too    |
    |                       | small                                       |
    | CAN_TSEG2_TOO_SMALL   | Value provided to configure TSEG2 is too    |
    |                       | small                                       |
    | CAN_SJW_TOO_BIG       | Value provided to configure synchronous jump|
    |                       | width (SJW) is too big.                     |

  Example 1: Using a default set for bitrate, tseg1, tseg2, and sjw and
             additional configuration parameters.
  @code
      mss_can_instance_t g_mss_can_0_lo;
      int e51(void)
      {
          MSS_CAN_init(&g_mss_can_0_lo, (CAN_SPEED_16M_500K | CAN_AUTO_RESTART | \
                       CAN_LITTLE_ENDIAN),(pmss_can_config_reg)0,16u,7u);

          return(0);
      }
  @endcode

  Example 2: Using custom settings for bitrate, tseg1, tseg2, and sjw.
  @code
      mss_can_instance_t g_mss_can_0_lo;

      #define SYSTEM_CLOCK    8000000
      #define BITS_PER_SECOND 10000

      int e51(void)
      {
          mss_can_config_reg canreg;

          canreg.CFG_BITRATE = (SYSTEM_CLOCK / (BITS_PER_SECOND * 8) - 1;
          canreg.CFG_TSEG1 = 4;
          canreg.CFG_TSEG2 = 1;
          canreg.AUTO_RESTART = 0;
          canreg.CFG_SJW = 0;
          canreg.SAMPLING_MODE = 0;
          canreg.EDGE_MODE = 0;
          canreg.ENDIAN = 1;

          MSS_CAN_init(&g_mss_can_0_lo,CAN_SPEED_MANUAL,&canreg,8,4);

          return(0);
      }
  @endcode
 */
uint8_t
MSS_CAN_init
(
    mss_can_instance_t* this_can,
    uint32_t bitrate,
    pmss_can_config_reg pcan_config,
    uint8_t basic_can_rx_mb,
    uint8_t basic_can_tx_mb
);

 /*------------------------------------------------------------------------*//**
  The MSS_CAN_set_config_reg() function  sets the configuration register and
  starts the CAN controller for normal mode operation. This function is used
  when one needs to change the configuration settings while the CAN controller
  was already initialized using MSS_CAN_init() function  and is running.
  MSS_CAN_set_config_reg() function should not be used when the CAN controller
  wasn't initialized yet.

  It performs following tasks:
      - Clears all pending interrupts
      - Stops CAN controller
      - Disable interrupts
      - Sets new configuration
      - Starts CAN controller

  @param this_can
    The this_can parameter is a pointer to the MSS_CAN_instance_t structure.

  @param cfg
    The cfg parameter is a 4 bytes variable used to set the configuration
    settings.

  @return
    This function does not return a value.

  Example:
  @code
      mss_can_instance_t g_mss_can_0_lo;

      int e51(void)
      {
          Return_status = MSS_CAN_init(&g_mss_can_0_lo,(CAN_SPEED_16M_500K | \
                                      CAN_AUTO_RESTART | CAN_LITTLE_ENDIAN),
                                      (pmss_can_config_reg)0,16,7);
          ....

          MSS_CAN_set_config_reg(&g_mss_can_0_lo, (CAN_SPEED_16M_500K | \
                                 CAN_AUTO_RESTART | CAN_LITTLE_ENDIAN));

          ....
          return(0);
      }
  @endcode
 */
void
MSS_CAN_set_config_reg
(
    mss_can_instance_t* this_can,
    uint32_t cfg
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_set_mode() function sets the CAN controller operating mode based
  on the mode parameter. After this operation CAN controller is not in
  operational, to do that invoke MSS_CAN_start() function.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param mode
    The mode parameter tells about desired operating mode of CAN controller.
    Possible operating modes are as mentioned below:

    |  Mode                    | Description                             |
    |--------------------------|-----------------------------------------|
    | CANOP_MODE_INT_LOOPBACK  | Sets normal operating mode              |
    | CANOP_MODE_LISTEN_ONLY   | In listen-only mode, the CAN controller |
    |                          | does not send any messages. Normally    |
    |                          | used for automatic bitrate detection    |
    | CANOP_MODE_INT_LOOPBACK  | Selects internal loopback mode. This is |
    |                          | used for self-test                      |
    | CANOP_MODE_EXT_LOOPBACK  | Selects external loopback. The CAN      |
    |                          | controller will receive a copy of each  |
    |                          | message sent.                           |
    | CANOP_SRAM_TEST_MODE     | Sets SRAM test mode                     |
    | CANOP_SW_RESET           | Issues a software reset                 |

  @return
    This function does not return a value.

  Example:
  @code
      int e51(void)
      {
          MSS_CAN_init(&g_mss_can_0_lo,(CAN_SPEED_16M_500K | CAN_AUTO_RESTART | \
                       CAN_LITTLE_ENDIAN),(pmss_can_config_reg)0,16,7);
          MSS_CAN_set_mode(&g_mss_can_0_lo,CANOP_MODE_INT_LOOPBACK);

          ....

          return(0);
      }
  @endcode
 */
void
MSS_CAN_set_mode
(
    mss_can_instance_t* this_can,
    mss_can_mode_t mode
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_start() function clears all pending interrupts and enable CAN
  controller to perform normal operation. It enables receive interrupts also.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @return
    This function does not return a value.

  Example:
  @code
      int e51(void)
      {
          MSS_CAN_init(&g_mss_can_0_lo,(CAN_SPEED_16M_500K | CAN_AUTO_RESTART | \
                       CAN_LITTLE_ENDIAN),(pmss_can_config_reg)0,16,7);
          MSS_CAN_set_mode(&g_mss_can_0_lo,CANOP_MODE_INT_LOOPBACK);
          MSS_CAN_start(&g_mss_can_0_lo);

          ....

          return(0);
      }
  @endcode
 */
void
MSS_CAN_start
(
    mss_can_instance_t* this_can
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_stop() function is used to disable the CAN controller.

  NOTE: Interrupt flags status remain unaffected.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @return
    This function does not return a value.

  Example:
  @code
      int e51(void)
      {
          MSS_CAN_init(&g_mss_can_0_lo,(CAN_SPEED_16M_500K | CAN_AUTO_RESTART | \
                       CAN_LITTLE_ENDIAN),(pmss_can_config_reg)0,16,7);
          MSS_CAN_set_mode(&g_mss_can_0_lo,CANOP_MODE_INT_LOOPBACK);
          MSS_CAN_start(&g_mss_can_0_lo);

          ....
          ....

          MSS_CAN_stop(&g_mss_can_0_lo);

          return(0);
      }
  @endcode
 */
void
MSS_CAN_stop
(
    mss_can_instance_t* this_can
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_id() function returns the message identifier. Bits right
  justified based on message identifier (Extended identifier) type.

  @param pmsg
    The pmsg parameter is a pointer to the message object.

  @return
    This function returns message identifier.

  Example:
  @code
      int e51(void)
      {
           ...
           return_id = MSS_CAN_get_id(&pmsg);

           return(0);
      }
  @endcode
 */
uint32_t
MSS_CAN_get_id
(
    pmss_can_msgobject pmsg
);


/*-------------------------------------------------------------------------*//**
  The MSS_CAN_set_id() function returns ID bits left justified based on IDE
  type. IDE type might be either standard or extended.

  @param pmsg
    The pmsg parameter is a pointer to the message object.

  @return
    This function returns message identifier.

  Example:
  @code
      int e51(void)
      {
          ....
          pmsg->ID = 0x120;
          MSS_CAN_set_id(&pmsg);

          ....
          return(0);
      }
  @endcode
*/
uint32_t
MSS_CAN_set_id
(
    pmss_can_msgobject pmsg
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_msg_filter_mask() function  packs the ID, IDE, and RTR bits
  together as they are used in the message filter mask and returns packed
  identifier.

  @param id
    The id parameter is a 4 byte variable to hold message identifier.

  @param ide
    The ide parameter is a 1 byte variable to indicate IDE type. Acceptable
    values are as mentioned below:

    |  Value     |   Description                 |
    |------------|-------------------------------|
    |  0         | Standard format               |
    |  1         | Extended format               |

  @param rtr
    The rtr parameter is a 1 byte variable to indicate message type. Acceptable
    values are as mentioned below:

    |  Value     |   Description                 |
    |------------|-------------------------------|
    |   0        | Regular message (data frame)  |
    |   1        | RTR message (remote frame)    |

  @return
    This function returns packed id.

  Example:
  @code
      int e51(void)
      {
          ....

          MSS_CAN_get_msg_filter_mask( 0x120, 1, 0);

          ....
          return(0);
      }
  @endcode
 */
uint32_t
MSS_CAN_get_msg_filter_mask
(
    uint32_t id,
    uint8_t ide,
    uint8_t rtr
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_set_int_ebl() function enable specific interrupt based on
  irq_flag parameter.

  @param this_can
    This is a pointer to an mss_can_instance_t structure.

  @param irq_flag
    The irq_flag parameter is a 4 byte variable indicates Interrupt type.
    Possible values are:

    |  Constants             |  Description                                   |
    |------------------------|------------------------------------------------|
    | CAN_INT_GLOBAL         | Indicates to enable global interrupts          |
    | CAN_INT_ARB_LOSS       | Indicates arbitration loss interrupt           |
    | CAN_INT_OVR_LOAD       | Indicates overload message detected interrupt  |
    | CAN_INT_BIT_ERR        | Indicates bit error interrupt                  |
    | CAN_INT_STUFF_ERR      | Indicates bit stuffing error interrupt         |
    | CAN_INT_ACK_ERR        | Indicates acknowledge error interrupt          |
    | CAN_INT_FORM_ERR       | Indicates format error interrupt               |
    | CAN_INT_CRC_ERR        | Indicates CRC error interrupt                  |
    | CAN_INT_BUS_OFF        | Indicates bus off interrupt                    |
    | CAN_INT_RX_MSG_LOST    | Indicates received message lost interrupt      |
    | CAN_INT_TX_MSG         | Indicates message transmit interrupt           |
    | CAN_INT_RX_MSG         | Indicates receive message available interrupt  |
    | CAN_INT_RTR_MSG        | Indicates RTR auto-reply message sent interrupt|
    | CAN_INT_STUCK_AT_0     | Indicates stuck at dominant error interrupt    |
    | CAN_INT_SST_FAILURE    | Indicates single shot transmission failure     |
    |                        | interrupt                                      |

  @return
    This function does not return a value.

  Example:
  @code
      int e51(void)
      {
          ....

          MSS_CAN_set_int_ebl(&g_mss_can_0_lo,CAN_INT_TX_MSG);

          ....
          return(0);
      }
  @endcode
 */
void
MSS_CAN_set_int_ebl
(
    mss_can_instance_t* this_can,
    uint32_t irq_flag
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_clear_int_ebl() function disable specific interrupt based on
  irq_flag parameter.

  @param this_can
    This is a pointer to an mss_can_instance_t structure.

  @param irq_flag
    The irq_flag parameter is a 4 byte variable indicates Interrupt type.
    Possible values are:

    |  Constant              |  Description                                   |
    |------------------------|------------------------------------------------|
    | CAN_INT_GLOBAL         | Indicates to enable global interrupts          |
    | CAN_INT_ARB_LOSS       | Indicates arbitration loss interrupt           |
    | CAN_INT_OVR_LOAD       | Indicates overload message detected interrupt  |
    | CAN_INT_BIT_ERR        | Indicates bit error interrupt                  |
    | CAN_INT_STUFF_ERR      | Indicates bit stuffing error interrupt         |
    | CAN_INT_ACK_ERR        | Indicates acknowledge error interrupt          |
    | CAN_INT_FORM_ERR       | Indicates format error interrupt               |
    | CAN_INT_CRC_ERR        | Indicates CRC error interrupt                  |
    | CAN_INT_BUS_OFF        | Indicates bus off interrupt                    |
    | CAN_INT_RX_MSG_LOST    | Indicates received message lost interrupt      |
    | CAN_INT_TX_MSG         | Indicates message transmit interrupt           |
    | CAN_INT_RX_MSG         | Indicates receive message available interrupt  |
    | CAN_INT_RTR_MSG        | Indicates RTR auto-reply message sent interrupt|
    | CAN_INT_STUCK_AT_0     | Indicates stuck at dominant error interrupt    |
    | CAN_INT_SST_FAILURE    | Indicates single shot transmission failure     |
    |                        | interrupt                                      |

  @return
    This function does not return a value.

  Example:
  @code
      int e51(void)
      {
          ....

          MSS_CAN_clear_int_ebl(&g_mss_can_0_lo,CAN_INT_TX_MSG);

          ....
          return(0);
      }
  @endcode
 */
void
MSS_CAN_clear_int_ebl
(
    mss_can_instance_t* this_can,
    uint32_t irq_flag
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_global_int_ebl() function returns the status of global
  interrupt enable flag.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @return
    This function returns global interrupt enable flag status.

  Example:
  @code
      int e51(void)
      {
          ....

          MSS_CAN_get_global_int_ebl(&g_mss_can_0_lo);
          ....
          return(0);
      }
  @endcode
 */
uint32_t
MSS_CAN_get_global_int_ebl
(
    mss_can_instance_t* this_can
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_int_ebl() function returns the status of interrupt enable
  flags.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @return
    This function returns interrupt enable flag status.

  Example:
  @code
      int e51(void)
      {
          ....

          MSS_CAN_get_int_ebl(&g_mss_can_0_lo);
          ....
          return(0);
      }
  @endcode
 */
uint32_t
MSS_CAN_get_int_ebl
(
    mss_can_instance_t* this_can
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_clear_int_status() function  clears the selected interrupt flags.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param irq_flag
    The irq_flag parameter is a 4 byte variable indicates Interrupt type.
    Possible values are:
    |  Constants             |  Description                                   |
    |------------------------|------------------------------------------------|
    | CAN_INT_GLOBAL         | Indicates to enable global interrupts          |
    | CAN_INT_ARB_LOSS       | Indicates arbitration loss interrupt           |
    | CAN_INT_OVR_LOAD       | Indicates overload message detected interrupt  |
    | CAN_INT_BIT_ERR        | Indicates bit error interrupt                  |
    | CAN_INT_STUFF_ERR      | Indicates bit stuffing error interrupt         |
    | CAN_INT_ACK_ERR        | Indicates acknowledge error interrupt          |
    | CAN_INT_FORM_ERR       | Indicates format error interrupt               |
    | CAN_INT_CRC_ERR        | Indicates CRC error interrupt                  |
    | CAN_INT_BUS_OFF        | Indicates bus off interrupt                    |
    | CAN_INT_RX_MSG_LOST    | Indicates received message lost interrupt      |
    | CAN_INT_TX_MSG         | Indicates message transmit interrupt           |
    | CAN_INT_RX_MSG         | Indicates receive message available interrupt  |
    | CAN_INT_RTR_MSG        | Indicates RTR auto-reply message sent interrupt|
    | CAN_INT_STUCK_AT_0     | Indicates stuck at dominant error interrupt    |
    | CAN_INT_SST_FAILURE    | Indicates single shot transmission failure     |
    |                        | interrupt                                      |

  @return
    This function does not return a value.

  Example:
  @code
      int e51(void)
      {
          ....
          MSS_CAN_clear_int_status(&g_mss_can_0_lo,CAN_INT_RX_MSG);
          ....
          return(0);
      }
  @endcode
 */
void
MSS_CAN_clear_int_status
(
    mss_can_instance_t* this_can,
    uint32_t irq_flag
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_int_status() function returns the status of interrupts.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @return
    This function returns status of existed interrupts.

  Example:
  @code
      int e51(void)
      {
          ....

          MSS_CAN_get_int_status(&g_mss_can_0_lo);
          ....
          return(0);
      }
  @endcode
 */
uint32_t
MSS_CAN_get_int_status
(
    mss_can_instance_t* this_can
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_set_rtr_message_n () function  loads mailbox with the given CAN
  message. This message will be sent out after receipt of a RTR message
  request. It verifies that whether the given mailbox is configured for Full
  CAN or not and also checks for RTR auto-reply is enabled or not.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param mailbox_number
    The mailbox_number parameter is a variable to hold the mailbox number to
    be used for message transfer.

  @param pmsg
    The pmsg parameter is a pointer to the message object.

  @return
    This function returns CAN_OK on successful execution, otherwise it will
    returns following error codes:

    |  Constants            |  Description                                |
    |-----------------------|---------------------------------------------|
    | CAN_BASIC_CAN_MAILBOX | Indicates that mailbox is configured for    |
    |                       | Basic CAN operation                         |
    | CAN_NO_RTR_MAILBOX    | Indicates that there is no mailbox for      |
    |                       | remote transmit request (RTR) frame         |


  Example:
  @code
      e51()
      {
          ...

          pmsg->ID = 0x120;
          pmsg->DATALOW = 0xAA5555AA;
          pmsg->DATAHIGH = 0xAA5555AA;

          MSS_CAN_set_rtr_message_n(&g_mss_can_0_lo, 5, &pmsg);

          ...
      }
  @endcode
 */
uint8_t
MSS_CAN_set_rtr_message_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number,
    pmss_can_msgobject pmsg
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_rtr_message_abort_n() function aborts a RTR message transmit
  request on mailbox mailbox_number and checks that message abort was
  successful.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param mailbox_number
    The mailbox_number parameter is a variable to hold the mailbox number to
    be used for message transfer.

  @return
    This function returns CAN_OK on successful execution, otherwise it will
    returns following error codes:

    |  Constants            |  Description                                |
    |-----------------------|---------------------------------------------|
    | CAN_ERR               | Indicates error condition                   |
    | CAN_BASIC_CAN_MAILBOX | Indicates that mailbox is configured for    |
    |                       | Basic CAN operation                         |

  Example:
  @code
      e51()
      {
          ...

          ret_status = MSS_CAN_get_rtr_message_abort_n((&g_mss_can_0_lo, 6);

          ...
      }
  @endcode
*/
uint8_t
MSS_CAN_get_rtr_message_abort_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_config_buffer() function configures receive mailboxes initialized
  for Basic CAN operation.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param pfilter
    The pfilter parameter is a pointer to the CAN message filter structure.

  @return
    This function returns CAN_OK on successful execution, otherwise it will
    returns following error codes:

    |  Constants            |  Description                                |
    |-----------------------|---------------------------------------------|
    | CAN_NO_MSG            | Indicates that there is no message received |
    | CAN_INVALID_MAILBOX   | Indicates invalid mailbox number            |


  Example:
  @code
      e51()
      {
          ...
          pfilter.ACR.L=0x00000000;
          pfilter.AMR.L= 0xFFFFFFFF;
          pfilter.AMCR_D.MASK= 0xFFFF;
          pfilter.AMCR_D.CODE= 0x00;

          ret_status = MSS_CAN_config_buffer(&g_mss_can_0_lo, &pfilter);
          ...
      }
  @endcode
 */
uint8_t
MSS_CAN_config_buffer
(
    mss_can_instance_t* this_can,
    pmss_can_filterobject pfilter
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_config_buffer_n() function  configures the receive mailbox
  specified in mailbox_number. The function checks that the mailbox is set for
  Full CAN operation, if not it return with error code.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param mailbox_number
    The mailbox_number parameter is a variable to hold the mailbox number to
    be used for message transfer.

  @param pmsg
    The pmsg parameter is a pointer to the message object.

  @return
    This function returns CAN_OK on successful execution, otherwise it will
    returns following error codes
      - CAN_BASIC_CAN_MAILBOX

    |  Constants            |  Description                                |
    |-----------------------|---------------------------------------------|
    | CAN_NO_MSG            | Indicates that there is no message received |
    | CAN_INVALID_MAILBOX   | Indicates invalid mailbox number            |
    | CAN_BASIC_CAN_MAILBOX | Indicates that mailbox is configured for    |
    |                       | Basic CAN operation                         |

  Example:
  @code
      e51()
      {
          ...
          rx_msg.ID = 0x200;
          rx_msg.DATAHIGH = 0u;
          rx_msg.DATALOW = 0u;
          rx_msg.AMR.L = 0xFFFFFFFF;
          rx_msg.ACR.L = 0x00000000;
          rx_msg.AMR_D = 0x0000FFFF;
          rx_msg.ACR_D = 0x00000000;
          rx_msg.RXB.DLC = 8u;
          rx_msg.RXB.IDE = 0;

          ret_status = MSS_CAN_config_buffer_n(&g_mss_can_0_lo, 3, &rx_msg);
          ...
      }
  @endcode
*/
uint8_t
MSS_CAN_config_buffer_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number,
    pmss_can_rxmsgobject pmsg
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_message_n() function read message from the receive mailbox
  specified in "mailbox_number" parameter and returns status of operation.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param mailbox_number
    The mailbox_number parameter is a variable to hold the mailbox number to
    be used for message transfer.

  @param pmsg
    The pmsg parameter is a pointer to the message object that will hold the
    received message.

  @return
    This function returns CAN_VALID_MSG on successful execution, otherwise it
    will returns following error codes:

    |  Constants            |  Description                                |
    |-----------------------|---------------------------------------------|
    | CAN_NO_MSG            | Indicates that there is no message received |
    | CAN_BASIC_CAN_MAILBOX | Indicates that mailbox is configured for    |
    |                       | Basic CAN operation                         |

  Example:
  @code
      e51()
      {
          pmss_can_msgobject msg;
          ...

          ret_status = MSS_CAN_get_message_n(&g_mss_can_0_lo, 3, &msg);
          ...
      }
  @endcode
 */
uint8_t
MSS_CAN_get_message_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number,
    pmss_can_msgobject pmsg
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_message() function read message from the first mailbox set
  for Basic CAN  operation that contains a message. Once the message has been
  read from the mailbox, the message receipt is acknowledged.
  Note: Since neither a hardware nor a software FIFO exists, message inversion
        might happen (example, a newer message might be read from the receive
        buffer prior to an older message).

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param pmsg
    The pmsg parameter is a pointer to the message object, that will hold the
    received message.

  @return
    This function returns CAN_VALID_MSG on successful execution, otherwise it
    will returns following error codes

    |  Constants            |  Description                                |
    |-----------------------|---------------------------------------------|
    | CAN_NO_MSG            | Indicates that there is no message received |
    | CAN_INVALID_MAILBOX   | Indicates invalid mailbox number            |

  Example:
  @code
      e51()
      {
          pmss_can_msgobject rx_buf;
          ...
          if(CAN_VALID_MSG == MSS_CAN_get_message_av(&g_mss_can_0_lo))
          {
             if(CAN_VALID_MSG != MSS_CAN_get_message(&g_mss_can_0_lo, &rx_buf))
             {
                  ....
             }
          }
          ...
      }
  @endcode
 */
uint8_t
MSS_CAN_get_message
(
    mss_can_instance_t* this_can,
    pmss_can_msgobject pmsg
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_message_av() function indicates if receive buffer contains a
  new message in Basic CAN operation.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @return
    This function returns CAN_VALID_MSG on successful execution, otherwise it
    will returns following error codes:

    |  Constants            |  Description                                |
    |-----------------------|---------------------------------------------|
    | CAN_NO_MSG            | Indicates that there is no message received |
    | CAN_INVALID_MAILBOX   | Indicates invalid mailbox number            |

  Example:
  @code
      e51()
      {
          pmss_can_msgobject rx_buf;
          ...
          if(CAN_VALID_MSG == MSS_CAN_get_message_av(&g_mss_can_0_lo))
          {
             MSS_CAN_get_message(&g_mss_can_0_lo, &rx_buf);
          }
          ...
      }
  @endcode
 */
uint8_t
MSS_CAN_get_message_av
(
    mss_can_instance_t* this_can
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_send_message_n() function sends a message using mailbox
  "mailbox_number". The function verifies that this mailbox is configured for
  Full CAN operation and is empty.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param mailbox_number
    The mailbox_number parameter is a variable to hold the mailbox number to
    be used for message transfer.

  @param pmsg
    The pmsg parameter is a pointer to the message object that holds the CAN
    message to transmit.

  @return
    This function returns CAN_VALID_MSG on successful execution, otherwise it
    will return the following error codes:

    |  Constants            |  Description                                |
    |-----------------------|---------------------------------------------|
    | CAN_NO_MSG            | Indicates that there is no message received |
    | CAN_INVALID_MAILBOX   | Indicates invalid mailbox number            |

  Example:
  @code
      e51()
      {
          pmss_can_msgobject pmsg;
          ...
          pmsg.ID=0x120;
          pmsg.DATALOW = 0x55555555;
          pmsg.DATAHIGH = 0x55555555;
          pmsg.L = ((0<<20)| 0x00080000);

          if (CAN_OK != MSS_CAN_send_message_n(&g_mss_can_0_lo, 0, &pmsg))
          {
             ...
          }

          ...
      }
  @endcode
 */
uint8_t
MSS_CAN_send_message_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number,
    pmss_can_msgobject pmsg
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_send_message_abort_n() function aborts a message transmit
  request for the specified mailbox number in mailbox_number and checks that
  message abort status.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param mailbox_number
    The mailbox_number parameter is a variable to hold the mailbox number to
    be used for message transfer.

  @return
    This function returns CAN_OK on successful execution, otherwise it
    will return the following error codes:

    |  Constants            |  Description                                |
    |-----------------------|---------------------------------------------|
    | CAN_ERR               | Indicates error condition                   |
    | CAN_BASIC_CAN_MAILBOX | Indicates that mailbox is configured for    |
    |                       | Basic CAN operation                         |

  Example:
  @code
      e51()
      {
          ...
          if (CAN_OK != MSS_CAN_send_message_abort_n(&g_mss_can_0_lo, 0))
          {
             ...
          }
          ...
      }
  @endcode
 */
uint8_t
MSS_CAN_send_message_abort_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_send_message_ready() function will identify the availability of
  mailbox to fill with new message in basic CAN operation.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @return
    This function returns CAN_OK on successful identification of free mailbox,
    otherwise it will returns following error codes:

    |  Constants            |  Description                                |
    |-----------------------|---------------------------------------------|
    | CAN_ERR               | Indicates error condition                   |
    | CAN_INVALID_MAILBOX   | Indicates invalid mailbox number            |

  Example:
  @code
      e51()
      {
          pmss_can_msgobject pmsg;
          ...
          pmsg.ID = 0x120;
          pmsg.DATALOW = 0x55555555;
          pmsg.DATAHIGH = 0x55555555;
          pmsg.L = ((0 << 20)| 0x00080000);

          if(CAN_OK == MSS_CAN_send_message_ready(&g_mss_can_0_lo))
          {
              MSS_CAN_send_message(&g_mss_can_0_lo, &pmsg);
          }
          ...
      }
  @endcode
 */
uint8_t
MSS_CAN_send_message_ready
(
    mss_can_instance_t* this_can
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_send_message() function will copy the data to the first available
  mailbox set for Basic CAN operation and send data on to the bus.
  Note: Since neither a hardware nor a software FIFO exists, message inversion
        might happen (example, a newer message might be send from the transmit
        buffer prior to an older message).

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param pmsg
    The pmsg parameter is a pointer to the message object that holds the CAN
    message to transmit.

  @return
    This function returns CAN_OK on successful identification of free mailbox,
    otherwise it will returns following error codes:

    |  Constants            |  Description                                |
    |-----------------------|---------------------------------------------|
    | CAN_ERR               | Indicates error condition                   |
    | CAN_INVALID_MAILBOX   | Indicates invalid mailbox number            |

  Example:
  @code
      e51()
      {
          pmss_can_msgobject pmsg;
          ...
          pmsg.ID = 0x120;
          pmsg.DATALOW = 0x55555555;
          pmsg.DATAHIGH = 0x55555555;
          pmsg.L = ((0 << 20)| 0x00080000);

          if (CAN_OK == MSS_CAN_send_message_ready(&g_mss_can_0_lo))
          {
              if (CAN_OK != MSS_CAN_send_message(&g_mss_can_0_lo, &pmsg))
              {
                 ...
              }
          }
          ...
      }
  @endcode
 */
uint8_t
MSS_CAN_send_message
(
    mss_can_instance_t* this_can,
    pmss_can_msgobject pmsg
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_mask_n() function returns the message filter settings of the
  selected receive mailbox. The function is valid for Full CAN operation only.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param mailbox_number
    The mailbox_number parameter is a variable to hold the mailbox number to
    be used for message transfer.

  @param pamr
    The pamr parameter is a pointer to the acceptance mask.

  @param pacr
    The pacr parameter is a pointer to the acceptance code.

  @param pdta_amr
    The pdta_amr parameter is a pointer to the acceptance mask of first two
    data bytes.

  @param pdta_acr
    The pdta_acr parameter is a pointer to the acceptance code of first two
    data bytes.

  @return
    This function will returns the following error codes:

    |  Constants            |  Description                                |
    |-----------------------|---------------------------------------------|
    | CAN_OK                | Indicates there is no error                 |
    | CAN_BASIC_CAN_MAILBOX | Indicates that mailbox is configured for    |
    |                       | Basic CAN operation                         |

  Example:
  @code
      e51()
      {
          ...

          if (CAN_OK != MSS_CAN_get_mask_n(&g_mss_can_0_lo,mailbox_number,&pamr,&pacr,
                                           &pdamr, &pdacr))
          {
              ...
          }
          ...
      }
  @endcode
*/
uint8_t
MSS_CAN_get_mask_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number,
    uint32_t *pamr,
    uint32_t *pacr,
    uint16_t *pdta_amr,
    uint16_t *pdta_acr
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_set_mask_n() function configures the message filter settings for
  the selected receive mailbox. The function is valid for Full CAN operation
  only.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param mailbox_number
    The mailbox_number parameter is a variable to hold the mailbox number to
    be used for message transfer.

  @param amr
    The amr parameter is a variable to hold the acceptance mask.

  @param acr
    The acr parameter is a variable to hold the acceptance code.

  @param dta_amr
    The dta_amr parameter is a variable to hold the acceptance mask of first two
    data bytes.

  @param dta_acr
    The dta_acr parameter is a variable to hold the acceptance code of first two
    data bytes.

  @return
    This function returns the following error codes:

    |  Constants            |  Description                                |
    |-----------------------|---------------------------------------------|
    | CAN_OK                | Indicates there is no error                 |
    | CAN_BASIC_CAN_MAILBOX | Indicates that mailbox is configured for    |
    |                       | Basic CAN operation                         |

  Example:
  @code
      e51()
      {
          ...
          if (CAN_OK != MSS_CAN_set_mask_n(&g_mss_can_0_lo,mailbox_number,&pamr,&pacr,
                                           &pdamr, &pdacr))
          {
              ...
          }
          ...
      }
  @endcode
 */
uint8_t
MSS_CAN_set_mask_n
(
    mss_can_instance_t* this_can,
    uint8_t mailbox_number,
    uint32_t amr,
    uint32_t acr,
    uint16_t dta_amr,
    uint16_t dta_acr
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_rx_buffer_status() function returns the buffer status of all
  receive (32) mailboxes.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @return
    This function returns status of receive buffers (32 buffers).

  Example:
  @code
      e51()
      {
          uint32_t return_status=0;
          ...
          return_status = MSS_CAN_get_rx_buffer_status(&g_mss_can_0_lo);
          ...
      }
  @endcode
 */
uint32_t
MSS_CAN_get_rx_buffer_status
(
    mss_can_instance_t* this_can
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_tx_buffer_status() function returns the buffer status of all
  transmit(32) mailboxes.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @return
    This function returns status of transmit buffers (32 buffers).

  Example:
  @code
      e51()
      {
          uint32_t return_status = 0;
          ...
          return_status = MSS_CAN_get_tx_buffer_status(&g_mss_can_0_lo);
          ...
      }
  @endcode
*/
uint32_t
MSS_CAN_get_tx_buffer_status
(
    mss_can_instance_t* this_can
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_error_status() function returns the present error state of
  the CAN controller. Error state might be error active or error passive or
  bus-off.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @param status
    The status parameter is a pointer to hold the content of error status
    register.

  @return
    The function shall return the following codes:
    | Codes  |  Descriptions                 |
    |--------|-------------------------------|
    |  0     | error active                  |
    |  1     | error passive                 |
    |  2     | bus-off                       |

  Example:
  @code
      e51()
      {
          uint8_t return_status = 0;
          ...
          return_status = MSS_CAN_get_error_status(&g_mss_can_0_lo);
          ...
      }
  @endcode
 */
uint8_t
MSS_CAN_get_error_status
(
    mss_can_instance_t* this_can,
    uint32_t* status
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_rx_error_count() function returns the current receive error
  counter value. Counter value ranges from 0x00 - 0xFF.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @return
    This function returns the receive error counter value.

  Example:
  @code
      e51()
      {
          uint32_t return_status = 0;
          ...
          return_status = MSS_CAN_get_rx_error_count(&g_mss_can_0_lo);
          ...
      }
  @endcode
 */
uint32_t
MSS_CAN_get_rx_error_count
(
    mss_can_instance_t* this_can
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_rx_gte96() function provides information about receive
  error count. It identifies that receive error count is greater than or equal
  to 96, and reports 1 if count exceeds 96.

  @param this_can
   The this_can parameter is a pointer to the mss_can_instance_t structure.

  @return
    This function returns following values:

    | Value |  Description                                         |
    |-------|------------------------------------------------------|
    |  0    | if receive error count less than 96.                 |
    |  1    | if receive error count greater than or equals to 96. |

  Example:
  @code
      e51()
      {
          uint32_t return_status = 0;
          ...
          return_status = MSS_CAN_get_rx_gte96(&g_mss_can_0_lo);
          ...
      }
  @endcode
 */
uint32_t
MSS_CAN_get_rx_gte96
(
    mss_can_instance_t* this_can
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_tx_error_count() function returns the current transmit error
  counter value. Counter value ranges from 0x00 - 0xFF.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @return
    This function returns the transmit error counter value.

  Example:
  @code
      e51()
      {
          uint32_t return_status = 0;
          ...
          return_status = MSS_CAN_get_tx_error_count(&g_mss_can_0_lo);
          ...
      }
  @endcode
 */
uint32_t
MSS_CAN_get_tx_error_count
(
    mss_can_instance_t* this_can
);

/*-------------------------------------------------------------------------*//**
  The MSS_CAN_get_tx_gte96() function provides information about transmit
  error count. It identifies that transmit error count is greater than or equals
  to 96, and reports 1 if count exceeds 96.

  @param this_can
    The this_can parameter is a pointer to the mss_can_instance_t structure.

  @return
    This function returns following values:

    | Value |  Description                                          |
    |-------|-------------------------------------------------------|
    |  0    | if transmit error count less than 96.                 |
    |  1    | if transmit error count greater than or equals to 96. |

  Example:
  @code
      e51()
      {
          uint32_t return_status = 0;
          ...
          return_status = MSS_CAN_get_tx_gte96(&g_mss_can_0_lo);
          ...
      }
  @endcode
 */
uint32_t
MSS_CAN_get_tx_gte96
(
    mss_can_instance_t* this_can
);

#ifdef __cplusplus
}
#endif

#endif /* MSS_CAN_H_ */
