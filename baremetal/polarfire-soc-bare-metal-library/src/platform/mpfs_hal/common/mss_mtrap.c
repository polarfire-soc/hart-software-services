/*******************************************************************************
 * Copyright 2019 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * @file mss_mtrap.c
 * @author Microchip FPGA Embedded Systems Solutions
 * @brief trap functions
 *
 */

#include "mpfs_hal/mss_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void handle_local_interrupt(uint8_t interrupt_no);
void handle_m_soft_interrupt(void);
void handle_m_timer_interrupt(void);
void illegal_insn_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc);
void misaligned_store_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc);
void misaligned_load_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc);
void pmp_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc);
void trap_from_machine_mode(uintptr_t * regs, uintptr_t dummy, uintptr_t mepc);
void bad_trap(uintptr_t* regs, uintptr_t dummy, uintptr_t mepc);

void bad_trap(uintptr_t* regs, uintptr_t dummy, uintptr_t mepc)
{
    (void)regs;
    (void)dummy;
    (void)mepc;
    while(1)
    {
    }
}

void misaligned_store_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc)
{
    (void)regs;
    (void)mcause;
    (void)mepc;
    while(1)
    {
    }
}

void misaligned_load_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc)
{
    (void)regs;
    (void)mcause;
    (void)mepc;
    while(1)
    {
    }
}

void illegal_insn_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc)
{
    (void)regs;
    (void)mcause;
    (void)mepc;
    while(1)
    {
    }
}

void pmp_trap(uintptr_t * regs, uintptr_t mcause, uintptr_t mepc)
{
    (void)regs;
    (void)mcause;
    (void)mepc;
    while(1)
    {
    }
}

/*------------------------------------------------------------------------------
 * RISC-V interrupt handler for external interrupts.
 */
