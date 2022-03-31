/*******************************************************************************
 * (c) Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 * All rights reserved.
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
 * PolarFire SoC Microprocessor Subsystem I2C bare metal software driver
 * public API.
 *
 * SVN $Revision$
 * SVN $Date$
 */
/*=========================================================================*//**
  @mainpage PolarFire SoC MSS I2C Bare Metal Driver.
  ==============================================================================
  Introduction
  ==============================================================================
  The PolarFire SoC microcontroller subsystem (MSS) includes two I2C peripherals
  for serial communication. This driver provides a set of functions for
  controlling the MSS I2Cs as part of a bare metal system where no
  operating system is available. The driver can be adapted for use as part of an
  operating system, but the implementation of the adaptation layer between the
  driver and the operating system's driver model is outside the scope of this
  driver.

  ==============================================================================
  Hardware Flow Dependencies
  ==============================================================================
  The configuration of all features of the MSS I2C peripherals is covered by
  this driver with the exception of the PolarFire SoC IOMUX configuration.
  PolarFire SoC allows multiple non-concurrent uses of some external pins
  through IOMUX configuration. This feature allows optimization of external pin
  usage by assigning external pins for use by either the microcontroller
  subsystem or the FPGA fabric. The MSS I2C serial signals are routed through
  IOMUXs to the PolarFire SoC device external pins. The MSS I2C serial signals
  may also be routed through IOMUXs to the PolarFire SoC FPGA fabric. For more
  information on IOMUX, refer to the IOMUX section of the PolarFire SoC
  Microcontroller Subsystem (MSS) User's Guide.

  The IOMUXs are configured using the PolarFire SoC MSS configurator tool. You
  must ensure that the MSS I2C peripherals are enabled and configured in the
  PolarFire SoC MSS configurator if you wish to use them. For more information
  on IOMUXs, refer to the IOMUX section of the PolarFire SoC Microcontroller
  Subsystem (MSS) User's Guide.

  On PolarFire SoC an AXI switch forms a bus matrix interconnect among
  multiple masters and multiple slaves. Five RISC-V CPUs connect to the Master
  ports M10 to M14 of the AXI switch. By default, all the APB peripherals are
  accessible on AXI-Slave 5 of the AXI switch via the AXI to AHB and AHB to APB
  bridges (referred as main APB bus). However, to support logical separation in
  the Asymmetric Multi-Processing (AMP) mode of operation, the APB peripherals
  can alternatively be accessed on the AXI-Slave 6 via the AXI to AHB and AHB
  to APB bridges (referred as the AMP APB bus).

  Application must make sure that the desired I2C instance is connected
  appropriately on the desired APB bus by configuring the PolarFire SoC system
  registers (SYSREG) as per the application need and that the appropriate data
  structures are provided to this driver as parameter to the functions provided
  by this driver.

  The base address and register addresses are defined in this driver as
  constants. The interrupt number assignment for the MSS I2C peripherals are
  defined as constants in the MPFS HAL. You must ensure that the latest MPFS
  HAL is included in the project settings of the SoftConsole tool chain and
  that it is generated into your project.

  ==============================================================================
  Theory of Operation
  ==============================================================================
  The MSS I2C driver functions are grouped into the following categories:
    - Initialization and configuration functions
    - Interrupt control
    - I2C slave address configuration
    - I2C master operations - functions to handle write, read and write-read
                              transactions
    - I2C slave operations  - functions to handle write, read and write-read
                              transactions
    - Mixed master and slave operations
    - SMBus interface configuration and control

  --------------------------------
  Initialization and Configuration
  --------------------------------
    The MSS I2C driver is initialized through a call to the MSS_I2C_init()
    function. This function takes the MSS I2C's configuration as parameters.
    The MSS_I2C_init() function must be called before any other MSS I2C driver
    functions can be called. The first parameter of the MSS_I2C_init() function
    is a pointer to one of four global data structures used by the driver to
    store state information for each MSS I2C. A pointer to these data
    structures is also used as the first parameter to any of the driver
    functions to identify which MSS I2C will be used by the called function.
    The names of these data structures are
              g_mss_i2c0_lo
              g_mss_i2c0_hi
              g_mss_i2c1_lo
              g_mss_i2c1_hi
    Therefore any call to an MSS I2C driver function should be of the form
    MSS_I2C_function_name( &g_mss_i2c0_lo, ... ) or
    MSS_I2C_function_name( &g_mss_i2c1_lo, ... ).

    The MSS_I2C_init() function call for each MSS I2C also takes the I2C serial
    address assigned to the MSS I2C and the serial clock divider to be used to
    generate its I2C clock as configuration parameters.

  --------------------------------
  Interrupt Control
  --------------------------------
    The MSS I2C driver is interrupt driven and it enables and disables the
    generation of INT interrupts by MSS I2C at various times when it is
    operating. The driver automatically handles MSS I2C interrupts internally,
    including enabling, disabling and clearing MSS I2C interrupts in the
    RISC-V interrupt controller when required.

    The function MSS_I2C_register_write_handler() is used to register a write
    handler function with the MSS I2C driver that it calls on completion of an
    I2C write transaction by the MSS I2C slave. It is your responsibility to
    create and register the implementation of this handler function that
    processes or triggers the processing of the received data.

    The SMBSUS and SMBALERT interrupts are related to the SMBus interface and
    are enabled and disabled through MSS_I2C_enable_smbus_irq() and
    MSS_I2C_disable_smbus_irq() respectively. It is your responsibility to
    create interrupt handler functions in your application to get the desired
    response for the SMBus interrupts.

  --------------------------------
  I2C Slave Address Configuration
  --------------------------------
    The PolarFire SoC MSS I2C can respond to two slave addresses:
      - Slave address - This is the address that is used for accessing an MSS
                        I2C peripheral when it acts as a slave in I2C
                        transactions. You must configure the slave address via
                        MSS_I2C_init().

      - General call address - An MSS I2C slave can be configured to respond to
                        a broadcast command by a master transmitting the general
                        call address of 0x00. Use the MSS_I2C_set_gca() function
                        to enable the slave to respond to the general call
                        address. If the I2C slave is not required to respond to
                        the general call address, disable this address by
                        calling MSS_I2C_clear_gca().

  --------------------------------
  Transaction Types
  --------------------------------
    The MSS I2C driver is designed to handle three types of I2C transactions:
      Write transactions
      Read transactions
      Write-read transactions

    Write transaction
      The master I2C device initiates a write transaction by sending a START bit
      as soon as the bus becomes free. The START bit is followed by the 7-bit
      serial address of the target slave device followed by the read/write bit
      indicating the direction of the transaction. The slave acknowledges the
      receipt of its address with an acknowledge bit. The master sends data one
      byte at a time to the slave, which must acknowledge the receipt of each
      byte for the next byte to be sent. The master sends a STOP bit to complete
      the transaction. The slave can abort the transaction by replying with a
      non-acknowledge bit instead of an acknowledge bit.

      The application programmer can choose not to send a STOP bit at the end of
      the transaction causing the next transaction to begin with a repeated
      START bit.

    Read transaction
      The master I2C device initiates a read transaction by sending a START bit
      as soon as the bus becomes free. The START bit is followed by the 7-bit
      serial address of the target slave device followed by the read/write bit
      indicating the direction of the transaction. The slave acknowledges the
      receipt of its slave address with an acknowledge bit. The slave sends data
      one byte at a time to the master, which must acknowledge receipt of each
      byte for the next byte to be sent. The master sends a non-acknowledge bit
      following the last byte it wishes to read followed by a STOP bit.

      The application programmer can choose not to send a STOP bit at the end of
      the transaction causing the next transaction to begin with a repeated
      START bit.

    Write-read transaction
      The write-read transaction is a combination of a write transaction
      immediately followed by a read transaction. There is no STOP bit between
      the write and read phases of a write-read transaction. A repeated START
      bit is sent between the write and read phases.

      Whilst the write handler is being executed, the slave holds the clock line
      low to stretch the clock until the response is ready.

      The write-read transaction is typically used to send a command or offset
      in the write transaction specifying the logical data to be transferred
      during the read phase.

      The application programmer can choose not to send a STOP bit at the end of
      the transaction causing the next transaction to begin with a repeated
      START bit.

  --------------------------------
  Master Operations
  --------------------------------
    The application can use the MSS_I2C_write(), MSS_I2C_read() and
    MSS_I2C_write_read() functions to initiate an I2C bus transaction. The
    application can then wait for the transaction to complete using the
    MSS_I2C_wait_complete() function or poll the status of the I2C transaction
    using the MSS_I2C_get_status() function until it returns a value different
    from MSS_I2C_IN_PROGRESS or register a call back function using
    MSS_I2C_register_transfer_completion_handler() to notify the completion of
    the previously initiated I2C transfer. The MSS_I2C_system_tick() function
    can be used to set a time base for the MSS_I2C_wait_complete() function's
    time out delay.

  --------------------------------
  Slave Operations
  --------------------------------
    The configuration of the MSS I2C driver to operate as an I2C slave requires
    the use of the following functions:
       - MSS_I2C_set_slave_tx_buffer()
       - MSS_I2C_set_slave_rx_buffer()
       - MSS_I2C_set_slave_mem_offset_length()
       - MSS_I2C_register_write_handler()
       - MSS_I2C_enable_slave()

    Use of all functions is not required if the slave I2C does not need to
    support all types of I2C read transactions. The subsequent sections list the
    functions that must be used to support each transaction type.

    Responding to read transactions
      The following functions are used to configure the MSS I2C driver to
      respond to I2C read transactions:
        - MSS_I2C_set_slave_tx_buffer()
        - MSS_I2C_enable_slave()

      The function MSS_I2C_set_slave_tx_buffer() specifies the data buffer that
      will be transmitted when the I2C slave is the target of an I2C read
      transaction. It is then up to the application to manage the content of
      that buffer to control the data that will be transmitted to the I2C master
      as a result of the read transaction.

      The function MSS_I2C_enable_slave() enables the MSS I2C hardware instance
      to respond to I2C transactions. It must be called after the MSS I2C driver
      has been configured to respond to the required transaction types.

    Responding to write transactions
      The following functions are used to configure the MSS I2C driver to
      respond to I2C write transactions:
        - MSS_I2C_set_slave_rx_buffer()
        - MSS_I2C_register_write_handler()
        - MSS_I2C_enable_slave()

      The function MSS_I2C_set_slave_rx_buffer() specifies the data buffer that
      will be used to store the data received by the I2C slave when it is the
      target an I2C  write transaction.

      The function MSS_I2C_register_write_handler() specifies the handler
      function that must be called on completion of the I2C write transaction.
      It is this handler function that will process or trigger the processing of
      the received data.

      The function MSS_I2C_enable_slave() enables the MSS I2C hardware instance
      to respond to I2C transactions. It must be called after the MSS I2C driver
      has been configured to respond to the required transaction types.

    Responding to write-read transactions
      The following functions are used to configure the MSS I2C driver to
      respond to write-read transactions:
        - MSS_I2C_set_slave_mem_offset_length()
        - MSS_I2C_set_slave_tx_buffer()
        - MSS_I2C_set_slave_rx_buffer()
        - MSS_I2C_register_write_handler()
        - MSS_I2C_enable_slave()

      The function MSS_I2C_set_slave_mem_offset_length() specifies the number of
      bytes expected by the I2C slave during the write phase of the write-read
      transaction.

      The function MSS_I2C_set_slave_tx_buffer() specifies the data that will be
      transmitted to the I2C master during the read phase of the write-read
      transaction. The value received by the I2C slave during the write phase of
      the transaction will be used as an index into the transmit buffer
      specified by this function to decide which part of the transmit buffer
      will be transmitted to the I2C master as part of the read phase of the
      write-read transaction.

      The function MSS_I2C_set_slave_rx_buffer() specifies the data buffer that
      will be used to store the data received by the I2C slave during the write
      phase of the write-read transaction. This buffer must be at least large
      enough to accommodate the number of bytes specified through the
      MSS_I2C_set_slave_mem_offset_length() function.

      The function MSS_I2C_register_write_handler() can optionally be used to
      specify a handler function that is called on completion of the write phase
      of the I2C write-read transaction. If a handler function is registered, it
      is responsible for processing the received data in the slave receive
      buffer and populating the slave transmit buffer with the data that will be
      transmitted to the I2C master as part of the read phase of the write-read
      transaction.

      The function MSS_I2C_enable_slave() enables the MSS I2C hardware instance
      to respond to I2C transactions. It must be called after the MSS I2C driver
      has been configured to respond to the required transaction types.

  --------------------------------
  Mixed Master and Slave Operations
  --------------------------------
      The MSS I2C device supports mixed master and slave operations. If the MSS
      I2C slave has a transaction in progress and your application attempts to
      begin a master mode transaction, the MSS I2C driver queues the master mode
      transaction until the bus is released and the MSS I2C can switch to master
      mode and acquire the bus. The MSS I2C master then starts the previously
      pended transaction.

  --------------------------------
  SMBus Interface Configuration and Control
  --------------------------------
    The MSS I2C driver enables the MSS I2C peripherals SMBus functionality
    using the MSS_I2C_smbus_init() function.

    The MSS_I2C_suspend_smbus_slave() function is used, with a master mode MSS
    I2C, to force slave devices on the SMBus to enter their power-down/suspend
    mode.

    The MSS_I2C_resume_smbus_slave() function is used to end the suspend
    operation on the SMBus.

    The MSS_I2C_reset_smbus() function is used, with a master mode MSS I2C, to
    force all devices on the SMBus to reset their SMBUs interface.

    The MSS_I2C_set_smsbus_alert() function is used, by a slave mode MSS I2C, to
    force communication with the SMBus master. Once communications with the
    master is initiated, the MSS_I2C_clear_smsbus_alert() function is used to
    clear the alert condition.

    The MSS_I2C_enable_smbus_irq() and MSS_I2C_disable_smbus_irq() functions are
    used to enable and disable the SMBSUS and SMBALERT SMBus interrupts.

 *//*=========================================================================*/

