/*******************************************************************************
 * Copyright 2019-2021 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * GNU linker script for Hart Software Services (HSS) ENVM Wrapper
 *  The purpose of the wrapper is to allow a DEFLATE compressed image to be 
 *  stored in ENVM, but decompressed (inflated) to L2-LIM at runtime...
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
               | eNVM (128KiB)       | ------------------+
               |                     |                   |
   0x2022_0100 +---------------------+                   | decompresses HSS
               | Zero Device         |                   | (inflate) from eNVM
               |                     |                   | to LIM
   0x0A00_0000 +---------------------+                   |
               |                     |                   |
   0x0820_0000 +---------------------+                   |
               | LIM (up to 1920KiB) | <-----------------+
               |                     |
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
    l2lim (rwx) :	ORIGIN = 0x08000000,	LENGTH = (1920k-144k)
    l2lim_stack (rwx) :	ORIGIN = 0x081BC000,	LENGTH = 80k
    l2lim_heap (rwx) :	ORIGIN = 0x081D0000,	LENGTH = 64k
    l2zerodevice (rwx) :ORIGIN = 0x0A000000,	LENGTH = 512k
    ddr (rwx) :		ORIGIN = 0x80000000,	LENGTH = 1024m
    ddrhi (rwx) :	ORIGIN = 0x1400000000,	LENGTH = 1024m
}

SECTIONS
{
    PROVIDE(__envm_start =		ORIGIN(envm));
    PROVIDE(__envm_end =		ORIGIN(envm)		+ LENGTH(envm));
    PROVIDE(__l2lim_start =		ORIGIN(l2lim));
    PROVIDE(__l2lim_end =		ORIGIN(l2lim)		+ LENGTH(l2lim));
    PROVIDE(__ddr_start =		ORIGIN(ddr));
    PROVIDE(__ddr_end =			ORIGIN(ddr)		+ LENGTH(ddr));
    PROVIDE(__ddrhi_start =		ORIGIN(ddrhi));
    PROVIDE(__ddrhi_end =		ORIGIN(ddrhi)		+ LENGTH(ddrhi));
    PROVIDE(__dtim_start =		ORIGIN(dtim));
    PROVIDE(__dtim_end =		ORIGIN(dtim)		+ LENGTH(dtim));
    PROVIDE(__e51itim_start =		ORIGIN(e51_itim));
    PROVIDE(__e51itim_end =		ORIGIN(e51_itim)	+ LENGTH(e51_itim));
    PROVIDE(__u54_1_itim_start =	ORIGIN(u54_1_itim));
    PROVIDE(__u54_1_itim_end =		ORIGIN(u54_1_itim)	+ LENGTH(u54_1_itim));
    PROVIDE(__u54_2_itim_start =	ORIGIN(u54_2_itim));
    PROVIDE(__u54_2_itim_end =		ORIGIN(u54_2_itim)	+ LENGTH(u54_2_itim));
    PROVIDE(__u54_3_itim_start =	ORIGIN(u54_3_itim));
    PROVIDE(__u54_3_itim_end =		ORIGIN(u54_3_itim)	+ LENGTH(u54_3_itim));
    PROVIDE(__u54_4_itim_start =	ORIGIN(u54_4_itim));
    PROVIDE(__u54_4_itim_end =		ORIGIN(u54_4_itim)	+ LENGTH(u54_4_itim));
    PROVIDE(__l2lim_start =		ORIGIN(l2lim));
    PROVIDE(__l2lim_end =		ORIGIN(l2lim)		+ LENGTH(l2lim));
    PROVIDE(_hss_start = 		__l2lim_start);

    PROVIDE(__l2lim_heap_base =		ORIGIN(l2lim_heap));
    PROVIDE(__l2lim_heap_end  =		ORIGIN(l2lim_heap)	+ LENGTH(l2lim_heap));

    . = __envm_start;

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
    } >envm

    .gnu_build_id : ALIGN(8) {
        PROVIDE(gnu_build_id = .);
        *(.note.gnu.build-id)
    } >envm

    PROVIDE(_hss_end = .);
    .l2_scratchpad : ALIGN(0x10)
    {
        __l2_scratchpad_load = LOADADDR(.l2_scratchpad);
        __l2_scratchpad_start = .;
        __l2_scratchpad_vma_start = .;
        *(.l2_scratchpad)
        . = ALIGN(0x10);
        __l2_scratchpad_end = .;
        __l2_scratchpad_vma_end = .;
    } >l2lim AT>envm

    .sdata : ALIGN(0x40)
    {
        __sdata_load = LOADADDR(.sdata);
        __sdata_start = .;
        __global_pointer$ = . + 0x800;
        *(.sdata .sdata.* .gnu.linkonce.s.*)
        . = ALIGN(0x10);
        __sdata_end = .;
    } >dtim AT>envm

    .data : ALIGN(0x40)
    {
        __data_load = LOADADDR(.data);
        __data_start = .;
        *(.got.plt) *(.got)
        *(.shdata)
        *(.data .data.* .gnu.linkonce.d.*)
        . = ALIGN(0x10);
        __data_end = .;
    } >dtim AT>envm

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
    } >l2lim_stack

    _end = .;
}
