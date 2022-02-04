#ifndef USER_CONFIG_MSS_USER_CONFIG_H_
#define USER_CONFIG_MSS_USER_CONFIG_H_

/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
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
#define HLS_DATA_IN_WFI                     0x12345678u
#define HLS_DATA_PASSED_WFI                 0x87654321u

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
#define BEU_ENABLE                  0x0ull
#define BEU_PLIC_INT                0x0ull
#define BEU_LOCAL_INT               0x0ull

/*
* You can over write any on the settings coming from Libero here
*
* e.g. Define how you want SEG registers configured, if you want to change from
* the default settings
*/

/*
*  For the Icicle kit we need to override the default settings coming from
*  the MSS Configurator
*  The valid AXI range should be set to a value matching the DDR being used.
*  The default setting should match the size of the DDR being used in the design.
*  The following are the registers that need adjusting based on the DDR being
*  used.
*  using 2MB ( Icicle kit)
*/
/* Uncomment if settings generated with pre 2021.2 */
#define USE_IF_2021_1_OR_EARLIER
#ifdef USE_IF_2021_1_OR_EARLIER
#define LIBERO_SETTING_TIP_CFG_PARAMS               0x07CFE02FUL
    /* ADDCMD_OFFSET                     [0:3]   RW value= 0x2 changed to 3*/
    /* BCKLSCLK_OFFSET                   [3:3]   RW value= 0x5 */
    /* WRCALIB_WRITE_COUNT               [6:7]   RW value= 0x0 */
    /* READ_GATE_MIN_READS               [13:8]  RW value= 0x7F */
    /* ADDRCMD_WAIT_COUNT                [22:8]  RW value= 0x1F */

/* from HW_DDR_IO_BANK_H_ */

#define LIBERO_SETTING_DPC_BITS                     0x00050422UL //DPC_VRGEN_H = 2
    /* DPC_VS                            [0:4]   RW value= 0x2 */
    /* DPC_VRGEN_H                       [4:6]   RW value= 0x2 */
    /* DPC_VRGEN_EN_H                    [10:1]  RW value= 0x1 */
    /* DPC_MOVE_EN_H                     [11:1]  RW value= 0x0 */
    /* DPC_VRGEN_V                       [12:6]  RW value= 0xC */
    /* DPC_VRGEN_EN_V                    [18:1]  RW value= 0x1 */
    /* DPC_MOVE_EN_V                     [19:1]  RW value= 0x0 */
    /* RESERVE01                         [20:12] RSVD */

#define LIBERO_SETTING_RPC_ODT_DQS                  0x00000006UL
/* Enabling VREFDQ training
* Configuring the VREFDQ training range to be one
* Configuring the VREFDQ value as 31.2%
*/
#define LIBERO_SETTING_CFG_VREFDQ_TRN_ENABLE    0x00000001UL
#define LIBERO_SETTING_CFG_VREFDQ_TRN_RANGE    0x00000001UL
#define LIBERO_SETTING_CFG_VREFDQ_TRN_VALUE    0x00000017UL

#endif /* USE_IF_2021_1_OR_EARLIER */

/*
* We need to redefine the following AXI address range if set incorrectly
* This is the case for Libero 12.5 and Libero 12.6
* If using MSS Configurator 2021.1 or later the following four lines can be
* removed.
* LIBERO_SETTING_CFG_AXI_END_ADDRESS_AXI1_0
* is the definition for cached axi addrress
* LIBERO_SETTING_CFG_AXI_END_ADDRESS_AXI2_0
* is the address for non-cached Libero address
* 0x7FFFFFFFUL => 2 GB address range
*
*/
#define LIBERO_SETTING_CFG_AXI_END_ADDRESS_AXI1_0    0x7FFFFFFFUL
#define LIBERO_SETTING_CFG_AXI_END_ADDRESS_AXI1_1    0x00000000UL
#define LIBERO_SETTING_CFG_AXI_END_ADDRESS_AXI2_0    0x7FFFFFFFUL
#define LIBERO_SETTING_CFG_AXI_END_ADDRESS_AXI2_1    0x00000000UL

/*
  * Changes are fixes to data mismatches seen when applying the new
  * DDR workload identified by the Linux boot failures on the icicle kit.
  * CFG_MIN_READ_IDLE helped it pass in DDR3/DDR4, and CFG_READ_TO_WRITE fixed
  * a different issue where 0's were being read back with the same workload on
  * LPDDR3.
  */
#define LIBERO_SETTING_CFG_MIN_READ_IDLE             0x00000007UL

/* For LPDDR3 only: */
//#define LIBERO_SETTING_CFG_READ_TO_WRITE             0x00000006UL
//#define LIBERO_SETTING_CFG_READ_TO_WRITE_ODT         0x00000006UL

/*
* The following three setting disable Data Mask and enable Read Write Modify
* This is required if accessing LPDDR4 with non-cached writes and using
* MSS Configurator 2021.1 or earlier.
*/
#define LIBERO_SETTING_CFG_DM_EN 0x00000000UL
#define LIBERO_SETTING_CFG_RMW_EN 0x00000001UL
#define LIBERO_SETTING_DDRPHY_MODE 0x00014A24UL

/*
 * IMAGE_LOADED_BY_BOOTLOADER
 * We set IMAGE_LOADED_BY_BOOTLOADER = 0 if we are a boot-loader
 * Set IMAGE_LOADED_BY_BOOTLOADER = 1 if loaded by a boot loader
 *
 * MPFS_HAL_HW_CONFIG is defined if we are a boot-loader. This is a
 * conditional compile switch is used to determine if MPFS HAL will perform the
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
 *
 */

/*
 * If not using item, comment out line below
 */
#define SGMII_SUPPORT
#define DDR_SUPPORT
#define MSSIO_SUPPORT

#define IMAGE_LOADED_BY_BOOTLOADER 0
#if (IMAGE_LOADED_BY_BOOTLOADER == 0)
#define MPFS_HAL_HW_CONFIG
#endif

#endif // USER_CONFIG_MSS_USER_CONFIG_H_
