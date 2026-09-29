/*******************************************************************************
 * Copyright 2019-2026 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * GNU linker script for sNVM Boot Mode 2 (BM2) Loader
 *
 * PolarFire SoC Boot Mode 2 (User Secure Boot): the system controller
 * authenticates the sNVM payload, copies it to the 8 KiB DTIM of the E51
 * monitor hart, then releases all five harts simultaneously to the DTIM
 * entry point.
 *
 * This loader must fit entirely in 8 KiB.  All code, read-only data, BSS
 * and per-hart stacks live in DTIM.  There are no split VMA/LMA sections
 * because the system controller handles the copy from sNVM.
 *
 * Two absolute symbols are provided so the C code can reference the eNVM
 * and L2LIM base addresses without hard-coding them:
 *
 *   __envm_start   0x20220100  eNVM base (128 KiB minus 256-byte header)
 *   __l2lim_start  0x08000000  L2LIM entry - l2lim-wrapper _start
 *
 * The loader calls config_l2_cache() so that the L2 way split is programmed
 * while the E51 is still executing from DTIM, rather than from L2LIM (see
 * bm2_main()).  config_l2_cache() validates the region sizes against the
 * design, so it needs the L2LIM and L2-scratchpad extents too.
 *
 * These MUST match the l2lim and l2zerodevice regions in
 * l2lim-wrapper/l2lim-wrapper.ld - the assertions in config_l2_cache() are
 * checking the memory the *wrapper* will use, not anything this loader owns.
 */

OUTPUT_ARCH("riscv")
ENTRY(_start)

PROVIDE(__envm_start  = 0x20220100);
PROVIDE(__l2lim_start = 0x08000000);
PROVIDE(__l2lim_end   = 0x08080000);    /* 512 KiB - keep in step with l2lim-wrapper.ld */
PROVIDE(__l2_scratchpad_vma_start = 0x0A000000);
PROVIDE(__l2_scratchpad_vma_end   = 0x0A080000);    /* 512 KiB */
PROVIDE(STACK_SIZE_PER_HART = 512);

MEMORY
{
    dtim (rwx) : ORIGIN = 0x01000000, LENGTH = 8k
}

SECTIONS
{
    . = ORIGIN(dtim);

    .text : ALIGN(4)
    {
        *(.entry)
        . = ALIGN(4);
        *(.text .text.*)
        *(.rodata .rodata.*)
        *(.srodata .srodata.*)
        . = ALIGN(8);
    } >dtim

    .data : ALIGN(8)
    {
        __data_start = .;
        *(.data .data.*)
        *(.sdata .sdata.*)
        . = ALIGN(8);
        __data_end = .;
    } >dtim

    .bss (NOLOAD) : ALIGN(16)
    {
        __bss_start = .;
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        *(COMMON)
        . = ALIGN(16);
        __bss_end = .;
    } >dtim

    /*
     * Per-hart stacks - one 512-byte slot per hart, placed at the top of
     * DTIM.  The stack for hart N starts at:
     *
     *   __stack_bottom + N * STACK_SIZE_PER_HART
     *
     * and the initial SP is set to the top of that slot minus 8.
     */
    .stack (NOLOAD) : ALIGN(64)
    {
        __stack_bottom = .;
        . += STACK_SIZE_PER_HART;   /* E51  (hart 0) */
        . += STACK_SIZE_PER_HART;   /* U54_1 (hart 1) */
        . += STACK_SIZE_PER_HART;   /* U54_2 (hart 2) */
        . += STACK_SIZE_PER_HART;   /* U54_3 (hart 3) */
        . += STACK_SIZE_PER_HART;   /* U54_4 (hart 4) */
        __stack_top = . - 8;
    } >dtim
}
