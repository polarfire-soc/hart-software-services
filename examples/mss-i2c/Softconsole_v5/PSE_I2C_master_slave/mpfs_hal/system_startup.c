/***************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * first C code called on startup. Will call user code created outside the HAL.
 *
 * SVN $Revision: 11160 $
 * SVN $Date: 2019-04-26 12:04:57 +0100 (Fri, 26 Apr 2019) $
 */
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "mss_hal.h"
#include "../mpfs_config/user_config/mss_user_config.h"


/*------------------------------------------------------------------------------
 * Symbols from the linker script used to locate the text, data and bss sections.
 */

extern unsigned int __data_load;
extern unsigned int __data_start;
extern unsigned int __data_end;

extern unsigned int __sbss_start;
extern unsigned int __sbss_end;

extern unsigned int __bss_start;
extern unsigned int __bss_end;

extern unsigned int __sdata_load;
extern unsigned int __sdata_start;
extern unsigned int __sdata_end;

/*LDRA_INSPECTED 440 S MR:R.11.1,R.11.2,R.11.4,R.11.6,R.11.7  Have to allocate number (address) as point reference*/
mss_sysreg_t*   SYSREG = ((mss_sysreg_t*) BASE32_ADDR_MSS_SYSREG);

/*
 * Function Declarations
 */
void e51(void);
void u54_1(void);
void u54_2(void);
void u54_3(void);
void u54_4(void);
void init_memory( void);
int main_other_hart(void);
int main_first_hart(void);

/*==============================================================================
 * E51 startup.
 * If you need to modify this function, create your own one if a user directory space
 * e.g. /hart0/e51.c
 */
__attribute__((weak)) int main_first_hart(void)
{
  volatile int i;
    uint32_t hartid = read_csr(mhartid);
    if(hartid == MPFS_HAL_FIRST_HART)
    {
        int hard_idx;
        init_memory();

        /* Start the other harts- the are put in wfi in entry.S  */
        for(hard_idx = MPFS_HAL_FIRST_HART + 1; hard_idx <= MPFS_HAL_LAST_HART; hard_idx++)
        {
            CLINT->MSIP[hard_idx] = 1;
        }

        main_other_hart();
    }

    /* should never get here */
    while(1U)
    {
        i++;                /* code required as debugger hangs if just {while(1) ;} */
        if(i == 0x1000U)
        {
            i = 0U;
        }
    }

    return (0U);
}

/*==============================================================================
 * U54s startup.
 * This is called from entry.S
 * If you need to modify this function, create your own one in a user directory space
 *
 * Please note: harts 1 to 4 will wait in startup code in entry.S as they run the
 * wfi (wait for interrupt) instruction.
 * They are woken up as required by the the MPFS_HAL_FIRST_HART ( It triggers a software
 * interrupt on the particular hart to be woken up )
 *
 */
__attribute__((weak)) int main_other_hart(void)
{

  uint32_t hartid = read_csr(mhartid);

    switch(hartid)
    {

    case 0U:
        e51();
        break;

    case 1U:
        u54_1();
        break;

    case 2U:
        u54_2();
        break;

    case 3U:
        u54_3();
        break;

    case 4U:
        u54_4();
        break;

    default:
        /* no more harts */
        break;
    }

       /* should never get here */
    volatile uint32_t i = 0;
    while(i != 0x10001U)        /* stay here forever */
    {
        i++;                /* code required as debugger hangs if just {while(1) ;} */
        if(i == 0x1000U)
        {
            i = 0U;
        }
    }

  return ((int)0);

}

