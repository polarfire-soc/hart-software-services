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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ip_database.h"

ip_db_info_t ip_database_info;

void ipdb_init(void)
{
    ip_database_info.ipv4_myAddress = 0;
    ip_database_info.ipv4_subnetMask = 0;
    ip_database_info.ipv4_router = 0;
    for(uint8_t x=0; x < MAX_DNS; x ++)
        ip_database_info.ipv4_dns[x] = 0;
    for(uint8_t x=0; x < MAX_NTP; x++)
        ip_database_info.ipv4_ntpAddress[x] = 0;
    ip_database_info.ipv4_tftpAddress = 0;
}

uint32_t makeStrToIpv4Address(char *str)
{
    uint32_t ip_addr;
    char *pch;    
    uint8_t x =3;
    pch = strtok(str,".");
    while((pch != NULL))
    {        
       ((uint8_t *)&ip_addr)[x]= (uint8_t)atoi((const char*)pch); //jira: CAE_MCU8-5647
        x--;     
        pch = strtok (NULL,".");
    }
    return ip_addr;
}

char *makeIpv4AddresstoStr(uint32_t addr)
{
    static char ip_str[16];
    
    sprintf(ip_str,"%3d.%3d.%3d.%3d",((uint8_t *)&addr)[3],((uint8_t *)&addr)[2],((uint8_t *)&addr)[1],((uint8_t *)&addr)[0]);
    
    return ip_str;
}
