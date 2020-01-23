/**
  TFTP Port Handler header file
	
  Company:
    Microchip Technology Inc.

  File Name:
    tftp_handler_table.h

  Summary:
    This is the header file tftp.c

  Description:
    This file consists of the TFTP Application call back table.

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


#ifndef TFTP_HANDLER_TABLE_H
#define	TFTP_HANDLER_TABLE_H

#include <stdint.h>


#define tableSize (sizeof(TFTP_callBackTable)/sizeof(*TFTP_callBackTable))

#define DEFAULT 0
#define EEPROM 1
#define EXTERNAL_STORAGE 2

#ifdef EEPROM_STORAGE
uint16_t store_type = 1;
#elif EXT_STORAGE
uint16_t store_type = 2;
#else
extern uint16_t store_type;
//Default storage type for TFTP handler is 0
#endif


typedef uint8_t (*TFTP_receive_function_ptr)(uint32_t , char *, uint16_t); 

typedef struct
{
    uint16_t storeNumber;
    TFTP_receive_function_ptr TFTP_CallBack;
} store_handler_t;

extern const store_handler_t TFTP_callBackTable[1];
#endif	/* TFTP_HANDLER_TABLE_H */

