/*******************************************************************************
 * Copyright 2019 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * @file mss_plic.h
 * @author Microchip FPGA Embedded Systems Solutions
 * @brief PolarFire SoC MSS PLIC and PRCI access data structures and functions.
 *
 * Definitions and functions associated with PLIC interrupts.
 *
 */

#ifndef MSS_PLIC_H
#define MSS_PLIC_H

#include <stdint.h>
#ifndef CONFIG_OPENSBI
#include "encoding.h"
#endif

#include "mss_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 *Return value from External IRQ handler. This will be used to disable the
 *Return External interrupt.
 */
#define EXT_IRQ_KEEP_ENABLED                                0U
#define EXT_IRQ_DISABLE                                     1U

/*------------------------------------------------------------------------------
 *
 */
uint8_t  PLIC_Invalid_IRQHandler(void);
uint8_t  PLIC_l2_metadata_corr_IRQHandler(void);
uint8_t  PLIC_l2_metadata_uncorr_IRQHandler(void);
uint8_t  PLIC_l2_data_corr_IRQHandler(void);
uint8_t  PLIC_l2_data_uncorr_IRQHandler(void);
uint8_t  PLIC_dma_ch0_DONE_IRQHandler(void);
uint8_t  PLIC_dma_ch0_ERR_IRQHandler(void);
uint8_t  PLIC_dma_ch1_DONE_IRQHandler(void);
uint8_t  PLIC_dma_ch1_ERR_IRQHandler(void);
uint8_t  PLIC_dma_ch2_DONE_IRQHandler(void);
uint8_t  PLIC_dma_ch2_ERR_IRQHandler(void);
uint8_t  PLIC_dma_ch3_DONE_IRQHandler(void);
uint8_t  PLIC_dma_ch3_ERR_IRQHandler(void);
uint8_t  PLIC_gpio0_bit0_or_gpio2_bit13_IRQHandler(void);
uint8_t  PLIC_gpio0_bit1_or_gpio2_bit13_IRQHandler(void);
uint8_t  PLIC_gpio0_bit2_or_gpio2_bit13_IRQHandler(void);
uint8_t  PLIC_gpio0_bit3_or_gpio2_bit13_IRQHandler(void);
uint8_t  PLIC_gpio0_bit4_or_gpio2_bit13_IRQHandler(void);
uint8_t  PLIC_gpio0_bit5_or_gpio2_bit13_IRQHandler(void);
uint8_t  PLIC_gpio0_bit6_or_gpio2_bit13_IRQHandler(void);
uint8_t  PLIC_gpio0_bit7_or_gpio2_bit13_IRQHandler(void);
uint8_t  PLIC_gpio0_bit8_or_gpio2_bit13_IRQHandler(void);
uint8_t  PLIC_gpio0_bit9_or_gpio2_bit13_IRQHandler(void);
uint8_t  PLIC_gpio0_bit10_or_gpio2_bit13_IRQHandler(void);
uint8_t  PLIC_gpio0_bit11_or_gpio2_bit13_IRQHandler(void);
uint8_t  PLIC_gpio0_bit12_or_gpio2_bit13_IRQHandler(void);

uint8_t  PLIC_gpio0_bit13_or_gpio2_bit13_IRQHandler(void);
uint8_t  PLIC_gpio1_bit0_or_gpio2_bit14_IRQHandler(void);
uint8_t  PLIC_gpio1_bit1_or_gpio2_bit15_IRQHandler(void);
uint8_t  PLIC_gpio1_bit2_or_gpio2_bit16_IRQHandler(void);
uint8_t  PLIC_gpio1_bit3_or_gpio2_bit17_IRQHandler(void);
uint8_t  PLIC_gpio1_bit4_or_gpio2_bit18_IRQHandler(void);
uint8_t  PLIC_gpio1_bit5_or_gpio2_bit19_IRQHandler(void);
uint8_t  PLIC_gpio1_bit6_or_gpio2_bit20_IRQHandler(void);
uint8_t  PLIC_gpio1_bit7_or_gpio2_bit21_IRQHandler(void);
uint8_t  PLIC_gpio1_bit8_or_gpio2_bit22_IRQHandler(void);
uint8_t  PLIC_gpio1_bit9_or_gpio2_bit23_IRQHandler(void);
uint8_t  PLIC_gpio1_bit10_or_gpio2_bit24_IRQHandler(void);
uint8_t  PLIC_gpio1_bit11_or_gpio2_bit25_IRQHandler(void);
uint8_t  PLIC_gpio1_bit12_or_gpio2_bit26_IRQHandler(void);
uint8_t  PLIC_gpio1_bit13_or_gpio2_bit27_IRQHandler(void);

uint8_t  PLIC_gpio1_bit14_or_gpio2_bit28_IRQHandler(void);
uint8_t  PLIC_gpio1_bit15_or_gpio2_bit29_IRQHandler(void);
uint8_t  PLIC_gpio1_bit16_or_gpio2_bit30_IRQHandler(void);
uint8_t  PLIC_gpio1_bit17_or_gpio2_bit31_IRQHandler(void);

uint8_t  PLIC_gpio1_bit18_IRQHandler(void);
uint8_t  PLIC_gpio1_bit19_IRQHandler(void);
uint8_t  PLIC_gpio1_bit20_IRQHandler(void);
uint8_t  PLIC_gpio1_bit21_IRQHandler(void);
uint8_t  PLIC_gpio1_bit22_IRQHandler(void);
uint8_t  PLIC_gpio1_bit23_IRQHandler(void);

