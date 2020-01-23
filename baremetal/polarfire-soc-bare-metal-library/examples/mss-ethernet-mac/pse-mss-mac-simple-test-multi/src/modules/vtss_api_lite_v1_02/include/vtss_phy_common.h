/* ******************************************************************************
* Copyright © 2015 Microsemi Corporation                                        *
* Permission is hereby granted, free of charge, to any person obtaining a copy  *
* of this software and associated documentation files (the "Software"), to deal *
* in the Software without restriction, including without limitation the rights  *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell     *
* copies of the Software, and to permit persons to whom the Software is         *
* furnished to do so, subject to the following conditions:                      *
* The above copyright notice and this permission notice shall be included in    *
* all copies or substantial portions of the Software.                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,      *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE   *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER        *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN     *
* THE SOFTWARE.                                                                 *
******************************************************************************* */

#ifndef _VSC_PHY_COMMON_H_
#define _VSC_PHY_COMMON_H_

#define VSC_API_LITE_VERSION  "1.2.0.0"

#define CHK_RC(x) { int32_t __rcode__ = (x); if (__rcode__ != 0) return __rcode__; }  /**< 0 == Ok; Anthing Else is a Fail */
#define VSC_PHY_MICRO_TIMEOUT        (500)  /*- Max Micro Timeout value */
#define VSC_PHY_CSR_BUSY_TIMEOUT     (255)  /*- Max Value 255 */

#define VSC_PHY_DEBUG   "VSC_PHY:"
#define VSC_8504_PHY    "VSC8504_PHY:"
#define VSC_8552_PHY    "VSC8552_PHY:"
#define VSC_8572_PHY    "VSC8572_PHY:"
#define VSC_8574_PHY    "VSC8574_PHY:"
#define VSC_8575_PHY    "VSC8575_PHY:"
#define VSC_8564_PHY    "VSC8564_PHY:"
#define VSC_8582_PHY    "VSC8582_PHY:"
#define VSC_8584_PHY    "VSC8584_PHY:"
#define VSC_8514_PHY    "VSC8514_PHY:"
#define VSC_8502_PHY    "VSC8502_PHY:"
#define VSC_8530_PHY    "VSC8530_PHY:"
#define VSC_8531_PHY    "VSC8531_PHY:"
#define VSC_8540_PHY    "VSC8540_PHY:"
#define VSC_8541_PHY    "VSC8541_PHY:"

#define VSC_PHY_STD_PAGE     (0x0)
#define VSC_PHY_EXT1_PAGE    (0x1)
#define VSC_PHY_EXT2_PAGE    (0x2)
#define VSC_PHY_EXT3_PAGE    (0x3)
#define VSC_PHY_EXT4_PAGE    (0x4)
#define VSC_PHY_GPIO_PAGE    (0x10)
#define VSC_PHY_TEST_PAGE    (0x2A30)
#define VSC_PHY_TR_PAGE      (0x52B5)

#define VSC_PHY_TRUE         (1)
#define VSC_PHY_FALSE        (0)

#define VSC_PHY_OK           (0)
#define VSC_PHY_ERROR        (-1)
#define VSC_PHY_INVALID_REQ  (-2)
#define VSC_PHY_TIMEOUT      (-3)
#define VSC_PHY_CSR_TIMEOUT  (-4)
#define VSC_PHY_SD6G_TIMEOUT (-5)
#define VSC_PHY_CRC_ERROR    (-6)
#define VSC_PHY_ERR_RECOV_CLK_CONF_NOT_SUPPORTED (-7)

/**< Tesla PHY Family Revisions  */
#define VSC_PHY_TESLA_REV_A 0
#define VSC_PHY_TESLA_REV_B 1
#define VSC_PHY_TESLA_REV_D 2

/**< Viper PHY Family Revisions  */
#define VSC_PHY_VIPER_REV_A 0
#define VSC_PHY_VIPER_REV_B 1

/**< Elise PHY Family Revisions  */
#define VSC_PHY_ELISE_REV_A 0

/**< Nano PHY Family Revisions  */
#define VSC_PHY_NANO_REV_A  1

/**< Mini PHY Family Revisions  */
#define VSC_PHY_MINI_REV_A  0


/** \brief PHY Family Identifiers */
typedef enum {
    VSC_PHY_FAMILY_NONE,     /**<  Unknown */
    VSC_PHY_FAMILY_TESLA,    /**<  VSC8552, VSC8572, VSC8504, VSC8574*/
    VSC_PHY_FAMILY_VIPER,    /**<  VSC8582, VSC8584, VSC8575, VSC8564, VSC8586*/
    VSC_PHY_FAMILY_ELISE,    /**<  VSC8514*/
    VSC_PHY_FAMILY_NANO,     /**<  VSC8502, VSC8501 */
    VSC_PHY_FAMILY_MINI,     /**<  VSC8541, VSC8540, VSC8531, VSC8530 */
} vsc_phy_family_t;

