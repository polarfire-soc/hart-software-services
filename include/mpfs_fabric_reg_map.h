#ifndef HSS_MPFS_FABRIC_REG_MAP_H
#define HSS_MPFS_FABRIC_REG_MAP_H

/*******************************************************************************
 * Copyright 2023 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Hart Software Services - FPGA Fabric Register Definitions
 *
 */

#include "mpfs_reg_map.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_USE_TAMPER
#define TAMPER_BASE_ADDR                (CONFIG_TAMPER_BASE_ADDRESS)
#else
#define TAMPER_BASE_ADDR                (NULL)
#endif
#define TAMPER_FLAGS_OFFSET             (0x0u)
#define TAMPER_RESET_REASON_OFFSET      (0x4u)
#define TAMPER_VOLT_REG_OFFSET          (0xCu)
#define TAMPER_RESET_OFFSET             (0x14u)
#define TAMPER_IRQ_EN_OFFSET            (0x20u)

#ifdef __cplusplus
}
#endif

#endif
