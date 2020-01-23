/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/***************************************************************************
 * @file mss_util.h
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief MACROs defines and prototypes associated with utility functions
 *
 * SVN $Revision: 12296 $
 * SVN $Date: 2019-09-30 19:00:02 +0530 (Mon, 30 Sep 2019) $
 */
#ifndef G5SOC_UTIL_H
#define G5SOC_UTIL_H

#include <stdint.h>
#include "encoding.h"
#include "mss_hart_ints.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DDR_BASE (0x80000000ul)

/*
 * Useful macros 
 */
#define WRITE_REG8(x, y) (*((volatile uint8_t *)(x)) = (y))
#define READ_REG8(x)     (*((volatile uint8_t *)(x)))

#define WRITE_REG32(x, y) (*((volatile uint32_t *)(x)) = (y))
#define READ_REG32(x)     (*((volatile uint32_t *)(x)))

#define WRITE_REG64(x, y) (*((volatile uint64_t *)(x)) = (y))
#define READ_REG64(x)     (*((volatile uint64_t *)(x)))

uint64_t readmcycle(void);
uint64_t readmtime(void);

void sleep_ms(uint64_t msecs);
void sleep_cycles(uint64_t ncycles);


uint64_t get_stack_pointer(void);
uint64_t get_program_counter(void) __attribute__((aligned(16)));

#ifdef MPFS_PRINTF_DEBUG_SUPPORTED
void display_address_of_interest(uint64_t * address_of_interest, int nb_locations);
#endif

void exit_simulation(void);

void __disable_irq(void);
void __disable_all_irqs(void);
void __enable_irq(void);
void __enable_local_irq(int8_t local_interrupt);
void __disable_local_irq(int8_t local_interrupt);

void mss_init_mutex(uint64_t address);
void mss_take_mutex(uint64_t address);
void mss_release_mutex(uint64_t address);

#ifdef __cplusplus
}
#endif

#endif  /* G5SOC_UTIL_H */
