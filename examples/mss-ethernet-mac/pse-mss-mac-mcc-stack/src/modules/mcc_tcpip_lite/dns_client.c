/**
  DNS client implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    dns_client.c

  Summary:
    This is the implementation of DNS client.

  Description:
    This source file provides the implementation of the DNS client stack.

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
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "dns_client.h"
#include "ipv4.h"
#include "udpv4.h"

#include "log.h"
#include "ip_database.h"

#ifdef ENABLE_NETWORK_DEBUG
#define logMsg(msg, msgSeverity, msgLogDest)    logMessage(msg, LOG_DNS, msgSeverity, msgLogDest)
#else
#define logMsg(msg, msgSeverity, msgLogDest)
#endif

typedef struct
{
    const char *dnsName;
    uint32_t ipAddress;
    uint32_t ttl;
    bool isValid;
    time_t birthSecond;
} dns_map_t;

const uint16_t dnsXidValue = 0x1234;

#define DNS_HEADER_SIZE 12
#define DNS_TYPE_SIZE   2
#define DNS_CLASS_SIZE  2
#define DNS_QUERY_SIZE (DNS_TYPE_SIZE + DNS_CLASS_SIZE)
#define DNS_PACKET_SIZE (DNS_HEADER_SIZE + DNS_QUERY_SIZE)

#define ARRAYSIZE(a)    (sizeof(a) / sizeof(*(a)))

#define DNS_MAP_SIZE 8
dns_map_t dnsCache[DNS_MAP_SIZE]; // maintain a small database of IP address & Host name

/* PMCS: Todo - check size of buffer required */
uint8_t dns_query[512];

void DNS_Init(void)
{
    memset(dnsCache,0,sizeof(dnsCache));
}

