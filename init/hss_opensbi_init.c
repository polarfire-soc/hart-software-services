/*******************************************************************************
 * Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
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

#include "hss_init.h"

#include <string.h>
#include <assert.h>

#include "csr_helper.h"

#include "hss_trigger.h"
#include "hss_state_machine.h"
#include "opensbi_service.h"
#include "hss_debug.h"

bool HSS_OpenSBIInit(void)
{
    bool result = true;

    HSS_OpenSBI_Setup();
    HSS_Trigger_Notify(EVENT_OPENSBI_INITIALIZED);

    return result;
}
