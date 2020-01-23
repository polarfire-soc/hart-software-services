/*
 * GNU Linker Script for Hart Software Service (HSS)
 */

OUTPUT_ARCH( "riscv" )

MEMORY
{
  envm    (RX) : ORIGIN = 0x20220000, LENGTH = 128K
  ddr     (RW) : ORIGIN = 0x80000000, LENGTH = 4096M
  e51dtim (RW) : ORIGIN = 0x01000000, LENGTH = 8K,
  l2lim   (RW) : ORIGIN = 0x08000000, LENGTH = 128K
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

  .code_start = .;
  .text.init : ALIGN(8) { crt.o(.text) } > envm
  .text      : ALIGN(8) { *(.text) } > envm
  .rodata    : ALIGN(8) { *(.rodata) *(.rodata*); } > envm
   . =         ALIGN(8);  /* Pad to 8 bytes  */
 
  .code_end = .;

  /*
   * Initialized RW data lives in L2LIM, but is initialized from eNVM (FLASH)
   * at power up
   */
  . = __l2lim_start;
  .data : ALIGN(4) 
  { 
    _rwdata_exec_start = .;
    _gp = . + 0x14000;
    *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2) *(.srodata*)
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    *(.data) . = ALIGN(4);
    _rwdata_exec_end = .;
  } >l2lim AT >envm

   _rwdata_load = LOADADDR(.data);


  /*
   * Uninitialized / Zero-Init data lives in L2LIM
   */

   _bss_start = .;
  .sbss : {
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
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
  _bss_end = .;
  _end = .;
}
