/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/* Add the required include Files
 */

/* This version of the driver uses either the Microsemi PolarFire SoC MSS
 * Ethernt MAC driver to implement the data transfer.
 */
/* MSS MAC based code... */
#include <string.h>
#include <stdint.h>

#include "mpfs_hal/mss_plic.h"
#include "config/hardware/hw_platform.h"
#include "mpfs_hal/mss_coreplex.h"

#include "drivers/mss_mac/mss_ethernet_registers.h"
#include "drivers/mss_mac/mss_ethernet_mac_user_config.h"
#include "drivers/mss_mac/mss_ethernet_mac_regs.h"
#include "drivers/mss_mac/mss_ethernet_mac.h"
#include "drivers/mss_mac/phy.h"

/* FreeRTOS headers */
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"
#include "netif/ppp/pppoe.h"

#include "mpfs_hal/mss_hal.h"

const uint8_t * sys_cfg_get_mac_address(void);

/* Define those to better describe your network interface. */
#define IFNAME0 'E0'

#define BUFFER_USED     1u
#define BUFFER_EMPTY    0u
#define RELEASE_BUFFER  BUFFER_EMPTY
#define RX_BUFFER_COUNT MSS_MAC_RX_RING_SIZE
#define TX_BUFFER_COUNT 1

uint32_t get_user_eth_speed_choice(void);

/* Buffers for Tx and Rx */
static uint8_t g_mac_tx_buffer[TX_BUFFER_COUNT][MSS_MAC_MAX_TX_BUF_SIZE] __attribute__ ((aligned (4)));
static uint8_t g_mac_rx_buffer[RX_BUFFER_COUNT][MSS_MAC_MAX_RX_BUF_SIZE] __attribute__ ((aligned (4)));

static volatile uint8_t g_mac_tx_buffer_used[TX_BUFFER_COUNT];
static volatile uint8_t g_mac_rx_buffer_data_valid[RX_BUFFER_COUNT];

struct netif * g_p_mac_netif = 0;

static uint8_t r_mac_addr[60] = {0};
static uint8_t r_mac_length = 0;
static uint8_t prev_packetset[6] = {0};

/* MAC configuration record */
mss_mac_cfg_t g_mac_config;
mss_mac_instance_t g_mac_instance;
xSemaphoreHandle xSemaphore = NULL;

static void mac_rx_callback
(
    /* mss_mac_instance_t */ void *this_mac,
    uint32_t queue_no,
    uint8_t * p_rx_packet,
    uint32_t pckt_length,
    mss_mac_rx_desc_t *cdesc,
    void * caller_info
);

static void ethernetif_input
(
    struct netif *netif,
    uint8_t * p_rx_packet,
    uint32_t pckt_length
);

static void packet_tx_complete_handler(/* mss_mac_instance_t*/ void *this_mac, uint32_t queue_no, mss_mac_tx_desc_t *cdesc, void * caller_info);

static void low_level_init(struct netif *netif);
static err_t low_level_output(struct netif *netif, struct pbuf *p);

static struct pbuf * low_level_input
(
    struct netif *netif,
    uint8_t * p_rx_packet,
    uint32_t pckt_length
);


/**=============================================================================
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif);
err_t
ethernetif_init(struct netif *netif)
{
#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    /*
     * Initialize the snmp variables and counters inside the struct netif.
     * The last argument should be replaced with your link speed, in units
     * of bits per second.
     */
    MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

    netif->state = 0;
    netif->name[0] = 'E';
    netif->name[1] = '0';

    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
#if LWIP_IPV4
    netif->output = etharp_output;
#endif
#if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
    netif->linkoutput = low_level_output;

    /* initialize the hardware */
    low_level_init(netif);

    return ERR_OK;
}

/**=============================================================================
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
    const uint8_t * own_hw_adr;
    int32_t count;

    /* We only have one network Interface */
    /* Initialize the Network interface */
    netif->num = 1;
    netif->hwaddr[0] = 0x00;
    netif->hwaddr[1] = 0xFC;
    netif->hwaddr[2] = 0x00;
    netif->hwaddr[3] = 0x12;
    netif->hwaddr[4] = 0x34;
    netif->hwaddr[5] = 0x56;

    netif->hwaddr_len = 6; /* Defined in LWIP ETHARP_HWADDR_LEN; */

    /* maximum transfer unit */
    netif->mtu = 1500;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
    /* RB : Added IGMP flag */
    netif->flags |= NETIF_FLAG_IGMP;

