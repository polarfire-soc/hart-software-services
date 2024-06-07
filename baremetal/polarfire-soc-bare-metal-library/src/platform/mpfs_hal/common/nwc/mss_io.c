/*******************************************************************************
 * Copyright 2019 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * @file mss_io.c
 * @author Microchip FPGA Embedded Systems Solutions
 * @brief MSS IO related code
 *
 */

#include <string.h>
#include <stdio.h>

#include "mpfs_hal/mss_hal.h"
#ifdef LIBERO_SETTING_ALT_IOMUX1_CR
#if ((LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS & (EMMC_CONFIGURED_MASK | SD_CONFIGURED_MASK)) == (EMMC_CONFIGURED_MASK | SD_CONFIGURED_MASK))
static uint8_t io_mux_and_bank_config_alt(void);
#endif
#endif

/*******************************************************************************
 * external functions
 */


/*
 * IOMUX values from Libero
 */
IOMUX_CONFIG   iomux_config_values = {
    LIBERO_SETTING_IOMUX0_CR, /* Selects whether the peripheral is connected to
                                 the Fabric or IOMUX structure. */
    LIBERO_SETTING_IOMUX1_CR, /* BNK4 SDV PAD 0 to 7, each IO has 4 bits   */
    LIBERO_SETTING_IOMUX2_CR, /* BNK4 SDV PAD 8 to 13     */
    LIBERO_SETTING_IOMUX3_CR, /* BNK2 SDV PAD 14 to 21    */
    LIBERO_SETTING_IOMUX4_CR, /* BNK2 SDV PAD 22 to 29    */
    LIBERO_SETTING_IOMUX5_CR, /* BNK2 PAD 30 to 37        */
    LIBERO_SETTING_IOMUX6_CR  /* Sets whether the MMC/SD Voltage select lines
                                 are inverted on entry to the IOMUX structure */
};

#ifdef LIBERO_SETTING_ALT_IOMUX1_CR
IOMUX_CONFIG   iomux_alt_config_values = {
    LIBERO_SETTING_ALT_IOMUX0_CR, /* Selects whether the peripheral is connected to
                                 the Fabric or IOMUX structure. */
    LIBERO_SETTING_ALT_IOMUX1_CR, /* BNK4 SDV PAD 0 to 7, each IO has 4 bits   */
    LIBERO_SETTING_ALT_IOMUX2_CR, /* BNK4 SDV PAD 8 to 13     */
    LIBERO_SETTING_ALT_IOMUX3_CR, /* BNK2 SDV PAD 14 to 21    */
    LIBERO_SETTING_ALT_IOMUX4_CR, /* BNK2 SDV PAD 22 to 29    */
    LIBERO_SETTING_ALT_IOMUX5_CR, /* BNK2 PAD 30 to 37        */
    LIBERO_SETTING_ALT_IOMUX6_CR  /* Sets whether the MMC/SD Voltage select lines
                                 are inverted on entry to the IOMUX structure */
};
#endif

/*
 * Bank 4 and 2 settings, the 38 MSSIO.
 */
MSSIO_BANK4_CONFIG mssio_bank4_io_config = {
    /* LIBERO_SETTING_mssio_bank4_io_cfg_0_cr
        x_vddi Ratio Rx<0-2> == 001
        drv<3-6> == 1111
        7:clamp   == 0
        enhyst   == 0
        lockdn_en == 1
        10:wpd  == 0
        atp_en`== 0
        lpmd_ibuf  == 0
        lpmd_obuf == 0
        persist == 0
        */
    LIBERO_SETTING_MSSIO_BANK4_IO_CFG_0_1_CR,
    LIBERO_SETTING_MSSIO_BANK4_IO_CFG_2_3_CR,
    LIBERO_SETTING_MSSIO_BANK4_IO_CFG_4_5_CR,
    LIBERO_SETTING_MSSIO_BANK4_IO_CFG_6_7_CR,
    LIBERO_SETTING_MSSIO_BANK4_IO_CFG_8_9_CR,
    LIBERO_SETTING_MSSIO_BANK4_IO_CFG_10_11_CR,
    LIBERO_SETTING_MSSIO_BANK4_IO_CFG_12_13_CR,
};

/*
 * Bank 4 and 2 settings, the 38 MSSIO.
 */
