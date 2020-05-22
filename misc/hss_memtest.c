/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
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
/*static*/ uint64_t* HSS_MemTestAddressBus(volatile uint64_t *baseAddr, const size_t numBytes)
{
    const size_t numWords = numBytes / sizeof(uint64_t);
    const size_t addrMask = numWords - 1u;
    size_t offset;
    size_t testOffset;
    uint64_t* result = NULL;

    const uint64_t pattern = (uint64_t)0xAAAAAAAAAAAAAAAAu;
    const uint64_t antiPattern = (uint64_t)0x5555555555555555u;

    //mHSS_FANCY_PRINTF("Walking up address bus, setting cells to pattern" CRLF);
    for (offset = 1u; (offset & addrMask) != 0u; offset <<= 1) {
        baseAddr[offset] = pattern;
    }

    testOffset = 0u;
    baseAddr[0] = antiPattern;

    //mHSS_FANCY_PRINTF("Walking up address bus, verifying set cells" CRLF);
    for (offset = 1u; (offset & addrMask) != 0u; offset <<= 1) {
        if (baseAddr[offset] != pattern) {
            mHSS_FANCY_ERROR_TEXT;
            mHSS_FANCY_PRINTF("1: 0x%016p==0x%016llx vs expected 0x%016llx" CRLF, 
                baseAddr + offset, baseAddr[offset], pattern);
            mHSS_FANCY_NORMAL_TEXT;
            result = ((uint64_t *)&baseAddr[offset]);
            break;
        }
    }

    baseAddr[0] = pattern;

    //mHSS_FANCY_PRINTF("Walking up address bus, setting cells to antipattern" CRLF);
    if (result == NULL) {
        for (testOffset = 1u; (testOffset & addrMask) != 0u; testOffset <<= 1) {
            baseAddr[testOffset] = antiPattern;

            // ensure that the base isn't affected
            if (baseAddr[0] != pattern) {
                mHSS_FANCY_ERROR_TEXT;
                mHSS_FANCY_PRINTF("2: 0x%016p==0x%016llx vs expected 0x%016llx" CRLF, 
                    baseAddr + offset, baseAddr[offset], pattern);
                mHSS_FANCY_NORMAL_TEXT;
                result = ((uint64_t *)&baseAddr[testOffset]);
                break;
            }

            if (result == NULL) {
                // walk up the address bus, ensuring that no other address is affected
                for (offset = 1u; (offset & addrMask) != 0u; offset <<= 1) {
                    if ((baseAddr[offset] != pattern) && (offset != testOffset)) {
                        mHSS_FANCY_ERROR_TEXT;
                        mHSS_FANCY_PRINTF("3: 0x%016p==0x%016llx vs expected 0x%016llx" CRLF, 
                            baseAddr + offset, baseAddr[offset], pattern);
                        mHSS_FANCY_NORMAL_TEXT;
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
        }
    }

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
    mHSS_FANCY_PRINTF("Write Seed Pattern to all cells" CRLF);

    for (pattern = 1u, offset = 0u; offset < numWords; pattern++, offset++) {
        baseAddr[offset] = pattern;
        HSS_ShowProgress(numWords, numWords - offset); 
    }

    // check each location for pass, and ...
    mHSS_FANCY_PRINTF("First Pass: Check each location" CRLF);

    for (pattern = 1u, offset = 0u; offset < numWords; pattern++, offset++) {
        if (baseAddr[offset] != pattern) {
            mHSS_FANCY_ERROR_TEXT;
            mHSS_FANCY_PRINTF("4: 0x%016p==0x%016llx vs expected 0x%016llx" CRLF, 
                baseAddr + offset, baseAddr[offset], pattern);
            mHSS_FANCY_NORMAL_TEXT;

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
    }

    // check each location for the inverted pattern
    mHSS_FANCY_PRINTF("Second Pass: Check each location" CRLF);
    if (result == NULL) {
        for (pattern = 1u, offset = 0u; offset < numWords; pattern++, offset++) {
            antiPattern = ~pattern;
            if (baseAddr[offset] != antiPattern) {
                mHSS_FANCY_ERROR_TEXT;
                mHSS_FANCY_PRINTF("5: 0x%016p==0x%016llx vs expected 0x%016llx" CRLF, 
                    baseAddr + offset, baseAddr[offset], antiPattern);
                mHSS_FANCY_NORMAL_TEXT;
                result = ((uint64_t *)&baseAddr[offset]);
                break;
            }

            HSS_ShowProgress(numWords, numWords - offset); 
        }
    }

    return result;
}


//
//
//
extern const uint64_t __ddr_start;
extern const uint64_t __ddr_end;

#define mGB_IN_BYTES (1024llu * 1024llu * 1024llu)
#define mMB_IN_BYTES (1024llu * 1024llu)

//
// We only want to specify the DDR location and size in one place - the linker script
//
// ddr_end is too high for the RV64 toolchain, sadly.  We get an error message concerning 
// "relocation truncated to fit: R_RISCV_PCREL_HI20 against symbol `__ddr_end' defined in 
//  .text.init section" as it is above 4GiB away from eNVM.
//
// See https://github.com/riscv/riscv-gnu-toolchain/issues/103 for background.
// 
// So we can't easily do ...
//
//     extern uint64_t __ddr_end;
//     const ptrdiff_t ddr_size = (size_t)((char *)&__ddr_end - (char *)&__ddr_start);
//
// However, we can workaround by using the GNU assembler to store the DDR size into a 64-bit memory 
// location and use this size in our C code
//
asm(".align 3\n"
    "__ddr_size: .quad (__ddr_end-__ddr_start)\n"
    ".globl   __ddr_size\n");
extern const size_t __ddr_size;

bool HSS_MemTestDDRFast(void)
{
    bool result = true;

    if (__ddr_size > mGB_IN_BYTES) {
        mHSS_FANCY_PRINTF("DDR size is %lu GiB" CRLF, (uint32_t)(__ddr_size/mGB_IN_BYTES));
    } else {
        mHSS_FANCY_PRINTF("DDR size is %lu MiB" CRLF, (uint32_t)(__ddr_size/mMB_IN_BYTES));
    }

    if ((HSS_MemTestDataBus((uint64_t *)&__ddr_start) != 0u) 
            || (HSS_MemTestAddressBus((uint64_t *)&__ddr_start, __ddr_size) != NULL)) {
        result = false;
    }

    return result;
}

bool HSS_MemTestDDRFull(void)
{
    bool result = HSS_MemTestDDRFast();

    if (result) {
        if (HSS_MemTestDevice((uint64_t *)&__ddr_start, __ddr_size) != NULL) {
            mHSS_FANCY_PRINTF("FAILED!" CRLF);
            result = false;
        }
    }

    return result;
}
