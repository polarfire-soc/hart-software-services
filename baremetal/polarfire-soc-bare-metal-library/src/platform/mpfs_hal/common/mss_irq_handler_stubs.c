/*******************************************************************************
 * Copyright 2019 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * @file mss_irq_handler_stubs.c
 * @author Microchip FPGA Embedded Systems Solutions
 * @brief MPFS MSS Interrupt Function stubs.
 *
 * The functions below will only be linked with the application code if the user
 * does not provide an implementation for these functions. These functions are
 * defined with weak linking so that they can be overridden by a function with
 * same prototype in the user's application code.
 *
 */

#include <stdint.h>
#include "mpfs_hal/mss_hal.h"


__attribute__((weak)) void handle_m_ext_interrupt(void)
{

}

__attribute__((weak)) void E51_software_IRQHandler(void)
{

}

__attribute__((weak)) void U54_1_software_IRQHandler(void)
{

}

__attribute__((weak)) void U54_2_software_IRQHandler(void)
{

}

__attribute__((weak)) void U54_3_software_IRQHandler(void)
{

}

__attribute__((weak)) void U54_4_software_IRQHandler(void)
{

}

__attribute__((weak)) void E51_sysTick_IRQHandler(void)
{

}

__attribute__((weak)) void U54_1_sysTick_IRQHandler(void)
{

}

__attribute__((weak)) void U54_2_sysTick_IRQHandler(void)
{

}

__attribute__((weak)) void U54_3_sysTick_IRQHandler(void)
{

}

__attribute__((weak)) void U54_4_sysTick_IRQHandler(void)
{

}

__attribute__((weak))  uint8_t PLIC_Invalid_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_dma_ch0_DONE_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_dma_ch0_ERR_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_dma_ch1_DONE_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_dma_ch1_ERR_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_dma_ch2_DONE_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_dma_ch2_ERR_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_dma_ch3_DONE_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_dma_ch3_ERR_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_l2_metadata_corr_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_l2_metadata_uncorr_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_l2_data_corr_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_l2_data_uncorr_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_gpio0_bit0_or_gpio2_bit13_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_gpio0_bit1_or_gpio2_bit13_IRQHandler(void)
        {
            return(0U);
        }

__attribute__((weak))  uint8_t  PLIC_gpio0_bit2_or_gpio2_bit13_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio0_bit3_or_gpio2_bit13_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio0_bit4_or_gpio2_bit13_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio0_bit5_or_gpio2_bit13_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio0_bit6_or_gpio2_bit13_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio0_bit7_or_gpio2_bit13_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio0_bit8_or_gpio2_bit13_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio0_bit9_or_gpio2_bit13_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio0_bit10_or_gpio2_bit13_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio0_bit11_or_gpio2_bit13_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio0_bit12_or_gpio2_bit13_IRQHandler(void)
{
    return(0U);
}


__attribute__((weak))  uint8_t  PLIC_gpio0_bit13_or_gpio2_bit13_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit0_or_gpio2_bit14_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit1_or_gpio2_bit15_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit2_or_gpio2_bit16_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit3_or_gpio2_bit17_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit4_or_gpio2_bit18_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit5_or_gpio2_bit19_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit6_or_gpio2_bit20_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit7_or_gpio2_bit21_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit8_or_gpio2_bit22_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit9_or_gpio2_bit23_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit10_or_gpio2_bit24_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit11_or_gpio2_bit25_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit12_or_gpio2_bit26_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit13_or_gpio2_bit27_IRQHandler(void)
{
    return(0U);
}


__attribute__((weak))  uint8_t  PLIC_gpio1_bit14_or_gpio2_bit28_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit15_or_gpio2_bit29_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit16_or_gpio2_bit30_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit17_or_gpio2_bit31_IRQHandler(void)
{
    return(0U);
}


__attribute__((weak))  uint8_t  PLIC_gpio1_bit18_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit19_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit20_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit21_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit22_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_bit23_IRQHandler(void)
{
    return(0U);
}


__attribute__((weak))  uint8_t  PLIC_gpio0_non_direct_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio1_non_direct_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_gpio2_non_direct_IRQHandler(void)
{
    return(0U);
}


