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
 *
 * PolarFire SoC (MPFS) Microprocessor Subsystem Watchdog bare metal software
 * driver public API.
 *
 *
 */
/*=========================================================================*//**
  @mainpage PolarFire SoC MSS Watchdog Bare Metal Driver

  ==============================================================================
  Introduction
  ==============================================================================
  The PolarFire SoC Microprocessor SubSystem (MSS) includes five instances of
  watchdog timer hardware blocks used to detect system lockups. This software
  driver provides a set of functions for controlling the MSS watchdog as part of
  a bare metal system where no operating system is available. The driver can be
  adapted for use as part of an operating system, but the implementation of the
  adaptation layer between the driver and the operating system's driver model is
  outside the scope of the driver.

  The MSS watchdog driver provides support for the following features:
    - Initialization of the MSS watchdog
    - Reading the current value and status of the watchdog timer
    - Refreshing the watchdog timer value
    - Enabling, disabling and clearing timeout and MVRP interrupts.

  ==============================================================================
  Hardware Flow Dependencies
  ==============================================================================
  The configuration of all the features of the PolarFire SoC MSS watchdog is
  covered by this driver. Besides, this driver does not require any other
  configuration.

  On PolarFire SoC an AXI switch forms a bus matrix interconnect among multiple
  masters and multiple slaves. Five RISC-V CPUs connect to the Master ports
  M10 to M14 of the AXI switch. By default, all the APB peripherals are
  accessible on AXI-Slave 5 of the AXI switch via the AXI to AHB and AHB to APB
  bridges (referred as main APB bus). However, to support logical separation in
  the Asymmetric Multi-Processing (AMP) mode of operation, the APB peripherals
  can alternatively be accessed on the AXI-Slave 6 via the AXI to AHB and AHB to
  APB bridges (referred as the AMP APB bus).

  Application must make sure that the desired MSS Watchdog instance is
  appropriately configured on one of the APB slaves described above by
  configuring the PolarFire SoC system registers (SYSREG) as per the application
  need and that the appropriate data structures are provided to this driver as
  parameter to the functions provided by this driver.

  The base address and register addresses are defined in this driver as
  constants. The interrupt number assignment for the MSS Watchdog peripherals
  are defined as constants in the MPFS HAL. You must ensure that the latest MPFS
  HAL is included in the project settings of the SoftConsole tool chain and that
  it is generated into your project.

  ==============================================================================
  Theory of Operation
  ==============================================================================
  The MSS watchdog driver functions are grouped into the following categories:
    - Initialization and configuration
    - Reading the current value and status of the watchdog timer
    - Refreshing the watchdog timer value
    - Support for enabling, disabling and clearing time-out and MVRP interrupts.

  --------------------------------
  Initialization and Configuration
  --------------------------------
  The MSS Watchdog driver provides the MSS_WD_configure() function to configure
  the MSS Watchdog with desired configuration values. It also provides the
  MSS_WD_get_config() function to read back the current configurations of the
  MSS Watchdog. You can use this function to retrieve the current configurations
  and then overwrite them with the application specific values, such as initial
  watchdog timer value, Maximum Value (up to which) Refresh (is) Permitted,
  watchdog time-out value, enable/disable forbidden region, enable/disable
  MVRP interrupt and interrupt type.

  The occurrence of a time out event before the system reset can be detected
  using the MSS_WD_timeout_occured() function. This function would typically be
  used at the start of the application to detect whether the application is
  starting as a result of a power-on reset or a watchdog reset. The time out
  event must be cleared through a call to function MSS_WD_clear_timeout_event()
  in order to allow the detection of subsequent time out events or
  differentiating between a RISC-V initiated system reset and watchdog reset.

  --------------------------------------------
  Reading the Watchdog Timer Value and Status
  --------------------------------------------
  MSS Watchdog is a down counter. A refresh forbidden window can be created by
  configuring the watchdog Maximum Value up to which Refresh is Permitted (MVRP).
  When the current value of the watchdog timer is greater than the MVRP value,
  refreshing the watchdog is forbidden. Attempting to refresh the watchdog timer
  in the forbidden window will assert a timeout interrupt. The
  MSS_WD_forbidden_status() function can be used to know whether the watchdog
  timer is in forbidden window or has crossed it. By default, the forbidden
  window is disabled. It can be enabled by providing an appropriate value as
  parameter to the MSS_WD_configure() function. When the forbidden window is
  disabled, any attempt to refresh the watchdog timer is ignored and the counter
  keeps on down counting.

  The current value of the watchdog timer can be read using the
  MSS_WD_current_value() function. This function can be called at any time.

  --------------------------------------------
  Refreshing the Watchdog Timer Value
  --------------------------------------------
  The watchdog timer value is refreshed using the MSS_WD_reload() function. The
  value reloaded into the watchdog timer down-counter is specified at the
  configuration time with an appropriate value as parameter to the
  MSS_WD_get_config() function.

  --------------------------------------------
  Interrupt Control
  --------------------------------------------
  The PolarFire SoC MSS Watchdog generates two interrupts, The MVRP interrupt and
  the timeout interrupt.
  The MVRP interrupt is generated when the watchdog down-counter crosses the
  Maximum Value up to which Refresh is Permitted (MVRP). Following functions to
  control MVRP interrupt:
    - MSS_WD_enable_mvrp_irq
    - MSS_WD_disable_mvrp_irq
    - MSS_WD_clear_mvrp_irq

  The timeout interrupt is generated when the watchdog down-counter crosses the
  watchdog timeout value. The timeout value is a non-zero value and it can be
  set to a maximum of MSS_WDOG_TRIGGER_MAX. The non-maskable interrupt is
  generated when the watchdog crosses this timeout value, the down counter
  keeps on down counting and a reset signal is generated when reaches zero.
  Following functions to control timeout interrupt:
    - MSS_WD_enable_timeout_irq
    - MSS_WD_disable_timeout_irq
    - MSS_WD_clear_timeout_irq

 *//*=========================================================================*/

