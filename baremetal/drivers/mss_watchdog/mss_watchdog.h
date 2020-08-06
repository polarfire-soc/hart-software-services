/*******************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions.  All rights reserved.
 * 
 * PolarFire SoC (PSE) Microcontroller Subsystem (MSS) Watch-dog bare metal software
 * driver.
 *
 * SVN $Revision: 10592 $
 * SVN $Date: 2018-11-23 12:58:09 +0000 (Fri, 23 Nov 2018) $
 */
/*=========================================================================*//**
  @section intro_sec Introduction
  The PSE microcontroller subsystem (MSS) includes a watch-dog timer
  used to detect system lockups. This software driver provides a set of
  functions for controlling the MSS watch-dog as part of a bare metal system
  where no operating system is available. The driver can be adapted for use as
  part of an operating system, but the implementation of the adaptation layer
  between the driver and the operating system's driver model is outside the
  scope of the driver.
  
  The MSS watch-dog driver provides support for the following features:
    - Initialization of the MSS watch-dog
    - Reading the current value and status of the watch-dog timer
    - Refreshing the watch-dog timer value
    - Enabling, disabling and clearing time-out and wake-up interrupts.
    
  @section hw_dependencies Hardware Flow Dependencies
  The MSS watch-dog must be configured by the PSE MSS configurator in
  the Libero hardware flow before using this driver in a firmware project. The
  MSS configurator is responsible for the configuration of the MSS watch-dog's
  mode of operation, reload value and refresh window. The MSS configurator sets
  the power-on reset state of the system registers that control the
  configuration of the MSS watch-dog.
  The MSS watch-dog configuration options are as follows:
    - Enable or disable the watch-dog after a power-on or system reset
    - Generate a system reset or an interrupt if a watch-dog counter timeout
      occurs
    - Set the reload value. This is the value that is loaded into the watch-dog
      counter when it is refreshed
    - Set the counter threshold value. This sets up permitted and forbidden
      refresh windows for the watch-dog counter
    - Lock the system register controlling watch-dog enable/disable and timeout
      mode to the selected configuration (read-only) or allow it to be modified
      by firmware
  This driver assumes that the Libero hardware flow enables the MSS watch-dog,
  selects its timeout mode and locks the system register that controls the
  configuration of the MSS watch-dog. If you want to have firmware control of
  the MSS watch-dog enable/disable and its timeout mode, you must ensure that
  the MSS configurator allows read-write access to the system register that
  controls the configuration of the MSS watch-dog. The driver does not provide
  functions to write to this system register.
  
  @section theory_op Theory of Operation
  The MSS watch-dog driver functions are grouped into the following categories:
    - Initialization  and configuration
    - Reading the current value and status of the watch-dog timer
    - Refreshing the watch-dog timer value
    - Support for enabling, disabling and clearing time-out and wake-up interrupts.
    
  Initialization and Configuration
  The MSS configurator in the Libero hardware flow is responsible for the
  configuration of the MSS watch-dog's mode of operation, reload value and
  refresh window. It configures the watch-dog to generate an interrupt or a
  system reset when its counter times out. It also configures the MSS watch-dog
  to be enabled or disabled when power-on reset is de-asserted.
  Note: The MSS watch-dog cannot be enabled or disabled by the driver.

  The occurrence of a time out event before the last system reset can be
  detected through the use of the MSS_WD_triggered() function. This
  function would be typically used at the start of the application to detect
  whether the application is starting as a result of a power-on reset or a
  watch-dog reset. The time out event must be cleared through a call to function
  MSS_WD_clear_timeout_event() in order to allow the detection of subsequent
  time out events or differentiating between a RISC-V initiated system reset
  and watch-dog reset.

  Reading the Watchdog Timer Value and Status
  The current value of the watch-dog timer can be read using the
  MSS_WD_current_value() function. The watch-dog status can be read using the
  MSS_WD_forbidden() function. These functions are typically required when using
  the watch-dog configured with a permitted refresh window to check whether a
  watch-dog reload is currently allowed. When the current value of the watch-dog
  timer is greater than the permitted refresh window value, refreshing the
  watch-dog is forbidden. Attempting to refresh the watch-dog timer in the
  forbidden window will cause a reset or interrupt, depending on the watch-dog
  configuration. The forbidden refresh window can be disabled by the MSS
  configurator in the hardware flow by specifying a permitted refresh window
  value equal to or higher than the watch-dog reload value.
  
  Refreshing the Watchdog Timer Value
  The watch-dog timer value is refreshed using the MSS_WD_reload() function. The
  value reloaded into the watch-dog timer down-counter is specified in the Libero
  hardware flow using the MSS configurator.

  Interrupt Control
  The watch-dog timer can generate interrupts instead of resetting the system
  when its down counter timer expires. The MSS watch-dog driver provides the
  following functions to control timeout interrupts:
    - MSS_WD_enable_timeout_irq
    - MSS_WD_disable_timeout_irq
    - MSS_WD_clear_trigger_irq
  The watch-dog timer is external to the RISC-V processor core and operates
  even when the RISC-V is in sleep mode. A wakeup interrupt can be generated
  by the watch-dog timer to wake up the RISC-V when the watch-dog timer value
  reaches the permitted refresh window while the RISC-V is in sleep mode. The
  MSS watch-dog driver provides the following functions to control wake-up
  interrupts:
    - MSS_WD_enable_mvrp_irq
    - MSS_WD_disable_mvrp_irq
    - MSS_WD_clear_mvrp_irq
    
 *//*=========================================================================*/

