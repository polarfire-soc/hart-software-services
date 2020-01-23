/**
  DNSv4 Client Protocol header file
	
  Company:
    Microchip Technology Inc.

  File Name:
    dns_client.h

  Summary:
    This is the header file for dns_client.h

  Description:
    This header file provides the API for the DNS client protocol

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

#ifndef DNS_CLIENT_H
#define	DNS_CLIENT_H

#include <stdint.h> 

/**
  Section: DNS Client functions
 */

/*DNS Client Initialization.
 * This function will set up DNS table.
 * 
 * @param None
 * 
 * @param return
 *      Nothing
 * 
 */
void DNS_Init(void);


/*Query DNS Server
 * This fucntion queries the DNS Server for the IPv4(type A) addresses of the domain names.
 * 
 * @param str
 *      Domain name
 * 
 * @param return
 *      Nothing
 * 
 */
void DNS_Query(const char *str);


/*Handler DNS packets
 * This function receives and process the DNS packet.
 * The length of the packet is passed as paramater.
 * 
 * @param length
 *      Length of the received DNS packet.
 * 
 * @param return
 *      Nothing
 * 
 */
void DNS_Handler(uint8_t *packet, int16_t length);   // jira:M8TS-608


/*Maintain DNS Table
 * This function maintains the DNS table.
 * Maps Domain names to the Internet Address.
 * 
 * @param dns_name
 *      Domain name
 * 
 * @param return
 *      32-bit IPv4 address
 * 
 */
uint32_t DNS_Lookup(const char *dns_name);

/*Update DNS Table
 * This functions updates the DNS table atleast for every 10 seconds to avoid DNS table aging.
 * 
 * @param None
 *      Domain name
 * 
 * @param return
 *      32-bit IPv4 address
 * 
 */
void DNS_Update(void);



#endif	/* DNS_H */

