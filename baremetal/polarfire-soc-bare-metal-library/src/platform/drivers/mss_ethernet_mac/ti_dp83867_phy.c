
/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * TI DP83867ISRGZ PHY interface driver implementation for use with the G5 SoC
 * Emulation Platform.
 *
 * This system uses the SGMII interface.
 *
 * The following are the default selections from the hardware strapping:
 *
 * RX_D0   - Strap option 4, PHY_ADD0 = 1, PHY_ADD1 = 1
 * RX_D2   - Strap option 0, PHY_ADD2 = 0, PHY_ADD1 = 0
 * RX_CTRL - Strap option 0, N/A - the following note applies:
 *           Strap modes 1 and 2 are not applicable for RX_CTRL. The RX_CTRL
 *           strap must be configured for strap mode 3 or strap mode 4. If the
 *           RX_CTRL pin cannot be strapped to mode 3 or mode 4, bit[7] of
 *           Configuration Register 4 (address 0x0031) must be cleared to 0.
 * GPIO_0  - Strap option 0, RGMII Clock Skew RX[0] = 0. GPIO_0 is connected to
 *           I/O pin AR22 on the VU9P FPGA device.
 * GPIO_1  - Strap option 0, RGMII Clock Skew RX[1] = 0,
 *           RGMII Clock Skew RX[2] = 0
 * LED_2   - Strap option 2, RGMII Clock Skew TX[0] = 1,
 *           RGMII Clock Skew TX[1] = 0
 * LED_1   - Strap option 2, RGMII Clock Skew TX[2] = 1, ANEG_SEL = 0
 * LED_0   - Strap option 2, SGMII_Enable = 1, Mirror Enable = 0
 *
 */
#include "mss_plic.h"
#include "fpga_design_config/fpga_design_config.h"

#include "drivers/mss_mac/mss_ethernet_registers.h"
#include "drivers/mss_mac/mss_ethernet_mac_regs.h"
#include "drivers/mss_mac/mss_ethernet_mac_user_config.h"
#include "drivers/mss_mac/mss_ethernet_mac.h"
#include "drivers/mss_mac/phy.h"
#include "hal/hal.h"
#include "mss_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

#if MSS_MAC_USE_PHY_DP83867

/**************************************************************************/
/* Preprocessor Macros                                                    */
/**************************************************************************/

#define BMSR_AUTO_NEGOTIATION_COMPLETE  (0x0020U)

/**************************************************************************//**
 *
 */
void MSS_MAC_DP83867_phy_init(/* mss_mac_instance_t*/ const void *v_this_mac, uint8_t phy_addr)
{
    const mss_mac_instance_t *this_mac = (const mss_mac_instance_t *)v_this_mac;
    uint16_t phy_reg;
    (void)phy_addr;

    /* Start by doing a software reset of the PHY */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_CTRL, CTRL_SW_RESET);

    /* Enable SGMII interface */
    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_PHYCR);
    phy_reg |= PHYCR_SGMII_EN;
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_PHYCR, phy_reg);

    /* Enable 6 wire SGMII so that the 625MHz clock to the SGMII core is active */
#if 0
    ti_write_extended_regs(this_mac, MII_TI_SGMIICTL1, SGMII_TYPE_6_WIRE);
#endif

    if(GMII_SGMII == this_mac->interface_type)
    {
        /* Reset SGMII core side of I/F. */
        phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR);
        phy_reg |= 0x9000U;    /* Reset and start autonegotiation */
        phy_reg &= 0xFBFFU;    /* Clear Isolate bit */
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR, phy_reg);

        phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR);
        phy_reg &= 0xFBFFU;    /* Clear Isolate bit */
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR, phy_reg);

        phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR);
        phy_reg |= 0x1000U;    /* Kick off autonegotiation - belt and braces approach...*/
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR, phy_reg);
    }
}


/**************************************************************************//**
 *
 */
