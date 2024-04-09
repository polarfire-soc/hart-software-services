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
#include "mss_sys_services.h"
#include <string.h>

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


#include "mss_sysreg.h"
bool HSS_BoardInit(void)
{
    RunInitFunctions(ARRAY_SIZE(boardInitFunctions), boardInitFunctions);

    return true;
}


bool HSS_BoardLateInit(void)
{
    bool result = false;
    uint8_t design_info[76];

    MSS_SYS_select_service_mode( MSS_SYS_SERVICE_POLLING_MODE, NULL);
    memset(design_info, 0, ARRAY_SIZE(design_info));

    if (MSS_SYS_SUCCESS == MSS_SYS_get_design_info(design_info, 0u)) {
        mHSS_FANCY_PRINTF(LOG_STATUS, "Design Info: \n");
        mHSS_PRINTF("    Design Name: ");
        for (int i = 2; i < 32; i++) {
            mHSS_PUTC(design_info[i]);
        }
        int version = ((int)design_info[33] << 8) + design_info[32];
        int year = version / 1000;
        int month = (version / 10) % 100;
        int patch = (version % 10);
        mHSS_PRINTF("\n    Design Version: %02d.%02d", year, month);
        if (patch) {
            mHSS_PRINTF(".%d", patch);
        }
        mHSS_PUTS("\n");
        result = true;
    } else {
        mHSS_FANCY_PRINTF(LOG_ERROR, "Couldn't read Design Information\n");
    }

#if defined(CONFIG_SERVICE_MMC_MODE_SDCARD) || defined(CONFIG_SERVICE_MMC_MODE_EMMC)
    mHSS_DEBUG_PRINTF(LOG_WARN, "Please ensure that jumpers J34/J43 are correct for "
#  if defined(CONFIG_SERVICE_MMC_BUS_VOLTAGE_1V8)
        "1.8V"
#  elif defined(CONFIG_SERVICE_MMC_BUS_VOLTAGE_3V3)
        "3.3V"
#  endif
        " MMC voltage... \n");
#endif

    return result;
}
