/**
  DHCP v4 client implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    dhcp_client.c

  Summary:
     This is the implementation of DHCP client.

  Description:
    This source file provides the implementation of the DHCP client protocol.

 */

/*

©  [2015] Microchip Technology Inc. and its subsidiaries.  You may use this
software and any derivatives exclusively with Microchip products. 
  
THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER EXPRESS, 
IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES
OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE, OR
ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR
USE IN ANY APPLICATION. 

IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER
RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF
THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED
BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS
SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY
TO MICROCHIP FOR THIS SOFTWARE.

MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
TERMS. 

*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include "log.h"
#include "ethernet_driver.h"
#include "mac_address.h"
#include "network.h"
#include "udpv4.h"
#include "udpv4_port_handler_table.h"
#include "ipv4.h"
#include "arpv4.h"
#include "dhcp_client.h"
#include "ip_database.h"
#include "lfsr.h"

#ifdef ENABLE_NETWORK_DEBUG
#define logMsg(msg, msgSeverity, msgLogDest)    logMessage(msg, LOG_DHCP, msgSeverity, msgLogDest)
#else
#define logMsg(msg, msgSeverity, msgLogDest)
#endif
#define DHCP_HEADER_SIZE 240

/* PMCS: DHCP_PACKET_SIZE is not defined and DHCP_REQUEST_LENGTH is not used... */
#if ( DHCP_PACKET_SIZE & 1)
#define ZERO_PAD_DHCP
#define DHCP_REQUEST_LENGTH (DHCP_PACKET_SIZE + 1)
#else
#undef ZERO_PAD_DHCP
#define DHCP_REQUEST_LENGTH DHCP_PACKET_SIZE
#endif

const char dhcpName[] = DHCP_NAME;

static mac48Address_t ethMAC;

/**
  Section: Enumeration Definition
*/
typedef enum
{
    DHCP_DISCOVER = 1,
    DHCP_OFFER,
    DHCP_REQUEST,
    DHCP_DECLINE,
    DHCP_ACK,
    DHCP_NACK,
    DHCP_RELEASE,
    DHCP_INFORM,
    DHCP_FORCERENEW,
    DHCP_LEASEQUERY,
    DHCP_LEASEUNASSIGNED,
    DHCP_LEASEUNKNOWN,
    DHCP_LEASEACTIVE,
    DHCP_BULKLEASEQUERY,
    DHCP_LEASEQUERYDONE
}dhcp_type;

typedef enum
{
    SELECTING,
    REQUESTING,
    RENEWLEASE,
    BOUND
}dhcp_rx_client_state;

typedef enum
{
    INIT_TIMER,
    WAITFORTIMER,
    STARTDISCOVER,
    STARTREQUEST
}dhcp_timer_client_state;


typedef struct
{
    /* option data */
    uint32_t dhcpIPAddress;
    uint32_t subnetMask;
    uint32_t routerAddress;
    uint32_t dnsAddress[2]; /* only capture 2 DNS addresses */
    uint32_t ntpAddress[2];
    uint32_t gatewayAddress;
    uint32_t xidValue;
    uint32_t t1; /* name per the RFC page 35 */
    uint32_t t2; /* name per the specification page 35 */
    uint32_t dhcpServerIdentifier;
} dhcp_data_t;

typedef struct
{
    /* state data */
    dhcp_rx_client_state rxClientState;
    dhcp_timer_client_state tmrClientState;
} dhcp_state_t;

dhcp_data_t dhcpData = {0,0,0,{0,0},{0,0},0,0,10,3600,0};
dhcp_state_t dhcpState = {SELECTING,INIT_TIMER};


uint8_t dhcp_request[512]; /* PMCS: Todo need to look at size of buffer */

bool sendRequest(dhcp_type type);

bool sendDHCPDISCOVER(void);
bool sendDHCPDISCOVER(void)
{
    dhcpData.dhcpIPAddress = ipdb_getAddress();
    dhcpData.xidValue = lfsrWithSeed(ethMAC.mac_array[2]);
    dhcpData.xidValue <<= 8;
    dhcpData.xidValue |= lfsrWithSeed(ethMAC.mac_array[3]);
    dhcpData.xidValue <<= 8;
    dhcpData.xidValue |= lfsrWithSeed(ethMAC.mac_array[4]);
    dhcpData.xidValue <<= 8;
    dhcpData.xidValue |= lfsrWithSeed(ethMAC.mac_array[5]);
    return sendRequest(DHCP_DISCOVER);
}