#if LWIP_IPV6 && LWIP_IPV6_MLD
    /*
     * For hardware/netifs that implement MAC filtering.
     * All-nodes link-local is handled by default, so we must let the hardware know
     * to allow multicast packets in.
     * Should set mld_mac_filter previously. */
    if (netif->mld_mac_filter != NULL) {
        ip6_addr_t ip6_allnodes_ll;
        ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
        netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
    }
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

    /* Keep track of netif pointer for use by interrupt service routine. */
    g_p_mac_netif = netif;
    
    /*--------------------- Initialize packet containers ---------------------*/
    g_mac_tx_buffer_used[0] = RELEASE_BUFFER;
    g_mac_rx_buffer_data_valid[0] = RELEASE_BUFFER;
    
    /*-------------------------- Initialize the MAC --------------------------*/
    /*
     * The interrupt can cause a context switch, so ensure its priority is
     * between configKERNEL_INTERRUPT_PRIORITY and
     * configMAX_SYSCALL_INTERRUPT_PRIORITY.
     */
/* Todo: What is needed for G5SoC? */
    /*
     * Get the default configuration for the Ethernet MAC and change settings
     * to match the system/application. The default values typically changed
     * are:
     *  - interface:
     *      Specifies the interface used to connect the Ethernet MAC to the PHY.
     *      Example choice are MII, GMII, TBI.
     *  - phy_addr:
     *      Specifies the MII management interface address of the external PHY.
     *  - mac_addr:
     *      Specifies the MAC address of the device. This number should be
     *      unique on the network the device is connected to.
     *  - speed_duplex_select:
     *      Specifies the allowed speed and duplex mode for setting up a link.
     *      This can be used to specify the set of allowed speeds and duplex
     *      modes used during auto-negotiation or force the link speed to a
     *      specific speed and duplex mode.
     */
    MSS_MAC_cfg_struct_def_init(&g_mac_config);

    g_mac_config.speed_duplex_select =  MSS_MAC_ANEG_100M_FD;//MSS_MAC_ANEG_ALL_SPEEDS;//get_user_eth_speed_choice();
    g_mac_config.mac_addr[0] = netif->hwaddr[0];
    g_mac_config.mac_addr[1] = netif->hwaddr[1];
    g_mac_config.mac_addr[2] = netif->hwaddr[2];
    g_mac_config.mac_addr[3] = netif->hwaddr[3];
    g_mac_config.mac_addr[4] = netif->hwaddr[4];
    g_mac_config.mac_addr[5] = netif->hwaddr[5];

#if MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_ALOE
    g_mac_config.phy_addr            = PHY_VSC8541_MDIO_ADDR;
    g_mac_config.phy_type            = MSS_MAC_DEV_PHY_VSC8541;
    g_mac_config.pcs_phy_addr        = SGMII_MDIO_ADDR;
    g_mac_config.interface_type      = GMII;
    g_mac_config.phy_autonegotiate   = MSS_MAC_VSC8541_phy_autonegotiate;
    g_mac_config.phy_get_link_status = MSS_MAC_VSC8541_phy_get_link_status;
    g_mac_config.phy_init            = MSS_MAC_VSC8541_phy_init;
    g_mac_config.phy_set_link_speed  = MSS_MAC_VSC8541_phy_set_link_speed;

#if MSS_MAC_USE_PHY_DP83867
    g_mac_config.phy_extended_read   = NULL_ti_read_extended_regs;
    g_mac_config.phy_extended_write  = NULL_ti_write_extended_regs;
#endif
#endif

#if MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_GMII
    g_mac_config.phy_addr            = PHY_VSC8575_MDIO_ADDR;
    g_mac_config.phy_type            = MSS_MAC_DEV_PHY_VSC8575;
    g_mac_config.pcs_phy_addr        = SGMII_MDIO_ADDR;
    g_mac_config.interface_type      = GMII_SGMII;
    g_mac_config.phy_autonegotiate   = MSS_MAC_VSC8575_phy_autonegotiate;
    g_mac_config.phy_get_link_status = MSS_MAC_VSC8575_phy_get_link_status;
    g_mac_config.phy_init            = MSS_MAC_VSC8575_phy_init;
    g_mac_config.phy_set_link_speed  = MSS_MAC_VSC8575_phy_set_link_speed;