#ifndef MSS_WATCHDOG_H_
#define MSS_WATCHDOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
  The following constants can be used to configure the MSS Watchdog where a
  zero or non-zero value such as enable or disable is to be provided as input
  parameter as shown below:
      wd0lo_config.forbidden_en = MSS_WDOG_DISABLE;
      MSS_WD_configure(MSS_WDOG0_LO, &wd0lo_config);
*/
#define MSS_WDOG_ENABLE                     1u
#define MSS_WDOG_DISABLE                    0u


/***************************************************************************//**
 The mss_watchdog_num_t is the Watchdog module number enumeration.
 The MSS_WDOG0_LO to MSS_WDOG4_LO Correspond to the Watchdog module number
 0 to 4 when the appear on the AXI switch Slave 5.
 The MSS_WDOG0_HI to MSS_WDOG4_HI Correspond to the Watchdog module number
 0 to 4 when the appear on the AXI switch Slave 6.
*/
typedef enum mss_watchdog_num{
    MSS_WDOG0_LO    = 0,
    MSS_WDOG1_LO    = 1,
    MSS_WDOG2_LO    = 2,
    MSS_WDOG3_LO    = 3,
    MSS_WDOG4_LO    = 4,
    MSS_WDOG0_HI    = 5,
    MSS_WDOG1_HI    = 6,
    MSS_WDOG2_HI    = 7,
    MSS_WDOG3_HI    = 8,
    MSS_WDOG4_HI    = 9,

} mss_watchdog_num_t;


/***************************************************************************//**
  The mss_watchdog_config_t type for the watchdog Configuration structure. This
  type is used as a parameter for the MSS_WD_configure() and the
  MSS_WD_get_config() functions.

  Following are the values as part of this structure
|  Parameter       |                         Description                       |
|------------------|-----------------------------------------------------------|
|   time_val       | The value from which the watchdog timer counts down       |
|   mvrp_val       | The Watchdog MVRP value                                   |
|   timeout_val    | The watchdog timeout value                                |
|   forbidden_en   | Enable/disable the forbidden window                       |
|                  | When set, if a refresh occurs in the forbidden window,    |
|                  | the watchdog timeout interrupt will be generated.         |


Time calculation example:

   time_val = 0xFFFFF0u
   mvrp_val = 0x989680u
   timeout_val = 0x3e8u

   A presaclar = 256 is used on the MSS AXI clock.
   Considering AXI clock = 25Mhz

   The MVRP interrupt will happen after
   (0xFFFFF0 - 0x989680) * ( 1/25MHz/256)
   mvrp interrupt will happen after 69 sec. after system reset

   (0xFFFFF0 - 0x3e8) * ( 1/25MHz/256)
   timeout interrupt will happen after 171 sec. after system reset

 */