#ifdef LIBERO_SETTING_ALT_IOMUX1_CR
MSSIO_BANK4_CONFIG mssio_alt_bank4_io_config = {
    /* LIBERO_SETTING_mssio_bank4_io_cfg_0_cr
        x_vddi Ratio Rx<0-2> == 001
        drv<3-6> == 1111
        7:clamp   == 0
        enhyst   == 0
        lockdn_en == 1
        10:wpd  == 0
        atp_en`== 0
        lpmd_ibuf  == 0
        lpmd_obuf == 0
        persist == 0
        */
    LIBERO_SETTING_ALT_MSSIO_BANK4_IO_CFG_0_1_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK4_IO_CFG_2_3_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK4_IO_CFG_4_5_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK4_IO_CFG_6_7_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK4_IO_CFG_8_9_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK4_IO_CFG_10_11_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK4_IO_CFG_12_13_CR,
};
#endif

/*
 * Bank 4 and 2 settings, the 38 MSSIO.
 */
MSSIO_BANK2_CONFIG mssio_bank2_io_config = {
    /* LIBERO_SETTING_mssio_bank4_io_cfg_0_cr
        x_vddi Ratio Rx<0-2> == 001
        drv<3-6> == 1111
        7:clamp   == 0
        enhyst   == 0
        lockdn_en == 1
        10:wpd  == 0
        atp_en`== 0
        lpmd_ibuf  == 0
        lpmd_obuf == 0
        persist == 0
        */
    LIBERO_SETTING_MSSIO_BANK2_IO_CFG_0_1_CR,
    LIBERO_SETTING_MSSIO_BANK2_IO_CFG_2_3_CR,
    LIBERO_SETTING_MSSIO_BANK2_IO_CFG_4_5_CR,
    LIBERO_SETTING_MSSIO_BANK2_IO_CFG_6_7_CR,
    LIBERO_SETTING_MSSIO_BANK2_IO_CFG_8_9_CR,
    LIBERO_SETTING_MSSIO_BANK2_IO_CFG_10_11_CR,
    LIBERO_SETTING_MSSIO_BANK2_IO_CFG_12_13_CR,
    LIBERO_SETTING_MSSIO_BANK2_IO_CFG_14_15_CR,
    LIBERO_SETTING_MSSIO_BANK2_IO_CFG_16_17_CR,
    LIBERO_SETTING_MSSIO_BANK2_IO_CFG_18_19_CR,
    LIBERO_SETTING_MSSIO_BANK2_IO_CFG_20_21_CR,
    LIBERO_SETTING_MSSIO_BANK2_IO_CFG_22_23_CR
};

#ifdef LIBERO_SETTING_ALT_IOMUX1_CR
MSSIO_BANK2_CONFIG mssio_alt_bank2_io_config = {
    /* LIBERO_SETTING_mssio_bank4_io_cfg_0_cr
        x_vddi Ratio Rx<0-2> == 001
        drv<3-6> == 1111
        7:clamp   == 0
        enhyst   == 0
        lockdn_en == 1
        10:wpd  == 0
        atp_en`== 0
        lpmd_ibuf  == 0
        lpmd_obuf == 0
        persist == 0
        */
    LIBERO_SETTING_ALT_MSSIO_BANK2_IO_CFG_0_1_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK2_IO_CFG_2_3_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK2_IO_CFG_4_5_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK2_IO_CFG_6_7_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK2_IO_CFG_8_9_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK2_IO_CFG_10_11_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK2_IO_CFG_12_13_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK2_IO_CFG_14_15_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK2_IO_CFG_16_17_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK2_IO_CFG_18_19_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK2_IO_CFG_20_21_CR,
    LIBERO_SETTING_ALT_MSSIO_BANK2_IO_CFG_22_23_CR
};
#endif

/*******************************************************************************
 * Local functions
 */
static uint8_t io_mux_and_bank_config(void);

/***************************************************************************//**
 *    MSSIO OFF Mode
 *
 *    The following settings are applied if MMSIO unused/off
 *
 *      The IO Buffers are disabled.
 *      Output drivers are disabled (set the drv<3:0> bits to 0000, output
 *      enable "mss_oe" bit to 0)
 *      Disable the WPU bit set to 0 and enable the WPD bit set to 1.
 *      Receivers are disabled. (Ibufmd<2:0> set to 7)
 *
 *      MSS can enable OFF mode through configurator bit for selective MSSIO
 *      from Bank2/Bank4 by making drv<3:0>/mss_oe bit to "0" for that
 *      particular MSSIO making Output driver disabled and ibufmd <2:0>  bit to
 *      "7" for that particular MSSIO making input receiver disabled.
 *
 */

