/*******************************************************************************
 * Copyright 2017-2025 Microchip FPGA Embedded Systems Solutions.
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
#ifdef CONFIG_USE_PCIE
    { "HSS_PCIeInit",           HSS_PCIeInit,           false, false },
#endif
#ifdef CONFIG_USE_TAMPER
    { "HSS_TamperInit",         HSS_TamperInit,         false, false },
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


bool HSS_BoardInit(void)
{
    RunInitFunctions(ARRAY_SIZE(boardInitFunctions), boardInitFunctions);

    return true;
}

bool HSS_BoardLateInit(void)
{
    bool result = true;

    return result;
}

/**
 * Is there a mux present for the SD-card/eMMC MSS controller?
 * @return true/false
 */
uint8_t fabric_sd_emmc_demux_present(void)
{
    return (uint8_t) false; // On BeagleV-Fire SD/eMMC is directly connected to eMMC.
}

/*
 * This function is used to switch external demux between SD card and eMMC, and
 * is board dependent. For the BeagleV-Fire, there are no external switching 
 * since on eMMC is supported by the SD-card/eMMC controller.
 * */
uint8_t switch_demux_using_fabric_ip(MSS_IO_OPTIONS option)
{
    uint8_t result = true;

    if (!fabric_sd_emmc_demux_present()) {
        mHSS_PUTS("BeagleV-Fire: eMMC direct connection to SD/eMMC controller\n");
    }

    return result;
}
