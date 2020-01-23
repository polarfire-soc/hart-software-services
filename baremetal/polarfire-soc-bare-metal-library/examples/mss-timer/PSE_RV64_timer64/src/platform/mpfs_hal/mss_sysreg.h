/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/***************************************************************************
 * @file mss_sysreg.h
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief Hardware register definitions.
 * updated sysreg defines Based on SVN rev 100432 of g5_mss_top_sysreg.xls
 *
 * SVN $Revision: 11865 $
 * SVN $Date: 2019-07-29 19:58:05 +0530 (Mon, 29 Jul 2019) $
 */
#ifndef MSS_SYSREG_H
#define MSS_SYSREG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

 /* IO definitions (access restrictions to peripheral registers) */
 /**
     \defgroup CMSIS_glob_defs CMSIS Global Defines
     <strong>IO Type Qualifiers</strong> are used
     \li to specify the access to peripheral variables.
     \li for automatic generation of peripheral register debug information.
 */
#ifndef __I
 #ifdef __cplusplus
   #define   __I     volatile               /*!< Defines 'read only' permissions */
 #else
   #define   __I     volatile const         /*!< Defines 'read only' permissions */
 #endif
#endif
#ifndef __O
 #define     __O     volatile               /*!< Defines 'write only' permissions */
#endif
#ifndef __IO
 #define     __IO    volatile               /*!< Defines 'read / write' permissions */
#endif
 /* following defines should be used for structure members */
#ifndef __IM
 #define     __IM     volatile const        /*! Defines 'read only' structure member permissions */
#endif
#ifndef __OM
 #define     __OM     volatile              /*! Defines 'write only' structure member permissions */
#endif
#ifndef __IOM
 #define __IOM volatile                     /*! Defines 'read / write' structure member permissions */
#endif

#include "mss_peripheral_base_add.h"

/* Defines all Top Register offsets*/
/* Date of Source Revision File: 12-Jul-18*/
/* PROTOCOL=MSS; BASE=32'h20012000*/
/* Hardware Base Address*/
#define BASE32_ADDR_MSS_SYSREG 0x20002000


/*Register for software use*/
#define TEMP0_OFFSET   0x0
    /* Scratch register for CPUS*/
    #define TEMP0_DATA_OFFSET  0x0
    #define TEMP0_DATA_MASK    (0xFFFFFFFF << 0x0)

/*Register for software use*/
#define TEMP1_OFFSET   0x4
    /* Scratch register for CPUS*/
    #define TEMP1_DATA_OFFSET  0x0
    #define TEMP1_DATA_MASK    (0xFFFFFFFF << 0x0)

/*Master clock configuration*/
#define CLOCK_CONFIG_CR_OFFSET   0x8
    /* "Sets the master synchronous clock divider bits [1:0] CPU clock divider           (Reset=/1 =0)bits [3:2] AXI clock divider            (Reset=/1 =0)bits [5:4] AHB/APB clock divider  (Reset=/2 =1)00=/1   01=/2  10=/4 11=/8  (AHB/APB divider may not be set to /1)Note at reset MSS corner clock is 80MHz therefore divider is set to divide by 1"*/
    #define CLOCK_CONFIG_CR_divider_OFFSET  0x0
    #define CLOCK_CONFIG_CR_divider_MASK    (0x3F << 0x0)
    /* When '1' requests G5_control to enable the 1mHz (2MHz) on-chip oscillator*/
    #define CLOCK_CONFIG_CR_enable_1mhz_OFFSET  0x8
    #define CLOCK_CONFIG_CR_enable_1mhz_MASK    (0x01 << 0x8)

/*RTC clock divider*/
#define RTC_CLOCK_CR_OFFSET   0xC
    /* "Sets the  division ratio to create the internal RTC clock from the reference clock. The defaults sets the reference clock to 1MHz assuming the reference clock is 100Mhz.If the reference clock is 125MHz then 125 will create a 1MHz clockMax divider value is 4095 and value must be an integer.RTC clock must be less 2X the AXI clock rate."*/
    #define RTC_CLOCK_CR_period_OFFSET  0x0
    #define RTC_CLOCK_CR_period_MASK    (0xFFF << 0x0)
    /* RTC Clock enable. When chaning the divider the enable should be trurned off first the divider changed and the enable turned back on.*/
    #define RTC_CLOCK_CR_enable_OFFSET  0x10
    #define RTC_CLOCK_CR_enable_MASK    (0x01 << 0x10)

/*Fabric Reset mask*/
#define FABRIC_RESET_CR_OFFSET   0x10
    /* Blocks the fabric Reset input preventing the fabric reseting the MSS*/
    #define FABRIC_RESET_CR_ENABLE_OFFSET  0x0
    #define FABRIC_RESET_CR_ENABLE_MASK    (0x01 << 0x0)

/**/
#define BOOT_FAIL_CR_OFFSET   0x14
    /* Written by firmware to indicate that the boot process failed drives the fab_boot_fail signal to the fabric. Is cleared by the fabric asserting fab_boot_fail_clear*/
    #define BOOT_FAIL_CR_boot_OFFSET  0x0
    #define BOOT_FAIL_CR_boot_MASK    (0x01 << 0x0)

/*Configuration lock*/
#define CONFIG_LOCK_CR_OFFSET   0x1C
    /* When written to '1' will cause all RWC registers to lock until a master reset occurs.*/
    #define CONFIG_LOCK_CR_lock_OFFSET  0x0
    #define CONFIG_LOCK_CR_lock_MASK    (0x01 << 0x0)

/*Indicates which reset caused the last reset. After a reset occurs register should be read and then zero written to allow the next reset event to be correctly captured.*/
#define RESET_SR_OFFSET   0x20
    /* Reset was caused by the SCB periphery reset signal*/
    #define RESET_SR_SCB_PERIPH_RESET_OFFSET  0x0
    #define RESET_SR_SCB_PERIPH_RESET_MASK    (0x01 << 0x0)
    /* Reset was caused by the SCB MSS reset register*/
    #define RESET_SR_SCB_MSS_RESET_OFFSET  0x1
    #define RESET_SR_SCB_MSS_RESET_MASK    (0x01 << 0x1)
    /* Reset was caused by the SCB CPU reset register*/
    #define RESET_SR_SCB_CPU_RESET_OFFSET  0x2
    #define RESET_SR_SCB_CPU_RESET_MASK    (0x01 << 0x2)
    /* Reset was caused by the Risc-V Debugger*/
    #define RESET_SR_DEBUGER_RESET_OFFSET  0x3
    #define RESET_SR_DEBUGER_RESET_MASK    (0x01 << 0x3)
    /* Reset was caused by the fabric*/
    #define RESET_SR_FABRIC_RESET_OFFSET  0x4
    #define RESET_SR_FABRIC_RESET_MASK    (0x01 << 0x4)
    /* Reset was caused by the watchdog*/
    #define RESET_SR_WDOG_RESET_OFFSET  0x5
    #define RESET_SR_WDOG_RESET_MASK    (0x01 << 0x5)
    /* Indicates that fabric asserted the GPIO reset inputs*/
    #define RESET_SR_GPIO_RESET_OFFSET  0x6
    #define RESET_SR_GPIO_RESET_MASK    (0x01 << 0x6)
    /* Indicates that SCB bus reset occurred (which causes warm reset of MSS)*/
    #define RESET_SR_SCB_BUS_RESET_OFFSET  0x7
    #define RESET_SR_SCB_BUS_RESET_MASK    (0x01 << 0x7)

/*Indicates the device status in particular the state of the FPGA fabric and the MSS IO banks*/
#define DEVICE_STATUS_OFFSET   0x24
    /* Indicates the status of the core_up input from G5 Control.*/
    #define DEVICE_STATUS_CORE_UP_OFFSET  0x0
    #define DEVICE_STATUS_CORE_UP_MASK    (0x01 << 0x0)
    /* Indicates the status of the lp_state input from G5 Control.*/
    #define DEVICE_STATUS_LP_STATE_OFFSET  0x1
    #define DEVICE_STATUS_LP_STATE_MASK    (0x01 << 0x1)
    /* Indicates the status of the ff_in_progress input from G5 Control.*/
    #define DEVICE_STATUS_FF_IN_PROGRESS_OFFSET  0x2
    #define DEVICE_STATUS_FF_IN_PROGRESS_MASK    (0x01 << 0x2)
    /* Indicates the status of the flash_valid input from G5 Control.*/
    #define DEVICE_STATUS_FLASH_VALID_OFFSET  0x3
    #define DEVICE_STATUS_FLASH_VALID_MASK    (0x01 << 0x3)
    /* Power status of IO bank 2*/
    #define DEVICE_STATUS_io_bank_b2_status_OFFSET  0x8
    #define DEVICE_STATUS_io_bank_b2_status_MASK    (0x01 << 0x8)
    /* Power status of IO bank 4*/
    #define DEVICE_STATUS_io_bank_b4_status_OFFSET  0x9
    #define DEVICE_STATUS_io_bank_b4_status_MASK    (0x01 << 0x9)
    /* Power status of IO bank 5*/
    #define DEVICE_STATUS_io_bank_b5_status_OFFSET  0xA
    #define DEVICE_STATUS_io_bank_b5_status_MASK    (0x01 << 0xA)
    /* Power status of IO bank 6*/
    #define DEVICE_STATUS_io_bank_b6_status_OFFSET  0xB
    #define DEVICE_STATUS_io_bank_b6_status_MASK    (0x01 << 0xB)
    /* Indicates the status of the io_en input from G5 Control.*/
    #define DEVICE_STATUS_IO_EN_OFFSET  0xC
    #define DEVICE_STATUS_IO_EN_MASK    (0x01 << 0xC)

/*MSS Build Info*/
#define MSS_BUILD_OFFSET   0x28
    /* Set to the SVN revision of the g5_mss_main.sv file*/
    #define MSS_BUILD_REVISION_OFFSET  0x0
    #define MSS_BUILD_REVISION_MASK    (0xFFFFFFFF << 0x0)

/*U54-1 Fabric interrupt enable*/
#define FAB_INTEN_U54_1_OFFSET   0x40
    /* Enables the F2H_interrupts[31:0] to interrupt U54_1 directly*/
    #define FAB_INTEN_U54_1_ENABLE_OFFSET  0x0
    #define FAB_INTEN_U54_1_ENABLE_MASK    (0xFFFFFFFF << 0x0)

/*U54-2 Fabric interrupt enable*/
#define FAB_INTEN_U54_2_OFFSET   0x44
    /* Enables the F2H_interrupts[31:0] to interrupt U54_1 directly*/
    #define FAB_INTEN_U54_2_ENABLE_OFFSET  0x0
    #define FAB_INTEN_U54_2_ENABLE_MASK    (0xFFFFFFFF << 0x0)

/*U54-3 Fabric interrupt enable*/
#define FAB_INTEN_U54_3_OFFSET   0x48
    /* Enables the F2H_interrupts[31:0] to interrupt U54_1 directly*/
    #define FAB_INTEN_U54_3_ENABLE_OFFSET  0x0
    #define FAB_INTEN_U54_3_ENABLE_MASK    (0xFFFFFFFF << 0x0)

/*U54-4 Fabric interrupt enable*/
#define FAB_INTEN_U54_4_OFFSET   0x4C
    /* Enables the F2H_interrupts[31:0] to interrupt U54_1 directly*/
    #define FAB_INTEN_U54_4_ENABLE_OFFSET  0x0
    #define FAB_INTEN_U54_4_ENABLE_MASK    (0xFFFFFFFF << 0x0)

/*Allows the Ethernat interrupts to be directly routed to the U54 CPUS.*/
#define FAB_INTEN_MISC_OFFSET   0x50
    /* Enables the Ethernet MAC0 to interrupt U54_1 directly*/
    #define FAB_INTEN_MISC_MAC0_U54_1_OFFSET  0x0
    #define FAB_INTEN_MISC_MAC0_U54_1_MASK    (0x01 << 0x0)
    /* Enables the Ethernet MAC0 to interrupt U54_1 directly*/
    #define FAB_INTEN_MISC_MAC0_U54_2_OFFSET  0x1
    #define FAB_INTEN_MISC_MAC0_U54_2_MASK    (0x01 << 0x1)
    /* Enables the Ethernet MAC1 to interrupt U54_1 directly*/
    #define FAB_INTEN_MISC_MAC1_U54_3_OFFSET  0x2
    #define FAB_INTEN_MISC_MAC1_U54_3_MASK    (0x01 << 0x2)
    /* Enables the Ethernet MAC1 to interrupt U54_1 directly*/
    #define FAB_INTEN_MISC_MAC1_U54_4_OFFSET  0x3
    #define FAB_INTEN_MISC_MAC1_U54_4_MASK    (0x01 << 0x3)

/*Switches GPIO interrupt from PAD to Fabric GPIO*/
#define GPIO_INTERRUPT_FAB_CR_OFFSET   0x54
    /* Setting these  bits will disable the Pad interrupt and enable the fabric GPIO interrupt for bits 31:0. When the bit is set the Pad interrupt will be ORED into the GPIO0 & GPIO1  non-direct  interrupts. When the bit is not set the Fabric interrupt is ORED into the GPIO2  non-direct interrupt. To prevent ORING then the interrupt should not be enabled in the GPIO block*/
    #define GPIO_INTERRUPT_FAB_CR_select_OFFSET  0x0
    #define GPIO_INTERRUPT_FAB_CR_select_MASK    (0xFFFFFFFF << 0x0)

/*"AMP Mode peripheral mapping register. When the register bit is '0' the peripheral is mapped into the 0x2000000 address range using AXI bus 5 from the Coreplex. When the register bit is '1' the peripheral is mapped into the 0x28000000 address range using AXI bus 6 from the Coreplex."*/
#define APBBUS_CR_OFFSET   0x80
    /* */
    #define APBBUS_CR_mmuart0_OFFSET  0x0
    #define APBBUS_CR_mmuart0_MASK    (0x01 << 0x0)
    /* */
    #define APBBUS_CR_mmuart1_OFFSET  0x1
    #define APBBUS_CR_mmuart1_MASK    (0x01 << 0x1)
    /* */
    #define APBBUS_CR_mmuart2_OFFSET  0x2
    #define APBBUS_CR_mmuart2_MASK    (0x01 << 0x2)
    /* */
    #define APBBUS_CR_mmuart3_OFFSET  0x3
    #define APBBUS_CR_mmuart3_MASK    (0x01 << 0x3)
    /* */
    #define APBBUS_CR_mmuart4_OFFSET  0x4
    #define APBBUS_CR_mmuart4_MASK    (0x01 << 0x4)
    /* */
    #define APBBUS_CR_wdog0_OFFSET  0x5
    #define APBBUS_CR_wdog0_MASK    (0x01 << 0x5)
    /* */
    #define APBBUS_CR_wdog1_OFFSET  0x6
    #define APBBUS_CR_wdog1_MASK    (0x01 << 0x6)
    /* */
    #define APBBUS_CR_wdog2_OFFSET  0x7
    #define APBBUS_CR_wdog2_MASK    (0x01 << 0x7)
    /* */
    #define APBBUS_CR_wdog3_OFFSET  0x8
    #define APBBUS_CR_wdog3_MASK    (0x01 << 0x8)
    /* */
    #define APBBUS_CR_wdog4_OFFSET  0x9
    #define APBBUS_CR_wdog4_MASK    (0x01 << 0x9)
    /* */
    #define APBBUS_CR_spi0_OFFSET  0xA
    #define APBBUS_CR_spi0_MASK    (0x01 << 0xA)
    /* */
    #define APBBUS_CR_spi1_OFFSET  0xB
    #define APBBUS_CR_spi1_MASK    (0x01 << 0xB)
    /* */
    #define APBBUS_CR_i2c0_OFFSET  0xC
    #define APBBUS_CR_i2c0_MASK    (0x01 << 0xC)
    /* */
    #define APBBUS_CR_i2c1_OFFSET  0xD
    #define APBBUS_CR_i2c1_MASK    (0x01 << 0xD)
    /* */
    #define APBBUS_CR_can0_OFFSET  0xE
    #define APBBUS_CR_can0_MASK    (0x01 << 0xE)
    /* */
    #define APBBUS_CR_can1_OFFSET  0xF
    #define APBBUS_CR_can1_MASK    (0x01 << 0xF)
    /* */
    #define APBBUS_CR_gem0_OFFSET  0x10
    #define APBBUS_CR_gem0_MASK    (0x01 << 0x10)
    /* */
    #define APBBUS_CR_gem1_OFFSET  0x11
    #define APBBUS_CR_gem1_MASK    (0x01 << 0x11)
    /* */
    #define APBBUS_CR_timer_OFFSET  0x12
    #define APBBUS_CR_timer_MASK    (0x01 << 0x12)
    /* */
    #define APBBUS_CR_gpio0_OFFSET  0x13
    #define APBBUS_CR_gpio0_MASK    (0x01 << 0x13)
    /* */
    #define APBBUS_CR_gpio1_OFFSET  0x14
    #define APBBUS_CR_gpio1_MASK    (0x01 << 0x14)
    /* */
    #define APBBUS_CR_gpio2_OFFSET  0x15
    #define APBBUS_CR_gpio2_MASK    (0x01 << 0x15)
    /* */
    #define APBBUS_CR_rtc_OFFSET  0x16
    #define APBBUS_CR_rtc_MASK    (0x01 << 0x16)
    /* */
    #define APBBUS_CR_h2fint_OFFSET  0x17
    #define APBBUS_CR_h2fint_MASK    (0x01 << 0x17)

