/******************************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
/***************************************************************************
 * @file system_startup.c
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief first C code called on startup. Will call user code created outside
 * the HAL.
 *
 * SVN $Revision$
 * SVN $Date$
 */
#include <stddef.h>
#include <stdbool.h>
#include "mss_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*LDRA_INSPECTED 440 S MR:R.11.1,R.11.2,R.11.4,R.11.6,R.11.7  Have to allocate number (address) as point reference*/
mss_sysreg_t*   SYSREG = ((mss_sysreg_t*) BASE32_ADDR_MSS_SYSREG);


/*==============================================================================
 * E51 startup.
 * If you need to modify this function, create your own one in a user directory
 * space
 * e.g. /hart0/e51.c
 */
__attribute__((weak)) int main_first_hart(void)
{
    uint64_t hartid = read_csr(mhartid);
    HLS_DATA* hls = NULL;

    if(hartid == MPFS_HAL_FIRST_HART)
    {
        uint8_t hard_idx;
        init_memory();
        (void)init_bus_error_unit();
        (void)init_mem_protection_unit();

        /*
         * Start the other harts. They are put in wfi in entry.S
         * When debugging, harts are released from reset separately,
         * so we need to make sure hart is in wfi before we try and release.
        */
        WFI_SM sm_check_thread = INIT_THREAD_PR;
        hard_idx = MPFS_HAL_FIRST_HART + 1U;
        while( hard_idx <= MPFS_HAL_LAST_HART)
        {
            uint32_t wait_count;

            switch(sm_check_thread)
            {
                default:
                case INIT_THREAD_PR:
                    hls = (HLS_DATA*)((uint8_t *)&__stack_bottom_h1$
                            + (((uint8_t *)&__stack_top_h1$ -
                                    (uint8_t *)&__stack_bottom_h1$) * hard_idx)
                                - (uint8_t *)(HLS_DEBUG_AREA_SIZE));
                    sm_check_thread = CHECK_WFI;
                    wait_count = 0U;
                    break;

                case CHECK_WFI:
                    if( hls->in_wfi_indicator == HLS_DATA_IN_WFI )
                    {
                        /* Separate state- to add a little delay */
                        sm_check_thread = SEND_WFI;
                    }
                    break;

                case SEND_WFI:
                    raise_soft_interrupt(hard_idx);
                    sm_check_thread = CHECK_WAKE;
                    wait_count = 0UL;
                    break;

                case CHECK_WAKE:
                    if( hls->in_wfi_indicator == HLS_DATA_PASSED_WFI )
                    {
                        sm_check_thread = INIT_THREAD_PR;
                        hard_idx++;
                        wait_count = 0UL;
                    }
                    else
                    {
                        wait_count++;
                        if(wait_count > 0x10U)
                        {
                            if( hls->in_wfi_indicator == HLS_DATA_IN_WFI )
                            {
                                raise_soft_interrupt(hard_idx);
                                wait_count = 0UL;
                            }
                        }
                    }
                    break;
            }
        }

        (void)main_other_hart();
    }

    /* should never get here */
    while(true)
    {
       volatile static uint64_t counter = 0U;

       /* Added some code as debugger hangs if in loop doing nothing */
       counter = counter + 1U;
    }

    return (0);
}

/*==============================================================================
 * U54s startup.
 * This is called from entry.S
 * If you need to modify this function, create your own one in a user directory
 * space
 *
 * Please note: harts 1 to 4 will wait in startup code in entry.S as they run
 * the wfi (wait for interrupt) instruction.
 * They are woken up as required by the the MPFS_HAL_FIRST_HART
 * ( It triggers a software interrupt on the particular hart to be woken up )
 */
__attribute__((weak)) int main_other_hart(void)
{
    uint64_t hartid = read_csr(mhartid);

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
    while(true)
    {
       volatile static uint64_t counter = 0U;

       /* Added some code as debugger hangs if in loop doing nothing */
       counter = counter + 1U;
    }

  return (0);

}

/*==============================================================================
 * E51 code executing after system startup.
 * In absence of an application function of this name with strong linkage, this
 * function will get linked.
 * This default implementation is for illustration purpose only. If you need to
 * modify this function, create your own one in an application directory space.
 */
 __attribute__((weak)) void e51(void)
 {
    /*Clear pending software interrupt in case there was any.
     Enable only the software interrupt so that other core can bring this core
     out of WFI by raising a software interrupt.
     Note that any other interrupt can also be used to bring CPU out of WFI*/
    clear_soft_interrupt();
    set_csr(mie, MIP_MSIP);

    /*put this hart into WFI.*/
    do
    {
        __asm("wfi");
    }while(0UL == (read_csr(mip) & MIP_MSIP));

    /*The hart is out of WFI, clear the SW interrupt. Here onwards Application
     can enable and use any interrupts as required*/
    clear_soft_interrupt();

    __enable_irq();

    while(true)
    {
       volatile static uint64_t counter = 0U;

       /* Added some code as debugger hangs if in loop doing nothing */
       counter = counter + 1U;
    }
}