#ifndef MSS_WATCHDOG_H_
#define MSS_WATCHDOG_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "mss_assert.h"
#include "mss_sysreg.h"
#include <stdint.h>
#include "mss_hart_ints.h"
#include "mss_plic.h"

#define WDOG_ENABLE                 1u
#define WDOG_DISABLE                0u

/*Use the logical OR of these constants to enable interrupt*/
#define WDOG_INTR_TYPE_LOCAL_E51                1u
#define WDOG_INTR_TYPE_LOCAL_U54                2u
#define WDOG_INTR_TYPE_GLOBAL_PLIC              4u

#define MSS_WDOG_REFRESH

#define MSS_WDOG_INTEN_MVRP             0u
#define MSS_WDOG_INTEN_TRIG             1u
#define MSS_WDOG_INTEN_SLEEP            2u
#define MSS_WDOG_ACTIVE_SLEEP           3u
#define MSS_WDOG_ENA_FORBIDDEN          4u

#define MSS_WDOG_INTEN_MVRP_MASK        ( 1u << MSS_WDOG_INTEN_MVRP)
#define MSS_WDOG_INTEN_TRIG_MASK        ( 1u << MSS_WDOG_INTEN_TRIG)
#define MSS_WDOG_INTEN_SLEEP_MASK       ( 1u << MSS_WDOG_INTEN_SLEEP)
#define MSS_WDOG_ACTIVE_SLEEP_MASK      ( 1u << MSS_WDOG_ACTIVE_SLEEP)
#define MSS_WDOG_ENA_FORBIDDEN_MASK     ( 1u << MSS_WDOG_ENA_FORBIDDEN)

#define MSS_WDOG_MVRP_TRIPPED           0u
#define MSS_WDOG_WDOG_TRIPPED           1u
#define MSS_WDOG_FORBIDDEN              2u
#define MSS_WDOG_TRIGGERED              3u
#define MSS_WDOG_LOCKED                 4u
#define MSS_WDOG_DEVRST                 5u

#define MSS_WDOG_MVRP_TRIPPED_MASK      ( 1u << MSS_WDOG_MVRP_TRIPPED)
#define MSS_WDOG_WDOG_TRIPPED_MASK      ( 1u << MSS_WDOG_WDOG_TRIPPED)
#define MSS_WDOG_FORBIDDEN_MASK         ( 1u << MSS_WDOG_FORBIDDEN)
#define MSS_WDOG_TRIGGERED_MASK         ( 1u << MSS_WDOG_TRIGGERED)
#define MSS_WDOG_LOCKED_MASK            ( 1u << MSS_WDOG_LOCKED)
#define MSS_WDOG_DEVRST_MASK            ( 1u << MSS_WDOG_DEVRST)

#define MSS_WDOG_TIME
#define MSS_WDOG_MSVP
#define MSS_WDOG_TRIGGER
#define MSS_WDOG_FORCE


#define WDOG_TRIGGER_MAX    4095U
#define WDOG_TIMER_MAX      16777200U            /*0xFFFFFFU*/