/** \brief PHY Part/Type Identifiers */
typedef enum {
    VSC_PHY_TYPE_NONE = 0,    /**<  Unknown */
    VSC_PHY_TYPE_8504 = 8504, /**<  VSC8504 (Tesla) */
    VSC_PHY_TYPE_8552 = 8552, /**<  VSC8552 (Tesla) */
    VSC_PHY_TYPE_8572 = 8572, /**<  VSC8572 (Tesla) */
    VSC_PHY_TYPE_8574 = 8574, /**<  VSC8574 (Tesla) */
    VSC_PHY_TYPE_8501 = 8501, /**<  VSC8502 (Nano) */
    VSC_PHY_TYPE_8502 = 8502, /**<  VSC8501 (Nano) */
    VSC_PHY_TYPE_8530 = 8530, /**<  VSC8530 (Mini) */
    VSC_PHY_TYPE_8531 = 8531, /**<  VSC8531 (Mini) */
    VSC_PHY_TYPE_8540 = 8540, /**<  VSC8540 (Mini) */
    VSC_PHY_TYPE_8541 = 8541, /**<  VSC8541 (Mini) */
    VSC_PHY_TYPE_8582 = 8582, /**<  VSC8582 (Viper) */
    VSC_PHY_TYPE_8584 = 8584, /**<  VSC8584 (Viper) */
    VSC_PHY_TYPE_8575 = 8575, /**<  VSC8575 (Viper) */
    VSC_PHY_TYPE_8564 = 8564, /**<  VSC8564 (Viper) */
    VSC_PHY_TYPE_8586 = 8586, /**<  VSC8586 (Viper) */
    VSC_PHY_TYPE_8514 = 8514, /**<  VSC8514 (Elise) */
} vsc_phy_part_number_t;


/** \brief PHY Identifier */
typedef struct phy_id_t {
    uint32_t                 oui;
    uint16_t                 model;
    uint8_t                  revision;
    uint8_t                  port_cnt;
    vsc_phy_family_t         family;
    vsc_phy_part_number_t    part_number;
} vsc_phy_id_t;


/** \brief PHY MAC Interface Definitions */
typedef enum
{
    PHY_MAC_IF_MODE_NO_CONNECTION, /**< No connection */
    PHY_MAC_IF_MODE_LOOPBACK,      /**< Internal loopback in MAC */
    PHY_MAC_IF_MODE_INTERNAL,      /**< Internal interface */
    PHY_MAC_IF_MODE_MII,           /**< MII (RMII does not exist) */
    PHY_MAC_IF_MODE_GMII,          /**< GMII */
    PHY_MAC_IF_MODE_RMII,          /**< RMII */
    PHY_MAC_IF_MODE_RGMII,         /**< RGMII */
    PHY_MAC_IF_MODE_TBI,           /**< TBI */
    PHY_MAC_IF_MODE_RTBI,          /**< RTBI */
    PHY_MAC_IF_MODE_SGMII,         /**< SGMII */
    PHY_MAC_IF_MODE_SERDES,        /**< SERDES */
    PHY_MAC_IF_MODE_VAUI,          /**< VAUI */
    PHY_MAC_IF_MODE_100FX,         /**< 100FX */
    PHY_MAC_IF_MODE_XAUI,          /**< XAUI */
    PHY_MAC_IF_MODE_RXAUI,         /**< RXAUI */
    PHY_MAC_IF_MODE_XGMII,         /**< XGMII */
    PHY_MAC_IF_MODE_SPI4,          /**< SPI4 */
    PHY_MAC_IF_MODE_QSGMII,        /**< QSGMII */
    PHY_MAC_IF_MODE_SFI,           /**< SFI/LAN */
    PHY_MAC_IF_MODE_MAX,
} vsc_phy_mac_interface_t;

/** \brief PHY MEDIA Interface Definitions */
typedef enum {
    PHY_MEDIA_IF_NONE,                    /**< No Interface */
    PHY_MEDIA_IF_CU,                      /**< Copper Interface */
    PHY_MEDIA_IF_SFP_PASSTHRU,            /**< Fiber/Cu SFP Pass-thru mode */
    PHY_MEDIA_IF_FI_1000BX,               /**< Fiber 1000Base-X */
    PHY_MEDIA_IF_FI_100FX,                /**< Fiber 100Base-FX */
    PHY_MEDIA_IF_AMS_CU_PASSTHRU,         /**< AMS - Cat5/SerDes/Cu Preferred passthru - Note the phy mode must be set to VSC_PHY_MODE_ANEG */
    PHY_MEDIA_IF_AMS_FI_PASSTHRU,         /**< AMS - Fiber passthru - Note the phy mode must be set to VSC_PHY_MODE_ANEG */
    PHY_MEDIA_IF_AMS_CU_1000BX,           /**< AMS - Cat5/1000BX/Cu preferred */
    PHY_MEDIA_IF_AMS_FI_1000BX,           /**< AMS - Fiber/1000BX/Fiber preferred  */
    PHY_MEDIA_IF_AMS_CU_100FX,            /**< AMS - Cat5/100FX/Cu preferred */
    PHY_MEDIA_IF_AMS_FI_100FX             /**< AMS - Fiber/100FX/Fiber preferred  */
} vsc_phy_media_interface_t;

