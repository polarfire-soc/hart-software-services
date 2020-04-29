/**
  ENC28J60.h
	
  Company:
    Microchip Technology Inc.

  File Name:
    ENC28J60.h

  Summary:
    Brief Description of the file (will placed in a table if using Doc-o-Matic)

  Description:
    This section is for a description of the file.  It should be in complete
    sentences describing the purpose of this file.

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


#ifndef __ENC28J60_H
#define __ENC28J60_H

#include "ethernet_driver.h"

#define SFR_BANK0  0x00
#define SFR_BANK1  0x40
#define SFR_BANK2  0x80
#define SFR_BANK3  0xC0
#define SFR_COMMON 0xE0
#define BANK_MASK  0xE0
#define SFR_MASK   0x1F



typedef enum
{
    rcr_inst = 0x00,
    rbm_inst = 0x3A,
    wcr_inst = 0x40,
    wbm_inst = 0x7A,
    bfs_inst = 0x80,
    bfc_inst = 0xa0,
    src_inst = 0xFF
}spi_inst_t;

typedef enum
{
	sfr_bank0 = SFR_BANK0,
	sfr_bank1 = SFR_BANK1,
	sfr_bank2 = SFR_BANK2,
	sfr_bank3 = SFR_BANK3,
	sfr_common = SFR_COMMON
} sfr_bank_t;

typedef enum {
	// bank 0
	J60_ERDPTL    = (SFR_BANK0 | 0x00),
	J60_ERDPTH    = (SFR_BANK0 | 0x01),
	J60_EWRPTL    = (SFR_BANK0 | 0x02),
	J60_EWRPTH    = (SFR_BANK0 | 0x03),
	J60_ETXSTL    = (SFR_BANK0 | 0x04),
	J60_ETXSTH    = (SFR_BANK0 | 0x05),
	J60_ETXNDL    = (SFR_BANK0 | 0x06),
	J60_ETXNDH    = (SFR_BANK0 | 0x07),
	J60_ERXSTL    = (SFR_BANK0 | 0x08),
	J60_ERXSTH    = (SFR_BANK0 | 0x09),
	J60_ERXNDL    = (SFR_BANK0 | 0x0A),
	J60_ERXNDH    = (SFR_BANK0 | 0x0B),
	J60_ERXRDPTL  = (SFR_BANK0 | 0x0C),
	J60_ERXRDPTH  = (SFR_BANK0 | 0x0D),
	J60_ERXWRPTL  = (SFR_BANK0 | 0x0E),
	J60_ERXWRPTH  = (SFR_BANK0 | 0x0F),
	J60_EDMASTL   = (SFR_BANK0 | 0x10),
	J60_EDMASTH   = (SFR_BANK0 | 0x11),
	J60_EDMANDL   = (SFR_BANK0 | 0x12),
	J60_EDMANDH   = (SFR_BANK0 | 0x13),
	J60_EDMADSTL  = (SFR_BANK0 | 0x14),
	J60_EDMADSTH  = (SFR_BANK0 | 0x15),
	J60_EDMACSL   = (SFR_BANK0 | 0x16),
	J60_EDMACSH   = (SFR_BANK0 | 0x17),
	RSRV_018  = (SFR_BANK0 | 0x18),
	RSRV_019  = (SFR_BANK0 | 0x19),
	RSRV_01A  = (SFR_BANK0 | 0x1A),
	// bank 1
	J60_EHT0      = (SFR_BANK1 | 0x00),
	J60_EHT1      = (SFR_BANK1 | 0x01),
	J60_EHT2      = (SFR_BANK1 | 0x02),
	J60_EHT3      = (SFR_BANK1 | 0x03),
	J60_EHT4      = (SFR_BANK1 | 0x04),
	J60_EHT5      = (SFR_BANK1 | 0x05),
	J60_EHT6      = (SFR_BANK1 | 0x06),
	J60_EHT7      = (SFR_BANK1 | 0x07),
	J60_EPMM0     = (SFR_BANK1 | 0x08),
	J60_EPMM1     = (SFR_BANK1 | 0x09),
	J60_EPMM2     = (SFR_BANK1 | 0x0A),
	J60_EPMM3     = (SFR_BANK1 | 0x0B),
	J60_EPMM4     = (SFR_BANK1 | 0x0C),
	J60_EPMM5     = (SFR_BANK1 | 0x0D),
	J60_EPMM6     = (SFR_BANK1 | 0x0E),
	J60_EPMM7     = (SFR_BANK1 | 0x0F),
	J60_EPMCSL    = (SFR_BANK1 | 0x10),
	J60_EPMCSH    = (SFR_BANK1 | 0x11),
	RSRV_112  = (SFR_BANK1 | 0x12),
	RSRV_113  = (SFR_BANK1 | 0x13),
	J60_EPMOL     = (SFR_BANK1 | 0x14),
	J60_EPMOH     = (SFR_BANK1 | 0x15),
	RSRV_116  = (SFR_BANK1 | 0x16),
	RSRV_117  = (SFR_BANK1 | 0x17),
	J60_ERXFCON   = (SFR_BANK1 | 0x18),
	J60_EPKTCNT   = (SFR_BANK1 | 0x19),
	RSRV_11A  = (SFR_BANK1 | 0x1A),
	// bank 2
	J60_MACON1    = (SFR_BANK2 | 0x00),
	RSRV_201  = (SFR_BANK2 | 0x01),
	J60_MACON3    = (SFR_BANK2 | 0x02),
	J60_MACON4    = (SFR_BANK2 | 0x03),
	J60_MABBIPG   = (SFR_BANK2 | 0x04),
	RSRV_205  = (SFR_BANK2 | 0x05),
	J60_MAIPGL    = (SFR_BANK2 | 0x06),
	J60_MAIPGH    = (SFR_BANK2 | 0x07),
	J60_MACLCON1  = (SFR_BANK2 | 0x08),
	J60_MACLCON2  = (SFR_BANK2 | 0x09),
	J60_MAMXFLL   = (SFR_BANK2 | 0x0A),
	J60_MAMXFLH   = (SFR_BANK2 | 0x0B),
	RSRV_20C  = (SFR_BANK2 | 0x0C),
	RSRV_20D  = (SFR_BANK2 | 0x0D),
	RSRV_20E  = (SFR_BANK2 | 0x0E),
	RSRV_20F  = (SFR_BANK2 | 0x0F),
	RSRV_210  = (SFR_BANK2 | 0x10),
	RSRV_211  = (SFR_BANK2 | 0x11),
	J60_MICMD     = (SFR_BANK2 | 0x12),
	RSRV_213  = (SFR_BANK2 | 0x13),
	J60_MIREGADR  = (SFR_BANK2 | 0x14),
	RSRV_215  = (SFR_BANK2 | 0x15),
	J60_MIWRL     = (SFR_BANK2 | 0x16),
	J60_MIWRH     = (SFR_BANK2 | 0x17),
	J60_MIRDL     = (SFR_BANK2 | 0x18),
	J60_MIRDH     = (SFR_BANK2 | 0x19),
	RSRV_21A  = (SFR_BANK2 | 0x1A),
	// bank 3
	J60_MAADR5   = (SFR_BANK3 | 0x00),
	J60_MAADR6   = (SFR_BANK3 | 0x01),
	J60_MAADR3   = (SFR_BANK3 | 0x02),
	J60_MAADR4   = (SFR_BANK3 | 0x03),
	J60_MAADR1   = (SFR_BANK3 | 0x04),
	J60_MAADR2   = (SFR_BANK3 | 0x05),
	J60_EBSTSD   = (SFR_BANK3 | 0x06),
	J60_EBSTCON  = (SFR_BANK3 | 0x07),
	J60_EBSTCSL  = (SFR_BANK3 | 0x08),
	J60_EBSTCSH  = (SFR_BANK3 | 0x09),
	J60_MISTAT   = (SFR_BANK3 | 0x0A),
	RSRV_30B = (SFR_BANK3 | 0x0B),
	RSRV_30C = (SFR_BANK3 | 0x0C),
	RSRV_30D = (SFR_BANK3 | 0x0D),
	RSRV_30E = (SFR_BANK3 | 0x0E),
	RSRV_30F = (SFR_BANK3 | 0x0F),
	RSRV_310 = (SFR_BANK3 | 0x10),
	RSRV_311 = (SFR_BANK3 | 0x11),
	J60_EREVID   = (SFR_BANK3 | 0x12),
	RSRV_313 = (SFR_BANK3 | 0x13),
	RSRV_314 = (SFR_BANK3 | 0x14),
	J60_ECOCON   = (SFR_BANK3 | 0x15),
	RSRV_316 = (SFR_BANK3 | 0x16),
	J60_EFLOCON  = (SFR_BANK3 | 0x17),
	J60_EPAUSL   = (SFR_BANK3 | 0x18),
	J60_EPAUSH   = (SFR_BANK3 | 0x19),
	RSRV_31A = (SFR_BANK3 | 0x1A),
	// the following 5 registers are in all banks
	J60_EIE       = (SFR_COMMON | 0x1B),
	J60_EIR       = (SFR_COMMON | 0x1C),
	J60_ESTAT     = (SFR_COMMON | 0x1D),
	J60_ECON2     = (SFR_COMMON | 0x1E),
	J60_ECON1     = (SFR_COMMON | 0x1F)
} enc28j60_registers_t;

typedef enum {
	J60_PHCON1  = 0x00,
	J60_PHSTAT1 = 0x01,
	J60_PHID1   = 0x02,
	J60_PHID2   = 0x03,
	J60_PHCON2  = 0x10,
	J60_PHSTAT2 = 0x11,
	J60_PHIE    = 0x12,
	J60_PHIR    = 0x13,
	J60_PHLCON  = 0x14 //display receive activity
} enc28j60_phy_registers_t;


// select register definitions
typedef union
{
    char val;
    struct
    {
        unsigned RXERIF:1;
        unsigned TXERIF:1;
        unsigned :1;
        unsigned TXIF:1;
        unsigned LINKIF:1;
        unsigned DMAIF:1;
        unsigned PKTIF:1;
        unsigned :1;
    };
} eir_t;


typedef union
{
    unsigned int val;
    struct
    {
        unsigned        :5;
        unsigned PLRITY :1;
        unsigned        :3;
        unsigned DPXSTAT:1;
        unsigned LSTAT  :1;
        unsigned COLSTAT:1;
        unsigned RXSTAT :1;
        unsigned TXSTAT :1;
        unsigned        :2;
        
    };
} phstat2_t;
uint8_t Control_Byte;


#endif // __ENC28J60_H