#ifndef MSS_I2C_H_
#define MSS_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "mss_plic.h"
#include "mss_i2c_regs.h"
#include "hal/hal.h"

/*-------------------------------------------------------------------------*//**
  The mss_i2c_clock_divider_t type is used to specify the divider to be applied
  to the MSS I2C PCLK or BCLK signal in order to generate the I2C clock.
  The MSS_I2C_BCLK_DIV_8 value selects a clock frequency based on division of
  BCLK, all other values select a clock frequency based on division of PCLK.
 */
typedef enum mss_i2c_clock_divider {
    MSS_I2C_PCLK_DIV_256 = 0u,
    MSS_I2C_PCLK_DIV_224,
    MSS_I2C_PCLK_DIV_192,
    MSS_I2C_PCLK_DIV_160,
    MSS_I2C_PCLK_DIV_960,
    MSS_I2C_PCLK_DIV_120,
    MSS_I2C_PCLK_DIV_60,
    MSS_I2C_BCLK_DIV_8
} mss_i2c_clock_divider_t;

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_RELEASE_BUS constant is used to specify the options parameter to
  functions MSS_I2C_read(), MSS_I2C_write() and MSS_I2C_write_read() to indicate
  that a STOP bit must be generated at the end of the I2C transaction to release
  the bus.
 */
#define MSS_I2C_RELEASE_BUS        0x00u

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_HOLD_BUS constant is used to specify the options parameter to
  functions MSS_I2C_read(), MSS_I2C_write() and MSS_I2C_write_read() to
  indicate that a STOP bit must not be generated at the end of the I2C
  transaction in order to retain the bus ownership. This causes the next
  transaction to begin with a repeated START bit and no STOP bit between the
  transactions.
 */
#define MSS_I2C_HOLD_BUS           0x01u

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_SMBALERT_IRQ constant is used with the MSS_I2C_enable_smbus_irq()
  and MSS_I2C_disable_smbus_irq() functions to enable or disable the SMBus
  SMBALERT interrupt.
 */
#define MSS_I2C_SMBALERT_IRQ       0x01u

/*-------------------------------------------------------------------------*//**
 The MSS_I2C_SMBSUS_IRQ constant is used with the MSS_I2C_enable_smbus_irq() and
 MSS_I2C_disable_smbus_irq() functions to enable or disable the SMBus
 SMBSUS interrupt.
 */
