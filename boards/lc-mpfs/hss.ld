/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * GNU linker script for Hart Software Services (HSS)
 *
 */

OUTPUT_ARCH( "riscv" )


/*******************************************************************************
 *
 * -- MSS hart Reset vector
 *
 * The MSS reset vector for each hart is configured by Libero and stored securely
 * in the MPFS.
 *
 * The most common usage will be where the reset vector for each hart will be set
 * to the start of the envm at address 0x2022_0000, giving 128K of contiguous
 * non-volatile storage. Normally this is where the initial boot-loader will
 * reside.
 *
 * Libero outputs the configured reset vector address to the xml file, see
 * LIBERO_SETTING_RESET_VECTOR_HART0 etc in <hw_memory.h>
 *
 * When debugging a bare metal program that is run out of reset from envm, a linker
 * script will be used whereby the progdtim will run from LIM instead of envm.
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
  ddr  (rwx)         : ORIGIN = 0x80000000, LENGTH = 8192M
  dtim (rwx)         : ORIGIN = 0x01000000, LENGTH = 7k /* DTIM */
  switch_code (rx)   : ORIGIN = 0x01001c00, LENGTH = 1k /* This 1K of DTIM is used to run code
                                                           * when switching the envm clock */
}

PROVIDE(HEAP_SIZE = 0k);               /* needs to be calculated for your application */
PROVIDE(STACK_SIZE_PER_HART = 16k);

/*******************************************************************************
 *
 * Memory Sections and Placement
 */
SECTIONS
{
    PROVIDE(__ddr_start = ORIGIN(ddr));
    PROVIDE(__ddr_end   = ORIGIN(ddr) + LENGTH(ddr));

    PROVIDE(__dtim_start       = ORIGIN(dtim));
    PROVIDE(__dtim_end         = ORIGIN(dtim) + LENGTH(dtim));

    . = __ddr_start;

     PROVIDE(_hss_start = .);

    /*
     * Code and RO data lives in DDR
     */
    . = __ddr_start;

    PROVIDE(_hss_start = .);

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
        *(.sdata2.*)
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
    } 

    .gnu_build_id : ALIGN(8) {
        PROVIDE(gnu_build_id = .);
        *(.note.gnu.build-id)
    } 

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
    } 


    /*******************************************************************************
     *
     * The .ram_code section will contain the code That is run from RAM.
     * We are using this code to switch the clocks including envm clock.
     * This can not be done when running from envm
     * This will need to be copied to ram, before any of this code is run.
     *
     */
    .ram_code :
    {
        . = ALIGN (4);
        __sc_load = LOADADDR (.ram_code);
        __sc_start = .;
        *(.ram_codetext)        /* .ram_codetext sections (code) */
        *(.ram_codetext*)       /* .ram_codetext* sections (code)  */
        *(.ram_coderodata)      /* read-only data (constants) */
        *(.ram_coderodata*)
        . = ALIGN (4);
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
    } 

    /*******************************************************************************
     *
     * (Explicitly) Initialized data section
     *
     */
    .data.payload : ALIGN(8) 
    {
      _payload_start = .;
      KEEP(boards/mpfs/payload.o(.*))
      _payload_end = .;
    }

    .data : ALIGN(0x40)
    {
        __data_load = LOADADDR(.data);
        __data_start = .;
        *(.got.plt) *(.got)
        *(.shdata)
        *(.data .data.* .gnu.linkonce.d.*)
        . = ALIGN(0x10);
        __data_end = .;
    } 


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
    } 

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
    } 

    /*
     * Reserved space for Hart stacks
     */
    .stack : ALIGN(0x40)
    {
        PROVIDE(__stack_bottom = .);

        PROVIDE(__stack_bottom_h0$ = .);
        . += STACK_SIZE_PER_HART;
        PROVIDE(__stack_top_h0$ = .);

        PROVIDE(__stack_bottom_h1$ = .);
        . += STACK_SIZE_PER_HART;
        PROVIDE(__stack_top_h1$ = .);

        PROVIDE(__stack_bottom_h2$ = .);
        . += STACK_SIZE_PER_HART;
        PROVIDE(__stack_top_h2$ = .);

        PROVIDE(__stack_bottom_h3$ = .);
        . += STACK_SIZE_PER_HART;
        PROVIDE(__stack_top_h3$ = .);

        PROVIDE(__stack_bottom_h4$ = .);
        . += STACK_SIZE_PER_HART;
        PROVIDE(__stack_top_h4$ = .);

        PROVIDE(__stack_top = .);
    } 

    _end = .;

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
