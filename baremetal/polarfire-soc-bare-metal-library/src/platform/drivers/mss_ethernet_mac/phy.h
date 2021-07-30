/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Register bit definitions for MII STA (station management entity) standard
 * interface. All basic MII register bits and enhanced capability register bits
 * are defined.
 * Complies with Clauses 22, 28, 37, 40 of IEEE RFC 802.3
 *
 * SVN $Revision$
 * SVN $Date$
 */
#ifndef PSE_PHY_H
#define PSE_PHY_H
#include "../mss_mac/mss_ethernet_mac_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************/
/* Public definitions                                                     */
/**************************************************************************/
/*------------------------------------------------------------------------------
 * MII register definitions.
 */
/* Generic MII registers. */
#define MII_BMCR                (0X00U)    /* Basic mode control register */
#define MII_BMSR                (0X01U)    /* Basic mode status register  */
#define MII_PHYSID1             (0X02U)    /* PHYS ID 1                   */
#define MII_PHYSID2             (0X03U)    /* PHYS ID 2                   */
#define MII_ADVERTISE           (0X04U)    /* Advertisement control reg   */
#define MII_LPA                 (0X05U)    /* Link partner ability reg    */
#define MII_EXPANSION           (0X06U)    /* Expansion register          */
#define MII_NPAR                (0X07U)
#define MII_LPNPA               (0X08U)
#define MII_CTRL1000            (0X09U)    /* 1000BASE-T control          */
#define MII_STAT1000            (0X0AU)    /* 1000BASE-T status           */
#define MII_ESTATUS             (0X0FU)    /* Extended Status */
#define MII_DCOUNTER            (0X12U)    /* Disconnect counter          */
#define MII_FCSCOUNTER          (0X13U)    /* False carrier counter       */
#define MII_EXTEND              (0X14U)    /* extended PHY specific ctrl  */
#define MII_RERRCOUNTER         (0X15U)    /* Receive error counter       */
#define MII_SREVISION           (0X16U)    /* Silicon revision            */
#define MII_RESV1               (0X17U)    /* Reserved...                 */
#define MII_LBRERROR            (0X18U)    /* Lpback, rx, bypass error    */
#define MII_PHYADDR             (0X19U)    /* PHY address                 */
#define MII_RESV2               (0X1AU)    /* Reserved...                 */
#define MII_TPISTATUS           (0X1BU)    /* TPI status for 10mbps       */
#define MII_NCONFIG             (0X1CU)    /* Network interface config    */
#define MII_LMCS                (0X1DU)
#define MII_PHYCTRL1            (0X1EU)
#define MII_PHYCTRL2            (0X1FU)

#define MII_TI_REGCR            (0X0DU)
#define MII_TI_ADDAR            (0X0EU)
#define MII_TI_PHYCR            (0X10U)
#define MII_TI_CTRL             (0X1FU)
#define MII_TI_SGMIICTL1        (0XD3U)

/* Basic mode control register. */
#define BMCR_RESV               (0x003FU)   /* Unused...                   */
#define BMCR_SPEED1000          (0x0040U)   /* MSB of Speed (1000)         */
#define BMCR_CTST               (0x0080U)   /* Collision test              */
#define BMCR_FULLDPLX           (0x0100U)   /* Full duplex                 */
#define BMCR_ANRESTART          (0x0200U)   /* Auto negotiation restart    */
#define BMCR_ISOLATE            (0x0400U)   /* Disconnect DP83840 from MII */
#define BMCR_PDOWN              (0x0800U)   /* Powerdown the DP83840       */
#define BMCR_ANENABLE           (0x1000U)   /* Enable auto negotiation     */
#define BMCR_SPEED100           (0x2000U)   /* Select 100Mbps              */
#define BMCR_LOOPBACK           (0x4000U)   /* TXD loopback bits           */
#define BMCR_RESET              (0x8000U)   /* Reset the DP83840           */