#define MSS_I2C_SMBSUS_IRQ         0x02u

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_NO_TIMEOUT constant is used to specify the timeout_ms parameter to
  the MSS_I2C_wait_complete() function to indicate that the function must not
  time out while waiting for the I2C transaction to complete.
 */
#define MSS_I2C_NO_TIMEOUT         0u

/*-------------------------------------------------------------------------*//**
  The mss_i2c_status_t type is used to report the status of I2C transactions.
 */
typedef enum mss_i2c_status
{
    MSS_I2C_SUCCESS = 0u,
    MSS_I2C_IN_PROGRESS,
    MSS_I2C_FAILED,
    MSS_I2C_TIMED_OUT
} mss_i2c_status_t;

/*-------------------------------------------------------------------------*//**
  The mss_i2c_slave_handler_ret_t type is used by slave write handler functions
  to indicate whether or not the received data buffer should be released.
 */
typedef enum mss_i2c_slave_handler_ret {
    MSS_I2C_REENABLE_SLAVE_RX = 0u,
    MSS_I2C_PAUSE_SLAVE_RX = 1u
} mss_i2c_slave_handler_ret_t;

typedef struct mss_i2c_instance mss_i2c_instance_t ;

/*-------------------------------------------------------------------------*//**
  Transfer completion call back handler functions prototype.
  This defines the function prototype that must be followed by MSS I2C master
  and slave transfer completion handler functions. These functions are registered
  with the MSS I2C driver through the MSS_I2C_register_transfer_completion_handler()
  function.

  Declaring and Implementing transfer completion call back functions:
    Transfer complete call back function should follow the following prototype:
    void i2c0_completion_handler
    (
        mss_i2c_instance_t *instance,
        mss_i2c_status_t status
    )

    The instance parameter is a pointer to the mss_i2c_instance_t for which this
    transfer completion callback handler has been declared.

    The status parameter provides the status information of the current transfer
    completion such as transfer successful or any error occurred.

  }
  */
typedef void (*mss_i2c_transfer_completion_t)( mss_i2c_instance_t *instance, mss_i2c_status_t status);

/*-------------------------------------------------------------------------*//**
  Slave write handler functions prototype.
  ------------------------------------------------------------------------------
  This defines the function prototype that must be followed by MSS I2C slave
  write handler functions. These functions are registered with the MSS I2C
  driver through the MSS_I2C_register_write_handler() function.

  Declaring and Implementing Slave Write Handler Functions:
    Slave write handler functions should follow the following prototype:
    mss_i2c_slave_handler_ret_t write_handler
    (
        mss_i2c_instance_t *instance, uint8_t * data, uint16_t size
    );

    The instance parameter is a pointer to the mss_i2c_instance_t for which this
    slave write handler has been declared.

    The data parameter is a pointer to a buffer (received data buffer) holding
    the data written to the MSS I2C slave.

    Defining the macro MSS_I2C_INCLUDE_SLA_IN_RX_PAYLOAD causes the driver to
    insert the actual address used to access the slave as the first byte in the
    buffer. This allows applications tailor their response based on the actual
    address used to access the slave (primary address or GCA).

    The size parameter is the number of bytes held in the received data buffer.
    Handler functions must return one of the following values:
        MSS_I2C_REENABLE_SLAVE_RX
        MSS_I2C_PAUSE_SLAVE_RX.

    If the handler function returns MSS_I2C_REENABLE_SLAVE_RX, the driver
    releases the received data buffer and allows further I2C write transactions
    to the MSS I2C slave to take place.

    If the handler function returns MSS_I2C_PAUSE_SLAVE_RX, the MSS I2C slave
    responds to subsequent write requests with a non-acknowledge bit (NACK),
    until the received data buffer content has been processed by some other part
    of the software application.

    A call to MSS_I2C_enable_slave() is required at some point after
    returning MSS_I2C_PAUSE_SLAVE_RX in order to release the received data
    buffer so it can be used to store data received by subsequent I2C write
    transactions.
 */
typedef mss_i2c_slave_handler_ret_t (*mss_i2c_slave_wr_handler_t)( mss_i2c_instance_t *instance, uint8_t * data, uint16_t size);

typedef struct
{
    volatile uint8_t  CTRL;
    uint8_t  RESERVED0;
    uint16_t RESERVED1;
    uint8_t  STATUS;
    uint8_t  RESERVED2;
    uint16_t RESERVED3;
    volatile  uint8_t  DATA;
    uint8_t  RESERVED4;
    uint16_t RESERVED5;
    volatile uint8_t  ADDR;
    uint8_t  RESERVED6;
    uint16_t RESERVED7;
    volatile uint8_t  SMBUS;
    uint8_t  RESERVED8;
    uint16_t RESERVED9;
    volatile uint8_t  FREQ;
    uint8_t  RESERVED10;
    uint16_t RESERVED11;
    volatile uint8_t  GLITCHREG;
    uint8_t  RESERVED12;
    uint16_t RESERVED13;
    volatile uint8_t  SLAVE1_ADDR;
    uint8_t  RESERVED14;
    uint16_t RESERVED15;
} I2C_TypeDef;

/*-------------------------------------------------------------------------*//**
  mss_i2c_instance_t
  ------------------------------------------------------------------------------
  There is one instance of this structure for each of the MSS I2Cs. Instances
  of this structure are used to identify a specific MSS I2C. A pointer to an
  instance of the mss_i2c_instance_t structure is passed as the first parameter
  to MSS I2C driver functions to identify which MSS I2C should perform the
  requested operation.
 */
struct mss_i2c_instance
{
    uint_fast8_t ser_address;

    /* Transmit related info:*/
    uint_fast8_t target_addr;

    /* Current transaction type (WRITE, READ, RANDOM_READ)*/
    uint8_t transaction;

    uint_fast16_t random_read_addr;

    uint8_t options;

    /* I2C hardware instance identification */
    PLIC_IRQn_Type  irqn;
    I2C_TypeDef * hw_reg;

    /* Master TX INFO: */
    const uint8_t * master_tx_buffer;
    uint_fast16_t master_tx_size;
    uint_fast16_t master_tx_idx;
    uint_fast8_t dir;

    /* Master RX INFO: */
    uint8_t * master_rx_buffer;
    uint_fast16_t master_rx_size;
    uint_fast16_t master_rx_idx;

    /* Master Status */
    volatile mss_i2c_status_t master_status;
    uint32_t master_timeout_ms;

    /* Slave TX INFO */
    const uint8_t * slave_tx_buffer;
    uint_fast16_t slave_tx_size;
    uint_fast16_t slave_tx_idx;

    /* Slave RX INFO */
    uint8_t * slave_rx_buffer;
    uint_fast16_t slave_rx_size;
    uint_fast16_t slave_rx_idx;

    /* Slave Status */
    volatile mss_i2c_status_t slave_status;

    /* Slave data: */
    uint_fast8_t slave_mem_offset_length;
    mss_i2c_slave_wr_handler_t slave_write_handler;
    uint8_t is_slave_enabled;

    /* Transfer completion handler. */
    mss_i2c_transfer_completion_t transfer_completion_handler;

    /* User  specific data */
    void *p_user_data ;

    /* I2C bus status */
    uint8_t bus_status;

    /* Is transaction pending flag */
    uint8_t is_transaction_pending;

    /* I2C Pending transaction */
    uint8_t pending_transaction;
};

/*-------------------------------------------------------------------------*//**
  This instance of mss_i2c_instance_t holds all data related to the operations
  performed by MSS I2C 0 connected on main APB bus. The MSS_I2C_init()function
  initializes this structure. A pointer to g_mss_i2c0_lo is passed as the first
  parameter to MSS I2C driver functions to indicate that MSS I2C 0 should
  perform the requested operation.
*/
extern mss_i2c_instance_t g_mss_i2c0_lo;

/*-------------------------------------------------------------------------*//**
  This instance of mss_i2c_instance_t holds all data related to the operations
  performed by MSS I2C 1 connected on main APB bus. The MSS_I2C_init()function
  initializes this structure. A pointer to g_mss_i2c1_lo is passed as the first
  parameter to MSS I2C driver functions to indicate that MSS I2C 1 should
  perform the requested operation.
*/
extern mss_i2c_instance_t g_mss_i2c1_lo;

