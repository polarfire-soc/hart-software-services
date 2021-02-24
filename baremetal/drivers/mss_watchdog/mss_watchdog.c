/***************************************************************************//**
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC (MPFS) Microprocessor SubSystem Watchdog bare metal software
 * driver implementation.
 *
 *
 */

#include "mss_hal.h"
#include "mss_watchdog.h"

#ifdef __cplusplus
extern "C" {
#endif

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
uint8_t MSS_WD_configure
(
    mss_watchdog_num_t wd_num,
    const mss_watchdog_config_t * config
)
{
    uint8_t error = 0u;

    /* Note that the MSVP register value must always be <= TIMER register value.
      After any write to register from offset 0x00 to 0x0c the TRIGGER,MSVP and
      TIME registers are locked. Hence write them in proper sequence.
     */
    if (config->timeout_val <= MSS_WDOG_TRIGGER_MAX)
    {
        wdog_hw_base[wd_num]->TRIGGER = config->timeout_val;
    }
    else
    {
        error = 1u;
    }

    if (config->time_val <= MSS_WDOG_TIMER_MAX)
    {
        wdog_hw_base[wd_num]->MSVP = config->mvrp_val;
    }
    else
    {
        error = 1u;
    }

    if (config->time_val <= MSS_WDOG_TIMER_MAX)
    {
        wdog_hw_base[wd_num]->TIME = config->time_val;
    }
    else
    {
        error = 1u;
    }

    wdog_hw_base[wd_num]->CONTROL =  (uint32_t)(config->forbidden_en <<
                                                         MSS_WDOG_ENA_FORBIDDEN);

    /* Reload watchdog with new load if it is not in forbidden window. */
    if (!(MSS_WDOG_FORBIDDEN_MASK & wdog_hw_base[wd_num]->STATUS))
    {
        wdog_hw_base[wd_num]->REFRESH = MSS_WDOG_REFRESH_KEY;
    }
    else
    {
        error = 1u;
    }

    return(error);
}

/***************************************************************************//**
 * See mss_watchdog.h for details of how to use this function.
 */
void MSS_WD_get_config
(
    mss_watchdog_num_t wd_num, mss_watchdog_config_t* config
)
{
    if((WATCHDOG_TypeDef*)0 != wdog_hw_base[wd_num])
    {
        config->time_val = wdog_hw_base[wd_num]->TIME;
        config->timeout_val = wdog_hw_base[wd_num]->TRIGGER;
        config->mvrp_val = wdog_hw_base[wd_num]->MSVP;

        config->forbidden_en = (uint8_t)((wdog_hw_base[wd_num]->CONTROL &
                                MSS_WDOG_ENA_FORBIDDEN_MASK)
                                                        >> MSS_WDOG_ENA_FORBIDDEN);
    }
}

#ifdef __cplusplus
}
#endif
