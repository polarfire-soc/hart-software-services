/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * file name : pse_emulation-ram.ld
 * fpr use with G5SoC emulation board
 *
 * SVN $Revision: 9187 $
 * SVN $Date: 2017-05-13 13:31:28 +0530 (Sat, 13 May 2017) $
 */
 
OUTPUT_ARCH( "riscv" )
ENTRY(_start)

/*----------------------------------------------------------------------*/
/* Sections:                                                             */
/* On Unleashed board
/* 0x08000000   LIM on startup- can be used for BM debugging.
/* 0x80000000    DDR  Can be used for begussing assuming has already be configured */
/* On G5 SoC emulation platform we have 8k of DTIM    */
/* ram (rwx) : ORIGIN  = 0x01000000, LENGTH = 8k      */
/* we also have 2M of L2 cache, which is by default configured as LIM */
/* also L2 Cache as Loosely Integrated Memory (0x08000000 0x09FFFFFF RWX L2-LIM ). */
/* ram (rwx) : ORIGIN = 0x08000000, LENGTH = 1920k */ 
/* */
/* As the memory being used for initial programming is E51 DTIM which is fixed inside CoreComplex RTL we are limited to the size of 8KB. However we can */
/* also use DDRC Cached area after DDR Calibration is complete in initial program. I think this may be tricky to implement  in order to load entire application */
/* from within SoftConsole as first user has to load the current UART driver program which also has DDR Calibration. And once DDR is calibrated then  user may */
/* be able to use cached region. Alternate would be to use L2 Cache as Loosely Integrated Memory (0x08000000 0x09FFFFFF RWX L2-LIM ). */
/*----------------------------------------------------------------------*/

/* Trying 0x80000000 G5 SoC Emulation Platform DDR instead of internal LIM at 0x08000000 */

MEMORY
{
    ram (rwx) : ORIGIN  = 0x08000000, LENGTH = 1920k
    ddr (rwx) : ORIGIN  = 0x0C0000000, LENGTH = 1920k
}

RAM_START_ADDRESS   = 0x08000000;       /* Must be the same value MEMORY region ram ORIGIN above. */
RAM_SIZE            = 1920k;              /* Must be the same value MEMORY region ram LENGTH above. */
DDR_START_ADDRESS   = 0x0C0000000;       /* Must be the same value MEMORY region ram ORIGIN above. */
DDR_SIZE            = 1920k;              /* Must be the same value MEMORY region ram LENGTH above. */
STACK_SIZE          = 80k;               /* needs to be calculated for your application */             
HEAP_SIZE           = 512k;               /* needs to be calculated for your application */

SECTIONS
{

/* text: test code section */
  . = 0x08000000;
  .text : ALIGN(0x10)
  {
     *(.text.init)
/*  *entry.o(.text); */
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
    
  } > ram

  /* short/global data section */
  .sdata : ALIGN(0x10)
  {
    __sdata_load = LOADADDR(.sdata);
    __sdata_start = .; 
    _gp = . + 0x800;
    *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2)
    *(.srodata*)
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    . = ALIGN(0x10);
    __sdata_end = .;
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
  
  .stack : ALIGN(0x1000)
  {
    __stack_bottom = .;
    . += STACK_SIZE;
    __stack_top = .;
  } > ram
}