/***************************************************************************//**
 The mss_watchdog_num_t is the Watchdog module number enumeration.
 The MSS_WDOG0_LO to MSS_WDOG4_LO Correspond to the Watchdog module number
 0 to 4 when the appear on the AXI switch Slave 5.
 The MSS_WDOG0_HI to MSS_WDOG4_HI Correspond to the Watchdog module number
 0 to 4 when the appear on the AXI switch Slave 6.
*/
typedef enum {
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
  The WATCHDOG_TypeDef is the hardware register structure for the PSE MSS Watchdog.
 */
typedef struct
{
    volatile   uint32_t  REFRESH;
    volatile   uint32_t  CONTROL;
    volatile   uint32_t  STATUS;
    volatile   uint32_t  TIME;
    volatile   uint32_t  MSVP;            /*Maximum Value Refresh Permitted Register*/
    volatile   uint32_t  TRIGGER;
    volatile   uint32_t  FORCE;
} WATCHDOG_TypeDef;

/***************************************************************************//**
  The mss_watchdog_config_t type for the watch-dog Configuration structure. This
  type is used as a parameter for the MSS_WD_configure() function and the
  MSS_WD_get_config() function.

  Following are the values as part of this structure

    time_val        ==> The TIME register value
    mvrp_val        ==> The MSVP register value. Maximum value refresh permitted.
    trigger_val     ==> The TRIGGER register value.
    forbidden_en    ==> The CONTROL register, ENABLE_FORBIDDEN bit value.
                        When set if a write occurs in the forbidden region the
                        watch-dog will trigger.
                        The forbidden region is the count value between the TIME and
                        the MSVP register values during which the refresh is
                        not allowed.

    wd_in_sleep     ==> The CONTROL register, ACTIVE_SLEEP bit value
                        When set the watch-dog is operational when the CPU is
                        sleeping

    mvrp_intr_in_sleep ==> The CONTROL register, INTEN_SLEEP bit value.
                           Enables the MVRP interrupt when the MVRP level is
                           passed and the CPU is sleeping. This holds good even
                           if MVRP interrupt is not enabled.

    mvrp_intr_en    ==> The CONTROL register, INTEN_MSVP bit value. Eanbles the
                        MVRP interrupt.

       * TIME = 0xFFFFF0
       * MSVP = 0x989680,
       * TRIGGER = 0x3e8
       *
       * A presaclar = 160 is used on the MSS PAB clock. (= AXI CLOCK = 25MHz)
       *
       * With these default setting the MVRP interrupt will happen after
       * (0xFFFFF0 - 0x989680) * ( 1/25MHz/160)
       * mvrp interrupt will happen after 43.3 sec. after reset

       * * (0xFFFFF0 - 0x3e8) * ( 1/25MHz/160)
       * trigger interrupt will happen after 107.3 sec. after reset
       *
       *
 */
typedef struct mss_watchdog_config mss_watchdog_config_t;

/*mvrp_intr_in_sleep is removed since it does not have effect as CPUs dont support sleep*/
struct mss_watchdog_config{
    uint32_t                time_val;
    uint32_t                mvrp_val;
    uint32_t                trigger_val;
    uint8_t                 forbidden_en;
    uint8_t                 mvrp_intr_en;
    uint8_t                 intr_type;
};

/*These addresses are derived from http://homestead/asic/regmap/mss/html/g5soc_mss_regmap_AXID.html*/
extern WATCHDOG_TypeDef* wdog_hw_base[10];

/***************************************************************************//**
  The MSS_WDOG_REFRESH_KEY macro holds the magic value which will cause a reload
  of the watch-dog's down counter when written to the watch-dog's WDOGREFRESH
  register.
 */
#define MSS_WDOG_REFRESH_KEY    (uint32_t)0xDEADC0DEU


/***************************************************************************//**
  The MSS_WDOG_FORCE_RESET_KEY macro holds the magic value which will force a
  reset if the Watchdog is already triggered (gone past the TRIGGER register value).
  Writing Any other value or writing this register at other times will trigger
  the watch-dog NMI sequence (i.e Raise the TRIGGER interrupt).
 */
#define MSS_WDOG_FORCE_RESET_KEY    (uint32_t)0xDEADU

/***************************************************************************//**
  The MSS_WD_get_config() function returns the current configuration of the
  PSE Watchdog module. The Watchdog module is pre-initialized by the flash bits
  at the design time. When used for the first time before calling the
  MSS_WD_configure() function, this function will return the default configuration
  as configured at the design time. You can reconfigure the Watchdog module
  using MSS_WD_configure() function. A call to MSS_WD_get_config() function will
  then return the current configuration values set by a previous call to
  MSS_WD_configure() function. You may not need to use this function if you do
  not want to know what the current configurations are. In that case, you can
  directly use the MSS_WD_configure() function to configure the Watchdog module
  to the values of your choice.

  Note that theMSS_WD_configure() function can be used only once, as the it writes
  into the TIME register. After a write into the TIME register, the TIME, TRIGGER
  and MSVP register values are frozen and can't be altered again.

 @param wd_num
    The wd_num parameter is the Watchdog module number in the PSE MSS on which
    the operation needs to be performed. The Watchdog module number can be
    chosen using mss_watchdog_num_t. The MSS_WDOG0_LO to MSS_WDOG4_LO Correspond
    to the Watchdog module number 0 to 5 when the appear on the AXI switch
    Slave 5.
    The MSS_WDOG0_HI to MSS_WDOG4_HI Correspond
    to the Watchdog module number 0 to 5 when the appear on the AXI switch
    Slave 6.

@param config
    The config parameter is the return parameter in which the current configurations
    of the watch-dog module will be stored.

    Following are the values returned as part of this parameter

    time_val        ==> The TIME register value
    mvrp_val        ==> The MSVP register value. Maximum value refresh permitted.
    trigger_val     ==> The TRIGGER register value.
    forbidden_en    ==> The CONTROL register, ENABLE_FORBIDDEN bit value.
                        When set if a write occurs in the forbidden region the
                        watch-dog will trigger.
                        The forbidden region is the count value between the TIME and
                        the MSVP register values during which the refresh is
                        not allowed.

    wd_in_sleep     ==> The CONTROL register, ACTIVE_SLEEP bit value
                        When set the watch-dog is operational when the CPU is
                        sleeping

    mvrp_intr_in_sleep ==> The CONTROL register, INTEN_SLEEP bit value.
                           Enables the MVRP interrupt when the MVRP level is
                           passed and the CPU is sleeping. This holds good even
                           if MVRP interrupt is not enabled.

    mvrp_intr_en    ==> The CONTROL register, INTEN_MSVP bit value. Eanbles the
                        MVRP interrupt.

  @return
    This function does not return any value.


  Example:
  @code
  #include "mss_watchdog.h"
  mss_watchdog_config_t wd0lo_config;

  int main( void )
  {
      MSS_WD_get_config(MSS_WDOG0_LO, &wd0lo_config);

      wd0lo_config.forbidden_en = WDOG_ENABLE;
      wd0lo_config.mvrp_intr_en = WDOG_ENABLE;
      wd0lo_config.mvrp_intr_in_sleep = WDOG_ENABLE;
      wd0lo_config.mvrp_val = 0xFFFF000;

      MSS_WD_configure(MSS_WDOG0_LO, &wd0lo_config);

      for(;;)
      {
          main_task();
      }
  }
*/
void MSS_WD_get_config(mss_watchdog_num_t wd_num, mss_watchdog_config_t* config);

/***************************************************************************//**
  The MSS_WD_configure() function configures the chosen watch-dog module. The
  Watchdog module is pre-initialized by the flash bits at the design time.
  You can reconfigure the Watchdog module using MSS_WD_configure() function.

  Note that theMSS_WD_configure() function can be used only once, as the it writes
  into the TIME register. After a write into the TIME register, the TIME, TRIGGER
  and MSVP register values are frozen and can't be altered again.

 @param wd_num
    The wd_num parameter is the Watchdog module number in the PSE MSS on which
    the operation needs to be performed. The Watchdog module number can be
    chosen using mss_watchdog_num_t. The MSS_WDOG0_LO to MSS_WDOG4_LO Correspond
    to the Watchdog module number 0 to 5 when the appear on the AXI switch
    Slave 5.
    The MSS_WDOG0_HI to MSS_WDOG4_HI Correspond
    to the Watchdog module number 0 to 5 when the appear on the AXI switch
    Slave 6.

@param config
    The config parameter is the input parameter in which the configurations to be
    applied to the watch-dog module are provided by the application.
    Following are the values are can be provided to the driver as part of this
    parameter

    time_val        ==> The TIME register value
    mvrp_val        ==> The MSVP register value. Maximum value refresh permitted.
    trigger_val     ==> The TRIGGER register value.
    forbidden_en    ==> The CONTROL register, ENABLE_FORBIDDEN bit value.
                        When set if a write occurs in the forbidden region the
                        watch-dog will trigger.
                        The forbidden region is the count value between the TIME and
                        the MSVP register values during which the refresh is
                        not allowed.

    wd_in_sleep     ==> The CONTROL register, ACTIVE_SLEEP bit value
                        When set the watch-dog is operational when the CPU is
                        sleeping

    mvrp_intr_in_sleep ==> The CONTROL register, INTEN_SLEEP bit value.
                           Enables the MVRP interrupt when the MVRP level is
                           passed and the CPU is sleeping. This holds good even
                           if MVRP interrupt is not enabled.

    mvrp_intr_en    ==> The CONTROL register, INTEN_MSVP bit value. Enables the
                        MVRP interrupt.

  @return
    This function does not return any value.

  Example:
  @code
  #include "mss_watchdog.h"
  mss_watchdog_config_t wd0lo_config;

  int main( void )
  {
      MSS_WD_get_config(MSS_WDOG0_LO, &wd0lo_config);

      wd0lo_config.forbidden_en = WDOG_ENABLE;
      wd0lo_config.mvrp_intr_en = WDOG_ENABLE;
      wd0lo_config.mvrp_intr_in_sleep = WDOG_ENABLE;
      wd0lo_config.mvrp_val = 0xFFFF000;

      MSS_WD_configure(MSS_WDOG0_LO, &wd0lo_config);

      for(;;)
      {
          main_task();
      }
  }
*/
void MSS_WD_configure(mss_watchdog_num_t wd_num, mss_watchdog_config_t* config);

/***************************************************************************//**
  The MSS_WD_reload() function causes the watch-dog to reload its down-counter
  timer with the load value configured through the the MSS configurator in the
  hardware flow. This function must be called regularly to avoid a system reset
  or a watch-dog interrupt.
 
  @return
    This function does not return a value.
 */
static inline void MSS_WD_reload(mss_watchdog_num_t wd_num)
{
    wdog_hw_base[wd_num]->REFRESH = MSS_WDOG_REFRESH_KEY;
}

/***************************************************************************//**
  The MSS_WD_current_value() function returns the current value of the
  watch-dog's down-counter.
 
  @return
    This function returns the current value of the watch-dog’s down-counter as
    a 32-bit unsigned integer.
 */
static inline uint32_t MSS_WD_current_value(mss_watchdog_num_t wd_num)
{
    return wdog_hw_base[wd_num]->REFRESH;
}

/***************************************************************************//**
  The MSS_WD_forbidden() function returns the refresh status of the watch-dog.
 
  @return 
    This function returns the refresh status of the watch-dog. A value of 1
    indicates that watch-dog's down-counter is within the forbidden window and
    that a reload should not be done. A value of 0 indicates that the watch-dog's
    down counter is within the permitted window and that a reload is allowed.
 */
static __inline uint32_t MSS_WD_forbidden(mss_watchdog_num_t wd_num)
{
    return ((wdog_hw_base[wd_num]->STATUS & MSS_WDOG_FORBIDDEN_MASK) >> MSS_WDOG_FORBIDDEN);
}

/***************************************************************************//**
  The MSS_WD_enable_mvrp_irq() function enables the MVRP interrupt.
  The WdogWakeup_IRQHandler() function will be called when a wake-up
  interrupt occurs.
  Note: A WdogWakeup_IRQHandler() default implementation is weakly defined in
        the PSE HAL. You must provide your own implementation of
        the WdogWakeup_IRQHandler() function, which will override the default
        implementation, to suit your application.


  Note: This function must be called from appropriate HART context.

  @return
    This function does not return a value.

  Example:
  @code
  #include "mss_watchdog.h"
  int main( void )
  {

      MSS_WD_enable_mvrp_irq();
      for (;;)
      {
          main_task();
          cortex_sleep();
      }
  }

  void WdogWakeup_IRQHandler( void )
  {
      process_wakeup();
      MSS_WD_clear_mvrp_irq();
  }
  @endcode
 */
static inline void MSS_WD_enable_mvrp_irq(mss_watchdog_num_t wd_num, uint8_t intr_type)
{
	mss_watchdog_num_t temp_num = MSS_WDOG0_HI;

	if(wd_num <= MSS_WDOG4_HI)
	{
		if(wd_num > MSS_WDOG4_LO)
			temp_num = wd_num - MSS_WDOG0_HI;
		else
			temp_num = wd_num;

		if(((MSS_WDOG0_LO == wd_num) || (MSS_WDOG0_HI == wd_num)) &&
				(intr_type & WDOG_INTR_TYPE_LOCAL_E51))
			__enable_local_irq(WDOG0_MVRP_E51_INT);

		if(intr_type & WDOG_INTR_TYPE_LOCAL_U54)
			__enable_local_irq(WDOGx_MVRP_U54_INT);

		if(intr_type & WDOG_INTR_TYPE_GLOBAL_PLIC)
			PLIC_EnableIRQ(WDOG0_MRVP_PLIC + temp_num);

		wdog_hw_base[wd_num]->CONTROL |= MSS_WDOG_INTEN_MVRP_MASK;
	}
}

/***************************************************************************//**
  The MSS_WD_disable_mvrp_irq() function disables the generation of the
  MVRP interrupt.
 
  Note: This function must be called from appropriate HART context.

  @return
    This function does not return a value.
 */
static __inline void MSS_WD_disable_mvrp_irq(mss_watchdog_num_t wd_num, uint8_t intr_type)
{
    mss_watchdog_num_t temp_num = MSS_WDOG0_HI;

	if(wd_num <= MSS_WDOG4_HI)
	{
	    wdog_hw_base[wd_num]->CONTROL &= ~(MSS_WDOG_INTEN_MVRP_MASK);

	    if(wd_num > MSS_WDOG4_LO)
			temp_num = wd_num - MSS_WDOG0_HI;
		else
			temp_num = wd_num;

		if(((MSS_WDOG0_LO == wd_num) || (MSS_WDOG0_HI == wd_num)) &&
				(intr_type & WDOG_INTR_TYPE_LOCAL_E51))
			__disable_local_irq(WDOG0_MVRP_E51_INT);

		if(intr_type & WDOG_INTR_TYPE_LOCAL_U54)
			__disable_local_irq(WDOGx_MVRP_U54_INT);

		if(intr_type & WDOG_INTR_TYPE_GLOBAL_PLIC)
			PLIC_DisableIRQ(WDOG0_MRVP_PLIC + temp_num);
	}
}

/***************************************************************************//**
  The MSS_WD_enable_trigger_irq() function enables the TRIGGER interrupt.
  The wdog0_tout_u51_local_IRQHandler_9() function will be called when a Trigger
  interrupt occurs on WD0. Same for the HART 1 to 4
  Note: A wdog0_tout_e51_local_IRQHandler_9() default implementation is weakly
        defined in  the PSE HAL. You must provide your own implementation of
        the wdog0_tout_u51_local_IRQHandler_9() function, which will override
        the default implementation, to suit your application. Same for the HART 1 to 4.

  Note: This function must be called from appropriate HART context.

  @return
    This function does not return a value.

  Example:
  @code

  @endcode
 */
static inline void MSS_WD_enable_trigger_irq(mss_watchdog_num_t wd_num, uint8_t intr_type)
{
	mss_watchdog_num_t temp_num = MSS_WDOG0_HI;

	if(wd_num <= MSS_WDOG4_HI)
	{
		if(wd_num > MSS_WDOG4_LO)
			temp_num = wd_num - MSS_WDOG0_HI;
		else
			temp_num = wd_num;

		if(intr_type & WDOG_INTR_TYPE_LOCAL_E51)
			__enable_local_irq(WDOG0_TOUT_E51_INT - temp_num);

		if(intr_type & WDOG_INTR_TYPE_LOCAL_U54)
			__enable_local_irq(WDOGx_TOUT_U54_INT);

		if(intr_type & WDOG_INTR_TYPE_GLOBAL_PLIC)
			PLIC_EnableIRQ(WDOG0_TOUT_PLIC + temp_num);
	}
}

/***************************************************************************//**
  The MSS_WD_disable_trigger_irq() function disables the generation of the
  TRIGGER interrupt. The trigger interrupt is enabled by default in the WD CONTROL
  register and cant be disabled. Only Disabling the Local interrupt on the E51.

  Note: This function must be called from appropriate HART context.

  @return
    This function does not return a value.
 */
static __inline void MSS_WD_disable_trigger_irq(mss_watchdog_num_t wd_num, uint8_t intr_type)
{
	mss_watchdog_num_t temp_num = MSS_WDOG0_HI;

	if(wd_num <= MSS_WDOG4_HI)
	{
		if(wd_num > MSS_WDOG4_LO)
			temp_num = wd_num - MSS_WDOG0_HI;
		else
			temp_num = wd_num;

		if(intr_type & WDOG_INTR_TYPE_LOCAL_E51)
			__disable_local_irq(WDOG0_TOUT_E51_INT - temp_num);

		if(intr_type & WDOG_INTR_TYPE_LOCAL_U54)
			__disable_local_irq(WDOGx_TOUT_U54_INT);

		if(intr_type & WDOG_INTR_TYPE_GLOBAL_PLIC)
			PLIC_DisableIRQ(WDOG0_TOUT_PLIC + temp_num);
	}
}

/***************************************************************************//**
  The MSS_WD_clear_trigger_irq() function clears the watch-dog’s timeout/trigger
  interrupt which is connected to the RISC-V NMI interrupt. Calling
  MSS_WD_clear_trigger_irq() results in clearing the RISC-V NMI interrupt.
  Note: You must call the MSS_WD_clear_trigger_irq() function as part of your
        implementation of the wdog0_tout_u51_local_IRQHandler_9() timeout
        interrupt service routine (ISR) in order to prevent the same interrupt
        event re-triggering a call to the timeout ISR.

  Note: This function must be called from appropriate HART context.

  @return
    The example below demonstrates the use of the MSS_WD_clear_trigger_irq()
    function as part of the NMI interrupt service routine.

    Example:
  @code
  void NMI_Handler( void )
  {
      process_timeout();
      MSS_WD_clear_trigger_irq();
  }
  @endcode
 */
static inline void MSS_WD_clear_trigger_irq(mss_watchdog_num_t wd_num)
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

/***************************************************************************//**
  The MSS_WD_clear_mvrp_irq() function clears the wakeup interrupt. This
  function also clears the interrupt in the RISC-V interrupt controller
  through a call to NVIC_ClearPendingIRQ().
  Note: You must call the MSS_WD_clear_mvrp_irq() function as part of  your
        implementation of the wdog0_msvp_u51_local_IRQHandler_10() wake-up interrupt service
        routine (ISR) in order to prevent the same interrupt event re-triggering
        a call to the wake-up ISR.
        
  @return
    This function does not return a value.

    Example:
    The example below demonstrates the use of the MSS_WD_clear_mvrp_irq() function
    as part of the wake-up interrupt service routine.
    @code
    void wdog0_msvp_u51_local_IRQHandler_10( void )
    {
        do_interrupt_processing();
        
        MSS_WD_clear_mvrp_irq();
    }
    @endcode
*/
static inline void MSS_WD_clear_mvrp_irq(mss_watchdog_num_t wd_num)
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

/***************************************************************************//**
  The MSS_WD_triggered() function reports the occurrence of a timeout
  event. It can be used to detect if the last RISC-V reset was due to a
  watch-dog timeout or a power-on reset.
 
  @return
    0: No watch-dog timeout event occurred.
    1: A timeout event occurred.

  Example:
  @code
  #include "mss_watchdog.h"
  int main( void )
  {
      uint32_t wdg_reset;

      wdg_reset = MSS_WD_triggered();
      if(wdg_reset)
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
static inline uint32_t MSS_WD_triggered(mss_watchdog_num_t wd_num)
{
    return ((wdog_hw_base[wd_num]->STATUS & MSS_WDOG_TRIGGERED_MASK) >> MSS_WDOG_TRIGGERED);
}

/***************************************************************************//**
  The MSS_WD_force_reset() function disables the generation of the
  MVRP interrupt.

  @return
    This function does not return a value.
 */
static __inline void MSS_WD_force_reset(mss_watchdog_num_t wd_num)
{
    if(MSS_WDOG_TRIGGERED_MASK == (wdog_hw_base[wd_num]->STATUS | MSS_WDOG_TRIGGERED_MASK))
        wdog_hw_base[wd_num]->FORCE = 0xDEADU;
    else
        wdog_hw_base[wd_num]->FORCE = 0x0U;
}


#ifdef __cplusplus
}
#endif

#endif /* MSS_WATCHDOG_H_ */

