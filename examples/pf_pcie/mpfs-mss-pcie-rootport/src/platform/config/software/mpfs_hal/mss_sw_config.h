/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/*******************************************************************************
 *
 * Platform definitions
 * Version based on requirements of MPFS MSS
 *
 */
 /*=========================================================================*//**
  @mainpage Sample file detailing how user_config.h should be constructed for
    the MPFS MSS

    @section intro_sec Introduction
    The  user_config.h is to be located in the project ./config/user_config/ directory.
    This file must be hand crafted when using the MPFS MSS.

    You can use this file as sample.
    Rename this file from sample_hw_platform.h to hw_platform.h and store it in
    the ./config/user_config/ folder of your project. Then customize it per your required Firmware design.

    @section driver_configuration Project configuration Instructions
    1. Set the TICK rate

*//*=========================================================================*/

#ifndef USER_CONFIG_MSS_USER_CONFIG_H_
#define USER_CONFIG_MSS_USER_CONFIG_H_

/*------------------------------------------------------------------------------
 * MPFS_HAL_FIRST_HART and MPFS_HAL_LAST_HART defines used to specify which harts
 * to actually start.
 * Set MPFS_HAL_FIRST_HART to a value other than 0 if you do not want your code
 * to start and execute code on the E51 hart.
 * Set MPFS_HAL_LAST_HART to a value smaller than 4 if you do not wish to use all
 * U54 harts.
 * Harts that are not started will remain in an infinite WFI loop unless used
 * through some other method
 */
#ifndef MPFS_HAL_FIRST_HART
#define MPFS_HAL_FIRST_HART  0
#endif

#ifndef MPFS_HAL_LAST_HART
#define MPFS_HAL_LAST_HART   4
#endif

/*------------------------------------------------------------------------------
 * Markers used to indicate startup status of hart
 */
#define HLS_DATA_IN_WFI                     0x12345678U
#define HLS_DATA_PASSED_WFI                 0x87654321U

/*------------------------------------------------------------------------------
 * Define the size of the HLS used
 * In our HAL, we are using Hart Local storage for debug data storage only
 * as well as flags for wfi instruction management.
 * The TLS will take memory from top of the stack if allocated
 *
 */
#define HLS_DEBUG_AREA_SIZE     64

/* define the required tick rate in Milliseconds */
/* if this program is running on one hart only, only that particulars hart value will be used */
#define HART0_TICK_RATE_MS	5UL
#define HART1_TICK_RATE_MS	5UL
#define HART2_TICK_RATE_MS	5UL
#define HART3_TICK_RATE_MS	5UL
#define HART4_TICK_RATE_MS	5UL

#define H2F_BASE_ADDRESS 0x20126000     /* or 0x28126000 */

/*
 * define how you want the Bus Error Unit configured
 */
#define BEU_ENABLE                  0x0U
#define BEU_PLIC_INT                0x0U
#define BEU_LOCAL_INT               0x0U

#endif /* USER_CONFIG_MSS_USER_CONFIG_H_ */
