/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/*******************************************************************************
 * @file mss_hal.h
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief MPFS HAL include file. This is the file intended for application to
 * include so that all the other MPFS files are then accessible to it.
 *
 * SVN $Revision: 11867 $
 * SVN $Date: 2019-07-29 23:56:04 +0530 (Mon, 29 Jul 2019) $
 */

#ifndef MSS_HAL_H
#define MSS_HAL_H

#include "atomic.h"
#include "bits.h"
#include "encoding.h"
#include "mss_clint.h"
#include "mss_coreplex.h"
#include "mss_h2f.h"
#include "mss_hart_ints.h"
#include "mss_ints.h"
#include "mss_mpu.h"
#include "mss_peripheral_base_add.h"
#include "mss_plic.h"
#include "mss_prci.h"
#include "mss_seg.h"
#include "mss_sysreg.h"
#include "mss_util.h"
#include "mtrap.h"
#include "system_startup.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t SysTick_Config(void);

#ifdef __cplusplus
}
#endif

#endif /* MSS_HAL_H */
