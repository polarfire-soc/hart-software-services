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

#include <assert.h>

void HSS_Debug_Timestamp(void)
{
    HSS_Debug_Highlight(HSS_DEBUG_LOG_TIMESTAMP);
    (void)sbi_printf("[%" PRIu64, HSS_GetTime() / ONE_SEC);
    (void)sbi_printf(".%05" PRIu64 "]", HSS_GetTime() % ONE_SEC);
}

void HSS_Debug_Highlight(HSS_Debug_LogLevel_t logLevel)
{
#if IS_ENABLED(CONFIG_COLOR_OUTPUT)
    static const char fancyTimestampHighlight[]       = "\033[32m";
    static const char fancyFunctionHighlight[]        = "\033[33m";

    static const char fancyErrorHighlight[]           = "\033[1;31m";
    static const char fancyWarnHighlight[]            = "\033[1;33m";
    static const char fancyStatusHighlight[]          = "\033[1;32m";
    static const char fancyNormalHighlight[]          = "\033[0m";
    static const char fancyStateTransitionHighlight[] = "\033[1;34m";

    char const * pHighlight;

    switch (logLevel) {
    default:
        __attribute__((fallthrough)); // deliberate fallthrough
    case HSS_DEBUG_LOG_NORMAL:
        pHighlight = fancyNormalHighlight;
        break;

    case HSS_DEBUG_LOG_FUNCTION:
        pHighlight = fancyFunctionHighlight;
        break;

    case HSS_DEBUG_LOG_TIMESTAMP:
        pHighlight = fancyTimestampHighlight;
        break;

    case HSS_DEBUG_LOG_ERROR:
        pHighlight = fancyErrorHighlight;
        break;

    case HSS_DEBUG_LOG_WARN:
        pHighlight = fancyWarnHighlight;
        break;

    case HSS_DEBUG_LOG_STATUS:
        pHighlight = fancyStatusHighlight;
        break;

    case HSS_DEBUG_LOG_STATE_TRANSITION:
        pHighlight = fancyStateTransitionHighlight;
        break;
    }

    mHSS_PUTS(pHighlight);
#else
    (void)logLevel;
#endif
}