/** \brief PHY Control Structure */
/**< The following structure provides the access for all PHY functions */
/**< The User must define the MDIO Read/Write Functions */
/**< The User must define the System uSleep Function */
/**< The User must define the MAC i/f for this particular Port of the PHY, Note: QSGMII is only configured for Port 0 of the PHY */
/**< The User must define the MEDIA i/f for this particular Port of the PHY */
/**< The phy_addr is only use by the registered MDIO Read/Write functions, so it can be a port_no of the PHY or an Address, etc.  */
typedef struct phy_control_t {
    uint32_t                   phy_addr;   /**< Address of the PHY Port being passed in, to be passed through and used by the registered MDIO functions to access PHY */
    uint32_t                   user_addr;  /**< Additional User Sub-Address of Port being passed, Passed through to MDIO Functions */
    vsc_phy_id_t               phy_id;     /**< PHY Type information, Model, Rev of Part, Product Number, etc. */
    vsc_phy_mac_interface_t    mac_if;     /**< Type of MAC Interface for this PHY Port */
    vsc_phy_media_interface_t  media_if;   /**< Type of Media Interface for this PHY Port */

    int32_t (* phy_usleep)(uint32_t usecs);    /**< Usec Sleep Timer for the given OS */

    /**< MDIO Read/Write Functions */
    int32_t (* phy_reg_read)(uint32_t addr, uint16_t reg, uint16_t *val);  /**< Register the MDIO Read Function */
    int32_t (* phy_reg_write)(uint32_t addr, uint16_t reg, uint16_t val);  /**< Register the MDIO Write Function */
} vsc_phy_control_t;


/** \brief PHY MODE Structure, used in PHY Config  */
typedef enum {
    VSC_PHY_MODE_ANEG,       /**< Auto negoatiation */
    VSC_PHY_MODE_FORCED,     /**< Forced mode */
    VSC_PHY_MODE_POWER_DOWN  /**< Power down (disabled) */
} vsc_phy_mode_t;

/** \brief PHY Port Speed, used in PHY Config and Status */
typedef enum
{
    VSC_SPEED_UNDEFINED,   /**< Undefined */
    VSC_SPEED_10M,         /**< 10 M */
    VSC_SPEED_100M,        /**< 100 M */
    VSC_SPEED_1G,          /**< 1 G */
    VSC_SPEED_2500M,       /**< 2.5G */
    VSC_SPEED_5G,          /**< 5G or 2x2.5G */
    VSC_SPEED_10G          /**< 10 G */
} vsc_port_speed_t;

/** \brief PHY Forced Mode Structure, used in PHY Config  */
typedef struct {
    vsc_port_speed_t  port_speed; /**< Speed */
    uint8_t           fdx;        /**< Full duplex=1, Half duplex=0 */
} vsc_phy_forced_t;

/** \brief PHY ANEG Advertisement Structure, used in PHY Config  */
typedef struct {
    uint8_t speed_10m_hdx;    /**< 10Mbps, half duplex */
    uint8_t speed_10m_fdx;    /**< 10Mbps, full duplex */
    uint8_t speed_100m_hdx;   /**< 100Mbps, half duplex */
    uint8_t speed_100m_fdx;   /**< 100Mbps, full duplex */
    uint8_t speed_1g_fdx;     /**< 1000Mpbs, full duplex */
    uint8_t speed_1g_hdx;     /**< 1000Mpbs, full duplex */
    uint8_t symmetric_pause;  /**< Symmetric pause */
    uint8_t asymmetric_pause; /**< Asymmetric pause */
    uint8_t tx_remote_fault;  /**< Local Application fault indication for Link Partner */
} vsc_phy_auto_neg_t;

/** \brief PHY MDI-MDIX Definitions, used in PHY Config  */
typedef enum {
    VSC_PHY_AUTO_MDIX,       /**< Copper media MDI auto detected */
    VSC_PHY_MDI,             /**< Copper media forced to MDI */
    VSC_PHY_MDIX,            /**< Copper media forced to MDI-X (Crossed cable) */
} vsc_phy_mdi_t;

