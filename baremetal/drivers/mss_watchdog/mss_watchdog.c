/*******************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions.  All rights reserved.
 * 
 * PolarFire SoC(PSE) microcontroller subsystem Watchdog bare metal software driver
 * implementation.
 *
 *
 * SVN $Revision: 10589 $
 * SVN $Date: 2018-11-23 05:49:09 +0000 (Fri, 23 Nov 2018) $
 */
#include "hal_assert.h"
#include "mss_sysreg.h"
#include "mss_hart_ints.h"
#include "mss_plic.h"
//#include "pse_util.h"

// TODO : stub this for now, resolve using full HAL when in git repo
void __enable_local_irq(int8_t intr);
void __disable_local_irq(int8_t intr);
// end TODO

#include "mss_watchdog.h"

#ifdef __cplusplus
extern "C" {
#endif 

/*These addresses are derived from http://homestead/asic/regmap/mss/html/g5soc_mss_regmap_AXID.html*/
WATCHDOG_TypeDef* wdog_hw_base[10] = {(WATCHDOG_TypeDef*)0x20001000,
                                      (WATCHDOG_TypeDef*)0x20101000,
                                      (WATCHDOG_TypeDef*)0x20103000,
                                      (WATCHDOG_TypeDef*)0x20105000,
                                      (WATCHDOG_TypeDef*)0x20107000,
                                      (WATCHDOG_TypeDef*)0x28001000,
                                      (WATCHDOG_TypeDef*)0x28101000,
                                      (WATCHDOG_TypeDef*)0x28103000,
                                      (WATCHDOG_TypeDef*)0x28105000,
                                      (WATCHDOG_TypeDef*)0x28107000};

/***************************************************************************//**
 * See mss_watchdog.h for details of how to use this function.
 */
void MSS_WD_get_config(mss_watchdog_num_t wd_num, mss_watchdog_config_t* config)
{
    config->time_val = wdog_hw_base[wd_num]->TIME;
    config->trigger_val = wdog_hw_base[wd_num]->TRIGGER;
    config->mvrp_val = wdog_hw_base[wd_num]->MSVP;

    config->forbidden_en = ((wdog_hw_base[wd_num]->CONTROL & MSS_WDOG_ENA_FORBIDDEN_MASK)
    						>> MSS_WDOG_ENA_FORBIDDEN);

    config->mvrp_intr_en = ((wdog_hw_base[wd_num]->CONTROL & MSS_WDOG_INTEN_MVRP_MASK)
    						>> MSS_WDOG_INTEN_MVRP);
}

/***************************************************************************//**
 * See mss_watchdog.h for details of how to use this function.
 */
void MSS_WD_configure(mss_watchdog_num_t wd_num, mss_watchdog_config_t* config)
{
    /*Note that the MSVP register value must always be <= TIMER register value.
     After any write to register from offset 0x00 to 0x0c the TRIGGER,MSVP and
     TIME registers are locked. Hence write them in proper sequence.
     */
    if(config->trigger_val <= WDOG_TRIGGER_MAX)
        wdog_hw_base[wd_num]->TRIGGER = config->trigger_val;

    if(config->time_val <= WDOG_TIMER_MAX)
        wdog_hw_base[wd_num]->MSVP = config->mvrp_val;

    if(config->time_val <= WDOG_TIMER_MAX)
        wdog_hw_base[wd_num]->TIME = config->time_val;

    wdog_hw_base[wd_num]->CONTROL =  ((config->mvrp_intr_en << MSS_WDOG_INTEN_MVRP) |
                                     (config->forbidden_en << MSS_WDOG_ENA_FORBIDDEN));

    /* Reload watchdog with new load if not in forbidden window. */
    if(!(MSS_WDOG_FORBIDDEN_MASK & wdog_hw_base[wd_num]->STATUS))
        wdog_hw_base[wd_num]->REFRESH = MSS_WDOG_REFRESH_KEY;
}

#ifdef __cplusplus
}
#endif

