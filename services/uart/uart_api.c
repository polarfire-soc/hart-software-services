/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file Virtual UART API
 * \brief UART Driver State Machine API function definitions
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"

#include "ssmb_ipi.h"

#include "uart_service.h"

enum IPIStatusCode HSS_UartTx_IPIHandler(TxId_t transaction_id, enum HSSHartId source, uint32_t immediate_arg, void *p_extended_buffer_in_ddr, void *p_ancilliary_buffer_in_ddr)
{
    (void)transaction_id;
    (void)source;
    (void)immediate_arg;
    (void)p_extended_buffer_in_ddr;
    (void)p_ancilliary_buffer_in_ddr;

    // IPI received from one of the U54s...
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "called" CRLF);

    return IPI_SUCCESS;
}

enum IPIStatusCode HSS_UartPollRx_IPIHandler(TxId_t transaction_id, enum HSSHartId source, uint32_t immediate_arg, void *p_extended_buffer_in_ddr, void *p_ancilliary_buffer_in_ddr)
{
    (void)transaction_id;
    (void)source;
    (void)immediate_arg;
    (void)p_extended_buffer_in_ddr;
    (void)p_ancilliary_buffer_in_ddr;

    // IPI received from one of the U54s...
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "called" CRLF);

    return IPI_SUCCESS;
}
