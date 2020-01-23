// See LICENSE.md for license details.

#include "mtrap.h"
#include "bits.h"

  .data
  .align 6
trap_table:
#define BAD_TRAP_VECTOR 0
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
#define TRAP_FROM_MACHINE_MODE_VECTOR 13
  .word __trap_from_machine_mode
  .word bad_trap
  .word bad_trap

  .option norvc
  .section .text.init,"ax",@progbits
  .align 8
  .globl reset_vector
reset_vector:
  j _start


  .globl trap_vector

trap_vector:
  STORE a0, 10*REGBYTES(sp)
  STORE a1, 11*REGBYTES(sp)

  csrr a1, mcause
  bgez a1, .Lhandle_trap_in_machine_mode

  # This is an interrupt.  Discard the mcause MSB and decode the rest.
  sll a1, a1, 1

  # Is it a machine timer interrupt?
  li a0, IRQ_M_TIMER * 2
  bne a0, a1, 1f

  # Yes.  Simply clear MSIE and raise SSIP. TODO: why disable MIE?
  li a0, MIP_MTIP
  csrc mip, a0 
  li a0, MIP_STIP
  csrs sip, a0 

.Lmret:
  # Go back whence we came.
  LOAD a0, 10*REGBYTES(sp)
  LOAD a1, 11*REGBYTES(sp)
  //csrrw sp, mscratch, sp
  mret

1:
  # Is it an IPI?
  li a0, IRQ_M_SOFT * 2
  bne a0, a1, .Lbad_trap

  # Now, decode the cause(s).
  call U54_HandleIPI
  bnez a0, 2f
  #li a0, MIP_MSIP
  #csrc mip, a0 
  #li a0, MIP_SSIP
  csrc mip, MIP_MSIP
  csrs mip, MIP_SSIP
  j 2f

2:

  # Yes.  First, clear the MIPI bit.
  LOAD a0, MENTRY_IPI_OFFSET(sp)
  sw x0, (a0)
  fence

  # Now, decode the cause(s).
#ifdef __riscv_atomic
  addi a0, sp, MENTRY_IPI_PENDING_OFFSET
  amoswap.w a0, x0, (a0)
#else
  lw a0, MENTRY_IPI_PENDING_OFFSET(a0)
  sw x0, MENTRY_IPI_PENDING_OFFSET(a0)
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
  sfence.vma
1:
  andi a1, a0, IPI_HALT
  beqz a1, 1f

  #jal 0x20220000 # jump into NVM
  #li a5, 0x20220000
  #jalr a5
  #j _start
  #wfi
  #j 1b
1:
  j .Lmret


.Lhandle_trap_in_machine_mode:
.Ltrap_from_machine_mode:
__trap_from_machine_mode:
.Lbad_trap:
  li a1, BAD_TRAP_VECTOR
  j .Lhandle_trap_in_machine_mode