void DNS_Query(const char *str)
{    
    uint32_t lock = 0;
    uint32_t index;
    uint32_t len_str;
    error_msg started;
    dns_map_t *entryPointer;
    dns_map_t *oldestEntry;
    time_t oldestAge;
    uint8_t *pkt_ptr;
        
    len_str = (uint32_t)strlen((char*)str);
    logMsg("DNS Query", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
    
    entryPointer = dnsCache;
    oldestEntry  = entryPointer;
    oldestAge    = entryPointer->birthSecond;
    
    /* find the oldest cache entry */
    for(index = 0; index < ARRAYSIZE(dnsCache); index++)
    {
        if(entryPointer->birthSecond > oldestAge)
        {
            oldestAge   = entryPointer->birthSecond;
            oldestEntry = entryPointer;
        }
        entryPointer++;
    }
    entryPointer = oldestEntry;
    pkt_ptr = dns_query;
    started = UDP_Start(dns_query, &pkt_ptr, ipdb_getDNS(),53,53);
    if(started == NET_SUCCESS)
    {
        /* DNS Header */
        IP_WRITE_WORD_OUT(pkt_ptr, dnsXidValue);
        IP_WRITE_WORD_OUT(pkt_ptr, 0x0100); /* QR Opcode AA|TC|RD|RA| Z  RCODE */
        IP_WRITE_WORD_OUT(pkt_ptr, 0x0001); /* QDCOUNT Questions */
        IP_WRITE_WORD_OUT(pkt_ptr, 0x0000); /* ANCOUNT Answer RRs */
        IP_WRITE_WORD_OUT(pkt_ptr, 0x0000); /* NSCOUNT Authority RRs */
        IP_WRITE_WORD_OUT(pkt_ptr, 0x0000); /* ARCOUNT Additional RRs */

         /* DNS Query */
        for(index = 0 ; index <= len_str ; index++)
        {
            if(str[index]=='.' || str[index]=='\0')
            {
                //len = i-lock;
                IP_WRITE_BYTE_OUT(pkt_ptr, (index - lock));
                for(; lock < index; lock++)    // jira:M8TS-608
                {
                    /* dns_q[lock+1]=str[lock]; */
                    IP_WRITE_BYTE_OUT(pkt_ptr, str[lock]);
                }

                lock = index + 1;
            }
        }
        IP_WRITE_BYTE_OUT(pkt_ptr, 0x00); /* End of String */

        IP_WRITE_WORD_OUT(pkt_ptr, 0x0001); /* QTYPE  Type - We use type 'A' for Ipv4 host address */
        IP_WRITE_WORD_OUT(pkt_ptr, 0x0001); /* QCLASS Class */

        UDP_Send(dns_query, (uint16_t)(pkt_ptr - dns_query));
        entryPointer->dnsName = str;
        entryPointer->isValid = false;
    }

}

void DNS_Handler(uint8_t *packet, int16_t length)    /* jira:M8TS-608 */
{
    uint16_t  xid_value;
    unsigned char dnsR[255];
    uint16_t answer;
    uint16_t authorityRR;
    uint32_t ipAddress = 0U;
    uint32_t ttl = 0U;
    uint16_t type, dataLength;
    uint8_t  temp_byte;
    uint16_t temp_word;
    uint32_t temp_lword;
    uint8_t *pkt_ptr;
    uint32_t index;
    dns_map_t *entryPointer;
    uint8_t nameLen;
    uint8_t lock =0;

    logMsg("DNS Packet Received", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
    
    entryPointer = dnsCache;
    pkt_ptr = packet;
    if(length > DNS_HEADER_SIZE)
    {
        IP_READ_WORD_IN(xid_value, pkt_ptr);
        if(xid_value == dnsXidValue)
        {
            IP_READ_LWORD_IN(temp_lword, pkt_ptr); /* Flags, Questions */
            IP_READ_WORD_IN(answer, pkt_ptr);      /* Answer RRs */
            IP_READ_WORD_IN(authorityRR, pkt_ptr); /* Authority RRs */
            IP_READ_WORD_IN(temp_word, pkt_ptr);   /* Additional RRs */
            length -= 12;

            if(length > 0)
            {
                IP_READ_BYTE_IN(nameLen, pkt_ptr);
                while(nameLen != 0x00)
                {
                    while(nameLen--)
                    {
                        IP_READ_BYTE_IN(dnsR[lock], pkt_ptr);
                        lock++;
                    }

                    dnsR[lock] = '.';
                    lock++;

                    IP_READ_BYTE_IN(nameLen, pkt_ptr);
                }

                dnsR[lock-1] ='\0';

                IP_READ_LWORD_IN(temp_lword, pkt_ptr);
                length -= lock + 5;
                /* DNS Answers */
                while(answer)
                {
                    IP_READ_WORD_IN(temp_word, pkt_ptr);  /* Name */
                    IP_READ_WORD_IN(type, pkt_ptr);       /* Type */
                    IP_READ_WORD_IN(temp_word, pkt_ptr);  /* Class */
                    IP_READ_LWORD_IN(ttl, pkt_ptr);       /* Time to Live */
                    IP_READ_WORD_IN(dataLength, pkt_ptr); /* Data length */
                    if(type == 0x0001)
                    {
                       IP_READ_LWORD_IN(ipAddress, pkt_ptr);
                    }
                    else
                    {
                        while(dataLength--)
                        {
                            IP_READ_BYTE_IN(temp_byte, pkt_ptr);
                        }
                        length -= 14 + dataLength;
                    }
/*                  if(answer > 1)
//                  {
//                      length = length - 21;
//                      UDP_dump(length);
//                  }
*/
                    answer--;
                }
            }
        }

        /* find the oldest entry in the table */
        dns_map_t *dnsPtr = dnsCache;
        for(index = DNS_MAP_SIZE; index !=0; index--)
        {
            if(entryPointer->birthSecond < dnsPtr->birthSecond)
            {
                entryPointer = dnsPtr;
            }
            /* Increment the pointer to get the next element from the array. */
            dnsPtr++;
        }

        for(index = 0; index < ARRAYSIZE(dnsCache); index++)
        {
            if(strcmp(entryPointer->dnsName, (char *)dnsR) == 0)   /* jira:M8TS-608 */
            {
                /* the entry_pointer is now pointing to the oldest entry
                 * replace the entry with the received data */
                entryPointer->birthSecond = time(0);
                entryPointer->ipAddress = ipAddress;
                entryPointer->ttl = ttl;
                entryPointer->isValid = true;
                break;
            }
            entryPointer ++;
        }
    }
}

void DNS_Update(void) // 
{
}

uint32_t DNS_Lookup(const char *dnsName)
{
    dns_map_t *entryPointer = dnsCache;
    uint32_t index;
    char str[80];
    uint32_t ret_val = 0U;
    
    logMsg("DNS Lookup Request", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
    
    for(index = 0; (index < DNS_MAP_SIZE) && (ret_val == 0U); index++)
    {
        if(entryPointer->isValid)
        {
            if(strcmp(entryPointer->dnsName, dnsName)==0)
            {
                sprintf(str,"DNS Found %s in cache %ul",dnsName, entryPointer->ipAddress);
                logMsg(str, LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                ret_val = entryPointer->ipAddress;
            }
        }
        entryPointer ++;
    }
    DNS_Query(dnsName);
    return 0;
}
