/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * 
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file Progress output utility for long-running functions
 * \brief Progress output utility for long-running functions
 *
 */

#include "config.h"
#include "hss_types.h"

#include "hss_debug.h"
#include "hss_progress.h"

void HSS_ShowProgress(size_t totalNumTasks, size_t numTasksRemaining)
{
    static uint32_t oldProgressPercent = 101u;

    uint32_t progressPercent = 
        (uint32_t)(((totalNumTasks - numTasksRemaining) * 100u) / totalNumTasks);

    if (oldProgressPercent != progressPercent) {
        mHSS_FANCY_PRINTF_EX(" % 3u%%" CR, progressPercent);
        oldProgressPercent = progressPercent;
    }
}
