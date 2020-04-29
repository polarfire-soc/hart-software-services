/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/*******************************************************************************
 * @file mss_pll.c
 * @author Microchip-FPGA Embedded Systems Solutions
 * @brief MPSS PLL setup
 *
 */

#include "mpfs_hal/mss_hal.h"
#include "mss_pll.h"


/**
 * We do it this way to avoid multiple LDAR warnings
 * alternate it to
 * #define MSS_SCB_MSS_PLL      (IOSCB_CFM_MSS *) )x7xxxxxxx    The actual
 * address * but above results in error every time we use the function
 */

PLL_TypeDef *MSS_SCB_MSS_PLL = ((PLL_TypeDef *) MSS_SCB_MSS_PLL_BASE);
PLL_TypeDef *MSS_SCB_DDR_PLL = ((PLL_TypeDef *) MSS_SCB_DDR_PLL_BASE);
PLL_TypeDef *MSS_SCB_SGMII_PLL = ((PLL_TypeDef *) MSS_SCB_SGMII_PLL_BASE);
IOSCB_CFM_MSS *MSS_SCB_CFM_MSS_MUX          =\
        ((IOSCB_CFM_MSS *) MSS_SCB_MSS_MUX_BASE);
IOSCB_CFM_SGMII *MSS_SCB_CFM_SGMII_MUX =\
        ((IOSCB_CFM_SGMII *) MSS_SCB_SGMII_MUX_BASE);
IOSCB_IO_CALIB_STRUCT *IOSCB_IO_CALIB_SGMII =\
        (IOSCB_IO_CALIB_STRUCT *)IOSCB_IO_CALIB_SGMII_BASE;
IOSCB_IO_CALIB_STRUCT *IOSCB_IO_CALIB_DDR 	=\
        (IOSCB_IO_CALIB_STRUCT *)IOSCB_IO_CALIB_DDR_BASE;


/*******************************************************************************-
 * Symbols from the linker script used to locate the text, data and bss
 * sections.
 ******************************************************************************/
extern uint32_t __sc_load;
extern uint32_t __sc_start;
extern uint32_t __sc_end;


/*******************************************************************************
 * Local Defines                                                               *
 ******************************************************************************/

/*******************************************************************************
 * Local function declarations
 */

static void copy_switch_code(void);


/*******************************************************************************
 * Instance definitions                                                        *
 ******************************************************************************/

/******************************************************************************
 * Public Functions - API                                                      *
 ******************************************************************************/
void mss_pll_config(void);


/*******************************************************************************
 Local functions															   *
*******************************************************************************/
static void sgmii_mux_config_via_scb(uint8_t option);

/***************************************************************************//**
 * set_RTC_divisor()
 * Change the RTC clock divisor, so RTC clock is 1MHz
 * When changing the divider the enable should be turned off first, the divider
 * changed and the enable turned back on.
 * @param state source of clock
 * @return
 */

uint8_t set_RTC_divisor(void)
{
    uint8_t ret_state = (uint8_t)0;

    SYSREG->RTC_CLOCK_CR &= ~(0x01U<<16); /* disable RTC clock */

    SYSREG->RTC_CLOCK_CR = (LIBERO_SETTING_MSS_EXT_SGMII_REF_CLK / \
            LIBERO_SETTING_MSS_RTC_TOGGLE_CLK);

    SYSREG->RTC_CLOCK_CR |= (0x01U<<16); /* enable RTC clock */

    return (ret_state);
}


/***************************************************************************//**
 * mss_mux_pre_mss_pll_config()
 *
 * Feed through required reference clks to PLL, configure PLL and wait for lock
 ******************************************************************************/
