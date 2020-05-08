/*******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/*******************************************************************************
 * @file mss_sgmii.c
 * @author Microchip-FPGA Embedded Systems Solutions
 * @brief sgmii related functions
 *
 */

#include <string.h>
#include <stdio.h>
#include "mpfs_hal/mss_hal.h"
#include "simulation.h"

/*
 * local functions
 */
static void setup_sgmii_rpc_per_config(void);
static uint32_t sgmii_channel_setup(void);

/*
 * extern functions
 */
extern void pre_configure_sgmii_and_ddr_pll_via_scb(uint8_t option);


uint32_t sgmii_setup(void)
{
#ifdef SGMII_SUPPORT
    uint32_t mode = LIBERO_SETTING_SGMII_MODE;
	/*
	 * Check if any tx/Rx channels enabled
	 */
    if((mode & (TX_RX_CH_EN_MASK<<TX_RX_CH_EN_OFFSET)) != 0U)   
    {
        sgmii_channel_setup();
    }
    else
    {
        sgmii_off_mode();
#ifdef DDR_SUPPORT
        {
            pre_configure_sgmii_and_ddr_pll_via_scb(0U); /* 0U => configure
                                                            using the SCB */
        }
#endif
    }
#else
    {
        sgmii_off_mode();
#ifdef DDR_SUPPORT
        {
            pre_configure_sgmii_and_ddr_pll_via_scb(0U); /* 0U => configure
                                                            using the SCB */
        }
#endif
    }
#endif
    return(0UL);
}


/**
 *
 * @param sgmii_instruction
 * @return
 */
