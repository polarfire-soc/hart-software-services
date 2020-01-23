/***************************************************************************
 * (c) Copyright 2008 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * MACROs defines and prototypes associated with utility functions
 *
 * SVN $Revision: 10515 $
 * SVN $Date: 2018-11-08 18:00:02 +0000 (Thu, 08 Nov 2018) $
 */
#ifndef G5SOC_UTIL_H
#define G5SOC_UTIL_H

#include <stdint.h>
#include "encoding.h"
#include "mss_hart_ints.h"

#define DDR_BASE (0x80000000ul)

/*
 * Useful macros 
 */
#define WRITE_REG8(x, y) (*((volatile uint8_t *)(x)) = y)
#define READ_REG8(x)     (*((volatile uint8_t *)(x)))

#define WRITE_REG32(x, y) (*((volatile uint32_t *)(x)) = y)
#define READ_REG32(x)     (*((volatile uint32_t *)(x)))

#define WRITE_REG64(x, y) (*((volatile uint64_t *)(x)) = y)
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
void __enable_irq(void);
void __enable_local_irq(int8_t local_interrupt);
void __disable_local_irq(int8_t local_interrupt);

#endif	/* G5SOC_UTIL_H */
