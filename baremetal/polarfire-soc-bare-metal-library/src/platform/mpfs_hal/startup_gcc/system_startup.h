/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
*/

/******************************************************************************
 * @file system_startup.h
 * @author Microchip-FPGA Embedded Systems Solutions
 * @brief Macros and APIs for the system_startup.c
 */

#ifndef SYSTEM_STARTUP_H
#define SYSTEM_STARTUP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum WFI_SM_
{
    INIT_THREAD_PR                      = 0x00,         /*!< 0 init pointer   */
    CHECK_WFI                           = 0x01,         /*!< is hart in wfi?  */
    SEND_WFI                            = 0x02,         /*!< separate state to
                                                            add a little delay*/
    CHECK_WAKE                          = 0x03,         /*!< has hart left wfi*/
} WFI_SM;

typedef struct HLS_DATA_
{
    volatile uint32_t in_wfi_indicator;
} HLS_DATA;

/*------------------------------------------------------------------------------
 * Symbols from the linker script used to locate the text, data and bss sections.
 */
extern unsigned long __stack_top_h0$;
extern unsigned long __stack_bottom_h0$;
extern unsigned long __stack_top_h1$;
extern unsigned long __stack_bottom_h1$;
extern unsigned long __stack_top_h2$;
extern unsigned long __stack_bottom_h2$;
extern unsigned long __stack_top_h3$;
extern unsigned long __stack_bottom_h3$;
extern unsigned long __stack_top_h4$;
extern unsigned long __stack_bottom_h4$;

extern unsigned long __data_load;
extern unsigned long __data_start;
extern unsigned long __data_end;

extern unsigned long __sbss_start;
extern unsigned long __sbss_end;

extern unsigned long __bss_start;
extern unsigned long __bss_end;

extern unsigned long __sdata_load;
extern unsigned long __sdata_start;
extern unsigned long __sdata_end;

extern unsigned long __text_load;
extern unsigned long __text_start;
extern unsigned long __text_end;

extern unsigned long __l2lim_end;

extern unsigned long __e51itim_start;
extern unsigned long __e51itim_end;

extern unsigned long __u54_1_itim_start;
extern unsigned long __u54_1_itim_end;

extern unsigned long __u54_2_itim_start;
extern unsigned long __u54_2_itim_end;

extern unsigned long __u54_3_itim_start;
extern unsigned long __u54_3_itim_end;

extern unsigned long __u54_4_itim_start;
extern unsigned long __u54_4_itim_end;

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
void init_ddr( void);
uint8_t init_mem_protection_unit(void);
uint8_t init_pmp(uint8_t hart_id);
uint8_t init_bus_error_unit( void);
uint8_t mss_set_apb_bus_cr(uint32_t reg_value);
uint8_t mss_get_apb_bus_cr(void);
char * memfill(void *dest, const void * src, size_t len);
char * config_copy(void *dest, const void * src, size_t len);
char * config_16_copy(void *dest, const void * src, size_t len);
char * config_32_copy(void *dest, const void * src, size_t len);
char * config_64_copy(void *dest, const void * src, size_t len);

void turn_on_fic0(void);
void turn_on_fic1(void);
void turn_on_fic2(void);
void turn_on_fic3(void);
void turn_on_mac0(void);
void turn_on_mac1(void);

void turn_off_fic0(void);
void turn_off_fic1(void);
void turn_off_fic2(void);
void turn_off_fic3(void);
void turn_off_mac0(void);
void turn_off_mac1(void);

void copy_section
(
    uint64_t * p_load,
    uint64_t * p_vma,
    uint64_t * p_vma_end
);
void zero_section
(
    uint64_t *__sbss_start,
    uint64_t * __sbss_end
);
void load_virtual_rom(void);

void count_section
(
    uint64_t * start_address,
    uint64_t * end_address,
    uint64_t * start_value
);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_STARTUP_H */