static void mss_mux_pre_mss_pll_config(void)
{
    /*
     * PLL RF clk mux selections
     *  [31:15] Reserved
     *  [14:10] pll1_fdr_sel
     *  [9:8]   pll1_rfclk1_sel
     *  [7:6]   pll1_rfclk0_sel
     *  [5:4]   pll0_rfclk1_sel
     *  [3:2]   pll0_rfclk0_sel
     *  [1:0]   clk_in_mac_tsu_sel
     *
     *  Each mux uses 2 configuration bits. These are decoded as follows:
     *  00  vss
     *  01  refclk_p,refclk_n
     *  10  scb_clk
     *  10  serdes_refclk_crn_mss<0>, serdes_refclk_crn_mss<1>
     *
     *  e.g.
     *  PLL_CKMUX = 0x00000154
     *    pll0_rfclk0_sel = 1 => refclk_p is used for pll0 ref0 and refclk_n is
     *    fed to MSS PLL ref1
     */
    /* CFM_MSS 0x3E002000   - 0x08 */
    MSS_SCB_CFM_MSS_MUX->PLL_CKMUX = LIBERO_SETTING_MSS_PLL_CKMUX;
    /*
     * MSS Clock mux selections
     *  [31:5]  Reserved
     *  [4]     clk_standby_sel
     *  [3:2]   mssclk_mux_md
     *  step 7: 7)  MSS Processor writes mssclk_mux_sel_int<0>=1 to select the
     *  MSS PLL clock.
     *  [1:0]   mssclk_mux_sel        MSS glitchless mux select
     *                                      00 - msspll_fdr_0=clk_standby
     *                                           msspll_fdr_1=clk_standby
     *                                      01 - msspll_fdr_0=pllout0
     *                                           msspll_fdr_1=clk_standby
     *                                      10 - msspll_fdr_0=clk_standby
     *                                           msspll_fdr_1=pllout1
     *                                      11 - msspll_fdr_0=pllout0
     *                                           msspll_fdr_1=pllout1
     *
     *
     */
    /*
     * We will not set as already set to 0, we feed through after we have setup
     * clock
     * MSS_SCB_CFM_MSS_MUX->MSSCLKMUX           = LIBERO_SETTING_MSS_MSSCLKMUX;
     */

    /*
     * Clock_Receiver
     *  [13]    en_rdiff
     *  [12]    clkbuf_en_pullup
     *  [11:10] en_rxmode_n
     *  [9:8]   en_term_n
     *  [7]     en_ins_hyst_n
     *  [6]     en_udrive_n
     *  [5:4]   en_rxmode_p
     *  [3:2]   en_term_p
     *  [1]     en_ins_hyst_p
     *  [0]     en_udrive_p
     */
    MSS_SCB_CFM_SGMII_MUX->CLK_XCVR         = LIBERO_SETTING_SGMII_CLK_XCVR;

    /*
     * 29:25    bclk5_sel
     * 24:20    bclk4_sel
     * 19:15    bclk3_sel
     * 14:10    bclk2_sel
     * 9:5  bclk1_sel
     * 4:0  bclk0_sel
     *
     * From SAC spec:
     * Table 9 1: Each gbim bank clock mux programming in MSS corner
     * The DDRPHY bank clocks bclk_horz<5:0> and bclk_vert<5:0> are driven
     * from mux’s gbim<5:0> in the MSS corner. Each mux uses 5 configuration
     * bits.
     *
     * BCLK mux selections
     *  bclk0_sel=0x8 (pll0_out_1k<2> selected)
     *  bclk1_sel=0x10 (pll0_out_1k<3> selected)
     *  bclk2_sel=0x1 (vss selected)
     *  bclk3_sel=0x1 (vss selected)
     *  bclk4_sel=0x1 (vss selected)
     *  bclk5_sel=0x1 (vss selected)
     *
     */
    MSS_SCB_CFM_MSS_MUX->BCLKMUX        =   LIBERO_SETTING_MSS_BCLKMUX;

    /*  MSS_SCB_CFM_MSS_MUX->SPARE0         =   BCLKMUX_USER_CONFIG;       */
    MSS_SCB_CFM_MSS_MUX->FMETER_ADDR    =   LIBERO_SETTING_MSS_FMETER_ADDR;

    MSS_SCB_CFM_MSS_MUX->FMETER_DATAW   =   LIBERO_SETTING_MSS_FMETER_DATAW;

    MSS_SCB_CFM_MSS_MUX->FMETER_DATAR   =   LIBERO_SETTING_MSS_FMETER_DATAR;

    /*
     *
     */
    volatile uint32_t i;
    for(i = 0U; i < 400U; i++)
    {
        i++;
    }
}


