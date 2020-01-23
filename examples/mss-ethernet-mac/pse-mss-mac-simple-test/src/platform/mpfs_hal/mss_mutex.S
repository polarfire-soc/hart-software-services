/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/***************************************************************************
 * @file mss_mutex.S
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief Mutex functions to help multiprocessor programming

 *
 * SVN $Revision: 11867 $
 * SVN $Date: 2019-07-29 19:26:04 +0100 (Mon, 29 Jul 2019) $
 */
  .text
  .align 3
  .globl mss_init_mutex
  .globl mss_take_mutex
  .globl mss_release_mutex

mss_init_mutex:
  sw x0,(a0)
  ret

mss_take_mutex:
  li t0, 1                      # Initialize swap value
take_loop:
  amoswap.w t1,t0,(a0)          # Attempt to acquire lock.
  bnez t1, take_loop            # Retry if held.
  ret

mss_release_mutex:
  amoswap.w x0,x0,(a0)          # Release lock by storing 0.
  ret

