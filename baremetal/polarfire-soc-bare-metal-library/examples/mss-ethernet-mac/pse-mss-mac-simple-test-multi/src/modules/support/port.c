/*
    FreeRTOS V8.2.3 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution and was contributed
    to the project by Technolution B.V. (www.technolution.nl,
    freertos-riscv@technolution.eu) under the terms of the FreeRTOS
    contributors license.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the RISC-V port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
//#include "FreeRTOS.h"
//#include "task.h"
#include <stdint.h>
#include "portmacro.h"
#define configCPU_CLOCK_HZ            ( ( unsigned long ) 100000000 )
#define configTICK_CLOCK_HZ           ( ( unsigned long ) 1000000 )
#define configTICK_RATE_HZ            ( ( unsigned long ) 1000 )

/* A variable is used to keep track of the critical section nesting.  This
variable has to be stored as part of the task context and must be initialised to
a non zero value to ensure interrupts don't inadvertently become unmasked before
the scheduler starts.  As it is stored as part of the task context it will
automatically be set to 0 when the first task is started. */

/* Contains context when starting scheduler, save all 31 registers */
#ifdef __gracefulExit
BaseType_t xStartContext[31] = {0};
#endif

volatile uint64_t* mtime = (volatile uint64_t*)0x0200bff8;
volatile uint64_t* timecmp = (volatile uint64_t*)0x02004000;

volatile uint64_t g_tick_counter = 0;


/*
 * Handler for timer interrupt
 */
void vPortSysTickHandler( void );

/*
 * Setup the timer to generate the tick interrupts.
 */
void vPortSetupTimer( void );

/*
 * Set the next interval for the timer
 */
static void prvSetNextTimerInterrupt( void );

/*-----------------------------------------------------------*/

/* Sets the next timer interrupt
 * Reads current timer register and adds tickrate
 * Does nothing if a Clint was not found in the hardware configuration string
 * Using previous timer compare may fail if interrupts were disabled for a long time,
 * which is likely for the very first interrupt. When that happens, compare timer + 
 * tickrate may already be behind current timer and prevent correctly programming
 * the 2nd interrupt
 */

void vPortEnterCritical( void )
{
    portDISABLE_INTERRUPTS();
//    uxCriticalNesting++;
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
//    uxCriticalNesting--;
//    if( uxCriticalNesting == 0 )
    {
        portENABLE_INTERRUPTS();
    }
}
/*-----------------------------------------------------------*/

static void prvSetNextTimerInterrupt(void)
{
    if (mtime && timecmp) 
        *timecmp = *mtime + (configTICK_CLOCK_HZ / configTICK_RATE_HZ);
}
/*-----------------------------------------------------------*/

/* Sets and enable the timer interrupt */
void vPortSetupTimer(void)
{

    /* reuse existing routine */
    prvSetNextTimerInterrupt();
    /* Enable timer interupt */
    __asm volatile("csrs mie,%0"::"r"(0x80));
}
/*-----------------------------------------------------------*/


/* Clear current interrupt mask and set given mask */
void vPortClearInterruptMask(int mask)
{
    __asm volatile("csrw mie, %0"::"r"(mask));
}
/*-----------------------------------------------------------*/

/* Set interrupt mask and return current interrupt enable register */
int vPortSetInterruptMask(void)
{
    int ret;
    __asm volatile("csrr %0,mie":"=r"(ret));
    __asm volatile("csrc mie,%0"::"i"(7));
    return ret;
}

/*-----------------------------------------------------------*/

void vPortSysTickHandler( void )
{
    prvSetNextTimerInterrupt();
    g_tick_counter++;
}

