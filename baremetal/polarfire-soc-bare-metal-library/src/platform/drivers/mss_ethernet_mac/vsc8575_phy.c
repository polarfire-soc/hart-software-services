/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Microsemi VSC8575 PHY interface driver implementation to support the
 * peripheral daughter board for the G5 SoC Emulation Platform.
 *
 * SVN $Revision$
 * SVN $Date$
 *
 */

#include "mss_plic.h"

#include "drivers/mss_mac/mss_ethernet_registers.h"
#include "drivers/mss_mac/mss_ethernet_mac_regs.h"
#include "drivers/mss_mac/mss_ethernet_mac_user_config.h"
#include "drivers/mss_mac/mss_ethernet_mac.h"
#include "drivers/mss_mac/phy.h"

#include "drivers/mss_mac/mss_ethernet_mac_types.h"

#include "mss_plic.h"
#include "fpga_design_config/fpga_design_config.h"
#include "mss_coreplex.h"

#include "hal/hal.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#if MSS_MAC_USE_PHY_VSC8575
#include "vtss_api.h"   /* For BOOL and friends */
#include "vtss_phy_api.h"   /* For PHY API Pre and Post Resets */
#include "vtss_init_api.h"

extern int32_t viper_fmc_board_init(vtss_init_conf_t *config);
#endif

#if MSS_MAC_USE_PHY_VSC8575_LITE
#include "vtss_phy_common.h"
#include "vtss_viper_phy_prototypes.h"

extern int32_t viper_fmc_board_init(struct phy_control_t *control);
#endif


#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************/
/* Preprocessor Macros                                                    */
/**************************************************************************/

#define BMSR_AUTO_NEGOTIATION_COMPLETE  (0x0020U)

/**************************************************************************//**
 *
 */


uint16_t VSC8575_reg_0[32];
uint16_t VSC8575_reg_1[16];
uint16_t VSC8575_reg_2[16];
uint16_t VSC8575_reg_3[20]; /* Additional 4 to hold MAC Serdes RX and TX stats */
uint16_t VSC8575_reg_4[16];
uint16_t VSC8575_reg_16[32];
uint16_t VSC8575_MSS_SGMII_reg16[17];
uint32_t VSC8575_MSS_MAC_reg[80];
uint32_t VSC8575_MSS_PLIC_REG[80];

void dump_vsc8575_regs(const mss_mac_instance_t * this_mac);
void dump_vsc8575_regs(const mss_mac_instance_t * this_mac)
{
    int32_t count;
    uint16_t page;
    uint16_t old_page;
    uint16_t *pdata;
    volatile psr_t lev;

    for(page = 0U; page <= 0x10U; page++)
    {
        if(0U == page)
        {
            pdata = VSC8575_reg_0;
        }
        else if(1U == page)
        {
            pdata = VSC8575_reg_1;
        }
        else if(2U == page)
        {
            pdata = VSC8575_reg_2;
        }
        else if(3U == page)
        {
            pdata = VSC8575_reg_3;
        }
        else if(4U == page)
        {
            pdata = VSC8575_reg_4;
        }
        else if(16U == page)
        {
            pdata = VSC8575_reg_16;
        }
        else
        {
            pdata = VSC8575_reg_0;
        }

        if((0U == page) || (0x10U == page))
        {
            for(count = 0; count <= 0x1F; count++)
            {
                lev = HAL_disable_interrupts();
                old_page = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU);

                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, page);

                pdata[count] = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, (uint8_t)count);

                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, old_page);
                HAL_restore_interrupts(lev);

            }
        }
        else
        {
            for(count = 0x10; count <= 0x1F; count++)
            {
                lev = HAL_disable_interrupts();
                old_page = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU);

                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, page);

                pdata[count - 0X10] = MSS_MAC_read_phy_reg(this_mac,(uint8_t) this_mac->phy_addr, (uint8_t)count);

                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, old_page);
                HAL_restore_interrupts(lev);
            }

            if(3U == page) /* Circle back and get MAC Serdes stats... */
            {
                lev = HAL_disable_interrupts();
                old_page = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU);

                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, page);
                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1DU, 0x4000U); /* Select MAC stats */
                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x16U, 0x4000U); /* Select MAC stats */

                pdata[0x10] = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1CU); /* Fetch MAC stats */
                pdata[0x11] = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1DU);
                pdata[0x12] = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x15U); /* Fetch MAC stats */
                pdata[0x13] = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x16U);

                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x16U, 0x0000U); /* Select Media stats */
                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1DU, 0x0000U); /* Select Media stats */
                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, old_page);
                HAL_restore_interrupts(lev);
            }
        }

        if(4U == page)
        {
            page = 0x0FU;
        }
    }

    for(count = 0; count <= 0x10; count++)
    {
        VSC8575_MSS_SGMII_reg16[count] = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, (uint8_t)count);
    }

