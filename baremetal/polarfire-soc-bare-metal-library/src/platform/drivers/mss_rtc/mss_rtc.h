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
 * PolarFire SoC Microprocessor subsystem RTC bare metal software driver public
 * APIs.
 *
 * SVN $Revision$
 * SVN $Date$
 */

/*=========================================================================*//**
  @mainpage PolarFire MSS RTC Bare Metal Driver.

  ==============================================================================
  Introduction
  ==============================================================================
  The PolarFire SoC Microprocessor Subsystem (MSS) includes a real time counter
  (RTC) that can generate alarms and wakeup functions in real time. The RTC core
  also provides the feature of real time clock. This software driver provides a
  set of functions for controlling the MSS RTC as part of a bare metal system
  where no operating system is available. The driver can be adapted for use as
  part of an operating system, but the implementation of the adaptation layer
  between the driver and the operating system's driver model is outside the
  scope of the driver.

  The MSS RTC driver provides support for the following features:
    - Initialization of the RTC
    - Configuration of the RTC time-base
    - Configuration as a calendar or binary mode counter
    - Set the current calendar or binary mode count
    - Get the current calendar or binary mode count
    - Start and stop the RTC counting
    - Set alarm conditions
    - Enable, disable and clear the wakeup interrupt

  ==============================================================================
  Hardware Flow Dependencies
  ==============================================================================
  The configuration of all features of the MSS RTC driver is covered by this
  driver except for the clock source driving the MSS RTC clock(RTCCLK) input.
  The PolarFire SoC MSS clock controller supplies single clock source of 1 MHz
  to the MSS RTC clock input.
  On PolarFire SoC an AXI switch forms a bus matrix interconnect among multiple
  masters and multiple slaves. Five RISC-V CPUs connect to the Master ports M10
  to M14 of the AXI switch. By default, all the APB peripherals are accessible
  on AXI-Slave 5 of the AXI switch via the AXI to AHB and AHB to APB bridges
  (referred as main APB bus). However, to support logical separation in the
  Asymmetric Multi-Processing (AMP) mode of operation, the APB peripherals can
  alternatively be accessed on the AXI-Slave 6 via the AXI to AHB and AHB to
  APB bridges (referred as the AMP APB bus).
  Application must make sure that the RTC is appropriately configured on one of
  the APB bus described above by configuring the PolarFire SoC system registers
  (SYSREG) as per the application need and that the appropriate data structures
  are provided to this driver as parameter to the functions provided by this
  driver.
  The base address and register addresses and interrupt number assignment for
  the MSS RTC block are defined as constants in the PolarFire SoC MPFS HAL. You
  must ensure that the latest PolarFire SoC MPFS HAL is included in the project
  settings of the software tool chain used to build your project and that it is
  generated into your project.

  ==============================================================================
  Theory of Operation
  ==============================================================================
  The MSS RTC driver functions are grouped into the following categories:
    - Initialization of the RTC driver and hardware
    - Setting and reading the RTC counter current value
    - Setting RTC alarm values
    - Starting and stopping the RTC
    - Interrupt Control

  --------------------------------
  Initialization of the RTC driver and hardware
  --------------------------------
  The MSS RTC driver is initialized through a call to the MSS_RTC_init()
  function. The MSS_RTC_init() function must be called before any other MSS RTC
  driver functions are called.
  The MSS_RTC_init() function:
    *   Stops the RTC counters and disables the RTC alarm
    *   Disables the RTC wakeup interrupt in the RTC and in the Platform Level
        Interrupt Controller (PLIC).
    *   Clears any pending RTC wakeup interrupt in the RTC and in the Platform
        Level Interrupt Controller (PLIC).
    *   Enables the RTC_WAKEUP_CR[0] mask bit in the MSS System Register to
        connect the RTC wakeup interrupt to the Platform Level Interrupt
        Controller.
    *   Resets the RTC counters, alarm and the compare registers
    *   Sets the RTC's operating mode to binary counter mode or calendar
        counter mode, as specified by the mode parameter
    *   Sets the RTC's prescaler register to the value specified by the
        prescaler parameter. The frequency of the clock source driving the MSS
        RTC clock (RTCCLK) input is required to calculate the prescaler value.

  --------------------------------------------
  Setting and Reading the RTC Counter Value
  --------------------------------------------
  The MSS RTC supports two mode of operation - binary mode and calendar mode.
  The following functions are used to set and read the current value of the
  counter when the MSS RTC is configured to operate in binary mode:
    *   MSS_RTC_set_binary_count() - This function is used to set the current
                                     value of the RTC binary counter.
    *   MSS_RTC_get_binary_count() - This function is used to read the current
                                      value of the RTC binary counter.

  The following functions are used to set and read the current value of the
  counter the MSS RTC is configured to operate in calendar mode:
    *   MSS_RTC_set_calendar_count() - This function is used to set the current
                                       value of the RTC calendar counter.
    *   MSS_RTC_get_calendar_count() - This function is used to read the current
                                       value of the RTC calendar counter.

  The following functions resets the RTC counter in either binary or calendar
  operating mode:
    *   MSS_RTC_reset_counter() - This function resets the RTC counter.


  --------------------------------------------
  Setting RTC Alarms
  --------------------------------------------
  The MSS RTC can generate alarms when the counter matches a specified count
  value in binary mode or a date and time in calendar mode.
  The following functions are used to set up alarms:
    *   MSS_RTC_set_binary_count_alarm() - This function sets up one-shot or
                                           periodic alarms when the MSS RTC is
                                           configured to operate in binary mode.
    *   MSS_RTC_set_calendar_count_alarm() - This function sets up one-shot or
                                             periodic alarms when the MSS RTC is
                                             configured to operate in calendar
                                             mode.
  Note: The alarm asserts a wakeup interrupt to the RISC-V Core. This function
  enables the RTC's wakeup interrupt output, however the RTC wakeup interrupt
  input to the RISC-V PLIC must be enabled separately by calling the
  MSS_RTC_enable_irq() function. The alarm can be disabled at any time by
  calling the MSS_RTC_disable_irq() function.

  --------------------------------------------
  Starting and Stopping the RTC Counter
  --------------------------------------------
  The following functions start and stop the RTC counter:
    - MSS_RTC_start() - This function starts the RTC counter.
    - MSS_RTC_stop() - This function stops the RTC counter.

  -------------------------------------------
  Interrupt Control
  -------------------------------------------
  The MSS_RTC_enable_irq () function enables the RTC_WAKEUP_CR[0] mask bit in
  the MSS System Register to connect the RTC wakeup interrupt to the Platform
  Level Interrupt Controller.
  An rtc_wakeup_plic_IRQHandler () default implementation is defined, with weak
  linkage, in the PolarFire SoC MPFS HAL. You must provide your own
  implementation of the rtc_wakeup_plic_IRQHandler () function, which will
  override the default implementation, to suit your application.
  The function prototype for the RTC wakeup interrupt handler is as follows:
      uint8_t  rtc_wakeup_plic_IRQHandler(void);

  The RTC wakeup interrupt is controlled using the following functions:
    *   MSS_RTC_enable_irq() - The MSS_RTC_enable_irq() function enables the RTC
                                to interrupt the MSS when a wakeup alarm occurs.
    *   MSS_RTC_disable_irq() - The MSS_RTC_disable_irq() function disables the
                                RTC from interrupting the MSS when a wakeup
                                alarm occurs.
    *   MSS_RTC_clear_irq() - The MSS_RTC_clear_irq() function clears a pending
                              RTC wakeup interrupt at the RTC wakeup output.
                              You must call the MSS_RTC_clear_irq() function as
                              part of your implementation of the
                              rtc_wakeup_plic_IRQHandler() interrupt service
                              routine (ISR) in order to prevent the same
                              interrupt event retriggering a call to the ISR.

*//*=========================================================================*/
#ifndef MSS_RTC_H_
#define MSS_RTC_H_