uint8_t  PLIC_gpio0_non_direct_IRQHandler(void);
uint8_t  PLIC_gpio1_non_direct_IRQHandler(void);
uint8_t  PLIC_gpio2_non_direct_IRQHandler(void);

uint8_t  PLIC_spi0_IRQHandler(void);
uint8_t  PLIC_spi1_IRQHandler(void);
uint8_t  PLIC_external_can0_IRQHandler(void);
uint8_t  PLIC_can1_IRQHandler(void);
uint8_t  PLIC_External_i2c0_main_IRQHandler(void);
uint8_t  PLIC_External_i2c0_alert_IRQHandler(void);
uint8_t  PLIC_i2c0_sus_IRQHandler(void);
uint8_t  PLIC_i2c1_main_IRQHandler(void);
uint8_t  PLIC_i2c1_alert_IRQHandler(void);
uint8_t  PLIC_i2c1_sus_IRQHandler(void);
uint8_t  PLIC_mac0_int_IRQHandler(void);
uint8_t  PLIC_mac0_queue1_IRQHandler(void);
uint8_t  PLIC_mac0_queue2_IRQHandler(void);
uint8_t  PLIC_mac0_queue3_IRQHandler(void);
uint8_t  PLIC_mac0_emac_IRQHandler(void);
uint8_t  PLIC_mac0_mmsl_IRQHandler(void);
uint8_t  PLIC_mac1_int_IRQHandler(void);
uint8_t  PLIC_mac1_queue1_IRQHandler(void);
uint8_t  PLIC_mac1_queue2_IRQHandler(void);
uint8_t  PLIC_mac1_queue3_IRQHandler(void);
uint8_t  PLIC_mac1_emac_IRQHandler(void);
uint8_t  PLIC_mac1_mmsl_IRQHandler(void);
uint8_t  PLIC_ddrc_train_IRQHandler(void);
uint8_t  PLIC_scb_interrupt_IRQHandler(void);
uint8_t  PLIC_ecc_error_IRQHandler(void);
uint8_t  PLIC_ecc_correct_IRQHandler(void);
uint8_t  PLIC_rtc_wakeup_IRQHandler(void);
uint8_t  PLIC_rtc_match_IRQHandler(void);
uint8_t  PLIC_timer1_IRQHandler(void);
uint8_t  PLIC_timer2_IRQHandler(void);
uint8_t  PLIC_envm_IRQHandler(void);
uint8_t  PLIC_qspi_IRQHandler(void);
uint8_t  PLIC_usb_dma_IRQHandler(void);
uint8_t  PLIC_usb_mc_IRQHandler(void);
uint8_t  PLIC_mmc_main_IRQHandler(void);
uint8_t  PLIC_mmc_wakeup_IRQHandler(void);
uint8_t  PLIC_mmuart0_IRQHandler(void);
uint8_t  PLIC_mmuart1_IRQHandler(void);
uint8_t  PLIC_mmuart2_IRQHandler(void);
uint8_t  PLIC_mmuart3_IRQHandler(void);
uint8_t  PLIC_mmuart4_IRQHandler(void);
uint8_t  PLIC_devrst_IRQHandler(void);
uint8_t  PLIC_g5c_message_IRQHandler(void);
uint8_t  PLIC_usoc_vc_interrupt_IRQHandler(void);
uint8_t  PLIC_usoc_smb_interrupt_IRQHandler(void);
uint8_t  PLIC_E51_Maintence_IRQHandler(void);

uint8_t  PLIC_wdog0_mvrp_IRQHandler(void);
uint8_t  PLIC_wdog1_mvrp_IRQHandler(void);
uint8_t  PLIC_wdog2_mvrp_IRQHandler(void);
uint8_t  PLIC_wdog3_mvrp_IRQHandler(void);
uint8_t  PLIC_wdog4_mvrp_IRQHandler(void);
uint8_t  PLIC_wdog0_tout_IRQHandler(void);
uint8_t  PLIC_wdog1_tout_IRQHandler(void);
uint8_t  PLIC_wdog2_tout_IRQHandler(void);
uint8_t  PLIC_wdog3_tout_IRQHandler(void);
uint8_t  PLIC_wdog4_tout_IRQHandler(void);
uint8_t  PLIC_g5c_mss_spi_IRQHandler(void);
uint8_t  PLIC_volt_temp_alarm_IRQHandler(void);

uint8_t  PLIC_athena_complete_IRQHandler(void);
uint8_t  PLIC_athena_alarm_IRQHandler(void);
uint8_t  PLIC_athena_bus_error_IRQHandler(void);
uint8_t  PLIC_usoc_axic_us_IRQHandler(void);
uint8_t  PLIC_usoc_axic_ds_IRQHandler(void);

uint8_t  PLIC_reserved_104_IRQHandler(void);

uint8_t  PLIC_f2m_0_IRQHandler(void);
uint8_t  PLIC_f2m_1_IRQHandler(void);
uint8_t  PLIC_f2m_2_IRQHandler(void);
uint8_t  PLIC_f2m_3_IRQHandler(void);
uint8_t  PLIC_f2m_4_IRQHandler(void);
uint8_t  PLIC_f2m_5_IRQHandler(void);
uint8_t  PLIC_f2m_6_IRQHandler(void);
uint8_t  PLIC_f2m_7_IRQHandler(void);
uint8_t  PLIC_f2m_8_IRQHandler(void);
uint8_t  PLIC_f2m_9_IRQHandler(void);

