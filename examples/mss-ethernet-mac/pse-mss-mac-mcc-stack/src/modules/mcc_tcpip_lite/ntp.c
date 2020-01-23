/**
 NTP client implementation
	
  Company:
    Microchip Technology Inc.

  File Name:
    ntp.c

  Summary:
    This is the implementation of NTP client.

  Description:
    This source file provides the implementation of the NTP client protocol.

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
#include <time.h>
#include "ntp.h"
#include "Ipv4.h"
#include "udpv4.h"
#include "rtcc.h"
#include "log.h"
#include "ip_database.h"

#define NTP_PACKET_SIZE 48

// the time offset converts from unix time (seconds since Jan 1 1970) to RFC 868 time (seconds since Jan 1 1900)
// the RFC indicates that this is sufficient until 2036
#define NTP_TIME_OFFSET 2208988800U

/* PMCS: Todo - check size of buffer required */
uint8_t ntp_packet[1500];


void NTP_Request(void)
{
    uint32_t ntpAddress;
    error_msg started;
    time_t myTime;
    uint8_t *pkt_ptr = ntp_packet;

    ntpAddress = ipdb_getNTP();
    myTime = time(0);
    myTime += NTP_TIME_OFFSET; // converting to RFC time
    
    //SYSLOG_log ("NTP Request", true, LOG_NTP, LOG_DEBUG);
    
    if(ntpAddress != 0)
    {
        started = UDP_Start(ntp_packet, &pkt_ptr, ntpAddress,123,123);
        if(started == NET_SUCCESS)
        {
            IP_WRITE_BYTE_OUT(pkt_ptr, 0xDB); //Leap Indicator - 11, Version number - 011 (3), Mode - 011 (3) client
            IP_WRITE_BYTE_OUT(pkt_ptr, 0x00); //Stratum level of local clock - 0-unspecified, 1-primary reference, 2 to n - secondary reference
            IP_WRITE_BYTE_OUT(pkt_ptr, 0x0A); //Poll - Max interval between succesive messages - 10 =  1024 secs
            IP_WRITE_BYTE_OUT(pkt_ptr, 0xEC); //Precision - Precision of local clock
            IP_WRITE_LWORD_OUT(pkt_ptr, 0x00003B68); //Synchronizing Distance - Roundtrip delay to primary synchronizing source
            IP_WRITE_LWORD_OUT(pkt_ptr, 0x00042673); //Estimated Drift Rate
            IP_WRITE_LWORD_OUT(pkt_ptr, ntpAddress);
            
            IP_WRITE_LWORD_OUT(pkt_ptr, (uint32_t)myTime); //Reference Timestamp in seconds
            IP_WRITE_LWORD_OUT(pkt_ptr, 0x0000);     // - Reference Timestamp Fraction assumed to 0
            IP_WRITE_LWORD_OUT(pkt_ptr, (uint32_t)myTime); //Origin Timestamp in seconds
            IP_WRITE_LWORD_OUT(pkt_ptr, 0x0000);     // - Origin Timestamp Fraction assumed to 0
            IP_WRITE_LWORD_OUT(pkt_ptr, (uint32_t)myTime); //Receive Timestamp in seconds
            IP_WRITE_LWORD_OUT(pkt_ptr, 0x0000);     // - Receive Timestamp Fraction assumed to 0
            IP_WRITE_LWORD_OUT(pkt_ptr, (uint32_t)myTime); //Transmit Timestamp in seconds
            IP_WRITE_LWORD_OUT(pkt_ptr, 0x0000);     // - Transmit Timestamp Fraction assumed to 0
			UDP_Send(ntp_packet, (uint16_t)(pkt_ptr - ntp_packet));
        }
    }
}

void NTP_Handler(uint8_t *packet, int16_t length)    // jira:M8TS-608
{
    uint32_t timeIntegralPart;
    uint32_t timeFractionalPart;
    uint8_t *pkt_ptr = packet;
    
    (void)packet;
    (void)length;

    pkt_ptr++; //Leap Indicator - 11, Version number - 011 (3), Mode - 011 (3) client
    pkt_ptr++; //Stratum level of local clock - 0-unspecified, 1-primary reference, 2 to n - secondary reference
    pkt_ptr++; //Poll - Max interval between succesive messages - 10 =  1024 secs
    pkt_ptr++; //Precision - Precision of local clock
    pkt_ptr += 4; //Synchronizing Distance - Round trip delay to primary synchronizing source
    pkt_ptr += 4; //Estimated Drift Rate
    pkt_ptr += 4;; // reference clock identifier
    
    // Reference Timestamp
    pkt_ptr += 4;
    pkt_ptr += 4;

    // Originate Timestamp
    pkt_ptr += 4;
    pkt_ptr += 4;

    // Receive Timestamp
    IP_READ_LWORD_IN(timeIntegralPart, pkt_ptr);
    IP_READ_LWORD_IN(timeFractionalPart, pkt_ptr);
    
    // Transmit timestamp
    pkt_ptr += 4;
    pkt_ptr += 4;
    
    timeIntegralPart -= NTP_TIME_OFFSET;
            
    rtcc_set((time_t *)timeIntegralPart);   // jira:M8TS-608
}