#if 0 /* Only enable as necessary as reading some regs can interfere with ISR operation */
    {
        uint32_t *pbigdata;

        pbigdata = (uint32_t *)0x20110000UL;
        for(count = 0; count < 19; count++, pbigdata++)
        {
            VSC8575_MSS_MAC_reg[count] = *pbigdata;
        }

        pbigdata =(uint32_t *)0x0C000000UL;
        for(count = 0; count < 8; count++, pbigdata++)
        {
            VSC8575_MSS_PLIC_REG[count] = *pbigdata;
        }
    }
#endif
}


/**************************************************************************//**
 * Note: The following assumes there is only one VSC8757 PHY in the system.
 *       We will need to revisit it at some stage to provide for multiple
 *       devices connected to separate GEMs or for the case of multiple
 *       GEMs connected to the same PHY but to different ports.
 *
 *       For now we need to at least record somewhere which GEM this PHY
 *       is associated with so that the MDIO functions can work correctly as
 *       it is currently assumed GEM0 is the one connected to the VSC8575.
 *
 *       May need to consider adding entry to vtss_inst_t to hold the mac
 *       reference so MDIO can figure out who to talk to. This will also mean
 *       that the some or all of following globals will need to be changed to
 *       allow for multiple connections...
 *
 */

mss_mac_instance_t     *g_my_mac = (mss_mac_instance_t *)0;


