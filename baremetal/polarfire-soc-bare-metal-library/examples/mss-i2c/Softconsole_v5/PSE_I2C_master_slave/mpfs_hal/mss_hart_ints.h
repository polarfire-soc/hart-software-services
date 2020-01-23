/*******************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions.  All rights reserved.
 *
 * @file mss_plic.h
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief MPFS local interrupt definitions
 *
 * Definitions and functions associated with local interrupts for each hart.
 *
 * SVN $Revision: 11020 $
 * SVN $Date: 2019-02-22 15:31:40 +0000 (Fri, 22 Feb 2019) $
 */
#ifndef MSS_HART_INT_H
#define MSS_HART_INT_H

#include <stdint.h>

#include "encoding.h"
#include "mss_hart_ints.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SIFIVE_HIFIVE_UNLEASHED
#define PLIC_NUM_SOURCES 187
#else
#define PLIC_NUM_SOURCES 54    /* 53 actual, source 0 is not used */
#endif

#define PLIC_NUM_PRIORITIES 7
#define PLIC_NUM_PRIORITIES 7

/*
 * Interrupt numbers U0
 */

#define MAINTENANCE_E51_INT               0
#define USOC_SMB_INTERRUPT_E51_INT        1
#define USOC_VC_INTERRUPT_E51_INT         2
#define G5C_MESSAGE_E51_INT               3
#define G5C_DEVRST_E51_INT                4
#define WDOG4_TOUT_E51_INT                5
#define WDOG3_TOUT_E51_INT                6
#define WDOG2_TOUT_E51_INT                7
#define WDOG1_TOUT_E51_INT                8
#define WDOG0_TOUT_E51_INT                9
#define WDOG0_MVRP_E51_INT               10
#define    MMUART0_E51_INT               11
#define    ENVM_E51_INT                  12
#define    ECC_CORRECT_E51_INT           13
#define    ECC_ERROR_E51_INT             14
#define    scb_INTERRUPT_E51_INT         15
#define    FABRIC_F2H_32_E51_INT         16
#define    FABRIC_F2H_33_E51_INT         17
#define    FABRIC_F2H_34_E51_INT         18
#define    FABRIC_F2H_35_E51_INT         19
#define    FABRIC_F2H_36_E51_INT         20
#define    FABRIC_F2H_37_E51_INT         21
#define    FABRIC_F2H_38_E51_INT         22
#define    FABRIC_F2H_39_E51_INT         23
#define    FABRIC_F2H_40_E51_INT         24
#define    FABRIC_F2H_41_E51_INT         25

#define    FABRIC_F2H_42_E51_INT         26
#define    FABRIC_F2H_43_E51_INT         27
#define    FABRIC_F2H_44_E51_INT         28
#define    FABRIC_F2H_45_E51_INT         29
#define    FABRIC_F2H_46_E51_INT         30
#define    FABRIC_F2H_47_E51_INT         31
#define    FABRIC_F2H_48_E51_INT         32
#define    FABRIC_F2H_49_E51_INT         33
#define    FABRIC_F2H_50_E51_INT         34
#define    FABRIC_F2H_51_E51_INT         35

#define    FABRIC_F2H_52_E51_INT         36
#define    FABRIC_F2H_53_E51_INT         37
#define    FABRIC_F2H_54_E51_INT         38
#define    FABRIC_F2H_55_E51_INT         39
#define    FABRIC_F2H_56_E51_INT         40
#define    FABRIC_F2H_57_E51_INT         41
#define    FABRIC_F2H_58_E51_INT         42
#define    FABRIC_F2H_59_E51_INT         43
#define    FABRIC_F2H_60_E51_INT         44
#define    FABRIC_F2H_61_E51_INT         45

#define    FABRIC_F2H_62_E51_INT         46
#define    FABRIC_F2H_63_E51_INT         47

/*
 * Interrupts associated with
 * MAINTENANCE_E51_INT
 *
 * A group of interrupt events are grouped into a single maintenance interrupt to the E51 CPU,
 * on receiving this interrupt the E51 should read the maintenance system register to find out
 * the interrupt source. The maintenance interrupts are defined below
 */
#define MAINTENANCE_E51_pll_INT                            0
#define MAINTENANCE_E51_mpu_INT                            1
#define MAINTENANCE_E51_lp_state_enter_INT                 2
#define MAINTENANCE_E51_lp_state_exit_INT                  3
#define MAINTENANCE_E51_ff_start_INT                       4
#define MAINTENANCE_E51_ff_end_INT                         5
#define MAINTENANCE_E51_fpga_on_INT                        6
#define MAINTENANCE_E51_fpga_off_INT                       7
#define MAINTENANCE_E51_scb_error_INT                      8
#define MAINTENANCE_E51_scb_fault_INT                      9
#define MAINTENANCE_E51_mesh_error_INT                     10
#define MAINTENANCE_E51_io_bank_b2_on_INT                  12
#define MAINTENANCE_E51_io_bank_b4_on_INT                  13
#define MAINTENANCE_E51_io_bank_b5_on_INT                  14
#define MAINTENANCE_E51_io_bank_b6_on_INT                  15
#define MAINTENANCE_E51_io_bank_b2_off_INT                 16
#define MAINTENANCE_E51_io_bank_b4_off_INT                 17
#define MAINTENANCE_E51_io_bank_b5_off_INT                 18
#define MAINTENANCE_E51_io_bank_b6_off_INT                 19


