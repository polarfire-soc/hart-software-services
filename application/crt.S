/*
 * The HSS software is released under the following software license:
 *
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Based on OpenSBI fw_base.S
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 */

#include <config.h>

#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_scratch.h>
#include <sbi/sbi_trap.h>

	.section .entry, "ax", %progbits
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.align	3
	.globl	_start
	.globl	_start_warm
_start:
	la	ra, _start
	call	_reset_regs

.macro LOAD_GP
        //
        // RISC-V Relaxing and the Global Pointer, GP
        //
        // The gp (Global Pointer) register optimises memory accesses within a single 4KB region.
        // This size is 4K because RISC-V immediate values are 12-bit signed values (+/- 2048)
        //
        // The linker uses the __global_pointer$ symbol definition to compare memory addresses
        // and, if within range, it replaces absolute/pc-relative addressing with gp-relative
        // addressing.
        //
        // This process can be disabled by -Wl,--no-relax.
        //
#if defined(CONFIG_LD_RELAX)
.option push
.option norelax
        la	gp, __global_pointer$
.option pop
#endif
.endm

	LOAD_GP

	// Preload HART details
	// s7 -> HART Count
	// s8 -> HART Stack Size
        // a0 -> current HART ID
	//
        la	a4, platform
	lwu	s7, SBI_PLATFORM_HART_COUNT_OFFSET(a4)
	//lwu	s8, SBI_PLATFORM_HART_STACK_SIZE_OFFSET(a4)
	la	s8, STACK_SIZE_PER_HART
        csrr	a0, CSR_MHARTID

        la	tp, __stack_bottom + 63
        and	tp, tp, -64
        li	a1, 1

.setup_stack:
	mul	a2, s8, a0
	add	tp, tp, a2
        la	sp, STACK_SIZE_PER_HART
	add	sp, sp, tp

.setup_scratches:
	// Setup scratch space for all the HARTs
	la	tp, scratches
	li	t2, 1 // Counter
	li	t1, 0 // hartid 0 is mandated by ISA

.disable_and_clear_interrupts:
	// Disable and clear all interrupts
	csrw	CSR_MIE, zero
	csrw	CSR_MIP, zero

.validate_hart_id:
	bge	a0, s7, _start_hang

.find_my_scratch_space:
	la	tp, scratches // find the scratch space for this hart
	li	a5, SBI_SCRATCH_SIZE
	mul	a5, a5, a0
	add	tp, tp, a5

	csrw	CSR_MSCRATCH, tp // update the mscratch

        csrr	a0, CSR_MHARTID
        beqz	a0, .boot_e51

.setup_u54_trap_handler:
	la	a4, _trap_handler
        call	.set_trap_handler

	csrw	CSR_SATP, 0 # clear SATP early on U54s, as it appears to be coming up randomly

.enable_interrupts:
        li	a2, MIP_MSIP // set MSIE bit to receive IPIs
        csrw	CSR_MIE, a2
        csrw    CSR_MSTATUS, a2

        jal	HSS_U54_Banner

#if defined(CONFIG_HSS_USE_IHC)
.enable_ihc:
        jal	HSS_IHCInit_U54
#endif

.spin_forever:
        wfi
        j	.spin_forever

.boot_e51:
#if defined(CONFIG_HSS_USE_IHC)
        // initialise PLIC if required for IHC
        jal	HSS_Setup_PLIC
#endif

	// Setup E51 trap handler
	la	a4, hss_e51_trap_handler
        call	.set_trap_handler

        // clearing of DTIM and L2LIM done from previous decompression stage

        li	a2, MIP_MSIP
	csrc	CSR_MSTATUS, a2

        j main

.set_trap_handler:
        csrw	CSR_MTVEC, a4
	//
        // Make sure that mtvec is updated
        //
        //
	csrr    a5, CSR_MTVEC
	bne	a4, a5, .set_trap_handler
	ret

	.section .entry, "ax", %progbits
	.align 3
	.globl _start_hang
_start_hang:
	wfi
	j	_start_hang

	.section .entry, "ax", %progbits
	.align 3
	.globl _trap_handler
_trap_handler:
	// Swap TP and MSCRATCH
	csrrw	tp, CSR_MSCRATCH, tp

	// Save T0 in scratch space
	REG_S	t0, SBI_SCRATCH_TMP0_OFFSET(tp)

	// Check which mode we came from
	csrr	t0, CSR_MSTATUS
	srl	t0, t0, MSTATUS_MPP_SHIFT
	and	t0, t0, PRV_M
	xori	t0, t0, PRV_M
	beq	t0, zero, _trap_handler_m_mode

	// We came from S-mode or U-mode
