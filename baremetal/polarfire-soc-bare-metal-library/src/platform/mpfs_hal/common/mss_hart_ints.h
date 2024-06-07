/*******************************************************************************
 * Copyright 2019 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * @file mss_hart_ints.h
 * @author Microchip FPGA Embedded Systems Solutions
 * @brief MPFS local interrupt definitions
 *
 * Definitions and functions associated with local interrupts for each hart.
 *
 */

#ifndef MSS_HART_INTS_H
#define MSS_HART_INTS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Local Interrupt offsets for the E51
 */

#define    E51_MAINTENANCE_INT_OFFSET               0
#define    E51_USOC_SMB_INTERRUPT_INT_OFFSET        1
#define    E51_USOC_VC_INTERRUPT_INT_OFFSET         2
#define    E51_G5C_MESSAGE_INT_OFFSET               3
#define    E51_G5C_DEVRST_INT_OFFSET                4
#define    E51_WDOG4_TOUT_INT_OFFSET                5
#define    E51_WDOG3_TOUT_INT_OFFSET                6
#define    E51_WDOG2_TOUT_INT_OFFSET                7
#define    E51_WDOG1_TOUT_INT_OFFSET                8
#define    E51_WDOG0_TOUT_INT_OFFSET                9
#define    E51_WDOG0_MVRP_INT_OFFSET                10
#define    E51_MMUART0_INT_OFFSET                   11
#define    E51_ENVM_INT_OFFSET                      12
#define    E51_ECC_CORRECT_INT_OFFSET               13
#define    E51_ECC_ERROR_INT_OFFSET                 14
#define    E51_scb_INTERRUPT_INT_OFFSET             15
#define    E51_F2M_32_INT_OFFSET                    16
#define    E51_F2M_33_INT_OFFSET                    17
#define    E51_F2M_34_INT_OFFSET                    18
#define    E51_F2M_35_INT_OFFSET                    19
#define    E51_F2M_36_INT_OFFSET                    20
#define    E51_F2M_37_INT_OFFSET                    21
#define    E51_F2M_38_INT_OFFSET                    22
#define    E51_F2M_39_INT_OFFSET                    23
#define    E51_F2M_40_INT_OFFSET                    24
#define    E51_F2M_41_INT_OFFSET                    25
#define    E51_F2M_42_INT_OFFSET                    26
#define    E51_F2M_43_INT_OFFSET                    27
#define    E51_F2M_44_INT_OFFSET                    28
#define    E51_F2M_45_INT_OFFSET                    29
#define    E51_F2M_46_INT_OFFSET                    30
#define    E51_F2M_47_INT_OFFSET                    31
#define    E51_F2M_48_INT_OFFSET                    32
#define    E51_F2M_49_INT_OFFSET                    33
#define    E51_F2M_50_INT_OFFSET                    34
#define    E51_F2M_51_INT_OFFSET                    35

#define    E51_F2M_52_INT_OFFSET                    36
#define    E51_F2M_53_INT_OFFSET                    37
#define    E51_F2M_54_INT_OFFSET                    38
#define    E51_F2M_55_INT_OFFSET                    39
#define    E51_F2M_56_INT_OFFSET                    40
#define    E51_F2M_57_INT_OFFSET                    41
#define    E51_F2M_58_INT_OFFSET                    42
#define    E51_F2M_59_INT_OFFSET                    43
#define    E51_F2M_60_INT_OFFSET                    44
#define    E51_F2M_61_INT_OFFSET                    45

#define    E51_F2M_62_INT_OFFSET                    46
#define    E51_F2M_63_INT_OFFSET                    47

#define    LOCAL_INT_MAX                   47U  /* Highest numbered */
#define    LOCAL_INT_UNUSED                127U /* Signifies unused interrupt */

