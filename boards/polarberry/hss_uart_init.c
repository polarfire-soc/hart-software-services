/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
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

#include "drivers/mss/mss_mmuart/mss_uart.h"
#include "uart_helper.h"

bool HSS_UARTInit(void)
{
    // initialise debug UART

    MSS_UART_init(HSS_UART_GetInstance(HSS_HART_E51), MSS_UART_115200_BAUD,
        MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    // default all UARTs to 115200 for now
    // subsequent OS loads can change these if needed...
    MSS_UART_init(HSS_UART_GetInstance(HSS_HART_U54_1), MSS_UART_115200_BAUD,
        MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    MSS_UART_init(HSS_UART_GetInstance(HSS_HART_U54_2), MSS_UART_115200_BAUD,
        MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    MSS_UART_init(HSS_UART_GetInstance(HSS_HART_U54_3), MSS_UART_115200_BAUD,
        MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    MSS_UART_init(HSS_UART_GetInstance(HSS_HART_U54_4), MSS_UART_115200_BAUD,
        MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    return true;
}