/** \brief PHY FLF and FLF2 Definitions, used in PHY Config  */
typedef enum {
    VSC_PHY_FAST_LINK_FAIL_ENABLE = 1,    /**< Enable fast link failure pin */
    VSC_PHY_FAST_LINK_FAIL_DISABLE,       /**< Disble fast link failure pin */
} vsc_phy_fast_link_fail_t;

/** \brief PHY FLF and FLF2 configuration */
typedef struct {
    uint8_t  port;                        /**< PHY Port, This is for GPIO, so config must be done on Port 0 to set other ports, For Quad PHY Values are 0-3 */
    vsc_phy_fast_link_fail_t   flf;       /**< Fast-Link Fail Enable/Disable                      */
    vsc_phy_fast_link_fail_t   flf_2;     /**< Fast-Link Fail2 Enable/Disable, If supported by HW */
} vsc_phy_conf_flf_t;

/** \brief PHY packet mode configuration */
typedef enum {
    VSC_PHY_PKT_MODE_IEEE_1_5_KB = 0,  /**< IEEE NORMAL 1.5KB Pkt Length */
    VSC_PHY_PKT_MODE_JUMBO_9_KB,       /**< JUMBO 9KB Pkt Length */
    VSC_PHY_PKT_MODE_JUMBO_12_KB,      /**< JUMBO 12KB Pkt Length */
} vsc_phy_pkt_mode_t;

/** \brief PHY SigDet Polarity Definitions, used in PHY Config  */
typedef enum {
    VSC_PHY_SIGDET_POLARITY_ACT_LOW = 1,   /**< Set Sigdet polarity Active low */
    VSC_PHY_SIGDET_POLARITY_ACT_HIGH,      /**< Set Sigdet polarity Active High */
} vsc_phy_sigdet_polarity_t;

/** \brief PHY Unidirectional Definitions, used in PHY Config  */
typedef enum {
    VSC_PHY_UNIDIR_DISABLE = 0,   /**< Disable Unidirectional (Default) */
    VSC_PHY_UNIDIR_ENABLE,        /**< Enable Unidirectional */
} vsc_phy_unidirectional_t;

/** \brief PHY MAC SerDes PCS SGMII Input Preamble Definitions, used in PHY Config */
typedef enum {
    VSC_PHY_MAC_SERD_PCS_SGMII_IN_PRE_NONE    = 0,    /**< MAC SerDes PCS Control, SGMII Input Preamble for 100BaseX - No Preamble Req'd */
    VSC_PHY_MAC_SERD_PCS_SGMII_IN_PRE_ONE     = 1,    /**< MAC SerDes PCS Control, SGMII Input Preamble for 100BaseX - One-Byte Preamble Req'd */
    VSC_PHY_MAC_SERD_PCS_SGMII_IN_PRE_TWO     = 2,    /**< MAC SerDes PCS Control, SGMII Input Preamble for 100BaseX - Two-Byte Preamble Req'd */
    VSC_PHY_MAC_SERD_PCS_SGMII_IN_PRE_RSVD    = 3     /**< MAC SerDes PCS Control, SGMII Input Preamble for 100BaseX - Reserved */
} vsc_phy_mac_serd_pcs_sgmii_pre_t;

/** \brief PHY MAC SerDes PCS Control Structure, used in PHY Config (See DS Reg16E3 definitions) */
typedef struct {
    uint8_t                             disable;            /**< MAC i/f disable: 1000BaseX MAC i/f disable when media link down  */
    uint8_t                             restart;            /**< MAC i/f restart: 1000BaseX MAC i/f restart on media link change  */
    uint8_t                             pd_enable;          /**< MAC i/f ANEG parallel detect enable             */
    uint8_t                             aneg_restart;       /**< Restart MAC i/f ANEG */
    uint8_t                             force_adv_ability;  /**< Force adv. ability from Reg18E3 */
    vsc_phy_mac_serd_pcs_sgmii_pre_t    sgmii_in_pre;       /**< SGMII Input Preamble for 100BaseFX */
    uint8_t                             sgmii_out_pre;      /**< SGMII Output Preamble */
    uint8_t                             serdes_aneg_ena;    /**< MAC SerDes ANEG Enable  */
    uint8_t                             serdes_pol_inv_in;  /**< Invert SerDes Polarity at input of MAC */
    uint8_t                             serdes_pol_inv_out; /**< Invert SerDes Polarity at output of MAC */
    uint8_t                             fast_link_stat_ena; /**< Fast Link Fail Status Enable */
    uint8_t                             inhibit_odd_start;  /**< Inhibit MAC Odd-Start delay */
} vsc_phy_mac_serd_pcs_cntl_t;

