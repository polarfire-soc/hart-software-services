/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
*/

/***********************************************************************************
 * @file entry.S
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief entry functions.

 * SVN $Revision: 11865 $
 * SVN $Date: 2019-07-29 19:58:05 +0530 (Mon, 29 Jul 2019) $
 */
#include "bits.h"
#include "encoding.h"
#include "mtrap.h"
#include "config/software/mpfs_hal/mss_sw_config.h"

  .option norvc
  .section .text.init,"ax", %progbits
  .globl reset_vector
  .globl _start

reset_vector:
_start:
    /* Setup trap handler */
    la a4, trap_vector
    csrw mtvec, a4                    # initalise machine trap vector address
    /* Make sure that mtvec is updated */
    1:
    csrr    a5, mtvec
    bne a4, a5, 1b

    /* Disable and clear all interrupts */
    li a2,  MSTATUS_MIE
    csrc mstatus, a2        # clear interrupt enable bit
    csrw mie, zero
    csrw mip, zero
    # mscratch must be init to zero- we are not using scratch memory
    csrw mscratch, zero
    csrw mcause, zero
    csrw mepc, zero
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

    # enable FPU and accelerator if present, setting ignored on E51
    li t0, MSTATUS_FS | MSTATUS_XS
    csrs mstatus, t0

     # make sure XLEN agrees with compilation choice, if not will loop here
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
    # See https://www.sifive.com/blog/2017/08/28/all-aboard-part-3-linker-relaxation-in-riscv-toolchain/
    # see: http://www.rowleydownload.co.uk/arm/documentation/gnu/as/RISC_002dV_002dDirectives.html
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop

    # get core id
    csrr a0, mhartid
    li a1,  0
    beq a0, a1, .hart0
    li a1,  1
    beq a0, a1, .hart1
    li a1,  2
    beq a0, a1, .hart2
    li a1,  3
    beq a0, a1, .hart3
    li a1,  4
    beq a0, a1, .hart4

.hart0:
    la a4, __stack_bottom_h0$      # keep bottom of stack in a5 so we can init later
    la sp, __stack_top_h0$
    j .continue
.hart1:
    la a4, __stack_bottom_h1$      # keep bottom of stack in a5 so we can init later
    la sp, __stack_top_h1$
    j .continue
.hart2:
    la a4, __stack_bottom_h2$      # keep bottom of stack in a5 so we can init later
    la sp, __stack_top_h2$
    j .continue
.hart3:
    la a4, __stack_bottom_h3$      # keep bottom of stack in a5 so we can init later
    la sp, __stack_top_h3$
    j .continue
.hart4:
    la a4, __stack_bottom_h4$      # keep bottom of stack in a5 so we can init later
    la sp, __stack_top_h4$

.continue:
    # clear HLS and stack
    mv  a5, sp
.init_stack:
    #csrw mepc, zero
    STORE x0, 0(a4)
    add a4, a4, __SIZEOF_POINTER__
    blt a4, a5, .init_stack
    # Allocate some space at top of stack for the HLS
    addi sp, sp, -HLS_DEBUG_AREA_SIZE
    # HLS grows up from new top of stack
    mv tp, sp
    # get core id
    csrr a0, mhartid
    li a1, MPFS_HAL_FIRST_HART
    bne a0, a1, .LOtherHartstoWFI
    # clear the common heap
    la  a4, __heap_start
    la  a5, __bss_end
.init_heap:
    #csrw mepc, zero
    STORE x0, 0(a4)
    add a4, a4, __SIZEOF_POINTER__
    blt a4, a5, .init_heap
    # core MPFS_HAL_FIRST_HART jumps to main_first_hart
    j main_first_hart
.LoopForeverMain:
    #in case of return, loop forever. nop's added so can be seen in debugger
    nop
    nop
    j .LoopForeverMain