#include "mss_rtc_regs.h"
#include "hal/cpu_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_BINARY_MODE constant is used to specify the mode parameter to the
  MSS_RTC_init() function. The RTC will run in binary mode if this constant is
  used. In binary mode, the calendar counter counts consecutively from 0 all the
  way to 2^43.
 */
#define MSS_RTC_BINARY_MODE               0u

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_CALENDAR_MODE constant is used to specify the mode parameter to
  the MSS_RTC_init() function. The RTC will run in calendar mode if this
  constant is used. In calendar mode, the calendar counter counts seconds,
  minutes, hours, days, months, years, weekdays and weeks.
 */
#define MSS_RTC_CALENDAR_MODE             1u

/*-------------------------------------------------------------------------*//**
  The alarm_value parameter of the MSS_RTC_set_calendar_count_alarm() function
  is a pointer to an mss_rtc_calender_t data structure specifying the date and
  time at which the alarm is to occur. You must assign the required date and
  time values to the mss_rtc_calender_t structure before calling the function.
  Any of the fields of the mss_rtc_calender_t structure can be set to
  MSS_RTC_CALENDAR_DONT_CARE, to indicate that they are not to be considered in
  deciding when the alarm will occur; this is necessary when setting periodic
  alarms.
 */
#define MSS_RTC_CALENDAR_DONT_CARE      0xFFu

