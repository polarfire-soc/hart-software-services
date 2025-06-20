/*******************************************************************************
 * Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

 /*========================================================================*//**
  @mainpage Configuration for the MiV-IHC driver

    @section intro_sec Introduction
    Used to configure the driver with base addresses from your Libero Projext.
    These addresses will not change unless you change the Libero design
    IHC subsytem design.
    This file is used for reference only.
    When usiing in a project copy to
    src/boards/your-board/platform-config/drivers_config/fpga-ip/miv_ihc
    and rename dropping the _reference.
    @section

*//*==========================================================================*/

#ifndef MIV_IHC_CONFIG_H_
#define MIV_IHC_CONFIG_H_

/*------------------------------------------------------------------------------
 * choose the interrupt mapping used in our system
 * Please see miv_ihc_regs.h for the defaults
 */
#define IHC_APP_X_H0_IRQHandler fabric_f2h_63_plic_IRQHandler
#define IHC_APP_X_H1_IRQHandler fabric_f2h_62_plic_IRQHandler
#define IHC_APP_X_H2_IRQHandler fabric_f2h_61_plic_IRQHandler
#define IHC_APP_X_H3_IRQHandler fabric_f2h_60_plic_IRQHandler
#define IHC_APP_X_H4_IRQHandler fabric_f2h_59_plic_IRQHandler

#endif /* MIV_IHC_CONFIG_H_ */