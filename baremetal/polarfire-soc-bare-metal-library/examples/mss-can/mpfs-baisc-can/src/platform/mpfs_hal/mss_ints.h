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
 * @file mss_ints.h
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief MPFS interrupt prototypes
 *
 * Interrupt function prototypes
 *
 * SVN $Revision: 12097 $
 * SVN $Date: 2019-08-12 19:43:31 +0530 (Mon, 12 Aug 2019) $
 */

#ifndef MSS_INTS_H
#define MSS_INTS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void handle_m_ext_interrupt(void);
void Software_h0_IRQHandler(void);
void Software_h1_IRQHandler(void);
void Software_h2_IRQHandler(void);
void Software_h3_IRQHandler(void);
void Software_h4_IRQHandler(void);
void SysTick_Handler(uint32_t hard_id);

/*
 *
 *   Local interrupt stubs
 *
 */
void maintenance_e51_local_IRQHandler_0(void);
void usoc_smb_interrupt_e51_local_IRQHandler_1(void);
void usoc_vc_interrupt_e51_local_IRQHandler_2(void);
void g5c_message_e51_local_IRQHandler_3(void);
void g5c_devrst_e51_local_IRQHandler_4(void);
void wdog4_tout_e51_local_IRQHandler_5(void);
void wdog3_tout_e51_local_IRQHandler_6(void);
void wdog2_tout_e51_local_IRQHandler_7(void);
void wdog1_tout_e51_local_IRQHandler_8(void);
void wdog0_tout_e51_local_IRQHandler_9(void);
void wdog0_mvrp_e51_local_IRQHandler_10(void);
void mmuart0_e51_local_IRQHandler_11(void);
void envm_e51_local_IRQHandler_12(void);
void ecc_correct_e51_local_IRQHandler_13(void);
void ecc_error_E51_local_IRQHandler_14(void);
void scb_interrupt_E51_local_IRQHandler_15(void);
void fabric_f2h_32_E51_local_IRQHandler_16(void);
void fabric_f2h_33_E51_local_IRQHandler_17(void);
void fabric_f2h_34_E51_local_IRQHandler_18(void);
void fabric_f2h_35_E51_local_IRQHandler_19(void);
void fabric_f2h_36_E51_local_IRQHandler_20(void);
void fabric_f2h_37_E51_local_IRQHandler_21(void);
void fabric_f2h_38_E51_local_IRQHandler_22(void);
void fabric_f2h_39_E51_local_IRQHandler_23(void);
void fabric_f2h_40_E51_local_IRQHandler_24(void);
void fabric_f2h_41_E51_local_IRQHandler_25(void);
void fabric_f2h_42_E51_local_IRQHandler_26(void);
void fabric_f2h_43_E51_local_IRQHandler_27(void);
void fabric_f2h_44_E51_local_IRQHandler_28(void);
void fabric_f2h_45_E51_local_IRQHandler_29(void);
void fabric_f2h_46_E51_local_IRQHandler_30(void);
void fabric_f2h_47_E51_local_IRQHandler_31(void);
void fabric_f2h_48_E51_local_IRQHandler_32(void);
void fabric_f2h_49_E51_local_IRQHandler_33(void);
void fabric_f2h_50_E51_local_IRQHandler_34(void);
void fabric_f2h_51_E51_local_IRQHandler_35(void);
void fabric_f2h_52_E51_local_IRQHandler_36(void);
void fabric_f2h_53_E51_local_IRQHandler_37(void);
void fabric_f2h_54_E51_local_IRQHandler_38(void);
void fabric_f2h_55_E51_local_IRQHandler_39(void);
void fabric_f2h_56_E51_local_IRQHandler_40(void);
void fabric_f2h_57_E51_local_IRQHandler_41(void);
void fabric_f2h_58_E51_local_IRQHandler_42(void);
void fabric_f2h_59_E51_local_IRQHandler_43(void);
void fabric_f2h_60_E51_local_IRQHandler_44(void);
void fabric_f2h_61_E51_local_IRQHandler_45(void);
void fabric_f2h_62_E51_local_IRQHandler_46(void);
void fabric_f2h_63_E51_local_IRQHandler_47(void);

/*
 * U54
 */
void spare_u54_local_IRQHandler_0(void);
void spare_u54_local_IRQHandler_1(void);
void spare_u54_local_IRQHandler_2(void);

void mac_mmsl_u54_1_local_IRQHandler_3(void);
void mac_emac_u54_1_local_IRQHandler_4(void);
void mac_queue3_u54_1_local_IRQHandler_5(void);
void mac_queue2_u54_1_local_IRQHandler_6(void);
void mac_queue1_u54_1_local_IRQHandler_7(void);
void mac_int_u54_1_local_IRQHandler_8(void);