/*"Enables the clock to the MSS peripheral. By turning clocks off dynamic power can be saved. When the clock is off the peripheral  should not be accessed the acess may be ignored return unspecified data or result in bus response error."*/
#define SUBBLK_CLOCK_CR_OFFSET   0x84
    /* */
    #define SUBBLK_CLOCK_CR_ENVM_OFFSET  0x0
    #define SUBBLK_CLOCK_CR_ENVM_MASK    (0x01 << 0x0)
    /* */
    #define SUBBLK_CLOCK_CR_MAC0_OFFSET  0x1
    #define SUBBLK_CLOCK_CR_MAC0_MASK    (0x01 << 0x1)
    /* */
    #define SUBBLK_CLOCK_CR_MAC1_OFFSET  0x2
    #define SUBBLK_CLOCK_CR_MAC1_MASK    (0x01 << 0x2)
    /* */
    #define SUBBLK_CLOCK_CR_MMC_OFFSET  0x3
    #define SUBBLK_CLOCK_CR_MMC_MASK    (0x01 << 0x3)
    /* */
    #define SUBBLK_CLOCK_CR_TIMER_OFFSET  0x4
    #define SUBBLK_CLOCK_CR_TIMER_MASK    (0x01 << 0x4)
    /* */
    #define SUBBLK_CLOCK_CR_MMUART0_OFFSET  0x5
    #define SUBBLK_CLOCK_CR_MMUART0_MASK    (0x01 << 0x5)
    /* */
    #define SUBBLK_CLOCK_CR_MMUART1_OFFSET  0x6
    #define SUBBLK_CLOCK_CR_MMUART1_MASK    (0x01 << 0x6)
    /* */
    #define SUBBLK_CLOCK_CR_MMUART2_OFFSET  0x7
    #define SUBBLK_CLOCK_CR_MMUART2_MASK    (0x01 << 0x7)
    /* */
    #define SUBBLK_CLOCK_CR_MMUART3_OFFSET  0x8
    #define SUBBLK_CLOCK_CR_MMUART3_MASK    (0x01 << 0x8)
    /* */
    #define SUBBLK_CLOCK_CR_MMUART4_OFFSET  0x9
    #define SUBBLK_CLOCK_CR_MMUART4_MASK    (0x01 << 0x9)
    /* */
    #define SUBBLK_CLOCK_CR_SPI0_OFFSET  0xA
    #define SUBBLK_CLOCK_CR_SPI0_MASK    (0x01 << 0xA)
    /* */
    #define SUBBLK_CLOCK_CR_SPI1_OFFSET  0xB
    #define SUBBLK_CLOCK_CR_SPI1_MASK    (0x01 << 0xB)
    /* */
    #define SUBBLK_CLOCK_CR_I2C0_OFFSET  0xC
    #define SUBBLK_CLOCK_CR_I2C0_MASK    (0x01 << 0xC)
    /* */
    #define SUBBLK_CLOCK_CR_I2C1_OFFSET  0xD
    #define SUBBLK_CLOCK_CR_I2C1_MASK    (0x01 << 0xD)
    /* */
    #define SUBBLK_CLOCK_CR_CAN0_OFFSET  0xE
    #define SUBBLK_CLOCK_CR_CAN0_MASK    (0x01 << 0xE)
    /* */
    #define SUBBLK_CLOCK_CR_CAN1_OFFSET  0xF
    #define SUBBLK_CLOCK_CR_CAN1_MASK    (0x01 << 0xF)
    /* */
    #define SUBBLK_CLOCK_CR_USB_OFFSET  0x10
    #define SUBBLK_CLOCK_CR_USB_MASK    (0x01 << 0x10)
    /* */
    #define SUBBLK_CLOCK_CR_RSVD_OFFSET  0x11
    #define SUBBLK_CLOCK_CR_RSVD_MASK    (0x01 << 0x11)
    /* */
    #define SUBBLK_CLOCK_CR_RTC_OFFSET  0x12
    #define SUBBLK_CLOCK_CR_RTC_MASK    (0x01 << 0x12)
    /* */
    #define SUBBLK_CLOCK_CR_QSPI_OFFSET  0x13
    #define SUBBLK_CLOCK_CR_QSPI_MASK    (0x01 << 0x13)
    /* */
    #define SUBBLK_CLOCK_CR_GPIO0_OFFSET  0x14
    #define SUBBLK_CLOCK_CR_GPIO0_MASK    (0x01 << 0x14)
    /* */
    #define SUBBLK_CLOCK_CR_GPIO1_OFFSET  0x15
    #define SUBBLK_CLOCK_CR_GPIO1_MASK    (0x01 << 0x15)
    /* */
    #define SUBBLK_CLOCK_CR_GPIO2_OFFSET  0x16
    #define SUBBLK_CLOCK_CR_GPIO2_MASK    (0x01 << 0x16)
    /* */
    #define SUBBLK_CLOCK_CR_DDRC_OFFSET  0x17
    #define SUBBLK_CLOCK_CR_DDRC_MASK    (0x01 << 0x17)
    /* */
    #define SUBBLK_CLOCK_CR_FIC0_OFFSET  0x18
    #define SUBBLK_CLOCK_CR_FIC0_MASK    (0x01 << 0x18)
    /* */
    #define SUBBLK_CLOCK_CR_FIC1_OFFSET  0x19
    #define SUBBLK_CLOCK_CR_FIC1_MASK    (0x01 << 0x19)
    /* */
    #define SUBBLK_CLOCK_CR_FIC2_OFFSET  0x1A
    #define SUBBLK_CLOCK_CR_FIC2_MASK    (0x01 << 0x1A)
    /* */
    #define SUBBLK_CLOCK_CR_FIC3_OFFSET  0x1B
    #define SUBBLK_CLOCK_CR_FIC3_MASK    (0x01 << 0x1B)
    /* */
    #define SUBBLK_CLOCK_CR_ATHENA_OFFSET  0x1C
    #define SUBBLK_CLOCK_CR_ATHENA_MASK    (0x01 << 0x1C)
    /* */
    #define SUBBLK_CLOCK_CR_CFM_OFFSET  0x1D
    #define SUBBLK_CLOCK_CR_CFM_MASK    (0x01 << 0x1D)

/*"Holds the MSS peripherals in reset. Whenin reset the peripheral  should not be accessed the acess may be ignored return unspecified data or result in bus response error."*/
#define SOFT_RESET_CR_OFFSET   0x88
    /* */
    #define SOFT_RESET_CR_ENVM_OFFSET  0x0
    #define SOFT_RESET_CR_ENVM_MASK    (0x01 << 0x0)
    /* */
    #define SOFT_RESET_CR_MAC0_OFFSET  0x1
    #define SOFT_RESET_CR_MAC0_MASK    (0x01 << 0x1)
    /* */
    #define SOFT_RESET_CR_MAC1_OFFSET  0x2
    #define SOFT_RESET_CR_MAC1_MASK    (0x01 << 0x2)
    /* */
    #define SOFT_RESET_CR_MMC_OFFSET  0x3
    #define SOFT_RESET_CR_MMC_MASK    (0x01 << 0x3)
    /* */
    #define SOFT_RESET_CR_TIMER_OFFSET  0x4
    #define SOFT_RESET_CR_TIMER_MASK    (0x01 << 0x4)
    /* */
    #define SOFT_RESET_CR_MMUART0_OFFSET  0x5
    #define SOFT_RESET_CR_MMUART0_MASK    (0x01 << 0x5)
    /* */
    #define SOFT_RESET_CR_MMUART1_OFFSET  0x6
    #define SOFT_RESET_CR_MMUART1_MASK    (0x01 << 0x6)
    /* */
    #define SOFT_RESET_CR_MMUART2_OFFSET  0x7
    #define SOFT_RESET_CR_MMUART2_MASK    (0x01 << 0x7)
    /* */
    #define SOFT_RESET_CR_MMUART3_OFFSET  0x8
    #define SOFT_RESET_CR_MMUART3_MASK    (0x01 << 0x8)
    /* */
    #define SOFT_RESET_CR_MMUART4_OFFSET  0x9
    #define SOFT_RESET_CR_MMUART4_MASK    (0x01 << 0x9)
    /* */
    #define SOFT_RESET_CR_SPI0_OFFSET  0xA
    #define SOFT_RESET_CR_SPI0_MASK    (0x01 << 0xA)
    /* */
    #define SOFT_RESET_CR_SPI1_OFFSET  0xB
    #define SOFT_RESET_CR_SPI1_MASK    (0x01 << 0xB)
    /* */
    #define SOFT_RESET_CR_I2C0_OFFSET  0xC
    #define SOFT_RESET_CR_I2C0_MASK    (0x01 << 0xC)
    /* */
    #define SOFT_RESET_CR_I2C1_OFFSET  0xD
    #define SOFT_RESET_CR_I2C1_MASK    (0x01 << 0xD)
    /* */
    #define SOFT_RESET_CR_CAN0_OFFSET  0xE
    #define SOFT_RESET_CR_CAN0_MASK    (0x01 << 0xE)
    /* */
    #define SOFT_RESET_CR_CAN1_OFFSET  0xF
    #define SOFT_RESET_CR_CAN1_MASK    (0x01 << 0xF)
    /* */
    #define SOFT_RESET_CR_USB_OFFSET  0x10
    #define SOFT_RESET_CR_USB_MASK    (0x01 << 0x10)
    /* */
    #define SOFT_RESET_CR_FPGA_OFFSET  0x11
    #define SOFT_RESET_CR_FPGA_MASK    (0x01 << 0x11)
    /* */
    #define SOFT_RESET_CR_RTC_OFFSET  0x12
    #define SOFT_RESET_CR_RTC_MASK    (0x01 << 0x12)
    /* */
    #define SOFT_RESET_CR_QSPI_OFFSET  0x13
    #define SOFT_RESET_CR_QSPI_MASK    (0x01 << 0x13)
    /* */
    #define SOFT_RESET_CR_GPIO0_OFFSET  0x14
    #define SOFT_RESET_CR_GPIO0_MASK    (0x01 << 0x14)
    /* */
    #define SOFT_RESET_CR_GPIO1_OFFSET  0x15
    #define SOFT_RESET_CR_GPIO1_MASK    (0x01 << 0x15)
    /* */
    #define SOFT_RESET_CR_GPIO2_OFFSET  0x16
    #define SOFT_RESET_CR_GPIO2_MASK    (0x01 << 0x16)
    /* */
    #define SOFT_RESET_CR_DDRC_OFFSET  0x17
    #define SOFT_RESET_CR_DDRC_MASK    (0x01 << 0x17)
    /* */
    #define SOFT_RESET_CR_FIC0_OFFSET  0x18
    #define SOFT_RESET_CR_FIC0_MASK    (0x01 << 0x18)
    /* */
    #define SOFT_RESET_CR_FIC1_OFFSET  0x19
    #define SOFT_RESET_CR_FIC1_MASK    (0x01 << 0x19)
    /* */
    #define SOFT_RESET_CR_FIC2_OFFSET  0x1A
    #define SOFT_RESET_CR_FIC2_MASK    (0x01 << 0x1A)
    /* */
    #define SOFT_RESET_CR_FIC3_OFFSET  0x1B
    #define SOFT_RESET_CR_FIC3_MASK    (0x01 << 0x1B)
    /* */
    #define SOFT_RESET_CR_ATHENA_OFFSET  0x1C
    #define SOFT_RESET_CR_ATHENA_MASK    (0x01 << 0x1C)
    /* */
    #define SOFT_RESET_CR_CFM_OFFSET  0x1D
    #define SOFT_RESET_CR_CFM_MASK    (0x01 << 0x1D)
    /* Reset to Corner SGMII block*/
    #define SOFT_RESET_CR_SGMII_OFFSET  0x1E
    #define SOFT_RESET_CR_SGMII_MASK    (0x01 << 0x1E)

/*Configures how many outstanding transfers the AXI-AHB bridges in front off the USB and Crypto blocks should allow. (See Synopsys AXI-AHB bridge documentation)*/
#define AHBAXI_CR_OFFSET   0x8C
    /* Number of outstanding write transactions to USB block*/
    #define AHBAXI_CR_USB_WBCNT_OFFSET  0x0
    #define AHBAXI_CR_USB_WBCNT_MASK    (0x0F << 0x0)
    /* Number of outstanding read transactions to USB block*/
    #define AHBAXI_CR_USB_RBCNT_OFFSET  0x4
    #define AHBAXI_CR_USB_RBCNT_MASK    (0x0F << 0x4)
    /* Number of outstanding write transactions to Athena block*/
    #define AHBAXI_CR_ATHENA_WBCNT_OFFSET  0x8
    #define AHBAXI_CR_ATHENA_WBCNT_MASK    (0x0F << 0x8)
    /* Number of outstanding read transactions to Athena block*/
    #define AHBAXI_CR_ATHENA_RBCNT_OFFSET  0xC
    #define AHBAXI_CR_ATHENA_RBCNT_MASK    (0x0F << 0xC)

/*Configures the two AHB-APB bridges on S5 and S6*/
#define AHBAPB_CR_OFFSET   0x90
    /* Enables posted mode on the AHB-APB bridge when set the AHB write cycle will complete before the APB write cycle completes.*/
    #define AHBAPB_CR_APB0_POSTED_OFFSET  0x0
    #define AHBAPB_CR_APB0_POSTED_MASK    (0x01 << 0x0)
    /* Enables posted mode on the AHB-APB bridge when set the AHB write cycle will complete before the APB write cycle completes.*/
    #define AHBAPB_CR_APB1_POSTED_OFFSET  0x1
    #define AHBAPB_CR_APB1_POSTED_MASK    (0x01 << 0x1)

/*MSS Corner APB interface controls*/
#define DFIAPB_CR_OFFSET   0x98
    /* Turns on the APB clock to the MSS Corner is off at reset. Once corner blocks is configured the firmware may turn off the clock but periodically should turn back on to allow refresh of TMR registers inside the corner block. */
    #define DFIAPB_CR_clockon_OFFSET  0x0
    #define DFIAPB_CR_clockon_MASK    (0x01 << 0x0)
    /* Asserts the APB reset to the MSS corner is asserted at MSS reset.*/
    #define DFIAPB_CR_reset_OFFSET  0x1
    #define DFIAPB_CR_reset_MASK    (0x01 << 0x1)

/*GPIO Blocks reset control*/
#define GPIO_CR_OFFSET   0x9C
    /* "This signal selects whether the associated byte is reset by soft reset or the the MSS_GPIO_RESET_N signal from the FPGA fabric. The allowed values are:• 0: Selects  MSS_GPIO_RESET_N signal from the FPGA fabric.• 1: Selects  the GPIO to be reset by the GPIO block soft reset signal .Bit 0 controls GPIO0 [7:0]  and bit  1 GPIO[15:8]The master MSS reset will also reset the GPIO register if not configured to use fabric reset."*/
    #define GPIO_CR_gpio0_soft_reset_select_OFFSET  0x0
    #define GPIO_CR_gpio0_soft_reset_select_MASK    (0x03 << 0x0)
    /* "Sets the reset value off the GPIO0 per byteBit 0 controls GPIO0 [7:0]  and bit  1 GPIO[15:8]"*/
    #define GPIO_CR_gpio0_default_OFFSET  0x4
    #define GPIO_CR_gpio0_default_MASK    (0x03 << 0x4)
    /* "This signal selects whether the associated byte is reset by soft reset or the the MSS_GPIO_RESET_N signal from the FPGA fabric. The allowed values are:• 0: Selects  MSS_GPIO_RESET_N signal from the FPGA fabric.• 1: Selects  the GPIO to be reset by the GPIO block soft reset signal .Bit 0 controls GPIO0 [7:0] bit  1 GPIO[15:8] and bit 2 GPIO[23:16]The master MSS reset will also reset the GPIO register if not configured to use fabric reset."*/
    #define GPIO_CR_gpio1_soft_reset_select_OFFSET  0x8
    #define GPIO_CR_gpio1_soft_reset_select_MASK    (0x07 << 0x8)
    /* "Sets the reset value off the GPIO0 per byteBit 0 controls GPIO0 [7:0] bit  1 GPIO[15:8] and bit 2 GPIO[23:16]"*/
    #define GPIO_CR_gpio1_default_OFFSET  0xC
    #define GPIO_CR_gpio1_default_MASK    (0x07 << 0xC)
    /* "This signal selects whether the associated byte is reset by soft reset or the the MSS_GPIO_RESET_N signal from the FPGA fabric. The allowed values are:• 0: Selects  MSS_GPIO_RESET_N signal from the FPGA fabric.• 1: Selects  the GPIO to be reset by the GPIO block soft reset signal .Bit 0 controls GPIO0 [7:0] bit  1 GPIO[15:8] and bit 1 GPIO[23:16] and bit 3 GPIO[31:24]The master MSS reset will also reset the GPIO register if not configured to use fabric reset."*/
    #define GPIO_CR_gpio2_soft_reset_select_OFFSET  0x10
    #define GPIO_CR_gpio2_soft_reset_select_MASK    (0x0F << 0x10)
    /* "Sets the reset value off the GPIO0 per byteBit 0 controls GPIO0 [7:0] bit  1 GPIO[15:8] and bit 1 GPIO[23:16] and bit 3 GPIO[31:24]"*/
    #define GPIO_CR_gpio2_default_OFFSET  0x14
    #define GPIO_CR_gpio2_default_MASK    (0x0F << 0x14)

/*MAC0 configuration register*/
#define MAC0_CR_OFFSET   0xA4
    /* Current speed mode on the MAC*/
    #define MAC0_CR_speed_mode_OFFSET  0x0
    #define MAC0_CR_speed_mode_MASK    (0x0F << 0x0)

/*MAC1 configuration register*/
#define MAC1_CR_OFFSET   0xA8
    /* Current speed mode on the MAC*/
    #define MAC1_CR_speed_mode_OFFSET  0x0
    #define MAC1_CR_speed_mode_MASK    (0x0F << 0x0)

/*USB Configuration register*/
#define USB_CR_OFFSET   0xAC
    /* "Configures USB for Single-Data Rate(SDR) mode or Double-Data Rate(DDR) mode. 0 - SDR Mode is selected1 - DDR Mode is selected (Not supported in G5 or G5)"*/
    #define USB_CR_DDR_SELECT_OFFSET  0x0
    #define USB_CR_DDR_SELECT_MASK    (0x01 << 0x0)
    /* When '1' will stops the clock to the USB core when the core asserts its POWERDOWN output. For G4 compatibility this bit defaults to 0.*/
    #define USB_CR_powerdown_enable_OFFSET  0x1
    #define USB_CR_powerdown_enable_MASK    (0x01 << 0x1)
    /* Indicates that the USB CLK may be stopped to save power. Derived from combination of signals from CLK & XCLK flip-flops AVALID VBUSVALID and LINESTATE. When asserted the USB clock into the core is stopped.*/
    #define USB_CR_POWERDOWN_OFFSET  0x2
    #define USB_CR_POWERDOWN_MASK    (0x01 << 0x2)
    /* Set when entry is made into CarKit mode and cleared on exit from CarKit mode.*/
    #define USB_CR_LPI_CARKIT_EN_OFFSET  0x3
    #define USB_CR_LPI_CARKIT_EN_MASK    (0x01 << 0x3)

/*Crypto Mesh control and status register*/
#define MESH_CR_OFFSET   0xB0
    /* Writing a 1 will start the Mesh System*/
    #define MESH_CR_START_OFFSET  0x0
    #define MESH_CR_START_MASK    (0x01 << 0x0)
    /* "Sets the amount of time that the mesh is held active for actual hold time includes up to 256 us of random variation.Minimum Time = 1 + 256 * value   usMaximum Time = 1 +  256 * (1+value)   usValue must be greater than 0"*/
    #define MESH_CR_HOLD_OFFSET  0x1
    #define MESH_CR_HOLD_MASK    (0xFFF << 0x1)
    /* When set will inject an error in the mesh*/
    #define MESH_CR_INJECT_ERROR_OFFSET  0x10
    #define MESH_CR_INJECT_ERROR_MASK    (0x01 << 0x10)
    /* Indicates that Mesh detected an error. Cleared by writing a '1'*/
    #define MESH_CR_MESH_ERROR_OFFSET  0x18
    #define MESH_CR_MESH_ERROR_MASK    (0x01 << 0x18)
    /* Indicates that the Mesh is functioning correctly. Will be set approximately  520 clock cycles after mesh started and stay set as long as the mesh is not detecting any errors.*/
    #define MESH_CR_OKAY_OFFSET  0x19
    #define MESH_CR_OKAY_MASK    (0x01 << 0x19)

