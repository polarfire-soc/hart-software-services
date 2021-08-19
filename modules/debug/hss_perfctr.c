/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file Debug logging
 * \brief Debug logging
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"
#include "hss_clock.h"
#include "hss_perfctr.h"

#include <assert.h>
#include <stdio.h>

#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
struct {
    int index;
    char const * pName;
    bool isAllocated;
    HSSTicks_t startTime;
    HSSTicks_t lapTime;
} perfCtrs[CONFIG_DEBUG_PERF_CTRS_NUM];
#endif

bool HSS_PerfCtr_Allocate(int *pIdx, char const * pName)
{
    bool result = false;

#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
    if ((*pIdx >= 0) && (*pIdx < ARRAY_SIZE(perfCtrs))) {
        if ((perfCtrs[*pIdx].isAllocated) && (perfCtrs[*pIdx].pName == pName)) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "found already allocated perf ctr for >>%s<<" CRLF, pName);
            result = true;
        }
    } else {
        assert(pIdx);

        *pIdx = PERF_CTR_UNINITIALIZED;

        for (int index = 0; index < ARRAY_SIZE(perfCtrs); index++) {
            if (!perfCtrs[index].isAllocated) {
                perfCtrs[index].isAllocated = true;
                perfCtrs[index].pName = pName;
                result = true;
                perfCtrs[index].startTime = HSS_GetTime();
                *pIdx = index;
                break;
            }
        }

        if (!result) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "failed to allocate perf ctr for >>%s<<" CRLF, pName);
        }
    }
#endif

    return result;
}

void HSS_PerfCtr_Deallocate(int index)
{
    if (index != PERF_CTR_UNINITIALIZED) {
#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
        assert(index < ARRAY_SIZE(perfCtrs));

        if ((index >= 0) && (perfCtrs[index].isAllocated)) {
            perfCtrs[index].isAllocated = false;
            perfCtrs[index].pName = NULL;
        }
#endif
    }
}

void HSS_PerfCtr_Start(int index)
{
    if (index != PERF_CTR_UNINITIALIZED) {
#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
        assert(index < ARRAY_SIZE(perfCtrs));
        if (index >= 0) {
            perfCtrs[index].startTime = HSS_GetTime();
        }
#endif
    }
}

void HSS_PerfCtr_Lap(int index)
{
    if (index != PERF_CTR_UNINITIALIZED) {
#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
        assert(index < ARRAY_SIZE(perfCtrs));
        if (index >= 0) {
            perfCtrs[index].lapTime = HSS_GetTime();
        }
#endif
    }
}

HSSTicks_t HSS_PerfCtr_GetTime(int index)
{
    HSSTicks_t result = 0u;

    if (index != PERF_CTR_UNINITIALIZED) {
#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
        assert(index < ARRAY_SIZE(perfCtrs));

        if (index >= 0) {
            result = perfCtrs[index].lapTime - perfCtrs[index].startTime;
        }
#endif
    }

    return result;
}

void HSS_PerfCtr_DumpAll(void)
{
#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
    for (int i = 0; i < ARRAY_SIZE(perfCtrs); i++) {
        if (perfCtrs[i].isAllocated) {
            size_t ticks = HSS_PerfCtr_GetTime(i);
            size_t millisecs = (ticks + (TICKS_PER_MILLISEC/2)) / TICKS_PER_MILLISEC;

            mHSS_DEBUG_PRINTF(LOG_NORMAL, "% 8lu ms (% 8lu ticks) - %s" CRLF,
                millisecs, ticks, perfCtrs[i].pName ? perfCtrs[i].pName : "(null)");
        }
    }
#endif
}
