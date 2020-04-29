/**
  Network header file
	
  Company:
    Microchip Technology Inc.

  File Name:
    network.h

  Summary:
    Header file for network.c.

  Description:
    This header file provides the API for the network helper.

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

#ifndef NETWORK_H
#define	NETWORK_H

#include <stdint.h>
#include "tcpip_types.h"

#include "mpfs_hal/mss_hal.h"
#include "mpfs_hal/mss_plic.h"
#include "drivers/mss_mac/mss_ethernet_registers.h"
#include "drivers/mss_mac/mss_ethernet_mac_user_config.h"
#include "drivers/mss_mac/mss_ethernet_mac_regs.h"
#include "drivers/mss_mac/mss_ethernet_mac.h"
#include "drivers/mss_mac/phy.h"

#define ENABLE_NETWORK_DEBUG


#define byteSwap16(a) ((uint16_t)((((uint16_t)a & (uint16_t)0xFF00U) >> 8) | (((uint16_t)a & (uint16_t)0x00FFU) << 8)))
#define byteReverse32(a) ((((uint32_t)a&(uint32_t)0xff000000) >> 24) | \
                          (((uint32_t)a&(uint32_t)0x00ff0000) >>  8) | \
                          (((uint32_t)a&(uint32_t)0x0000ff00) <<  8) | \
                          (((uint32_t)a&(uint32_t)0x000000ff) << 24) )

#define byteReverse24(a) (((((uint32_t)a & (uint32_t)0x00FF00) >> 8) | (((uint32_t)a & (uint32_t)0x0000FF) << 8)) << 8 | (uint32_t)a >> 0x10)

// host to network & network to host macros
#ifndef htons
#define htons(a) byteSwap16(a)
#endif
#ifndef ntohs
#define ntohs(a) byteSwap16(a)
#endif
#ifndef htonl
#define htonl(a) byteReverse32(a)
#endif
#ifndef ntohl
#define ntohl(a) byteReverse32(a)
#endif

#define convert_hton24(a)  byteReverse24(a)

extern mss_mac_instance_t *g_tcp_mac;

/*Network Initializer.
 * The function will perform initialization of the network protocols.
 * 
 * @param None
 * 
 * @param return
 *      Nothing
 * 
 */
void Network_Init(void);


/*Reading Packets.
 * The function will read the packets in the network.
 * 
 * @param None
 * 
 * @param return
 *      Nothing
 * 
 */
void Network_Read(void);


/*Managing Packets.
 * The function will handle the packets in the network .
 * 
 * @param None
 * 
 * @param return
 *      Nothing
 * 
 */
void Network_Manage(void);


/*Wait for limk.
 * The function will wait for the link by reading PHY registers.
 * 
 * @param None
 * 
 * @param return
 *      Nothing
 * 
 */
void Network_WaitForLink(void);
uint16_t Network_GetStartPosition(void);

void timersInit(void);



#endif	/* NETWORK_H */

