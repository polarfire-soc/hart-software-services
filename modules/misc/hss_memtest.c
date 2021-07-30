/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file Simple Memory Tester
 * \brief Simple Memory Tester
 *
 * Simple memory tester, based on
 *   Barr, Michael. "Software-Based Memory Testing," Embedded Systems Programming,
 *   July 2000, pp. 28-40.
 *
 * The original source code for these memory tests is in the public domain and is available at
 *   https://barrgroup.com/resources/free-source-code-memory-tests-c
 */

#include "config.h"

#include <stddef.h>
#include "hss_types.h"

#include "hss_debug.h"
#include "hss_memtest.h"
#include "hss_progress.h"
#include "uart_helper.h"

#include "hss_perfctr.h"

// Walking Ones test of the Data Bus wiring
static uint64_t HSS_MemTestDataBus(volatile uint64_t *address)
{
    uint64_t pattern;
    uint64_t result = 0u;

    for (pattern = 1u; pattern != 0u; pattern <<=1) {
        *address = pattern;
        if (*address != pattern) {
            result = pattern;
            break;
        }
    }

    return result;
}

// Walking Ones test of the Address Bus wiring
static uint64_t* HSS_MemTestAddressBus(volatile uint64_t *baseAddr, const size_t numBytes)
{
    const size_t numWords = numBytes / sizeof(uint64_t);
    const size_t addrMask = numWords - 1u;
    size_t offset;
    size_t testOffset;
    uint64_t* result = NULL;
    uint8_t rx_char;

    const uint64_t pattern = (uint64_t)0xAAAAAAAAAAAAAAAAu;
    const uint64_t antiPattern = (uint64_t)0x5555555555555555u;

    //mHSS_FANCY_PRINTF(LOG_NORMAL, "Walking up address bus, setting cells to pattern" CRLF);
    for (offset = 1u; (offset & addrMask) != 0u; offset <<= 1) {
        baseAddr[offset] = pattern;

        if (uart_getchar(&rx_char, 0, false) && ((rx_char == '\003') || (rx_char == '\033'))) {
            goto do_return;
        }
    }

    testOffset = 0u;
    baseAddr[0] = antiPattern;

    //mHSS_FANCY_PRINTF(LOG_NORMAL, "Walking up address bus, verifying set cells" CRLF);
    for (offset = 1u; (offset & addrMask) != 0u; offset <<= 1) {
        if (baseAddr[offset] != pattern) {
            mHSS_FANCY_PRINTF(LOG_ERROR, "1: 0x%016p==0x%016llx vs expected 0x%016llx" CRLF,
                baseAddr + offset, baseAddr[offset], pattern);
            result = ((uint64_t *)&baseAddr[offset]);
            break;
        }

        if (uart_getchar(&rx_char, 0, false) && ((rx_char == '\003') || (rx_char == '\033'))) {
            goto do_return;
        }
    }

    baseAddr[0] = pattern;

    //mHSS_FANCY_PRINTF(LOG_NORMAL, "Walking up address bus, setting cells to antipattern" CRLF);
    if (result == NULL) {
        for (testOffset = 1u; (testOffset & addrMask) != 0u; testOffset <<= 1) {
            baseAddr[testOffset] = antiPattern;

            // ensure that the base isn't affected
            if (baseAddr[0] != pattern) {
                mHSS_FANCY_PRINTF(LOG_ERROR, "2: 0x%016p==0x%016llx vs expected 0x%016llx" CRLF,
                    baseAddr + offset, baseAddr[offset], pattern);
                result = ((uint64_t *)&baseAddr[testOffset]);
                break;
            }

            if (result == NULL) {
                // walk up the address bus, ensuring that no other address is affected
                for (offset = 1u; (offset & addrMask) != 0u; offset <<= 1) {
                    if ((baseAddr[offset] != pattern) && (offset != testOffset)) {
                        mHSS_FANCY_PRINTF(LOG_ERROR, "3: 0x%016p==0x%016llx vs expected 0x%016llx" CRLF,
                            baseAddr + offset, baseAddr[offset], pattern);
                        result = ((uint64_t *)&baseAddr[testOffset]);
                        offset = 0u; testOffset = 0u; // terminate loops
                        break;
                    }
                }
            }

            // if okay, write the pattern back and move on
            if (result == NULL) {
                baseAddr[testOffset] = pattern;
            }

            if (uart_getchar(&rx_char, 0, false) && ((rx_char == '\003') || (rx_char == '\033'))) {
                goto do_return;
            }
        }
    }

do_return:
    return result;
}

