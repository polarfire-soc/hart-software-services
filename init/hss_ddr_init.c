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

#include <assert.h>
#include <string.h>

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
#  include "hw_ddr_segs.h"
#  include "nwc/mss_nwc_init.h"
#endif

#if IS_ENABLED(CONFIG_PLATFORM_MPFS)
const struct segment {
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

const uint32_t seg_regs[2][8] = {
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

uint64_t seg_regOffset_to_addrOffset(uint32_t offset, const int segment_index)
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
                    "%14s: SEG%d_%d: offset address 0x%010lx, physical DDR 0x%08lx" CRLF, 
                    segment[j].description, i, j,
                    segment[j].baseAddr,
                    seg_regOffset_to_addrOffset(seg_regs[i][j], j));
            }
        }
    }
#endif

    return true;
}

/*!
 * \brief Hook for DDR Setup
 */
bool HSS_DDRInit(void)
{
    //mHSS_DEBUG_PRINTF(lOG_NORMAL, "Initializing DDR..." CRLF);
#if IS_ENABLED(CONFIG_PLATFORM_MPFS)
    assert(mss_nwc_init() == 0);
#endif

    return true;
}

