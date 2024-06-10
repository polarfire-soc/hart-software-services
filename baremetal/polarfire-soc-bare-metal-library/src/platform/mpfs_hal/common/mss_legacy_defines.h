/*******************************************************************************
 * Copyright 2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * @file mss_legacy_defines.h
 * @author Microchip FPGA Embedded Systems Solutions
 * @brief MPFS legacy defines
 *
 */

#ifndef MSS_LEGACY_DEFINES_H
#define MSS_LEGACY_DEFINES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define    maintenance_e51_local_IRQHandler_0          E51_maintenance_local_IRQHandler
#define    usoc_smb_interrupt_e51_local_IRQHandler_1   E51_usoc_smb_local_IRQHandler
#define    usoc_vc_interrupt_e51_local_IRQHandler_2    E51_usoc_vc_local_IRQHandler
#define    g5c_message_e51_local_IRQHandler_3          E51_g5c_message_local_IRQHandler
#define    g5c_devrst_e51_local_IRQHandler_4           E51_g5c_devrst_local_IRQHandler
#define    wdog4_tout_e51_local_IRQHandler_5           E51_wdog4_tout_local_IRQHandler
#define    wdog3_tout_e51_local_IRQHandler_6           E51_wdog3_tout_local_IRQHandler
#define    wdog2_tout_e51_local_IRQHandler_7           E51_wdog2_tout_local_IRQHandler
#define    wdog1_tout_e51_local_IRQHandler_8           E51_wdog1_tout_local_IRQHandler
#define    wdog0_tout_e51_local_IRQHandler_9           E51_wdog0_tout_local_IRQHandler
#define    wdog0_mvrp_e51_local_IRQHandler_10          E51_wdog0_mvrp_local_IRQHandler
#define    mmuart0_e51_local_IRQHandler_11             E51_mmuart0_local_IRQHandler
#define    envm_e51_local_IRQHandler_12                E51_envm_local_IRQHandler
#define    ecc_correct_e51_local_IRQHandler_13         E51_ecc_correct_local_IRQHandler
#define    ecc_error_e51_local_IRQHandler_14           E51_ecc_error_local_IRQHandler
#define    scb_interrupt_e51_local_IRQHandler_15       E51_scb_local_IRQHandler
#define    fabric_f2h_32_e51_local_IRQHandler_16       E51_f2m_32_local_IRQHandler
#define    fabric_f2h_33_e51_local_IRQHandler_17       E51_f2m_33_local_IRQHandler
#define    fabric_f2h_34_e51_local_IRQHandler_18       E51_f2m_34_local_IRQHandler
#define    fabric_f2h_35_e51_local_IRQHandler_19       E51_f2m_35_local_IRQHandler
#define    fabric_f2h_36_e51_local_IRQHandler_20       E51_f2m_36_local_IRQHandler
#define    fabric_f2h_37_e51_local_IRQHandler_21       E51_f2m_37_local_IRQHandler
#define    fabric_f2h_38_e51_local_IRQHandler_22       E51_f2m_38_local_IRQHandler
#define    fabric_f2h_39_e51_local_IRQHandler_23       E51_f2m_39_local_IRQHandler
#define    fabric_f2h_40_e51_local_IRQHandler_24       E51_f2m_40_local_IRQHandler
#define    fabric_f2h_41_e51_local_IRQHandler_25       E51_f2m_41_local_IRQHandler
#define    fabric_f2h_42_e51_local_IRQHandler_26       E51_f2m_42_local_IRQHandler
#define    fabric_f2h_43_e51_local_IRQHandler_27       E51_f2m_43_local_IRQHandler
#define    fabric_f2h_44_e51_local_IRQHandler_28       E51_f2m_44_local_IRQHandler
#define    fabric_f2h_45_e51_local_IRQHandler_29       E51_f2m_45_local_IRQHandler
#define    fabric_f2h_46_e51_local_IRQHandler_30       E51_f2m_46_local_IRQHandler
#define    fabric_f2h_47_e51_local_IRQHandler_31       E51_f2m_47_local_IRQHandler
#define    fabric_f2h_48_e51_local_IRQHandler_32       E51_f2m_48_local_IRQHandler
#define    fabric_f2h_49_e51_local_IRQHandler_33       E51_f2m_49_local_IRQHandler
#define    fabric_f2h_50_e51_local_IRQHandler_34       E51_f2m_50_local_IRQHandler
#define    fabric_f2h_51_e51_local_IRQHandler_35       E51_f2m_51_local_IRQHandler
#define    fabric_f2h_52_e51_local_IRQHandler_36       E51_f2m_52_local_IRQHandler
#define    fabric_f2h_53_e51_local_IRQHandler_37       E51_f2m_53_local_IRQHandler
#define    fabric_f2h_54_e51_local_IRQHandler_38       E51_f2m_54_local_IRQHandler
#define    fabric_f2h_55_e51_local_IRQHandler_39       E51_f2m_55_local_IRQHandler
#define    fabric_f2h_56_e51_local_IRQHandler_40       E51_f2m_56_local_IRQHandler
#define    fabric_f2h_57_e51_local_IRQHandler_41       E51_f2m_57_local_IRQHandler
#define    fabric_f2h_58_e51_local_IRQHandler_42       E51_f2m_58_local_IRQHandler
#define    fabric_f2h_59_e51_local_IRQHandler_43       E51_f2m_59_local_IRQHandler
#define    fabric_f2h_60_e51_local_IRQHandler_44       E51_f2m_60_local_IRQHandler
#define    fabric_f2h_61_e51_local_IRQHandler_45       E51_f2m_61_local_IRQHandler
#define    fabric_f2h_62_e51_local_IRQHandler_46       E51_f2m_62_local_IRQHandler
#define    fabric_f2h_63_e51_local_IRQHandler_47       E51_f2m_63_local_IRQHandler

#define    spare_u54_local_IRQHandler_0                U54_spare_0_local_IRQHandler
#define    spare_u54_local_IRQHandler_1                U54_spare_1_local_IRQHandler
#define    spare_u54_local_IRQHandler_2                U54_spare_2_local_IRQHandler

#define    mac_mmsl_u54_1_local_IRQHandler_3           U54_1_mac0_mmsl_local_IRQHandler
#define    mac_emac_u54_1_local_IRQHandler_4           U54_1_mac0_emac_local_IRQHandler
#define    mac_queue3_u54_1_local_IRQHandler_5         U54_1_mac0_queue3_local_IRQHandler
#define    mac_queue2_u54_1_local_IRQHandler_6         U54_1_mac0_queue2_local_IRQHandler
#define    mac_queue1_u54_1_local_IRQHandler_7         U54_1_mac0_queue1_local_IRQHandler
#define    mac_int_u54_1_local_IRQHandler_8            U54_1_mac0_int_local_IRQHandler

#define    mac_mmsl_u54_2_local_IRQHandler_3           U54_2_mac0_mmsl_local_IRQHandler
#define    mac_emac_u54_2_local_IRQHandler_4           U54_2_mac0_emac_local_IRQHandler
#define    mac_queue3_u54_2_local_IRQHandler_5         U54_2_mac0_queue3_local_IRQHandler
#define    mac_queue2_u54_2_local_IRQHandler_6         U54_2_mac0_queue2_local_IRQHandler
#define    mac_queue1_u54_2_local_IRQHandler_7         U54_2_mac0_queue1_local_IRQHandler
#define    mac_int_u54_2_local_IRQHandler_8            U54_2_mac0_int_local_IRQHandler

