/**
  IPv4 implementation

  Company:
    Microchip Technology Inc.

  File Name:
    ipv4.c

  Summary:
    This is the implementation of IP version 4 stack

  Description:
    This source file provides the implementation of the API for the IP v4 stack.

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

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "network.h"
#include "ipv4.h"
#include "icmp.h"
#include "arpv4.h"
#include "udpv4.h"
#include "udpv4_port_handler_table.h"
#include "tcpv4.h"
#include "tcpip_types.h"
#include "ethernet_driver.h"
#include "log.h"
#include "ip_database.h"

#include "mpfs_hal/mss_plic.h"
#include "drivers/mss_mac/mss_ethernet_registers.h"
#include "drivers/mss_mac/mss_ethernet_mac_user_config.h"
#include "drivers/mss_mac/mss_ethernet_mac.h"

#ifdef ENABLE_NETWORK_DEBUG
#define logMsg(msg, msgSeverity, msgLogDest)    logMessage(msg, LOG_KERN, msgSeverity, msgLogDest)  
#else
#define logMsg(msg, msgSeverity, msgLogDest)
#endif

ipv4Header_t   ipv4Header;  /* Original MCC one which is non portable */
ipv4Header_u_t ipv4HeaderU; /* Unpacked structure with extracted fields */
ipv4Header_p_t ipv4HeaderP; /* Packed structure with merged fields */

uint32_t remoteIpv4Address;

void IPV4_Init(void)
{
    ipdb_init();
}

uint16_t IPV4_PseudoHeaderChecksum(uint16_t payloadLen)
{
    ipv4_pseudo_header_t tmp;
    uint8_t len;
    uint32_t cksm = 0;
    uint16_t *v;

    tmp.srcIpAddress  = ipv4HeaderP.srcIpAddress;
    tmp.dstIpAddress  = ipv4HeaderP.dstIpAddress;
    tmp.protocol      = ipv4HeaderP.protocol;
    tmp.z             = 0;
    tmp.length        = payloadLen;

    len = sizeof(tmp);
    len = (uint8_t)(len >> 1);

    v = (uint16_t *) &tmp;

    while(len)
    {
        cksm += *v;
        len--;
        v++;
    }

    // wrap the checksum
    while(cksm >> 16)
    {
        cksm = (cksm & 0x0FFFFU) + (cksm>>16);
    }

    // Return the resulting checksum
    return((uint16_t)cksm);
}


