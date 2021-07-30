/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file Stack Checking guard functions
 * \brief Stack Checking guard functions
 */

#include "config.h"
#include "hss_types.h"

#include "hss_debug.h"

const unsigned long __stack_chk_guard = 0xDEAD0BAD;

void __stack_chk_guard_setup(void);
void __stack_chk_fail(void);

__attribute__((weak)) void __stack_chk_guard_setup(void)
{
    //__stack_chk_guard is already set-up as 0xDEAD0BAD
}

__attribute__((weak)) void __stack_chk_fail(void)
{
    // Print a canary to know that we've had stack corruption...
    //
    // to help debug this, it might help to disable the print statement
    // once stack corruption is detected...
    mHSS_DEBUG_PUTS("__stack_chk_fail(): stack corruption detected!!" CRLF);
    asm("ebreak");
}

#ifdef __linux
// if using the Linux compiler, we might need __memset_chk() also
#include <string.h> // for memset
void* __memset_chk(void *dst, int c, size_t len, size_t dstlen);
__attribute__((weak)) void* __memset_chk(void *dst, int c, size_t len, size_t dstlen)
{
    void * result = NULL;

    if (dstlen < len) {
        mHSS_DEBUG_PUTS("__memset_chk(): dstlen < len!!" CRLF);
        asm("ebreak");
    } else {
        result = memset(dst, c, len);
    }

    return result;
}
#endif
