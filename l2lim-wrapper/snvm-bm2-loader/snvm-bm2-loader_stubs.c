/*******************************************************************************
 * Copyright 2019-2026 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Minimal freestanding stubs for the sNVM BM2 loader.
 *
 * The loader is linked with -nostdlib so standard C library functions are
 * not available.  Only the small subset needed by fec_secded7264_bare.c is
 * provided here.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

void *memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;
    while (n-- != 0u) {
        *p++ = (uint8_t)c;
    }
    return s;
}