uint8_t  PLIC_f2m_10_IRQHandler(void);
uint8_t  PLIC_f2m_11_IRQHandler(void);
uint8_t  PLIC_f2m_12_IRQHandler(void);
uint8_t  PLIC_f2m_13_IRQHandler(void);
uint8_t  PLIC_f2m_14_IRQHandler(void);
uint8_t  PLIC_f2m_15_IRQHandler(void);
uint8_t  PLIC_f2m_16_IRQHandler(void);
uint8_t  PLIC_f2m_17_IRQHandler(void);
uint8_t  PLIC_f2m_18_IRQHandler(void);
uint8_t  PLIC_f2m_19_IRQHandler(void);

uint8_t  PLIC_f2m_20_IRQHandler(void);
uint8_t  PLIC_f2m_21_IRQHandler(void);
uint8_t  PLIC_f2m_22_IRQHandler(void);
uint8_t  PLIC_f2m_23_IRQHandler(void);
uint8_t  PLIC_f2m_24_IRQHandler(void);
uint8_t  PLIC_f2m_25_IRQHandler(void);
uint8_t  PLIC_f2m_26_IRQHandler(void);
uint8_t  PLIC_f2m_27_IRQHandler(void);
uint8_t  PLIC_f2m_28_IRQHandler(void);
uint8_t  PLIC_f2m_29_IRQHandler(void);

uint8_t  PLIC_f2m_30_IRQHandler(void);
uint8_t  PLIC_f2m_31_IRQHandler(void);

uint8_t  PLIC_f2m_32_IRQHandler(void);
uint8_t  PLIC_f2m_33_IRQHandler(void);
uint8_t  PLIC_f2m_34_IRQHandler(void);
uint8_t  PLIC_f2m_35_IRQHandler(void);
uint8_t  PLIC_f2m_36_IRQHandler(void);
uint8_t  PLIC_f2m_37_IRQHandler(void);
uint8_t  PLIC_f2m_38_IRQHandler(void);
uint8_t  PLIC_f2m_39_IRQHandler(void);
uint8_t  PLIC_f2m_40_IRQHandler(void);
uint8_t  PLIC_f2m_41_IRQHandler(void);

uint8_t PLIC_f2m_42_IRQHandler(void);
uint8_t PLIC_f2m_43_IRQHandler(void);
uint8_t PLIC_f2m_44_IRQHandler(void);
uint8_t PLIC_f2m_45_IRQHandler(void);
uint8_t PLIC_f2m_46_IRQHandler(void);
uint8_t PLIC_f2m_47_IRQHandler(void);
uint8_t PLIC_f2m_48_IRQHandler(void);
uint8_t PLIC_f2m_49_IRQHandler(void);
uint8_t PLIC_f2m_50_IRQHandler(void);
uint8_t PLIC_f2m_51_IRQHandler(void);

uint8_t PLIC_f2m_52_IRQHandler(void);
uint8_t PLIC_f2m_53_IRQHandler(void);
uint8_t PLIC_f2m_54_IRQHandler(void);
uint8_t PLIC_f2m_55_IRQHandler(void);
uint8_t PLIC_f2m_56_IRQHandler(void);
uint8_t PLIC_f2m_57_IRQHandler(void);
uint8_t PLIC_f2m_58_IRQHandler(void);
uint8_t PLIC_f2m_59_IRQHandler(void);
uint8_t PLIC_f2m_60_IRQHandler(void);
uint8_t PLIC_f2m_61_IRQHandler(void);

uint8_t PLIC_f2m_62_IRQHandler(void);
uint8_t PLIC_f2m_63_IRQHandler(void);

uint8_t PLIC_E51_bus_error_unit_IRQHandler(void);
uint8_t PLIC_U54_1_bus_error_unit_IRQHandler(void);
uint8_t PLIC_U54_2_bus_error_unit_IRQHandler(void);
uint8_t PLIC_U54_3_bus_error_unit_IRQHandler(void);
uint8_t PLIC_U54_4_bus_error_unit_IRQHandler(void);

/***************************************************************************//**
 * PLIC source Interrupt numbers:
 */
