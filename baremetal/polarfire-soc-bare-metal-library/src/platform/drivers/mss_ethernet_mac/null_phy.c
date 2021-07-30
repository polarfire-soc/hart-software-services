/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * NULL PHY implementation.
 *
 * This PHY interface is used when there is a direct connection between two GEM
 * instances which does not involve the use of a PHY device.
 *
 * Also used when setting up the default config so that the pointers for the
 * PHY functions in the config always have some valid values.
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

#if MSS_MAC_USE_PHY_NULL

/**************************************************************************//**
 *
 */


void MSS_MAC_NULL_phy_init(/* mss_mac_instance_t*/ const void *v_this_mac, uint8_t phy_addr)
{
    /* Nothing to see here... */
    (void)v_this_mac;
    (void)phy_addr;
}

/**************************************************************************//**
 *
 */
void MSS_MAC_NULL_phy_set_link_speed(/* mss_mac_instance_t*/ const void *v_this_mac, uint32_t speed_duplex_select)
{
    /* Nothing to see here... */
    (void)v_this_mac;
    (void)speed_duplex_select;
}

/**************************************************************************//**
 *
 */
void MSS_MAC_NULL_phy_autonegotiate(/* mss_mac_instance_t*/ const void *v_this_mac)
{
    /* Nothing to see here... */
    (void)v_this_mac;
}

/**************************************************************************//**
 *
 */
uint8_t MSS_MAC_NULL_phy_get_link_status
(
        /* mss_mac_instance_t*/ const void *v_this_mac,
    mss_mac_speed_t * speed,
    uint8_t *     fullduplex
)
{
    uint8_t link_status;

    (void)v_this_mac;
    /* Assume link is up. */
    link_status = MSS_MAC_LINK_UP;

    /* Pick fastest for now... */

    *fullduplex = MSS_MAC_FULL_DUPLEX;
    *speed = MSS_MAC_1000MBPS;

    return link_status;
}

#if MSS_MAC_USE_PHY_DP83867
/**************************************************************************//**
 *
 */
uint16_t NULL_ti_read_extended_regs(/* mss_mac_instance_t*/ const void *v_this_mac, uint16_t reg)
{
    (void)v_this_mac;
    (void)reg;

    return(0);
}

/**************************************************************************//**
 *
 */
void NULL_ti_write_extended_regs(/* mss_mac_instance_t*/ const void *v_this_mac, uint16_t reg, uint16_t data)
{
    (void)v_this_mac;
    (void)reg;
    (void)data;

    /* Nothing to see here... */
}
#endif

#endif /* MSS_MAC_USE_PHY_NULL */
#ifdef __cplusplus
}
#endif

/******************************** END OF FILE ******************************/