/*-------------------------------------------------------------------------*//**
  Days of the week.
 */
#define MSS_RTC_SUNDAY      1u
#define MSS_RTC_MONDAY      2u
#define MSS_RTC_TUESDAY     3u
#define MSS_RTC_WEDNESDAY   4u
#define MSS_RTC_THRUSDAY    5u
#define MSS_RTC_FRIDAY      6u
#define MSS_RTC_SATURDAY    7u

/***************************************************************************//**
  MSS RTC base addresses.
  These definitions provides access to the MSS RTC mapped at two different
  memory regions. User can provide one of these constants to the MSS_RTC_init()
  function for configuring the MSS RTC block.
 */
#define MSS_RTC_LO_BASE                 (RTC_TypeDef *)MSS_RTC_LO_ADDR
#define MSS_RTC_HI_BASE                 (RTC_TypeDef *)MSS_RTC_HI_ADDR

/*-------------------------------------------------------------------------*//**
  The mss_rtc_alarm_type_t enumeration is used as the alarm_type parameter for
  the MSS_RTC_set_calendar_count_alarm() and MSS_RTC_set_binary_count_alarm()
  functions to specify whether the requested alarm should occur only one time or
  periodically.
 */
typedef enum {
    MSS_RTC_SINGLE_SHOT_ALARM,
    MSS_RTC_PERIODIC_ALARM
} mss_rtc_alarm_type_t;

/*-------------------------------------------------------------------------*//**
  A pointer to an instance of the mss_rtc_calender_t data structure is used to
  write new date and time values to the RTC using the
  MSS_RTC_set_rtc_calendar_count() and MSS_RTC_set_calendar_count_alarm()
  functions. The MSS_RTC_get_calendar_count() function also uses a pointer to an
  instance of the mss_rtc_calender_t data structure to read the current date and
  time value from the RTC.
 */