typedef struct mss_watchdog_config{
    uint32_t                time_val;
    uint32_t                mvrp_val;
    uint32_t                timeout_val;
    uint8_t                 forbidden_en;
    uint8_t                 intr_type;

} mss_watchdog_config_t;

/***************************************************************************//**
 Internal constants and types
*******************************************************************************/
#define MSS_WDOG_INTEN_MVRP                 0u
#define MSS_WDOG_INTEN_TRIG                 1u
#define MSS_WDOG_INTEN_SLEEP                2u
#define MSS_WDOG_ACTIVE_SLEEP               3u
#define MSS_WDOG_ENA_FORBIDDEN              4u

#define MSS_WDOG_INTEN_MVRP_MASK            ( 1u << MSS_WDOG_INTEN_MVRP)
#define MSS_WDOG_INTEN_TRIG_MASK            ( 1u << MSS_WDOG_INTEN_TRIG)
#define MSS_WDOG_INTEN_SLEEP_MASK           ( 1u << MSS_WDOG_INTEN_SLEEP)
#define MSS_WDOG_ACTIVE_SLEEP_MASK          ( 1u << MSS_WDOG_ACTIVE_SLEEP)
#define MSS_WDOG_ENA_FORBIDDEN_MASK         ( 1u << MSS_WDOG_ENA_FORBIDDEN)

#define MSS_WDOG_MVRP_TRIPPED               0u
#define MSS_WDOG_WDOG_TRIPPED               1u
#define MSS_WDOG_FORBIDDEN                  2u
#define MSS_WDOG_TRIGGERED                  3u
#define MSS_WDOG_LOCKED                     4u
#define MSS_WDOG_DEVRST                     5u

#define MSS_WDOG_MVRP_TRIPPED_MASK          ( 1u << MSS_WDOG_MVRP_TRIPPED)
#define MSS_WDOG_WDOG_TRIPPED_MASK          ( 1u << MSS_WDOG_WDOG_TRIPPED)
#define MSS_WDOG_FORBIDDEN_MASK             ( 1u << MSS_WDOG_FORBIDDEN)
#define MSS_WDOG_TRIGGERED_MASK             ( 1u << MSS_WDOG_TRIGGERED)
#define MSS_WDOG_LOCKED_MASK                ( 1u << MSS_WDOG_LOCKED)
#define MSS_WDOG_DEVRST_MASK                ( 1u << MSS_WDOG_DEVRST)

#define MSS_WDOG_TRIGGER_MAX                4095u
#define MSS_WDOG_TIMER_MAX                  16777200u            /*0xFFFFFFu*/

/*
  The WATCHDOG_TypeDef is the hardware register structure for the PolarFire SoC
  MSS Watchdog.
 */
typedef struct
{
    volatile   uint32_t  REFRESH;
    volatile   uint32_t  CONTROL;
    volatile   uint32_t  STATUS;
    volatile   uint32_t  TIME;
    volatile   uint32_t  MSVP;
    volatile   uint32_t  TRIGGER;
    volatile   uint32_t  FORCE;
} WATCHDOG_TypeDef;

extern WATCHDOG_TypeDef* wdog_hw_base[10];

/***************************************************************************//**
  The MSS_WDOG_REFRESH_KEY macro holds the magic value which will cause a reload
  of the watchdog's down counter when written to the watchdog's WDOGREFRESH
  register.
 */
#define MSS_WDOG_REFRESH_KEY    (uint32_t)0xDEADC0DEU

/***************************************************************************//**
  The MSS_WDOG_FORCE_RESET_KEY macro holds the magic value which will force a
  reset if the Watchdog is already timed out (gone past the timeout value).
  Writing Any other value or writing TRIGGER register at other times will trigger
  the watchdog NMI sequence (i.e Raise the timeout interrupt).
 */
#define MSS_WDOG_FORCE_RESET_KEY    (uint32_t)0xDEADU

