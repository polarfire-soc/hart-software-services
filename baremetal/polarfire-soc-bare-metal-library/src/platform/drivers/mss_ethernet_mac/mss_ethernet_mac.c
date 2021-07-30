/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC 10/100/1000 Mbps Ethernet MAC bare metal software driver implementation.
 *
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "mss_plic.h"
#include "mss_util.h"
#include "mss_ints.h"
#include "fpga_design_config/fpga_design_config.h"

#include "drivers/mss_mac/mss_ethernet_registers.h"
#include "drivers/mss_mac/mss_ethernet_mac_regs.h"
#include "drivers/mss_mac/mss_ethernet_mac_user_config.h"

#if defined(USING_FREERTOS)
#include "FreeRTOS.h"
#endif

#include "drivers/mss_mac/mss_ethernet_mac.h"
#include "drivers/mss_mac/phy.h"
#include "hal/hal.h"
#include "mss_assert.h"
#include "mss_sysreg.h"

#if defined (TI_PHY)
#include "mss_gpio.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************/
/* Preprocessor Macros                                                    */
/**************************************************************************/

#define NULL_POINTER                    (void *)0U

#if !defined(NDEBUG)
#define IS_STATE(x)                     ( ((x) == MSS_MAC_ENABLE) || ((x) == MSS_MAC_DISABLE) )

#endif  /* NDEBUG */

#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
#define IS_WORD_ALIGNED(x)              ((uint64_t)0U == ((uint64_t)(x) & (uint64_t)3U))
#else
#define IS_WORD_ALIGNED(x)              ((uint32_t)0U == ((uint32_t)(x) & (uint32_t)3U))
#endif

#define BMSR_AUTO_NEGOTIATION_COMPLETE  (0x0020U)

#define INVALID_INDEX                   (0xFFFFFFFFU)

#define PHY_ADDRESS_MIN                 (0U)
#define PHY_ADDRESS_MAX                 (31U)

/*
 * Defines for determining DMA descriptor sizes
 */
#if 0
 #if !defined(MSS_MAC_TIME_STAMPED_MODE)
#define MSS_MAC_TIME_STAMPED_MODE      0 /* Default to non time stamped descriptors */
#endif
#endif

#if defined(TARGET_ALOE)
#define MSS_MAC0_BASE     (0x10090000U);
#elif defined(TARGET_G5_SOC)
#define MSS_MAC0_BASE     (0x20110000U)
#define MSS_EMAC0_BASE    (0x20111000U)
#define MSS_MAC1_BASE     (0x20112000U)
#define MSS_EMAC1_BASE    (0x20113000U)

#define MSS_MAC0_BASE_HI  (0x28110000U)
#define MSS_EMAC0_BASE_HI (0x28111000U)
#define MSS_MAC1_BASE_HI  (0x28112000U)
#define MSS_EMAC1_BASE_HI (0x28113000U)

#define MSS_MAC_GEM0_ABP_BIT (0x00010000U)
#define MSS_MAC_GEM1_ABP_BIT (0x00020000U)
#else
#warning "No target platform defined for MSS Ethernet MAC"
#endif

/**************************************************************************/
/* Private variables                                                      */
/**************************************************************************/

#if defined(TARGET_ALOE)
static volatile uint32_t *GEMGXL_tx_clk_sel = (volatile uint32_t *)0x100A0000UL;
static volatile uint32_t *GEMGXL_speed_mode = (volatile uint32_t *)0x100A0020UL;
#endif

/**************************************************************************
 * Global variables                                                       *
 *                                                                        *
 * Note: there are two instances of the GMAC for G5 SOC and each has a    *
 * primary MAC and a secondary eMAC for time sensative traffic. The FU540 *
 * device on the Aloe board has a single primary MAC.                     *
 **************************************************************************/

#if defined(TARGET_G5_SOC)
mss_mac_instance_t g_mac0;
mss_mac_instance_t g_mac1;
mss_mac_instance_t g_emac0;
mss_mac_instance_t g_emac1;
#endif

#if defined(MSS_MAC_USE_DDR)
#if MSS_MAC_USE_DDR == MSS_MAC_MEM_DDR
uint8_t *g_mss_mac_ddr_ptr = (uint8_t *)0xC0000000LLU;
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_FIC0
uint8_t *g_mss_mac_ddr_ptr = (uint8_t *)0x60000000LLU;
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_FIC1
uint8_t *g_mss_mac_ddr_ptr = (uint8_t *)0xE0000000LLU;
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_CRYPTO
uint8_t *g_mss_mac_ddr_ptr = (uint8_t *)0x22002000LLU;
#else
#error "bad memory region defined"
#endif
#endif


#if defined(TARGET_ALOE)
mss_mac_instance_t g_mac0;
#endif


/**************************************************************************/
/* Private Functions                                                      */
/**************************************************************************/
static void mac_reset(void);
static void config_mac_hw(mss_mac_instance_t *this_mac, const mss_mac_cfg_t * cfg);
static void tx_desc_ring_init(mss_mac_instance_t *this_mac);
static void rx_desc_ring_init(mss_mac_instance_t *this_mac);
static void assign_station_addr(mss_mac_instance_t *this_mac, const uint8_t mac_addr[MSS_MAC_MAC_LEN]);
static void generic_mac_irq_handler(mss_mac_instance_t *this_mac, uint32_t queue_no);
static void rxpkt_handler(mss_mac_instance_t *this_mac, uint32_t queue_no);
static void txpkt_handler(mss_mac_instance_t *this_mac, uint32_t queue_no);
static void update_mac_cfg(const mss_mac_instance_t *this_mac);
static uint8_t probe_phy(const mss_mac_instance_t *this_mac);
static void instances_init(mss_mac_instance_t *this_mac, mss_mac_cfg_t *cfg);

static void msgmii_init(const mss_mac_instance_t *this_mac);
static void msgmii_autonegotiate(const mss_mac_instance_t *this_mac);

/**************************************************************************/
/* Public Functions                                                       */
/**************************************************************************/
/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */
void
MSS_MAC_init
(
    mss_mac_instance_t *this_mac,
    mss_mac_cfg_t      *cfg
)
{
    int32_t queue_no;
    ASSERT(cfg != NULL_POINTER);
#if defined(TARGET_ALOE)
    ASSERT(this_mac == &g_mac0);

    instances_init(this_mac, cfg);

    if((cfg != NULL_POINTER) && (this_mac == &g_mac0))
#endif
#if defined(TARGET_G5_SOC)
    ASSERT((this_mac == &g_mac0) || (this_mac == &g_mac1) || (this_mac == &g_emac0) || (this_mac == &g_emac1));
    instances_init(this_mac, cfg);

    /*
     * Always reset GEM if the pMAC is selected for init but not if eMAC
     * The pMAC should always be initialised first followed by the eMAC so this
     * is ok...
     */
    if(this_mac == &g_mac0)
    {
        SYSREG->SUBBLK_CLOCK_CR |= (uint32_t)2U;
        /* Reset MAC */
        SYSREG->SOFT_RESET_CR |= (uint32_t)2U;
        {
            volatile int64_t index;

            index = 0;
            while(1000 != index) /* Don't know if necessary, but delay a bit before de-asserting reset... */
            {
                index++;
            }
        }
        /* Take MAC out of reset. */
        SYSREG->SOFT_RESET_CR &= (uint32_t)~2U;
    }

    if(this_mac == &g_mac1)
    {
        SYSREG->SUBBLK_CLOCK_CR |= (uint32_t)4U;
        /* Reset MAC */
        SYSREG->SOFT_RESET_CR |= (uint32_t)4U;
        {
            volatile int64_t index;

            index = 0;
            while(1000 != index) /* Don't know if necessary, but delay a bit before de-asserting reset... */
            {
                index++;
            }
        }
        /* Take MAC out of reset. */
        SYSREG->SOFT_RESET_CR &= (uint32_t)~4U;
    }


/* PMCS: for the Emulation platform we need to select the non default TSU clock
 * for the moment or TX won't work */

    if(0U == this_mac->is_emac)
    {
        this_mac->mac_base->USER_IO = 1U;
    }

    if((cfg != NULL_POINTER) && ((this_mac == &g_mac0) || (this_mac == &g_mac1) || (this_mac == &g_emac0) || (this_mac == &g_emac1)))
#endif /* defined(TARGET_G5_SOC) */
    {
        this_mac->phy_addr            = cfg->phy_addr;
        this_mac->pcs_phy_addr        = cfg->pcs_phy_addr;
        this_mac->interface_type      = cfg->interface_type;
        this_mac->jumbo_frame_enable  = cfg->jumbo_frame_enable;
        this_mac->phy_type            = cfg->phy_type;
        this_mac->phy_autonegotiate   = cfg->phy_autonegotiate;
        this_mac->phy_get_link_status = cfg->phy_get_link_status;
        this_mac->phy_init            = cfg->phy_init;
        this_mac->phy_set_link_speed  = cfg->phy_set_link_speed;
        this_mac->append_CRC          = cfg->append_CRC;
#if MSS_MAC_USE_PHY_DP83867
        this_mac->phy_extended_read   = cfg->phy_extended_read;
        this_mac->phy_extended_write  = cfg->phy_extended_write;
#endif

#if defined(TARGET_G5_SOC)
        if(0U != cfg->use_local_ints)
        {
            __disable_local_irq(this_mac->mac_q_int[0]);
            if(0U == this_mac->is_emac)
            {
                __disable_local_irq(this_mac->mac_q_int[1]);
                __disable_local_irq(this_mac->mac_q_int[2]);
                __disable_local_irq(this_mac->mac_q_int[3]);
                __disable_local_irq(this_mac->mmsl_int);
            }
        }
        else
#endif
        {
            /*
             * In the following if an interrupt is set to NoInterrupt_IRQn then
             * the PLIC will ignore it as interrupt 0 is a dummy one.
             */
            PLIC_DisableIRQ(this_mac->mac_q_int[0]);
            PLIC_SetPriority(this_mac->mac_q_int[0], cfg->queue0_int_priority);

#if defined(TARGET_G5_SOC)
            PLIC_DisableIRQ(this_mac->mac_q_int[1]);
            PLIC_SetPriority(this_mac->mac_q_int[1], cfg->queue1_int_priority);
            PLIC_DisableIRQ(this_mac->mac_q_int[2]);
            PLIC_SetPriority(this_mac->mac_q_int[2], cfg->queue2_int_priority);
            PLIC_DisableIRQ(this_mac->mac_q_int[3]);
            PLIC_SetPriority(this_mac->mac_q_int[3], cfg->queue3_int_priority);
            PLIC_DisableIRQ(this_mac->mmsl_int);
            PLIC_SetPriority(this_mac->mmsl_int, cfg->mmsl_int_priority);
#endif
        }
        mac_reset();

        config_mac_hw(this_mac, cfg);

        /* Assign MAC station address */
        assign_station_addr(this_mac, cfg->mac_addr);

        /* Intialize Tx & Rx descriptor rings */
        tx_desc_ring_init(this_mac);
        rx_desc_ring_init(this_mac);

        this_mac->rx_discard = 0U; /* Ensure normal RX operation */

        for(queue_no = 0; queue_no < MSS_MAC_QUEUE_COUNT; queue_no++)
        {
            /* Initialize Tx descriptors related variables. */
            this_mac->queue[queue_no].nb_available_tx_desc    = MSS_MAC_TX_RING_SIZE;

            /* Initialize Rx descriptors related variables. */
            this_mac->queue[queue_no].nb_available_rx_desc    = MSS_MAC_RX_RING_SIZE;
            this_mac->queue[queue_no].next_free_rx_desc_index = 0U;
            this_mac->queue[queue_no].first_rx_desc_index     = 0U;

            /* initialize default interrupt handlers */
            this_mac->queue[queue_no].pckt_tx_callback        = (mss_mac_transmit_callback_t)NULL_POINTER;
            this_mac->queue[queue_no].pckt_rx_callback        = (mss_mac_receive_callback_t)NULL_POINTER;

            /* Added these to MAC structure to make them MAC specific... */

            this_mac->queue[queue_no].ingress     = 0U;
            this_mac->queue[queue_no].egress      = 0U;
            this_mac->queue[queue_no].rx_overflow = 0U;
            this_mac->queue[queue_no].hresp_error = 0U;
            this_mac->queue[queue_no].rx_restart  = 0U;
        }
#if 0
        /* Initialize PHY interface */
        if(MSS_MAC_AUTO_DETECT_PHY_ADDRESS == cfg->phy_addr)
        {
            cfg->phy_addr = probe_phy();
        }
#endif
    if(0U == this_mac->is_emac) /* Only do the PHY stuff for primary MAC */
        {
            if(TBI == this_mac->interface_type)
            {
                msgmii_init(this_mac);
            }
#if defined(TARGET_ALOE)
            this_mac->phy_addr = 0U;
#endif

            this_mac->phy_init(this_mac, 0U);
            this_mac->phy_set_link_speed(this_mac, cfg->speed_duplex_select);
            this_mac->phy_autonegotiate(this_mac);

            if(TBI == this_mac->interface_type)
            {
                msgmii_autonegotiate(this_mac);
            }
        }
        update_mac_cfg(this_mac);

        /*
         * Enable TX Packet and TX Packet Bus Error interrupts.
         * We don't enable the tx underrun interrupt as we only send on demand
         * and don't need to explicitly note an underrun as that is the "normal"
         * state for the interface to be in.
         */
        /*
         * Enable RX Packet, RX Packet Overflow and RX Packet Bus Error
         * interrupts.
         */
        if(0U != this_mac->is_emac)
        {
            this_mac->emac_base->INT_ENABLE = GEM_RECEIVE_OVERRUN_INT | GEM_TRANSMIT_COMPLETE |
                                              GEM_RX_USED_BIT_READ | GEM_RECEIVE_COMPLETE | GEM_RESP_NOT_OK_INT;
        }
        else
        {
            for(queue_no = 0; queue_no < MSS_MAC_QUEUE_COUNT; queue_no++)
            {
                /* Enable pause related interrupts if pause control is selected */
                if((MSS_MAC_ENABLE == cfg->rx_flow_ctrl) || (MSS_MAC_ENABLE == cfg->tx_flow_ctrl))
                {
                    *this_mac->queue[queue_no].int_enable = GEM_RECEIVE_OVERRUN_INT | GEM_TRANSMIT_COMPLETE |
                            GEM_RX_USED_BIT_READ | GEM_RECEIVE_COMPLETE | GEM_RESP_NOT_OK_INT |
                            GEM_PAUSE_FRAME_TRANSMITTED | GEM_PAUSE_TIME_ELAPSED |
                            GEM_PAUSE_FRAME_WITH_NON_0_PAUSE_QUANTUM_RX;
                }
                else
                {
                    *this_mac->queue[queue_no].int_enable = GEM_RECEIVE_OVERRUN_INT | GEM_TRANSMIT_COMPLETE |
                            GEM_RX_USED_BIT_READ | GEM_RECEIVE_COMPLETE | GEM_RESP_NOT_OK_INT;
                }

#if 1 /* PMCS - set this to 0 if you want to check for un-handled interrupt conditions */
                *this_mac->queue[queue_no].int_enable |= GEM_PAUSE_FRAME_TRANSMITTED | GEM_PAUSE_TIME_ELAPSED |
                        GEM_PAUSE_FRAME_WITH_NON_0_PAUSE_QUANTUM_RX | GEM_LINK_CHANGE |
                        GEM_TX_LOCKUP_DETECTED | GEM_RX_LOCKUP_DETECTED |
                        GEM_AMBA_ERROR  | GEM_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION |
                        GEM_TRANSMIT_UNDER_RUN;
#endif
            }
        }

        /*
         * At this stage, the MSS MAC interrupts are disabled and won't be enabled
         * until at least one of the FIFOs is configured with a buffer(s) for
         * data transfer.
         */
    }

    this_mac->mac_available = MSS_MAC_AVAILABLE;
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */
void
MSS_MAC_update_hw_address
(
    mss_mac_instance_t *this_mac,
    const mss_mac_cfg_t * cfg
)
{
    ASSERT(cfg != NULL_POINTER);

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(cfg != NULL_POINTER)
        {
            /* Assign MAC station address */
            assign_station_addr(this_mac, cfg->mac_addr);
        }
    }
}


/******************************************************************************
 *
 */
