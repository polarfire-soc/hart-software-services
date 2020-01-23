/***************************************************************************
 * (c) Copyright 2007-2018 Microsemi SoC Products Group. All rights reserved.
 * 
 * Hardware register definitions.
 * updated sysreg defines Based on SVN rev 100432 of g5_mss_top_sysreg.xls
 * 
 * SVN $Revision: 9661 $
 * SVN $Date: 2018-01-15 10:43:33 +0000 (Mon, 15 Jan 2018) $
 */
#ifndef MSS_SYSREG_H
#define MSS_SYSREG_H
 
#include <stdint.h>
 



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
	/* "Sets the master synchronous clock dividerbits [1:0] CPU clock dividerbits [3:2] AXI clock dividerbits [5:4] AHB/APB clock divider00=/1   01=/2  10=/4 11=/8  (AHB/APB divider may not be set to /1)"*/
	#define CLOCK_CONFIG_CR_divider_OFFSET  0x0
	#define CLOCK_CONFIG_CR_divider_MASK    (0x3F << 0x0)

/*RTC clock divider*/
#define RTC_CLOCK_CR_OFFSET   0xC
	/* "Sets the  division ratio to create the internal RTC clock from the reference clock. The defaults sets the reference clock to 1MHz assuming the reference clock is 100Mhz.If the reference clock is 125MHz then 125 will create a 1MHz clockMax divider value is 4095 and value must be an integer.RTC clock must be less 2X the AXI clock rate."*/
	#define RTC_CLOCK_CR_period_OFFSET  0x0
	#define RTC_CLOCK_CR_period_MASK    (0xFFF << 0x0)

/*Fabric Reset mask*/
#define FABRIC_RESET_CR_OFFSET   0x10
	/* Blocks the fabric Reset input preventing the fabric reseting the MSS*/
	#define FABRIC_RESET_CR_ENABLE_OFFSET  0x0
	#define FABRIC_RESET_CR_ENABLE_MASK    (0x01 << 0x0)

/**/
#define BOOT_FAIL_CR_OFFSET   0x14
	/* "Written by firmware to indicate that the boot process failed*/
	#define BOOT_FAIL_CR_boot_OFFSET  0x0
	#define BOOT_FAIL_CR_boot_MASK    (0x01 << 0x0)

/*Controls the MSS Corner low power mode signals*/
#define MSS_LOW_POWER_CR_OFFSET   0x18
	/* Direct control of MSS Corner LP state control*/
	#define MSS_LOW_POWER_CR_lp_state_mss_OFFSET  0x0
	#define MSS_LOW_POWER_CR_lp_state_mss_MASK    (0x01 << 0x0)

/*Configuration lock*/
#define CONFIG_LOCK_CR_OFFSET   0x1C
	/* When written to '1' will cause all RWC registers to lock until a master reset occurs.*/
	#define CONFIG_LOCK_CR_lock_OFFSET  0x0
	#define CONFIG_LOCK_CR_lock_MASK    (0x01 << 0x0)

/*Indicates which reset caused the last reset. After a reset occurs register should be read and then zro written to allow the next reset event to be correctly captured.*/
#define RESET_SR_OFFSET   0x20
	/* Reset was caused by the SCB periphery reset signal*/
	#define RESET_SR_SCB_PERIPH_RESET_OFFSET  0x0
	#define RESET_SR_SCB_PERIPH_RESET_MASK    (0x01 << 0x0)

/*Indicates the device status in particular the state of the FPGA fabric and the MSS IO banks*/
#define DEVICE_STATUS_OFFSET   0x24
	/* Indicates the status of the core_up input from G5 Control.*/
	#define DEVICE_STATUS_CORE_UP_OFFSET  0x0
	#define DEVICE_STATUS_CORE_UP_MASK    (0x01 << 0x0)

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

/*"Enables the clock to the MSS peripheral. By turning clocks off dynamic power can be saved. When the clock is off the peripheral  should not be accessed*/
#define SUBBLK_CLOCK_CR_OFFSET   0x84

/*"Holds the MSS peripherals in reset. Whenin reset the peripheral  should not be accessed*/
#define SOFT_RESET_CR_OFFSET   0x88

/*Configures how many outstanding transfers the AXI-AHB bridges in front off the USB and Crypto blocks should allow. (See Synopsys AXI-AHB bridge documentation)*/
#define AHBAXI_CR_OFFSET   0x8C
	/* Number of outstanding write transactions to USB block*/
	#define AHBAXI_CR_USB_WBCNT_OFFSET  0x0
	#define AHBAXI_CR_USB_WBCNT_MASK    (0x0F << 0x0)

/*MSS Corner APB interface controls*/
#define DFIAPB_CR_OFFSET   0x98
	/* Turns on the APB clock to the MSS Corne is off at reset. Once corner blocks is configured the firmware may turn off the clock but periodically should turn back on to allow refresh of TMR registers inside the corner block. */
	#define DFIAPB_CR_clockon_OFFSET  0x0
	#define DFIAPB_CR_clockon_MASK    (0x01 << 0x0)