/***************************************************************************//**
 * mss_mux_post_mss_pll_config(void)
 *
 * Once MSS locked, feed through to MSS
 * We must run this code from RAM, as we need to modify the clock of the eNVM
 * The first thing we do is change the eNVM clock, to prevent L1 cache accessing
 * eNVM as it will do as we approach the return instruction
 * The mb() makes sure order of processing is not changed by the compiler
 ******************************************************************************/
__attribute__((section(".ram_codetext"))) \
        static void mss_mux_post_mss_pll_config(void)
{
	/*
	 * Modify the eNVM clock, so it now matches new MSS clock
	 *
	 * [5:0]
	 * Sets the number of AHB cycles used to generate the PNVM clock,.
	 * Clock period = (Value+1) * (1000/AHBFREQMHZ)
	 * Value must be 1 to 63 (0 defaults to 15)
	 * e.g.
	 * 7 will generate a 40ns period 25MHz clock if the AHB clock is 200MHz
	 * 11 will generate a 40ns period 25MHz clock if the AHB clock is 250MHz
	 * 15 will generate a 40ns period 25MHz clock if the AHB clock is 400MHz
	 *
	 */
	SYSREG->ENVM_CR = LIBERO_SETTING_MSS_ENVM_CR;

	mb();	/* make sure we change clock in eNVM first so ready by the time we
	           leave */

	/*
	 * When you’re changing the eNVM clock frequency, there is a bit
	 * (ENVM_CR_clock_okay) in the eNVM_CR which can be polled to check that
	 * the frequency change has happened before bumping up the AHB frequency.
	 */
	volatile uint32_t wait_for_true;
    while ((SYSREG->ENVM_CR & ENVM_CR_CLOCK_OKAY_MASK) !=\
            ENVM_CR_CLOCK_OKAY_MASK)
    {
#ifdef RENODE_DEBUG
        break;
#endif
        wait_for_true++; /* need something here to stop debugger freezing */
    }

	/*
	 * Change the MSS clock as required.
	 *
	 * CLOCK_CONFIG_CR
	 * [5:0]
	 * Sets the master synchronous clock divider
	 * bits [1:0] CPU clock divider
	 * bits [3:2] AXI clock divider
	 * bits [5:4] AHB/APB clock divider
	 * 00=/1 01=/2 10=/4 11=/8 (AHB/APB divider may not be set to /1)
	 * Reset = 0x3F
	 *
	 * SYSREG->CLOCK_CONFIG_CR = (0x0U<<0U) | (0x1U<<2U) | (0x2U<<4U);
	 * MSS clk= 80Mhz, implies CPU = 80Mhz, AXI = 40Mhz, AHB/APB = 20Mhz
	 * Until we switch in MSS PLL clock (MSS_SCB_CFM_MSS_MUX->MSSCLKMUX = 0x01)
	 * e.g. If MSS clk 800Mhz
	 * MSS clk= 800Mhz, implies CPU = 800Mhz, AXI = 400Mhz, AHB/APB = 200Mhz
	*/
	SYSREG->CLOCK_CONFIG_CR = LIBERO_SETTING_MSS_CLOCK_CONFIG_CR;

	/*
	 * Feed clock from MSS PLL to MSS, using glitch-less mux
	 *
	 * MSS Clock mux selections
	 * 	[31:5]	Reserved
	 * 	[4]		clk_standby_sel
	 * 	[3:2]	mssclk_mux_md
	 * 	step 7: 7)	MSS Processor writes mssclk_mux_sel_int<0>=1 to select the
	 * 	MSS PLL clock.
	 * 	[1:0]	mssclk_mux_sel        MSS glitchless mux select
	 *										00 - msspll_fdr_0=clk_standby
 	 *       									 msspll_fdr_1=clk_standby
	 *										01 - msspll_fdr_0=pllout0
 	 *       									 msspll_fdr_1=clk_standby
	 *										10 - msspll_fdr_0=clk_standby
	 *        									 msspll_fdr_1=pllout1
	 *										11 - msspll_fdr_0=pllout0
 	 *       									 msspll_fdr_1=pllout1
	 *
	 *
	 */
	MSS_SCB_CFM_MSS_MUX->MSSCLKMUX 	= LIBERO_SETTING_MSS_MSSCLKMUX;

	/*
	 * Change the RTC clock divisor, so RTC clock is 1MHz
	 */
	set_RTC_divisor();
}

