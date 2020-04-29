/**
  ICMP protocol implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    icmp.c

  Summary:
     This is the implementation of ICMP version 4 stack.

  Description:
    This source file provides the implementation of the API for the ICMP Echo Ping Request/Reply.

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
#include "tcpip_types.h"
#include "ethernet_driver.h"
#include "ipv4.h"
#include "icmp.h"
#include "ip_database.h"
#include "log.h"

#ifdef ENABLE_NETWORK_DEBUG
#define logMsg(msg, msgSeverity, msgLogDest)    logMessage(msg, LOG_ICMP, msgSeverity, msgLogDest)
#else
#define logMsg(msg, msgSeverity, msgLogDest)
#endif

/* Port 0 is N/A in both UDP and TCP */
uint16_t portUnreachable = 0;

/**
 * ICMP packet receive
 * @param ipv4_header
 * @return
 */
error_msg ICMP_Receive(uint8_t *packet, uint8_t **pkt_read, ipv4Header_p_t *ipv4Hdr)
{
    icmpHeader_t icmpHdr;
    error_msg ret = NET_SUCCESS;
    uint8_t *pkt_ptr = *pkt_read;
    
    IP_MOVE_BYTES_IN(icmpHdr, pkt_ptr, sizeof(icmpHeader_t));
    if(ipv4Hdr->dstIpAddress == SPECIAL_IPV4_BROADCAST_ADDRESS)
    {
        ret =  DEST_IP_NOT_MATCHED;
    }

    if(NET_SUCCESS == ret)
    {
        switch(ntohs((icmpTypeCodes_t)icmpHdr.typeCode))
        {
            case ECHO_REQUEST:
                logMsg("ICMP ECHO Request Received", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                {
                    *pkt_read = pkt_ptr;
                    ret = ICMP_EchoReply(packet, pkt_read, ipv4Hdr);
                }
                break;

            case UNASSIGNED_ECHO_TYPE_CODE_REQUEST_1:
                logMsg("ICMP ECHO Request 1 Received", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                {
                    *pkt_read = pkt_ptr;
                    ret = ICMP_EchoReply(packet, pkt_read, ipv4Hdr);
                }
                break;

            case UNASSIGNED_ECHO_TYPE_CODE_REQUEST_2:
                logMsg("ICMP ECHO Request 2 Received", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                {
                    *pkt_read = pkt_ptr;
                    ret = ICMP_EchoReply(packet, pkt_read, ipv4Hdr);
                }
                break;

            case DEST_PORT_UNREACHABLE:
                logMsg("ICMP Destination Port Unreachable Received", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                pkt_ptr += 4;
                IP_MOVE_BYTES_IN(*ipv4Hdr, pkt_ptr, sizeof(ipv4Header_p_t));
                if(5 == (ipv4Hdr->ihl_version & 0x0F))
                {
                    IP_READ_WORD_IN(portUnreachable, pkt_ptr);
                }
                break;
            default:
                break;
        }
    }

    return ret;
}

/**
 * ICMP Packet Start
 * @param icmp_header
 * @param dest_address
 * @param protocol
 * @param payload_length
 * @return
 */
uint8_t icmp_echo_reply[512]; /* PMCS: todo check size */

error_msg ICMP_EchoReply(uint8_t *packet, uint8_t **pkt_read, ipv4Header_p_t *ipv4Hdr)
{
    uint16_t cksm = 0;
    error_msg ret = NET_ERROR;
    uint16_t identifier;
    uint16_t sequence;
    uint8_t *pkt_ptr = *pkt_read;
    uint8_t *reply_ptr = icmp_echo_reply;

    (void)packet;

    IP_READ_WORD_IN(identifier, pkt_ptr);
    IP_READ_WORD_IN(sequence, pkt_ptr);
    ret = IPv4_Start(icmp_echo_reply, &reply_ptr, ipv4Hdr->srcIpAddress, ipv4Hdr->protocol);
    if(ret == NET_SUCCESS)
    {
        uint32_t icmp_cksm_start;
        uint16_t ipv4PayloadLength;
        uint16_t CopyLength;
        uint8_t *cksum_loc;

        ipv4PayloadLength = (uint16_t)(ipv4Hdr->length - (uint16_t)((ipv4Hdr->ihl_version & 0x0F) << 2));

        IP_WRITE_WORD_OUT(reply_ptr, ECHO_REPLY);
        cksum_loc = reply_ptr; /* Record position of checksum */
        IP_WRITE_WORD_OUT(reply_ptr, 0); /* checksum */

        IP_WRITE_WORD_OUT(reply_ptr, identifier);
        IP_WRITE_WORD_OUT(reply_ptr, sequence);
        
        /* copy the next N bytes from the RX buffer into the TX buffer */
        CopyLength = (uint16_t)((ipv4PayloadLength - sizeof(icmpHeader_t)) - 4);
        memcpy(reply_ptr, pkt_ptr, CopyLength);

        /* compute a checksum over the ICMP payload */
        icmp_cksm_start = sizeof(ethernetFrame_t) + sizeof(ipv4Header_p_t);
        cksm = ntohs(IPV4_ComputeChecksum(icmp_echo_reply + icmp_cksm_start, ipv4PayloadLength, 0));

        memcpy(cksum_loc, &cksm, 2);
        logMsg("ICMP Echo Reply Sent", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
        ret = IPV4_Send(icmp_echo_reply, ipv4PayloadLength);
    }
    else
    {
        logMsg("ICMP Echo Reply Failure", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
    }
    return(ret);
}

error_msg ICMP_PortUnreachable(uint8_t *packet, uint8_t **pkt_read, uint32_t srcIPAddress, uint32_t destIPAddress, uint16_t length)
{    
    error_msg ret = NET_ERROR;
    uint16_t cksm = 0;  
    uint8_t *reply_ptr = icmp_echo_reply;
    uint8_t *pkt_ptr = *pkt_read;
    
    (void)packet;

    if(srcIPAddress!=ipdb_getAddress())
    {
        return DEST_IP_NOT_MATCHED;
    }
    
    ret = IPv4_Start(icmp_echo_reply, &reply_ptr, destIPAddress, ICMP_TCPIP);
    if(ret == NET_SUCCESS)
    {        
        uint32_t icmp_cksm_start;
        uint8_t *cksum_loc;

        IP_WRITE_WORD_OUT(reply_ptr, DEST_PORT_UNREACHABLE);
        cksum_loc = reply_ptr; /* Record position of checksum */
        IP_WRITE_WORD_OUT(reply_ptr, 0); /* checksum */

        IP_WRITE_LWORD_OUT(reply_ptr, 0U); /* unused and next-hop */

        memcpy(reply_ptr, pkt_ptr, sizeof(ipv4Header_p_t) + length);

        icmp_cksm_start = sizeof(ethernetFrame_t) + sizeof(ipv4Header_p_t);
        cksm =  ntohs(IPV4_ComputeChecksum(icmp_echo_reply + icmp_cksm_start, (uint16_t)(sizeof(icmpHeader_t)+ sizeof(ipv4Header_p_t) + length), 0));

        memcpy(cksum_loc, &cksm, 2);
        logMsg("ICMP Port Unreachable  Sent", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
        ret = IPV4_Send(icmp_echo_reply, (uint16_t)(sizeof(icmpHeader_t) + sizeof(ipv4Header_p_t) + length));
       
    }
    return ret;
}

bool isPortUnreachable(uint16_t port)
{
    if(portUnreachable == port)
        return true;
    else
        return false;
}

void resetPortUnreachable(void)
{
    portUnreachable = 0;
}