/* Basic mode status register. */
#define BMSR_ERCAP              (0x0001U)   /* Ext-reg capability          */
#define BMSR_JCD                (0x0002U)   /* Jabber detected             */
#define BMSR_LSTATUS            (0x0004U)   /* Link status                 */
#define BMSR_ANEGCAPABLE        (0x0008U)   /* Able to do auto-negotiation */
#define BMSR_RFAULT             (0x0010U)   /* Remote fault detected       */
#define BMSR_ANEGCOMPLETE       (0x0020U)   /* Auto-negotiation complete   */
#define BMSR_RESV               (0x00c0U)   /* Unused...                   */
#define BMSR_ESTATEN            (0x0100U)   /* Extended Status in R15 */
#define BMSR_100HALF2           (0x0200U)   /* Can do 100BASE-T2 HDX */
#define BMSR_100FULL2           (0x0400U)   /* Can do 100BASE-T2 FDX */
#define BMSR_10HALF             (0x0800U)   /* Can do 10mbps, half-duplex  */
#define BMSR_10FULL             (0x1000U)   /* Can do 10mbps, full-duplex  */
#define BMSR_100HALF            (0x2000U)   /* Can do 100mbps, half-duplex */
#define BMSR_100FULL            (0x4000U)   /* Can do 100mbps, full-duplex */
#define BMSR_100BASE4           (0x8000U)   /* Can do 100mbps, 4k packets  */

/* Advertisement control register. */
#define ADVERTISE_SLCT          (0x001FU)   /* Selector bits               */
#define ADVERTISE_CSMA          (0x0001U)   /* Only selector supported     */
#define ADVERTISE_10HALF        (0x0020U)   /* Try for 10mbps half-duplex  */
#define ADVERTISE_1000XFULL     (0x0020U)   /* Try for 1000BASE-X full-duplex */
#define ADVERTISE_10FULL        (0x0040U)   /* Try for 10mbps full-duplex  */
#define ADVERTISE_1000XHALF     (0x0040U)   /* Try for 1000BASE-X half-duplex */
#define ADVERTISE_100HALF       (0x0080U)   /* Try for 100mbps half-duplex */
#define ADVERTISE_1000XPAUSE    (0x0080U)   /* Try for 1000BASE-X pause    */
#define ADVERTISE_100FULL       (0x0100U)   /* Try for 100mbps full-duplex */
#define ADVERTISE_1000XPSE_ASYM (0x0100U)   /* Try for 1000BASE-X asym pause */
#define ADVERTISE_100BASE4      (0x0200U)   /* Try for 100mbps 4k packets  */
#define ADVERTISE_PAUSE_CAP     (0x0400U)   /* Try for pause               */
#define ADVERTISE_PAUSE_ASYM    (0x0800U)   /* Try for asymetric pause     */
#define ADVERTISE_RESV          (0x1000U)   /* Unused...                   */
#define ADVERTISE_RFAULT        (0x2000U)   /* Say we can detect faults    */
#define ADVERTISE_LPACK         (0x4000U)   /* Ack link partners response  */
#define ADVERTISE_NPAGE         (0x8000U)   /* Next page bit               */

#define ADVERTISE_FULL (ADVERTISE_100FULL | ADVERTISE_10FULL | \
                        ADVERTISE_CSMA)
#define ADVERTISE_ALL (ADVERTISE_10HALF | ADVERTISE_10FULL | \
                       ADVERTISE_100HALF | ADVERTISE_100FULL)

/* Link partner ability register. */
#define LPA_SLCT                (0x001FU)   /* Same as advertise selector  */
#define LPA_10HALF              (0x0020U)   /* Can do 10mbps half-duplex   */
#define LPA_1000XFULL           (0x0020U)   /* Can do 1000BASE-X full-duplex */
#define LPA_10FULL              (0x0040U)   /* Can do 10mbps full-duplex   */
#define LPA_1000XHALF           (0x0040U)   /* Can do 1000BASE-X half-duplex */
#define LPA_100HALF             (0x0080U)   /* Can do 100mbps half-duplex  */
#define LPA_1000XPAUSE          (0x0080U)   /* Can do 1000BASE-X pause     */
#define LPA_100FULL             (0x0100U)   /* Can do 100mbps full-duplex  */
#define LPA_1000XPAUSE_ASYM     (0x0100U)   /* Can do 1000BASE-X pause asym*/
#define LPA_100BASE4            (0x0200U)   /* Can do 100mbps 4k packets   */
#define LPA_PAUSE_CAP           (0x0400U)   /* Can pause                   */
#define LPA_PAUSE_ASYM          (0x0800U)   /* Can pause asymetrically     */
#define LPA_RESV                (0x1000U)   /* Unused...                   */
#define LPA_RFAULT              (0x2000U)   /* Link partner faulted        */
#define LPA_LPACK               (0x4000U)   /* Link partner acked us       */
#define LPA_NPAGE               (0x8000U)   /* Next page bit               */

