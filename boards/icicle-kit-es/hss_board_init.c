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
    { "HSS_Setup_Clocks",              HSS_Setup_Clocks,              false },
    { "HSS_Setup_PAD_IO",              HSS_Setup_PAD_IO,              false },
    { "HSS_ZeroTIMs",                  HSS_ZeroTIMs,                  false },
    { "HSS_Setup_PLIC",                HSS_Setup_PLIC,                false },
    { "HSS_Setup_BusErrorUnit",        HSS_Setup_BusErrorUnit,        false },
    { "HSS_Setup_MPU",                 HSS_Setup_MPU,                 false },
    { "HSS_DDRInit",                   HSS_DDRInit,                   false },
    { "HSS_ZeroDDR",                   HSS_ZeroDDR,                   false },
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
#if 0
    bool HSS_TinyCLI_Parser(void);
    void init_memory(void);
    void init_bus_error_unit(void);
    void init_mem_protection_unit(void);
    void init_pmp(uint8_t);
    uint8_t mss_nwc_init(void);

    HSS_Setup_Clocks();
    HSS_Setup_PAD_IO();
    init_memory();
    init_bus_error_unit();
    init_mem_protection_unit();
    init_pmp(0);

    HSS_DDRInit();

    //mss_nwc_init();
    //HSS_UARTInit();

    //HSS_TinyCLI_Parser();

    HSS_ZeroDDR();

#else
    RunInitFunctions(spanOfBoardInitFunctions, boardInitFunctions);
#endif

    return true;
}