/***************************************************************************//**
 * sgmii_mux_config_via_scb(uint8_t option)
 * @param option 1 => soft reset, load RPC settings
 *               0 => write values using SCB
 ******************************************************************************/
static void sgmii_mux_config_via_scb(uint8_t option)
{
    switch(option)
    {

        case 0:   /* write to   SCB register */

                /*
                 * SCB address: 0x3E20 0008
                 * MSS Clock mux selections
                 *
                 * [31:0] SGMII_CLKMUX
                 */
                /* CFM_ETH - 0x3E200000 - - 0x08 */
                MSS_SCB_CFM_SGMII_MUX->SGMII_CLKMUX =\
                LIBERO_SETTING_SGMII_SGMII_CLKMUX;
                /*
                 * SCB address: 0x3E20 0010
                 * Clock_Receiver
                 *
                 * [13] 	en_rdiff
                 * [12] 	clkbuf_en_pullup
                 * [11:10] 	en_rxmode_n
                 * [9:8] 	en_term_n
                 * [7] 		en_ins_hyst_n
                 * [6] 		en_udrive_n
                 * [5:4] 	en_rxmode_p
                 * [3:2] 	en_term_p
                 * [1] 		en_ins_hyst_p
                 * [0] 		en_udrive_p
                 */
                MSS_SCB_CFM_SGMII_MUX->CLK_XCVR =\
                        LIBERO_SETTING_SGMII_CLK_XCVR;   /* 0x2011 */
                /*
                 * SCB address: 0x3E20 0004
                 * PLL RF clk mux selections
                 *
                 * [3:2] pll0_rfclk1_sel
                 * 								00 => vss
                 * 								01 => refclk_p muxed to DDR PLL
                 * 								    and SGMII PLL ( see
                 * 								10 => scb_clk
                 * 								11 => serdes_refclk_crn_mss<1>
                 * [1:0] pll0_rfclk0_sel
                 * 								00 => vss
                 * 								01 => refclk_n muxed to DDR PLL
                 * 								    and SGMII PLL
                 * 								10 => scb_clk
                 * 								11 => serdes_refclk_crn_mss<1>
                 *
                 *
                 */
                /* 0x05 => ref to SGMII and DDR */
                MSS_SCB_CFM_SGMII_MUX->RFCKMUX	=\
                        LIBERO_SETTING_SGMII_SGMII_CLKMUX;
                break;

        case 1:
            /*
             * set the NV map reset
             * This will load the APB registers, set via SGMII TIP.
             * */
            MSS_SCB_CFM_SGMII_MUX->SOFT_RESET = 1U;
            break;
    }
}


/***************************************************************************//**
 * pre_configure_sgmii_and_ddr_pll_via_scb(option)
 * @param option 1 => soft reset, load RPC settings
 *               0 => write values using SCB
 ******************************************************************************/
void pre_configure_sgmii_and_ddr_pll_via_scb(uint8_t option)
{
	sgmii_mux_config_via_scb(option);
}

void post_configure_sgmii_and_ddr_pll_via_scb(void)
{

}