#define    mac_mmsl_u54_3_local_IRQHandler_3           U54_3_mac1_mmsl_local_IRQHandler
#define    mac_emac_u54_3_local_IRQHandler_4           U54_3_mac1_emac_local_IRQHandler
#define    mac_queue3_u54_3_local_IRQHandler_5         U54_3_mac1_queue3_local_IRQHandler
#define    mac_queue2_u54_3_local_IRQHandler_6         U54_3_mac1_queue2_local_IRQHandler
#define    mac_queue1_u54_3_local_IRQHandler_7         U54_3_mac1_queue1_local_IRQHandler
#define    mac_int_u54_3_local_IRQHandler_8            U54_3_mac1_int_local_IRQHandler

#define    mac_mmsl_u54_4_local_IRQHandler_3           U54_4_mac1_mmsl_local_IRQHandler
#define    mac_emac_u54_4_local_IRQHandler_4           U54_4_mac1_emac_local_IRQHandler
#define    mac_queue3_u54_4_local_IRQHandler_5         U54_4_mac1_queue3_local_IRQHandler
#define    mac_queue2_u54_4_local_IRQHandler_6         U54_4_mac1_queue2_local_IRQHandler
#define    mac_queue1_u54_4_local_IRQHandler_7         U54_4_mac1_queue1_local_IRQHandler
#define    mac_int_u54_4_local_IRQHandler_8            U54_4_mac1_int_local_IRQHandler

#define    wdog_tout_u54_h1_local_IRQHandler_9         U54_1_wdog_tout_local_IRQHandler

/*
 * Update your watchdog code if using  mvrp_u54_local_IRQHandler_10()
 * to use the following instead:
 *   U54_1_wdog_mvrp_local_IRQHandler()
 *   U54_2_wdog_mvrp_local_IRQHandler()
 *   U54_3_wdog_mvrp_local_IRQHandler()
 *   U54_4_wdog_mvrp_local_IRQHandler()
 */
#define    mmuart_u54_h1_local_IRQHandler_11           U54_1_mmuart1_local_IRQHandler

#define    wdog_tout_u54_h2_local_IRQHandler_9         U54_2_wdog_tout_local_IRQHandler
#define    mmuart_u54_h2_local_IRQHandler_11           U54_2_mmuart2_local_IRQHandler

#define    wdog_tout_u54_h3_local_IRQHandler_9         U54_3_wdog_tout_local_IRQHandler
#define    mmuart_u54_h3_local_IRQHandler_11           U54_3_mmuart3_local_IRQHandler

#define    wdog_tout_u54_h4_local_IRQHandler_9         U54_4_wdog_tout_local_IRQHandler
#define    mmuart_u54_h4_local_IRQHandler_11           U54_4_mmuart4_local_IRQHandler

#define    spare_u54_local_IRQHandler_12               U54_spare_3_local_IRQHandler
#define    spare_u54_local_IRQHandler_13               U54_spare_4_local_IRQHandler
#define    spare_u54_local_IRQHandler_14               U54_spare_5_local_IRQHandler
#define    spare_u54_local_IRQHandler_15               U54_spare_6_local_IRQHandler
#define    fabric_f2h_0_u54_local_IRQHandler_16        U54_f2m_0_local_IRQHandler
#define    fabric_f2h_1_u54_local_IRQHandler_17        U54_f2m_1_local_IRQHandler
#define    fabric_f2h_2_u54_local_IRQHandler_18        U54_f2m_2_local_IRQHandler
#define    fabric_f2h_3_u54_local_IRQHandler_19        U54_f2m_3_local_IRQHandler
#define    fabric_f2h_4_u54_local_IRQHandler_20        U54_f2m_4_local_IRQHandler
#define    fabric_f2h_5_u54_local_IRQHandler_21        U54_f2m_5_local_IRQHandler
#define    fabric_f2h_6_u54_local_IRQHandler_22        U54_f2m_6_local_IRQHandler
#define    fabric_f2h_7_u54_local_IRQHandler_23        U54_f2m_7_local_IRQHandler
#define    fabric_f2h_8_u54_local_IRQHandler_24        U54_f2m_8_local_IRQHandler
#define    fabric_f2h_9_u54_local_IRQHandler_25        U54_f2m_9_local_IRQHandler
#define    fabric_f2h_10_u54_local_IRQHandler_26       U54_f2m_10_local_IRQHandler
#define    fabric_f2h_11_u54_local_IRQHandler_27       U54_f2m_11_local_IRQHandler
#define    fabric_f2h_12_u54_local_IRQHandler_28       U54_f2m_12_local_IRQHandler
#define    fabric_f2h_13_u54_local_IRQHandler_29       U54_f2m_13_local_IRQHandler
#define    fabric_f2h_14_u54_local_IRQHandler_30       U54_f2m_14_local_IRQHandler
#define    fabric_f2h_15_u54_local_IRQHandler_31       U54_f2m_15_local_IRQHandler
#define    fabric_f2h_16_u54_local_IRQHandler_32       U54_f2m_16_local_IRQHandler
#define    fabric_f2h_17_u54_local_IRQHandler_33       U54_f2m_17_local_IRQHandler
#define    fabric_f2h_18_u54_local_IRQHandler_34       U54_f2m_18_local_IRQHandler
#define    fabric_f2h_19_u54_local_IRQHandler_35       U54_f2m_19_local_IRQHandler
#define    fabric_f2h_20_u54_local_IRQHandler_36       U54_f2m_20_local_IRQHandler
#define    fabric_f2h_21_u54_local_IRQHandler_37       U54_f2m_21_local_IRQHandler
#define    fabric_f2h_22_u54_local_IRQHandler_38       U54_f2m_22_local_IRQHandler
#define    fabric_f2h_23_u54_local_IRQHandler_39       U54_f2m_23_local_IRQHandler
#define    fabric_f2h_24_u54_local_IRQHandler_40       U54_f2m_24_local_IRQHandler
#define    fabric_f2h_25_u54_local_IRQHandler_41       U54_f2m_25_local_IRQHandler
#define    fabric_f2h_26_u54_local_IRQHandler_42       U54_f2m_26_local_IRQHandler
#define    fabric_f2h_27_u54_local_IRQHandler_43       U54_f2m_27_local_IRQHandler
#define    fabric_f2h_28_u54_local_IRQHandler_44       U54_f2m_28_local_IRQHandler
#define    fabric_f2h_29_u54_local_IRQHandler_45       U54_f2m_29_local_IRQHandler
#define    fabric_f2h_30_u54_local_IRQHandler_46       U54_f2m_30_local_IRQHandler
#define    fabric_f2h_31_u54_local_IRQHandler_47       U54_f2m_31_local_IRQHandler

#define    MAINTENANCE_E51_INT               0
#define    USOC_SMB_INTERRUPT_E51_INT        1
#define    USOC_VC_INTERRUPT_E51_INT         2
#define    G5C_MESSAGE_E51_INT               3
#define    G5C_DEVRST_E51_INT                4
#define    WDOG4_TOUT_E51_INT                5
#define    WDOG3_TOUT_E51_INT                6
#define    WDOG2_TOUT_E51_INT                7
#define    WDOG1_TOUT_E51_INT                8
#define    WDOG0_TOUT_E51_INT                9
#define    WDOG0_MVRP_E51_INT               10
#define    MMUART0_E51_INT                  11
#define    ENVM_E51_INT                     12
#define    ECC_CORRECT_E51_INT              13
#define    ECC_ERROR_E51_INT                14
#define    scb_INTERRUPT_E51_INT            15
#define    FABRIC_F2H_32_E51_INT            16
#define    FABRIC_F2H_33_E51_INT            17
#define    FABRIC_F2H_34_E51_INT            18
#define    FABRIC_F2H_35_E51_INT            19
#define    FABRIC_F2H_36_E51_INT            20
#define    FABRIC_F2H_37_E51_INT            21
#define    FABRIC_F2H_38_E51_INT            22
#define    FABRIC_F2H_39_E51_INT            23
#define    FABRIC_F2H_40_E51_INT            24
#define    FABRIC_F2H_41_E51_INT            25

