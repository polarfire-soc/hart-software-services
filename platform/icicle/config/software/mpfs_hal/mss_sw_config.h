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
 /*========================================================================*//**
  @mainpage Sample file detailing how mss_sw_config.h should be constructed for
    the MPFS MSS

    @section intro_sec Introduction
    The mss_sw_config.h is to be located in the project
    ./src/platform/config/software/mpfs_hal directory.
    This file must be hand crafted when using the MPFS MSS.


    @section

*//*==========================================================================*/


#ifndef USER_CONFIG_MSS_USER_CONFIG_H_
#define USER_CONFIG_MSS_USER_CONFIG_H_

/* #define RENODE_DEBUG */

/*------------------------------------------------------------------------------
 * MPFS_HAL_FIRST_HART and MPFS_HAL_LAST_HART defines used to specify which
 * harts to actually start.
 * Set MPFS_HAL_FIRST_HART to a value other than 0 if you do not want your code
 * to start and execute code on the E51 hart.
 * Set MPFS_HAL_LAST_HART to a value smaller than 4 if you do not wish to use
 * all U54 harts.
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
/* if this program is running on one hart only, only that particular hart value
 * will be used */
#define HART0_TICK_RATE_MS  5UL
#define HART1_TICK_RATE_MS  5UL
#define HART2_TICK_RATE_MS  5UL
#define HART3_TICK_RATE_MS  5UL
#define HART4_TICK_RATE_MS  5UL

#define H2F_BASE_ADDRESS 0x20126000     /* or 0x28126000 */

/*
 * define how you want the Bus Error Unit configured
 */
#define BEU_ENABLE                  0x0ULL
#define BEU_PLIC_INT                0x0ULL
#define BEU_LOCAL_INT               0x0ULL

/*
 * Clear memory on startup
 * 0 => do not clear DTIM and L2
 * 1 => Clears memory
 */
#ifndef MPFS_HAL_CLEAR_MEMORY
#define MPFS_HAL_CLEAR_MEMORY  0
#endif

/*
 * If not using item, comment out line below
 */
//#define SGMII_SUPPORT
#define DDR_SUPPORT
#define MSSIO_SUPPORT
#define SIMULATION_TEST_FEEDBACK

/*
 * You can over write any on the settings coming from Libero here
 *
 * e.g. Define how you want SEG registers configured, if you want to change from
 * the default settings
 */

#define LIBERO_SETTING_SEG0_0     (-(0x0080000000LL >> 24u))
#define LIBERO_SETTING_SEG0_1     (-(0x1000000000LL >> 24u))
#define LIBERO_SETTING_SEG1_2     (-(0x00C0000000LL >> 24u))
#define LIBERO_SETTING_SEG1_3     (-(0x1400000000LL >> 24u))
#define LIBERO_SETTING_SEG1_4     (-(0x00D0000000LL >> 24u))
#define LIBERO_SETTING_SEG1_5     (-(0x1800000000LL >> 24u))

/*
 * Turn off DDR, bits 0:3 == 0x7
 */
//#define LIBERO_SETTING_DDRPHY_MODE    0x04000127UL

/*
 * .cpz 1.0.112  PFSOC_MSS_INT
 *  Needs following overwrites for lpddr4 (x16) on mss peripheral verification
 *  board
 */
#define TEMP_LPDDR4_16x_OVERRIDES
#ifdef TEMP_LPDDR4_16x_OVERRIDES
/* in hw_ddr_io_bank.h */
#define LIBERO_SETTING_DPC_BITS    0x0004C4A1UL
    /* DPC_VS                            [0:4]   RW value= 0x1 */
    /* DPC_VRGEN_H                       [4:6]   RW value= 0x8 */
    /* DPC_VRGEN_EN_H                    [10:1]  RW value= 0x1 */
    /* DPC_MOVE_EN_H                     [11:1]  RW value= 0x0 */
    /* DPC_VRGEN_V                       [12:6]  RW value= 0x10 */
    /* DPC_VRGEN_EN_V                    [18:1]  RW value= 0x1 */
    /* DPC_MOVE_EN_V                     [19:1]  RW value= 0x0 */
    /* RESERVE01                         [20:12] RSVD */
#define LIBERO_SETTING_RPC_ODT_DQ       0x00000002UL
#define LIBERO_SETTING_RPC_ODT_DQS      0x00000002UL
#define LIBERO_SETTING_RPC_ODT_ADDCMD   0x00000002UL
#define LIBERO_SETTING_RPC_ODT_CLK      0x00000002UL
/* hw_ddr_mode.h */
#define LIBERO_SETTING_DDRPHY_MODE    0x00010B04UL
#define LIBERO_SETTING_DATA_LANES_USED    0x00000002UL
/* hw_ddr_options.h */
#define LIBERO_SETTING_USER_INPUT_PHY_RANKS_TO_TRAIN    0x00000001UL
#define LIBERO_SETTING_TIP_CFG_PARAMS    0x07C3E022UL
/* hw_ddrc.h */
#define LIBERO_SETTING_CFG_DQ_ODT    0x00000002UL
#define LIBERO_SETTING_CFG_CA_ODT    0x00000002UL
#define LIBERO_SETTING_CFG_DFI_T_RDDATA_EN    0x00000015UL
#define LIBERO_SETTING_PHY_EYE_TRAIN_DELAY    0x0000003FUL
#endif

/*
 * Will review address settings in Libero, tie in, sanity check with SEG
 * settings
 */
#define LIBERO_SETTING_DDR_32_NON_CACHE 0xC0000000ULL

#endif /* USER_CONFIG_MSS_USER_CONFIG_H_ */

