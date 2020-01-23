/**
  UDP protocol v4 implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    udpv4.c

  Summary:
     This is the implementation of UDP version 4 protocol

  Description:
    This source file provides the implementation of the API for the UDP v4 protocol.

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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include "ipv4.h"
#include "udpv4.h"
#include "udpv4_port_handler_table.h"
#include "network.h"
#include "ethernet_driver.h"
#include "tcpip_types.h"
#include "icmp.h"

/**
  Section: Macro Declarations
*/

/**
  Section: Local Variables
*/

uint16_t destPort;
udpHeader_t udpHeader;

/**
  Section: UDP Library APIs
*/

error_msg UDP_Start(uint8_t *packet, uint8_t **pkt_write_ptr, uint32_t destIP, uint16_t srcPort, uint16_t dstPort)
{
     // Start a UDP Packet to Write UDP Header
    error_msg ret = NET_ERROR;
    uint8_t *pkt_ptr;

    // Start IPv4 Packet to Write IPv4 Header
    ret = IPv4_Start(packet, pkt_write_ptr, destIP, UDP_TCPIP);
    pkt_ptr = *pkt_write_ptr;
    if(ret == NET_SUCCESS)
    {
        IP_WRITE_WORD_OUT(pkt_ptr, srcPort); /* Write UDP Source Port */
        IP_WRITE_WORD_OUT(pkt_ptr, dstPort); /* Write UDP Destination Port */
        IP_WRITE_WORD_OUT(pkt_ptr, 0);       /* Write DataLength; Initially set to '0' */
        IP_WRITE_WORD_OUT(pkt_ptr, 0);       /* Write UDP Checksum; Initially set to '0' */
    }

    *pkt_write_ptr = pkt_ptr;
    return ret;
}

error_msg UDP_Send(uint8_t *packet, uint16_t udpLength)
{
    uint16_t cksm;
    error_msg ret = NET_ERROR;
    uint8_t *pkt_ptr = packet + OFFSET_UDP_LEN;

    IP_WRITE_WORD_OUT(pkt_ptr, udpLength);
    
    /*
     * Add the UDP header checksum
     *
     * We take into account the TCP Pseudo header which consists of source address,
     * destination address, 1 byte of 0, 1 byte with the protocol number and the
     * overall packet length by relying on the fact that the 16 bit checksum field
     * and the 16 bit length field currently are 0. We add the length and protocol
     * number and use these as the initial seed value for the checksum calculation
     * and then set the payload checksum area to start at the source address in the
     * IPv4 header 8 bytes before the UDP header and increase the size to checksum
     * by 8... This works when we don't have any IP header options to transmit.
     */
    cksm = (uint16_t)(udpLength + UDP_TCPIP); /* Add in length and protocol component */
    cksm = IPV4_ComputeChecksum(packet + (OFFSET_UDP - 8), (uint16_t)(udpLength + 8), cksm);

    /* if the computed checksum is "0" set it to 0xFFFF */
    if (cksm == 0)
    {
        cksm = 0xffff;
    }

    pkt_ptr = packet + OFFSET_UDP_CHECKSUM;
    IP_WRITE_WORD_OUT(pkt_ptr, cksm);

    ret = IPV4_Send(packet, udpLength);

    return ret;
}

error_msg UDP_Receive(uint8_t *packet, uint8_t **pkt_read, uint32_t pkt_length, uint16_t udpcksm) // catch all UDP packets and dispatch them to the appropriate callback
{
    error_msg ret = NET_ERROR;
    udp_table_iterator_t  hptr;
    uint8_t *pkt_ptr = *pkt_read;

    (void)pkt_length;
    IP_MOVE_BYTES_IN(udpHeader, pkt_ptr, sizeof(udpHeader)); /* PMCS: todo check structure alignment */
    if((udpHeader.checksum == 0) || (udpcksm == 0))
    {
        udpHeader.dstPort = ntohs(udpHeader.dstPort); // reverse the port number
        destPort = ntohs(udpHeader.srcPort);
        udpHeader.length = ntohs(udpHeader.length);
        ret = PORT_NOT_AVAILABLE;
        // scan the udp port handlers and find a match.
        // call the port handler callback on a match
        hptr = udp_table_getIterator();
        
        while(hptr != NULL)
        {
            if(hptr->portNumber == udpHeader.dstPort)
            {          
                if(udpHeader.length == IPV4_GetDatagramLength())
                {
                    hptr->callBack(pkt_ptr, (int16_t)(udpHeader.length - sizeof(udpHeader)));
                }
                ret = NET_SUCCESS;
                break;
            }
            hptr = udp_table_nextEntry(hptr);
        }
        if(ret== PORT_NOT_AVAILABLE)
        {
            
            //Send Port unreachable                
            ICMP_PortUnreachable(packet, &pkt_ptr, UDP_GetSrcIP(), UDP_GetDestIP(), DEST_UNREACHABLE_LEN);  //jira: CAE_MCU8-5706
        }
    }
    else
    {
        ret = UDP_CHECKSUM_FAILS;
    }
    return ret;
}

void udp_test(uint8_t *packet, int len)    // print the UDP packet
{
    uint8_t *pkt_ptr = packet;
    uint8_t  character;

    while(len--)
    {
        IP_READ_BYTE_IN(character, pkt_ptr);
#if 0
        putch(character);
#endif
    }
}
