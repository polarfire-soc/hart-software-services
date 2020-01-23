/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

#ifndef __IP_DATABASE_H
#define	__IP_DATABASE_H

#define MAX_DNS 2
#define MAX_NTP 2

#define IPV4_ZERO_ADDRESS              0
#define SPECIAL_IPV4_BROADCAST_ADDRESS 0xFFFFFFFF
#define CLASS_A_IPV4_BROADCAST_MASK    0XFF000000
#define CLASS_B_IPV4_BROADCAST_MASK    0xFFFF0000
#define CLASS_C_IPV4_BROADCAST_MASK    0xFFFFFF00
#define ALL_HOST_MULTICAST_ADDRESS     0xE0000001
#define LOCAL_HOST_ADDRESS             0x7F000001

#define CLASS_A_IPV4_REVERSE_BROADCAST_MASK    0x00FFFFFF
#define CLASS_B_IPV4_REVERSE_BROADCAST_MASK    0x0000FFFF
#define CLASS_C_IPV4_REVERSE_BROADCAST_MASK    0X000000FF

 typedef struct  {
    uint32_t ipv4_myAddress;
    uint32_t ipv4_dns[MAX_DNS]; // allow a primary & secondary DNS
    uint32_t ipv4_subnetMask;
    uint32_t ipv4_router;
    uint32_t ipv4_gateway;
    uint32_t ipv4_ntpAddress[MAX_NTP];
    uint32_t ipv4_tftpAddress;
} ip_db_info_t;


extern ip_db_info_t ip_database_info;

#define ipdb_getAddress()		(ip_database_info.ipv4_myAddress)
#define ipdb_getDNS()  			(ip_database_info.ipv4_dns[0]) // decide how to get a primary or secondary DNS
#define ipdb_getSubNetMASK()	(ip_database_info.ipv4_subnetMask)
#define ipdb_getRouter()		(ip_database_info.ipv4_router)
#define ipdb_getNTP()			(ip_database_info.ipv4_ntpAddress[0])
#define ipdb_getTFTP() 			(ip_database_info.ipv4_tftpAddress)
#define ipdb_classAbroadcastAddress()  (ip_database_info.ipv4_myAddress|CLASS_A_IPV4_REVERSE_BROADCAST_MASK)
#define ipdb_classBbroadcastAddress()  (ip_database_info.ipv4_myAddress|CLASS_B_IPV4_REVERSE_BROADCAST_MASK)
#define ipdb_classCbroadcastAddress()  (ip_database_info.ipv4_myAddress|CLASS_C_IPV4_REVERSE_BROADCAST_MASK)
#define ipdb_specialbroadcastAddress() SPECIAL_IPV4_BROADCAST_ADDRESS

#define ipdb_setAddress(a)		do{ ip_database_info.ipv4_myAddress = a; } while(0)
#define ipdb_setDNS(x,v)		do{ if(x < MAX_DNS) ip_database_info.ipv4_dns[x] = v; } while(0)
#define ipdb_setSubNetMASK(m)	do{ ip_database_info.ipv4_subnetMask = m; } while(0)
#define ipdb_setRouter(r) 		do{ ip_database_info.ipv4_router = r; } while(0)
#define ipdb_setGateway(g) 		do{ ip_database_info.ipv4_gateway = g; } while(0)
#define ipdb_setNTP(x,n) 		do{ if(x < MAX_NTP) ip_database_info.ipv4_ntpAddress[x] = n; } while(0)
#define ipdb_setTFTP(a) 		do{ ip_database_info.ipv4_tftpAddress = a; } while(0)

void ipdb_init(void);
uint32_t makeStrToIpv4Address(char *str);
char *makeIpv4AddresstoStr(uint32_t addr);

#endif	/* __IP_DATABASE_H */