/*GPIO Blocks reset control*/
#define GPIO_CR_OFFSET   0x9C
	/* "This signal selects whether the associated byte is reset by soft reset or the the MSS_GPIO_RESET_N signal from the FPGA fabric. The allowed values are:• 0: Selects  MSS_GPIO_RESET_N signal from the FPGA fabric.• 1: Selects  the GPIO to be reset by the GPIO block soft reset signal .Bit 0 controls GPIO0 [7:0] */
	#define GPIO_CR_gpio0_soft_reset_select_OFFSET  0x0
	#define GPIO_CR_gpio0_soft_reset_select_MASK    (0x03 << 0x0)
	/* "Sets the reset value off the GPIO0 per byteBit 0 controls GPIO0 [7:0] */
	#define GPIO_CR_gpio0_default_OFFSET  0x4
	#define GPIO_CR_gpio0_default_MASK    (0x03 << 0x4)
	/* "This signal selects whether the associated byte is reset by soft reset or the the MSS_GPIO_RESET_N signal from the FPGA fabric. The allowed values are:• 0: Selects  MSS_GPIO_RESET_N signal from the FPGA fabric.• 1: Selects  the GPIO to be reset by the GPIO block soft reset signal .Bit 0 controls GPIO0 [7:0] */
	#define GPIO_CR_gpio1_soft_reset_select_OFFSET  0x8
	#define GPIO_CR_gpio1_soft_reset_select_MASK    (0x07 << 0x8)
	/* "Sets the reset value off the GPIO0 per byteBit 0 controls GPIO0 [7:0] */
	#define GPIO_CR_gpio1_default_OFFSET  0xC
	#define GPIO_CR_gpio1_default_MASK    (0x07 << 0xC)
	/* "This signal selects whether the associated byte is reset by soft reset or the the MSS_GPIO_RESET_N signal from the FPGA fabric. The allowed values are:• 0: Selects  MSS_GPIO_RESET_N signal from the FPGA fabric.• 1: Selects  the GPIO to be reset by the GPIO block soft reset signal .Bit 0 controls GPIO0 [7:0] */
	#define GPIO_CR_gpio2_soft_reset_select_OFFSET  0x10
	#define GPIO_CR_gpio2_soft_reset_select_MASK    (0x0F << 0x10)
	/* poreset_n*/
	#define GPIO_CR__OFFSET  0x4

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
	/* "Configures USB for Single-Data Rate(SDR) mode or Double-Data Rate(DDR) mode. 0 - SDR Mode is selected1 - DDR Mode is selected."*/
	#define USB_CR_DDR_SELECT_OFFSET  0x0
	#define USB_CR_DDR_SELECT_MASK    (0x01 << 0x0)

/*Crypto Mesh control and status register*/
#define MESH_CR_OFFSET   0xB0
	/* Writing a 1 will start the Mesh System*/
	#define MESH_CR_START_OFFSET  0x0
	#define MESH_CR_START_MASK    (0x01 << 0x0)

/*Crypto mesh seed and update rate*/
#define MESH_SEED_CR_OFFSET   0xB4
	/* "Sets the mesh seed value any value may be used*/
	#define MESH_SEED_CR_seed_OFFSET  0x0
	#define MESH_SEED_CR_seed_MASK    (0x7FFFFF << 0x0)
	/* "Sets the rate that the mesh value is changed. Rate = AHBCLK/(clkrate+1). Rate must be less than 1MHz*/
	#define MESH_SEED_CR_clkrate_OFFSET  0x18
	#define MESH_SEED_CR_clkrate_MASK    (0xFF << 0x18)

/*ENVM AHB Controller setup*/
#define ENVM_CR_OFFSET   0xB8
	/* "Sets the number of  AHB cycles used to generate the PNVM clockClock  period = (Value+1) * (1000/AHBFREQMHZ)         Value must be 1 to 63  (0 defaults to 15)e.g.11  will generate a 40ns period  25MHz clock if the AHB clock is 250MHz15  will generate a 40ns period  25MHz clock if the AHB clock is 400MHz"*/
	#define ENVM_CR_clock_period_OFFSET  0x0
	#define ENVM_CR_clock_period_MASK    (0x3F << 0x0)

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

/*QOS Configuration Coreplex*/
#define qos_cplexio_cr_OFFSET   0xC4
	/* Sets the QOS value from the specified device into the switch*/
	#define qos_cplexio_cr_device0_read_OFFSET  0x0
	#define qos_cplexio_cr_device0_read_MASK    (0x0F << 0x0)

/*QOS configuration DDRC*/
#define qos_cplexddr_cr_OFFSET   0xC8
	/* Sets the QOS value from the specified device into the switch*/
	#define qos_cplexddr_cr_cache_read_OFFSET  0x0
	#define qos_cplexddr_cr_cache_read_MASK    (0x0F << 0x0)