#if MSS_MAC_USE_PHY_VSC8575
void MSS_MAC_VSC8575_phy_init(/* mss_mac_instance_t*/ const void *v_this_mac, uint8_t phy_addr)
{
    static vtss_inst_t             g_vtss_inst_p;
    static vtss_inst_create_t      g_vtss_create_inst;
    static vtss_init_conf_t        g_vtss_init_conf;
    static vtss_phy_conf_t         g_phy;
    static vtss_phy_conf_1g_t      g_phy_conf_1g;
    static vtss_phy_reset_conf_t   g_phy_reset_conf;

    static vtss_phy_reset_conf_t vts_phy_init_params = {VTSS_PORT_INTERFACE_SGMII, VTSS_PHY_MEDIA_IF_CU, {1,2},
         {TRUE},VTSS_PHY_FORCE_RESET, VTSS_PHY_PKT_MODE_IEEE_1_5_KB, TRUE};

    const mss_mac_instance_t *this_mac = (const mss_mac_instance_t *)v_this_mac;
    volatile vtss_rc vrc;
    volatile int32_t vrc_i32;

    (void)phy_addr;
    g_my_mac = this_mac; /* For now, simply record which MAC we are associated with
                          * assuming there is only one...
                          */
    (void)memset (&g_phy, 0, sizeof(vtss_phy_conf_t));

    g_vtss_create_inst.target = VTSS_TARGET_CU_PHY;

    vrc = vtss_inst_create(&g_vtss_create_inst, &g_vtss_inst_p);

    vrc = vtss_init_conf_get(g_vtss_inst_p, &g_vtss_init_conf);
    vrc_i32 = viper_fmc_board_init(&g_vtss_init_conf);

    g_vtss_init_conf.warm_start_enable = 0;
    g_vtss_init_conf.restart_info_src  = VTSS_RESTART_INFO_SRC_CU_PHY;
    g_vtss_init_conf.restart_info_port = 0;

    vrc = vtss_init_conf_set(g_vtss_inst_p, &g_vtss_init_conf);

    (void)memset(&g_phy_reset_conf, 0, sizeof(g_phy_reset_conf));

    g_phy_reset_conf.mac_if = VTSS_PORT_INTERFACE_SGMII;
    g_phy_reset_conf.media_if = VTSS_PHY_MEDIA_IF_CU;
    g_phy_reset_conf.rgmii.rx_clk_skew_ps = 0;
    g_phy_reset_conf.rgmii.tx_clk_skew_ps = 0;
    g_phy_reset_conf.tbi.aneg_enable = 1;

    vrc = vtss_phy_pre_reset(g_vtss_inst_p, 0);
    /* HH:  unsure why vts_phy_init_params being passed instead of g_phy_reset_conf to vtss_phy_reset(...) ??? */
    vrc = vtss_phy_reset(g_vtss_inst_p, 0, &vts_phy_init_params);
    /* HH:  moved SerDes calibration to after the MAC PCS settings configured */

    /* Configure PHY 1G master/slave preference (for SyncE timing) */
    (void)memset(&g_phy_conf_1g, 0, sizeof(g_phy_conf_1g));
    /* HH:  conf_get to pre-populate all entries with valid defaults, prior to application settings */
    vtss_phy_conf_get(g_vtss_inst_p, 0 /* port_no */, &g_phy);

    g_phy_conf_1g.master.cfg = TRUE;   /* 1=Enabled */
    g_phy_conf_1g.master.val = TRUE;   /* 1=Master */

    /* HH move conf get/set until after port conf setting configured below */

    g_phy.mode = VTSS_PHY_MODE_ANEG;

    /* Example for PHY speed support for auto-neg  */
    g_phy.aneg.speed_10m_hdx  = 1;
    g_phy.aneg.speed_10m_fdx  = 1;
    g_phy.aneg.speed_100m_hdx = 1;
    g_phy.aneg.speed_100m_fdx = 1;
    g_phy.aneg.speed_1g_hdx   = 0;
    g_phy.aneg.speed_1g_fdx   = 1;

    g_phy.sigdet = VTSS_PHY_SIGDET_POLARITY_ACT_HIGH;

    /* Example for PHY flow control settings  */
    g_phy.aneg.symmetric_pause =  1;
    g_phy.aneg.tx_remote_fault =  1;

    g_phy.mdi = VTSS_PHY_MDIX_AUTO; /* always enable auto detection of crossed/non-crossed cables */
    g_phy.flf = VTSS_PHY_FAST_LINK_FAIL_DISABLE;

    /*  Setup the MAC Interface PCS Parameters */
    g_phy.mac_if_pcs.disable           = 0;
    g_phy.mac_if_pcs.restart           = 0;
    g_phy.mac_if_pcs.pd_enable         = 0;
    g_phy.mac_if_pcs.aneg_restart      = 0;
    g_phy.mac_if_pcs.force_adv_ability = 0;
    g_phy.mac_if_pcs.sgmii_in_pre = VTSS_PHY_MAC_SERD_PCS_SGMII_IN_PRE_NONE;
    g_phy.mac_if_pcs.sgmii_out_pre      = 0;
    g_phy.mac_if_pcs.serdes_aneg_ena    = 1;
    g_phy.mac_if_pcs.serdes_pol_inv_in  = 0;
    g_phy.mac_if_pcs.serdes_pol_inv_out = 0;
    g_phy.mac_if_pcs.fast_link_stat_ena = 0;
    g_phy.mac_if_pcs.inhibit_odd_start  = 0; /* Does nothing as this bit id default on... */

/*  Setup the MEDIA Interface PCS Parameters */
    g_phy.media_if_pcs.remote_fault       = VTSS_PHY_MEDIA_SERD_PCS_REM_FAULT_NO_ERROR;
    g_phy.media_if_pcs.aneg_pd_detect     = 0;
    g_phy.media_if_pcs.force_adv_ability  = 0;
    g_phy.media_if_pcs.serdes_pol_inv_in  = 0;
    g_phy.media_if_pcs.serdes_pol_inv_out = 0;
    g_phy.media_if_pcs.inhibit_odd_start  = 1;
    g_phy.media_if_pcs.force_hls          = 0;
    g_phy.media_if_pcs.force_fefi         = 0;
    g_phy.media_if_pcs.force_fefi_value   = 0;

#if 0
    {
        vtss_phy_clock_conf_t clock_conf;
        clock_conf.freq = VTSS_PHY_FREQ_125M;
        clock_conf.squelch = VTSS_PHY_CLK_SQUELCH_MAX;
        clock_conf.src = 0;
        vtss_phy_clock_conf_set(g_vtss_inst_p, 0, 0, &clock_conf);
    }
#endif

     vrc = vtss_phy_conf_set(g_vtss_inst_p, 0 /*port_no*/, &g_phy);

    /* HH:  moved SerDes calibration here */
    vrc = vtss_phy_post_reset(g_vtss_inst_p, 0);

    {
        uint16_t old_page;
        uint16_t temp_reg;

        old_page = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU);
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, 3U);
        temp_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x17U);
        temp_reg &= (uint16_t)(~0x0010U); /* Clear media inhibit odd start delay  bit */
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x17U, temp_reg);

        temp_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x10U);
        temp_reg &= (uint16_t)(~0x0004U); /* Clear mac inhibit odd start delay  bit */
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x10U, temp_reg);

        temp_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x10U);
        temp_reg &= (uint16_t)(~0x0100U); /* Turn off 2 byte preamble */
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x10U, temp_reg);

        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, old_page);
    }
    {
        uint16_t phy_reg;
        volatile uint32_t sgmii_aneg_timeout = 100000U;
        uint16_t autoneg_complete;

        if(TBI == this_mac->interface_type)
        {
            phy_reg = (uint16_t)this_mac->mac_base->PCS_CONTROL;
            phy_reg |= 0x1000U;
            this_mac->mac_base->PCS_CONTROL = phy_reg;
            phy_reg |= 0x0200U;
            this_mac->mac_base->PCS_CONTROL = phy_reg;

            /* Wait for SGMII auto-negotiation to complete. */
            do {
                phy_reg = (uint16_t)this_mac->mac_base->PCS_STATUS;
                autoneg_complete = phy_reg & BMSR_AUTO_NEGOTIATION_COMPLETE;
                --sgmii_aneg_timeout;
            } while(((0U == autoneg_complete) && (0U != sgmii_aneg_timeout)) || (0xFFFF == phy_reg));
        }

        if(GMII_SGMII == this_mac->interface_type)
        {
            /*
             * SGMII to GMII core with embedded MDIO interface for SGMII
             * link control.
             *
             * Reset SGMII core side of I/F.
             */
            phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR);
            phy_reg |= 0x9000U;    /* Reset and start autonegotiation */
            phy_reg &= 0xFBFFU;    /* Clear Isolate bit */
            MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR, phy_reg);

            phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR);
            phy_reg |= 0x1000U;    /* start autonegotiation */
            phy_reg &= 0xFBFFU;    /* Clear gmii Isolate bit */
            MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR, phy_reg);
        }