static uint32_t sgmii_channel_setup(void)
{
    /*
     * List from Eugene
     * 1.  Full SGMII test- both channels enabled, external loopback and rx_lock
     *     for both channels. (This test is written)
     *     (Will do a version of this for refclk = 125 mhz also- already
     *      supported in sw)
     * 2.  SGMII test, similar to 1 , except chan1 is off.
     * 3.  TEST with both CHAn0 and CHAn 1 off. This is the default state for
     *     sgmii anyway. The SGMII PLL will not be turned on either. This is our
     *     SGMII OFF MODE test.
     * 4.  RECALIB mode on (RECAL_CNTL-reg_recal_start_en =1). Both chans off.
     *     As I understand this is for DDR IO recalibration. I am not sure what
     *     I check for here. SO I will ask Jeff.
     * 5.  PVT recalibration..as per Srikanths presentation...SCB
     *     reg_calib_start =1 and APB calib_start =1 (see my earlier email.)
     *     This allows us check status of calib_start and calib_lock and
     *     calib_intrpt to be checked as per Srikanths presentation.
     *
     */

    /*
     * Hi Malachy,
     * These are the forces that I have been applying in my test. I suppose we
     * should integrate them in the code now.
     * I have got the loopback working, with the rx_side locking.
     * As we discussed before I am forcing DYN_CTRL (0x2000_7c1c)
     * dyn_cntl_reg_lane0_dynen, dyn_cntl_reg_lane1_dynen = 00.
     * These are required to start the tx_clock. (I realize you have a different
     * strategy here, which is included in the current code.)
     *
     * The CHAN0_CNTL reg (0x2000_7c0c): the reg_rx1_ibufmd_p [20:18] must be
     * set = 0x4. They default = 0x7 which is off (ref Ranjeets email)
     * Same for CHAN1_CNTL (0x2000_7c10).
     * This turns on the input buffers. Loopback implemented in testbench.
     * The SUBBLK_CLOCK_CR reg (2000_2084) should include setting [2:1] MAC0,
     * MAC1 = 11. Need the mac clocks running.
     * The nw_config register for mac0 (0x2011_0004): I am forcing 'gigabit' and
     * 'tbi' bits = 11.
     * The same for Mac1.
     * This starts up the tx_mac_clocks for the 2 macs.
     *
     * The SOFT_RESET_CR (0x2000_2088) register:  [2:1] MAC0, MAC1 resets should
     * be = 00 (ie turn off reset)
     * We should probably discuss these settings. (unless everything is clear!)
     * -eugene.
     *
     */
    SIM_FEEDBACK0(100U);
    CFG_DDR_SGMII_PHY->SOFT_RESET_SGMII.SOFT_RESET_SGMII = \
            (0x01 << 8U) | 1U; /* PERIPH   soft reset */
    CFG_DDR_SGMII_PHY->SOFT_RESET_SGMII.SOFT_RESET_SGMII = 1U;
    SIM_FEEDBACK1(1U);
    setup_sgmii_rpc_per_config();      /* load RPC SGMII_MODE register ext */

    /* Enable the Bank controller */
    {
        /*
         * Set soft reset on IP to load RPC to SCB regs (dynamic mode)
         * Bring the sgmii bank controller out of reset =- ioscb_bank_ctrl_sgmii
         */
        (*((uint32_t *) 0x3E400000U)) = 1U;  /* DPC_BITS      NV_MAP  reset */
        /*
         * Check the IO_EN signal here.
         * This is an output from the bank controller power detector, which are
         * turned on using MSS_IO_EN
         */
        volatile uint32_t timer_out=0U;
        /* aro_ioen_bnk  - PVT calibrator IOEN  */
        while((CFG_DDR_SGMII_PHY->PVT_STAT.PVT_STAT & (0x01U<<6U)) == 0U)
        {
            timer_out++;
        }

        /*
         * IO power ramp wait time
         * After IOEN is received from power detectors DDR and SGMii, extra time
         * required for voltage to ramp.
         * This time will come from the user- Dependent on ramp time of power
         * supply
         * Approximately - Bank power timer (from ioen_in to ioen_out = 10uS)?
         *
         */
        /*todo: implement proper timer- user will supply ramp time */
        timer_out=0U;
        while(timer_out < 0xFU)
        {
            timer_out++;
        }
    }

    SIM_FEEDBACK0(101U);
    {
        /*
         * fixme- not sure if we should be setting this register
         * From regmap detail:
         * bit 8 of MSS_RESET_CR
         * Asserts a reset the SGMII block containing the MSS reference clock
         * input.
         * Warning that setting this bit causes the external reference clock
         * input to the
         * MSS PLL to disappear.
         * It is advisable to use the SGMII channel soft resets in the PHY
         * instead of this bit.
         * However if E51 software wants to set this bit, the MSS clock source
         * should be switched over to the standby source in advance.
         */
        SCB_REGS->MSS_RESET_CR.MSS_RESET_CR = 0;
        SIM_FEEDBACK0(102U);
    }


    {
        /*
         * I ran the sim past the place where we set the nvmap_reset in the
         * SOFT_RESET_SGMII register and it did not result in any
         * change from the DLL default bits.
         * But I traced the 'flashing' signal on one of these regs back to
         * 'dll0_soft_reset_nv_map' (not 'pll0_soft_reset_periph').
         * Now the only place I can find 'dll0_soft_reset_nv_map' is in SCB
         * space...ie 0x3e10_0000 SOFT_RESET register.
         *
         */
        /*
         * so we have to use scb register to reset as no APB register available
         * to soft reset the IP
         * ioscb_dll_sgmii
         * */
        SIM_FEEDBACK1(2U);
        *((uint32_t *) 0x3E100000U) = (0x01U << 0x00U);  /*  reset sgmii DLL */
        SIM_FEEDBACK0(103U);
    }
    /*
      * I have discovered the problem with the tx channels (soft reset issue)
      * So we require the:
      *
      * sgmiiphy_lane 01 soft-reset register (0x3650_0000) to be written to
      * with 0x1 (to set the nv_map bit[0] =1 (self clears))
      * same for sgmiiphy_lane 23 soft-reset register (0x3651_0000).
      *
      * This will result in the rpc bits for the Lane controls to get loaded.
      * Not happening currently.
      *
      * The soft_reset_sgmii occurs in the mss_ddr.c line 436, so I suppose
      * we put the 2 new soft reset writes after that.
      *
      * PMCS: Temporary fix as these registers are not currently defined in the
      * headers.
      */
    {
        volatile uint32_t *temp_reg;
        // sgmiiphy_lane 01 soft-reset register (0x3650_0000)
        temp_reg = (uint32_t *)0x36500000;
        *temp_reg = 0x000000001U;
        // sgmiiphy_lane 23 soft-reset register (0x3650_0000)
        temp_reg = (uint32_t *)0x36510000;
        *temp_reg = 0x000000001U;
    }
    SIM_FEEDBACK0(104U);

    /*
     * Kick-off calibration, by taking calibration IP out of reset
     */
    /*
     * Soft reset
     */
    {
        /* PVT soft reset - APB*/
        SIM_FEEDBACK1(4U);
        /* reg_pvt_soft_reset_periph  */
        CFG_DDR_SGMII_PHY->DYN_CNTL.DYN_CNTL    = (0x01U<< 10U) | (0x7FU<<0U);
        /* reg_pvt_soft_reset_periph  */
        CFG_DDR_SGMII_PHY->DYN_CNTL.DYN_CNTL    = (0x7FU<<0U);
        SIM_FEEDBACK0(105U);
        /* PVT soft reset - SCB */
        SIM_FEEDBACK1(5U);
        /* make sure out of reset */
        IOSCB_IO_CALIB_SGMII->SOFT_RESET_IOCALIB     = 0x1U;
        /* make sure out of reset */
        IOSCB_IO_CALIB_SGMII->SOFT_RESET_IOCALIB     = 0x0U;
        SIM_FEEDBACK0(106U);
    }

    /*
     * Verify calibration
     * Bank 5 PVT calibrator can be controlled by MSS firmware through APB
     * registers to do initial calibration and re-calibration. During startup,
     * the initial calibration can be started by default when MSS releases SGMII
     * reset. Re-calibration is enabled by default with reg_pvt_calib_start/lock
     * bits being set to 1 before startup, and MSS firmware can start
     * re-calibration after startup by toggling  pvt_calib_start/lock bits per
     * PVT calibrator spec.
     *
     */
    SIM_FEEDBACK1(6U);
    while((CFG_DDR_SGMII_PHY->PVT_STAT.PVT_STAT & (1U << 14U)) == 0U)
    {
        SIM_FEEDBACK0(107U);
        SIM_FEEDBACK1(7U);
        SIM_FEEDBACK1(8U);
    }

    /*
     * now assert calib lock
     *    calibrated pcode and ncode will be written.
     * */
    {
        CFG_DDR_SGMII_PHY->PVT_STAT.PVT_STAT |= 0x40000000UL;
        IOSCB_IO_CALIB_SGMII->IOC_REG0 |= (0x01U<<14U);
    }
    SIM_FEEDBACK0(108U);

    /*
      * SGMii Step 3)   Wait for PLL and DLL lock
      * Delay codes generated
      */
    SIM_FEEDBACK1(9U);
    /* 0U => configure using scb, 1U => NVMAP reset */
    pre_configure_sgmii_and_ddr_pll_via_scb(1U);
    SIM_FEEDBACK1(10U);
    /* 0U => configure using scb, 1U => NVMAP reset */
    sgmii_pll_config_scb(1U);
    SIM_FEEDBACK0(109U);
    /*
     * Soft reset the DLL
     */
    //      CFG_DDR_SGMII_PHY->DYN_CNTL.DYN_CNTL   = (1U << 9U)|(0x7FU<<0U);
    /* todo: I think the above should soft reset should auto clear- Eugene to
     * confirm */
    //      CFG_DDR_SGMII_PHY->DYN_CNTL.DYN_CNTL   = (0U << 9U)|(0x7FU<<0U);
    /*
     * SGMii Step 4)    Monitor the DLL codes for Voltage and Temp variation
     * MSS E51 software sets the magnitude value of variation to flag.
     * MSS E51 software can poll this flag.
     * Re-calibration, if needed, is controlled by E51 software if needed.
     */
    /* ML step 4- This is a monitoring step- to be run constantly in the back
     * ground */

    SIM_FEEDBACK1(11U);
    /* PERIPH   soft reset */
    //   CFG_DDR_SGMII_PHY->SOFT_RESET_SGMII.SOFT_RESET_SGMII = 1U;

    /*
     * 5)   SGMII lane ctrl and Rx data alignment must wait for
     * the DLL codes generation (Step 5) completion before release from reset
     *
     */
    SIM_FEEDBACK1(12U);

    /*
     * Provide mac clocks
     * The nw_config register for mac0 (0x2011_0004): I am forcing 'gigabit' and
     * 'tbi' bits = 11.
     * The same for Mac1.
     * This starts up the tx_mac_clocks for the 2 macs.
     *
     */
    *((uint32_t *) 0x20110004U) |= (0x01U << 10U) | (0x01U << 11U);   /* GEM0 */
    SIM_FEEDBACK1(13U);
    *((uint32_t *) 0x20112004U) |= (0x01U << 10U) | (0x01U << 11U);   /* GEM1 */
    SIM_FEEDBACK1(14U);

    /*
     * DLL soft reset                   - Already configured
     * PVT soft reset                   - Already configured
     * Bank controller soft reset       - Already configured
     * CLKMUX soft reset                - Already configured
     * Lane0 soft reset                 - must be soft reset here
     * Lane1 soft reset                 - must be soft reset here
     *
     *      __IO  uint32_t               reg_lane0_soft_reset_periph :1;  bit 13
     *      __IO  uint32_t               reg_lane1_soft_reset_periph :1;  bit 14
     */
    CFG_DDR_SGMII_PHY->DYN_CNTL.DYN_CNTL  = (1U << 14U)|(1U << 13U)|(0x7FU<<0U);
    SIM_FEEDBACK1(15U);
    /* todo: I think the above should soft reset should auto clear- Eugene to
     * confirm */
    CFG_DDR_SGMII_PHY->DYN_CNTL.DYN_CNTL  = (0U << 14U)|(0U << 13U)|(0x7FU<<0U);
    SIM_FEEDBACK1(16U);

    /*
     *
     */
#if 0
    if (sgmii_instruction == SGMII_RECALIB)
    {
        SIM_FEEDBACK1(17U);
        recalib();
        SIM_FEEDBACK1(18U);
    }

    if (sgmii_instruction == SGMII_PVT_MONITOR)
    {
        SIM_FEEDBACK1(13U);
        recalib_monitor_test((uint8_t) (1U));
        SIM_FEEDBACK1(14U);
    }
#endif
#if 0
    if ((sgmii_instruction == SGMII_MAC0_LOOPBACK_TEST)||\
            (sgmii_instruction == SGMII_MAC1_LOOPBACK_TEST))
    {
        /*
         * Once SGMII setup, we can configure mss pll as external clock will be
         * available
         * */
        SIM_FEEDBACK0(19U);
        mss_pll_config();
        SIM_FEEDBACK1(20U);
        if(sgmii_instruction == SGMII_MAC0_LOOPBACK_TEST)
        {
            mac_test(0U);
        }
        else
        {
            mac_test(1U);
        }
        SIM_FEEDBACK1(21U);
    }
#endif
#if 0
    if (sgmii_instruction == SGMII_TRIM_IO)
    {
        SIM_FEEDBACK1(21U);
        calib_trim_test((uint8_t) 1U);
        SIM_FEEDBACK1(22U);
    }
#endif


    return(0U);
}