uint8_t (*ext_irq_handler_table[PLIC_NUM_SOURCES])(void) =
{
  PLIC_Invalid_IRQHandler,
  PLIC_l2_metadata_corr_IRQHandler,
  PLIC_l2_metadata_uncorr_IRQHandler,
  PLIC_l2_data_corr_IRQHandler,
  PLIC_l2_data_uncorr_IRQHandler,
  PLIC_dma_ch0_DONE_IRQHandler,
  PLIC_dma_ch0_ERR_IRQHandler,
  PLIC_dma_ch1_DONE_IRQHandler,
  PLIC_dma_ch1_ERR_IRQHandler,
  PLIC_dma_ch2_DONE_IRQHandler,
  PLIC_dma_ch2_ERR_IRQHandler,
  PLIC_dma_ch3_DONE_IRQHandler,
  PLIC_dma_ch3_ERR_IRQHandler,
  PLIC_gpio0_bit0_or_gpio2_bit13_IRQHandler,
  PLIC_gpio0_bit1_or_gpio2_bit13_IRQHandler,
  PLIC_gpio0_bit2_or_gpio2_bit13_IRQHandler,
  PLIC_gpio0_bit3_or_gpio2_bit13_IRQHandler,
  PLIC_gpio0_bit4_or_gpio2_bit13_IRQHandler,
  PLIC_gpio0_bit5_or_gpio2_bit13_IRQHandler,
  PLIC_gpio0_bit6_or_gpio2_bit13_IRQHandler,
  PLIC_gpio0_bit7_or_gpio2_bit13_IRQHandler,
  PLIC_gpio0_bit8_or_gpio2_bit13_IRQHandler,
  PLIC_gpio0_bit9_or_gpio2_bit13_IRQHandler,
  PLIC_gpio0_bit10_or_gpio2_bit13_IRQHandler,
  PLIC_gpio0_bit11_or_gpio2_bit13_IRQHandler,
  PLIC_gpio0_bit12_or_gpio2_bit13_IRQHandler,

  PLIC_gpio0_bit13_or_gpio2_bit13_IRQHandler,
  PLIC_gpio1_bit0_or_gpio2_bit14_IRQHandler,
  PLIC_gpio1_bit1_or_gpio2_bit15_IRQHandler,
  PLIC_gpio1_bit2_or_gpio2_bit16_IRQHandler,
  PLIC_gpio1_bit3_or_gpio2_bit17_IRQHandler,
  PLIC_gpio1_bit4_or_gpio2_bit18_IRQHandler,
  PLIC_gpio1_bit5_or_gpio2_bit19_IRQHandler,
  PLIC_gpio1_bit6_or_gpio2_bit20_IRQHandler,
  PLIC_gpio1_bit7_or_gpio2_bit21_IRQHandler,
  PLIC_gpio1_bit8_or_gpio2_bit22_IRQHandler,
  PLIC_gpio1_bit9_or_gpio2_bit23_IRQHandler,
  PLIC_gpio1_bit10_or_gpio2_bit24_IRQHandler,
  PLIC_gpio1_bit11_or_gpio2_bit25_IRQHandler,
  PLIC_gpio1_bit12_or_gpio2_bit26_IRQHandler,
  PLIC_gpio1_bit13_or_gpio2_bit27_IRQHandler,

  PLIC_gpio1_bit14_or_gpio2_bit28_IRQHandler,
  PLIC_gpio1_bit15_or_gpio2_bit29_IRQHandler,
  PLIC_gpio1_bit16_or_gpio2_bit30_IRQHandler,
  PLIC_gpio1_bit17_or_gpio2_bit31_IRQHandler,

  PLIC_gpio1_bit18_IRQHandler,
  PLIC_gpio1_bit19_IRQHandler,
  PLIC_gpio1_bit20_IRQHandler,
  PLIC_gpio1_bit21_IRQHandler,
  PLIC_gpio1_bit22_IRQHandler,
  PLIC_gpio1_bit23_IRQHandler,

  PLIC_gpio0_non_direct_IRQHandler,
  PLIC_gpio1_non_direct_IRQHandler,
  PLIC_gpio2_non_direct_IRQHandler,

  PLIC_spi0_IRQHandler,
  PLIC_spi1_IRQHandler,
  PLIC_external_can0_IRQHandler,
  PLIC_can1_IRQHandler,
  PLIC_External_i2c0_main_IRQHandler,
  PLIC_External_i2c0_alert_IRQHandler,
  PLIC_i2c0_sus_IRQHandler,
  PLIC_i2c1_main_IRQHandler,
  PLIC_i2c1_alert_IRQHandler,
  PLIC_i2c1_sus_IRQHandler,
  PLIC_mac0_int_IRQHandler,
  PLIC_mac0_queue1_IRQHandler,
  PLIC_mac0_queue2_IRQHandler,
  PLIC_mac0_queue3_IRQHandler,
  PLIC_mac0_emac_IRQHandler,
  PLIC_mac0_mmsl_IRQHandler,
  PLIC_mac1_int_IRQHandler,
  PLIC_mac1_queue1_IRQHandler,
  PLIC_mac1_queue2_IRQHandler,
  PLIC_mac1_queue3_IRQHandler,
  PLIC_mac1_emac_IRQHandler,
  PLIC_mac1_mmsl_IRQHandler,
  PLIC_ddrc_train_IRQHandler,
  PLIC_scb_interrupt_IRQHandler,
  PLIC_ecc_error_IRQHandler,
  PLIC_ecc_correct_IRQHandler,
  PLIC_rtc_wakeup_IRQHandler,
  PLIC_rtc_match_IRQHandler,
  PLIC_timer1_IRQHandler,
  PLIC_timer2_IRQHandler,
  PLIC_envm_IRQHandler,
  PLIC_qspi_IRQHandler,
  PLIC_usb_dma_IRQHandler,
  PLIC_usb_mc_IRQHandler,
  PLIC_mmc_main_IRQHandler,
  PLIC_mmc_wakeup_IRQHandler,
  PLIC_mmuart0_IRQHandler,
  PLIC_mmuart1_IRQHandler,
  PLIC_mmuart2_IRQHandler,
  PLIC_mmuart3_IRQHandler,
  PLIC_mmuart4_IRQHandler,

  PLIC_devrst_IRQHandler,
  PLIC_g5c_message_IRQHandler,
  PLIC_usoc_vc_interrupt_IRQHandler,
  PLIC_usoc_smb_interrupt_IRQHandler,
  PLIC_E51_Maintence_IRQHandler,

  PLIC_wdog0_mvrp_IRQHandler,
  PLIC_wdog1_mvrp_IRQHandler, /*100 contains multiple interrupts- */
  PLIC_wdog2_mvrp_IRQHandler,
  PLIC_wdog3_mvrp_IRQHandler,
  PLIC_wdog4_mvrp_IRQHandler,
  PLIC_wdog0_tout_IRQHandler,
  PLIC_wdog1_tout_IRQHandler,
  PLIC_wdog2_tout_IRQHandler,
  PLIC_wdog3_tout_IRQHandler,
  PLIC_wdog4_tout_IRQHandler,

  PLIC_g5c_mss_spi_IRQHandler,
  PLIC_volt_temp_alarm_IRQHandler,
  PLIC_athena_complete_IRQHandler,
  PLIC_athena_alarm_IRQHandler,
  PLIC_athena_bus_error_IRQHandler,
  PLIC_usoc_axic_us_IRQHandler,
  PLIC_usoc_axic_ds_IRQHandler,

  PLIC_reserved_104_IRQHandler,

  PLIC_f2m_0_IRQHandler,
  PLIC_f2m_1_IRQHandler,
  PLIC_f2m_2_IRQHandler,
  PLIC_f2m_3_IRQHandler,
  PLIC_f2m_4_IRQHandler,
  PLIC_f2m_5_IRQHandler,
  PLIC_f2m_6_IRQHandler,
  PLIC_f2m_7_IRQHandler,
  PLIC_f2m_8_IRQHandler,
  PLIC_f2m_9_IRQHandler,

  PLIC_f2m_10_IRQHandler,
  PLIC_f2m_11_IRQHandler,
  PLIC_f2m_12_IRQHandler,
  PLIC_f2m_13_IRQHandler,
  PLIC_f2m_14_IRQHandler,
  PLIC_f2m_15_IRQHandler,
  PLIC_f2m_16_IRQHandler,
  PLIC_f2m_17_IRQHandler,
  PLIC_f2m_18_IRQHandler,
  PLIC_f2m_19_IRQHandler,

  PLIC_f2m_20_IRQHandler,
  PLIC_f2m_21_IRQHandler,
  PLIC_f2m_22_IRQHandler,
  PLIC_f2m_23_IRQHandler,
  PLIC_f2m_24_IRQHandler,
  PLIC_f2m_25_IRQHandler,
  PLIC_f2m_26_IRQHandler,
  PLIC_f2m_27_IRQHandler,
  PLIC_f2m_28_IRQHandler,
  PLIC_f2m_29_IRQHandler,

  PLIC_f2m_30_IRQHandler,
  PLIC_f2m_31_IRQHandler,

  PLIC_f2m_32_IRQHandler,
  PLIC_f2m_33_IRQHandler,
  PLIC_f2m_34_IRQHandler,
  PLIC_f2m_35_IRQHandler,
  PLIC_f2m_36_IRQHandler,
  PLIC_f2m_37_IRQHandler,
  PLIC_f2m_38_IRQHandler,
  PLIC_f2m_39_IRQHandler,
  PLIC_f2m_40_IRQHandler,
  PLIC_f2m_41_IRQHandler,

  PLIC_f2m_42_IRQHandler,
  PLIC_f2m_43_IRQHandler,
  PLIC_f2m_44_IRQHandler,
  PLIC_f2m_45_IRQHandler,
  PLIC_f2m_46_IRQHandler,
  PLIC_f2m_47_IRQHandler,
  PLIC_f2m_48_IRQHandler,
  PLIC_f2m_49_IRQHandler,
  PLIC_f2m_50_IRQHandler,
  PLIC_f2m_51_IRQHandler,

  PLIC_f2m_52_IRQHandler,
  PLIC_f2m_53_IRQHandler,
  PLIC_f2m_54_IRQHandler,
  PLIC_f2m_55_IRQHandler,
  PLIC_f2m_56_IRQHandler,
  PLIC_f2m_57_IRQHandler,
  PLIC_f2m_58_IRQHandler,
  PLIC_f2m_59_IRQHandler,
  PLIC_f2m_60_IRQHandler,
  PLIC_f2m_61_IRQHandler,

  PLIC_f2m_62_IRQHandler,
  PLIC_f2m_63_IRQHandler,

  PLIC_E51_bus_error_unit_IRQHandler,
  PLIC_U54_1_bus_error_unit_IRQHandler,
  PLIC_U54_2_bus_error_unit_IRQHandler,
  PLIC_U54_3_bus_error_unit_IRQHandler,
  PLIC_U54_4_bus_error_unit_IRQHandler
};