/*-------------------------------------------------------------------------*//**
  This instance of mss_i2c_instance_t holds all data related to the operations
  performed by MSS I2C 0 connected on main APB bus. The MSS_I2C_init()function
  initializes this structure. A pointer to g_mss_i2c0_lo is passed as the first
  parameter to MSS I2C driver functions to indicate that MSS I2C 0 should
  perform the requested operation.
*/
extern mss_i2c_instance_t g_mss_i2c0_hi;

/*-------------------------------------------------------------------------*//**
  This instance of mss_i2c_instance_t holds all data related to the operations
  performed by MSS I2C 1 connected on main APB bus. The MSS_I2C_init()function
  initializes this structure. A pointer to g_mss_i2c1_lo is passed as the first
  parameter to MSS I2C driver functions to indicate that MSS I2C 1 should
  perform the requested operation.
*/
extern mss_i2c_instance_t g_mss_i2c1_hi;

/*-------------------------------------------------------------------------*//**
  MSS I2C initialization routine.
  ------------------------------------------------------------------------------
  structures of one of the PolarFire SoC MSS I2Cs.
  ------------------------------------------------------------------------------
  The MSS_I2C_init() function initializes and configures hardware and data
  structures of one of the PolarFire SoC MSS I2Cs.

  @param this_i2c
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @param ser_address
    This parameter sets the I2C serial address for the MSS I2C peripheral being
    initialized. It is the I2C bus address to which the MSS I2C instance
    responds. MSS I2C peripherals can operate in master or slave mode and the
    serial address is significant only in the case of I2C slave mode. In master
    mode, MSS I2C does not require a serial address and the value of this
    parameter is not important. If you do not intend to use the I2C device in
    slave mode, then any dummy slave address value can be provided to this
    parameter. However, in systems where the MSS I2C may be expected to switch
    from master mode to slave mode, it is advisable to initialize the MSS I2C
    device with a valid serial slave address.
    You need to call the MSS_I2C_init() function whenever it is required to
    change the slave address as there is no separate function to set the slave
    address of an I2C device.

  @param ser_clock_speed
    This parameter sets the I2C serial clock frequency. It selects the divider
    that will be used to generate the serial clock from the APB PCLK or from
    the BCLK. It can be one of the following:
        MSS_I2C_PCLK_DIV_256
        MSS_I2C_PCLK_DIV_224
        MSS_I2C_PCLK_DIV_192
        MSS_I2C_PCLK_DIV_160
        MSS_I2C_PCLK_DIV_960
        MSS_I2C_PCLK_DIV_120
        MSS_I2C_PCLK_DIV_60
        MSS_I2C_BCLK_DIV_8

    Note: serial_clock_speed value is not critical for devices that only operate
          as slaves and can be set to any of the above values.

  @return
    This function does not return a value.

  Example:
  @code
    #define SLAVE_SER_ADDR_0   0x10u
    #define SLAVE_SER_ADDR_1   0x20u
    void system_init( void )
    {
        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR_0, MSS_I2C_PCLK_DIV_256 );
        MSS_I2C_init( &g_mss_i2c1_lo, SLAVE_SER_ADDR_1, MSS_I2C_PCLK_DIV_256 );
    }
  @endcode
*/
void MSS_I2C_init
(
    mss_i2c_instance_t * this_i2c,
    uint8_t ser_address,
    mss_i2c_clock_divider_t ser_clock_speed
);

/*******************************************************************************
 *******************************************************************************
 *
 *                           Master specific functions
 *
 * The following functions are only used within an I2C master's implementation.
 */

/*-------------------------------------------------------------------------*//**
  I2C master write function.
  ------------------------------------------------------------------------------
  This function initiates an I2C master write transaction. This function returns
  immediately after initiating the transaction. The content of the write buffer
  passed as parameter should not be modified until the write transaction
  completes. It also means that the memory allocated for the write buffer should
  not be freed or should not go out of scope before the write completes.
  You can check for the write transaction completion using the MSS_I2C_status()
  function. Additionally, driver will notify write transaction completion if
  callback function is registered.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @param serial_addr:
    This parameter specifies the serial address of the target I2C device.

  @param write_buffer:
    This parameter is a pointer to a buffer holding the data to be written to
    the target I2C device.
    Care must be taken not to release the memory used by this buffer before the
    write transaction completes. For example, it is not appropriate to return
    from a function allocating this buffer as an auto array variable before the
    write transaction completes as this would result in the buffer's memory
    being de-allocated from the stack when the function returns. This memory
    could then be subsequently reused and modified causing unexpected data to be
    written to the target I2C device.

  @param write_size:
    Number of bytes held in the write_buffer to be written to the target I2C
    device.

 @param options:
    The options parameter is used to indicate if the I2C bus should be released
    on completion of the write transaction. Using the MSS_I2C_RELEASE_BUS
    constant for the options parameter causes a STOP bit to be generated at the
    end of the write transaction causing the bus to be released for other I2C
    devices to use. Using the MSS_I2C_HOLD_BUS constant as options parameter
    prevents a STOP bit from being generated at the end of the write
    transaction, preventing other I2C devices from initiating a bus transaction.

  @return
    This function does not return a value.

  Example:
  @code
    #define I2C_DUMMY_ADDR   0x10u
    #define DATA_LENGTH      16u

    uint8_t  tx_buffer[DATA_LENGTH];
    uint8_t  write_length = DATA_LENGTH;

    void main( void )
    {
        uint8_t  target_slave_addr = 0x12;
        mss_i2c_status_t status;

        // Initialize MSS I2C peripheral
        MSS_I2C_init( &g_mss_i2c0_lo, I2C_DUMMY_ADDR, MSS_I2C_PCLK_DIV_256 );

        // Write data to slave.
        MSS_I2C_write( &g_mss_i2c0_lo, target_slave_addr, tx_buffer, write_length,
                       MSS_I2C_RELEASE_BUS );

        // Wait for completion and record the outcome
        status = MSS_I2C_wait_complete( &g_mss_i2c0_lo, MSS_I2C_NO_TIMEOUT );
    }
  @endcode
 */
void MSS_I2C_write
(
    mss_i2c_instance_t * this_i2c,
    uint8_t serial_addr,
    const uint8_t * write_buffer,
    uint16_t write_size,
    uint8_t options
);

/*-------------------------------------------------------------------------*//**
  I2C master read.
  ------------------------------------------------------------------------------
  This function initiates an I2C master read transaction. This function returns
  immediately after initiating the transaction.
  The content of the read buffer passed as the parameter should not be modified
  until the read transaction completes. It also means that the memory allocated
  for the read buffer should not be freed or should not go out of scope before
  the read completes. You can check for the read transaction completion using
  the MSS_I2C_status() function. Additionally, driver will notify read
  transaction completion, if callback function is registered.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver..

  @param serial_addr:
    This parameter specifies the serial address of the target I2C device.

  @param read_buffer
    This is a pointer to a buffer where the data received from the target device
    will be stored.
    Care must be taken not to release the memory used by this buffer before the
    read transaction completes. For example, it is not appropriate to return
    from a function allocating this buffer as an auto array variable before the
    read transaction completes as this would result in the buffer's memory being
    de-allocated from the stack when the function returns. This memory could
    then be subsequently reallocated resulting in the read transaction
    corrupting the newly allocated memory.

  @param read_size:
    This parameter specifies the number of bytes to read from the target device.
    This size must not exceed the size of the read_buffer buffer.

  @param options:
    The options parameter is used to indicate if the I2C bus should be released
    on completion of the read transaction. Using the MSS_I2C_RELEASE_BUS
    constant for the options parameter causes a STOP bit to be generated at the
    end of the read transaction causing the bus to be released for other I2C
    devices to use. Using the MSS_I2C_HOLD_BUS constant as options parameter
    prevents a STOP bit from being generated at the end of the read transaction,
    preventing other I2C devices from initiating a bus transaction.

  @return
    This function does not return a value.

  Example:
  @code
    #define I2C_DUMMY_ADDR   0x10u
    #define DATA_LENGTH      16u

    uint8_t  rx_buffer[DATA_LENGTH];
    uint8_t  read_length = DATA_LENGTH ;

    void main( void )
    {
        uint8_t  target_slave_addr = 0x12;
        mss_i2c_status_t status;

        // Initialize MSS I2C peripheral
        MSS_I2C_init( &g_mss_i2c0_lo, I2C_DUMMY_ADDR, MSS_I2C_PCLK_DIV_256 );

        // Read data from target slave using MSS I2C 0.
        MSS_I2C_read( &g_mss_i2c0_lo, target_slave_addr, rx_buffer, read_length,
                      MSS_I2C_RELEASE_BUS );

        status = MSS_I2C_wait_complete( &g_mss_i2c0_lo, MSS_I2C_NO_TIMEOUT );
    }
  @endcode
 */