__attribute__((weak))  uint8_t  PLIC_spi0_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_spi1_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_external_can0_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_can1_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_External_i2c0_main_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_External_i2c0_alert_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_i2c0_sus_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_i2c1_main_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_i2c1_alert_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_i2c1_sus_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mac0_int_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mac0_queue1_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mac0_queue2_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mac0_queue3_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mac0_emac_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mac0_mmsl_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mac1_int_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mac1_queue1_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mac1_queue2_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mac1_queue3_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mac1_emac_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mac1_mmsl_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_ddrc_train_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_scb_interrupt_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_ecc_error_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_ecc_correct_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_rtc_wakeup_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_rtc_match_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_timer1_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_timer2_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_envm_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_qspi_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_usb_dma_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_usb_mc_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mmc_main_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mmc_wakeup_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mmuart0_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mmuart1_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mmuart2_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mmuart3_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_mmuart4_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_devrst_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak))  uint8_t  PLIC_g5c_message_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_usoc_vc_interrupt_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_usoc_smb_interrupt_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak)) uint8_t  PLIC_E51_Maintence_IRQHandler(void)
{
    return(0U);
}


__attribute__((weak)) uint8_t  PLIC_wdog0_mvrp_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak)) uint8_t  PLIC_wdog1_mvrp_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak)) uint8_t  PLIC_wdog2_mvrp_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak)) uint8_t  PLIC_wdog3_mvrp_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak)) uint8_t  PLIC_wdog4_mvrp_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak)) uint8_t  PLIC_wdog0_tout_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak)) uint8_t  PLIC_wdog1_tout_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak)) uint8_t  PLIC_wdog2_tout_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak)) uint8_t  PLIC_wdog3_tout_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak)) uint8_t  PLIC_wdog4_tout_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak)) uint8_t  PLIC_g5c_mss_spi_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak)) uint8_t  PLIC_volt_temp_alarm_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak))  uint8_t  PLIC_athena_complete_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak))  uint8_t  PLIC_athena_alarm_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak))  uint8_t  PLIC_athena_bus_error_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak))  uint8_t  PLIC_usoc_axic_us_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak))  uint8_t  PLIC_usoc_axic_ds_IRQHandler(void)
{
    return(0U);
}
__attribute__((weak))  uint8_t  PLIC_reserved_104_IRQHandler(void)
{
    return(0U);
}



__attribute__((weak))  uint8_t  PLIC_f2m_0_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_1_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_2_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_3_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_4_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_5_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_6_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_7_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_8_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_9_IRQHandler(void)
{
    return(0U);
}


__attribute__((weak))  uint8_t  PLIC_f2m_10_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_11_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_12_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_13_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_14_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_15_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_16_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_17_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_18_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_19_IRQHandler(void)
{
    return(0U);
}


__attribute__((weak))  uint8_t  PLIC_f2m_20_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_21_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_22_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_23_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_24_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_25_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_26_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_27_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_28_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_29_IRQHandler(void)
{
    return(0U);
}


__attribute__((weak))  uint8_t  PLIC_f2m_30_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_31_IRQHandler(void)
{
    return(0U);
}


__attribute__((weak))  uint8_t  PLIC_f2m_32_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_33_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_34_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_35_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_36_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_37_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_38_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_39_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_40_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t  PLIC_f2m_41_IRQHandler(void)
{
    return(0U);
}


__attribute__((weak))  uint8_t PLIC_f2m_42_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_43_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_44_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_45_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_46_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_47_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_48_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_49_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_50_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_51_IRQHandler(void)
{
    return(0U);
}


__attribute__((weak))  uint8_t PLIC_f2m_52_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_53_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_54_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_55_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_56_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_57_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_58_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_59_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_60_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_61_IRQHandler(void)
{
    return(0U);
}


__attribute__((weak))  uint8_t PLIC_f2m_62_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_f2m_63_IRQHandler(void)
{
    return(0U);
}


__attribute__((weak))  uint8_t PLIC_E51_bus_error_unit_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_U54_1_bus_error_unit_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_U54_2_bus_error_unit_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_U54_3_bus_error_unit_IRQHandler(void)
{
    return(0U);
}

__attribute__((weak))  uint8_t PLIC_U54_4_bus_error_unit_IRQHandler(void)
{
    return(0U);
}