#if MSS_MAC_USE_PHY_DP83867
    g_mac_config.phy_extended_read   = NULL_ti_read_extended_regs;
    g_mac_config.phy_extended_write  = NULL_ti_write_extended_regs;
#endif
#endif

#if MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_DUAL_EX_VTS
    g_mac_config.phy_addr            = PHY_VSC8575_MDIO_ADDR;
    g_mac_config.phy_type            = MSS_MAC_DEV_PHY_VSC8575;
    g_mac_config.pcs_phy_addr        = SGMII_MDIO_ADDR;
    g_mac_config.interface_type      = GMII_SGMII;
    g_mac_config.phy_autonegotiate   = MSS_MAC_VSC8575_phy_autonegotiate;
    g_mac_config.phy_get_link_status = MSS_MAC_VSC8575_phy_get_link_status;
    g_mac_config.phy_init            = MSS_MAC_VSC8575_phy_init;
    g_mac_config.phy_set_link_speed  = MSS_MAC_VSC8575_phy_set_link_speed;
#if MSS_MAC_USE_PHY_DP83867
    g_mac_config.phy_extended_read   = NULL_ti_read_extended_regs;
    g_mac_config.phy_extended_write  = NULL_ti_write_extended_regs;
#endif
#endif

#if MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_TI_GMII
    g_mac_config.phy_addr            = PHY_DP83867_MDIO_ADDR;
    g_mac_config.phy_type            = MSS_MAC_DEV_PHY_DP83867;
    g_mac_config.pcs_phy_addr        = SGMII_MDIO_ADDR;
    g_mac_config.interface_type      = GMII_SGMII;
    g_mac_config.phy_autonegotiate   = MSS_MAC_DP83867_phy_autonegotiate;
    g_mac_config.phy_get_link_status = MSS_MAC_DP83867_phy_get_link_status;
    g_mac_config.phy_init            = MSS_MAC_DP83867_phy_init;
    g_mac_config.phy_set_link_speed  = MSS_MAC_DP83867_phy_set_link_speed;
#if MSS_MAC_USE_PHY_DP83867
    g_mac_config.phy_extended_read   = ti_read_extended_regs;
    g_mac_config.phy_extended_write  = ti_write_extended_regs;
#endif
#endif

    vSemaphoreCreateBinary(xSemaphore);

    if( xSemaphore == NULL )
    {
        while(1);// could not create semaphore
    }

    /*
     * Initialize MAC with specified configuration. The Ethernet MAC is
     * functional after this function returns but still requires transmit and
     * receive buffers to be allocated for communications to take place.
     */
#if defined(MSS_MAC_USE_DDR)
    MSS_MAC_init((mss_mac_instance_t *)0xC0000000, &g_mac_config);
#else
#if defined(MSS_MAC_LWIP_USE_EMAC)
    MSS_MAC_init(&g_mac0, &g_mac_config);
    MSS_MAC_init(&g_emac0, &g_mac_config);
#else
#if defined(G5_SOC_EMU_USE_GEM0)
    MSS_MAC_init(&g_mac0, &g_mac_config);
#else
    MSS_MAC_init(&g_mac1, &g_mac_config);
#endif
#endif
#endif
    /*
     * Register MAC interrupt handler listener functions. These functions will
     * be called  by the MAC driver when a packet has been sent or received.
     * These callback functions are intended to help managing transmit and
     * receive buffers by indicating when a transmit buffer can be released or
     * a receive buffer has been filled with an rx packet.
     */
#if defined(G5_SOC_EMU_USE_GEM0)
    MSS_MAC_set_tx_callback(&g_mac0, 0, packet_tx_complete_handler);
    MSS_MAC_set_rx_callback(&g_mac0, 0, mac_rx_callback);
#else
    MSS_MAC_set_tx_callback(&g_mac1, packet_tx_complete_handler);
    MSS_MAC_set_rx_callback(&g_mac1, mac_rx_callback);
