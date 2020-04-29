/**
 Sending log messages to console
	
  Company:
    Microchip Technology Inc.

  File Name:
    log_console.c

  Summary:
    Implementation for logging log messages to console

  Description:
    This source file provides the implementation of the APIs for message logging to the console.

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

/**
 Section: Included Files
 */


#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include "log.h"
#include "ip_database.h"


#include "mpfs_hal/mss_hal.h"
#if PSE
#include "drivers/mss_uart/mss_uart.h"
#else
#include "drivers/FU540_uart/FU540_uart.h"
#endif

#if defined(TARGET_ALOE)
#define PRINT_STRING(x)MSS_FU540_UART_polled_tx(&g_mss_FU540_uart0, x, strlen(x));
#else
#define PRINT_STRING(x) MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t *)x);
#endif

uint8_t logConsole(const char *message, uint16_t priorityVal)
{
    char info_string[256];
    struct tm * SYSLOG_time1;   
    time_t time1;   
    unsigned long ip;
    
    ip = ipdb_getAddress();   
    time1 = time(NULL);
    SYSLOG_time1 = gmtime(&time1); 
        
    sprintf(info_string,"<%d>%d %d-%.2d-%.2dT%.2d:%.2d:%.2dZ %d.%d.%d.%d %s %s %s %s[%s]\r\n", priorityVal, SYSLOG_VERSION, SYSLOG_time1->tm_year, SYSLOG_time1->tm_mon, SYSLOG_time1->tm_mday, SYSLOG_time1->tm_hour, SYSLOG_time1->tm_min, SYSLOG_time1->tm_sec, ((char*)&ip)[3],((char*)&ip)[2],((char*)&ip)[1],((char*)&ip)[0], LOG_NILVALUE, LOG_NILVALUE, LOG_NILVALUE, LOG_NILVALUE, message);
    PRINT_STRING(info_string);

    return 1;
}
