/**
 lfsr implementation

  Company:
    Microchip Technology Inc.

  File Name:
    lfsr.c

  Summary:
    lfsr implementation.

  Description:
    This file provides the lfsr implementation.

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

#include "lfsr.h"

#define lfsr_seed  0x40
#define lfsrOutputMask  0x7fu          //jira: CAE_MCU8-5647

static uint8_t sequenceIndex = 0;      //jira: CAE_MCU8-5647
const lfsr_t xorSequences[] = {0x41,0x44,0x47,0x48,0x4E,0x53,0x55,0x5C,0x5F,0x60,0x65,0x69,0x6A,0x72,0x77,0x78,0x7B,0x7E};
lfsr_t xor_mask = 0x41;  // note this comes from the first entry in the sequence list
lfsr_t lfsr_value = lfsr_seed;

lfsr_t lfsr_getSequence(void)
{
	return xorSequences[sequenceIndex];
}

int lfsr_getSequenceCount(void)
{
	return sizeof(xorSequences)/sizeof(*xorSequences);
}

void lfsr_reset(void)
{
	lfsr_value = lfsr_seed;
}

void lfsr_setSequence(uint16_t s)           //jira: CAE_MCU8-5647
{
	s %= sizeof(xorSequences)/sizeof(*xorSequences);
	xor_mask = xorSequences[s];
}

lfsr_t lfsr(void)
{
	unsigned char lsb = lfsr_value & 1u;     //jira: CAE_MCU8-5647
	lfsr_value >>= 1u;                       //jira: CAE_MCU8-5647
	if (lsb)
		lfsr_value ^= xor_mask;
	return lfsr_value & lfsrOutputMask;
}

lfsr_t lfsrWithSeed(uint8_t lfsrSeed)
{
    lfsr_value = lfsrSeed;  
	unsigned char lsb = lfsr_value & 1u;     //jira: CAE_MCU8-5647
	lfsr_value >>= 1;
	if (lsb)
		lfsr_value ^= xor_mask;
	return lfsr_value & lfsrOutputMask;
}