error_msg IPV4_Packet(uint8_t *packet, uint8_t **pkt_read, uint32_t packet_length)
{
    uint16_t cksm = 0;
    uint16_t length = 0;
    char msg[40];
    uint8_t hdrLen;
    uint8_t *pkt_ptr = *pkt_read;

    (void)packet;
    (void)packet_length;

    //calculate the IPv4 checksum
    hdrLen = (*pkt_ptr & 0x0FU) << 2;
    cksm = IPV4_ComputeChecksum(pkt_ptr, hdrLen, 0);
    if (cksm != 0)
    {
        return IPV4_CHECKSUM_FAILS;
    }
    
    IP_MOVE_BYTES_IN(ipv4HeaderP, pkt_ptr, sizeof(ipv4Header_p_t));
    if(((ipv4HeaderP.ihl_version >> 4) & 0x0F) != 4)
    {
        return IP_WRONG_VERSION; /* Incorrect version number */
    }

    /*
     * Do a speculative add to ARP table as we may need to respond to this
     * The add will only succeed if it is a unicast packet which is fine,
     * broadcast packets won't make it in and cause the cache to churn.
     */
    ARPV4_Add(ipv4HeaderP.dstIpAddress, ipv4HeaderP.srcIpAddress, ntohs(ETHERTYPE_IPV4), (mac48Address_t *)&packet[6]);

    ipv4HeaderP.dstIpAddress = ntohl(ipv4HeaderP.dstIpAddress);
    ipv4HeaderP.srcIpAddress = ntohl(ipv4HeaderP.srcIpAddress);

    if(ipv4HeaderP.srcIpAddress == SPECIAL_IPV4_BROADCAST_ADDRESS)
        return DEST_IP_NOT_MATCHED;

    // jira:M8TS-608
    if(ipv4HeaderP.dstIpAddress == ipdb_getAddress() ||
        (ipv4HeaderP.dstIpAddress == IPV4_ZERO_ADDRESS)||
        (ipv4HeaderP.dstIpAddress == SPECIAL_IPV4_BROADCAST_ADDRESS) ||
        ((ipdb_getAddress() | CLASS_A_IPV4_REVERSE_BROADCAST_MASK) == ipv4HeaderP.dstIpAddress) ||
        ((ipdb_getAddress() | CLASS_B_IPV4_REVERSE_BROADCAST_MASK) == ipv4HeaderP.dstIpAddress) ||
        ((ipdb_getAddress() | CLASS_C_IPV4_REVERSE_BROADCAST_MASK) == ipv4HeaderP.dstIpAddress) ||
        (ipv4HeaderP.dstIpAddress == ALL_HOST_MULTICAST_ADDRESS))
    {
        ipv4HeaderP.length = ntohs(ipv4HeaderP.length);

        if(hdrLen < 20)
            return INCORRECT_IPV4_HLEN;

        if (hdrLen > 20)                                      /* jira: CAE_MCU8-5737 */
        {
            /* Do not process the IPv4 Options field */
            pkt_ptr += hdrLen - sizeof(ipv4Header_p_t);
        }
        
        switch((ipProtocolNumbers)ipv4HeaderP.protocol)
        {
            case ICMP_TCPIP:
                {
                    /* calculate and check the ICMP checksum */
                    logMsg("IPv4 RX ICMP", LOG_INFO, LOG_DEST_CONSOLE);
                    if(ipv4HeaderP.dstIpAddress == IPV4_ZERO_ADDRESS)     // jira:M8TS-608
                    {
                        return DEST_IP_NOT_MATCHED;
                    }
                    length = (uint16_t)(ipv4HeaderP.length - hdrLen);
                    cksm = IPV4_ComputeChecksum(pkt_ptr, length, 0);

                    if (cksm == 0)
                    {
                        ICMP_Receive(packet, &pkt_ptr, &ipv4HeaderP);
                    }
                    else
                    {
                        sprintf(msg, "icmp wrong cksm : %x",cksm);
                        logMsg(msg, LOG_INFO, LOG_DEST_CONSOLE);
                        return ICMP_CHECKSUM_FAILS;
                    }
                }
                break;
            case UDP_TCPIP:
                /*  check the UDP header checksum */
                logMsg("IPv4 RX UDP", LOG_INFO, LOG_DEST_CONSOLE);
                length = (uint16_t)(ipv4HeaderP.length - hdrLen);
                cksm = IPV4_PseudoHeaderChecksum(length); /* Calculate pseudo header checksum */
                cksm = IPV4_ComputeChecksum(pkt_ptr, length, cksm); /* 1's complement of pseudo header checksum + 1's complement of UDP header, data */
                UDP_Receive(packet, &pkt_ptr, length, cksm);
                break;
            case TCP_TCPIP:
                /* accept only uni cast TCP packets */
                /* check the TCP header checksum */
                logMsg("IPv4 RX TCP", LOG_INFO, LOG_DEST_CONSOLE);
                length = (uint16_t)(ipv4HeaderP.length - hdrLen);
                cksm = IPV4_PseudoHeaderChecksum(length);
                IPV4_ComputeChecksum(pkt_ptr, length, cksm);

                /* accept only packets with valid CRC Header */
                if (cksm == 0 && (ipv4HeaderP.dstIpAddress != SPECIAL_IPV4_BROADCAST_ADDRESS) && (ipv4HeaderP.dstIpAddress != IPV4_ZERO_ADDRESS))
                {
                    remoteIpv4Address = ipv4HeaderP.srcIpAddress;
                    TCP_Recv(packet, &pkt_ptr, remoteIpv4Address, length);
                }else{
                    logMsg("IPv4 RX bad TCP chksm",LOG_DEBUG,LOG_DEST_CONSOLE);
                }
                break;
            default:
                pkt_ptr += ipv4HeaderP.length;
                break;
        }
        return(NET_SUCCESS);
    }
    else
    {
        return DEST_IP_NOT_MATCHED;
    }
}

