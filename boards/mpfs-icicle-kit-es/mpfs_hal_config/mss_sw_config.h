/*******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
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
 * MPFS_HAL_HW_CONFIG
 * Conditional compile switch is used to determine if MPFS HAL will perform the
 * hardware configurations or not.
 * Defined      => This program acts as a First stage bootloader and performs
 *                 hardware configurations.
 * Not defined  => This program assumes that the hardware configurations are
 *                 already performed (Typically by a previous boot stage)
 *
 * List of items initialised when MPFS_HAL_HW_CONFIG is enabled
 * - load virtual rom (see load_virtual_rom(void) in system_startup.c)
 * - l2 cache config
 * - Bus error unit config
 * - MPU config
 * - pmp config
 * - I/O, clock and clock mux's, DDR and SGMII
 * - will start other harts, see text describing MPFS_HAL_FIRST_HART,
 *   MPFS_HAL_LAST_HART above
 */
#ifndef MPFS_HAL_HW_CONFIG
#define MPFS_HAL_HW_CONFIG
#endif

/*
 * If not using item, comment out line below
 */
#define SGMII_SUPPORT
#define DDR_SUPPORT
#define MSSIO_SUPPORT
//#define SIMULATION_TEST_FEEDBACK
//#define E51_ENTER_SLEEP_STATE

/*
 * DDR software options
 */

/*
 * Debug DDR startup through a UART
 * Comment out in normal operation. May be useful for debug purposes in bring-up
 * of a new board design.
 * See the weak function setup_ddr_debug_port(mss_uart_instance_t * uart)
 * If you need to edit this function, make a copy of of the function without the
 * weak declaration in your application code.
 * */
//#define DEBUG_DDR_INIT
//#define DEBUG_DDR_RD_RW_FAIL
//#define DEBUG_DDR_RD_RW_PASS
//#define DEBUG_DDR_CFG_DDR_SGMII_PHY
//#define DEBUG_DDR_DDRCFG

/*
 * Set up 1MiB of L2 cache
 */
#define LIBERO_SETTING_WAY_ENABLE    0x00000007UL

/*
 * Libero v12.6 support
 */
#if !defined(LIBERO_SETTING_WAY_MASK_M0)
/*Way mask register master 0 (hart0) */
#  define LIBERO_SETTING_WAY_MASK_M0    0x00000000UL
#endif

#if !defined(LIBERO_SETTING_WAY_MASK_M1)
/*Way mask register master 1 (hart1) */
#  define LIBERO_SETTING_WAY_MASK_M1    0x00000000UL
#endif

#if !defined(LIBERO_SETTING_WAY_MASK_M2)
/*Way mask register master 2 (hart2) */
#  define LIBERO_SETTING_WAY_MASK_M2    0x00000000UL
#endif

#if !defined(LIBERO_SETTING_WAY_MASK_M3)
/*Way mask register master 3 (hart3) */
#  define LIBERO_SETTING_WAY_MASK_M3    0x00000000UL
#endif

#if !defined(LIBERO_SETTING_WAY_MASK_M4)
/*Way mask register master 4 (hart4) */
#  define LIBERO_SETTING_WAY_MASK_M4    0x00000000UL
#endif

/*
 *  For the Icicle kit we need to override the default settings coming from
 *  the MSS Configurator
 *  The valid AXI range should be set to a value matching the DDR being used.
 *  The default setting should match the size of the DDR being used in the design.
 *  The following are the registers that need adjusting based on the DDR being
 *  used.
 *  using 2MB ( Icicle kit)
*/
/* DCT diff's from Libero */
#define LIBERO_SETTING_TIP_CFG_PARAMS               0x07CFE02FUL
    /* ADDCMD_OFFSET                     [0:3]   RW value= 0x2 changed to 3*/
    /* BCKLSCLK_OFFSET                   [3:3]   RW value= 0x5 */
    /* WRCALIB_WRITE_COUNT               [6:7]   RW value= 0x0 */
    /* READ_GATE_MIN_READS               [13:8]  RW value= 0x7F */
    /* ADDRCMD_WAIT_COUNT                [22:8]  RW value= 0x1F */

/* from HW_DDR_IO_BANK_H_ */

#define LIBERO_SETTING_DPC_BITS                     0x00050422UL
    /* DPC_VS                            [0:4]   RW value= 0x2 */
    /* DPC_VRGEN_H                       [4:6]   RW value= 0x2 */
    /* DPC_VRGEN_EN_H                    [10:1]  RW value= 0x1 */
    /* DPC_MOVE_EN_H                     [11:1]  RW value= 0x0 */
    /* DPC_VRGEN_V                       [12:6]  RW value= 0xC */
    /* DPC_VRGEN_EN_V                    [18:1]  RW value= 0x1 */
    /* DPC_MOVE_EN_V                     [19:1]  RW value= 0x0 */
    /* RESERVE01                         [20:12] RSVD */

#define LIBERO_SETTING_RPC_ODT_DQ                   0x00000006UL
#define LIBERO_SETTING_RPC_ODT_DQS                  0x00000006UL   //4 2

#endif /* USER_CONFIG_MSS_USER_CONFIG_H_ */