#define    FABRIC_F2H_42_E51_INT            26
#define    FABRIC_F2H_43_E51_INT            27
#define    FABRIC_F2H_44_E51_INT            28
#define    FABRIC_F2H_45_E51_INT            29
#define    FABRIC_F2H_46_E51_INT            30
#define    FABRIC_F2H_47_E51_INT            31
#define    FABRIC_F2H_48_E51_INT            32
#define    FABRIC_F2H_49_E51_INT            33
#define    FABRIC_F2H_50_E51_INT            34
#define    FABRIC_F2H_51_E51_INT            35

#define    FABRIC_F2H_52_E51_INT            36
#define    FABRIC_F2H_53_E51_INT            37
#define    FABRIC_F2H_54_E51_INT            38
#define    FABRIC_F2H_55_E51_INT            39
#define    FABRIC_F2H_56_E51_INT            40
#define    FABRIC_F2H_57_E51_INT            41
#define    FABRIC_F2H_58_E51_INT            42
#define    FABRIC_F2H_59_E51_INT            43
#define    FABRIC_F2H_60_E51_INT            44
#define    FABRIC_F2H_61_E51_INT            45

#define    FABRIC_F2H_62_E51_INT            46
#define    FABRIC_F2H_63_E51_INT            47

/* U0 (first U54) and U1 connected to mac0 */
#define    MAC0_INT_U54_INT                 8    /* determine source mac using hart ID */
#define    MAC0_QUEUE1_U54_INT              7
#define    MAC0_QUEUE2_U54_INT              6
#define    MAC0_QUEUE3_U54_INT              5
#define    MAC0_EMAC_U54_INT                4
#define    MAC0_MMSL_U54_INT                3

/* U2 and U3 connected to mac1 */
#define    MAC1_INT_U54_INT                 8    /* determine source mac using hart ID */
#define    MAC1_QUEUE1_U54_INT              7
#define    MAC1_QUEUE2_U54_INT              6
#define    MAC1_QUEUE3_U54_INT              5
#define    MAC1_EMAC_U54_INT                4
#define    MAC1_MMSL_U54_INT                3

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



#define     Invalid_IRQHandler   PLIC_Invalid_IRQHandler
#define     l2_metadata_corr_IRQHandler  PLIC_l2_metadata_corr_IRQHandler
#define     l2_metadata_uncorr_IRQHandler    PLIC_l2_metadata_uncorr_IRQHandler
#define     l2_data_corr_IRQHandler  PLIC_l2_data_corr_IRQHandler
#define     l2_data_uncorr_IRQHandler    PLIC_l2_data_uncorr_IRQHandler
#define     dma_ch0_DONE_IRQHandler  PLIC_dma_ch0_DONE_IRQHandler
#define     dma_ch0_ERR_IRQHandler   PLIC_dma_ch0_ERR_IRQHandler
#define     dma_ch1_DONE_IRQHandler  PLIC_dma_ch1_DONE_IRQHandler
#define     dma_ch1_ERR_IRQHandler   PLIC_dma_ch1_ERR_IRQHandler
#define     dma_ch2_DONE_IRQHandler  PLIC_dma_ch2_DONE_IRQHandler
#define     dma_ch2_ERR_IRQHandler   PLIC_dma_ch2_ERR_IRQHandler
#define     dma_ch3_DONE_IRQHandler  PLIC_dma_ch3_DONE_IRQHandler
#define     dma_ch3_ERR_IRQHandler   PLIC_dma_ch3_ERR_IRQHandler
#define     gpio0_bit0_or_gpio2_bit13_plic_0_IRQHandler  PLIC_gpio0_bit0_or_gpio2_bit13_IRQHandler
#define     gpio0_bit1_or_gpio2_bit13_plic_1_IRQHandler  PLIC_gpio0_bit1_or_gpio2_bit13_IRQHandler
#define     gpio0_bit2_or_gpio2_bit13_plic_2_IRQHandler  PLIC_gpio0_bit2_or_gpio2_bit13_IRQHandler
#define     gpio0_bit3_or_gpio2_bit13_plic_3_IRQHandler  PLIC_gpio0_bit3_or_gpio2_bit13_IRQHandler
#define     gpio0_bit4_or_gpio2_bit13_plic_4_IRQHandler  PLIC_gpio0_bit4_or_gpio2_bit13_IRQHandler
#define     gpio0_bit5_or_gpio2_bit13_plic_5_IRQHandler  PLIC_gpio0_bit5_or_gpio2_bit13_IRQHandler
#define     gpio0_bit6_or_gpio2_bit13_plic_6_IRQHandler  PLIC_gpio0_bit6_or_gpio2_bit13_IRQHandler
#define     gpio0_bit7_or_gpio2_bit13_plic_7_IRQHandler  PLIC_gpio0_bit7_or_gpio2_bit13_IRQHandler
#define     gpio0_bit8_or_gpio2_bit13_plic_8_IRQHandler  PLIC_gpio0_bit8_or_gpio2_bit13_IRQHandler
#define     gpio0_bit9_or_gpio2_bit13_plic_9_IRQHandler  PLIC_gpio0_bit9_or_gpio2_bit13_IRQHandler
#define     gpio0_bit10_or_gpio2_bit13_plic_10_IRQHandler    PLIC_gpio0_bit10_or_gpio2_bit13_IRQHandler
#define     gpio0_bit11_or_gpio2_bit13_plic_11_IRQHandler    PLIC_gpio0_bit11_or_gpio2_bit13_IRQHandler
#define     gpio0_bit12_or_gpio2_bit13_plic_12_IRQHandler    PLIC_gpio0_bit12_or_gpio2_bit13_IRQHandler

#define     gpio0_bit13_or_gpio2_bit13_plic_13_IRQHandler    PLIC_gpio0_bit13_or_gpio2_bit13_IRQHandler
#define     gpio1_bit0_or_gpio2_bit14_plic_14_IRQHandler PLIC_gpio1_bit0_or_gpio2_bit14_IRQHandler
#define     gpio1_bit1_or_gpio2_bit15_plic_15_IRQHandler PLIC_gpio1_bit1_or_gpio2_bit15_IRQHandler
#define     gpio1_bit2_or_gpio2_bit16_plic_16_IRQHandler PLIC_gpio1_bit2_or_gpio2_bit16_IRQHandler
#define     gpio1_bit3_or_gpio2_bit17_plic_17_IRQHandler PLIC_gpio1_bit3_or_gpio2_bit17_IRQHandler
#define     gpio1_bit4_or_gpio2_bit18_plic_18_IRQHandler PLIC_gpio1_bit4_or_gpio2_bit18_IRQHandler
#define     gpio1_bit5_or_gpio2_bit19_plic_19_IRQHandler PLIC_gpio1_bit5_or_gpio2_bit19_IRQHandler
#define     gpio1_bit6_or_gpio2_bit20_plic_20_IRQHandler PLIC_gpio1_bit6_or_gpio2_bit20_IRQHandler
#define     gpio1_bit7_or_gpio2_bit21_plic_21_IRQHandler PLIC_gpio1_bit7_or_gpio2_bit21_IRQHandler
#define     gpio1_bit8_or_gpio2_bit22_plic_22_IRQHandler PLIC_gpio1_bit8_or_gpio2_bit22_IRQHandler
#define     gpio1_bit9_or_gpio2_bit23_plic_23_IRQHandler PLIC_gpio1_bit9_or_gpio2_bit23_IRQHandler
#define     gpio1_bit10_or_gpio2_bit24_plic_24_IRQHandler    PLIC_gpio1_bit10_or_gpio2_bit24_IRQHandler
#define     gpio1_bit11_or_gpio2_bit25_plic_25_IRQHandler    PLIC_gpio1_bit11_or_gpio2_bit25_IRQHandler
#define     gpio1_bit12_or_gpio2_bit26_plic_26_IRQHandler    PLIC_gpio1_bit12_or_gpio2_bit26_IRQHandler
#define     gpio1_bit13_or_gpio2_bit27_plic_27_IRQHandler    PLIC_gpio1_bit13_or_gpio2_bit27_IRQHandler