/**
 * setup_sgmii_rpc_per_config
 * @param sgmii_instruction
 */
static void setup_sgmii_rpc_per_config(void)
{
    CFG_DDR_SGMII_PHY->SGMII_MODE.SGMII_MODE    = LIBERO_SETTING_SGMII_MODE;
    CFG_DDR_SGMII_PHY->CH0_CNTL.CH0_CNTL        = LIBERO_SETTING_CH0_CNTL;
    CFG_DDR_SGMII_PHY->CH1_CNTL.CH1_CNTL        = LIBERO_SETTING_CH1_CNTL;
    CFG_DDR_SGMII_PHY->RECAL_CNTL.RECAL_CNTL    = LIBERO_SETTING_RECAL_CNTL;
    CFG_DDR_SGMII_PHY->CLK_CNTL.CLK_CNTL        = LIBERO_SETTING_CLK_CNTL;
    /* ibuffmx_p and _n rx1, bit 22 and 23 , rx0, bit 20 and 21 */
    CFG_DDR_SGMII_PHY->SPARE_CNTL.SPARE_CNTL    = LIBERO_SETTING_SPARE_CNTL;
    CFG_DDR_SGMII_PHY->PLL_CNTL.PLL_CNTL        = LIBERO_SETTING_PLL_CNTL;
}