#endif
    /* 
     * Allocate receive buffers.
     *
     * We prime the pump with a full set of packet buffers and then re use them
     * as each packet is handled.
     *
     * This function will need to be called each time a packet is received to
     * hand back the receive buffer to the MAC driver.
     */
    for(count = 0; count < RX_BUFFER_COUNT; ++count)
    {
        /*
         * We allocate the buffers with the Ethernet MAC interrupt disabled
         * until we get to the last one. For the last one we ensure the Ethernet
         * MAC interrupt is enabled on return from MSS_MAC_receive_pkt().
         */
        if(count != (RX_BUFFER_COUNT - 1))
        {
#if defined(G5_SOC_EMU_USE_GEM0)
            MSS_MAC_receive_pkt(&g_mac0, 0, g_mac_rx_buffer[count], 0, 0);
#else
            MSS_MAC_receive_pkt(&g_mac1, 0, g_mac_rx_buffer[count], 0, 0);
#endif
        }
        else
        {
#if defined(G5_SOC_EMU_USE_GEM0)
            MSS_MAC_receive_pkt(&g_mac0, 0, g_mac_rx_buffer[count], 0, -1);
#else
            MSS_MAC_receive_pkt(&g_mac1, 0, g_mac_rx_buffer[count], 0, -1);
#endif
        }
    }
}

/**=============================================================================
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
    struct pbuf *q;
    uint16_t pckt_length = 0u;
    uint32_t pbuf_chain_end = 0u;
    
    uint8_t tx_status;

    (void)netif;
#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    /*--------------------------------------------------------------------------
     * Wait for packet buffer to become free.
     */
    // Block waiting for the semaphore to become available.
    if( xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE )
    {

        /*--------------------------------------------------------------------------
         * Copy pbuf chain into single buffer.
         */
        q = p;
        do {
            memcpy(&g_mac_tx_buffer[0][pckt_length], q->payload, q->len);
            pckt_length = (uint16_t)(pckt_length + q->len);
            if(q->len == q->tot_len)
            {
                pbuf_chain_end = 1u;
            }
            else
            {
                q = q->next;
            }
        } while(0u == pbuf_chain_end);

        /*--------------------------------------------------------------------------
         * Initiate packet transmit. Keep retrying until there is room in the MAC Tx
         * ring.
         */
        do {
#if defined(MSS_MAC_USE_DDR)
        	memcpy((void *)0xC0030000LLU, g_mac_tx_buffer[0], pckt_length);
        	tx_status = MSS_MAC_send_pkt((mss_mac_instance_t *)0xC0000000LLU, (void *)0xC0030000LLU, pckt_length, (void *)&g_mac_tx_buffer_used[0]);

#else
#if defined(G5_SOC_EMU_USE_GEM0)
        	tx_status = MSS_MAC_send_pkt(&g_mac0, 0, g_mac_tx_buffer[0], pckt_length, (void *)&g_mac_tx_buffer_used[0]);
#else
        	tx_status = MSS_MAC_send_pkt(&g_mac1, 0, g_mac_tx_buffer[0], pckt_length, (void *)&g_mac_tx_buffer_used[0]);
#endif
#endif
            if(MSS_MAC_SUCCESS != tx_status)
            {
            	vTaskDelay(1);
            }
        } while(MSS_MAC_SUCCESS != tx_status);
    }
    
    MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);
    if (((u8_t*)p->payload)[0] & 1) {
      /* broadcast or multicast packet*/
      MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
    } else {
      /* unicast packet */
      MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
    }
    /* increase ifoutdiscards or ifouterrors on error */
#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
    LINK_STATS_INC(link.xmit);
    return ERR_OK;
}

/**=============================================================================
 *
 */
extern BaseType_t g_mac_context_switch;

//static void packet_tx_complete_handler(void *this_mac, void * caller_info)
static void packet_tx_complete_handler(/* mss_mac_instance_t*/ void *this_mac, uint32_t queue_no, mss_mac_tx_desc_t *cdesc, void * caller_info)

{
    static signed portBASE_TYPE xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;

    (void)caller_info;
    (void)this_mac;

    // Unblock the task by releasing the semaphore.
    xSemaphoreGiveFromISR( xSemaphore, &xHigherPriorityTaskWoken );
    if(pdTRUE == xHigherPriorityTaskWoken)
    {
    	g_mac_context_switch = 1;
    }
}

