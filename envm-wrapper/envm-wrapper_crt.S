/*
 * The HSS software is released under the following software license:
 *
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 */


#include "config.h"
#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_scratch.h>
#include <sbi/sbi_trap.h>

/*
 * These must match the definition in include/hss_types.h!
 *
 */
#define mHSS_COMPRESSED_VERSION_FASTLZ 1u
#define mHSS_COMPRESSED_VERSION_DEFLATE  2u

#define OFFSET_OF(TYPE, FIELD) TYPE##_##FIELD##_##OFFSET
#define HSS_CompressedImage_magic_OFFSET		0u
#define HSS_CompressedImage_version_OFFSET		4u
#define HSS_CompressedImage_headerLength_OFFSET		8u
#define HSS_CompressedImage_headerCrc_OFFSET		16u
#define HSS_CompressedImage_compressedCrc_OFFSET	20u
#define HSS_CompressedImage_originalCrc_OFFSET		24u
#define HSS_CompressedImage_compressedImageLen_OFFSET	32u
#define HSS_CompressedImage_originalImageLen_OFFSET	40u
#define HSS_CompressedImage_hash_OFFSET			48u
#define HSS_CompressedImage_ecdsaSig_OFFSET		80u

/*
 */
	.section .entry, "ax", @progbits
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.align	3
	.globl	_start

_start:
	#j _start
	la	t0, .early_trap_handler
	csrw	CSR_MTVEC, t0
	li	ra, 0
.reset_regs:
	// flush the instruction cache
	fence.i

	// Reset all registers except ra, a0, a1 and a2
	li	gp,	0
	li	sp,	0
	li	tp,	0
	li	t0,	0
	li	t1,	0
	li	t2,	0
	li	s0,	0
	li	s1,	0
	li	a3,	0
	li	a4,	0
	li	a5,	0
	li	a6,	0
	li	a7,	0
	li	s2,	0
	li	s3,	0
	li	s4,	0
	li	s5,	0
	li	s6,	0
	li	s7,	0
	li	s8,	0
	li	s9,	0
	li	s10,	0
	li	s11,	0
	li	t3,	0
	li	t4,	0
	li	t5,	0
	li	t6,	0
	csrw	CSR_MSCRATCH, 0

.configure:
	li	a0, 0
	la	a1, decompress_done_flag
	REG_S	a0, 0(a1)

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

.disable_and_clear_interrupts:
	csrw	CSR_MIE, zero			# disable all interrupts
	csrw	CSR_MIP, zero			# clear all interrupts

.check_if_e51:
        beqz	a0, .e51_decompress

.wait_for_decompress_done:
	la	a1, decompress_done_flag
	REG_L	a0, 0(a1)
	beqz	a0, .wait_for_decompress_done
        j	.decompress_done

.e51_decompress:
        call	.clear_dtim
        call	.clear_l2lim
	call	init_memory
	call	HSS_Setup_Clocks

	#ebreak
	#call	config_l2_cache_ex				# setup L2Cache
	#ebreak

	la	a0, hss_l2lim_lz				# compressed image header
	lw	a1, OFFSET_OF(HSS_CompressedImage, version)(a0)	# version
	li	a2, mHSS_COMPRESSED_VERSION_DEFLATE
	beq	a1, a2, .e51_miniz_decompress
.spin_unknown_compressor:
	j	.spin_unknown_compressor

.e51_miniz_decompress:
	# a3 => compressed length...
        REG_L	a3, OFFSET_OF(HSS_CompressedImage, compressedImageLen)(a0)

	# a2 => pointer to source
	REG_L	a2, OFFSET_OF(HSS_CompressedImage, headerLength)(a0)
	add	a2, a0, a2			# ... jump source past header to payload

	# a1 => pointer to store decompressed size, seeded with inital value from image header
	REG_L	a0, OFFSET_OF(HSS_CompressedImage, originalImageLen)(a0)
	la	a1, decompressed_size
	REG_S	a0, 0(a1)

	# a0 => pointer to destination (where to decompress to)
	la	a0, __l2lim_start		# destination
	call 	mz_uncompress
	j	.clear_progress

.clear_progress:
	call	validate_crc

	li	a0, 1
	la	a1, decompress_done_flag
	REG_S	a0, 0(a1)

.decompress_done:
	fence.i
	nop
	nop

	call	_hss_start
	la	a0, _hss_start
	csrw	CSR_MEPC, a0
	csrw	CSR_MIE, zero			# disable all interrupts
	csrr	a0, CSR_MSTATUS
	and	a0, a0, ~MSTATUS_MPIE
	li	a1, ~MSTATUS_MPP
	and	a0, a0, a1
	li	a1, (PRV_M << 11)
	or	a0, a0, a1
	csrw	CSR_MSTATUS, a0
	csrr	a0, CSR_MHARTID
	li	a1, 0
	mret

	.align	3
.early_trap_handler:
	j .early_trap_handler

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
	.text
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
 * The following config_copy() symbol overrides the weak symbol in the HAL and does
 * a safe copy of HW config data
 */
        // config_copy helper function:
        //	a0 = dest
        //	a1 = src
        //	a2 = length
        .globl  config_copy
        .type   config_copy, @function
config_copy:
        li      a5,0                     # initialize offset
.check_if_config_copy_done:
        beq     a2,a5, .config_copy_done # if offset == length, goto config_copy_done
.do_config_copy:
        add     a4,a1,a5                 # csrc = src + offset
        lbu     a3,0(a4)                 # tmp = *csrc
        add     a4,a0,a5                 # cdest = dest + offset
        addi    a5,a5,1                  # offset = offset + 1
        sb      a3,0(a4)                 # *cdest = *csrc
        j       .check_if_config_copy_done
.config_copy_done:
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
	la 	a4, __l2lim_start
	la	a5, __l2lim_heap_end
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

/***********************************************************************************
 *
 */
	.section .text, "ax", @progbits
	.option push
	.option norelax
	.align 4
	.globl	hss_l2lim_lz
hss_l2lim_lz:
	.incbin	"Default/hss-l2lim.bin.deflate"

	.section .data
	.align 8
decompressed_size:
	.long	0
	.align 4
	.data
decompress_done_flag:
	.int	0

	.option pop
