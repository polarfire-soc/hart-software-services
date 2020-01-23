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
  @mainpage Sample file detailing how hw_cfg_mssio.h should be constructed for
    the MPFS MSS

    @section intro_sec Introduction
    The  hw_cfg_clocks.h is to be located in the project
    /platform/config/hardware/clocks/
    directory.
    Currently this file must be hand crafted when using the MPFS MSS.

    You can use this file as sample.
    hw_platform.h must be copied to
    the ./config/hw_config/ folder of your project. Then customize it per your HW design.

    @section driver_configuration Project configuration Instructions
    1. Change MPFS MSS CLOCK DEFINITIONS to match design used.

*//*=========================================================================*/

#ifndef HW_CFG_MSSIO_H_
#define HW_CFG_MSSIO_H_

/*
 * There are 38 general purpose IO pads, referred to as MSSIO, to support peripheral devices.
 * System registers will select which signals are connected to the IO pads. These are in addition to the SGMII IO
 * for the Ethernet MACs, DDR I/O and two Ios to allow interfacing to an external 32kHz crystal. All of these MSSIOs
 * are bonded out to pins in all packages. The MSSIOs may be configured as the IOs of any of the following MSS
 * peripherals:
 * ...
 */


#endif /* HW_CFG_MSSIO_H_ */


