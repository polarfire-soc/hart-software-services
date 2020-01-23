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

#include <mss_pdma.h>

bool HSS_PDMAInit(void)
{
    // initialise PDMA
    MSS_PDMA_init();

    return true;
}