/*
 * Interrupts associated with
 * MAINTENANCE_INT_OFFSET
 * This maps to the E51_maintenance_local_IRQHandler(void) handler
 *
 * A group of interrupt events are grouped into a single maintenance interrupt
 * to the E51 CPU.
 * These interrupts are individually enabled using the following MSS system
 * register:
 *     SYSREG->MAINTENANCE_INTEN_CR
 * On receiving this interrupt the E51 should read the following system register
 * to determine the source of the interrupt
 *     SYSREG->MAINTENANCE_INT_SR
 * The bit defines associated with the MAINTENANCE_INTEN_CR and
 * MAINTENANCE_INT_SR registers are listed in the file mss_sysreg.h
 * see:
 *        MAINTENANCE_INTEN_CR_PLL_MASK                        (0x01 << 0x0)
 *        MAINTENANCE_INTEN_CR_MPU_MASK                        (0x01 << 0x1)
 *        MAINTENANCE_INTEN_CR_DECODE_MASK                     (0x01 << 0x2)
 *        MAINTENANCE_INTEN_CR_LP_STATE_ENTER_MASK             (0x01 << 0x3)
 *        MAINTENANCE_INTEN_CR_LP_STATE_EXIT_MASK              (0x01 << 0x4)
 *        MAINTENANCE_INTEN_CR_FF_START_MASK                   (0x01 << 0x5)
 *        MAINTENANCE_INTEN_CR_FF_END_MASK                     (0x01 << 0x6)
 *        MAINTENANCE_INTEN_CR_FPGA_ON_MASK                    (0x01 << 0x7)
 *        MAINTENANCE_INTEN_CR_FPGA_OFF_MASK                   (0x01 << 0x8)
 *        MAINTENANCE_INTEN_CR_SCB_ERROR_MASK                  (0x01 << 0x9)
 *        MAINTENANCE_INTEN_CR_SCB_FAULT_MASK                  (0x01 << 0xA)
 *        MAINTENANCE_INTEN_CR_MESH_ERROR_MASK                 (0x01 << 0xB)
 *        MAINTENANCE_INTEN_CR_IO_BANK_B2_ON_MASK              (0x01 << 0xC)
 *        MAINTENANCE_INTEN_CR_IO_BANK_B4_ON_MASK              (0x01 << 0xD)
 *        MAINTENANCE_INTEN_CR_IO_BANK_B5_ON_MASK              (0x01 << 0xE)
 *        MAINTENANCE_INTEN_CR_IO_BANK_B6_ON_MASK              (0x01 << 0xF)
 *        MAINTENANCE_INTEN_CR_IO_BANK_B2_OFF_MASK             (0x01 << 0x10)
 *        MAINTENANCE_INTEN_CR_IO_BANK_B4_OFF_MASK             (0x01 << 0x11)
 *        MAINTENANCE_INTEN_CR_IO_BANK_B5_OFF_MASK             (0x01 << 0x12)
 *        MAINTENANCE_INTEN_CR_IO_BANK_B6_OFF_MASK             (0x01 << 0x13)
 *        MAINTENANCE_INTEN_CR_DLL_MASK                        (0x01 << 0x14)
 *
 *  The PLL, MPU and DLL  maintenance interrupts have their own sub group
 *  registers for enabling and clearing multiple associated events
 *  See the defines for
 *      SYSREG->PLL_STATUS_INTEN_CR
 *      SYSREG->PLL_STATUS_SR
 *
 *      SYSREG->MPU_VIOLATION_INTEN_CR
 *      SYSREG->MPU_VIOLATION_SR
 *
 *      SYSREG->DLL_STATUS_CR
 *      SYSREG->DLL_STATUS_SR
 *
 */

/*
 * Interrupt numbers U54's
 */

/* U0 (first U54) and U1 connected to mac0 */
#define    U54_MAC0_INT_INT_OFFSET                 8    /* determine source mac using hart ID */
#define    U54_MAC0_QUEUE1_INT_OFFSET              7
#define    U54_MAC0_QUEUE2_INT_OFFSET              6
#define    U54_MAC0_QUEUE3_INT_OFFSET              5
#define    U54_MAC0_EMAC_INT_OFFSET                4
#define    U54_MAC0_MMSL_INT_OFFSET                3

/* U2 and U3 connected to mac1 */
#define    U54_MAC1_INT_INT_OFFSET                 8    /* determine source mac using hart ID */
#define    U54_MAC1_QUEUE1_INT_OFFSET              7
#define    U54_MAC1_QUEUE2_INT_OFFSET              6
#define    U54_MAC1_QUEUE3_INT_OFFSET              5
#define    U54_MAC1_EMAC_INT_OFFSET                4
#define    U54_MAC1_MMSL_INT_OFFSET                3

/* MMUART1 connected to U54 0 */
/* MMUART2 connected to U54 1 */
/* MMUART3 connected to U54 2 */
/* MMUART4 connected to U54 3 */
#define    U54_MMUARTx_INT_OFFSET                  11    /* MMUART1 connected to U54 0 */
#define    U54_WDOGx_MVRP_INT_OFFSET               10    /* determine source mac using hart ID */
#define    U54_WDOGx_TOUT_INT_OFFSET                9    /* determine source mac using hart ID */

#define    U54_F2M_0_INT_OFFSET          16
#define    U54_F2M_1_INT_OFFSET          17
#define    U54_F2M_2_INT_OFFSET          18
#define    U54_F2M_3_INT_OFFSET          19
#define    U54_F2M_4_INT_OFFSET          20
#define    U54_F2M_5_INT_OFFSET          21
#define    U54_F2M_6_INT_OFFSET          22
#define    U54_F2M_7_INT_OFFSET          23
#define    U54_F2M_8_INT_OFFSET          24
#define    U54_F2M_9_INT_OFFSET          25

