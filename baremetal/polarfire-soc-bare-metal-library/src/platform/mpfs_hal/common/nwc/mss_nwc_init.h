/*******************************************************************************
 * Copyright 2019 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * @file mss_nwc_init.h
 * @author Microchip FPGA Embedded Systems Solutions
 * @brief defines for mss_nwc_init.c
 *
 */

/*=========================================================================*//**
  @page MPFS MSS NWC configuration
  ==============================================================================
  @section intro_sec Introduction
  ==============================================================================
  The MPFS microcontroller subsystem (MSS) includes a number of hard core
  components physically located in the north west corner of the MSS on the die.

  ==============================================================================
  @section Items located in the north west corner
  ==============================================================================
  MSS PLL
  SGMII
  DDR phy
  MSSIO

  ==============================================================================
  @section Flow diagram
  ==============================================================================
  todo: remove, added line here as test *****
  Simplified flow diagram
                              +-----------------+
                              |     start       |
                              |  NWC setup      |
                              +-------+---------+
                                      v
                              +-----------------+
                              |   set SCB access|
                              |   Parameters    |
                              +-------+---------+
                                      |
                              +-------v---------+
                              | Release APB NWC |
                              | Turn on APB clk |
                              +-------+---------+
                                      |
                              +-------v---------+
                              | Set Dynamic     |
                              | enable bits     |
                              +-------++--------+
                                      |
                              +-------v---------+
                              | Setup signals   |
                              | DCE,CORE_UP,    |
                              | Flash_Valid,    |
                              | MSS_IO_EN       |
                              +-------+---------+
                                      |
                              +-------v---------+
                              | Setup SGMII     |
                              |                 |
                              +-------+---------+
                                      |
                              +-------v---------+
                              | Setup DDR       |
                              |                 |
                              +-------+---------+
                                      |
                              +-------v---------+
                              | Setup MSSIO     |
                              |                 |
                              +-------+---------+
                                      |
                              +-------v---------+
                              |    Finished     |
                              +-----------------+

 *//*=========================================================================*/
#ifndef __MSS_NWC_INIT_H_
#define __MSS_NWC_INIT_H_ 1


#include <stddef.h>
#include <stdint.h>
#include "../encoding.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DELAY_CYCLES_500_NS            ((uint32_t)(0.0000005 * LIBERO_SETTING_MSS_COREPLEX_CPU_CLK))
#define DELAY_CYCLES_1_MICRO           ((uint32_t)(DELAY_CYCLES_500_NS * 2U))
#define DELAY_CYCLES_5_MICRO           ((uint32_t)(DELAY_CYCLES_500_NS * 10U))
#define DELAY_CYCLES_50_MICRO          ((uint32_t)(DELAY_CYCLES_500_NS * 100U))
#define DELAY_CYCLES_150_MICRO         ((uint32_t)(DELAY_CYCLES_500_NS * 300U))
#define DELAY_CYCLES_250_MICRO         ((uint32_t)(DELAY_CYCLES_500_NS * 500U))
#define DELAY_CYCLES_500_MICRO         ((uint32_t)(DELAY_CYCLES_500_NS * 1000U))
#define DELAY_CYCLES_2MS               ((uint32_t)(DELAY_CYCLES_500_NS * 4000U))
#define DELAY_CYCLES_100MS             ((uint32_t)(DELAY_CYCLES_2MS * 50U))

/***************************************************************************//**

 */
typedef enum {
    DDR_BOOT_PROGRESS      = 0x00      //!< DDR_BOOT_PROGRESS
} MSS_REPORT_STATUS;

/*-------------------------------------------------------------------------*//**
 * delay()
 * Simple wait delay based on mcycles count
 * @param n Number of mcycles to wait.
 */
static inline void delay(uint32_t n)
{
    volatile uint64_t cycles_end = rdcycle() + n ;
    while (rdcycle() < cycles_end)
    {

    }
}

/***************************************************************************//**
  MSS_SCB_ACCESS_CONFIG_ON_RESET

  SCB access settings on reset.

  Bits 23:16 Sets the number of cycles that the bus is held mastered by the MSS
  after grant removal. During this period the MSS must not start any new SCB
  cycles. This allows for any active SCB cycles to complete.
  This must be four greater than the number of pipelines (4) in the SCB ring  from
  the MSS to G5C

  Bits 15:8 Sets how long SCB request is held active after SCB bus granted.
  Allows SCB bus master-ship to maintained across multiple SCB access
  cycles
  Bits 7:0 Set the timeout for an SCB access in CPU cycles.

  Note: These settings are used even after we change the MSS clock from SCB
  80MHz default setting.
  Min 143 Hclk cycles for simulation set-up, making 160
  */
#ifndef MSS_SCB_ACCESS_CONFIG_TIMEOUT
#define MSS_SCB_ACCESS_CONFIG_TIMEOUT           ((0x80U)&(0xFFU))
#endif
#ifndef MSS_SCB_ACCESS_CONFIG_REQUST_TIME
#define MSS_SCB_ACCESS_CONFIG_REQUST_TIME       ((160UL<<8U)&(0xFFU<<8U))
#endif
#ifndef MSS_SCB_ACCESS_CONFIG_BUSHOLD
#define MSS_SCB_ACCESS_CONFIG_BUSHOLD           ((8UL<<16U)&(0xFFU<<16U))
#endif

#ifndef MSS_SCB_ACCESS_CONFIG
#define MSS_SCB_ACCESS_CONFIG   (MSS_SCB_ACCESS_CONFIG_BUSHOLD|MSS_SCB_ACCESS_CONFIG_REQUST_TIME|MSS_SCB_ACCESS_CONFIG_TIMEOUT)
#endif

/***************************************************************************//**
  mss_nwc_init()
  Called on start-up, initializes clocks, sgmii, ddr, mssio
 */
uint8_t
mss_nwc_init
(
    void
);

/***************************************************************************//**
  mss_nwc_init_ddr()
  Called on start-up, initializes ddr
 */
uint8_t
mss_nwc_init_ddr
(
    void
);


/***************************************************************************//**
  mtime_delay(x) delay function, passes microseconds
  waits x microseconds
  Assumption 1 is we have ensured clock is 1MHz
  Assumption 2 is we have not setup tick timer when using this function. It is
  only used by the startup code.

  Example:
  @code

      mtime_delay(100UL);

  @endcode

 */
void
mtime_delay
(
    uint32_t microseconds
);

/***************************************************************************//**
    ddr_status()

    Application can create own function to show progress

    Example:
    @code

    ddr_status(void){
        static uint32_ count = 0U;
        count++;
        display_progress_banner(count);
    }

    @endcode

 */
void
ddr_report_progress
(
    void
);


#ifdef __cplusplus
}
#endif

#endif /* __MSS_DDRC_H_ */


