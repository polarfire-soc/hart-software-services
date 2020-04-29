/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/*******************************************************************************
 * @file mss_io.h
 * @author Microchip-FPGA Embedded Systems Solutions
 * @brief MSS IO related code
 *
 */
#include <string.h>
#include <stdio.h>
#include "mpfs_hal/mss_hal.h"
#ifdef MSSIO_TEST
#include "drivers/mss_gpio/mss_gpio.h"
#endif



/*
 * IOMUX values from Libero
 */
IOMUX_CONFIG 	iomux_config_values = {
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


/*
 * Bank 4 and 2 settings, the 38 MSSIO.
 */
MSSIO_BANK_CONFIG mssio_bank_config = {
    /* LIBERO_SETTING_MSSIO_BANK4_CFG_CR
         bank_pcode                        [0:6]   RW value= 0x0
         bank_ncode                        [6:6]   RW value= 0x0
         vs                                [12:4]  RW value= 0x0 */
        LIBERO_SETTING_MSSIO_BANK4_CFG_CR,
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
    /* LIBERO_SETTING_mssio_bank2_cfg_cr
         bank_pcode                        [0:6]   RW value= 0x0
         bank_ncode                        [6:6]   RW value= 0x0
         vs                                [12:4]  RW value= 0x0 */
    LIBERO_SETTING_MSSIO_BANK2_CFG_CR,
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



/*******************************************************************************
 * Local functions
 */
static int32_t io_mux_and_bank_config(void);

/***************************************************************************//**
 *    MSSIO OFF Mode
 *
 *    The following settings are applied if MMSIO unused/off
 *
 *      The IO Buffers are disabled.
 *      Output drivers are disabled (set the drv<3:0> bits to 0000, output
 *      enable “mss_oe” bit to 0)
 *      Disable the WPU bit set to 0 and enable the WPD bit set to 1.
 *      Receivers are disabled. (Ibufmd<2:0> set to 7)
 *
 *      MSS can enable OFF mode through configurator bit for selective MSSIO
 *      from Bank2/Bank4 by making drv<3:0>/mss_oe bit to “0” for that
 *      particular MSSIO making Output driver disabled and ibufmd <2:0>  bit to
 *      “7” for that particular MSSIO making input receiver disabled.
 *
 */
/* Used during full chip simulation
    todo: remove or alter HSS_Setup_PAD_IO_test() once the
           hardware is verified */
#ifdef MSSIO_OFF_MODE_TEST

MSSIO_BANK_CONFIG mssio_bank_config_all_off = {
    0x00008247UL,    /* mssio_bank4_cfg_cr;
                       *  bank_pcode           :6;
                       *  reserved_01          :2;
                       *  bank_ncode           :6;
                       *  reserved_02          :2;
                       *  vs                   :4;
                       *  reserved_03          :12;  */
    0x04070407UL,    /* mssio_bank4_io_cfg_0_cr;  full drv strength
                      * rpc_io_cfg_0_ibufmd  Rx<0-2> == 111
                      * drv<3-6> == 0000
                      * 7:clamp   == 0
                      * enhyst   == 0
                      * lockdn_en == 0
                      * 10:wpd  == 1
                      * 11:rpc_io_cfg_0_wpu == 0
                      * atp_en`== 0
                      * lpmd_ibuf  == 0
                      * lpmd_obuf == 0
                      * persist == 0
                      */
    /* ((7U<<0U) | (0U<<3U) | (1U<<10U)) == 0x04003U */
    0x04070407UL,  /* mssio_bank4_io_cfg_1_cr; */
    0x04070407UL,  /* mssio_bank4_io_cfg_2_cr; */
    0x04070407UL,  /* mssio_bank4_io_cfg_3_cr; */
    0x04070407UL,  /* mssio_bank4_io_cfg_4_cr; */
    0x04070407UL,  /* mssio_bank4_io_cfg_5_cr; */
    0x04070407UL,  /* mssio_bank4_io_cfg_6_cr; */
    0x00008247UL,  /* mssio_bank2_cfg_cr; */
    0x04070407UL,  /* mssio_bank2_io_cfg_0_cr; */
    0x04070407UL,  /* mssio_bank2_io_cfg_1_cr; */
    0x04070407UL,  /* mssio_bank2_io_cfg_2_cr; */
    0x04070407UL,  /* mssio_bank2_io_cfg_3_cr; */
    0x04070407UL,  /* mssio_bank2_io_cfg_4_cr; */
    0x04070407UL,  /* mssio_bank2_io_cfg_5_cr; */
    0x04070407UL,  /* mssio_bank2_io_cfg_6_cr; */
    0x04070407UL,  /* mssio_bank2_io_cfg_7_cr; */
    0x04070407UL,  /* mssio_bank2_io_cfg_8_cr; */
    0x04070407UL,  /* mssio_bank2_io_cfg_9_cr; */
    0x04070407UL,  /* mssio_bank2_io_cfg_10_cr; */
    0x04070407UL   /* mssio_bank2_io_cfg_11_cr; */
};


/**
 *
 *   MSSIO OFF Mode
 *
 *      The IO Buffers are disabled.
 *      Output drivers are disabled (set the drv<3:0> bits to 0000, output
 *      enable “mss_oe” bit to 0)
 *      Disable the WPU bit set to 0 and enable the WPD bit set to 1.
 *      Receivers are disabled. (Ibufmd<2:0> set to 7)
 *
 *      MSS can enable OFF mode through configurator bit for selective MSSIO
 *      from Bank2/Bank4 by making drv<3:0>/mss_oe bit to “0” for that
 *      particular MSSIO making Output driver disabled and ibufmd <2:0>  bit
 *      to “7” for that particular MSSIO making input receiver disabled.
 *
 */
int32_t io_bank_off_mode(void)
{
    extern MSSIO_BANK_CONFIG mssio_bank_config_all_off;
    /* Configure MSS IO banks */
    memcpy((void *)(&(SYSREG->MSSIO_BANK4_CFG_CR)),
            &(mssio_bank_config_all_off), sizeof(MSSIO_BANK_CONFIG));
    return(0L);
}
#endif


/***************************************************************************//**
 * mssio_setup()
 *
 * Setup the IOMUX and IO bank 2 and 4.
 *
 * To setup bank 2 and 4, ncode and pcode registers rpc registers in system
 * register block are set as per Libero supplied values.
 * These need to be transferred to I/0
 * Todo: I need to revisit transfer step. This has been verified in full chip
 * using test code at the end of this file, but intention is to do this in a
 * different order in hal.
 *
 * @return 0 => pass
 */
int32_t mssio_setup(void)
{
    uint32_t ret_status;
    ret_status = io_mux_and_bank_config();
    (void)ret_status; // reference to avoid compiler warning
    /*
     * todo: Verify: There may be an extra step required here to transfer
     * mssio_bank*_io_cfg_*_*_cr as in dynamic mode.
     * Detail conditions for transfer.
     */
    return (0L);
}

/***************************************************************************//**
 * io_mux_and_bank_config(void)
 * sets up the IOMUX and bank 2 and 4 pcodes and n codes
 * @return 0 => OK
 */
static int32_t io_mux_and_bank_config(void)
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
    memcpy((void *)(&(SYSREG->IOMUX0_CR)), &(iomux_config_values)\
            , sizeof(IOMUX_CONFIG));
    /*
     * Configure MSS IO banks
     *    sets pcode and ncode using (mssio_bank2_cfg_cr/mssio_bank4_cfg_cr)
     *
     * The MSS IO pad configuration is provided by nineteen system registers
     * each configuring two IO’s using 15-bits per IO
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
    memcpy((void *)(&(SYSREG->MSSIO_BANK4_CFG_CR)),\
        &(mssio_bank_config), sizeof(mssio_bank_config));
    return(0L);
}

/***************************************************************************//**
 * \brief PAD I/O Configuration
 *
 * There is no dynamic PVT calibration required for the MSSIOs. Instead, pcode
 * and ncode values are programmed byPolarFire-SoC Embedded Software to system
 * registers.
 *
 *
 * The pcode and ncode values to be applied to each bank depend on the user
 * selection of voltage threshold standard (specified in Libero MSS
 * configurator) to be  applied to the bank.
 *
 * MPFS HSS Embedded Software performs the setting of the pcode and ncode values
 * to the MSSIOs, to avoid requiring dedicated hardware decode them from the
 * system registers. The E51 will need to setup per-bank and per-IO
 * configurations for MSSIO.
 *
 * Per bank configuration includes weak pullup/pulldown, drive strength etc.
 * configured as per user requirement (specified in MSS configurator).
 *
 * Per I/O configuration includes mux configuration.
 */
#ifdef MSSIO_TEST /* Used during full chip simulation
                    todo: remove or alter HSS_Setup_PAD_IO_test() once the
                           hardware is verified */
void HSS_Setup_PAD_IO_test(void)
{



    uint32_t test;
    int32_t i;
    int32_t write_value;
    mss_gpio_id_t       port_id;
    uint32_t ret_value;

    //mss_dce_out[1] MSS IO Control register bit 1, will enable the IOs when set
    /*
     * verify default values in simulation
     * Verify 0x3704 0278  and 0x3704 027C, are writable and take value as
     * expected
     *
     */
    SIM_FEEDBACK1(1U);

    uint32_t reverse = ((simulation_instruction & 0x0100U)>>8U);
    uint32_t odd_mode;
    uint32_t even_mode;
    uint32_t write_add;
    uint32_t read_add;

    if(reverse == 0U)
    {
        even_mode   = MSS_GPIO_OUTPUT_MODE;
        odd_mode    = MSS_GPIO_INOUT_MODE;
        write_add = 0U;
        read_add = 0U;
    }
    else
    {
        even_mode   = MSS_GPIO_INOUT_MODE;
        odd_mode    = MSS_GPIO_OUTPUT_MODE;
        write_add = 1;
        read_add = -1;
    }


    /*
     * For initial power on, the configuration bits of the IO and the do/oe
     * signals are initialized before the IO are enabled. The IO output drivers
     * are always enabled after the receivers are enabled. The MSSIO bank
     * buffers have an approx. 10uS delay between the inputs and the outputs
     * being enabled.
     */
    /*
     * Init GPIO
     *
     * config of test:
     *    Bank 4 Pad 0 (output) to Bank 4 Pad 1 (input)
     *    Bank 4 Pad 2 (output) to Bank 4 Pad 3 (input)
     *    Bank 4 Pad 4 (output) to Bank 4 Pad 5 (input)
     *    Bank 4 Pad 6 (output) to Bank 4 Pad 7 (input)
     *    Bank 4 Pad 8 (output) to Bank 4 Pad 9 (input)
     *    Bank 4 Pad 10 (output) to Bank 4 Pad 11 (input)
     *    Bank 4 Pad 12 (output) to Bank 4 Pad 13 (input)
     *    Bank 2 Pad 0 (output) to Bank 2 Pad 1 (input)
     *    Bank 2 Pad 2 (output) to Bank 2 Pad 3 (input)
     *    Bank 2 Pad 4 (output) to Bank 2 Pad 5 (input)
     *    Bank 2 Pad 6 (output) to Bank 2 Pad 7 (input)
     *    Bank 2 Pad 8 (output) to Bank 2 Pad 9 (input)
     *    Bank 2 Pad 10 (output) to Bank 2 Pad 11 (input)
     *    Bank 2 Pad 12 (output) to Bank 2 Pad 13 (input)
     *    Bank 2 Pad 14 (output) to Bank 2 Pad 15 (input)
     *    Bank 2 Pad 16 (output) to Bank 2 Pad 17 (input)
     *    Bank 2 Pad 18 (output) to Bank 2 Pad 19 (input)
     *    Bank 2 Pad 20 (output) to Bank 2 Pad 21 (input)
     *    Bank 2 Pad 22 (output) to Bank 2 Pad 23 (input)
     *
     */
     /*GPIO1*/
     SIM_FEEDBACK1(20U);
     MSS_GPIO_init(GPIO1_LO);
     SIM_FEEDBACK1(21U);
     /*    Bank 2 Pad 0 (output) to Bank 2 Pad 1 (input) */
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_0, even_mode);
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_1, odd_mode);
     /*    Bank 2 Pad 2 (output) to Bank 2 Pad 3 (input) */
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_2, even_mode);
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_3, odd_mode);
     /*    Bank 2 Pad 4 (output) to Bank 2 Pad 5 (input) */
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_4, even_mode);
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_5, odd_mode);
     /*    Bank 2 Pad 6 (output) to Bank 2 Pad 7 (input) */
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_6, even_mode);
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_7, odd_mode);
     /*    Bank 2 Pad 8 (output) to Bank 2 Pad 9 (input) */
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_8, even_mode);
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_9, odd_mode);
     /*    Bank 2 Pad 10 (output) to Bank 2 Pad 11 (input) */
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_10, even_mode);
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_11, odd_mode);
     /*    Bank 2 Pad 12 (output) to Bank 2 Pad 13 (input) */
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_12, even_mode);
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_13, odd_mode);
     /*    Bank 2 Pad 14 (output) to Bank 2 Pad 15 (input) */
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_14, even_mode);
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_15, odd_mode);
     /*    Bank 2 Pad 16 (output) to Bank 2 Pad 17 (input) */
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_16, even_mode);
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_17, odd_mode);
     /*    Bank 2 Pad 18 (output) to Bank 2 Pad 19 (input) */
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_18, even_mode);
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_19, odd_mode);
     /*    Bank 2 Pad 20 (output) to Bank 2 Pad 21 (input) */
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_20, even_mode);
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_21, odd_mode);
     /*    Bank 2 Pad 22 (output) to Bank 2 Pad 23 (input) */
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_22, even_mode);
     MSS_GPIO_config(GPIO1_LO, MSS_GPIO_23, odd_mode);

     /*GPIO0*/
     MSS_GPIO_init(GPIO0_LO);
     MSS_GPIO_config(GPIO0_LO, MSS_GPIO_0, even_mode );
     MSS_GPIO_config(GPIO0_LO, MSS_GPIO_1, odd_mode );
     MSS_GPIO_config(GPIO0_LO, MSS_GPIO_2, even_mode );
     MSS_GPIO_config(GPIO0_LO, MSS_GPIO_3, odd_mode );
     MSS_GPIO_config(GPIO0_LO, MSS_GPIO_4, even_mode );
     MSS_GPIO_config(GPIO0_LO, MSS_GPIO_5, odd_mode );
     MSS_GPIO_config(GPIO0_LO, MSS_GPIO_6, even_mode );
     MSS_GPIO_config(GPIO0_LO, MSS_GPIO_7, odd_mode );
     MSS_GPIO_config(GPIO0_LO, MSS_GPIO_8, even_mode );
     MSS_GPIO_config(GPIO0_LO, MSS_GPIO_9, odd_mode );
     MSS_GPIO_config(GPIO0_LO, MSS_GPIO_10, even_mode );
     MSS_GPIO_config(GPIO0_LO, MSS_GPIO_11, odd_mode );
     MSS_GPIO_config(GPIO0_LO, MSS_GPIO_12, even_mode );
     MSS_GPIO_config(GPIO0_LO, MSS_GPIO_13, odd_mode );


    /*
     * There are two registers -
     * Verify 0x3704 0278  and 0x3704 027C, are writable and take value as
     * expected
     * These will only be used for:
     *  defaults on startup
     *  parametric production testing.
     *
     */
    SIM_FEEDBACK1(2U);
    io_mux_and_bank_config();
    SIM_FEEDBACK1(4U);

    /*
     * set control signals
     */
    SIM_FEEDBACK1(5U);
    /* DCE:111, CORE_UP:1, FLASH_VALID:0, mss_io_en:0 */
    SCB_REGS->MSSIO_CONTROL_CR.MSSIO_CONTROL_CR = (0x07U<<8U)|(0x01U<<11U)|\
            (0x00U<<12U)|(0x00U<<13U);
    SIM_FEEDBACK1(6U);
    delay((uint32_t) 10U);
    /* DCE:000, CORE_UP:1, FLASH_VALID:0, mss_io_en:0 */
    SCB_REGS->MSSIO_CONTROL_CR.MSSIO_CONTROL_CR = (0x00U<<8U)|(0x01U<<11U)|\
            (0x00U<<12U)|(0x00U<<13U);
    SIM_FEEDBACK1(7U);
    delay((uint32_t) 10U);
    /* DCE:000, CORE_UP:1, FLASH_VALID:1, mss_io_en:0 */
    SCB_REGS->MSSIO_CONTROL_CR.MSSIO_CONTROL_CR = (0x00U<<8U)|(0x01U<<11U)|\
            (0x01U<<12U)|(0x00U<<13U);
    SIM_FEEDBACK1(8U);
    delay((uint32_t) 10U);
    /* DCE:000, CORE_UP:1, FLASH_VALID:1, mss_io_en:1 */
    SCB_REGS->MSSIO_CONTROL_CR.MSSIO_CONTROL_CR = (0x00U<<8U)|(0x01U<<11U)|\
            (0x01U<<12U)|(0x01U<<13U);
    SIM_FEEDBACK1(9U);

    /*
     * We should check the bank power here
     */
    //...
    /*
     * writeGPIO
     */
    SIM_FEEDBACK0(16U);

    uint8_t value = 0U;
    for(port_id = MSS_GPIO_0; port_id < MSS_GPIO_13; port_id += 2U )
    {
            MSS_GPIO_set_output( GPIO0_LO, port_id + write_add, value);
    }
    for(port_id = MSS_GPIO_0; port_id < MSS_GPIO_23; port_id += 2U )
    {
            MSS_GPIO_set_output( GPIO1_LO, port_id + write_add, value);
    }

    SIM_FEEDBACK1(34U);

    /* read back */
    ret_value = MSS_GPIO_get_inputs( GPIO0_LO );
    for(port_id = MSS_GPIO_1; port_id <= MSS_GPIO_13; port_id += 2U )
    {
        if(((ret_value >> (MSS_GPIO_1  + read_add)) & 0x01U) != value)
        {
            SIM_FEEDBACK1(100U + port_id + read_add);
        }
        else
        {
             SIM_FEEDBACK1(1000U + port_id + read_add);
        }
    }
    ret_value = MSS_GPIO_get_inputs( GPIO1_LO );
    for(port_id = MSS_GPIO_1; port_id <= MSS_GPIO_23; port_id += 2U )
    {
        if(((ret_value >> (MSS_GPIO_1  + read_add)) & 0x01U) != value)
        {
            SIM_FEEDBACK1(200U + port_id + read_add);
        }
        else
        {
             SIM_FEEDBACK1(2000U + port_id + read_add);
        }
    }

    /*
     * write 1 to GPIO's
     */
    SIM_FEEDBACK0(26U);
    value = 1U;
    for(port_id = MSS_GPIO_0; port_id < MSS_GPIO_13; port_id += 2U )
    {
            MSS_GPIO_set_output( GPIO0_LO, port_id + write_add, value);
    }
    for(port_id = MSS_GPIO_0; port_id < MSS_GPIO_23; port_id += 2U )
    {
            MSS_GPIO_set_output( GPIO1_LO, port_id + write_add, value);
    }
    SIM_FEEDBACK1(35U);

    /*
     * read back
     * */
    ret_value = MSS_GPIO_get_inputs( GPIO0_LO );
    for(port_id = MSS_GPIO_1; port_id <= MSS_GPIO_13; port_id += 2U )
    {
        if(((ret_value >> (MSS_GPIO_1  + read_add)) & 0x01U) != value)
        {
            SIM_FEEDBACK1(100U + port_id + read_add);
        }
        else
        {
             SIM_FEEDBACK1(1000U + port_id + read_add);
        }
    }
    ret_value = MSS_GPIO_get_inputs( GPIO1_LO );
    for(port_id = MSS_GPIO_1; port_id <= MSS_GPIO_23; port_id += 2U )
    {
        if(((ret_value >> (MSS_GPIO_1  + read_add)) & 0x01U) != value)
        {
            SIM_FEEDBACK1(200U + port_id + read_add);
        }
        else
        {
             SIM_FEEDBACK1(2000U + port_id + read_add);
        }
    }

    /*
     * write 0's
     * */
    SIM_FEEDBACK0(36U);
    value = 0U;
    for(port_id = MSS_GPIO_0; port_id <= MSS_GPIO_13; port_id += 2U )
    {
            MSS_GPIO_set_output( GPIO0_LO, port_id + write_add, value);
    }
    for(port_id = MSS_GPIO_0; port_id <= MSS_GPIO_23; port_id += 2U )
    {
            MSS_GPIO_set_output( GPIO1_LO, port_id + write_add, value);
    }

    /* read back */
    SIM_FEEDBACK0(46U);

    ret_value = MSS_GPIO_get_inputs( GPIO0_LO );
    for(port_id = MSS_GPIO_1; port_id <= MSS_GPIO_13; port_id += 2U )
    {
        if(((ret_value >> (MSS_GPIO_1  + read_add)) & 0x01U) != value)
        {
            SIM_FEEDBACK1(100U + port_id + read_add);
        }
        else
        {
             SIM_FEEDBACK1(1000U + port_id + read_add);
        }
    }
    ret_value = MSS_GPIO_get_inputs( GPIO1_LO );
    for(port_id = MSS_GPIO_1; port_id <= MSS_GPIO_23; port_id += 2U )
    {
        if(((ret_value >> (MSS_GPIO_1  + read_add)) & 0x01U) != value)
        {
            SIM_FEEDBACK1(200U + port_id + read_add);
        }
        else
        {
             SIM_FEEDBACK1(2000U + port_id + read_add);
        }
    }


    /* Eugene wanted test to finish by writing ones */

    /*
     * write 1 to GPIO's
     */
    SIM_FEEDBACK0(300U);
    value = 1U;
    for(port_id = MSS_GPIO_0; port_id < MSS_GPIO_13; port_id += 2U )
    {
            MSS_GPIO_set_output( GPIO0_LO, port_id + write_add, value);
    }
    for(port_id = MSS_GPIO_0; port_id < MSS_GPIO_23; port_id += 2U )
    {
            MSS_GPIO_set_output( GPIO1_LO, port_id + write_add, value);
    }
    SIM_FEEDBACK1(400U);


