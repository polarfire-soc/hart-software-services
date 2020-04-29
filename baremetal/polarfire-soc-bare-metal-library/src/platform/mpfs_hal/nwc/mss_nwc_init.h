/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/*******************************************************************************
 * @file mss_nwc_init.h
 * @author Microchip-FPGA Embedded Systems Solutions
 * @brief defines for mss_nwc_init.c
 *
 * SVN $Revision: 11867 $
 * SVN $Date: 2019-07-29 19:26:04 +0100 (Mon, 29 Jul 2019) $
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


#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************//**
  MSS_SCB_ACCESS_CONFIG_ON_RESET

  SCB access settings on reset.
  Bits 15:8 Sets how long SCB request is held active after SCB bus granted.
  Allows SCB bus master-ship to maintained across multiple SCB access
  cycles
  Bits 7:0 Set the timeout for an SCB access in CPU cycles.

  Note: These settings are used even after we change the MSS clock from SCB
  80MHz default setting. todo: This needs to be confirmed as OK, there will be
  no potential timing issues:
  Min 143 Hclk cycles for simulation set-up, making 160
  todo: review setting
  */

#define MSS_SCB_ACCESS_CONFIG   ((160UL<<8U)|(0x80U))


/***************************************************************************//**
  MSS_NWC_init()
  Called on start-up, initializes clocks, sgmii, ddr, mssio
 */
uint8_t
MSS_NWC_init
(
    void
);


#ifdef __cplusplus
}
#endif

#endif /* __MSS_DDRC_H_ */