_trap_handler_s_mode:
	// Set T0 to original SP
	add	t0, sp, zero

	// Setup exception stack
	add	sp, tp, -(SBI_TRAP_REGS_SIZE)

	// Jump to code common for all modes
	j	_trap_handler_all_mode

	// We came from M-mode
_trap_handler_m_mode:
	// Set T0 to original SP
	add	t0, sp, zero

	// Re-use current SP as exception stack
	add	sp, sp, -(SBI_TRAP_REGS_SIZE)

_trap_handler_all_mode:
	// Save original SP (from T0) on stack
	REG_S	t0, SBI_TRAP_REGS_OFFSET(sp)(sp)

	// Restore T0 from scratch space
	REG_L	t0, SBI_SCRATCH_TMP0_OFFSET(tp)

	// Save T0 on stack
	REG_S	t0, SBI_TRAP_REGS_OFFSET(t0)(sp)

	// Swap TP and MSCRATCH
	csrrw	tp, CSR_MSCRATCH, tp

	// Save MEPC and MSTATUS CSRs
	csrr	t0, CSR_MEPC
	REG_S	t0, SBI_TRAP_REGS_OFFSET(mepc)(sp)
	csrr	t0, CSR_MSTATUS
	REG_S	t0, SBI_TRAP_REGS_OFFSET(mstatus)(sp)
	REG_S	zero, SBI_TRAP_REGS_OFFSET(mstatusH)(sp)

	// Save all general regisers except SP and T0
	//REG_S	zero, SBI_TRAP_REGS_OFFSET(zero)(sp)
	REG_S	ra, SBI_TRAP_REGS_OFFSET(ra)(sp)
	REG_S	gp, SBI_TRAP_REGS_OFFSET(gp)(sp)
	REG_S	tp, SBI_TRAP_REGS_OFFSET(tp)(sp)
	REG_S	t1, SBI_TRAP_REGS_OFFSET(t1)(sp)
	REG_S	t2, SBI_TRAP_REGS_OFFSET(t2)(sp)
	REG_S	s0, SBI_TRAP_REGS_OFFSET(s0)(sp)
	REG_S	s1, SBI_TRAP_REGS_OFFSET(s1)(sp)
	REG_S	a0, SBI_TRAP_REGS_OFFSET(a0)(sp)
	REG_S	a1, SBI_TRAP_REGS_OFFSET(a1)(sp)
	REG_S	a2, SBI_TRAP_REGS_OFFSET(a2)(sp)
	REG_S	a3, SBI_TRAP_REGS_OFFSET(a3)(sp)
	REG_S	a4, SBI_TRAP_REGS_OFFSET(a4)(sp)
	REG_S	a5, SBI_TRAP_REGS_OFFSET(a5)(sp)
	REG_S	a6, SBI_TRAP_REGS_OFFSET(a6)(sp)
	REG_S	a7, SBI_TRAP_REGS_OFFSET(a7)(sp)
	REG_S	s2, SBI_TRAP_REGS_OFFSET(s2)(sp)
	REG_S	s3, SBI_TRAP_REGS_OFFSET(s3)(sp)
	REG_S	s4, SBI_TRAP_REGS_OFFSET(s4)(sp)
	REG_S	s5, SBI_TRAP_REGS_OFFSET(s5)(sp)
	REG_S	s6, SBI_TRAP_REGS_OFFSET(s6)(sp)
	REG_S	s7, SBI_TRAP_REGS_OFFSET(s7)(sp)
	REG_S	s8, SBI_TRAP_REGS_OFFSET(s8)(sp)
	REG_S	s9, SBI_TRAP_REGS_OFFSET(s9)(sp)
	REG_S	s10, SBI_TRAP_REGS_OFFSET(s10)(sp)
	REG_S	s11, SBI_TRAP_REGS_OFFSET(s11)(sp)
	REG_S	t3, SBI_TRAP_REGS_OFFSET(t3)(sp)
	REG_S	t4, SBI_TRAP_REGS_OFFSET(t4)(sp)
	REG_S	t5, SBI_TRAP_REGS_OFFSET(t5)(sp)
	REG_S	t6, SBI_TRAP_REGS_OFFSET(t6)(sp)

	LOAD_GP

	// Call C routine
	csrr 	a0, CSR_MCAUSE
	sext.w	a0, a0
	blt	a0, zero, .checkIfOpenSBITrap

