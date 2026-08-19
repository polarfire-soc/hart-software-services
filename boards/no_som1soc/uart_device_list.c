/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Allow overrides (per board) of default UART ports
 */

#include "config.h"
#include "hss_types.h"

#include <assert.h>

#include "hss_debug.h"

#include "drivers/mss/mss_mmuart/mss_uart.h"
#include <string.h>
#include <stdint.h>

#include "hss_boot_init.h"
#include "uart_helper.h"

extern mss_uart_instance_t * const pUartDeviceList[];

void *HSS_UART_GetInstance(int hartid)
{
    mss_uart_instance_t *pUart;
    const bool postBoot = HSS_BootInit_IsPostInit();

    switch (hartid) {
    default:
        pUart = pUartDeviceList[0];
        break;

    case HSS_HART_E51:
        if (postBoot) {
            pUart = pUartDeviceList[CONFIG_UART_POST_BOOT];
        } else {
            pUart = pUartDeviceList[0];
        }
        break;

    case HSS_HART_U54_1:
        pUart = pUartDeviceList[1];
        break;

    case HSS_HART_U54_2:
        pUart = pUartDeviceList[2];
        break;

    case HSS_HART_U54_3:
        pUart = pUartDeviceList[3];
        break;

    case HSS_HART_U54_4:
        pUart = pUartDeviceList[4];
        break;
    }

    return pUart;
}
