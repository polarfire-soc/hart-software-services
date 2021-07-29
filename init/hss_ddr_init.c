/*******************************************************************************
 * Copyright 2019-2021 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS DDR Initalization
 * \brief DDR Initialization
 */

#include "config.h"
#include "hss_types.h"

#include "hss_debug.h"
#include "hss_perfctr.h"

#include <assert.h>
#include <string.h>

#if IS_ENABLED(CONFIG_PLATFORM_MPFS)
#  include "ddr/hw_ddr_segs.h"
#  include "nwc/mss_nwc_init.h"
#  include "mss_hal.h"
#  include "mss_l2_cache.h"
#endif


#include "hss_init.h"

/*!
 * \brief DDR Training
 *
 * The E51 ensures that DDR is setup prior to code download,
 * and thus perform an DDR training and configuration required to achieve this.
 *
 * The intention is to allow as much flexibility as possible in DDR training,
 * so it is driven by MPFS HSS Embedded Software, with hardware hooks to
 * perform real-time critical functions.
 *
 * TBD: is periodic re-calibration required during operation (e.g. temperature induced
 * or other)
 */

#if IS_ENABLED(CONFIG_PLATFORM_MPFS)
static const struct segment {
    char const * const description;
    uint64_t const baseAddr;
} segment[8] = {
    { "Cached",         0x0080000000lu },
    { "Cached",         0x1000000000lu },
    { "Non-cached",     0x00c0000000lu },
    { "Non-cached",     0x1400000000lu },
    { "Non-cached WCB", 0x00d0000000lu },
    { "Non-cached WCB", 0x1800000000lu },
    { "Trace",          0x0000000000lu },
    { "DDR blocker",    0x0000000000lu }
};

static const uint32_t seg_regs[2][8] = {
    {
        LIBERO_SETTING_SEG0_0, LIBERO_SETTING_SEG0_1, LIBERO_SETTING_SEG0_2, LIBERO_SETTING_SEG0_3,
        LIBERO_SETTING_SEG0_4, LIBERO_SETTING_SEG0_5, LIBERO_SETTING_SEG0_6, LIBERO_SETTING_SEG0_7
    },
    {
        LIBERO_SETTING_SEG1_0, LIBERO_SETTING_SEG1_1, LIBERO_SETTING_SEG1_2, LIBERO_SETTING_SEG1_3,
        LIBERO_SETTING_SEG1_4, LIBERO_SETTING_SEG1_5, LIBERO_SETTING_SEG1_6, LIBERO_SETTING_SEG1_7
    }
};
#endif

static uint64_t seg_regOffset_to_addrOffset_(uint32_t offset, const int segment_index);
static uint64_t seg_regOffset_to_addrOffset_(uint32_t offset, const int segment_index)
{
#if IS_ENABLED(CONFIG_PLATFORM_MPFS)
    uint64_t result = 0u;

    if (offset & (1u << 14)) {
        offset = offset & 0x3FFFu;
        result = segment[segment_index].baseAddr - ((0x4000lu - offset) << 24);
        }

    return result;
#endif
}

bool HSS_DDRPrintSegConfig(void)
{
#if IS_ENABLED(CONFIG_PLATFORM_MPFS)
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Segment Configuration:" CRLF);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 8; j++) {
            if (seg_regs[i][j] & 0xFFFFu) {
                mHSS_DEBUG_PRINTF_EX(
                    "%14s: SEG%d_%d: offset 0x%010lx, physical DDR 0x%08lx" CRLF,
                    segment[j].description, i, j,
                    segment[j].baseAddr,
                    seg_regOffset_to_addrOffset_(seg_regs[i][j], j));
            }
        }
    }
#endif

    return true;
}

//
// We use the GCC intrinsic __builtin_popcount() to count cache way bits set
// if we don't have an implementation for __popcountdi2, we'll use the weakly
// bound one here, which does some common bit tricks
 __attribute__((weak)) int64_t __popcountdi2(int64_t n);
__attribute__((weak)) int64_t __popcountdi2(int64_t n)
{
    n = n - ((n >> 1) & 0x5555555555555555ul);
    n = ((n >> 2) & 0x3333333333333333ul) + (n & 0x3333333333333333ul);
    n = ((n >> 4) + n)  & 0x0f0f0f0f0f0f0f0ful;

    n = ((n >> 32) + n); // & 0x00000000fffffffful;
    n = ((n >> 16) + n); // & 0x000000000000fffful;
    n = ((n >> 8) + n)  & 0x000000000000007ful;

    return n;
}