void MSS_MAC_DP83867_phy_set_link_speed(/* mss_mac_instance_t*/ const void *v_this_mac, uint32_t speed_duplex_select)
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

    phy_reg |= 0x0C00U; /* Set Asymmetric pause and symmetric pause bits */

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

    /* Set 1000Mbps advertisement. */
    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_CTRL1000);
    phy_reg &= (uint16_t)(~(ADVERTISE_1000FULL | ADVERTISE_1000HALF));

    if((speed_duplex_select & MSS_MAC_ANEG_1000M_FD) != 0U)
    {
        phy_reg |= ADVERTISE_1000FULL;
    }

    if((speed_duplex_select & MSS_MAC_ANEG_1000M_HD) != 0U)
    {
        phy_reg |= ADVERTISE_1000HALF;
    }

    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_CTRL1000, phy_reg);
}


/**************************************************************************//**
 *
 */
void MSS_MAC_DP83867_phy_autonegotiate(/* mss_mac_instance_t*/ const void *v_this_mac)
{
    const mss_mac_instance_t *this_mac = (const mss_mac_instance_t *)v_this_mac;
    volatile uint16_t phy_reg;
    uint16_t autoneg_complete;
    volatile uint32_t copper_aneg_timeout = 1000000U;
    volatile uint32_t sgmii_aneg_timeout  = 100000U;

    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 2);
    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 3);

    /* Enable auto-negotiation. */
    phy_reg = 0x1340U;
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_BMCR, phy_reg);

    /* Wait for copper auto-negotiation to complete. */
    do {
        phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_BMSR);
        autoneg_complete = phy_reg & BMSR_AUTO_NEGOTIATION_COMPLETE;
        --copper_aneg_timeout;
    } while((0u == autoneg_complete) && (0U != copper_aneg_timeout) && (0xFFFFU != phy_reg));

    if(GMII_SGMII == this_mac->interface_type)
    {
        /* Initiate auto-negotiation on the SGMII link. */
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


/**************************************************************************//**
 *
 */
uint8_t MSS_MAC_DP83867_phy_get_link_status
(
        /* mss_mac_instance_t*/ const void *v_this_mac,
    mss_mac_speed_t * speed,
    uint8_t *     fullduplex
)
{
    const mss_mac_instance_t *this_mac = (const mss_mac_instance_t *)v_this_mac;
    uint16_t phy_reg;
    uint16_t link_up;
    uint8_t link_status;

    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_BMSR);
    link_up = phy_reg & BMSR_LSTATUS;

    if(link_up != MSS_MAC_LINK_DOWN)
    {
        uint16_t duplex;
        uint16_t speed_field;

        /* Link is up. */
        link_status = MSS_MAC_LINK_UP;

        phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x11U); /* Device Auxillary Control and Status */
        duplex = phy_reg & 0x2000U;
        speed_field = phy_reg >> 14;

        if(MSS_MAC_HALF_DUPLEX == duplex)
        {
            *fullduplex = MSS_MAC_HALF_DUPLEX;
        }
        else
        {
            *fullduplex = MSS_MAC_FULL_DUPLEX;
        }

        switch(speed_field)
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


/**************************************************************************//**
 *
 */

/*
 * TI DP83867 PHY has 32 standard registers and a collection of additional
 * registers that are accessed through the use of registers 0x0D and 0x0E.
 * Register 0x0D (REGCR) holds the control bits which determine what register
 * 0x0E (ADDAR) does.
 *
 * The following extended registers are available:
 *
 * 0x0025 - Testmode Channel Control
 * 0x002D - Fast Link Drop Configuration
 * 0x0031 - Configuration Register 4
 * 0x0032 - RGMII Control Register
 * 0x0033 - RGMII Control Register 2
 * 0x0037 - SGMII Auto-Negotiation Status
 * 0x0043 - 100BASE-TX Configuration
 * 0x0055 - Skew FIFO Status
 * 0x006E - Strap Configuration Status Register 1
 * 0x006F - Strap Configuration Status Register 2
 * 0x0071 - BIST Control and Status Register 1
 * 0x0072 - BIST Control and Status Register 2
 * 0x0086 - RGMII Delay Control Register
 * 0x00D3 - SGMII Control Register 1
 * 0x00E9 - Sync FIFO Control
 * 0x00FE - Loopback Configuration Register
 * 0x0134 - Receive Configuration Register
 * 0x0135 - Receive Status Register
 * 0x0136 - Pattern Match Data Register 1
 * 0x0137 - Pattern Match Data Register 2
 * 0x0138 - Pattern Match Data Register 3
 * 0x0139 - SecureOn Pass Register 1
 * 0x013A - SecureOn Pass Register 2
 * 0x013B - SecureOn Pass Register 3
 * 0x013C - 0x015B - Receive Pattern Registers 1 to 32
 * 0x015C - Receive Pattern Byte Mask Register 1
 * 0x015D - Receive Pattern Byte Mask Register 2
 * 0x015E - Receive Pattern Byte Mask Register 3
 * 0x015F - Receive Pattern Byte Mask Register 4
 * 0x0161 - Receive Pattern Control
 * 0x016F - 10M SGMII Configuration
 * 0x0170 - I/O configuration
 * 0x0172 - GPIO Mux Control Register
 * 0x0180 - TDR General Configuration Register 1
 * 0x01A7 - Advanced Link Cable Diagnostics Control Register
 * 0x0000 - MMD3 PCS Control Register - indirect addressing only
 *
 */

uint16_t phy_reg_list[25] =
{
    0x0025U, /* 00 Testmode Channel Control */
    0x002DU, /* 01 Fast Link Drop Configuration */
    0x0031U, /* 02 Configuration Register 4 */
    0x0032U, /* 03 RGMII Control Register */
    0x0033U, /* 04 RGMII Control Register 2 */
    0x0037U, /* 05 SGMII Auto-Negotiation Status */
    0x0043U, /* 06 100BASE-TX Configuration */
    0x0055U, /* 07 Skew FIFO Status */
    0x006EU, /* 08 Strap Configuration Status Register 1 */
    0x006FU, /* 09 Strap Configuration Status Register 2 */
    0x0071U, /* 10 BIST Control and Status Register 1 */
    0x0072U, /* 11 BIST Control and Status Register 2 */
    0x0086U, /* 12 RGMII Delay Control Register */
    0x00D3U, /* 13 SGMII Control Register 1 */
    0x00E9U, /* 14 Sync FIFO Control */
    0x00FEU, /* 15 Loopback Configuration Register */
    0x0134U, /* 16 Receive Configuration Register */
    0x0135U, /* 17 Receive Status Register */
    0x016FU, /* 18 10M SGMII Configuration */
    0x0170U, /* 19 I/O configuration */
    0x0172U, /* 20 GPIO Mux Control Register */
    0x0180U, /* 21 TDR General Configuration Register 1 */
    0x01A7U, /* 22 Advanced Link Cable Diagnostics Control Register */
    0x0000U, /* 23 MMD3 PCS Control Register - indirect addressing only */
    0xFFFFU  /* 24 End of list... */
};

uint16_t TI_reg_0[32];
uint16_t TI_reg_1[25];
uint16_t TI_MSS_SGMII_reg[17];
uint32_t TI_MSS_MAC_reg[80];
uint32_t TI_MSS_PLIC_REG[80];


void dump_ti_regs(const mss_mac_instance_t * this_mac);
void dump_ti_regs(const mss_mac_instance_t * this_mac)
{
    int32_t count;
    uint16_t old_ctrl;
    uint16_t old_addr;
    uint32_t *pbigdata;
    volatile psr_t lev;

    for(count = 0; count <= 0x1F; count++)
    {
        lev = HAL_disable_interrupts();
        TI_reg_0[count] = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, (uint8_t)count);
        HAL_restore_interrupts(lev);
    }

    lev = HAL_disable_interrupts();
    old_ctrl = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR);  /* Fetch current REGCR value */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, 0x001FU);    /* Select Address mode */
    old_addr = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_ADDAR);  /* Fetch current indirect address */
    HAL_restore_interrupts(lev);

    for(count = 0; 0xFFFFU != phy_reg_list[count]; count++)
    {
        lev = HAL_disable_interrupts();
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, 0x001FU);              /* Select Address mode */
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_ADDAR, phy_reg_list[count]); /* Select new indirect Address */
        MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, 0x401FU);              /* Select simple data mode */
        TI_reg_1[count] = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_ADDAR);     /* Finally, read the data */
        HAL_restore_interrupts(lev);
    }

    lev = HAL_disable_interrupts();
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, 0x001FU);    /* Select Address mode */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_ADDAR, old_addr);  /* Restore old address */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, old_ctrl);  /* Restore old control mode */
    HAL_restore_interrupts(lev);

    for(count = 0; count <= 0x10; count++)
    {
        TI_MSS_SGMII_reg[count] = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, (uint8_t)count);
    }

    pbigdata = (uint32_t *)0x20110000UL;
    for(count = 0; count < 19; count++)
    {
        TI_MSS_MAC_reg[count] = *pbigdata;
        pbigdata++;
    }
    pbigdata =(uint32_t *)0x0C000000UL;
    for(count = 0; count < 8; count++)
    {
        TI_MSS_PLIC_REG[count] = *pbigdata;
        pbigdata++;
    }
}