void mac_mmsl_u54_2_local_IRQHandler_3(void);
void mac_emac_u54_2_local_IRQHandler_4(void);
void mac_queue3_u54_2_local_IRQHandler_5(void);
void mac_queue2_u54_2_local_IRQHandler_6(void);
void mac_queue1_u54_2_local_IRQHandler_7(void);
void mac_int_u54_2_local_IRQHandler_8(void);

void mac_mmsl_u54_3_local_IRQHandler_3(void);
void mac_emac_u54_3_local_IRQHandler_4(void);
void mac_queue3_u54_3_local_IRQHandler_5(void);
void mac_queue2_u54_3_local_IRQHandler_6(void);
void mac_queue1_u54_3_local_IRQHandler_7(void);
void mac_int_u54_3_local_IRQHandler_8(void);

void mac_mmsl_u54_4_local_IRQHandler_3(void);
void mac_emac_u54_4_local_IRQHandler_4(void);
void mac_queue3_u54_4_local_IRQHandler_5(void);
void mac_queue2_u54_4_local_IRQHandler_6(void);
void mac_queue1_u54_4_local_IRQHandler_7(void);
void mac_int_u54_4_local_IRQHandler_8(void);

void wdog_tout_u54_h1_local_IRQHandler_9(void);
void wdog_tout_u54_h2_local_IRQHandler_9(void);
void wdog_tout_u54_h3_local_IRQHandler_9(void);
void wdog_tout_u54_h4_local_IRQHandler_9(void);
void mvrp_u54_local_IRQHandler_10(void);
void mmuart_u54_h1_local_IRQHandler_11(void);
void mmuart_u54_h2_local_IRQHandler_11(void);
void mmuart_u54_h3_local_IRQHandler_11(void);
void mmuart_u54_h4_local_IRQHandler_11(void);
void spare_u54_local_IRQHandler_12(void);
void spare_u54_local_IRQHandler_13(void);
void spare_u54_local_IRQHandler_14(void);
void spare_u54_local_IRQHandler_15(void);
void fabric_f2h_0_u54_local_IRQHandler_16(void);
void fabric_f2h_1_u54_local_IRQHandler_17(void);
void fabric_f2h_2_u54_local_IRQHandler_18(void);
void fabric_f2h_3_u54_local_IRQHandler_19(void);
void fabric_f2h_4_u54_local_IRQHandler_20(void);
void fabric_f2h_5_u54_local_IRQHandler_21(void);
void fabric_f2h_6_u54_local_IRQHandler_22(void);
void fabric_f2h_7_u54_local_IRQHandler_23(void);
void fabric_f2h_8_u54_local_IRQHandler_24(void);
void fabric_f2h_9_u54_local_IRQHandler_25(void);
void fabric_f2h_10_u54_local_IRQHandler_26(void);
void fabric_f2h_11_u54_local_IRQHandler_27(void);
void fabric_f2h_12_u54_local_IRQHandler_28(void);
void fabric_f2h_13_u54_local_IRQHandler_29(void);
void fabric_f2h_14_u54_local_IRQHandler_30(void);
void fabric_f2h_15_u54_local_IRQHandler_31(void);
void fabric_f2h_16_u54_local_IRQHandler_32(void);
void fabric_f2h_17_u54_local_IRQHandler_33(void);
void fabric_f2h_18_u54_local_IRQHandler_34(void);
void fabric_f2h_19_u54_local_IRQHandler_35(void);
void fabric_f2h_20_u54_local_IRQHandler_36(void);
void fabric_f2h_21_u54_local_IRQHandler_37(void);
void fabric_f2h_22_u54_local_IRQHandler_38(void);
void fabric_f2h_23_u54_local_IRQHandler_39(void);
void fabric_f2h_24_u54_local_IRQHandler_40(void);
void fabric_f2h_25_u54_local_IRQHandler_41(void);
void fabric_f2h_26_u54_local_IRQHandler_42(void);
void fabric_f2h_27_u54_local_IRQHandler_43(void);
void fabric_f2h_28_u54_local_IRQHandler_44(void);
void fabric_f2h_29_u54_local_IRQHandler_45(void);
void fabric_f2h_30_u54_local_IRQHandler_46(void);
void fabric_f2h_31_u54_local_IRQHandler_47(void);

#ifdef __cplusplus
}
#endif

#endif /* MSS_INTS_H */