#define     gpio1_bit14_or_gpio2_bit28_plic_28_IRQHandler    PLIC_gpio1_bit14_or_gpio2_bit28_IRQHandler
#define     gpio1_bit15_or_gpio2_bit29_plic_29_IRQHandler    PLIC_gpio1_bit15_or_gpio2_bit29_IRQHandler
#define     gpio1_bit16_or_gpio2_bit30_plic_30_IRQHandler    PLIC_gpio1_bit16_or_gpio2_bit30_IRQHandler
#define     gpio1_bit17_or_gpio2_bit31_plic_31_IRQHandler    PLIC_gpio1_bit17_or_gpio2_bit31_IRQHandler

#define     gpio1_bit18_plic_32_IRQHandler   PLIC_gpio1_bit18_IRQHandler
#define     gpio1_bit19_plic_33_IRQHandler   PLIC_gpio1_bit19_IRQHandler
#define     gpio1_bit20_plic_34_IRQHandler   PLIC_gpio1_bit20_IRQHandler
#define     gpio1_bit21_plic_35_IRQHandler   PLIC_gpio1_bit21_IRQHandler
#define     gpio1_bit22_plic_36_IRQHandler   PLIC_gpio1_bit22_IRQHandler
#define     gpio1_bit23_plic_37_IRQHandler   PLIC_gpio1_bit23_IRQHandler

#define     gpio0_non_direct_plic_IRQHandler PLIC_gpio0_non_direct_IRQHandler
#define     gpio1_non_direct_plic_IRQHandler PLIC_gpio1_non_direct_IRQHandler
#define     gpio2_non_direct_plic_IRQHandler PLIC_gpio2_non_direct_IRQHandler

#define     spi0_plic_IRQHandler PLIC_spi0_IRQHandler
#define     spi1_plic_IRQHandler PLIC_spi1_IRQHandler
#define     external_can0_plic_IRQHandler    PLIC_external_can0_IRQHandler
#define     can1_IRQHandler  PLIC_can1_IRQHandler
#define     External_i2c0_main_plic_IRQHandler   PLIC_External_i2c0_main_IRQHandler
#define     External_i2c0_alert_plic_IRQHandler  PLIC_External_i2c0_alert_IRQHandler
#define     i2c0_sus_plic_IRQHandler PLIC_i2c0_sus_IRQHandler
#define     i2c1_main_plic_IRQHandler    PLIC_i2c1_main_IRQHandler
#define     i2c1_alert_plic_IRQHandler   PLIC_i2c1_alert_IRQHandler
#define     i2c1_sus_plic_IRQHandler PLIC_i2c1_sus_IRQHandler
#define     mac0_int_plic_IRQHandler PLIC_mac0_int_IRQHandler
#define     mac0_queue1_plic_IRQHandler  PLIC_mac0_queue1_IRQHandler
#define     mac0_queue2_plic_IRQHandler  PLIC_mac0_queue2_IRQHandler
#define     mac0_queue3_plic_IRQHandler  PLIC_mac0_queue3_IRQHandler
#define     mac0_emac_plic_IRQHandler    PLIC_mac0_emac_IRQHandler
#define     mac0_mmsl_plic_IRQHandler    PLIC_mac0_mmsl_IRQHandler
#define     mac1_int_plic_IRQHandler PLIC_mac1_int_IRQHandler
#define     mac1_queue1_plic_IRQHandler  PLIC_mac1_queue1_IRQHandler
#define     mac1_queue2_plic_IRQHandler  PLIC_mac1_queue2_IRQHandler
#define     mac1_queue3_plic_IRQHandler  PLIC_mac1_queue3_IRQHandler
#define     mac1_emac_plic_IRQHandler    PLIC_mac1_emac_IRQHandler
#define     mac1_mmsl_plic_IRQHandler    PLIC_mac1_mmsl_IRQHandler
#define     ddrc_train_plic_IRQHandler   PLIC_ddrc_train_IRQHandler
#define     scb_interrupt_plic_IRQHandler    PLIC_scb_interrupt_IRQHandler
#define     ecc_error_plic_IRQHandler    PLIC_ecc_error_IRQHandler
#define     ecc_correct_plic_IRQHandler  PLIC_ecc_correct_IRQHandler
#define     rtc_wakeup_plic_IRQHandler   PLIC_rtc_wakeup_IRQHandler
#define     rtc_match_plic_IRQHandler    PLIC_rtc_match_IRQHandler
#define     timer1_plic_IRQHandler   PLIC_timer1_IRQHandler
#define     timer2_plic_IRQHandler   PLIC_timer2_IRQHandler
#define     envm_plic_IRQHandler PLIC_envm_IRQHandler
#define     qspi_plic_IRQHandler PLIC_qspi_IRQHandler
#define     usb_dma_plic_IRQHandler  PLIC_usb_dma_IRQHandler
#define     usb_mc_plic_IRQHandler   PLIC_usb_mc_IRQHandler
#define     mmc_main_plic_IRQHandler PLIC_mmc_main_IRQHandler
#define     mmc_wakeup_plic_IRQHandler   PLIC_mmc_wakeup_IRQHandler
#define     mmuart0_plic_77_IRQHandler   PLIC_mmuart0_IRQHandler
#define     mmuart1_plic_IRQHandler  PLIC_mmuart1_IRQHandler
#define     mmuart2_plic_IRQHandler  PLIC_mmuart2_IRQHandler
#define     mmuart3_plic_IRQHandler  PLIC_mmuart3_IRQHandler
#define     mmuart4_plic_IRQHandler  PLIC_mmuart4_IRQHandler
#define     g5c_devrst_plic_IRQHandler   PLIC_devrst_IRQHandler
#define     g5c_message_plic_IRQHandler  PLIC_g5c_message_IRQHandler
#define     usoc_vc_interrupt_plic_IRQHandler    PLIC_usoc_vc_interrupt_IRQHandler
#define     usoc_smb_interrupt_plic_IRQHandler   PLIC_usoc_smb_interrupt_IRQHandler
#define     e51_0_Maintence_plic_IRQHandler  PLIC_E51_Maintence_IRQHandler

#define     wdog0_mvrp_plic_IRQHandler   PLIC_wdog0_mvrp_IRQHandler
#define     wdog1_mvrp_plic_IRQHandler   PLIC_wdog1_mvrp_IRQHandler
#define     wdog2_mvrp_plic_IRQHandler   PLIC_wdog2_mvrp_IRQHandler
#define     wdog3_mvrp_plic_IRQHandler   PLIC_wdog3_mvrp_IRQHandler
#define     wdog4_mvrp_plic_IRQHandler   PLIC_wdog4_mvrp_IRQHandler
#define     wdog0_tout_plic_IRQHandler   PLIC_wdog0_tout_IRQHandler
#define     wdog1_tout_plic_IRQHandler   PLIC_wdog1_tout_IRQHandler
#define     wdog2_tout_plic_IRQHandler   PLIC_wdog2_tout_IRQHandler
#define     wdog3_tout_plic_IRQHandler   PLIC_wdog3_tout_IRQHandler
#define     wdog4_tout_plic_IRQHandler   PLIC_wdog4_tout_IRQHandler
#define     g5c_mss_spi_plic_IRQHandler  PLIC_g5c_mss_spi_IRQHandler
#define     volt_temp_alarm_plic_IRQHandler  PLIC_volt_temp_alarm_IRQHandler