/***************************************************************************//**
  The MSS_WD_get_config() function returns the current configurations of the
  PolarFire SoC MSS Watchdog. The MSS Watchdog is pre-initialized by the flash
  bits at the design time. When used for the first time before calling the
  MSS_WD_configure() function, this function will return the default
  configurations as configured at the design time. You can reconfigure the
  MSS Watchdog using MSS_WD_configure() function. A call to MSS_WD_get_config()
  function will then return the current configuration values set by a previous
  call to MSS_WD_configure() function. You may not need to use this function if
  you do not want to know what the current configurations are. In that case, you
  can directly use the MSS_WD_configure() function to configure the MSS Watchdog
  to the values of your choice.

  @param wd_num
    The wd_num parameter is the Watchdog module number in the PolarFire SoC MSS
    on which the operation needs to be performed. The Watchdog module number can
    be chosen using mss_watchdog_num_t. The MSS_WDOG0_LO to MSS_WDOG4_LO
    correspond to the Watchdog module number 0 to 5 when the appear on the AXI
    switch Slave 5. The MSS_WDOG0_HI to MSS_WDOG4_HI Correspond to the Watchdog
    module number 0 to 5 when they appear on the AXI switch Slave 6.

  @param config
    The config parameter is the return parameter in which the current
    configurations of the watchdog module will be stored.

    Please see the description of mss_watchdog_config_t for details.

  @return
    This function does not return any value.


  Example:
  @code
  #include "mss_watchdog.h"
  mss_watchdog_config_t wd0lo_config;

  void e51( void )
  {
      MSS_WD_get_config(MSS_WDOG0_LO, &wd0lo_config);

      wd0lo_config.forbidden_en = WDOG_ENABLE;
      wd0lo_config.mvrp_val = 0xFFFF000u;

      MSS_WD_configure(MSS_WDOG0_LO, &wd0lo_config);

      for(;;)
      {
          main_task();
      }
  }
*/
void MSS_WD_get_config
(
    mss_watchdog_num_t wd_num,
    mss_watchdog_config_t* config
);

/***************************************************************************//**
  The MSS_WD_configure() function configures the desired watchdog module. The
  Watchdog module is pre-initialized by the flash bits at the design time to the
  default values. You can reconfigure the Watchdog module using
  MSS_WD_configure() function.

  Note that the MSS_WD_configure() function can be used only once, as it writes
  into the TIME register. After a write into the TIME register, the TIME, TRIGGER
  and MSVP register values are frozen and can't be altered again unless a system
  reset happens.

  Note also that the MSS Watchdog is not enabled at reset, calling this function
  will start the watchdog, it cannot then be disabled and must be refreshed
  periodically.

  @param wd_num
    The wd_num parameter is the Watchdog module number in the PolarFire SoC MSS
    on which the operation needs to be performed. The Watchdog module number can
    be chosen using mss_watchdog_num_t. The MSS_WDOG0_LO to MSS_WDOG4_LO
    correspond to the Watchdog module number 0 to 5 when the appear on the AXI
    switch Slave 5. The MSS_WDOG0_HI to MSS_WDOG4_HI correspond to the watchdog
    module number 0 to 5 when the appear on the AXI switch Slave 6.

  @param config
    The config parameter is the input parameter in which the configurations to be
    applied to the watchdog module are provided by the application.
    Please see the description of mss_watchdog_config_t for details.

  @return
    This function returns a zero value when executed successfully. A non-zero
    value is returned when the configuration values are out of bound.

  Example:
  @code
  #include "mss_watchdog.h"
  mss_watchdog_config_t wd0lo_config;

  void e51( void )
  {
      MSS_WD_get_config(MSS_WDOG0_LO, &wd0lo_config);

      wd0lo_config.forbidden_en = WDOG_ENABLE;
      wd0lo_config.mvrp_val = 0xFFFF000u;

      MSS_WD_configure(MSS_WDOG0_LO, &wd0lo_config);

      for(;;)
      {
          main_task();
      }
  }
*/
uint8_t MSS_WD_configure
(
    mss_watchdog_num_t wd_num,
    const mss_watchdog_config_t * config
);

/***************************************************************************//**
  The MSS_WD_reload() function causes the watchdog to reload its down-counter
  timer with the load value configured through the MSS configurator in the
  hardware flow. This function must be called regularly to avoid a system reset
  or a watchdog interrupt.

  Note that the MSS Watchdog is not enabled at reset, calling this function
  will start the watchdog, it cannot then be disabled and must be refreshed
  periodically.

  @param wd_num
    The wd_num parameter is the Watchdog module number in the PolarFire SoC MSS
    on which the operation needs to be performed. The Watchdog module number can
    be chosen using mss_watchdog_num_t. The MSS_WDOG0_LO to MSS_WDOG4_LO
    correspond to the Watchdog module number 0 to 5 when the appear on the AXI
    switch Slave 5. The MSS_WDOG0_HI to MSS_WDOG4_HI correspond to the watchdog
    module number 0 to 5 when they appear on the AXI switch Slave 6.

  @return
    This function does not return a value.
 */
