/**
  TCP/IP Stack User configuration options header file

  Company:
    Microchip Technology Inc.

  File Name:
    tcpip_config.h

  Summary:
    Header file for TCP/IP Stack User configuration options

  Description:
    This header file provides the TCP/IP Stack User configuration options.

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

#ifndef TCPIP_CONFIG_H
#define	TCPIP_CONFIG_H



/**
  Section: Included Files
*/

/**
  Section: Macro Declarations
*/

/* Build the IPv4 Address*/
#define MAKE_IPV4_ADDRESS(a,b,c,d) ((uint32_t)(((uint32_t)a << 24) | ((uint32_t)b<<16) | ((uint32_t)c << 8) | (uint32_t)d))


/******************************** ARP Protocol Defines *********************************/
#define ARP_MAP_SIZE 8


/******************************** DHCP Protocol Defines ********************************/
#define DHCP_NAME "MPFS TCPIP Demo";
// unfortunately, you must defined the macro below as sizeof does not work in the preprocessor
#define DHCP_NAME_SIZE 20

/******************************** TFTP Protocol Defines ********************************/
#define SOURCEPORT_TFTP     65531

/******************************** IP Protocol Defines ********************************/
#define IPv4_TTL            64u

/******************************** TCP Protocol Defines *********************************/
// Define the maximum segment size for the 
#define TCP_MAX_SEG_SIZE    1460u
#define TICK_SECOND 1

// TCP Timeout and retransmit numbers
#define TCP_START_TIMEOUT_VAL           ((unsigned long)TICK_SECOND*2)	// Timeout to retransmit unacked data

#define TCP_MAX_RETRIES                 (5u)                // Maximum number of retransmission attempts
#define TCP_MAX_SYN_RETRIES             (3u)                // Smaller than all other retries to reduce SYN flood DoS duration

#define LOCAL_TCP_PORT_START_NUMBER     (1024u)             // define the lower port number to be used as a local port
#define LOCAL_TCP_PORT_END_NUMBER       (65535u)            // define the highest port number to be used as a local port

/************************ Neighbor Discovery Protocol Defines **************************/

/******************************** TCP/IP stack debug Defines *********************************/

//#define ENABLE_TCP_DEBUG
//#define ENABLE_IP_DEBUG
//#define ENABLE_NET_DEBUG

#endif	/* TCPIP_CONFIG_H */
