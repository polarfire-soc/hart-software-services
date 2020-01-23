/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * 
 * MPFS HSS Embedded Software
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

/******************************************************************************************************/

#include "hss_debug.h"

int my_printf(char const * const fmt, ... )
{
#ifndef __riscv
    char buffer[1024];
#endif
    int result = 0;
    va_list args;
    va_start(args, fmt);
#ifndef __riscv
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    result = printf("%s", buffer);
#endif
    va_end(args);
    return result;
}

#ifdef __riscv
int puts(const char *string)
{
    int i = 0, debugStrLen = strlen(string);
    while (i < debugStrLen) {
	*(volatile uint32_t *)(0x20000000u) = (uint32_t)(string[i]);
	i++;
    }
    return debugStrLen;
}
#endif