/* See section on PLIC Interrupt Sources in User Guide */
#define OFFSET_TO_MSS_GLOBAL_INTS 13U
typedef enum
{
    PLIC_INVALID_INT_OFFSET                 = 0,
    PLIC_L2_METADATA_CORR_INT_OFFSET        = 1,
    PLIC_L2_METADAT_UNCORR_INT_OFFSET       = 2,
    PLIC_L2_DATA_CORR_INT_OFFSET            = 3,
    PLIC_L2_DATA_UNCORR_INT_OFFSET          = 4,
    PLIC_DMA_CH0_DONE_INT_OFFSET            = 5,
    PLIC_DMA_CH0_ERR_INT_OFFSET             = 6,
    PLIC_DMA_CH1_DONE_INT_OFFSET            = 7,
    PLIC_DMA_CH1_ERR_INT_OFFSET             = 8,
    PLIC_DMA_CH2_DONE_INT_OFFSET            = 9,
    PLIC_DMA_CH2_ERR_INT_OFFSET             = 10,
    PLIC_DMA_CH3_DONE_INT_OFFSET            = 11,
    PLIC_DMA_CH3_ERR_INT_OFFSET             = 12,
    /* see PLIC_I2C Interrupt Multiplexing in the User Guide */
    PLIC_GPIO0_BIT0_or_GPIO2_BIT0_INT_OFFSET         = 0 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO0_BIT1_or_GPIO2_BIT1_INT_OFFSET         = 1 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO0_BIT2_or_GPIO2_BIT2_INT_OFFSET         = 2 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO0_BIT3_or_GPIO2_BIT3_INT_OFFSET         = 3 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO0_BIT4_or_GPIO2_BIT4_INT_OFFSET         = 4 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO0_BIT5_or_GPIO2_BIT5_INT_OFFSET         = 5 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO0_BIT6_or_GPIO2_BIT6_INT_OFFSET         = 6 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO0_BIT7_or_GPIO2_BIT7_INT_OFFSET         = 7 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO0_BIT8_or_GPIO2_BIT8_INT_OFFSET         = 8 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO0_BIT9_or_GPIO2_BIT9_INT_OFFSET         = 9 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO0_BIT10_or_GPIO2_BIT10_INT_OFFSET      = 10 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO0_BIT11_or_GPIO2_BIT11_INT_OFFSET      = 11 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO0_BIT12_or_GPIO2_BIT12_INT_OFFSET      = 12 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_GPIO0_BIT13_or_GPIO2_BIT13_INT_OFFSET      = 13 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT0_or_GPIO2_BIT14_INT_OFFSET       = 14 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT1_or_GPIO2_BIT15_INT_OFFSET       = 15 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT2_or_GPIO2_BIT16_INT_OFFSET       = 16 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT3_or_GPIO2_BIT17_INT_OFFSET       = 17 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT4_or_GPIO2_BIT18_INT_OFFSET       = 18 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT5_or_GPIO2_BIT19_INT_OFFSET       = 19 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT6_or_GPIO2_BIT20_INT_OFFSET       = 20 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT7_or_GPIO2_BIT21_INT_OFFSET       = 21 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT8_or_GPIO2_BIT22_INT_OFFSET       = 22 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT9_or_GPIO2_BIT23_INT_OFFSET       = 23 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT10_or_GPIO2_BIT24_INT_OFFSET      = 24 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT11_or_GPIO2_BIT25_INT_OFFSET      = 25 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT12_or_GPIO2_BIT26_INT_OFFSET      = 26 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT13_or_GPIO2_BIT27_INT_OFFSET      = 27 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_GPIO1_BIT14_or_GPIO2_BIT28_INT_OFFSET       = 28 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT15_or_GPIO2_BIT29_INT_OFFSET       = 29 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT16_or_GPIO2_BIT30_INT_OFFSET       = 30 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT17_or_GPIO2_BIT31_INT_OFFSET       = 31 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_GPIO1_BIT18_INT_OFFSET           = 32 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT19_INT_OFFSET           = 33 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT20_INT_OFFSET           = 34 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT21_INT_OFFSET           = 35 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT22_INT_OFFSET           = 36 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_BIT23_INT_OFFSET           = 37 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_GPIO0_NON_DIRECT_INT_OFFSET         = 38 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO1_NON_DIRECT_INT_OFFSET         = 39 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_GPIO2_NON_DIRECT_INT_OFFSET         = 40 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_SPI0_INT_OFFSET                    = 41 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_SPI1_INT_OFFSET                    = 42 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_CAN0_INT_OFFSET                    = 43 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_CAN1_INT_OFFSET                    = 44 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_I2C0_MAIN_INT_OFFSET               = 45 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_I2C0_ALERT_INT_OFFSET              = 46 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_I2C0_SUS_INT_OFFSET                = 47 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_I2C1_MAIN_INT_OFFSET               = 48 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_I2C1_ALERT_INT_OFFSET              = 49 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_I2C1_SUS_INT_OFFSET                = 50 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MAC0_INT_INT_OFFSET                = 51 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MAC0_QUEUE1_INT_OFFSET             = 52 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MAC0_QUEUE2_INT_OFFSET             = 53 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MAC0_QUEUE3_INT_OFFSET             = 54 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MAC0_EMAC_INT_OFFSET               = 55 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MAC0_MMSL_INT_OFFSET               = 56 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MAC1_INT_INT_OFFSET                = 57 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MAC1_QUEUE1_INT_OFFSET             = 58 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MAC1_QUEUE2_INT_OFFSET             = 59 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MAC1_QUEUE3_INT_OFFSET             = 60 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MAC1_EMAC_INT_OFFSET               = 61 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MAC1_MMSL_INT_OFFSET               = 62 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_DDRC_TRAIN_INT_OFFSET              = 63 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_SCB_INTERRUPT_INT_OFFSET           = 64 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_ECC_ERROR_INT_OFFSET               = 65 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_ECC_CORRECT_INT_OFFSET             = 66 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_RTC_WAKEUP_INT_OFFSET              = 67 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_RTC_MATCH_INT_OFFSET               = 68 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_TIMER1_INT_OFFSET                  = 69 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_TIMER2_INT_OFFSET                  = 70 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_ENVM_INT_OFFSET                    = 71 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_QSPI_INT_OFFSET                    = 72 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_USB_DMA_INT_OFFSET                 = 73 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_USB_MC_INT_OFFSET                  = 74 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MMC_main_INT_OFFSET                = 75 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MMC_wakeup_INT_OFFSET              = 76 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MMUART0_INT_OFFSET                 = 77 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MMUART1_INT_OFFSET                 = 78 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MMUART2_INT_OFFSET                 = 79 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MMUART3_INT_OFFSET                 = 80 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_MMUART4_INT_OFFSET                 = 81 + OFFSET_TO_MSS_GLOBAL_INTS,

    G5C_DEVRST_INT_OFFSET              = 82 + OFFSET_TO_MSS_GLOBAL_INTS,
    g5c_MESSAGE_INT_OFFSET             = 83 + OFFSET_TO_MSS_GLOBAL_INTS,
    USOC_VC_INTERRUPT_INT_OFFSET       = 84 + OFFSET_TO_MSS_GLOBAL_INTS,
    USOC_SMB_INTERRUPT_INT_OFFSET      = 85 + OFFSET_TO_MSS_GLOBAL_INTS,
    /* contains multiple interrupts- */
    E51_0_MAINTENACE_INT_OFFSET        = 86 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_WDOG0_MRVP_INT_OFFSET              = 87 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_WDOG1_MRVP_INT_OFFSET              = 88 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_WDOG2_MRVP_INT_OFFSET              = 89 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_WDOG3_MRVP_INT_OFFSET              = 90 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_WDOG4_MRVP_INT_OFFSET              = 91 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_WDOG0_TOUT_INT_OFFSET              = 92 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_WDOG1_TOUT_INT_OFFSET              = 93 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_WDOG2_TOUT_INT_OFFSET              = 94 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_WDOG3_TOUT_INT_OFFSET              = 95 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_WDOG4_TOUT_INT_OFFSET              = 96 + OFFSET_TO_MSS_GLOBAL_INTS,
    G5C_MSS_SPI_INT_OFFSET             = 97 + OFFSET_TO_MSS_GLOBAL_INTS,
    VOLT_TEMP_ALARM_INT_OFFSET         = 98 + OFFSET_TO_MSS_GLOBAL_INTS,
    ATHENA_COMPLETE_INT_OFFSET         = 99 + OFFSET_TO_MSS_GLOBAL_INTS,
    ATHENA_ALARM_INT_OFFSET            = 100 + OFFSET_TO_MSS_GLOBAL_INTS,
    ATHENA_BUS_ERROR_INT_OFFSET        = 101 + OFFSET_TO_MSS_GLOBAL_INTS,
    USOC_AXIC_US_INT_OFFSET            = 102 + OFFSET_TO_MSS_GLOBAL_INTS,
    USOC_AXIC_DS_INT_OFFSET            = 103 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_F2M_0_INT_OFFSET            = 105 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_1_INT_OFFSET            = 106 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_2_INT_OFFSET            = 107 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_3_INT_OFFSET            = 108 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_4_INT_OFFSET            = 109 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_5_INT_OFFSET            = 110 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_6_INT_OFFSET            = 111 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_7_INT_OFFSET            = 112 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_8_INT_OFFSET            = 113 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_9_INT_OFFSET            = 114 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_F2M_10_INT_OFFSET           = 115 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_11_INT_OFFSET           = 116 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_12_INT_OFFSET           = 117 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_13_INT_OFFSET           = 118 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_14_INT_OFFSET           = 119 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_15_INT_OFFSET           = 120 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_16_INT_OFFSET           = 121 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_17_INT_OFFSET           = 122 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_18_INT_OFFSET           = 123 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_19_INT_OFFSET           = 124 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_F2M_20_INT_OFFSET           = 125 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_21_INT_OFFSET           = 126 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_22_INT_OFFSET           = 127 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_23_INT_OFFSET           = 128 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_24_INT_OFFSET           = 129 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_25_INT_OFFSET           = 130 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_26_INT_OFFSET           = 131 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_27_INT_OFFSET           = 132 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_28_INT_OFFSET           = 133 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_29_INT_OFFSET           = 134 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_F2M_30_INT_OFFSET           = 135 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_31_INT_OFFSET           = 136 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_F2M_32_INT_OFFSET           = 137 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_33_INT_OFFSET           = 138 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_34_INT_OFFSET           = 139 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_35_INT_OFFSET           = 140 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_36_INT_OFFSET           = 141 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_37_INT_OFFSET           = 142 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_38_INT_OFFSET           = 143 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_39_INT_OFFSET           = 144 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_40_INT_OFFSET           = 145 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_41_INT_OFFSET           = 146 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_F2M_42_INT_OFFSET           = 147 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_43_INT_OFFSET           = 148 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_44_INT_OFFSET           = 149 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_45_INT_OFFSET           = 150 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_46_INT_OFFSET           = 151 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_47_INT_OFFSET           = 152 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_48_INT_OFFSET           = 153 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_49_INT_OFFSET           = 154 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_50_INT_OFFSET           = 155 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_51_INT_OFFSET           = 156 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_F2M_52_INT_OFFSET           = 157 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_53_INT_OFFSET           = 158 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_54_INT_OFFSET           = 159 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_55_INT_OFFSET           = 160 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_56_INT_OFFSET           = 161 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_57_INT_OFFSET           = 162 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_58_INT_OFFSET           = 163 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_59_INT_OFFSET           = 164 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_60_INT_OFFSET           = 165 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_61_INT_OFFSET           = 166 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_F2M_62_INT_OFFSET           = 167 + OFFSET_TO_MSS_GLOBAL_INTS,
    PLIC_F2M_63_INT_OFFSET           = 168 + OFFSET_TO_MSS_GLOBAL_INTS,

    PLIC_E51_BUS_ERROR_UNIT_OFFSET        = 182,
    PLIC_U54_1_BUS_ERROR_UNIT_OFFSET      = 183,
    PLIC_U54_2_BUS_ERROR_UNIT_OFFSET      = 184,
    PLIC_U54_3_BUS_ERROR_UNIT_OFFSET      = 185,
    PLIC_U54_4_BUS_ERROR_UNIT_OFFSET      = 186

} PLIC_IRQn_Type;

