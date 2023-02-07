/*******************************************************************************
 * Copyright 2019-2023 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS Tamper Initalization
 * \brief Tamper Initialization
 */

#include "config.h"
#include "hss_types.h"

#include "hss_init.h"
#include "hss_debug.h"

#include <assert.h>
#include "csr_helper.h"

#include "mss_plic.h"
#include "mpfs_reg_map.h"

// Register Defintions
#define TAMPER_IRQn                    (PLIC_F2M_6_INT_OFFSET)

// Set base address of tamper and register offsets
#define TAMPER_BASE_ADDR               (CONFIG_TAMPER_BASE_ADDRESS)
#define TAMPER_FLAGS_OFFSET            (0x0u)
#define TAMPER_RESET_REASON_OFFSET     (0x4u)
#define TAMPER_VOLT_REG_OFFSET         (0xCu)
#define TAMPER_RESET_OFFSET            (0x14u)
#define TAMPER_IRQ_EN_OFFSET           (0x20u)

bool HSS_TamperInit(void)
{
    // Clear all flags and disable interrupts from the tamper block
    mHSS_WriteRegU32(TAMPER, FLAGS, 0xFFFFFFFFu);
    mHSS_WriteRegU32(TAMPER, VOLT_REG, 0xFFFFFFFFu);
    mHSS_WriteRegU32(TAMPER, IRQ_EN, 0u);

    // Claim an interrupt if one was pending before disabling interrupts from the tamper block
    // Set a priority for tamper hwirq
    // Set a context threshold for hart 0
    // enable tamper interrupt
    PLIC_SetPriority(TAMPER_IRQn, 1u);
    PLIC_SetPriority_Threshold(1u);
    PLIC_EnableIRQ(TAMPER_IRQn);

    // check if an interrupt is pending
    // if so, attempt to clear (with timeout)
    PLIC_ClaimIRQ();

    // disable tamper hwirq
    // Set a context threshold for hart 0 back to 0
    // Set priority for tamper hwirq back to 0
    PLIC_DisableIRQ(TAMPER_IRQn);
    PLIC_SetPriority_Threshold(0u);
    PLIC_SetPriority(TAMPER_IRQn, 0u);

    return (1u);
}
