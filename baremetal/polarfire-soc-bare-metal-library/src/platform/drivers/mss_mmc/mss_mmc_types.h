/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 *  PolarFire SoC MSS eMMC SD driver data structures.
 *
 * This eMMC Interface header file provides a subset of definitions from the eMMC
 * protocol JESD84-B51
 *
 * SVN $Revision: 12579 $
 * SVN $Date: 2019-12-04 16:41:30 +0530 (Wed, 04 Dec 2019) $
 */

#ifndef __MSS_MMC_TYPE_H
#define __MSS_MMC_TYPE_H

#ifdef __cplusplus
extern "C"
#endif

#include "hal/cpu_types.h"

/***************************************************************************//**
 * Macro Definitions
 */

/* MMC/SD/SDIO commands */

#define MMC_CMD_15_GOTO_INACTIVE_STATE      15u    /* No Rsp        */
#define MMC_CMD_4_SET_DSR                   4u     /* No Rsp        */
#define MMC_CMD_0_GO_IDLE_STATE             0u     /* No Rsp        */
#define MMC_CMD_6_SWITCH                    6u     /* R1b Rsp       */
#define MMC_CMD_7_SELECT_DESELECT_CARD      7u     /* R1/R1b Rsp    */

#define MMC_CMD_3_SET_RELATIVE_ADDR         3u     /* R1 Rsp        */
#define MMC_CMD_17_READ_SINGLE_BLOCK        17u    /* R1 Rsp        */
#define MMC_CMD_18_READ_MULTIPLE_BLOCK      18u    /* R1 Rsp        */
#define MMC_CMD_24_WRITE_SINGLE_BLOCK       24u    /* R1 Rsp        */
#define MMC_CMD_23_SET_BLOCK_COUNT          23u   /* R1 Rsp        */
#define MMC_CMD_25_WRITE_MULTI_BLOCK        25u    /* R1 Rsp        */
#define MMC_CMD_13_SEND_STATUS              13u    /* R1 Rsp        */
#define MMC_CMD_12_STOP_TRANSMISSION        12u    /* R1/R1b Rsp    */
#define MMC_CMD_8_SEND_EXT_CSD              8u     /* R1 Rsp        */
#define MMC_CMD_21_SEND_TUNE_BLK            21u     /* R1 Rsp        */

#define MMC_CMD_14_BUSTEST_R                14u     /* R1 Rsp        */
#define MMC_CMD_19_BUSTEST_W                19u     /* R1 Rsp        */

#define MMC_CMD_2_ALL_SEND_CID              2u     /* R2 Rsp        */
#define MMC_CMD_9_SEND_CSD                  9u     /* R2 Rsp        */
#define MMC_CMD_10_SEND_CID                 10u    /* R2 Rsp        */
#define MMC_CMD_1_SEND_OP_COND              1u     /* R3  Rsp       */
#define MMC_CMD_39_FAST_IO                  39u    /* R4  Rsp       */
#define MMC_CMD_40_GO_IRQ_STATE             40u    /* R5  Rsp       */

#define SD_CMD_8_SEND_IF_COND               8u     /* R7 Rsp        */
#define SD_ACMD_41_SEND_OP_COND             41u    /* R3 Rsp        */
#define SD_ACMD_42_SET_CLR_CARD_DETECT      42u    /* R1 Rsp        */

#define SD_CMD_11_VOLAGE_SWITCH             11u    /* R1 Rsp        */
#define SD_CMD_19_SEND_TUNING_BLK           19u    /* R1 Rsp        */
#define SD_CMD_55                           55u

#define SD_CMD_5                            5u    /* R4 Rsp        */
#define SD_ACMD_6                           6u    /* R1 Rsp        */
#define SD_ACMD_51                          51u    /* R1 Rsp        */
#define SD_CMD_6                            6u    /* R1 Rsp        */
#define SD_CMD_16                           16u    /* R1 Rsp        */

#define SDIO_CMD_52_IO_RW_DIRECT            52u    /*R5  Rsp */
#define SDIO_CMD_53_IO_RW_EXTENDED          53u    /*R5  Rsp */

