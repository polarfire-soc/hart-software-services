/*******************************************************************************
 * Copyright 2019-2026 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * GNU linker script for Hart Software Services (HSS) L2LIM Wrapper
 *
 * The sNVM boot code FEC-decodes the FEC-protected image from eNVM into L2LIM,
 * then jumps to L2LIM entry for all five harts. This wrapper (running from
 * L2LIM) then decompresses the embedded DEFLATE-compressed HSS image to
 * L2-Scratchpad for execution.
 *
 * Unlike envm-wrapper (which executes directly from eNVM), this wrapper is
 * linked to run entirely from L2LIM. It must NOT clear L2LIM on startup since
 * its own code and the compressed payload reside there.
 */

/*
 PolarFire SoC Memory map (ditaa diagram)
 ----------------------------------------
                                                                                   +-------------+
                                                      +-----------+                | non-cache   |
                                                      | non-cache |                |  WCB (SEG1) |
               +---------------------+                |  (SEG1)   | 0x18_0000_0000 +-------------+
               | DDR cached          | 0x14_0000_0000 +-----------+                | non-cache   |
               |  (SEG0)             |                | non-cache |                |  WCB (SEG1) |
0x10_0000_0000 +---------------------+                |  (SEG1)   |    0xD000_0000 +-------------+
               | DDR cached          |    0xC000_0000 +-----------+
               |  (SEG0)             |
   0x8000_0000 +---------------------+
               | eNVM (128KiB)       |  FEC-encoded image stored here
               |  [FEC(l2lim-wrap    |  -----------+
               |   + compressed HSS)]|             |
   0x2022_0100 +---------------------+             |  sNVM boot code FEC-decodes
               | Zero Device         |             |  to L2LIM, jumps to 0x0800_0000
               |                     |             |
               | L2Scratch (512KiB)  | <------ decompresses HSS here
               |                     |             |
   0x0A00_0000 +---------------------+             |
               |                     |             |
   0x0820_0000 +---------------------+             |
               | LIM (up to 1920KiB) | <-----------+
               | [wrapper + HSS.lz]  |
   0x0800_0000 +---------------------+
               | U54_4 ITIM (28KiB)  |
               |                     |
   0x0182_0000 +---------------------+
               | U54_3 ITIM (28KiB)  |
               |                     |
   0x0181_8000 +---------------------+
               | U54_2 ITIM (28KiB)  |
               |                     |
   0x0181_0000 +---------------------+
               | U54_1 ITIM (28KiB)  |
               |                     |
   0x0180_8000 +---------------------+
               | E51 ITIM (28KiB)    |
               |                     |
   0x0180_0000 +---------------------+
               | DTIM (8KiB)         |
               |                     |
   0x0100_0000 +---------------------+
               |                     |
               +---------------------+
               | Debug               |
   0x0000_0000 +---------------------+
 */

OUTPUT_ARCH( "riscv" )
ENTRY(_start)

PROVIDE(HEAP_SIZE = 64k);
PROVIDE(STACK_SIZE_PER_HART = 16k);
MEMORY
{
    envm (rx) :		ORIGIN = 0x20220100,	LENGTH = (128k-256)
    dtim (rwx) :	ORIGIN = 0x01000000,	LENGTH = 8k
    e51_itim (rwx) :	ORIGIN = 0x01800000,	LENGTH = 28k
    u54_1_itim (rwx) :	ORIGIN = 0x01808000,	LENGTH = 28k
    u54_2_itim (rwx) :	ORIGIN = 0x01810000,	LENGTH = 28k
    u54_3_itim (rwx) :	ORIGIN = 0x01818000,	LENGTH = 28k
    u54_4_itim (rwx) :	ORIGIN = 0x01820000,	LENGTH = 28k
    l2lim (rwx) :	ORIGIN = 0x08000000,	LENGTH = 512k
    l2zerodevice (rwx) :ORIGIN = 0x0A000000,	LENGTH = 512k
    ddr (rwx) :		ORIGIN = 0x80000000,	LENGTH = 1024m
    ddrhi (rwx) :	ORIGIN = 0x1400000000,	LENGTH = 1024m
}