static void update_mac_cfg(const mss_mac_instance_t *this_mac)
{
    mss_mac_speed_t speed;
    uint8_t fullduplex;
    uint8_t link_up;
    uint32_t temp_cr;

    link_up = this_mac->phy_get_link_status(this_mac, &speed, &fullduplex);

    if(link_up != MSS_MAC_LINK_DOWN)
    {
        if(0U != this_mac->is_emac)
        {
            temp_cr = this_mac->emac_base->NETWORK_CONFIG;
        }
        else
        {
            temp_cr = this_mac->mac_base->NETWORK_CONFIG;
        }

        temp_cr &= ~(GEM_GIGABIT_MODE_ENABLE | GEM_SPEED | GEM_FULL_DUPLEX);

        if(MSS_MAC_1000MBPS == speed)
        {
#if defined(TARGET_ALOE)
            *GEMGXL_tx_clk_sel = (uint32_t)0U;
            *GEMGXL_speed_mode = (uint32_t)2U;
#endif
            temp_cr |= GEM_GIGABIT_MODE_ENABLE;
        }
        else
        {
            if(MSS_MAC_100MBPS == speed)
            {
#if defined(TARGET_ALOE)
                *GEMGXL_tx_clk_sel = (uint32_t)1U;
                *GEMGXL_speed_mode = (uint32_t)1U;
#endif
                temp_cr |= (uint32_t)GEM_SPEED;
            }
#if defined(TARGET_ALOE)
            else
            {
                *GEMGXL_tx_clk_sel = (uint32_t)1U;
                *GEMGXL_speed_mode = (uint32_t)0U;
            }
#endif
        }
        /* Configure duplex mode */
        if(MSS_MAC_FULL_DUPLEX == fullduplex)
        {
            temp_cr |= GEM_FULL_DUPLEX;
        }

        if(0U != this_mac->is_emac)
        {
            this_mac->emac_base->NETWORK_CONFIG = temp_cr;
        }
        else
        {
            this_mac->mac_base->NETWORK_CONFIG = temp_cr;
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */
uint8_t MSS_MAC_get_link_status
(
    const mss_mac_instance_t *this_mac,
    mss_mac_speed_t * speed,
    uint8_t *     fullduplex
)
{
    /* Todo: These statics will only work for the single MAC Aloe case... */
    static mss_mac_speed_t previous_speed = INVALID_SPEED;
    static uint8_t previous_duplex = 0xAAU;
    mss_mac_speed_t link_speed;
    uint8_t link_fullduplex;
    uint8_t link_up;

    link_up = MSS_MAC_LINK_DOWN; /* Default condition in case we are not active yet */

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        link_up = this_mac->phy_get_link_status(this_mac, &link_speed, &link_fullduplex);

        if(link_up != MSS_MAC_LINK_DOWN)
        {
            /*----------------------------------------------------------------------
             * Update MAC configuration if link characteristics changed.
             */
            if((link_speed != previous_speed) || (link_fullduplex != previous_duplex))
            {
                uint32_t temp_cr;

                if(0U != this_mac->is_emac)
                {
                    temp_cr = this_mac->emac_base->NETWORK_CONFIG;
                }
                else
                {
                    temp_cr = this_mac->mac_base->NETWORK_CONFIG;
                }

                temp_cr &= ~(GEM_GIGABIT_MODE_ENABLE | GEM_SPEED | GEM_FULL_DUPLEX);

                if(MSS_MAC_1000MBPS == link_speed)
                {
    #if defined(TARGET_ALOE)
                    *GEMGXL_tx_clk_sel = (uint32_t)0U;
                    *GEMGXL_speed_mode = (uint32_t)2U;
    #endif
                    temp_cr |= GEM_GIGABIT_MODE_ENABLE;
                }
                else
                {
                    if(MSS_MAC_100MBPS == link_speed)
                    {
    #if defined(TARGET_ALOE)
                        *GEMGXL_tx_clk_sel = (uint32_t)1U;
                        *GEMGXL_speed_mode = (uint32_t)1U;
    #endif
                        temp_cr |= (uint32_t)GEM_SPEED;
                    }
    #if defined(TARGET_ALOE)
                    else
                    {
                        *GEMGXL_tx_clk_sel = (uint32_t)1U;
                        *GEMGXL_speed_mode = (uint32_t)0U;
                    }
    #endif
                }
                /* Configure duplex mode */
                if(MSS_MAC_FULL_DUPLEX == link_fullduplex)
                {
                    temp_cr |= GEM_FULL_DUPLEX;
                }

                if(0U != this_mac->is_emac)
                {
                    this_mac->emac_base->NETWORK_CONFIG = temp_cr;
                }
                else
                {
                    this_mac->mac_base->NETWORK_CONFIG = temp_cr;
                }
            }

            previous_speed = link_speed;
            previous_duplex = link_fullduplex;

            /*----------------------------------------------------------------------
             * Return current link speed and duplex mode.
             */
            if(speed != NULL_POINTER)
            {
                *speed = link_speed;
            }

            if(fullduplex != NULL_POINTER)
            {
                *fullduplex = link_fullduplex;
            }
            if(GMII_SGMII == this_mac->interface_type) /* Emulation platform with embedded SGMII link for PHY connection and GMII for MAC connection */
            {
                uint16_t phy_reg;
                uint16_t sgmii_link_up;

                /*
                 * Find out if link is up on SGMII link between GMII core and
                 * external PHY.
                 *
                 * The link status bit latches 0 state until read to record fact
                 * link has failed since last read so you need to read it twice to
                 * get the current status...
                 */
                phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMSR);
                phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMSR);
                sgmii_link_up = phy_reg & BMSR_LSTATUS;

                if(0U == sgmii_link_up)
                {
                    /* Initiate auto-negotiation on the SGMII link. */
                    phy_reg = MSS_MAC_read_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR);
                    phy_reg |= BMCR_ANENABLE;
                    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR, phy_reg);
                    phy_reg |= BMCR_ANRESTART;
                    MSS_MAC_write_phy_reg(this_mac, (uint8_t)this_mac->pcs_phy_addr, MII_BMCR, phy_reg);
                }
             }

#if 0 /* TBD old SF2 stuff keep here for now for guidance... */
#if ((MSS_MAC_PHY_INTERFACE == SGMII) || (MSS_MAC_PHY_INTERFACE == TBI))
#if (MSS_MAC_PHY_INTERFACE == TBI)
/*----------------------------------------------------------------------
 * Make sure SGMII interface link is up. if interface is TBI
 */
#define MDIO_PHY_ADDR   SF2_MSGMII_PHY_ADDR
#endif /* #if (MSS_MAC_PHY_INTERFACE == TBI) */

#if (MSS_MAC_PHY_INTERFACE == SGMII)
/*----------------------------------------------------------------------
 * Make sure SGMII/1000baseX interface link is up. if interface is
 * SGMII/1000baseX
 */
#define MDIO_PHY_ADDR   MSS_MAC_INTERFACE_MDIO_ADDR
#endif /* #if ((MSS_MAC_PHY_INTERFACE == SGMII) || (MSS_MAC_PHY_INTERFACE == BASEX1000)) */

        {
            uint16_t phy_reg;
            uint16_t sgmii_link_up;

            /* Find out if link is up on SGMII link between MAC and external PHY. */
            phy_reg = MSS_MAC_read_phy_reg(MDIO_PHY_ADDR, MII_BMSR);
            sgmii_link_up = phy_reg & BMSR_LSTATUS;

            if(0U == sgmii_link_up)
            {
                /* Initiate auto-negotiation on the SGMII link. */
                phy_reg = MSS_MAC_read_phy_reg(MDIO_PHY_ADDR, MII_BMCR);
                phy_reg |= BMCR_ANENABLE;
                MSS_MAC_write_phy_reg(MDIO_PHY_ADDR, MII_BMCR, phy_reg);
                phy_reg |= BMCR_ANRESTART;
                MSS_MAC_write_phy_reg(MDIO_PHY_ADDR, MII_BMCR, phy_reg);
            }
         }
#endif
#endif
        }
    }

    return link_up;
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */
void
MSS_MAC_cfg_struct_def_init
(
    mss_mac_cfg_t * cfg
)
{
    ASSERT(NULL_POINTER != cfg);
    if(NULL_POINTER != cfg)
    {
        (void)memset(cfg, 0, sizeof(mss_mac_cfg_t)); /* Start with clean slate */

        cfg->speed_duplex_select = MSS_MAC_ANEG_ALL_SPEEDS;
#if defined(TARGET_ALOE)
        cfg->phy_addr            = 0U;
#endif
#if defined(TARGET_G5_SOC)
        cfg->phy_addr            = PHY_NULL_MDIO_ADDR;
#endif
        cfg->tx_edc_enable       = MSS_MAC_ERR_DET_CORR_DISABLE;
        cfg->rx_edc_enable       = MSS_MAC_ERR_DET_CORR_DISABLE;
        cfg->jumbo_frame_enable  = MSS_MAC_JUMBO_FRAME_DISABLE;
        cfg->jumbo_frame_default = MSS_MAC_MAX_PACKET_SIZE;
        cfg->length_field_check  = MSS_MAC_LENGTH_FIELD_CHECK_ENABLE;
        cfg->append_CRC          = MSS_MAC_CRC_ENABLE;
        cfg->loopback            = MSS_MAC_LOOPBACK_DISABLE;
        cfg->rx_flow_ctrl        = MSS_MAC_RX_FLOW_CTRL_ENABLE;
        cfg->tx_flow_ctrl        = MSS_MAC_TX_FLOW_CTRL_ENABLE;
        cfg->ipg_multiplier      = MSS_MAC_IPG_DEFVAL;
        cfg->ipg_divisor         = MSS_MAC_IPG_DEFVAL;
        cfg->phyclk              = MSS_MAC_DEF_PHY_CLK;
        cfg->mac_addr[0]         = 0x00U;
        cfg->mac_addr[1]         = 0x00U;
        cfg->mac_addr[2]         = 0x00U;
        cfg->mac_addr[3]         = 0x00U;
        cfg->mac_addr[4]         = 0x00U;
        cfg->mac_addr[5]         = 0x00U;
        cfg->queue_enable[0]     = MSS_MAC_QUEUE_DISABLE;
        cfg->queue_enable[1]     = MSS_MAC_QUEUE_DISABLE;
        cfg->queue_enable[2]     = MSS_MAC_QUEUE_DISABLE;
        cfg->phy_type            = MSS_MAC_DEV_PHY_NULL;
        cfg->interface_type      = NULL_PHY;
        cfg->phy_autonegotiate   = MSS_MAC_NULL_phy_autonegotiate;
        cfg->phy_get_link_status = MSS_MAC_NULL_phy_get_link_status;
        cfg->phy_init            = MSS_MAC_NULL_phy_init;
        cfg->phy_set_link_speed  = MSS_MAC_NULL_phy_set_link_speed;
        cfg->use_hi_address      = MSS_MAC_DISABLE;
        cfg->use_local_ints      = MSS_MAC_DISABLE;
        cfg->queue0_int_priority = 7U; /* Give them highest priority by default */
        cfg->queue1_int_priority = 7U;
        cfg->queue2_int_priority = 7U;
        cfg->queue3_int_priority = 7U;
        cfg->mmsl_int_priority   = 7U;
#if MSS_MAC_USE_PHY_DP83867
        cfg->phy_extended_read   = NULL_ti_read_extended_regs;
        cfg->phy_extended_write  = NULL_ti_write_extended_regs;
#endif
    }
}


/******************************************************************************
 *
 */
static void
mac_reset
(
    void
)
{
}

#if defined(TARGET_ALOE)

/******************************************************************************
 * PLL and Reset registers after reset in "wait for debug mode"
 * 0x10000000 : 0x10000000 <Hex Integer>
 * Address   0 - 3     4 - 7     8 - B     C - F
 * 10000000  C0000000  030187C1  00000000  030187C1
 * 10000010  00000000  00000000  00000000  030187C1
 * 10000020  00000000  00000001  00000000  00000004
 *
 * PLL and Reset registers after Linux boot.
 *
 * 0x10000000 : 0x10000000 <Hex Integer>
 * Address   0 - 3     4 - 7     8 - B     C - F
 * 10000000  C0000000  82110EC0  00000000  82110DC0
 * 10000010  80000000  00000000  00000000  82128EC0
 * 10000020  80000000  00000000  0000002F  00000004
 *
 */

/******************************************************************************
 *
 */
#define __I  const volatile
#define __IO volatile
#define __O volatile

typedef struct
{
    __IO uint32_t  HFXOSCCFG;      /* 0x0000 */
    __IO uint32_t  COREPLLCFG0;    /* 0x0004 */
    __IO uint32_t  reserved0;      /* 0x0008 */
    __IO uint32_t  DDRPLLCFG0;     /* 0x000C */
    __IO uint32_t  DDRPLLCFG1;     /* 0x0010 */
    __IO uint32_t  reserved1;      /* 0x0014 */
    __IO uint32_t  reserved2;      /* 0x0018 */
    __IO uint32_t  GEMGXLPLLCFG0;  /* 0x001C */
    __IO uint32_t  GEMGXLPLLCFG1;  /* 0x0020 */
    __IO uint32_t  CORECLKSEL;     /* 0x0024 */
    __IO uint32_t  DEVICERESETREG; /* 0x0028 */
} AloePRCI_TypeDef;

AloePRCI_TypeDef *g_aloe_prci = (AloePRCI_TypeDef *)0x10000000UL;

typedef struct
{
    __IO uint32_t  PWMCFG;         /* 0x0000 */
    __IO uint32_t  reserved0;      /* 0x0004 */
    __IO uint32_t  PWMCOUNT;       /* 0x0008 */
    __IO uint32_t  reserved1;      /* 0x000C */
    __IO uint32_t  PWMS;           /* 0x0010 */
    __IO uint32_t  reserved2;      /* 0x0014 */
    __IO uint32_t  reserved3;      /* 0x0018 */
    __IO uint32_t  reserved4;      /* 0x001C */
    __IO uint32_t  PWMCMP0;        /* 0x0020 */
    __IO uint32_t  PWMCMP1;        /* 0x0024 */
    __IO uint32_t  PWMCMP2;        /* 0x0028 */
    __IO uint32_t  PWMCMP3;        /* 0x002C */
} AloePWM_TypeDef;

AloePWM_TypeDef *g_aloe_pwm0 = (AloePWM_TypeDef *)0x10020000UL;


static void config_mac_pll_and_reset(void);
static void config_mac_pll_and_reset(void)
{
    volatile int64_t ix;
    volatile int64_t counter;
    volatile int64_t loops = 0;

    /*
     * COREPLLCFG0 reset value = 0x030187C1
     *  divr = 1
     *  divf = 1F
     *  divq = 3
     *  range = 0
     *  bypass = 1
     *  fse = 1
     *  lock = 0
     *
     *  Desired value = 82110EC0
     *  divr = 0
     *  divf = 1D
     *  divq = 2
     *  range = 4
     *  bypass = 0
     *  fse = 1
     *  lock = 1
     */
#if 0 /* Test code for proving Core clock speed switching works */
    g_aloe_pwm0->PWMCFG = 0x0000020EU;
    g_aloe_pwm0->PWMCMP0 = 0x0000FFFFU;
    g_aloe_pwm0->PWMCMP1 = 0x0000FFFFU;
    g_aloe_pwm0->PWMCMP2 = 0x0000FFFFU;
    g_aloe_pwm0->PWMCMP3 = 0x0000FFFFU;

    while(loops < 16)
    {
        if(ix & 1)
        {
            g_aloe_pwm0->PWMCMP0 = 0x00000000U;
        }
        else
        {
            g_aloe_pwm0->PWMCMP0 = 0x0000FFFFU;
        }

        if(ix & 2)
        {
            g_aloe_pwm0->PWMCMP1 = 0x00000000U;
        }
        else
        {
            g_aloe_pwm0->PWMCMP1 = 0x0000FFFFU;
        }

        if(ix & 4)
        {
            g_aloe_pwm0->PWMCMP2 = 0x00000000U;
        }
        else
        {
            g_aloe_pwm0->PWMCMP2 = 0x0000FFFFU;
        }

        if(ix & 8)
        {
            g_aloe_pwm0->PWMCMP3 = 0x00000000U;
        }
        else
        {
            g_aloe_pwm0->PWMCMP3 = 0x0000FFFFU;
        }

        ix++;
        for(counter = 0; counter != 100000; counter++)
            ;

        loops++;
    }
#endif

    g_aloe_prci->COREPLLCFG0 = 0x03110EC0U; /* Configure Core Clock PLL */
    while(g_aloe_prci->COREPLLCFG0 & 0x80000000U) /* Wait for lock with PLL bypassed */
        ix++;

    g_aloe_prci->COREPLLCFG0 = 0x02110EC0U; /* Take PLL out of bypass */
    g_aloe_prci->CORECLKSEL  = 0x00000000U; /* Switch to PLL as clock source */

#if 0 /* Test code for proving Core clock speed switching works */
    loops = 0;
    while(loops < 20)
    {
        if(ix & 1)
        {
            g_aloe_pwm0->PWMCMP0 = 0x0000FFFFU;
        }
        else
        {
            g_aloe_pwm0->PWMCMP0 = 0x00000000U;
        }

        ix++;
        for(counter = 0; counter != 3000000; counter++)
            ;

        loops++;
    }
#endif
    /*
     * GEMGXLPLLCFG0 reset value = 0x030187C1
     *  divr = 1
     *  divf = 1F
     *  divq = 3
     *  range = 0
     *  bypass = 1
     *  fse = 1
     *  lock = 0
     *
     *  Desired value = 82128EC0
     *  divr = 0
     *  divf = 3B
     *  divq = 5
     *  range = 4
     *  bypass = 0
     *  fse = 1
     *  lock = 1
     */

    g_aloe_prci->GEMGXLPLLCFG0 = 0x03128EC0U; /* Configure GEM Clock PLL */
    while(g_aloe_prci->GEMGXLPLLCFG0 & 0x80000000U) /* Wait for lock with PLL bypassed */
        ix++;

    g_aloe_prci->GEMGXLPLLCFG0 = 0x02128EC0U; /* Take PLL out of bypass */
    g_aloe_prci->GEMGXLPLLCFG1  = 0x80000000U; /* Switch to PLL as clock source */

    g_aloe_prci->DEVICERESETREG |= 0x00000020U; /* Release MAC from reset */

}
#endif


/******************************************************************************
 *
 */
