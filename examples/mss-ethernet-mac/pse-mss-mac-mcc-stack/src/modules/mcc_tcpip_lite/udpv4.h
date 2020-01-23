/**
  UDP v4 Protocol header file
	
  Company:
    Microchip Technology Inc.

  File Name:
    udpv4.h

  Summary:
    This is the header file for the udpv4.c

  Description:
    This header file provides the API for the UDP stack.

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

/*
 * File:   udpv4.h
 * Author: joe
 *
 * Created on December 18, 2012, 3:15 PM
 */

#ifndef UDPV4_H
#define	UDPV4_H



/**
  Section: Included Files
*/
#include "tcpip_types.h"
#include <stdbool.h>
#include "ethernet_driver.h"

extern uint16_t destPort;
extern udpHeader_t udpHeader;
extern ipv4Header_p_t ipv4HeaderP; // re evaluate this dependancy sometime

#define   UDP_ReadBlock(data,length)  ETH_ReadBlock(data,length)
#define   UDP_Read8()                 ETH_Read8()
#define   UDP_Read16()                ETH_Read16()
#define   UDP_Read24()                ETH_Read24()
#define   UDP_Read32()                ETH_Read32()
#define   UDP_Write8(data)            ETH_Write8(data)
#define   UDP_Write16(data)           ETH_Write16(data)
#define   UDP_Write24(data)           ETH_Write24(data)
#define   UDP_Write32(data)           ETH_Write32(data)
#define   UDP_WriteBlock(data,length) ETH_WriteBlock(data,length)
#define   UDP_WriteString(data)       ETH_WriteString(data)
#define   UDP_GetDestPort()           (destPort)
#define   UDP_GetSrcPort()            (udpHeader.dstPort)
#define   UDP_GetDestIP()             (ipv4HeaderP.srcIpAddress)
#define   UDP_GetSrcIP()              (ipv4HeaderP.dstIpAddress)
#define   UDP_DataLength()            ((udpHeader.length) - 8)
#define   UDP_FlushTXPackets()        ETH_TxReset()
#define   UDP_FlushRxdPacket()        ETH_Flush()

error_msg UDP_Start(uint8_t *packet, uint8_t **pkt_write_ptr, uint32_t destIP, uint16_t srcPort, uint16_t dstPort);
error_msg UDP_Send(uint8_t *packet, uint16_t udpLength);
error_msg UDP_Receive(uint8_t *packet, uint8_t **pkt_read, uint32_t pkt_length, uint16_t udpcksm);
void udp_test(uint8_t *packet, int len);


#endif	/* UDPV4_H */

