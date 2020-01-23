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
 /*========================================================================*//**
  @mainpage Sample file detailing how hw_cfg_mssio.h should be constructed for
    the MPFS MSS

    @section intro_sec Introduction
    The  hw_cfg_sgmii.h is to be located in the project
    /platform/config/hardware/sgmii/
    directory.
    Currently this file must be hand crafted when using the MPFS MSS.

    You can use this file as sample.
    hw_platform.h must be copied to
    the ./config/hw_config/ folder of your project. Then customize it per your
    HW design.

    @section driver_configuration Project configuration Instructions
    1. Change MPFS MSS SGMII DEFINITIONS to match design used.

*//*==========================================================================*/

#ifndef HW_CFG_SGMII_H
#define HW_CFG_SGMII_H

/*
 * SGMII_MODE
 */
#define SGMII_MODE_DEFAULT              ((0U<<31)|(0U<<30)|(0U<<29)|(0U<<28)|(0U<<24)|(0U<<23)|(1U<<22)|(0U<<16)|(1U<<15)|(1U<<14)|\
                                        (9U<<10)|(2U<<8)|(0U<<7)|(0U<<6)|(0U<<5)|(0U<<4)|(1U<<3)|(1U<<2)|(0U<<1)|(0U<<0))

#define SGMII_MODE_REFCLK_ON            ((1U<<3)|(1U<<2))
#define SGMII_MODE_CHANNEL0_ON          ((1U<<5)|(1U<<4)|(1U<<1)|(1U<<0))
#define SGMII_MODE_CHANNEL1_ON          ((1U<<7)|(1U<<6)|(1U<<1)|(1U<<0))
#define SGMII_MODE_RECALIB_ON           ((1U<<1)|(1U<<0))

#define SGMII_MODE_REFCLK_OFF           (0)
#define SGMII_MODE_CHANNEL0_OFF         (0)
#define SGMII_MODE_CHANNEL1_OFF         (0)
#define SGMII_MODE_RECALIB_OFF          (0)
/*
 * CH0_CNTL
 */
#define CH0_CNTL_DEFAULT                ((7U<<21)|(7U<<18))
#define CH0_CNTL_REFCLK_ON              (0)
#define CH0_CNTL_CHANNEL0_ON            (0)
#define CH0_CNTL_CHANNEL1_ON            (0)
#define CH0_CNTL_RECALIB_ON             (0)

#define CH0_CNTL_REFCLK_OFF             (0)
#define CH0_CNTL_CHANNEL0_OFF           (0)
#define CH0_CNTL_CHANNEL1_OFF           (0)
#define CH0_CNTL_RECALIB_OFF            (0)
 /*
  * CH1_CNTL
  */
#define CH1_CNTL_DEFAULT                ((7U<<21)|(7U<<18))

#define CH1_CNTL_REFCLK_ON              (0)
#define CH1_CNTL_CHANNEL0_ON            (0)
#define CH1_CNTL_CHANNEL1_ON            (0)
#define CH1_CNTL_RECALIB_ON             (0)

#define CH1_CNTL_REFCLK_OFF             (0)
#define CH1_CNTL_CHANNEL0_OFF           (0)
#define CH1_CNTL_CHANNEL1_OFF           (0)
#define CH1_CNTL_RECALIB_OFF            (0)
 /*
  * RECAL_CNTL
  */
#define RECAL_CNTL_DEFAULT                ((2U<<12)|(1U<<7)|(1U<<6)|(8U<<0))

#define RECAL_CNTL_REFCLK_ON              (0)
#define RECAL_CNTL_CHANNEL0_ON            (0)
#define RECAL_CNTL_CHANNEL1_ON            (0)
#define RECAL_CNTL_RECALIB_ON             (0)

#define RECAL_CNTL_REFCLK_OFF             (0)
#define RECAL_CNTL_CHANNEL0_OFF           (0)
#define RECAL_CNTL_CHANNEL1_OFF           (0)
#define RECAL_CNTL_RECALIB_OFF            (0)
 /*
  * CLK_CNTL
  */
#define CLK_CNTL_DEFAULT                ((0xF000U<<16))

#define CLK_CNTL_REFCLK_ON              (0)
#define CLK_CNTL_CHANNEL0_ON            (0)
#define CLK_CNTL_CHANNEL1_ON            (0)
#define CLK_CNTL_RECALIB_ON             (0)

#define CLK_CNTL_REFCLK_OFF             (0)
#define CLK_CNTL_CHANNEL0_OFF           (0)
#define CLK_CNTL_CHANNEL1_OFF           (0)
#define CLK_CNTL_RECALIB_OFF            (0)


 /*
  * DYN_CNTL
  */
#define DYN_CNTL_DEFAULT                ((1U<<10))   /* soft reset */

#define DYN_CNTL_REFCLK_ON              (0)
#define DYN_CNTL_CHANNEL0_ON            (0)
#define DYN_CNTL_CHANNEL1_ON            (0)
#define DYN_CNTL_RECALIB_ON             (0)

#define DYN_CNTL_REFCLK_OFF             (0)
#define DYN_CNTL_CHANNEL0_OFF           (0)
#define DYN_CNTL_CHANNEL1_OFF           (0)
#define DYN_CNTL_RECALIB_OFF            (0)

 /*
  * PVT_STAT
  */
#define PVT_STAT_DEFAULT                (0)

#define PVT_STAT_REFCLK_ON              (0)
#define PVT_STAT_CHANNEL0_ON            (0)
#define PVT_STAT_CHANNEL1_ON            (0)
#define PVT_STAT_RECALIB_ON             (0)

#define PVT_STAT_REFCLK_OFF             (0)
#define PVT_STAT_CHANNEL0_OFF           (0)
#define PVT_STAT_CHANNEL1_OFF           (0)
#define PVT_STAT_RECALIB_OFF            (0)


#define SGMII_MODE_ALL_ON (SGMII_MODE_DEFAULT | SGMII_MODE_REFCLK_ON | SGMII_MODE_CHANNEL0_ON | SGMII_MODE_CHANNEL1_ON)
#define CH0_CNTL_ALL_ON (CH0_CNTL_DEFAULT | CH0_CNTL_REFCLK_ON | CH0_CNTL_CHANNEL0_ON | CH0_CNTL_CHANNEL1_ON)
#define CH1_CNTL_ALL_ON (CH1_CNTL_DEFAULT | CH1_CNTL_REFCLK_ON | CH1_CNTL_CHANNEL0_ON | CH1_CNTL_CHANNEL1_ON)
#define RECAL_CNTL_ALL_ON (RECAL_CNTL_DEFAULT | RECAL_CNTL_REFCLK_ON | RECAL_CNTL_CHANNEL0_ON | RECAL_CNTL_CHANNEL1_ON)
#define CLK_CNTL_ALL_ON (CLK_CNTL_DEFAULT | CLK_CNTL_REFCLK_ON | CLK_CNTL_CHANNEL0_ON | CLK_CNTL_CHANNEL1_ON)
#define DYN_CNTL_ALL_ON (DYN_CNTL_DEFAULT | DYN_CNTL_REFCLK_ON | DYN_CNTL_CHANNEL0_ON | DYN_CNTL_CHANNEL1_ON)


#endif /* HW_CFG_SGMII_H */
