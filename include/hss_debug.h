#ifndef HSS_DEBUG_H
#define HSS_DEBUG_H

/*******************************************************************************
 * Copyright 2019-2020 Microchip Corporation.
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

#ifdef CONFIG_HAS_INTTYPES
#    include "inttypes.h"
#else
#  define __PRI64_PREFIX	"l"
#  define __PRIPTR_PREFIX	"l"
#  define PRIu64		__PRI64_PREFIX "u"
#endif

#include "hss_clock.h"

#define CRLF "\r\n"
#define CR   "\r"

#ifdef CONFIG_COLOR_OUTPUT
#    define mHSS_DEBUG_ERROR_TEXT            mHSS_DEBUG_PRINTF_EX("\033[37;41m")
#    define mHSS_DEBUG_WARN_TEXT             mHSS_DEBUG_PRINTF_EX("\033[1;33m")
#    define mHSS_DEBUG_STATUS_TEXT           mHSS_DEBUG_PRINTF_EX("\033[1;32m")
#    define mHSS_DEBUG_NORMAL_TEXT           mHSS_DEBUG_PRINTF_EX("\033[0m"); 
#    define mHSS_DEBUG_STATE_TRANSITION_TEXT mHSS_DEBUG_PRINTF_EX("\033[37;44m"); 
#else
#  define mHSS_DEBUG_ERROR_TEXT 
#  define mHSS_DEBUG_STATUS_TEXT
#  define mHSS_DEBUG_NORMAL_TEXT
#  define mHSS_DEBUG_STATE_TRANSITION_TEXT
#endif

#ifdef __riscv
int ee_printf(const char *fmt, ...);
int ee_puts(const char *buf);
int ee_putc(const char c);
#    define mHSS_PUTS ee_puts
#    define mHSS_PUTC ee_putc
#    define mHSS_FANCY_PRINTF ee_printf("%" PRIu64 " %s(): ", HSS_GetTime(),  __func__), ee_printf
#    define mHSS_FANCY_PRINTF_EX ee_printf
#    define mHSS_FANCY_PUTS ee_printf("%" PRIu64 " %s(): ", HSS_GetTime(),  __func__), ee_puts
#else
#    define mHSS_PUTS (void)
#    define mHSS_PUTC (void)
#    define mHSS_FANCY_PRINTF (void)
#    define mHSS_FANCY_PRINTF_EX (void)
#    define mHSS_FANCY_PUTS (void)
#endif

#ifndef mHSS_DEBUG_PRINTF
#  ifdef DEBUG
extern int my_printf(char const * const fmt, ...);
#    ifdef __riscv
#      define mHSS_DEBUG_PRINTF ee_printf("%" PRIu64 " %s(): ", HSS_GetTime(),  __func__), ee_printf
#      define mHSS_DEBUG_PRINTF_EX ee_printf
#      define mHSS_DEBUG_PUTS ee_puts
#    else
#      define mHSS_DEBUG_PRINTF my_printf("%" PRIu64 " %s(): ", HSS_GetTime(),  __func__), my_printf
#      define mHSS_DEBUG_PRINTF_EX my_printf
#      define mHSS_DEBUG_PUTS my_printf
#    endif
#  else
#    define mHSS_DEBUG_PRINTF (void) 
#    define mHSS_DEBUG_PRINTF_EX (void)
#    define mHSS_DEBUG_PUTS (void)
#  endif
#endif

#define DO_PRAGMA(x) _Pragma(#x)
#define TODO(x) DO_PRAGMA(message ("TODO: " #x))

#ifdef __cplusplus
}
#endif

#endif
