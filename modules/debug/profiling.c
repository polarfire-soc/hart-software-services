/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
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

struct Node {
    void *pFunc;
    uint64_t entryTime;
    uint64_t timeCount;
} functionNode[CONFIG_DEBUG_PROFILING_MAX_NUM_FUNCTIONS] = { 0 };
size_t allocationCount = 0u;

extern void g5soc_init_mutex(volatile uint32_t * p_mutex);
extern void g5soc_take_mutex(volatile uint32_t * p_mutex);
extern void g5soc_release_mutex(volatile uint32_t * p_mutex);

volatile uint32_t profiling_mutex = 0u;

#define mATOMIC_ENTER g5soc_take_mutex(&profiling_mutex);
#define mATOMIC_EXIT g5soc_release_mutex(&profiling_mutex);

void __attribute__((no_instrument_function)) __cyg_profile_func_enter (void *pFunc, void *pCaller)
{
    enum HSSHartId const myHartId = current_hartid();
    (void) pCaller;

    if (myHartId != 0) { return; }

    assert(pFunc != NULL);
    struct Node *pNode = NULL;

    size_t i;
    for (i = 0u; i < allocationCount; i++) {
        if (functionNode[i].pFunc == pFunc) {
            pNode = &(functionNode[i]);
            break;
        }
    }

    if (!pNode) {
        assert(allocationCount < ARRAY_SIZE(functionNode));
        pNode = &(functionNode[allocationCount]);
        allocationCount++;
        mATOMIC_ENTER;
        pNode->pFunc = pFunc;
        pNode->timeCount = 0lu;
        mATOMIC_EXIT;
    }

    if (pNode) {
        mATOMIC_ENTER;
        pNode->entryTime = CSR_GetTickCount();
        mATOMIC_EXIT;
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
        if (functionNode[i].pFunc == pFunc) {
            mATOMIC_ENTER;
            functionNode[i].timeCount += (CSR_GetTickCount() - functionNode[i].entryTime);
            mATOMIC_EXIT;
            break;
        }
    }

    return;
}


void __attribute__((no_instrument_function)) dump_profile(void);

void __attribute__((no_instrument_function)) dump_profile(void)
{
    size_t i;
    for (i = 0u; i < allocationCount; i++) {
        mHSS_DEBUG_PRINTF_EX("PROFILE: %p, %lu" CRLF, functionNode[i].pFunc, functionNode[i].timeCount);
    }
}