/** \brief  PHY MEDIA SerDes PCS Remote Fault Indication Definition, See Clause 37, Table 37-3 */
typedef enum {
    VSC_PHY_MEDIA_SERD_PCS_REM_FAULT_NO_ERROR   = 0,    /**< Media SerDes PCS Control, Most Recent Clause 37 ANEG Exchg - Table 37-3 */
    VSC_PHY_MEDIA_SERD_PCS_REM_FAULT_OFFLINE    = 1,    /**< Media SerDes PCS Control, Most Recent Clause 37 ANEG Exchg */
    VSC_PHY_MEDIA_SERD_PCS_REM_FAULT_LINK_FAIL  = 2,    /**< Media SerDes PCS Control, Most Recent Clause 37 ANEG Exchg */
    VSC_PHY_MEDIA_SERD_PCS_REM_FAULT_ANEG_ERROR = 3     /**< Media SerDes PCS Control, Most Recent Clause 37 ANEG Exchg */
} vsc_phy_media_rem_fault_t;

/** \brief PHY MEDIA SerDes PCS Control Struct, used in PHY Config (See DS Reg23E3 */
typedef struct {
    vsc_phy_media_rem_fault_t     remote_fault;         /**< Remote Fault to Media indication sent in most recent Clause 37 ANEG  */
    uint8_t                       aneg_pd_detect;       /**< SerDes MEDIA ANEG parallel detect enable             */
    uint8_t                       force_adv_ability;    /**< Force adv. ability from Reg25E3 */
    uint8_t                       serdes_pol_inv_in;    /**< Invert SerDes Polarity at input of Media SerDes */
    uint8_t                       serdes_pol_inv_out;   /**< Invert SerDes Polarity at output of Media SerDes */
    uint8_t                       inhibit_odd_start;    /**< Inhibit Media Odd-Start delay */
    uint8_t                       force_hls;            /**< Forces 100BaseFX to Tx HSL continuously  */
    uint8_t                       force_fefi;           /**< Forces 100BaseFX Far-End-Fault Indication */
    uint8_t                       force_fefi_value;     /**< Forces/Suppress FEFI */
} vsc_phy_media_serd_pcs_cntl_t;

typedef enum {
    VSC_PHY_RGMII_CLK_DELAY_200_PS =  0,     /**< RGMII/GMII Clock Delay (Skew), For VSC8502, See Reg20E2  */
    VSC_PHY_RGMII_CLK_DELAY_800_PS =  1,     /**< RGMII/GMII Clock Delay (Skew), For VSC8502, See Reg20E2  */
    VSC_PHY_RGMII_CLK_DELAY_1100_PS = 2,     /**< RGMII/GMII Clock Delay (Skew), For VSC8502, See Reg20E2  */
    VSC_PHY_RGMII_CLK_DELAY_1700_PS = 3,     /**< RGMII/GMII Clock Delay (Skew), For VSC8502, See Reg20E2  */
    VSC_PHY_RGMII_CLK_DELAY_2000_PS = 4,     /**< RGMII/GMII Clock Delay (Skew), For VSC8502, See Reg20E2  */
    VSC_PHY_RGMII_CLK_DELAY_2300_PS = 5,     /**< RGMII/GMII Clock Delay (Skew), For VSC8502, See Reg20E2  */
    VSC_PHY_RGMII_CLK_DELAY_2600_PS = 6,     /**< RGMII/GMII Clock Delay (Skew), For VSC8502, See Reg20E2  */
    VSC_PHY_RGMII_CLK_DELAY_3400_PS = 7      /**< RGMII/GMII Clock Delay (Skew), For VSC8502, See Reg20E2  */
} vsc_phy_rgmii_gmii_clk_skew_t;

/** \brief PHY Slew Rate Definitions (Clock Edge Rate Control) in PHY Config  */
typedef enum {
    VSC_PHY_CLK_SLEW_RATE_0 =  0,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
    VSC_PHY_CLK_SLEW_RATE_1 =  1,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
    VSC_PHY_CLK_SLEW_RATE_2 =  2,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
    VSC_PHY_CLK_SLEW_RATE_3 =  3,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
    VSC_PHY_CLK_SLEW_RATE_4 =  4,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
    VSC_PHY_CLK_SLEW_RATE_5 =  5,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
    VSC_PHY_CLK_SLEW_RATE_6 =  6,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
    VSC_PHY_CLK_SLEW_RATE_7 =  7,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
} vsc_phy_clk_slew_t;

/** \brief PHY 1G configuration for Setting Master/Slave */
typedef struct {
    struct {
        uint8_t cfg;                   /**< Manual Master/Slave Config. 1=enabled */
        uint8_t val;                   /**< Master/Slave Config value, 1=Master */
    } master;                          /**< Master/Slave Mode */
} vsc_phy_conf_1g_t;