#define E51_LOCAL_NUM_SOURCES 48U


void (*E51_local_irq_handler_table[E51_LOCAL_NUM_SOURCES])(void) =
{
    E51_maintenance_local_IRQHandler,    /* reference multiple interrupts */
    E51_usoc_smb_local_IRQHandler,
    E51_usoc_vc_local_IRQHandler,
    E51_g5c_message_local_IRQHandler,
    E51_g5c_devrst_local_IRQHandler,
    E51_wdog4_tout_local_IRQHandler,
    E51_wdog3_tout_local_IRQHandler,
    E51_wdog2_tout_local_IRQHandler,
    E51_wdog1_tout_local_IRQHandler,
    E51_wdog0_tout_local_IRQHandler,
    E51_wdog0_mvrp_local_IRQHandler,
    E51_mmuart0_local_IRQHandler,
    E51_envm_local_IRQHandler,
    E51_ecc_correct_local_IRQHandler,
    E51_ecc_error_local_IRQHandler,
    E51_scb_local_IRQHandler,
    E51_f2m_32_local_IRQHandler,
    E51_f2m_33_local_IRQHandler,
    E51_f2m_34_local_IRQHandler,
    E51_f2m_35_local_IRQHandler,
    E51_f2m_36_local_IRQHandler,
    E51_f2m_37_local_IRQHandler,
    E51_f2m_38_local_IRQHandler,
    E51_f2m_39_local_IRQHandler,
    E51_f2m_40_local_IRQHandler,
    E51_f2m_41_local_IRQHandler,

    E51_f2m_42_local_IRQHandler,
    E51_f2m_43_local_IRQHandler,
    E51_f2m_44_local_IRQHandler,
    E51_f2m_45_local_IRQHandler,
    E51_f2m_46_local_IRQHandler,
    E51_f2m_47_local_IRQHandler,
    E51_f2m_48_local_IRQHandler,
    E51_f2m_49_local_IRQHandler,
    E51_f2m_50_local_IRQHandler,
    E51_f2m_51_local_IRQHandler,

    E51_f2m_52_local_IRQHandler,
    E51_f2m_53_local_IRQHandler,
    E51_f2m_54_local_IRQHandler,
    E51_f2m_55_local_IRQHandler,
    E51_f2m_56_local_IRQHandler,
    E51_f2m_57_local_IRQHandler,
    E51_f2m_58_local_IRQHandler,
    E51_f2m_59_local_IRQHandler,
    E51_f2m_60_local_IRQHandler,
    E51_f2m_61_local_IRQHandler,

    E51_f2m_62_local_IRQHandler,
    E51_f2m_63_local_IRQHandler
};


