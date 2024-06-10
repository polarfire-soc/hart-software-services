/*******************************************************************************
 * Copyright 2019 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * @file mss_ddr_defs.h
 * @author Microchip FPGA Embedded Systems Solutions
 * @brief mss_ddr_debug related defines
 *
 */

#ifndef SRC_PLATFORM_MPFS_HAL_NWC_MSS_DDR_DEFS_H_
#define SRC_PLATFORM_MPFS_HAL_NWC_MSS_DDR_DEFS_H_

typedef struct PATTERN_TEST_PARAMS_
{
    uint64_t base;
    uint64_t size;
    uint32_t pattern_type;
    uint8_t pattern_offset;
    uint8_t offset_cnt;
    uint8_t num_offsets_to_try;
    uint8_t padding;
} PATTERN_TEST_PARAMS;


#define PATTERN_INCREMENTAL     (0x01U << 0U)
#define PATTERN_WALKING_ONE     (0x01U << 1U)
#define PATTERN_WALKING_ZERO    (0x01U << 2U)
#define PATTERN_RANDOM          (0x01U << 3U)
#define PATTERN_0xCCCCCCCC      (0x01U << 4U)
#define PATTERN_0x55555555      (0x01U << 5U)
#define PATTERN_ZEROS           (0x01U << 6U)
#define MAX_NO_PATTERNS         7U

/* Training types status offsets */
#define BCLK_SCLK_BIT            (0x1U<<0U)
#define ADDCMD_BIT               (0x1U<<1U)
#define WRLVL_BIT                (0x1U<<2U)
#define RDGATE_BIT               (0x1U<<3U)
#define DQ_DQS_BIT               (0x1U<<4U)

/*  The first five bits represent the currently supported training in the TIP */
/*  This value will not change unless more training possibilities are added to
 *  the TIP */
#define TRAINING_MASK             (BCLK_SCLK_BIT|\
                                       ADDCMD_BIT|\
                                       WRLVL_BIT|\
                                       RDGATE_BIT|\
                                       DQ_DQS_BIT)

/* supported clk speeds, these values come from the MSS Configurator
   in the following define - LIBERO_SETTING_DDR_CLK                           */
#define DDR_1067_MHZ                            1067000000UL
#define DDR_1333_MHZ                            1332000000UL
#define DDR_1600_MHZ                            1600000000UL
#define DDR_FREQ_MARGIN                         10UL

/*
 * Error flags for ADD_CMD
 */
#define DDR_ADD_CMD_A5_OFFSET_PASS              0x00
#define DDR_ADD_CMD_A5_OFFSET_FAIL              0x01
#define DDR_ADD_CMD_A5_OFFSET_FAIL_LOW_FREQ     0x04

#define BASE_ADDRESS_CACHED_32_DDR              0x80000000UL
#define BASE_ADDRESS_NON_CACHED_32_DDR          0xC0000000UL
#define BASE_ADDRESS_CACHED_64_DDR              0x1000000000ULL
#define BASE_ADDRESS_NON_CACHED_64_DDR          0x1400000000ULL

#endif /* SRC_PLATFORM_MPFS_HAL_NWC_MSS_DDR_DEFS_H_ */