#define MAX_PLIC_INT PLIC_U54_4_BUS_ERROR_UNIT_OFFSET

typedef struct
{
    volatile uint32_t PRIORITY_THRESHOLD;
    volatile uint32_t CLAIM_COMPLETE;
    volatile uint32_t reserved[(0x1000/4)-2];
} IRQ_Target_Type;

typedef struct
{
    volatile uint32_t ENABLES[32U];
} Target_Enables_Type;


#define PLIC_SET_UP_REGISTERS 6U

#define PLIC_NUM_SOURCES 187U

#define PLIC_NUM_PRIORITIES 7U
#define NUM_CLAIM_REGS      9U


/* The FU540-C000 has 53 interrupt sources. */
 typedef struct
{
    volatile uint32_t RESERVED0;
    /*-------------------- Source Priority --------------------*/
    volatile uint32_t SOURCE_PRIORITY[PLIC_NUM_SOURCES];
    volatile uint32_t RESERVED1[(0x1000 / 4) - (PLIC_NUM_SOURCES + 1)];
    /*-------------------- Pending array --------------------*/
    volatile const uint32_t PENDING_ARRAY[PLIC_SET_UP_REGISTERS];
    volatile uint32_t RESERVED2[(0x1000/4) - PLIC_SET_UP_REGISTERS];

    /*-----------------Hart0 Mode Enables--------------------*/
    volatile uint32_t HART0_MMODE_ENA[PLIC_SET_UP_REGISTERS];
    volatile uint32_t RESERVED3[(0x80/4) - PLIC_SET_UP_REGISTERS];

    /*-----------------Hart1 Mode Enables--------------------*/
    volatile uint32_t HART1_MMODE_ENA[PLIC_SET_UP_REGISTERS];
    volatile uint32_t RESERVED4a[(0x80/4) - PLIC_SET_UP_REGISTERS];
    volatile uint32_t HART1_SMODE_ENA[PLIC_SET_UP_REGISTERS];
    volatile uint32_t RESERVED4[(0x80/4) - PLIC_SET_UP_REGISTERS];

    /*-----------------Hart2 Mode Enables--------------------*/
    volatile uint32_t HART2_MMODE_ENA[PLIC_SET_UP_REGISTERS];
    volatile uint32_t RESERVED5a[(0x80/4) - PLIC_SET_UP_REGISTERS];
    volatile uint32_t HART2_SMODE_ENA[PLIC_SET_UP_REGISTERS];
    volatile uint32_t RESERVED5[(0x80/4) - PLIC_SET_UP_REGISTERS];

    /*-----------------Hart3 Mode Enables--------------------*/
    volatile uint32_t HART3_MMODE_ENA[PLIC_SET_UP_REGISTERS];
    volatile uint32_t RESERVED6a[(0x80/4) - PLIC_SET_UP_REGISTERS];
    volatile uint32_t HART3_SMODE_ENA[PLIC_SET_UP_REGISTERS];
    volatile uint32_t RESERVED6[(0x80/4) - PLIC_SET_UP_REGISTERS];

    /*-----------------Hart4 Mode Enables--------------------*/
    volatile uint32_t HART4_MMODE_ENA[PLIC_SET_UP_REGISTERS];
    volatile uint32_t RESERVED7a[(0x80/4) - PLIC_SET_UP_REGISTERS];
    volatile uint32_t HART4_SMODE_ENA[PLIC_SET_UP_REGISTERS];
    volatile uint32_t RESERVED7[(0x80/4) - PLIC_SET_UP_REGISTERS];

    volatile uint32_t RESERVED8[(0x0C200000 - 0x0C002480)/4];

    /*--- Target Priority threshold and claim/complete---------*/
    IRQ_Target_Type TARGET[NUM_CLAIM_REGS];   /* See PLIC Register Map or
                                                 TARGET_OFFSET defines below
                                                 for offset details */

} PLIC_Type;