typedef void (*local_int_p_t)(void);

/* U54 1 */
local_int_p_t local_irq_handler_1_table[E51_LOCAL_NUM_SOURCES] =
{
    /*reference multiple interrupts*/
    U54_spare_0_local_IRQHandler,
    U54_spare_1_local_IRQHandler,
    U54_spare_2_local_IRQHandler,

    /*parse hart ID to discover which mac is the source*/
    U54_1_mac0_mmsl_local_IRQHandler,
    U54_1_mac0_emac_local_IRQHandler,
    U54_1_mac0_queue3_local_IRQHandler,
    U54_1_mac0_queue2_local_IRQHandler,
    U54_1_mac0_queue1_local_IRQHandler,
    U54_1_mac0_int_local_IRQHandler,

    /*parse hart ID to discover which wdog is the source*/
    U54_1_wdog_tout_local_IRQHandler,
    U54_1_wdog_mvrp_local_IRQHandler,
    U54_1_mmuart1_local_IRQHandler,

    U54_spare_3_local_IRQHandler,
    U54_spare_4_local_IRQHandler,
    U54_spare_5_local_IRQHandler,
    U54_spare_6_local_IRQHandler,

    U54_f2m_0_local_IRQHandler,
    U54_f2m_1_local_IRQHandler,
    U54_f2m_2_local_IRQHandler,
    U54_f2m_3_local_IRQHandler,
    U54_f2m_4_local_IRQHandler,
    U54_f2m_5_local_IRQHandler,
    U54_f2m_6_local_IRQHandler,
    U54_f2m_7_local_IRQHandler,
    U54_f2m_8_local_IRQHandler,
    U54_f2m_9_local_IRQHandler,

    U54_f2m_10_local_IRQHandler,
    U54_f2m_11_local_IRQHandler,
    U54_f2m_12_local_IRQHandler,
    U54_f2m_13_local_IRQHandler,
    U54_f2m_14_local_IRQHandler,
    U54_f2m_15_local_IRQHandler,
    U54_f2m_16_local_IRQHandler,
    U54_f2m_17_local_IRQHandler,
    U54_f2m_18_local_IRQHandler,
    U54_f2m_19_local_IRQHandler,

    U54_f2m_20_local_IRQHandler,
    U54_f2m_21_local_IRQHandler,
    U54_f2m_22_local_IRQHandler,
    U54_f2m_23_local_IRQHandler,
    U54_f2m_24_local_IRQHandler,
    U54_f2m_25_local_IRQHandler,
    U54_f2m_26_local_IRQHandler,
    U54_f2m_27_local_IRQHandler,
    U54_f2m_28_local_IRQHandler,
    U54_f2m_29_local_IRQHandler,

    U54_f2m_30_local_IRQHandler,
    U54_f2m_31_local_IRQHandler
};

