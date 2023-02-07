/*******************************************************************************
 * Copyright 2017-2022 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS Board Initalization
 * \brief Board Initialization
 */

#include "config.h"
#include "hss_types.h"
#include <assert.h>

#include "hss_debug.h"

#include "hss_init.h"
#include "hss_state_machine.h"
#include "ssmb_ipi.h"
#include "hss_registry.h"
#ifndef __IO
#  define __IO volatile
#endif
#include "mss_io_config.h"
#include "io/hw_mssio_mux.h"

/******************************************************************************************************/
/*!
 * \brief Board Init Function Registration Table
 *
 * The following structure is used to connect in new board init functions.
 */

#include "hss_init.h"
#include "hss_boot_pmp.h"
#include "hss_sys_setup.h"
#include "hss_board_init.h"

const struct InitFunction /*@null@*/ boardInitFunctions[] = {
    // Name                     FunctionPointer         Halt   Restart
    { "HSS_ZeroTIMs",           HSS_ZeroTIMs,           false, false },
    { "HSS_Setup_PLIC",         HSS_Setup_PLIC,         false, false },
    { "HSS_Setup_BusErrorUnit", HSS_Setup_BusErrorUnit, false, false },
    { "HSS_Setup_MPU",          HSS_Setup_MPU,          false, false },
    { "HSS_DDRInit",            HSS_DDRInit,            false, false },
    { "HSS_ZeroDDR",            HSS_ZeroDDR,            false, false },
#ifdef CONFIG_USE_PCIE
    { "HSS_PCIeInit",           HSS_PCIeInit,           false, false },
#endif
    { "HSS_USBInit",            HSS_USBInit,            false, false },
};

/******************************************************************************************************/

/**
 * \brief Board Initialization Function
 *
 * All other initialization routines to be chained off this...
 */

/****************************************************************************/


#include "mss_sysreg.h"
bool HSS_BoardInit(void)
{
    RunInitFunctions(ARRAY_SIZE(boardInitFunctions), boardInitFunctions);

    return true;
}

bool HSS_BoardLateInit(void)
{
#if defined(CONFIG_SERVICE_MMC_MODE_SDCARD) || defined(CONFIG_SERVICE_MMC_MODE_EMMC)
    mHSS_DEBUG_PRINTF(LOG_WARN, "Please ensure that jumpers J16/J35/J36 are correct for "
#  if defined(CONFIG_SERVICE_MMC_BUS_VOLTAGE_1V8)
        "1.8V"
#  elif defined(CONFIG_SERVICE_MMC_BUS_VOLTAGE_3V3)
        "3.3V"
#  endif
        " MMC voltage... \n");
#endif
    return true;
}

/*
 * this function is used to switch external demux, and is board dependent
 * For the MPFS-Video kit, we are changing I/O pullup and pull down states
 * The setup for this is being automated in a future MSS Configurator release
 * */
uint8_t switch_external_mux(MSS_IO_OPTIONS option)
{
    uint8_t result = false;

    switch(option)
    {
        case SD_MSSIO_CONFIGURATION:
            SYSREG->IOMUX5_CR = LIBERO_SETTING_IOMUX5_CR;
            SYSREG->IOMUX5_CR &= ~((0xF << 0) | (0xF << 16) | (0xF << 28));
            SYSREG->IOMUX5_CR |= ((0xE << 0) | (0xE << 16) | (0xD << 28));

            //making usb pin pull down
            SYSREG->IOMUX4_CR = LIBERO_SETTING_IOMUX4_CR;
            SYSREG->IOMUX4_CR &= ~(0xF << 16);
            SYSREG->IOMUX4_CR |= (0xD << 16);
            break;

        case EMMC_MSSIO_CONFIGURATION:
            SYSREG->IOMUX5_CR = LIBERO_SETTING_IOMUX5_CR;
            //masking pads need to make 30, 34, 37 and force pull down (0xD)
            SYSREG->IOMUX5_CR &= ~((0xF << 0) | (0xF << 16) | (0xF << 28));
            SYSREG->IOMUX5_CR |= ((0xD << 0) | (0xD << 16) | (0xD << 28));

            //making usb pin pull down
            SYSREG->IOMUX4_CR = LIBERO_SETTING_IOMUX4_CR;
            SYSREG->IOMUX4_CR &= ~(0xF << 16);
            SYSREG->IOMUX4_CR |= (0xD << 16);
            break;

        case NO_SUPPORT_MSSIO_CONFIGURATION:
            break;

        case NOT_SETUP_MSSIO_CONFIGURATION:
            break;
    }
    result = true;

    return result;
}