/***************************************************************************//**
 * mssio_setup()
 *
 * Setup the IOMUX and IO bank 2 and 4.
 *
 * To setup bank 2 and 4, ncode and pcode scb registers in system
 * register block are set as per Libero supplied values.
 * These need to be transferred to I/0
 *
 * @return 0 => pass
 */
uint8_t mssio_setup(void)
{
    uint8_t ret_status = 0U;
    ret_status = io_mux_and_bank_config();
    set_bank2_and_bank4_volts(DEFAULT_MSSIO_CONFIGURATION);
    return (ret_status);
}

/***************************************************************************//**
 * io_mux_and_bank_config(void)
 * sets up the IOMUX and bank 2 and 4 pcodes and n codes
 * @return 0 => OK
 */
static uint8_t io_mux_and_bank_config(void)
{
    /* Configure IO mux's
     *
     * IOMUX1_CR - IOMUX5_CR, five 32-bit registers, with four bits four bits
     * for each I/O determine what is connected to each pad
     *
     * All internal peripherals are also connected to the fabric (apart from
     * MMC/SDIO/GPIO/USB). The IOMUX0 register configures whether the IO
     * function is connected to the fabric or the IOMUX.
     *
     * IOMUX6_CR Sets whether the MMC/SD Voltage select lines are inverted on
     * entry to the IOMUX structure
     *
     * */
    config_32_copy((void *)(&(SYSREG->IOMUX0_CR)),
                &(iomux_config_values),
                sizeof(IOMUX_CONFIG));

    /*
     * Configure MSS IO banks
     *    sets pcode and ncode using (mssio_bank2_cfg_cr/mssio_bank4_cfg_cr)
     *
     * The MSS IO pad configuration is provided by nineteen system registers
     * each configuring two IO's using 15-bits per IO
     * - (mssio_bank*_io_cfg_*_*_cr).

        | mssio_bank*_io_cfg_*_*_cr | offset        | info |
        | field                     | offset        | info |
        |:-------------------------:|:-------------:|:-----|
        |      io_cfg_ibufmd_0      |0              |      |
        |      io_cfg_ibufmd_1      |1              |      |
        |      io_cfg_ibufmd_2      |2              |      |
        |      io_cfg_drv_0         |3              |      |
        |      Io_cfg_drv_1         |4              |      |
        |      Io_cfg_drv_2         |5              |      |
        |      io_cfg_drv_3         |6              |      |
        |      io_cfg_clamp         |7              |      |
        |      io_cfg_enhyst        |8              |      |
        |      io_cfg_lockdn_en     |9              |      |
        |      io_cfg_wpd           |10             |      |
        |      io_cfg_wpu           |11             |      |
        |      io_cfg_atp_en        |12             |      |
        |      io_cfg_lp_persist_en |13             |      |
        |      io_cfg_lp_bypass_en  |14             |      |
     * */

    config_32_copy((void *)(&(SYSREG->MSSIO_BANK4_IO_CFG_0_1_CR)),
                &(mssio_bank4_io_config),
                sizeof(MSSIO_BANK4_CONFIG));

    config_32_copy((void *)(&(SYSREG->MSSIO_BANK2_IO_CFG_0_1_CR)),
                &(mssio_bank2_io_config),
                sizeof(MSSIO_BANK2_CONFIG));

    set_bank2_and_bank4_volts(DEFAULT_MSSIO_CONFIGURATION);

    return(0L);
}

/***************************************************************************//**
 * io_mux_and_bank_config_alt(void)
 * Configures alt setting
 * @return
 */