#define LPA_DUPLEX              (LPA_10FULL | LPA_100FULL)
#define LPA_100                 (LPA_100FULL | LPA_100HALF | LPA_100BASE4)

/* Expansion register for auto-negotiation. */
#define EXPANSION_NWAY          (0X0001U)   /* Can do N-way auto-nego      */
#define EXPANSION_LCWP          (0X0002U)   /* Got new RX page code word   */
#define EXPANSION_ENABLENPAGE   (0X0004U)   /* This enables npage words    */
#define EXPANSION_NPCAPABLE     (0X0008U)   /* Link partner supports npage */
#define EXPANSION_MFAULTS       (0X0010U)   /* Multiple faults detected    */
#define EXPANSION_RESV          (0XFFE0U)   /* Unused...                   */

#define ESTATUS_1000_TFULL      (0x2000U)   /* Can do 1000BT Full */
#define ESTATUS_1000_THALF      (0x1000U)   /* Can do 1000BT Half */

/* N-way test register. */
#define NWAYTEST_RESV1          (0X00FFU)   /* Unused...                   */
#define NWAYTEST_LOOPBACK       (0X0100U)   /* Enable loopback for N-way   */
#define NWAYTEST_RESV2          (0XFE00U)   /* Unused...                   */

/* 1000BASE-T Control register */
#define ADVERTISE_1000FULL      (0x0200U)   /* Advertise 1000BASE-T full duplex */
#define ADVERTISE_1000HALF      (0x0100U)   /* Advertise 1000BASE-T half duplex */

/* 1000BASE-T Status register */
#define LPA_1000LOCALRXOK       (0x2000U)   /* Link partner local receiver status */
#define LPA_1000REMRXOK         (0x1000U)   /* Link partner remote receiver status */
#define LPA_1000FULL            (0x0800U)   /* Link partner 1000BASE-T full duplex */
#define LPA_1000HALF            (0x0400U)   /* Link partner 1000BASE-T half duplex */

/* Indicates what features are supported by the interface. */
#define SUPPORTED_10baseT_Half          (1U << 0)
#define SUPPORTED_10baseT_Full          (1U << 1)
#define SUPPORTED_100baseT_Half         (1U << 2)
#define SUPPORTED_100baseT_Full         (1U << 3)
#define SUPPORTED_1000baseT_Half        (1U << 4)
#define SUPPORTED_1000baseT_Full        (1U << 5)
#define SUPPORTED_Autoneg               (1U << 6)
#define SUPPORTED_TP                    (1U << 7)
#define SUPPORTED_AUI                   (1U << 8)
#define SUPPORTED_MII                   (1U << 9)
#define SUPPORTED_FIBRE                 (1U << 10)
#define SUPPORTED_BNC                   (1U << 11)
#define SUPPORTED_10000baseT_Full       (1U << 12)
#define SUPPORTED_Pause                 (1U << 13)
#define SUPPORTED_Asym_Pause            (1U << 14)
#define SUPPORTED_2500baseX_Full        (1U << 15)
#define SUPPORTED_Backplane             (1U << 16)
#define SUPPORTED_1000baseKX_Full       (1U << 17)
#define SUPPORTED_10000baseKX4_Full     (1U << 18)
#define SUPPORTED_10000baseKR_Full      (1U << 19)
#define SUPPORTED_10000baseR_FEC        (1U << 20)

