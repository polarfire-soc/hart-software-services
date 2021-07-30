#ifndef HSS_DEBUG_H
#define HSS_DEBUG_H

/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
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
 *
 *
 * Hart Software Services - HSS Debugging support
 *
 */

/**
 * \file Debug Logging Functions
 * \brief Debug Logging Functions
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "hss_types.h"

#if IS_ENABLED(CONFIG_CC_HAS_INTTYPES)
#    include "inttypes.h"
#    ifndef PRIu64
#        define PRIu64 "llu"
#    endif
#else
#  define __PRI64_PREFIX	"l"
#  define __PRIPTR_PREFIX	"l"
#  define PRIu64		__PRI64_PREFIX "u"
#endif

#include "hss_clock.h"

#define CRLF "\n"
#define CR   "\r"

typedef enum {
    HSS_DEBUG_LOG_NORMAL,
    HSS_DEBUG_LOG_FUNCTION,
    HSS_DEBUG_LOG_TIMESTAMP,
    HSS_DEBUG_LOG_ERROR,
    HSS_DEBUG_LOG_WARN,
    HSS_DEBUG_LOG_STATUS,
    HSS_DEBUG_LOG_STATE_TRANSITION,
} HSS_Debug_LogLevel_t;

void HSS_Debug_Highlight(HSS_Debug_LogLevel_t logLevel);

#ifndef __SBI_CONSOLE_H__
int sbi_printf(const char *fmt, ...);
void sbi_puts(const char *buf);
void sbi_putc(char c);
#endif

void HSS_Debug_Timestamp(void);
#    define mHSS_TIMESTAMP HSS_Debug_Timestamp()

#    define mHSS_PUTS sbi_puts
#    define mHSS_PUTC sbi_putc
#    define mHSS_FANCY_PRINTF(logLevel, ...) { \
         mHSS_TIMESTAMP; \
         HSS_Debug_Highlight(HSS_DEBUG_LOG_FUNCTION); \
         (void)sbi_printf(" %s(): ", __func__); \
         HSS_Debug_Highlight(HSS_DEBUG_##logLevel); \
         sbi_printf(__VA_ARGS__); \
         HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL); \
     }
#    define mHSS_FANCY_PUTS(logLevel, ...) { \
         mHSS_TIMESTAMP; \
         HSS_Debug_Highlight(HSS_DEBUG_LOG_FUNCTION); \
         (void)sbi_printf(" %s(): ", __func__); \
         HSS_Debug_Highlight(HSS_DEBUG_##logLevel); \
         sbi_puts(__VA_ARGS__); \
         HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL); \
     }

#    define mHSS_PRINTF sbi_printf
#    define mHSS_FANCY_PRINTF_EX sbi_printf

#ifndef mHSS_DEBUG_PRINTF
//#  ifdef DEBUG
#    define mHSS_DEBUG_PRINTF(logLevel, ...) { \
         mHSS_TIMESTAMP; \
         HSS_Debug_Highlight(HSS_DEBUG_LOG_FUNCTION); \
         (void)sbi_printf(" %s(): ", __func__); \
         HSS_Debug_Highlight(HSS_DEBUG_##logLevel); \
         sbi_printf(__VA_ARGS__); \
         HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL); \
     }
#      define mHSS_DEBUG_PRINTF_EX sbi_printf
#      define mHSS_DEBUG_PUTS sbi_puts
//#  else
//#    define mHSS_DEBUG_PRINTF(logLevel, ...) (void)(0)
//#    define mHSS_DEBUG_PRINTF_EX (void)
//#    define mHSS_DEBUG_PUTS (void)
///#  endif
#endif

#define mDO_PRAGMA(x) _Pragma(#x)
#define mTODO(x) mDO_PRAGMA(message ("TODO: " #x))

#ifdef __cplusplus
}
#endif

#endif