#ifdef LIBERO_SETTING_ALT_IOMUX1_CR
#if ((LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS & (EMMC_CONFIGURED_MASK | SD_CONFIGURED_MASK)) == (EMMC_CONFIGURED_MASK | SD_CONFIGURED_MASK))
static uint8_t io_mux_and_bank_config_alt(void)
{
    /* Configure IO mux's
     *
     * IOMUX1_CR - IOMUX5_CR, five 32-bit registers, with four bits four bits
     * for each I/O determine what is connected to each pad
     *
     * All internal peripherals are also connected to the fabric (apart from
     * MMC/SDIO/GPIO/USB). The IOMUX0 register configures whether the IO
     * function is connected to the fabric or the IOMUX.
     *
     * IOMUX6_CR Sets whether the MMC/SD Voltage select lines are inverted on
     * entry to the IOMUX structure
     *
     * */

    config_32_copy((void *)(&(SYSREG->IOMUX0_CR)),
                &(iomux_alt_config_values),
                sizeof(IOMUX_CONFIG));

    /*
     * Configure MSS IO banks
     *    sets pcode and ncode using (mssio_bank2_cfg_cr/mssio_bank4_cfg_cr)
     *
     * The MSS IO pad configuration is provided by nineteen system registers
     * each configuring two IO's using 15-bits per IO
     * - (mssio_bank*_io_cfg_*_*_cr).

        | mssio_bank*_io_cfg_*_*_cr | offset        | info |
        | field                     | offset        | info |
        |:-------------------------:|:-------------:|:-----|
        |      io_cfg_ibufmd_0      |0              |      |
        |      io_cfg_ibufmd_1      |1              |      |
        |      io_cfg_ibufmd_2      |2              |      |
        |      io_cfg_drv_0         |3              |      |
        |      Io_cfg_drv_1         |4              |      |
        |      Io_cfg_drv_2         |5              |      |
        |      io_cfg_drv_3         |6              |      |
        |      io_cfg_clamp         |7              |      |
        |      io_cfg_enhyst        |8              |      |
        |      io_cfg_lockdn_en     |9              |      |
        |      io_cfg_wpd           |10             |      |
        |      io_cfg_wpu           |11             |      |
        |      io_cfg_atp_en        |12             |      |
        |      io_cfg_lp_persist_en |13             |      |
        |      io_cfg_lp_bypass_en  |14             |      |
     * */

    config_32_copy((void *)(&(SYSREG->MSSIO_BANK4_IO_CFG_0_1_CR)),
                &(mssio_alt_bank4_io_config),
                sizeof(MSSIO_BANK4_CONFIG));

    config_32_copy((void *)(&(SYSREG->MSSIO_BANK2_IO_CFG_0_1_CR)),
                &(mssio_alt_bank2_io_config),
                sizeof(MSSIO_BANK2_CONFIG));

    set_bank2_and_bank4_volts(DEFAULT_MSSIO_CONFIGURATION);

    return(0L);
}
#endif
#endif

/**
 * set_bank2_and_bank4_volts(void)
 * sets bank voltage parameters
 *   bank_pcode
 *   bank_ncode
 *   vs
 * @return
 */
void set_bank2_and_bank4_volts(MSSIO_CONFIG_OPTION config)
{

    switch(config)
    {
        default:
        SCB_REGS->MSSIO_BANK2_CFG_CR.MSSIO_BANK2_CFG_CR =\
                    (uint32_t)LIBERO_SETTING_MSSIO_BANK2_CFG_CR;
        SCB_REGS->MSSIO_BANK4_CFG_CR.MSSIO_BANK4_CFG_CR =\
                    (uint32_t)LIBERO_SETTING_MSSIO_BANK4_CFG_CR;
        break;

#ifdef LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS
        case ALT_MSSIO_CONFIGURATION:
            break;
#endif

    }
    return;
}

/***************************************************************************//**
 * alternate_io_configured()
 * Answers question is alternate I/O configuration present
 * @return true/false
 */
uint8_t  mss_is_alternate_io_configured(void)
{
    uint8_t result = false;
#ifdef LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS
    if ((LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS & (EMMC_CONFIGURED_MASK | SD_CONFIGURED_MASK)) == (EMMC_CONFIGURED_MASK | SD_CONFIGURED_MASK))
    {
        result = true;
    }
#endif
    return result;
}

/***************************************************************************//**
 * alternate_io_setting_sd()
 * Answers question is alternate setting SD?
 * @return returns true if sd is alternate setting
 */
uint8_t  mss_is_alternate_io_setting_sd(void)
{
    uint8_t result = false;
#ifdef LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS
    if ((LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS & (EMMC_CONFIGURED_MASK | SD_CONFIGURED_MASK)) == (EMMC_CONFIGURED_MASK | SD_CONFIGURED_MASK))
    {
        if ((LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS & DEFAULT_ON_START_MASK)!=DEFAULT_ON_START_MASK)
        {
            result = true;
        }
    }
#endif
    return result;
}

/***************************************************************************//**
 * alternate_io_setting_sd()
 * Answers question is alternate setting emmc?
 * @return returns true if sd is alternate setting
 */
