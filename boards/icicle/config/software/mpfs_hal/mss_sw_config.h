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
#define DDR_FULL_32BIT_NC_CHECK_EN

#define PATTERN_INCREMENTAL     (0x01U << 0U)
#define PATTERN_WALKING_ONE     (0x01U << 1U)
#define PATTERN_WALKING_ZERO    (0x01U << 2U)
#define PATTERN_RANDOM          (0x01U << 3U)
#define PATTERN_0xCCCCCCCC      (0x01U << 4U)
#define PATTERN_0x55555555      (0x01U << 5U)
#define PATTERN_ZEROS           (0x01U << 6U)
#define MAX_NO_PATTERNS         7U
/* number of test writes to perform */
#define SW_CFG_NUM_READS_WRITES        0x20000U
/*
 * what test patterns to write/read on start-up
 * */
#define SW_CONFIG_PATTERN (PATTERN_INCREMENTAL|\
                                        PATTERN_WALKING_ONE|\
                                        PATTERN_WALKING_ZERO|\
                                        PATTERN_RANDOM|\
                                        PATTERN_0xCCCCCCCC|\
                                        PATTERN_0x55555555)
/* Training types status offsets */
#define BCLK_SCLK_BIT                   (0x1U<<0U)
#define ADDCMD_BIT                      (0x1U<<1U)
#define WRLVL_BIT                       (0x1U<<2U)
#define RDGATE_BIT                      (0x1U<<3U)
#define DQ_DQS_BIT                      (0x1U<<4U)
/*  The first five bits represent the currently supported training in the TIP */
/*  This value will not change unless more training possibilities are added to
 *  the TIP */
#define TRAINING_MASK                   (BCLK_SCLK_BIT|\
                                        ADDCMD_BIT|\
                                        WRLVL_BIT|\
                                        RDGATE_BIT|\
                                        DQ_DQS_BIT)
/*
 * Debug DDR startup through a UART
 * Comment out in normal operation. May be useful for debug purposes in bring-up
 * of a new board design.
 * See the weak function setup_ddr_debug_port(mss_uart_instance_t * uart)
 * If you need to edit this function, make a copy of of the function without the
 * weak declaration in your application code.
 * */
//#define DEBUG_DDR_INIT
#define DEBUG_DDR_RD_RW_FAIL
//#define DEBUG_DDR_RD_RW_PASS
//#define DEBUG_DDR_CFG_DDR_SGMII_PHY
//#define DEBUG_DDR_DDRCFG

/*
 * You can over write any on the settings coming from Libero here
 *
 * e.g. Define how you want SEG registers configured, if you want to change from
 * the default settings
 */

#define LIBERO_SETTING_SEG0_0     (-(0x0080000000LL >> 24U))
#define LIBERO_SETTING_SEG0_1     (-(0x1000000000LL >> 24U))
#define LIBERO_SETTING_SEG1_2     (-(0x00C0000000LL >> 24U))
#define LIBERO_SETTING_SEG1_3     (-(0x1400000000LL >> 24U))
#define LIBERO_SETTING_SEG1_4     (-(0x00D0000000LL >> 24U))
#define LIBERO_SETTING_SEG1_5     (-(0x1800000000LL >> 24U))

/*  comment out any of these defines if you do not want to sweep values
 *      SUPPORT_ADDR_CMD_OFFSET_SWEEP
 *      SUPPORT_BCLK_SCLK_SWEEP
 *      SUPPORT_DPC_SWEEP
 *  Alternatively, modify the sweep values. This helps when calibrating a new
 *  board design. Enabling DEBUG_DDR_INIT define above will display the
 *  calibration sweep.
 */
//#define SWEEP_ENABLED
#define SUPPORT_ADDR_CMD_OFFSET_SWEEP
#define SUPPORT_BCLK_SCLK_SWEEP
#define SUPPORT_DPC_SWEEP

#define LIBERO_SETTING_MAX_ADDRESS_CMD_OFFSET       4UL
#define LIBERO_SETTING_MIN_ADDRESS_CMD_OFFSET       2UL
#define MAX_NUMBER_ADDR_CMD_OFFSET_SWEEPS           ((LIBERO_SETTING_MAX_ADDRESS_CMD_OFFSET-LIBERO_SETTING_MIN_ADDRESS_CMD_OFFSET) +1U)

#define LIBERO_SETTING_MAX_ADDRESS_BCLK_SCLK_OFFSET 5UL
#define LIBERO_SETTING_MIN_ADDRESS_BCLK_SCLK_OFFSET 5UL
#define MAX_NUMBER__BCLK_SCLK_OFFSET_SWEEPS         ((LIBERO_SETTING_MAX_ADDRESS_BCLK_SCLK_OFFSET-LIBERO_SETTING_MIN_ADDRESS_BCLK_SCLK_OFFSET)+1U)