/* Indicates what features are advertised by the interface. */
#define ADVERTISED_10baseT_Half         (1U << 0)
#define ADVERTISED_10baseT_Full         (1U << 1)
#define ADVERTISED_100baseT_Half        (1U << 2)
#define ADVERTISED_100baseT_Full        (1U << 3)
#define ADVERTISED_1000baseT_Half       (1U << 4)
#define ADVERTISED_1000baseT_Full       (1U << 5)
#define ADVERTISED_Autoneg              (1U << 6)
#define ADVERTISED_TP                   (1U << 7)
#define ADVERTISED_AUI                  (1U << 8)
#define ADVERTISED_MII                  (1U << 9)
#define ADVERTISED_FIBRE                (1U << 10)
#define ADVERTISED_BNC                  (1U << 11)
#define ADVERTISED_10000baseT_Full      (1U << 12)
#define ADVERTISED_Pause                (1U << 13)
#define ADVERTISED_Asym_Pause           (1U << 14)
#define ADVERTISED_2500baseX_Full       (1U << 15)
#define ADVERTISED_Backplane            (1U << 16)
#define ADVERTISED_1000baseKX_Full      (1U << 17)
#define ADVERTISED_10000baseKX4_Full    (1U << 18)
#define ADVERTISED_10000baseKR_Full     (1U << 19)
#define ADVERTISED_10000baseR_FEC       (1U << 20)

/* TI DP83867 PHY Control Register */

#define PHYCR_TX_FIFO_DEPTH             (0xC000U)
#define PHYCR_RX_FIFO_DEPTH             (0x3000U)
#define PHYCR_SGMII_EN                  (0x0800U)
#define PHYCR_FORCE_LINK_GOOD           (0x0400U)
#define PHYCR_POWER_SAVE_MODE           (0x0300U)
#define PHYCR_DEEP_POWER_DOWN_EN        (0x0080U)
#define PHYCR_MDI_CROSSOVER             (0x0060U)
#define PHYCR_DISABLE_CLK_125           (0x0010U)
#define PHYCR_STANDBY_MODE              (0x0004U)
#define PHYCR_LINE_DRIVER_INV_EN        (0x0002U)
#define PHYCR_DISABLE_JABBER            (0x0001U)

/* TI DP83867 Control Register */

#define CTRL_SW_RESET                   (0x8000U)
#define CTRL_SW_RESTART                 (0x4000U)

/* TI DP83867 SGMII Control Register 1 */

#define SGMII_TYPE_6_WIRE               (0x4000U)

/* Different PHY MDIO addresses for our current designs */

#define PHY_VSC8541_MDIO_ADDR (0U)  /* Aloe board PHY */
#define PHY_VSC8575_MDIO_ADDR (4U)  /* G5 SoC Emulation Platform Peripheral Daughter Board PHY */
#define PHY_DP83867_MDIO_ADDR (3U)  /* G5 SoC Emulation Platform native PHY */
#define PHY_NULL_MDIO_ADDR    (0U)  /* No PHY here actually... */
#define SGMII_MDIO_ADDR       (16U) /* Internal PHY in G5 SoC Emulation Platform SGMII to GMII core */


/**************************************************************************/
/* Public function declarations                                           */
/**************************************************************************/

/***************************************************************************//**
  void MSS_MAC_phy_init(mss_mac_instance_t *this_mac, uint8_t phy_addr);
 */
#if MSS_MAC_USE_PHY_VSC8541
void MSS_MAC_VSC8541_phy_init(/* mss_mac_instance_t */ const void *v_this_mac, uint8_t phy_addr);
#endif

#if MSS_MAC_USE_PHY_VSC8575 || MSS_MAC_USE_PHY_VSC8575_LITE
void MSS_MAC_VSC8575_phy_init(/* mss_mac_instance_t */ const void *v_this_mac, uint8_t phy_addr);
#endif

#if MSS_MAC_USE_PHY_DP83867
void MSS_MAC_DP83867_phy_init(/* mss_mac_instance_t */ const void *v_this_mac, uint8_t phy_addr);
#endif