/* U54 2 */
local_int_p_t local_irq_handler_2_table[E51_LOCAL_NUM_SOURCES] =
{
    /*reference multiple interrupts*/
    U54_spare_0_local_IRQHandler,
    U54_spare_1_local_IRQHandler,
    U54_spare_2_local_IRQHandler,

    /*parse hart ID to discover which mac is the source*/
    U54_2_mac0_mmsl_local_IRQHandler,
    U54_2_mac0_emac_local_IRQHandler,
    U54_2_mac0_queue3_local_IRQHandler,
    U54_2_mac0_queue2_local_IRQHandler,
    U54_2_mac0_queue1_local_IRQHandler,
    U54_2_mac0_int_local_IRQHandler,

    /*parse hart ID to discover which wdog is the source*/
    U54_2_wdog_tout_local_IRQHandler,
    U54_2_wdog_mvrp_local_IRQHandler,
    U54_2_mmuart2_local_IRQHandler,

    U54_spare_3_local_IRQHandler,
    U54_spare_4_local_IRQHandler,
    U54_spare_5_local_IRQHandler,
    U54_spare_6_local_IRQHandler,

    U54_f2m_0_local_IRQHandler,
    U54_f2m_1_local_IRQHandler,
    U54_f2m_2_local_IRQHandler,
    U54_f2m_3_local_IRQHandler,
    U54_f2m_4_local_IRQHandler,
    U54_f2m_5_local_IRQHandler,
    U54_f2m_6_local_IRQHandler,
    U54_f2m_7_local_IRQHandler,
    U54_f2m_8_local_IRQHandler,
    U54_f2m_9_local_IRQHandler,

    U54_f2m_10_local_IRQHandler,
    U54_f2m_11_local_IRQHandler,
    U54_f2m_12_local_IRQHandler,
    U54_f2m_13_local_IRQHandler,
    U54_f2m_14_local_IRQHandler,
    U54_f2m_15_local_IRQHandler,
    U54_f2m_16_local_IRQHandler,
    U54_f2m_17_local_IRQHandler,
    U54_f2m_18_local_IRQHandler,
    U54_f2m_19_local_IRQHandler,

    U54_f2m_20_local_IRQHandler,
    U54_f2m_21_local_IRQHandler,
    U54_f2m_22_local_IRQHandler,
    U54_f2m_23_local_IRQHandler,
    U54_f2m_24_local_IRQHandler,
    U54_f2m_25_local_IRQHandler,
    U54_f2m_26_local_IRQHandler,
    U54_f2m_27_local_IRQHandler,
    U54_f2m_28_local_IRQHandler,
    U54_f2m_29_local_IRQHandler,

    U54_f2m_30_local_IRQHandler,
    U54_f2m_31_local_IRQHandler
};