void MSS_I2C_read
(
    mss_i2c_instance_t * this_i2c,
    uint8_t serial_addr,
    uint8_t * read_buffer,
    uint16_t read_size,
    uint8_t options
);

/*-------------------------------------------------------------------------*//**
  I2C master write-read
  ------------------------------------------------------------------------------
  This function initiates an I2C write-read transaction where data is first
  written to the target device before issuing a restart condition and changing
  the direction of the I2C transaction in order to read from the target device.

  The same warnings about buffer allocation in MSS_I2C_write() and
  MSS_I2C_read() apply to this function.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @param serial_addr:
    This parameter specifies the serial address of the target I2C device.

  @param addr_offset:
    This parameter is a pointer to the buffer containing the data that will be
    sent to the slave during the write phase of the write-read transaction. This
    data is typically used to specify an address offset specifying to the I2C
    slave device what data it must return during the read phase of the
    write-read transaction.

  @param offset_size:
    This parameter specifies the number of offset bytes to be written during the
    write phase of the write-read transaction. This is typically the size of the
    buffer pointed to by the addr_offset parameter.

  @param read_buffer:
    This parameter is a pointer to the buffer where the data read from the I2C
    slave will be stored.

  @param read_size:
    This parameter specifies the number of bytes to read from the target I2C
    slave device. This size must not exceed the size of the buffer pointed to by
    the read_buffer parameter.

  @param options:
    The options parameter is used to indicate if the I2C bus should be released
    on completion of the write-read transaction. Using the MSS_I2C_RELEASE_BUS
    constant for the options parameter causes a STOP bit to be generated at the
    end of the write-read transaction causing the bus to be released for other
    I2C devices to use. Using the MSS_I2C_HOLD_BUS constant as options parameter
    prevents a STOP bit from being generated at the end of the write-read
    transaction, preventing other I2C devices from initiating a bus transaction.

  @return
    This function does not return a value.

  Example:
  @code
    #define I2C_DUMMY_ADDR   0x10u
    #define TX_LENGTH        16u
    #define RX_LENGTH        8u

    uint8_t  rx_buffer[RX_LENGTH];
    uint8_t  read_length = RX_LENGTH;
    uint8_t  tx_buffer[TX_LENGTH];
    uint8_t  write_length = TX_LENGTH;

    void main( void )
    {
        uint8_t  target_slave_addr = 0x12;
        mss_i2c_status_t status;

        // Initialize MSS I2C peripheral
        MSS_I2C_init( &g_mss_i2c0_lo, I2C_DUMMY_ADDR, MSS_I2C_PCLK_DIV_256 );

        MSS_I2C_write_read( &g_mss_i2c0_lo, target_slave_addr, tx_buffer,
                            write_length, rx_buffer, read_length,
                            MSS_I2C_RELEASE_BUS );

        status = MSS_I2C_wait_complete( &g_mss_i2c0_lo, MSS_I2C_NO_TIMEOUT );
    }
  @endcode
 */
void MSS_I2C_write_read
(
    mss_i2c_instance_t * this_i2c,
    uint8_t serial_addr,
    const uint8_t * addr_offset,
    uint16_t offset_size,
    uint8_t * read_buffer,
    uint16_t read_size,
    uint8_t options
);

/*-------------------------------------------------------------------------*//**
  I2C status
  ------------------------------------------------------------------------------
  This function indicates the current state of an MSS I2C instance.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @return
    The return value indicates the current state of a MSS I2C instance or the
    outcome of the previous transaction if no transaction is in progress.
    Possible return values are:
      MSS_I2C_SUCCESS
        The last I2C transaction has completed successfully.
      MSS_I2C_IN_PROGRESS
        There is an I2C transaction in progress.
      MSS_I2C_FAILED
        The last I2C transaction failed.
      MSS_I2C_TIMED_OUT
        The request has failed to complete in the allotted time.

  Example:
  @code
    while( MSS_I2C_IN_PROGRESS == MSS_I2C_get_status( &g_mss_i2c0_lo ) )
    {
        // Do something useful while waiting for I2C operation to complete
        our_i2c_busy_task();
    }

    if( MSS_I2C_SUCCESS != MSS_I2C_get_status( &g_mss_i2c0_lo ) )
    {
        // Something went wrong...
        our_i2c_error_recovery( &g_mss_i2c0_lo );
    }
  @endcode
 */
mss_i2c_status_t MSS_I2C_get_status
(
    mss_i2c_instance_t * this_i2c
);

/*-------------------------------------------------------------------------*//**
  Wait for I2C transaction completion.
  ------------------------------------------------------------------------------
  This function waits for the current I2C transaction to complete. The return
  value indicates whether the last I2C transaction was successful or not.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @param timeout_ms:
    The timeout_ms parameter specifies the delay within which the current I2C
    transaction is expected to complete. The time out delay is given in
    milliseconds. MSS_I2C_wait_complete() will return MSS_I2C_TIMED_OUT if the
    current transaction does not complete before the time out delay expires.
    Alternatively, the timeout_ms parameter can be set to MSS_I2C_NO_TIMEOUT to
    indicate that the MSS_I2C_wait_complete() function must not time out.
    Note: If you set the timeout_ms parameter to a value other than
          MSS_I2C_NO_TIMEOUT, you must call the MSS_I2C_system_tick() function
          from an implementation of the SysTick timer interrupt
          service routine SysTick_Handler() in your application. Otherwise
          the time out will not take effect and the MSS_I2C_wait_complete()
          function will not time out.

  @return
    The return value indicates the outcome of the last I2C transaction. It can
    be one of the following:
      MSS_I2C_SUCCESS
        The last I2C transaction has completed successfully.
      MSS_I2C_FAILED
        The last I2C transaction failed.
      MSS_I2C_TIMED_OUT
        The last transaction failed to complete within the time out delay
        specified by the timeout_ms parameter.

  Example:
  @code
    #define I2C_DUMMY_ADDR   0x10u
    #define DATA_LENGTH      16u

    uint8_t  rx_buffer[DATA_LENGTH];
    uint8_t  read_length = DATA_LENGTH;

    void main( void )
    {
        uint8_t  target_slave_addr = 0x12;
        mss_i2c_status_t status;

        // Initialize MSS I2C peripheral
        MSS_I2C_init( &g_mss_i2c0_lo, I2C_DUMMY_ADDR, MSS_I2C_PCLK_DIV_256 );

        // Read data from slave.
        MSS_I2C_read( &g_mss_i2c0_lo, target_slave_addr, rx_buffer, read_length,
                       MSS_I2C_RELEASE_BUS );

        // Wait for completion and record the outcome
        status = MSS_I2C_wait_complete( &g_mss_i2c0_lo, MSS_I2C_NO_TIMEOUT );
    }
  @endcode
 */
mss_i2c_status_t MSS_I2C_wait_complete
(
    mss_i2c_instance_t * this_i2c,
    uint32_t timeout_ms
);

