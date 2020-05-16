/******************************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
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
 * @author Microchip-FPGA Embedded Systems Solutions
 * @brief first C code called on startup. Will call user code created outside
 * the HAL.
 */
#include <stddef.h>
#include <stdbool.h>
#include "mss_hal.h"
#ifdef  MPFS_HAL_HW_CONFIG
#include "nwc/mss_nwc_init.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

static void copy_section(uint64_t * p_load, uint64_t * p_vma, uint64_t * p_vma_end);

#ifdef  MPFS_HAL_HW_CONFIG
static void load_virtual_rom(void);
#endif  /* MPFS_HAL_HW_CONFIG */

/*LDRA_INSPECTED 440 S MR:R.11.1,R.11.2,R.11.4,R.11.6,R.11.7  Have to allocate number (address) as point reference*/
mss_sysreg_t * const SYSREG = ((mss_sysreg_t * const) BASE32_ADDR_MSS_SYSREG);


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
        /*
         * We only use code within the conditional compile
         * #ifdef MPFS_HAL_HW_CONFIG
         * if this program is used as part of the initial board bring-up
         * Please comment/uncomment MPFS_HAL_HW_CONFIG define in
         * platform/config/software/mpfs_hal/sw_config.h
         * as required.
         */
#ifdef  MPFS_HAL_HW_CONFIG
        load_virtual_rom();
        config_l2_cache();
#endif  /* MPFS_HAL_HW_CONFIG */
        init_memory();

#ifdef  MPFS_HAL_HW_CONFIG
        (void)init_bus_error_unit();
        (void)init_mem_protection_unit();
        (void)init_pmp((uint8_t)MPFS_HAL_FIRST_HART);
#endif  /* MPFS_HAL_HW_CONFIG */
        /*
         * Initialise NWC
         *      Clocks
         *      SGMII
         *      DDR
         *      IOMUX
         */
#ifdef  MPFS_HAL_HW_CONFIG
        (void)mss_nwc_init();
#endif  /* MPFS_HAL_HW_CONFIG */
        /*
         * Copies text section if relocation required
         */
        (void)copy_section(&__text_load, &__text_start, &__text_end);

#ifdef  MPFS_HAL_HW_CONFIG
        /*
         * Start the other harts. They are put in wfi in entry.S
         * When debugging, harts are released from reset separately,
         * so we need to make sure hart is in wfi before we try and release.
        */
        WFI_SM sm_check_thread = INIT_THREAD_PR;
        hard_idx = MPFS_HAL_FIRST_HART + 1U;
        while( hard_idx <= MPFS_HAL_LAST_HART)
        {
            uint32_t wait_count = 0u; // initialize to avoid compiler warning

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

#endif /* MPFS_HAL_HW_CONFIG */
        (void)main_other_hart();
    }

    /* should never get here */
    while(true)
    {
        static volatile uint64_t counter = 0U;

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
    extern char __app_stack_top_h0;
    extern char __app_stack_top_h1;
    extern char __app_stack_top_h2;
    extern char __app_stack_top_h3;
    extern char __app_stack_top_h4;

    const uint64_t app_stack_top_h0 = (const uint64_t)&__app_stack_top_h0;
    const uint64_t app_stack_top_h1 = (const uint64_t)&__app_stack_top_h1;
    const uint64_t app_stack_top_h2 = (const uint64_t)&__app_stack_top_h2;
    const uint64_t app_stack_top_h3 = (const uint64_t)&__app_stack_top_h3;
    const uint64_t app_stack_top_h4 = (const uint64_t)&__app_stack_top_h4;

    uint64_t hartid = read_csr(mhartid);

    volatile uint64_t dummy;

    switch(hartid)
    {

    case 0U:
        __asm volatile ("add sp, x0, %1" : "=r"(dummy) : "r"(app_stack_top_h0));
        e51();
        break;

    case 1U:
        (void)init_pmp((uint8_t)1);
        __asm volatile ("add sp, x0, %1" : "=r"(dummy) : "r"(app_stack_top_h1));
        u54_1();
        break;

    case 2U:
        (void)init_pmp((uint8_t)2);
        __asm volatile ("add sp, x0, %1" : "=r"(dummy) : "r"(app_stack_top_h2));
        u54_2();
        break;

    case 3U:
        (void)init_pmp((uint8_t)3);
        __asm volatile ("add sp, x0, %1" : "=r"(dummy) : "r"(app_stack_top_h3));
        u54_3();
        break;

    case 4U:
        (void)init_pmp((uint8_t)4);
        __asm volatile ("add sp, x0, %1" : "=r"(dummy) : "r"(app_stack_top_h4));
        u54_4();
        break;

    default:
        /* no more harts */
        break;
    }

    /* should never get here */
    while(true)
    {
        static volatile uint64_t counter = 0U;

       /* Added some code as debugger hangs if in loop doing nothing */
       counter = counter + 1U;
    }

  return (0);

}

