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
  @mainpage Sample file detailing how hw_cfg_clocks.h should be constructed for
    the MPFS MSS

    @section intro_sec Introduction
    The  hw_cfg_clocks.h is to be located in the project
    /platform/config/hardware/clocks/
    directory.
    Currently this file must be hand crafted when using the MPFS MSS.

    You can use this file as sample.
    hw_platform.h must be copied to
    the ./config/hw_config/ folder of your project. Then customize it per your
    HW design.

    @section driver_configuration Project configuration Instructions
    1. Change MPFS MSS CLOCK DEFINITIONS to match design used.

*//*==========================================================================*/

#ifndef HW_CFG_CLOCKS_H
#define HW_CFG_CLOCKS_H

/***************************************************************************//**
 * MPFS MSS CLOCK DEFINITIONS
 */
#ifndef SIFIVE_HIFIVE_UNLEASHED
#define MSS_COREPLEX_CPU_CLK            25000000UL
#define MSS_SYSTEM_CLK                  20000000UL   /* PLIC,CLINT,DEBUG,PORTS  */
#define MSS_RTC_TOGGLE_CLK              10000000UL   /* RTC clk input, must run at less than 1/2 rate of MSS_SYSTEM_CLK  */
#define MSS_AXI_SWITCH_CLK              25000000UL
#define MSS_AHB_APB_CLK                 20000000UL
#else
#define MSS_COREPLEX_CPU_CLK            50000000UL
#define MSS_RTC_TOGGLE_CLK              10000000UL   /* RTC clk input, must run at less than 1/2 rate of MSS_SYSTEM_CLK  */
#define MSS_AXI_SWITCH_CLK              25000000UL
#endif


/*
 * MSS SCB REG PLL configuration
 */
#define    MSS_PLL_CTRL_USER_CONFIG				(0x0000003eU | (0x01U << 24U))
#define    MSS_PLL_CAL_USER_CONFIG 				0x00000d06U
#define    MSS_PLL_REF_FB_USER_CONFIG 			0x00000100U
#define    MSS_PLL_DIV_0_1_USER_CONFIG 			0x04000100U
#define    MSS_PLL_DIV_2_3_USER_CONFIG 			0x0A000400U
#define    MSS_PLL_CTRL2_USER_CONFIG 			((0x8U << 9U) | (0x1U << 5U))
#define    MSS_PLL_FRACN_USER_CONFIG 			0x00000001U
#define    MSS_PLL_SSCG_REG_0_USER_CONFIG 		0x00000000U
#define    MSS_PLL_SSCG_REG_1_USER_CONFIG 		0x00000000U
#define    MSS_PLL_SSCG_REG_2_USER_CONFIG 		0x00000019U
#define    MSS_PLL_SSCG_REG_3_USER_CONFIG 		0x00000001U
#define    MSS_PLL_PHADJ_USER_CONFIG 			0x00004003U

/*
 * MSS CFM user config
 */
#define    BCLKMUX_USER_CONFIG    				0x02108608U
#define    PLL_CKMUX_USER_CONFIG    			0x00000154U
#define    MSSCLKMUX_USER_CONFIG    			0x00000001U
#define    SPARE0_USER_CONFIG       			0x00002011U
#define    FMETER_ADDR_USER_CONFIG    			0x00000000U
#define    FMETER_DATAW_USER_CONFIG    			0x00000000U
#define    FMETER_DATAR_USER_CONFIG    			0x00000000U
#define    TEST_CTRL_USER_CONFIG    			0x00000000U


/*******************************************************************************
 * End of user edit section
 */
#endif /* HW_CFG_CLOCKS_H */