#if 0
        /* Far-end loopback - enable at your peril... DO NOT try this on an open network*/
        phy_reg =  MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x17U);
        phy_reg |= 0x8U;
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x17U, phy_reg);
#endif
        /* LED control - configure LED 0 as RX indicator and LED 1 as TX indicator */
        phy_reg =  MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1DU);
        phy_reg &= 0xFF00U;
        phy_reg |= 0x00ABU;
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1DU, phy_reg);

        phy_reg =  MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1EU);
        phy_reg |= 0x4000U;
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1EU, phy_reg);
#if 0
        /* Ethernet Packet Generator - enable at your peril... DO NOT try this on an open network */
        old_page = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU);

        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, 1U);

        phy_reg =  MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1DU);
        phy_reg |= 0xC000U;
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1DU, phy_reg);

        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, old_page);
#endif
    }
}
#endif /* MSS_MAC_USE_PHY_VSC8575 */


#if MSS_MAC_USE_PHY_VSC8575_LITE
extern int32_t usleep(uint32_t usecs);

void MSS_MAC_VSC8575_phy_init(/* mss_mac_instance_t*/ const void *v_this_mac, uint8_t phy_addr)
{
    static vsc_phy_loopback_t    loopback;
    static vsc_phy_control_t     cntrl;
    static vsc_phy_conf_t        phy_config;
    static vsc_phy_port_status_t phy_status;

    uint16_t old_page;
    uint16_t temp_reg;
    uint16_t phy_reg;
    volatile uint32_t sgmii_aneg_timeout = 100000U;
    uint16_t autoneg_complete;
    const mss_mac_instance_t *this_mac = (const mss_mac_instance_t *)v_this_mac;
    volatile int32_t rc; /* Volatile so we can see rc when debugging as
                          * otherwise it tends to get removed as we don't use
                          * it.
                          */

    (void)phy_addr;
    /* For now, simply record which MAC we are associated with, this is not
     * going to work with more than 1 VSC8575 in the system so we should really
     * have this stored in the cntrl structure but this involves modifying the
     * API code...
     */
    g_my_mac = this_mac;

    /* Start out blank for consistency */
    (void)memset (&cntrl, 0, sizeof(cntrl));
    (void)memset (&phy_config, 0, sizeof(phy_config));
    (void)memset (&phy_status, 0, sizeof(phy_status));
    (void)memset (&loopback, 0, sizeof(vsc_phy_loopback_t));

    phy_config.mode = VSC_PHY_MODE_ANEG;

    /* Forced Mode Config */
    phy_config.forced.port_speed = VSC_SPEED_1G;
    phy_config.forced.fdx = 1;

    /* ANEG Config */
    phy_config.aneg.speed_10m_hdx    = 1U;
    phy_config.aneg.speed_10m_fdx    = 1U;
    phy_config.aneg.speed_100m_hdx   = 1U;
    phy_config.aneg.speed_100m_fdx   = 1U;
    phy_config.aneg.speed_1g_hdx     = 0U;
    phy_config.aneg.speed_1g_fdx     = 1U;
    phy_config.aneg.symmetric_pause  = 1U;
    phy_config.aneg.asymmetric_pause = 0U;
    phy_config.aneg.tx_remote_fault  = 1U;

    /* forced Mode, 1G Config of Master/Slave */
    phy_config.conf_1g.master.cfg = 0U;  /* Manual Master/Slave config to Force Master cfg  */
    phy_config.conf_1g.master.val = 0U;  /* Master=1, Slave=0 */

    /* MDI/MDIX Config */
    phy_config.mdi = VSC_PHY_AUTO_MDIX;

    /* FastLink Fail Config */
    phy_config.flf.flf = VSC_PHY_FAST_LINK_FAIL_DISABLE;

    /* SigDet Config */
    phy_config.sigdet = VSC_PHY_SIGDET_POLARITY_ACT_HIGH;

    /* UniDirectional Config */
    phy_config.unidir = VSC_PHY_UNIDIR_DISABLE;

    /* Pkt Mode Config */
    phy_config.pkt_mode = VSC_PHY_PKT_MODE_IEEE_1_5_KB;

    /* MAC PCS Config */
    /* Note: MAC PCS Config only sets bits, does not clear bits */
    /*       Therefore, to use Chip Defaults, just use all 0's  */
    phy_config.mac_if_pcs.disable            = 0U;
    phy_config.mac_if_pcs.restart            = 0U;
    phy_config.mac_if_pcs.pd_enable          = 0U;
    phy_config.mac_if_pcs.aneg_restart       = 0U;
    phy_config.mac_if_pcs.force_adv_ability  = 0U;
    phy_config.mac_if_pcs.sgmii_in_pre       = 0U;
    phy_config.mac_if_pcs.sgmii_out_pre      = 0U;
    phy_config.mac_if_pcs.serdes_aneg_ena    = 1U;
    phy_config.mac_if_pcs.serdes_pol_inv_in  = 0U;
    phy_config.mac_if_pcs.serdes_pol_inv_out = 0U;
    phy_config.mac_if_pcs.fast_link_stat_ena = 0U;
    phy_config.mac_if_pcs.inhibit_odd_start  = 0U;

    /* Media PCS Config */
    /* Note: Media PCS Config only sets bits, does not clear bits */
    /*       Therefore, to use Chip Defaults, just use all 0's  */
    phy_config.media_if_pcs.remote_fault       = VSC_PHY_MEDIA_SERD_PCS_REM_FAULT_NO_ERROR;
    phy_config.media_if_pcs.aneg_pd_detect     = 1U;
    phy_config.media_if_pcs.force_adv_ability  = 0U;
    phy_config.media_if_pcs.serdes_pol_inv_in  = 0U;
    phy_config.media_if_pcs.serdes_pol_inv_out = 0U;
    phy_config.media_if_pcs.inhibit_odd_start  = 1U;
    phy_config.media_if_pcs.force_hls          = 0U;
    phy_config.media_if_pcs.force_fefi         = 0U;
    phy_config.media_if_pcs.force_fefi_value   = 0U;

    rc = viper_fmc_board_init(&cntrl);

    cntrl.phy_addr = 0U; /* This is actually the port number 0-3 on the VSC8575 */
    cntrl.phy_usleep = usleep;

    rc = vsc_get_phy_type(&cntrl);
    cntrl.mac_if    = PHY_MAC_IF_MODE_SGMII;
    cntrl.media_if  = PHY_MEDIA_IF_CU;

    /*------------------------------------------------------------------------------------------------
     * Initialize PHY, Only run on Base Port of the PHY, Run any Init Scripts and Micro-Patch Downloads
     *------------------------------------------------------------------------------------------------*/
    rc = initialize_viper_phy(&cntrl);

    /*------------------------------------------------------------------------------------------------
     * Reset the PHY Port, Run on each Port of the PHY, Run any configs, etc.
     *------------------------------------------------------------------------------------------------*/
    rc = reset_viper_phy(&cntrl);
    rc = viper_phy_config_set(&cntrl, &phy_config);

    /* The cntrl struct only saves the MEDIA i/f for the current port */
    /* Adjust the 1G SerDes SigDet Input Threshold and Signal Sensitivity for 100FX */
    /* This function gets called from Port 0, ie. cntrl->portAddr == BasePort
     * The tgt_port_no is the port_no that the operation is to occur upon.
     * The tgt_media_if is the tgt_port_no's Media i/f setting, which may or may
     * not be the same as Port 0, found in the cntl struct.
     */
#if 0 /* Not needed for our setup as we are Cu only... */
    cntrl.phy_addr = 0U;
    rc = viper_phy_media_sig_adjust(&cntrl, cntrl.media_if, 0);
#endif

    /*------------------------------------------------------------------------------------------------
     *  Post-Reset the PHY, Run on the Base Port of the PHY, This will release Comma Mode
     *------------------------------------------------------------------------------------------------*/
    /* Now Run Post Reset on PHY Port 0, Initialized the 6G SerDes */
    rc = post_reset_viper_phy(&cntrl);

    /*------------------------------------------------------------------------------------------------
     *  Manually tweak some settings in the PHY
     *------------------------------------------------------------------------------------------------*/
    old_page = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU);
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, 3U);
    temp_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x17U);
    temp_reg &= (uint16_t)(~0x0010U); /* Clear media inhibit odd start delay  bit */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x17U, temp_reg);

    temp_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x10U);
    temp_reg &= (uint16_t)(~0x0004U); /* Clear mac inhibit odd start delay  bit */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x10U, temp_reg);

    temp_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x10U);
    temp_reg &= (uint16_t)(~0x0100U); /* Turn off 2 byte preamble */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x10U, temp_reg);

    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, old_page);

    if(TBI == this_mac->interface_type)
    {
        phy_reg = (uint16_t)this_mac->mac_base->PCS_CONTROL;
        phy_reg |= 0x1000U;
        this_mac->mac_base->PCS_CONTROL = phy_reg;
        phy_reg |= 0x0200U;
        this_mac->mac_base->PCS_CONTROL = phy_reg;

        /* Wait for SGMII auto-negotiation to complete. */
        do {
            phy_reg = (uint16_t)this_mac->mac_base->PCS_STATUS;
            autoneg_complete = phy_reg & BMSR_AUTO_NEGOTIATION_COMPLETE;
            --sgmii_aneg_timeout;
        } while(((0U == autoneg_complete) && (0U != sgmii_aneg_timeout)) || (0xFFFFU == phy_reg));
    }

    if(GMII_SGMII == this_mac->interface_type)
    {
        /*
         * SGMII to GMII core with embedded MDIO interface for SGMII
         * link control.
         *
         * Reset SGMII core side of I/F.
         */
        phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR);
        phy_reg |= 0x9000U;    /* Reset and start autonegotiation */
        phy_reg &= 0xFBFFU;    /* Clear Isolate bit */
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR, phy_reg);

        phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR);
        phy_reg |= 0x1000U;    /* start autonegotiation */
        phy_reg &= 0xFBFFU;    /* Clear gmii Isolate bit */
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR, phy_reg);
    }
