/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */
/*******************************************************************************
 *
 * file name : mpfs_lim.ld
 * Used when debugging code. The debugger loads the code to LIM.
 *
 * You can find details on the PolarFireSoC Memory map in the mpfs-memory-hierarchy.md
 * which can be found under the link below:
 * https://github.com/polarfire-soc/polarfire-soc-documentation
 *
 */

OUTPUT_ARCH( "riscv" )
ENTRY(_start)

/*-----------------------------------------------------------------------------

-- MSS hart Reset vector

The MSS reset vector for each hart is stored securely in the MPFS.
The most common usage will be where the reset vector for each hart will be set
to the start of the envm at address 0x2022_0100, giving 128K-256B of contiguous
non-volatile storage. Normally this is where the initial boot-loader will
reside. (Note: The first 256B page of envm is used for metadata associated with
secure boot. When not using secure boot (mode 0,1), this area is still reserved
by convention. It allows easier transition from non-secure to secure boot flow
during the development process.
When debugging a bare metal program that is run out of reset from envm, a linker
script will be used whereby the program will run from LIM instead of envm.
In this case, the reset vector in the linker script is normally set to the
start of LIM, 0x0800_0000.
This means you are not continually programming the envm each time you load a
program and there is no limitation with break points when debugging.
See the mpfs-lim.ld example linker script when runing from LIM.


------------------------------------------------------------------------------*/

MEMORY
{
    dtim (rwx) : ORIGIN = 0x01000000, LENGTH = 8k
    e51_itim (rwx) : ORIGIN = 0x01800000, LENGTH = 28k
    scratchpad(rwx)  : ORIGIN = 0x0A000000, LENGTH = 256k

    l2lim (rwx) : ORIGIN = 0x08000000, LENGTH = 512k
    l2zerodevice (rwx) : ORIGIN = 0x0A000000, LENGTH = 512k
}

HEAP_SIZE           = 0k;               /* needs to be calculated for your application if using */
PROVIDE(STACK_SIZE_PER_HART = 4k);

PROVIDE(__sc_load = 0);
PROVIDE(__sc_start = 0);
PROVIDE(__sc_end = 0);

/* STACK_SIZE_PER_HART needs to be calculated for your */
/* application. Must be aligned */
/* Also Thread local storage (AKA hart local storage) allocated for each hart */
/* as part of the stack
/* So memory map will look like once apportion in startup code: */
/*   */
/* stack hart0  Actual Stack size = (STACK_SIZE_PER_HART - HLS_DEBUG_AREA_SIZE) */
/* TLS hart 0   */
/* stack hart1  */
/* TLS hart 1   */
/* etc */
/* note: HLS_DEBUG_AREA_SIZE is defined in mss_sw_config.h */
/* STACK_SIZE_PER_HART = 8k; */

/*
 * There is common area for shared variables, accessed from a pointer in a harts HLS
 */
SIZE_OF_COMMON_HART_MEM = 128;

/*
 * Stack size for each hart's application.
 * These are the stack sizes that will be allocated to each hart before starting
 * each hart's application function, e51(), u54_1(), u54_2(), u54_3(), u54_4().
 */
STACK_SIZE_E51_APPLICATION = 2k;