/***************************************************************************//**
 *
 * On startup, MSS supplied with 80MHz SCB clock
 *
 *
 *
 *
	From the DDRPHY SBMii SAC spec, section 9.2

	9.2	Power on procedure for the MSS PLL clock

	During POR:
	Keep PLL in power down mode. powerdown_int_b=0

	After POR, Power-On steps:
	1)	mssclk_mux_sel_int<0>=0 & powerdown_int_b=0 & clk_standby_sel=0
	MSS PLL is powered down and selects clk_standby=scb_clk
	2)	G5C Processor writes powerdown_int_b=1 & divq0_int_en=1
	MSS PLL powers up, then lock asserts when locked.
	3)	G5C Processor switches mssclk_mux_sel_int<0>=1
	MSS PLL clock is now sent to MSS.
	4)	When BOOT authentication is complete
	a.	G5C processor writes mssclk_mux_sel_int<0>=0 to select clk_standby.
	b.	G5C Processor writes powerdown_int_b=0 to power down the PLL
	>>>>>>>> G5 Soc User code >>>>>>>>>>>>>>
	c.	MSS Processor writes new parameters to the MSS PLL
	5)	MSS Processor writes powerdown_int_b=1
	Start up the PLL with NEW parameters.
	Wait for LOCK
	6)	MSS Processor enables all 4 PLL outputs.
	7)	MSS Processor writes mssclk_mux_sel_int<0>=1 to select the MSS PLL
	    clock.
 *
 ******************************************************************************/
void mss_pll_config(void)
{
    copy_switch_code(); /* copy switch code to RAM */

    MSS_SCB_MSS_PLL->SOFT_RESET     = 0x00000000U;
    /*
        Enable the PLL by removing the reset-
        PERIPH / periph_reset_b  - This asserts the functional reset of the
            block.
        It is asserted at power up. When written is stays asserted until written
        to 0.
    */
    /*
     * 4. c.    MSS Processor writes new parameters to the MSS PLL
     */
    /*
     *  [0] REG_BYPASS_GO_B
     *  [0] BYPCK_SEL
     *  [0] RESETONLOCK
     *  [0] REG_RFCLK_SEL
     *  [0] REG_DIVQ3_EN
     *  [0] REG_DIVQ2_EN
     *  [0] REG_DIVQ1_EN
     *  [0] REG_DIVQ0_EN
     *  [0] REG_RFDIV_EN
     *  [0] REG_POWERDOWN_B
     */

    /* MSS PLL - 0x3E001000 - */
    MSS_SCB_MSS_PLL->PLL_CTRL       = LIBERO_SETTING_MSS_PLL_CTRL;
    /* PLL calibration register */

    MSS_SCB_MSS_PLL->PLL_CAL        = LIBERO_SETTING_MSS_PLL_CAL;

    MSS_SCB_MSS_PLL->PLL_REF_FB     = LIBERO_SETTING_MSS_PLL_REF_FB;

    MSS_SCB_MSS_PLL->PLL_DIV_0_1    = LIBERO_SETTING_MSS_PLL_DIV_0_1;

    MSS_SCB_MSS_PLL->PLL_DIV_2_3    = LIBERO_SETTING_MSS_PLL_DIV_2_3;

    MSS_SCB_MSS_PLL->PLL_CTRL2      = LIBERO_SETTING_MSS_PLL_CTRL2;

    MSS_SCB_MSS_PLL->PLL_FRACN      = LIBERO_SETTING_MSS_PLL_FRACN;
    MSS_SCB_MSS_PLL->SSCG_REG_0     = LIBERO_SETTING_MSS_SSCG_REG_0;
    MSS_SCB_MSS_PLL->SSCG_REG_1     = LIBERO_SETTING_MSS_SSCG_REG_1;

    MSS_SCB_MSS_PLL->SSCG_REG_2     = LIBERO_SETTING_MSS_SSCG_REG_2;
    MSS_SCB_MSS_PLL->SSCG_REG_3     = LIBERO_SETTING_MSS_SSCG_REG_3;

    /* PLL phase registers */
    MSS_SCB_MSS_PLL->PLL_PHADJ      = LIBERO_SETTING_MSS_PLL_PHADJ;

    /*
     * 5)   MSS Processor writes powerdown_int_b=1
     *      Start up the PLL with NEW parameters.
            Wait for LOCK
     */
    mss_mux_pre_mss_pll_config(); /* feed required inputs */
    /* bit 0 == REG_POWERDOWN_B */
    MSS_SCB_MSS_PLL->PLL_CTRL       = (LIBERO_SETTING_MSS_PLL_CTRL) | 0x01U;
    /*
     * Start up the PLL with NEW parameters.
     * Wait for LOCK
     *  todo: make wait clock based
     */
    volatile uint32_t timer_out=0x000000FFU;
    while(((MSS_SCB_MSS_PLL->PLL_CTRL & ((0x01U) << 25U))) == 0U)
    {
#ifdef RENODE_DEBUG
        break;
#endif
        if (timer_out != 0U)
        {
            timer_out--;
        }
        else
        {
            //todo: add failure mode
        }
    }

    /*
     * 6)   MSS Processor enables all 4 PLL outputs.
     * 7)   MSS Processor writes mssclk_mux_sel_int<0>=1 to select the MSS PLL
     *      clock.
     */
    mss_mux_post_mss_pll_config();
}