/* Local interrupt stubs */
__attribute__((weak))  void E51_maintenance_local_IRQHandler(void)
{
}
__attribute__((weak))  void E51_usoc_smb_local_IRQHandler(void)
{
}
__attribute__((weak))  void E51_usoc_vc_local_IRQHandler(void)
{
}
__attribute__((weak))  void E51_g5c_message_local_IRQHandler(void)
{
}
__attribute__((weak))  void E51_g5c_devrst_local_IRQHandler(void)
{
}
__attribute__((weak))  void E51_wdog4_tout_local_IRQHandler(void)
{
}
__attribute__((weak))  void E51_wdog3_tout_local_IRQHandler(void)
{
}
__attribute__((weak))  void E51_wdog2_tout_local_IRQHandler(void)
{
}
__attribute__((weak))  void E51_wdog1_tout_local_IRQHandler(void)
{
}
__attribute__((weak))  void E51_wdog0_tout_local_IRQHandler(void)
{
}
__attribute__((weak))  void E51_wdog0_mvrp_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_mmuart0_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_envm_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_ecc_correct_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_ecc_error_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_scb_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_32_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_33_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_34_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_35_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_36_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_37_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_38_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_39_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_40_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_41_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_42_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_43_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_44_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_45_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_46_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_47_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_48_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_49_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_50_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_51_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_52_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_53_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_54_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_55_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_56_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_57_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_58_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_59_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_60_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_61_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_62_local_IRQHandler(void)
{
}

__attribute__((weak))  void E51_f2m_63_local_IRQHandler(void)
{
}


/*
 * U54
 */
__attribute__((weak))  void U54_spare_0_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_spare_1_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_spare_2_local_IRQHandler(void)
{
}

/* Ethernet MACs - GEM0 is on U54s 1 and 2, GEM1 is on U54s 3 and 4 */

/* U54 1 */
__attribute__((weak))  void U54_1_mac0_mmsl_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_1_mac0_emac_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_1_mac0_queue3_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_1_mac0_queue2_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_1_mac0_queue1_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_1_mac0_int_local_IRQHandler(void)
{
}

/* U54 2 */
__attribute__((weak))  void U54_2_mac0_mmsl_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_2_mac0_emac_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_2_mac0_queue3_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_2_mac0_queue2_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_2_mac0_queue1_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_2_mac0_int_local_IRQHandler(void)
{
}

/* U54 3 */
__attribute__((weak))  void U54_3_mac1_mmsl_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_3_mac1_emac_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_3_mac1_queue3_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_3_mac1_queue2_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_3_mac1_queue1_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_3_mac1_int_local_IRQHandler(void)
{
}

/* U54 4 */
__attribute__((weak))  void U54_4_mac1_mmsl_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_4_mac1_emac_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_4_mac1_queue3_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_4_mac1_queue2_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_4_mac1_queue1_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_4_mac1_int_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_1_wdog_tout_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_2_wdog_tout_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_3_wdog_tout_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_4_wdog_tout_local_IRQHandler(void)
{
}
__attribute__((weak))  void mvrp_u54_local_IRQHandler_10(void)
{
}
__attribute__((weak))  void U54_1_wdog_mvrp_local_IRQHandler(void)
{
    mvrp_u54_local_IRQHandler_10();
}
__attribute__((weak))  void U54_2_wdog_mvrp_local_IRQHandler(void)
{
    mvrp_u54_local_IRQHandler_10();
}
__attribute__((weak))  void U54_3_wdog_mvrp_local_IRQHandler(void)
{
    mvrp_u54_local_IRQHandler_10();
}
__attribute__((weak))  void U54_4_wdog_mvrp_local_IRQHandler(void)
{
    mvrp_u54_local_IRQHandler_10();
}
__attribute__((weak))  void U54_1_mmuart1_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_2_mmuart2_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_3_mmuart3_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_4_mmuart4_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_spare_3_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_spare_4_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_spare_5_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_spare_6_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_0_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_1_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_2_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_3_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_4_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_5_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_6_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_7_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_8_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_9_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_10_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_11_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_12_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_13_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_14_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_15_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_16_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_17_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_18_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_19_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_20_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_21_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_22_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_23_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_24_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_25_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_26_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_27_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_28_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_29_local_IRQHandler(void)
{
}

__attribute__((weak))  void U54_f2m_30_local_IRQHandler(void)
{
}
__attribute__((weak))  void U54_f2m_31_local_IRQHandler(void)
{
}
