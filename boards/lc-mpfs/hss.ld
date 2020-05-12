/*
 * GNU Linker Script for Hart Software Service (HSS)
 */

OUTPUT_ARCH( "riscv" )

MEMORY
{
  ddr     (RW) : ORIGIN = 0x80000000, LENGTH = 8192M
}


ENTRY(_start)


SECTIONS
{
  _ddr_start = ORIGIN(ddr);
  _ddr_end   = ORIGIN(ddr) + LENGTH(ddr);

  . = _ddr_start;

   PROVIDE(_hss_start = .);

  .text.init         : ALIGN(8) { crt.o(.entry) } 
  .text.init         : ALIGN(8) { crt.o(.text) } 
  .text              : ALIGN(8) { *(.text) } 
  .rodata            : ALIGN(8) { *(.rodata) *(.rodata*) } 

  .gnu_build_id      : ALIGN(8) {
    PROVIDE(gnu_build_id = .);
    *(.note.gnu.build-id)
   }
 
  .sbss : {
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
  } 
  .bss : { *(.bss) } 

  /* thread-local data segment */
  .tdata :
  {
    _tls_data = .;
    crt.o(.tdata.begin)
    *(.tdata)
    crt.o(.tdata.end)
  } 

  .tbss :
  {
    *(.tbss)
    crt.o(.tbss.end)
  }

  .data.payload : ALIGN(8) 
  {
    PROVIDE(_payload_start = .);
    boards/lc-mpfs/payload.o(.data) 
    PROVIDE(_payload_end = .);
  }

  .data : ALIGN(8) 
  { 
    *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2) *(.srodata*)
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    *(.data) . = ALIGN(8);

    _gp = . ;
    PROVIDE(_end = .);
  }

  PROVIDE(_hss_end = .);
}