/**
 * SGMII Off mode
 */
void    sgmii_off_mode(void)
{
    /*
     * do soft reset of SGMII TIP
     */
    CFG_DDR_SGMII_PHY->SOFT_RESET_SGMII.SOFT_RESET_SGMII = (0x01 << 8U) | 1U;
    CFG_DDR_SGMII_PHY->SOFT_RESET_SGMII.SOFT_RESET_SGMII = 1U;

    /*
     *
     */
    setup_sgmii_rpc_per_config();

    /*
     * Resetting the SCB register only required in already in dynamic mode. If
     * not reset, IO will not be configured.
     */
    // todo- remove magic value -  reset the sgmii by doing an SCB right
    *((uint32_t *) 0x3E100000U) = (0x01U << 0x00U);         /*  reset sgmii */

}



/**
 *
 */
void ddr_pvt_calibration(void)
{
    /*
     * R3.1
     * PVT calibration
     * Wait for IOEN from power detectors DDR and SGMII - IO enable signal from
     * System Control powers on
     *
     * From DDR phy SAC spec:
     *      MSS processor releases dce bus to send RPC bits to IO buffer,
     *      setting each to it’s programmed mode and then asserts
     *      ioen high at end of this state.
     *
     *
     *      Following verification required for MSS IO Calibration (DDRPHY,
     *      SGMII and MSSIO)
     *          Auto-calibration supply ramp time settings
     *          Calibration in reset until ioen_bnk goes high, timer complete
     *          and setup of bits complete
     *              scbclk divider setting (÷1)
     *              calibration clkdiv setting
     *              VS bit settings
     *          Initial non-calibrated codes to IOs (functional max codes)
     *          Calibration signal transitions
     *              pvt_calib_status ,        r in reg     DYN_CNTL
     *              reg_calib_reset,        w/r in reg     IOC_REG6
     *              calib_clkdiv,           w/r in reg     IOC_REG6
     *              soft_reset_periph_b,
     *              calib_lock,             w/r in reg     IOC_REG0
     *              calib_start,            w/r in reg     IOC_REG0
     *              calib_intrpt              r in reg
     *          Final calibration codes
     *          Lane latching of codes
     *          IO Glitching
     */
    volatile uint32_t timer_out=0U;

    #ifndef RENODE_DEBUG
    /* sro_ioen_out  */
    while((CFG_DDR_SGMII_PHY->IOC_REG1.IOC_REG1 & (1U<<4U)) == 0U)
    {
        timer_out++;
        /*todo: add a fail break */
    }
    #endif

    /*
     * R3.2  Trigger timer and wait for completion
     * PVT calibration
     * After IOEN is received from power detectors DDR and SGMII, extra time
     * required for voltage to ramp.
     * This time will come from the user- Dependent on ramp time of power supply
     * Approximately - Bank power timer (from ioen_in to ioen_out = 10uS)?
     *
     */
    /*todo: implement proper timer- user will supply ramp time */
    timer_out=0U;
    while(timer_out < 0xFU)
    {
        timer_out++;
    }

    /*
     * R3.2 Initiate calibration:
     *
     * IOC_REG6
     *  bit 2:1 reg_calib_clkdiv
     *  bit 0  reg_calib_reset
     *
     *      DDRIO:  calib_reset: 1 -> 0
     *      mss_write(0x20007000 + 0x21C,0x00000004);
     *      DDRIO: calib_rst_b: 0 -> 1
     *      mss_write(0x20007000 + 0x220,0x00000000);
     *      SGMII: calib_rst_b: 0 -> 1
     *      mss_write(0x20007000 + 0xC1C,0x00000000);
     *
     */
    /*
     * Soft reset
     */
    /* PVT soft reset - APB*/
    /* DDRIO:  calib_reset: 1 -> 0, clk divider changed - from 2 - to 3 */
    CFG_DDR_SGMII_PHY->IOC_REG6.IOC_REG6                    =    0x00000006U;

    /* PVT soft nv reset - SCB, should load from RPC */
    IOSCB_IO_CALIB_DDR->SOFT_RESET_IOCALIB       = 0x1U; /* make sure reset */
    IOSCB_IO_CALIB_DDR->SOFT_RESET_IOCALIB       = 0x0U; /* make sure reset */

    /*
     * R3.4 Wait for PVT calibration to complete
     * Check:
     * bit 2 sro_calib_status
     *
     * The G5 Memory controller needs to see that the IO calibration has
     * completed before kicking off DDR training.
     * It uses the calib_status signal as a flag for this.
     */
    timer_out=0U;
    #ifndef RENODE_DEBUG
    {
        /* PVT I/O - sro_calib_status -  wait for calibration to complete */
        while((IOSCB_IO_CALIB_DDR->IOC_REG1 & 0x00000004U) == 0U)
        {
            timer_out++;
        }
        /* PVT I/O - sro_calib_status -  wait for calibration to complete */
        while((CFG_DDR_SGMII_PHY->IOC_REG1.IOC_REG1 & 0x00000004U) == 0U)
        {
            timer_out++;
        }
    }
    #endif
    /*
     * now assert calib lock
     *
     * */
    {
        CFG_DDR_SGMII_PHY->IOC_REG0.IOC_REG0    &= ~(0x01U<<14U);
        IOSCB_IO_CALIB_DDR->IOC_REG0            &= ~(0x01U<<14U);
        CFG_DDR_SGMII_PHY->IOC_REG0.IOC_REG0    |= (0x01U<<14U);
        IOSCB_IO_CALIB_DDR->IOC_REG0            |= (0x01U<<14U);
    }
}


