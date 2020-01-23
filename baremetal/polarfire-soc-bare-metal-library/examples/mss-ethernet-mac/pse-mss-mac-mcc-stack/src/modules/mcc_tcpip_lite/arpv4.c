/**
 ARP v4 implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    arpv4.c

  Summary:
    This is the implementation of ARP version 4 stack

  Description:
    This source file provides the implementation of the API for the ARP v4 stack.

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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tcpip_types.h"
#include "network.h"
#include "arpv4.h"
#include "ethernet_driver.h"
#include "mac_address.h"
#include "ipv4.h"// needed to know my IP address
#include "tcpip_config.h"
#include "ip_database.h"

#include "mpfs_hal/mss_plic.h"
#include "drivers/mss_mac/mss_ethernet_registers.h"
#include "drivers/mss_mac/mss_ethernet_mac_user_config.h"
#include "drivers/mss_mac/mss_ethernet_mac.h"

typedef struct
{
    uint16_t htype;         // Hardware Type
    uint16_t ptype;         // Protocol Type
    uint8_t  hlen;          // Hardware Address Length
    uint8_t  plen;          // Protocol Address Length
    uint16_t oper;          // Operation
    mac48Address_t  sha;    // Sender Hardware Address
    uint32_t spa;           // Sender Protocol Address
    mac48Address_t  tha;    // Target Hardware Address
    uint32_t tpa;           // Target Protocol Address
} arpHeader_t;

/*
 * Use two static buffers for this, could use one but might be useful to be able
 * to examine last request and last response after the events...
 */
uint8_t arp_response[60]; /* Minimum packet size excluding Ethernet CRC */
uint8_t arp_request[60];

#define ARP_REQUEST 1
#define ARP_REPLY   2
#define ARP_NAK     10

mac48Address_t hostMacAddress;

arpMap_t arpMap[ARP_MAP_SIZE]; // maintain a small database of IP address & MAC addresses

/**
 * ARP Initialization
 */

void ARPV4_Init(void)
{
    memset(&arpMap, 0, sizeof(arpMap));
    memcpy((uint8_t*)&hostMacAddress, g_tcp_mac->mac_addr, sizeof(g_tcp_mac->mac_addr));
}

/**
 * ARP Packet received
 * @return
 */