SECTIONS
{
    PROVIDE(__dtim_start = ORIGIN(dtim));
    PROVIDE(__dtim_end = ORIGIN(dtim) + LENGTH(dtim));
    PROVIDE(__e51itim_start = ORIGIN(e51_itim));
    PROVIDE(__e51itim_end = ORIGIN(e51_itim) + LENGTH(e51_itim));

    PROVIDE(__l2lim_start = ORIGIN(l2lim));
    PROVIDE(__l2lim_end = ORIGIN(l2lim) + LENGTH(l2lim));
    PROVIDE(__l2_start = ORIGIN(l2zerodevice));
    PROVIDE(__l2_end = ORIGIN(l2zerodevice) + LENGTH(l2zerodevice));

    /* text: text code section */

    .text : ALIGN(0x10)
    {
        __text_load = LOADADDR(.text);
        __text_start = .;
        *(.entry)
         *(.text.init)
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
        *(.sdata2 .sdata2.* .gnu.linkonce.s2.*)
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

        *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2)
        *(.srodata*)

        . = ALIGN(0x10);
        __text_end = .;
        . = ALIGN(0x10);
    } > e51_itim

    .l2_scratchpad : ALIGN(0x10)
    {
        . = ALIGN (0x10);
        __l2_scratchpad_load = LOADADDR(.l2_scratchpad);
        __l2_scratchpad_start = .;
        __l2_scratchpad_vma_start = .;
        *(.l2_scratchpad)
        . = ALIGN(0x10);
        __l2_scratchpad_end = .;
        __l2_scratchpad_vma_end = .;
    } >scratchpad AT> e51_itim

    /* short/global data section */
    .sdata : ALIGN(0x10)
    {
        __sdata_load = LOADADDR(.sdata);
        __sdata_start = .;
        /* offset used with gp(gloabl pointer) are +/- 12 bits, so set point to middle of expected sdata range */
        /* If sdata more than 4K, linker used direct addressing. Perhaps we should add check/warning to linker script if sdata is > 4k */
        __global_pointer$ = . + 0x800;
        *(.sdata .sdata.* .gnu.linkonce.s.*)
        . = ALIGN(0x10);
        __sdata_end = .;
    } > dtim AT>e51_itim

    /* data section */
    .data : ALIGN(0x10)
    {
        __data_load = LOADADDR(.data);
        __data_start = .;
        *(.got.plt) *(.got)
        *(.shdata)
        *(.data .data.* .gnu.linkonce.d.*)
        . = ALIGN(0x10);
        __data_end = .;
    } > dtim AT>e51_itim

    /* sbss section */
    .sbss : ALIGN(0x10)
    {
        __sbss_start = .;
        *(.sbss .sbss.* .gnu.linkonce.sb.*)
        *(.scommon)
        . = ALIGN(0x10);
        __sbss_end = .;
    } > dtim

    /* sbss section */
    .bss : ALIGN(0x10)
    {
        __bss_start = .;
        *(.shbss)
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(0x10);
        __bss_end = .;
    } > dtim

    /* End of uninitialized data segment */
    _end = .;

    .heap : ALIGN(0x10)
    {
        __heap_start = .;
        . += HEAP_SIZE;
        __heap_end = .;
        . = ALIGN(0x10);
        _heap_end = __heap_end;
    } > dtim

    /* must be on 4k boundary- corresponds to page size */
    .stack : ALIGN(0x1000)
    {
        __stack_bottom = .;

        PROVIDE(__stack_bottom_h0$ = .);
        PROVIDE(__app_stack_bottom_h0 = .);
        . += STACK_SIZE_E51_APPLICATION;
        PROVIDE(__app_stack_top_h0 = .);
        PROVIDE(__stack_top_h0$ = .);

    } > dtim

    /*
     * memory shared accross harts.
     * The boot Hart Local Storage holds a pointer to this area for each hart if
     * when enabled by setting MPFS_HAL_SHARED_MEM_ENABLED define in the
     * mss_sw_config.h
     */
    .app_hart_common : /* ALIGN(0x1000) */
    {
        PROVIDE(__app_hart_common_start = .);
        . += SIZE_OF_COMMON_HART_MEM;
        PROVIDE(__app_hart_common_end = .);
    } > dtim

    /*
     * These are unused it the bootloader program but need to be preset for
     * compilation, as used in non-bootloader function.
     */
    .unused_non_bootloader : ALIGN(0x10)
    {
        PROVIDE(__uninit_bottom$ = .);
        PROVIDE(__uninit_top_h$ = .);
        PROVIDE(__app_stack_bottom = .);
        PROVIDE(__app_stack_top = .);
        PROVIDE(__uninit_top_h$ = .);
    } > dtim

    __dtim_trampoline_end = .;
}
