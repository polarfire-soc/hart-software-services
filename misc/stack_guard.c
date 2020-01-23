/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
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

unsigned long __stack_chk_guard;

void __stack_chk_guard_setup(void);
void __stack_chk_fail(void);

void __stack_chk_guard_setup(void)
{
    __stack_chk_guard = 0xDEAD0BAD;
}

void __stack_chk_fail(void)
{
    mHSS_DEBUG_PUTS("__stack_chk_fail(): stack corruption detected!!" CRLF);
    while (1); // stack has been corrupted
}