/*Crypto mesh seed and update rate*/
#define MESH_SEED_CR_OFFSET   0xB4
    /* Sets the mesh seed value any value may be used zero should be avoided*/
    #define MESH_SEED_CR_seed_OFFSET  0x0
    #define MESH_SEED_CR_seed_MASK    (0x7FFFFF << 0x0)
    /* Sets the rate that the mesh value is changed. Rate = AHBCLK/(clkrate+1). Rate must be less than 1MHz setting slower will reduce power consumption.*/
    #define MESH_SEED_CR_clkrate_OFFSET  0x18
    #define MESH_SEED_CR_clkrate_MASK    (0xFF << 0x18)

/*ENVM AHB Controller setup*/
#define ENVM_CR_OFFSET   0xB8
    /* "Sets the number of  AHB cycles used to generate the PNVM clockClock  period = (Value+1) * (1000/AHBFREQMHZ)         Value must be 1 to 63  (0 defaults to 15)e.g.11  will generate a 40ns period  25MHz clock if the AHB clock is 250MHz15  will generate a 40ns period  25MHz clock if the AHB clock is 400MHz"*/
    #define ENVM_CR_clock_period_OFFSET  0x0
    #define ENVM_CR_clock_period_MASK    (0x3F << 0x0)
    /* Indicates the eNVM is running at the configured divider rate. */
    #define ENVM_CR_clock_okay_OFFSET  0x6
    #define ENVM_CR_clock_okay_MASK    (0x01 << 0x6)
    /* When '1' the PNVM clock will be always generated and not stopped between access cycles. Setting this will increase access latency but mean that the PNVM clock operates at a stable rate.*/
    #define ENVM_CR_clock_continuous_OFFSET  0x8
    #define ENVM_CR_clock_continuous_MASK    (0x01 << 0x8)
    /* When set suppresses clock edge between C-Bus access cycles so that they appear as consecutive access cycles.*/
    #define ENVM_CR_clock_suppress_OFFSET  0x9
    #define ENVM_CR_clock_suppress_MASK    (0x01 << 0x9)
    /* "Enables ""read-ahead"" on the ENVM controller. The controller will automatically read the next PNVM location as soon as possible ahead of the AHB request. This will improve read performance when incrementing though memory as the NVM reads and AHB cycles are pipelined. When set  non incrementing accesses will take longer as the controller may be in the process of reading the next address and the PNVM cycle needs to complete prior to starting the required read"*/
    #define ENVM_CR_readahead_OFFSET  0x10
    #define ENVM_CR_readahead_MASK    (0x01 << 0x10)
    /* When '1' the controller will initiate separate ENVM reads for all reads. No buffering or speculative operations will be carried out. When performing word reads incrementing through PNVM each location will  be read twice (intended for test use)*/
    #define ENVM_CR_slowread_OFFSET  0x11
    #define ENVM_CR_slowread_MASK    (0x01 << 0x11)
    /* Enable the ENVM interrupt*/
    #define ENVM_CR_interrupt_enable_OFFSET  0x12
    #define ENVM_CR_interrupt_enable_MASK    (0x01 << 0x12)
    /* "Sets the duration of the timer used to detect a non response of slow response from the PNVM on C and R bus accesses.Timer Duration = Value *  (1000/AHBFREQMHZ)   0x00: Timer disabled. If the timer expires the AHB cycle is terminates using the HRESP protocol"*/
    #define ENVM_CR_timer_OFFSET  0x18
    #define ENVM_CR_timer_MASK    (0xFF << 0x18)

/*Reserved*/
#define RESERVED_BC_OFFSET   0xBC
    /* Reserved register address*/
    #define RESERVED_BC_Reserved_OFFSET  0x0
    #define RESERVED_BC_Reserved_MASK    (0x01 << 0x0)

/*QOS Athena USB & MMC Configuration*/
#define qos_peripheral_cr_OFFSET   0xC0
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_peripheral_cr_athena_read_OFFSET  0x0
    #define qos_peripheral_cr_athena_read_MASK    (0x0F << 0x0)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_peripheral_cr_athena_write_OFFSET  0x4
    #define qos_peripheral_cr_athena_write_MASK    (0x0F << 0x4)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_peripheral_cr_usb_read_OFFSET  0x8
    #define qos_peripheral_cr_usb_read_MASK    (0x0F << 0x8)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_peripheral_cr_usb_write_OFFSET  0xC
    #define qos_peripheral_cr_usb_write_MASK    (0x0F << 0xC)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_peripheral_cr_mmc_read_OFFSET  0x10
    #define qos_peripheral_cr_mmc_read_MASK    (0x0F << 0x10)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_peripheral_cr_mmc_write_OFFSET  0x14
    #define qos_peripheral_cr_mmc_write_MASK    (0x0F << 0x14)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_peripheral_cr_trace_read_OFFSET  0x18
    #define qos_peripheral_cr_trace_read_MASK    (0x0F << 0x18)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_peripheral_cr_trace_write_OFFSET  0x1C
    #define qos_peripheral_cr_trace_write_MASK    (0x0F << 0x1C)

/*QOS Configuration Coreplex*/
#define qos_cplexio_cr_OFFSET   0xC4
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_cplexio_cr_device0_read_OFFSET  0x0
    #define qos_cplexio_cr_device0_read_MASK    (0x0F << 0x0)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_cplexio_cr_device0_write_OFFSET  0x4
    #define qos_cplexio_cr_device0_write_MASK    (0x0F << 0x4)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_cplexio_cr_device1_read_OFFSET  0x8
    #define qos_cplexio_cr_device1_read_MASK    (0x0F << 0x8)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_cplexio_cr_device1_write_OFFSET  0xC
    #define qos_cplexio_cr_device1_write_MASK    (0x0F << 0xC)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_cplexio_cr_fabric0_read_OFFSET  0x10
    #define qos_cplexio_cr_fabric0_read_MASK    (0x0F << 0x10)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_cplexio_cr_fabric0_write_OFFSET  0x14
    #define qos_cplexio_cr_fabric0_write_MASK    (0x0F << 0x14)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_cplexio_cr_fabric1_read_OFFSET  0x18
    #define qos_cplexio_cr_fabric1_read_MASK    (0x0F << 0x18)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_cplexio_cr_fabric1_write_OFFSET  0x1C
    #define qos_cplexio_cr_fabric1_write_MASK    (0x0F << 0x1C)

/*QOS configuration DDRC*/
#define qos_cplexddr_cr_OFFSET   0xC8
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_cplexddr_cr_cache_read_OFFSET  0x0
    #define qos_cplexddr_cr_cache_read_MASK    (0x0F << 0x0)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_cplexddr_cr_cache_write_OFFSET  0x4
    #define qos_cplexddr_cr_cache_write_MASK    (0x0F << 0x4)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_cplexddr_cr_ncache_read_OFFSET  0x8
    #define qos_cplexddr_cr_ncache_read_MASK    (0x0F << 0x8)
    /* Sets the QOS value from the specified device into the switch*/
    #define qos_cplexddr_cr_ncache_write_OFFSET  0xC
    #define qos_cplexddr_cr_ncache_write_MASK    (0x0F << 0xC)

/*Indicates that a master caused a MPU violation. Interrupts via maintenance interrupt.*/
#define mpu_violation_sr_OFFSET   0xF0
    /* Bit is set on violation. Cleared by writing '1'*/
    #define mpu_violation_sr_fic0_OFFSET  0x0
    #define mpu_violation_sr_fic0_MASK    (0x01 << 0x0)
    /* Bit is set on violation. Cleared by writing '1'*/
    #define mpu_violation_sr_fic1_OFFSET  0x1
    #define mpu_violation_sr_fic1_MASK    (0x01 << 0x1)
    /* Bit is set on violation. Cleared by writing '1'*/
    #define mpu_violation_sr_fic2_OFFSET  0x2
    #define mpu_violation_sr_fic2_MASK    (0x01 << 0x2)
    /* Bit is set on violation. Cleared by writing '1'*/
    #define mpu_violation_sr_athena_OFFSET  0x3
    #define mpu_violation_sr_athena_MASK    (0x01 << 0x3)
    /* Bit is set on violation. Cleared by writing '1'*/
    #define mpu_violation_sr_gem0_OFFSET  0x4
    #define mpu_violation_sr_gem0_MASK    (0x01 << 0x4)
    /* Bit is set on violation. Cleared by writing '1'*/
    #define mpu_violation_sr_gem1_OFFSET  0x5
    #define mpu_violation_sr_gem1_MASK    (0x01 << 0x5)
    /* Bit is set on violation. Cleared by writing '1'*/
    #define mpu_violation_sr_usb_OFFSET  0x6
    #define mpu_violation_sr_usb_MASK    (0x01 << 0x6)
    /* Bit is set on violation. Cleared by writing '1'*/
    #define mpu_violation_sr_mmc_OFFSET  0x7
    #define mpu_violation_sr_mmc_MASK    (0x01 << 0x7)
    /* Bit is set on violation. Cleared by writing '1'*/
    #define mpu_violation_sr_scb_OFFSET  0x8
    #define mpu_violation_sr_scb_MASK    (0x01 << 0x8)
    /* Bit is set on violation. Cleared by writing '1'*/
    #define mpu_violation_sr_trace_OFFSET  0x9
    #define mpu_violation_sr_trace_MASK    (0x01 << 0x9)

/*Enables interrupts on MPU violations*/
#define mpu_violation_inten_cr_OFFSET   0xF4
    /* Enables the interrupt*/
    #define mpu_violation_inten_cr_fic0_OFFSET  0x0
    #define mpu_violation_inten_cr_fic0_MASK    (0x01 << 0x0)
    /* Enables the interrupt*/
    #define mpu_violation_inten_cr_fic1_OFFSET  0x1
    #define mpu_violation_inten_cr_fic1_MASK    (0x01 << 0x1)
    /* Enables the interrupt*/
    #define mpu_violation_inten_cr_fic2_OFFSET  0x2
    #define mpu_violation_inten_cr_fic2_MASK    (0x01 << 0x2)
    /* Enables the interrupt*/
    #define mpu_violation_inten_cr_athena_OFFSET  0x3
    #define mpu_violation_inten_cr_athena_MASK    (0x01 << 0x3)
    /* Enables the interrupt*/
    #define mpu_violation_inten_cr_gem0_OFFSET  0x4
    #define mpu_violation_inten_cr_gem0_MASK    (0x01 << 0x4)
    /* Enables the interrupt*/
    #define mpu_violation_inten_cr_gem1_OFFSET  0x5
    #define mpu_violation_inten_cr_gem1_MASK    (0x01 << 0x5)
    /* Enables the interrupt*/
    #define mpu_violation_inten_cr_usb_OFFSET  0x6
    #define mpu_violation_inten_cr_usb_MASK    (0x01 << 0x6)
    /* Enables the interrupt*/
    #define mpu_violation_inten_cr_mmc_OFFSET  0x7
    #define mpu_violation_inten_cr_mmc_MASK    (0x01 << 0x7)
    /* Enables the interrupt*/
    #define mpu_violation_inten_cr_scb_OFFSET  0x8
    #define mpu_violation_inten_cr_scb_MASK    (0x01 << 0x8)
    /* Enables the interrupt*/
    #define mpu_violation_inten_cr_trace_OFFSET  0x9
    #define mpu_violation_inten_cr_trace_MASK    (0x01 << 0x9)

/*AXI switch decode fail*/
#define SW_FAIL_ADDR0_CR_OFFSET   0xF8
    /* The address (bits 31:0) that failed. Reading this address as 64-bits will return the 38-bit address in a single read combined with additional information in the next register*/
    #define SW_FAIL_ADDR0_CR_addr_OFFSET  0x0
    #define SW_FAIL_ADDR0_CR_addr_MASK    (0xFFFFFFFF << 0x0)

/*AXI switch decode fail*/
#define SW_FAIL_ADDR1_CR_OFFSET   0xFC
    /* Upper 6 bits off address [37:32]*/
    #define SW_FAIL_ADDR1_CR_addr_OFFSET  0x0
    #define SW_FAIL_ADDR1_CR_addr_MASK    (0x3F << 0x0)
    /* AXI ID off failure*/
    #define SW_FAIL_ADDR1_CR_ID_OFFSET  0x8
    #define SW_FAIL_ADDR1_CR_ID_MASK    (0xFF << 0x8)
    /* AXI write=1 or read=0*/
    #define SW_FAIL_ADDR1_CR_write_OFFSET  0x10
    #define SW_FAIL_ADDR1_CR_write_MASK    (0x01 << 0x10)
    /* */
    #define SW_FAIL_ADDR1_CR_failed_OFFSET  0x11
    #define SW_FAIL_ADDR1_CR_failed_MASK    (0x01 << 0x11)

/*Set when an ECC event happens*/
#define EDAC_SR_OFFSET   0x100
    /* */
    #define EDAC_SR_MMC_1E_OFFSET  0x0
    #define EDAC_SR_MMC_1E_MASK    (0x01 << 0x0)
    /* */
    #define EDAC_SR_MMC_2E_OFFSET  0x1
    #define EDAC_SR_MMC_2E_MASK    (0x01 << 0x1)
    /* */
    #define EDAC_SR_DDRC_1E_OFFSET  0x2
    #define EDAC_SR_DDRC_1E_MASK    (0x01 << 0x2)
    /* */
    #define EDAC_SR_DDRC_2E_OFFSET  0x3
    #define EDAC_SR_DDRC_2E_MASK    (0x01 << 0x3)
    /* */
    #define EDAC_SR_MAC0_1E_OFFSET  0x4
    #define EDAC_SR_MAC0_1E_MASK    (0x01 << 0x4)
    /* */
    #define EDAC_SR_MAC0_2E_OFFSET  0x5
    #define EDAC_SR_MAC0_2E_MASK    (0x01 << 0x5)
    /* */
    #define EDAC_SR_MAC1_1E_OFFSET  0x6
    #define EDAC_SR_MAC1_1E_MASK    (0x01 << 0x6)
    /* */
    #define EDAC_SR_MAC1_2E_OFFSET  0x7
    #define EDAC_SR_MAC1_2E_MASK    (0x01 << 0x7)
    /* */
    #define EDAC_SR_USB_1E_OFFSET  0x8
    #define EDAC_SR_USB_1E_MASK    (0x01 << 0x8)
    /* */
    #define EDAC_SR_USB_2E_OFFSET  0x9
    #define EDAC_SR_USB_2E_MASK    (0x01 << 0x9)
    /* */
    #define EDAC_SR_CAN0_1E_OFFSET  0xA
    #define EDAC_SR_CAN0_1E_MASK    (0x01 << 0xA)
    /* */
    #define EDAC_SR_CAN0_2E_OFFSET  0xB
    #define EDAC_SR_CAN0_2E_MASK    (0x01 << 0xB)
    /* */
    #define EDAC_SR_CAN1_1E_OFFSET  0xC
    #define EDAC_SR_CAN1_1E_MASK    (0x01 << 0xC)
    /* */
    #define EDAC_SR_CAN1_2E_OFFSET  0xD
    #define EDAC_SR_CAN1_2E_MASK    (0x01 << 0xD)

/*Enables ECC interrupt on event*/
#define EDAC_INTEN_CR_OFFSET   0x104
    /* */
    #define EDAC_INTEN_CR_MMC_1E_OFFSET  0x0
    #define EDAC_INTEN_CR_MMC_1E_MASK    (0x01 << 0x0)
    /* */
    #define EDAC_INTEN_CR_MMC_2E_OFFSET  0x1
    #define EDAC_INTEN_CR_MMC_2E_MASK    (0x01 << 0x1)
    /* */
    #define EDAC_INTEN_CR_DDRC_1E_OFFSET  0x2
    #define EDAC_INTEN_CR_DDRC_1E_MASK    (0x01 << 0x2)
    /* */
    #define EDAC_INTEN_CR_DDRC_2E_OFFSET  0x3
    #define EDAC_INTEN_CR_DDRC_2E_MASK    (0x01 << 0x3)
    /* */
    #define EDAC_INTEN_CR_MAC0_1E_OFFSET  0x4
    #define EDAC_INTEN_CR_MAC0_1E_MASK    (0x01 << 0x4)
    /* */
    #define EDAC_INTEN_CR_MAC0_2E_OFFSET  0x5
    #define EDAC_INTEN_CR_MAC0_2E_MASK    (0x01 << 0x5)
    /* */
    #define EDAC_INTEN_CR_MAC1_1E_OFFSET  0x6
    #define EDAC_INTEN_CR_MAC1_1E_MASK    (0x01 << 0x6)
    /* */
    #define EDAC_INTEN_CR_MAC1_2E_OFFSET  0x7
    #define EDAC_INTEN_CR_MAC1_2E_MASK    (0x01 << 0x7)
    /* */
    #define EDAC_INTEN_CR_USB_1E_OFFSET  0x8
    #define EDAC_INTEN_CR_USB_1E_MASK    (0x01 << 0x8)
    /* */
    #define EDAC_INTEN_CR_USB_2E_OFFSET  0x9
    #define EDAC_INTEN_CR_USB_2E_MASK    (0x01 << 0x9)
    /* */
    #define EDAC_INTEN_CR_CAN0_1E_OFFSET  0xA
    #define EDAC_INTEN_CR_CAN0_1E_MASK    (0x01 << 0xA)
    /* */
    #define EDAC_INTEN_CR_CAN0_2E_OFFSET  0xB
    #define EDAC_INTEN_CR_CAN0_2E_MASK    (0x01 << 0xB)
    /* */
    #define EDAC_INTEN_CR_CAN1_1E_OFFSET  0xC
    #define EDAC_INTEN_CR_CAN1_1E_MASK    (0x01 << 0xC)
    /* */
    #define EDAC_INTEN_CR_CAN1_2E_OFFSET  0xD
    #define EDAC_INTEN_CR_CAN1_2E_MASK    (0x01 << 0xD)

/*Count off single bit errors*/
#define EDAC_CNT_MMC_OFFSET   0x108
    /* */
    #define EDAC_CNT_MMC_COUNT_OFFSET  0x0
    #define EDAC_CNT_MMC_COUNT_MASK    (0x3FF << 0x0)

/*Count off single bit errors*/
#define EDAC_CNT_DDRC_OFFSET   0x10C
    /* */
    #define EDAC_CNT_DDRC_COUNT_OFFSET  0x0
    #define EDAC_CNT_DDRC_COUNT_MASK    (0x3FF << 0x0)

/*Count off single bit errors*/
#define EDAC_CNT_MAC0_OFFSET   0x110
    /* */
    #define EDAC_CNT_MAC0_COUNT_OFFSET  0x0
    #define EDAC_CNT_MAC0_COUNT_MASK    (0x3FF << 0x0)

/*Count off single bit errors*/
#define EDAC_CNT_MAC1_OFFSET   0x114
    /* */
    #define EDAC_CNT_MAC1_COUNT_OFFSET  0x0
    #define EDAC_CNT_MAC1_COUNT_MASK    (0x3FF << 0x0)

/*Count off single bit errors*/
#define EDAC_CNT_USB_OFFSET   0x118
    /* */
    #define EDAC_CNT_USB_COUNT_OFFSET  0x0
    #define EDAC_CNT_USB_COUNT_MASK    (0x3FF << 0x0)

/*Count off single bit errors*/
#define EDAC_CNT_CAN0_OFFSET   0x11C
    /* */
    #define EDAC_CNT_CAN0_COUNT_OFFSET  0x0
    #define EDAC_CNT_CAN0_COUNT_MASK    (0x3FF << 0x0)