#define     athena_complete_plic_IRQHandler  PLIC_athena_complete_IRQHandler
#define     athena_alarm_plic_IRQHandler PLIC_athena_alarm_IRQHandler
#define     athena_bus_error_plic_IRQHandler PLIC_athena_bus_error_IRQHandler
#define     usoc_axic_us_plic_IRQHandler PLIC_usoc_axic_us_IRQHandler
#define     usoc_axic_ds_plic_IRQHandler PLIC_usoc_axic_ds_IRQHandler

#define     reserved_104_plic_IRQHandler PLIC_reserved_104_IRQHandler

#define     fabric_f2h_0_plic_IRQHandler    PLIC_f2m_0_IRQHandler
#define     fabric_f2h_1_plic_IRQHandler    PLIC_f2m_1_IRQHandler
#define     fabric_f2h_2_plic_IRQHandler    PLIC_f2m_2_IRQHandler
#define     fabric_f2h_3_plic_IRQHandler    PLIC_f2m_3_IRQHandler
#define     fabric_f2h_4_plic_IRQHandler    PLIC_f2m_4_IRQHandler
#define     fabric_f2h_5_plic_IRQHandler    PLIC_f2m_5_IRQHandler
#define     fabric_f2h_6_plic_IRQHandler    PLIC_f2m_6_IRQHandler
#define     fabric_f2h_7_plic_IRQHandler    PLIC_f2m_7_IRQHandler
#define     fabric_f2h_8_plic_IRQHandler    PLIC_f2m_8_IRQHandler
#define     fabric_f2h_9_plic_IRQHandler    PLIC_f2m_9_IRQHandler

#define     fabric_f2h_10_plic_IRQHandler   PLIC_f2m_10_IRQHandler
#define     fabric_f2h_11_plic_IRQHandler   PLIC_f2m_11_IRQHandler
#define     fabric_f2h_12_plic_IRQHandler   PLIC_f2m_12_IRQHandler
#define     fabric_f2h_13_plic_IRQHandler   PLIC_f2m_13_IRQHandler
#define     fabric_f2h_14_plic_IRQHandler   PLIC_f2m_14_IRQHandler
#define     fabric_f2h_15_plic_IRQHandler   PLIC_f2m_15_IRQHandler
#define     fabric_f2h_16_plic_IRQHandler   PLIC_f2m_16_IRQHandler
#define     fabric_f2h_17_plic_IRQHandler   PLIC_f2m_17_IRQHandler
#define     fabric_f2h_18_plic_IRQHandler   PLIC_f2m_18_IRQHandler
#define     fabric_f2h_19_plic_IRQHandler   PLIC_f2m_19_IRQHandler

#define     fabric_f2h_20_plic_IRQHandler   PLIC_f2m_20_IRQHandler
#define     fabric_f2h_21_plic_IRQHandler   PLIC_f2m_21_IRQHandler
#define     fabric_f2h_22_plic_IRQHandler   PLIC_f2m_22_IRQHandler
#define     fabric_f2h_23_plic_IRQHandler   PLIC_f2m_23_IRQHandler
#define     fabric_f2h_24_plic_IRQHandler   PLIC_f2m_24_IRQHandler
#define     fabric_f2h_25_plic_IRQHandler   PLIC_f2m_25_IRQHandler
#define     fabric_f2h_26_plic_IRQHandler   PLIC_f2m_26_IRQHandler
#define     fabric_f2h_27_plic_IRQHandler   PLIC_f2m_27_IRQHandler
#define     fabric_f2h_28_plic_IRQHandler   PLIC_f2m_28_IRQHandler
#define     fabric_f2h_29_plic_IRQHandler   PLIC_f2m_29_IRQHandler

#define     fabric_f2h_30_plic_IRQHandler   PLIC_f2m_30_IRQHandler
#define     fabric_f2h_31_plic_IRQHandler   PLIC_f2m_31_IRQHandler

#define     fabric_f2h_32_plic_IRQHandler   PLIC_f2m_32_IRQHandler
#define     fabric_f2h_33_plic_IRQHandler   PLIC_f2m_33_IRQHandler
#define     fabric_f2h_34_plic_IRQHandler   PLIC_f2m_34_IRQHandler
#define     fabric_f2h_35_plic_IRQHandler   PLIC_f2m_35_IRQHandler
#define     fabric_f2h_36_plic_IRQHandler   PLIC_f2m_36_IRQHandler
#define     fabric_f2h_37_plic_IRQHandler   PLIC_f2m_37_IRQHandler
#define     fabric_f2h_38_plic_IRQHandler   PLIC_f2m_38_IRQHandler
#define     fabric_f2h_39_plic_IRQHandler   PLIC_f2m_39_IRQHandler
#define     fabric_f2h_40_plic_IRQHandler   PLIC_f2m_40_IRQHandler
#define     fabric_f2h_41_plic_IRQHandler   PLIC_f2m_41_IRQHandler

#define     fabric_f2h_42_plic_IRQHandler   PLIC_f2m_42_IRQHandler
#define     fabric_f2h_43_plic_IRQHandler   PLIC_f2m_43_IRQHandler
#define     fabric_f2h_44_plic_IRQHandler   PLIC_f2m_44_IRQHandler
#define     fabric_f2h_45_plic_IRQHandler   PLIC_f2m_45_IRQHandler
#define     fabric_f2h_46_plic_IRQHandler   PLIC_f2m_46_IRQHandler
#define     fabric_f2h_47_plic_IRQHandler   PLIC_f2m_47_IRQHandler
#define     fabric_f2h_48_plic_IRQHandler   PLIC_f2m_48_IRQHandler
#define     fabric_f2h_49_plic_IRQHandler   PLIC_f2m_49_IRQHandler
#define     fabric_f2h_50_plic_IRQHandler   PLIC_f2m_50_IRQHandler
#define     fabric_f2h_51_plic_IRQHandler   PLIC_f2m_51_IRQHandler

#define     fabric_f2h_52_plic_IRQHandler   PLIC_f2m_52_IRQHandler
#define     fabric_f2h_53_plic_IRQHandler   PLIC_f2m_53_IRQHandler
#define     fabric_f2h_54_plic_IRQHandler   PLIC_f2m_54_IRQHandler
#define     fabric_f2h_55_plic_IRQHandler   PLIC_f2m_55_IRQHandler
#define     fabric_f2h_56_plic_IRQHandler   PLIC_f2m_56_IRQHandler
#define     fabric_f2h_57_plic_IRQHandler   PLIC_f2m_57_IRQHandler
#define     fabric_f2h_58_plic_IRQHandler   PLIC_f2m_58_IRQHandler
#define     fabric_f2h_59_plic_IRQHandler   PLIC_f2m_59_IRQHandler
#define     fabric_f2h_60_plic_IRQHandler   PLIC_f2m_60_IRQHandler
#define     fabric_f2h_61_plic_IRQHandler   PLIC_f2m_61_IRQHandler

#define     fabric_f2h_62_plic_IRQHandler   PLIC_f2m_62_IRQHandler
#define     fabric_f2h_63_plic_IRQHandler   PLIC_f2m_63_IRQHandler

#define     bus_error_unit_hart_0_plic_IRQHandler    PLIC_E51_bus_error_unit_IRQHandler
#define     bus_error_unit_hart_1_plic_IRQHandler    PLIC_U54_1_bus_error_unit_IRQHandler
#define     bus_error_unit_hart_2_plic_IRQHandler    PLIC_U54_2_bus_error_unit_IRQHandler
#define     bus_error_unit_hart_3_plic_IRQHandler    PLIC_U54_3_bus_error_unit_IRQHandler
#define     bus_error_unit_hart_4_plic_IRQHandler    PLIC_U54_4_bus_error_unit_IRQHandler