bool HSS_DDRPrintL2CacheConfig(void)
{
#if IS_ENABLED(CONFIG_PLATFORM_MPFS)
    mHSS_DEBUG_PRINTF(LOG_STATUS, "L2 Cache Configuration:" CRLF);

    //unsigned int way_enable = LIBERO_SETTING_NUM_SCRATCH_PAD_WAYS;
    //int cache_ways = (unsigned int)(way_enable - LIBERO_SETTING_NUM_SCRATCH_PAD_WAYS);

    const unsigned int way_enable = CACHE_CTRL->WAY_ENABLE + 1u;
    const unsigned int way_mask = (1u << way_enable) - 1u;
    const unsigned int cache_ways = __builtin_popcount(CACHE_CTRL->WAY_MASK_E51_DCACHE & way_mask);
    const unsigned int scratch_ways = way_enable - cache_ways;
    const unsigned int lim_ways =  (unsigned int)(16u - way_enable);

    assert(way_enable <= 16u);
    assert(cache_ways <= 16u);

    mHSS_DEBUG_PRINTF_EX("    L2-Scratchpad: % 2u way%c (%u KiB)" CRLF, scratch_ways, scratch_ways==1u ? ' ':'s', scratch_ways * 128u);
    mHSS_DEBUG_PRINTF_EX("         L2-Cache: % 2u way%c (%u KiB)" CRLF, cache_ways, cache_ways==1u ? ' ':'s', cache_ways * 128u);
    mHSS_DEBUG_PRINTF_EX("           L2-LIM: % 2u way%c (%u KiB)" CRLF, lim_ways, lim_ways==1 ? ' ':'s', lim_ways * 128);

#define CONFIG_DUMP_L2CACHE_WAYMASKS 1
#    if IS_ENABLED(CONFIG_DUMP_L2CACHE_WAYMASKS)
    // enable these if you want extra debug on the way mask settings...
    struct {
         char const * const name;
         uint64_t volatile const * const pWayMask;
    } wayMaskTable[] = {
        { .name = "DMA",               .pWayMask = &(CACHE_CTRL->WAY_MASK_DMA) },
        { .name = "AXI4_SLAVE_PORT_0", .pWayMask = &(CACHE_CTRL->WAY_MASK_AXI4_SLAVE_PORT_0) },
        { .name = "AXI4_SLAVE_PORT_1", .pWayMask = &(CACHE_CTRL->WAY_MASK_AXI4_SLAVE_PORT_1) },
        { .name = "AXI4_SLAVE_PORT_2", .pWayMask = &(CACHE_CTRL->WAY_MASK_AXI4_SLAVE_PORT_2) },
        { .name = "AXI4_SLAVE_PORT_3", .pWayMask = &(CACHE_CTRL->WAY_MASK_AXI4_SLAVE_PORT_3) },
        { .name = "E51_ICACHE",        .pWayMask = &(CACHE_CTRL->WAY_MASK_E51_ICACHE) },
        { .name = "U54_1_DCACHE",      .pWayMask = &(CACHE_CTRL->WAY_MASK_U54_1_DCACHE) },
        { .name = "U54_1_ICACHE",      .pWayMask = &(CACHE_CTRL->WAY_MASK_U54_1_ICACHE) },
        { .name = "U54_2_DCACHE",      .pWayMask = &(CACHE_CTRL->WAY_MASK_U54_2_DCACHE) },
        { .name = "U54_2_ICACHE",      .pWayMask = &(CACHE_CTRL->WAY_MASK_U54_2_ICACHE) },
        { .name = "U54_3_DCACHE",      .pWayMask = &(CACHE_CTRL->WAY_MASK_U54_3_DCACHE) },
        { .name = "U54_3_ICACHE",      .pWayMask = &(CACHE_CTRL->WAY_MASK_U54_3_ICACHE) },
        { .name = "U54_4_DCACHE",      .pWayMask = &(CACHE_CTRL->WAY_MASK_U54_4_DCACHE) },
        { .name = "U54_4_ICACHE",      .pWayMask = &(CACHE_CTRL->WAY_MASK_U54_4_ICACHE) },
        { .name = "E51_DCACHE",        .pWayMask = &(CACHE_CTRL->WAY_MASK_E51_DCACHE) }
    };

    mHSS_DEBUG_PRINTF(LOG_STATUS, "L2 Cache Way Masks:" CRLF);
    for (int i = 0; i < ARRAY_SIZE(wayMaskTable); i++) {
        assert(wayMaskTable[i].name);
        assert(wayMaskTable[i].pWayMask);
	mHSS_DEBUG_PRINTF_EX("% 17s: 0x%x" CRLF, wayMaskTable[i].name, *(wayMaskTable[i].pWayMask));
    }
#    endif
#endif
    return true;
}

/*!
 * \brief Hook for DDR Setup
 */
bool HSS_DDRInit(void)
{
#if IS_ENABLED(CONFIG_PLATFORM_MPFS)
    int perf_ctr_index = PERF_CTR_UNINITIALIZED;
    HSS_PerfCtr_Allocate(&perf_ctr_index, "NWC Init");
    assert(mss_nwc_init() == 0);
    HSS_PerfCtr_Lap(perf_ctr_index);
#endif

    return true;
}
