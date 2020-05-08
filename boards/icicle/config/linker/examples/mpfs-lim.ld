/*******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */
/*******************************************************************************
 * 
 * file name : mpfs_lim.ld
 * Code located in LIM. Used when debugging start-up code.
 */
 
OUTPUT_ARCH( "riscv" )
ENTRY(_start)


/*-----------------------------------------------------------------------------

  -- PolarFire SoC Memorp map


     0x0800_0000  +-----------------------+
                  |   DTIM  Len=8K        |
                  +-----------------------+

     0x0180_0000  +-----------------------+
                  |   ITIM Hart 0         |
                  +-----------------------+
                                                        eNVM detail
     0x0180_8000  +-----------------------+        +-------------------+   -
                  |   ITIM Hart 1 Len=28K |        |Sector 2           |   ^
                  +-----------------------+        |0x2022_0000 8K     |   |
                                                   +-------------------+   |
     0x0181_0000  +-----------------------+        |Sector 0           |   1
                  |   ITIM Hart 2 Len=28K |        |0x2022_2000 56K    |   2
                  +-----------------------+        +-------------------+   8
                                                   |Sector 1           |   k
     0x0181_8000  +-----------------------+        |0x2022_3000 56K    |   |
                  |   ITIM Hart 3 Len=28K |        +-------------------+   |
                  +-----------------------+        |Sector 3           |   |
                                                   |0x2022_3E00 8K     |   v
     0x0182_0000  +-----------------------+        +-------------------+   -
                  |   ITIM Hart 4 Len=28K |
                  +-----------------------+

     0x0800_0000  +-----------------------+
                  |   LIM  Len max=1920K  |  +------>+------+
                  +-----------------------+  |       |      |
                                             |       |      |
     0x2022_0000  +-----------------------+  |       |      |
                  |   eNVM 128K           |  |       |      |
                  +-----------------------+  |       |      |
                                             |       |      |
     0x8000_0000  +-----------------------+--+       |  DDR |
  0x10_0000_0000  | DDR cached            |          |      |
                  |   SEG0                |     +--->|      |
                  +-----------------------+     |    |      |
                                                |    |      |
     0xC000_0000  +-----------------------+-----+    |      |
  0x14_0000_0000  | DDR non-cached        |          |      |
                  |    SEG1               |          |      |
                  +-----------------------+      +-->+      |
                                                 |   |      |
     0xD000_0000  +-----------------------+------+   |      |
  0x18_0000_0000  | Write Combine buffer  |          +------+
                  |    SEG1               |
                  +-----------------------+
  ----------------------------------------------------------------------------*/
  
/*-----------------------------------------------------------------------------

-- MSS hart Reset vector

The MSS reset vector for each hart is configured by Libero and stored securely
in the MPFS.
The most common usage will be where the reset vector for each hart will be set
to the start of the eNVM at address 0x2022_0000, giving 128K of contiguous
non-volatile storage. Normally this is where the initial boot-loader will 
reside.
Libero outputs the configured reset vector address to the xml file, see 
LIBERO_SETTING_RESET_VECTOR_HART0 etc in <hw_memory.h>
See mpfs_envm.ld example linker script when running from eNVM.


When debugging a bare metal program that is run out of reset from eNVM, a linker 
script will be used whereby the program will run from LIM instead of eNVM.
In this case, set the reset vector in the linker script to 0x0800_0000.
This means you are not continually programming the eNVM each time you load a 
program and there is no limitation with hardware break points whn debugging.
See mpfs-lim.ld example linker script when runing from LIM.

------------------------------------------------------------------------------*/

MEMORY
{
    /* eNVM can be made into 128K section or split as required */
    /* In this example, our reset vector is set to point to the */
    /* start of SEC0 at  0x20222000. The initial 8K is reserved for data 
    /* storage in this example */
    eNVM_SEC2 (rx) : ORIGIN  = 0x20220000, LENGTH = 8k
    eNVM_SEC0_1_3 (rx) : ORIGIN  = 0x20222000, LENGTH = 120k
    /* LIM - we place code here in this example */
    ram (rwx) :     ORIGIN  = 0x08000000, LENGTH = 1920k
    ram_DTIM (rwx) : ORIGIN  = 0x01000000, LENGTH = 7k
    scratchpad(rwx):  ORIGIN = 0x0A000000, LENGTH = 512k
    /* This 1k of DTIM is used to run code when switching the eNVM clock */
    switch_code (rx) : ORIGIN = 0x01001c00, LENGTH = 1k     
}

HEAP_SIZE           = 8k;               /* needs to be calculated for your application if using */

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
/* STACK_SIZE_PER_HART = 1k; */                     

