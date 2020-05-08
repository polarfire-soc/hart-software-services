/*
 * GNU Linker Script for Hart Software Service (HSS)
 */

OUTPUT_ARCH( "riscv" )

MEMORY
{
  envm    (RX) :     ORIGIN = 0x20220000, LENGTH = 128K
  ddr     (RWX) :    ORIGIN = 0x80000000, LENGTH = 4096M
  e51dtim (RWX) :    ORIGIN = 0x01000000, LENGTH = 8K,
  switch_code (RX) : ORIGIN = 0x01001C00, LENGTH = 1K,
  l2lim   (RW) :     ORIGIN = 0x08000000, LENGTH = 128K
}


ENTRY(_start)


SECTIONS
{
  __envm_start  = ORIGIN(envm);
  __envm_end    = ORIGIN(envm) + LENGTH(envm);

  __l2lim_start = ORIGIN(l2lim);
  __l2lim_end   = ORIGIN(l2lim) + LENGTH(l2lim);

  __ddr_start   = ORIGIN(ddr);
  __ddr_end     = ORIGIN(ddr) + LENGTH(ddr);

  /*
   * Code and RO data live in eNVM (FLASH)
   */

  . = __envm_start;

  PROVIDE(_hss_start = .);
  PROVIDE(.code_start = .);
  .text.init         : ALIGN(8) { crt.o(.entry) }  > envm
  .text.init         : ALIGN(8) { crt.o(.text) } > envm
  .text              : ALIGN(8) { *(.text) } > envm
  .note.gnu.build-id : ALIGN(8) { *(.note.gnu.build-id) } > envm
  .rodata            : ALIGN(8) { *(.rodata) *(.rodata*) } > envm
  . =         ALIGN(8);  /* Pad to 8 bytes  */

  .gnu_build_id      : ALIGN(8) {
    PROVIDE(gnu_build_id = .);
    *(.note.gnu.build-id)
   }
 
  PROVIDE(_hss_end = .);
  PROVIDE(.code_end = .);

  /*
   * Initialized RW data lives in L2LIM, but is initialized from eNVM (FLASH)
   * at power up
   */
  . = __l2lim_start;
  .data : ALIGN(4) 
  { 
    _rwdata_exec_start = .;
    *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2) *(.srodata*)
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    *(.data) . = ALIGN(4);
    _rwdata_exec_end = .;
  } >l2lim AT >envm

  _rwdata_load = LOADADDR(.data);

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
  } >switch_code AT>envm

  /*
   * Uninitialized / Zero-Init data lives in L2LIM
   */

  PROVIDE(_hss_l2lim_start = .);
  PROVIDE(_bss_start = .);
  sbss : {
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
    . = . + 0x14000;
    _gp = . ;
  } > l2lim
  .bss : { *(.bss) } > l2lim

  /* thread-local data segment */
  .tdata :
  {
    _tls_data = .;
    crt.o(.tdata.begin)
    *(.tdata)
    crt.o(.tdata.end)
  } > l2lim

  .tbss :
  {
    *(.tbss)
    crt.o(.tbss.end)
  } > l2lim

  /* End of uninitalized data segements */
  PROVIDE(_bss_end = .);
  PROVIDE(_hss_l2lim_end = .);
  PROVIDE(_end = .);
}