/*Indicates that a master caused a MPU violation. Interrupts via maintenance interrupt.*/
#define mpu_violation_sr_OFFSET   0xF0
	/* Bit is set on violation. Cleared by writing '1'*/
	#define mpu_violation_sr_fic0_OFFSET  0x0
	#define mpu_violation_sr_fic0_MASK    (0x01 << 0x0)

/*Enables interrupts on MPU violations*/
#define mpu_violation_inten_cr_OFFSET   0xF4
	/* Enables the interrupt*/
	#define mpu_violation_inten_cr_fic0_OFFSET  0x0
	#define mpu_violation_inten_cr_fic0_MASK    (0x01 << 0x0)

/*AXI switch decode fail*/
#define SW_FAIL_ADDR0_CR_OFFSET   0xF8
	/* The address (bits 31:0) that failed. Redaing this address as 64-bits will return the 38-bit address in assingle read combined with additional information in the next register*/
	#define SW_FAIL_ADDR0_CR_addr_OFFSET  0x0
	#define SW_FAIL_ADDR0_CR_addr_MASK    (0xFFFFFFFF << 0x0)

/*AXI switch decode fail*/
#define SW_FAIL_ADDR1_CR_OFFSET   0xFC
	/* Upper 6 bits off address [37:32]*/
	#define SW_FAIL_ADDR1_CR_addr_OFFSET  0x0
	#define SW_FAIL_ADDR1_CR_addr_MASK    (0x3F << 0x0)

/*Set when an ECC event happens*/
#define EDAC_SR_OFFSET   0x100
	/* */
	#define EDAC_SR_MMC_1E_OFFSET  0x0
	#define EDAC_SR_MMC_1E_MASK    (0x01 << 0x0)

/*Enables ECC interrupt on event*/
#define EDAC_INTEN_CR_OFFSET   0x104
	/* */
	#define EDAC_INTEN_CR_MMC_1E_OFFSET  0x0
	#define EDAC_INTEN_CR_MMC_1E_MASK    (0x01 << 0x0)

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

/*Maintenance Interrupt Enable.*/
#define MAINTENANCE_INTEN_CR_OFFSET   0x140
	/* Enables interrupt on a PLL event PLL_STATUS_INTEN_CR should also be set*/
	#define MAINTENANCE_INTEN_CR_pll_OFFSET  0x0
	#define MAINTENANCE_INTEN_CR_pll_MASK    (0x01 << 0x0)

/*PLL Status interrupt enables*/
#define PLL_STATUS_INTEN_CR_OFFSET   0x144
	/* Enables interrupt on CPU PLL locking*/
	#define PLL_STATUS_INTEN_CR_CPU_LOCK_OFFSET  0x0
	#define PLL_STATUS_INTEN_CR_CPU_LOCK_MASK    (0x01 << 0x0)

/*Maintenace interrupt indicates fault and status events.*/
#define MAINTENANCE_INT_SR_OFFSET   0x148
	/* Indicates that one off the PLLs whent into the lock or unlock state. Cleared via PLL event register*/
	#define MAINTENANCE_INT_SR_pll_OFFSET  0x0
	#define MAINTENANCE_INT_SR_pll_MASK    (0x01 << 0x0)

/*PLL interrupt register*/
#define PLL_STATUS_SR_OFFSET   0x14C
	/* "Indicates that the CPU PLL has locked*/
	#define PLL_STATUS_SR_CPU_LOCK_OFFSET  0x0
	#define PLL_STATUS_SR_CPU_LOCK_MASK    (0x01 << 0x0)
	/* "Indicates that the DFI PLL has locked*/
	#define PLL_STATUS_SR_DFI_LOCK_OFFSET  0x1
	#define PLL_STATUS_SR_DFI_LOCK_MASK    (0x01 << 0x1)
	/* "Indicates that the SGMII PLL has locked*/
	#define PLL_STATUS_SR_SGMII_LOCK_OFFSET  0x2
	#define PLL_STATUS_SR_SGMII_LOCK_MASK    (0x01 << 0x2)
	/* "Indicates that the CPU PLL has unlocked*/
	#define PLL_STATUS_SR_CPU_UNLOCK_OFFSET  0x4
	#define PLL_STATUS_SR_CPU_UNLOCK_MASK    (0x01 << 0x4)
	/* "Indicates that the DFI PLL has unlocked*/
	#define PLL_STATUS_SR_DFI_UNLOCK_OFFSET  0x5
	#define PLL_STATUS_SR_DFI_UNLOCK_MASK    (0x01 << 0x5)
	/* "Indicates that the SGMII PLL has unlocked*/
	#define PLL_STATUS_SR_SGMII_UNLOCK_OFFSET  0x6
	#define PLL_STATUS_SR_SGMII_UNLOCK_MASK    (0x01 << 0x6)
	/* Current state off CPU PLL locked signal*/
	#define PLL_STATUS_SR_CPU_LOCK_NOW_OFFSET  0x8
	#define PLL_STATUS_SR_CPU_LOCK_NOW_MASK    (0x01 << 0x8)

