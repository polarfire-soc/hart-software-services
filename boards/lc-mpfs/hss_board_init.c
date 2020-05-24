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

const struct InitFunction /*@null@*/ boardInitFunctions[] = { };


/******************************************************************************************************/

/**
 * \brief Board Initialization Function
 *
 * All other initialization routines to be chained off this...
 */

/****************************************************************************/


bool HSS_BoardInit(void)
{
    RunInitFunctions(mSPAN_OF(boardInitFunctions), boardInitFunctions);

    return true;
}
