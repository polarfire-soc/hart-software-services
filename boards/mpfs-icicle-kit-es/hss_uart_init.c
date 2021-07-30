/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
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

#include "drivers/mss_uart/mss_uart.h"

bool HSS_UARTInit(void)
{
    // initialise debug UART

#if IS_ENABLED(CONFIG_PLATFORM_MPFS)
    MSS_UART_init(&g_mss_uart0_lo, MSS_UART_115200_BAUD,
        MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    // default all UARTs to 115200 for now
    // subsequent OS loads can change these if needed...
    MSS_UART_init(&g_mss_uart1_lo, MSS_UART_115200_BAUD,
        MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    MSS_UART_init(&g_mss_uart2_lo, MSS_UART_115200_BAUD,
        MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    MSS_UART_init(&g_mss_uart3_lo, MSS_UART_115200_BAUD,
        MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    MSS_UART_init(&g_mss_uart4_lo, MSS_UART_115200_BAUD,
        MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);
#else
#  error Unknown PLATFORM
#endif

    return true;
}
