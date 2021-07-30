
/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Microsemi VSC8541 PHY interface driver implementation to support the FU540
 * Aloe board.
 *
 * SVN $Revision$
 * SVN $Date$
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

#if MSS_MAC_USE_PHY_VSC8541

/**************************************************************************/
/* Preprocessor Macros                                                    */
/**************************************************************************/

#define BMSR_AUTO_NEGOTIATION_COMPLETE  (0x0020U)

/**************************************************************************//**
 *
 */

typedef struct
{
    __IO uint32_t  INPUT_VAL;   /* 0x0000 */
    __IO uint32_t  INPUT_EN;    /* 0x0004 */
    __IO uint32_t  OUTPUT_VAL;  /* 0x0008 */
    __IO uint32_t  OUTPUT_EN;   /* 0x000C */
    __IO uint32_t  PUE;         /* 0x0010 */
    __IO uint32_t  DS;          /* 0x0014 */
    __IO uint32_t  RISE_IE;     /* 0x0018 */
    __IO uint32_t  RISE_IP;     /* 0x001C */
    __IO uint32_t  FALL_IE;     /* 0x0020 */
    __IO uint32_t  FALL_IP;     /* 0x0024 */
    __IO uint32_t  HIGH_IE;     /* 0x0028 */
    __IO uint32_t  HIGH_IP;     /* 0x002C */
    __IO uint32_t  LOW_IE;      /* 0x0030 */
    __IO uint32_t  LOW_IP;      /* 0x0034 */
    __IO uint32_t  reserved0;   /* 0x0038 */
    __IO uint32_t  reserved1;   /* 0x003C */
    __IO uint32_t  OUT_XOR;     /* 0x0040 */
} AloeGPIO_TypeDef;


void MSS_MAC_VSC8541_phy_init(/* mss_mac_instance_t*/ const void *v_this_mac, uint8_t phy_addr)
{
#if defined(TARGET_ALOE)
    volatile uint32_t loop;
    AloeGPIO_TypeDef *g_aloe_gpio = (AloeGPIO_TypeDef *)0x10060000UL;

    (void)v_this_mac;
    (void)phy_addr;
/*
 * Init includes toggling the reset line which is connected to GPIO 0 pin 12.
 * This is the only pin I can see on the 16 GPIO which is currently set as an.
 * output. We will hard code the setup here to avoid having to have a GPIO
 * driver as well...
 *
 * The Aloe board is strapped for unmanaged mode and needs two pulses of the
 * reset line to configure the device properly.
 *
 * The RX_CLK, TX_CLK and RXD7 pins are strapped high and the remainder low.
 * This selects GMII mode with auto 10/100/1000 and 125MHz clkout.
 */
    g_aloe_gpio->OUTPUT_EN  |= 0x00001000UL;  /* Configure pin 12 as an output */
    g_aloe_gpio->OUTPUT_VAL &= 0x0000EFFFUL;  /* Clear pin 12 to reset PHY */
    for(loop = 0U; loop != 1000U; loop++)     /* Short delay, I'm not sure how much is needed... */
    {
        ;
    }
    g_aloe_gpio->OUTPUT_VAL  |= 0x00001000UL; /* Take PHY^ out of reset */
    for(loop = 0U; loop != 1000U; loop++)     /* Short delay, I'm not sure how much is needed... */
    {
        ;
    }
    g_aloe_gpio->OUTPUT_VAL &= 0x0000EFFFUL;  /* Second reset pulse */
    for(loop = 0U; loop != 1000U; loop++)     /* Short delay, I'm not sure how much is needed... */
    {
        ;
    }
    g_aloe_gpio->OUTPUT_VAL  |= 0x00001000UL; /* Out of reset once more */

    /* Need at least 15mS delay before accessing PHY after reset... */
    for(loop = 0U; loop != 10000000U; loop++)     /* Long delay, I'm not sure how much is needed... */
    {
        ;
    }
#endif
}

/**************************************************************************//**
 *
 */
void MSS_MAC_VSC8541_phy_set_link_speed(/* mss_mac_instance_t*/ const void *v_this_mac, uint32_t speed_duplex_select)
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
        speed_select = speed_select >> 1U;
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
void MSS_MAC_VSC8541_phy_autonegotiate(/* mss_mac_instance_t*/ const void *v_this_mac)
{
    const mss_mac_instance_t *this_mac = (const mss_mac_instance_t *)v_this_mac;
    volatile uint16_t phy_reg;
    uint16_t autoneg_complete;
    volatile uint32_t copper_aneg_timeout = 1000000U;

    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 2U);
    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 3U);

    /* Enable auto-negotiation. */
    phy_reg = 0x1340U;
    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_BMCR, phy_reg);

    /* Wait for copper auto-negotiation to complete. */
    do {
        phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, MII_BMSR);
        autoneg_complete = phy_reg & BMSR_AUTO_NEGOTIATION_COMPLETE;
        --copper_aneg_timeout;
    } while(((0U == autoneg_complete) && (copper_aneg_timeout != 0u)) || (0xFFFF == phy_reg));
}


/**************************************************************************//**
 *
 */
uint8_t MSS_MAC_VSC8541_phy_get_link_status
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

        phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1CU); /* Device Auxillary Control and Status */
        duplex = phy_reg & 0x0020U;
        speed_field = phy_reg & 0x0018U;

        if(MSS_MAC_HALF_DUPLEX == duplex)
        {
            *fullduplex = MSS_MAC_HALF_DUPLEX;
        }
        else
        {
            *fullduplex = MSS_MAC_FULL_DUPLEX;
        }

        switch(speed_field >> 3)
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
uint16_t VSC8541_reg_0[32];
uint16_t VSC8541_reg_1[16];
uint16_t VSC8541_reg_2[16];
uint16_t VSC8541_reg_16[32];

void dump_vsc8541_regs(const mss_mac_instance_t * this_mac);
void dump_vsc8541_regs(const mss_mac_instance_t * this_mac)
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
            pdata = VSC8541_reg_0;
        }
        else if(1U == page)
        {
            pdata = VSC8541_reg_1;
        }
        else if(2U == page)
        {
            pdata = VSC8541_reg_2;
        }
        else if(16U == page)
        {
            pdata = VSC8541_reg_16;
        }
        else
        {
            pdata = VSC8541_reg_0;
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

                pdata[count - 0X10] = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, (uint8_t)count);

                MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->phy_addr, 0x1FU, old_page);
                HAL_restore_interrupts(lev);
            }
        }

        if(2U == page)
        {
            page = 0x0FU;
        }
    }
}

#endif /* #if defined(TARGET_ALOE) */
#ifdef __cplusplus
}
#endif

/******************************** END OF FILE ******************************/