/* eMMC/SD Response Type  */
typedef enum
{
    MSS_MMC_RESPONSE_NO_RESP = 0u,
    MSS_MMC_RESPONSE_R1 = 1u,
    MSS_MMC_RESPONSE_R1B = 2u,
    MSS_MMC_RESPONSE_R2 = 3u,
    MSS_MMC_RESPONSE_R3 = 4u,
    MSS_MMC_RESPONSE_R4 = 5u,
    MSS_MMC_RESPONSE_R5 = 6u,
    MSS_MMC_RESPONSE_R5B = 7u,
    MSS_MMC_RESPONSE_R6 = 8u,
    MSS_MMC_RESPONSE_R7 = 9u,
    MSS_MMC_RESPONSE_R1A = 10u
} MSS_MMC_response_type;

typedef enum
{
    /* access mode - SDR12 default (CLK: max 25MHz, DT: max 12MB/s) */
    MSS_MMC_ACCESS_MODE_SDR12 = 0u,
    /* access mode - SDR15 default (CLK: max 50MHz, DT: max 25MB/s) */
    MSS_MMC_ACCESS_MODE_SDR25 = 1u,
    /* access mode - SDR50 default (CLK: max 100MHz, DT: max 50MB/s) */
    MSS_MMC_ACCESS_MODE_SDR50 = 2u,
    /* access mode - SDR104 default (CLK: max 208MHz, DT: max 104MB/s) */
    MSS_MMC_ACCESS_MODE_SDR104 = 3u,
    /* access mode - DDR50 default (CLK: max 50MHz, DT: max 50MB/s) */
    MSS_MMC_ACCESS_MODE_DDR50 = 4u,
    /* access mode - ultra high speed II mode */
    MSS_MMC_ACCESS_MODE_UHSII = 5u,
    /* MMC access mode - legacy mode (CLK: max 26MHz, DT: max 26MB/s) */
    MSS_MMC_ACCESS_MODE_MMC_LEGACY = 6u,
    /* MMC access mode - high speed SDR mode (CLK: max 26MHz, DT: max 26MB/s) */
    MSS_MMC_ACCESS_MODE_HS_SDR = 7u,
    /* MMC access mode - high speed DDR mode (CLK: max 52MHz, DT: max 104MB/s) */
    MSS_MMC_ACCESS_MODE_HS_DDR = 8u,
    /* MMC access mode - HS200 mode (CLK: max 200MHz, DT: max 200MB/s) */
    MSS_MMC_ACCESS_MODE_HS_200 = 9u,
    /* MMC access mode - HS400 mode (CLK: max 200MHz, DT: max 400MB/s) */
    MSS_MMC_ACCESS_MODE_HS_400 = 10u,
    /* MMC access mode - HS400 using Enhanced Strobe (CLK: max 200MHz, DT: max 400MB/s) */
    MSS_MMC_ACCESS_MODE_HS_400_ES = 11u,
} MSS_MMC_speed_mode;

/* PHY configuration delay type */
typedef enum
{
    /* delay in the input path for High Speed work mode */
    MSS_MMC_PHY_DELAY_INPUT_HIGH_SPEED = 0u,
    /* delay in the input path for Default Speed work mode */
    MSS_MMC_PHY_DELAY_INPUT_DEFAULT_SPEED = 1u,
    /* delay in the input path for SDR12 work mode */
    MSS_MMC_PHY_DELAY_INPUT_SDR12 = 2u,
    /* delay in the input path for SDR25 work mode */
    MSS_MMC_PHY_DELAY_INPUT_SDR25 = 3u,
    /* delay in the input path for SDR50 work mode */
    MSS_MMC_PHY_DELAY_INPUT_SDR50 = 4u,
    /* delay in the input path for DDR50 work mode */
    MSS_MMC_PHY_DELAY_INPUT_DDR50 = 5u,
    /* delay in the input path for eMMC legacy work mode */
    MSS_MMC_PHY_DELAY_INPUT_MMC_LEGACY = 6u,
    /* delay in the input path for eMMC SDR work mode */
    MSS_MMC_PHY_DELAY_INPUT_MMC_SDR = 7u,
    /* delay in the input path for eMMC DDR work mode */
    MSS_MMC_PHY_DELAY_INPUT_MMC_DDR = 8u,
    /* Value of the delay introduced on the sdclk output for all modes except
     *  HS200, HS400 and HS400_ES
     */
    MSS_MMC_PHY_DELAY_DLL_SDCLK = 11u,
    /* Value of the delay introduced on the sdclk output for HS200, HS400 and
     *  HS400_ES speed mode
     */
    MSS_MMC_PHY_DELAY_DLL_HS_SDCLK = 12u,
    /* Value of the delay introduced on the dat_strobe input used in
     * HS400 / HS400_ES speed mode.
     */
    MSS_MMC_PHY_DELAY_DLL_DAT_STROBE = 13u,
} MSS_MMC_phydelay;