.LOtherHartstoWFI:
    li a2,  MSTATUS_MIE
    csrc mstatus, a2        # clear interrupt enable bit
    csrw mie, zero
    csrw mip, zero
    li a2, MIP_MSIP
    csrw mie, a2            # Set MSIE bit to receive IPI. This needs to be enabled- otherwise stays in wfi.
                            # Other interrupts appera to bring out of wfi, even if not enabled.
    li a1, HLS_DATA_IN_WFI
    STORE a1, 0(tp)
    /* flush the instruction cache */
    fence.i
.LwaitOtherHart:
    # We assume wfi instruction will be run before main hart attampts to take out of wfi
    wfi
    # Only start if MIP_MSIP is set - the wfi will ensure this, but adding breakpoints in the debugger (halt)
    # will wakeup wfi, so the following code will make sure we remain here until we get a software interrupt
    csrr a2, mip
    andi a2, a2, MIP_MSIP
    beqz a2, .LwaitOtherHart
    /* Disable and clear all interrupts- should be only a sw interrupt */
    li a2,  MSTATUS_MIE
    csrc mstatus, a2        # clear interrupt enable bit
    csrw mie, zero
    csrw mip, zero
    # set marker as to where we are
    li a1, HLS_DATA_PASSED_WFI
    STORE a1, 0(tp)
    j main_other_hart
.LoopForeverOther:
    #in case of return, loop forever. nop's added so can be seen in debugger
    nop
    nop
    j .LoopForeverOther

/****************************************************************************************/
/******************************interrupt handeling below here****************************/
/****************************************************************************************/
trap_vector:
    # The mscratch register is an XLEN-bit read/write register dedicated for use by machine mode.
    # Typically, it is used to hold a pointer to a machine-mode hart-local context space and swapped
    # with a user register upon entry to an M-mode trap handler.
    # In this implementation, we are noty using HLS
    # csrrw sp, mscratch, sp                    #copy sp to mscratch, and mscrath to sp

    addi sp, sp, -INTEGER_CONTEXT_SIZE     # moves sp down stack to make INTEGER_CONTEXT_SIZE area
    # Preserve the registers.
    STORE sp, 2*REGBYTES(sp)               # sp
    STORE a0, 10*REGBYTES(sp)              # save a0,a1 in the created CONTEXT
    STORE a1, 11*REGBYTES(sp)
    STORE ra, 1*REGBYTES(sp)
    STORE gp, 3*REGBYTES(sp)
    STORE tp, 4*REGBYTES(sp)
    STORE t0, 5*REGBYTES(sp)
    STORE t1, 6*REGBYTES(sp)
    STORE t2, 7*REGBYTES(sp)
    STORE s0, 8*REGBYTES(sp)
    STORE s1, 9*REGBYTES(sp)
    STORE a2,12*REGBYTES(sp)
    STORE a3,13*REGBYTES(sp)
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
    # Invoke the handler.
    mv a0, sp                               # a0 <- regs
    # Please note: mtval is the newer name for register mbadaddr
    # If you get a compile failure here, use the newer name
    # At this point (2019), both are supported in latest compiler
    # older compiler versions only support mbadaddr, so going with this.
    # See: https://github.com/riscv/riscv-gcc/issues/133
    csrr a1, mbadaddr                       # useful for anaysis when things go wrong
    csrr a2, mepc
    jal trap_from_machine_mode

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
    addi sp, sp, +INTEGER_CONTEXT_SIZE     # moves sp up stack to reclaim INTEGER_CONTEXT_SIZE area
    mret

 /****************************************************************************************/
 /******************************interrupt handeling above here****************************/
 /****************************************************************************************/

.enable_sw_int:
    li a2, MIP_MSIP
    csrw mie, a2            # Set MSIE bit to receive IPI
    li a2,  MSTATUS_MIE
    csrs mstatus, a2        # enable interrupts
    /* flush the instruction cache */
    fence.i
    ret