typedef struct mss_rtc_calender
{
     uint8_t second;
     uint8_t minute;
     uint8_t hour;
     uint8_t day;
     uint8_t month;
     uint8_t year;
     uint8_t weekday;
     uint8_t week;
} mss_rtc_calender_t ;

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_init() function initializes the RTC driver and hardware to a known
  state. To initialize the RTC hardware, this function:
    *   Stops the RTC counters and disables the RTC alarm
    *   Disables the RTC wakeup interrupt in the RTC and in the PolarFire SoC
        MSS Platform Level Interrupt Controller (PLIC).
    *   Clears any pending RTC wakeup interrupt in the RTC and in the PolarFire
        SoC MSS Platform Level Interrupt Controller (PLIC).
    *   Resets the RTC counters and the alarm and compare registers
    *   Sets the RTC's operating mode to binary counter mode or calendar counter
        mode, as specified by the mode parameter
    *   Sets the RTC's prescaler register to the value specified by the
        prescaler parameter
    *   The MSS clock controller can supply one of three clock sources to the
        RTC clock input (RTCCLK):
        -   Crystal Oscillator 32.768 kHz
        -   1MHz Oscillator
        -   50MHz Oscillator.  (25 MHz in a 1.0v part).

  For calendar mode, program the prescaler register to generate a 1Hz signal
  from the active RTCCLK according to the following equation:
        prescaler = RTCCLK - 1  (where RTCCLK unit is Hz)
  For a 32.768 kHz clock, set the prescaler to 32768 - 1 = 32767.
  The prescaler register is 26 bits wide, allowing clock sources of up to 67 MHz
  to generate the 1Hz time base.

  For binary mode, the prescaler register can be programmed to generate a 1Hz
  time base or a different time base, as required.

  @param base_address
    The base address parameter provides the base address of the MSS RTC
    peripheral. The MSS RTC can appear on either the AXI slave 5 or slave 6 per
    SYSREG configurations. The corresponding base address of this peripheral
    must be provided per your configuration.

  @param mode
    The mode parameter is used to specify the operating mode of the RTC. The
    allowed values for mode are:
      - MSS_RTC_BINARY_MODE
      - MSS_RTC_CALENDAR_MODE

  @param prescaler
    The prescaler parameter specifies the value to divide the incoming RTC clock
    by, to generate the RTC time base signal. For calendar mode, set the
    prescaler value to generate a 1Hz time base from the incoming RTC clock
    according to the following equation:
        prescaler = RTCCLK - 1    (where the RTCCLK unit is Hz)
    For binary mode, set the prescaler value to generate a 1Hz time base or a
    different time base, as required.
    The prescaler parameter can be any integer value in the range 2 to 2^26.

  @return
    This function does not return any value.

  Example:
  The example code below shows how the RTC can be initialized only after a power-on
  reset.
  @code
    #define PO_RESET_DETECT_MASK    0x00000001u

    void e51(void)
    {
        uint32_t power_on_reset;
        power_on_reset = SYSREG->RESET_SOURCE_CR & PO_RESET_DETECT_MASK;
        if(power_on_reset)
        {
            MSS_RTC_init(MSS_RTC_LO_BASE, MSS_RTC_BINARY_MODE, RTC_PERIPH_PRESCALER/ 10u );
            SYSREG->RESET_SOURCE_CR = PO_RESET_DETECT_MASK;
        }
    }

  @endcode
 */
void
MSS_RTC_init
(
    RTC_TypeDef *base_address,
    uint8_t mode,
    uint32_t prescaler
);

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_set_binary_count() function sets the current value of the RTC
  binary counter.
  Note: This function must only be used when the RTC is configured to operate in
        binary counter mode.

  @param new_rtc_value
    The new_rtc_value parameter specifies the new count value from which the RTC
    will increment. The binary counter is 43 bits wide, so the maximum allowed
    binary value is 2^43.

  @return
    This function does not return a value.
 */

void
MSS_RTC_set_binary_count
(
    uint64_t new_rtc_value
);

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_set_rtc_calendar_count() function sets the current value of the
  RTC calendar counter.
  Note: This function must only be used when the RTC is configured to operate in
        calendar counter mode.

  @param new_rtc_value
    The new_rtc_value parameter is a pointer to an mss_rtc_calender_t data
    structure specifying the new date and time value from which the RTC will
    increment. You must populate the mss_rtc_calender_t structure with the
    required date and time values before calling this function.

  @return
    This function does not return a value.
 */

void
MSS_RTC_set_calendar_count
(
    const mss_rtc_calender_t *new_rtc_value
);

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_get_calendar_count() function returns the current value of the RTC
  calendar counter via the data structure pointed to by the p_rtc_calendar
  parameter.
  Note: This function must only be used when the RTC is configured to operate in
        calendar counter mode.

  @param p_rtc_calendar
    The p_rtc_calendar parameter is a pointer to an mss_rtc_calender_t data
    structure where the current value of the calendar counter will be written by
    the MSS_RTC_get_calendar_count() function

  @return
    This function does not return a value.
 */
void
MSS_RTC_get_calendar_count
(
    mss_rtc_calender_t *p_rtc_calendar
);

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_get_binary_count() function returns the current value of the RTC
  binary counter.
  Note: This function must only be used when the RTC is configured to operate in
        binary counter mode.

  @param
    This function takes no parameters.

  @return
    This function returns the current value of the RTC binary counter as an
    unsigned 64-bit integer.
 */