#if 0
    /* Far-end loopback - enable at your peril... DO NOT try this on an open network*/
    phy_reg =  MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x17U);
    phy_reg |= 0x8U;
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x17U, phy_reg);
#endif
    /* LED control - configure LED 0 as RX indicator and LED 1 as TX indicator */
    phy_reg =  MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1DU);
    phy_reg &= 0xFF00U;
    phy_reg |= 0x00ABU;
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1DU, phy_reg);

    phy_reg =  MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1EU);
    phy_reg |= 0x4000U;
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1EU, phy_reg);
#if 0
    /* Ethernet Packet Generator - enable at your peril... DO NOT try this on an open network */
    old_page = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU);

    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, 1U);

    phy_reg =  MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1DU);
    phy_reg |= 0xC000U;
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1DU, phy_reg);

    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, old_page);
#endif
}
#endif /* MSS_MAC_USE_PHY_VSC8575_LITE */


/**************************************************************************//**
 *
 */
void MSS_MAC_VSC8575_phy_set_link_speed(/* mss_mac_instance_t*/ const void *v_this_mac, uint32_t speed_duplex_select)
{
    const mss_mac_instance_t *this_mac = (const mss_mac_instance_t *)v_this_mac;
    uint16_t phy_reg;
    uint32_t inc;
    uint32_t speed_select;
    const uint16_t mii_advertise_bits[4] = {ADVERTISE_10FULL, ADVERTISE_10HALF,
                                            ADVERTISE_100FULL, ADVERTISE_100HALF};

    /* Set auto-negotiation advertisement. */

    /* Set 10Mbps and 100Mbps advertisement. */
    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_ADVERTISE);
    phy_reg &= (uint16_t)(~(ADVERTISE_10HALF | ADVERTISE_10FULL |
                 ADVERTISE_100HALF | ADVERTISE_100FULL));
    speed_select = speed_duplex_select;
    for(inc = 0U; inc < 4U; ++inc)
    {
        uint32_t advertise;
        advertise = speed_select & 0x00000001U;
        if(advertise != 0U)
        {
            phy_reg |= mii_advertise_bits[inc];
        }
        speed_select = speed_select >> 1;
    }

    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_ADVERTISE, phy_reg);
}


