/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/***************************************************************************
 * @file mss_util.c
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief Utility functions
 *
 * SVN $Revision: 12296 $
 * SVN $Date: 2019-09-30 19:00:02 +0530 (Mon, 30 Sep 2019) $
 */
#include <stddef.h>
#include <stdbool.h>
#include "mss_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------------
 * Disable all interrupts.
 */
void __disable_irq(void)
{
    clear_csr(mstatus, MSTATUS_MIE);
    clear_csr(mstatus, MSTATUS_MPIE);
}

void __disable_all_irqs(void)
{
    __disable_irq();
    write_csr(mie, 0x00U);
    write_csr(mip, 0x00);
}

/*------------------------------------------------------------------------------
 * Enable all interrupts.
 */
void __enable_irq(void)
{
    set_csr(mstatus, MSTATUS_MIE);  /* mstatus Register- Machine Interrupt Enable */
}

/*------------------------------------------------------------------------------
 * Enable particular local interrupt
 */
void __enable_local_irq(int8_t local_interrupt)
{
    if((local_interrupt > 0) && (local_interrupt <= LOCAL_INT_MAX))
    {
        set_csr(mie, (1LLU << (local_interrupt + 16U)));  /* mie Register- Machine Interrupt Enable Register */
    }
}

/*------------------------------------------------------------------------------
 * Disable particular local interrupt
 */
void __disable_local_irq(int8_t local_interrupt)
{
    if((local_interrupt > 0U) && (local_interrupt <= LOCAL_INT_MAX))
    {
        clear_csr(mie, (0x01U << (local_interrupt + 16U)));  /* mie Register- Machine Interrupt Enable Register */
    }
}

/*
 * Functions
 */
uint64_t readmtime(void)
{
    volatile uint64_t hartid = read_csr(mhartid);
    volatile uint64_t * mtime_hart = NULL;
    uint64_t mtime = 0ULL;

    switch(hartid) {
    case 0:
        mtime_hart = (volatile uint64_t *)U5CP_MTIME_H0;
        break;

    case 1:
        mtime_hart = (volatile uint64_t *)U5CP_MTIME_H1;
        break;

    case 2:
        mtime_hart = (volatile uint64_t *)U5CP_MTIME_H2;
        break;

    case 3:
        mtime_hart = (volatile uint64_t *)U5CP_MTIME_H3;
        break;

    case 4:
        mtime_hart = (volatile uint64_t *)U5CP_MTIME_H4;
        break;

    default:
        return (0ULL);
        break;
    }

    mtime = *mtime_hart;
    return (mtime);
}


uint64_t readmcycle(void)
{
    return (read_csr(mcycle));
}

void sleep_ms(uint64_t msecs)
{
    uint64_t starttime = readmtime();
    volatile uint64_t endtime = 0U;

    while(endtime < (starttime+msecs)) {
        endtime = readmtime();
    }
}

void sleep_cycles(uint64_t ncycles)
{
    uint64_t starttime = readmcycle();
    volatile uint64_t endtime = 0U;

    while(endtime < (starttime + ncycles)) {
        endtime = readmcycle();
    }
}

void exit_simulation(void) {
    uint64_t hartid = read_csr(mhartid);
    volatile uint32_t * exit_simulation_p = (uint32_t *)0x60000000U;


    *exit_simulation_p = 1U;
}

__attribute__((aligned(16))) uint64_t get_program_counter(void)
{
    uint64_t prog_counter;
    asm volatile ("auipc %0, 0" : "=r"(prog_counter));
    return (prog_counter);
}

uint64_t get_stack_pointer(void)
{
    uint64_t prog_counter;
    asm volatile ("addi %0, sp, 0" : "=r"(prog_counter));
    return (prog_counter);
}

#ifdef PRINTF_DEBUG_SUPPORTED
void display_address_of_interest(uint64_t * address_of_interest, int nb_locations) {
  uint64_t * p_addr_of_interest = address_of_interest;
  int inc;
  mpfs_printf(" Displaying address of interest: 0x%lx\n", p_addr_of_interest);

  for (inc = 0U; inc < nb_locations; ++inc) {
    mpfs_printf("  address of interest: 0x%lx: 0x%lx\n", p_addr_of_interest, *p_addr_of_interest);
    p_addr_of_interest = p_addr_of_interest + 8;
  }
}
#endif

#ifdef __cplusplus
}
#endif