uint64_t
MSS_RTC_get_binary_count
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_start() function starts the RTC incrementing.

  @param
    This function takes no parameters.

  @return
    This function does not return a value.
 */
void
MSS_RTC_start
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_stop() function stops the RTC from incrementing.

  @param
    This function takes no parameters.

  @return
    This function does not return a value.
 */
void
MSS_RTC_stop
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_reset_counter() function resets the RTC counters. If the counter
  was running before calling this function, then it continues incrementing from
  the counter's reset value.

  @param
    This function takes no parameters.

  @return
    This function does not return a value.
 */
void
MSS_RTC_reset_counter
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_enable_irq() function enables the RTC wakeup output to interrupt
  the MSS when an alarm occurs. It enables the RTC wakeup interrupt
  (RTC_Wakeup_IRQn) in the PoalrFire SoC PLIC. The rtc_wakeup_plic_IRQHandler()
  function will be called when an RTC wakeup interrupt occurs.

  Note: The rtc_wakeup_plic_IRQHandler() default implementation is defined,
  with weak linkage, in the PoalrFire SoC MPFS HAL. You must provide your own
  implementation of the rtc_wakeup_plic_IRQHandler() function, which will
  override the default implementation, to suit your application.

  Note: This function only enables the RTC wakeup interrupt at the PolarFire SoC
  PLIC level. The alarm setting functions enable the wakeup interrupt output
  from the RTC.

   @param
     This function takes no parameters.

   @return
     This function does not return a value.
 */
void
MSS_RTC_enable_irq
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_disable_irq() function disables the RTC wakeup interrupt
  (RTC_WAKEUP_PLIC)in the PolarFire SoC MSS PLIC.
  Note: This function only disables the RTC wakeup interrupt at the PolarFire
  SoC PLIC level. It does not disable the wakeup interrupt output from the RTC.

   @param
     This function takes no parameters.

   @return
     This function does not return a value.
 */
void
MSS_RTC_disable_irq
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_clear_irq() function clears a pending wakeup interrupt from the
  RTC. This function does not clear the interrupt in the PolarFire SoC PLIC; it
  only clears the wakeup output from the RTC.
  Note: You must call the MSS_RTC_clear_irq() function as part of your
  implementation of the rtc_wakeup_plic_IRQHandler() RTC wakeup interrupt
  service routine (ISR) in order to prevent the same interrupt event
  retriggering a call to the ISR.

  @param
    This function takes no parameters.

  @return
    This function does not return a value.

  Example:
  The example code below demonstrates how the MSS_RTC_clear_irq() function is
  intended to be used as part of the RTC wakeup interrupt service routine used
  by an application to handle RTC alarms.
  @code
    #if defined(__GNUC__)
    __attribute__((__interrupt__)) void rtc_wakeup_plic_IRQHandler( void )
    #else
    void rtc_wakeup_plic_IRQHandler( void )
    #endif
    {
        process_alarm();
        MSS_RTC_clear_irq();
    }
  @endcode