/*==============================================================================
 * Load the virtual ROM located at address 0x20003120 within the SCB system
 * registers with an executable allowing to park a hart in an infinite loop.
 */
#ifdef  MPFS_HAL_HW_CONFIG
#define VIRTUAL_BOOTROM_BASE_ADDR   0x20003120UL
#define NB_BOOT_ROM_WORDS           8U
static void load_virtual_rom(void)
{
    int inc;
    volatile uint32_t * p_virtual_bootrom = (uint32_t *)VIRTUAL_BOOTROM_BASE_ADDR;

    const uint32_t rom[NB_BOOT_ROM_WORDS] =
    {
            0x00000513U,    /* li a0, 0 */
            0x34451073U,    /* csrw mip, a0 */
            0x10500073U,    /* wfi */
            0xFF5FF06FU,    /* j 0x20003120 */
            0xFF1FF06FU,    /* j 0x20003120 */
            0xFEDFF06FU,    /* j 0x20003120 */
            0xFE9FF06FU,    /* j 0x20003120 */
            0xFE5FF06FU     /* j 0x20003120 */
    };

    for(inc = 0; inc < NB_BOOT_ROM_WORDS; ++inc)
    {
        p_virtual_bootrom[inc] = rom[inc];
    }
}
#endif  /* MPFS_HAL_HW_CONFIG */

/*==============================================================================
 * Put the hart executing this code into an infinite loop executing from the
 * SCB system register memory space.
 * This allows preventing a hart from accessing memory regardless of memory
 * hierarchy configuration or compiler/linker settings.
 * This function relies on load_virtual_rom() having been called previously to
 * populate the virtual ROM with a suitable executable.
 */
static void park_hart(void)
{
    clear_csr(mstatus, MSTATUS_MIE);
    __asm volatile("fence.i");
    __asm volatile("li ra,0x20003120");
    __asm volatile("ret");
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
    /* Put hart in safe infinite WFI loop. */
     park_hart();
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
    /* Put hart in safe infinite WFI loop. */
     park_hart();
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
    /* Put hart in safe infinite WFI loop. */
    park_hart();
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
    /* Put hart in safe infinite WFI loop. */
     park_hart();
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
    /* Put hart in safe infinite WFI loop. */
     park_hart();
}

 /*==============================================================================
  * Copy hardware configuration to registers.
  * This function should be used in place of memcpy() to cover the use case
  * where C library code has not yet been copied from its LMA to VMA. For
  * example copying before the copy_section of the .text section has taken
  * place.
  */
 char * config_copy(void *dest, const void * src, size_t len)
 {
     char *csrc = (char *)src;
     char *cdest = (char *)dest;

     for(int inc = 0; inc < len; inc++)
     {
         cdest[inc] = csrc[inc];
     }

     return(csrc);
 }


/**
 * copy_section
 * @param p_load
 * @param p_vma
 * @param p_vma_end
 */
static void copy_section
(
    uint64_t * p_load,
    uint64_t * p_vma,
    uint64_t * p_vma_end)
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
static void zero_section(uint64_t * start, uint64_t * end)
{
    uint64_t * p_zero = start;

    while(p_zero < end)
    {
        *p_zero = 0UL;
        ++p_zero;
    }
}

 /*-----------------------------------------------------------------------------
  * _start() function called invoked
  * This function is called on power up and warm reset.
  */
 __attribute__((weak)) void init_memory( void)
 {
    extern uint64_t __l2_scratchpad_load;
    extern uint64_t __l2_scratchpad_start;
    extern uint64_t __l2_scratchpad_end;

    copy_section(&__sdata_load, &__sdata_start, &__sdata_end);
    copy_section(&__data_load, &__data_start, &__data_end);
    copy_section(&__l2_scratchpad_load, &__l2_scratchpad_start, &__l2_scratchpad_end);
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
#ifndef SIFIVE_HIFIVE_UNLEASHED
    mpu_configure();
#endif
    return (0U);
}

/**
 * init_pmp(void)
 * add this function to you code and configure as required
 * @return
 */
__attribute__((weak)) uint8_t init_pmp(uint8_t hart_id)
{
    pmp_configure(hart_id);
    return (0U);
}

#ifdef __cplusplus
}
#endif