/**************************************************************************//**
 *
 */
void MSS_MAC_VSC8575_phy_autonegotiate(/* mss_mac_instance_t*/ const void *v_this_mac)
{
    const mss_mac_instance_t *this_mac = (const mss_mac_instance_t *)v_this_mac;
    uint8_t link_fullduplex;
    mss_mac_speed_t link_speed;
    uint8_t copper_link_up;
    volatile uint16_t phy_reg;
    uint16_t autoneg_complete;
    volatile uint32_t sgmii_aneg_timeout = 100000U;

    copper_link_up = this_mac->phy_get_link_status(this_mac, &link_speed, &link_fullduplex);
    if(1U == copper_link_up)
    {
#if 0 /* PMCS fixup */
        SYSREG->MAC_CR = (SYSREG->MAC_CR & ~MAC_CONFIG_SPEED_MASK) | link_speed;
        /* Configure duplex mode */
        if(0U == link_fullduplex)
        {
            /* half duplex */
            MAC->CFG2 &= ~CFG2_FDX_MASK;
        }
        else
        {
            /* full duplex */
            MAC->CFG2 |= CFG2_FDX_MASK;
        }
#endif
        if(TBI == this_mac->interface_type)
        {
            phy_reg = (uint16_t)this_mac->mac_base->PCS_CONTROL;
            phy_reg |= 0x1000U;
            this_mac->mac_base->PCS_CONTROL = phy_reg;
            phy_reg |= 0x0200U;
            this_mac->mac_base->PCS_CONTROL = phy_reg;

            /* Wait for SGMII auto-negotiation to complete. */
            do {
                phy_reg = (uint16_t)this_mac->mac_base->PCS_STATUS;
                autoneg_complete = phy_reg & BMSR_AUTO_NEGOTIATION_COMPLETE;
                --sgmii_aneg_timeout;
            } while(((0U == autoneg_complete) && (0U != sgmii_aneg_timeout)) || (0xFFFFU == phy_reg));
        }

        if(GMII_SGMII == this_mac->interface_type)
        {
            /*
             * SGMII to GMII core with embedded MDIO interface for SGMII
             * link control.
             *
             * Initiate auto-negotiation on the SGMII link.
             */
            phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, 0x00U);
            phy_reg |= 0x1000U;
            MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, 0x00U, phy_reg);
            phy_reg |= 0x0200U;
            MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, 0x00U, phy_reg);

            /* Wait for SGMII auto-negotiation to complete. */
            do {
                phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMSR);
                autoneg_complete = phy_reg & BMSR_AUTO_NEGOTIATION_COMPLETE;
                --sgmii_aneg_timeout;
            } while((0U == autoneg_complete) && (0U != sgmii_aneg_timeout));
        }
    }
}


