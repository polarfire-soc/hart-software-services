/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS PDMA Initalization
 * \brief PDMA Initialization
 */

#include "config.h"
#include "hss_types.h"

#include <assert.h>

#include "hss_debug.h"

#if IS_ENABLED(CONFIG_USE_PDMA)
#  include "drivers/mss_pdma/mss_pdma.h"
#endif

bool HSS_PDMAInit(void)
{
#if IS_ENABLED(CONFIG_USE_PDMA)
    // initialise PDMA
    MSS_PDMA_init();
#endif

    return true;
}
