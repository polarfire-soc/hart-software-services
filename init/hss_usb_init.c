/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS USB Initalization
 * \brief USB Initialization
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#include <string.h>
#include <sys/types.h>

#undef ROUNDUP
#undef ROUNDDOWN
#include "mss_hal.h"
#include "mss_assert.h"
#include "flash_drive_app.h"
#include "mss_plic.h"
#include "uart_helper.h"
#include "usbdmsc_service.h"
#include "hss_init.h"


#include "mpfs_reg_map.h"

#include "mss_usb_core_regs.h"

#define MSS_USB_BASE_ADDR				(uintptr_t)(USB_BASE)
#define MSS_USB_ADDR_UPPER_OFFSET			0x3FCu

bool HSS_USBInit(void)
{
    // To allow the USB DMA engine to support the MSS 38-bit address bus, an additional
    // configuration register is provided using one of the spare USB configuration addresses
    // (0x3FC). Logic is added in wrapper layer around the USB core detects a write or read to this
    // address, masks the select to the core, and sets a local register for the upper address lines

    // Address		Register	Bits	Function
    // 0x202013fc	ADDR_UPPER	5:0	Set the upper 6-bits of the Address bus for DMA
    //						operations.

    SYSREG->SOFT_RESET_CR &= ~ (1u << 16u);
    mHSS_WriteRegU32(MSS_USB, ADDR_UPPER, 0x10u);

    return true;
}