/**=============================================================================
    Bottom-half of receive packet handler
*/

int _rx_counter = 0;
static void mac_rx_callback
(
    /* mss_mac_instance_t */ void *this_mac,
    uint32_t queue_no,
    uint8_t * p_rx_packet,
    uint32_t pckt_length,
    mss_mac_rx_desc_t *cdesc,
    void * caller_info
)
{
    (void)caller_info;
    if(g_p_mac_netif != 0)
    {
        ethernetif_input(g_p_mac_netif, p_rx_packet, pckt_length);
    }
    _rx_counter++;
}

/**=============================================================================
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void
ethernetif_input
(
    struct netif *netif,
    uint8_t * p_rx_packet,
    uint32_t pckt_length
)
{
    struct pbuf *p;

    /* move received packet into a new pbuf */
    p = low_level_input(netif, p_rx_packet, pckt_length);
    /* no packet could be read, silently ignore this */

    if (p != NULL)
    {
        /* pass all packets to ethernet_input, which decides what packets it supports */
        if (netif->input(p, netif) != ERR_OK)
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
            pbuf_free(p);
        }
    }
}

/**=============================================================================
 * 
 */
void ethernetif_tick(void);
void ethernetif_tick(void)
{
}

/**=============================================================================
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input
(
    struct netif *netif,
    uint8_t * p_rx_packet,
    uint32_t pckt_length
)
{
    struct pbuf *p, *q;
    uint16_t len;

    (void)netif;
    p = NULL;
    
    /* Obtain the size of the packet and put it into the "len"
       variable. */
    len = (uint16_t)pckt_length;

    if(len > 0)
    {

#if ETH_PAD_SIZE
        len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

        /* We allocate a pbuf chain of pbufs from the pool. */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
        if (p != NULL)
        {
            uint32_t length = 0;

#if ETH_PAD_SIZE
            pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
            /* We iterate over the pbuf chain until we have read the entire
             * packet into the pbuf. */
            for(q = p; q != NULL; q = q->next)
            {
                /* Read enough bytes to fill this pbuf in the chain. The
                 * available data in the pbuf is given by the q->len
                 * variable. */
                /* read data into(q->payload, q->len); */
                memcpy(q->payload, &p_rx_packet[length], q->len);
                length += q->len;
            }
            /* Reassign packet buffer for reception once we have copied it */
#if defined(G5_SOC_EMU_USE_GEM0)
            MSS_MAC_receive_pkt(&g_mac0, 0, p_rx_packet, 0, 1);
#else
            MSS_MAC_receive_pkt(&g_mac1, 0, p_rx_packet, 0, 1);
#endif
#if ETH_PAD_SIZE
            pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

            MIB2_STATS_NETIF_ADD(netif, ifinoctets, p->tot_len);
            if (((u8_t*)p->payload)[0] & 1) {
              /* broadcast or multicast packet*/
              MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
            } else {
              /* unicast packet*/
              MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
            }
            LINK_STATS_INC(link.recv);
        }
        else
        {
            /*
             *  Silently drop current packet and reassign packet buffer for
             *  reception
             */
#if defined(G5_SOC_EMU_USE_GEM0)
            MSS_MAC_receive_pkt(&g_mac0, 0, p_rx_packet, 0, 1);
#else
            MSS_MAC_receive_pkt(&g_mac1, 0, p_rx_packet, 0, 1);
#endif
            LINK_STATS_INC(link.memerr);
            LINK_STATS_INC(link.drop);
            MIB2_STATS_NETIF_INC(netif, ifindiscards);
        }
    }
    
    return p;
}

/**************************************************************************//**
 *
 */
void read_mac_address(uint8_t * mac_addr, uint8_t *length);
void read_mac_address(uint8_t * mac_addr, uint8_t *length)
{
    memcpy(mac_addr, r_mac_addr, r_mac_length);
    *length = r_mac_length;
}
/**************************************************************************//**
 *
 */
void clear_mac_buf(void);
void clear_mac_buf(void)
{
    r_mac_length = 0;
    memset(r_mac_addr, 0, 60);
    memset(prev_packetset, 0, 6);
}