static uint64_t *HSS_MemTestDevice(volatile uint64_t *baseAddr, size_t numBytes)
{
    size_t offset;
    size_t numWords = numBytes / sizeof(uint64_t);
    uint64_t* result = NULL;

    uint64_t pattern;
    uint64_t antiPattern;
    uint8_t rx_char;

    // write pattern to every cell
    mHSS_FANCY_PRINTF(LOG_NORMAL, "Write Seed Pattern to all cells" CRLF);

    for (pattern = 1u, offset = 0u; offset < numWords; pattern++, offset++) {
        baseAddr[offset] = pattern;
        HSS_ShowProgress(numWords, numWords - offset);

        if (uart_getchar(&rx_char, 0, false) && ((rx_char == '\003') || (rx_char == '\033'))) {
            goto do_return;
        }
    }
    HSS_ShowProgress(numWords, 0u); // clear progress indicator

    // check each location for pass, and ...
    mHSS_FANCY_PRINTF(LOG_NORMAL, "First Pass: Check each location" CRLF);

    for (pattern = 1u, offset = 0u; offset < numWords; pattern++, offset++) {
        if (baseAddr[offset] != pattern) {
            mHSS_FANCY_PRINTF(LOG_ERROR, "4: 0x%016p==0x%016llx vs expected 0x%016llx" CRLF,
                baseAddr + offset, baseAddr[offset], pattern);

            result = ((uint64_t *)&baseAddr[offset]);
            offset = numWords;
            break;
        }

        // if okay, invert the pattern
        if (result == NULL) {
            antiPattern = ~pattern;
            baseAddr[offset] = antiPattern;
        }

        HSS_ShowProgress(numWords, numWords - offset);

        if (uart_getchar(&rx_char, 0, false) && ((rx_char == '\003') || (rx_char == '\033'))) {
            goto do_return;
        }
    }
    HSS_ShowProgress(numWords, 0u); // clear progress indicator

    // check each location for the inverted pattern
    mHSS_FANCY_PRINTF(LOG_NORMAL, "Second Pass: Check each location" CRLF);
    if (result == NULL) {
        for (pattern = 1u, offset = 0u; offset < numWords; pattern++, offset++) {
            antiPattern = ~pattern;
            if (baseAddr[offset] != antiPattern) {
                mHSS_FANCY_PRINTF(LOG_ERROR, "5: 0x%016p==0x%016llx vs expected 0x%016llx" CRLF,
                    baseAddr + offset, baseAddr[offset], antiPattern);
                result = ((uint64_t *)&baseAddr[offset]);
                break;
            }

            HSS_ShowProgress(numWords, numWords - offset);

            if (uart_getchar(&rx_char, 0, false) && ((rx_char == '\003') || (rx_char == '\033'))) {
                goto do_return;
            }
        }
    }

do_return:
    HSS_ShowProgress(numWords, 0u); // clear progress indicator
    return result;
}


//
//
//
#define mGB_IN_BYTES (1024llu * 1024llu * 1024llu)
#define mMB_IN_BYTES (1024llu * 1024llu)

#include "ddr_service.h"

bool HSS_MemTestDDRFast(void)
{
    bool result = true;

    if (HSS_DDR_GetSize() >= mGB_IN_BYTES) {
        mHSS_FANCY_PRINTF(LOG_NORMAL, "DDR-Lo size is %lu GiB" CRLF,
            (uint32_t)(HSS_DDR_GetSize()/mGB_IN_BYTES));
    } else {
        mHSS_FANCY_PRINTF(LOG_NORMAL, "DDR-Lo size is %lu MiB" CRLF,
            (uint32_t)(HSS_DDR_GetSize()/mMB_IN_BYTES));
    }

    int perf_ctr_index = PERF_CTR_UNINITIALIZED;
    HSS_PerfCtr_Allocate(&perf_ctr_index, "MemTest(DDR32)");
    if ((HSS_MemTestDataBus((uint64_t *)HSS_DDR_GetStart()) != 0u)
            || (HSS_MemTestAddressBus((uint64_t *)HSS_DDR_GetStart(), HSS_DDR_GetSize()) != NULL)) {
        result = false;
    }
    HSS_PerfCtr_Lap(perf_ctr_index);

    if (HSS_DDRHi_GetSize() >= mGB_IN_BYTES) {
        mHSS_FANCY_PRINTF(LOG_NORMAL, "DDR-Hi size is %lu GiB" CRLF,
            (uint32_t)(HSS_DDRHi_GetSize()/mGB_IN_BYTES));
    } else {
        mHSS_FANCY_PRINTF(LOG_NORMAL, "DDR-Hi size is %lu MiB" CRLF,
            (uint32_t)(HSS_DDRHi_GetSize()/mMB_IN_BYTES));
    }

    //mHSS_FANCY_PRINTF(LOG_NORMAL, "DDRHi start is %p" CRLF, HSS_DDRHi_GetStart());

    HSS_PerfCtr_Allocate(&perf_ctr_index, "MemTest(DDR64)");
    if ((HSS_MemTestDataBus((uint64_t *)HSS_DDRHi_GetStart()) != 0u)
            || (HSS_MemTestAddressBus((uint64_t *)HSS_DDRHi_GetStart(), HSS_DDRHi_GetSize()) != NULL)) {
        result = false;
    }
    HSS_PerfCtr_Lap(perf_ctr_index);

    return result;
}

bool HSS_MemTestDDRFull(void)
{
    bool result = HSS_MemTestDDRFast();

    if (result) {
        if (HSS_MemTestDevice((uint64_t *)HSS_DDR_GetStart(), HSS_DDR_GetSize()) != NULL) {
            mHSS_FANCY_PRINTF(LOG_ERROR, "FAILED!" CRLF);
            result = false;
        }
    }

    return result;
}

bool HSS_MemTestDDR_Ex(volatile uint64_t *baseAddr, size_t numBytes)
{
    bool result = true;

    if ((HSS_MemTestDataBus(baseAddr) != 0u)
            || (HSS_MemTestAddressBus(baseAddr, numBytes) != NULL)
            || (HSS_MemTestDevice(baseAddr, numBytes) != NULL)) {
            mHSS_FANCY_PRINTF(LOG_ERROR, "FAILED!" CRLF);
        result = false;
    }

    return result;
}
