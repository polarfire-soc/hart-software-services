/*
    Copyright (c) 2013, The Regents of the University of California (Regents).
    All Rights Reserved.
    
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the Regents nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
    
    IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
    SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
    OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
    BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    
    REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
    HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
    MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

/***********************************************************************************
 *
 * SVN $Revision: 11073 $
 * SVN $Date: 2019-03-08 16:27:59 +0000 (Fri, 08 Mar 2019) $
 */

#include "bits.h"
#include "encoding.h"
#include "../mpfs_config/user_config/mss_user_config.h"
#include "mtrap.h"

#define BAD_TRAP_VECTOR                (0)
#define TRAP_FROM_MACHINE_MODE_VECTOR (13)

  .data
  .align 6
trap_table:
  .word bad_trap
  .word pmp_trap
  .word illegal_insn_trap
  .word bad_trap
  .word misaligned_load_trap
  .word pmp_trap
  .word misaligned_store_trap
  .word pmp_trap
  .word bad_trap
  .word mcall_trap
  .word bad_trap
  .word bad_trap
  .word bad_trap
  .word __trap_from_machine_mode
  .word bad_trap
  .word bad_trap

  .option norvc
  .section .text.init,"ax",@progbits
  .globl reset_vector
  .globl _start
  .globl switch_to_u_mode
  .globl switch_to_s_mode
  .globl init_s_mode
  reset_vector:
_start:
  j do_reset

trap_vector:
#The mscratch register is an XLEN-bit read/write register dedicated for use by machine mode.
#Typically, it is used to hold a pointer to a machine-mode hart-local context space and swapped
#with a user register upon entry to an M-mode trap handler.
  csrrw sp, mscratch, sp                    #copy sp to mscratch, and mscrath to sp
  beqz sp, .Ltrap_from_machine_mode         #if zero, no HLS, jump

  STORE a0, 10*REGBYTES(sp)                 # we are now using HLS
  STORE a1, 11*REGBYTES(sp)                 # save a0, a1

  csrr a1, mcause                           #save cause in a1
  bgez a1, .Lhandle_trap_in_machine_mode

  # This is an interrupt.  Discard the mcause MSB and decode the rest.
  sll a1, a1, 1

  # Is it a machine timer interrupt?
  li a0, IRQ_M_TIMER * 2
  bne a0, a1, 1f

  # Yes. Simply clear MSIE and raise SSIP
  li a0, MIP_MTIP
  csrc mie, a0
  li a0, MIP_STIP
  csrs mip, a0

.Lmret:
  # Go back to whence we came.
  LOAD a0, 10*REGBYTES(sp)
  LOAD a1, 11*REGBYTES(sp)
  csrrw sp, mscratch, sp
  mret

1:
  # Is it an IPI?
  li a0, IRQ_M_SOFT * 2
  bne a0, a1, .Lbad_trap

  # Yes. First clear the MIPI bit.
  LOAD a0, MENTRY_IPI_OFFSET(sp)
  sw x0, (a0)
  fence

  # Now, decode the cause(s).
#ifdef __riscv_atomic
  addi a0, sp, MENTRY_IPI_PENDING_OFFSET
  amoswap.w a0, x0, (a0)
#else
# error "only supporting atomic right now"
#endif
  and a1, a0, IPI_SOFT
  beqz a1, 1f
  csrs mip, MIP_SSIP
1:
  andi a1, a0, IPI_FENCE_I
  beqz a1, 1f
  fence.i
1:
  andi a1, a0, IPI_SFENCE_VMA
  beqz a1, 1f
  # sfence.vma - TODO
1:
  j .Lmret

.Lhandle_trap_in_machine_mode:
  # Preserve the registers.  Compute the address of the trap handler.
  STORE ra, 1*REGBYTES(sp)
  STORE gp, 3*REGBYTES(sp)
  STORE tp, 4*REGBYTES(sp)
  STORE t0, 5*REGBYTES(sp)
