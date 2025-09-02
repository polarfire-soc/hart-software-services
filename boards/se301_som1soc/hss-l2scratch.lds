/*******************************************************************************
 * Copyright 2019-2021 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * GNU linker script for Hart Software Services (HSS)
 *
 */

#include "config.h"

OUTPUT_ARCH( "riscv" )

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
               | envm (128KiB)       |
               |                     |
   0x2022_0100 +---------------------+
               | Zero Device         |
               |                     |
   0x0A00_0000 +---------------------+
               |                     |
   0x0820_0000 +---------------------+
               | LIM (up to 1920KiB) |
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
               | Debug               |
   0x0000_0000 +---------------------+

 */

/*******************************************************************************
 *
 * -- MSS hart Reset vector
 *
 * The MSS reset vector for each hart is configured by Libero and stored securely
 * in the MPFS.
 *
 * The most common usage will be where the reset vector for each hart will be set
 * to the start of the envm at address 0x2022_0100, giving (128KiB-256bytes)
 * of contiguous non-volatile storage. Normally this is where the initial
 * boot-loader will reside.
 *
 * Libero outputs the configured reset vector address to the xml file, see
 * LIBERO_SETTING_RESET_VECTOR_HART0 etc in <hw_memory.h>
 *
 * When debugging a bare metal program that is run out of reset from envm, a linker
 * script will be used whereby the program will run from LIM instead of envm.
 * In this case, set the reset vector in the linker script to 0x0800_0000.
 * This means you are not continually programming the envm each time you load a
 * program and there is no limitation with hardware break points whn debugging.
 */
ENTRY(_start)


/*******************************************************************************
 *
 * Memory Segments
 *
 * must be on 4k boundary (0x1000) - corresponds to page size, when using memory mem
 */
MEMORY
{
    envm (rx)          : ORIGIN = 0x20220100, LENGTH = (128k-256)

    dtim (rwx)         : ORIGIN = 0x01000000, LENGTH = 7k /* DTIM */
    switch_code (rx)   : ORIGIN = 0x01001c00, LENGTH = 1k /* This 1K of DTIM is used to run code
                                                           * when switching the envm clock */
    e51_itim (rwx)     : ORIGIN = 0x01800000, LENGTH = 28k
    u54_1_itim (rwx)   : ORIGIN = 0x01808000, LENGTH = 28k
    u54_2_itim (rwx)   : ORIGIN = 0x01810000, LENGTH = 28k
    u54_3_itim (rwx)   : ORIGIN = 0x01818000, LENGTH = 28k
    u54_4_itim (rwx)   : ORIGIN = 0x01820000, LENGTH = 28k
    l2lim (rwx)        : ORIGIN = 0x08000000, LENGTH = 512k
    l2zerodevice (rwx) : ORIGIN = 0x0A000000, LENGTH = 512k
    ddr (rwx)          : ORIGIN = 0x80000000, LENGTH = 32m
    ddrhi (rwx)        : ORIGIN = 0x1000000000, LENGTH = 1888m
    ncddrhi (rwx)      : ORIGIN = 0x1400000000, LENGTH = 2048m
}

PROVIDE(HEAP_SIZE = 0k);
PROVIDE(STACK_SIZE_PER_HART = 16k);

/*******************************************************************************
 *
 * Memory Sections and Placement
 */