/**
 *
 */
void ddr_pvt_recalibration(void)
{
    volatile uint32_t timer_out=0U;

    /*
     * now assert calib start
     *
     * */
    {
        CFG_DDR_SGMII_PHY->IOC_REG0.IOC_REG0    &= ~(0x01U<<13U);
        IOSCB_IO_CALIB_DDR->IOC_REG0            &= ~(0x01U<<13U);
        CFG_DDR_SGMII_PHY->IOC_REG0.IOC_REG0    |= (0x01U<<13U);
        IOSCB_IO_CALIB_DDR->IOC_REG0            |= (0x01U<<13U);
    }

    /*
     * R3.4 Wait for PVT calibration to complete
     * Check:
     * bit 2 sro_calib_status
     *
     * The G5 Memory controller needs to see that the IO calibration has
     * completed before kicking off DDR training.
     * It uses the calib_status signal as a flag for this.
     */
    timer_out=0U;
    #ifndef RENODE_DEBUG
    {
        /* PVT I/O - sro_calib_status -  wait for calibration to complete */
        while((IOSCB_IO_CALIB_DDR->IOC_REG1 & 0x00000004U) == 0U)
        {
            timer_out++;
        }
        /* PVT I/O - sro_calib_status -  wait for calibration to complete */
        while((CFG_DDR_SGMII_PHY->IOC_REG1.IOC_REG1 & 0x00000004U) == 0U)
        {
            timer_out++;
        }
    }
    #endif
    /*
     * now assert calib lock
     *
     * */
    {
#if 0   /*
         * fixme: this appears to cause wite calibration to fail, investigating
         */
        CFG_DDR_SGMII_PHY->IOC_REG0.IOC_REG0    &= ~(0x01U<<14U);
        IOSCB_IO_CALIB_DDR->IOC_REG0            &= ~(0x01U<<14U);
        CFG_DDR_SGMII_PHY->IOC_REG0.IOC_REG0    |= (0x01U<<14U);
        IOSCB_IO_CALIB_DDR->IOC_REG0            |= (0x01U<<14U);
#endif
    }
}