/*==============================================================================
 * E51 code executing after system startup.
 * If you need to modify this function, create your own one if a user directory space
 */
 __attribute__((weak)) void e51(void)
 {
    uint32_t hartid = read_csr(mhartid);
    volatile uint32_t   i = 0U;
    volatile uint32_t   test = 0U;

    clear_soft_interrupt();
    while(( test & MIP_MSIP) == 0U)
    {
        __asm("nop");
        __asm("nop");
        __asm("wfi");          /* wait for interprocess from hart0 to wake up */
        test = read_csr(mip);
    }

    while(i != 0x1001U)
    {
        /* add code here */
        i++;                /* added some code as debugger hangs if in loop doing nothing */
        if(i == 0x1000U)
        {
            i = 0U;
        }
    }
};

 /*==============================================================================
  * First U54.
  * If you need to modify this function, create your own one if a user directory space
  */
 __attribute__((weak)) void u54_1(void)
 {
     uint32_t hartid = read_csr(mhartid);
     volatile uint32_t   i = 0U;
     volatile uint32_t   test = 0U;

     clear_soft_interrupt();
     while(( test & MIP_MSIP) == 0U)
     {
         __asm("nop");
         __asm("nop");
         __asm("wfi");          /* wait for interprocess from hart0 to wake up */
         test = read_csr(mip);
     }

     while(i != 0x1001U)
     {
        i++;                /* added some code as debugger hangs if in loop doing nothing */
        if(i == 0x1000U)
        {
             i = 0U;
        }
     }
 };


/*==============================================================================
 * Second U54.
 * If you need to modify this function, create your own one if a user directory space
 */
__attribute__((weak)) void u54_2(void)
{
    uint32_t hartid = read_csr(mhartid);
    volatile uint32_t   i = 0U;
    volatile uint32_t   test = 0U;

    clear_soft_interrupt();
    while(( test & MIP_MSIP) == 0U)
    {
        __asm("nop");
        __asm("nop");
        __asm("wfi");          /* wait for interprocess from hart0 to wake up */
        test = read_csr(mip);
    }

    while(i != 0x1001U)
    {
        i++;                /* added some code as debugger hangs if in loop doing nothing */
        if(i == 0x1000U)
        {
            i = 0U;
        }
    }
}


/*==============================================================================
 * Third U54.
 * If you need to modify this function, create your own one if a user directory space
 */
 __attribute__((weak)) void u54_3(void)
 {
    uint32_t hartid = read_csr(mhartid);
    volatile uint32_t   i = 0U;
    volatile uint32_t   test = 0U;

    clear_soft_interrupt();
    while(( test & MIP_MSIP) == 0U)
    {
        __asm("nop");
        __asm("nop");
        __asm("wfi");          /* wait for interprocess from hart0 to wake up */
        test = read_csr(mip);
    }

    while(i != 0x1001U)
    {
        i++;                /* added some code as debugger hangs if in loop doing nothing */
        if(i == 0x1000U)
        {
            i = 0U;
        }
    }
}


/*==============================================================================
 * Fourth U54.
 * If you need to modify this function, create your own one if a user directory space
 */
 __attribute__((weak)) void u54_4(void)
 {
    uint32_t hartid = read_csr(mhartid);
    volatile uint32_t   i = 0U;
    volatile uint32_t   test = 0U;

    clear_soft_interrupt();
    while(( test & MIP_MSIP) == 0U)
    {
        __asm("nop");
        __asm("nop");
        __asm("wfi");          /* wait for interprocess from hart0 to wake up */
        test = read_csr(mip);
    }

    while(i != 0x1001U)
    {
        i++;                /* added some code as debugger hangs if in loop doing nothing */
        if(i == 0x1000U)
        {
            i = 0U;
        }
    }
}

/**
 * copy_section
 * @param p_load
 * @param p_vma
 * @param p_vma_end
 */
static void copy_section(uint32_t * p_load, uint32_t * p_vma, uint32_t * p_vma_end)
{
    if ( p_vma != p_load)
    {
         while(p_vma < p_vma_end)
        {
             *p_vma = *p_load;
             ++p_load;
             ++p_vma;
        }
    }
}

/**
 * zero_section
 * @param start
 * @param end
 */
 static void zero_section(uint32_t * start, uint32_t * end)
{
    uint32_t * p_zero = start;

    while(p_zero < end)
    {
         *p_zero = 0U;
         ++p_zero;
    }
}
 /*------------------------------------------------------------------------------
  * _start() function called invoked
  * This function is called on power up and warm reset.
  */
 __attribute__((weak)) void init_memory( void)
 {
    copy_section(&__sdata_load, &__sdata_start, &__sdata_end);
    copy_section(&__data_load, &__data_start, &__data_end);
    zero_section(&__sbss_start, &__sbss_end);
    zero_section(&__bss_start, &__bss_end);

    PLIC_init_on_reset();
 }