/*
 * E51-0 is Maintenance Interrupt CPU needs to read status register to determine exact cause:
 * These defines added here for clarity need to replay with status register defines
 * for determining interrupt cause
 */
#if FOR_CLARITY

#define     mpu_fail_plic             0
#define     lp_state_enter_plic       1
#define     lp_state_exit_plic        2
#define     ff_start_plic             3
#define     ff_end_plic               4
#define     fpga_on_plic              5
#define     fpga_off_plic             6
#define     scb_error_plic            7
#define     scb_fault_plic            8
#define     mesh_fail_plic            9

#endif

/*
 * Interrupt numbers U54's
 */

/* U0 (first U54) and U1 connected to mac0 */
#define    MAC0_1_INT_U54_INT               8    /* determine source mac using hart ID */
#define    MAC0_1__QUEUE1_U54_INT           7
#define    MAC0_1__QUEUE2_U54_INT           6
#define    MAC0_1__QUEUE3_U54_INT           5
#define    MAC0_1__emac_U54_INT             4
#define    MAC0_1__mmsl_U54_INT             3

/* U2 and U3 connected to mac1 */
#define    MAC1_1_INT_U54_INT               8    /* determine source mac using hart ID */
#define    MAC1_1__QUEUE1_U54_INT           7
#define    MAC1_1__QUEUE2_U54_INT           6
#define    MAC1_1__QUEUE3_U54_INT           5
#define    MAC1_1__EMAC_U54_INT             4
#define    MAC1_1__MMSL_U54_INT             3

/* MMUART1 connected to U54 0 */
/* MMUART2 connected to U54 1 */
/* MMUART3 connected to U54 2 */
/* MMUART4 connected to U54 3 */
#define    MMUARTx_U54_INT                  11    /* MMUART1 connected to U54 0 */
#define    WDOGx_MVRP_U54_INT               10    /* determine source mac using hart ID */
#define    WDOGx_TOUT_U54_INT                9    /* determine source mac using hart ID */

#define    H2_FABRIC_F2H_0_U54_INT          16
#define    H2_FABRIC_F2H_1_U54_INT          17
#define    H2_FABRIC_F2H_2_U54_INT          18
#define    H2_FABRIC_F2H_3_U54_INT          19
#define    H2_FABRIC_F2H_4_U54_INT          20
#define    H2_FABRIC_F2H_5_U54_INT          21
#define    H2_FABRIC_F2H_6_U54_INT          22
#define    H2_FABRIC_F2H_7_U54_INT          23
#define    H2_FABRIC_F2H_8_U54_INT          24
#define    H2_FABRIC_F2H_9_U54_INT          25

#define    H2_FABRIC_F2H_10_U54_INT         26
#define    H2_FABRIC_F2H_11_U54_INT         27
#define    H2_FABRIC_F2H_12_U54_INT         28
#define    H2_FABRIC_F2H_13_U54_INT         29
#define    H2_FABRIC_F2H_14_U54_INT         30
#define    H2_FABRIC_F2H_15_U54_INT         31
#define    H2_FABRIC_F2H_16_U54_INT         32
#define    H2_FABRIC_F2H_17_U54_INT         33
#define    H2_FABRIC_F2H_18_U54_INT         34
#define    H2_FABRIC_F2H_19_U54_INT         35

#define    H2_FABRIC_F2H_20_U54_INT         36
#define    H2_FABRIC_F2H_21_U54_INT         37
#define    H2_FABRIC_F2H_22_U54_INT         38
#define    H2_FABRIC_F2H_23_U54_INT         39
#define    H2_FABRIC_F2H_24_U54_INT         40
#define    H2_FABRIC_F2H_25_U54_INT         41
#define    H2_FABRIC_F2H_26_U54_INT         42
#define    H2_FABRIC_F2H_27_U54_INT         43
#define    H2_FABRIC_F2H_28_U54_INT         44
#define    H2_FABRIC_F2H_29_U54_INT         45

#define    H2_FABRIC_F2H_30_U54_INT         46
#define    H2_FABRIC_F2H_31_U54_INT         47


#ifdef __cplusplus
}
#endif

#endif  /* RISCV_PLIC_H */
