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

#include "uart_helper.h"
#include "hss_debug.h"
#include "hss_progress.h"

void HSS_ShowProgress(size_t totalNumTasks, size_t numTasksRemaining)
{
    static uint32_t oldProgressPercent = 101u;

    uint32_t progressPercent = 
        (uint32_t)(((totalNumTasks - numTasksRemaining) * 100u) / totalNumTasks);

    if (oldProgressPercent != progressPercent) {
        mHSS_PRINTF(" %03u%%" CR, progressPercent);
        oldProgressPercent = progressPercent;
    }
}

bool HSS_ShowTimeout(char const * const msg, uint32_t timeout_sec, uint8_t *pRcvBuf)
{
    bool keyPressedFlag = false;

    mHSS_PUTS(msg);
    mHSS_PRINTF("Timeout in %u seconds" CRLF, timeout_sec);
    mHSS_PUTC('.');

    if (uart_getchar(pRcvBuf, timeout_sec, true)) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Character %u pressed " CRLF, *pRcvBuf);

        if (*pRcvBuf != 27) { // ESC => done
            keyPressedFlag = true;
        }
    }

    mHSS_PUTS(CRLF);

    return keyPressedFlag;
}