error_msg ARPV4_Packet(uint8_t *packet, uint8_t **pkt_read, uint32_t pkt_length)
{
    arpHeader_t  header;
    error_msg    ret_val;
    uint8_t     *pkt_ptr = *pkt_read;
    uint8_t      tx_status;

    (void)packet;

    ret_val = NET_SUCCESS; /* Ever the optimist... */

    if(pkt_length >= sizeof(arpHeader_t))
    {
        /* Unpack the ARP info, making no assumptions about alignment... */
        IP_MOVE_WORD_IN(header.htype, pkt_ptr);         /* Hardware Type */
        IP_MOVE_WORD_IN(header.ptype, pkt_ptr);         /* Protocol Type */
        IP_MOVE_BYTE_IN(header.hlen,  pkt_ptr);         /* Hardware Address Length */
        IP_MOVE_BYTE_IN(header.plen,  pkt_ptr);         /* Protocol Address Length */
        IP_MOVE_WORD_IN(header.oper,  pkt_ptr);         /* Operation */
        IP_MOVE_BYTES_IN(header.sha,  pkt_ptr, 6);      /* Sender Hardware Address */
        IP_MOVE_LWORD_IN(header.spa,  pkt_ptr);         /* Sender Protocol Address */
        IP_MOVE_BYTES_IN(header.tha,  pkt_ptr, 6);      /* Target Hardware Address */
        IP_MOVE_LWORD_IN(header.tpa,  pkt_ptr);         /* Target Protocol Address */

        /* Last error here wins */
        if(htons(header.htype) != INETADDRESSTYPE_IPV4)
        {
            ret_val =  ARP_WRONG_HARDWARE_ADDR_TYPE;
        }

        if(htons(header.ptype) != ETHERTYPE_IPV4)
        {
            ret_val = ARP_WRONG_PROTOCOL_TYPE;
        }

        if(header.hlen != ETHERNET_ADDR_LEN)
        {
            ret_val = ARP_WRONG_HARDWARE_ADDR_LEN;
        }

        if(header.plen != IP_ADDR_LEN)
        {
            ret_val =  ARP_WRONG_PROTOCOL_LEN;
        }

        /* searching the arp table for a matching ip & protocol */
        if(ret_val == NET_SUCCESS)
        {
            /* assume that all hardware & protocols are supported */
            ARPV4_Add(header.tpa, header.spa, header.ptype, &header.sha);

            /* Was this sent to our IP address? */
            if(ipdb_getAddress() && (ipdb_getAddress() == ntohl(header.tpa)))
            {
                /* We have an assigned IP address and it's an ARP request so respond */
                if(header.oper == ntohs(ARP_REQUEST))
                {
                    /* Build our response packet */
                    memset(arp_response, 0, sizeof(arp_response));
                    pkt_ptr = arp_response;

                    IP_MOVE_BYTES_OUT(pkt_ptr, header.sha, 6);      /* Destination Hardware Address */
                    IP_MOVE_BYTES_OUT(pkt_ptr, hostMacAddress, 6);  /* Source Hardware Address */
                    IP_WRITE_WORD_OUT(pkt_ptr, ETHERTYPE_ARP);      /* Packet type high/low order*/

                    header.tha.s = header.sha.s;
                    memcpy((void*)&header.sha.s, (void*)&hostMacAddress.s, sizeof(mac48Address_t));
                    header.tpa = header.spa;
                    header.spa = htonl(ipdb_getAddress());
                    header.oper = htons(ARP_REPLY);

                    IP_MOVE_WORD_OUT(pkt_ptr, header.htype);        /* Hardware Type */
                    IP_MOVE_WORD_OUT(pkt_ptr, header.ptype);        /* Protocol Type */
                    IP_MOVE_BYTE_OUT(pkt_ptr, header.hlen);         /* Hardware Address Length */
                    IP_MOVE_BYTE_OUT(pkt_ptr, header.plen);         /* Protocol Address Length */
                    IP_MOVE_WORD_OUT(pkt_ptr, header.oper);         /* Operation */
                    IP_MOVE_BYTES_OUT(pkt_ptr, header.sha, 6);      /* Sender Hardware Address */
                    IP_MOVE_LWORD_OUT(pkt_ptr, header.spa);         /* Sender Protocol Address */
                    IP_MOVE_BYTES_OUT(pkt_ptr, header.tha, 6);      /* Target Hardware Address */
                    IP_MOVE_LWORD_OUT(pkt_ptr, header.tpa);         /* Target Protocol Address */

                    tx_status = MSS_MAC_send_pkt(&g_mac0, 0, arp_response, sizeof(arp_response), (void *)0);
                    if(tx_status != MSS_MAC_SUCCESS)
                    {
                        ret_val = TX_FAILURE;
                    }
                }
            }
            else
            {
                ret_val = ARP_IP_NOT_MATCHED;
            }
        }
    }
    else
    {
        ret_val = PACKET_TOO_SHORT;
    }

    return ret_val;
}

/**
 * Adds a new entry or updates an existing entry in the table.
 * May cause ARP table entries to be dropped.
 */

void ARPV4_Add(uint32_t tpa, uint32_t spa, uint16_t ptype, mac48Address_t *mac_address)
{
    arpMap_t *entryPointer = arpMap;
    bool      mergeFlag;
    uint32_t  index;

    /* assume that all hardware & protocols are supported */
    mergeFlag = false;
    entryPointer = arpMap;

    for(index = ARP_MAP_SIZE; index > 0; index--)
    {
        if( (ntohl(spa) == entryPointer->ipAddress) && (ptype == entryPointer->protocolType))
        {
            entryPointer->age = 0; /* reset the age */
            entryPointer->macAddress.s = mac_address->s;
            mergeFlag = true;
            break;
        }

        entryPointer++;
    }

    /* Only try inserting if the packet was a unicast directed at us */
    if(ipdb_getAddress() && (ipdb_getAddress() == ntohl(tpa)) && !mergeFlag)
    {
        /* find the oldest entry in the table */
        entryPointer = arpMap;
        arpMap_t *arpPtr = arpMap;

        for(index = ARP_MAP_SIZE; index != 0; index--)
        {
            if(entryPointer->age < arpPtr->age)
            {
                entryPointer = arpPtr;
            }
            /* Increment the pointer to get the next element from the array. */
            arpPtr++;
        }
        /* the entry_pointer is now pointing to the oldest entry */
        /* replace the entry with the received data */
        entryPointer->age = 0;
        entryPointer->macAddress.s = mac_address->s;
        entryPointer->ipAddress    = ntohl(spa);
        entryPointer->protocolType = ptype;
    }
}