/*Enable to CFM Timer */
#define CFM_TIMER_CR_OFFSET   0x150
	/* When set and the CFM channel is in timer mode and CFM channel is set to 2 (Group C) this register allows the timet to count. Allows software to start and stop the timers.*/
	#define CFM_TIMER_CR_Enable_OFFSET  0x0
	#define CFM_TIMER_CR_Enable_MASK    (0x1F << 0x0)

/*Miscellanous Register*/
#define MISC_SR_OFFSET   0x154
	/* Indicates that Interrupt from the G5C MSS SCB SPI controller is active*/
	#define MISC_SR_CONT_SPI_INTERRUPT_OFFSET  0x0
	#define MISC_SR_CONT_SPI_INTERRUPT_MASK    (0x01 << 0x0)

/*Puts all the RAMS in that block into low leakage mode. RAM contents and Q value preserved.*/
#define RAM_LIGHTSLEEP_CR_OFFSET   0x168
	/* */
	#define RAM_LIGHTSLEEP_CR_can0_OFFSET  0x0
	#define RAM_LIGHTSLEEP_CR_can0_MASK    (0x01 << 0x0)

/*Puts all the RAMS in that block into deep sleep mode. RAM contents preserved. Powers down the periphery circuits.*/
#define RAM_DEEPSLEEP_CR_OFFSET   0x16C
	/* */
	#define RAM_DEEPSLEEP_CR_can0_OFFSET  0x0
	#define RAM_DEEPSLEEP_CR_can0_MASK    (0x01 << 0x0)

/*Puts all the RAMS in that block into shut down mode. RAM contents not preserved.  Powers down the RAM and periphery circuits.*/
#define RAM_SHUTDOWN_CR_OFFSET   0x170
	/* */
	#define RAM_SHUTDOWN_CR_can0_OFFSET  0x0
	#define RAM_SHUTDOWN_CR_can0_MASK    (0x01 << 0x0)

/*Selects whether the peripheral is connected to the Fabric or IOMUX structure.*/
#define IOMUX0_CR_OFFSET   0x200
	/* */
	#define IOMUX0_CR_spi0_fabric_OFFSET  0x0
	#define IOMUX0_CR_spi0_fabric_MASK    (0x01 << 0x0)

/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
#define IOMUX1_CR_OFFSET   0x204
	/* */
	#define IOMUX1_CR_pad0_OFFSET  0x0
	#define IOMUX1_CR_pad0_MASK    (0x0F << 0x0)

/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
#define IOMUX2_CR_OFFSET   0x208
	/* */
	#define IOMUX2_CR_pad8_OFFSET  0x0
	#define IOMUX2_CR_pad8_MASK    (0x0F << 0x0)

/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
#define IOMUX3_CR_OFFSET   0x20C
	/* */
	#define IOMUX3_CR_pad14_OFFSET  0x0
	#define IOMUX3_CR_pad14_MASK    (0x0F << 0x0)

/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
#define IOMUX4_CR_OFFSET   0x210
	/* */
	#define IOMUX4_CR_pad22_OFFSET  0x0
	#define IOMUX4_CR_pad22_MASK    (0x0F << 0x0)

/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
#define IOMUX5_CR_OFFSET   0x214
	/* */
	#define IOMUX5_CR_pad30_OFFSET  0x0
	#define IOMUX5_CR_pad30_MASK    (0x0F << 0x0)

/*Sets whether the MMC/SD Voltage select lines are inverted on entry to the IOMUX structure*/
#define IOMUX6_CR_OFFSET   0x218
	/* */
	#define IOMUX6_CR_VLT_SEL_OFFSET  0x0
	#define IOMUX6_CR_VLT_SEL_MASK    (0x01 << 0x0)

/*Configures the MSSIO block*/
#define mssio_bank4_cfg_cr_OFFSET   0x230
	/* Sets the PCODE value*/
	#define mssio_bank4_cfg_cr_   bank_pcode_OFFSET  0x0
	#define mssio_bank4_cfg_cr_   bank_pcode_MASK    (0x3F << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank4_io_cfg_0_cr_OFFSET   0x234
	/* */
	#define mssio_bank4_io_cfg_0_cr_   io_cfg_0_OFFSET  0x0
	#define mssio_bank4_io_cfg_0_cr_   io_cfg_0_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank4_io_cfg_1_cr_OFFSET   0x238
	/* */
	#define mssio_bank4_io_cfg_1_cr_   io_cfg_2_OFFSET  0x0
	#define mssio_bank4_io_cfg_1_cr_   io_cfg_2_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank4_io_cfg_2_cr_OFFSET   0x23C
	/* */
	#define mssio_bank4_io_cfg_2_cr_   io_cfg_4_OFFSET  0x0
	#define mssio_bank4_io_cfg_2_cr_   io_cfg_4_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank4_io_cfg_3_cr_OFFSET   0x240
	/* */
	#define mssio_bank4_io_cfg_3_cr_   io_cfg_6_OFFSET  0x0
	#define mssio_bank4_io_cfg_3_cr_   io_cfg_6_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank4_io_cfg_4_cr_OFFSET   0x244
	/* */
	#define mssio_bank4_io_cfg_4_cr_   io_cfg_8_OFFSET  0x0
	#define mssio_bank4_io_cfg_4_cr_   io_cfg_8_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank4_io_cfg_5_cr_OFFSET   0x248
	/* */
	#define mssio_bank4_io_cfg_5_cr_   io_cfg_10_OFFSET  0x0
	#define mssio_bank4_io_cfg_5_cr_   io_cfg_10_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank4_io_cfg_6_cr_OFFSET   0x24C
	/* */
	#define mssio_bank4_io_cfg_6_cr_   io_cfg_12_OFFSET  0x0
	#define mssio_bank4_io_cfg_6_cr_   io_cfg_12_MASK    (0xFFFF << 0x0)

