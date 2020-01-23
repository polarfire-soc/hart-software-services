/*******************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions.  All rights reserved.
 *
 * PSE HAL include files
 *
 * SVN $Revision:  $
 * SVN $Date:  $
 */

#ifndef PSE_HAL_H_
#define PSE_HAL_H_

#include "atomic.h"
#include "bits.h"
#include "encoding.h"
#include "mpu.h"
#include "mss_address_map.h"
#include "mss_sysreg.h"
#include "mss_h2f.h"
#include "mss_clint.h"
#include "mss_hart_ints.h"
#include "mss_plic.h"
#include "mss_prci.h"
#include "mtrap.h"
#include "pse_coreplex.h"
#include "pse_util.h"

uint32_t SysTick_Config(void);

#endif /* PSE_HAL_H_ */
