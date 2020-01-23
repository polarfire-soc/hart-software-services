/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * 
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS DDR Initalization
 * \brief DDR Initialization
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"
#ifdef CONFIG_MEMTEST
#    include "hss_memtest.h"
#endif

#include <assert.h>


#include <mss_uart.h>

/*!
 * \brief DDR Training
 *
 * The E51 ensures that DDR is setup prior to code download, 
 * and thus perform an DDR training and configuration required to achieve this.
 *
 * The intention is to allow as much flexibility as possible in DDR training, 
 * so it is driven by MPFS HSS Embedded Software, with hardware hooks to 
 * perform real-time critical functions. 
 *
 * TBD: is periodic re-calibration required during operation (e.g. temperature induced 
 * or other)
 */
enum HSSHartId CSR_GetHartId(void);
void HSS_DDR_Train(void)
{
    mHSS_DEBUG_PRINTF("\trunning DDR training on hart %u..." CRLF, CSR_GetHartId());
}

/*!
 * \brief Hook for DDR Setup
 */
bool HSS_DDRInit(void)
{
#ifdef CONFIG_MEMTEST
    mHSS_DEBUG_PRINTF("\tMemory Testing DDR..." CRLF);
    HSS_MemTestDDRFast();
#endif

    mHSS_DEBUG_PRINTF("\tInitializing DDR..." CRLF);
    HSS_DDR_Train();

    return true;
}
