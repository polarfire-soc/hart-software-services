/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file Code Profiling
 * \brief Code Profiling
 */

#include "config.h"
#include "hss_types.h"
#include "csr_helper.h"

#include <assert.h>

struct ProfileNode {
    void *pFunc;
    uint64_t entryTime;
    uint64_t timeCount;
} profileStats[CONFIG_DEBUG_PROFILING_MAX_NUM_FUNCTIONS] = { 0 };
size_t allocationCount = 0u;

void __attribute__((no_instrument_function)) __cyg_profile_func_enter (void *pFunc, void *pCaller)
{
    enum HSSHartId const myHartId = current_hartid();
    (void) pCaller;

    if (myHartId != 0) { return; }

    assert(pFunc != NULL);
    struct ProfileNode *pProfileNode = NULL;

    size_t i;
    for (i = 0u; i < allocationCount; i++) {
        if (profileStats[i].pFunc == pFunc) {
            pProfileNode = &(profileStats[i]);
            break;
        }
    }

    if (!pProfileNode) {
        if (allocationCount < ARRAY_SIZE(profileStats)) {
            pProfileNode = &(profileStats[allocationCount]);
            allocationCount++;
            pProfileNode->pFunc = pFunc;
            pProfileNode->timeCount = 0lu;
        }
    }

    if (pProfileNode) {
        pProfileNode->entryTime = CSR_GetTickCount();
    }

    return;
}

void __attribute__((no_instrument_function)) __cyg_profile_func_exit (void *pFunc, void *pCaller)
{
    enum HSSHartId const myHartId = current_hartid();
    (void) pCaller;

    assert(pFunc != NULL);

    if (myHartId != 0) { return; }

    size_t i;
    for (i = 0u; i < allocationCount; i++) {
        if (profileStats[i].pFunc == pFunc) {
            profileStats[i].timeCount += (CSR_GetTickCount() - profileStats[i].entryTime);
            break;
        }
    }

    return;
}


void __attribute__((no_instrument_function)) HSS_Profile_DumpAll(void);
void __attribute__((no_instrument_function)) HSS_Profile_DumpAll(void)
{
    mHSS_DEBUG_PRINTF_EX("# Profile Information Dump\n# FuncPtr, TickCount\n");
    size_t i;
    for (i = 0u; i < allocationCount; i++) {
        mHSS_DEBUG_PRINTF_EX("%p, %lu\n", profileStats[i].pFunc, profileStats[i].timeCount);
    }
}

int64_t __attribute__((used)) __bswapdi2(uint64_t u);
int64_t __bswapdi2(uint64_t u) {
    return (
        (((u) & 0xff00000000000000ULL) >> 56) |
        (((u) & 0x00ff000000000000ULL) >> 40) |
        (((u) & 0x0000ff0000000000ULL) >> 24) |
        (((u) & 0x000000ff00000000ULL) >> 8)  |
        (((u) & 0x00000000ff000000ULL) << 8)  |
        (((u) & 0x0000000000ff0000ULL) << 24) |
        (((u) & 0x000000000000ff00ULL) << 40) |
        (((u) & 0x00000000000000ffULL) << 56));
}