/** \brief PHY Configuration Struct, used in PHY Config  */
typedef struct phy_conf_t {
    vsc_phy_mode_t                 mode;         /**< PHY mode */
    vsc_phy_forced_t               forced;       /**< Forced mode configuration */
    vsc_phy_auto_neg_t             aneg;         /**< Auto-negotiation mode configuration */
    vsc_phy_mdi_t                  mdi;          /**< Cu cable MDI (Crossed cable / normal cable) */
    vsc_phy_pkt_mode_t             pkt_mode;     /**< Jumbo Packet Mode */
    vsc_phy_conf_flf_t             flf;          /**< Fast link failure & Fast Link Fail2 configuration */
    vsc_phy_sigdet_polarity_t      sigdet;       /**< Sigdet pin polarity configuration */
    vsc_phy_unidirectional_t       unidir;       /**< Unidirectional Configuration */
    vsc_phy_mac_serd_pcs_cntl_t    mac_if_pcs;   /**< PHY MAC SerDes PCS Control (Reg16E3) */
    vsc_phy_media_serd_pcs_cntl_t  media_if_pcs; /**< PHY MAC SerDes PCS Control (Reg23E3) */
    vsc_phy_rgmii_gmii_clk_skew_t  rx_clk_skew;  /**< PHY MAC RGMII/GMII Rx Clk Delay */
    vsc_phy_rgmii_gmii_clk_skew_t  tx_clk_skew;  /**< PHY MAC RGMII/GMII Tx Clk Delay */
    vsc_phy_clk_slew_t             clk_slew_rate;/**< PHY MAC Clk Slew Rate, ie. Edge Rate Control */
    vsc_phy_conf_1g_t              conf_1g;      /**< PHY 1G Master/Slave Manual Config for Forced Mode */
} vsc_phy_conf_t;

/** \brief PHY Clause_37 LP Ability Structure, used in PHY Status of CuSFP  */
typedef struct {
    vsc_port_speed_t  speed;      /**< Speed: 1000M(1G)=2; 100M=1; 10M=0 */
    uint8_t           fdx;        /**< Full duplex=1, Half duplex=0 */
    uint8_t           link;       /**< Link-Up Status=1, Link-Dn Status=0 */
} vsc_phy_cl37_status_t;

/** \brief PHY Port Status Struct, used in PHY Status  */
typedef struct phy_port_status_t {
    uint8_t           link_down;       /**< Link down event occurred since last call */
    uint8_t           link;            /**< Link is up. Remaining fields only valid if TRUE */
    vsc_port_speed_t  speed;           /**< Speed */
    uint8_t           fdx;             /**< Full duplex */
    uint8_t           remote_fault;    /**< Remote fault signalled */
    uint8_t           aneg_complete;   /**< Autoneg completed (for clause_37 and Cisco aneg) */
    uint8_t           ext_status_ena;  /**< Autoneg completed (for clause_37 and Cisco aneg) */
    uint8_t           unidir_able;     /**<TRUE: PHY able to transmit from media independent interface regardless of whether the PHY has 
                                           determined that a valid link has been established.FALSE: PHY able to transmit from media 
                                           independent interface only when the PHY has determined that a valid link has been established. 
                                           Note This bit is only applicable to 100BASE-FX and 1000BASE-X fiber media modes.*/
    uint8_t           aneg_obey_pause; /**< ANEG Result, This port should obey PAUSE frames */
    uint8_t           aneg_generate_pause; /**< ANEG Result, Link partner obeys PAUSE frames */
    uint8_t           mdi_cross;       /**< Indication of if Auto-MDIX crossover is performed */
    uint8_t           fiber;           /**< Indication of if the link is a fiber link, TRUE if link is a fiber link. FALSE if link is cu link */
    vsc_phy_cl37_status_t  cl37_status; /* Speed, Duplex, and Link from Cl_37 Link Partner for CuSFP Mode */ 
} vsc_phy_port_status_t;


/* - Clock configuration ---------- -------------------------------- */
/** \brief PHY active clock out */
typedef enum {
   VSC_PHY_RECOV_CLK1 = 0,        /**< RCVRD_CLK1 */
   VSC_PHY_RECOV_CLK2,            /**< RCVRD_CLK2 */
   VSC_PHY_RECOV_CLK_NUM          /**< Number of recovered clocks */
} vsc_phy_recov_clk_t;            /**< Container of recovered clock out identifier */

/** \brief PHY clock sources */
typedef enum {
    VSC_PHY_CLK_DISABLED, /**< Recovered Clock Disable */
    VSC_PHY_SERDES_MEDIA, /**< SerDes PHY */
    VSC_PHY_COPPER_MEDIA, /**< Copper PHY */
    VSC_PHY_TCLK_OUT,     /**< Transmitter TCLK */
    VSC_PHY_LOCAL_XTAL    /**< Local XTAL */
} vsc_phy_clk_src_t;

/** \brief PHY clock frequencies */
typedef enum {
    VSC_PHY_FREQ_25M,  /**< 25 MHz */
    VSC_PHY_FREQ_125M, /**< 125 MHz */
    VSC_PHY_FREQ_3125M /**< 31.25 MHz This is only valid on ATOM family - NOT Enzo*/
} vsc_phy_freq_t;