SECTIONS
{
    PROVIDE(__l2lim_start =             ORIGIN(l2lim));
    PROVIDE(__l2lim_end =               ORIGIN(l2lim)        + LENGTH(l2lim));
    PROVIDE(__l2lim_end_max =           ORIGIN(l2lim)        + (128k*(16-1)));
    PROVIDE(__l2_start =                ORIGIN(l2zerodevice));
    PROVIDE(__l2_end =                  ORIGIN(l2zerodevice) + LENGTH(l2zerodevice));
    PROVIDE(__ddr_start =               ORIGIN(ddr));
    PROVIDE(__ddr_end =                 ORIGIN(ddr)          + LENGTH(ddr));
    PROVIDE(__ddrhi_start =             ORIGIN(ddrhi));
    PROVIDE(__ddrhi_end =               ORIGIN(ddrhi)        + LENGTH(ddrhi));
    PROVIDE(__dtim_start =              ORIGIN(dtim));
    PROVIDE(__dtim_end =                ORIGIN(dtim)         + LENGTH(dtim));
    PROVIDE(__e51itim_start =           ORIGIN(e51_itim));
    PROVIDE(__e51itim_end =             ORIGIN(e51_itim)     + LENGTH(e51_itim));
    PROVIDE(__u54_1_itim_start =        ORIGIN(u54_1_itim));
    PROVIDE(__u54_1_itim_end =          ORIGIN(u54_1_itim)   + LENGTH(u54_1_itim));
    PROVIDE(__u54_2_itim_start =        ORIGIN(u54_2_itim));
    PROVIDE(__u54_2_itim_end =          ORIGIN(u54_2_itim)   + LENGTH(u54_2_itim));
    PROVIDE(__u54_3_itim_start =        ORIGIN(u54_3_itim));
    PROVIDE(__u54_3_itim_end =          ORIGIN(u54_3_itim)   + LENGTH(u54_3_itim));
    PROVIDE(__u54_4_itim_start =        ORIGIN(u54_4_itim));
    PROVIDE(__u54_4_itim_end =          ORIGIN(u54_4_itim)   + LENGTH(u54_4_itim));
    PROVIDE(_hss_start =                ORIGIN(l2zerodevice));
    PROVIDE(__l2_scratchpad_vma_start = ORIGIN(l2zerodevice));
    PROVIDE(__l2_scratchpad_vma_end =   ORIGIN(l2zerodevice) + LENGTH(l2zerodevice));

    . = __l2lim_start;

    /* Wrapper code and embedded compressed HSS payload - run and load from L2LIM */
    .text : ALIGN(0x10)
    {
        *(.entry)
        . = ALIGN(0x10);
        *(.text .text.* .gnu.linkonce.t.*)
        *(.plt)
        . = ALIGN(0x10);
        KEEP (*crtbegin.o(.ctors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*crtend.o(.ctors))
        KEEP (*crtbegin.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*crtend.o(.dtors))
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2)
        *(.srodata*)
        *(.sdata2*)
        *(.gcc_except_table)
        *(.eh_frame_hdr)
        *(.eh_frame)
        KEEP (*(.init))
        KEEP (*(.fini))
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(.fini_array))
        KEEP (*(SORT(.fini_array.*)))
        PROVIDE_HIDDEN (__fini_array_end = .);
        . = ALIGN(0x10);
    } >l2lim

    .gnu_build_id : ALIGN(8) {
        PROVIDE(gnu_build_id = .);
        *(.note.gnu.build-id)
    } >l2lim

    /* R/W data: loaded from L2LIM (LMA), executed from DTIM (VMA) */
    .sdata : ALIGN(0x40)
    {
        __sdata_load = LOADADDR(.sdata);
        __sdata_start = .;
        __global_pointer$ = . + 0x800;
        *(.sdata .sdata.* .gnu.linkonce.s.*)
        . = ALIGN(0x10);
        __sdata_end = .;
    } >dtim AT>l2lim

    .data : ALIGN(0x40)
    {
        __data_load = LOADADDR(.data);
        __data_start = .;
        *(.got.plt) *(.got)
        *(.shdata)
        *(.data .data.* .gnu.linkonce.d.*)
        . = ALIGN(0x10);
        __data_end = .;
    } >dtim AT>l2lim
    _end = .;

    .sbss : ALIGN(0x40)
    {
        __sbss_start = .;
        *(.sbss .sbss.* .gnu.linkonce.sb.*)
        *(.scommon)
        . = ALIGN(0x10);
        __sbss_end = .;
    } >dtim

    .bss : ALIGN(0x40)
    {
        __bss_start = .;
        *(.shbss)
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(0x10);
        __bss_end = .;
    } >dtim

    /*
     * Per-hart stacks placed in L2LIM, after the code and compressed payload.
     * The sNVM FEC-decode wrote valid ECC for the code region; the stack area
     * is safe because the first stack accesses are always writes (push/sd).
     */
    .stack : ALIGN(0x40)
    {
        __stack_bottom = .;
        __stack_bottom_h0$ = .;
        . += STACK_SIZE_PER_HART;
        __stack_top_h0$ = . - 8;
        __stack_bottom_h1$ = .;
        . += STACK_SIZE_PER_HART;
        __stack_top_h1$ = . - 8;
        __stack_bottom_h2$ = .;
        . += STACK_SIZE_PER_HART;
        __stack_top_h2$ = . - 8;
        __stack_bottom_h3$ = .;
        . += STACK_SIZE_PER_HART;
        __stack_top_h3$ = . - 8;
        __stack_bottom_h4$ = .;
        . += STACK_SIZE_PER_HART;
        __stack_top_h4$ = . - 8;
        __stack_top = . - 8;
    } >l2lim

   .heap : ALIGN(0x40)
    {
        PROVIDE(__l2lim_heap_base = .);
        . += HEAP_SIZE;
    } >l2lim
    PROVIDE(__l2lim_heap_end  =.);

    PROVIDE(_hss_end = .);
}
