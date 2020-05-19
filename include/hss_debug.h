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
#    ifndef PRIu64
#        define PRIu64 "llu"
#    endif
#else
#  define __PRI64_PREFIX	"l"
#  define __PRIPTR_PREFIX	"l"
#  define PRIu64		__PRI64_PREFIX "u"
#endif

#include "hss_clock.h"

#define CRLF "\r\n"
#define CR   "\r"

#ifdef CONFIG_COLOR_OUTPUT
#    define mHSS_FANCY_ERROR_TEXT            mHSS_FANCY_PRINTF_EX("\033[1;31m")
#    define mHSS_FANCY_WARN_TEXT             mHSS_FANCY_PRINTF_EX("\033[1;33m")
#    define mHSS_FANCY_STATUS_TEXT           mHSS_FANCY_PRINTF_EX("\033[1;32m")
#    define mHSS_FANCY_NORMAL_TEXT           mHSS_FANCY_PRINTF_EX("\033[0m");
#    define mHSS_FANCY_STATE_TRANSITION_TEXT mHSS_FANCY_PRINTF_EX("\033[1;34m");
#else
#  define mHSS_FANCY_ERROR_TEXT
#  define mHSS_FANCY_WARN_TEXT
#  define mHSS_FANCY_STATUS_TEXT
#  define mHSS_FANCY_NORMAL_TEXT
#  define mHSS_FANCY_STATE_TRANSITION_TEXT
#endif

int sbi_printf(const char *fmt, ...);
void sbi_puts(const char *buf);
void sbi_putc(char c);
#    define mHSS_PUTS sbi_puts
#    define mHSS_PUTC sbi_putc
#    define mHSS_FANCY_PRINTF (void)sbi_printf("%" PRIu64 " %s(): ", HSS_GetTime(),  __func__), sbi_printf
#    define mHSS_PRINTF sbi_printf
#    define mHSS_FANCY_PRINTF_EX sbi_printf
#    define mHSS_FANCY_PUTS (void)sbi_printf("%" PRIu64 " %s(): ", HSS_GetTime(),  __func__), sbi_puts

#ifndef mHSS_DEBUG_PRINTF
#  ifdef DEBUG
#      define mHSS_DEBUG_PRINTF (void)sbi_printf("%" PRIu64 " %s(): ", HSS_GetTime(),  __func__), sbi_printf
#      define mHSS_DEBUG_PRINTF_EX sbi_printf
#      define mHSS_DEBUG_PUTS sbi_puts
#  else
#    define mHSS_DEBUG_PRINTF (void)
#    define mHSS_DEBUG_PRINTF_EX (void)
#    define mHSS_DEBUG_PUTS (void)
#  endif
#endif

#define mDO_PRAGMA(x) _Pragma(#x)
#define mTODO(x) mDO_PRAGMA(message ("TODO: " #x))

#ifdef __cplusplus
}
#endif

#endif
