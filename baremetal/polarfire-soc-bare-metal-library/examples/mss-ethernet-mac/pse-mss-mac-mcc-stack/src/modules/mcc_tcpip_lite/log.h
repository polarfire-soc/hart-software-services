/**
  Syslog API Header File
	
  Company:
    Microchip Technology Inc.

  File Name:
    log.h

  Summary:
    Header file for log.c

  Description:
    This header file provides APIs for sending log messages.

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

#ifndef __LOG_H
#define	__LOG_H

#include <stdint.h>
#include <stdbool.h>


#define LOG_DEST_ETHERNET  0x0001
#define LOG_DEST_CONSOLE   0x0002
#define LOG_DEST_EEPROM    0x0004

#define LOG_ETHERNET_MASK(m)     (m & LOG_DEST_ETHERNET) 
#define LOG_CONSOLE_MASK(m)      (m & LOG_DEST_CONSOLE)
#define LOG_EEPROM_MASK(m)       (m & LOG_DEST_EEPROM)          // TODO API for logging messages to EEPROM

#define SYSLOG_VERSION 1
#define LOG_NILVALUE "-"

/*PRI - Priorities Severity*/
typedef enum {  LOG_EMERGENCY	=0,	/* system is unusable */
                LOG_ALERTS      =1,	/* action must be taken immediately */
                LOG_CRITICAL	=2,	/* critical conditions */
                LOG_ERROR		=3,	/* error conditions */
                LOG_WARNING     =4,	/* warning conditions */
                LOG_NOTICE      =5,	/* normal but signification condition */
                LOG_INFO        =6,	/* informational messages*/
                LOG_DEBUG       =7, /* debug-level messages */
                LOG_DEFAULT     =8  /* Indicates to Log_Init to use default table */
} LOG_SEVERITY;

/*PRI - facility codes */
// these facility codes represent the Connected Lighting Stack
// Facility codes MUST be in the range of 0 to 23 inclusive
typedef enum {
	LOG_KERN,		/* kernel messages */                                /* 0 */
	LOG_DTLS,       /* DTLS messages */                                  /* 1 */
	LOG_ECC,        /* ECC system */                                     /* 2 */
	LOG_DAEMON,     /* system daemons */                                 /* 3 */
	LOG_AUTH,       /* security/authorization messages */                /* 4 */
	LOG_SYSLOG,     /* messages generated internally by syslogd */       /* 5 */
	LOG_LPR,        /* line printer subsystem */                         /* 6 */
	LOG_NEWS,       /* network news subsystem */                         /* 7 */
	LOG_TFTP,       /* TFTP Service */                                   /* 8 */
	LOG_UUID,       /* UUID Service */                                   /* 9 */
    LOG_COAP,       /* CoAP Service */                                   /* 10 */
	LOG_FTP,		/* ftp daemon */                                     /* 11 */
	LOG_NTP,		/* NTP subsystem */                                  /* 12 */
	LOG_HTTP,		/* HTTP Server*/                                     /* 13 */
	LOG_SNMP,		/* SNMP subsystem */                                 /* 14 */
	LOG_RTCC,       /* Real Time Clock Service */                        /* 15 */
    LOG_ICMP,       /* ICMP Service */                                   /* 16 */
	LOG_UDP,		/* UDP System */                                     /* 17 */
	LOG_TCP,		/* TCP system */                                     /* 18 */
	LOG_DHCP,		/* DHCP Service */                                   /* 19 */
	LOG_DNS,		/* DNS Service */                                    /* 20 */
	LOG_LLDP,		/* LLDP Service */                                   /* 21 */
    LOG_LINK,       /* URI Service  */                                   /* 22 */
            
    LOG_LAST        /* keep this one at the end of the list */           /* 23 */
} LOG_FACILITY;


typedef struct
{
    LOG_FACILITY logFacility;
    LOG_SEVERITY severityThreshold;
} logFields_t;


extern const char *LOG_Month[];

// Set the severity threshold for each facility
extern const logFields_t severityThresholdTable[];

/*Log Initialization.
 * The function initializes the service to send log messages.
 * 
 * @param return
 *      Nothing
 * 
 */
void LOG_Init(LOG_SEVERITY default_severity);


/*Log message generation.
 * The function will create a log message with specified fields.
 * 
 * @param message
 *      Message
 * 
 * @param facility
 *      Message facility
 * 
 * @param severity
 *      Message severity
 * 
 * @param logDest
 *      Message logging destination
 * 
 * @param return
 *      Nothing
 * 
 */
void logMessage(const char *message, LOG_FACILITY facility, LOG_SEVERITY severity, uint8_t logDest);


LOG_SEVERITY logGetPriority(LOG_FACILITY facility);
void logSetPriority(LOG_FACILITY facility, LOG_SEVERITY severity);


#endif	/* __LOG_H */