#define TARGET_OFFSET_HART0_M 0U
#define TARGET_OFFSET_HART1_M 1U
#define TARGET_OFFSET_HART1_S 2U
#define TARGET_OFFSET_HART2_M 3U
#define TARGET_OFFSET_HART2_S 4U
#define TARGET_OFFSET_HART3_M 5U
#define TARGET_OFFSET_HART3_S 6U
#define TARGET_OFFSET_HART4_M 7U
#define TARGET_OFFSET_HART4_S 8U

extern const unsigned long plic_hart_lookup[5U];

/***************************************************************************//**
 * PLIC: Platform Level Interrupt Controller
 */
#define PLIC_BASE_ADDR 0x0C000000UL

#define PLIC    ((PLIC_Type * const)PLIC_BASE_ADDR)

/*-------------------------------------------------------------------------*//**
 * The function PLIC_init() initializes the PLIC controller and enables the
 * global external interrupt bit.
 */

/*-----------------Hart Mode Enables--------------------*/

static inline void PLIC_init(void)
{
    uint32_t inc;
    uint64_t hart_id  = read_csr(mhartid);

    /* Disable all interrupts for the current hart. */
    switch(hart_id)
    {
        case 0:
            for(inc = 0UL; inc < PLIC_SET_UP_REGISTERS; inc++)
            {
                PLIC->HART0_MMODE_ENA[inc] = 0U;
            }

            /* Set the threshold to zero.
             * PLIC provides context based threshold register for the settings of a
             * interrupt priority threshold of each context. The threshold register
             * is a WARL field. The PLIC will mask all PLIC interrupts of a priority
             * less than or equal to threshold. For example, a threshold value of zero
             * permits all interrupts with non-zero priority.*/

            PLIC->TARGET[TARGET_OFFSET_HART0_M].PRIORITY_THRESHOLD  = 0U;
            break;
        case 1:
            for(inc = 0UL; inc < PLIC_SET_UP_REGISTERS; inc++)
            {
                PLIC->HART1_MMODE_ENA[inc] = 0U;
                PLIC->HART1_SMODE_ENA[inc] = 0U;
            }

            PLIC->TARGET[TARGET_OFFSET_HART1_M].PRIORITY_THRESHOLD  = 0U;
            /* Disable supervisor level */
            PLIC->TARGET[TARGET_OFFSET_HART1_S].PRIORITY_THRESHOLD  = 7U;
            break;
        case 2:
            for(inc = 0UL; inc < PLIC_SET_UP_REGISTERS; inc++)
            {
                PLIC->HART2_MMODE_ENA[inc] = 0U;
                PLIC->HART2_SMODE_ENA[inc] = 0U;
            }

            PLIC->TARGET[TARGET_OFFSET_HART2_M].PRIORITY_THRESHOLD  = 0U;
            /* Disable supervisor level */
            PLIC->TARGET[TARGET_OFFSET_HART2_S].PRIORITY_THRESHOLD  = 7U;
            break;
        case 3:
            for(inc = 0UL; inc < PLIC_SET_UP_REGISTERS; inc++)
            {
                PLIC->HART3_MMODE_ENA[inc] = 0U;
                PLIC->HART3_SMODE_ENA[inc] = 0U;
            }

            PLIC->TARGET[TARGET_OFFSET_HART3_M].PRIORITY_THRESHOLD  = 0U;
            /* Disable supervisor level */
            PLIC->TARGET[TARGET_OFFSET_HART3_S].PRIORITY_THRESHOLD  = 7U;
            break;
        case 4:
            for(inc = 0UL; inc < PLIC_SET_UP_REGISTERS; inc++)
            {
                PLIC->HART4_MMODE_ENA[inc] = 0U;
                PLIC->HART4_SMODE_ENA[inc] = 0U;
            }

            PLIC->TARGET[TARGET_OFFSET_HART4_M].PRIORITY_THRESHOLD  = 0U;
            /* Disable supervisor level */
            PLIC->TARGET[TARGET_OFFSET_HART4_S].PRIORITY_THRESHOLD  = 7U;
            break;
        default:
            break;
    }

    /* Enable machine external interrupts. */
    set_csr(mie, MIP_MEIP);
}