static void config_mac_hw(mss_mac_instance_t *this_mac, const mss_mac_cfg_t * cfg)
{
    uint32_t temp_net_config = 0U;
    uint32_t temp_net_control = 0U;
    uint32_t temp_length;

    /* Check for validity of configuration parameters */
    ASSERT( IS_STATE(cfg->tx_edc_enable) );
    ASSERT( IS_STATE(cfg->rx_edc_enable) );
    ASSERT( MSS_MAC_PREAMLEN_MAXVAL >= cfg->preamble_length );
    ASSERT( IS_STATE(cfg->jumbo_frame_enable) );
    ASSERT( IS_STATE(cfg->length_field_check) );
    ASSERT( IS_STATE(cfg->append_CRC) );
    ASSERT( IS_STATE(cfg->loopback) );
    ASSERT( IS_STATE(cfg->rx_flow_ctrl) );
    ASSERT( IS_STATE(cfg->tx_flow_ctrl) );

#if defined(TARGET_ALOE)
    config_mac_pll_and_reset();
#endif
    /*--------------------------------------------------------------------------
     * Configure MAC Network Control register
     */
    temp_net_control = GEM_MAN_PORT_EN | GEM_CLEAR_ALL_STATS_REGS | GEM_PFC_ENABLE;
#if 0
    temp_net_control |= GEM_LOOPBACK_LOCAL; /* PMCS: Enable this to force local loop back */
#endif
    if(MSS_MAC_ENABLE == cfg->loopback)
    {
        temp_net_control |= GEM_LOOPBACK_LOCAL;
    }

#if defined(MSS_MAC_TIME_STAMPED_MODE)
    temp_net_control |= GEM_PTP_UNICAST_ENA;
#endif

    /*
     *  eMAC has to be configured as external TSU although it is actually using
     *  the pMAC TSU. There is only really 1 TSU per GEM instance and all
     *  adjustments etc should really be done via the pMAC.
     */
    if(0U != this_mac->is_emac)
    {
        temp_net_control |= GEM_EXT_TSU_PORT_ENABLE;
    }
    /*--------------------------------------------------------------------------
     * Configure MAC Network Config and Network Control registers
     */

#if defined(TARGET_G5_SOC)
    if(TBI == this_mac->interface_type)
    {
        temp_net_config = (((uint32_t)(1UL)) << GEM_DATA_BUS_WIDTH_SHIFT) | ((cfg->phyclk & GEM_MDC_CLOCK_DIVISOR_MASK) << GEM_MDC_CLOCK_DIVISOR_SHIFT) | GEM_PCS_SELECT | GEM_SGMII_MODE_ENABLE;
    }
    else
    {
        /* Actually for the G5 SoC Emulation Platform the interface is GMII... */
        temp_net_config = (((uint32_t)(1UL)) << GEM_DATA_BUS_WIDTH_SHIFT) | ((cfg->phyclk & GEM_MDC_CLOCK_DIVISOR_MASK) << GEM_MDC_CLOCK_DIVISOR_SHIFT);
    }
#endif
#if defined(TARGET_ALOE)
    /* No pause frames received in memory, divide PCLK by 224 for MDC */
    temp_net_config = (cfg->phyclk & GEM_MDC_CLOCK_DIVISOR_MASK) << GEM_MDC_CLOCK_DIVISOR_SHIFT;
#endif

    if((MSS_MAC_ENABLE == cfg->rx_flow_ctrl) || (MSS_MAC_ENABLE == cfg->tx_flow_ctrl))
    {
        temp_net_config |= GEM_FCS_REMOVE | GEM_DISABLE_COPY_OF_PAUSE_FRAMES | GEM_RECEIVE_1536_BYTE_FRAMES | GEM_PAUSE_ENABLE | GEM_FULL_DUPLEX | GEM_GIGABIT_MODE_ENABLE;
    }
    else
    {
        temp_net_config |= GEM_FCS_REMOVE | GEM_DISABLE_COPY_OF_PAUSE_FRAMES | GEM_RECEIVE_1536_BYTE_FRAMES | GEM_FULL_DUPLEX | GEM_GIGABIT_MODE_ENABLE;
    }

    if(MSS_MAC_ENABLE == cfg->length_field_check)
    {
        temp_net_config |= GEM_LENGTH_FIELD_ERROR_FRAME_DISCARD;
    }

    if(MSS_MAC_IPG_DEFVAL != cfg->ipg_multiplier) /* If we have a non zero value here then enable IPG stretching */
    {
        uint32_t stretch;
        temp_net_config |= GEM_IPG_STRETCH_ENABLE;

        stretch  = cfg->ipg_multiplier & GEM_IPG_STRETCH_MUL_MASK;
        stretch |= (cfg->ipg_divisor & GEM_IPG_STRETCH_DIV_MASK) << GEM_IPG_STRETCH_DIV_SHIFT;

        if(0U != this_mac->is_emac)
        {
            this_mac->emac_base->STRETCH_RATIO = stretch;
        }
        else
        {
            this_mac->mac_base->STRETCH_RATIO = stretch;
        }
    }

    if(0U != this_mac->is_emac)
    {
        this_mac->emac_base->NETWORK_CONTROL = temp_net_control;
        this_mac->emac_base->NETWORK_CONFIG  = temp_net_config;
        this_mac->mac_base->NETWORK_CONFIG   = temp_net_config;
    }
    else
    {
        this_mac->mac_base->NETWORK_CONTROL  = temp_net_control;
        this_mac->mac_base->NETWORK_CONFIG   = temp_net_config;
#if defined(TARGET_G5_SOC)
        this_mac->emac_base->NETWORK_CONFIG  = temp_net_config;
#endif
    }

    /*--------------------------------------------------------------------------
     * Reset PCS
     */
    if(0U == this_mac->is_emac)
    {
        this_mac->mac_base->PCS_CONTROL |= (uint32_t)0x8000UL;
    }

    /*--------------------------------------------------------------------------
     * Configure MAC Network DMA Config register
     */
    if(0U != this_mac->is_emac)
    {
#if defined(MSS_MAC_TIME_STAMPED_MODE)
        this_mac->emac_base->DMA_CONFIG = GEM_DMA_ADDR_BUS_WIDTH_1 | (MSS_MAC_RX_BUF_VALUE << GEM_RX_BUF_SIZE_SHIFT) |
                                         GEM_TX_PBUF_SIZE | (((uint32_t)(0x3UL)) << GEM_RX_PBUF_SIZE_SHIFT) | ((uint32_t)(16UL)) |
                                         GEM_TX_BD_EXTENDED_MODE_EN | GEM_RX_BD_EXTENDED_MODE_EN;

        /* Record TS for all packets by default */
        this_mac->emac_base->TX_BD_CONTROL = GEM_BD_TS_MODE;
        this_mac->emac_base->RX_BD_CONTROL = GEM_BD_TS_MODE;
#else
        this_mac->emac_base->DMA_CONFIG = GEM_DMA_ADDR_BUS_WIDTH_1 | (MSS_MAC_RX_BUF_VALUE << GEM_RX_BUF_SIZE_SHIFT) |
                                         GEM_TX_PBUF_SIZE | (((uint32_t)(0x3UL)) << GEM_RX_PBUF_SIZE_SHIFT) | ((uint32_t)(16UL));

#endif
    }
    else
    {
        int32_t queue_index;

#if defined(MSS_MAC_TIME_STAMPED_MODE)
        this_mac->mac_base->DMA_CONFIG = GEM_DMA_ADDR_BUS_WIDTH_1 |  (MSS_MAC_RX_BUF_VALUE << GEM_RX_BUF_SIZE_SHIFT) |
                                         GEM_TX_PBUF_SIZE | (((uint32_t)(0x3UL)) << GEM_RX_PBUF_SIZE_SHIFT) | ((uint32_t)(16UL)) |
                                         GEM_TX_BD_EXTENDED_MODE_EN | GEM_RX_BD_EXTENDED_MODE_EN;

        /* Record TS for all packets by default */
        this_mac->mac_base->TX_BD_CONTROL = GEM_BD_TS_MODE;
        this_mac->mac_base->RX_BD_CONTROL = GEM_BD_TS_MODE;
#else
        this_mac->mac_base->DMA_CONFIG  = GEM_DMA_ADDR_BUS_WIDTH_1 |  (MSS_MAC_RX_BUF_VALUE << GEM_RX_BUF_SIZE_SHIFT) |
                                         GEM_TX_PBUF_SIZE | (((uint32_t)(0x3UL)) << GEM_RX_PBUF_SIZE_SHIFT) | (uint32_t)(16UL);
#endif
#if (MSS_MAC_QUEUE_COUNT > 1)
        for(queue_index = 1; queue_index < MSS_MAC_QUEUE_COUNT; queue_index++)
        {
            *(this_mac->queue[queue_index].dma_rxbuf_size) = ((uint32_t)MSS_MAC_RX_BUF_VALUE);
        }
#endif
    }

    /*
     * Disable the other queues as the GEM reset leaves them enabled with an
     * address pointer of 0 for some unfathomable reason... This screws things
     * up when we enable transmission or reception.
     *
     * Setting b0 of the queue pointer disables a queue.
     */
#if (MSS_MAC_QUEUE_COUNT > 1)
    if(0U == this_mac->is_emac)
    {     /* Added these to MAC structure to make them MAC specific... */
        this_mac->mac_base->TRANSMIT_Q1_PTR = ((uint32_t)(uint64_t)this_mac->queue[0].tx_desc_tab) | 1U; /* Use address of valid descriptor but set b0 to disable */
        this_mac->mac_base->TRANSMIT_Q2_PTR = ((uint32_t)(uint64_t)this_mac->queue[0].tx_desc_tab) | 1U;
        this_mac->mac_base->TRANSMIT_Q3_PTR = ((uint32_t)(uint64_t)this_mac->queue[0].tx_desc_tab) | 1U;
        this_mac->mac_base->RECEIVE_Q1_PTR  = ((uint32_t)(uint64_t)this_mac->queue[0].rx_desc_tab) | 1U;
        this_mac->mac_base->RECEIVE_Q2_PTR  = ((uint32_t)(uint64_t)this_mac->queue[0].rx_desc_tab) | 1U;
        this_mac->mac_base->RECEIVE_Q3_PTR  = ((uint32_t)(uint64_t)this_mac->queue[0].rx_desc_tab) | 1U;
    }
#endif

    /* Set up maximum initial jumbo frame size - but bounds check first  */
    if(cfg->jumbo_frame_default > MSS_MAC_JUMBO_MAX)
    {
        temp_length = MSS_MAC_JUMBO_MAX;
    }
    else
    {
        temp_length = cfg->jumbo_frame_default;
    }

    if(0U != this_mac->is_emac)
    {
        this_mac->emac_base->JUMBO_MAX_LENGTH = temp_length;
    }
    else
    {
        this_mac->mac_base->JUMBO_MAX_LENGTH = temp_length;
    }

    /*--------------------------------------------------------------------------
     * Disable all ints for now
     */
    if(0U != this_mac->is_emac)
    {
        this_mac->emac_base->INT_DISABLE = ((uint32_t)0xFFFFFFFFUL); /* Only one queue here... */
    }
    else
    {
        int32_t queue_no;
        for(queue_no = 0; queue_no < MSS_MAC_QUEUE_COUNT; queue_no++)
        {
            *(this_mac->queue[queue_no].int_disable) = ((uint32_t)0xFFFFFFFFUL);
        }
    }

}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */
void
MSS_MAC_write_phy_reg
(
    const mss_mac_instance_t *this_mac,
    uint8_t phyaddr,
    uint8_t regaddr,
    uint16_t regval
)
{
    volatile uint32_t phy_op;
    psr_t lev;

    ASSERT(MSS_MAC_PHYADDR_MAXVAL >= phyaddr);
    ASSERT(MSS_MAC_PHYREGADDR_MAXVAL >= regaddr);
    /*
     * Write PHY address in MII Mgmt address register.
     * Makes previous register address 0 & invalid.
     *
     * Don't check mac_available flag here as we may be called in the MAC init
     * phase before everything else is in place...
     */
    if((MSS_MAC_PHYADDR_MAXVAL >= phyaddr) &&
       (MSS_MAC_PHYREGADDR_MAXVAL >= regaddr))
    {
        phy_op = GEM_WRITE1 | (GEM_PHY_OP_CL22_WRITE << GEM_OPERATION_SHIFT) | (((uint32_t)(2UL)) << GEM_WRITE10_SHIFT) | (uint32_t)regval;
        phy_op |= ((uint32_t)phyaddr << GEM_PHY_ADDRESS_SHIFT) & GEM_PHY_ADDRESS;
        phy_op |= ((uint32_t)regaddr << GEM_REGISTER_ADDRESS_SHIFT) & GEM_REGISTER_ADDRESS;

        lev = HAL_disable_interrupts();
        /*
         * Always use the pMAC for this as the eMAC MDIO interface is not
         * connected to the outside world...
         */
        /* Wait for MII Mgmt interface to complete previous operation. */
        do
        {
            volatile int32_t ix;
            ix++;
        } while(0U == (this_mac->mac_base->NETWORK_STATUS & GEM_MAN_DONE));

        this_mac->mac_base->PHY_MANAGEMENT = phy_op;
        HAL_restore_interrupts(lev);
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */
uint16_t
MSS_MAC_read_phy_reg
(
    const mss_mac_instance_t *this_mac,
    uint8_t phyaddr,
    uint8_t regaddr
)
{
    volatile uint32_t phy_op;
    psr_t lev;

    ASSERT(MSS_MAC_PHYADDR_MAXVAL >= phyaddr);
    ASSERT(MSS_MAC_PHYREGADDR_MAXVAL >= regaddr);
    /*
     * Write PHY address in MII Mgmt address register.
     * Makes previous register address 0 & invalid.
     *
     * Don't check mac_available flag here as we may be called in the MAC init
     * phase before everything else is in place...
     */
    if((MSS_MAC_PHYADDR_MAXVAL >= phyaddr) &&
       (MSS_MAC_PHYREGADDR_MAXVAL >= regaddr))
    {
        phy_op = GEM_WRITE1 | (GEM_PHY_OP_CL22_READ << GEM_OPERATION_SHIFT) | (((uint32_t)(2UL)) << GEM_WRITE10_SHIFT);
        phy_op |= ((uint32_t)phyaddr << GEM_PHY_ADDRESS_SHIFT) & GEM_PHY_ADDRESS;
        phy_op |= ((uint32_t)regaddr << GEM_REGISTER_ADDRESS_SHIFT) & GEM_REGISTER_ADDRESS;

        /*
         * Always use the pMAC for this as the eMAC MDIO interface is not
         * connected to the outside world...
         */
        lev = HAL_disable_interrupts();
        /* Wait for MII Mgmt interface to complete previous operation. */
        do
        {
            volatile int32_t ix;
            ix++;
        } while(0U == (this_mac->mac_base->NETWORK_STATUS & GEM_MAN_DONE));

        this_mac->mac_base->PHY_MANAGEMENT = phy_op;

        do
        {
            volatile int32_t ix;
            ix++;
        } while(0U == (this_mac->mac_base->NETWORK_STATUS & GEM_MAN_DONE));

        phy_op = this_mac->mac_base->PHY_MANAGEMENT;
        HAL_restore_interrupts(lev);
    }
    else
    {
        phy_op = 0U;
    }

    return((uint16_t)phy_op);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

/* Divide by 4 as offset is in bytes but pointer is 4 bytes */
#define GEM_REG_OFFSET(x) (offsetof(MAC_TypeDef, x) / 4)

uint32_t
MSS_MAC_read_stat
(
    const mss_mac_instance_t *this_mac,
    mss_mac_stat_t stat
)
{
    uint32_t stat_val = 0u;

    static uint64_t const stat_regs_lut[] =
    {
        GEM_REG_OFFSET(OCTETS_TXED_BOTTOM),
        GEM_REG_OFFSET(OCTETS_TXED_TOP),
        GEM_REG_OFFSET(FRAMES_TXED_OK),
        GEM_REG_OFFSET(BROADCAST_TXED),
        GEM_REG_OFFSET(MULTICAST_TXED),
        GEM_REG_OFFSET(PAUSE_FRAMES_TXED),
        GEM_REG_OFFSET(FRAMES_TXED_64),
        GEM_REG_OFFSET(FRAMES_TXED_65),
        GEM_REG_OFFSET(FRAMES_TXED_128),
        GEM_REG_OFFSET(FRAMES_TXED_256),
        GEM_REG_OFFSET(FRAMES_TXED_512),
        GEM_REG_OFFSET(FRAMES_TXED_1024),
        GEM_REG_OFFSET(FRAMES_TXED_1519),
        GEM_REG_OFFSET(TX_UNDERRUNS),
        GEM_REG_OFFSET(SINGLE_COLLISIONS),
        GEM_REG_OFFSET(MULTIPLE_COLLISIONS),
        GEM_REG_OFFSET(EXCESSIVE_COLLISIONS),
        GEM_REG_OFFSET(LATE_COLLISIONS),
        GEM_REG_OFFSET(DEFERRED_FRAMES),
        GEM_REG_OFFSET(CRS_ERRORS),
        GEM_REG_OFFSET(OCTETS_RXED_BOTTOM),
        GEM_REG_OFFSET(OCTETS_RXED_TOP),
        GEM_REG_OFFSET(FRAMES_RXED_OK),
        GEM_REG_OFFSET(BROADCAST_RXED),
        GEM_REG_OFFSET(MULTICAST_RXED),
        GEM_REG_OFFSET(PAUSE_FRAMES_RXED),
        GEM_REG_OFFSET(FRAMES_RXED_64),
        GEM_REG_OFFSET(FRAMES_RXED_65),
        GEM_REG_OFFSET(FRAMES_RXED_128),
        GEM_REG_OFFSET(FRAMES_RXED_256),
        GEM_REG_OFFSET(FRAMES_RXED_512),
        GEM_REG_OFFSET(FRAMES_RXED_1024),
        GEM_REG_OFFSET(FRAMES_RXED_1519),
        GEM_REG_OFFSET(UNDERSIZE_FRAMES),
        GEM_REG_OFFSET(EXCESSIVE_RX_LENGTH),
        GEM_REG_OFFSET(RX_JABBERS),
        GEM_REG_OFFSET(FCS_ERRORS),
        GEM_REG_OFFSET(RX_LENGTH_ERRORS),
        GEM_REG_OFFSET(RX_SYMBOL_ERRORS),
        GEM_REG_OFFSET(ALIGNMENT_ERRORS),
        GEM_REG_OFFSET(RX_RESOURCE_ERRORS),
        GEM_REG_OFFSET(RX_OVERRUNS),
        GEM_REG_OFFSET(RX_IP_CK_ERRORS),
        GEM_REG_OFFSET(RX_TCP_CK_ERRORS),
        GEM_REG_OFFSET(RX_UDP_CK_ERRORS),
        GEM_REG_OFFSET(AUTO_FLUSHED_PKTS)
    };

    ASSERT(MSS_MAC_LAST_STAT > stat);

    if((MSS_MAC_LAST_STAT > stat) && (MSS_MAC_AVAILABLE == this_mac->mac_available))
    {
        if(0U != this_mac->is_emac)
        {
            stat_val = *(((uint32_t *)this_mac->emac_base) + stat_regs_lut[stat]);
        }
        else
        {
            stat_val = *(((uint32_t *)this_mac->mac_base) + stat_regs_lut[stat]);
        }
    }

    return stat_val;
}


/*******************************************************************************
 See mss_ethernet_mac.h for details of how to use this function
*/
void MSS_MAC_clear_statistics
(
    const mss_mac_instance_t *this_mac
)
{
    if((0U != this_mac->is_emac) && (MSS_MAC_AVAILABLE == this_mac->mac_available))
    {
        this_mac->emac_base->NETWORK_CONTROL |= GEM_CLEAR_ALL_STATS_REGS;
    }
    else
    {
        this_mac->mac_base->NETWORK_CONTROL |= GEM_CLEAR_ALL_STATS_REGS;
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */
uint8_t
MSS_MAC_receive_pkt
(
    mss_mac_instance_t *this_mac,
    uint32_t queue_no,
    uint8_t * rx_pkt_buffer,
    void * p_user_data,
    mss_mac_rx_int_ctrl_t enable
)
{
    uint8_t status = MSS_MAC_FAILED;
    /* Make this function atomic w.r.to EMAC interrupt */

    /* PLIC_DisableIRQ() et al should not be called from the associated interrupt... */
    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U == this_mac->queue[queue_no].in_isr)
        {
            if(0U != this_mac->use_local_ints)
            {
                __disable_local_irq(this_mac->mac_q_int[queue_no]);
            }
            else
            {
                PLIC_DisableIRQ(this_mac->mac_q_int[queue_no]); /* Single interrupt from GEM? */
            }
        }

        ASSERT(NULL_POINTER != rx_pkt_buffer);
        ASSERT(IS_WORD_ALIGNED(rx_pkt_buffer));

        if(this_mac->queue[queue_no].nb_available_rx_desc > 0U)
        {
            uint32_t next_rx_desc_index;

            if(MSS_MAC_INT_DISABLE == enable)
            {
                /*
                 * When setting up the chain of buffers, we don't want the DMA
                 * engine active so shut down reception.
                 */
                if(0U != this_mac->is_emac)
                {
                    this_mac->emac_base->NETWORK_CONTROL &= ~GEM_ENABLE_RECEIVE;
                }
                else
                {
                    this_mac->mac_base->NETWORK_CONTROL &= ~GEM_ENABLE_RECEIVE;
                }
            }

            --this_mac->queue[queue_no].nb_available_rx_desc;
            next_rx_desc_index = this_mac->queue[queue_no].next_free_rx_desc_index;

            if((MSS_MAC_RX_RING_SIZE - 1U) == next_rx_desc_index)
            {
                this_mac->queue[queue_no].rx_desc_tab[next_rx_desc_index].addr_low = (uint32_t)((uint64_t)rx_pkt_buffer | 2UL);  /* Set wrap bit */
            }
            else
            {
                this_mac->queue[queue_no].rx_desc_tab[next_rx_desc_index].addr_low = (uint32_t)((uint64_t)rx_pkt_buffer);
            }

#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
            this_mac->queue[queue_no].rx_desc_tab[next_rx_desc_index].addr_high = (uint32_t)((uint64_t)rx_pkt_buffer >> 32);
#endif
            this_mac->queue[queue_no].rx_caller_info[next_rx_desc_index] = p_user_data;

            /*
               If the RX is found disabled, then it might be because this is the
               first time a packet is scheduled for reception or the RX ENABLE is
               made zero by RX overflow or RX bus error. In either case, this
               function tries to schedule the current packet for reception.

               Don't bother if we are not enabling interrupt at the end of all this...
            */
            if(MSS_MAC_INT_ARM == enable)
            {
                /*
                 * Starting receive operations off with a chain of buffers set up.
                 */

                if(0U != this_mac->is_emac)
                {
                    this_mac->emac_base->NETWORK_CONTROL &= ~GEM_ENABLE_RECEIVE;
                    this_mac->emac_base->RECEIVE_Q_PTR = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab);
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
                    this_mac->emac_base->UPPER_RX_Q_BASE_ADDR = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab >> 32);
#endif
                    this_mac->emac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
                }
                else
                {
                    this_mac->mac_base->NETWORK_CONTROL &= ~GEM_ENABLE_RECEIVE;
                    *(this_mac->queue[queue_no].receive_q_ptr) = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab);
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
                    this_mac->mac_base->UPPER_RX_Q_BASE_ADDR = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab >> 32);
#endif
                    this_mac->mac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
                }
            }
            else
            {
                if(MSS_MAC_INT_DISABLE != enable)
                {
                    uint32_t temp_cr;

                    if(0U != this_mac->is_emac)
                    {
                        temp_cr = this_mac->emac_base->NETWORK_CONFIG;
                    }
                    else
                    {
                        temp_cr = this_mac->mac_base->NETWORK_CONFIG;
                    }
                    if(0U == (temp_cr & GEM_ENABLE_RECEIVE))
                    {
                        /* RX disabled so restart it... */
                        if(0U != this_mac->is_emac)
                        {
                            this_mac->emac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
                        }
                        else
                        {
                            this_mac->mac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
                        }
                    }
                }
            }

            /* Point the next_rx_desc to next free descriptor in the ring */
            /* Wrap around in case next descriptor is pointing to last in the ring */
            ++this_mac->queue[queue_no].next_free_rx_desc_index;
            this_mac->queue[queue_no].next_free_rx_desc_index %= MSS_MAC_RX_RING_SIZE;

        }

        /*
         * Only call Ethernet Interrupt Enable function if the user says so.
         * See note above for disable...
         */
        if((MSS_MAC_INT_ARM == enable) && (0U == this_mac->queue[queue_no].in_isr))
        {
            if(0U != this_mac->use_local_ints)
            {
                __enable_local_irq(this_mac->mac_q_int[queue_no]);
            }
            else
            {
                PLIC_EnableIRQ(this_mac->mac_q_int[queue_no]); /* Single interrupt from GEM? */
            }
        }
    }

    return status;
}


