/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */
/*******************************************************************************
 * 
 * SVN $Revision: 11865 $
 * SVN $Date: 2019-07-29 19:58:05 +0530 (Mon, 29 Jul 2019) $
 */
#ifndef CPU_TYPES_H
#define CPU_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*------------------------------------------------------------------------------
 * addr_t: address type.
 * Used to specify the address of peripherals present in the processor's memory
 * map.
 */
typedef unsigned int addr_t;

/*------------------------------------------------------------------------------
 * psr_t: processor state register.
 * Used by HAL_disable_interrupts() and HAL_restore_interrupts() to store the
 * processor's state between disabling and restoring interrupts.
 */
typedef unsigned int psr_t;

#ifdef __cplusplus
}
#endif

#endif  /* CPU_TYPES_H */