/*Count off single bit errors*/
#define EDAC_CNT_CAN1_OFFSET   0x120
    /* */
    #define EDAC_CNT_CAN1_COUNT_OFFSET  0x0
    #define EDAC_CNT_CAN1_COUNT_MASK    (0x3FF << 0x0)

/*"Will Corrupt write data to rams 1E corrupts bit 0 2E bits 1 and 2.Injects Errors into all RAMS in the block as long as the bits are set. Setting 1E and 2E will inject a 3-bit error"*/
#define EDAC_INJECT_CR_OFFSET   0x124
    /* */
    #define EDAC_INJECT_CR_MMC_1E_OFFSET  0x0
    #define EDAC_INJECT_CR_MMC_1E_MASK    (0x01 << 0x0)
    /* */
    #define EDAC_INJECT_CR_MMC_2E_OFFSET  0x1
    #define EDAC_INJECT_CR_MMC_2E_MASK    (0x01 << 0x1)
    /* */
    #define EDAC_INJECT_CR_DDRC_1E_OFFSET  0x2
    #define EDAC_INJECT_CR_DDRC_1E_MASK    (0x01 << 0x2)
    /* */
    #define EDAC_INJECT_CR_DDRC_2E_OFFSET  0x3
    #define EDAC_INJECT_CR_DDRC_2E_MASK    (0x01 << 0x3)
    /* */
    #define EDAC_INJECT_CR_MAC0_1E_OFFSET  0x4
    #define EDAC_INJECT_CR_MAC0_1E_MASK    (0x01 << 0x4)
    /* */
    #define EDAC_INJECT_CR_MAC0_2E_OFFSET  0x5
    #define EDAC_INJECT_CR_MAC0_2E_MASK    (0x01 << 0x5)
    /* */
    #define EDAC_INJECT_CR_MAC1_1E_OFFSET  0x6
    #define EDAC_INJECT_CR_MAC1_1E_MASK    (0x01 << 0x6)
    /* */
    #define EDAC_INJECT_CR_MAC1_2E_OFFSET  0x7
    #define EDAC_INJECT_CR_MAC1_2E_MASK    (0x01 << 0x7)
    /* */
    #define EDAC_INJECT_CR_USB_1E_OFFSET  0x8
    #define EDAC_INJECT_CR_USB_1E_MASK    (0x01 << 0x8)
    /* */
    #define EDAC_INJECT_CR_USB_2E_OFFSET  0x9
    #define EDAC_INJECT_CR_USB_2E_MASK    (0x01 << 0x9)
    /* */
    #define EDAC_INJECT_CR_CAN0_1E_OFFSET  0xA
    #define EDAC_INJECT_CR_CAN0_1E_MASK    (0x01 << 0xA)
    /* */
    #define EDAC_INJECT_CR_CAN0_2E_OFFSET  0xB
    #define EDAC_INJECT_CR_CAN0_2E_MASK    (0x01 << 0xB)
    /* */
    #define EDAC_INJECT_CR_CAN1_1E_OFFSET  0xC
    #define EDAC_INJECT_CR_CAN1_1E_MASK    (0x01 << 0xC)
    /* */
    #define EDAC_INJECT_CR_CAN1_2E_OFFSET  0xD
    #define EDAC_INJECT_CR_CAN1_2E_MASK    (0x01 << 0xD)

/*Maintenance Interrupt Enable.*/
#define MAINTENANCE_INTEN_CR_OFFSET   0x140
    /* Enables interrupt on a PLL event PLL_STATUS_INTEN_CR should also be set*/
    #define MAINTENANCE_INTEN_CR_pll_OFFSET  0x0
    #define MAINTENANCE_INTEN_CR_pll_MASK    (0x01 << 0x0)
    /* Enables interrupt on a MPU access violation */
    #define MAINTENANCE_INTEN_CR_mpu_OFFSET  0x1
    #define MAINTENANCE_INTEN_CR_mpu_MASK    (0x01 << 0x1)
    /* Enables interrupt on a AXI switch decode error*/
    #define MAINTENANCE_INTEN_CR_decode_OFFSET  0x2
    #define MAINTENANCE_INTEN_CR_decode_MASK    (0x01 << 0x2)
    /* Enables interrupt as lp_state goes high*/
    #define MAINTENANCE_INTEN_CR_lp_state_enter_OFFSET  0x3
    #define MAINTENANCE_INTEN_CR_lp_state_enter_MASK    (0x01 << 0x3)
    /* Enables interrupt as lp_state goes low*/
    #define MAINTENANCE_INTEN_CR_lp_state_exit_OFFSET  0x4
    #define MAINTENANCE_INTEN_CR_lp_state_exit_MASK    (0x01 << 0x4)
    /* Enables interrupt as flash_freeze goes high*/
    #define MAINTENANCE_INTEN_CR_ff_start_OFFSET  0x5
    #define MAINTENANCE_INTEN_CR_ff_start_MASK    (0x01 << 0x5)
    /* Enables interrupt as flash_freeze goes low*/
    #define MAINTENANCE_INTEN_CR_ff_end_OFFSET  0x6
    #define MAINTENANCE_INTEN_CR_ff_end_MASK    (0x01 << 0x6)
    /* Enables interrupt when FPGA turned on*/
    #define MAINTENANCE_INTEN_CR_fpga_on_OFFSET  0x7
    #define MAINTENANCE_INTEN_CR_fpga_on_MASK    (0x01 << 0x7)
    /* Enables interrupt when FPGA turned off*/
    #define MAINTENANCE_INTEN_CR_fpga_off_OFFSET  0x8
    #define MAINTENANCE_INTEN_CR_fpga_off_MASK    (0x01 << 0x8)
    /* Enables interrupt on SCB error*/
    #define MAINTENANCE_INTEN_CR_scb_error_OFFSET  0x9
    #define MAINTENANCE_INTEN_CR_scb_error_MASK    (0x01 << 0x9)
    /* Enables interrupt on SCB failure*/
    #define MAINTENANCE_INTEN_CR_scb_fault_OFFSET  0xA
    #define MAINTENANCE_INTEN_CR_scb_fault_MASK    (0x01 << 0xA)
    /* Enables interrupt on Mesh violation detection */
    #define MAINTENANCE_INTEN_CR_mesh_error_OFFSET  0xB
    #define MAINTENANCE_INTEN_CR_mesh_error_MASK    (0x01 << 0xB)
    /* Enables interrupt on bank2 powered on*/
    #define MAINTENANCE_INTEN_CR_io_bank_b2_on_OFFSET  0xC
    #define MAINTENANCE_INTEN_CR_io_bank_b2_on_MASK    (0x01 << 0xC)
    /* Enables interrupt on bank4 powered on*/
    #define MAINTENANCE_INTEN_CR_io_bank_b4_on_OFFSET  0xD
    #define MAINTENANCE_INTEN_CR_io_bank_b4_on_MASK    (0x01 << 0xD)
    /* Enables interrupt on bank5 powered on*/
    #define MAINTENANCE_INTEN_CR_io_bank_b5_on_OFFSET  0xE
    #define MAINTENANCE_INTEN_CR_io_bank_b5_on_MASK    (0x01 << 0xE)
    /* Enables interrupt on bank6 powered on*/
    #define MAINTENANCE_INTEN_CR_io_bank_b6_on_OFFSET  0xF
    #define MAINTENANCE_INTEN_CR_io_bank_b6_on_MASK    (0x01 << 0xF)
    /* Enables interrupt on bank2 powered off*/
    #define MAINTENANCE_INTEN_CR_io_bank_b2_off_OFFSET  0x10
    #define MAINTENANCE_INTEN_CR_io_bank_b2_off_MASK    (0x01 << 0x10)
    /* Enables interrupt on bank4 powered off*/
    #define MAINTENANCE_INTEN_CR_io_bank_b4_off_OFFSET  0x11
    #define MAINTENANCE_INTEN_CR_io_bank_b4_off_MASK    (0x01 << 0x11)
    /* Enables interrupt on bank5 powered off*/
    #define MAINTENANCE_INTEN_CR_io_bank_b5_off_OFFSET  0x12
    #define MAINTENANCE_INTEN_CR_io_bank_b5_off_MASK    (0x01 << 0x12)
    /* Enables interrupt on bank6 powered off*/
    #define MAINTENANCE_INTEN_CR_io_bank_b6_off_OFFSET  0x13
    #define MAINTENANCE_INTEN_CR_io_bank_b6_off_MASK    (0x01 << 0x13)
    /* Enables interrupt on a DLL event DLL_STATUS_INTEN_CR should also be set*/
    #define MAINTENANCE_INTEN_CR_dll_OFFSET  0x14
    #define MAINTENANCE_INTEN_CR_dll_MASK    (0x01 << 0x14)

/*PLL Status interrupt enables*/
#define PLL_STATUS_INTEN_CR_OFFSET   0x144
    /* Enables interrupt on CPU PLL locking*/
    #define PLL_STATUS_INTEN_CR_CPU_LOCK_OFFSET  0x0
    #define PLL_STATUS_INTEN_CR_CPU_LOCK_MASK    (0x01 << 0x0)
    /* Enables interrupt on DFT PLL locking*/
    #define PLL_STATUS_INTEN_CR_DFI_LOCK_OFFSET  0x1
    #define PLL_STATUS_INTEN_CR_DFI_LOCK_MASK    (0x01 << 0x1)
    /* Enables interrupt on SGMII PLL locking*/
    #define PLL_STATUS_INTEN_CR_SGMII_LOCK_OFFSET  0x2
    #define PLL_STATUS_INTEN_CR_SGMII_LOCK_MASK    (0x01 << 0x2)
    /* Enables interrupt on CPU PLL unlocking*/
    #define PLL_STATUS_INTEN_CR_CPU_UNLOCK_OFFSET  0x4
    #define PLL_STATUS_INTEN_CR_CPU_UNLOCK_MASK    (0x01 << 0x4)
    /* Enables interrupt on DFT PLL unlocking*/
    #define PLL_STATUS_INTEN_CR_DFI_UNLOCK_OFFSET  0x5
    #define PLL_STATUS_INTEN_CR_DFI_UNLOCK_MASK    (0x01 << 0x5)
    /* Enables interrupt on SGMII PLL unlocking*/
    #define PLL_STATUS_INTEN_CR_SGMII_UNLOCK_OFFSET  0x6
    #define PLL_STATUS_INTEN_CR_SGMII_UNLOCK_MASK    (0x01 << 0x6)

/*Maintenance interrupt indicates fault and status events.*/
#define MAINTENANCE_INT_SR_OFFSET   0x148
    /* Indicates that one off the PLLs whent into the lock or unlock state. Cleared via PLL status register*/
    #define MAINTENANCE_INT_SR_pll_OFFSET  0x0
    #define MAINTENANCE_INT_SR_pll_MASK    (0x01 << 0x0)
    /* Indicates that one off the MPUS signaled a MPU violation. Cleared via MPU Violation Register*/
    #define MAINTENANCE_INT_SR_mpu_OFFSET  0x1
    #define MAINTENANCE_INT_SR_mpu_MASK    (0x01 << 0x1)
    /* Indicates that the AXI switch detected an illegal address. Cleared when SREG.SW_FAIL.ADDR1_CR_FAILED is cleared.*/
    #define MAINTENANCE_INT_SR_decode_OFFSET  0x2
    #define MAINTENANCE_INT_SR_decode_MASK    (0x01 << 0x2)
    /* Indicates the device has entered the lower power state cleared by writing '1'*/
    #define MAINTENANCE_INT_SR_lp_state_enter_OFFSET  0x3
    #define MAINTENANCE_INT_SR_lp_state_enter_MASK    (0x01 << 0x3)
    /* Indicates the device has exited the lower power state cleared by writing '1'*/
    #define MAINTENANCE_INT_SR_lp_state_exit_OFFSET  0x4
    #define MAINTENANCE_INT_SR_lp_state_exit_MASK    (0x01 << 0x4)
    /* Indicates the device has entered the flash freezer state cleared by writing '1'*/
    #define MAINTENANCE_INT_SR_ff_start_OFFSET  0x5
    #define MAINTENANCE_INT_SR_ff_start_MASK    (0x01 << 0x5)
    /* Indicates the device has exited the flash freezer state cleared by writing '1'*/
    #define MAINTENANCE_INT_SR_ff_end_OFFSET  0x6
    #define MAINTENANCE_INT_SR_ff_end_MASK    (0x01 << 0x6)
    /* Indicates that the FPGA array has been turned on cleared by writing a '1'*/
    #define MAINTENANCE_INT_SR_fpga_on_OFFSET  0x7
    #define MAINTENANCE_INT_SR_fpga_on_MASK    (0x01 << 0x7)
    /* Indicates that the FPGA array has been turned off cleared by writing a '1'*/
    #define MAINTENANCE_INT_SR_fpga_off_OFFSET  0x8
    #define MAINTENANCE_INT_SR_fpga_off_MASK    (0x01 << 0x8)
    /* Indicates that the SCB slave reported an error cleared via SCB controller*/
    #define MAINTENANCE_INT_SR_scb_error_OFFSET  0x9
    #define MAINTENANCE_INT_SR_scb_error_MASK    (0x01 << 0x9)
    /* Indicates that the SCB bus fault occurred cleared via SCB controller*/
    #define MAINTENANCE_INT_SR_scb_fault_OFFSET  0xA
    #define MAINTENANCE_INT_SR_scb_fault_MASK    (0x01 << 0xA)
    /* Indicates that the mesh over the Crypto triggered cleared via Mesh system error*/
    #define MAINTENANCE_INT_SR_mesh_error_OFFSET  0xB
    #define MAINTENANCE_INT_SR_mesh_error_MASK    (0x01 << 0xB)
    /* Indicates that IO bank 2 has turned on cleared by writing a '1'*/
    #define MAINTENANCE_INT_SR_io_bank_b2_on_OFFSET  0xC
    #define MAINTENANCE_INT_SR_io_bank_b2_on_MASK    (0x01 << 0xC)
    /* Indicates that IO bank 4 has turned on cleared by writing a '1'*/
    #define MAINTENANCE_INT_SR_io_bank_b4_on_OFFSET  0xD
    #define MAINTENANCE_INT_SR_io_bank_b4_on_MASK    (0x01 << 0xD)
    /* Indicates that IO bank 5  has turned on cleared by writing a '1'*/
    #define MAINTENANCE_INT_SR_io_bank_b5_on_OFFSET  0xE
    #define MAINTENANCE_INT_SR_io_bank_b5_on_MASK    (0x01 << 0xE)
    /* Indicates that IO bank 6  has turned on cleared by writing a '1'*/
    #define MAINTENANCE_INT_SR_io_bank_b6_on_OFFSET  0xF
    #define MAINTENANCE_INT_SR_io_bank_b6_on_MASK    (0x01 << 0xF)
    /* Indicates that IO bank 2 has turned off cleared by writing a '1'*/
    #define MAINTENANCE_INT_SR_io_bank_b2_off_OFFSET  0x10
    #define MAINTENANCE_INT_SR_io_bank_b2_off_MASK    (0x01 << 0x10)
    /* Indicates that IO bank 4 has turned off cleared by writing a '1'*/
    #define MAINTENANCE_INT_SR_io_bank_b4_off_OFFSET  0x11
    #define MAINTENANCE_INT_SR_io_bank_b4_off_MASK    (0x01 << 0x11)
    /* Indicates that IO bank 5  has turned off cleared by writing a '1'*/
    #define MAINTENANCE_INT_SR_io_bank_b5_off_OFFSET  0x12
    #define MAINTENANCE_INT_SR_io_bank_b5_off_MASK    (0x01 << 0x12)
    /* Indicates that one off the DLLs when into the lock or unlock state. Cleared via DLL status register*/
    #define MAINTENANCE_INT_SR_io_bank_b6_off_OFFSET  0x13
    #define MAINTENANCE_INT_SR_io_bank_b6_off_MASK    (0x01 << 0x13)
    /* Indicates that IO bank 6  has turned off cleared by writing a '1'*/
    #define MAINTENANCE_INT_SR_dll_OFFSET  0x14
    #define MAINTENANCE_INT_SR_dll_MASK    (0x01 << 0x14)

/*PLL interrupt register*/
#define PLL_STATUS_SR_OFFSET   0x14C
    /* Indicates that the CPU PLL has locked cleared by writing a '1'*/
    #define PLL_STATUS_SR_CPU_LOCK_OFFSET  0x0
    #define PLL_STATUS_SR_CPU_LOCK_MASK    (0x01 << 0x0)
    /* Indicates that the DFI PLL has locked cleared by writing a '1'*/
    #define PLL_STATUS_SR_DFI_LOCK_OFFSET  0x1
    #define PLL_STATUS_SR_DFI_LOCK_MASK    (0x01 << 0x1)
    /* Indicates that the SGMII PLL has locked cleared by writing a '1'*/
    #define PLL_STATUS_SR_SGMII_LOCK_OFFSET  0x2
    #define PLL_STATUS_SR_SGMII_LOCK_MASK    (0x01 << 0x2)
    /* Indicates that the CPU PLL has unlocked cleared by writing a '1'*/
    #define PLL_STATUS_SR_CPU_UNLOCK_OFFSET  0x4
    #define PLL_STATUS_SR_CPU_UNLOCK_MASK    (0x01 << 0x4)
    /* Indicates that the DFI PLL has unlocked cleared by writing a '1'*/
    #define PLL_STATUS_SR_DFI_UNLOCK_OFFSET  0x5
    #define PLL_STATUS_SR_DFI_UNLOCK_MASK    (0x01 << 0x5)
    /* Indicates that the SGMII PLL has unlocked cleared by writing a '1'*/
    #define PLL_STATUS_SR_SGMII_UNLOCK_OFFSET  0x6
    #define PLL_STATUS_SR_SGMII_UNLOCK_MASK    (0x01 << 0x6)
    /* Current state off CPU PLL locked signal*/
    #define PLL_STATUS_SR_CPU_LOCK_NOW_OFFSET  0x8
    #define PLL_STATUS_SR_CPU_LOCK_NOW_MASK    (0x01 << 0x8)
    /* Current state off DFI PLL locked signal*/
    #define PLL_STATUS_SR_DFI_LOCK_NOW_OFFSET  0x9
    #define PLL_STATUS_SR_DFI_LOCK_NOW_MASK    (0x01 << 0x9)
    /* Current state off SGMII PLL locked signal*/
    #define PLL_STATUS_SR_SGMII_LOCK_NOW_OFFSET  0xA
    #define PLL_STATUS_SR_SGMII_LOCK_NOW_MASK    (0x01 << 0xA)

/*Enable to CFM Timer */
#define CFM_TIMER_CR_OFFSET   0x150
    /* When set and the CFM channel is in timer mode and CFM channel is set to 2 (Group C) this register allows the timet to count. Allows software to start and stop the timers.*/
    #define CFM_TIMER_CR_Enable_OFFSET  0x0
    #define CFM_TIMER_CR_Enable_MASK    (0x1F << 0x0)

