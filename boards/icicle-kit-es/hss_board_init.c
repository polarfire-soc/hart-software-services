/*******************************************************************************
 * Copyright 2017-2020 Microchip Corporation.
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
#include "ssmb/ipi/ssmb_ipi.h"
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
    { "HSS_Init_RWDATA_BSS",    HSS_Init_RWDATA_BSS,    false, false },
    { "HSS_Setup_Clocks",       HSS_Setup_Clocks,       false, false },
    { "HSS_Setup_PAD_IO",       HSS_Setup_PAD_IO,       false, false },
    { "HSS_ZeroTIMs",           HSS_ZeroTIMs,           false, false },
    { "HSS_Setup_PLIC",         HSS_Setup_PLIC,         false, false },
    { "HSS_Setup_BusErrorUnit", HSS_Setup_BusErrorUnit, false, false },
    { "HSS_Setup_MPU",          HSS_Setup_MPU,          false, false },
    { "HSS_DDRInit",            HSS_DDRInit,            false, false },
    //{ "HSS_ZeroDDR",            HSS_ZeroDDR,            false, false },
};
const size_t spanOfBoardInitFunctions = mSPAN_OF(boardInitFunctions);


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
    RunInitFunctions(spanOfBoardInitFunctions, boardInitFunctions);

    return true;
}
