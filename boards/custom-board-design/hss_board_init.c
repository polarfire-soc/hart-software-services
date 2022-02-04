/*******************************************************************************
 * Copyright 2017-2021 Microchip Corporation.
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
    { "HSS_Setup_Clocks",       HSS_Setup_Clocks,       false, false },
    //{ "HSS_Setup_PAD_IO",       HSS_Setup_PAD_IO,       false, false },
    { "HSS_ZeroTIMs",           HSS_ZeroTIMs,           false, false },
    { "HSS_Setup_BusErrorUnit", HSS_Setup_BusErrorUnit, false, false },
    { "HSS_Setup_MPU",          HSS_Setup_MPU,          false, false },
    { "HSS_DDRInit",            HSS_DDRInit,            false, false },
    { "HSS_ZeroDDR",            HSS_ZeroDDR,            false, false },
#ifdef CONFIG_USE_PCIE
    { "HSS_PCIeInit",           HSS_PCIeInit,           false, false },
#endif
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
    mHSS_DEBUG_PRINTF(LOG_WARN, "Please ensure that jumpers J34/J43 are correct for "
#  if defined(CONFIG_SERVICE_MMC_BUS_VOLTAGE_1V8)
        "1.8V"
#  elif defined(CONFIG_SERVICE_MMC_BUS_VOLTAGE_3V3)
        "3.3V"
#  endif
        " MMC voltage... " CRLF);
#endif
    return true;
}