/*Miscellaneous Register*/
#define MISC_SR_OFFSET   0x154
    /* Indicates that Interrupt from the G5C MSS SCB SPI controller is active*/
    #define MISC_SR_CONT_SPI_INTERRUPT_OFFSET  0x0
    #define MISC_SR_CONT_SPI_INTERRUPT_MASK    (0x01 << 0x0)
    /* Indicates that the user voltage or temperature detectors are signaling an alarm condition.*/
    #define MISC_SR_VOLT_TEMP_ALARM_OFFSET  0x1
    #define MISC_SR_VOLT_TEMP_ALARM_MASK    (0x01 << 0x1)

/*DLL Interrupt enables*/
#define DLL_STATUS_CR_OFFSET   0x158
    /* Enables the DLL0 lock interrupt*/
    #define DLL_STATUS_CR_FIC0_LOCK_OFFSET  0x0
    #define DLL_STATUS_CR_FIC0_LOCK_MASK    (0x01 << 0x0)
    /* Enables the DLL1 lock interrupt*/
    #define DLL_STATUS_CR_FIC1_LOCK_OFFSET  0x1
    #define DLL_STATUS_CR_FIC1_LOCK_MASK    (0x01 << 0x1)
    /* Enables the DLL2 lock interrupt*/
    #define DLL_STATUS_CR_FIC2_LOCK_OFFSET  0x2
    #define DLL_STATUS_CR_FIC2_LOCK_MASK    (0x01 << 0x2)
    /* Enables the DLL3 lock interrupt*/
    #define DLL_STATUS_CR_FIC3_LOCK_OFFSET  0x4
    #define DLL_STATUS_CR_FIC3_LOCK_MASK    (0x01 << 0x4)
    /* Enables the DLL4 (Crypto) lock interrupt*/
    #define DLL_STATUS_CR_FIC4_LOCK_OFFSET  0x5
    #define DLL_STATUS_CR_FIC4_LOCK_MASK    (0x01 << 0x5)
    /* Enables the DLL0 unlock interrupt*/
    #define DLL_STATUS_CR_FIC0_UNLOCK_OFFSET  0x8
    #define DLL_STATUS_CR_FIC0_UNLOCK_MASK    (0x01 << 0x8)
    /* Enables the DLL1 unlock interrupt*/
    #define DLL_STATUS_CR_FIC1_UNLOCK_OFFSET  0x9
    #define DLL_STATUS_CR_FIC1_UNLOCK_MASK    (0x01 << 0x9)
    /* Enables the DLL2 unlock interrupt*/
    #define DLL_STATUS_CR_FIC2_UNLOCK_OFFSET  0xA
    #define DLL_STATUS_CR_FIC2_UNLOCK_MASK    (0x01 << 0xA)
    /* Enables the DLL3 unlock interrupt*/
    #define DLL_STATUS_CR_FIC3_UNLOCK_OFFSET  0xB
    #define DLL_STATUS_CR_FIC3_UNLOCK_MASK    (0x01 << 0xB)
    /* Enables the DLL4 (crypto) unlock interrupt*/
    #define DLL_STATUS_CR_FIC4_UNLOCK_OFFSET  0xC
    #define DLL_STATUS_CR_FIC4_UNLOCK_MASK    (0x01 << 0xC)

/*DLL interrupt register*/
#define DLL_STATUS_SR_OFFSET   0x15C
    /* Indicates that the FIC0 DLL has locked cleared by writing a '1'*/
    #define DLL_STATUS_SR_FIC0_LOCK_OFFSET  0x0
    #define DLL_STATUS_SR_FIC0_LOCK_MASK    (0x01 << 0x0)
    /* Indicates that the FIC1 DLL has locked cleared by writing a '1'*/
    #define DLL_STATUS_SR_FIC1_LOCK_OFFSET  0x1
    #define DLL_STATUS_SR_FIC1_LOCK_MASK    (0x01 << 0x1)
    /* Indicates that the FIC2 DLL has locked cleared by writing a '1'*/
    #define DLL_STATUS_SR_FIC2_LOCK_OFFSET  0x2
    #define DLL_STATUS_SR_FIC2_LOCK_MASK    (0x01 << 0x2)
    /* Indicates that the FIC3 DLL has locked cleared by writing a '1'*/
    #define DLL_STATUS_SR_FIC3_LOCK_OFFSET  0x4
    #define DLL_STATUS_SR_FIC3_LOCK_MASK    (0x01 << 0x4)
    /* Indicates that the FIC4 (Crypto) DLL has locked cleared by writing a '1'*/
    #define DLL_STATUS_SR_FIC4_LOCK_OFFSET  0x5
    #define DLL_STATUS_SR_FIC4_LOCK_MASK    (0x01 << 0x5)
    /* Indicates that the FIC0 DLL has unlocked cleared by writing a '1'*/
    #define DLL_STATUS_SR_FIC0_UNLOCK_OFFSET  0x8
    #define DLL_STATUS_SR_FIC0_UNLOCK_MASK    (0x01 << 0x8)
    /* Indicates that the FIC1 DLL has unlocked cleared by writing a '1'*/
    #define DLL_STATUS_SR_FIC1_UNLOCK_OFFSET  0x9
    #define DLL_STATUS_SR_FIC1_UNLOCK_MASK    (0x01 << 0x9)
    /* Indicates that the FIC2 DLL has unlocked cleared by writing a '1'*/
    #define DLL_STATUS_SR_FIC2_UNLOCK_OFFSET  0xA
    #define DLL_STATUS_SR_FIC2_UNLOCK_MASK    (0x01 << 0xA)
    /* Indicates that the FIC3 DLL has unlocked cleared by writing a '1'*/
    #define DLL_STATUS_SR_FIC3_UNLOCK_OFFSET  0xB
    #define DLL_STATUS_SR_FIC3_UNLOCK_MASK    (0x01 << 0xB)
    /* Indicates that the FIC4 (Crypto) DLL has unlocked cleared by writing a '1'*/
    #define DLL_STATUS_SR_FIC4_UNLOCK_OFFSET  0xC
    #define DLL_STATUS_SR_FIC4_UNLOCK_MASK    (0x01 << 0xC)
    /* Current state off FIC0 DLL locked signal*/
    #define DLL_STATUS_SR_FIC0_LOCK_NOW_OFFSET  0x10
    #define DLL_STATUS_SR_FIC0_LOCK_NOW_MASK    (0x01 << 0x10)
    /* Current state off FIC1 DLL locked signal*/
    #define DLL_STATUS_SR_FIC1_LOCK_NOW_OFFSET  0x11
    #define DLL_STATUS_SR_FIC1_LOCK_NOW_MASK    (0x01 << 0x11)
    /* Current state off FIC2 DLL locked signal*/
    #define DLL_STATUS_SR_FIC2_LOCK_NOW_OFFSET  0x12
    #define DLL_STATUS_SR_FIC2_LOCK_NOW_MASK    (0x01 << 0x12)
    /* Current state off FIC3 DLL locked signal*/
    #define DLL_STATUS_SR_FIC3_LOCK_NOW_OFFSET  0x13
    #define DLL_STATUS_SR_FIC3_LOCK_NOW_MASK    (0x01 << 0x13)
    /* Current state off FIC4 DLL locked signal*/
    #define DLL_STATUS_SR_FIC4_LOCK_NOW_OFFSET  0x14
    #define DLL_STATUS_SR_FIC4_LOCK_NOW_MASK    (0x01 << 0x14)

/*Puts all the RAMS in that block into low leakage mode. RAM contents and Q value preserved.*/
#define RAM_LIGHTSLEEP_CR_OFFSET   0x168
    /* */
    #define RAM_LIGHTSLEEP_CR_can0_OFFSET  0x0
    #define RAM_LIGHTSLEEP_CR_can0_MASK    (0x01 << 0x0)
    /* */
    #define RAM_LIGHTSLEEP_CR_can1_OFFSET  0x1
    #define RAM_LIGHTSLEEP_CR_can1_MASK    (0x01 << 0x1)
    /* */
    #define RAM_LIGHTSLEEP_CR_usb_OFFSET  0x2
    #define RAM_LIGHTSLEEP_CR_usb_MASK    (0x01 << 0x2)
    /* */
    #define RAM_LIGHTSLEEP_CR_gem0_OFFSET  0x3
    #define RAM_LIGHTSLEEP_CR_gem0_MASK    (0x01 << 0x3)
    /* */
    #define RAM_LIGHTSLEEP_CR_gem1_OFFSET  0x4
    #define RAM_LIGHTSLEEP_CR_gem1_MASK    (0x01 << 0x4)
    /* */
    #define RAM_LIGHTSLEEP_CR_mmc_OFFSET  0x5
    #define RAM_LIGHTSLEEP_CR_mmc_MASK    (0x01 << 0x5)
    /* */
    #define RAM_LIGHTSLEEP_CR_athena_OFFSET  0x6
    #define RAM_LIGHTSLEEP_CR_athena_MASK    (0x01 << 0x6)
    /* */
    #define RAM_LIGHTSLEEP_CR_ddrc_OFFSET  0x7
    #define RAM_LIGHTSLEEP_CR_ddrc_MASK    (0x01 << 0x7)
    /* */
    #define RAM_LIGHTSLEEP_CR_e51_OFFSET  0x8
    #define RAM_LIGHTSLEEP_CR_e51_MASK    (0x01 << 0x8)
    /* */
    #define RAM_LIGHTSLEEP_CR_u54_1_OFFSET  0x9
    #define RAM_LIGHTSLEEP_CR_u54_1_MASK    (0x01 << 0x9)
    /* */
    #define RAM_LIGHTSLEEP_CR_u54_2_OFFSET  0xA
    #define RAM_LIGHTSLEEP_CR_u54_2_MASK    (0x01 << 0xA)
    /* */
    #define RAM_LIGHTSLEEP_CR_u54_3_OFFSET  0xB
    #define RAM_LIGHTSLEEP_CR_u54_3_MASK    (0x01 << 0xB)
    /* */
    #define RAM_LIGHTSLEEP_CR_u54_4_OFFSET  0xC
    #define RAM_LIGHTSLEEP_CR_u54_4_MASK    (0x01 << 0xC)
    /* */
    #define RAM_LIGHTSLEEP_CR_l2_OFFSET  0xD
    #define RAM_LIGHTSLEEP_CR_l2_MASK    (0x01 << 0xD)

/*Puts all the RAMS in that block into deep sleep mode. RAM contents preserved. Powers down the periphery circuits.*/
#define RAM_DEEPSLEEP_CR_OFFSET   0x16C
    /* */
    #define RAM_DEEPSLEEP_CR_can0_OFFSET  0x0
    #define RAM_DEEPSLEEP_CR_can0_MASK    (0x01 << 0x0)
    /* */
    #define RAM_DEEPSLEEP_CR_can1_OFFSET  0x1
    #define RAM_DEEPSLEEP_CR_can1_MASK    (0x01 << 0x1)
    /* */
    #define RAM_DEEPSLEEP_CR_usb_OFFSET  0x2
    #define RAM_DEEPSLEEP_CR_usb_MASK    (0x01 << 0x2)
    /* */
    #define RAM_DEEPSLEEP_CR_gem0_OFFSET  0x3
    #define RAM_DEEPSLEEP_CR_gem0_MASK    (0x01 << 0x3)
    /* */
    #define RAM_DEEPSLEEP_CR_gem1_OFFSET  0x4
    #define RAM_DEEPSLEEP_CR_gem1_MASK    (0x01 << 0x4)
    /* */
    #define RAM_DEEPSLEEP_CR_mmc_OFFSET  0x5
    #define RAM_DEEPSLEEP_CR_mmc_MASK    (0x01 << 0x5)
    /* */
    #define RAM_DEEPSLEEP_CR_athena_OFFSET  0x6
    #define RAM_DEEPSLEEP_CR_athena_MASK    (0x01 << 0x6)
    /* */
    #define RAM_DEEPSLEEP_CR_ddrc_OFFSET  0x7
    #define RAM_DEEPSLEEP_CR_ddrc_MASK    (0x01 << 0x7)
    /* */
    #define RAM_DEEPSLEEP_CR_e51_OFFSET  0x8
    #define RAM_DEEPSLEEP_CR_e51_MASK    (0x01 << 0x8)
    /* */
    #define RAM_DEEPSLEEP_CR_u54_1_OFFSET  0x9
    #define RAM_DEEPSLEEP_CR_u54_1_MASK    (0x01 << 0x9)
    /* */
    #define RAM_DEEPSLEEP_CR_u54_2_OFFSET  0xA
    #define RAM_DEEPSLEEP_CR_u54_2_MASK    (0x01 << 0xA)
    /* */
    #define RAM_DEEPSLEEP_CR_u54_3_OFFSET  0xB
    #define RAM_DEEPSLEEP_CR_u54_3_MASK    (0x01 << 0xB)
    /* */
    #define RAM_DEEPSLEEP_CR_u54_4_OFFSET  0xC
    #define RAM_DEEPSLEEP_CR_u54_4_MASK    (0x01 << 0xC)
    /* */
    #define RAM_DEEPSLEEP_CR_l2_OFFSET  0xD
    #define RAM_DEEPSLEEP_CR_l2_MASK    (0x01 << 0xD)

/*Puts all the RAMS in that block into shut down mode. RAM contents not preserved.  Powers down the RAM and periphery circuits.*/
#define RAM_SHUTDOWN_CR_OFFSET   0x170
    /* */
    #define RAM_SHUTDOWN_CR_can0_OFFSET  0x0
    #define RAM_SHUTDOWN_CR_can0_MASK    (0x01 << 0x0)
    /* */
    #define RAM_SHUTDOWN_CR_can1_OFFSET  0x1
    #define RAM_SHUTDOWN_CR_can1_MASK    (0x01 << 0x1)
    /* */
    #define RAM_SHUTDOWN_CR_usb_OFFSET  0x2
    #define RAM_SHUTDOWN_CR_usb_MASK    (0x01 << 0x2)
    /* */
    #define RAM_SHUTDOWN_CR_gem0_OFFSET  0x3
    #define RAM_SHUTDOWN_CR_gem0_MASK    (0x01 << 0x3)
    /* */
    #define RAM_SHUTDOWN_CR_gem1_OFFSET  0x4
    #define RAM_SHUTDOWN_CR_gem1_MASK    (0x01 << 0x4)
    /* */
    #define RAM_SHUTDOWN_CR_mmc_OFFSET  0x5
    #define RAM_SHUTDOWN_CR_mmc_MASK    (0x01 << 0x5)
    /* */
    #define RAM_SHUTDOWN_CR_athena_OFFSET  0x6
    #define RAM_SHUTDOWN_CR_athena_MASK    (0x01 << 0x6)
    /* */
    #define RAM_SHUTDOWN_CR_ddrc_OFFSET  0x7
    #define RAM_SHUTDOWN_CR_ddrc_MASK    (0x01 << 0x7)
    /* */
    #define RAM_SHUTDOWN_CR_e51_OFFSET  0x8
    #define RAM_SHUTDOWN_CR_e51_MASK    (0x01 << 0x8)
    /* */
    #define RAM_SHUTDOWN_CR_u54_1_OFFSET  0x9
    #define RAM_SHUTDOWN_CR_u54_1_MASK    (0x01 << 0x9)
    /* */
    #define RAM_SHUTDOWN_CR_u54_2_OFFSET  0xA
    #define RAM_SHUTDOWN_CR_u54_2_MASK    (0x01 << 0xA)
    /* */
    #define RAM_SHUTDOWN_CR_u54_3_OFFSET  0xB
    #define RAM_SHUTDOWN_CR_u54_3_MASK    (0x01 << 0xB)
    /* */
    #define RAM_SHUTDOWN_CR_u54_4_OFFSET  0xC
    #define RAM_SHUTDOWN_CR_u54_4_MASK    (0x01 << 0xC)
    /* */
    #define RAM_SHUTDOWN_CR_l2_OFFSET  0xD
    #define RAM_SHUTDOWN_CR_l2_MASK    (0x01 << 0xD)

/*Allows each bank of the L2 Cache to be powered down ORed with global shutdown */
#define L2_SHUTDOWN_CR_OFFSET   0x174
    /* */
    #define L2_SHUTDOWN_CR_L2_RAMS_OFFSET  0x0
    #define L2_SHUTDOWN_CR_L2_RAMS_MASK    (0x0F << 0x0)

/*Selects whether the peripheral is connected to the Fabric or IOMUX structure.*/
#define IOMUX0_CR_OFFSET   0x200
    /* */
    #define IOMUX0_CR_spi0_fabric_OFFSET  0x0
    #define IOMUX0_CR_spi0_fabric_MASK    (0x01 << 0x0)
    /* */
    #define IOMUX0_CR_spi1_fabric_OFFSET  0x1
    #define IOMUX0_CR_spi1_fabric_MASK    (0x01 << 0x1)
    /* */
    #define IOMUX0_CR_i2c0_fabric_OFFSET  0x2
    #define IOMUX0_CR_i2c0_fabric_MASK    (0x01 << 0x2)
    /* */
    #define IOMUX0_CR_i2c1_fabric_OFFSET  0x3
    #define IOMUX0_CR_i2c1_fabric_MASK    (0x01 << 0x3)
    /* */
    #define IOMUX0_CR_can0_fabric_OFFSET  0x4
    #define IOMUX0_CR_can0_fabric_MASK    (0x01 << 0x4)
    /* */
    #define IOMUX0_CR_can1_fabric_OFFSET  0x5
    #define IOMUX0_CR_can1_fabric_MASK    (0x01 << 0x5)
    /* */
    #define IOMUX0_CR_qspi_fabric_OFFSET  0x6
    #define IOMUX0_CR_qspi_fabric_MASK    (0x01 << 0x6)
    /* */
    #define IOMUX0_CR_mmuart0_fabric_OFFSET  0x7
    #define IOMUX0_CR_mmuart0_fabric_MASK    (0x01 << 0x7)
    /* */
    #define IOMUX0_CR_mmuart1_fabric_OFFSET  0x8
    #define IOMUX0_CR_mmuart1_fabric_MASK    (0x01 << 0x8)
    /* */
    #define IOMUX0_CR_mmuart2_fabric_OFFSET  0x9
    #define IOMUX0_CR_mmuart2_fabric_MASK    (0x01 << 0x9)
    /* */
    #define IOMUX0_CR_mmuart3_fabric_OFFSET  0xA
    #define IOMUX0_CR_mmuart3_fabric_MASK    (0x01 << 0xA)
    /* */
    #define IOMUX0_CR_mmuart4_fabric_OFFSET  0xB
    #define IOMUX0_CR_mmuart4_fabric_MASK    (0x01 << 0xB)
    /* */
    #define IOMUX0_CR_mdio0_fabric_OFFSET  0xC
    #define IOMUX0_CR_mdio0_fabric_MASK    (0x01 << 0xC)
    /* */
    #define IOMUX0_CR_mdio1_fabric_OFFSET  0xD
    #define IOMUX0_CR_mdio1_fabric_MASK    (0x01 << 0xD)