/*******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */
uint8_t
MSS_MAC_send_pkt
(
    mss_mac_instance_t *this_mac,
    uint32_t queue_no,
    uint8_t const * tx_buffer,
    uint32_t length,
    void * p_user_data
)
{
    /*
     * Simplified transmit operation which depends on the following assumptions:
     *
     * 1. The TX DMA buffer size is big enough to contain a full packet.
     * 2. We will only transmit one packet at a time.
     *
     * We do transmission by using two buffer descriptors. The first contains the
     * packet to transmit and the second is a dummy one with the USED bit set. This
     * halts transmission once the first packet is transmitted. We always reset the
     * TX DMA to point to the first packet when we send a packet so we don't have
     * to juggle buffer positions or worry about wrap.
     */
    uint32_t tx_length = length;
    uint8_t status = MSS_MAC_FAILED;
    /* Hack for system testing. If b31 of the tx_length is 1 then we want to
     * send without a CRC appended. This is used for loopback testing where we
     * can get the GEM to receive packets with CRC appended and send them
     * straight back.
     * */
    int32_t no_crc = 0;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        /* Is config option for disabling CRC set? */
        if(MSS_MAC_CRC_DISABLE == this_mac->append_CRC)
        {
            no_crc = 1;
        }

        /* Or has the user requested just this packet? */
        if(0U != (tx_length & 0x80000000U))
        {
            no_crc = 1;
        }

        tx_length &= 0x7FFFFFFFU; /* Make sure high bit is now clear */

        /* Make this function atomic w.r.to EMAC interrupt */
        /* PLIC_DisableIRQ() et al should not be called from the associated interrupt... */
        if(0U == this_mac->queue[queue_no].in_isr)
        {
            if(0U != this_mac->use_local_ints)
            {
                __disable_local_irq(this_mac->mac_q_int[queue_no]);
            }
            else
            {
                PLIC_DisableIRQ(this_mac->mac_q_int[queue_no]); /* Single interrupt from GEM? */
            }
        }

        ASSERT(NULL_POINTER != tx_buffer);
        ASSERT(0U != tx_length);
        ASSERT(IS_WORD_ALIGNED(tx_buffer));

#if defined(MSS_MAC_SIMPLE_TX_QUEUE)
        if(this_mac->queue[queue_no].nb_available_tx_desc == (uint32_t)MSS_MAC_TX_RING_SIZE)
        {
            this_mac->queue[queue_no].nb_available_tx_desc    = 0U;
            this_mac->queue[queue_no].tx_desc_tab[0].addr_low = (uint32_t)((uint64_t)tx_buffer);
            this_mac->queue[queue_no].tx_desc_tab[0].status   = (tx_length & GEM_TX_DMA_BUFF_LEN) | GEM_TX_DMA_LAST; /* Mark as last buffer for frame */
            if(0 != no_crc)
            {
                this_mac->queue[queue_no].tx_desc_tab[0].status |= GEM_TX_DMA_NO_CRC;
            }
#if 0
            this_mac->tx_desc_tab[0].status = (tx_length & GEM_TX_DMA_BUFF_LEN) | GEM_TX_DMA_LAST | GEM_TX_DMA_USED; /* PMCS deliberate error ! */
#endif
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
            this_mac->queue[queue_no].tx_desc_tab[0].addr_high = (uint32_t)((uint64_t)tx_buffer >> 32);
            this_mac->queue[queue_no].tx_desc_tab[0].unused    = 0U;
#endif
            this_mac->queue[queue_no].tx_caller_info[0] = p_user_data;

            if(0U != this_mac->is_emac)
            {
                this_mac->emac_base->NETWORK_CONTROL |= GEM_ENABLE_TRANSMIT | GEM_TRANSMIT_HALT;
                this_mac->emac_base->TRANSMIT_Q_PTR   = (uint32_t)((uint64_t)this_mac->queue[queue_no].tx_desc_tab);
                this_mac->emac_base->NETWORK_CONTROL |= GEM_TRANSMIT_START;
            }
            else
            {
                this_mac->mac_base->NETWORK_CONTROL       |= GEM_ENABLE_TRANSMIT | GEM_TRANSMIT_HALT;
                *this_mac->queue[queue_no].transmit_q_ptr  = (uint32_t)((uint64_t)this_mac->queue[queue_no].tx_desc_tab);
                this_mac->mac_base->NETWORK_CONTROL       |= GEM_TRANSMIT_START;
            }

            this_mac->queue[queue_no].egress += tx_length;
            status = MSS_MAC_SUCCESS;
        }
#else
        /* TBD PMCS need to implement multi packet queuing... */
#warning "Nothing implemented for multi packet tx yet"
#endif
        /* Ethernet Interrupt Enable function. */
        /* PLIC_DisableIRQ() et al should not be called from the associated interrupt... */
        if(0U == this_mac->queue[queue_no].in_isr)
        {
            if(0U != this_mac->use_local_ints)
            {
                __enable_local_irq(this_mac->mac_q_int[queue_no]);
            }
            else
            {
                PLIC_EnableIRQ(this_mac->mac_q_int[queue_no]); /* Single interrupt from GEM? */
            }
        }
    }
    return status;
}


/******************************************************************************
 *
 */
#if defined(USING_LWIP)
extern BaseType_t g_mac_context_switch;
#endif

#if defined(USING_FREERTOS)
extern UBaseType_t uxCriticalNesting;
#endif
#if defined(TARGET_ALOE)
uint8_t  MAC0_plic_53_IRQHandler(void);
uint8_t  MAC0_plic_53_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 0U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 0U);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}
#else


/******************************************************************************
 *
 */
uint8_t mac0_int_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 0U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 0U);
#endif

    return(EXT_IRQ_KEEP_ENABLED);
}


/******************************************************************************
 *
 */
uint8_t mac0_queue1_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 1U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 1U);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}


/******************************************************************************
 *
 */
uint8_t mac0_queue2_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 2U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 2U);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}


/******************************************************************************
 *
 */
uint8_t mac0_queue3_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 3U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 3U);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}


/******************************************************************************
 *
 */
uint8_t mac0_emac_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_emac0, 0U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_emac0, 0U);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}


/******************************************************************************
 *
 */
uint8_t mac0_mmsl_plic_IRQHandler(void)
{
    return(EXT_IRQ_KEEP_ENABLED);
}


/******************************************************************************
 *
 */
uint8_t mac1_int_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 0U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 0U);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}


/******************************************************************************
 *
 */
uint8_t mac1_queue1_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 1U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 1U);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}


/******************************************************************************
 *
 */
uint8_t mac1_queue2_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 2U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 2U);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}


/******************************************************************************
 *
 */
uint8_t mac1_queue3_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 3U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 3U);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}


/******************************************************************************
 *
 */
uint8_t mac1_emac_plic_IRQHandler(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_emac1, 0U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_emac1, 0U);
#endif
    return(EXT_IRQ_KEEP_ENABLED);
}


/******************************************************************************
 *
 */
uint8_t mac1_mmsl_plic_IRQHandler(void)
{
    return(EXT_IRQ_KEEP_ENABLED);
}


/******************************************************************************
 *
 */
/* U54 1 */
void mac_mmsl_u54_1_local_IRQHandler_3(void)
{
}


/******************************************************************************
 *
 */