uint8_t  mss_is_alternate_io_setting_emmc(void)
{
    uint8_t result = false;
#ifdef LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS
    if ((LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS & (EMMC_CONFIGURED_MASK | SD_CONFIGURED_MASK)) == (EMMC_CONFIGURED_MASK | SD_CONFIGURED_MASK))
    {
        if ((LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS & DEFAULT_ON_START_MASK)==DEFAULT_ON_START_MASK)
        {
            result = true;
        }
    }
#endif
    return result;
}

/**
 * Determines if MSSIO alt switch support in MSS configurator version
 * Does not indicate if alternate has been configured.
 * Indicates you can use the following to determine setup
 *  mss_io_defaut_setting(void)
 *  mss_is_alternate_io_configured(void)
 *  mss_is_alternate_io_setting_emmc(void)
 *  mss_is_alternate_io_setting_sd(void)
 *
 * @return return true/false
 */
uint8_t  mss_does_xml_ver_support_switch(void)
{
    uint8_t result = false;
#ifdef LIBERO_SETTING_HEADER_GENERATOR_VERSION_MAJOR
    uint32_t header_ver = (LIBERO_SETTING_HEADER_GENERATOR_VERSION_MAJOR*100) + (LIBERO_SETTING_HEADER_GENERATOR_VERSION_MINOR*10) + LIBERO_SETTING_HEADER_GENERATOR_VERSION_PATCH;
    uint32_t xml_ver = (LIBERO_SETTING_XML_VERSION_MAJOR*100) + (LIBERO_SETTING_XML_VERSION_MINOR*10) + LIBERO_SETTING_XML_VERSION_PATCH;

    if ((header_ver >= 64U) && (xml_ver >= 56U))
    {
        result = true;
    }
#else
    (void)result;
#endif
    return result;
}


/**
 * mss_io_default_setting(void)
 * @return returns what is configured on default if mss configurator version supports this.
 */
#ifdef LIBERO_SETTING_ALT_IOMUX1_CR
uint8_t  mss_io_default_setting(void)
{
    uint8_t result;

    if  ( mss_does_xml_ver_support_switch() == false )
    {
        result = NO_SUPPORT_MSSIO_CONFIGURATION;
    }
    else
    {
#ifdef LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS
#if ((LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS & (SD_CONFIGURED_MASK | DEFAULT_ON_START_MASK)) == (SD_CONFIGURED_MASK | DEFAULT_ON_START_MASK))

    result = SD_MSSIO_CONFIGURATION;

#elif  ((LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS & (EMMC_CONFIGURED_MASK | DEFAULT_ON_START_MASK)) == (EMMC_CONFIGURED_MASK))

    result = SD_MSSIO_CONFIGURATION;
#else
    result = NOT_SETUP_MSSIO_CONFIGURATION;
#endif
#endif
    }

    return(result);
}
#endif

/**
 * Set the MSSIO to a desired config
 * @param option  SD or eMMC
 * @return
 */
uint8_t switch_mssio_config(MSS_IO_OPTIONS option)
{
    uint8_t result = false;
#ifdef LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS
#if ((LIBERO_SETTING_MSSIO_CONFIGURATION_OPTIONS & (EMMC_CONFIGURED_MASK | SD_CONFIGURED_MASK)) == (EMMC_CONFIGURED_MASK | SD_CONFIGURED_MASK))
    switch(option)
    {
        case SD_MSSIO_CONFIGURATION:
            if (mss_is_alternate_io_setting_sd() == true)
            {
                io_mux_and_bank_config_alt();
            }
            else
            {
                io_mux_and_bank_config();
            }
            switch_demux_using_fabric_ip(SD_MSSIO_CONFIGURATION);
            break;

        case EMMC_MSSIO_CONFIGURATION:
            if (mss_is_alternate_io_setting_emmc() == true)
            {
                io_mux_and_bank_config_alt();
            }
            else
            {
                io_mux_and_bank_config();
            }
            switch_demux_using_fabric_ip(EMMC_MSSIO_CONFIGURATION);
            break;

        case NO_SUPPORT_MSSIO_CONFIGURATION:
            break;

        case NOT_SETUP_MSSIO_CONFIGURATION:
            break;
    }
    result = true;
#else
    (void)option;
    result = false;
#endif
#endif
    return result;
}

/**
 * Is there a mux present, define is true by default
 * @return true/false
 */
__attribute__((weak)) uint8_t fabric_sd_emmc_demux_present(void)
{
    return (uint8_t) FABRIC_SD_EMMC_DEMUX_SELECT_PRESENT;
}