1:auipc t0, %pcrel_hi(trap_table)  # t0 <- %hi(trap_table)
  STORE t1, 6*REGBYTES(sp)
  sll t1, a1, 2                    # t1 <- mcause << 2
  STORE t2, 7*REGBYTES(sp)
  add t1, t0, t1                   # t1 <- %hi(trap_table)[mcause]
  STORE s0, 8*REGBYTES(sp)
  LWU t1, %pcrel_lo(1b)(t1)         # t1 <- trap_table[mcause]
  STORE s1, 9*REGBYTES(sp)
  mv a0, sp                        # a0 <- regs
  STORE a2,12*REGBYTES(sp)
  csrr a2, mepc                    # a2 <- mepc
  STORE a3,13*REGBYTES(sp)
  csrrw t0, mscratch, x0           # t0 <- user sp
  STORE a4,14*REGBYTES(sp)
  STORE a5,15*REGBYTES(sp)
  STORE a6,16*REGBYTES(sp)
  STORE a7,17*REGBYTES(sp)
  STORE s2,18*REGBYTES(sp)
  STORE s3,19*REGBYTES(sp)
  STORE s4,20*REGBYTES(sp)
  STORE s5,21*REGBYTES(sp)
  STORE s6,22*REGBYTES(sp)
  STORE s7,23*REGBYTES(sp)
  STORE s8,24*REGBYTES(sp)
  STORE s9,25*REGBYTES(sp)
  STORE s10,26*REGBYTES(sp)
  STORE s11,27*REGBYTES(sp)
  STORE t3,28*REGBYTES(sp)
  STORE t4,29*REGBYTES(sp)
  STORE t5,30*REGBYTES(sp)
  STORE t6,31*REGBYTES(sp)
  STORE t0, 2*REGBYTES(sp)         # sp

#ifndef __riscv_flen
  # commented out line below-  Not being used- cause issue with FreeRTOS
  #lw tp, (sp) # Move the emulated FCSR from x0's save slot into tp.
#endif
  STORE x0, (sp) # Zero x0's save slot.

  # Invoke the handler.
  jalr t1

#ifndef __riscv_flen
  # commented out line below-  Not being used- cause issue with FreeRTOS
  #sw tp, (sp) # Move the emulated FCSR from tp into x0's save slot.
#endif

restore_mscratch:
  # Restore mscratch, so future traps will know they didn't come from M-mode.
  csrw mscratch, sp

restore_regs:
  # Restore all of the registers.
  LOAD ra, 1*REGBYTES(sp)
  LOAD gp, 3*REGBYTES(sp)
  LOAD tp, 4*REGBYTES(sp)
  LOAD t0, 5*REGBYTES(sp)
  LOAD t1, 6*REGBYTES(sp)
  LOAD t2, 7*REGBYTES(sp)
  LOAD s0, 8*REGBYTES(sp)
  LOAD s1, 9*REGBYTES(sp)
  LOAD a0,10*REGBYTES(sp)
  LOAD a1,11*REGBYTES(sp)
  LOAD a2,12*REGBYTES(sp)
  LOAD a3,13*REGBYTES(sp)
  LOAD a4,14*REGBYTES(sp)
  LOAD a5,15*REGBYTES(sp)
  LOAD a6,16*REGBYTES(sp)
  LOAD a7,17*REGBYTES(sp)
  LOAD s2,18*REGBYTES(sp)
  LOAD s3,19*REGBYTES(sp)
  LOAD s4,20*REGBYTES(sp)
  LOAD s5,21*REGBYTES(sp)
  LOAD s6,22*REGBYTES(sp)
  LOAD s7,23*REGBYTES(sp)
  LOAD s8,24*REGBYTES(sp)
  LOAD s9,25*REGBYTES(sp)
  LOAD s10,26*REGBYTES(sp)
  LOAD s11,27*REGBYTES(sp)
  LOAD t3,28*REGBYTES(sp)
  LOAD t4,29*REGBYTES(sp)
  LOAD t5,30*REGBYTES(sp)
  LOAD t6,31*REGBYTES(sp)
  LOAD sp, 2*REGBYTES(sp)
  mret

.Ltrap_from_machine_mode:
  csrr sp, mscratch                      # copy the mscratch to sp. mscratch can the the actual sp memory pointer or HLS at this point, depends on if HLS non-zero or not
  addi sp, sp, -INTEGER_CONTEXT_SIZE     # moves sp down stack to make INTEGER_CONTEXT_SIZE area
  STORE a0, 10*REGBYTES(sp)              # save a0,a1 in the created CONTEXT
  STORE a1, 11*REGBYTES(sp)
  li a1, TRAP_FROM_MACHINE_MODE_VECTOR   # not sure where we use this
  j .Lhandle_trap_in_machine_mode

