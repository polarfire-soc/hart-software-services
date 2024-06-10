/*******************************************************************************
 * Copyright 2019 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * @file mss_beu.c
 * @author Microchip FPGA Embedded Systems Solutions
 * @brief PolarFire SoC MSS MPU driver for configuring the Bus Error Unit
 *
 */

#include <stdio.h>
#include <string.h>
#include "mpfs_hal/mss_hal.h"

/**
 * \brief BEU user configuration for BEU enables
 *
 */
const uint64_t    beu_enable[] = {
    LIBERO_SETTING_BEU_ENABLE_HART0,
    LIBERO_SETTING_BEU_ENABLE_HART1,
    LIBERO_SETTING_BEU_ENABLE_HART2,
    LIBERO_SETTING_BEU_ENABLE_HART3,
    LIBERO_SETTING_BEU_ENABLE_HART4
};

/**
 * \brief BEU user configuration for BEU PLIC enables
 *
 */
const uint64_t    beu_plic_enable[] = {
    LIBERO_SETTING_BEU_PLIC_ENABLE_HART0,
    LIBERO_SETTING_BEU_PLIC_ENABLE_HART1,
    LIBERO_SETTING_BEU_PLIC_ENABLE_HART2,
    LIBERO_SETTING_BEU_PLIC_ENABLE_HART3,
    LIBERO_SETTING_BEU_PLIC_ENABLE_HART4
};

/**
 * \brief BEU user configuration for BEU local interrupt enables
 *
 */
const uint64_t    beu_local_enable[] = {
    LIBERO_SETTING_BEU_LOCAL_ENABLE_HART0,
    LIBERO_SETTING_BEU_LOCAL_ENABLE_HART1,
    LIBERO_SETTING_BEU_LOCAL_ENABLE_HART2,
    LIBERO_SETTING_BEU_LOCAL_ENABLE_HART3,
    LIBERO_SETTING_BEU_LOCAL_ENABLE_HART4
};


/**
 * This function is configured by editing parameters in
 * mss_sw_config.h as required.
 * @return
 */
__attribute__((weak)) uint8_t init_bus_error_unit(void)
{
   uint8_t hart_id;
   /* Init BEU in all harts - enable local interrupt */
   for(hart_id = MPFS_HAL_FIRST_HART; hart_id <= MPFS_HAL_LAST_HART; hart_id++)
   {
       BEU->regs[hart_id].ENABLE      = beu_enable[hart_id];
       BEU->regs[hart_id].PLIC_INT    = beu_plic_enable[hart_id];
       BEU->regs[hart_id].LOCAL_INT   = beu_local_enable[hart_id];
       BEU->regs[hart_id].CAUSE       = 0ULL;
       BEU->regs[hart_id].ACCRUED     = 0ULL;
       BEU->regs[hart_id].VALUE       = 0ULL;
   }
   return (0U);
}

/**
 * This interrupt is called if BEU->regs[hart_id].LOCAL_INT's is enabled.
 * If using, instantiate in your code, and add handling of errors as required.
 */
__attribute__((weak)) void handle_local_beu_interrupt(void)
{
}

