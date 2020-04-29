/**
  UDP protocol v4 implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    udpv4_port_handler_table.c

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
#include "tcpip_config.h"
#include "dns_client.h"
#include "dhcp_client.h"
#include "ntp.h"
#include "udpv4_port_handler_table.h"
#include "tftp.h"

void BACnet_Handler(uint8_t *packet, int16_t length);

const udp_handler_t UDP_CallBackTable[] = \
{    
    {53,  DNS_Handler},
    {68, DHCP_Handler},     
    {123, NTP_Handler},
    {0xBAC0, BACnet_Handler},
    {SOURCEPORT_TFTP, TFTP_Handler},
};

// ***************** Leave the stuff below this line alone *********************

udp_table_iterator_t udp_table_getIterator(void)
{
    return (udp_table_iterator_t) UDP_CallBackTable;
}

udp_table_iterator_t udp_table_nextEntry(udp_table_iterator_t i)
{
    i ++;
    if(i < UDP_CallBackTable + sizeof(UDP_CallBackTable))
    {
        return (udp_table_iterator_t) i;
    }
    else
        return (udp_table_iterator_t) NULL;
}