/*-------------------------------------------------------------------------*//**
  Time out delay expiration.
  ------------------------------------------------------------------------------
  This function is used to control the expiration of the time out delay
  specified as a parameter to the MSS_I2C_wait_complete() function. It must be
  called from the interrupt service routine of a periodic interrupt source such
  as the SysTick timer interrupt. It takes the period of the interrupt
  source as its ms_since_last_tick parameter and uses it as the time base for
  the MSS_I2C_wait_complete() function's time out delay.

  Note: This function does not need to be called if the MSS_I2C_wait_complete()
        function is called with a timeout_ms value of MSS_I2C_NO_TIMEOUT.
  Note: If this function is not called then the MSS_I2C_wait_complete() function
        will behave as if its timeout_ms was specified as MSS_I2C_NO_TIMEOUT and
        it will not time out.
  Note: If this function is being called from an interrupt handler (e.g SysTick)
        it is important that the calling interrupt have a lower priority than
        the MSS I2C interrupt(s) to ensure any updates to shared data are
        protected.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @param ms_since_last_tick:
    The ms_since_last_tick parameter specifies the number of milliseconds that
    elapsed since the last call to MSS_I2C_system_tick(). This parameter would
    typically be a constant specifying the interrupt rate of a timer used to
    generate system ticks.

  @return
    This function does not return a value.

  Example:
    The example below shows an example of how the MSS_I2C_system_tick() function
    would be called in a RISC-V based system. MSS_I2C_system_tick() is called
    for each MSS I2C peripheral from the RISC-V SysTick timer interrupt
    service routine. The SysTick is configured to generate an interrupt every 10
    milliseconds in the example below.
  @code
    #define SYSTICK_INTERVAL_MS 10

    void SysTick_Handler(void)
    {
        MSS_I2C_system_tick(&g_mss_i2c0_lo, SYSTICK_INTERVAL_MS);
        MSS_I2C_system_tick(&g_mss_i2c1_lo, SYSTICK_INTERVAL_MS);
    }
  @endcode
 */
void MSS_I2C_system_tick
(
    mss_i2c_instance_t * this_i2c,
    uint32_t ms_since_last_tick
);

/*******************************************************************************
 *******************************************************************************
 *
 *                           Slave specific functions
 *
 * The following functions are only used within the implementation of an I2C
 * slave device.
 */

/*-------------------------------------------------------------------------*//**
  I2C slave transmit buffer configuration.
  ------------------------------------------------------------------------------
  This function specifies the memory buffer holding the data that will be sent
  to the I2C master when this MSS I2C instance is the target of an I2C read or
  write-read transaction.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @param tx_buffer:
    This parameter is a pointer to the memory buffer holding the data to be
    returned to the I2C master when this MSS I2C instance is the target of an
    I2C read or write-read transaction.

  @param tx_size:
    Size of the transmit buffer pointed to by the tx_buffer parameter.

  @return
    This function does not return a value.

  Example:
  @code
    #define SLAVE_SER_ADDR         0x10u
    #define SLAVE_TX_BUFFER_SIZE   10u

    uint8_t g_slave_tx_buffer[SLAVE_TX_BUFFER_SIZE] = { 1, 2, 3, 4, 5,
                                                        6, 7, 8, 9, 10 };

    void main( void )
    {
        // Initialize the MSS I2C driver with its I2C serial address and serial
        // clock divider.
        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256 );

        // Specify the transmit buffer containing the data that will be
        // returned to the master during read and write-read transactions.
        MSS_I2C_set_slave_tx_buffer( &g_mss_i2c0_lo, g_slave_tx_buffer,
                                     sizeof(g_slave_tx_buffer) );
    }
  @endcode
 */
void MSS_I2C_set_slave_tx_buffer
(
    mss_i2c_instance_t * this_i2c,
    const uint8_t * tx_buffer,
    uint16_t tx_size
);

/*-------------------------------------------------------------------------*//**
  I2C slave receive buffer configuration.
  ------------------------------------------------------------------------------
  This function specifies the memory buffer that will be used by the MSS I2C
  instance to receive data when it is a slave. This buffer is the memory where
  data will be stored when the MSS I2C is the target of an I2C master write
  transaction (i.e. when it is the slave).
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @param rx_buffer:
    This parameter is a pointer to the memory buffer allocated by the caller
    software to be used as a slave receive buffer.

  @param rx_size:
    Size of the slave receive buffer. This is the amount of memory that is
    allocated to the buffer pointed to by rx_buffer.
    Note:   This buffer size indirectly specifies the maximum I2C write
            transaction length this MSS I2C instance can be the target of.
            This is because this MSS I2C instance responds to further received
            bytes with a non-acknowledge bit (NACK) as soon as its receive
            buffer is full. This causes the write transaction to fail.

  @return none.

  Example:
  @code
    #define SLAVE_SER_ADDR     0x10u
    #define SLAVE_RX_BUFFER_SIZE 10u

    uint8_t g_slave_rx_buffer[SLAVE_RX_BUFFER_SIZE];

    void main( void )
    {
        // Initialize the MSS I2C driver with its I2C serial address and
        // serial clock divider.
        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256 );

        // Specify the buffer used to store the data written by the I2C master.
        MSS_I2C_set_slave_rx_buffer( &g_mss_i2c0_lo, g_slave_rx_buffer,
                                     sizeof(g_slave_rx_buffer) );
    }
  @endcode
 */
void MSS_I2C_set_slave_rx_buffer
(
    mss_i2c_instance_t * this_i2c,
    uint8_t * rx_buffer,
    uint16_t rx_size
);

/*-------------------------------------------------------------------------*//**
  I2C slave memory offset length configuration.
  ------------------------------------------------------------------------------
  This function is used as part of the configuration of an MSS I2C instance for
  operation as a slave supporting write-read transactions. It specifies the
  number of bytes expected as part of the write phase of a write-read
  transaction. The bytes received during the write phase of a write-read
  transaction are interpreted as an offset into the slave's transmit buffer.
  This allows random access into the I2C slave transmit buffer from a remote
  I2C master.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @param offset_length:
    The offset_length parameter configures the number of bytes to be interpreted
    by the MSS I2C slave as a memory offset value during the write phase of
    write-read transactions. The maximum value for the offset_length parameter
    is two. The value of offset_length has the following effect on the
    interpretation of the received data.

      If offset_length is 0, the offset into the transmit buffer is fixed at 0.

      If offset_length is 1, a single byte of received data is interpreted as an
      unsigned 8 bit offset value in the range 0 to 255.

      If offset_length is 2, 2 bytes of received data are interpreted as an
      unsigned 16 bit offset value in the range 0 to 65535. The first byte
      received in this case provides the high order bits of the offset and
      the second byte provides the low order bits.

    If the number of bytes received does not match the non 0 value of
    offset_length the transmit buffer offset is set to 0.

  @return none.

  Example:
  @code
    #define SLAVE_SER_ADDR       0x10u
    #define SLAVE_TX_BUFFER_SIZE 10u

    uint8_t g_slave_tx_buffer[SLAVE_TX_BUFFER_SIZE] = { 1, 2, 3, 4, 5,
                                                        6, 7, 8, 9, 10 };

    void main( void )
    {
        // Initialize the MSS I2C driver with its I2C serial address and serial
        // clock divider.
        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256 );
        MSS_I2C_set_slave_tx_buffer( &g_mss_i2c0_lo, g_slave_tx_buffer,
                                     sizeof(g_slave_tx_buffer) );
        MSS_I2C_set_slave_mem_offset_length( &g_mss_i2c0_lo, 1 );
    }
  @endcode
 */
void MSS_I2C_set_slave_mem_offset_length
(
    mss_i2c_instance_t * this_i2c,
    uint8_t offset_length
);

/*-------------------------------------------------------------------------*//**
  I2C write handler registration.
  ------------------------------------------------------------------------------
  Register the function that is called to process the data written to this MSS
  I2C instance when it is the slave in an I2C write transaction.
  Note: If a write handler is registered, it is called on completion of the
        write phase of a write-read transaction and responsible for processing
        the received data in the slave receive buffer and populating the slave
        transmit buffer with the data that will be transmitted to the I2C master
        as part of the read phase of the write-read transaction. If a write
        handler is not registered, the write data of a write read transaction is
        interpreted as an offset into the slaves transmit buffer and handled by
        the driver.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @param handler:
    Pointer to the function that will process the I2C write request.

  @return none.

  Example:
  @code
    #define SLAVE_SER_ADDR       0x10u
    #define SLAVE_TX_BUFFER_SIZE 10u

    uint8_t g_slave_tx_buffer[SLAVE_TX_BUFFER_SIZE] = { 1, 2, 3, 4, 5,
                                                       6, 7, 8, 9, 10 };

    local function prototype
    void slave_write_handler
    (
        mss_i2c_instance_t * this_i2c,
        uint8_t * p_rx_data,
        uint16_t rx_size
    );

    void main( void )
    {
        // Initialize the MSS I2C driver with its I2C serial address and serial
        // clock divider.
        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256 );
        MSS_I2C_set_slave_tx_buffer( &g_mss_i2c0_lo, g_slave_tx_buffer,
                                     sizeof(g_slave_tx_buffer) );
        MSS_I2C_set_slave_mem_offset_length( &g_mss_i2c0_lo, 1 );
        MSS_I2C_register_write_handler( &g_mss_i2c0_lo, slave_write_handler );
    }
  @endcode
 */
