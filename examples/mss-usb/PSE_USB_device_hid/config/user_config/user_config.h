/*******************************************************************************
 * (c) Copyright 2018 Microsemi Corporation.  All rights reserved.
 *
 * Platform definitions
 * Version based on requirements of PSE MSS
 *
 */
 /*=========================================================================*//**
  @mainpage Sample file detailing how user_config.h should be constructed for
    the PSE MSS

    @section intro_sec Introduction
    The  user_config.h is to be located in the project ./config/user_config/ directory.
    This file must be hand crafted when using the PSE MSS.

    You can use this file as sample.
    Rename this file from sample_hw_platform.h to hw_platform.h and store it in
    the ./config/user_config/ folder of your project. Then customize it per your required Firmware design.

    @section driver_configuration Project configuration Instructions
    1. Set the TICK rate

*//*=========================================================================*/

#ifndef USER_CONFIG_USER_CONFIG_H_
#define USER_CONFIG_USER_CONFIG_H_

/* define the required tick rate in Milliseconds */
/* if this program is running on one hart only, only that particulars hart value will be used */
#define HART0_TICK_RATE_MS	10000UL
#define HART1_TICK_RATE_MS	5UL
#define HART2_TICK_RATE_MS	5UL
#define HART3_TICK_RATE_MS	5UL
#define HART4_TICK_RATE_MS	5UL

#define H2F_BASE_ADDRESS 0x20126000     /* or 0x28126000 */

#endif /* USER_CONFIG_USER_CONFIG_H_ */
