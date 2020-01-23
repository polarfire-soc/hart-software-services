 /*******************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions.  All rights reserved.
 *
 * PRCI definitions
 *
 *
 * SVN $Revision: 9986 $
 * SVN $Date: 2018-05-06 11:12:49 +0530 (Sun, 06 May 2018) $
 */

#ifndef MSS_PRCI_H_
#define MSS_PRCI_H_

/*******************************************************************************
 * (c) Copyright 2016-2018 Microsemi-PRO Embedded Systems Solutions.  All rights reserved.
 *
 * @file mss_clint.h
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief PSE PLIC and PRCI access data structures and functions.
 *
 * SVN $Revision: 9575 $
 * SVN $Date: 2017-11-14 14:23:11 +0530 (Tue, 14 Nov 2017) $
 */
#ifndef MSS_PRCI_H
#define MSS_PRCI_H

#include <stdint.h>

#include "encoding.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "encoding.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
 * PRCI: Power, Reset, Clock, Interrupt
 */
//#define PRCI_BASE   0x44000000UL
#define PRCI_BASE   0x10000000UL	/* FU540-C000 on unleashed board- 0x10000000UL	*/

#if 0
typedef struct
{
    volatile uint32_t MSIP[4095];
    volatile uint32_t reserved;
    volatile uint64_t MTIMECMP[4095];
    volatile const uint64_t MTIME;
} PRCI_Type;

#define PRCI    ((PRCI_Type *)PRCI_BASE)

/*==============================================================================
 * The function raise_soft_interrupt() raises a synchronous software interrupt by
 * writing into the MSIP register.
 */
static inline void raise_soft_interrupt()
{
    unsigned long hart_id = read_csr(mhartid);

    /*You need to make sure that the global interrupt is enabled*/
    set_csr(mie, MIP_MSIP);       /*Enable software interrupt bit */
    PRCI->MSIP[hart_id] = 0x01;   /*raise soft interrupt for hart0*/
}
#endif


#ifdef __cplusplus
}
#endif

#endif	/*	MSS_CLINT_H */


#endif /* MSS_PRCI_H_ */