/***************************************************************************//**
 *
 * Test
 * Pll_out[0] = 800 MHz
 * Pll_out[1] = 800 Mhz
 * Pll_out[2] = 200 MHz
 * Pll_out[3] = 80 MHz
 *
 ******************************************************************************/
void ddr_pll_config_scb(void)
{
    /* PERIPH / periph_reset_b  -This asserts the functional reset of the block.
       It is asserted at power up. When written is stays asserted until written
       to 0. */
	MSS_SCB_DDR_PLL->SOFT_RESET 	= 0x00000000U;
	/* MSS PLL - 0x3E001000 - */
    MSS_SCB_DDR_PLL->PLL_CTRL       = LIBERO_SETTING_DDR_PLL_CTRL;
    /* PLL calibration register */

    MSS_SCB_DDR_PLL->PLL_CAL        = LIBERO_SETTING_DDR_PLL_CAL;

    MSS_SCB_DDR_PLL->PLL_REF_FB     = LIBERO_SETTING_DDR_PLL_REF_FB;


    MSS_SCB_DDR_PLL->PLL_DIV_0_1    = LIBERO_SETTING_DDR_PLL_DIV_0_1;

    MSS_SCB_DDR_PLL->PLL_DIV_2_3    = LIBERO_SETTING_DDR_PLL_DIV_2_3;


    MSS_SCB_DDR_PLL->PLL_CTRL2      = LIBERO_SETTING_DDR_PLL_CTRL2;

    MSS_SCB_DDR_PLL->PLL_FRACN      = LIBERO_SETTING_DDR_PLL_FRACN;
    MSS_SCB_DDR_PLL->SSCG_REG_0     = LIBERO_SETTING_DDR_SSCG_REG_0;
    MSS_SCB_DDR_PLL->SSCG_REG_1     = LIBERO_SETTING_DDR_SSCG_REG_1;

    MSS_SCB_DDR_PLL->SSCG_REG_2     = LIBERO_SETTING_DDR_SSCG_REG_2;
    MSS_SCB_DDR_PLL->SSCG_REG_3     = LIBERO_SETTING_DDR_SSCG_REG_3;

    /* PLL phase registers */

    MSS_SCB_DDR_PLL->PLL_PHADJ      = LIBERO_SETTING_DDR_PLL_PHADJ;
    /* bit 0 == REG_POWERDOWN_B */
    MSS_SCB_DDR_PLL->PLL_CTRL       = (LIBERO_SETTING_DDR_PLL_CTRL) | 0x01U;

    volatile uint32_t timer_out=0x000000FFU;
    while(((MSS_SCB_MSS_PLL->PLL_CTRL & ((0x01U) << 25U))) == 0U)
    {
#ifdef RENODE_DEBUG
        break;
#endif
        // todo: make clock based
        if (timer_out != 0U)
        {
            timer_out--;
        }
        else
        {
            //todo: add failure mode
        }
    }
}

/***************************************************************************//**
 *
 ******************************************************************************/