void MSS_I2C_register_write_handler
(
    mss_i2c_instance_t * this_i2c,
    mss_i2c_slave_wr_handler_t handler
);

/*-------------------------------------------------------------------------*//**
  I2C slave enable.
  ------------------------------------------------------------------------------
  This function enables slave mode operation for an MSS I2C peripheral. It
  enables the MSS I2C slave to receive data when it is the target of an I2C
  read, write or write-read transaction.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @return none.

  Example:
  @code
    // Enable I2C slave.
    MSS_I2C_enable_slave( &g_mss_i2c0_lo );
  @endcode
 */
void MSS_I2C_enable_slave
(
    mss_i2c_instance_t * this_i2c
);

/*-------------------------------------------------------------------------*//**
  I2C slave disable.
  ------------------------------------------------------------------------------
  This function disables slave mode operation for an MSS I2C peripheral. It
  stops the MSS I2C slave acknowledging I2C read, write or write-read
  transactions targeted at it.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @return
    This function does not return a value.

  Example:
  @code
    // Disable I2C slave.
    MSS_I2C_disable_slave( &g_mss_i2c0_lo );
  @endcode
 */
void MSS_I2C_disable_slave
(
    mss_i2c_instance_t * this_i2c
);

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_set_gca() function is used to set the general call acknowledgment
  bit of an MSS I2C slave device. This allows the slave device respond to a
  general call or broadcast message from an I2C master.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @return
    This function does not return a value.

  Example:
  @code
    // Enable recognition of the General Call Address
    MSS_I2C_set_gca( &g_mss_i2c0_lo );
  @endcode
 */
void MSS_I2C_set_gca
(
    mss_i2c_instance_t * this_i2c
);

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_clear_gca() function is used to clear the general call
  acknowledgment bit of an MSS I2C slave device. This will stop the I2C slave
  device responding to any general call or broadcast message from the master.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @return
    This function does not return a value.

  Example:
  @code
    // Disable recognition of the General Call Address
    MSS_I2C_clear_gca( &g_mss_i2c0_lo );
  @endcode
 */
void MSS_I2C_clear_gca
(
    mss_i2c_instance_t * this_i2c
);

/*------------------------------------------------------------------------------
                      I2C SMBUS specific APIs
 ----------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_smbus_init() function enables SMBus timeouts and status logic. Set
  the frequency parameter to the MSS I2Cs PCLK frequency for 25ms SMBus
  timeouts, or to any frequency between 1 MHz and 255 MHz for to adjust the
  timeout.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @param frequency
    The frequency parameter specifies a frequency in MHz from 1 to 255. It can
    be the MSS I2Cs PCLK frequency to specify 25ms SMBus timeouts, or a higher
    or lower frequency than the PCLK for increased or decreased timeouts.

  @return
    This function does not return a value.

  Example:
  @code
    #define SLAVE_SER_ADDR  0x10u

    void system_init( void )
    {
        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256 );
        // Initialize SMBus feature
        MSS_I2C_smbus_init( &g_mss_i2c0_lo, 100 );
    }
  @endcode
 */
void MSS_I2C_smbus_init
(
    mss_i2c_instance_t * this_i2c,
    uint8_t frequency
);

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_enable_smbus_irq() function is used to enable the MSS I2Cs SMBSUS
  and SMBALERT SMBus interrupts.

  If this function is used to enable an MSS I2C SMBus interrupt source, the
  appropriate interrupt handler must be implemented in the application to
  override the weak stub function implemented in the CMSIS-HAL startup code:
    - MSS I2C 0 SMBALERT - I2C0_SMBAlert_IRQHandler( ).
    - MSS I2C 0 SMBSUS - I2C0_SMBust_IRQHandler( ).
    - MSS I2C 1 SMBALERT - I2C1_SMBAlert_IRQHandler( ).
    - MSS I2C 1 SMBSUS - I2C1_SMBus_IRQHandler( ).
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @param irq_type
    The irq_type parameter specifies which SMBus interrupt(s) to enable. The two
    possible interrupts are:
      MSS_I2C_SMBALERT_IRQ
      MSS_I2C_SMBSUS_IRQ
    To enable both ints in one call, use MSS_I2C_SMBALERT_IRQ |
    MSS_I2C_SMBSUS_IRQ.

  @return
    This function does not return a value.

  Example:
  @code
    #define SLAVE_SER_ADDR     0x10u
    void I2C0_SMBAlert_IRQHandler( void )
    {
        // MSS I2C 0 application specific SMBALERT code goes here ...
    }

    void I2C0_SMBus_IRQHandler( void )
    {
        // MSS I2C 0 application specific SMBus code goes here ...
    }

    void main( void )
    {
        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256 );

        // Initialize SMBus feature
        MSS_I2C_smbus_init( &g_mss_i2c0_lo, 100 );

        // Enable both SMBALERT & SMBSUS interrupts
        MSS_I2C_enable_smbus_irq( &g_mss_i2c0_lo,
                          (uint8_t)(MSS_I2C_SMBALERT_IRQ | MSS_I2C_SMBSUS_IRQ));
   }
   @endcode
 */
void MSS_I2C_enable_smbus_irq
(
    mss_i2c_instance_t * this_i2c,
    uint8_t  irq_type
);

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_disable_smbus_irq() function is used to disable the MSS I2C's
  SMBSUS and SMBALERT SMBus interrupts.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @param irq_type
    The irq_type parameter specifies the SMBUS interrupt to be disabled.
    The two possible interrupts are:
      MSS_I2C_SMBALERT_IRQ
      MSS_I2C_SMBSUS_IRQ
    To disable both interrupts in one call, use MSS_I2C_SMBALERT_IRQ |
    MSS_I2C_SMBSUS_IRQ.

  @return
    This function does not return a value.

  Example:
  @code
    #define SLAVE_SER_ADDR     0x10u
    void I2C0_SMBAlert_IRQHandler( void )
    {
        // MSS I2C 0 application specific SMBALERT code goes here ...
    }

    void I2C0_SMBus_IRQHandler( void )
    {
        // MSS I2C 0 application specific SMBus code goes here ...
    }

    void main( void )
    {
        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256 );

        // Initialize SMBus feature
        MSS_I2C_smbus_init( &g_mss_i2c0_lo, 100 );

        // Enable both SMBALERT & SMBSUS interrupts
        MSS_I2C_enable_smbus_irq( &g_mss_i2c0_lo,
                          (uint8_t)(MSS_I2C_SMBALERT_IRQ | MSS_I2C_SMBSUS_IRQ));

        ...

        // Disable the SMBALERT interrupt
        MSS_I2C_disable_smbus_irq( &g_mss_i2c0_lo, MSS_I2C_SMBALERT_IRQ );
    }
  @endcode
 */
void MSS_I2C_disable_smbus_irq
(
    mss_i2c_instance_t * this_i2c,
    uint8_t  irq_type
);

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_suspend_smbus_slave() function forces any SMBUS slave devices
  connected to an MSS I2C peripheral into power down or suspend mode by
  asserting the MSS I2Cs I2C_X_SMBSUS_NO output signal. The MSS I2C is the
  SMBus master in this case.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @return
    This function does not return a value.

  Example:
  @code
    #define SLAVE_SER_ADDR     0x10u

    void main( void )
    {
        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256 );

        // Initialize SMBus feature
        MSS_I2C_smbus_init( &g_mss_i2c0_lo, 100 );

        // suspend SMBus slaves
        MSS_I2C_suspend_smbus_slave( &g_mss_i2c0_lo );

        ...

        // Re-enable SMBus slaves
        MSS_I2C_resume_smbus_slave( &g_mss_i2c0_lo );
    }
  @endcode
 */