static inline void MSS_WD_reload(mss_watchdog_num_t wd_num)
{
    if ((WATCHDOG_TypeDef*)0 != wdog_hw_base[wd_num])
    {
        wdog_hw_base[wd_num]->REFRESH = MSS_WDOG_REFRESH_KEY;
    }
}

/***************************************************************************//**
  The MSS_WD_current_value() function returns the current value of the
  watchdog's down-counter.

 @param wd_num
    The wd_num parameter is the Watchdog module number in the PolarFire SoC MSS
    on which the operation needs to be performed. The Watchdog module number can
    be chosen using mss_watchdog_num_t. The MSS_WDOG0_LO to MSS_WDOG4_LO
    correspond to the Watchdog module number 0 to 5 when the appear on the AXI
    switch Slave 5. The MSS_WDOG0_HI to MSS_WDOG4_HI correspond to the watchdog
    module number 0 to 5 when the appear on the AXI switch Slave 6.

  @return
    This function returns the current value of the watchdog's down-counter as
    a 32-bit unsigned integer.
 */
static inline uint32_t MSS_WD_current_value(mss_watchdog_num_t wd_num)
{
    return wdog_hw_base[wd_num]->REFRESH;
}

/***************************************************************************//**
  The MSS_WD_forbidden_status() function returns the refresh status of the
  MSS Watchdog.

  @param wd_num
    The wd_num parameter is the Watchdog module number in the PolarFire SoC MSS
    on which the operation needs to be performed. The Watchdog module number can
    be chosen using mss_watchdog_num_t. The MSS_WDOG0_LO to MSS_WDOG4_LO
    correspond to the Watchdog module number 0 to 5 when the appear on the AXI
    switch Slave 5. The MSS_WDOG0_HI to MSS_WDOG4_HI correspond to the watchdog
    module number 0 to 5 when the appear on the AXI switch Slave 6.

  @return
    This function returns the refresh status of the watchdog. A value of 1
    indicates that watchdog's down-counter is within the forbidden window and
    that a reload should not be done. A value of 0 indicates that the watchdog's
    down counter is within the permitted window and that a reload is allowed.
 */
static inline uint32_t MSS_WD_forbidden_status(mss_watchdog_num_t wd_num)
{
    return ((wdog_hw_base[wd_num]->STATUS & MSS_WDOG_FORBIDDEN_MASK) >>
                                                            MSS_WDOG_FORBIDDEN);
}

/***************************************************************************//**
  The MSS_WD_enable_mvrp_irq() function enables the MVRP interrupt.
  The MSS Watchdog 0 to 4 generate a local MVRP interrupt to HART0 to 4
  respectively. At the same time these interrupts are also available over the
  PLIC. This function allows you to choose which interrupt type should be
  enabled for each interrupt. The corresponding interrupt handler gets called
  when the interrupt asserts.

  Note: The Watchdog MVRP interrupt handler default implementations are
        weakly defined in  the PolarFire SoC HAL. You must provide your own
        implementation of these functions, which will override the default
        implementation, to suit your application. Please refer mss_ints.h in the
        MPFS HAL for the actual names and the prototypes of these functions.

  Note: This function must be called from appropriate HART context.

   @param wd_num
    The wd_num parameter is the Watchdog module number in the PolarFire SoC MSS
    on which the operation needs to be performed. The Watchdog module number can
    be chosen using mss_watchdog_num_t. The MSS_WDOG0_LO to MSS_WDOG4_LO
    correspond to the Watchdog module number 0 to 5 when the appear on the AXI
    switch Slave 5. The MSS_WDOG0_HI to MSS_WDOG4_HI correspond to the watchdog
    module number 0 to 5 when the appear on the AXI switch Slave 6.

  @param intr_type
    The intr_type parameter indicates the type of interrupt that must be enabled.
    The MVRP interrupt for each hart can either be local interrupt to that hart
    or it can be accessed as a PLIC interrupt.

  @return
    This function does not return a value.

  Example:
  @code
  #include "mss_watchdog.h"
  void e51( void )
  {

      MSS_WD_enable_mvrp_irq(wd_num);
      for (;;)
      {
          main_task();
          cortex_sleep();
      }
  }

  void wdog0_mvrp_E51_local_IRQHandler_10(void)
  {
      process_timeout();
      MSS_WD_clear_mvrp_irq();
  }
  @endcode
 */