#define    U54_F2M_10_INT_OFFSET         26
#define    U54_F2M_11_INT_OFFSET         27
#define    U54_F2M_12_INT_OFFSET         28
#define    U54_F2M_13_INT_OFFSET         29
#define    U54_F2M_14_INT_OFFSET         30
#define    U54_F2M_15_INT_OFFSET         31
#define    U54_F2M_16_INT_OFFSET         32
#define    U54_F2M_17_INT_OFFSET         33
#define    U54_F2M_18_INT_OFFSET         34
#define    U54_F2M_19_INT_OFFSET         35

#define    U54_F2M_20_INT_OFFSET         36
#define    U54_F2M_21_INT_OFFSET         37
#define    U54_F2M_22_INT_OFFSET         38
#define    U54_F2M_23_INT_OFFSET         39
#define    U54_F2M_24_INT_OFFSET         40
#define    U54_F2M_25_INT_OFFSET         41
#define    U54_F2M_26_INT_OFFSET         42
#define    U54_F2M_27_INT_OFFSET         43
#define    U54_F2M_28_INT_OFFSET         44
#define    U54_F2M_29_INT_OFFSET         45

#define    U54_F2M_30_INT_OFFSET         46
#define    U54_F2M_31_INT_OFFSET         47


void handle_m_ext_interrupt(void);
void E51_software_IRQHandler(void);
void U54_1_software_IRQHandler(void);
void U54_2_software_IRQHandler(void);
void U54_3_software_IRQHandler(void);
void U54_4_software_IRQHandler(void);
void E51_sysTick_IRQHandler(void);
void U54_1_sysTick_IRQHandler(void);
void U54_2_sysTick_IRQHandler(void);
void U54_3_sysTick_IRQHandler(void);
void U54_4_sysTick_IRQHandler(void);

/*
 *
 *   Local interrupt defines
 *
 */
void E51_maintenance_local_IRQHandler(void);
void E51_usoc_smb_local_IRQHandler(void);
void E51_usoc_vc_local_IRQHandler(void);
void E51_g5c_message_local_IRQHandler(void);
void E51_g5c_devrst_local_IRQHandler(void);
void E51_wdog4_tout_local_IRQHandler(void);
void E51_wdog3_tout_local_IRQHandler(void);
void E51_wdog2_tout_local_IRQHandler(void);
void E51_wdog1_tout_local_IRQHandler(void);
void E51_wdog0_tout_local_IRQHandler(void);
void E51_wdog0_mvrp_local_IRQHandler(void);
void E51_mmuart0_local_IRQHandler(void);
void E51_envm_local_IRQHandler(void);
void E51_ecc_correct_local_IRQHandler(void);
void E51_ecc_error_local_IRQHandler(void);
void E51_scb_local_IRQHandler(void);
void E51_f2m_32_local_IRQHandler(void);
void E51_f2m_33_local_IRQHandler(void);
void E51_f2m_34_local_IRQHandler(void);
void E51_f2m_35_local_IRQHandler(void);
void E51_f2m_36_local_IRQHandler(void);
void E51_f2m_37_local_IRQHandler(void);
void E51_f2m_38_local_IRQHandler(void);
void E51_f2m_39_local_IRQHandler(void);
void E51_f2m_40_local_IRQHandler(void);
void E51_f2m_41_local_IRQHandler(void);
void E51_f2m_42_local_IRQHandler(void);
void E51_f2m_43_local_IRQHandler(void);
void E51_f2m_44_local_IRQHandler(void);
void E51_f2m_45_local_IRQHandler(void);
void E51_f2m_46_local_IRQHandler(void);
void E51_f2m_47_local_IRQHandler(void);
void E51_f2m_48_local_IRQHandler(void);
void E51_f2m_49_local_IRQHandler(void);
void E51_f2m_50_local_IRQHandler(void);
void E51_f2m_51_local_IRQHandler(void);
void E51_f2m_52_local_IRQHandler(void);
void E51_f2m_53_local_IRQHandler(void);
void E51_f2m_54_local_IRQHandler(void);
void E51_f2m_55_local_IRQHandler(void);
void E51_f2m_56_local_IRQHandler(void);
void E51_f2m_57_local_IRQHandler(void);
void E51_f2m_58_local_IRQHandler(void);
void E51_f2m_59_local_IRQHandler(void);
void E51_f2m_60_local_IRQHandler(void);
void E51_f2m_61_local_IRQHandler(void);
void E51_f2m_62_local_IRQHandler(void);
void E51_f2m_63_local_IRQHandler(void);

