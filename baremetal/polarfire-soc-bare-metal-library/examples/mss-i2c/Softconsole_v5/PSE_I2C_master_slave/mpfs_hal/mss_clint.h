/*******************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions.  All rights reserved.
 *
 * @file mss_clint.h
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief CLINT access data structures and functions.
 *
 * SVN $Revision: 10608 $
 * SVN $Date: 2018-11-26 14:38:17 +0000 (Mon, 26 Nov 2018) $
 */
#ifndef MSS_CLINT_H
#define MSS_CLINT_H

#include "encoding.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

#define RTC_PRESCALER 100
#define SUCCESS 0
#define ERROR   1


/*==============================================================================
 * CLINT: Core Local Interrupter
 */
//#define CLINT_BASE   0x02000000UL		defined in encoding.h		/* FU540-C000 on unleashed board- 0x10000000UL	*/

typedef struct
{
    volatile uint32_t MSIP[5];
    volatile uint32_t reserved1[(0x4000 - 0x14)/4];
    volatile uint64_t MTIMECMP[5];	/* mtime compare value for each hart. When mtime equals this value, interrupt is generated for particular hart */
    volatile uint32_t reserved2[((0xbff8 - 0x4028)/4)];
    volatile uint64_t MTIME;		/* contains the current mtime value */
} CLINT_Type;

#define CLINT    ((CLINT_Type *)CLINT_BASE)

/*==============================================================================
 * The function raise_soft_interrupt() raises a synchronous software interrupt by
 * writing into the MSIP register.
 */
static inline void raise_soft_interrupt(unsigned long hart_id)
{
    /*You need to make sure that the global interrupt is enabled*/
	/*Note:  set_csr(mie, MIP_MSIP) needs to be set on hart you are setting sw interrupt */
    CLINT->MSIP[hart_id] = 0x01;   /*raise soft interrupt for hart(x) where x== hart ID*/
}

/*==============================================================================
 * The function clear_soft_interrupt() clears a synchronous software interrupt by
 * clearing the MSIP register.
 */
static inline void clear_soft_interrupt(void)
{
    unsigned long hart_id = read_csr(mhartid);
    CLINT->MSIP[hart_id] = 0x00U;   /*clear soft interrupt for hart0*/
}



#ifdef __cplusplus
}
#endif

#endif	/*	MSS_CLINT_H */