void mac_emac_u54_1_local_IRQHandler_4(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_emac0, 0U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_emac0, 0U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_queue3_u54_1_local_IRQHandler_5(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 3U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 3U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_queue2_u54_1_local_IRQHandler_6(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 2U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 2U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_queue1_u54_1_local_IRQHandler_7(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 1U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 1U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_int_u54_1_local_IRQHandler_8(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 0U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 0U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
/* U54 2 */
void mac_mmsl_u54_2_local_IRQHandler_3(void)
{
}


/******************************************************************************
 *
 */
void mac_emac_u54_2_local_IRQHandler_4(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_emac0, 0U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_emac0, 0U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_queue3_u54_2_local_IRQHandler_5(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 3U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 3U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_queue2_u54_2_local_IRQHandler_6(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 2U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 2U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_queue1_u54_2_local_IRQHandler_7(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 1U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 1U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_int_u54_2_local_IRQHandler_8(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac0, 0U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac0, 0U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
/* U54 3 */
void mac_mmsl_u54_3_local_IRQHandler_3(void)
{
}


/******************************************************************************
 *
 */
void mac_emac_u54_3_local_IRQHandler_4(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_emac1, 0U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_emac1, 0U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_queue3_u54_3_local_IRQHandler_5(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 3U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 3U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_queue2_u54_3_local_IRQHandler_6(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 2U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 2U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_queue1_u54_3_local_IRQHandler_7(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 1U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 1U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_int_u54_3_local_IRQHandler_8(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 0U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 0U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
/* U54 4 */
void mac_mmsl_u54_4_local_IRQHandler_3(void)
{
}


/******************************************************************************
 *
 */
void mac_emac_u54_4_local_IRQHandler_4(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_emac1, 0U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_emac1, 0U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_queue3_u54_4_local_IRQHandler_5(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 3U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 3U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_queue2_u54_4_local_IRQHandler_6(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 2U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 2U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_queue1_u54_4_local_IRQHandler_7(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 1U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 1U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}


/******************************************************************************
 *
 */
void mac_int_u54_4_local_IRQHandler_8(void)
{
#if defined(USING_FREERTOS)
    uxCriticalNesting++;
    generic_mac_irq_handler(&g_mac1, 0U);
    uxCriticalNesting--;
#else
    generic_mac_irq_handler(&g_mac1, 0U);
#endif
#if defined(USING_LWIP)
    if(0 != g_mac_context_switch)
    {
        g_mac_context_switch = 0;
        vPortYieldISR();
    }
#endif
}

#endif


/* Define the following if your GEM is configured to clear on read for int flags
 * In this case you should not write to the int status reg ... */
/* #define GEM_FLAGS_CLR_ON_RD */

static void generic_mac_irq_handler(mss_mac_instance_t *this_mac, uint32_t queue_no)
{
    volatile uint32_t int_pending;  /* We read and hold a working copy as many
                                     * of the bits can be clear on read if
                                     * the GEM is configured that way... */
    volatile uint32_t *rx_status;   /* Address of receive status register */
    volatile uint32_t *tx_status;   /* Address of transmit status register */
    volatile uint32_t *int_status;  /* Address of interrupt status register */

    this_mac->queue[queue_no].in_isr = 1U;
    int_status  = this_mac->queue[queue_no].int_status;
    int_pending =  *int_status & ~(*this_mac->queue[queue_no].int_mask);

    if(0U != this_mac->is_emac)
    {
        rx_status   = &this_mac->emac_base->RECEIVE_STATUS;
        tx_status   = &this_mac->emac_base->TRANSMIT_STATUS;
    }
    else
    {
        rx_status   = &this_mac->mac_base->RECEIVE_STATUS;
        tx_status   = &this_mac->mac_base->TRANSMIT_STATUS;
    }

    /*
     * Note, in the following code we generally clear any flags first and then
     * handle the condition as this allows any new events which occur in the
     * course of the ISR to be picked up later.
     */

    /* Packet received interrupt - first in line as most time critical */
    if((int_pending & GEM_RECEIVE_COMPLETE) != 0U)
    {
       *rx_status = GEM_FRAME_RECEIVED;
#if !defined(GEM_FLAGS_CLR_ON_RD)
#if defined(TARGET_ALOE)
        *int_status = (uint32_t)3U; /* PMCS: Should be 2 but that does not work on Aloe... */
#else
        *int_status = (uint32_t)2U;
#endif
        rxpkt_handler(this_mac, queue_no);
        this_mac->queue[queue_no].overflow_counter = 0U; /* Reset counter as we have received something */
#endif
    }

    if((int_pending & GEM_RECEIVE_OVERRUN_INT) != 0U)
    {
        *rx_status = GEM_RECEIVE_OVERRUN;
#if !defined(GEM_FLAGS_CLR_ON_RD)
        *int_status = GEM_RECEIVE_OVERRUN_INT;
#endif
        rxpkt_handler(this_mac, queue_no);
        this_mac->queue[queue_no].overflow_counter++;
        this_mac->queue[queue_no].rx_overflow++;
    }

    if((int_pending & GEM_RX_USED_BIT_READ) != 0U)
    {
        *rx_status = GEM_BUFFER_NOT_AVAILABLE;
#if !defined(GEM_FLAGS_CLR_ON_RD)
        *int_status = GEM_RX_USED_BIT_READ;
#endif
        rxpkt_handler(this_mac, queue_no);
        this_mac->queue[queue_no].rx_overflow++;
        this_mac->queue[queue_no].overflow_counter++;
    }

    if((int_pending & GEM_RESP_NOT_OK_INT) != 0U) /* Hope this is transient and restart rx... */
    {
        *rx_status = GEM_RX_RESP_NOT_OK;
#if !defined(GEM_FLAGS_CLR_ON_RD)
        *int_status = GEM_RESP_NOT_OK_INT;
#endif
        rxpkt_handler(this_mac, queue_no);
        if(0U != this_mac->is_emac)
        {
            this_mac->emac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
        }
        else
        {
            this_mac->mac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
        }

        this_mac->queue[queue_no].hresp_error++;
    }

    /* Transmit packet sent interrupt */
    if((int_pending & GEM_TRANSMIT_COMPLETE) != 0U)
    {
        if((*tx_status & GEM_STAT_TRANSMIT_COMPLETE) != 0U) /* If loopback test or other hasn't taken care of this... */
        {
            *tx_status  = GEM_STAT_TRANSMIT_COMPLETE;
#if !defined(GEM_FLAGS_CLR_ON_RD)
            *int_status = GEM_TRANSMIT_COMPLETE;
#endif
            txpkt_handler(this_mac, queue_no);
        }
    }

    if(this_mac->queue[queue_no].overflow_counter > 4U) /* looks like we are stuck in a rut here... */
    {
        uint32_t descriptor;
        /* Restart receive operation from scratch */
        this_mac->queue[queue_no].overflow_counter = 0U;
        this_mac->queue[queue_no].rx_restart++;

        if(0U != this_mac->is_emac)
        {
            this_mac->emac_base->NETWORK_CONTROL &= ~GEM_ENABLE_RECEIVE;
        }
        else
        {
            this_mac->mac_base->NETWORK_CONTROL &= ~GEM_ENABLE_RECEIVE;
        }

        this_mac->queue[queue_no].nb_available_rx_desc = MSS_MAC_RX_RING_SIZE;
        this_mac->queue[queue_no].next_free_rx_desc_index = 0U;
        this_mac->queue[queue_no].first_rx_desc_index = 0U;

        for(descriptor = 0U; descriptor < MSS_MAC_RX_RING_SIZE; descriptor++) /* Discard everything */
        {
            this_mac->queue[queue_no].rx_desc_tab[descriptor].addr_low &= GEM_RX_DMA_USED;
        }

        if(0U != this_mac->is_emac)
        {
            this_mac->emac_base->RECEIVE_Q_PTR = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab);
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
            this_mac->emac_base->UPPER_RX_Q_BASE_ADDR = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab >> 32);
#endif
            this_mac->emac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
        }
        else
        {
            this_mac->mac_base->RECEIVE_Q_PTR = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab);
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
            this_mac->mac_base->UPPER_RX_Q_BASE_ADDR = (uint32_t)((uint64_t)this_mac->queue[queue_no].rx_desc_tab >> 32);
#endif
            this_mac->mac_base->NETWORK_CONTROL |= GEM_ENABLE_RECEIVE;
        }
    }

    if((int_pending & GEM_PAUSE_FRAME_TRANSMITTED) != 0U)
    {
        *int_status = GEM_PAUSE_FRAME_TRANSMITTED;
        this_mac->tx_pause++;
    }
    if((int_pending & GEM_PAUSE_TIME_ELAPSED) != 0U)
    {
        *int_status = GEM_PAUSE_TIME_ELAPSED;
        this_mac->pause_elapsed++;
    }
    if((int_pending & GEM_PAUSE_FRAME_WITH_NON_0_PAUSE_QUANTUM_RX) != 0U)
    {
        *int_status = GEM_PAUSE_FRAME_WITH_NON_0_PAUSE_QUANTUM_RX;
        this_mac->rx_pause++;
    }

    /* Mask off checked ints and see if any left pending */
    int_pending &= ~(GEM_RECEIVE_OVERRUN_INT | GEM_RX_USED_BIT_READ | GEM_RECEIVE_COMPLETE |
                     GEM_TRANSMIT_COMPLETE | GEM_RESP_NOT_OK_INT | GEM_PAUSE_FRAME_TRANSMITTED |
                     GEM_PAUSE_TIME_ELAPSED | GEM_PAUSE_FRAME_WITH_NON_0_PAUSE_QUANTUM_RX);
    if(0U != int_pending)
    {
        if((int_pending & GEM_AMBA_ERROR) != 0U)
        {
            *int_status = (uint32_t)0x60U; /* Should be 0x40 but that doesn't clear it... */
            *tx_status  = GEM_STAT_AMBA_ERROR;
            this_mac->queue[queue_no].tx_amba_errors++;
            this_mac->queue[queue_no].nb_available_tx_desc = MSS_MAC_TX_RING_SIZE;
        }
        else
        {
            volatile int32_t index;
            ASSERT(0); /* Need to think about how to deal with this... */
            while(1)
            {
                index++;
            }
        }
    }

    this_mac->queue[queue_no].in_isr = 0U;
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */
void MSS_MAC_set_tx_callback
(
    mss_mac_instance_t *this_mac,
    uint32_t queue_no,
    mss_mac_transmit_callback_t tx_complete_handler
)
{
    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        this_mac->queue[queue_no].pckt_tx_callback = tx_complete_handler;
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */
void MSS_MAC_set_rx_callback
(
    mss_mac_instance_t *this_mac,
    uint32_t queue_no,
    mss_mac_receive_callback_t rx_callback
)
{
    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        this_mac->queue[queue_no].pckt_rx_callback = rx_callback;
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_init_TSU(const mss_mac_instance_t *this_mac, const mss_mac_tsu_config_t *tsu_cfg)
{
    uint32_t temp;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        temp  = (tsu_cfg->sub_ns_inc & 0xFFU) << 24;
        temp |= (tsu_cfg->sub_ns_inc >> 8) & 0xFFFFU;

        if(0U != this_mac->is_emac)
        {
#if 0 /* Shouldn't really allow setting of tsu through eMAC as it is slaved to pMAC TSU */
            this_mac->emac_base->TSU_TIMER_INCR_SUB_NSEC = temp;
            this_mac->emac_base->TSU_TIMER_INCR          = tsu_cfg->ns_inc;

            /* PMCS: I'm not 100% sure about the sequencing here... */

            this_mac->emac_base->TSU_TIMER_MSB_SEC       = tsu_cfg->secs_msb;
            this_mac->emac_base->TSU_TIMER_SEC           = tsu_cfg->secs_lsb;
            this_mac->emac_base->TSU_TIMER_NSEC          = tsu_cfg->nanoseconds;
#endif
        }
        else
        {
            this_mac->mac_base->TSU_TIMER_INCR_SUB_NSEC = temp;
            this_mac->mac_base->TSU_TIMER_INCR          = tsu_cfg->ns_inc;
            this_mac->mac_base->TSU_TIMER_MSB_SEC       = tsu_cfg->secs_msb;
            this_mac->mac_base->TSU_TIMER_SEC           = tsu_cfg->secs_lsb;
            this_mac->mac_base->TSU_TIMER_NSEC          = tsu_cfg->nanoseconds;
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_read_TSU(const mss_mac_instance_t *this_mac, mss_mac_tsu_time_t *tsu_time)
{
    int32_t got_time = 0;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        do
        {
            if(0U != this_mac->is_emac)
            {
                tsu_time->secs_lsb    = this_mac->emac_base->TSU_TIMER_SEC;
                tsu_time->secs_msb    = this_mac->emac_base->TSU_TIMER_MSB_SEC;
                tsu_time->nanoseconds = this_mac->emac_base->TSU_TIMER_NSEC;

                /* Check for nanoseconds roll over and exit loop if none otherwise do again */
                if(tsu_time->secs_lsb == this_mac->emac_base->TSU_TIMER_SEC)
                {
                    got_time = 1;
                }
            }
            else
            {
                tsu_time->secs_lsb    = this_mac->mac_base->TSU_TIMER_SEC;
                tsu_time->secs_msb    = this_mac->mac_base->TSU_TIMER_MSB_SEC;
                tsu_time->nanoseconds = this_mac->mac_base->TSU_TIMER_NSEC;

                /* Check for nanoseconds roll over and exit loop if none otherwise do again */
                if(tsu_time->secs_lsb == this_mac->mac_base->TSU_TIMER_SEC)
                {
                    got_time = 1;
                }
            }
        } while(0 == got_time);
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_TSU_rx_mode(const mss_mac_instance_t *this_mac, mss_mac_tsu_mode_t tsu_mode)
{
    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            this_mac->emac_base->RX_BD_CONTROL = ((uint32_t)tsu_mode << GEM_BD_TS_MODE_SHIFT) & GEM_BD_TS_MODE;
        }
        else
        {
            this_mac->mac_base->RX_BD_CONTROL = ((uint32_t)tsu_mode << GEM_BD_TS_MODE_SHIFT) & GEM_BD_TS_MODE;
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_TSU_tx_mode(const mss_mac_instance_t *this_mac, mss_mac_tsu_mode_t tsu_mode)
{
    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            this_mac->emac_base->TX_BD_CONTROL = ((uint32_t)tsu_mode << GEM_BD_TS_MODE_SHIFT) & GEM_BD_TS_MODE;
        }
        else
        {
            this_mac->mac_base->TX_BD_CONTROL = ((uint32_t)tsu_mode << GEM_BD_TS_MODE_SHIFT) & GEM_BD_TS_MODE;
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

mss_mac_tsu_mode_t MSS_MAC_get_TSU_rx_mode(const mss_mac_instance_t *this_mac)
{
    mss_mac_tsu_mode_t ret_val = MSS_MAC_TSU_MODE_DISABLED;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            ret_val = (mss_mac_tsu_mode_t)((this_mac->emac_base->RX_BD_CONTROL & GEM_BD_TS_MODE) >> GEM_BD_TS_MODE_SHIFT);
        }
        else
        {
            ret_val = (mss_mac_tsu_mode_t)((this_mac->mac_base->RX_BD_CONTROL & GEM_BD_TS_MODE) >> GEM_BD_TS_MODE_SHIFT);
        }
    }

    return(ret_val);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

mss_mac_tsu_mode_t MSS_MAC_get_TSU_tx_mode(const mss_mac_instance_t *this_mac)
{
    mss_mac_tsu_mode_t ret_val= MSS_MAC_TSU_MODE_DISABLED;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            ret_val = (mss_mac_tsu_mode_t)((this_mac->emac_base->TX_BD_CONTROL & GEM_BD_TS_MODE) >> GEM_BD_TS_MODE_SHIFT);
        }
        else
        {
            ret_val = (mss_mac_tsu_mode_t)((this_mac->mac_base->TX_BD_CONTROL & GEM_BD_TS_MODE) >> GEM_BD_TS_MODE_SHIFT);
        }
    }

    return(ret_val);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_TSU_oss_mode(const mss_mac_instance_t *this_mac, mss_mac_oss_mode_t oss_mode)
{
    volatile uint32_t temp_control;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            temp_control = this_mac->emac_base->NETWORK_CONTROL;
        }
        else
        {
            temp_control = this_mac->mac_base->NETWORK_CONTROL;
        }

        /*
         * Note. The docs don't say these are mutually exclusive but I don't think
         * it makes sense to allow both modes at once...
         */
        if(MSS_MAC_OSS_MODE_DISABLED == oss_mode)
        {
            temp_control &= ~(GEM_OSS_CORRECTION_FIELD | GEM_ONE_STEP_SYNC_MODE);
        }
        else if(MSS_MAC_OSS_MODE_REPLACE == oss_mode)
        {
            temp_control &= ~GEM_OSS_CORRECTION_FIELD;
            temp_control |= GEM_ONE_STEP_SYNC_MODE;
        }
        else
        {
            if(MSS_MAC_OSS_MODE_ADJUST == oss_mode)
            {
                temp_control |= GEM_OSS_CORRECTION_FIELD;
                temp_control &= ~GEM_ONE_STEP_SYNC_MODE;
            }
        }

        if(0U != this_mac->is_emac)
        {
            this_mac->emac_base->NETWORK_CONTROL = temp_control;
        }
        else
        {
            this_mac->mac_base->NETWORK_CONTROL = temp_control;
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

mss_mac_oss_mode_t MSS_MAC_get_TSU_oss_mode(const mss_mac_instance_t *this_mac)
{
    mss_mac_oss_mode_t ret_val = MSS_MAC_OSS_MODE_DISABLED;
    volatile uint32_t temp_control;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            temp_control = this_mac->emac_base->NETWORK_CONTROL;
        }
        else
        {
            temp_control = this_mac->mac_base->NETWORK_CONTROL;
        }

        /*
         * Note. The docs don't say these are mutually exclusive but I don't think
         * it makes sense to allow both modes at once so report this as invalid...
         */
        if((GEM_OSS_CORRECTION_FIELD | GEM_ONE_STEP_SYNC_MODE) == (temp_control & (GEM_OSS_CORRECTION_FIELD | GEM_ONE_STEP_SYNC_MODE)))
        {
            ret_val = MSS_MAC_OSS_MODE_INVALID;
        }
        else if(GEM_OSS_CORRECTION_FIELD == (temp_control & GEM_OSS_CORRECTION_FIELD))
        {
            ret_val = MSS_MAC_OSS_MODE_ADJUST;
        }
        else if(GEM_ONE_STEP_SYNC_MODE == (temp_control & GEM_ONE_STEP_SYNC_MODE))
        {
            ret_val = MSS_MAC_OSS_MODE_REPLACE;
        }
        else
        {
            ret_val = MSS_MAC_OSS_MODE_DISABLED;
        }
    }

    return(ret_val);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_TSU_unicast_addr(const mss_mac_instance_t *this_mac, mss_mac_tsu_addr_t select, uint32_t ip_address)
{
    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            if(MSS_MAC_TSU_UNICAST_RX == select)
            {
                this_mac->emac_base->RX_PTP_UNICAST = ip_address;
            }
            else
            {
                this_mac->emac_base->TX_PTP_UNICAST = ip_address;
            }
        }
        else
        {
            if(MSS_MAC_TSU_UNICAST_RX == select)
            {
                this_mac->mac_base->RX_PTP_UNICAST = ip_address;
            }
            else
            {
                this_mac->mac_base->TX_PTP_UNICAST = ip_address;
            }
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint32_t MSS_MAC_get_TSU_unicast_addr(const mss_mac_instance_t *this_mac, mss_mac_tsu_addr_t select)
{
    uint32_t ret_val = 0U;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            if(MSS_MAC_TSU_UNICAST_RX == select)
            {
                ret_val = this_mac->emac_base->RX_PTP_UNICAST;
            }
            else
            {
                ret_val = this_mac->emac_base->TX_PTP_UNICAST;
            }
        }
        else
        {
            if(MSS_MAC_TSU_UNICAST_RX == select)
            {
                ret_val = this_mac->mac_base->RX_PTP_UNICAST;
            }
            else
            {
                ret_val = this_mac->mac_base->TX_PTP_UNICAST;
            }
        }
    }

    return(ret_val);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_VLAN_only_mode(const mss_mac_instance_t *this_mac, bool enable)
{
    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            if(false == enable)
            {
                this_mac->emac_base->NETWORK_CONFIG &= ~GEM_DISCARD_NON_VLAN_FRAMES;
            }
            else
            {
                this_mac->emac_base->NETWORK_CONFIG |= GEM_DISCARD_NON_VLAN_FRAMES;
            }
        }
        else
        {
            if(false == enable)
            {
                this_mac->mac_base->NETWORK_CONFIG &= ~GEM_DISCARD_NON_VLAN_FRAMES;
            }
            else
            {
                this_mac->mac_base->NETWORK_CONFIG |= GEM_DISCARD_NON_VLAN_FRAMES;
            }
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

bool MSS_MAC_get_VLAN_only_mode(const mss_mac_instance_t *this_mac)
{
    bool ret_val = false;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            ret_val = 0U != (this_mac->emac_base->NETWORK_CONFIG & GEM_DISCARD_NON_VLAN_FRAMES);
        }
        else
        {
            ret_val = 0U != (this_mac->mac_base->NETWORK_CONFIG & GEM_DISCARD_NON_VLAN_FRAMES);
        }
    }

    return(ret_val);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_stacked_VLAN(const mss_mac_instance_t *this_mac, uint16_t tag)
{
    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            if(GEM_VLAN_ETHERTYPE_MIN > tag)
            {
                this_mac->emac_base->STACKED_VLAN = 0U;
            }
            else
            {
                this_mac->emac_base->STACKED_VLAN = (uint32_t)tag | GEM_ENABLE_PROCESSING;
            }
        }
        else
        {
            if(GEM_VLAN_ETHERTYPE_MIN > tag)
            {
                this_mac->mac_base->STACKED_VLAN = 0U;
            }
            else
            {
                this_mac->mac_base->STACKED_VLAN = (uint32_t)tag | GEM_ENABLE_PROCESSING;
            }
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint16_t MSS_MAC_get_stacked_VLAN(const mss_mac_instance_t *this_mac)
{
    uint16_t ret_val = GEM_VLAN_NO_STACK; /* Return 0 if stacked VLANs not enabled */

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            if(0U != (this_mac->emac_base->STACKED_VLAN & GEM_ENABLE_PROCESSING))
            {
                ret_val = (uint16_t)this_mac->emac_base->STACKED_VLAN;
            }
        }
        else
        {
            if(0U != (this_mac->mac_base->STACKED_VLAN & GEM_ENABLE_PROCESSING))
            {
                ret_val = (uint16_t)this_mac->mac_base->STACKED_VLAN;
            }
        }
    }

    return(ret_val);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_hash(const mss_mac_instance_t *this_mac, uint64_t hash_in)
{
    uint64_t hash = hash_in; /* Avoids warning about modifying fn parameter passed by value */

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            if(0ULL == hash) /* Short cut for disabling */
            {
                this_mac->emac_base->NETWORK_CONFIG &= ~(GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE);
            }

            this_mac->emac_base->HASH_BOTTOM = (uint32_t)hash;
            hash >>= 32;
            this_mac->emac_base->HASH_TOP = (uint32_t)hash;
        }
        else
        {
            if(0ULL == hash) /* Short cut for disabling */
            {
                this_mac->mac_base->NETWORK_CONFIG &= ~(GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE);
            }

            this_mac->mac_base->HASH_BOTTOM = (uint32_t)hash;
            hash >>= 32;
            this_mac->mac_base->HASH_TOP = (uint32_t)hash;
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint64_t MSS_MAC_get_hash(const mss_mac_instance_t *this_mac)
{
    uint64_t ret_val = 0U;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            ret_val   = (uint64_t)this_mac->emac_base->HASH_TOP;
            ret_val <<= 32;
            ret_val  |= (uint64_t)this_mac->emac_base->HASH_BOTTOM;
        }
        else
        {
            ret_val   = (uint64_t)this_mac->mac_base->HASH_TOP;
            ret_val <<= 32;
            ret_val  |= (uint64_t)this_mac->mac_base->HASH_BOTTOM;
        }
    }

    return(ret_val);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_hash_mode(const mss_mac_instance_t *this_mac, mss_mac_hash_mode_t mode)
{
    uint32_t temp;

    /*
     * Enum values are matched to register bits but just to be safe we mask them
     * to ensure only the two hash control bits are modified...
     */
    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            temp = this_mac->emac_base->NETWORK_CONFIG & ~(GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE);
            temp |= (uint32_t)mode & (GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE);
            this_mac->emac_base->NETWORK_CONFIG = temp;
        }
        else
        {
            temp = this_mac->mac_base->NETWORK_CONFIG & ~(GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE);
            temp |= (uint32_t)mode & (GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE);
            this_mac->mac_base->NETWORK_CONFIG = temp;
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

mss_mac_hash_mode_t MSS_MAC_get_hash_mode(const mss_mac_instance_t *this_mac)
{
    mss_mac_hash_mode_t ret_val = 0;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            ret_val = (mss_mac_hash_mode_t)(this_mac->emac_base->NETWORK_CONFIG & (GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE));
        }
        else
        {
            ret_val = (mss_mac_hash_mode_t)(this_mac->mac_base->NETWORK_CONFIG & (GEM_UNICAST_HASH_ENABLE | GEM_MULTICAST_HASH_ENABLE));
        }
    }

    return(ret_val);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_type_filter(const mss_mac_instance_t *this_mac, uint32_t filter, uint16_t value)
{
    volatile uint32_t *p_reg;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            p_reg = &this_mac->emac_base->SPEC_TYPE1;
        }
        else
        {
            p_reg = &this_mac->mac_base->SPEC_TYPE1;
        }
        if((filter <= 4U) && (filter > 0U)) /* Filter is in range 1 to 4 to match register naming */
        {
            p_reg += filter - 1U;
            if(0U == value) /* Disable filter if match value is 0 as this should not be a valid type */
            {
                *p_reg = 0U;
            }
            else
            {
                *p_reg = (uint32_t)(0x80000000UL | (uint32_t)value);
            }
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint16_t MSS_MAC_get_type_filter(const mss_mac_instance_t *this_mac, uint32_t filter)
{
    volatile uint32_t *p_reg;
    uint16_t ret_val = 0U;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            p_reg = &this_mac->emac_base->SPEC_TYPE1;
        }
        else
        {
            p_reg = &this_mac->mac_base->SPEC_TYPE1;
        }

        if((filter <= 4U) && (filter > 0U)) /* Filter is in range 1 to 4 to match register naming */
        {
            p_reg += filter - 1U;
            if(0U != (*p_reg & 0x80000000UL)) /* Not disabled filter... */
            {
                ret_val = (uint16_t)*p_reg;
            }
        }
    }

    return(ret_val);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_sa_filter(const mss_mac_instance_t *this_mac, uint32_t filter, uint16_t control, const uint8_t *mac_addr)
{
    volatile uint32_t *p_reg;
    uint32_t address32_l;
    uint32_t address32_h;

    ASSERT(NULL_POINTER!= mac_addr);

    if((MSS_MAC_AVAILABLE == this_mac->mac_available) && (NULL_POINTER != mac_addr))
    {
        if((filter >= 2U) && (filter <= 4U)) /* SA Filter 1 is for our address */
        {
            if(0U != this_mac->is_emac)
            {
                p_reg = &this_mac->emac_base->SPEC_ADD2_BOTTOM;
            }
            else
            {
                p_reg = &this_mac->mac_base->SPEC_ADD2_BOTTOM;
            }

            p_reg = p_reg + ((filter - 2U) * 2U);

            if(MSS_MAC_SA_FILTER_DISABLE == control)
            {
                /*
                 * Clear filter and disable - must be done in this order...
                 *  Writing to [0] disables and [1] enables So this sequence
                 *  clears the registers and disables the filter as opposed to
                 *  setting a destination address filter for 00:00:00:00:00:00.
                 */
                p_reg[1] = 0U;
                p_reg[0] = 0U;
            }
            else
            {
                /* Assemble correct register values */
                address32_l  = ((uint32_t)mac_addr[3]) << 24;
                address32_l |= ((uint32_t)mac_addr[2]) << 16;
                address32_l |= ((uint32_t)mac_addr[1]) << 8;
                address32_l |= ((uint32_t)mac_addr[0]);
                address32_h =  ((uint32_t)mac_addr[5]) << 8;
                address32_h |= ((uint32_t)mac_addr[4]);

                address32_h |= (uint32_t)control << 16;

                /* Update hardware registers - must be done in this order... */
                p_reg[0] = address32_l;
                p_reg[1] = address32_h;
            }
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint16_t MSS_MAC_get_sa_filter(const mss_mac_instance_t *this_mac, uint32_t filter, uint8_t *mac_addr)
{
    volatile uint32_t *p_reg;
    uint32_t temp_reg;
    uint16_t ret_val = 0U;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(NULL_POINTER != mac_addr)
        {
            (void)memset(mac_addr, 0, 6); /* Consistent result if bad parameters passed... */
        }

        if((filter >= 2U) && (filter <= 4U))
        {
            if(0U != this_mac->is_emac)
            {
                p_reg = &this_mac->emac_base->SPEC_ADD2_BOTTOM;
            }
            else
            {
                p_reg = &this_mac->mac_base->SPEC_ADD2_BOTTOM;
            }

            p_reg = p_reg + ((filter - 2U) * 2U);

            if(NULL_POINTER != mac_addr) /* Want MAC address and filter control info? */
            {
                temp_reg = p_reg[0];
                mac_addr[0] = (uint8_t)temp_reg & BITS_08;
                temp_reg >>= 8;
                mac_addr[1] = (uint8_t)temp_reg & BITS_08;
                temp_reg >>= 8;
                mac_addr[2] = (uint8_t)temp_reg & BITS_08;
                temp_reg >>= 8;
                mac_addr[3] = (uint8_t)temp_reg & BITS_08;

                temp_reg = p_reg[1];
                mac_addr[4] = (uint8_t)temp_reg & BITS_08;
                temp_reg >>= 8;
                mac_addr[5] = (uint8_t)temp_reg & BITS_08;
            }

            ret_val = (uint16_t)(p_reg[1] >> 16);
        }
    }

    return(ret_val);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_type_1_filter(const mss_mac_instance_t *this_mac, uint32_t filter_no, const mss_mac_type_1_filter_t *filter)
{
    if((MSS_MAC_AVAILABLE == this_mac->mac_available) && (filter_no < MSS_MAC_TYPE_1_SCREENERS))
    {
        volatile uint32_t *p_reg;
        uint32_t temp_reg;

        if(0U != this_mac->is_emac)
        {
            p_reg = &this_mac->emac_base->SCREENING_TYPE_1_REGISTER_0;
        }
        else
        {
            p_reg = &this_mac->mac_base->SCREENING_TYPE_1_REGISTER_0;
        }

        temp_reg  = (uint32_t)filter->queue_no & GEM_QUEUE_NUMBER;
        temp_reg |= ((uint32_t)filter->dstc << GEM_DSTC_MATCH_SHIFT) & GEM_DSTC_MATCH;
        temp_reg |= ((uint32_t)filter->udp_port << GEM_UDP_PORT_MATCH_SHIFT) & GEM_UDP_PORT_MATCH;
        if(0U != filter->drop_on_match)
        {
            temp_reg |= GEM_DROP_ON_MATCH;
        }

        if(0U != filter->dstc_enable)
        {
            temp_reg |= GEM_DSTC_ENABLE;
        }

        if(0U != filter->udp_port_enable)
        {
            temp_reg |= GEM_UDP_PORT_MATCH_ENABLE;
        }

        p_reg[filter_no] = temp_reg;
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_get_type_1_filter(const mss_mac_instance_t *this_mac, uint32_t filter_no, mss_mac_type_1_filter_t *filter)
{

    if((MSS_MAC_AVAILABLE == this_mac->mac_available) && (filter_no < MSS_MAC_TYPE_1_SCREENERS))
    {
        volatile uint32_t *p_reg;
        uint32_t temp_reg;

        (void)memset(filter, 0, sizeof(mss_mac_type_1_filter_t)); /* Blank canvass to start */

        if(0U != this_mac->is_emac)
        {
            p_reg = &this_mac->emac_base->SCREENING_TYPE_1_REGISTER_0;
        }
        else
        {
            p_reg = &this_mac->mac_base->SCREENING_TYPE_1_REGISTER_0;
        }

        temp_reg = p_reg[filter_no];

        filter->queue_no = (uint8_t)(temp_reg & GEM_QUEUE_NUMBER);
        temp_reg >>= 4;

        filter->dstc = (uint8_t)(temp_reg & GEM_DSTC_MATCH);
        temp_reg >>= GEM_DSTC_MATCH_SHIFT;

        filter->udp_port = (uint16_t)(temp_reg & GEM_UDP_PORT_MATCH);
        temp_reg >>= GEM_UDP_PORT_MATCH_SHIFT;

        filter->dstc_enable = (uint8_t)(temp_reg & 1U);
        temp_reg >>= 1;

        filter->udp_port_enable = (uint8_t)(temp_reg & 1U);
        temp_reg >>= 1;

        filter->drop_on_match = (uint8_t)(temp_reg & 1U);
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_type_2_ethertype(const mss_mac_instance_t *this_mac, uint32_t ethertype_no, uint16_t ethertype)
{
    volatile uint32_t *p_reg;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U == this_mac->is_emac) /* Ethertype filter not supported on eMAC */
        {
            p_reg = &this_mac->mac_base->SCREENING_TYPE_2_ETHERTYPE_REG_0;
            if(ethertype_no < MSS_MAC_TYPE_2_ETHERTYPES)
            {
                p_reg[ethertype_no] = (uint32_t)ethertype;
            }
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint16_t MSS_MAC_get_type_2_ethertype(const mss_mac_instance_t *this_mac, uint32_t ethertype_no)
{
    volatile uint32_t *p_reg;
    uint16_t temp_reg = 0U;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U == this_mac->is_emac) /* Ethertype filter not supported on eMAC */
        {
            p_reg = &this_mac->mac_base->SCREENING_TYPE_2_ETHERTYPE_REG_0;
            if(ethertype_no < MSS_MAC_TYPE_2_ETHERTYPES)
            {
                temp_reg = (uint16_t)p_reg[ethertype_no];
            }
        }
    }

    return(temp_reg);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_type_2_compare(const mss_mac_instance_t *this_mac, uint32_t comparer_no_in, const mss_mac_type_2_compare_t *comparer)
{
    volatile uint32_t *p_reg;
    uint32_t limit;
    uint32_t temp_reg;
    uint32_t comparer_no = comparer_no_in;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac) /* eMAC limits are different to pMAC ones */
        {
            p_reg = &this_mac->emac_base->TYPE2_COMPARE_0_WORD_0;
            limit = MSS_MAC_EMAC_TYPE_2_COMPARERS;
        }
        else
        {
            p_reg = &this_mac->mac_base->TYPE2_COMPARE_0_WORD_0;
            limit = MSS_MAC_TYPE_2_COMPARERS;
        }

        if(comparer_no < limit)
        {
            comparer_no *= 2U; /* Working with consecutive pairs of registers for this one */
            if(0U != comparer->disable_mask)
            {
                p_reg[comparer_no] = comparer->data; /* Mask disabled so just 4 byte compare value */
                temp_reg = GEM_DISABLE_MASK;         /* and no mask */
            }
            else
            {
                temp_reg  = comparer->data << 16;     /* 16 bit compare value and 16 bit mask */
                temp_reg |= (uint32_t)comparer->mask;
                p_reg[comparer_no] = temp_reg;
                temp_reg = 0U;
            }

            if(0U != comparer->compare_vlan_c_id)
            {
                temp_reg |= GEM_COMPARE_VLAN_ID;
            }
            else if(0U != comparer->compare_vlan_s_id)
            {
                temp_reg |= GEM_COMPARE_VLAN_ID | GEM_COMPARE_S_TAG;
            }
            else
            {
                temp_reg |= (uint32_t)comparer->offset_value & BITS_07;
                temp_reg |= ((uint32_t)comparer->compare_offset & BITS_02) << GEM_COMPARE_OFFSET_SHIFT;
            }

            p_reg[comparer_no + 1] = temp_reg; /* Set second word of comparer */
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_get_type_2_compare(const mss_mac_instance_t *this_mac, uint32_t comparer_no_in, mss_mac_type_2_compare_t *comparer)
{
    volatile uint32_t *p_reg;
    uint32_t limit;
    uint32_t comparer_no = comparer_no_in;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac) /* eMAC limits are different to pMAC ones */
        {
            p_reg = &this_mac->emac_base->TYPE2_COMPARE_0_WORD_0;
            limit = MSS_MAC_EMAC_TYPE_2_COMPARERS;
        }
        else
        {
            p_reg = &this_mac->mac_base->TYPE2_COMPARE_0_WORD_0;
            limit = MSS_MAC_TYPE_2_COMPARERS;
        }

        (void)memset(comparer, 0, sizeof(mss_mac_type_2_compare_t));
        if(comparer_no < limit)
        {
            comparer_no *= 2U; /* Working with consecutive pairs of registers for this one */
            if(0U != (p_reg[comparer_no + 1] & GEM_DISABLE_MASK))
            {
                comparer->data = p_reg[comparer_no]; /* Mask disabled so just 4 byte compare value */
                comparer->disable_mask = 1;          /* and no mask */
            }
            else
            {
                comparer->data = p_reg[comparer_no] >> 16;     /* 16 bit compare value and 16 bit mask */
                comparer->mask = (uint16_t)p_reg[comparer_no];
            }

            if(0U != (p_reg[comparer_no + 1U] & GEM_COMPARE_VLAN_ID))
            {
                if(0U != (p_reg[comparer_no + 1U] & GEM_COMPARE_S_TAG))
                {
                    comparer->compare_vlan_s_id = 1U;
                }
                else
                {
                    comparer->compare_vlan_c_id = 1U;
                }
            }
            else
            {
                comparer->compare_offset = (uint8_t)((p_reg[comparer_no + 1U] >> GEM_COMPARE_OFFSET_SHIFT) & BITS_02);
            }

            comparer->offset_value = (uint8_t)(p_reg[comparer_no + 1U] & GEM_OFFSET_VALUE);
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_type_2_filter(const mss_mac_instance_t *this_mac, uint32_t filter_no, const mss_mac_type_2_filter_t *filter)
{
    volatile uint32_t *p_reg;
    uint32_t limit;
    uint32_t temp_reg = 0U;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac) /* eMAC limits are different to pMAC ones */
        {
            p_reg = &this_mac->emac_base->SCREENING_TYPE_2_REGISTER_0;
            limit = MSS_MAC_EMAC_TYPE_2_SCREENERS;
        }
        else
        {
            p_reg = &this_mac->mac_base->SCREENING_TYPE_2_REGISTER_0;
            limit = MSS_MAC_TYPE_2_SCREENERS;
        }

        if(filter_no < limit) /* Lets go build a filter... */
        {
            if(0u != (filter->drop_on_match))
            {
                temp_reg = GEM_T2_DROP_ON_MATCH;
            }
            else
            {
                temp_reg = 0U;
            }

            if(0U != (filter->compare_a_enable))
            {
                temp_reg |= GEM_COMPARE_A_ENABLE;
                temp_reg |= ((uint32_t)filter->compare_a_index & BITS_05) << GEM_COMPARE_A_SHIFT;
            }

            if(0U != (filter->compare_b_enable))
            {
                temp_reg |= GEM_COMPARE_B_ENABLE;
                temp_reg |= ((uint32_t)filter->compare_b_index & BITS_05) << GEM_COMPARE_B_SHIFT;
            }

            if(0U != (filter->compare_c_enable))
            {
                temp_reg |= GEM_COMPARE_C_ENABLE;
                temp_reg |= ((uint32_t)filter->compare_c_index & BITS_05) << GEM_COMPARE_C_SHIFT;
            }

            if(0U != (filter->ethertype_enable))
            {
                temp_reg |= GEM_ETHERTYPE_ENABLE;
                temp_reg |= ((uint32_t)filter->ethertype_index & BITS_03) << GEM_ETHERTYPE_REG_INDEX_SHIFT;
            }

            if(0U != (filter->vlan_priority_enable))
            {
                temp_reg |= GEM_VLAN_ENABLE;
                temp_reg |= ((uint32_t)filter->vlan_priority & BITS_03) << GEM_VLAN_PRIORITY_SHIFT;
            }

            temp_reg |= (uint32_t)filter->queue_no & GEM_QUEUE_NUMBER;

            p_reg[filter_no] = temp_reg; /* Set filter up at last */
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_get_type_2_filter(const mss_mac_instance_t *this_mac, uint32_t filter_no, mss_mac_type_2_filter_t *filter)
{
    volatile uint32_t *p_reg;
    uint32_t limit;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        (void)memset(filter, 0, sizeof(mss_mac_type_2_filter_t));
        if(0U != this_mac->is_emac) /* eMAC limits are different to pMAC ones */
        {
            p_reg = &this_mac->emac_base->SCREENING_TYPE_2_REGISTER_0;
            limit = MSS_MAC_EMAC_TYPE_2_SCREENERS;
        }
        else
        {
            p_reg = &this_mac->mac_base->SCREENING_TYPE_2_REGISTER_0;
            limit = MSS_MAC_TYPE_2_SCREENERS;
        }

        if(filter_no < limit) /* Lets go fetch a filter... */
        {
            if(0U != (p_reg[filter_no] & GEM_T2_DROP_ON_MATCH))
            {
                filter->drop_on_match = 1;
            }

            if(0U != (p_reg[filter_no] & GEM_COMPARE_A_ENABLE))
            {
                filter->compare_a_enable = 1;
            }

            if(0U != (p_reg[filter_no] & GEM_COMPARE_B_ENABLE))
            {
                filter->compare_b_enable = 1;
            }

            if(0U != (p_reg[filter_no] & GEM_COMPARE_C_ENABLE))
            {
                filter->compare_c_enable = 1;
            }

            if(0U != (p_reg[filter_no] & GEM_ETHERTYPE_ENABLE))
            {
                filter->ethertype_enable = 1;
            }

            if(0U != (p_reg[filter_no] & GEM_VLAN_ENABLE))
            {
                filter->vlan_priority_enable = 1;
            }

            filter->compare_a_index = (uint8_t)((p_reg[filter_no] & GEM_COMPARE_A) >> GEM_COMPARE_A_SHIFT);
            filter->compare_b_index = (uint8_t)((p_reg[filter_no] & GEM_COMPARE_B) >> GEM_COMPARE_B_SHIFT);
            filter->compare_c_index = (uint8_t)((p_reg[filter_no] & GEM_COMPARE_C) >> GEM_COMPARE_C_SHIFT);
            filter->ethertype_index = (uint8_t)((p_reg[filter_no] & GEM_ETHERTYPE_REG_INDEX) >> GEM_ETHERTYPE_REG_INDEX_SHIFT);
            filter->vlan_priority   = (uint8_t)((p_reg[filter_no] & GEM_VLAN_PRIORITY) >> GEM_VLAN_PRIORITY_SHIFT);
            filter->queue_no        = (uint8_t)(p_reg[filter_no] & GEM_QUEUE_NUMBER);
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_mmsl_mode(const mss_mac_instance_t *this_mac, const mss_mac_mmsl_config_t *mmsl_cfg)
{
    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != mmsl_cfg->preemption) /* Preemption is enabled so deal with it */
        {
            /*
             * Need to shut preemption down in case it is already enabled
             * otherwise the new settings may not be recognised properly.
             */
            this_mac->mac_base->MMSL_CONTROL = 0U;

            if(0U != mmsl_cfg->verify_disable)
            {
                this_mac->mac_base->MMSL_CONTROL |= ((uint32_t)(mmsl_cfg->frag_size)) | GEM_VERIFY_DISABLE;
                this_mac->mac_base->MMSL_CONTROL |= GEM_PRE_ENABLE;
            }
            else
            {
                this_mac->mac_base->MMSL_CONTROL |= (uint32_t)(mmsl_cfg->frag_size);
                this_mac->mac_base->MMSL_CONTROL |= GEM_PRE_ENABLE;
            }
        }
        else /* Preemption is not enabled so see which MAC we want to use */
        {
            if(0U != mmsl_cfg->use_pmac)
            {
                this_mac->mac_base->MMSL_CONTROL = GEM_ROUTE_RX_TO_PMAC;
            }
            else
            {
                this_mac->mac_base->MMSL_CONTROL = 0U;
            }
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_get_mmsl_mode(const mss_mac_instance_t *this_mac, mss_mac_mmsl_config_t *mmsl_cfg)
{
    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        (void)memset(mmsl_cfg, 0, sizeof(mss_mac_mmsl_config_t));
        if(0U != (this_mac->mac_base->MMSL_CONTROL & GEM_ROUTE_RX_TO_PMAC))
        {
            mmsl_cfg->use_pmac = 1U;
        }

        if(0U != (this_mac->mac_base->MMSL_CONTROL & GEM_PRE_ENABLE))
        {
            mmsl_cfg->preemption = 1U;
        }

        if(0U != (this_mac->mac_base->MMSL_CONTROL & GEM_VERIFY_DISABLE))
        {
            mmsl_cfg->verify_disable = 1U;
        }

        mmsl_cfg->frag_size = (mss_mac_frag_size_t)(this_mac->mac_base->MMSL_CONTROL & BITS_02);
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_start_preemption_verify(const mss_mac_instance_t *this_mac)
{
    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != (this_mac->mac_base->MMSL_CONTROL & GEM_PRE_ENABLE)) /* Preemption is enabled */
        {
            this_mac->mac_base->MMSL_CONTROL |= GEM_RESTART_VER;
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint32_t MSS_MAC_get_mmsl_status(const mss_mac_instance_t *this_mac)
{
    uint32_t ret_val = 0U;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        ret_val = this_mac->mac_base->MMSL_STATUS; /* Just return raw value, user can decode using defined bits */
    }

    return(ret_val);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_get_mmsl_stats(const mss_mac_instance_t *this_mac, mss_mac_mmsl_stats_t *stats)
{
    /*
     * We return these differently to the general statistics as they are pMAC
     * specific and don't have a corresponding eMAC equivalent.
     */
    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        (void)memset(stats, 0, sizeof(mss_mac_mmsl_stats_t));
        stats->smd_err_count = (this_mac->mac_base->MMSL_ERR_STATS & GEM_SMD_ERR_COUNT) >> GEM_SMD_ERR_COUNT_SHIFT;
        stats->ass_err_count = this_mac->mac_base->MMSL_ERR_STATS & GEM_ASS_ERR_COUNT;
        stats->ass_ok_count  = this_mac->mac_base->MMSL_ASS_OK_COUNT;
        stats->frag_count_rx = this_mac->mac_base->MMSL_FRAG_COUNT_RX;
        stats->frag_count_tx = this_mac->mac_base->MMSL_FRAG_COUNT_TX;
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_tx_cutthru(const mss_mac_instance_t *this_mac, uint32_t level)
{
    volatile uint32_t *p_reg;
    uint32_t mask;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        p_reg = (0U != this_mac->is_emac) ? &this_mac->emac_base->PBUF_TXCUTTHRU : &this_mac->mac_base->PBUF_TXCUTTHRU;
        mask  = (0U != this_mac->is_emac) ? GEM_DMA_EMAC_CUTTHRU_THRESHOLD : GEM_DMA_TX_CUTTHRU_THRESHOLD;

        if(0U == level) /* Disabling cutthru? */
        {
            *p_reg = 0U;
        }
        else
        {
            *p_reg = GEM_DMA_CUTTHRU | (level & mask);
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_rx_cutthru(const mss_mac_instance_t *this_mac, uint32_t level)
{
    volatile uint32_t *p_reg;
    uint32_t mask;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        p_reg = (0U != this_mac->is_emac) ? &this_mac->emac_base->PBUF_RXCUTTHRU : &this_mac->mac_base->PBUF_RXCUTTHRU;
        mask  = (0U != this_mac->is_emac) ? GEM_DMA_EMAC_CUTTHRU_THRESHOLD : GEM_DMA_RX_CUTTHRU_THRESHOLD;

        if(0U == level) /* Disabling cutthru? */
        {
            *p_reg = 0U;
        }
        else
        {
            *p_reg = GEM_DMA_CUTTHRU | (level & mask);
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint32_t MSS_MAC_get_tx_cutthru(const mss_mac_instance_t *this_mac)
{
    uint32_t temp_reg = 0U;
    volatile uint32_t *p_reg;
    uint32_t mask;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        p_reg = (0U != this_mac->is_emac) ? &this_mac->emac_base->PBUF_TXCUTTHRU : &this_mac->mac_base->PBUF_TXCUTTHRU;
        mask  = (0U != this_mac->is_emac) ? GEM_DMA_EMAC_CUTTHRU_THRESHOLD : GEM_DMA_TX_CUTTHRU_THRESHOLD;

        temp_reg = *p_reg;
        if(0U == (temp_reg & GEM_DMA_CUTTHRU))
        {
            temp_reg = 0U;
        }
        else
        {
            temp_reg &= mask;
        }
    }

    return(temp_reg);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint32_t MSS_MAC_get_rx_cutthru(const mss_mac_instance_t *this_mac)
{
    uint32_t temp_reg = 0U;
    volatile uint32_t *p_reg;
    uint32_t mask;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        p_reg = (0U != this_mac->is_emac) ? &this_mac->emac_base->PBUF_RXCUTTHRU : &this_mac->mac_base->PBUF_RXCUTTHRU;
        mask  = (0U != this_mac->is_emac) ? GEM_DMA_EMAC_CUTTHRU_THRESHOLD : GEM_DMA_RX_CUTTHRU_THRESHOLD;

        temp_reg = *p_reg;
        if(0U == (temp_reg & GEM_DMA_CUTTHRU))
        {
            temp_reg = 0U;
        }
        else
        {
            temp_reg &= mask;
        }
    }

    return(temp_reg);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_tx_enable(const mss_mac_instance_t *this_mac)
{
    /* Don't do this if already done in case it has side effects... */
    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            if(0U == (this_mac->emac_base->NETWORK_CONTROL & GEM_ENABLE_TRANSMIT))
            {
                this_mac->emac_base->NETWORK_CONTROL |= GEM_ENABLE_TRANSMIT;
            }
        }
        else
        {
            if(0U == (this_mac->mac_base->NETWORK_CONTROL & GEM_ENABLE_TRANSMIT))
            {
                this_mac->mac_base->NETWORK_CONTROL |= GEM_ENABLE_TRANSMIT;
            }
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_jumbo_frames_mode(const mss_mac_instance_t *this_mac, bool state)
{
    volatile uint32_t *p_reg = &this_mac->mac_base->NETWORK_CONFIG;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            p_reg = &this_mac->emac_base->NETWORK_CONFIG;
        }

        if(0U != this_mac->jumbo_frame_enable) /* Only look at this if the feature is enabled in config */
        {
            if(state)
            {
                *p_reg |= GEM_JUMBO_FRAMES;
            }
            else
            {
                *p_reg &= ~GEM_JUMBO_FRAMES;
            }
        }
        else /* Ensure it is disabled if not allowed... */
        {
            *p_reg &= ~GEM_JUMBO_FRAMES;
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

bool MSS_MAC_get_jumbo_frames_mode(const mss_mac_instance_t *this_mac)
{
    bool ret_val = false;
    volatile uint32_t *p_reg = &this_mac->mac_base->NETWORK_CONFIG;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            p_reg = &this_mac->emac_base->NETWORK_CONFIG;
        }

        ret_val = ((*p_reg & GEM_JUMBO_FRAMES) != 0U);
    }

    return(ret_val);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_jumbo_frame_length(const mss_mac_instance_t *this_mac, uint32_t length_in)
{
    volatile uint32_t *p_reg = &this_mac->mac_base->JUMBO_MAX_LENGTH;
    uint32_t length = length_in; /* Avoids warning about modifying parameter passed by value */

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            p_reg = &this_mac->emac_base->JUMBO_MAX_LENGTH;
        }

        /* Set up maximum jumbo frame size - but bounds check first  */
        if(length > MSS_MAC_JUMBO_MAX)
        {
            length = MSS_MAC_JUMBO_MAX;
        }

        if(0U != this_mac->jumbo_frame_enable) /* Only look at this if the feature is enabled in config */
        {
            *p_reg = length;
        }
        else /* Ensure it is set to reset value if not allowed... */
        {
            *p_reg = MSS_MAC_JUMBO_MAX;
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

uint32_t MSS_MAC_get_jumbo_frame_length(const mss_mac_instance_t *this_mac)
{
    uint32_t ret_val = 0;
    volatile uint32_t *p_reg = &this_mac->mac_base->JUMBO_MAX_LENGTH;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            p_reg = &this_mac->emac_base->JUMBO_MAX_LENGTH;
        }

        ret_val = *p_reg;
    }

    return(ret_val);
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

void MSS_MAC_set_pause_frame_copy_to_mem(const mss_mac_instance_t *this_mac, bool state)
{
    volatile uint32_t *p_reg = &this_mac->mac_base->NETWORK_CONFIG;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            p_reg = &this_mac->emac_base->NETWORK_CONFIG;
        }
        /*
         * Logic is inverted as enabling copy to memory means disabling
         * GEM_DISABLE_COPY_OF_PAUSE_FRAMES
         */
        if(state)
        {
            *p_reg &= ~GEM_DISABLE_COPY_OF_PAUSE_FRAMES;
        }
        else
        {
            *p_reg |= GEM_DISABLE_COPY_OF_PAUSE_FRAMES;
        }
    }
}


/******************************************************************************
 * See mss_ethernet_mac.h for details of how to use this function.
 */

bool MSS_MAC_get_pause_frame_copy_to_mem(const mss_mac_instance_t *this_mac)
{
    bool ret_val = false;
    volatile uint32_t *p_reg = &this_mac->mac_base->NETWORK_CONFIG;

    if(MSS_MAC_AVAILABLE == this_mac->mac_available)
    {
        if(0U != this_mac->is_emac)
        {
            p_reg = &this_mac->emac_base->NETWORK_CONFIG;
        }

        /*
         * Logic is inverted as enabling copy to memory means disabling
         * GEM_DISABLE_COPY_OF_PAUSE_FRAMES
         */
        ret_val = 0U == (*p_reg & GEM_DISABLE_COPY_OF_PAUSE_FRAMES);
    }

    return(ret_val);
}


/**************************************************************************/
/* Private Function definitions                                           */
/**************************************************************************/

/******************************************************************************
 * This is default "Receive packet interrupt handler. This function finds the
 * descriptor that received the packet and caused the interrupt.
 * This informs the received packet size to the application and
 * relinquishes the packet buffer from the associated DMA descriptor.
 */
static void
rxpkt_handler
(
    mss_mac_instance_t *this_mac, uint32_t queue_no
)
{
    mss_mac_queue_t *this_queue = &this_mac->queue[queue_no];
    mss_mac_rx_desc_t * cdesc = &this_queue->rx_desc_tab[this_queue->first_rx_desc_index];

    if(0U != (cdesc->addr_low & GEM_RX_DMA_USED)) /* Check in case we already got it... */
    {
        /* Execution comes here because at-least one packet is received. */
        do
        {
            uint8_t * p_rx_packet;
            uint32_t pckt_length;
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
            uint64_t addr_temp;
#else
            uint32_t addr_temp;
#endif

            ++this_queue->nb_available_rx_desc;
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
            addr_temp  = (uint64_t)(cdesc->addr_low & ~(GEM_RX_DMA_WRAP | GEM_RX_DMA_USED | GEM_RX_DMA_TS_PRESENT));
            addr_temp |= (uint64_t)cdesc->addr_high << 32;
#else
            addr_temp = (cdesc->addr_low & ~(GEM_RX_DMA_WRAP | GEM_RX_DMA_USED | GEM_RX_DMA_TS_PRESENT));
#endif
            p_rx_packet = (uint8_t *)addr_temp;
            /*
             * Pass received packet up to application layer - if enabled...
             *
             * Note if rx_packet comes back as 0 we can't recover and will leave a
             * used packet stuck in the queue...
             */
            if((NULL_POINTER != this_queue->pckt_rx_callback) && (NULL_POINTER != p_rx_packet) && (0U == this_mac->rx_discard))
            {
                pckt_length = cdesc->status & (GEM_RX_DMA_BUFF_LEN | GEM_RX_DMA_JUMBO_BIT_13);
                this_queue->ingress += pckt_length;

                this_queue->pckt_rx_callback(this_mac, queue_no, p_rx_packet, pckt_length, cdesc, this_queue->rx_caller_info[this_queue->first_rx_desc_index]);
            }
            if((NULL_POINTER != p_rx_packet) && (0U != this_mac->rx_discard))
            {
                /*
                 * Need to return receive packet buffer to the queue as rx handler
                 * hasn't been called to do it for us...
                 */
                (void)MSS_MAC_receive_pkt(this_mac, queue_no, p_rx_packet, this_queue->rx_caller_info[this_queue->first_rx_desc_index], MSS_MAC_INT_ENABLE);
            }

            cdesc->addr_low &= ~GEM_RX_DMA_USED; /* Mark buffer as unused again */

            /* Point the curr_rx_desc to next descriptor in the ring */
            /* Wrap around in case next descriptor is pointing to last in the ring */
            ++this_queue->first_rx_desc_index;
            this_queue->first_rx_desc_index %= MSS_MAC_RX_RING_SIZE;

           cdesc = &this_queue->rx_desc_tab[this_queue->first_rx_desc_index];
        } while(0 != (cdesc->addr_low & GEM_RX_DMA_USED)); /* loop while there are packets available */
    }
}


/******************************************************************************
 * This is default "Transmit packet interrupt handler. This function finds the
 * descriptor that transmitted the packet and caused the interrupt.
 * This relinquishes the packet buffer from the associated DMA descriptor.
 */
static void
txpkt_handler
(
        mss_mac_instance_t *this_mac, uint32_t queue_no
)
{
#if defined(MSS_MAC_SIMPLE_TX_QUEUE)
    mss_mac_queue_t *this_queue = &this_mac->queue[queue_no];

    /*
     * Simple single packet TX queue where only the one packet buffer is needed
     * but two descriptors are required to stop DMA engine running over itself...
     */
    if(NULL_POINTER != this_queue->pckt_tx_callback)
    {
        this_queue->pckt_tx_callback(this_mac, queue_no, this_queue->tx_desc_tab, this_queue->tx_caller_info[0]);
    }

    this_queue->nb_available_tx_desc = MSS_MAC_TX_RING_SIZE; /* Release transmit queue... */

#else
    uint32_t empty_flag;
    uint32_t index;
    uint32_t completed = 0U;
    ASSERT(g_mac.first_tx_index != INVALID_INDEX);
    ASSERT(g_mac.last_tx_index != INVALID_INDEX);
    /* TBD PMCS multi packet tx queue not implemented yet */
    index = g_mac.first_tx_index;
    do
    {
        ++g_mac.nb_available_tx_desc;
        /* Call packet Tx completion handler if it exists. */
        if(NULL_POINTER != g_mac.pckt_tx_callback)
        {
            g_mac.pckt_tx_callback(g_mac.tx_desc_tab[index].caller_info);
        }

        if(index == g_mac.last_tx_index)
        {
            /* all pending tx packets sent. */
            g_mac.first_tx_index = INVALID_INDEX;
            g_mac.last_tx_index = INVALID_INDEX;
            completed = 1U;
        }
        else
        {
            /* Move on to next transmit descriptor. */
            ++index;
            index %= MSS_MAC_TX_RING_SIZE;
            g_mac.first_tx_index = index;
            /* Check if we reached a descriptor still pending tx. */
            empty_flag = g_mac.tx_desc_tab[index].pkt_size & DMA_DESC_EMPTY_FLAG_MASK;
            if(0U == empty_flag)
            {
                completed = 1U;
            }
        }


        /* Clear the tx packet sent interrupt. Please note that this must be
         * done for every packet sent as it decrements the TXPKTCOUNT. */
        set_bit_reg32(&MAC->DMA_TX_STATUS, DMA_TXPKTSENT);
    } while(0U == completed);
#endif
}


/******************************************************************************
 *
 */
static void tx_desc_ring_init(mss_mac_instance_t *this_mac)
{
    int32_t inc;
    int32_t queue_no;

    for(queue_no = 0; queue_no < MSS_MAC_QUEUE_COUNT; queue_no++)
    {
#if defined(MSS_MAC_USE_DDR)
        this_mac->queue[queue_no].tx_desc_tab = g_mss_mac_ddr_ptr;
        g_mss_mac_ddr_ptr += MSS_MAC_TX_RING_SIZE * sizeof(mss_mac_tx_desc_t);
#endif
        for(inc = 0; inc < MSS_MAC_TX_RING_SIZE; ++inc)
        {
            this_mac->queue[queue_no].tx_desc_tab[inc].addr_low = 0U;
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
            this_mac->queue[queue_no].tx_desc_tab[inc].addr_high = 0U;
#endif
            this_mac->queue[queue_no].tx_desc_tab[inc].status = GEM_TX_DMA_USED;
        }

        inc--; /* Step back to last buffer descriptor and mark as end of list */
        this_mac->queue[queue_no].tx_desc_tab[inc].status |= GEM_TX_DMA_WRAP;
    }
}


/******************************************************************************
 *
 */
static void rx_desc_ring_init(mss_mac_instance_t *this_mac)
{
    uint32_t inc;
    int32_t queue_no;

    for(queue_no = 0; queue_no < MSS_MAC_QUEUE_COUNT; queue_no++)
    {
#if defined(MSS_MAC_USE_DDR)
        this_mac->queue[queue_no].rx_desc_tab = g_mss_mac_ddr_ptr;
        g_mss_mac_ddr_ptr += MSS_MAC_RX_RING_SIZE * sizeof(mss_mac_rx_desc_t);
#endif

        for(inc = 0U; inc < MSS_MAC_RX_RING_SIZE; ++inc)
        {
            this_mac->queue[queue_no].rx_desc_tab[inc].addr_low = 0U; /* Mark buffers as used for now in case DMA gets enabled before we attach a buffer */
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
            this_mac->queue[queue_no].rx_desc_tab[inc].addr_high = 0U;
#endif
            this_mac->queue[queue_no].rx_desc_tab[inc].status = 0U;
        }

        inc--; /* Step back to last buffer descriptor and mark as end of list */
        this_mac->queue[queue_no].rx_desc_tab[inc].addr_low |= GEM_RX_DMA_WRAP;
    }
}


/******************************************************************************
 *
 */
static void assign_station_addr
(
    mss_mac_instance_t *this_mac,
    const uint8_t mac_addr[MSS_MAC_MAC_LEN]
)
{
    uint32_t address32_l;
    uint32_t address32_h;

    ASSERT(NULL_POINTER != mac_addr);

    if((NULL_POINTER != mac_addr) && (NULL_POINTER != this_mac))
    {
        /* Update current instance data */
        (void)memcpy(this_mac->mac_addr, mac_addr, MSS_MAC_MAC_LEN);

        /* Assemble correct register values */
        address32_l  = ((uint32_t)mac_addr[3]) << 24;
        address32_l |= ((uint32_t)mac_addr[2]) << 16;
        address32_l |= ((uint32_t)mac_addr[1]) << 8;
        address32_l |= ((uint32_t)mac_addr[0]);
        address32_h =  ((uint32_t)mac_addr[5]) << 8;
        address32_h |= ((uint32_t)mac_addr[4]);

        /* Update hardware registers */
        if(0U != this_mac->is_emac)
        {
            this_mac->emac_base->SPEC_ADD1_BOTTOM = address32_l;
            this_mac->emac_base->SPEC_ADD1_TOP    = address32_h;
            this_mac->emac_base->SPEC_ADD2_BOTTOM = 0U;
            this_mac->emac_base->SPEC_ADD2_TOP    = 0U;
            this_mac->emac_base->SPEC_ADD3_BOTTOM = 0U;
            this_mac->emac_base->SPEC_ADD3_TOP    = 0U;
            this_mac->emac_base->SPEC_ADD4_BOTTOM = 0U;
            this_mac->emac_base->SPEC_ADD4_TOP    = 0U;
        }
        else
        {
            this_mac->mac_base->SPEC_ADD1_BOTTOM  = address32_l;
            this_mac->mac_base->SPEC_ADD1_TOP     = address32_h;
            this_mac->mac_base->SPEC_ADD2_BOTTOM  = 0U;
            this_mac->mac_base->SPEC_ADD2_TOP     = 0U;
            this_mac->mac_base->SPEC_ADD3_BOTTOM  = 0U;
            this_mac->mac_base->SPEC_ADD3_TOP     = 0U;
            this_mac->mac_base->SPEC_ADD4_BOTTOM  = 0U;
            this_mac->mac_base->SPEC_ADD4_TOP     = 0U;
        }
    }
}


/*******************************************************************************
 * Auto-detect the PHY's address by attempting to read the PHY identification
 * register containing the PHY manufacturer's identifier.
 * Attempting to read a PHY register using an incorrect PHY address will result
 * in a value with all bits set to one on the MDIO bus. Reading any other value
 * means that a PHY responded to the read request, therefore we have found the
 * PHY's address.
 * This function returns the detected PHY's address or 32 (PHY_ADDRESS_MAX + 1)
 * if no PHY is responding.
 */
static uint8_t probe_phy(const mss_mac_instance_t *this_mac)
{
    uint8_t phy_address = PHY_ADDRESS_MIN;
    const uint16_t ALL_BITS_HIGH = 0xFFFFU;
    const uint8_t PHYREG_PHYID1R = 0x02U;   /* PHY Identifier 1 register address. */
    uint32_t found;

    do
    {
        uint16_t reg;

        reg = MSS_MAC_read_phy_reg(this_mac, phy_address, PHYREG_PHYID1R);
        if (reg != ALL_BITS_HIGH)
        {
            found = 1U;
        }
        else
        {
            found = 0U;
            ++phy_address;
        }
    }
    while ((phy_address <= PHY_ADDRESS_MAX) && (0U == found));

    return phy_address;
}


/*******************************************************************************
 * MSS MAC TBI interface
 */
static void msgmii_init(const mss_mac_instance_t *this_mac)
{
    if(GMII_SGMII == this_mac->interface_type)
    {
        if(0U == this_mac->is_emac)
        {
            this_mac->mac_base->PCS_CONTROL = 0x9000UL; /* Reset and enable autonegotiation */
        }
    }

    if(TBI == this_mac->interface_type)
    {
        if(0U == this_mac->is_emac)
        {
            this_mac->mac_base->PCS_CONTROL = 0x9000UL; /* Reset and enable autonegotiation */
        }
    }
}


/*******************************************************************************
 *
 */
 static void msgmii_autonegotiate(const mss_mac_instance_t *this_mac)
 {
    uint16_t phy_reg;
    uint16_t autoneg_complete;
    uint8_t link_fullduplex;
    mss_mac_speed_t link_speed;
    uint8_t copper_link_up;

    volatile uint32_t sgmii_aneg_timeout = 100000U;

    copper_link_up = this_mac->phy_get_link_status(this_mac, &link_speed, &link_fullduplex);

    if(MSS_MAC_LINK_UP == copper_link_up)
    {
        /* Initiate auto-negotiation on the TBI SGMII link. */
        if(TBI == this_mac->interface_type)
        {
            phy_reg = (uint16_t)this_mac->mac_base->PCS_CONTROL;
            phy_reg |= 0x1000U;
            this_mac->mac_base->PCS_CONTROL = phy_reg;
            phy_reg |= 0x0200U;
            this_mac->mac_base->PCS_CONTROL = phy_reg;

            /* Wait for SGMII auto-negotiation to complete. */
            do
            {
                phy_reg = (uint16_t)this_mac->mac_base->PCS_STATUS;
                autoneg_complete = phy_reg & BMSR_AUTO_NEGOTIATION_COMPLETE;
                --sgmii_aneg_timeout;
            } while(((0U == autoneg_complete) && (sgmii_aneg_timeout != 0U)) || (0xFFFFU == phy_reg));
        }
    }
}


#if 0
/*******************************************************************************
 * SGMII or 1000BaseX interface with CoreSGMII
 */
#if (MSS_MAC_PHY_INTERFACE == SGMII)

#define CORE_SGMII_PHY_ADDR    MSS_MAC_INTERFACE_MDIO_ADDR
/*******************************************************************************
 *
 */
static void coresgmii_init(void)
{
    uint16_t phy_reg;

    /* Reset C-SGMII. */
    MSS_MAC_write_phy_reg(CORE_SGMII_PHY_ADDR, 0x00U, 0x9000U);
    /* Register 0x04 of C-SGMII must be always be set to 0x0001. */
    MSS_MAC_write_phy_reg(CORE_SGMII_PHY_ADDR, 0x04U, 0x0001U);

    /* Enable auto-negotiation inside CoreSGMII block. */
    phy_reg = MSS_MAC_read_phy_reg(CORE_SGMII_PHY_ADDR, 0x00U);
    phy_reg |= 0x1000U;
    MSS_MAC_write_phy_reg(CORE_SGMII_PHY_ADDR, 0x00U, phy_reg);
}


/*******************************************************************************
 *
 */
static void coresgmii_autonegotiate(void)
{
    uint16_t phy_reg;
    uint16_t autoneg_complete;
    volatile uint32_t sgmii_aneg_timeout = 1000000U;

    uint8_t link_fullduplex;
    mss_mac_speed_t link_speed;
    uint8_t copper_link_up;

    copper_link_up = MSS_MAC_phy_get_link_status(&link_speed, &link_fullduplex);

    if(MSS_MAC_LINK_UP == copper_link_up)
    {
        SYSREG->MAC_CR = (SYSREG->MAC_CR & ~MAC_CONFIG_SPEED_MASK) | link_speed;

        /* Configure duplex mode */
        if(MSS_MAC_HALF_DUPLEX == link_fullduplex)
        {
            /* half duplex */
            MAC->CFG2 &= ~CFG2_FDX_MASK;
        }
        else
        {
            /* full duplex */
            MAC->CFG2 |= CFG2_FDX_MASK;
        }
        /* Initiate auto-negotiation on the SGMII link. */
        phy_reg = MSS_MAC_read_phy_reg(CORE_SGMII_PHY_ADDR, 0x00U);
        phy_reg |= 0x1000U;
        MSS_MAC_write_phy_reg(CORE_SGMII_PHY_ADDR, 0x00U, phy_reg);
        phy_reg |= 0x0200U;
        MSS_MAC_write_phy_reg(CORE_SGMII_PHY_ADDR, 0x00U, phy_reg);

        /* Wait for SGMII auto-negotiation to complete. */
        do {
            phy_reg = MSS_MAC_read_phy_reg(CORE_SGMII_PHY_ADDR, MII_BMSR);
            autoneg_complete = phy_reg & BMSR_AUTO_NEGOTIATION_COMPLETE;
            --sgmii_aneg_timeout;
        } while(((0U == autoneg_complete) && (sgmii_aneg_timeout != 0U)) || (0xFFFFU == phy_reg));
    }
}


/*******************************************************************************
 * Generate clock 2.5/25/125MHz for 10/100/1000Mbps using Clock Condition Circuit(CCC)
 */
static void coresgmii_set_link_speed(uint32_t speed)
{
    uint16_t phy_reg;

    phy_reg = MSS_MAC_read_phy_reg(CORE_SGMII_PHY_ADDR, 0x11U);
    phy_reg |= (speed << 2);
    MSS_MAC_write_phy_reg(CORE_SGMII_PHY_ADDR, 0x11U, phy_reg);
}

#endif /* #if (MSS_MAC_PHY_INTERFACE == SGMII) */
#endif


/*******************************************************************************
 * Setup hardware addresses etc for instance structure(s).
 */
static void instances_init(mss_mac_instance_t *this_mac, mss_mac_cfg_t *cfg)
{
#if defined(TARGET_ALOE)
    /* These are unused for Aloe as there is only one pMAC and no alternate
     * address support required
     */
    (void)this_mac;
    (void)cfg;

    (void)memset(&g_mac0, 0, sizeof(g_mac0));

    g_mac0.use_hi_address = cfg->use_hi_address; /* Not really needed but to be consistent... */

    g_mac0.is_emac                 = 0U;
    g_mac0.mac_base                = (MAC_TypeDef  *)MSS_MAC0_BASE;
    g_mac0.emac_base               = (eMAC_TypeDef *)MSS_MAC0_BASE;
    g_mac0.mac_q_int[0]            = ethernet_PLIC_53;
    g_mac0.queue[0].int_status     = &g_mac0.mac_base->INT_STATUS;
    g_mac0.queue[0].int_mask       = &g_mac0.mac_base->INT_MASK;
    g_mac0.queue[0].int_enable     = &g_mac0.mac_base->INT_ENABLE;
    g_mac0.queue[0].int_disable    = &g_mac0.mac_base->INT_DISABLE;
    g_mac0.queue[0].receive_q_ptr  = &g_mac0.mac_base->RECEIVE_Q_PTR;
    g_mac0.queue[0].transmit_q_ptr = &g_mac0.mac_base->TRANSMIT_Q_PTR;
    g_mac0.queue[0].dma_rxbuf_size = &g_mac0.mac_base->DMA_RXBUF_SIZE_Q1; /* Not really true as this is done differently for queue 0 */
#endif
#if defined(TARGET_G5_SOC)
    (void)memset(this_mac, 0, sizeof(mss_mac_instance_t)); /* Start with blank canvas */

    this_mac->use_hi_address = cfg->use_hi_address; /* Need to remember this for sanity checking */
    this_mac->use_local_ints = cfg->use_local_ints; /* Need to remember this for interrupt management */

    if(&g_mac0 == this_mac)
    {
        if(MSS_MAC_DISABLE == cfg->use_hi_address)
        {
            this_mac->mac_base    = (MAC_TypeDef  *)MSS_MAC0_BASE;
            this_mac->emac_base   = (eMAC_TypeDef *)MSS_EMAC0_BASE;

            SYSREG->APBBUS_CR &= ~MSS_MAC_GEM0_ABP_BIT;
        }
        else
        {
            this_mac->mac_base    = (MAC_TypeDef  *)MSS_MAC0_BASE_HI;
            this_mac->emac_base   = (eMAC_TypeDef *)MSS_EMAC0_BASE_HI;

            SYSREG->APBBUS_CR |= MSS_MAC_GEM0_ABP_BIT;
        }

        if(0U != this_mac->use_local_ints)
        {
            PLIC_DisableIRQ(MAC0_INT_PLIC);
            PLIC_DisableIRQ(MAC0_QUEUE1_PLIC);
            PLIC_DisableIRQ(MAC0_QUEUE2_PLIC);
            PLIC_DisableIRQ(MAC0_QUEUE3_PLIC);
            PLIC_DisableIRQ(MAC0_MMSL_PLIC);

            this_mac->mac_q_int[0] = MAC0_INT_U54_INT;
            this_mac->mac_q_int[1] = MAC0_QUEUE1_U54_INT;
            this_mac->mac_q_int[2] = MAC0_QUEUE2_U54_INT;
            this_mac->mac_q_int[3] = MAC0_QUEUE3_U54_INT;
            this_mac->mmsl_int     = MAC0_MMSL_U54_INT;
        }
        else
        {
            __disable_local_irq(MAC0_INT_U54_INT);
            __disable_local_irq(MAC0_QUEUE1_U54_INT);
            __disable_local_irq(MAC0_QUEUE2_U54_INT);
            __disable_local_irq(MAC0_QUEUE3_U54_INT);
            __disable_local_irq(MAC0_MMSL_U54_INT);

            this_mac->mac_q_int[0] = MAC0_INT_PLIC;
            this_mac->mac_q_int[1] = MAC0_QUEUE1_PLIC;
            this_mac->mac_q_int[2] = MAC0_QUEUE2_PLIC;
            this_mac->mac_q_int[3] = MAC0_QUEUE3_PLIC;
            this_mac->mmsl_int     = MAC0_MMSL_PLIC;
        }
    }
    else if(&g_mac1 == this_mac)
    {
        if(MSS_MAC_DISABLE == cfg->use_hi_address)
        {
            this_mac->mac_base    = (MAC_TypeDef  *)MSS_MAC1_BASE;
            this_mac->emac_base   = (eMAC_TypeDef *)MSS_EMAC1_BASE;

            SYSREG->APBBUS_CR &= ~MSS_MAC_GEM1_ABP_BIT;
        }
        else
        {
            this_mac->mac_base    = (MAC_TypeDef  *)MSS_MAC1_BASE_HI;
            this_mac->emac_base   = (eMAC_TypeDef *)MSS_EMAC1_BASE_HI;

            SYSREG->APBBUS_CR |= MSS_MAC_GEM1_ABP_BIT;
        }

        if(0U != this_mac->use_local_ints)
        {
            PLIC_DisableIRQ(MAC1_INT_PLIC);
            PLIC_DisableIRQ(MAC1_QUEUE1_PLIC);
            PLIC_DisableIRQ(MAC1_QUEUE2_PLIC);
            PLIC_DisableIRQ(MAC1_QUEUE3_PLIC);
            PLIC_DisableIRQ(MAC1_MMSL_PLIC);

            this_mac->mac_q_int[0] = MAC1_INT_U54_INT;
            this_mac->mac_q_int[1] = MAC1_QUEUE1_U54_INT;
            this_mac->mac_q_int[2] = MAC1_QUEUE2_U54_INT;
            this_mac->mac_q_int[3] = MAC1_QUEUE3_U54_INT;
            this_mac->mmsl_int     = MAC1_MMSL_U54_INT;
        }
        else
        {
            __disable_local_irq(MAC1_INT_U54_INT);
            __disable_local_irq(MAC1_QUEUE1_U54_INT);
            __disable_local_irq(MAC1_QUEUE2_U54_INT);
            __disable_local_irq(MAC1_QUEUE3_U54_INT);
            __disable_local_irq(MAC1_MMSL_U54_INT);

            this_mac->mac_q_int[0] = MAC1_INT_PLIC;
            this_mac->mac_q_int[1] = MAC1_QUEUE1_PLIC;
            this_mac->mac_q_int[2] = MAC1_QUEUE2_PLIC;
            this_mac->mac_q_int[3] = MAC1_QUEUE3_PLIC;
            this_mac->mmsl_int     = MAC1_MMSL_PLIC;
        }
    }
    else if(&g_emac0 == this_mac)
    {
        this_mac->is_emac = 1;
        if(NULL_POINTER != g_mac0.mac_base) /* If pMAC already configured must use same */
        {
            cfg->use_hi_address = g_mac0.use_hi_address;
        }

        if(MSS_MAC_DISABLE == cfg->use_hi_address)
        {
            this_mac->mac_base    = (MAC_TypeDef  *)MSS_MAC0_BASE;
            this_mac->emac_base   = (eMAC_TypeDef *)MSS_EMAC0_BASE;

            SYSREG->APBBUS_CR &= ~MSS_MAC_GEM0_ABP_BIT;
        }
        else
        {
            this_mac->mac_base    = (MAC_TypeDef  *)MSS_MAC0_BASE_HI;
            this_mac->emac_base   = (eMAC_TypeDef *)MSS_EMAC0_BASE_HI;
        }

        if(0U != this_mac->use_local_ints)
        {
            PLIC_DisableIRQ(MAC0_EMAC_PLIC);

            this_mac->mac_q_int[0] = MAC0_EMAC_U54_INT;
            this_mac->mac_q_int[1] = LOCAL_INT_UNUSED;
            this_mac->mac_q_int[2] = LOCAL_INT_UNUSED;
            this_mac->mac_q_int[3] = LOCAL_INT_UNUSED;
            this_mac->mmsl_int     = LOCAL_INT_UNUSED;
        }
        else
        {
            __disable_local_irq(MAC0_EMAC_U54_INT);

            this_mac->mac_q_int[0] = MAC0_EMAC_PLIC;
            this_mac->mac_q_int[1] = INVALID_IRQn;
            this_mac->mac_q_int[2] = INVALID_IRQn;
            this_mac->mac_q_int[3] = INVALID_IRQn;
            this_mac->mmsl_int     = INVALID_IRQn;
        }
    }
    else
    {
        if(&g_emac1 == this_mac)
        {
            this_mac->is_emac = 1U;
            if(MSS_MAC_DISABLE == cfg->use_hi_address)
            {
                this_mac->mac_base    = (MAC_TypeDef  *)MSS_MAC1_BASE;
                this_mac->emac_base   = (eMAC_TypeDef *)MSS_EMAC1_BASE;

                SYSREG->APBBUS_CR &= ~MSS_MAC_GEM1_ABP_BIT;
            }
            else
            {
                this_mac->mac_base    = (MAC_TypeDef  *)MSS_MAC1_BASE_HI;
                this_mac->emac_base   = (eMAC_TypeDef *)MSS_EMAC1_BASE_HI;

                SYSREG->APBBUS_CR |= MSS_MAC_GEM1_ABP_BIT;
            }

            if(0U != this_mac->use_local_ints)
            {
                PLIC_DisableIRQ(MAC1_EMAC_PLIC);

                this_mac->mac_q_int[0] = MAC1_EMAC_U54_INT;
                this_mac->mac_q_int[1] = LOCAL_INT_UNUSED;
                this_mac->mac_q_int[2] = LOCAL_INT_UNUSED;
                this_mac->mac_q_int[3] = LOCAL_INT_UNUSED;
                this_mac->mmsl_int     = LOCAL_INT_UNUSED;
            }
            else
            {
                __disable_local_irq(MAC1_EMAC_U54_INT);

                this_mac->mac_q_int[0] = MAC1_EMAC_PLIC;
                this_mac->mac_q_int[1] = INVALID_IRQn;
                this_mac->mac_q_int[2] = INVALID_IRQn;
                this_mac->mac_q_int[3] = INVALID_IRQn;
                this_mac->mmsl_int     = INVALID_IRQn;
            }
        }
    }

    if(0U == this_mac->is_emac)
    {
        this_mac->queue[0].int_status = &this_mac->mac_base->INT_STATUS;
        this_mac->queue[1].int_status = &this_mac->mac_base->INT_Q1_STATUS;
        this_mac->queue[2].int_status = &this_mac->mac_base->INT_Q2_STATUS;
        this_mac->queue[3].int_status = &this_mac->mac_base->INT_Q3_STATUS;

        this_mac->queue[0].int_mask = &this_mac->mac_base->INT_MASK;
        this_mac->queue[1].int_mask = &this_mac->mac_base->INT_Q1_MASK;
        this_mac->queue[2].int_mask = &this_mac->mac_base->INT_Q2_MASK;
        this_mac->queue[3].int_mask = &this_mac->mac_base->INT_Q3_MASK;

        this_mac->queue[0].int_enable = &this_mac->mac_base->INT_ENABLE;
        this_mac->queue[1].int_enable = &this_mac->mac_base->INT_Q1_ENABLE;
        this_mac->queue[2].int_enable = &this_mac->mac_base->INT_Q2_ENABLE;
        this_mac->queue[3].int_enable = &this_mac->mac_base->INT_Q3_ENABLE;

        this_mac->queue[0].int_disable = &this_mac->mac_base->INT_DISABLE;
        this_mac->queue[1].int_disable = &this_mac->mac_base->INT_Q1_DISABLE;
        this_mac->queue[2].int_disable = &this_mac->mac_base->INT_Q2_DISABLE;
        this_mac->queue[3].int_disable = &this_mac->mac_base->INT_Q3_DISABLE;

        this_mac->queue[0].receive_q_ptr = &this_mac->mac_base->RECEIVE_Q_PTR;
        this_mac->queue[1].receive_q_ptr = &this_mac->mac_base->RECEIVE_Q1_PTR;
        this_mac->queue[2].receive_q_ptr = &this_mac->mac_base->RECEIVE_Q2_PTR;
        this_mac->queue[3].receive_q_ptr = &this_mac->mac_base->RECEIVE_Q3_PTR;

        this_mac->queue[0].transmit_q_ptr = &this_mac->mac_base->TRANSMIT_Q_PTR;
        this_mac->queue[1].transmit_q_ptr = &this_mac->mac_base->TRANSMIT_Q1_PTR;
        this_mac->queue[2].transmit_q_ptr = &this_mac->mac_base->TRANSMIT_Q2_PTR;
        this_mac->queue[3].transmit_q_ptr = &this_mac->mac_base->TRANSMIT_Q3_PTR;

        this_mac->queue[0].dma_rxbuf_size = &this_mac->mac_base->DMA_RXBUF_SIZE_Q1; /* Not really true as this is done differently for queue 0 */
        this_mac->queue[1].dma_rxbuf_size = &this_mac->mac_base->DMA_RXBUF_SIZE_Q1;
        this_mac->queue[2].dma_rxbuf_size = &this_mac->mac_base->DMA_RXBUF_SIZE_Q2;
        this_mac->queue[3].dma_rxbuf_size = &this_mac->mac_base->DMA_RXBUF_SIZE_Q3;
    }
    else
    {
        this_mac->queue[0].int_status = &this_mac->emac_base->INT_STATUS;
        this_mac->queue[1].int_status = &this_mac->emac_base->INT_STATUS; /* Not Really correct but don't want 0 here... */
        this_mac->queue[2].int_status = &this_mac->emac_base->INT_STATUS;
        this_mac->queue[3].int_status = &this_mac->emac_base->INT_STATUS;

        this_mac->queue[0].int_mask = &this_mac->emac_base->INT_MASK;
        this_mac->queue[1].int_mask = &this_mac->emac_base->INT_MASK;
        this_mac->queue[2].int_mask = &this_mac->emac_base->INT_MASK;
        this_mac->queue[3].int_mask = &this_mac->emac_base->INT_MASK;

        this_mac->queue[0].int_enable = &this_mac->emac_base->INT_ENABLE;
        this_mac->queue[1].int_enable = &this_mac->emac_base->INT_ENABLE;
        this_mac->queue[2].int_enable = &this_mac->emac_base->INT_ENABLE;
        this_mac->queue[3].int_enable = &this_mac->emac_base->INT_ENABLE;

        this_mac->queue[0].int_disable = &this_mac->emac_base->INT_DISABLE;
        this_mac->queue[1].int_disable = &this_mac->emac_base->INT_DISABLE;
        this_mac->queue[2].int_disable = &this_mac->emac_base->INT_DISABLE;
        this_mac->queue[3].int_disable = &this_mac->emac_base->INT_DISABLE;

        this_mac->queue[0].receive_q_ptr = &this_mac->emac_base->RECEIVE_Q_PTR;
        this_mac->queue[1].receive_q_ptr = &this_mac->emac_base->RECEIVE_Q_PTR;
        this_mac->queue[2].receive_q_ptr = &this_mac->emac_base->RECEIVE_Q_PTR;
        this_mac->queue[3].receive_q_ptr = &this_mac->emac_base->RECEIVE_Q_PTR;

        this_mac->queue[0].transmit_q_ptr = &this_mac->emac_base->TRANSMIT_Q_PTR;
        this_mac->queue[1].transmit_q_ptr = &this_mac->emac_base->TRANSMIT_Q_PTR;
        this_mac->queue[2].transmit_q_ptr = &this_mac->emac_base->TRANSMIT_Q_PTR;
        this_mac->queue[3].transmit_q_ptr = &this_mac->emac_base->TRANSMIT_Q_PTR;

        this_mac->queue[0].dma_rxbuf_size = &this_mac->mac_base->DMA_RXBUF_SIZE_Q1; /* Not really true as this is done differently for queue 0 */
        this_mac->queue[1].dma_rxbuf_size = &this_mac->mac_base->DMA_RXBUF_SIZE_Q1; /* Not Really correct but don't want 0 here... */
        this_mac->queue[2].dma_rxbuf_size = &this_mac->mac_base->DMA_RXBUF_SIZE_Q2;
        this_mac->queue[3].dma_rxbuf_size = &this_mac->mac_base->DMA_RXBUF_SIZE_Q3;
    }
#endif /* defined(TARGET_G5_SOC) */
}

#ifdef __cplusplus
}
#endif

/******************************** END OF FILE ******************************/
