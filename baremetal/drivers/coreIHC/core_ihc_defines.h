/*******************************************************************************
 * Copyright 2021-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

 /*========================================================================*//**
  @mainpage Fixed defines for the Core IHC driver

    @section The defines in this file are used by the user configuration
    header and the driver files. The header files need to be included in a
    project in the following order to allow over-ride of user settings.
    core_ihc_defines.h
    core_ihc_config.h    we can over-ride default setting in this file
                         This config file is stored in the boards directory and
                         is called from the mss_sw_config.h file.
    core_ihc_regs.h
    core_ihc.h
    @section

*//*==========================================================================*/

#ifndef CORE_IHC_FIXED_DEFINES_H_
#define CORE_IHC_FIXED_DEFINES_H_

/*------------------------------------------------------------------------------
 * hart mask defines
 */
#define HART0_ID                0U
#define HART1_ID                1U
#define HART2_ID                2U
#define HART3_ID                3U
#define HART4_ID                4U

#define HART0_MASK              1U
#define HART1_MASK              2U
#define HART2_MASK              4U
#define HART3_MASK              8U
#define HART4_MASK              0x10U

/*------------------------------------------------------------------------------
 * Remote harts interrupts enable bits in the concentrator
 */
#define HSS_HART_DEFAULT_INT_EN     (0U<<0U)

#define HSS_HART_MP_INT_EN          (1U<<0U)
#define HSS_HART_ACK_INT_EN         (1U<<1U)

#define HART1_MP_INT_EN             (1U<<2U)
#define HART1_ACK_INT_EN            (1U<<3U)

#define HART2_MP_INT_EN             (1U<<4U)
#define HART2_ACK_INT_EN            (1U<<5U)

#define HART3_MP_INT_EN             (1U<<6U)
#define HART3_ACK_INT_EN            (1U<<7U)

#define HART4_MP_INT_EN             (1U<<8U)
#define HART4_ACK_INT_EN            (1U<<9U)

#endif /* CORE_IHC_FIXED_DEFINES_H_ */

