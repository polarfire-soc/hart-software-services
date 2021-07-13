/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/***************************************************************************
 * @file mss_util.c
 * @author Microchip-FPGA Embedded Systems Solutions
 * @brief Utility functions
 *
 */
#include <stddef.h>
#include <stdbool.h>
#include "mpfs_hal/mss_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------------
 *
 */
void enable_interrupts(void) {
    __enable_irq();
}

/*------------------------------------------------------------------------------
 *
 */
uint64_t disable_interrupts(void) {
    uint64_t psr;
    psr = read_csr(mstatus);
    __disable_irq();
    return(psr);
}

/*------------------------------------------------------------------------------
 *
 */
void restore_interrupts(uint64_t saved_psr) {
    write_csr(mstatus, saved_psr);
}

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
void __enable_local_irq(uint8_t local_interrupt)
{
    if((local_interrupt > (int8_t)0) && (local_interrupt <= LOCAL_INT_MAX))
    {
        set_csr(mie, (0x1LLU << (int8_t)(local_interrupt + 16U)));  /* mie Register- Machine Interrupt Enable Register */
    }
}

/*------------------------------------------------------------------------------
 * Disable particular local interrupt
 */
void __disable_local_irq(uint8_t local_interrupt)
{
    if((local_interrupt > (int8_t)0) && (local_interrupt <= LOCAL_INT_MAX))
    {
        clear_csr(mie, (0x1LLU << (int8_t)(local_interrupt + 16U)));  /* mie Register- Machine Interrupt Enable Register */
    }
}

/**
 * readmcycle(void)
 * @return returns the mcycle count from hart CSR
 */
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

/**
 * sleep_cycles(uint64_t ncycles)
 * @param number of cycles to sleep
 */
void sleep_cycles(uint64_t ncycles)
{
    uint64_t starttime = readmcycle();
    volatile uint64_t endtime = 0U;

    while(endtime < (starttime + ncycles)) {
        endtime = readmcycle();
    }
}

/**
 * get_program_counter(void)
 * @return returns the program counter
 */
__attribute__((aligned(16))) uint64_t get_program_counter(void)
{
    uint64_t prog_counter;
    asm volatile ("auipc %0, 0" : "=r"(prog_counter));
    return (prog_counter);
}

/**
 * get_stack_pointer(void)
 * @return Return the stack pointer
 */
uint64_t get_stack_pointer(void)
{
    uint64_t stack_pointer;
    asm volatile ("addi %0, sp, 0" : "=r"(stack_pointer));
    return (stack_pointer);
}

/**
 * Return the tp register
 * The tp register holds the value of the Hart Common memory HLS once not in an
 * interrupt. If the tp value is used in an interrupt, it is saved first and
 * restored on exit. This conforms to OpenSBI implementation.
 *
 * @return returns the tp register value
 */
uint64_t get_tp_reg(void)
{
    uint64_t tp_reg_val;
    asm volatile ("addi %0, tp, 0" : "=r"(tp_reg_val));
    return (tp_reg_val);
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