/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
#define IOMUX1_CR_OFFSET   0x204
    /* */
    #define IOMUX1_CR_pad0_OFFSET  0x0
    #define IOMUX1_CR_pad0_MASK    (0x0F << 0x0)
    /* */
    #define IOMUX1_CR_pad1_OFFSET  0x4
    #define IOMUX1_CR_pad1_MASK    (0x0F << 0x4)
    /* */
    #define IOMUX1_CR_pad2_OFFSET  0x8
    #define IOMUX1_CR_pad2_MASK    (0x0F << 0x8)
    /* */
    #define IOMUX1_CR_pad3_OFFSET  0xC
    #define IOMUX1_CR_pad3_MASK    (0x0F << 0xC)
    /* */
    #define IOMUX1_CR_pad4_OFFSET  0x10
    #define IOMUX1_CR_pad4_MASK    (0x0F << 0x10)
    /* */
    #define IOMUX1_CR_pad5_OFFSET  0x14
    #define IOMUX1_CR_pad5_MASK    (0x0F << 0x14)
    /* */
    #define IOMUX1_CR_pad6_OFFSET  0x18
    #define IOMUX1_CR_pad6_MASK    (0x0F << 0x18)
    /* */
    #define IOMUX1_CR_pad7_OFFSET  0x1C
    #define IOMUX1_CR_pad7_MASK    (0x0F << 0x1C)

/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
#define IOMUX2_CR_OFFSET   0x208
    /* */
    #define IOMUX2_CR_pad8_OFFSET  0x0
    #define IOMUX2_CR_pad8_MASK    (0x0F << 0x0)
    /* */
    #define IOMUX2_CR_pad9_OFFSET  0x4
    #define IOMUX2_CR_pad9_MASK    (0x0F << 0x4)
    /* */
    #define IOMUX2_CR_pad10_OFFSET  0x8
    #define IOMUX2_CR_pad10_MASK    (0x0F << 0x8)
    /* */
    #define IOMUX2_CR_pad11_OFFSET  0xC
    #define IOMUX2_CR_pad11_MASK    (0x0F << 0xC)
    /* */
    #define IOMUX2_CR_pad12_OFFSET  0x10
    #define IOMUX2_CR_pad12_MASK    (0x0F << 0x10)
    /* */
    #define IOMUX2_CR_pad13_OFFSET  0x14
    #define IOMUX2_CR_pad13_MASK    (0x0F << 0x14)

/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
#define IOMUX3_CR_OFFSET   0x20C
    /* */
    #define IOMUX3_CR_pad14_OFFSET  0x0
    #define IOMUX3_CR_pad14_MASK    (0x0F << 0x0)
    /* */
    #define IOMUX3_CR_pad15_OFFSET  0x4
    #define IOMUX3_CR_pad15_MASK    (0x0F << 0x4)
    /* */
    #define IOMUX3_CR_pad16_OFFSET  0x8
    #define IOMUX3_CR_pad16_MASK    (0x0F << 0x8)
    /* */
    #define IOMUX3_CR_pad17_OFFSET  0xC
    #define IOMUX3_CR_pad17_MASK    (0x0F << 0xC)
    /* */
    #define IOMUX3_CR_pad18_OFFSET  0x10
    #define IOMUX3_CR_pad18_MASK    (0x0F << 0x10)
    /* */
    #define IOMUX3_CR_pad19_OFFSET  0x14
    #define IOMUX3_CR_pad19_MASK    (0x0F << 0x14)
    /* */
    #define IOMUX3_CR_pad20_OFFSET  0x18
    #define IOMUX3_CR_pad20_MASK    (0x0F << 0x18)
    /* */
    #define IOMUX3_CR_pad21_OFFSET  0x1C
    #define IOMUX3_CR_pad21_MASK    (0x0F << 0x1C)

/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
#define IOMUX4_CR_OFFSET   0x210
    /* */
    #define IOMUX4_CR_pad22_OFFSET  0x0
    #define IOMUX4_CR_pad22_MASK    (0x0F << 0x0)
    /* */
    #define IOMUX4_CR_pad23_OFFSET  0x4
    #define IOMUX4_CR_pad23_MASK    (0x0F << 0x4)
    /* */
    #define IOMUX4_CR_pad24_OFFSET  0x8
    #define IOMUX4_CR_pad24_MASK    (0x0F << 0x8)
    /* */
    #define IOMUX4_CR_pad25_OFFSET  0xC
    #define IOMUX4_CR_pad25_MASK    (0x0F << 0xC)
    /* */
    #define IOMUX4_CR_pad26_OFFSET  0x10
    #define IOMUX4_CR_pad26_MASK    (0x0F << 0x10)
    /* */
    #define IOMUX4_CR_pad27_OFFSET  0x14
    #define IOMUX4_CR_pad27_MASK    (0x0F << 0x14)
    /* */
    #define IOMUX4_CR_pad28_OFFSET  0x18
    #define IOMUX4_CR_pad28_MASK    (0x0F << 0x18)
    /* */
    #define IOMUX4_CR_pad29_OFFSET  0x1C
    #define IOMUX4_CR_pad29_MASK    (0x0F << 0x1C)

/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
#define IOMUX5_CR_OFFSET   0x214
    /* */
    #define IOMUX5_CR_pad30_OFFSET  0x0
    #define IOMUX5_CR_pad30_MASK    (0x0F << 0x0)
    /* */
    #define IOMUX5_CR_pad31_OFFSET  0x4
    #define IOMUX5_CR_pad31_MASK    (0x0F << 0x4)
    /* */
    #define IOMUX5_CR_pad32_OFFSET  0x8
    #define IOMUX5_CR_pad32_MASK    (0x0F << 0x8)
    /* */
    #define IOMUX5_CR_pad33_OFFSET  0xC
    #define IOMUX5_CR_pad33_MASK    (0x0F << 0xC)
    /* */
    #define IOMUX5_CR_pad34_OFFSET  0x10
    #define IOMUX5_CR_pad34_MASK    (0x0F << 0x10)
    /* */
    #define IOMUX5_CR_pad35_OFFSET  0x14
    #define IOMUX5_CR_pad35_MASK    (0x0F << 0x14)
    /* */
    #define IOMUX5_CR_pad36_OFFSET  0x18
    #define IOMUX5_CR_pad36_MASK    (0x0F << 0x18)
    /* */
    #define IOMUX5_CR_pad37_OFFSET  0x1C
    #define IOMUX5_CR_pad37_MASK    (0x0F << 0x1C)

/*Sets whether the MMC/SD Voltage select lines are inverted on entry to the IOMUX structure*/
#define IOMUX6_CR_OFFSET   0x218
    /* */
    #define IOMUX6_CR_VLT_SEL_OFFSET  0x0
    #define IOMUX6_CR_VLT_SEL_MASK    (0x01 << 0x0)
    /* */
    #define IOMUX6_CR_VLT_EN_OFFSET  0x1
    #define IOMUX6_CR_VLT_EN_MASK    (0x01 << 0x1)
    /* */
    #define IOMUX6_CR_VLT_CMD_DIR_OFFSET  0x2
    #define IOMUX6_CR_VLT_CMD_DIR_MASK    (0x01 << 0x2)
    /* */
    #define IOMUX6_CR_VLT_DIR_0_OFFSET  0x3
    #define IOMUX6_CR_VLT_DIR_0_MASK    (0x01 << 0x3)
    /* */
    #define IOMUX6_CR_VLT_DIR_1_3_OFFSET  0x4
    #define IOMUX6_CR_VLT_DIR_1_3_MASK    (0x01 << 0x4)
    /* */
    #define IOMUX6_CR_SD_LED_OFFSET  0x5
    #define IOMUX6_CR_SD_LED_MASK    (0x01 << 0x5)
    /* */
    #define IOMUX6_CR_SD_VOLT_0_OFFSET  0x6
    #define IOMUX6_CR_SD_VOLT_0_MASK    (0x01 << 0x6)
    /* */
    #define IOMUX6_CR_SD_VOLT_1_OFFSET  0x7
    #define IOMUX6_CR_SD_VOLT_1_MASK    (0x01 << 0x7)
    /* */
    #define IOMUX6_CR_SD_VOLT_2_OFFSET  0x8
    #define IOMUX6_CR_SD_VOLT_2_MASK    (0x01 << 0x8)

/*Configures the MSSIO block*/
#define mssio_bank4_cfg_cr_OFFSET   0x230
    /* Sets the PCODE value*/
    #define mssio_bank4_cfg_cr_bank_pcode_OFFSET  0x0
    #define mssio_bank4_cfg_cr_bank_pcode_MASK    (0x3F << 0x0)
    /* Sets the NCODE value*/
    #define mssio_bank4_cfg_cr_bank_ncode_OFFSET  0x6
    #define mssio_bank4_cfg_cr_bank_ncode_MASK    (0x3F << 0x6)
    /* Sets the voltage controls.*/
    #define mssio_bank4_cfg_cr_vs_OFFSET  0xC
    #define mssio_bank4_cfg_cr_vs_MASK    (0x0F << 0xC)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank4_io_cfg_0_1_cr_OFFSET   0x234
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_ibufmd_0_OFFSET  0x0
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_ibufmd_1_OFFSET  0x1
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_ibufmd_2_OFFSET  0x2
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_drv_0_OFFSET  0x3
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_drv_1_OFFSET  0x4
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_drv_2_OFFSET  0x5
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_drv_3_OFFSET  0x6
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_clamp_OFFSET  0x7
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_enhyst_OFFSET  0x8
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_lockdn_en_OFFSET  0x9
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_wpd_OFFSET  0xA
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_wpu_OFFSET  0xB
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_atp_en_OFFSET  0xC
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_lp_persist_en_OFFSET  0xD
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_lp_bypass_en_OFFSET  0xE
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_0_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_ibufmd_0_OFFSET  0x10
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_ibufmd_1_OFFSET  0x11
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_ibufmd_2_OFFSET  0x12
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_drv_0_OFFSET  0x13
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_drv_1_OFFSET  0x14
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_drv_2_OFFSET  0x15
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_drv_3_OFFSET  0x16
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_clamp_OFFSET  0x17
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_enhyst_OFFSET  0x18
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_lockdn_en_OFFSET  0x19
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_wpd_OFFSET  0x1A
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_wpu_OFFSET  0x1B
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_atp_en_OFFSET  0x1C
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_lp_persist_en_OFFSET  0x1D
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank4_io_cfg_0_1_cr_rpc_io_cfg_1_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank4_io_cfg_2_3_cr_OFFSET   0x238
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_ibufmd_0_OFFSET  0x0
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_ibufmd_1_OFFSET  0x1
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_ibufmd_2_OFFSET  0x2
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_drv_0_OFFSET  0x3
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_drv_1_OFFSET  0x4
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_drv_2_OFFSET  0x5
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_drv_3_OFFSET  0x6
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_clamp_OFFSET  0x7
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_enhyst_OFFSET  0x8
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_lockdn_en_OFFSET  0x9
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_wpd_OFFSET  0xA
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_wpu_OFFSET  0xB
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_atp_en_OFFSET  0xC
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_lp_persist_en_OFFSET  0xD
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_lp_bypass_en_OFFSET  0xE
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_2_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_ibufmd_0_OFFSET  0x10
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_ibufmd_1_OFFSET  0x11
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_ibufmd_2_OFFSET  0x12
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_drv_0_OFFSET  0x13
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_drv_1_OFFSET  0x14
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_drv_2_OFFSET  0x15
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_drv_3_OFFSET  0x16
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_clamp_OFFSET  0x17
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_enhyst_OFFSET  0x18
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_lockdn_en_OFFSET  0x19
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_wpd_OFFSET  0x1A
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_wpu_OFFSET  0x1B
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_atp_en_OFFSET  0x1C
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_lp_persist_en_OFFSET  0x1D
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank4_io_cfg_2_3_cr_rpc_io_cfg_3_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank4_io_cfg_4_5_cr_OFFSET   0x23C
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_ibufmd_0_OFFSET  0x0
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_ibufmd_1_OFFSET  0x1
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_ibufmd_2_OFFSET  0x2
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_drv_0_OFFSET  0x3
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_drv_1_OFFSET  0x4
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_drv_2_OFFSET  0x5
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_drv_3_OFFSET  0x6
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_clamp_OFFSET  0x7
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_enhyst_OFFSET  0x8
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_lockdn_en_OFFSET  0x9
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_wpd_OFFSET  0xA
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_wpu_OFFSET  0xB
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_atp_en_OFFSET  0xC
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_lp_persist_en_OFFSET  0xD
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_lp_bypass_en_OFFSET  0xE
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_4_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_ibufmd_0_OFFSET  0x10
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_ibufmd_1_OFFSET  0x11
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_ibufmd_2_OFFSET  0x12
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_drv_0_OFFSET  0x13
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_drv_1_OFFSET  0x14
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_drv_2_OFFSET  0x15
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_drv_3_OFFSET  0x16
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_clamp_OFFSET  0x17
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_enhyst_OFFSET  0x18
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_lockdn_en_OFFSET  0x19
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_wpd_OFFSET  0x1A
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_wpu_OFFSET  0x1B
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_atp_en_OFFSET  0x1C
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_lp_persist_en_OFFSET  0x1D
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank4_io_cfg_4_5_cr_rpc_io_cfg_5_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank4_io_cfg_6_7_cr_OFFSET   0x240
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_ibufmd_0_OFFSET  0x0
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_ibufmd_1_OFFSET  0x1
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_ibufmd_2_OFFSET  0x2
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_drv_0_OFFSET  0x3
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_drv_1_OFFSET  0x4
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_drv_2_OFFSET  0x5
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_drv_3_OFFSET  0x6
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_clamp_OFFSET  0x7
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_enhyst_OFFSET  0x8
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_lockdn_en_OFFSET  0x9
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_wpd_OFFSET  0xA
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_wpu_OFFSET  0xB
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_atp_en_OFFSET  0xC
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_lp_persist_en_OFFSET  0xD
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_lp_bypass_en_OFFSET  0xE
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_6_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_ibufmd_0_OFFSET  0x10
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_ibufmd_1_OFFSET  0x11
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_ibufmd_2_OFFSET  0x12
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_drv_0_OFFSET  0x13
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_drv_1_OFFSET  0x14
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_drv_2_OFFSET  0x15
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_drv_3_OFFSET  0x16
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_clamp_OFFSET  0x17
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_enhyst_OFFSET  0x18
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_lockdn_en_OFFSET  0x19
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_wpd_OFFSET  0x1A
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_wpu_OFFSET  0x1B
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_atp_en_OFFSET  0x1C
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_lp_persist_en_OFFSET  0x1D
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank4_io_cfg_6_7_cr_rpc_io_cfg_7_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank4_io_cfg_8_9_cr_OFFSET   0x244
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_ibufmd_0_OFFSET  0x0
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_ibufmd_1_OFFSET  0x1
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_ibufmd_2_OFFSET  0x2
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_drv_0_OFFSET  0x3
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_drv_1_OFFSET  0x4
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_drv_2_OFFSET  0x5
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_drv_3_OFFSET  0x6
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_clamp_OFFSET  0x7
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_enhyst_OFFSET  0x8
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_lockdn_en_OFFSET  0x9
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_wpd_OFFSET  0xA
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_wpu_OFFSET  0xB
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_atp_en_OFFSET  0xC
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_lp_persist_en_OFFSET  0xD
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_lp_bypass_en_OFFSET  0xE
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_8_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_ibufmd_0_OFFSET  0x10
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_ibufmd_1_OFFSET  0x11
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_ibufmd_2_OFFSET  0x12
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_drv_0_OFFSET  0x13
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_drv_1_OFFSET  0x14
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_drv_2_OFFSET  0x15
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_drv_3_OFFSET  0x16
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_clamp_OFFSET  0x17
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_enhyst_OFFSET  0x18
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_lockdn_en_OFFSET  0x19
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_wpd_OFFSET  0x1A
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_wpu_OFFSET  0x1B
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_atp_en_OFFSET  0x1C
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_lp_persist_en_OFFSET  0x1D
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank4_io_cfg_8_9_cr_rpc_io_cfg_9_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank4_io_cfg_10_11_cr_OFFSET   0x248
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_ibufmd_0_OFFSET  0x0
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_ibufmd_1_OFFSET  0x1
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_ibufmd_2_OFFSET  0x2
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_drv_0_OFFSET  0x3
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_drv_1_OFFSET  0x4
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_drv_2_OFFSET  0x5
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_drv_3_OFFSET  0x6
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_clamp_OFFSET  0x7
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_enhyst_OFFSET  0x8
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_lockdn_en_OFFSET  0x9
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_wpd_OFFSET  0xA
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_wpu_OFFSET  0xB
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_atp_en_OFFSET  0xC
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_lp_persist_en_OFFSET  0xD
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_lp_bypass_en_OFFSET  0xE
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_10_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_ibufmd_0_OFFSET  0x10
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_ibufmd_1_OFFSET  0x11
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_ibufmd_2_OFFSET  0x12
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_drv_0_OFFSET  0x13
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_drv_1_OFFSET  0x14
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_drv_2_OFFSET  0x15
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_drv_3_OFFSET  0x16
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_clamp_OFFSET  0x17
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_enhyst_OFFSET  0x18
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_lockdn_en_OFFSET  0x19
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_wpd_OFFSET  0x1A
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_wpu_OFFSET  0x1B
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_atp_en_OFFSET  0x1C
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_lp_persist_en_OFFSET  0x1D
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank4_io_cfg_10_11_cr_rpc_io_cfg_11_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank4_io_cfg_12_13_cr_OFFSET   0x24C
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_ibufmd_0_OFFSET  0x0
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_ibufmd_1_OFFSET  0x1
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_ibufmd_2_OFFSET  0x2
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_drv_0_OFFSET  0x3
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_drv_1_OFFSET  0x4
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_drv_2_OFFSET  0x5
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_drv_3_OFFSET  0x6
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_clamp_OFFSET  0x7
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_enhyst_OFFSET  0x8
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_lockdn_en_OFFSET  0x9
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_wpd_OFFSET  0xA
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_wpu_OFFSET  0xB
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_atp_en_OFFSET  0xC
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_lp_persist_en_OFFSET  0xD
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_lp_bypass_en_OFFSET  0xE
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_12_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_ibufmd_0_OFFSET  0x10
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_ibufmd_1_OFFSET  0x11
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_ibufmd_2_OFFSET  0x12
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_drv_0_OFFSET  0x13
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_drv_1_OFFSET  0x14
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_drv_2_OFFSET  0x15
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_drv_3_OFFSET  0x16
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_clamp_OFFSET  0x17
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_enhyst_OFFSET  0x18
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_lockdn_en_OFFSET  0x19
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_wpd_OFFSET  0x1A
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_wpu_OFFSET  0x1B
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_atp_en_OFFSET  0x1C
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_lp_persist_en_OFFSET  0x1D
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank4_io_cfg_12_13_cr_rpc_io_cfg_13_lp_bypass_en_MASK    (0x01 << 0x1E)

