/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * 
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS OpenSBI Initalization
 * \brief OpenSBI Initialization
 */

#include "config.h"
#include "hss_types.h"

#include <string.h>
#include <assert.h>

#include "csr_helper.h"

#include "hss_state_machine.h"
#include "hss_debug.h"

bool HSS_OpenSBIInit(void)
{
    bool result = false;
    mHSS_DEBUG_PRINTF("\tInitializing OpenSBI" CRLF);

    /*
     * Nothing to do on the E51...
    */

    return result;
}
