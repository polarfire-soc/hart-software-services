/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file Tiny CLI parser
 * \brief Tiny CLI parser
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"
#include "assert.h"

#include <stddef.h>

#define STEP_SIZE 16
void HSS_TinyCLI_HexDump(uint8_t *pStart, ptrdiff_t count);
void HSS_TinyCLI_HexDump(uint8_t *pStart, ptrdiff_t count)
{
    if ((!pStart) || (count == 0u)) { return; }

    for (ptrdiff_t i = 0u; i < count; i+=STEP_SIZE) {
        mHSS_PRINTF("%08x:%08x  ",
            (uint32_t)(((ptrdiff_t)pStart + i) >> 32),  // upper 32-bits of address
            (uint32_t)(((ptrdiff_t)pStart + i) & ((1lu<<32)-1u))); // lower 32-bits of address

        for (ptrdiff_t j = 0u; j < STEP_SIZE; j++) {
            if ((i + j) >= count) {
                mHSS_PRINTF("   ");
            } else {
                mHSS_PRINTF("%02x ", *(pStart + i + j));
		if ((j % 4) == 3) {
                    mHSS_PRINTF(" ");
		}
            }
        }

        mHSS_PRINTF(" ");
        for (ptrdiff_t j = 0u; j < STEP_SIZE; j++) {
            if ((i + j) >= count) {
                mHSS_PRINTF(" ");
            } else {
                uint8_t octet = *(pStart + i + j);
                if ((octet > 31u) && (octet < 128u)) {
                    ;
                } else {
                    octet = '.';
                }
                mHSS_PRINTF("%c", (char)octet);
            }
        }
        mHSS_PRINTF(CRLF);
    }
}