#if MSS_MAC_USE_PHY_NULL
void MSS_MAC_NULL_phy_init(/* mss_mac_instance_t */ const void *v_this_mac, uint8_t phy_addr);
#endif

/***************************************************************************//**

 */
#if MSS_MAC_USE_PHY_VSC8541
void MSS_MAC_VSC8541_phy_set_link_speed(/* mss_mac_instance_t */ const void *v_this_mac, uint32_t speed_duplex_select);
#endif

#if MSS_MAC_USE_PHY_VSC8575 || MSS_MAC_USE_PHY_VSC8575_LITE
void MSS_MAC_VSC8575_phy_set_link_speed(/* mss_mac_instance_t */ const void *v_this_mac, uint32_t speed_duplex_select);
#endif

#if MSS_MAC_USE_PHY_DP83867
void MSS_MAC_DP83867_phy_set_link_speed(/* mss_mac_instance_t */ const void *v_this_mac, uint32_t speed_duplex_select);
#endif

#if MSS_MAC_USE_PHY_NULL
void MSS_MAC_NULL_phy_set_link_speed(/* mss_mac_instance_t */ const void *v_this_mac, uint32_t speed_duplex_select);
#endif

/***************************************************************************//**

 */
#if MSS_MAC_USE_PHY_VSC8541
void MSS_MAC_VSC8541_phy_autonegotiate(/* mss_mac_instance_t */ const void *v_this_mac);
#endif

#if MSS_MAC_USE_PHY_VSC8575 || MSS_MAC_USE_PHY_VSC8575_LITE
void MSS_MAC_VSC8575_phy_autonegotiate(/* mss_mac_instance_t */ const void *v_this_mac);
#endif

#if MSS_MAC_USE_PHY_DP83867
void MSS_MAC_DP83867_phy_autonegotiate(/* mss_mac_instance_t */ const void *v_this_mac);
#endif

#if MSS_MAC_USE_PHY_NULL
void MSS_MAC_NULL_phy_autonegotiate(/* mss_mac_instance_t */ const void *v_this_mac);
#endif

/***************************************************************************//**

 */

#if MSS_MAC_USE_PHY_VSC8541
uint8_t MSS_MAC_VSC8541_phy_get_link_status
(
    /* mss_mac_instance_t */ const void *v_this_mac,
    mss_mac_speed_t * speed,
    uint8_t * fullduplex
);
#endif

#if MSS_MAC_USE_PHY_VSC8575 || MSS_MAC_USE_PHY_VSC8575_LITE
uint8_t MSS_MAC_VSC8575_phy_get_link_status
(
    /* mss_mac_instance_t */ const void *v_this_mac,
    mss_mac_speed_t * speed,
    uint8_t * fullduplex
);
#endif

#if MSS_MAC_USE_PHY_DP83867
uint8_t MSS_MAC_DP83867_phy_get_link_status
(
    /* mss_mac_instance_t */ const void *v_this_mac,
    mss_mac_speed_t * speed,
    uint8_t * fullduplex
);
#endif

#if MSS_MAC_USE_PHY_NULL
uint8_t MSS_MAC_NULL_phy_get_link_status
(
    /* mss_mac_instance_t */ const void *v_this_mac,
    mss_mac_speed_t * speed,
    uint8_t * fullduplex
);
#endif


#if MSS_MAC_USE_PHY_DP83867
/***************************************************************************//**

 */
void ti_write_extended_regs(/* mss_mac_instance_t */ const void *v_this_mac, uint16_t reg, uint16_t data);

#if MSS_MAC_USE_PHY_NULL
void NULL_ti_write_extended_regs(/* mss_mac_instance_t */ const void *v_this_mac, uint16_t reg, uint16_t data);
#endif

/***************************************************************************//**

 */
uint16_t ti_read_extended_regs(/* mss_mac_instance_t */ const void *v_this_mac, uint16_t reg);

#if MSS_MAC_USE_PHY_NULL
uint16_t NULL_ti_read_extended_regs(/* mss_mac_instance_t */ const void *v_this_mac, uint16_t reg);
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif /* PSE_PHY_H */