/**********************************************************************
 * Enumerations
 **********************************************************************/
/* CCCR card control registers definitions */
typedef enum
{
    /* CCCR version number and SDIO specification version number register */
    MSS_MMC_CCCR_SDIO_REV = 0u,
    /* SD version number register */
    MSS_MMC_CCCR_SD_SPEC_REV = 1u,
    /* IO enable function register */
    MSS_MMC_CCCR_IO_ENABLE = 2u,
    /* IO ready function register */
    MSS_MMC_CCCR_IO_READY = 3u,
    /* interrupt enable register */
    MSS_MMC_CCCR_INT_ENABLE = 4u,
    /* interrupt pending register */
    MSS_MMC_CCCR_INT_PENDING = 5u,
    /* IO Abort register. It used to stop a function transfer. */
    MSS_MMC_CCCR_ABORT = 6u,
    /* Bus interface control register */
    MSS_MMC_CCCR_BUS_CONTROL = 7u,
    /* Card capability register */
    MSS_MMC_CCCR_CARD_CAPABILITY = 8u,
    /* Pointer to card's common Card Information Structure (CIS) */
    MSS_MMC_CCCR_CIS_POINTER = 9u,
    /* Bus suspend register */
    MSS_MMC_CCCR_BUS_SUSPENDED = 12u,
    /* Function select register */
    MSS_MMC_CCCR_FUNCTION_SELECT = 13u,
    /* Exec flags register. The bits of this register are used by the host to
     * determine the current execution status of all functions (1-7) and memory (0).
     */
    MSS_MMC_CCCR_EXEC_FLAGS = 14u,
    /* Ready flags register. The bits of this register tell the host the read
     * or write busy status for functions (1-7) and memory (0).
     */
    MSS_MMC_CCCR_READY_FLAGS = 15u,
    /* I/O block size for Function 0 */
    MSS_MMC_CCCR_FN0_BLOCK_SIZE = 16u,
    /* Power control register */
    MSS_MMC_CCCR_POWER_CONTROL = 18u,
    /* Bus speed select */
    MSS_MMC_CCCR_HIGH_SPEED = 19u,
    /* UHS-I support info */
    MSS_MMC_CCCR_UHSI_SUPPORT = 20u,
    /* Driver Strength */
    MSS_MMC_CCCR_DRIVER_STRENGTH = 21u,
    /* Interrupt extension */
    MSS_MMC_CCCR_INT_EXT = 22u,
} MSS_MMC_cccr_reg_addr;

/* FBR card control registers definitions */
typedef enum
{
    MSS_MMC_FBR_STD_SDIO_FN = 0u,
    MSS_MMC_FBR_EXT_SDIO_FN = 1u,
    MSS_MMC_FBR_POWER_SEL = 2u,
    MSS_MMC_FBR_ADDR_CIS = 9u,
    MSS_MMC_FBR_ADDR_CSA = 12u,
    MSS_MMC_FBR_DATA_CSA = 15u,
    MSS_MMC_FBR_BLOCK_SIZE = 16u,
} MSS_MMC_fbr_reg_addr;

/* Tuple names definitions of SDIO card */
typedef enum
{
    /* NULL tuple */
    MSS_MMC_TUPLE_CISTPL_NULL = 0u,
    /* Checksum control */
    MSS_MMC_TUPLE_CISTPL_CHECKSUM = 16u,
    /* Level 1 version/product information */
    MSS_MMC_TUPLE_CISTPL_VERS_1 = 21u,
    /* Alternate language string tuple */
    MSS_MMC_TUPLE_CISTPL_ALTSTR = 22u,
    /* Manufacturer identification string tuple */
    MSS_MMC_TUPLE_CISTPL_MANFID = 32u,
    /* Function identification tuple */
    MSS_MMC_TUPLE_CISTPL_FUNCID = 33u,
    /* Additional information for functions built to support application
     * specifications for standard SDIO functions.
     */
    MSS_MMC_TUPLE_CISTPL_SDIO_STD = 145u,
    /* Reserved for future use with SDIO devices */
    MSS_MMC_TUPLE_CISTPL_SDIO_EXT = 146u,
    /* The End-of-chain Tuple */
    MSS_MMC_TUPLE_CISTPL_END = 255u,
} MSS_MMC_tuple_code;

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_MMC_TYPE_H */
