/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"
#include "hss_progress.h"
#include "hss_trigger.h"
#include "uart_helper.h"

bool HSS_Trigger_IsNotified(enum HSS_Event event) {
    return TRUE;
}

void HSS_Debug_Timestamp(void)
{
    ;
}

void HSS_Debug_Highlight(HSS_Debug_LogLevel_t logLevel)
{
    ;
}


void HSS_ShowProgress(size_t totalNumTasks, size_t numTasksRemaining)
{
    ;
}

int sbi_printf(const char *fmt, ...) {
    (void)fmt;
    return 0;
}

void *sbi_memcpy(void *dest, const void *src, size_t n);
void *sbi_memcpy(void *dest, const void *src, size_t n)
{
    char *pSrc = (char *)src;
    char *pDest = (char *)dest;
    while (n) {
        *pDest = *pSrc;
        ++pDest; ++pSrc; --n;
    }
    return dest;
}


void *sbi_memset(void *s, int c, size_t n);
void *sbi_memset(void *s, int c, size_t n)
{
    char *pChar = (char *)s;
    while (n) {
        *pChar = c;
        ++pChar; --n;
    }
    return s;
}

void sbi_puts(const char *buf)
{
    (void)buf;
}

size_t sbi_strlen(const char *str);
size_t sbi_strlen(const char *s)
{
    size_t result = 0;

    while (*s) {
        ++s;
        ++result;
    }

    return result;
}


extern uint64_t __l2lim_heap_base;
uintptr_t malloc_base = (uintptr_t)&__l2lim_heap_base;
void *malloc(size_t size);
void *malloc(size_t size)
{
    void *result = (void *)malloc_base;
    malloc_base += size;
    return result;
}

void free(void *ptr);
void free(void *ptr)
{
    ;
}

#if IS_ENABLED(CONFIG_DEBUG_PROFILING_SUPPORT)
void __attribute__((no_instrument_function, weak)) __cyg_profile_func_enter (void *pFunc, void *pCaller)
{
}
void __attribute__((no_instrument_function, weak)) __cyg_profile_func_exit (void *pFunc, void *pCaller)
{
}
#endif