/*Configures the MSSIO block*/
#define mssio_bank2_cfg_cr_OFFSET   0x250
	/* Sets the PCODE value*/
	#define mssio_bank2_cfg_cr_   bank_pcode_OFFSET  0x0
	#define mssio_bank2_cfg_cr_   bank_pcode_MASK    (0x3F << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_0_cr_OFFSET   0x254
	/* */
	#define mssio_bank2_io_cfg_0_cr_   io_cfg_0_OFFSET  0x0
	#define mssio_bank2_io_cfg_0_cr_   io_cfg_0_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_1_cr_OFFSET   0x258
	/* */
	#define mssio_bank2_io_cfg_1_cr_   io_cfg_2_OFFSET  0x0
	#define mssio_bank2_io_cfg_1_cr_   io_cfg_2_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_2_cr_OFFSET   0x25C
	/* */
	#define mssio_bank2_io_cfg_2_cr_   io_cfg_4_OFFSET  0x0
	#define mssio_bank2_io_cfg_2_cr_   io_cfg_4_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_3_cr_OFFSET   0x260
	/* */
	#define mssio_bank2_io_cfg_3_cr_   io_cfg_6_OFFSET  0x0
	#define mssio_bank2_io_cfg_3_cr_   io_cfg_6_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_4_cr_OFFSET   0x264
	/* */
	#define mssio_bank2_io_cfg_4_cr_   io_cfg_8_OFFSET  0x0
	#define mssio_bank2_io_cfg_4_cr_   io_cfg_8_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_5_cr_OFFSET   0x268
	/* */
	#define mssio_bank2_io_cfg_5_cr_   io_cfg_10_OFFSET  0x0
	#define mssio_bank2_io_cfg_5_cr_   io_cfg_10_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_6_cr_OFFSET   0x26C
	/* */
	#define mssio_bank2_io_cfg_6_cr_   io_cfg_12_OFFSET  0x0
	#define mssio_bank2_io_cfg_6_cr_   io_cfg_12_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_7_cr_OFFSET   0x270
	/* */
	#define mssio_bank2_io_cfg_7_cr_   io_cfg_14_OFFSET  0x0
	#define mssio_bank2_io_cfg_7_cr_   io_cfg_14_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_8_cr_OFFSET   0x274
	/* */
	#define mssio_bank2_io_cfg_8_cr_   io_cfg_16_OFFSET  0x0
	#define mssio_bank2_io_cfg_8_cr_   io_cfg_16_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_9_cr_OFFSET   0x278
	/* */
	#define mssio_bank2_io_cfg_9_cr_   io_cfg_18_OFFSET  0x0
	#define mssio_bank2_io_cfg_9_cr_   io_cfg_18_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_10_cr_OFFSET   0x27C
	/* */
	#define mssio_bank2_io_cfg_10_cr_   io_cfg_20_OFFSET  0x0
	#define mssio_bank2_io_cfg_10_cr_   io_cfg_20_MASK    (0xFFFF << 0x0)

/*IO electrical configuration for MSSIO pad*/
#define mssio_bank2_io_cfg_11_cr_OFFSET   0x280
	/* */
	#define mssio_bank2_io_cfg_11_cr_   io_cfg_22_OFFSET  0x0
	#define mssio_bank2_io_cfg_11_cr_   io_cfg_22_MASK    (0xFFFF << 0x0)

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
	/* "No function*/
	#define SPARE_REGISTER_RW_DATA_OFFSET  0x0
	#define SPARE_REGISTER_RW_DATA_MASK    (0xFF << 0x0)

/*Register for ECO usage*/
#define SPARE_REGISTER_W1P_OFFSET   0x2D4
	/* No function provided for future ECO use*/
	#define SPARE_REGISTER_W1P_DATA_OFFSET  0x0
	#define SPARE_REGISTER_W1P_DATA_MASK    (0xFF << 0x0)

/*Register for ECO usage*/
#define SPARE_REGISTER_RO_OFFSET   0x2D8
	/* "Provides readback of { W1P RW } registers. No function*/
	#define SPARE_REGISTER_RO_DATA_OFFSET  0x0
	#define SPARE_REGISTER_RO_DATA_MASK    (0xFFFF << 0x0)