static inline void
MSS_WD_enable_mvrp_irq
(
    mss_watchdog_num_t wd_num
)
{
    if ((WATCHDOG_TypeDef*)0 != wdog_hw_base[wd_num])
    {
        wdog_hw_base[wd_num]->CONTROL |= MSS_WDOG_INTEN_MVRP_MASK;
    }
}

/***************************************************************************//**
  The MSS_WD_disable_mvrp_irq() function disables the generation of the
  MVRP interrupt.

  Note: This function must be called from appropriate HART context.

 @param wd_num
    The wd_num parameter is the Watchdog module number in the PolarFire SoC MSS
    on which the operation needs to be performed. The Watchdog module number can
    be chosen using mss_watchdog_num_t. The MSS_WDOG0_LO to MSS_WDOG4_LO
    correspond to the Watchdog module number 0 to 5 when the appear on the AXI
    switch Slave 5. The MSS_WDOG0_HI to MSS_WDOG4_HI correspond to the watchdog
    module number 0 to 5 when the appear on the AXI switch Slave 6.

  @return
    This function does not return a value.
 */
static __inline void
MSS_WD_disable_mvrp_irq
(
    mss_watchdog_num_t wd_num
)
{
    if ((WATCHDOG_TypeDef*)0 != wdog_hw_base[wd_num])
    {
        wdog_hw_base[wd_num]->CONTROL &= ~(MSS_WDOG_INTEN_MVRP_MASK);
    }
}

/***************************************************************************//**
  The MSS_WD_clear_timeout_irq() function clears the watchdog's timeout
  interrupt which is connected to the RISC-V NMI interrupt. Calling
  MSS_WD_clear_timeout_irq() results in clearing the RISC-V NMI interrupt.
  Note: You must call the MSS_WD_clear_timeout_irq() function as part of your
        implementation of the wdog0_tout_u51_local_IRQHandler_9() timeout
        interrupt service routine (ISR) in order to prevent the same interrupt
        event re-triggering a call to the timeout ISR.

  Note: This function must be called from appropriate HART context.

  Note: The MSS_WD_enable_timeout_irq() and MSS_WD_disable_timeout_irq() are
        removed as in the PolarFire SoC MSS Watchdog the timeout interrupt is
        permanently enabled by default and it can not be disabled.

 @param wd_num
    The wd_num parameter is the Watchdog module number in the PolarFire SoC MSS
    on which the operation needs to be performed. The Watchdog module number can
    be chosen using mss_watchdog_num_t. The MSS_WDOG0_LO to MSS_WDOG4_LO
    correspond to the Watchdog module number 0 to 5 when the appear on the AXI
    switch Slave 5. The MSS_WDOG0_HI to MSS_WDOG4_HI correspond to the watchdog
    module number 0 to 5 when the appear on the AXI switch Slave 6.

  @return
    This function does not return any value.

 */
static inline void MSS_WD_clear_timeout_irq(mss_watchdog_num_t wd_num)
{
    if ((WATCHDOG_TypeDef*)0 != wdog_hw_base[wd_num])
    {
        wdog_hw_base[wd_num]->STATUS |=  MSS_WDOG_WDOG_TRIPPED_MASK;
        /*
         * Perform a second write to ensure that the first write completed before
         * returning from this function. This is to account for posted writes across
         * the AHB matrix. The second write ensures that the first write has
         * completed and that the interrupt line has been de-asserted by the time
         * the function returns. Omitting the second write may result in a delay
         * in the de-assertion of the interrupt line going to the RISC-V and a
         * retriggering of the interrupt.
         */
        wdog_hw_base[wd_num]->STATUS |=  MSS_WDOG_WDOG_TRIPPED_MASK;
    }
}

