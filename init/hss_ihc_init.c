/*******************************************************************************
 * Copyright 2019-2021 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS Debug UART Initalization
 * \brief Debug UART Initialization
 */

#include "config.h"
#include "hss_types.h"
#include "hss_init.h"
#include "csr_helper.h"

#include <assert.h>

#include "hss_debug.h"
#include "hss_types.h"

#include "miv_ihc.h"

bool HSS_IHCInit(void)
{
    IHC_global_init();

    IHC_local_context_init((uint32_t)HSS_HART_E51);
    IHC_local_context_init((uint32_t)HSS_HART_U54_1);
    IHC_local_context_init((uint32_t)HSS_HART_U54_2);
    IHC_local_context_init((uint32_t)HSS_HART_U54_3);
    IHC_local_context_init((uint32_t)HSS_HART_U54_4);

    return true;
}