/*==============================================================================
  * First U54.
 * In absence of an application function of this name with strong linkage, this
 * function will get linked.
 * This default implementation is for illustration purpose only. If you need to
 * modify this function, create your own one in an application directory space.
  */
 __attribute__((weak)) void u54_1(void)
 {
     /*Clear pending software interrupt in case there was any.
      Enable only the software interrupt so that other core can bring this core
      out of WFI by raising a software interrupt.
      Note that any other interrupt can also be used to bring CPU out of WFI*/
     clear_soft_interrupt();
     set_csr(mie, MIP_MSIP);

     /*put this hart into WFI.*/
     do
     {
         __asm("wfi");
     }while(0UL == (read_csr(mip) & MIP_MSIP));

     /*The hart is out of WFI, clear the SW interrupt. Here onwards Application
      can enable and use any interrupts as required*/
     clear_soft_interrupt();

     __enable_irq();

     while(true)
     {
        volatile static uint64_t counter = 0U;

        /* Added some code as debugger hangs if in loop doing nothing */
        counter = counter + 1U;
     }
 }


/*==============================================================================
 * Second U54.
 * In absence of an application function of this name with strong linkage, this
 * function will get linked.
 * This default implementation is for illustration purpose only. If you need to
 * modify this function, create your own one in an application directory space.
 */
__attribute__((weak)) void u54_2(void)
{
    /*Clear pending software interrupt in case there was any.
     Enable only the software interrupt so that other core can bring this core
     out of WFI by raising a software interrupt.
     Note that any other interrupt can also be used to bring CPU out of WFI*/
    clear_soft_interrupt();
    set_csr(mie, MIP_MSIP);

    /*put this hart into WFI.*/
    do
    {
        __asm("wfi");
    }while(0UL == (read_csr(mip) & MIP_MSIP));

    /*The hart is out of WFI, clear the SW interrupt. Here onwards Application
     can enable and use any interrupts as required*/
    clear_soft_interrupt();

    __enable_irq();

    while(true)
    {
       volatile static uint64_t counter = 0U;

       /* Added some code as debugger hangs if in loop doing nothing */
       counter = counter + 1U;
    }
}

/*==============================================================================
 * Third U54.
 * In absence of an application function of this name with strong linkage, this
 * function will get linked.
 * This default implementation is for illustration purpose only. If you need to
 * modify this function, create your own one in an application directory space.
 */
 __attribute__((weak)) void u54_3(void)
 {
     /*Clear pending software interrupt in case there was any.
      Enable only the software interrupt so that other core can bring this core
      out of WFI by raising a software interrupt.
      Note that any other interrupt can also be used to bring CPU out of WFI*/
     clear_soft_interrupt();
     set_csr(mie, MIP_MSIP);

     /*put this hart into WFI.*/
     do
     {
         __asm("wfi");
     }while(0UL == (read_csr(mip) & MIP_MSIP));

     /*The hart is out of WFI, clear the SW interrupt. Here onwards Application
      can enable and use any interrupts as required*/
     clear_soft_interrupt();

     __enable_irq();

     while(true)
     {
        volatile static uint64_t counter = 0U;

        /* Added some code as debugger hangs if in loop doing nothing */
        counter = counter + 1U;
     }
}

/*==============================================================================
 * Fourth U54.
 * In absence of an application function of this name with strong linkage, this
 * function will get linked.
 * This default implementation is for illustration purpose only. If you need to
 * modify this function, create your own one in an application directory space.
 */
 __attribute__((weak)) void u54_4(void)
 {
     /*Clear pending software interrupt in case there was any.
      Enable only the software interrupt so that other core can bring this core
      out of WFI by raising a software interrupt.
      Note that any other interrupt can also be used to bring CPU out of WFI*/
     clear_soft_interrupt();
     set_csr(mie, MIP_MSIP);

     /*put this hart into WFI.*/
     do
     {
         __asm("wfi");
     }while(0UL == (read_csr(mip) & MIP_MSIP));

     /*The hart is out of WFI, clear the SW interrupt. Here onwards Application
      can enable and use any interrupts as required*/
     clear_soft_interrupt();

     __enable_irq();

     while(true)
     {
        volatile static uint64_t counter = 0U;

        /* Added some code as debugger hangs if in loop doing nothing */
        counter = counter + 1U;
     }
}

/**
 * copy_section
 * @param p_load
 * @param p_vma
 * @param p_vma_end
 */
static void copy_section(uint32_t * p_load, uint32_t * p_vma,
                                                          uint32_t * p_vma_end)
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
         *p_zero = 0UL;
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

    __disable_all_irqs();      /* disables local and global interrupt enable */
    PLIC_init_on_reset();
 }


 /**
  * This function is configured by editing parameters in
  * mss_sw_config.h as required.
  * @return
  */

__attribute__((weak)) uint8_t init_bus_error_unit(void)
{
#ifndef SIFIVE_HIFIVE_UNLEASHED
    uint8_t hard_idx;
    /* Init BEU in all harts - enable local interrupt */
    for(hard_idx = MPFS_HAL_FIRST_HART; hard_idx <= MPFS_HAL_LAST_HART; hard_idx++)
    {
        BEU->regs[hard_idx].ENABLE      = (uint64_t)BEU_ENABLE;
        BEU->regs[hard_idx].PLIC_INT    = (uint64_t)BEU_PLIC_INT;
        BEU->regs[hard_idx].LOCAL_INT   = (uint64_t)BEU_LOCAL_INT;
    }
#endif
    return (0U);
}

/**
 * init_mem_protection_unit(void)
 * add this function to you code and configure as required
 * @return
 */
__attribute__((weak)) uint8_t init_mem_protection_unit(void)
{
    return (0U);
}

#ifdef __cplusplus
}
#endif