/**************************************************************************//**
 *
 */
uint16_t ti_read_extended_regs(/* mss_mac_instance_t*/ const void *v_this_mac, uint16_t reg)
{
    const mss_mac_instance_t *this_mac = (const mss_mac_instance_t *)v_this_mac;
    uint16_t old_ctrl;
    uint16_t old_addr;
    uint16_t ret_val = 0U;
    volatile psr_t lev;

    lev = HAL_disable_interrupts();
    old_ctrl = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR);  /* Fetch current REGCR value */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, 0x001FU);    /* Select Address mode */
    old_addr = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_ADDAR);  /* Fetch current indirect address */

    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, 0x001FU);    /* Select Address mode */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_ADDAR, reg);       /* Select new indirect Address */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, 0x401FU);    /* Select simple data mode */
    ret_val = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_ADDAR);   /* Finally, read the data */

    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, 0x001FU);    /* Select Address mode */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_ADDAR, old_addr);  /* Restore old address */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, old_ctrl);  /* Restore old control mode */
    HAL_restore_interrupts(lev);

    return(ret_val);
}


/**************************************************************************//**
 *
 */
void ti_write_extended_regs(/* mss_mac_instance_t*/ const void *v_this_mac, uint16_t reg, uint16_t data)
{
    const mss_mac_instance_t *this_mac = (const mss_mac_instance_t *)v_this_mac;
    uint16_t old_ctrl;
    uint16_t old_addr;
    volatile psr_t lev;

    lev = HAL_disable_interrupts();
    old_ctrl = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR);  /* Fetch current REGCR value */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, 0x001FU);    /* Select Address mode */
    old_addr = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_ADDAR);  /* Fetch current indirect address */

    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, 0x001FU);    /* Select Address mode */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_ADDAR, reg);       /* Select new indirect Address */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, 0x401FU);    /* Select simple data mode */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_ADDAR, data);      /* Now write the data */

    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, 0x001FU);    /* Select Address mode */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_ADDAR, old_addr);  /* Restore old address */
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_TI_REGCR, old_ctrl);  /* Restore old control mode */
    HAL_restore_interrupts(lev);
}
#endif /* #if defined(TARGET_ALOE) */
#ifdef __cplusplus
}
#endif

/******************************** END OF FILE ******************************/