.checkIfHssIpi:
	call 	HSS_U54_HandleIPI 	// check if it is a HSS IPI
	bnez	a0, .skipOpenSbi

.checkIfOpenSBITrap:
	add	a0, sp, zero
	csrr	a1, CSR_MSCRATCH
	call	sbi_trap_handler

.skipOpenSbi:
	// Restore all general regisers except SP and T0
	REG_L	ra, SBI_TRAP_REGS_OFFSET(ra)(sp)
	REG_L	gp, SBI_TRAP_REGS_OFFSET(gp)(sp)
	REG_L	tp, SBI_TRAP_REGS_OFFSET(tp)(sp)
	REG_L	t1, SBI_TRAP_REGS_OFFSET(t1)(sp)
	REG_L	t2, SBI_TRAP_REGS_OFFSET(t2)(sp)
	REG_L	s0, SBI_TRAP_REGS_OFFSET(s0)(sp)
	REG_L	s1, SBI_TRAP_REGS_OFFSET(s1)(sp)
	REG_L	a0, SBI_TRAP_REGS_OFFSET(a0)(sp)
	REG_L	a1, SBI_TRAP_REGS_OFFSET(a1)(sp)
	REG_L	a2, SBI_TRAP_REGS_OFFSET(a2)(sp)
	REG_L	a3, SBI_TRAP_REGS_OFFSET(a3)(sp)
	REG_L	a4, SBI_TRAP_REGS_OFFSET(a4)(sp)
	REG_L	a5, SBI_TRAP_REGS_OFFSET(a5)(sp)
	REG_L	a6, SBI_TRAP_REGS_OFFSET(a6)(sp)
	REG_L	a7, SBI_TRAP_REGS_OFFSET(a7)(sp)
	REG_L	s2, SBI_TRAP_REGS_OFFSET(s2)(sp)
	REG_L	s3, SBI_TRAP_REGS_OFFSET(s3)(sp)
	REG_L	s4, SBI_TRAP_REGS_OFFSET(s4)(sp)
	REG_L	s5, SBI_TRAP_REGS_OFFSET(s5)(sp)
	REG_L	s6, SBI_TRAP_REGS_OFFSET(s6)(sp)
	REG_L	s7, SBI_TRAP_REGS_OFFSET(s7)(sp)
	REG_L	s8, SBI_TRAP_REGS_OFFSET(s8)(sp)
	REG_L	s9, SBI_TRAP_REGS_OFFSET(s9)(sp)
	REG_L	s10, SBI_TRAP_REGS_OFFSET(s10)(sp)
	REG_L	s11, SBI_TRAP_REGS_OFFSET(s11)(sp)
	REG_L	t3, SBI_TRAP_REGS_OFFSET(t3)(sp)
	REG_L	t4, SBI_TRAP_REGS_OFFSET(t4)(sp)
	REG_L	t5, SBI_TRAP_REGS_OFFSET(t5)(sp)
	REG_L	t6, SBI_TRAP_REGS_OFFSET(t6)(sp)

	// Restore MEPC and MSTATUS CSRs
	REG_L	t0, SBI_TRAP_REGS_OFFSET(mepc)(sp)
	csrw	CSR_MEPC, t0
	REG_L	t0, SBI_TRAP_REGS_OFFSET(mstatus)(sp)
	csrw	CSR_MSTATUS, t0

	// Restore T0
	REG_L	t0, SBI_TRAP_REGS_OFFSET(t0)(sp)

	// Restore SP
	REG_L	sp, SBI_TRAP_REGS_OFFSET(sp)(sp)

	mret

	.section .entry, "ax", %progbits
	.align	3
	.globl	_reset_regs
_reset_regs:
	// flush the instruction cache
	fence.i

	// Reset all registers except ra, a0, a1 and a2
	li gp,	0
	li sp,	0
	li tp,	0
	li t0,	0
	li t1,	0
	li t2,	0
	li s0,	0
	li s1,	0
	li a3,	0
	li a4,	0
	li a5,	0
	li a6,	0
	li a7,	0
	li s2,	0
	li s3,	0
	li s4,	0
	li s5,	0
	li s6,	0
	li s7,	0
	li s8,	0
	li s9,	0
	li s10,	0
	li s11,	0
	li t3,	0
	li t4,	0
	li t5,	0
	li t6,	0
	csrw	CSR_MSCRATCH, 0

	ret

	.section .entry, "ax", %progbits
	.align	3
	.globl	hss_e51_trap_handler
