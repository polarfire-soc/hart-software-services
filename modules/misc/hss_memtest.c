/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
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
static void log_error_(const int count, volatile void *ptr, const uint64_t value, const uint64_t expected)
{
    mHSS_FANCY_PRINTF(LOG_ERROR, "%d: 0x%016p==0x%016llx vs expected 0x%016llx\n",
        count, ptr, value, expected);
}

static bool check_if_interrupted(void)
{
    uint8_t rx_char;
    return (uart_getchar(&rx_char, 0, false) && ((rx_char == '\003') || (rx_char == '\033')));
}

static uint64_t* HSS_MemTestAddressBus(volatile uint64_t *baseAddr, const size_t numBytes)
{
    const size_t numWords = numBytes / sizeof(uint64_t);
    const size_t addrMask = numWords - 1u;
    size_t offset;
    size_t testOffset;
    uint64_t* result = NULL;

    const uint64_t pattern = (uint64_t)0xAAAAAAAAAAAAAAAAu;
    const uint64_t antiPattern = (uint64_t)0x5555555555555555u;

    // Walking up address bus, setting cells to pattern
    for (offset = 1u; (offset & addrMask) != 0u; offset <<= 1) {
        baseAddr[offset] = pattern;

        if (check_if_interrupted()) {
            goto do_return;
        }
    }

    testOffset = 0u;
    baseAddr[0] = antiPattern;

    // Walking up address bus, verifying set cells
    for (offset = 1u; (offset & addrMask) != 0u; offset <<= 1) {
        if (baseAddr[offset] != pattern) {
            log_error_(1, baseAddr + offset, baseAddr[offset], pattern);
            result = ((uint64_t *)&baseAddr[offset]);
            break;
        }

        if (check_if_interrupted()) {
            goto do_return;
        }
    }

    baseAddr[0] = pattern;

    // Walking up address bus, setting cells to antipattern
    if (result == NULL) {
        for (testOffset = 1u; (testOffset & addrMask) != 0u; testOffset <<= 1) {
            baseAddr[testOffset] = antiPattern;

            // ensure that the base isn't affected
            if (baseAddr[0] != pattern) {
                log_error_(2, baseAddr + offset, baseAddr[offset], pattern);
                result = ((uint64_t *)&baseAddr[testOffset]);
                break;
            }

            if (result == NULL) {
                // walk up the address bus, ensuring that no other address is affected
                for (offset = 1u; (offset & addrMask) != 0u; offset <<= 1) {
                    if ((baseAddr[offset] != pattern) && (offset != testOffset)) {
                        log_error_(3, baseAddr + offset, baseAddr[offset], pattern);
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

            if (check_if_interrupted()) {
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

    // write pattern to every cell
    mHSS_FANCY_PRINTF(LOG_NORMAL, "Write Seed Pattern to all cells\n");

    for (pattern = 1u, offset = 0u; offset < numWords; pattern++, offset++) {
        baseAddr[offset] = pattern;
        HSS_ShowProgress(numWords, numWords - offset);

        if (check_if_interrupted()) {
            goto do_return;
        }
    }
    HSS_ShowProgress(numWords, 0u); // clear progress indicator

    // check each location for pass, and ...
    mHSS_FANCY_PRINTF(LOG_NORMAL, "First Pass: Check each location\n");

    for (pattern = 1u, offset = 0u; offset < numWords; pattern++, offset++) {
        if (baseAddr[offset] != pattern) {
            log_error_(4, baseAddr + offset, baseAddr[offset], pattern);

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

        if (check_if_interrupted()) {
            goto do_return;
        }
    }
    HSS_ShowProgress(numWords, 0u); // clear progress indicator

    // check each location for the inverted pattern
    mHSS_FANCY_PRINTF(LOG_NORMAL, "Second Pass: Check each location\n");
    if (result == NULL) {
        for (pattern = 1u, offset = 0u; offset < numWords; pattern++, offset++) {
            antiPattern = ~pattern;
            if (baseAddr[offset] != antiPattern) {
                log_error_(5, baseAddr + offset, baseAddr[offset], pattern);
                result = ((uint64_t *)&baseAddr[offset]);
                break;
            }

            HSS_ShowProgress(numWords, numWords - offset);

            if (check_if_interrupted()) {
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
#define mMiB_IN_BYTES (1024llu * 1024llu)

#include "ddr_service.h"

bool HSS_MemTestDDRFast(void)
{
    bool result = true;

    mHSS_FANCY_PRINTF(LOG_NORMAL, "DDR-Lo size is % 4lu MiB\n",
        (uint32_t)(HSS_DDR_GetSize()/mMiB_IN_BYTES));

    static int perf_ctr_index_mem32 = PERF_CTR_UNINITIALIZED;
    static int perf_ctr_index_mem64 = PERF_CTR_UNINITIALIZED;
    HSS_PerfCtr_Allocate(&perf_ctr_index_mem32, "MemTest(DDR32)");
    if ((HSS_MemTestDataBus((uint64_t *)HSS_DDR_GetStart()) != 0u)
            || (HSS_MemTestAddressBus((uint64_t *)HSS_DDR_GetStart(), HSS_DDR_GetSize()) != NULL)) {
        result = false;
    }
    HSS_PerfCtr_Lap(perf_ctr_index_mem32);

    mHSS_FANCY_PRINTF(LOG_NORMAL, "DDR-Hi size is % 4lu MiB\n",
        (uint32_t)(HSS_DDRHi_GetSize()/mMiB_IN_BYTES));

    HSS_PerfCtr_Allocate(&perf_ctr_index_mem64, "MemTest(DDR64)");
    if ((HSS_MemTestDataBus((uint64_t *)HSS_DDRHi_GetStart()) != 0u)
            || (HSS_MemTestAddressBus((uint64_t *)HSS_DDRHi_GetStart(), HSS_DDRHi_GetSize()) != NULL)) {
        result = false;
    }
    HSS_PerfCtr_Lap(perf_ctr_index_mem64);

    return result;
}

bool HSS_MemTestDDRFull(void)
{
    bool result = HSS_MemTestDDRFast();

    if (result) {
        if (HSS_MemTestDevice((uint64_t *)HSS_DDR_GetStart(), HSS_DDR_GetSize()) != NULL) {
            //mHSS_FANCY_PRINTF(LOG_ERROR, "FAILED!\n");
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
            //mHSS_FANCY_PRINTF(LOG_ERROR, "FAILED!\n");
        result = false;
    }

    return result;
}
