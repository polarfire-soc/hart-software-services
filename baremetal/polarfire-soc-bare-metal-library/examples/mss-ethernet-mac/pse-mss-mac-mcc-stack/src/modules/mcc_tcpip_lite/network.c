/**
  Network layer implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    network.c

  Summary:
    Network layer handling.

  Description:
    This file provides the network layer implementation for TCP/IP stack.

 */

/*

©  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this software 
and any derivatives exclusively with Microchip products. 
  
THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER EXPRESS, 
IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF 
NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE, OR ITS 
INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE 
IN ANY APPLICATION. 

IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL 
OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED 
TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY 
OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S 
TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED 
THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE TERMS. 

*/

/**
 Section: Included Files
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "network.h"
#include "tcpip_types.h"
#include "dhcp_client.h"
#include "arpv4.h"
#include "ipv4.h"
#include "tcpv4.h"
#include "rtcc.h"
#include "ethernet_driver.h"
#include "log.h"
#include "lldp.h"
#include "lldp_tlv_handler_table.h"
#include "ip_database.h"
#ifdef ENABLE_NETWORK_DEBUG
#define logMsg(msg, msgSeverity, msgLogDest)    logMessage(msg, LOG_KERN, msgSeverity, msgLogDest) 
#else
#define logMsg(msg, msgSeverity, msgLogDest)
#endif

time_t arpTimer;
static void Network_SaveStartPosition(void);
uint16_t networkStartPosition;

mss_mac_instance_t *g_tcp_mac = &g_mac0;

const char *network_errors[] =
{
    "ERROR",
    "SUCCESS",
    "LINK_NOT_FOUND",
    "BUFFER_BUSY",
    "TX_LOGIC_NOT_IDLE",
    "MAC_NOT_FOUND",
    "IP_WRONG_VERSION",
    "IPV4_CHECKSUM_FAILS",
    "DEST_IP_NOT_MATCHED",
    "ICMP_CHECKSUM_FAILS",
    "UDP_CHECKSUM_FAILS",
    "TCP_CHECKSUM_FAILS",
    "DMA_TIMEOUT",
    "PORT_NOT_AVAILABLE",
    "ARP_IP_NOT_MATCHED",
    "EAPol_PACKET_FAILURE",
    "INCORRECT_IPV4_HLEN",
    "IPV4_NO_OPTIONS",
    "TX_QUEUED",
    "IPV6_CHECKSUM_FAILS",
    "IPV6_LOCAL_ADDR_RESOLVE",
    "IPV6_LOCAL_ADDR_INVALID",
    "NO_GATEWAY",
    "ADDRESS_RESOLUTION",
    "GLOBAL_DESTINATION",
    "ARP_WRONG_HARDWARE_ADDR_TYPE",
    "ARP_WRONG_PROTOCOL_TYPE",
    "ARP_WRONG_HARDWARE_ADDR_LEN",
    "ARP_WRONG_PROTOCOL_LEN",
    "PACKET_TOO_SHORT",
    "TX_FAILURE"
};


void Network_Init(void)
{
   // ETH_Init();
    ARPV4_Init();
    IPV4_Init();
    DHCP_init();
    TCP_Init();
    rtcc_init();
    Network_WaitForLink();  
    timersInit();
    LOG_Init(LOG_DEFAULT);
}

void timersInit()
{
    time(&arpTimer);
    arpTimer += 10;  
}

void Network_WaitForLink(void)
{
    /* TODO: PMCS see what to do for this... */
/*    while(!ETH_CheckLinkUp()) NOP(); */
}

void Network_Manage(void)
{
    time_t now;
    static time_t nowPv = 0;

   // ETH_EventHandler();
    Network_Read(); // handle any packets that have arrived...
    DHCP_Manage(); // update the DHCP status every second

    // manage any outstanding timeouts
    time(&now);
    if(now >= arpTimer)
    {
        ARPV4_Update();
        arpTimer = now + 10;
    }    
    if(now > nowPv) // at least 1 second has elapsed
    {
        // is defined as a minimum of 1 seconds in RFC973
        TCP_Update();  // handle timeouts
        LLDP_DecTTR();
        setLLDPTick();
        nowPv = now;
    }
}


uint8_t rx_packet[1536]; /* This is the current packet we are processing */