#if 0
    i = 0U;
    uint32_t write_value[] = {0,}
    while(i < 14U)
    {

        value = 0U;
        for(port_id = MSS_GPIO_0; port_id <= MSS_GPIO_13; port_id += 2U )
        {
                MSS_GPIO_set_output( GPIO0_LO, port_id, value);
        }
        for(port_id = MSS_GPIO_1; port_id <= MSS_GPIO_23; port_id += 2U )
        {
                MSS_GPIO_set_output( GPIO0_LO, port_id, value);
        }

        delay(20);
        test = MSS_GPIO_get_inputs( GPIO0_LO );
        if(test != write_value)
        {
            SIM_FEEDBACK0(0x1000U + i);
            SIM_FEEDBACK0(test);
            SIM_FEEDBACK0(write_value);
        }
        i++;
    }
    i = 0U;
    while(i < 24U)
    {
        write_value = 0x1U << i;
        MSS_GPIO_set_outputs(GPIO1_LO, write_value);
        i++;
        delay(20);
        test = MSS_GPIO_get_inputs( GPIO0_LO );
        if(test != write_value)
        {
            SIM_FEEDBACK0(0x2000U + i);
            SIM_FEEDBACK0(test);
            SIM_FEEDBACK0(write_value);
        }
        i++;
    }
#endif

}

#endif /* end MSSIO_TEST */
