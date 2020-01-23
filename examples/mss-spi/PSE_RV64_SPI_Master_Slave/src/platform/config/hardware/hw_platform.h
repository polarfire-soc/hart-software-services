/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software example
 *
 */
/*******************************************************************************
 *
 * Platform definitions
 * Version based on requirements of MPFS MSS
 *
 */
 /*=========================================================================*//**
  @mainpage Sample file detailing how hw_platform.h should be constructed for
    the MPFS MSS

    @section intro_sec Introduction
    The  hw_platform.h is to be located in the project platform/config/hardware/ directory.
    Currently this file must be hand crafted when using the MPFS MSS.

    You can use this file as sample.
    hw_platform.h must be copied to
    the platform/config/hardware/ folder of your project. Then customize it per your HW design
    by including required hw_cfg_xx.h files

    @section driver_configuration Project configuration Instructions
    1. Change MPFS MSS CLOCK DEFINITIONS to match design used.

*//*=========================================================================*/

#ifndef HW_PLATFORM_H
#define HW_PLATFORM_H

#include "clocks/hw_cfg_clocks.h"

#endif /* HW_PLATFORM_H */


