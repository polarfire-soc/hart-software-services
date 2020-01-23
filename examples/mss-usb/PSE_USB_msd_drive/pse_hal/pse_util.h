/***************************************************************************
 * (c) Copyright 2008 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * MACROs defines and prototypes associated with utility functions
 *
 * SVN $Revision: 9661 $
 * SVN $Date: 2018-01-15 10:43:33 +0000 (Mon, 15 Jan 2018) $
 */
#ifndef G5SOC_UTIL_H
#define G5SOC_UTIL_H

#include <stdint.h>
#include "encoding.h"
#include "mss_hart_ints.h"

#define DDR_BASE (0x80000000ul)
//#define DDR_SIZE (0x40000000ul)
#define DDR_SIZE (0x10000000ul)

/*
 * Useful macros 
 */
#define WRITE_REG8(x, y) (*((volatile uint8_t *)(x)) = y)
#define READ_REG8(x)     (*((volatile uint8_t *)(x)))

#define WRITE_REG32(x, y) (*((volatile uint32_t *)(x)) = y)
#define READ_REG32(x)     (*((volatile uint32_t *)(x)))

#define WRITE_REG64(x, y) (*((volatile uint64_t *)(x)) = y)
#define READ_REG64(x)     (*((volatile uint64_t *)(x)))

void pse_printf(const char * s, ...);

uint64_t readmcycle(void);

void sleep_ms(uint64_t msecs);
void sleep_cycles(uint64_t ncycles);


uint64_t get_stack_pointer(void);
uint64_t get_program_counter(void) __attribute__((aligned(16)));

void display_address_of_interest(uint64_t * address_of_interest, int nb_locations);

void exit_simulation(void);

void __disable_irq(void);
void __enable_irq(void);
void __enable_local_irq(int8_t local_interrupt);
void __disable_local_irq(int8_t local_interrupt);

#endif	/* G5SOC_UTIL_H */
