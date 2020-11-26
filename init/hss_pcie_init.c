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

bool HSS_PCIeInit(void)
{
    uint32_t status;

    // turn off SEC - on ctrl1
    // clear out stragglers
    mHSS_WriteRegU32(PCIE1_CTRL, SEC_ERROR_MASK, 0); // writel(0, pcie_ctrl_base_addr  + SEC_ERROR_MASK);
    status = mHSS_ReadRegU32(PCIE1_CTRL, SEC_ERROR_INT); // status = readl(pcie_ctrl_base_addr + SEC_ERROR_INT);
    if (status) {
        mHSS_WriteRegU32(PCIE1_CTRL, SEC_ERROR_INT, status); // writel(status, pcie_ctrl_base_addr + SEC_ERROR_INT);
    }

    // turn off DED - on ctrl1
    mHSS_WriteRegU32(PCIE1_CTRL, DED_ERROR_MASK, 0); // writel(0, pcie_ctrl_base_addr  + DED_ERROR_MASK);
    status = mHSS_ReadRegU32(PCIE1_CTRL, DED_ERROR_INT); // status = readl(pcie_ctrl_base_addr + DED_ERROR_INT);
    if (status) {
        mHSS_WriteRegU32(PCIE1_CTRL, DED_ERROR_INT, 0); // writel(status, pcie_ctrl_base_addr + DED_ERROR_INT);
    }

    // turn of PCIe EVENTs - on ctrl1
    status = mHSS_ReadRegU32(PCIE1_CTRL, PCIE_EVENT_INT); // status = readl(pcie_ctrl_base_addr + PCIE_EVENT_INT);
    // Clear outstanding and disable future PCIE_EVENT_INTS
    mHSS_WriteRegU32(PCIE1_CTRL, DED_ERROR_INT, 0x0000FFFFu); // writel(0x0000ffff, pcie_ctrl_base_addr + DED_ERROR_INT);

    // turn off LOCAL interrupts - on bridge1
    mHSS_WriteRegU32(PCIE1_BRIDGE, IMASK_LOCAL, 0); // writel(0, pcie_bridge_base_addr  + IMASK_LOCAL);
    status = mHSS_ReadRegU32(PCIE1_BRIDGE, ISTATUS_LOCAL); // status = readl(pcie_bridge_base_addr + ISTATUS_LOCAL);
    if (status) {
        mHSS_WriteRegU32(PCIE1_BRIDGE, ISTATUS_LOCAL, status); // writel(status, pcie_bridge_base_addr + ISTATUS_LOCAL);
    }

    // turn off HOST interrupts - on bridge1
    mHSS_WriteRegU32(PCIE1_BRIDGE, IMASK_HOST, 0); // writel(0, pcie_bridge_base_addr  + IMASK_HOST);
    status = mHSS_ReadRegU32(PCIE1_BRIDGE, ISTATUS_HOST); // status = readl(pcie_bridge_base_addr + ISTATUS_HOST);
    if (status) {
        mHSS_WriteRegU32(PCIE1_BRIDGE, ISTATUS_HOST, status); // writel(status, pcie_bridge_base_addr + ISTATUS_HOST);
    }

    return true;
}
