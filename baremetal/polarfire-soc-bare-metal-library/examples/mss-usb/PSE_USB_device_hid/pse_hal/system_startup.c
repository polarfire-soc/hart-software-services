/***************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * first C code called on startup. Will call user code created outside the HAL.
 *
 * SVN $Revision: 9661 $
 * SVN $Date: 2018-01-15 10:43:33 +0000 (Mon, 15 Jan 2018) $
 */
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pse_hal.h"

/*------------------------------------------------------------------------------
 * Symbols from the linker script used to locate the text, data and bss sections.
 */

extern unsigned int __text_load;
extern unsigned int __text_start;
extern unsigned int _etext;
extern unsigned int _uPROM_start;
extern unsigned int _uPROM_end;

extern unsigned int __data_load;
extern unsigned int __data_start;
extern unsigned int _edata;

extern unsigned int __sbss_start;
extern unsigned int __sbss_end;

extern unsigned int __bss_start;
extern unsigned int __bss_end;

extern unsigned int __sc_load;
extern unsigned int __sc_start;
extern unsigned int __sc_end;

extern unsigned int __vector_table_load;
extern unsigned int _vector_table_end_load;


/*
 * Function Declarations
 */
void e51(void);
void u54_1(void);
void u54_2(void);
void u54_3(void);
void u54_4(void);
void init_memory( void);


/*==============================================================================
 * E51 startup.
 * If you need to modify this function, create your own one if a user directory space
 * e.g. /hart0/e51.c
 */
__attribute__((weak)) int main_first_hart(void)
{
  volatile int i;

	/* mscratch must be init to zero- On the unleashed issue occurs in trap handling code if not init. */
	write_csr(mscratch, 0);
	write_csr(mcause, 0);
	write_csr(mepc, 0);

  uint32_t hartid = read_csr(mhartid);
  if(hartid == 0)
  {
	  init_memory();
#if PSE
        SYSREG->SUBBLK_CLOCK_CR = 0xffffffff;       /* all clocks on */
#endif
        e51();
  }

   /* should never get here */
	while(1)
	{
		/* add code here */
		i++;				/* added some code as debugger hangs if in loop doing nothing */
		if(i == 0x1000)
			i = 0;
	}

  return 0;
}

/*==============================================================================
 * U54s startup.
 * This is called from entry.S
 * If you need to modify this function, create your own one if a user directory space
 */
__attribute__((weak)) int main_other_hart(void)
{

  uint32_t hartid = read_csr(mhartid);

  /* mscratch must be init to zero- On the unleashed issue occurs in trap handling code if not init. */
    write_csr(mscratch, 0);
    write_csr(mcause, 0);
    write_csr(mepc, 0);

  switch(hartid)
  {
    case 1:
      u54_1();
      break;

    case 2:
      u54_2();
      break;

    case 3:
      u54_3();
      break;

    case 4:
      u54_4();
      break;

    default:

      break;
  }

  	 /* should never get here */
	volatile uint32_t i;
	while(1)
	{
		/* add code here */
		i++;				/* added some code as debugger hangs if in loop doing nothing */
		if(i == 0x1000)
			i = 0;
	}

  return 0;

}

/*==============================================================================
 * E51 code executing after system startup.
 * If you need to modify this function, create your own one if a user directory space
 */
 __attribute__((weak)) void e51(void)
 {
	uint32_t hartid = read_csr(mhartid);
	volatile uint32_t i;

	while(1)
	{
		/* add code here */
		i++;				/* added some code as debugger hangs if in loop doing nothing */
		if(i == 0x1000)
			i = 0;
	}
};

 /*==============================================================================
  * First U54.
  * If you need to modify this function, create your own one if a user directory space
  */
 __attribute__((weak)) void u54_1(void)
 {
 	uint32_t hartid = read_csr(mhartid);
 	volatile uint32_t i;

 	while(1)
 	{
 		/* add code here */
 		i++;				/* added some code as debugger hangs if in loop doing nothing */
 		if(i == 0x1000)
 			i = 0;
 	}
 };


/*==============================================================================
 * Second U54.
 * If you need to modify this function, create your own one if a user directory space
 */
__attribute__((weak)) void u54_2(void)
{
	uint32_t hartid = read_csr(mhartid);
	volatile uint32_t i;

	while(1)
	{
		/* add code here */
		i++;				/* added some code as debugger hangs if in loop doing nothing */
		if(i == 0x1000)
			i = 0;
	}
};


/*==============================================================================
 * Third U54.
 * If you need to modify this function, create your own one if a user directory space
 */
 __attribute__((weak)) void u54_3(void)
 {
	uint32_t hartid = read_csr(mhartid);
	volatile uint32_t i;

	while(1)
	{
		/* add code here */
		i++;				/* added some code as debugger hangs if in loop doing nothing */
		if(i == 0x1000)
			i = 0;
	}
};


/*==============================================================================
 * Fourth U54.
 * If you need to modify this function, create your own one if a user directory space
 */
 __attribute__((weak)) void u54_4(void)
 {
	uint32_t hartid = read_csr(mhartid);
	volatile uint32_t i;

	while(1)
	{
		/* add code here */
		i++;				/* added some code as debugger hangs if in loop doing nothing */
		if(i == 0x1000)
			i = 0;
	}
};


 /*------------------------------------------------------------------------------
  * _start() function called invoked
  * This function is called from  startup_cortexm1.S on power up and warm reset.
  */
 __attribute__((weak)) void init_memory( void)
 {
#if 0 /* todo: show example, not currently required */
     /*
      * Copy text section if required (copy executable from LMA to VMA).
      */
     {
         unsigned int * text_lma = &__text_load;
         unsigned int * end_text_vma = &_etext;
         unsigned int * text_vma = &__text_start;

         if ( text_vma != text_lma)
         {
             while ( text_vma <= end_text_vma)
             {
                 *text_vma++ = *text_lma++;
             }
         }
     }
#endif
#if 0 /* todo: show example, not currently required */
     /*
      * Copy data section if required (initialized variables).
      */
     {
         unsigned int * data_lma = &__data_load;
         unsigned int * end_data_vma = &_edata;
         unsigned int * data_vma = &__data_start;

         if ( data_vma != data_lma )
         {
             while ( data_vma <= end_data_vma )
             {
                 *data_vma++ = *data_lma++;
             }
         }
     }
#endif
     /*
      * Zero out the bss section (set non-initialized variables to 0).
      */
     {
         unsigned int * bss = &__sbss_start;
         unsigned int * bss_end = &__sbss_end;

         if ( bss_end > bss)
         {
             while ( bss <= bss_end )
             {
                 *bss++ = 0;
             }
         }

    	 bss = &__bss_start;
    	 bss_end = &__bss_end;

         if ( bss_end > bss)
         {
             while ( bss <= bss_end )
             {
                 *bss++ = 0;
             }
         }
     }

 }




