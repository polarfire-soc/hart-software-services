OUTPUT_ARCH( "riscv" )
ENTRY(_start)
MEMORY
{
    envm (rx) : ORIGIN = 0x20220100, LENGTH = (128k-256)
    dtim (rwx) : ORIGIN = 0x01000000, LENGTH = 7k
    switch_code (rx) : ORIGIN = 0x01001c00, LENGTH = 1k
    e51_itim (rwx) : ORIGIN = 0x01800000, LENGTH = 28k
    u54_1_itim (rwx) : ORIGIN = 0x01808000, LENGTH = 28k
    u54_2_itim (rwx) : ORIGIN = 0x01810000, LENGTH = 28k
    u54_3_itim (rwx) : ORIGIN = 0x01818000, LENGTH = 28k
    u54_4_itim (rwx) : ORIGIN = 0x01820000, LENGTH = 28k
    l2lim (rwx) : ORIGIN = 0x08000000, LENGTH = 512k
    l2zerodevice (rwx) : ORIGIN = 0x0A000000, LENGTH = 512k
    ddr (rwx) : ORIGIN = 0x80000000, LENGTH = 32m
    ddrhi (rwx) : ORIGIN = 0x1000000000, LENGTH = 1888m
    ncddrhi (rwx) : ORIGIN = 0x1400000000, LENGTH = 2048m
}
PROVIDE(HEAP_SIZE = 0k);
PROVIDE(STACK_SIZE_PER_HART = 16k);
SECTIONS
{
    PROVIDE(__envm_start = ORIGIN(envm));
    PROVIDE(__envm_end = ORIGIN(envm) + LENGTH(envm));
    PROVIDE(__l2lim_start = ORIGIN(l2lim));
    PROVIDE(__l2lim_end = ORIGIN(l2lim) + LENGTH(l2lim));
    PROVIDE(__l2_start = ORIGIN(l2zerodevice));
    PROVIDE(__l2_end = ORIGIN(l2zerodevice) + LENGTH(l2zerodevice));
    PROVIDE(__ddr_start = ORIGIN(ddr));
    PROVIDE(__ddr_end = ORIGIN(ddr) + LENGTH(ddr));
    PROVIDE(__ddrhi_start = ORIGIN(ddrhi));
    PROVIDE(__ddrhi_end = ORIGIN(ddrhi) + LENGTH(ddrhi));
    PROVIDE(__ddrhi_max_end = ORIGIN(ddrhi) + LENGTH(ncddrhi));
    PROVIDE(__ncddrhi_start = ORIGIN(ncddrhi));
    PROVIDE(__ncddrhi_end = ORIGIN(ncddrhi) + LENGTH(ncddrhi));
    PROVIDE(__dtim_start = ORIGIN(dtim));
    PROVIDE(__dtim_end = ORIGIN(dtim) + LENGTH(dtim));
    PROVIDE(__e51itim_start = ORIGIN(e51_itim));
    PROVIDE(__e51itim_end = ORIGIN(e51_itim) + LENGTH(e51_itim));
    PROVIDE(__u54_1_itim_start = ORIGIN(u54_1_itim));
    PROVIDE(__u54_1_itim_end = ORIGIN(u54_1_itim) + LENGTH(u54_1_itim));
    PROVIDE(__u54_2_itim_start = ORIGIN(u54_2_itim));
    PROVIDE(__u54_2_itim_end = ORIGIN(u54_2_itim) + LENGTH(u54_2_itim));
    PROVIDE(__u54_3_itim_start = ORIGIN(u54_3_itim));
    PROVIDE(__u54_3_itim_end = ORIGIN(u54_3_itim) + LENGTH(u54_3_itim));
    PROVIDE(__u54_4_itim_start = ORIGIN(u54_4_itim));
    PROVIDE(__u54_4_itim_end = ORIGIN(u54_4_itim) + LENGTH(u54_4_itim));
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
    .ram_code : ALIGN(0x10)
    {
        __sc_load = LOADADDR (.ram_code);
        __sc_start = .;
        *(.ram_codetext)
        *(.ram_codetext*)
        *(.ram_coderodata)
        *(.ram_coderodata*)
        . = ALIGN (0x10);
        __sc_end = .;
    } >switch_code
    .sdata : ALIGN(0x40)
    {
        __sdata_load = LOADADDR(.sdata);
        __sdata_start = .;
        __global_pointer$ = . + 0x800;
        *(.sdata .sdata.* .gnu.linkonce.s.*)
        . = ALIGN(0x10);
        __sdata_end = .;
    } >l2zerodevice
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
    .sbss : ALIGN(0x40)
    {
        __sbss_start = .;
        *(.sbss .sbss.* .gnu.linkonce.sb.*)
        *(.scommon)
        . = ALIGN(0x10);
        __sbss_end = .;
    } >l2zerodevice
    .bss : ALIGN(0x40)
    {
        __bss_start = .;
        *(.shbss)
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(0x10);
        __bss_end = .;
    } >l2zerodevice
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
}