*/
void
MSS_RTC_clear_irq
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_set_calendar_count_alarm() function sets up the RTC to generate an
  alarm when the RTC count reaches the time/date specified by the alarm_value
  parameter. The alarm asserts a wakeup interrupt to the MSS. This function
  enables the RTC's wakeup interrupt output, however the RTC wakeup interrupt
  input to the PolarFire SoC PLIC must be enabled separately by calling the
  MSS_RTC_enable_irq() function. The alarm can be disabled at any time by
  calling the MSS_RTC_disable_irq() function.

  Single-shot alarm
  The alarm can be a single-shot alarm, which will generate a single wakeup
  interrupt the first time the RTC count reaches the time/date specified by
  alarm_value. A single shot alarm is achieved by specifying a value for every
  field of the mss_rtc_calender_t data structure pointed to by the alarm_value
  parameter. The RTC counter will keep incrementing after a single shot alarm
  occurs.

  Periodic alarm
  The alarm can also be a periodic alarm, which will generate a wakeup interrupt
  every time the RTC count reaches the time/date specified by alarm_value, with
  the counter running in a continuous loop. The periodic alarm can be set to
  occur every minute, hour, day, month, year, week, day of the week, or any
  valid combination of these. This is achieved by setting some of the fields of
  the mss_rtc_calender_t data structure pointed to by the alarm_value parameter,
  to MSS_RTC_CALENDAR_DONT_CARE. For example, setting the weekday field to
  MSS_RTC_MONDAY and all other fields to MSS_RTC_CALENDAR_DONT_CARE will result
  in an alarm occurring every Monday. You can refine the time at which the alarm
  will occur by specifying values for the hour, minute and second fields.

  Note: This function must only be used when the RTC is configured to operate
        in calendar counter mode.


  @param alarm_value
    The alarm_value parameter is a pointer to an mss_rtc_calender_t data
    structure specifying the date and time at which the alarm is to occur. You
    must assign the required date and time values to the mss_rtc_calender_t
    structure before calling this function. Some of the fields within the
    mss_rtc_calender_t structure can be set to MSS_RTC_CALENDAR_DONT_CARE, to
    indicate that they are not to be considered in deciding when the alarm will
    occur; this is necessary when setting periodic alarms.

  @return
    This function does not return a value.

  Examples:

  The following example code demonstrates how to configure the RTC to generate a
  single calendar alarm at a specific date and time. The alarm will only occur
  once and the RTC will keep incrementing regardless of the alarm taking place.

  @code
    const mss_rtc_calender_t initial_calendar_count =
    {
        15u,     second
        30u,     minute
        6u,      hour
        6u,      day
        9u,      month
        12u,     year
        5u,      weekday
        37u      week
    };

    mss_rtc_calender_t alarm_calendar_count =
    {
        17u,     second
        30u,     minute
        6u,      hour
        6u,      day
        9u,      month
        12u,     year
        5u,      weekday
        37u      week
    };

    MSS_RTC_init(MSS_RTC_LO_BASE, MSS_RTC_BINARY_MODE, RTC_PERIPH_PRESCALER/ 10u );
    MSS_RTC_clear_irq();
    MSS_RTC_set_calendar_count(&initial_calendar_count);
    MSS_RTC_enable_irq();
    MSS_RTC_start();

    MSS_RTC_set_calendar_count_alarm(&alarm_calendar_count);
  @endcode

  The following example code demonstrates how to configure the RTC to generate a
  periodic calendar alarm. The RTC is configured to generate an alarm every
  Tuesday at 16:45:00. The alarm will reoccur every week until the RTC wakeup
  interrupt is disabled using a call to MSS_RTC_disable_irq().

  @code
    mss_rtc_calender_t initial_calendar_count =
    {
        58u,                            <--second
        59u,                            <--minute
        23u,                            <--hour
        10u,                            <--day
        9u,                             <--month
        12u,                            <--year
        MSS_RTC_MONDAY,                 <--weekday
        37u                             <--week
    };

    mss_rtc_calender_t alarm_calendar_count =
    {
        MSS_RTC_CALENDAR_DONT_CARE,     <--second
        45u,                            <--minute
        16u,                            <--hour
        MSS_RTC_CALENDAR_DONT_CARE,     <--day
        MSS_RTC_CALENDAR_DONT_CARE,     <--month
        MSS_RTC_CALENDAR_DONT_CARE,     <--year
        MSS_RTC_TUESDAY,                <--weekday
        MSS_RTC_CALENDAR_DONT_CARE      <--week
    };

    MSS_RTC_init(MSS_RTC_LO_BASE, MSS_RTC_BINARY_MODE, RTC_PERIPH_PRESCALER/ 10u );
    MSS_RTC_set_calendar_count(&initial_calendar_count);
    MSS_RTC_enable_irq();
    MSS_RTC_start();

    MSS_RTC_set_calendar_count_alarm(&alarm_calendar_count);
  @endcode

  The following example code demonstrates the code that you need to include in
  your application to handle alarms. It is the interrupt service routine for the
  RTC wakeup interrupt input to the PolarFire SoC PLIC. You need to add your
  application code in this function in place of the process_alarm() function but
  you must retain the call to MSS_RTC_clear_irq() to ensure that the same alarm
  does not retrigger the interrupt.

  @code
    #if defined(__GNUC__)
    __attribute__((__interrupt__)) void rtc_wakeup_plic_IRQHandler( void )
    #else
    void rtc_wakeup_plic_IRQHandler( void )
    #endif
    {
        process_alarm();
        MSS_RTC_clear_irq();
    }
  @endcode
 */