/**************************************************************************//**
 *
 */

uint8_t MSS_MAC_VSC8575_phy_get_link_status
(
        /* mss_mac_instance_t*/ const void *v_this_mac,
    mss_mac_speed_t * speed,
    uint8_t *     fullduplex
)
{
    const mss_mac_instance_t *this_mac = (const mss_mac_instance_t *)v_this_mac;
    volatile uint16_t phy_reg;
    uint16_t copper_link_up;
    uint8_t link_status;

    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_BMSR);
    copper_link_up = phy_reg & BMSR_LSTATUS;

    if(copper_link_up != MSS_MAC_LINK_DOWN)
    {
        uint16_t op_mode;

        /* Link is up. */
        link_status = MSS_MAC_LINK_UP;

        phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1CU);

        op_mode = (phy_reg >> 3) & 0x0003U;
        if(0U == (phy_reg & 0x0020U))
        {
            *fullduplex = MSS_MAC_HALF_DUPLEX;
        }
        else
        {
            *fullduplex = MSS_MAC_FULL_DUPLEX;
        }

        switch(op_mode)
        {
            case 0U:
                *speed = MSS_MAC_10MBPS;
            break;

            case 1U:
                *speed = MSS_MAC_100MBPS;
            break;

            case 2U:
                *speed = MSS_MAC_1000MBPS;
            break;

            default:
                link_status = (uint8_t)MSS_MAC_LINK_DOWN;
            break;
        }
    }
    else
    {
        /* Link is down. */
        link_status = (uint8_t)MSS_MAC_LINK_DOWN;
    }

    return link_status;
}

#ifdef __cplusplus
}
#endif
/******************************** END OF FILE ******************************/