/** \brief PHY clock squelch levels */
typedef enum {
    VSC_PHY_CLK_SQUELCH_MAX = 0, /**< Automatically squelch clock to low when the link is not up, is unstable, is up in a mode that does not support the generation of arecovered clock (1000BASE-T master or 10BASE-T), or is up in EEE mode (100BASE-TX or 1000BASE-T slave).*/

    VSC_PHY_CLK_SQUELCH_MED = 1, /**< Same as VTSS_PHY_CLK_SQUELCH_MAX except that the clock is also generated in 1000BASE-T master and 10BASE-T link-up modes. This mode also generates a recovered clock output in EEE mode during reception of LP_IDLE.*/
    VSC_PHY_CLK_SQUELCH_MIN = 2, /**< Squelch only when the link is not up*/
    VSC_PHY_CLK_SQUELCH_NONE= 3, /**< Disable clock squelch.*/
} vsc_phy_clk_squelch;

/** \brief PHY Recovered clock configuration */
typedef struct {
    uint16_t             clk_src_sel;    /**< Source Port for PHY Recovered Clk bits 14:11*/
    vsc_phy_clk_src_t    src;            /**< Clock src selection for specified PHY port */
    vsc_phy_freq_t       freq;           /**< Clock Output frequency */
    vsc_phy_clk_squelch  squelch;        /**< Clock squelch level */
} vsc_phy_clk_conf_t;

/** \brief PHY CLock-Out Definitions, used in the CLKOUT PHY Config  */
typedef enum {
    VSC_PHY_CLOCK_OUT_25Mhz  = 0, /**< CLKOUT Freq Selection for Reg13G.14:13   */
    VSC_PHY_CLOCK_OUT_50Mhz  = 1, /**< CLKOUT Freq Selection for Reg13G.14:13   */
    VSC_PHY_CLOCK_OUT_125Mhz = 2, /**< CLKOUT Freq Selection for Reg13G.14:13   */
} vsc_phy_clockout_freq_t;

/** \brief PHY 1G configuration for Setting Master/Slave */
typedef struct {
    uint8_t                    clkout_enable;  /**< Enable/Disable of ClkOut for RMII, 1=enabled */
    vsc_phy_clockout_freq_t    freq_select;    /**< Frequency Select */
} vsc_phy_clockout_conf_t;


/** \brief PHY 1G configuration for Setting Loopback Config */
typedef struct {
    uint8_t near_end_enable;                     /**< Enable/Disable Near-End Loopback */
    uint8_t far_end_enable;                      /**< Enable/Disable Far-End Loopback */
    uint8_t connector_enable;                    /**< Enable/Disable Connector Loopback */
} vsc_phy_loopback_t;


/** \brief  The following structures are defined for Firmware DnLoad processing and PHY init_script processing */
// DO NOT USE __KERNEL_ONLY__ option at this time, as this requires files to be populated in certain linux directories
#ifdef __KERNEL_ONLY__

typedef struct firmware   phy_init_scripts_struct;
typedef struct firmware   phy_micro_patch_struct;

#else

struct firmware {
    size_t     size;
    uint8_t   *data;
};

typedef struct firmware  phy_init_scripts_struct;
typedef struct firmware  phy_micro_patch_struct;

#endif

