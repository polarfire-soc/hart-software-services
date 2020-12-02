/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS PCIe Initalization
 * \brief PCIe Initialization
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#include <assert.h>
#include "mpfs_reg_map.h"
#include "csr_helper.h"


// Register Defintions
#define PLIC_PRIORITY_BASE_ADDR		(PLIC_BASE_ADDR)
#define PLIC_PRIORITY_PCIE_OFFSET		(119 * 4)
#define PLIC_ENABLE_BASE_ADDR		(PLIC_BASE_ADDR + 0x2000)
#define PLIC_HART0_ENABLE_BASE_ADDR	(PLIC_ENABLE_BASE_ADDR + 0x0)
#define PLIC_HART0_ENABLE_REG3_OFFSET		((119 / 32) * 4)
#define PLIC_CONTEXT_BASE_ADDR		(PLIC_BASE_ADDR + 0x200000)
#define PLIC_HART0_CONTEXT_BASE_ADDR	(PLIC_CONTEXT_BASE_ADDR)
#define PLIC_HART0_CONTEXT_THRESHOLD_OFFSET	(0)
#define PLIC_HART0_CONTEXT_CLAIM_OFFSET		(4)

//
// Will constraint base addr of PCIe to 0x43000000u
// and constrain to use Bridge and Ctrl [1]
#define PCIE_BASE_ADDR			(uintptr_t)0x43000000u

#define PCIE1_BRIDGE_BASE_ADDR		(PCIE_BASE_ADDR + 0x00008000u)
#define PCIE1_BRIDGE_IMASK_LOCAL_OFFSET		0x180
#define PCIE1_BRIDGE_ISTATUS_LOCAL_OFFSET	0x184
#define PCIE1_BRIDGE_IMASK_HOST_OFFSET		0x188
#define PCIE1_BRIDGE_ISTATUS_HOST_OFFSET	0x18c

#define PCIE1_CTRL_BASE_ADDR		(PCIE_BASE_ADDR + 0x0000a000u)
#define PCIE1_CTRL_SEC_ERROR_INT_OFFSET		0x28
#define PCIE1_CTRL_SEC_ERROR_MASK_OFFSET	0x2c
#define PCIE1_CTRL_DED_ERROR_INT_OFFSET		0x30
#define PCIE1_CTRL_DED_ERROR_MASK_OFFSET	0x34
#define PCIE1_CTRL_PCIE_EVENT_INT_OFFSET	0x14c

#define SENTINAL_MAX				(255)

bool HSS_PCIeInit(void)
{
    uint32_t status;
    uint32_t claim;
    uint32_t pcie_hwirq = 119;
    uint32_t hwirq_mask = 1 << (pcie_hwirq % 32);
    uint32_t sentinal = 0;

    // Set a priority for pcie hwirq
    mHSS_WriteRegU32(PLIC_PRIORITY, PCIE, 1);

    // Set a context threshold for hart 0
    mHSS_WriteRegU32(PLIC_HART0_CONTEXT, THRESHOLD, 1);

    // enable pcie hwirq
    status = mHSS_ReadRegU32(PLIC_HART0_ENABLE, REG3);
    status |= hwirq_mask;
    mHSS_WriteRegU32(PLIC_HART0_ENABLE, REG3, status);

    // don't hang around forever - if an interrupt is stubborn - signal failure
    while ((sentinal < SENTINAL_MAX) && (claim = mHSS_ReadRegU32(PLIC_HART0_CONTEXT, CLAIM))) {
        sentinal++;
        // turn off SEC - on ctrl1
        // clear out stragglers
        mHSS_WriteRegU32(PCIE1_CTRL, SEC_ERROR_MASK, 0xffff);
        status = mHSS_ReadRegU32(PCIE1_CTRL, SEC_ERROR_INT);
        if (status) {
            mHSS_WriteRegU32(PCIE1_CTRL, SEC_ERROR_INT, status);
        }

        // turn off DED - on ctrl1
        mHSS_WriteRegU32(PCIE1_CTRL, DED_ERROR_MASK, 0xffff);
        status = mHSS_ReadRegU32(PCIE1_CTRL, DED_ERROR_INT);
        if (status) {
            mHSS_WriteRegU32(PCIE1_CTRL, DED_ERROR_INT, status);
        }

        // turn of PCIe EVENTs - on ctrl1
        // Clear outstanding and disable future PCIE_EVENT_INTS
        status = mHSS_ReadRegU32(PCIE1_CTRL, PCIE_EVENT_INT);
        if (status & 0x7) {
            mHSS_WriteRegU32(PCIE1_CTRL, PCIE_EVENT_INT, 0x0070007u);
        }

        // turn off LOCAL interrupts - on bridge1
        mHSS_WriteRegU32(PCIE1_BRIDGE, IMASK_LOCAL, 0);
        status = mHSS_ReadRegU32(PCIE1_BRIDGE, ISTATUS_LOCAL);
        if (status) {
            mHSS_WriteRegU32(PCIE1_BRIDGE, ISTATUS_LOCAL, status);
        }

        // turn off HOST interrupts - on bridge1
        mHSS_WriteRegU32(PCIE1_BRIDGE, IMASK_HOST, 0);
        status = mHSS_ReadRegU32(PCIE1_BRIDGE, ISTATUS_HOST);
        if (status) {
            mHSS_WriteRegU32(PCIE1_BRIDGE, ISTATUS_HOST, status);
        }

        // complete the claim on the plic
        mHSS_WriteRegU32(PLIC_HART0_CONTEXT, CLAIM, claim);
    }

    // disable pcie hwirq
    status = mHSS_ReadRegU32(PLIC_HART0_ENABLE, REG3);
    status &= ~hwirq_mask;
    mHSS_WriteRegU32(PLIC_HART0_ENABLE, REG3, status);

    // Set a context threshold for hart 0 back to 0
    mHSS_WriteRegU32(PLIC_HART0_CONTEXT, THRESHOLD, 0);

    // Set priority for pcie hwirq back to 0
    mHSS_WriteRegU32(PLIC_PRIORITY, PCIE, 0);

    if (sentinal >= 255) {
        return false;
    }

    return true;
}