#define     INVALID_IRQn                        PLIC_INVALID_INT_OFFSET
#define     L2_METADATA_CORR_IRQn               PLIC_L2_METADATA_CORR_INT_OFFSET
#define     L2_METADAT_UNCORR_IRQn              PLIC_L2_METADAT_UNCORR_INT_OFFSET
#define     L2_DATA_CORR_IRQn                   PLIC_L2_DATA_CORR_INT_OFFSET
#define     L2_DATA_UNCORR_IRQn                 PLIC_L2_DATA_UNCORR_INT_OFFSET
#define     DMA_CH0_DONE_IRQn                   PLIC_DMA_CH0_DONE_INT_OFFSET
#define     DMA_CH0_ERR_IRQn                    PLIC_DMA_CH0_ERR_INT_OFFSET
#define     DMA_CH1_DONE_IRQn                   PLIC_DMA_CH1_DONE_INT_OFFSET
#define     DMA_CH1_ERR_IRQn                    PLIC_DMA_CH1_ERR_INT_OFFSET
#define     DMA_CH2_DONE_IRQn                   PLIC_DMA_CH2_DONE_INT_OFFSET
#define     DMA_CH2_ERR_IRQn                    PLIC_DMA_CH2_ERR_INT_OFFSET
#define     DMA_CH3_DONE_IRQn                   PLIC_DMA_CH3_DONE_INT_OFFSET
#define     DMA_CH3_ERR_IRQn                    PLIC_DMA_CH3_ERR_INT_OFFSET

#define     GPIO0_BIT0_or_GPIO2_BIT0_PLIC_0                 PLIC_GPIO0_BIT0_or_GPIO2_BIT0_INT_OFFSET
#define     GPIO0_BIT1_or_GPIO2_BIT1_PLIC_1                 PLIC_GPIO0_BIT1_or_GPIO2_BIT1_INT_OFFSET
#define     GPIO0_BIT2_or_GPIO2_BIT2_PLIC_2                 PLIC_GPIO0_BIT2_or_GPIO2_BIT2_INT_OFFSET
#define     GPIO0_BIT3_or_GPIO2_BIT3_PLIC_3                 PLIC_GPIO0_BIT3_or_GPIO2_BIT3_INT_OFFSET
#define     GPIO0_BIT4_or_GPIO2_BIT4_PLIC_4                 PLIC_GPIO0_BIT4_or_GPIO2_BIT4_INT_OFFSET
#define     GPIO0_BIT5_or_GPIO2_BIT5_PLIC_5                 PLIC_GPIO0_BIT5_or_GPIO2_BIT5_INT_OFFSET
#define     GPIO0_BIT6_or_GPIO2_BIT6_PLIC_6                 PLIC_GPIO0_BIT6_or_GPIO2_BIT6_INT_OFFSET
#define     GPIO0_BIT7_or_GPIO2_BIT7_PLIC_7                 PLIC_GPIO0_BIT7_or_GPIO2_BIT7_INT_OFFSET
#define     GPIO0_BIT8_or_GPIO2_BIT8_PLIC_8                 PLIC_GPIO0_BIT8_or_GPIO2_BIT8_INT_OFFSET
#define     GPIO0_BIT9_or_GPIO2_BIT9_PLIC_9                 PLIC_GPIO0_BIT9_or_GPIO2_BIT9_INT_OFFSET
#define     GPIO0_BIT10_or_GPIO2_BIT10_PLIC_10              PLIC_GPIO0_BIT10_or_GPIO2_BIT10_INT_OFFSET
#define     GPIO0_BIT11_or_GPIO2_BIT11_PLIC_11              PLIC_GPIO0_BIT11_or_GPIO2_BIT11_INT_OFFSET
#define     GPIO0_BIT12_or_GPIO2_BIT12_PLIC_12              PLIC_GPIO0_BIT12_or_GPIO2_BIT12_INT_OFFSET

#define     GPIO0_BIT13_or_GPIO2_BIT13_PLIC_13              PLIC_GPIO0_BIT13_or_GPIO2_BIT13_INT_OFFSET
#define     GPIO1_BIT0_or_GPIO2_BIT14_PLIC_14               PLIC_GPIO1_BIT0_or_GPIO2_BIT14_INT_OFFSET
#define     GPIO1_BIT1_or_GPIO2_BIT15_PLIC_15               PLIC_GPIO1_BIT1_or_GPIO2_BIT15_INT_OFFSET
#define     GPIO1_BIT2_or_GPIO2_BIT16_PLIC_16               PLIC_GPIO1_BIT2_or_GPIO2_BIT16_INT_OFFSET
#define     GPIO1_BIT3_or_GPIO2_BIT17_PLIC_17               PLIC_GPIO1_BIT3_or_GPIO2_BIT17_INT_OFFSET
#define     GPIO1_BIT4_or_GPIO2_BIT18_PLIC_18               PLIC_GPIO1_BIT4_or_GPIO2_BIT18_INT_OFFSET
#define     GPIO1_BIT5_or_GPIO2_BIT19_PLIC_19               PLIC_GPIO1_BIT5_or_GPIO2_BIT19_INT_OFFSET
#define     GPIO1_BIT6_or_GPIO2_BIT20_PLIC_20               PLIC_GPIO1_BIT6_or_GPIO2_BIT20_INT_OFFSET
#define     GPIO1_BIT7_or_GPIO2_BIT21_PLIC_21               PLIC_GPIO1_BIT7_or_GPIO2_BIT21_INT_OFFSET
#define     GPIO1_BIT8_or_GPIO2_BIT22_PLIC_22               PLIC_GPIO1_BIT8_or_GPIO2_BIT22_INT_OFFSET
#define     GPIO1_BIT9_or_GPIO2_BIT23_PLIC_23               PLIC_GPIO1_BIT9_or_GPIO2_BIT23_INT_OFFSET
#define     GPIO1_BIT10_or_GPIO2_BIT24_PLIC_24              PLIC_GPIO1_BIT10_or_GPIO2_BIT24_INT_OFFSET
#define     GPIO1_BIT11_or_GPIO2_BIT25_PLIC_25              PLIC_GPIO1_BIT11_or_GPIO2_BIT25_INT_OFFSET
#define     GPIO1_BIT12_or_GPIO2_BIT26_PLIC_26              PLIC_GPIO1_BIT12_or_GPIO2_BIT26_INT_OFFSET
#define     GPIO1_BIT13_or_GPIO2_BIT27_PLIC_27              PLIC_GPIO1_BIT13_or_GPIO2_BIT27_INT_OFFSET

#define     GPIO1_BIT14_or_GPIO2_BIT28_PLIC_28              PLIC_GPIO1_BIT14_or_GPIO2_BIT28_INT_OFFSET
#define     GPIO1_BIT15_or_GPIO2_BIT29_PLIC_29              PLIC_GPIO1_BIT15_or_GPIO2_BIT29_INT_OFFSET
#define     GPIO1_BIT16_or_GPIO2_BIT30_PLIC_30              PLIC_GPIO1_BIT16_or_GPIO2_BIT30_INT_OFFSET
#define     GPIO1_BIT17_or_GPIO2_BIT31_PLIC_31              PLIC_GPIO1_BIT17_or_GPIO2_BIT31_INT_OFFSET

