/*******************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions.  All rights reserved.
 *
 * MPFS HAL include files
 *
 * SVN $Revision: 10510 $
 * SVN $Date: 2018-11-06 12:08:20 +0000 (Tue, 06 Nov 2018) $
 */

#ifndef MSS_HAL_H_
#define MSS_HAL_H_

#include "atomic.h"
#include "bits.h"
#include "encoding.h"
#include "mss_peripheral_base_add.h"
#include "mss_sysreg.h"
#include "mss_h2f.h"
#include "mss_clint.h"
#include "mss_hart_ints.h"
#include "mss_plic.h"
#include "mss_prci.h"
#include "mtrap.h"
#include "mss_coreplex.h"
#include "mss_util.h"

uint32_t SysTick_Config(void);

#endif /* MSS_HAL_H_ */
