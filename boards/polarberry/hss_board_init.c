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
#ifdef CONFIG_USE_TAMPER
    { "HSS_TamperInit",       HSS_TamperInit,       false, false },
#endif
    //{ "HSS_USBInit",            HSS_USBInit,            false, false }, // if using 64-bit upper memory only, uncomment this
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
    return true;
}


/**
 * Is there a mux present, define is true by default
 * @return true/false
 */
__attribute__((weak)) uint8_t fabric_sd_emmc_demux_present(void)
{
    return (uint8_t) false; // In the PolalBerry board SD/eMMC demuxer is alwqays disabled due to HW architecture
//FABRIC_SD_EMMC_DEMUX_SELECT_PRESENT;
}

/*
 * this function is used to switch external demux, and is board dependent
 * For the MPFS-Video kit, we are changing I/O pullup and pull down states
 * The setup for this is being automated in a future MSS Configurator release
 * */
uint8_t switch_demux_using_fabric_ip(MSS_IO_OPTIONS option)
{
    uint8_t result = false;

    if (!fabric_sd_emmc_demux_present())
    {
	mHSS_PUTS("SD/eMMC internal demuxer is disabled!\n");
	result = true;
	return result;
    }

    switch(option)
    {
        case SD_MSSIO_CONFIGURATION:
	    mHSS_PUTS("SD_MSSIO_CONFIGURATION\n");
/*
            SYSREG->IOMUX5_CR = LIBERO_SETTING_IOMUX5_CR;
            SYSREG->IOMUX5_CR &= ~((0xF << 0) | (0xF << 16) | (0xF << 28));
            SYSREG->IOMUX5_CR |= ((0xE << 0) | (0xE << 16) | (0xD << 28));

            //making usb pin pull down
            SYSREG->IOMUX4_CR = LIBERO_SETTING_IOMUX4_CR;
            SYSREG->IOMUX4_CR &= ~(0xF << 16);
            SYSREG->IOMUX4_CR |= (0xD << 16);
*/
        break;

        case EMMC_MSSIO_CONFIGURATION:
	    mHSS_PUTS("EMMC_MSSIO_CONFIGURATION\n");

/*            SYSREG->IOMUX5_CR = LIBERO_SETTING_IOMUX5_CR;
            //masking pads need to make 30, 34, 37 and force pull down (0xD)
            SYSREG->IOMUX5_CR &= ~((0xF << 0) | (0xF << 16) | (0xF << 28));
            SYSREG->IOMUX5_CR |= ((0xD << 0) | (0xD << 16) | (0xD << 28));

            //making usb pin pull down
            SYSREG->IOMUX4_CR = LIBERO_SETTING_IOMUX4_CR;
            SYSREG->IOMUX4_CR &= ~(0xF << 16);
            SYSREG->IOMUX4_CR |= (0xD << 16);
*/
        break;

        case NO_SUPPORT_MSSIO_CONFIGURATION:
	    mHSS_PUTS("NO_SUPPORT_MSSIO_CONFIGURATION\n");
        break;

        case NOT_SETUP_MSSIO_CONFIGURATION:
	    mHSS_PUTS("NOT_SUPPORT_MSSIO_CONFIGURATION\n");
        break;
    }
    result = true;

    return result;
}