void ddr_pll_config_scb_turn_off(void)
{
    /* PERIPH / periph_reset_b */
    MSS_SCB_DDR_PLL->PLL_CTRL       &= ~0x00000001UL;
}


/***************************************************************************//**
 * sgmii_pll_config_scb(uint8_t option)
 * @param option 1 => soft reset, load RPC settings
 *               0 => write values using SCB
 ******************************************************************************/
void sgmii_pll_config_scb(uint8_t option)
{

    switch(option)
    {

        case 0:   /* write to   SCB register */
            /* PERIPH / periph_reset_b  - This asserts the functional reset of
             * the block. It is asserted at power up. When written is stays
             * asserted until written to 0. */

            MSS_SCB_SGMII_PLL->SOFT_RESET       = 0x00000000U;


            /* MSS PLL - 0x3E001000 - */
            MSS_SCB_SGMII_PLL->PLL_CTRL      = LIBERO_SETTING_SGMII_PLL_CTRL;
            /* PLL calibration register */

            MSS_SCB_SGMII_PLL->PLL_CAL       = LIBERO_SETTING_SGMII_PLL_CAL;

            MSS_SCB_SGMII_PLL->PLL_REF_FB    = LIBERO_SETTING_SGMII_PLL_REF_FB;


            MSS_SCB_SGMII_PLL->PLL_DIV_0_1   = LIBERO_SETTING_SGMII_PLL_DIV_0_1;

            MSS_SCB_SGMII_PLL->PLL_DIV_2_3   = LIBERO_SETTING_SGMII_PLL_DIV_2_3;


            MSS_SCB_SGMII_PLL->PLL_CTRL2     = LIBERO_SETTING_SGMII_PLL_CTRL2;

            MSS_SCB_SGMII_PLL->PLL_FRACN     = LIBERO_SETTING_SGMII_PLL_FRACN;
            MSS_SCB_SGMII_PLL->SSCG_REG_0    = LIBERO_SETTING_SGMII_SSCG_REG_0;
            MSS_SCB_SGMII_PLL->SSCG_REG_1    = LIBERO_SETTING_SGMII_SSCG_REG_1;

            MSS_SCB_SGMII_PLL->SSCG_REG_2    = LIBERO_SETTING_SGMII_SSCG_REG_2;
            MSS_SCB_SGMII_PLL->SSCG_REG_3    = LIBERO_SETTING_SGMII_SSCG_REG_3;

            /* PLL phase registers */

            MSS_SCB_SGMII_PLL->PLL_PHADJ     = LIBERO_SETTING_MSS_PLL_PHADJ;

            MSS_SCB_SGMII_PLL->PLL_CTRL      = (LIBERO_SETTING_SGMII_PLL_CTRL)\
                    | 0x01U;  /* bit 0 == REG_POWERDOWN_B */

            break;


        case 1:
            /*
             * set the NV map reset
             * This will load the APB registers, set via SGMII TIP.
             * */
            /* bit 0 == REG_POWERDOWN_B */
            MSS_SCB_SGMII_PLL->SOFT_RESET  = 0x01U;
            break;
    }

    /*
     * check lock has occurred
     * todo: make clock based
     */
    volatile uint32_t timer_out=0x00000FFFU;
    while(((MSS_SCB_MSS_PLL->PLL_CTRL & ((0x01U) << 25U))) == 0U)
    {
#ifdef RENODE_DEBUG
        break;
#endif
        if (timer_out == 0U)
        {
            timer_out--;
        }
    }
}


/***************************************************************************//**
 * Copy switch code routine to RAM.
 * Copy locations have been defined in the linker script
 ******************************************************************************/
static void copy_switch_code(void)
{
    uint32_t * sc_lma = &__sc_load;
    uint32_t * end_sc_vma = &__sc_end;
    uint32_t * sc_vma = &__sc_start;

	if ( sc_vma != sc_lma )
	{
		while ( sc_vma < end_sc_vma )
		{
			*sc_vma = *sc_lma;
			sc_vma++ ; sc_lma++;
		}
	}
}




