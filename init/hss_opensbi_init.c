/*******************************************************************************
 * Copyright 2019-2021 Microchip Corporation.
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
#include "opensbi_service.h"
#include "hss_debug.h"

bool HSS_OpenSBIInit(void)
{
    bool result = true;

    HSS_OpenSBI_Setup(current_hartid());

    /*
     * Nothing to do on the E51...
    */

    return result;
}