/* U54 3 */
local_int_p_t local_irq_handler_3_table[E51_LOCAL_NUM_SOURCES] =
{
    /*reference multiple interrupts*/
    U54_spare_0_local_IRQHandler,
    U54_spare_1_local_IRQHandler,
    U54_spare_2_local_IRQHandler,

    /*parse hart ID to discover which mac is the source*/
    U54_3_mac1_mmsl_local_IRQHandler,
    U54_3_mac1_emac_local_IRQHandler,
    U54_3_mac1_queue3_local_IRQHandler,
    U54_3_mac1_queue2_local_IRQHandler,
    U54_3_mac1_queue1_local_IRQHandler,
    U54_3_mac1_int_local_IRQHandler,

    /*parse hart ID to discover which wdog is the source*/
    U54_3_wdog_tout_local_IRQHandler,
    U54_3_wdog_mvrp_local_IRQHandler,
    U54_3_mmuart3_local_IRQHandler,

    U54_spare_3_local_IRQHandler,
    U54_spare_4_local_IRQHandler,
    U54_spare_5_local_IRQHandler,
    U54_spare_6_local_IRQHandler,

    U54_f2m_0_local_IRQHandler,
    U54_f2m_1_local_IRQHandler,
    U54_f2m_2_local_IRQHandler,
    U54_f2m_3_local_IRQHandler,
    U54_f2m_4_local_IRQHandler,
    U54_f2m_5_local_IRQHandler,
    U54_f2m_6_local_IRQHandler,
    U54_f2m_7_local_IRQHandler,
    U54_f2m_8_local_IRQHandler,
    U54_f2m_9_local_IRQHandler,

    U54_f2m_10_local_IRQHandler,
    U54_f2m_11_local_IRQHandler,
    U54_f2m_12_local_IRQHandler,
    U54_f2m_13_local_IRQHandler,
    U54_f2m_14_local_IRQHandler,
    U54_f2m_15_local_IRQHandler,
    U54_f2m_16_local_IRQHandler,
    U54_f2m_17_local_IRQHandler,
    U54_f2m_18_local_IRQHandler,
    U54_f2m_19_local_IRQHandler,

    U54_f2m_20_local_IRQHandler,
    U54_f2m_21_local_IRQHandler,
    U54_f2m_22_local_IRQHandler,
    U54_f2m_23_local_IRQHandler,
    U54_f2m_24_local_IRQHandler,
    U54_f2m_25_local_IRQHandler,
    U54_f2m_26_local_IRQHandler,
    U54_f2m_27_local_IRQHandler,
    U54_f2m_28_local_IRQHandler,
    U54_f2m_29_local_IRQHandler,

    U54_f2m_30_local_IRQHandler,
    U54_f2m_31_local_IRQHandler
};

/* U54 4 */
local_int_p_t local_irq_handler_4_table[E51_LOCAL_NUM_SOURCES] =
{
    /*reference multiple interrupts*/
    U54_spare_0_local_IRQHandler,
    U54_spare_1_local_IRQHandler,
    U54_spare_2_local_IRQHandler,

    /*parse hart ID to discover which mac is the source*/
    U54_4_mac1_mmsl_local_IRQHandler,
    U54_4_mac1_emac_local_IRQHandler,
    U54_4_mac1_queue3_local_IRQHandler,
    U54_4_mac1_queue2_local_IRQHandler,
    U54_4_mac1_queue1_local_IRQHandler,
    U54_4_mac1_int_local_IRQHandler,

    /*parse hart ID to discover which wdog is the source*/
    U54_4_wdog_tout_local_IRQHandler,
    U54_4_wdog_mvrp_local_IRQHandler,
    U54_4_mmuart4_local_IRQHandler,

    U54_spare_3_local_IRQHandler,
    U54_spare_4_local_IRQHandler,
    U54_spare_5_local_IRQHandler,
    U54_spare_6_local_IRQHandler,

    U54_f2m_0_local_IRQHandler,
    U54_f2m_1_local_IRQHandler,
    U54_f2m_2_local_IRQHandler,
    U54_f2m_3_local_IRQHandler,
    U54_f2m_4_local_IRQHandler,
    U54_f2m_5_local_IRQHandler,
    U54_f2m_6_local_IRQHandler,
    U54_f2m_7_local_IRQHandler,
    U54_f2m_8_local_IRQHandler,
    U54_f2m_9_local_IRQHandler,

    U54_f2m_10_local_IRQHandler,
    U54_f2m_11_local_IRQHandler,
    U54_f2m_12_local_IRQHandler,
    U54_f2m_13_local_IRQHandler,
    U54_f2m_14_local_IRQHandler,
    U54_f2m_15_local_IRQHandler,
    U54_f2m_16_local_IRQHandler,
    U54_f2m_17_local_IRQHandler,
    U54_f2m_18_local_IRQHandler,
    U54_f2m_19_local_IRQHandler,

    U54_f2m_20_local_IRQHandler,
    U54_f2m_21_local_IRQHandler,
    U54_f2m_22_local_IRQHandler,
    U54_f2m_23_local_IRQHandler,
    U54_f2m_24_local_IRQHandler,
    U54_f2m_25_local_IRQHandler,
    U54_f2m_26_local_IRQHandler,
    U54_f2m_27_local_IRQHandler,
    U54_f2m_28_local_IRQHandler,
    U54_f2m_29_local_IRQHandler,

    U54_f2m_30_local_IRQHandler,
    U54_f2m_31_local_IRQHandler
};