#define LIBERO_SETTING_MAX_DPC_V_GEN                9UL
#define LIBERO_SETTING_MIN_DPC_V_GEN                9UL
#define MAX_NUMBER_DPC_V_GEN_SWEEPS                 ((LIBERO_SETTING_MAX_DPC_V_GEN-LIBERO_SETTING_MIN_DPC_V_GEN)+1U)

#define LIBERO_SETTING_MAX_DPC_H_GEN                2UL
#define LIBERO_SETTING_MIN_DPC_H_GEN                2UL
#define MAX_NUMBER_DPC_H_GEN_SWEEPS                 ((LIBERO_SETTING_MAX_DPC_H_GEN-LIBERO_SETTING_MIN_DPC_H_GEN)+1U)

#define LIBERO_SETTING_MAX_DPC_VS_GEN               2UL
#define LIBERO_SETTING_MIN_DPC_VS_GEN               2UL
#define MAX_NUMBER_DPC_VS_GEN_SWEEPS                ((LIBERO_SETTING_MAX_DPC_VS_GEN-LIBERO_SETTING_MIN_DPC_VS_GEN)+1U)

/*
 * Define SW_CONFIG_LPDDR_WR_CALIB_FN if we want to use lpddr4 wr calib function
 */
//#define SW_CONFIG_LPDDR_WR_CALIB_FN
/*
 *  Temporally write Icicle/peripheral board differences here
 */ 
#define ICICLE_BOARD
#ifdef ICICLE_BOARD

/*
 * over-write value from Libero todo: remove once verifid in Libero design
 */

#define LIBERO_SETTING_DPC_BITS         0x00049432UL
#define LIBERO_SETTING_DDRPHY_MODE      0x00014B24UL
#define LIBERO_SETTING_DATA_LANES_USED  0x00000004UL
#define LIBERO_SETTING_CFG_DQ_WIDTH     0x00000000UL

#define LIBERO_SETTING_TIP_CFG_PARAMS    0x07CFE02AUL // 0x07CFE02AUL//0x07CFE02AUL
    /* ADDCMD_OFFSET                     [0:3]   RW value= 0x2 5*/
    /* BCKLSCLK_OFFSET                   [3:3]   RW value= 0x4 */
    /* WRCALIB_WRITE_COUNT               [6:7]   RW value= 0x0 */
    /* READ_GATE_MIN_READS               [13:8]  RW value= 0x1F */
    /* ADDRCMD_WAIT_COUNT                [22:8]  RW value= 0x1F */

/*
 * over write value from Libero
 */
#define LIBERO_SETTING_CFG_DFI_T_PHY_WRLAT          0x00000003UL

/*
 * Temporarily over write values from Libero
 */
#define LIBERO_SETTING_RPC_ODT_ADDCMD           2
#define LIBERO_SETTING_RPC_ODT_CLK              2
#define LIBERO_SETTING_RPC_ODT_DQ               6 //6
#define LIBERO_SETTING_RPC_ODT_DQS              6 //2 for peripheral board

#else /* peripheral board */
/*
 * over-write value from Libero todo: remove once verifid in Libero design
 */
#define LIBERO_SETTING_TIP_CFG_PARAMS    0x07C3E035UL //0x07C3E025UL
    /* ADDCMD_OFFSET                     [0:3]   RW value= 0x2 5*/
    /* BCKLSCLK_OFFSET                   [3:3]   RW value= 0x4 */
    /* WRCALIB_WRITE_COUNT               [6:7]   RW value= 0x0 */
    /* READ_GATE_MIN_READS               [13:8]  RW value= 0x1F */
    /* ADDRCMD_WAIT_COUNT                [22:8]  RW value= 0x1F */

/*
 * over write value from Libero
 */
#define LIBERO_SETTING_CFG_DFI_T_PHY_WRLAT          0x00000003UL

/*
 * Temporarily over write values from Libero
 */
#define LIBERO_SETTING_RPC_ODT_ADDCMD           2
#define LIBERO_SETTING_RPC_ODT_CLK              2
#define LIBERO_SETTING_RPC_ODT_DQ               6 
#define LIBERO_SETTING_RPC_ODT_DQS              2 

#endif /* not ICICLE */

/*
 * 0 implies all IP traing's used. This should be the default
 * setting.
 */
#define LIBERO_SETTING_TRAINING_SKIP_SETTING        0x00000000UL
/*
 * 1 implies sw BCLK_SCK traing carried out before IP training. This should be
 * the default
 * setting.
 */
#define USE_SW_BCLK_SCK_TRAINING                    0x00000001UL
#define SW_TRAING_BCLK_SCLK_OFFSET                  0x00000006UL

/*
 * 0x6DU => setting vref_ca to 40%
 * This (0x6DU) is the default setting.
 * */
#define DDR_MODE_REG_VREF_VALUE       0x6DU

/*
 * Will review address settings in Libero, tie in, sanity check with SEG
 * settings
 */
#define LIBERO_SETTING_DDR_32_NON_CACHE 0xC0000000ULL

#endif /* USER_CONFIG_MSS_USER_CONFIG_H_ */