bool sendDHCPREQUEST(void);
bool sendDHCPREQUEST(void)
{
    logMsg("requesting", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
    return sendRequest(DHCP_REQUEST);
}

bool sendDHCPDECLINE(void);
bool sendDHCPDECLINE(void)
{
    return sendRequest(DHCP_DECLINE);
}

bool sendRequest(dhcp_type type)
{
    /* creating a DHCP request */
    error_msg started;
    uint8_t *pkt_ptr = dhcp_request;

    started = UDP_Start(dhcp_request, &pkt_ptr, 0xFFFFFFFF, 68, 67);
    if(started == NET_SUCCESS)
    {
        IP_WRITE_LWORD_OUT(pkt_ptr, 0x01010600);        /* OP, HTYPE, HLEN, HOPS */
        IP_WRITE_LWORD_OUT(pkt_ptr, dhcpData.xidValue); /* XID : made up number... */
        IP_WRITE_LWORD_OUT(pkt_ptr, 0x00008000);        /* SECS, FLAGS (broadcast) */
        if(type == DHCP_REQUEST)
        {
            IP_WRITE_LWORD_OUT(pkt_ptr, 0);
        }
        else
        {
            IP_WRITE_LWORD_OUT(pkt_ptr, dhcpData.dhcpIPAddress);   /* CIADDR */
        }
        IP_WRITE_LWORD_OUT(pkt_ptr, 0);               /* YIADDR */
        IP_WRITE_LWORD_OUT(pkt_ptr, 0);               /* SIADDR */
        IP_WRITE_LWORD_OUT(pkt_ptr, 0);               /* GIADDR */
        IP_MOVE_BYTES_OUT(pkt_ptr, hostMacAddress,6); /* Hardware Address */
        IP_WRITE_BYTES_OUT(pkt_ptr, 0, 202);          /* 0 padding  + 192 bytes of BOOTP padding */
        IP_WRITE_LWORD_OUT(pkt_ptr, 0x63825363);      /* MAGIC COOKIE - Options to Follow */

        /* send the options */
        IP_WRITE_BYTE_OUT(pkt_ptr, 12);                         /* option 12 - DHCP Name */
        IP_WRITE_BYTE_OUT(pkt_ptr, strlen(dhcpName));
        IP_MOVE_BYTES_OUT(pkt_ptr, dhcpName, strlen(dhcpName));

        IP_WRITE_BYTE_OUT(pkt_ptr,  42);                        /* option 42 - NTP server name */
        IP_WRITE_BYTE_OUT(pkt_ptr,  4);
        IP_WRITE_LWORD_OUT(pkt_ptr, 0);

        IP_WRITE_BYTE_OUT(pkt_ptr, 53);                         /* option 53 - Request type */
        IP_WRITE_BYTE_OUT(pkt_ptr, 1);
        IP_WRITE_BYTE_OUT(pkt_ptr, type);
        if(type == DHCP_REQUEST)
        {
            /* option 50 - requested IP address... DO this if we have a valid one to re-request */
            IP_WRITE_BYTE_OUT(pkt_ptr, 50);
            IP_WRITE_BYTE_OUT(pkt_ptr, 4);
            IP_WRITE_LWORD_OUT(pkt_ptr, dhcpData.dhcpIPAddress);

            IP_WRITE_BYTE_OUT(pkt_ptr, 54); /* option 54 - Server identifier */
            IP_WRITE_BYTE_OUT(pkt_ptr, 4);
            IP_WRITE_LWORD_OUT(pkt_ptr, dhcpData.dhcpServerIdentifier); // JIRA: CAE_MCU8-5661
        }
        //
        IP_WRITE_BYTE_OUT(pkt_ptr, 55);                         /* option 55 - parameter request list */
        IP_WRITE_BYTE_OUT(pkt_ptr, 4);
        IP_WRITE_BYTE_OUT(pkt_ptr, 1);
        IP_WRITE_BYTE_OUT(pkt_ptr, 3);
        IP_WRITE_BYTE_OUT(pkt_ptr, 6);
        IP_WRITE_BYTE_OUT(pkt_ptr, 15);
        //
        IP_WRITE_BYTE_OUT(pkt_ptr, 57);                         /* option 57 - Maximum DHCP Packet */
        IP_WRITE_BYTE_OUT(pkt_ptr, 2);
        IP_WRITE_WORD_OUT(pkt_ptr, 512);
        //
        IP_WRITE_BYTE_OUT(pkt_ptr, 61);                         /* send option 61 (MAC address) */
        IP_WRITE_BYTE_OUT(pkt_ptr, 7);
        IP_WRITE_BYTE_OUT(pkt_ptr, 1);
        IP_MOVE_BYTES_OUT(pkt_ptr, hostMacAddress, 6);
        IP_WRITE_BYTE_OUT(pkt_ptr, 255); /* finished */
        
    #ifdef ZERO_PAD_DHCP
        UDP_Write8(0);   /* add a byte of padding to make the total length even */
    #endif
        UDP_Send(dhcp_request, (uint16_t)(pkt_ptr - dhcp_request));
        return true;
    }
    return false;
}

void DHCP_init(void)
{
    memcpy((uint8_t *) &ethMAC, g_tcp_mac->mac_addr, sizeof(g_tcp_mac->mac_addr));
}

void DHCP_Manage(void)
{
    static time_t managementTimer = 0;    
    time_t now;

    now = time(0);
    
    if(managementTimer <= now)
    {
        switch(dhcpState.tmrClientState)
        {
            case INIT_TIMER:
                dhcpData.t1 = 4;
                dhcpData.t2 = 2;
                dhcpState.tmrClientState = WAITFORTIMER;
                break;
            case WAITFORTIMER:
                if(!ethData.up)
                {
                    dhcpData.t1 = 2;
                    dhcpData.t2 = 4;
                }

                if(dhcpData.t1 == 2)
                {
                    ipdb_setAddress(0);              
                    dhcpState.tmrClientState = STARTREQUEST;
                }
                else
                {
                    dhcpData.t1 --;
                }

                if(dhcpData.t2 == 2)
                {
                    dhcpState.tmrClientState = STARTDISCOVER;
                }
                else
                {
                    dhcpData.t2 --;
                }
                break;
            case STARTDISCOVER:
                if(sendDHCPDISCOVER())
                {
                    dhcpData.t2 = 10; /* retry in 10 seconds */
                    dhcpData.t1 = LONG_MAX;
                    dhcpState.rxClientState = SELECTING;
                    dhcpState.tmrClientState = WAITFORTIMER;                    
                }
                break;
            case STARTREQUEST:
                if(ethData.up)
                {
                    if(sendDHCPREQUEST())
                    {
                        dhcpData.t1 = 15;
                        if(dhcpState.rxClientState == BOUND )
                        {
                            dhcpState.rxClientState = RENEWLEASE;
                        }
                        else
                        {
                            dhcpState.rxClientState = REQUESTING;
                        }

                        dhcpState.tmrClientState = WAITFORTIMER;
                    }
                }
                break;
            default:              
                logMsg("why am I at default", LOG_WARNING, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                dhcpState.tmrClientState = INIT_TIMER;
                break;
        }
    }
    managementTimer = now + 1;
}

void DHCP_Handler(uint8_t *packet, int16_t length)
{
    /* this function gets called by the UDP port handler for port 67 */
    uint8_t chaddr[16];
    uint8_t messageType=0;
    uint8_t temp_byte;
    uint16_t temp_word;
    uint32_t temp_lword;
    uint8_t *pkt_ptr = packet;
    uint32_t count;
    bool acceptOffers  = false;
    bool declineOffers = false;
    bool acceptNACK    = false;
    bool acceptACK     = false;

    uint32_t siaddr;


    logMsg("DHCP Handler", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
    
    switch(dhcpState.rxClientState)
    {
        default:
        case SELECTING: /* we can accept offers... drop the rest. */
            logMsg("DHCP SELECTING", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
            acceptOffers    = true;
            declineOffers   = false;
            acceptNACK      = false;
            acceptACK       = false;
            break;
        case REQUESTING: /* we can accept an ACK or NACK... decline offers and drop the rest */
            logMsg("DHCP REQUESTING", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
            acceptOffers    = false;
            declineOffers   = true; /* db */
            acceptNACK      = true;
            acceptACK       = true;
            break;
        case RENEWLEASE:
            logMsg("DHCP RENEW LEASE", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
            acceptOffers    = false;
            acceptNACK      = true;/* a bound lease renewal can have a Nack back */
            acceptACK       = true;/* a bound lease renewal can have an ack back */
            break;            
        case BOUND:     /* drop everything... the timer will pull us out of here. */
            logMsg("DHCP BOUND", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
            acceptOffers    = false;
            acceptNACK      = false;
            acceptACK       = false;
            break;
    }
    
    if(acceptOffers || declineOffers || acceptNACK || acceptACK)
    {
        dhcp_data_t localData = {0,0,0,{0,0},{0,0},0,0,3600,3600,0};
        localData.xidValue = dhcpData.xidValue;
        
        if(length > DHCP_HEADER_SIZE)
        {
            logMsg("DHCP PROCESSING", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
            IP_READ_WORD_IN(temp_word, pkt_ptr);
            if(0x0201 == temp_word)
            {
                IP_READ_BYTE_IN(temp_byte, pkt_ptr);
                if(0x06 == temp_byte) /* HLEN - is set to 6 because an Ethernet address is 6 Octets long */
                {

                    IP_READ_BYTE_IN(temp_byte, pkt_ptr);        /* HOPS */
                    IP_READ_LWORD_IN(temp_lword, pkt_ptr);
                    if(dhcpData.xidValue == temp_lword)         /* xid check */
                    {
                        IP_READ_WORD_IN(temp_word, pkt_ptr);    /* SECS */
                        IP_READ_WORD_IN(temp_word, pkt_ptr);    /* FLAGS */
                        IP_READ_LWORD_IN(temp_lword, pkt_ptr);   /* CIADDR */
                        IP_READ_LWORD_IN(localData.dhcpIPAddress, pkt_ptr); /* YIADDR */
                        if((localData.dhcpIPAddress != SPECIAL_IPV4_BROADCAST_ADDRESS) && (localData.dhcpIPAddress != LOCAL_HOST_ADDRESS))
                        {
                            IP_READ_LWORD_IN(siaddr, pkt_ptr); /* SIADDR */
                            if((siaddr != SPECIAL_IPV4_BROADCAST_ADDRESS) && (siaddr != LOCAL_HOST_ADDRESS))
                                {

                                IP_READ_LWORD_IN(temp_lword, pkt_ptr); /* GIADDR */
                                IP_MOVE_BYTES_IN(chaddr, pkt_ptr, sizeof(chaddr));  /* read chaddr */
                                if(memcmp(chaddr, &hostMacAddress.s, 6)== 0 || memcmp(chaddr, &broadcastMAC.s, 6)== 0 || (strlen((char *)chaddr)== 0)) // only compare 6 bytes of MAC address.
                                {
                                    pkt_ptr += 64; /* drop SNAME */
                                    pkt_ptr += 128; /* drop the filename */
                                    IP_READ_LWORD_IN(temp_lword, pkt_ptr);
                                    if(temp_lword == 0x63825363)
                                    {
                                        length -= (int16_t)DHCP_HEADER_SIZE;
                                        while(length>0)
                                        {
                                            /* options are here!!! */
                                            uint8_t option, optionLength;
                                            IP_READ_BYTE_IN(option, pkt_ptr);
                                            IP_READ_BYTE_IN(optionLength, pkt_ptr);
                                            length -= (int16_t)(2U + optionLength);
                                            switch(option)
                                            {
                                                case 1: /* subnet mask */
                                                    logMsg("DHCP option 1", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                                                    IP_READ_LWORD_IN(localData.subnetMask, pkt_ptr);
                                                    break;
                                                case 3: /* router */
                                                    logMsg("DHCP option 3", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                                                    IP_READ_LWORD_IN(localData.routerAddress, pkt_ptr);
                                                    break;
                                                case 6: // DNS List
                                                    count = 0;
                                                    logMsg("DHCP option 6", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                                                    while(optionLength >= 4)
                                                    {
                                                        IP_READ_LWORD_IN(temp_lword, pkt_ptr);
                                                        if(count < 2)
                                                            localData.dnsAddress[count++] = temp_lword;

                                                        optionLength -= (uint8_t)4U;
                                                    }
                                                    break;
                                                case 42: // NTP server
                                                    count = 0;
                                                    logMsg("DHCP option 42", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                                                    while(optionLength >= 4)
                                                    {
                                                        IP_READ_LWORD_IN(temp_lword, pkt_ptr);
                                                        if(count < 2)
                                                            localData.ntpAddress[count++] = temp_lword;

                                                        optionLength -= (uint8_t)4U;
                                                    }
                                                    break;
                                                case 51: // lease time
                                                    logMsg("DHCP option 51", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                                                    if(optionLength >= 4)
                                                    {
                                                        uint32_t origLeaseTime;

                                                        IP_READ_LWORD_IN(origLeaseTime, pkt_ptr);
            /*                                            localData.t2 = localData.t1 - 100; // 100 seconds faster for requests */
                                                        localData.t1 = origLeaseTime >> 1; /* be default set to 1/2 the lease length */
                                                        localData.t2 = localData.t1;       /* be default set to 1/2 the lease length */
                                                        origLeaseTime = localData.t1 >> 1;
                                                        localData.t2 += origLeaseTime;
                                                        localData.t2 += origLeaseTime >> 1;

                                                        optionLength -= (uint8_t)4U;
                                                    }
                                                    break;
                                                case 54: /* DHCP server */
                                                    logMsg("DHCP option 54", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                                                    IP_READ_LWORD_IN(localData.dhcpServerIdentifier, pkt_ptr);
            /*                                        printf("case54:%u\r\n",localData.dhcpServerIdentifier); */
                                                    optionLength -= (uint8_t)4U;
                                                    break;
                                                case 53:
                                                    logMsg("DHCP option 53", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                                                    IP_READ_BYTE_IN(messageType, pkt_ptr);
                                                    optionLength -= (uint8_t)1U;
                                                    break;
                                                default:
                                                    pkt_ptr += optionLength; /* dump any unused bytes */
                                                    break;
                                            } /* option switch */
                                        } /* length loop */
                                    } /* magic number test */
                                    else
                                    {
                                        logMsg("DHCP fail Magic Number check", LOG_WARNING, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                                    }
                                } /* mac address test */
                                else
                                {
                                    logMsg("DHCP fail MAC address check", LOG_WARNING, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
                                    return;
                                }
                            } /* SIADDR test */
                            else
                            {
                                logMsg("DHCP fail SIADDR check", LOG_WARNING, (LOG_DEST_CONSOLE));
                            }
                        } /* YIADDR test */
                        else
                        {
                            logMsg("DHCP fail YIADDR check", LOG_WARNING, (LOG_DEST_CONSOLE));
                        }
                    } /* xid test */
                    else
                    {                   
                        logMsg("DHCP fail XID check", LOG_WARNING, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));   
                    }
                } /* HLEN test */
                else
                {
                    logMsg("DHCP fail Hardware Length", LOG_WARNING, (LOG_DEST_CONSOLE));
                }
            } /* 201 test */
            else
            {
                logMsg("DHCP fail 201 check", LOG_WARNING, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
            }
        } /* short packet test */
        else
        {
            logMsg("DHCP fail short packet check", LOG_WARNING, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
        }

        switch(messageType)
        {
            case DHCP_OFFER:
                if(acceptOffers)
                {
                    dhcpData = localData; /* capture the data in the offer */
                    sendDHCPREQUEST();
                    dhcpState.rxClientState = REQUESTING;
                }

                if(declineOffers)
                {
                    sendDHCPDECLINE();
                }
                break;
            case DHCP_ACK:
                if(acceptACK)
                {
                    dhcpData.t1 = localData.t1;
                    dhcpData.t2 = localData.t2;
                    ipdb_setAddress(dhcpData.dhcpIPAddress);
                    ipdb_setDNS(0,dhcpData.dnsAddress[0]);
                    ipdb_setDNS(1,dhcpData.dnsAddress[1]);
                    ipdb_setRouter(dhcpData.routerAddress);
                    ipdb_setGateway(dhcpData.gatewayAddress);
                    ipdb_setSubNetMASK(dhcpData.subnetMask);
                    if(dhcpData.ntpAddress[0])
                    {
                        ipdb_setNTP(0,dhcpData.ntpAddress[0]);
                        if(dhcpData.ntpAddress[1])
                        {
                            ipdb_setNTP(1,dhcpData.ntpAddress[1]);
                        }
                    }
                    dhcpState.rxClientState = BOUND;
                }
                break;
            case DHCP_NACK:
                if(acceptNACK)
                {
                    dhcpData.t1 = 4;
                    dhcpData.t2 = 2;
                    dhcpState.rxClientState = SELECTING;
                }
                break;
            default:
                break;
        }
    } /* accept types test */
    else
    {
        logMsg("DHCP fail accept types  check", LOG_INFO, (LOG_DEST_CONSOLE|LOG_DEST_ETHERNET));
    }
}

#if 0
/* PMCS: Todo - not used? */
void DHCP_WriteZeros(uint16_t length)
{
    while(length--)
    {
        UDP_Write8(0);
    }
}
#endif
