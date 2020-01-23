/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
*/

/***********************************************************************************
 * @file mcall.h
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief mcall definitions

 * SVN $Revision: 11865 $
 * SVN $Date: 2019-07-29 15:28:05 +0100 (Mon, 29 Jul 2019) $
 */
#ifndef RISCV_SBI_H
#define RISCV_SBI_H

#ifdef __cplusplus
extern "C" {
#endif

#define SBI_SET_TIMER              0
#define SBI_CLEAR_IPI              3
#define SBI_SEND_IPI               4
#define SBI_REMOTE_FENCE_I         5
#define SBI_REMOTE_SFENCE_VMA      6
#define SBI_REMOTE_SFENCE_VMA_ASID 7

#ifdef __cplusplus
}
#endif

#endif //RISCV_SBI_H