/***************************************************************************//**
  The MSS_WD_clear_mvrp_irq() function clears the mvrp interrupt. This
  function also clears the interrupt in the RISC-V interrupt controller
  through a call to NVIC_ClearPendingIRQ().
  Note: You must call the MSS_WD_clear_mvrp_irq() function as part of  your
        implementation of the wdog0_msvp_u51_local_IRQHandler_10() mvrp interrupt service
        routine (ISR) in order to prevent the same interrupt event re-triggering
        a call to the mvrp ISR.

 @param wd_num
    The wd_num parameter is the Watchdog module number in the PolarFire SoC MSS
    on which the operation needs to be performed. The Watchdog module number can
    be chosen using mss_watchdog_num_t. The MSS_WDOG0_LO to MSS_WDOG4_LO
    correspond to the Watchdog module number 0 to 5 when the appear on the AXI
    switch Slave 5. The MSS_WDOG0_HI to MSS_WDOG4_HI correspond to the watchdog
    module number 0 to 5 when the appear on the AXI switch Slave 6.

  @return
    This function does not return a value.

*/
static inline void MSS_WD_clear_mvrp_irq(mss_watchdog_num_t wd_num)
{
    if ((WATCHDOG_TypeDef*)0 != wdog_hw_base[wd_num])
    {
        wdog_hw_base[wd_num]->STATUS |=  MSS_WDOG_MVRP_TRIPPED_MASK;
        /*
         * Perform a second write to ensure that the first write completed before
         * returning from this function. This is to account for posted writes across
         * the AHB matrix. The second write ensures that the first write has
         * completed and that the interrupt line has been de-asserted by the time
         * the function returns. Omitting the second write may result in a delay
         * in the de-assertion of the interrupt line going to the RISC-V and a
         * re-triggering of the interrupt.
         */
        wdog_hw_base[wd_num]->STATUS |=  MSS_WDOG_MVRP_TRIPPED_MASK;
    }
}

/***************************************************************************//**
  The MSS_WD_timeout_occured() function reports the occurrence of a timeout
  event.

 @param wd_num
    The wd_num parameter is the Watchdog module number in the PolarFire SoC MSS
    on which the operation needs to be performed. The Watchdog module number can
    be chosen using mss_watchdog_num_t. The MSS_WDOG0_LO to MSS_WDOG4_LO
    correspond to the Watchdog module number 0 to 5 when the appear on the AXI
    switch Slave 5. The MSS_WDOG0_HI to MSS_WDOG4_HI correspond to the watchdog
    module number 0 to 5 when the appear on the AXI switch Slave 6.

  @return
    A zero value indicates no watchdog timeout event occurred. A value of 1
    indicates that a timeout event occurred.

  Example:
  @code
  #include "mss_watchdog.h"
  void e51( void )
  {
      uint32_t wdg_reset;

      wdg_reset = MSS_WD_timeout_occured();
      if (wdg_reset)
      {
          log_watchdog_event();
          MSS_WD_clear_timeout_event();
      }

      for(;;)
      {
          main_task();
      }
  }
*/
static inline uint32_t MSS_WD_timeout_occured(mss_watchdog_num_t wd_num)
{
    return((wdog_hw_base[wd_num]->STATUS & MSS_WDOG_TRIGGERED_MASK) >>
                                                            MSS_WDOG_TRIGGERED);
}

/***************************************************************************//**
  The MSS_WD_force_reset() function is used to force an immediate reset
  if the watchdog has already triggered. Writing any value in this condition
  will cause an NMI sequence. Moreover any attempt to force reset when the
  watchdog is not in triggered condition will also cause an NMI sequence.

  @param wd_num
    The wd_num parameter is the Watchdog module number in the PolarFire SoC MSS
    on which the operation needs to be performed. The Watchdog module number can
    be chosen using mss_watchdog_num_t. The MSS_WDOG0_LO to MSS_WDOG4_LO
    correspond to the Watchdog module number 0 to 5 when the appear on the AXI
    switch Slave 5. The MSS_WDOG0_HI to MSS_WDOG4_HI correspond to the watchdog
    module number 0 to 5 when the appear on the AXI switch Slave 6.

  @return
    This function does not return a value.
 */
static inline void MSS_WD_force_reset(mss_watchdog_num_t wd_num)
{
    if (MSS_WDOG_TRIGGERED_MASK ==
                       (uint32_t)(wdog_hw_base[wd_num]->STATUS |
                                                       MSS_WDOG_TRIGGERED_MASK))
    {
        wdog_hw_base[wd_num]->FORCE = 0xDEADu;
    }
    else
    {
        wdog_hw_base[wd_num]->FORCE = 0x0u;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* MSS_WATCHDOG_H_ */