error_msg IPv4_Start(uint8_t *packet, uint8_t **pkt_write_ptr, uint32_t destAddress, ipProtocolNumbers protocol)
{
    error_msg ret = NET_ERROR;
    // get the dest mac address
    const mac48Address_t *destMacAddress; // Renamed from macAddress per CAE_MCU8-5648
    uint32_t targetAddress;
    uint8_t *pkt_ptr = *pkt_write_ptr;
    bool got_address = true;

    (void)packet;

    // Check if we have a valid IPadress and if it's different then 127.0.0.1
    if(((ipdb_getAddress() != 0) || (protocol == UDP_TCPIP))
     && (ipdb_getAddress() != 0x7F000001))
    {
        if(((destAddress == SPECIAL_IPV4_BROADCAST_ADDRESS)
             |((destAddress | CLASS_A_IPV4_BROADCAST_MASK) == SPECIAL_IPV4_BROADCAST_ADDRESS)
                |((destAddress | CLASS_B_IPV4_BROADCAST_MASK )== SPECIAL_IPV4_BROADCAST_ADDRESS)
                    |((destAddress | CLASS_C_IPV4_BROADCAST_MASK) == SPECIAL_IPV4_BROADCAST_ADDRESS))==0) // this is NOT a broadcast message
        {
            /* See if destination is on our subnet or needs the help of a router? */
            if( ((destAddress ^ ipdb_getAddress()) & ipdb_getSubNetMASK()) == 0)
            {
                targetAddress = destAddress;
            }
            else
            {
                targetAddress = ipdb_getRouter();
            }
            destMacAddress = ARPV4_Lookup(targetAddress);
            if(destMacAddress == 0)
            {
                ret = ARPV4_Request(targetAddress); // schedule an arp request
                got_address = false;
            }
        }
        else
        {
            destMacAddress = &broadcastMAC;
        }

        if(got_address)
        {
            IP_MOVE_BYTES_OUT(pkt_ptr,  *destMacAddress, 6);  /* Destination Hardware Address */
            IP_MOVE_BYTES_OUT(pkt_ptr,  hostMacAddress, 6);  /* Source Hardware Address */
            IP_WRITE_WORD_OUT(pkt_ptr,  ETHERTYPE_IPV4);     /* Packet protocol type */
            IP_WRITE_WORD_OUT(pkt_ptr,  0x4500);             /* VERSION, IHL, DSCP, ECN */
            IP_WRITE_WORD_OUT(pkt_ptr,  0x0000);             /* total packet length */
            IP_WRITE_LWORD_OUT(pkt_ptr, 0xAA554000);         /* My IPV4 magic Number..., FLAGS, Fragment Offset */
            IP_WRITE_BYTE_OUT(pkt_ptr,  IPv4_TTL);           /* TTL */
            IP_WRITE_BYTE_OUT(pkt_ptr,  protocol);           /* protocol */
            IP_WRITE_WORD_OUT(pkt_ptr,  0x0000);             /* checksum. set to zero and overwrite with correct value */
            IP_WRITE_LWORD_OUT(pkt_ptr, ipdb_getAddress());
            IP_WRITE_LWORD_OUT(pkt_ptr, destAddress);

            // fill the pseudo header for checksum calculation
            ipv4HeaderP.srcIpAddress = ipdb_getAddress();
            ipv4HeaderP.dstIpAddress = destAddress;
            ipv4HeaderP.protocol = protocol;

            ret = NET_SUCCESS;
        }
    }

    *pkt_write_ptr = pkt_ptr;
    return ret;
}

error_msg IPV4_Send(uint8_t *packet, uint16_t payloadLength)
{
    uint16_t totalLength;
    uint16_t cksm;
    error_msg ret_val;
    uint8_t *pkt_ptr = packet + OFFSET_IPV4_LEN;
    uint8_t tx_status;

    totalLength = (uint16_t)(20U + payloadLength);        /* Add in header length */
    IP_WRITE_WORD_OUT(pkt_ptr, totalLength); /* Insert IPv4 Total Length */

    cksm = IPV4_ComputeChecksum(packet + LEN_ETH_HEADER, LEN_BASE_IPV4_HEADER, 0);

    //Insert Ipv4 Header Checksum
    pkt_ptr = packet + OFFSET_IPV4_CHECKSUM;
    IP_WRITE_WORD_OUT(pkt_ptr, cksm);

    tx_status = MSS_MAC_send_pkt(&g_mac0, 0, packet, (uint32_t)(totalLength + LEN_ETH_HEADER), (void *)0);
    if(tx_status != MSS_MAC_SUCCESS)
    {
        ret_val = TX_FAILURE;
    }
    else
    {
        ret_val = NET_SUCCESS;
    }

    return ret_val;
}

uint16_t IPV4_GetDatagramLength(void)
{
    return ((uint16_t)((ipv4HeaderP.length) - sizeof(ipv4Header_p_t)));
}


uint16_t IPV4_ComputeChecksum(uint8_t *packet, uint16_t len, uint16_t seed)
{
    uint32_t cksm;
    uint16_t value;
    uint8_t *pkt_ptr = packet;

    cksm = seed;

    while(len > 1)
    {
        ((uint8_t *)&value)[1] = *pkt_ptr++;
        ((uint8_t *)&value)[0] = *pkt_ptr++;
        cksm += value;
        len -= 2;
    }

    if(len)
    {
        ((uint8_t *)&value)[1] = *pkt_ptr++;
        ((uint8_t *)&value)[0] = 0;
        cksm += value;
    }

    // wrap the checksum
    while(cksm >> 16)
    {
        cksm = (cksm & 0x0FFFFU) + (cksm>>16);
    }

    // invert the number.
    cksm = ~cksm;

    // Return the resulting checksum
    return (uint16_t)cksm;
}