/*Spare signal back to G5C*/
#define SPARE_PERIM_RW_OFFSET   0x2DC
	/* "Allows the MSS to control the perim_spare_out bits [2] & [6]. No function*/
	#define SPARE_PERIM_RW_DATA_OFFSET  0x0
	#define SPARE_PERIM_RW_DATA_MASK    (0x03 << 0x0)
/*********************************************************************************
********************TOP LEVEL REGISTER STRUCTURE**********************************
**********************************************************************************/

typedef struct _mss_sysreg
{


	/*Register for software use*/
	 uint32_t TEMP0;

	/*Register for software use*/
	 uint32_t TEMP1;

	/*Master clock configuration*/
	 uint32_t CLOCK_CONFIG_CR;

	/*RTC clock divider*/
	 uint32_t RTC_CLOCK_CR;

	/*Fabric Reset mask*/
	 uint32_t FABRIC_RESET_CR;

	/**/
	 uint32_t BOOT_FAIL_CR;

	/*Controls the MSS Corner low power mode signals*/
	 uint32_t MSS_LOW_POWER_CR;

	/*Configuration lock*/
	 uint32_t CONFIG_LOCK_CR;

	/*Indicates which reset caused the last reset. After a reset occurs register should be read and then zro written to allow the next reset event to be correctly captured.*/
	 uint32_t RESET_SR;

	/*Indicates the device status in particular the state of the FPGA fabric and the MSS IO banks*/
	 uint32_t DEVICE_STATUS ;

	/* Padding reserved 32-bit registers.*/
	 uint32_t reservedReg32b_0;
	 uint32_t reservedReg32b_1;
	 uint32_t reservedReg32b_2;
	 uint32_t reservedReg32b_3;
	 uint32_t reservedReg32b_4;
	 uint32_t reservedReg32b_5;

	/*U54-1 Fabric interrupt enable*/
	 uint32_t FAB_INTEN_U54_1;

	/*U54-2 Fabric interrupt enable*/
	 uint32_t FAB_INTEN_U54_2;

	/*U54-3 Fabric interrupt enable*/
	 uint32_t FAB_INTEN_U54_3;

	/*U54-4 Fabric interrupt enable*/
	 uint32_t FAB_INTEN_U54_4;

	/*Allows the Ethernat interrupts to be directly routed to the U54 CPUS.*/
	 uint32_t FAB_INTEN_MISC;

	/*Switches GPIO interrupt from PAD to Fabric GPIO*/
	 uint32_t GPIO_INTERRUPT_FAB_CR;

	/* Padding reserved 32-bit registers.*/
	 uint32_t reservedReg32b_6;
	 uint32_t reservedReg32b_7;
	 uint32_t reservedReg32b_8;
	 uint32_t reservedReg32b_9;
	 uint32_t reservedReg32b_10;
	 uint32_t reservedReg32b_11;
	 uint32_t reservedReg32b_12;
	 uint32_t reservedReg32b_13;
	 uint32_t reservedReg32b_14;
	 uint32_t reservedReg32b_15;

	/*"AMP Mode peripheral mapping register. When the register bit is '0' the peripheral is mapped into the 0x2000000 address range using AXI bus 5 from the Coreplex. When the register bit is '1' the peripheral is mapped into the 0x28000000 address range using AXI bus 6 from the Coreplex."*/
	 uint32_t APBBUS_CR;

	/*"Enables the clock to the MSS peripheral. By turning clocks off dynamic power can be saved. When the clock is off the peripheral  should not be accessed*/
	 uint32_t SUBBLK_CLOCK_CR;

	/*"Holds the MSS peripherals in reset. Whenin reset the peripheral  should not be accessed*/
	 uint32_t SOFT_RESET_CR;

	/*Configures how many outstanding transfers the AXI-AHB bridges in front off the USB and Crypto blocks should allow. (See Synopsys AXI-AHB bridge documentation)*/
	 uint32_t AHBAXI_CR;

	/* Padding reserved 32-bit registers.*/
	 uint32_t reservedReg32b_16;
	 uint32_t reservedReg32b_17;

	/*MSS Corner APB interface controls*/
	 uint32_t DFIAPB_CR;

	/*GPIO Blocks reset control*/
	 uint32_t GPIO_CR;

	/* Padding reserved 32-bit registers.*/
	 uint32_t reservedReg32b_18;

	/*MAC0 configuration register*/
	 uint32_t MAC0_CR;

	/*MAC1 configuration register*/
	 uint32_t MAC1_CR;

	/*USB Configuration register*/
	 uint32_t USB_CR;

	/*Crypto Mesh control and status register*/
	 uint32_t MESH_CR;

	/*Crypto mesh seed and update rate*/
	 uint32_t MESH_SEED_CR;

	/*ENVM AHB Controller setup*/
	 uint32_t ENVM_CR;

	/*Reserved*/
	 uint32_t RESERVED_BC;

	/*QOS Athena USB & MMC Configuration*/
	 uint32_t qos_peripheral_cr;

	/*QOS Configuration Coreplex*/
	 uint32_t qos_cplexio_cr;

	/*QOS configuration DDRC*/
	 uint32_t qos_cplexddr_cr;

	/* Padding reserved 32-bit registers.*/
	 uint32_t reservedReg32b_19;
	 uint32_t reservedReg32b_20;
	 uint32_t reservedReg32b_21;
	 uint32_t reservedReg32b_22;
	 uint32_t reservedReg32b_23;
	 uint32_t reservedReg32b_24;
	 uint32_t reservedReg32b_25;
	 uint32_t reservedReg32b_26;
	 uint32_t reservedReg32b_27;

	/*Indicates that a master caused a MPU violation. Interrupts via maintenance interrupt.*/
	 uint32_t mpu_violation_sr;

	/*Enables interrupts on MPU violations*/
	 uint32_t mpu_violation_inten_cr;

	/*AXI switch decode fail*/
	 uint32_t SW_FAIL_ADDR0_CR;

	/*AXI switch decode fail*/
	 uint32_t SW_FAIL_ADDR1_CR;

	/*Set when an ECC event happens*/
	 uint32_t EDAC_SR;

	/*Enables ECC interrupt on event*/
	 uint32_t EDAC_INTEN_CR;

	/*Count off single bit errors*/
	 uint32_t EDAC_CNT_MMC;

	/*Count off single bit errors*/
	 uint32_t EDAC_CNT_DDRC;

	/*Count off single bit errors*/
	 uint32_t EDAC_CNT_MAC0;

	/*Count off single bit errors*/
	 uint32_t EDAC_CNT_MAC1;

	/*Count off single bit errors*/
	 uint32_t EDAC_CNT_USB;

	/*Count off single bit errors*/
	 uint32_t EDAC_CNT_CAN0;

	/*Count off single bit errors*/
	 uint32_t EDAC_CNT_CAN1;

	/*"Will Corrupt write data to rams 1E corrupts bit 0 2E bits 1 and 2.Injects Errors into all RAMS in the block as long as the bits are set. Setting 1E and 2E will inject a 3-bit error"*/
	 uint32_t EDAC_INJECT_CR;

	/* Padding reserved 32-bit registers.*/
	 uint32_t reservedReg32b_28;
	 uint32_t reservedReg32b_29;
	 uint32_t reservedReg32b_30;
	 uint32_t reservedReg32b_31;
	 uint32_t reservedReg32b_32;
	 uint32_t reservedReg32b_33;

	/*Maintenance Interrupt Enable.*/
	 uint32_t MAINTENANCE_INTEN_CR;

	/*PLL Status interrupt enables*/
	 uint32_t PLL_STATUS_INTEN_CR;

	/*Maintenace interrupt indicates fault and status events.*/
	 uint32_t MAINTENANCE_INT_SR;

	/*PLL interrupt register*/
	 uint32_t PLL_STATUS_SR;

	/*Enable to CFM Timer */
	 uint32_t CFM_TIMER_CR;

	/*Miscellanous Register*/
	 uint32_t MISC_SR;

	/* Padding reserved 32-bit registers.*/
	 uint32_t reservedReg32b_34;
	 uint32_t reservedReg32b_35;
	 uint32_t reservedReg32b_36;
	 uint32_t reservedReg32b_37;

	/*Puts all the RAMS in that block into low leakage mode. RAM contents and Q value preserved.*/
	 uint32_t RAM_LIGHTSLEEP_CR;

	/*Puts all the RAMS in that block into deep sleep mode. RAM contents preserved. Powers down the periphery circuits.*/
	 uint32_t RAM_DEEPSLEEP_CR;

	/*Puts all the RAMS in that block into shut down mode. RAM contents not preserved.  Powers down the RAM and periphery circuits.*/
	 uint32_t RAM_SHUTDOWN_CR;

	/* Padding reserved 32-bit registers.*/
	 uint32_t reservedReg32b_38;
	 uint32_t reservedReg32b_39;
	 uint32_t reservedReg32b_40;
	 uint32_t reservedReg32b_41;
	 uint32_t reservedReg32b_42;
	 uint32_t reservedReg32b_43;
	 uint32_t reservedReg32b_44;
	 uint32_t reservedReg32b_45;
	 uint32_t reservedReg32b_46;
	 uint32_t reservedReg32b_47;
	 uint32_t reservedReg32b_48;
	 uint32_t reservedReg32b_49;
	 uint32_t reservedReg32b_50;
	 uint32_t reservedReg32b_51;
	 uint32_t reservedReg32b_52;
	 uint32_t reservedReg32b_53;
	 uint32_t reservedReg32b_54;
	 uint32_t reservedReg32b_55;
	 uint32_t reservedReg32b_56;
	 uint32_t reservedReg32b_57;
	 uint32_t reservedReg32b_58;
	 uint32_t reservedReg32b_59;
	 uint32_t reservedReg32b_60;
	 uint32_t reservedReg32b_61;
	 uint32_t reservedReg32b_62;
	 uint32_t reservedReg32b_63;
	 uint32_t reservedReg32b_64;
	 uint32_t reservedReg32b_65;
	 uint32_t reservedReg32b_66;
	 uint32_t reservedReg32b_67;
	 uint32_t reservedReg32b_68;
	 uint32_t reservedReg32b_69;
	 uint32_t reservedReg32b_70;
	 uint32_t reservedReg32b_71;
	 uint32_t reservedReg32b_72;

	/*Selects whether the peripheral is connected to the Fabric or IOMUX structure.*/
	 uint32_t IOMUX0_CR;

	/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
	 uint32_t IOMUX1_CR;

	/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
	 uint32_t IOMUX2_CR;

	/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
	 uint32_t IOMUX3_CR;

	/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
	 uint32_t IOMUX4_CR;

	/*Configures the IO Mux structure for each IO pad. See the MSS MAS specification for for description.*/
	 uint32_t IOMUX5_CR;

	/*Sets whether the MMC/SD Voltage select lines are inverted on entry to the IOMUX structure*/
	 uint32_t IOMUX6_CR;

	/* Padding reserved 32-bit registers.*/
	 uint32_t reservedReg32b_73;
	 uint32_t reservedReg32b_74;
	 uint32_t reservedReg32b_75;
	 uint32_t reservedReg32b_76;
	 uint32_t reservedReg32b_77;

	/*Configures the MSSIO block*/
	 uint32_t mssio_bank4_cfg_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank4_io_cfg_0_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank4_io_cfg_1_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank4_io_cfg_2_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank4_io_cfg_3_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank4_io_cfg_4_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank4_io_cfg_5_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank4_io_cfg_6_cr;

	/*Configures the MSSIO block*/
	 uint32_t mssio_bank2_cfg_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank2_io_cfg_0_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank2_io_cfg_1_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank2_io_cfg_2_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank2_io_cfg_3_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank2_io_cfg_4_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank2_io_cfg_5_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank2_io_cfg_6_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank2_io_cfg_7_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank2_io_cfg_8_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank2_io_cfg_9_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank2_io_cfg_10_cr;

	/*IO electrical configuration for MSSIO pad*/
	 uint32_t mssio_bank2_io_cfg_11_cr;

	/* Padding reserved 32-bit registers.*/
	 uint32_t reservedReg32b_78;
	 uint32_t reservedReg32b_79;
	 uint32_t reservedReg32b_80;
	 uint32_t reservedReg32b_81;
	 uint32_t reservedReg32b_82;
	 uint32_t reservedReg32b_83;
	 uint32_t reservedReg32b_84;
	 uint32_t reservedReg32b_85;
	 uint32_t reservedReg32b_86;

	/*Sets H2F [31:0] Spares out signals*/
	 uint32_t mss_spare0_cr;

	/*Sets H2F [37:32] Spares out signals*/
	 uint32_t mss_spare1_cr;

	/*Read H2F [31:0] Spares out signals*/
	 uint32_t mss_spare0_sr;

	/*Read H2F [37:32] Spares out signals*/
	 uint32_t mss_spare1_sr;

	/*Read F2H [31:0] Spares in1 signals*/
	 uint32_t mss_spare2_sr;

	/*Read F2H [37:32] Spares in1 signals*/
	 uint32_t mss_spare3_sr;

	/*Read F2H [31:0] Spares in2 signals*/
	 uint32_t mss_spare4_sr;

	/*Read F2H [37:32] Spares in2 signals*/
	 uint32_t mss_spare5_sr;

	/* Padding reserved 32-bit registers.*/
	 uint32_t reservedReg32b_87;
	 uint32_t reservedReg32b_88;

	/*Register for ECO usage*/
	 uint32_t SPARE_REGISTER_RW;

	/*Register for ECO usage*/
	 uint32_t SPARE_REGISTER_W1P;

	/*Register for ECO usage*/
	 uint32_t SPARE_REGISTER_RO;

	/*Spare signal back to G5C*/
	 uint32_t SPARE_PERIM_RW;
} mss_sysreg_t;

#define SYSREG_ATHENACR_RESET		(1u << 0u)
#define SYSREG_ATHENACR_PURGE		(1u << 1u)
#define SYSREG_ATHENACR_GO			(1u << 2u)
#define SYSREG_ATHENACR_RINGOSCON	(1u << 3u)
#define SYSREG_ATHENACR_COMPLETE	    (1u << 8u)
#define SYSREG_ATHENACR_ALARM		(1u << 9u)
#define SYSREG_ATHENACR_BUSERROR	    (1u << 10u)
#define SYSREG_SOFTRESET_ENVM		(1u << 0u)
#define SYSREG_SOFTRESET_TIMER		(1u << 4u)
#define SYSREG_SOFTRESET_MMUART0	    (1u << 5u)
#define SYSREG_SOFTRESET_DDRC		(1u << 23u)
#define SYSREG_SOFTRESET_FIC3		(1u << 27u)
#define SYSREG_SOFTRESET_ATHENA		(1u << 28u)
#define SYSREG ((mss_sysreg_t*) BASE32_ADDR_MSS_SYSREG) 



#endif /*MSS_SYSREG_H*/
