/***************************************************************************//**
 * (c) Copyright 2007-2018 Microsemi SoC Products Group. All rights reserved.
 * 
 * Legacy interrupt control functions for the Microsemi driver library hardware
 * abstraction layer.
 *
 * SVN $Revision: 10718 $
 * SVN $Date: 2018-12-14 13:32:25 +0000 (Fri, 14 Dec 2018) $
 */
#include "hal.h"
#include <stdio.h>
#include "../mpfs_hal/atomic.h"
#include "../mpfs_hal/encoding.h"
#include "mss_util.h"
#include "../mpfs_hal/mcall.h"
#include "../mpfs_hal/mtrap.h"

/*------------------------------------------------------------------------------
 * 
 */
void HAL_enable_interrupts(void) {
    __enable_irq();
}

/*------------------------------------------------------------------------------
 * 
 */
psr_t HAL_disable_interrupts(void) {
    psr_t psr;
    psr = read_csr(mstatus);
    __disable_irq();
    return(psr);
}

/*------------------------------------------------------------------------------
 * 
 */
void HAL_restore_interrupts(psr_t saved_psr) {
    write_csr(mstatus, saved_psr);
}