/**
 * Returns the fabric mux address
 * @return address of the mux in fabric
 */
__attribute__((weak)) uint32_t * fabric_sd_emmc_demux_address(void)
{
    return (uint32_t *)FABRIC_SD_EMMC_DEMUX_SELECT_ADDRESS;
}

/**
 * switch_demux_using_fabric_ip()
 * Requires fpga switch hdl. This comes with the reference icicle kit design.
 * You will need to create your own or copy when creating your own fpga design
 * along with an external mux in your board design if you wish to use SD/eMMC
 * muxing in your hardware design.
 * Please note this function will cause a hang if you do not have support
 * for switching in your fpga design. Only use if you have this support if your
 * fabric design.
 * @param option SD_MSSIO_CONFIGURATION/EMMC_MSSIO_CONFIGURATION
 * @return
 */
__attribute__((weak)) uint8_t switch_demux_using_fabric_ip(MSS_IO_OPTIONS option)
{
    uint8_t result = false;

    if (fabric_sd_emmc_demux_present() == true)
    {
        volatile uint32_t *reg_pt = fabric_sd_emmc_demux_address();
        switch(option)
        {
            case SD_MSSIO_CONFIGURATION:
                *reg_pt = CMD_SD_EMMC_DEMUX_SD_ON;
                break;

            case EMMC_MSSIO_CONFIGURATION:
                *reg_pt = CMD_SD_EMMC_DEMUX_EMMC_ON;
                break;

            case NO_SUPPORT_MSSIO_CONFIGURATION:
                break;

            case NOT_SETUP_MSSIO_CONFIGURATION:
                break;
        }
        result = true;
    }
    else
    {
        result = false;
    }
    return result;
}


/***************************************************************************//**
 * See mss_io_config.h for details of how to use this function.
 */
__attribute__((weak)) void mss_set_gpio_interrupt_fab_cr(uint32_t reg_value)
{
    SYSREG->GPIO_INTERRUPT_FAB_CR = reg_value;
}

/***************************************************************************//**
 * See mss_peripherals.h for details of how to use this function.
 */
__attribute__((weak)) uint32_t mss_get_gpio_interrupt_fab_cr(void)
{
    return (SYSREG->GPIO_INTERRUPT_FAB_CR);
}


#ifdef EXAMPLE_MSSIO_APP_CODE
#include "drivers/mss_gpio/mss_gpio.h"
/**
 *
 * @return 0 => OK
 */
int32_t gpio_toggle_test(void)
{
    SYSREG->TEMP0 = 0x11111111;

    for (int l = 0 ; l < 14 ; l++)
    {
        for (int i = 0 ; i < 14 ; i++)
        {
            SYSREG->TEMP0 = 0x12345678;
            MSS_GPIO_set_output(GPIO0_LO, i, 0x0);
        }
        for (int i = 0 ; i < 24 ; i++)
        {
            SYSREG->TEMP0 = 0x12345678;
            MSS_GPIO_set_output(GPIO1_LO, i, 0x0);
        }
        SYSREG->TEMP0 = 0xFFFFFFFFUL;
        for (int i = 0 ; i < 14 ; i++)
        {
            SYSREG->TEMP0 = 0x12345678;
            MSS_GPIO_set_output(GPIO0_LO, i, 0x1);
        }
        for (int i = 0 ; i < 24 ; i++)
        {
            SYSREG->TEMP0 = 0x12345678;
            MSS_GPIO_set_output(GPIO1_LO, i, 0x1);
        }
    }
    return(0UL);
}


/**
 *
 * @return 0 => OK
 */
int32_t gpio_set_config(void)
{
    SYSREG->SOFT_RESET_CR &= ~((1U<<20U)|(1U<<21U)|(1U<<22U));
    SYSREG->SUBBLK_CLOCK_CR |= ((1U<<20U)|(1U<<21U)|(1U<<22U));
    MSS_GPIO_init(GPIO0_LO);
    MSS_GPIO_init(GPIO1_LO);
    for (int i = 0 ; i < 14 ; i++)
    {
        MSS_GPIO_config(GPIO0_LO, i, MSS_GPIO_OUTPUT_MODE);
    }
    for (int i = 0 ; i < 24 ; i++)
    {
        MSS_GPIO_config(GPIO1_LO, i, MSS_GPIO_OUTPUT_MODE);
    }
    return(0UL);
}
#endif

