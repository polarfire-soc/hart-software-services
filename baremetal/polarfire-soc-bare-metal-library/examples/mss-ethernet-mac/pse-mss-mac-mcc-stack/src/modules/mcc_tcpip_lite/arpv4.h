/**
  ARPv4 Protocol Header file

  Company:
    Microchip Technology Inc.

  File Name:
    arpv4.h

  Summary:
    Header file for arpv4 protocol implementation.

  Description:
    This header file provides the API for the ARPv4 protocol.

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

#ifndef TCPIP_ARPV4_H
#define TCPIP_ARPV4_H

/**
  Section: Included Files
*/
#include "ethernet_driver.h"
#include "tcpip_config.h"

/* ARP database entry */
typedef struct
{
    mac48Address_t macAddress;
    uint32_t ipAddress;
    uint16_t protocolType;
    uint8_t age;            // replace oldest entry with new data as required.
} arpMap_t;

extern arpMap_t arpMap[];

/**
  Section: ARP functions
 */

/**ARP Initialization.
 * This function will set up ARP table.
 *
 */
void ARPV4_Init(void);

/**ARP Packet received.
 * This fucntion receives all ARP packets on the network.
 * Searches ARP table for existing ARP entry, if present updates table.
 * If ARP packet is for the device and its operation is REQUEST, replies with device MAC address.
 *
 * @return
 *      (1) - ARP REPLY sent successfully
 * @return
 *      (!=1) - ARP REPLY sent fails or ARP packet not destined to the device
 */
error_msg ARPV4_Packet(uint8_t *packet, uint8_t **pkt_read, uint32_t pkt_length);


/**This functions updates the ARP table atleast for every 10 seconds to avoid ARP table aging.
 *
 */
void ARPV4_Update(void);


/**This function maintains an ARP table.
 * Maps Hardware Address to Internet Address.
 *
 * @param
 *      32-bit Destination IPv4 address in Host Order.
 * @return
 *      Pointer to the Destination MAC address.
 */
mac48Address_t* ARPV4_Lookup(uint32_t ipAddress);


/**Sends ARP Request.
 *
 * @param destAddress
 *      32-bit Destination IPv4 address.
 * @return
 *      (1) - ARP REQUEST sent successfully.
 * @return
 *      (!=1) - ARP REQUEST sent fails.
 *
 */
error_msg ARPV4_Request(uint32_t destAddress);

/**
 * Adds a new entry or updates an existing entry in the table.
 * May cause ARP table entries to be dropped.
 */

void ARPV4_Add(uint32_t tpa, uint32_t spa, uint16_t ptype, mac48Address_t *mac_address);

#endif // TCPIP_ARPV4_H