/*Configures the MSSIO block*/
#define mssio_bank2_cfg_cr_OFFSET   0x250
    /* Sets the PCODE value*/
    #define mssio_bank2_cfg_cr_bank_pcode_OFFSET  0x0
    #define mssio_bank2_cfg_cr_bank_pcode_MASK    (0x3F << 0x0)
    /* Sets the NCODE value*/
    #define mssio_bank2_cfg_cr_bank_ncode_OFFSET  0x6
    #define mssio_bank2_cfg_cr_bank_ncode_MASK    (0x3F << 0x6)
    /* Sets the voltage controls.*/
    #define mssio_bank2_cfg_cr_vs_OFFSET  0xC
    #define mssio_bank2_cfg_cr_vs_MASK    (0x0F << 0xC)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_0_1_cr_OFFSET   0x254
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_ibufmd_0_OFFSET  0x0
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_ibufmd_1_OFFSET  0x1
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_ibufmd_2_OFFSET  0x2
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_drv_0_OFFSET  0x3
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_drv_1_OFFSET  0x4
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_drv_2_OFFSET  0x5
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_drv_3_OFFSET  0x6
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_clamp_OFFSET  0x7
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_enhyst_OFFSET  0x8
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_lockdn_en_OFFSET  0x9
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_wpd_OFFSET  0xA
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_wpu_OFFSET  0xB
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_atp_en_OFFSET  0xC
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_lp_persist_en_OFFSET  0xD
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_lp_bypass_en_OFFSET  0xE
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_0_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_ibufmd_0_OFFSET  0x10
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_ibufmd_1_OFFSET  0x11
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_ibufmd_2_OFFSET  0x12
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_drv_0_OFFSET  0x13
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_drv_1_OFFSET  0x14
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_drv_2_OFFSET  0x15
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_drv_3_OFFSET  0x16
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_clamp_OFFSET  0x17
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_enhyst_OFFSET  0x18
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_lockdn_en_OFFSET  0x19
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_wpd_OFFSET  0x1A
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_wpu_OFFSET  0x1B
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_atp_en_OFFSET  0x1C
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_lp_persist_en_OFFSET  0x1D
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank2_io_cfg_0_1_cr_rpc_io_cfg_1_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_2_3_cr_OFFSET   0x258
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_ibufmd_0_OFFSET  0x0
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_ibufmd_1_OFFSET  0x1
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_ibufmd_2_OFFSET  0x2
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_drv_0_OFFSET  0x3
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_drv_1_OFFSET  0x4
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_drv_2_OFFSET  0x5
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_drv_3_OFFSET  0x6
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_clamp_OFFSET  0x7
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_enhyst_OFFSET  0x8
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_lockdn_en_OFFSET  0x9
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_wpd_OFFSET  0xA
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_wpu_OFFSET  0xB
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_atp_en_OFFSET  0xC
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_lp_persist_en_OFFSET  0xD
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_lp_bypass_en_OFFSET  0xE
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_2_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_ibufmd_0_OFFSET  0x10
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_ibufmd_1_OFFSET  0x11
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_ibufmd_2_OFFSET  0x12
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_drv_0_OFFSET  0x13
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_drv_1_OFFSET  0x14
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_drv_2_OFFSET  0x15
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_drv_3_OFFSET  0x16
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_clamp_OFFSET  0x17
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_enhyst_OFFSET  0x18
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_lockdn_en_OFFSET  0x19
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_wpd_OFFSET  0x1A
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_wpu_OFFSET  0x1B
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_atp_en_OFFSET  0x1C
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_lp_persist_en_OFFSET  0x1D
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank2_io_cfg_2_3_cr_rpc_io_cfg_3_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_4_5_cr_OFFSET   0x25C
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_ibufmd_0_OFFSET  0x0
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_ibufmd_1_OFFSET  0x1
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_ibufmd_2_OFFSET  0x2
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_drv_0_OFFSET  0x3
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_drv_1_OFFSET  0x4
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_drv_2_OFFSET  0x5
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_drv_3_OFFSET  0x6
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_clamp_OFFSET  0x7
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_enhyst_OFFSET  0x8
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_lockdn_en_OFFSET  0x9
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_wpd_OFFSET  0xA
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_wpu_OFFSET  0xB
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_atp_en_OFFSET  0xC
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_lp_persist_en_OFFSET  0xD
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_lp_bypass_en_OFFSET  0xE
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_4_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_ibufmd_0_OFFSET  0x10
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_ibufmd_1_OFFSET  0x11
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_ibufmd_2_OFFSET  0x12
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_drv_0_OFFSET  0x13
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_drv_1_OFFSET  0x14
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_drv_2_OFFSET  0x15
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_drv_3_OFFSET  0x16
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_clamp_OFFSET  0x17
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_enhyst_OFFSET  0x18
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_lockdn_en_OFFSET  0x19
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_wpd_OFFSET  0x1A
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_wpu_OFFSET  0x1B
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_atp_en_OFFSET  0x1C
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_lp_persist_en_OFFSET  0x1D
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank2_io_cfg_4_5_cr_rpc_io_cfg_5_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_6_7_cr_OFFSET   0x260
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_ibufmd_0_OFFSET  0x0
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_ibufmd_1_OFFSET  0x1
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_ibufmd_2_OFFSET  0x2
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_drv_0_OFFSET  0x3
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_drv_1_OFFSET  0x4
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_drv_2_OFFSET  0x5
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_drv_3_OFFSET  0x6
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_clamp_OFFSET  0x7
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_enhyst_OFFSET  0x8
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_lockdn_en_OFFSET  0x9
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_wpd_OFFSET  0xA
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_wpu_OFFSET  0xB
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_atp_en_OFFSET  0xC
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_lp_persist_en_OFFSET  0xD
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_lp_bypass_en_OFFSET  0xE
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_6_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_ibufmd_0_OFFSET  0x10
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_ibufmd_1_OFFSET  0x11
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_ibufmd_2_OFFSET  0x12
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_drv_0_OFFSET  0x13
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_drv_1_OFFSET  0x14
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_drv_2_OFFSET  0x15
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_drv_3_OFFSET  0x16
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_clamp_OFFSET  0x17
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_enhyst_OFFSET  0x18
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_lockdn_en_OFFSET  0x19
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_wpd_OFFSET  0x1A
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_wpu_OFFSET  0x1B
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_atp_en_OFFSET  0x1C
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_lp_persist_en_OFFSET  0x1D
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank2_io_cfg_6_7_cr_rpc_io_cfg_7_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_8_9_cr_OFFSET   0x264
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_ibufmd_0_OFFSET  0x0
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_ibufmd_1_OFFSET  0x1
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_ibufmd_2_OFFSET  0x2
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_drv_0_OFFSET  0x3
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_drv_1_OFFSET  0x4
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_drv_2_OFFSET  0x5
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_drv_3_OFFSET  0x6
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_clamp_OFFSET  0x7
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_enhyst_OFFSET  0x8
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_lockdn_en_OFFSET  0x9
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_wpd_OFFSET  0xA
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_wpu_OFFSET  0xB
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_atp_en_OFFSET  0xC
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_lp_persist_en_OFFSET  0xD
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_lp_bypass_en_OFFSET  0xE
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_8_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_ibufmd_0_OFFSET  0x10
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_ibufmd_1_OFFSET  0x11
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_ibufmd_2_OFFSET  0x12
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_drv_0_OFFSET  0x13
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_drv_1_OFFSET  0x14
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_drv_2_OFFSET  0x15
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_drv_3_OFFSET  0x16
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_clamp_OFFSET  0x17
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_enhyst_OFFSET  0x18
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_lockdn_en_OFFSET  0x19
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_wpd_OFFSET  0x1A
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_wpu_OFFSET  0x1B
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_atp_en_OFFSET  0x1C
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_lp_persist_en_OFFSET  0x1D
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank2_io_cfg_8_9_cr_rpc_io_cfg_9_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_10_11_cr_OFFSET   0x268
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_ibufmd_0_OFFSET  0x0
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_ibufmd_1_OFFSET  0x1
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_ibufmd_2_OFFSET  0x2
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_drv_0_OFFSET  0x3
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_drv_1_OFFSET  0x4
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_drv_2_OFFSET  0x5
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_drv_3_OFFSET  0x6
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_clamp_OFFSET  0x7
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_enhyst_OFFSET  0x8
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_lockdn_en_OFFSET  0x9
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_wpd_OFFSET  0xA
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_wpu_OFFSET  0xB
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_atp_en_OFFSET  0xC
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_lp_persist_en_OFFSET  0xD
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_lp_bypass_en_OFFSET  0xE
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_10_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_ibufmd_0_OFFSET  0x10
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_ibufmd_1_OFFSET  0x11
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_ibufmd_2_OFFSET  0x12
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_drv_0_OFFSET  0x13
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_drv_1_OFFSET  0x14
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_drv_2_OFFSET  0x15
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_drv_3_OFFSET  0x16
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_clamp_OFFSET  0x17
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_enhyst_OFFSET  0x18
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_lockdn_en_OFFSET  0x19
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_wpd_OFFSET  0x1A
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_wpu_OFFSET  0x1B
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_atp_en_OFFSET  0x1C
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_lp_persist_en_OFFSET  0x1D
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank2_io_cfg_10_11_cr_rpc_io_cfg_11_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_12_13_cr_OFFSET   0x26C
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_ibufmd_0_OFFSET  0x0
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_ibufmd_1_OFFSET  0x1
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_ibufmd_2_OFFSET  0x2
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_drv_0_OFFSET  0x3
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_drv_1_OFFSET  0x4
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_drv_2_OFFSET  0x5
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_drv_3_OFFSET  0x6
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_clamp_OFFSET  0x7
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_enhyst_OFFSET  0x8
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_lockdn_en_OFFSET  0x9
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_wpd_OFFSET  0xA
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_wpu_OFFSET  0xB
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_atp_en_OFFSET  0xC
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_lp_persist_en_OFFSET  0xD
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_lp_bypass_en_OFFSET  0xE
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_12_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_ibufmd_0_OFFSET  0x10
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_ibufmd_1_OFFSET  0x11
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_ibufmd_2_OFFSET  0x12
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_drv_0_OFFSET  0x13
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_drv_1_OFFSET  0x14
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_drv_2_OFFSET  0x15
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_drv_3_OFFSET  0x16
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_clamp_OFFSET  0x17
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_enhyst_OFFSET  0x18
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_lockdn_en_OFFSET  0x19
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_wpd_OFFSET  0x1A
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_wpu_OFFSET  0x1B
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_atp_en_OFFSET  0x1C
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_lp_persist_en_OFFSET  0x1D
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank2_io_cfg_12_13_cr_rpc_io_cfg_13_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_14_15_cr_OFFSET   0x270
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_ibufmd_0_OFFSET  0x0
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_ibufmd_1_OFFSET  0x1
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_ibufmd_2_OFFSET  0x2
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_drv_0_OFFSET  0x3
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_drv_1_OFFSET  0x4
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_drv_2_OFFSET  0x5
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_drv_3_OFFSET  0x6
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_clamp_OFFSET  0x7
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_enhyst_OFFSET  0x8
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_lockdn_en_OFFSET  0x9
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_wpd_OFFSET  0xA
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_wpu_OFFSET  0xB
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_atp_en_OFFSET  0xC
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_lp_persist_en_OFFSET  0xD
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_lp_bypass_en_OFFSET  0xE
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_14_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_ibufmd_0_OFFSET  0x10
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_ibufmd_1_OFFSET  0x11
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_ibufmd_2_OFFSET  0x12
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_drv_0_OFFSET  0x13
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_drv_1_OFFSET  0x14
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_drv_2_OFFSET  0x15
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_drv_3_OFFSET  0x16
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_clamp_OFFSET  0x17
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_enhyst_OFFSET  0x18
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_lockdn_en_OFFSET  0x19
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_wpd_OFFSET  0x1A
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_wpu_OFFSET  0x1B
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_atp_en_OFFSET  0x1C
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_lp_persist_en_OFFSET  0x1D
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank2_io_cfg_14_15_cr_rpc_io_cfg_15_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_16_17_cr_OFFSET   0x274
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_ibufmd_0_OFFSET  0x0
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_ibufmd_1_OFFSET  0x1
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_ibufmd_2_OFFSET  0x2
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_drv_0_OFFSET  0x3
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_drv_1_OFFSET  0x4
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_drv_2_OFFSET  0x5
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_drv_3_OFFSET  0x6
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_clamp_OFFSET  0x7
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_enhyst_OFFSET  0x8
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_lockdn_en_OFFSET  0x9
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_wpd_OFFSET  0xA
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_wpu_OFFSET  0xB
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_atp_en_OFFSET  0xC
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_lp_persist_en_OFFSET  0xD
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_lp_bypass_en_OFFSET  0xE
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_16_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_ibufmd_0_OFFSET  0x10
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_ibufmd_1_OFFSET  0x11
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_ibufmd_2_OFFSET  0x12
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_drv_0_OFFSET  0x13
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_drv_1_OFFSET  0x14
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_drv_2_OFFSET  0x15
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_drv_3_OFFSET  0x16
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_clamp_OFFSET  0x17
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_enhyst_OFFSET  0x18
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_lockdn_en_OFFSET  0x19
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_wpd_OFFSET  0x1A
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_wpu_OFFSET  0x1B
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_atp_en_OFFSET  0x1C
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_lp_persist_en_OFFSET  0x1D
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank2_io_cfg_16_17_cr_rpc_io_cfg_17_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_18_19_cr_OFFSET   0x278
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_ibufmd_0_OFFSET  0x0
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_ibufmd_1_OFFSET  0x1
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_ibufmd_2_OFFSET  0x2
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_drv_0_OFFSET  0x3
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_drv_1_OFFSET  0x4
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_drv_2_OFFSET  0x5
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_drv_3_OFFSET  0x6
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_clamp_OFFSET  0x7
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_enhyst_OFFSET  0x8
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_lockdn_en_OFFSET  0x9
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_wpd_OFFSET  0xA
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_wpu_OFFSET  0xB
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_atp_en_OFFSET  0xC
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_lp_persist_en_OFFSET  0xD
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_lp_bypass_en_OFFSET  0xE
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_18_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_ibufmd_0_OFFSET  0x10
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_ibufmd_1_OFFSET  0x11
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_ibufmd_2_OFFSET  0x12
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_drv_0_OFFSET  0x13
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_drv_1_OFFSET  0x14
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_drv_2_OFFSET  0x15
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_drv_3_OFFSET  0x16
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_clamp_OFFSET  0x17
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_enhyst_OFFSET  0x18
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_lockdn_en_OFFSET  0x19
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_wpd_OFFSET  0x1A
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_wpu_OFFSET  0x1B
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_atp_en_OFFSET  0x1C
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_lp_persist_en_OFFSET  0x1D
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank2_io_cfg_18_19_cr_rpc_io_cfg_19_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_20_21_cr_OFFSET   0x27C
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_ibufmd_0_OFFSET  0x0
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_ibufmd_1_OFFSET  0x1
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_ibufmd_2_OFFSET  0x2
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_drv_0_OFFSET  0x3
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_drv_1_OFFSET  0x4
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_drv_2_OFFSET  0x5
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_drv_3_OFFSET  0x6
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_clamp_OFFSET  0x7
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_enhyst_OFFSET  0x8
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_lockdn_en_OFFSET  0x9
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_wpd_OFFSET  0xA
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_wpu_OFFSET  0xB
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_atp_en_OFFSET  0xC
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_lp_persist_en_OFFSET  0xD
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_lp_bypass_en_OFFSET  0xE
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_20_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_ibufmd_0_OFFSET  0x10
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_ibufmd_1_OFFSET  0x11
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_ibufmd_2_OFFSET  0x12
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_drv_0_OFFSET  0x13
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_drv_1_OFFSET  0x14
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_drv_2_OFFSET  0x15
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_drv_3_OFFSET  0x16
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_clamp_OFFSET  0x17
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_enhyst_OFFSET  0x18
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_lockdn_en_OFFSET  0x19
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_wpd_OFFSET  0x1A
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_wpu_OFFSET  0x1B
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_atp_en_OFFSET  0x1C
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_lp_persist_en_OFFSET  0x1D
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank2_io_cfg_20_21_cr_rpc_io_cfg_21_lp_bypass_en_MASK    (0x01 << 0x1E)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_22_23_cr_OFFSET   0x280
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_ibufmd_0_OFFSET  0x0
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_ibufmd_0_MASK    (0x01 << 0x0)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_ibufmd_1_OFFSET  0x1
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_ibufmd_1_MASK    (0x01 << 0x1)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_ibufmd_2_OFFSET  0x2
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_ibufmd_2_MASK    (0x01 << 0x2)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_drv_0_OFFSET  0x3
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_drv_0_MASK    (0x01 << 0x3)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_drv_1_OFFSET  0x4
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_drv_1_MASK    (0x01 << 0x4)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_drv_2_OFFSET  0x5
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_drv_2_MASK    (0x01 << 0x5)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_drv_3_OFFSET  0x6
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_drv_3_MASK    (0x01 << 0x6)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_clamp_OFFSET  0x7
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_clamp_MASK    (0x01 << 0x7)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_enhyst_OFFSET  0x8
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_enhyst_MASK    (0x01 << 0x8)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_lockdn_en_OFFSET  0x9
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_lockdn_en_MASK    (0x01 << 0x9)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_wpd_OFFSET  0xA
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_wpd_MASK    (0x01 << 0xA)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_wpu_OFFSET  0xB
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_wpu_MASK    (0x01 << 0xB)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_atp_en_OFFSET  0xC
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_atp_en_MASK    (0x01 << 0xC)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_lp_persist_en_OFFSET  0xD
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_lp_persist_en_MASK    (0x01 << 0xD)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_lp_bypass_en_OFFSET  0xE
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_22_lp_bypass_en_MASK    (0x01 << 0xE)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_ibufmd_0_OFFSET  0x10
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_ibufmd_0_MASK    (0x01 << 0x10)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_ibufmd_1_OFFSET  0x11
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_ibufmd_1_MASK    (0x01 << 0x11)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_ibufmd_2_OFFSET  0x12
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_ibufmd_2_MASK    (0x01 << 0x12)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_drv_0_OFFSET  0x13
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_drv_0_MASK    (0x01 << 0x13)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_drv_1_OFFSET  0x14
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_drv_1_MASK    (0x01 << 0x14)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_drv_2_OFFSET  0x15
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_drv_2_MASK    (0x01 << 0x15)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_drv_3_OFFSET  0x16
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_drv_3_MASK    (0x01 << 0x16)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_clamp_OFFSET  0x17
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_clamp_MASK    (0x01 << 0x17)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_enhyst_OFFSET  0x18
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_enhyst_MASK    (0x01 << 0x18)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_lockdn_en_OFFSET  0x19
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_lockdn_en_MASK    (0x01 << 0x19)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_wpd_OFFSET  0x1A
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_wpd_MASK    (0x01 << 0x1A)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_wpu_OFFSET  0x1B
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_wpu_MASK    (0x01 << 0x1B)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_atp_en_OFFSET  0x1C
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_atp_en_MASK    (0x01 << 0x1C)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_lp_persist_en_OFFSET  0x1D
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_lp_persist_en_MASK    (0x01 << 0x1D)
    /* */
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_lp_bypass_en_OFFSET  0x1E
    #define mssio_bank2_io_cfg_22_23_cr_rpc_io_cfg_23_lp_bypass_en_MASK    (0x01 << 0x1E)

/*Sets H2F [31:0] Spares out signals*/
#define mss_spare0_cr_OFFSET   0x2A8
    /* See MSS MAS specification for full description*/
    #define mss_spare0_cr_data_OFFSET  0x0
    #define mss_spare0_cr_data_MASK    (0xFFFFFFFF << 0x0)

