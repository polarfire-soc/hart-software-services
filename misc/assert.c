/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * 
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file assert implementation
 * \brrief Local assert
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#include <stdlib.h>


/*!
 * \brief Local implemention of assert fail
 */

//void __assert_func(const char *__assertion, const char *__file, 
//        unsigned int __line, const char *__function) //__attribute__ ((__noreturn__))
void __assert_func(const char *__file, unsigned int __line, 
        const char *__function, const char *__assertion) //__attribute__ ((__noreturn__))
{
    mHSS_DEBUG_PRINTF("\t%s:%d: %s() Assertion failed:" CRLF "\t%s" CRLF, 
        __file, __line, __function, __assertion);

#ifndef __riscv
    exit(1);
#else
    while(1);
#endif
}

//void __assert_fail(const char *__assertion, const char *__file, 
//        unsigned int __line, const char *__function) //__attribute__ ((__noreturn__))
void __assert_fail(const char *__file, unsigned int __line, 
        const char *__function, const char *__assertion) //__attribute__ ((__noreturn__))
{
   __assert_func(__file, __line, __function, __assertion);
}