/**
 * Updates the ARP table
 */

void ARPV4_Update(void) // call me every 10 seconds or so and I will age the arp table.
{
    arpMap_t *entryPointer = arpMap;
    uint32_t index;

    /* PMCS: Todo increments arp times irrespective of whether the entry is in use or not... */
    for(index = 0; index < ARP_MAP_SIZE; index++)
    {
        entryPointer->age++;
        entryPointer++;
    }
}

/**
 * ARP send Request
 * @param dest_address
 * @return
 */
error_msg ARPV4_Request(uint32_t destAddress)
{
    error_msg  ret_val;
    uint8_t   *pkt_ptr;
    uint8_t    tx_status;

    ret_val = NET_SUCCESS;

    arpHeader_t header;
    header.htype = htons(1);
    header.ptype = htons(0x0800);
    header.hlen = 6;
    header.plen = 4;
    header.oper = htons(ARP_REQUEST);
    memcpy((void*)&header.sha, (void*)&hostMacAddress, sizeof(mac48Address_t));
    header.spa = htonl(ipdb_getAddress());
    header.tpa= htonl(destAddress);
    header.tha.s.byte1 = 0;
    header.tha.s.byte2 = 0;
    header.tha.s.byte3 = 0;
    header.tha.s.byte4 = 0;
    header.tha.s.byte5 = 0;
    header.tha.s.byte6 = 0;

    memset(arp_response, 0, sizeof(arp_response));
    pkt_ptr = arp_response;

    IP_MOVE_BYTES_OUT(pkt_ptr,  broadcastMAC, 6);  /* Destination Hardware Address */
    IP_MOVE_BYTES_OUT(pkt_ptr,  hostMacAddress, 6);  /* Source Hardware Address */
    IP_WRITE_WORD_OUT(pkt_ptr,  ETHERTYPE_ARP);     /* Packet protocol type */

    IP_MOVE_WORD_OUT(pkt_ptr, header.htype);        /* Hardware Type */
    IP_MOVE_WORD_OUT(pkt_ptr, header.ptype);        /* Protocol Type */
    IP_MOVE_BYTE_OUT(pkt_ptr, header.hlen);         /* Hardware Address Length */
    IP_MOVE_BYTE_OUT(pkt_ptr, header.plen);         /* Protocol Address Length */
    IP_MOVE_WORD_OUT(pkt_ptr, header.oper);         /* Operation */
    IP_MOVE_BYTES_OUT(pkt_ptr, header.sha, 6);      /* Sender Hardware Address */
    IP_MOVE_LWORD_OUT(pkt_ptr, header.spa);         /* Sender Protocol Address */
    IP_MOVE_BYTES_OUT(pkt_ptr, header.tha, 6);      /* Target Hardware Address */
    IP_MOVE_LWORD_OUT(pkt_ptr, header.tpa);         /* Target Protocol Address */

    tx_status = MSS_MAC_send_pkt(&g_mac0, 0, arp_response, sizeof(arp_response), (void *)0);
    if(tx_status != MSS_MAC_SUCCESS)
    {
        ret_val = TX_FAILURE;
    }
    else
    {
        ret_val = MAC_NOT_FOUND; /* Signal back up the chain that we had to start an ARP resolution cycle */
    }

    return ret_val;
}


/**
 * ARP Lookup Table
 * @param ip_address
 * @return
 */
mac48Address_t* ARPV4_Lookup(uint32_t ip_address)
{
    arpMap_t *entry_pointer = arpMap;
    uint32_t index;
    mac48Address_t *ret_val = (mac48Address_t *)0;
    
    for(index = 0; (index < ARP_MAP_SIZE) && (ret_val == (mac48Address_t *)0); index++)
    {
        if(entry_pointer->ipAddress == ip_address)
        {
            ret_val = &entry_pointer->macAddress;
        }

        entry_pointer++;
    }

    return ret_val;
}