/*Sets H2F [37:32] Spares out signals*/
#define mss_spare1_cr_OFFSET   0x2AC
    /* See MSS MAS specification for full description*/
    #define mss_spare1_cr_data_OFFSET  0x0
    #define mss_spare1_cr_data_MASK    (0x3F << 0x0)

/*Read H2F [31:0] Spares out signals*/
#define mss_spare0_sr_OFFSET   0x2B0
    /* See MSS MAS specification for full description*/
    #define mss_spare0_sr_data_OFFSET  0x0
    #define mss_spare0_sr_data_MASK    (0xFFFFFFFF << 0x0)

/*Read H2F [37:32] Spares out signals*/
#define mss_spare1_sr_OFFSET   0x2B4
    /* See MSS MAS specification for full description*/
    #define mss_spare1_sr_data_OFFSET  0x0
    #define mss_spare1_sr_data_MASK    (0x3F << 0x0)

/*Read F2H [31:0] Spares in1 signals*/
#define mss_spare2_sr_OFFSET   0x2B8
    /* See MSS MAS specification for full description*/
    #define mss_spare2_sr_data_OFFSET  0x0
    #define mss_spare2_sr_data_MASK    (0xFFFFFFFF << 0x0)

/*Read F2H [37:32] Spares in1 signals*/
#define mss_spare3_sr_OFFSET   0x2BC
    /* See MSS MAS specification for full description*/
    #define mss_spare3_sr_data_OFFSET  0x0
    #define mss_spare3_sr_data_MASK    (0x3F << 0x0)

/*Read F2H [31:0] Spares in2 signals*/
#define mss_spare4_sr_OFFSET   0x2C0
    /* See MSS MAS specification for full description*/
    #define mss_spare4_sr_data_OFFSET  0x0
    #define mss_spare4_sr_data_MASK    (0xFFFFFFFF << 0x0)

/*Read F2H [37:32] Spares in2 signals*/
#define mss_spare5_sr_OFFSET   0x2C4
    /* See MSS MAS specification for full description*/
    #define mss_spare5_sr_data_OFFSET  0x0
    #define mss_spare5_sr_data_MASK    (0x3F << 0x0)

/*Register for ECO usage*/
#define SPARE_REGISTER_RW_OFFSET   0x2D0
    /* No function provided for future ECO use*/
    #define SPARE_REGISTER_RW_DATA_OFFSET  0x0
    #define SPARE_REGISTER_RW_DATA_MASK    (0xFF << 0x0)

/*Register for ECO usage*/
#define SPARE_REGISTER_W1P_OFFSET   0x2D4
    /* No function provided for future ECO use*/
    #define SPARE_REGISTER_W1P_DATA_OFFSET  0x0
    #define SPARE_REGISTER_W1P_DATA_MASK    (0xFF << 0x0)

/*Register for ECO usage*/
#define SPARE_REGISTER_RO_OFFSET   0x2D8
    /* Provides read-back of { W1P RW } registers. No function provided for future ECO use.*/
    #define SPARE_REGISTER_RO_DATA_OFFSET  0x0
    #define SPARE_REGISTER_RO_DATA_MASK    (0xFFFF << 0x0)

/*Spare signal back to G5C*/
#define SPARE_PERIM_RW_OFFSET   0x2DC
    /* Allows the MSS to control the perim_spare_out bits [2] & [6]. No function provided for future ECO use.*/
    #define SPARE_PERIM_RW_DATA_OFFSET  0x0
    #define SPARE_PERIM_RW_DATA_MASK    (0x03 << 0x0)

/*Unused FIC resets*/
#define SPARE_FIC_OFFSET   0x2E0
    /* Connected to spare FIC 0-3 Reset inputs to provide simple RO bits. No defined use*/
    #define SPARE_FIC_RESET_OFFSET  0x0
    #define SPARE_FIC_RESET_MASK    (0x0F << 0x0)
/*********************************************************************************
********************TOP LEVEL REGISTER STRUCTURE**********************************
**********************************************************************************/

typedef struct _mss_sysreg
{
    /*Register for software use*/
    __IO uint32_t TEMP0;

    /*Register for software use*/
     __IO uint32_t TEMP1;

    /*Master clock configuration*/
     __IO uint32_t CLOCK_CONFIG_CR;

    /*RTC clock divider*/
     __IO uint32_t RTC_CLOCK_CR;

    /*Fabric Reset mask*/
     __IO uint32_t FABRIC_RESET_CR;

    /**/
     __IO uint32_t BOOT_FAIL_CR;

    /* Padding reserved 32-bit registers.*/
     __I uint32_t reservedReg32b_0;

    /*Configuration lock*/
     __IO uint32_t CONFIG_LOCK_CR;

    /*Indicates which reset caused the last reset. After a reset occurs register should be read and then zero written to allow the next reset event to be correctly captured.*/
     __IO uint32_t RESET_SR;

    /*Indicates the device status in particular the state of the FPGA fabric and the MSS IO banks*/
     __IO uint32_t DEVICE_STATUS ;

    /*MSS Build Info*/
     __I uint32_t MSS_BUILD;

    /* Padding reserved 32-bit registers.*/
     __I uint32_t reservedReg32b_1;
     __I uint32_t reservedReg32b_2;
     __I uint32_t reservedReg32b_3;
     __I uint32_t reservedReg32b_4;
     __I uint32_t reservedReg32b_5;

    /*U54-1 Fabric interrupt enable*/
     __IO uint32_t FAB_INTEN_U54_1;

    /*U54-2 Fabric interrupt enable*/
     __IO uint32_t FAB_INTEN_U54_2;

    /*U54-3 Fabric interrupt enable*/
     __IO uint32_t FAB_INTEN_U54_3;

    /*U54-4 Fabric interrupt enable*/
     __IO uint32_t FAB_INTEN_U54_4;

    /*Allows the Ethernet interrupts to be directly routed to the U54 CPUS.*/
     __IO uint32_t FAB_INTEN_MISC;

    /*Switches GPIO interrupt from PAD to Fabric GPIO*/
     __IO uint32_t GPIO_INTERRUPT_FAB_CR;

    /* Padding reserved 32-bit registers.*/
     __I uint32_t reservedReg32b_6;
     __I uint32_t reservedReg32b_7;
     __I uint32_t reservedReg32b_8;
     __I uint32_t reservedReg32b_9;
     __I uint32_t reservedReg32b_10;
     __I uint32_t reservedReg32b_11;
     __I uint32_t reservedReg32b_12;
     __I uint32_t reservedReg32b_13;
     __I uint32_t reservedReg32b_14;
     __I uint32_t reservedReg32b_15;

    /*"AMP Mode peripheral mapping register. When the register bit is '0' the peripheral is mapped into the 0x2000000 address range using AXI bus 5 from the Coreplex. When the register bit is '1' the peripheral is mapped into the 0x28000000 address range using AXI bus 6 from the Coreplex."*/
     __IO uint32_t APBBUS_CR;

    /*"Enables the clock to the MSS peripheral. By turning clocks off dynamic power can be saved. When the clock is off the peripheral  should not be accessed*/
     __IO uint32_t SUBBLK_CLOCK_CR;

    /*"Holds the MSS peripherals in reset. When in reset the peripheral should not be accessed*/
     __IO uint32_t SOFT_RESET_CR;

    /*Configures how many outstanding transfers the AXI-AHB bridges in front off the USB and Crypto blocks should allow. (See Synopsys AXI-AHB bridge documentation)*/
     __IO uint32_t AHBAXI_CR;

    /*Configures the two AHB-APB bridges on S5 and S6*/
     __IO uint32_t AHBAPB_CR;

    /* Padding reserved 32-bit registers.*/
     uint32_t reservedReg32b_16;

    /*MSS Corner APB interface controls*/
     __IO uint32_t DFIAPB_CR;

    /*GPIO Blocks reset control*/
     __IO uint32_t GPIO_CR;

    /* Padding reserved 32-bit registers.*/
     uint32_t reservedReg32b_17;

    /*MAC0 configuration register*/
     __IO uint32_t MAC0_CR;

    /*MAC1 configuration register*/
     __IO uint32_t MAC1_CR;

    /*USB Configuration register*/
     __IO uint32_t USB_CR;

    /*Crypto Mesh control and status register*/
     __IO uint32_t MESH_CR;

    /*Crypto mesh seed and update rate*/
     __IO uint32_t MESH_SEED_CR;

    /*ENVM AHB Controller setup*/
     __IO uint32_t ENVM_CR;

    /*Reserved*/
     __I uint32_t reserved_BC;

    /*QOS Athena USB & MMC Configuration*/
     __IO uint32_t qos_peripheral_cr;

    /*QOS Configuration Coreplex*/
     __IO uint32_t qos_cplexio_cr;

    /*QOS configuration DDRC*/
     __IO uint32_t qos_cplexddr_cr;

    /* Padding reserved 32-bit registers.*/
     __I uint32_t reservedReg32b_18;
     __I uint32_t reservedReg32b_19;
     __I uint32_t reservedReg32b_20;
     __I uint32_t reservedReg32b_21;
     __I uint32_t reservedReg32b_22;
     __I uint32_t reservedReg32b_23;
     __I uint32_t reservedReg32b_24;
     __I uint32_t reservedReg32b_25;
     __I uint32_t reservedReg32b_26;

    /*Indicates that a master caused a MPU violation. Interrupts via maintenance interrupt.*/
     __IO uint32_t mpu_violation_sr;

    /*Enables interrupts on MPU violations*/
     __IO uint32_t mpu_violation_inten_cr;

    /*AXI switch decode fail*/
     __IO uint32_t SW_FAIL_ADDR0_CR;

    /*AXI switch decode fail*/
     __IO uint32_t SW_FAIL_ADDR1_CR;

    /*Set when an ECC event happens*/
     __IO uint32_t EDAC_SR;

    /*Enables ECC interrupt on event*/
     __IO uint32_t EDAC_INTEN_CR;

    /*Count off single bit errors*/
     __IO uint32_t EDAC_CNT_MMC;

    /*Count off single bit errors*/
     __IO uint32_t EDAC_CNT_DDRC;

    /*Count off single bit errors*/
     __IO uint32_t EDAC_CNT_MAC0;

    /*Count off single bit errors*/
     __IO uint32_t EDAC_CNT_MAC1;

    /*Count off single bit errors*/
     __IO uint32_t EDAC_CNT_USB;

    /*Count off single bit errors*/
     __IO uint32_t EDAC_CNT_CAN0;

    /*Count off single bit errors*/
     __IO uint32_t EDAC_CNT_CAN1;

    /*"Will Corrupt write data to rams 1E corrupts bit 0 2E bits 1 and 2.Injects Errors into all RAMS in the block as long as the bits are set. Setting 1E and 2E will inject a 3-bit error"*/
     __IO uint32_t EDAC_INJECT_CR;

    /* Padding reserved 32-bit registers.*/
     __I uint32_t reservedReg32b_27;
     __I uint32_t reservedReg32b_28;
     __I uint32_t reservedReg32b_29;
     __I uint32_t reservedReg32b_30;
     __I uint32_t reservedReg32b_31;
     __I uint32_t reservedReg32b_32;

    /*Maintenance Interrupt Enable.*/
     __IO uint32_t MAINTENANCE_INTEN_CR;

    /*PLL Status interrupt enables*/
     __IO uint32_t PLL_STATUS_INTEN_CR;

    /*Maintenance interrupt indicates fault and status events.*/
     __IO uint32_t MAINTENANCE_INT_SR;

    /*PLL interrupt register*/
     __IO uint32_t PLL_STATUS_SR;

    /*Enable to CFM Timer */
     __IO uint32_t CFM_TIMER_CR;

    /*Miscellaneous Register*/
     uint32_t MISC_SR;

    /*DLL Interrupt enables*/
     __IO uint32_t DLL_STATUS_CR;

    /*DLL interrupt register*/
     __IO uint32_t DLL_STATUS_SR;

    /* Padding reserved 32-bit registers.*/
     __I uint32_t reservedReg32b_33;
     __I uint32_t reservedReg32b_34;

    /*Puts all the RAMS in that block into low leakage mode. RAM contents and Q value preserved.*/
     __IO uint32_t RAM_LIGHTSLEEP_CR;

    /*Puts all the RAMS in that block into deep sleep mode. RAM contents preserved. Powers down the periphery circuits.*/
     __IO uint32_t RAM_DEEPSLEEP_CR;

    /*Puts all the RAMS in that block into shut down mode. RAM contents not preserved.  Powers down the RAM and periphery circuits.*/
     __IO uint32_t RAM_SHUTDOWN_CR;

    /*Allows each bank of the L2 Cache to be powered down ORed with global shutdown */
     __IO uint32_t L2_SHUTDOWN_CR;

    /* Padding reserved 32-bit registers.*/
     __I uint32_t reservedReg32b_35;
     __I uint32_t reservedReg32b_36;
     __I uint32_t reservedReg32b_37;
     __I uint32_t reservedReg32b_38;
     __I uint32_t reservedReg32b_39;
     __I uint32_t reservedReg32b_40;
     __I uint32_t reservedReg32b_41;
     __I uint32_t reservedReg32b_42;
     __I uint32_t reservedReg32b_43;
     __I uint32_t reservedReg32b_44;
     __I uint32_t reservedReg32b_45;
     __I uint32_t reservedReg32b_46;
     __I uint32_t reservedReg32b_47;
     __I uint32_t reservedReg32b_48;
     __I uint32_t reservedReg32b_49;
     __I uint32_t reservedReg32b_50;
     __I uint32_t reservedReg32b_51;
     __I uint32_t reservedReg32b_52;
     __I uint32_t reservedReg32b_53;
     __I uint32_t reservedReg32b_54;
     __I uint32_t reservedReg32b_55;
     __I uint32_t reservedReg32b_56;
     __I uint32_t reservedReg32b_57;
     __I uint32_t reservedReg32b_58;
     __I uint32_t reservedReg32b_59;
     __I uint32_t reservedReg32b_60;
     __I uint32_t reservedReg32b_61;
     __I uint32_t reservedReg32b_62;
     __I uint32_t reservedReg32b_63;
     __I uint32_t reservedReg32b_64;
     __I uint32_t reservedReg32b_65;
     __I uint32_t reservedReg32b_66;
     __I uint32_t reservedReg32b_67;
     __I uint32_t reservedReg32b_68;

    /*Selects whether the peripheral is connected to the Fabric or IOMUX structure.*/
     __IO uint32_t IOMUX0_CR;

    /*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
     __IO uint32_t IOMUX1_CR;

    /*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
     __IO uint32_t IOMUX2_CR;

    /*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
     __IO uint32_t IOMUX3_CR;

    /*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
     __IO uint32_t IOMUX4_CR;

    /*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
     __IO uint32_t IOMUX5_CR;

    /*Sets whether the MMC/SD Voltage select lines are inverted on entry to the IOMUX structure*/
     __IO uint32_t IOMUX6_CR;

    /* Padding reserved 32-bit registers.*/
     __I uint32_t reservedReg32b_69;
     __I uint32_t reservedReg32b_70;
     __I uint32_t reservedReg32b_71;
     __I uint32_t reservedReg32b_72;
     __I uint32_t reservedReg32b_73;

    /*Configures the MSSIO block*/
     __IO uint32_t mssio_bank4_cfg_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank4_io_cfg_0_1_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank4_io_cfg_2_3_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank4_io_cfg_4_5_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank4_io_cfg_6_7_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank4_io_cfg_8_9_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank4_io_cfg_10_11_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank4_io_cfg_12_13_cr;

    /*Configures the MSSIO block*/
     __IO uint32_t mssio_bank2_cfg_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank2_io_cfg_0_1_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank2_io_cfg_2_3_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank2_io_cfg_4_5_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank2_io_cfg_6_7_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank2_io_cfg_8_9_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank2_io_cfg_10_11_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank2_io_cfg_12_13_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank2_io_cfg_14_15_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank2_io_cfg_16_17_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank2_io_cfg_18_19_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank2_io_cfg_20_21_cr;

    /*IO electrical configuration for MSSIO pad*/
     __IO uint32_t mssio_bank2_io_cfg_22_23_cr;

    /* Padding reserved 32-bit registers.*/
     __I uint32_t reservedReg32b_74;
     __I uint32_t reservedReg32b_75;
     __I uint32_t reservedReg32b_76;
     __I uint32_t reservedReg32b_77;
     __I uint32_t reservedReg32b_78;
     __I uint32_t reservedReg32b_79;
     __I uint32_t reservedReg32b_80;
     __I uint32_t reservedReg32b_81;
     __I uint32_t reservedReg32b_82;

    /*Sets H2F [31:0] Spares out signals*/
     __IO uint32_t mss_spare0_cr;

    /*Sets H2F [37:32] Spares out signals*/
     __IO uint32_t mss_spare1_cr;

    /*Read H2F [31:0] Spares out signals*/
     __IO uint32_t mss_spare0_sr;

    /*Read H2F [37:32] Spares out signals*/
     __IO uint32_t mss_spare1_sr;

    /*Read F2H [31:0] Spares in1 signals*/
     __IO uint32_t mss_spare2_sr;

    /*Read F2H [37:32] Spares in1 signals*/
     __IO uint32_t mss_spare3_sr;

    /*Read F2H [31:0] Spares in2 signals*/
     __IO uint32_t mss_spare4_sr;

    /*Read F2H [37:32] Spares in2 signals*/
     __IO uint32_t mss_spare5_sr;

    /* Padding reserved 32-bit registers.*/
     __I uint32_t reservedReg32b_83;
     __I uint32_t reservedReg32b_84;

    /*Register for ECO usage*/
     __IO uint32_t SPARE_REGISTER_RW;

    /*Register for ECO usage*/
     __IO uint32_t SPARE_REGISTER_W1P;

    /*Register for ECO usage*/
     __I uint32_t SPARE_REGISTER_RO;

    /*Spare signal back to G5C*/
     __IO uint32_t SPARE_PERIM_RW;

    /*Unused FIC resets*/
     __I uint32_t SPARE_FIC;
} mss_sysreg_t;

#define SYSREG_ATHENACR_RESET       (1U << 0U)
#define SYSREG_ATHENACR_PURGE       (1U << 1U)
#define SYSREG_ATHENACR_GO          (1U << 2U)
#define SYSREG_ATHENACR_RINGOSCON   (1U << 3U)
#define SYSREG_ATHENACR_COMPLETE    (1U << 8U)
#define SYSREG_ATHENACR_ALARM       (1U << 9U)
#define SYSREG_ATHENACR_BUSERROR    (1U << 10U)
#define SYSREG_SOFTRESET_ENVM       (1U << 0U)
#define SYSREG_SOFTRESET_TIMER      (1U << 4U)
#define SYSREG_SOFTRESET_MMUART0    (1U << 5U)
#define SYSREG_SOFTRESET_DDRC       (1U << 23U)
#define SYSREG_SOFTRESET_FIC3       (1U << 27U)
#define SYSREG_SOFTRESET_ATHENA     (1U << 28U)

extern mss_sysreg_t*   SYSREG;

#ifdef __cplusplus
}
#endif

#endif /*MSS_SYSREG_H*/