/*
 * U54
 */
void U54_spare_0_local_IRQHandler(void);
void U54_spare_1_local_IRQHandler(void);
void U54_spare_2_local_IRQHandler(void);

void U54_1_mac0_mmsl_local_IRQHandler(void);
void U54_1_mac0_emac_local_IRQHandler(void);
void U54_1_mac0_queue3_local_IRQHandler(void);
void U54_1_mac0_queue2_local_IRQHandler(void);
void U54_1_mac0_queue1_local_IRQHandler(void);
void U54_1_mac0_int_local_IRQHandler(void);

void U54_2_mac0_mmsl_local_IRQHandler(void);
void U54_2_mac0_emac_local_IRQHandler(void);
void U54_2_mac0_queue3_local_IRQHandler(void);
void U54_2_mac0_queue2_local_IRQHandler(void);
void U54_2_mac0_queue1_local_IRQHandler(void);
void U54_2_mac0_int_local_IRQHandler(void);

void U54_3_mac1_mmsl_local_IRQHandler(void);
void U54_3_mac1_emac_local_IRQHandler(void);
void U54_3_mac1_queue3_local_IRQHandler(void);
void U54_3_mac1_queue2_local_IRQHandler(void);
void U54_3_mac1_queue1_local_IRQHandler(void);
void U54_3_mac1_int_local_IRQHandler(void);

void U54_4_mac1_mmsl_local_IRQHandler(void);
void U54_4_mac1_emac_local_IRQHandler(void);
void U54_4_mac1_queue3_local_IRQHandler(void);
void U54_4_mac1_queue2_local_IRQHandler(void);
void U54_4_mac1_queue1_local_IRQHandler(void);
void U54_4_mac1_int_local_IRQHandler(void);

void U54_1_wdog_tout_local_IRQHandler(void);
void U54_2_wdog_tout_local_IRQHandler(void);
void U54_3_wdog_tout_local_IRQHandler(void);
void U54_4_wdog_tout_local_IRQHandler(void);
void mvrp_u54_local_IRQHandler_10(void);    /* legacy name */
void U54_1_wdog_mvrp_local_IRQHandler(void);
void U54_2_wdog_mvrp_local_IRQHandler(void);
void U54_3_wdog_mvrp_local_IRQHandler(void);
void U54_4_wdog_mvrp_local_IRQHandler(void);
void U54_1_mmuart1_local_IRQHandler(void);
void U54_2_mmuart2_local_IRQHandler(void);
void U54_3_mmuart3_local_IRQHandler(void);
void U54_4_mmuart4_local_IRQHandler(void);
void U54_spare_3_local_IRQHandler(void);
void U54_spare_4_local_IRQHandler(void);
void U54_spare_5_local_IRQHandler(void);
void U54_spare_6_local_IRQHandler(void);
void U54_f2m_0_local_IRQHandler(void);
void U54_f2m_1_local_IRQHandler(void);
void U54_f2m_2_local_IRQHandler(void);
void U54_f2m_3_local_IRQHandler(void);
void U54_f2m_4_local_IRQHandler(void);
void U54_f2m_5_local_IRQHandler(void);
void U54_f2m_6_local_IRQHandler(void);
void U54_f2m_7_local_IRQHandler(void);
void U54_f2m_8_local_IRQHandler(void);
void U54_f2m_9_local_IRQHandler(void);
void U54_f2m_10_local_IRQHandler(void);
void U54_f2m_11_local_IRQHandler(void);
void U54_f2m_12_local_IRQHandler(void);
void U54_f2m_13_local_IRQHandler(void);
void U54_f2m_14_local_IRQHandler(void);
void U54_f2m_15_local_IRQHandler(void);
void U54_f2m_16_local_IRQHandler(void);
void U54_f2m_17_local_IRQHandler(void);
void U54_f2m_18_local_IRQHandler(void);
void U54_f2m_19_local_IRQHandler(void);
void U54_f2m_20_local_IRQHandler(void);
void U54_f2m_21_local_IRQHandler(void);
void U54_f2m_22_local_IRQHandler(void);
void U54_f2m_23_local_IRQHandler(void);
void U54_f2m_24_local_IRQHandler(void);
void U54_f2m_25_local_IRQHandler(void);
void U54_f2m_26_local_IRQHandler(void);
void U54_f2m_27_local_IRQHandler(void);
void U54_f2m_28_local_IRQHandler(void);
void U54_f2m_29_local_IRQHandler(void);
void U54_f2m_30_local_IRQHandler(void);
void U54_f2m_31_local_IRQHandler(void);


#ifdef __cplusplus
}
#endif

#endif  /* MSS_HART_INTS_H */