void Network_Read(void)
{
    ethernetFrame_t header;
    char debug_str[80];
    uint8_t *pkt_ptr;
    uint32_t pkt_length;
    uint32_t queue_no;
    uint8_t * p_rx_packet;
    mss_mac_rx_desc_t *cdesc;
    void * caller_info;
    uint32_t temp_count;
#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
    uint64_t addr_temp;
#else
    uint32_t addr_temp;
#endif

    if(ethData.pktReady) /* At least one packet has arrived */
    {
        /*
         * Grab a copy of the initial values.
         *
         * For a first cut we will then process through the buffers until the
         * count goes to 0. If we do it fast enough we won't overrun...
         */
        queue_no    = ethData.rx_queue_no;
        p_rx_packet = ethData.p_rx_packet;
        pkt_length  = ethData.rx_pckt_length;
        cdesc       = ethData.rx_cdesc;
        caller_info = ethData.rx_caller_info;

        do
        {
            /*
             * PMCS: Todo figure out how to get packet into rx_packet[]...
             * Most likely are either access rx buffers directly and maintain the chain
             * or implement fifo using circular buffer and poll until packet arrives.
             */

            pkt_ptr = p_rx_packet;
            IP_MOVE_BYTES_IN(header, pkt_ptr, sizeof(header));
            header.id.type = ntohs(header.id.type); /* reverse the type field */
            switch (header.id.type)
            {
                case ETHERTYPE_VLAN:
                    logMsg("VLAN Packet Dropped", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                    break;
                case ETHERTYPE_ARP:
                    logMsg("RX ARPV4 Packet", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                    ARPV4_Packet(p_rx_packet, &pkt_ptr, pkt_length);
                    break;
                case ETHERTYPE_IPV4:
                    logMsg("RX IPV4 Packet", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                    IPV4_Packet(p_rx_packet, &pkt_ptr, pkt_length);
                    break;
                case ETHERTYPE_IPV6:
                    logMsg("RX IPV6 Packet Dropped", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                    break;
                case ETHERTYPE_LLDP:
                    logMsg("LLDP Packet Received", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                    LLDP_Packet(p_rx_packet, &pkt_ptr, pkt_length);
                    break;
                default:
                    {
                        long t = header.id.type;
                        if(t < 0x05dc) // this is a length field
                        {
                            sprintf(debug_str,"802.3 length 0x%04lX",t);
                        }
                        else
                            sprintf(debug_str,"802.3 type 0x%04lX",t);

                        logMsg(debug_str, LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                    }
                    break;
            }

            /* Make this function atomic w.r.to EMAC interrupt */
            /* PLIC_DisableIRQ() et al should not be called from the associated interrupt... */
            if(0U != g_tcp_mac->use_local_ints)
            {
                __disable_local_irq(g_tcp_mac->mac_q_int[queue_no]);
            }
            else
            {
                PLIC_DisableIRQ(g_tcp_mac->mac_q_int[queue_no]); /* Single interrupt from GEM? */
            }

            if(ethData.pktReady != 0) /* Sanity check to stop us going postal on the buffer if we decrement to 0xFFFFFFFF */
            {
                ethData.pktReady--;
            }

            temp_count = ethData.pktReady; /* Record the value in case it increments again after we enable ints... */

            /* Ethernet Interrupt Enable function. */
            /* PLIC_DisableIRQ() et al should not be called from the associated interrupt... */
            if(0U != g_tcp_mac->use_local_ints)
            {
                __enable_local_irq(g_tcp_mac->mac_q_int[queue_no]);
            }
            else
            {
                PLIC_EnableIRQ(g_tcp_mac->mac_q_int[queue_no]); /* Single interrupt from GEM? */
            }

            if(temp_count != 0) /* Some more packets to process? */
            {
                cdesc++;
                if(cdesc > &g_tcp_mac->queue[queue_no].rx_desc_tab[MSS_MAC_RX_RING_SIZE - 1])
                {
                    cdesc = &g_tcp_mac->queue[queue_no].rx_desc_tab[0];
                }

#if defined(MSS_MAC_64_BIT_ADDRESS_MODE)
                addr_temp  = (uint64_t)(cdesc->addr_low & ~(GEM_RX_DMA_WRAP | GEM_RX_DMA_USED | GEM_RX_DMA_TS_PRESENT));
                addr_temp |= (uint64_t)cdesc->addr_high << 32;
#else
                addr_temp = (cdesc->addr_low & ~(GEM_RX_DMA_WRAP | GEM_RX_DMA_USED | GEM_RX_DMA_TS_PRESENT));
#endif
                p_rx_packet = (uint8_t *)addr_temp;
                pkt_length = cdesc->status & (GEM_RX_DMA_BUFF_LEN | GEM_RX_DMA_JUMBO_BIT_13);
            }

        } while(temp_count != 0);
    }
}