/**< Function prototypes for functions used for ALL VSC PHY's */
/**< PHY Common Function Prototypes   */
/**
 * \brief  Get the PHY Type and Rev information
 *
 * \param cntrl [IN]       PHY Device/Port information used to access PHY
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t vsc_get_phy_type(vsc_phy_control_t * cntrl);

/**
 * \brief  Reset the PHY LCPLL
 *
 * \param cntrl [IN]       PHY Device/Port information used to access PHY
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t vsc_reset_phy_lcpll(vsc_phy_control_t * cntrl);

/**
 * \brief  Debug Function to Set the PHY 6G SerDes ob_level (Amplitude Control) value
 *
 * \param cntrl [IN]     PHY Device/Port information used to access PHY
 * \param ob_level [IN]  Modify ob_lev settings in serdes6g_ob_cfg1 for 6G SerDes Macro (See TN1052), 6 bits, default:0x18, Customizeable Amplitude Control.
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t vsc_phy_sd6g_ob_level(vsc_phy_control_t    *cntrl, uint8_t  ob_level);

/**
 * \brief Debug function for modifying the 6G SerDes ob_post0 and ob_post1 values.
 *
 * \param cntrl [IN]     PHY Device/Port information used to access PHY
 * \param ob_post0 [IN]  Modify ob_post0 settings in serdes6g_ob_cfg for 6G SerDes Macro (See TN1052), 6 bits, default:0, Customizeable.
 * \param ob_post1 [IN]  Modify ob_post1 settings in serdes6g_ob_cfg for 6G SerDes Macro (See TN1052), 5 bits, default:0, Do not change.
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t vsc_phy_sd6g_ob_post(vsc_phy_control_t  *cntrl, uint8_t  ob_post0, uint8_t  ob_post1);

/**
 * \brief  Set the PHY Config Values
 *
 * \param cntrl [IN]     PHY Device/Port information used to access PHY
 * \param conf [IN]      PHY configuration.
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t vsc_phy_conf_set(vsc_phy_control_t * cntrl, vsc_phy_conf_t * conf);

/**
 * \brief  Get the PHY Status Values
 *
 * \param cntrl [IN]     PHY Device/Port information used to access PHY
 * \param status [OUT]   PHY Status for the given port
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t vsc_phy_status_get(vsc_phy_control_t * cntrl, vsc_phy_port_status_t *status);

/**
 * \brief Set PHY clock configuration.
 *        RESTRICTION: This function can ONLY be run on the BASE PORT of the PHY, Port 0
 *
 * \param cntrl [IN]       PHY Device/Port information used to access PHY
 * \param clock_port [IN]  Set configuration for this clock port: Recov_Clk1 or Recov_Clk2.
 * \param conf [IN]        PHY clock configuration.
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t vsc_phy_clock_conf_set(vsc_phy_control_t          *cntrl,
                                      const vsc_phy_recov_clk_t   clock_port,
                                      const vsc_phy_clk_conf_t   *const conf);


/**
 * \brief Get PHY clock configuration.
 *        RESTRICTION: This function can ONLY be run on the BASE PORT of the PHY, Port 0
 *
 * \param cntrl [IN]           PHY Device/Port information used to access PHY
 * \param clock_port [IN]      Get configuration for this clock port: Recov_Clk1 or Recov_Clk2.
 * \param conf [OUT]           PHY clock configuration.
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t vsc_phy_clock_conf_get(vsc_phy_control_t          *cntrl,
                                      const vsc_phy_recov_clk_t   clock_port,
                                      vsc_phy_clk_conf_t         *conf);


/**
 * \brief  READ a particular PHY CSR Register and get it's Value.
 *   VALID FOR: VIPER & ELISE FAMILY ONLY -- VSC8584 & VSC8514 
 *   CSR Registers are Indirect access
 *
 * \param cntrl [IN]           PHY Device/Port information used to access PHY
 * \param csr_reg [IN]         CSR Register to be read
 * \param csr_reg_val [OUT]    CSR Register Value
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t vsc_phy_sd6g_csr_reg_read_debug(vsc_phy_control_t       *cntrl,
                                               const uint32_t           csr_reg,
                                               uint32_t                *csr_reg_val);

/**
 * \brief  Apply the 1G SerDes Patch to the PHY
 *         Adjust the 1G SerDes SigDet Input Threshold and Signal Sensitivity for 100FX
 *         For Luton and Tesla, This is done in the Micro-Patch
 *
 *   NOTE: Execute on Port 0 of the PHY only!
 *         Use the cntrl to indicate port_0 and the Tgt_Port_no for desired port on the PHY
 *   VALID FOR: VIPER VSC8584 & VSC8582 
 *
 * \param cntrl [IN]           PHY Device/Port information used to access PHY
 * \param tgt_port_no [IN]     Target Port_no for the slave 
 * \param csr_reg_val [OUT]    CSR Register Value
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t vsc_phy_sd1g_patch(vsc_phy_control_t       *cntrl,
                                  const uint16_t           tgt_port_no,
                                  const uint8_t            is_100fx);


/**
 * \brief  Setup the Manual Master/Slave mode when PHY set to 1G
 *
 * \param cntrl [IN]           PHY Device/Port information used to access PHY
 * \param conf_1g [IN]         Manual Master/Slave config for when PHY in 1G Forced Mode 
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t vsc_phy_conf_1g_set(vsc_phy_control_t *cntrl, vsc_phy_conf_1g_t   *conf_1g);


/**
 * \brief  Setup the Loopback Configuration mode for the PHY port 
 *
 * \param cntrl [IN]           PHY Device/Port information used to access PHY
 * \param loopback [IN]        Loopback Config settings for Near/Far/Connector Loopbacks 
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t vsc_phy_loopback_set(vsc_phy_control_t *cntrl, vsc_phy_loopback_t   *loopback);



/**
 * \brief  Setup the Jumbo Packet Mode Configuration mode for the PHY port 
 *
 * \param cntrl [IN]           PHY Device/Port information used to access PHY
 * \param pkt_mode [IN]        Jumbo Packet Mode of operation
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t  vsc_phy_packet_mode_set(vsc_phy_control_t *cntrl, vsc_phy_pkt_mode_t   pkt_mode);




#endif