local_int_p_t *local_int_mux[5] =
{
    E51_local_irq_handler_table,
    local_irq_handler_1_table,
    local_irq_handler_2_table,
    local_irq_handler_3_table,
    local_irq_handler_4_table
};

/*------------------------------------------------------------------------------
 *
 */
void handle_m_ext_interrupt(void)
{

    volatile uint32_t int_num  = PLIC_ClaimIRQ();

    if (PLIC_INVALID_INT_OFFSET == int_num)
    {
       return;
    }

    uint8_t disable = EXT_IRQ_KEEP_ENABLED;
    disable = ext_irq_handler_table[int_num]();

    PLIC_CompleteIRQ(int_num);

    if(EXT_IRQ_DISABLE == disable)
    {
        PLIC_DisableIRQ((PLIC_IRQn_Type)int_num);
    }

}


/*------------------------------------------------------------------------------
 *
 */
void handle_local_interrupt(uint8_t interrupt_no)
{
    uint64_t mhart_id = read_csr(mhartid);
    uint8_t local_interrupt_no = (uint8_t)(interrupt_no - 16U);
    local_int_p_t *local_int_table = local_int_mux[mhart_id];

    (*local_int_table[local_interrupt_no])();
}

/*------------------------------------------------------------------------------
 *
 */
void trap_from_machine_mode(uintptr_t * regs, uintptr_t dummy, uintptr_t mepc)
{
    volatile uintptr_t mcause = read_csr(mcause);

    if (((mcause & MCAUSE_INT) == MCAUSE_INT) && ((mcause & MCAUSE_CAUSE) >=\
            IRQ_M_LOCAL_MIN)&& ((mcause & MCAUSE_CAUSE)  <= IRQ_M_LOCAL_MAX))
    {
        handle_local_interrupt((uint8_t)(mcause & MCAUSE_CAUSE));
    }
    else if (((mcause & MCAUSE_INT) == MCAUSE_INT) && ((mcause & MCAUSE_CAUSE)\
            == IRQ_M_EXT))
    {
        handle_m_ext_interrupt();
    }
    else if (((mcause & MCAUSE_INT) == MCAUSE_INT) && ((mcause & MCAUSE_CAUSE)\
            == IRQ_M_SOFT))
    {
        handle_m_soft_interrupt();
    }
    else if (((mcause & MCAUSE_INT) == MCAUSE_INT) && ((mcause & MCAUSE_CAUSE)\
            == IRQ_M_TIMER))
    {
        handle_m_timer_interrupt();
    }
    else if (((mcause & MCAUSE_INT) == MCAUSE_INT) && ((mcause & MCAUSE_CAUSE)\
            == IRQ_M_BEU ))
    {
        handle_local_beu_interrupt();
    }
    else
    {
        uint32_t i = 0U;
        while(1U)
        {
            /* wait for watchdog */
            i++;
            if(i == 0x1000U)
            {
                i = (uint32_t)mcause; /* so mcause is not optimised out */
            }
        }
        switch(mcause)
        {

            case CAUSE_LOAD_PAGE_FAULT:
                break;
            case CAUSE_STORE_PAGE_FAULT:
                break;
            case CAUSE_FETCH_ACCESS:
                break;
            case CAUSE_LOAD_ACCESS:
                break;
            case CAUSE_STORE_ACCESS:
                break;
            default:
                bad_trap(regs, dummy, mepc);
                break;
        }
    }
}

#ifdef __cplusplus
}
#endif