/***************************************************************************//**
 * The function PLIC_EnableIRQ() enables the external interrupt for the
 * interrupt number indicated by the parameter IRQn.
 */
static inline void PLIC_EnableIRQ(PLIC_IRQn_Type IRQn)
{
    uint32_t current;
    uint64_t hart_id  = read_csr(mhartid);

    switch(hart_id)
    {
        case 0:
            current  = PLIC->HART0_MMODE_ENA[IRQn / 32U];
            current |= (uint32_t)1 << (IRQn % 32U);
            PLIC->HART0_MMODE_ENA[IRQn / 32U]  = current;
            break;
        case 1:
            current  = PLIC->HART1_MMODE_ENA[IRQn / 32U];
            current |= (uint32_t)1 << (IRQn % 32U);
            PLIC->HART1_MMODE_ENA[IRQn / 32U]  = current;
            break;
        case 2:
            current  = PLIC->HART2_MMODE_ENA[IRQn / 32U];
            current |= (uint32_t)1 << (IRQn % 32U);
            PLIC->HART2_MMODE_ENA[IRQn / 32U]  = current;
            break;
        case 3:
            current  = PLIC->HART3_MMODE_ENA[IRQn / 32U];
            current |= (uint32_t)1 << (IRQn % 32U);
            PLIC->HART3_MMODE_ENA[IRQn / 32U]  = current;
            break;
        case 4:
            current  = PLIC->HART4_MMODE_ENA[IRQn / 32U];
            current |= (uint32_t)1 << (IRQn % 32U);
            PLIC->HART4_MMODE_ENA[IRQn / 32U]  = current;
            break;
        default:
            break;
    }
}

/***************************************************************************//**
 * The function PLIC_DisableIRQ() disables the external interrupt for the
 * interrupt number indicated by the parameter IRQn.

 * NOTE:
 *     This function can be used to disable the external interrupt from outside
 *     external interrupt handler function.
 *     This function MUST NOT be used from within the External Interrupt
 *     handler.
 *     If you wish to disable the external interrupt while the interrupt handler
 *     for that external interrupt is executing then you must use the return
 *     value EXT_IRQ_DISABLE to return from the extern interrupt handler.
 */
static inline void PLIC_DisableIRQ(PLIC_IRQn_Type IRQn)
{
    uint32_t current;
    uint64_t hart_id  = read_csr(mhartid);

    switch(hart_id)
    {
        case 0:
            current = PLIC->HART0_MMODE_ENA[IRQn / 32U];
            current &= ~((uint32_t)1 << (IRQn % 32U));
            PLIC->HART0_MMODE_ENA[IRQn / 32U] = current;
            break;
        case 1:
            current = PLIC->HART1_MMODE_ENA[IRQn / 32U];
            current &= ~((uint32_t)1 << (IRQn % 32U));
            PLIC->HART1_MMODE_ENA[IRQn / 32U] = current;
            break;
        case 2:
            current = PLIC->HART2_MMODE_ENA[IRQn / 32U];
            current &= ~((uint32_t)1 << (IRQn % 32U));
            PLIC->HART2_MMODE_ENA[IRQn / 32U] = current;
            break;
        case 3:
            current = PLIC->HART3_MMODE_ENA[IRQn / 32U];
            current &= ~((uint32_t)1 << (IRQn % 32U));
            PLIC->HART3_MMODE_ENA[IRQn / 32U] = current;
            break;
        case 4:
            current = PLIC->HART4_MMODE_ENA[IRQn / 32U];
            current &= ~((uint32_t)1 << (IRQn % 32U));
            PLIC->HART4_MMODE_ENA[IRQn / 32U] = current;
            break;
        default:
            break;
    }
}

