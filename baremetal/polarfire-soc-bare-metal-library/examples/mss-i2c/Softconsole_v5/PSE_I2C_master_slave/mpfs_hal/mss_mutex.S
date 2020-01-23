/***************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * mss_mutex.S
 *
 * SVN $Revision: 10597 $
 * SVN $Date: 2018-11-23 15:48:29 +0000 (Fri, 23 Nov 2018) $
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