/*
 * Stack size for each hart's application.
 * These are the stack sizes that will be allocated to each hart before starting
 * each hart's application function, e51(), u54_1(), u54_2(), u54_3(), u54_4().
 */
STACK_SIZE_E51_APPLICATION = 1k;
STACK_SIZE_U54_1_APPLICATION = 1k;
STACK_SIZE_U54_2_APPLICATION = 1k;
STACK_SIZE_U54_3_APPLICATION = 1k;
STACK_SIZE_U54_4_APPLICATION = 1k;

SECTIONS
{

/* text: test code section */
  . = 0x08000000;
  .text : ALIGN(0x10)
  {
    __text_load = LOADADDR(.text);
    __text_start = .; 
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
    __text_end = .;
  } > ram
 
  .l2_scratchpad : ALIGN(0x10)
  { 
    __l2_scratchpad_load = LOADADDR(.l2_scratchpad);
    __l2_scratchpad_start = .;
    __l2_scratchpad_vma_start = .; 
    *(.l2_scratchpad)
    . = ALIGN(0x10);
    __l2_scratchpad_end = .;
    __l2_scratchpad_vma_end = .;
  } >scratchpad AT> ram
  
  /* 
   *   The .ram_code section will contain the code That is run from RAM.
   *   We are using this code to switch the clocks including eNVM clock.
   *   This can not be done when running from eNVM
   *   This will need to be copied to ram, before any of this code is run.
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
  } >switch_code AT> ram     /* On the MPFS for startup code use,  >switch_code AT>eNVM */

  /* short/global data section */
  .sdata : ALIGN(0x10)
  {
    __sdata_load = LOADADDR(.sdata);
    __sdata_start = .; 
    /* offset used with gp(gloabl pointer) are +/- 12 bits, so set point to middle of expected sdata range */
    /* If sdata more than 4K, linker used direct addressing. Perhaps we should add check/warning to linker script if sdata is > 4k */
    __global_pointer$ = . + 0x800;
    *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2)
    *(.srodata*)
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    . = ALIGN(0x10);
    __sdata_end = .;
  } > ram

  /*
   * The s2data section is required when using cetrtain versions of newlib-nano
   * The _global_impure_ptr used in that libary is initialised to point here.
   */
  .sdata2 : ALIGN(0x10)
  {
      *(.sdata2 .sdata2.* .gnu.linkonce.s2.*)
  } > ram

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
  } > ram

  /* sbss section */
  .sbss : ALIGN(0x10)
  {
    __sbss_start = .;
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
    . = ALIGN(0x10);
    __sbss_end = .;
  } > ram
  
  /* sbss section */
  .bss : ALIGN(0x10)
  { 
    __bss_start = .;
    *(.shbss)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN(0x10);
    __bss_end = .;
  } > ram

  /* End of uninitialized data segment */
  _end = .;
  
  .heap : ALIGN(0x10)
  {
    __heap_start = .;
    . += HEAP_SIZE;
    __heap_end = .;
    . = ALIGN(0x10);
    _heap_end = __heap_end;
  } > ram
  
  /* must be on 4k boundary- corresponds to page size */
  .stack : ALIGN(0x1000)
  {
    PROVIDE(__stack_bottom_h0$ = .);
    PROVIDE(__app_stack_bottom_h0 = .);
    . += STACK_SIZE_E51_APPLICATION;
    PROVIDE(__app_stack_top_h0 = .);
    PROVIDE(__stack_top_h0$ = .);
    
    PROVIDE(__stack_bottom_h1$ = .);
    PROVIDE(__app_stack_bottom_h1$ = .);
    . += STACK_SIZE_U54_1_APPLICATION;
    PROVIDE(__app_stack_top_h1 = .);
    PROVIDE(__stack_top_h1$ = .);
    
    PROVIDE(__stack_bottom_h2$ = .);
    PROVIDE(__app_stack_bottom_h2 = .);
    . += STACK_SIZE_U54_2_APPLICATION;
    PROVIDE(__app_stack_top_h2 = .);
    PROVIDE(__stack_top_h2$ = .);
    
    PROVIDE(__stack_bottom_h3$ = .);
    PROVIDE(__app_stack_bottom_h3 = .);
    . += STACK_SIZE_U54_3_APPLICATION;
    PROVIDE(__app_stack_top_h3 = .);
    PROVIDE(__stack_top_h3$ = .);
    
    PROVIDE(__stack_bottom_h4$ = .);
    PROVIDE(__app_stack_bottom_h4 = .);
    . += STACK_SIZE_U54_4_APPLICATION;
    PROVIDE(__app_stack_top_h4 = .);
    PROVIDE(__stack_top_h4$ = .);
    
  } > ram
}