void
MSS_RTC_set_calendar_count_alarm
(
    const mss_rtc_calender_t * alarm_value
);

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_set_binary_count_alarm() function sets up the RTC to generate an
  alarm when the RTC count reaches the value specified by the alarm_value
  parameter. The alarm asserts a wakeup interrupt to the MSS. This function
  enables the RTC's wakeup interrupt output, however the RTC wakeup interrupt
  input to the PolarFire SoC PLIC must be enabled separately by calling the
  MSS_RTC_enable_irq() function. The alarm can be disabled at any time by
  calling the MSS_RTC_disable_irq() function.

  Single-shot alarm
  The alarm can be a single-shot alarm, which will generate a single wakeup
  interrupt the first time the RTC count reaches the value specified by the
  alarm_value parameter. Setting the alarm_value parameter to
  MSS_RTC_PERIODIC_ALARM produces a single-shot alarm. The RTC counter continues
  incrementing when a single shot alarm occurs.

  Periodic alarm
  The alarm can also be a periodic alarm, which will generate a wakeup interrupt
  every time the RTC count reaches the value specified by the alarm_value
  parameter. Setting the alarm_value parameter to MSS_RTC_SINGLE_SHOT_ALARM
  produces a periodic alarm. The RTC counter automatically wraps around to zero
  and continues incrementing when a periodic alarm occurs.

  Note: This function must only be used when the RTC is configured to operate
        in binary counter mode.

  @param alarm_value
    The alarm_value parameter is a 64-bit unsigned value specifying the RTC
    counter value that must be reached for the requested alarm to occur.

  @param alarm_type
    The alarm_type parameter specifies whether the requested alarm is a single
    shot or periodic alarm. It can only take one of these two values:
     - MSS_RTC_SINGLE_SHOT_ALARM,
     - MSS_RTC_PERIODIC_ALARM

  @return
    This function does not return a value.
 */
void
MSS_RTC_set_binary_count_alarm
(
    uint64_t alarm_value,
    mss_rtc_alarm_type_t alarm_type
);

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_get_update_flag() function indicates if the RTC counter has
  incremented since the last call to MSS_RTC_clear_update_flag(). It returns
  zero if no RTC counter increment has occurred. It returns a non-zero value if
  the RTC counter has incremented. This function can be used whether the RTC is
  configured to operate in calendar or binary counter mode.

  @return
   This function returns,

   |Value    |  Description                                              |
   |---------|-----------------------------------------------------------|
   |zero:    |  if the RTC has not incremented since the last call to    |
   |         |  MSS_RTC_clear_update_flag(),                             |
   |---------|-----------------------------------------------------------|
   |non-zero:|  if the RTC has incremented since the last call to        |
   |         |  MSS_RTC_clear_update_flag().                             |
   |         |                                                           |

  Example
  This example waits for the RTC timer to increment by one second.
  @code
    void wait_start_of_second(void)
    {
        uint32_t rtc_count_updated;
        MSS_RTC_clear_update_flag();
        do {
            rtc_count_updated = MSS_RTC_get_update_flag();
        } while(!rtc_count_updated)
    }
  @endcode
 */
uint32_t
MSS_RTC_get_update_flag
(
    void
);

/*-------------------------------------------------------------------------*//**
  The MSS_RTC_clear_update_flag() function clears the CONTROL register flag that
  is set when the RTC counter increments. It is used alongside function
  MSS_RTC_get_update_flag() to detect RTC counter increments.

  @return
    This function does not return a value.

  Example
  The example below will wait for the RTC timer to increment by one second.
  @code
  void wait_start_of_second(void)
  {
      uint32_t rtc_count_updated;
      MSS_RTC_clear_update_flag();
      do {
          rtc_count_updated = MSS_RTC_get_update_flag();
      } while(!rtc_count_updated)
  }
  @endcode
 */
void
MSS_RTC_clear_update_flag
(
    void
);

#ifdef __cplusplus
}
#endif

#endif /* MSS_RTC_H_ */