void MSS_I2C_suspend_smbus_slave
(
    mss_i2c_instance_t * this_i2c
);

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_resume_smbus_slave() function de-asserts the MSS I2C's
  I2C_X_SMBSUS_NO output signal to take any connected slave devices out of
  suspend mode. The MSS I2C is the SMBus master in this case.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @return
    This function does not return a value.

  Example:
  @code
    #define SLAVE_SER_ADDR     0x10u

    void main( void )
    {
        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256 );

        // Initialize SMBus feature
        MSS_I2C_smbus_init( &g_mss_i2c0_lo, 100 );

        // suspend SMBus slaves
        MSS_I2C_suspend_smbus_slave( &g_mss_i2c0_lo );

        ...

        // Re-enable SMBus slaves
        MSS_I2C_resume_smbus_slave( &g_mss_i2c0_lo );
    }
  @endcode
 */
void MSS_I2C_resume_smbus_slave
(
    mss_i2c_instance_t * this_i2c
);

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_reset_smbus() function resets the MSS I2C's SMBus connection by
  forcing SCLK low for 35mS. The reset is automatically cleared after 35ms have
  elapsed. The MSS I2C is the SMBus master in this case.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @return
    This function does not return a value.

  Example:
  @code
    #define SLAVE_SER_ADDR     0x10u

    void main( void )
    {
        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256 );

        // Initialize SMBus feature
        MSS_I2C_smbus_init( &g_mss_i2c0_lo, 100 );

        // Make sure the SMBus channel is in a known state by resetting it
        MSS_I2C_reset_smbus( &g_mss_i2c0_lo );
    }
  @endcode
 */
void MSS_I2C_reset_smbus
(
    mss_i2c_instance_t * this_i2c
);

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_set_smbus_alert() function is used to force master communication
  with an I2C slave device by asserting the MSS I2C's I2C_X_SMBALERT_NO signal.
  The MSS I2C is the SMBus master in this case.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @return
    This function does not return a value.

  Example:
  @code
    #define SLAVE_SER_ADDR     0x10u

    void main( void )
    {
        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256 );

        // Initialize SMBus feature
        MSS_I2C_smbus_init( &g_mss_i2c0_lo, 100 );

        // Get the SMBus masters attention
        MSS_I2C_set_smbus_alert( &g_mss_i2c0_lo );

        ...

        // Once we are happy, drop the alert
        MSS_I2C_clear_smbus_alert( &g_mss_i2c0_lo );
    }
  @endcode
 */
void MSS_I2C_set_smbus_alert
(
    mss_i2c_instance_t * this_i2c
);

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_clear_smbus_alert() function is used de-assert the MSS I2Cs
  I2C_X_SMBALERT_NO signal once a slave device has had a response from the
  master. The MSS I2C is the SMBus slave in this case.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @return
    This function does not return a value.

  Example:
  @code
    #define SLAVE_SER_ADDR     0x10u

    void main( void )
    {
        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256 );

        // Initialize SMBus feature
        MSS_I2C_smbus_init( &g_mss_i2c0_lo, 100 );

        // Get the SMBus masters attention
        MSS_I2C_set_smbus_alert( &g_mss_i2c0_lo );

        ...

        // Once we are happy, drop the alert
        MSS_I2C_clear_smbus_alert( &g_mss_i2c0_lo );
    }
  @endcode
 */
void MSS_I2C_clear_smbus_alert
(
    mss_i2c_instance_t * this_i2c
);

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_set_user_data() function is used to allow the association of a
  block of application specific data with an MDD I2C peripheral. The composition
  of the data block is an application matter and the driver simply provides the
  means for the application to set and retrieve the pointer. This may for
  example be used to provide additional channel specific information to the
  slave write handler.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @param p_user_data
    The p_user_data parameter is a pointer to the user specific data block for
    this MSS I2C peripheral. It is defined as void * as the driver does not
    know the actual type of data being pointed to and simply stores the pointer
    for later retrieval by the application.

  @return
    This function does not return a value.

  Example
  @code
    #define SLAVE_SER_ADDR     0x10u

    app_data_t channel_xdata;

    void main( void )
    {
        app_data_t *p_xdata;

        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256 );

        // Store location of user data in instance structure
        MSS_I2C_set_user_data( &g_mss_i2c0_lo, (void *)&channel_xdata );

        ...

        // Retrieve location of user data and do some work on it
        p_xdata = (app_data_t *)MSS_I2C_get_user_data( &g_mss_i2c0_lo );
        if( NULL != p_xdata )
        {
            p_xdata->foo = 123;
        }
    }
  @endcode
 */
void MSS_I2C_set_user_data
(
    mss_i2c_instance_t * this_i2c,
    void * p_user_data
);

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_get_user_data() function is used to allow the retrieval of the
  address of a block of application specific data associated with an MSS I2C
  peripheral. The composition of the data block is an application matter and the
  driver simply provides the means for the application to set and retrieve the
  pointer. This may for example be used to provide additional channel specific
  information to the slave write handler.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @return
    This function returns a pointer to the user specific data block for this
    MSS I2C peripheral. It is defined as void * as the driver does not know the
    actual type of data being pointed to. If no user data has been registered
    for this channel a NULL pointer is returned.

  Example
  @code
    #define SLAVE_SER_ADDR     0x10u

    app_data_t channel_xdata;

    void main( void )
    {
        app_data_t *p_xdata;

        MSS_I2C_init( &g_mss_i2c0_lo, SLAVE_SER_ADDR, MSS_I2C_PCLK_DIV_256 );

        // Store location of user data in instance structure
        MSS_I2C_set_user_data( &g_mss_i2c0_lo, (void *)&channel_xdata );

        ...

        // Retrieve location of user data and do some work on it
        p_xdata = (app_data_t *)MSS_I2C_get_user_data( &g_mss_i2c0_lo );
        if( NULL != p_xdata )
        {
            p_xdata->foo = 123;
        }
    }
  @endcode
 */
void * MSS_I2C_get_user_data
(
    mss_i2c_instance_t * this_i2c
);

/*-------------------------------------------------------------------------*//**
  The MSS_I2C_register_transfer_completion_handler() function is used register
  transfer completion call back function. This mechanism is used to
  notify the completion of the previously initiated I2C transfer when MSS I2C
  instance is operating as I2C Master. This call back function will be called
  when the transfer is completed. It will also inform the transfer status as a
  parameter of the completion handler function.
  This function must be called after I2C initialization and before starting any
  transmit or receive operations.
  ------------------------------------------------------------------------------
  @param this_i2c:
    The this_i2c parameter is a pointer to an mss_i2c_instance_t structure
    identifying the MSS I2C hardware block to be initialized. There are four
    such data structures, g_mss_i2c0_lo and g_mss_i2c1_lo, associated with MSS
    I2C 0 and MSS I2C 1 when they are connected on the AXI switch slave 5 (main
    APB bus) and g_mss_i2c0_hi and g_mss_i2c1_hi, associated with MSS I2C 0 to
    MSS I2C 1 when they are connected on the AXI switch slave 6 (AMP APB bus).
    This parameter must point to one of these four global data structure defined
    within I2C driver.

  @param completion_handler:
    The completion_handler parameter pointers to the function that informs to
    application previously initiated I2C transfer is completed along with
    transfer status.

  @return
    This function does not return a value.

  Example
  @code
    void i2c0_completion_handler(mss_i2c_instance_t * instance, mss_i2c_status_t status)
    {
        if (status == MSS_I2C_SUCCESS)
        {
            MSS_UART_polled_tx_string(gp_my_uart, (const uint8_t*)"\rI2C0 Transfer completed.\n\r");
        }
    }

    void main()
    {
        MSS_I2C_init(I2C_MASTER, MASTER_SER_ADDR, MSS_I2C_BCLK_DIV_8);
        MSS_I2C_register_transfer_completion_handler(I2C_MASTER, i2c0_completion_handler);
    }
  @endcode
 */
void MSS_I2C_register_transfer_completion_handler
(
    mss_i2c_instance_t * this_i2c,
    mss_i2c_transfer_completion_t completion_handler
);

#ifdef __cplusplus
}
#endif

#endif /*MSS_I2C_H_*/
