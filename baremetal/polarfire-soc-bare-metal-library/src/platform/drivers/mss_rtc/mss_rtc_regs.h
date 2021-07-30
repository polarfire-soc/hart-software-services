 /*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Register bit offsets and masks definitions for PolarFire SoC MSS RTC Driver.
 *
 * SVN $Revision$
 * SVN $Date$
 */
#ifndef MSS_RTC_REG_H__
#define MSS_RTC_REG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mss_hal.h"

/******************************************************************************/
/*                Device Specific Peripheral registers structures             */
/******************************************************************************/
typedef struct
{
    volatile uint32_t CONTROL_REG             ;
    volatile uint32_t MODE_REG                ;
    volatile uint32_t PRESCALER_REG           ;
    volatile uint32_t ALARM_LOWER_REG         ;
    volatile uint32_t ALARM_UPPER_REG         ;
    volatile uint32_t COMPARE_LOWER_REG       ;
    volatile uint32_t COMPARE_UPPER_REG       ;
             uint32_t RESERVED0               ;
    volatile uint32_t DATE_TIME_LOWER_REG     ;
    volatile uint32_t DATE_TIME_UPPER_REG     ;

             uint32_t RESERVED1[2]            ;
    volatile uint32_t SECONDS_REG             ;
    volatile uint32_t MINUTES_REG             ;
    volatile uint32_t HOURS_REG               ;
    volatile uint32_t DAY_REG                 ;
    volatile uint32_t MONTH_REG               ;
    volatile uint32_t YEAR_REG                ;
    volatile uint32_t WEEKDAY_REG             ;
    volatile uint32_t WEEK_REG                ;

    volatile uint32_t SECONDS_CNT_REG         ;
    volatile uint32_t MINUTES_CNT_REG         ;
    volatile uint32_t HOURS_CNT_REG           ;
    volatile uint32_t DAY_CNT_REG             ;
    volatile uint32_t MONTH_CNT_REG           ;
    volatile uint32_t YEAR_CNT_REG            ;
    volatile uint32_t WEEKDAY_CNT_REG         ;
    volatile uint32_t WEEK_CNT_REG            ;
} RTC_TypeDef;

/******************************************************************************/
/*                         Peripheral declaration                             */
/******************************************************************************/
#define MSS_RTC_LO_ADDR                  0x20124000u
#define MSS_RTC_HI_ADDR                  0x28124000u

#define COMPARE_ALL_BITS                 0xFFFFFFFFu

#ifdef __cplusplus
}
#endif

#endif  /* MSS_RTC_REG_H__ */

