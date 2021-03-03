/*******************************************************************************
 * Copyright 2019-2021 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS Debug UART Initalization
 * \brief Debug UART Initialization
 */

#include "config.h"
#include "hss_types.h"
#include "hss_init.h"

#include <assert.h>

#include "hss_debug.h"

bool HSS_UARTInit(void)
{
    // initialise debug UART

#if defined(CONFIG_PLATFORM_FU540)
    char *pUartBase = (char *)0x10010000;
    *(uint32_t volatile *)(pUartBase + 0x08) = 1 | 2;
    *(uint32_t volatile *)(pUartBase + 0x10) = 0; // disable UART interrupts
    *(uint32_t volatile *)(pUartBase + 0x14) = 0; // clear pending UART interrupts

    pUartBase = (char *)0x10011000;
    *(uint32_t volatile *)(pUartBase + 0x08) = 1 | 2;
    *(uint32_t volatile *)(pUartBase + 0x10) = 0; // disable UART interrupts
    *(uint32_t volatile *)(pUartBase + 0x14) = 0; // clear pending UART interrupts
#else
#  error Unknown PLATFORM
#endif

    return true;
}