.Lbad_trap:
  li a1, BAD_TRAP_VECTOR
  j .Lhandle_trap_in_machine_mode

  .globl __redirect_trap
__redirect_trap:
  # Reset sp to top of M-Mode stack
  li t0, MACHINE_STACK_SIZE
  add sp, sp, t0
  neg t0, t0
  and sp, sp, t0
  addi sp, sp, -MENTRY_FRAME_SIZE
  j restore_mscratch

__trap_from_machine_mode:
  jal trap_from_machine_mode
  j restore_regs

do_reset:
  la t0, trap_vector
  csrw mtvec, t0                    # initalise machine trap vector address

  #clear the pending register, this is init with random value
  li a0, 0
  csrw mip, a0
  # mscratch must be init to zero- On the unleashed issue occurs in trap handling code if not init.
  csrw mscratch, a0
  csrw mcause, a0
  csrw mepc, a0

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

  # enable FPU and accelerator if present
  li t0, MSTATUS_FS | MSTATUS_XS
  csrs mstatus, t0

  # make sure XLEN agrees with compilation choice
  csrr t0, misa
#if __riscv_xlen == 64
  bltz t0, .LxlenCheck
#else
  bgez t0, .LxlenCheck
#endif
.LxlenCheck:

  # initialize global pointer, global data
  # The __global_pointer is allocated in the linker script. It points to a location 2k after sdata start
  # as the offsets used in the gp are +/- 2k
  # This location is a magic number that is used by the compiler
  # once
  # The sdata can be upto 4k
  # See https://www.sifive.com/blog/2017/08/28/all-aboard-part-3-linker-relaxation-in-riscv-toolchain/
  # fixme: did not like the above code, interrupt handler crashed when code below used >>>
  #.option push
  #.option norelax
  #1:auipc gp, %pcrel_hi(__global_pointer$)
  #addi  gp, gp, %pcrel_lo(1b)
  #.option pop
  # fixme: did not like the above code, interrupt handler crashed when code above used <<<
  #la gp, __global_pointer$
  la gp, _gp
  # see: http://www.rowleydownload.co.uk/arm/documentation/gnu/as/RISC_002dV_002dDirectives.html
  #.option push
  #.option norelax
  #la gp, __global_pointer$
  #.option pop

  # the following forces end alignment to 64byte boundery
  la  tp, _heap_end + 63
  and tp, tp, -64

  # get core id
  csrr a0, mhartid

  # give each core 128KB of stack + TLS
  #define STKSHIFT 17
  #change to 1K for the emulation platform ( dependant on bitfile )
#  #define STKSHIFT 10
  sll a2, a0, STKSHIFT
  add tp, tp, a2
  add sp, a0, 1
  sll sp, sp, STKSHIFT
  add sp, sp, tp

  # get core id
  csrr a0, mhartid
  li a1, MPFS_HAL_FIRST_HART
  bne a0, a1, .LOtherHartstoWFI
  # core MPFS_HAL_FIRST_HART jumps to main_first_hart
  j main_first_hart
.LoopForeverMain:
  #in case of return, loop forever. nop's added so can be seen in debugger
  nop
  nop
  j .LoopForeverMain


.LOtherHartstoWFI:
  # Set MSIE bit to receive IPI
  li a2, MIP_MSIP
  csrw mie, a2

.LwaitOtherHart:
  wfi
  # Only start if MIP_MSIP is set - the wfi will ensure this, but using breakpoints in the debugger
  # will wakeup wfi, so the following code will make sure we remain here until we get a software interrupt
  csrr a2, mip
  andi a2, a2, MIP_MSIP
  beqz a2, .LwaitOtherHart
  j main_other_hart
.LoopForeverOther:
  #in case of return, loop forever. nop's added so can be seen in debugger
  nop
  nop
  j .LoopForeverOther


.section ".tdata.begin"
.globl _tdata_begin
_tdata_begin:

.section ".tdata.end"
.globl _tdata_end
_tdata_end:

.section ".tbss.end"
.globl _tbss_end
_tbss_end:

# .section ".tohost","aw",@progbits
# .align 6
# .globl tohost
# tohost: .dword 0
# .align 6

# .globl fromhost
# fromhost: .dword 0