hss_e51_trap_handler:
        wfi
        j	hss_e51_trap_handler

/***********************************************************************************
 *
 * The following init_memory() symbol overrides the weak symbol in the HAL and does
 * a safe copy of RW data and clears zero-init memory
 *
 */
        // zero_section helper function:
        //       a0 = exec_start_addr
        //       a1 = exec_end_addr
        //
	.type	.zero_section, @function
.zero_section:
	beq	a0, a1, .zero_section_done
	sd	zero, (a0)
	addi	a0, a0, 8
	j	.zero_section
.zero_section_done:
	ret

        // copy_section helper function:
        //	a0 = load_addr
        //	a1 = exec_start_addr
        //	a2 = exec_end_addr
	.globl	copy_section
	.type	copy_section, @function
copy_section:
        beq     a1, a0, .copy_section_done // if load_addr == exec_start_addr, goto copy_section_done
.check_if_copy_section_done:
	beq	a1, a2, .copy_section_done // if offset != length, goto keep_copying
.keep_copying:
	ld	a3, 0(a0)                  // val = *load_addr
	sd	a3, 0(a1)                  // *exec_start_addr = val;
	addi	a0, a0, 8                  // load_addr = load_addr + 8
	addi	a1, a1, 8                  // exec_start_addr = exec_start_addr + 8
        j	.check_if_copy_section_done
.copy_section_done:
        ret

        // init_memory function, used to initialize memory early before C code runs
        //
	.globl	init_memory
	.type	init_memory, @function
init_memory:
	addi	sp,sp,-16
	sd	ra,8(sp)

        //
        // Initialize R/W data
        //  (sdata and data sections)
        //
        la	a0, __sdata_load
        la	a1, __sdata_start
        la	a2, __sdata_end
	call	copy_section

        la	a0, __data_load
        la	a1, __data_start
        la	a2, __data_end
	call	copy_section

        la	a0, __l2_scratchpad_load
        la	a1, __l2_scratchpad_start
        la	a2, __l2_scratchpad_end
	call	copy_section

        //
        // Clear zero-init memory
        //  (SBSS and BSS sections)
        //
        la	a0, __sbss_start
        la	a1, __sbss_end
	call	.zero_section

        la	a0, __bss_start
        la	a1, __bss_end

	ld	ra,8(sp)
	addi	sp,sp,16
	tail	.zero_section

/***********************************************************************************
 *
 * The following copy_switch_code() symbol overrides the weak symbol in the HAL and does
 * a safe copy of HW config data
 */
	.globl	copy_switch_code
	.type	copy_switch_code, @function
copy_switch_code:
        la      a5, __sc_start               // a5 = __sc_start
        la      a4, __sc_load                // a4 = __sc_load
	beq	a5,a4,.copy_switch_code_done // if a5 == a4, goto copy_switch_code_done
        la      a3, __sc_end                 // a3 = __sc_end
	beq	a5,a3,.copy_switch_code_done // if a5 == a3, goto copy_switch_code_done
.copy_switch_code_loop:
	lw	a2,0(a4)                     // a2 = *a4
	sw	a2,0(a5)                     // *a5 = a2
	addi	a5,a5,4                      // a5+=4
	addi	a4,a4,4                      // a4+=4

	bltu	a5,a3,.copy_switch_code_loop // if a5 < a3, goto copy_switch_code_loop
.copy_switch_code_done:
	ret

/***********************************************************************************
 *
 */

.clear_l2lim:
	// Clear the LIM
	//
	// On reset, the first 15 ways are L2 and the last way is cache
	// We can initialize all, as cache write through to DDR is blocked
	// until DDR in initialized, so will have no effect other than clear ECC
	//
	// NOTE: we need to check if we are debugging from LIM,if so do not initialize
	//
	la	a2, _hss_start
	la 	a4, __l2lim_start
	beq	a2, a4, .done_clear
	la	a5, __l2lim_end
        j	1f
.clear_dtim:
        //
        // Clear the E51 DTIM to prevent any memory errors on initial access
        // to the cache
        //
        la	a4, __dtim_start
        la	a5, __dtim_end
1:
        // common loop used by both .clear_l2lim and .clear_dtim
	REG_S	x0, 0(a4)
	add	a4, a4, __SIZEOF_POINTER__
	blt	a4, a5, 1b
.done_clear:
	ret
