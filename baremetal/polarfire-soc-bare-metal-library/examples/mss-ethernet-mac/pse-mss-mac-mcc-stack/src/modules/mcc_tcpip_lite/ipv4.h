/**
  IPv4 Protocol header file
	
  Company:
    Microchip Technology Inc.

  File Name:
    ipv4.h

  Summary:
    This is the header file for ipv4.c

  Description:
    This header file provides the API for the IP protocol.

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


#ifndef IPV4_H
#define IPV4_H

/**
  Section: Included Files
*/
#include "tcpip_types.h"
#include "ethernet_driver.h"
#include <stdbool.h>

/**
  Section: Macro Declarations
*/

/*
 * Packet manipulation macros for dealing with copies that may not be aligned
 *
 * the _IN macros take data from the packet and the _OUT macros write data
 * to the packet. The packet pointer is assumed to be a pointer to uint8_t
 * and is adjusted based on the length of data.
 *
 * The move operations assume data is in correct order. The write and read
 *  operations follow the high byte first, low byte last order (except for the
 *  block read/write).
 *
 * All macros follow the memcpy(), destination, source approach.
 *
 */
#define IP_MOVE_BYTE_IN(x, y)      *((uint8_t *)&x) = *y; y++
#define IP_MOVE_BYTES_IN(x, y, z)  memcpy((void *)&x, (void *)y, z); y += z
#define IP_MOVE_WORD_IN(x, y)      memcpy((void *)&x, (void *)y, 2); y += 2
#define IP_MOVE_LWORD_IN(x, y)     memcpy((void *)&x, (void *)y, 4); y += 4

#define IP_MOVE_BYTE_OUT(x, y)      *x = *((uint8_t *)&y); x++
#define IP_MOVE_BYTES_OUT(x, y, z)  memcpy((void *)x, (void *)&y, z); x += z
#define IP_MOVE_STRING_OUT(x, y)    strcpy((void *)x, (void *)&y); x += strlen((void *)x)
#define IP_MOVE_WORD_OUT(x, y)      memcpy((void *)x, (void *)&y, 2); x += 2
#define IP_MOVE_LWORD_OUT(x, y)     memcpy((void *)x, (void *)&y, 4); x += 4

#define IP_WRITE_BYTE_OUT(x, y)      *x = (uint8_t)(y); x++
#define IP_WRITE_BYTES_OUT(x, y, z)   memset(x, y, z); x += z
#define IP_WRITE_WORD_OUT(x, y)      *x = (uint8_t)((y) >> 8); x++;*x = (uint8_t)(y); x++
#define IP_WRITE_TRIPLE_OUT(x, y)    *x = (uint8_t)((y) >> 16); x++;*x = (uint8_t)((y) >> 8); x++;*x = (uint8_t)(y); x++
#define IP_WRITE_LWORD_OUT(x, y)     *x = (uint8_t)((y) >> 24); x++;*x = (uint8_t)((y) >> 16); x++;*x = (uint8_t)((y) >> 8); x++;*x = (uint8_t)(y); x++

#define IP_READ_BYTE_IN(x, y)      x = *y; y++
#define IP_READ_WORD_IN(x, y)      x = (uint16_t)((uint16_t)(*y) << 8); y++; x = (uint16_t)(x + (uint16_t)(*y)); y++
#define IP_READ_TRIPLE_IN(x, y)    x = ((uint32_t)(*y)) << 16; y++; x += ((uint32_t)(*y)) << 8; y++; x += (uint32_t)(*y); y++
#define IP_READ_LWORD_IN(x, y)     x = ((uint32_t)(*y)) << 24; y++; x += ((uint32_t)(*y)) << 16; y++; x += ((uint32_t)(*y)) << 8; y++; x += (uint32_t)(*y); y++

/**
  Section: Data Types Definitions
*/

/**
  Section: DHCP Client Functions
 */

/**IPv4 Initialization.
 *
 */
void IPV4_Init(void);

/**Receives IPv4 Packet.
 * This function reads the IPv4 header and filters the upper layer protocols.
 *
 * @return
 */
error_msg IPV4_Packet(uint8_t *packet, uint8_t **pkt_read, uint32_t packet_length);

/**Starts the IPv4 Packet.
 * This function starts the Ethernet Packet and writes the IPv4 header.
 * Initially Checksum and Payload length are set to '0'
 *
 * @param dest_address
 *          32-bit Destination Ipv4 Address.
 *
 * @param protocol
 *          Protocol Number.
 *
 * @return
 *      An error code if there has been an error accepting.
 *      An error code if something goes wrong. For the possible errors please,
 *      see the error description in tcpip_types.h
 */
error_msg IPv4_Start(uint8_t *packet, uint8_t **pkt_write_ptr, uint32_t dstAddress, ipProtocolNumbers protocol);


/**This function computes the pseudo header checksum for transport layer protocols.
 *
 * @param payload_len
 *      Length of the transport layer packet
 *
 * @return
 *      16-bit checksum
 */
uint16_t IPV4_PseudoHeaderChecksum(uint16_t payloadLen);


/**Send IPv4 Packet.
 * This function inserts the toatl length of IPv4 packet, computes and inserts the Ipv4 header checksum.
 *
 * @param payload_length
 *      Data length of the transport packet.
 *
 * @return
 *      1 - IP Packet was sent Successfully
 * @return
 *      (!=1) - IP Packet sent Fails
 */
error_msg IPV4_Send(uint8_t *packet, uint16_t payloadLength);

/**This function returns the length of the IPv4 Datagram
 * 
 * @return 
 */

uint16_t IPV4_GetDatagramLength(void);


/**This function calculates the checksum for a block of data
 *
 * @return
 */
uint16_t IPV4_ComputeChecksum(uint8_t *packet, uint16_t len, uint16_t seed);

#endif
