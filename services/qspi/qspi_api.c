/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * 
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file QSPI (null) Service
 * \brief Setup QSPI
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"

#include "ssmb_ipi.h"

#include <assert.h>

#include "qspi_service.h"
#include "mss_qspi.h"

/*
 * QSPI doesn't need a "service" to run every super-loop, but it does need to be
 * initialized early...
 */

bool HSS_QSPIInit(void)
{
    static mss_qspi_config_t qspiConfig =
    { 
        .xip = 1,
        .xip_addr = 4, //num_bytes_in_XIP_mode
        .spi_mode = MSS_QSPI_MODE0,
        .clk_div = MSS_QSPI_CLK_DIV_30,
        .io_format = MSS_QSPI_QUAD_FULL,
        .sample = 0
    };

#if 0
/* TODO: disabling for RENODE */
    MSS_QSPI_init();
    MSS_QSPI_enable();
    MSS_QSPI_configure(&qspiConfig);
#else
    (void)qspiConfig;
#endif

    return true;
}
