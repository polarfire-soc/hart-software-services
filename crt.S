# See LICENSE.md for license details.

#include "encoding.h"

.section ".text"

  .globl _start
_start:
  # write mtvec and make sure it sticks
  la t0, trap_vector
  csrw mtvec, t0
  csrr t1, mtvec
1: bne t0, t1, 1b

  li  x1, 0
  li  x2, 0
  li  x3, 0
  li  x4, 0
  li  x5, 0
  li  x6, 0
  li  x7, 0
  li  x8, 0
  li  x9, 0
  li  x10,0
  li  x11,0
  li  x12,0
  li  x13,0
  li  x14,0
  li  x15,0
  li  x16,0
  li  x17,0
  li  x18,0
  li  x19,0
  li  x20,0
  li  x21,0
  li  x22,0
  li  x23,0
  li  x24,0
  li  x25,0
  li  x26,0
  li  x27,0
  li  x28,0
  li  x29,0
  li  x30,0
  li  x31,0
  csrw mscratch, x0

1:
  # initialize global pointer
  lla gp, _gp					// load address of _gp section into global pointer

  lla  tp, _end + 63				// load address of _end+63 into tp (thread pointer)
  addi tp, tp, 63

  and tp, tp, -64				// and with (-64) to align to 64-byte address

  # get hart id
  csrr a0, mhartid				// load mhartid into a0
  # for now, assume only 1 hart
  li a1, 1        				// load immediate 1 into a1...
#1:bgeu a0, a1, 1b 				// branch to 1b if ((unsigned)a0 >= (unsigned)a1)

  # give each hart 16KB of stack + TLS
#define STKSHIFT 14
  slli a2, a0, STKSHIFT				// multiply a0 (mhartid) by 16KB (1<<14) and store into a2
  add tp, tp, a2				// add a2 to the thread pointer
  add sp, a0, 1					// sp = a0 + 1 => mhartid + 1
  slli sp, sp, STKSHIFT				// multiply sp by 16KB
  add sp, sp, tp				// add sp tp thread pointer

  # set MSIE bit to receive IPIs
  li a2, MIP_MSIP
  csrw mie, a2

  beqz a0, .boot_e51

  # U54s will WFI until E51 has BSS memory intiialized
  # at which point E51 will send wakeup IPIs
  jal u54_banner
.spin_u54:
  wfi
  j .spin_u54

.boot_e51:
  j main;