#define     GPIO1_BIT18_PLIC_32                 PLIC_GPIO1_BIT18_INT_OFFSET
#define     GPIO1_BIT19_PLIC_33                 PLIC_GPIO1_BIT19_INT_OFFSET
#define     GPIO1_BIT20_PLIC_34                 PLIC_GPIO1_BIT20_INT_OFFSET
#define     GPIO1_BIT21_PLIC_35                 PLIC_GPIO1_BIT21_INT_OFFSET
#define     GPIO1_BIT22_PLIC_36                 PLIC_GPIO1_BIT22_INT_OFFSET
#define     GPIO1_BIT23_PLIC_37                 PLIC_GPIO1_BIT23_INT_OFFSET

#define     GPIO0_NON_DIRECT_PLIC               PLIC_GPIO0_NON_DIRECT_INT_OFFSET
#define     GPIO1_NON_DIRECT_PLIC               PLIC_GPIO1_NON_DIRECT_INT_OFFSET
#define     GPIO2_NON_DIRECT_PLIC               PLIC_GPIO2_NON_DIRECT_INT_OFFSET

#define     SPI0_PLIC                           PLIC_SPI0_INT_OFFSET
#define     SPI1_PLIC                           PLIC_SPI1_INT_OFFSET
#define     CAN0_PLIC                           PLIC_CAN0_INT_OFFSET
#define     CAN1_PLIC                           PLIC_CAN1_INT_OFFSET
#define     I2C0_MAIN_PLIC                      PLIC_I2C0_MAIN_INT_OFFSET
#define     I2C0_ALERT_PLIC                     PLIC_I2C0_ALERT_INT_OFFSET
#define     I2C0_SUS_PLIC                       PLIC_I2C0_SUS_INT_OFFSET
#define     I2C1_MAIN_PLIC                      PLIC_I2C1_MAIN_INT_OFFSET
#define     I2C1_ALERT_PLIC                     PLIC_I2C1_ALERT_INT_OFFSET
#define     I2C1_SUS_PLIC                       PLIC_I2C1_SUS_INT_OFFSET
#define     MAC0_INT_PLIC                       PLIC_MAC0_INT_INT_OFFSET
#define     MAC0_QUEUE1_PLIC                    PLIC_MAC0_QUEUE1_INT_OFFSET
#define     MAC0_QUEUE2_PLIC                    PLIC_MAC0_QUEUE2_INT_OFFSET
#define     MAC0_QUEUE3_PLIC                    PLIC_MAC0_QUEUE3_INT_OFFSET
#define     MAC0_EMAC_PLIC                      PLIC_MAC0_EMAC_INT_OFFSET
#define     MAC0_MMSL_PLIC                      PLIC_MAC0_MMSL_INT_OFFSET
#define     MAC1_INT_PLIC                       PLIC_MAC1_INT_INT_OFFSET
#define     MAC1_QUEUE1_PLIC                    PLIC_MAC1_QUEUE1_INT_OFFSET
#define     MAC1_QUEUE2_PLIC                    PLIC_MAC1_QUEUE2_INT_OFFSET
#define     MAC1_QUEUE3_PLIC                    PLIC_MAC1_QUEUE3_INT_OFFSET
#define     MAC1_EMAC_PLIC                      PLIC_MAC1_EMAC_INT_OFFSET
#define     MAC1_MMSL_PLIC                      PLIC_MAC1_MMSL_INT_OFFSET
#define     DDRC_TRAIN_PLIC                     PLIC_DDRC_TRAIN_INT_OFFSET
#define     SCB_INTERRUPT_PLIC                  PLIC_SCB_INTERRUPT_INT_OFFSET
#define     ECC_ERROR_PLIC                      PLIC_ECC_ERROR_INT_OFFSET
#define     ECC_CORRECT_PLIC                    PLIC_ECC_CORRECT_INT_OFFSET
#define     RTC_WAKEUP_PLIC                     PLIC_RTC_WAKEUP_INT_OFFSET
#define     RTC_MATCH_PLIC                      PLIC_RTC_MATCH_INT_OFFSET
#define     TIMER1_PLIC                         PLIC_TIMER1_INT_OFFSET
#define     TIMER2_PLIC                         PLIC_TIMER2_INT_OFFSET
#define     ENVM_PLIC                           PLIC_ENVM_INT_OFFSET
#define     QSPI_PLIC                           PLIC_QSPI_INT_OFFSET
#define     USB_DMA_PLIC                        PLIC_USB_DMA_INT_OFFSET
#define     USB_MC_PLIC                         PLIC_USB_MC_INT_OFFSET
#define     MMC_main_PLIC                       PLIC_MMC_main_INT_OFFSET
#define     MMC_wakeup_PLIC                     PLIC_MMC_wakeup_INT_OFFSET
#define     MMUART0_PLIC_77                     PLIC_MMUART0_INT_OFFSET
#define     MMUART1_PLIC                        PLIC_MMUART1_INT_OFFSET
#define     MMUART2_PLIC                        PLIC_MMUART2_INT_OFFSET
#define     MMUART3_PLIC                        PLIC_MMUART3_INT_OFFSET
#define     MMUART4_PLIC                        PLIC_MMUART4_INT_OFFSET

#define     G5C_DEVRST_PLIC                     G5C_DEVRST_INT_OFFSET
#define     g5c_MESSAGE_PLIC                    g5c_MESSAGE_INT_OFFSET
#define     USOC_VC_INTERRUPT_PLIC              USOC_VC_INTERRUPT_INT_OFFSET
#define     USOC_SMB_INTERRUPT_PLIC             USOC_SMB_INTERRUPT_INT_OFFSET

#define     E51_0_MAINTENACE_PLIC               E51_0_MAINTENACE_INT_OFFSET

#define     WDOG0_MRVP_PLIC                     PLIC_WDOG0_MRVP_INT_OFFSET
#define     WDOG1_MRVP_PLIC                     PLIC_WDOG1_MRVP_INT_OFFSET
#define     WDOG2_MRVP_PLIC                     PLIC_WDOG2_MRVP_INT_OFFSET
#define     WDOG3_MRVP_PLIC                     PLIC_WDOG3_MRVP_INT_OFFSET
#define     WDOG4_MRVP_PLIC                     PLIC_WDOG4_MRVP_INT_OFFSET
#define     WDOG0_TOUT_PLIC                     PLIC_WDOG0_TOUT_INT_OFFSET
#define     WDOG1_TOUT_PLIC                     PLIC_WDOG1_TOUT_INT_OFFSET
#define     WDOG2_TOUT_PLIC                     PLIC_WDOG2_TOUT_INT_OFFSET
#define     WDOG3_TOUT_PLIC                     PLIC_WDOG3_TOUT_INT_OFFSET
#define     WDOG4_TOUT_PLIC                     PLIC_WDOG4_TOUT_INT_OFFSET
#define     G5C_MSS_SPI_PLIC                    G5C_MSS_SPI_INT_OFFSET
#define     VOLT_TEMP_ALARM_PLIC                VOLT_TEMP_ALARM_INT_OFFSET
#define     ATHENA_COMPLETE_PLIC                ATHENA_COMPLETE_INT_OFFSET
#define     ATHENA_ALARM_PLIC                   ATHENA_ALARM_INT_OFFSET
#define     ATHENA_BUS_ERROR_PLIC               ATHENA_BUS_ERROR_INT_OFFSET
#define     USOC_AXIC_US_PLIC                   USOC_AXIC_US_INT_OFFSET
#define     USOC_AXIC_DS_PLIC                   USOC_AXIC_DS_INT_OFFSET