/***************************************************************************//**
 * The function PLIC_SetPriority() sets the priority for the external interrupt
 * for the interrupt number indicated by the parameter IRQn.
 */
static inline void PLIC_SetPriority(PLIC_IRQn_Type IRQn, uint32_t priority)
{
    if((IRQn > PLIC_INVALID_INT_OFFSET) && (IRQn < PLIC_NUM_SOURCES))
    {
        PLIC->SOURCE_PRIORITY[IRQn-1] = priority;
    }
}

/***************************************************************************//**
 * The function PLIC_GetPriority() returns the priority for the external
 * interrupt for the interrupt number indicated by the parameter IRQn.
 */
static inline uint32_t PLIC_GetPriority(PLIC_IRQn_Type IRQn)
{
    uint32_t ret_val = 0U;

    if((IRQn > PLIC_INVALID_INT_OFFSET) && (IRQn < PLIC_NUM_SOURCES))
    {
        ret_val = PLIC->SOURCE_PRIORITY[IRQn-1];
    }

    return(ret_val);
}


static inline uint32_t PLIC_pending(PLIC_IRQn_Type IRQn)
{
    return (PLIC->PENDING_ARRAY[IRQn/32U] & (0x01U<<(IRQn%32U)));
}

/***************************************************************************//**
 * The function PLIC_ClaimIRQ() claims the interrupt from the PLIC controller.
 */
static inline uint32_t PLIC_ClaimIRQ(void)
{
    uint64_t hart_id  = read_csr(mhartid);

    return (PLIC->TARGET[plic_hart_lookup[hart_id]].CLAIM_COMPLETE);
}

/***************************************************************************//**
 * The function PLIC_CompleteIRQ() indicates to the PLIC controller the
 * interrupt is processed and claim is complete.
 */
static inline void PLIC_CompleteIRQ(uint32_t source)
{
    uint64_t hart_id  = read_csr(mhartid);

    ASSERT(source <= MAX_PLIC_INT);

    PLIC->TARGET[plic_hart_lookup[hart_id]].CLAIM_COMPLETE  = source;
}

/***************************************************************************//**
 *
 * The function PLIC_SetPriority_Threshold() sets the threshold for a particular
 * hart. The default threshold on reset is 0.
 * The PFSoC Core Complex supports setting of an interrupt priority threshold
 * via the threshold register. The threshold is a WARL field, where the PFSoC
 * Core Complex supports a maximum threshold of 7.
 * The PFSoC Core Complex will mask all PLIC interrupts of a priority less than
 * or equal to threshold. For example, a threshold value of zero permits all
 * interrupts with non-zero priority, whereas a value of 7 masks all
 * interrupts.
 */
static inline void PLIC_SetPriority_Threshold(uint32_t threshold)
{
    uint64_t hart_id  = read_csr(mhartid);

    ASSERT(threshold <= 7);

    PLIC->TARGET[plic_hart_lookup[hart_id]].PRIORITY_THRESHOLD  = threshold;
}

/***************************************************************************//**
 *  PLIC_ClearPendingIRQ(void)
 *  This is only called by the startup hart and only once
 *  Clears any pending interrupts as PLIC can be in unknown state on startup
 */
static inline void PLIC_ClearPendingIRQ(void)
{
    volatile uint32_t int_num  = PLIC_ClaimIRQ();
    volatile int32_t wait_possible_int;

    while ( int_num != PLIC_INVALID_INT_OFFSET)
    {
        PLIC_CompleteIRQ(int_num);
        wait_possible_int = 0xFU;
        while (wait_possible_int)
        {
            wait_possible_int--;
        }
        int_num  = PLIC_ClaimIRQ(); /* obtain interrupt, auto clears  */
    }
}

/***************************************************************************//**
 * This function is only called from one hart on startup
 */
static inline void PLIC_init_on_reset(void)
{
    uint32_t inc;

    /* default all priorities so effectively disabled */
    for(inc = 0U; inc < PLIC_NUM_SOURCES; ++inc)
    {
        /* priority must be greater than threshold to be enabled, so setting to
         * 7 disables */
        PLIC->SOURCE_PRIORITY[inc]  = 0U;
    }

    for(inc = 0U; inc < NUM_CLAIM_REGS; ++inc)
    {
        PLIC->TARGET[inc].PRIORITY_THRESHOLD  = 7U;
    }

    /* and clear all the enables */
    for(inc = 0UL; inc < PLIC_SET_UP_REGISTERS; inc++)
    {
        PLIC->HART0_MMODE_ENA[inc] = 0U;
        PLIC->HART1_MMODE_ENA[inc] = 0U;
        PLIC->HART1_SMODE_ENA[inc] = 0U;
        PLIC->HART2_MMODE_ENA[inc] = 0U;
        PLIC->HART2_SMODE_ENA[inc] = 0U;
        PLIC->HART3_MMODE_ENA[inc] = 0U;
        PLIC->HART3_SMODE_ENA[inc] = 0U;
        PLIC->HART4_MMODE_ENA[inc] = 0U;
        PLIC->HART4_SMODE_ENA[inc] = 0U;
    }

    /* clear any pending interrupts- in case already there */
    PLIC_ClearPendingIRQ();
}

#ifdef __cplusplus
}
#endif

#endif  /* MSS_PLIC_H */