SECTIONS
{
    PROVIDE(__envm_start       = ORIGIN(envm));
    PROVIDE(__envm_end         = ORIGIN(envm) + LENGTH(envm));

    PROVIDE(__l2lim_start      = ORIGIN(l2lim));
    PROVIDE(__l2lim_end        = ORIGIN(l2lim) + LENGTH(l2lim));

    PROVIDE(__l2_start         = ORIGIN(l2zerodevice));
    PROVIDE(__l2_end           = ORIGIN(l2zerodevice) + LENGTH(l2zerodevice));

    PROVIDE(__ddr_start        = ORIGIN(ddr));
    PROVIDE(__ddr_end          = ORIGIN(ddr) + LENGTH(ddr));

    PROVIDE(__ddrhi_start      = ORIGIN(ddrhi));
    PROVIDE(__ddrhi_end        = ORIGIN(ddrhi) + LENGTH(ddrhi));
    PROVIDE(__ddrhi_max_end    = ORIGIN(ddrhi) + LENGTH(ncddrhi));

    PROVIDE(__ncddrhi_start    = ORIGIN(ncddrhi));
    PROVIDE(__ncddrhi_end      = ORIGIN(ncddrhi) + LENGTH(ncddrhi));

    PROVIDE(__dtim_start       = ORIGIN(dtim));
    PROVIDE(__dtim_end         = ORIGIN(dtim) + LENGTH(dtim));

    PROVIDE(__e51itim_start    = ORIGIN(e51_itim));
    PROVIDE(__e51itim_end      = ORIGIN(e51_itim) + LENGTH(e51_itim));

    PROVIDE(__u54_1_itim_start = ORIGIN(u54_1_itim));
    PROVIDE(__u54_1_itim_end   = ORIGIN(u54_1_itim) + LENGTH(u54_1_itim));

    PROVIDE(__u54_2_itim_start = ORIGIN(u54_2_itim));
    PROVIDE(__u54_2_itim_end   = ORIGIN(u54_2_itim) + LENGTH(u54_2_itim));

    PROVIDE(__u54_3_itim_start = ORIGIN(u54_3_itim));
    PROVIDE(__u54_3_itim_end   = ORIGIN(u54_3_itim) + LENGTH(u54_3_itim));

    PROVIDE(__u54_4_itim_start = ORIGIN(u54_4_itim));
    PROVIDE(__u54_4_itim_end   = ORIGIN(u54_4_itim) + LENGTH(u54_4_itim));

    /*
     * Code and RO data lives in l2lim
     */
    . = __l2_start;

    PROVIDE(_hss_start = .);
    PROVIDE(__l2_scratchpad_vma_start = .);

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
    } >l2zerodevice

    .gnu_build_id : ALIGN(8) {
        PROVIDE(gnu_build_id = .);
        *(.note.gnu.build-id)
    } >l2zerodevice

    PROVIDE(_hss_end = .);

    /*******************************************************************************
     *
     * The .ram_code section will contain the code That is run from RAM.
     * We are using this code to switch the clocks including envm clock.
     * This can not be done when running from envm
     * This will need to be copied to ram, before any of this code is run.
     *
     */
    .ram_code : ALIGN(0x10)
    {
        __sc_load = LOADADDR (.ram_code);
        __sc_start = .;
        *(.ram_codetext)        /* .ram_codetext sections (code) */
        *(.ram_codetext*)       /* .ram_codetext* sections (code)  */
        *(.ram_coderodata)      /* read-only data (constants) */
        *(.ram_coderodata*)
        . = ALIGN (0x10);
        __sc_end = .;
    } >switch_code

    /*******************************************************************************
     *
     * Short/global data section
     *
     */
    .sdata : ALIGN(0x40) /* short/global data section */
    {
        __sdata_load = LOADADDR(.sdata);
        __sdata_start = .;

        /*
         * offset used with gp(gloabl pointer) are +/- 12 bits, so set
         * point to middle of expected sdata range
         *
         * If sdata more than 4K, linker used direct addressing.
         * Perhaps we should add check/warning to linker script if sdata is > 4k
         */
        __global_pointer$ = . + 0x800;
        *(.sdata .sdata.* .gnu.linkonce.s.*)
        . = ALIGN(0x10);
        __sdata_end = .;
    } >l2zerodevice

    /*******************************************************************************
     *
     * (Explicitly) Initialized data section
     *
     */
#ifdef CONFIG_SERVICE_BOOT_USE_PAYLOAD 
    .data.payload : ALIGN(16) 
    {
      _payload_start = .;
      KEEP(boards/se301_som1soc/payload.o(.*))
      _payload_end = .;
    }
#endif

    .data : ALIGN(0x40)
    {
        __data_load = LOADADDR(.data);
        __data_start = .;
        *(.got.plt) *(.got)
        *(.shdata)
        *(.data .data.* .gnu.linkonce.d.*)
        . = ALIGN(0x10);
        __data_end = .;
    } >l2zerodevice

    /*******************************************************************************
     *
     * Uninitialized (zero-initialized) section
     */

    /*
     * Short zero-initialized section
     * The name BSS is an anacronym for "Block Started by Symbol" from a mid 1950s
     * assembly language for the IBM 704.
     *
     */
    .sbss : ALIGN(0x40)
    {
        __sbss_start = .;
        *(.sbss .sbss.* .gnu.linkonce.sb.*)
        *(.scommon)
        . = ALIGN(0x10);
        __sbss_end = .;
    } >l2zerodevice

    /*
     * General Zero-initialized section
     * The name BSS is an anacronym for "Block Started by Symbol" from a mid 1950s
     * assembly language for the IBM 704.
     */
    .bss : ALIGN(0x40)
    {
        __bss_start = .;
        *(.shbss)
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(0x10);
        __bss_end = .;
    } >l2zerodevice

    /*
     * Reserved space for Hart stacks
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

        __stack_top = .;
    } >l2zerodevice

    _end = .;
    PROVIDE(__l2_scratchpad_vma_end = .);

    /*
     * End of uninitialized data segment
     *
     *******************************************************************************/

    /*
    .heap : ALIGN(0x10)
    {
        __heap_start = .;
        . += HEAP_SIZE;
        __heap_end = .;
        . = ALIGN(0x10);
        _heap_end = __heap_end;
    } >dtim
    */
}
