/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
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
#include "hss_clock.h"
#include "hss_progress.h"

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
    } else {
        result = segment[segment_index].baseAddr;
    }

    return result;
#endif
}

bool HSS_DDRPrintSegConfig(void)
{
#if IS_ENABLED(CONFIG_PLATFORM_MPFS)
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Segment Configuration:\n");
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 8; j++) {
            if (seg_regs[i][j] & 0xFFFFu) {
                mHSS_DEBUG_PRINTF_EX(
                    "%14s: SEG%d_%d: offset 0x%010lx, physical DDR 0x%08lx\n",
                    segment[j].description, i, j,
                    segment[j].baseAddr,
                    seg_regOffset_to_addrOffset_(seg_regs[i][j], j));
            }
        }
    }
#endif

    return true;
}

bool HSS_DDRPrintL2CacheWaysConfig(void)
{
    mHSS_DEBUG_PRINTF(LOG_STATUS, "L2 Cache Configuration:\n");

    const unsigned int way_enable = CACHE_CTRL->WAY_ENABLE + 1u;
    const unsigned int way_mask = (1u << way_enable) - 1u;
    const unsigned int cache_ways = __builtin_popcount(CACHE_CTRL->WAY_MASK_E51_DCACHE & way_mask);
    const unsigned int scratch_ways = way_enable - cache_ways;
    const unsigned int lim_ways =  (unsigned int)(16u - way_enable);

    assert(way_enable <= 16u);
    assert(cache_ways <= 16u);

    mHSS_DEBUG_PRINTF_EX("    L2-Scratchpad: % 2u way%c (%u KiB)\n", scratch_ways, scratch_ways==1u ? ' ':'s', scratch_ways * 128u);
    mHSS_DEBUG_PRINTF_EX("         L2-Cache: % 2u way%c (%u KiB)\n", cache_ways, cache_ways==1u ? ' ':'s', cache_ways * 128u);
    mHSS_DEBUG_PRINTF_EX("           L2-LIM: % 2u way%c (%u KiB)\n", lim_ways, lim_ways==1 ? ' ':'s', lim_ways * 128);

    // sanity check on L2 Size and LIM size...
    // to ensure Libero and the HSS linker script match
    extern const uint64_t __l2_start;
    extern const uint64_t _hss_start;
    extern const uint64_t _hss_end;
    const uintptr_t libero_l2_end = (uintptr_t)&__l2_start + (scratch_ways * 128u * 1024u);

    assert(&_hss_start == &__l2_start);
    assert((uintptr_t)&_hss_end <= libero_l2_end);

    return true;
}

bool HSS_DDRPrintL2CacheWayMasks(void)
{
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

    mHSS_DEBUG_PRINTF(LOG_STATUS, "L2 Cache Way Masks:\n");
    for (int i = 0; i < ARRAY_SIZE(wayMaskTable); i++) {
        assert(wayMaskTable[i].name);
        assert(wayMaskTable[i].pWayMask);
	mHSS_DEBUG_PRINTF_EX("% 17s: 0x%x\n", wayMaskTable[i].name, *(wayMaskTable[i].pWayMask));
    }

    return true;
}

/*!
 * \brief Hook for DDR Setup
 */

static size_t ddr_training_progress = 0;
#define TYPICAL_DDR_TRAINING_ITERATIONS 5u

bool HSS_DDRInit(void)
{
    bool result = true;
#if IS_ENABLED(CONFIG_PLATFORM_MPFS)
    if (!IS_ENABLED(CONFIG_SKIP_DDR)) {
	const char ddr_training_prefix[] = "DDR training ...";

        sbi_printf("%s", ddr_training_prefix);
        sbi_printf("\n");
        int perf_ctr_index = PERF_CTR_UNINITIALIZED;
        HSS_PerfCtr_Allocate(&perf_ctr_index, "DDR Init");

#    define CURSOR_UP "\033[A"
        HSS_ShowProgress(TYPICAL_DDR_TRAINING_ITERATIONS, TYPICAL_DDR_TRAINING_ITERATIONS);
        uint8_t retval = mss_nwc_init_ddr();
        HSS_ShowProgress(TYPICAL_DDR_TRAINING_ITERATIONS, 0u);

        if (retval != 0) {
            sbi_printf(CURSOR_UP "%s Failed\n", ddr_training_prefix);
            result = false;
        } else {
            HSS_PerfCtr_Lap(perf_ctr_index);
            sbi_printf(CURSOR_UP "%s Passed");
#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
            sbi_printf(" ( %d ms)", ddr_training_prefix,
                HSS_PerfCtr_GetTime(perf_ctr_index)/TICKS_PER_MILLISEC);
#endif
            sbi_printf("\n");
        }
        HSS_PerfCtr_Lap(perf_ctr_index);
#  endif
    }

    return result;
}


void ddr_report_progress(void) // override weak symbol to report training progress...
{
    HSS_ShowProgress(TYPICAL_DDR_TRAINING_ITERATIONS,
        (ddr_training_progress < TYPICAL_DDR_TRAINING_ITERATIONS) ?
            TYPICAL_DDR_TRAINING_ITERATIONS - ddr_training_progress : 1);

    ++ddr_training_progress;

    return;
}
