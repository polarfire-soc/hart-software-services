/**
  ICMPv4 Protocol Header file
	
  Company:
    Microchip Technology Inc.

  File Name:
    icmp.h

  Summary:
    This is the header file for icmp.c

  Description:
    This header file provides the API for the ICMP protocol

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

#ifndef ICMP_H
#define	ICMP_H

#define DEST_UNREACHABLE_LEN 64    //jira: CAE_MCU8-5706

/**
  Section: ICMP Functions
 */

/**Receives ICMP Packet
 *This function process only ICMP Ping Echo Requests.
 *
 * @param ipv4_hdr
 *      IPv4 Header of the received Packet
 *
 * @return
 *      (1) - SUCCESS
 * @return
 *      (!=1) - FAILURE
 */
error_msg ICMP_Receive(uint8_t *packet, uint8_t **pkt_read, ipv4Header_p_t *ipv4Hdr);


/**This function sends an Echo Reply Packet to the destination.
 *
 * @param ipv4_hdr
 *      IPv4 Header of the received Packet.
 *
 * @return
 */
error_msg ICMP_EchoReply(uint8_t *packet, uint8_t **pkt_read, ipv4Header_p_t *ipv4Hdr);
/**This function sends an port unreachable ICMP messages to the destination
 * 
 * @param srcIPAddress
 *      Source IP address
 * @param destIPAddress
 *     Destination IP address
 * @param length
 *      Length of the IP datagram
 * @return 
 */

error_msg ICMP_PortUnreachable(uint8_t *packet, uint8_t **pkt_read, uint32_t srcIPAddress,uint32_t destIPAddress, uint16_t length);

/**This function checks for the portUnreachable flag is set to the the Port number of the application
 * 
 * @param port
 *      Port Number of the application
 * @return 
 */

bool isPortUnreachable(uint16_t port);


/**Resets the portUnreachable flag to zero
 * 
 */
void resetPortUnreachable(void);
 
#endif	/* ICMP_H */
