/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file Crypto Driver API
 * \brief MSS Crypto Driver API
 */

#include <stdlib.h>
#include <stdint.h>

#include "ssmb_ipi.h"

#include "hss_state_machine.h"
#include "hss_debug.h"

#include "crypto_types.h"

#include "crypto_service.h"

void HSS_Crypto_Init(void)
{
    // placeholder function incase any Athena or G5C specific initialisation is required...
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "called" CRLF);
}

//static uint32_t dummy = 0u;
//static uint32_t* pSSI_SystemServiceDescriptor = &dummy;

void HSS_SecureNVMWrite(void const *pPayload, const size_t count)
{
    (void)pPayload;
    (void)count;
    // setup state machine to do transfer
}

void HSS_SecureNVMRead(void *pPayload, const size_t count)
{
    (void)pPayload;
    (void)count;
    // setup state machine to do transfer
}