#define     FABRIC_F2H_0_PLIC                   PLIC_F2M_0_INT_OFFSET
#define     FABRIC_F2H_1_PLIC                   PLIC_F2M_1_INT_OFFSET
#define     FABRIC_F2H_2_PLIC                   PLIC_F2M_2_INT_OFFSET
#define     FABRIC_F2H_3_PLIC                   PLIC_F2M_3_INT_OFFSET
#define     FABRIC_F2H_4_PLIC                   PLIC_F2M_4_INT_OFFSET
#define     FABRIC_F2H_5_PLIC                   PLIC_F2M_5_INT_OFFSET
#define     FABRIC_F2H_6_PLIC                   PLIC_F2M_6_INT_OFFSET
#define     FABRIC_F2H_7_PLIC                   PLIC_F2M_7_INT_OFFSET
#define     FABRIC_F2H_8_PLIC                   PLIC_F2M_8_INT_OFFSET
#define     FABRIC_F2H_9_PLIC                   PLIC_F2M_9_INT_OFFSET

#define     FABRIC_F2H_10_PLIC                  PLIC_F2M_10_INT_OFFSET
#define     FABRIC_F2H_11_PLIC                  PLIC_F2M_11_INT_OFFSET
#define     FABRIC_F2H_12_PLIC                  PLIC_F2M_12_INT_OFFSET
#define     FABRIC_F2H_13_PLIC                  PLIC_F2M_13_INT_OFFSET
#define     FABRIC_F2H_14_PLIC                  PLIC_F2M_14_INT_OFFSET
#define     FABRIC_F2H_15_PLIC                  PLIC_F2M_15_INT_OFFSET
#define     FABRIC_F2H_16_PLIC                  PLIC_F2M_16_INT_OFFSET
#define     FABRIC_F2H_17_PLIC                  PLIC_F2M_17_INT_OFFSET
#define     FABRIC_F2H_18_PLIC                  PLIC_F2M_18_INT_OFFSET
#define     FABRIC_F2H_19_PLIC                  PLIC_F2M_19_INT_OFFSET

#define     FABRIC_F2H_20_PLIC                  PLIC_F2M_20_INT_OFFSET
#define     FABRIC_F2H_21_PLIC                  PLIC_F2M_21_INT_OFFSET
#define     FABRIC_F2H_22_PLIC                  PLIC_F2M_22_INT_OFFSET
#define     FABRIC_F2H_23_PLIC                  PLIC_F2M_23_INT_OFFSET
#define     FABRIC_F2H_24_PLIC                  PLIC_F2M_24_INT_OFFSET
#define     FABRIC_F2H_25_PLIC                  PLIC_F2M_25_INT_OFFSET
#define     FABRIC_F2H_26_PLIC                  PLIC_F2M_26_INT_OFFSET
#define     FABRIC_F2H_27_PLIC                  PLIC_F2M_27_INT_OFFSET
#define     FABRIC_F2H_28_PLIC                  PLIC_F2M_28_INT_OFFSET
#define     FABRIC_F2H_29_PLIC                  PLIC_F2M_29_INT_OFFSET

#define     FABRIC_F2H_30_PLIC                  PLIC_F2M_30_INT_OFFSET
#define     FABRIC_F2H_31_PLIC                  PLIC_F2M_31_INT_OFFSET

#define     FABRIC_F2H_32_PLIC                  PLIC_F2M_32_INT_OFFSET
#define     FABRIC_F2H_33_PLIC                  PLIC_F2M_33_INT_OFFSET
#define     FABRIC_F2H_34_PLIC                  PLIC_F2M_34_INT_OFFSET
#define     FABRIC_F2H_35_PLIC                  PLIC_F2M_35_INT_OFFSET
#define     FABRIC_F2H_36_PLIC                  PLIC_F2M_36_INT_OFFSET
#define     FABRIC_F2H_37_PLIC                  PLIC_F2M_37_INT_OFFSET
#define     FABRIC_F2H_38_PLIC                  PLIC_F2M_38_INT_OFFSET
#define     FABRIC_F2H_39_PLIC                  PLIC_F2M_39_INT_OFFSET
#define     FABRIC_F2H_40_PLIC                  PLIC_F2M_40_INT_OFFSET
#define     FABRIC_F2H_41_PLIC                  PLIC_F2M_41_INT_OFFSET

#define     FABRIC_F2H_42_PLIC                  PLIC_F2M_42_INT_OFFSET
#define     FABRIC_F2H_43_PLIC                  PLIC_F2M_43_INT_OFFSET
#define     FABRIC_F2H_44_PLIC                  PLIC_F2M_44_INT_OFFSET
#define     FABRIC_F2H_45_PLIC                  PLIC_F2M_45_INT_OFFSET
#define     FABRIC_F2H_46_PLIC                  PLIC_F2M_46_INT_OFFSET
#define     FABRIC_F2H_47_PLIC                  PLIC_F2M_47_INT_OFFSET
#define     FABRIC_F2H_48_PLIC                  PLIC_F2M_48_INT_OFFSET
#define     FABRIC_F2H_49_PLIC                  PLIC_F2M_49_INT_OFFSET
#define     FABRIC_F2H_50_PLIC                  PLIC_F2M_50_INT_OFFSET
#define     FABRIC_F2H_51_PLIC                  PLIC_F2M_51_INT_OFFSET

#define     FABRIC_F2H_52_PLIC                  PLIC_F2M_52_INT_OFFSET
#define     FABRIC_F2H_53_PLIC                  PLIC_F2M_53_INT_OFFSET
#define     FABRIC_F2H_54_PLIC                  PLIC_F2M_54_INT_OFFSET
#define     FABRIC_F2H_55_PLIC                  PLIC_F2M_55_INT_OFFSET
#define     FABRIC_F2H_56_PLIC                  PLIC_F2M_56_INT_OFFSET
#define     FABRIC_F2H_57_PLIC                  PLIC_F2M_57_INT_OFFSET
#define     FABRIC_F2H_58_PLIC                  PLIC_F2M_58_INT_OFFSET
#define     FABRIC_F2H_59_PLIC                  PLIC_F2M_59_INT_OFFSET
#define     FABRIC_F2H_60_PLIC                  PLIC_F2M_60_INT_OFFSET
#define     FABRIC_F2H_61_PLIC                  PLIC_F2M_61_INT_OFFSET

#define     FABRIC_F2H_62_PLIC                  PLIC_F2M_62_INT_OFFSET
#define     FABRIC_F2H_63_PLIC                  PLIC_F2M_63_INT_OFFSET

#define     BUS_ERROR_UNIT_HART_0               PLIC_E51_BUS_ERROR_UNIT_OFFSET
#define     BUS_ERROR_UNIT_HART_1               PLIC_U54_1_BUS_ERROR_UNIT_OFFSET
#define     BUS_ERROR_UNIT_HART_2               PLIC_U54_2_BUS_ERROR_UNIT_OFFSET
#define     BUS_ERROR_UNIT_HART_3               PLIC_U54_3_BUS_ERROR_UNIT_OFFSET
#define     BUS_ERROR_UNIT_HART_4               PLIC_U54_4_BUS_ERROR_UNIT_OFFSET

#define     Software_h0_IRQHandler              E51_software_IRQHandler
#define     Software_h1_IRQHandler              U54_1_software_IRQHandler
#define     Software_h2_IRQHandler              U54_2_software_IRQHandler
#define     Software_h3_IRQHandler              U54_3_software_IRQHandler
#define     Software_h4_IRQHandler              U54_4_software_IRQHandler


#define     SysTick_Handler_h0_IRQHandler       E51_sysTick_IRQHandler
#define     SysTick_Handler_h1_IRQHandler       U54_1_sysTick_IRQHandler
#define     SysTick_Handler_h2_IRQHandler       U54_2_sysTick_IRQHandler
#define     SysTick_Handler_h3_IRQHandler       U54_3_sysTick_IRQHandler
#define     SysTick_Handler_h4_IRQHandler       U54_4_sysTick_IRQHandler


#ifdef __cplusplus
}
#endif

#endif  /* MSS_HART_INTS_H */

