/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
*/

/***********************************************************************************
 * @file system_startup.h
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief Macros and APIs for the system_startup.c

 * SVN $Revision$
 * SVN $Date$
 */

#ifndef SYSTEM_STARTUP_H
#define SYSTEM_STARTUP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum WFI_SM_
{
    INIT_THREAD_PR                      = 0x00,         /*!< 0 init pointer                        */
    CHECK_WFI                           = 0x01,         /*!< is hart in wfi?                        */
    SEND_WFI                            = 0x02,         /*!< separate state to add a little delay   */
    CHECK_WAKE                          = 0x03,         /*!< has hart left wfi                      */
} WFI_SM;

typedef struct HLS_DATA_
{
    volatile uint32_t in_wfi_indicator;
} HLS_DATA;

/*------------------------------------------------------------------------------
 * Symbols from the linker script used to locate the text, data and bss sections.
 */

extern unsigned int __stack_top_h1$;
extern unsigned int __stack_bottom_h1$;

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

/*
 * Function Declarations
 */
int main_first_hart(void);
int main_other_hart(void);
void e51(void);
void u54_1(void);
void u54_2(void);
void u54_3(void);
void u54_4(void);
void init_memory( void);
uint8_t init_mem_protection_unit(void);
uint8_t init_bus_error_unit( void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_STARTUP_H */
