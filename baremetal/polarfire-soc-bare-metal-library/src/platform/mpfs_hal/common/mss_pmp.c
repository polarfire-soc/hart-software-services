/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */
/*******************************************************************************
 * @file mss_mpu.c
 * @author Microchip-FPGA Embedded Systems Solutions
 * @brief PolarFire SoC MSS MPU driver for configuring access regions for the
 * external masters.
 *
 */
/*=========================================================================*//**

 *//*=========================================================================*/
#include <stdio.h>
#include <string.h>
#include "mss_hal.h"

/**
 * \brief PMP configuration from Libero
 *
 */
const uint64_t pmp_values[][18] = {
        /* hart 0 */
        {LIBERO_SETTING_HART0_CSR_PMPCFG0,
        LIBERO_SETTING_HART0_CSR_PMPCFG2,
        LIBERO_SETTING_HART0_CSR_PMPADDR0,
        LIBERO_SETTING_HART0_CSR_PMPADDR1,
        LIBERO_SETTING_HART0_CSR_PMPADDR2,
        LIBERO_SETTING_HART0_CSR_PMPADDR3,
        LIBERO_SETTING_HART0_CSR_PMPADDR4,
        LIBERO_SETTING_HART0_CSR_PMPADDR5,
        LIBERO_SETTING_HART0_CSR_PMPADDR6,
        LIBERO_SETTING_HART0_CSR_PMPADDR7,
        LIBERO_SETTING_HART0_CSR_PMPADDR8,
        LIBERO_SETTING_HART0_CSR_PMPADDR9,
        LIBERO_SETTING_HART0_CSR_PMPADDR10,
        LIBERO_SETTING_HART0_CSR_PMPADDR11,
        LIBERO_SETTING_HART0_CSR_PMPADDR12,
        LIBERO_SETTING_HART0_CSR_PMPADDR13,
        LIBERO_SETTING_HART0_CSR_PMPADDR14,
        LIBERO_SETTING_HART0_CSR_PMPADDR15},
        /* hart 1 */
        {LIBERO_SETTING_HART1_CSR_PMPCFG0,
        LIBERO_SETTING_HART1_CSR_PMPCFG2,
        LIBERO_SETTING_HART1_CSR_PMPADDR0,
        LIBERO_SETTING_HART1_CSR_PMPADDR1,
        LIBERO_SETTING_HART1_CSR_PMPADDR2,
        LIBERO_SETTING_HART1_CSR_PMPADDR3,
        LIBERO_SETTING_HART1_CSR_PMPADDR4,
        LIBERO_SETTING_HART1_CSR_PMPADDR5,
        LIBERO_SETTING_HART1_CSR_PMPADDR6,
        LIBERO_SETTING_HART1_CSR_PMPADDR7,
        LIBERO_SETTING_HART1_CSR_PMPADDR8,
        LIBERO_SETTING_HART1_CSR_PMPADDR9,
        LIBERO_SETTING_HART1_CSR_PMPADDR10,
        LIBERO_SETTING_HART1_CSR_PMPADDR11,
        LIBERO_SETTING_HART1_CSR_PMPADDR12,
        LIBERO_SETTING_HART1_CSR_PMPADDR13,
        LIBERO_SETTING_HART1_CSR_PMPADDR14,
        LIBERO_SETTING_HART1_CSR_PMPADDR15},
        /* hart 2 */
        {LIBERO_SETTING_HART2_CSR_PMPCFG0,
        LIBERO_SETTING_HART2_CSR_PMPCFG2,
        LIBERO_SETTING_HART2_CSR_PMPADDR0,
        LIBERO_SETTING_HART2_CSR_PMPADDR1,
        LIBERO_SETTING_HART2_CSR_PMPADDR2,
        LIBERO_SETTING_HART2_CSR_PMPADDR3,
        LIBERO_SETTING_HART2_CSR_PMPADDR4,
        LIBERO_SETTING_HART2_CSR_PMPADDR5,
        LIBERO_SETTING_HART2_CSR_PMPADDR6,
        LIBERO_SETTING_HART2_CSR_PMPADDR7,
        LIBERO_SETTING_HART2_CSR_PMPADDR8,
        LIBERO_SETTING_HART2_CSR_PMPADDR9,
        LIBERO_SETTING_HART2_CSR_PMPADDR10,
        LIBERO_SETTING_HART2_CSR_PMPADDR11,
        LIBERO_SETTING_HART2_CSR_PMPADDR12,
        LIBERO_SETTING_HART2_CSR_PMPADDR13,
        LIBERO_SETTING_HART2_CSR_PMPADDR14,
        LIBERO_SETTING_HART2_CSR_PMPADDR15},
        /* hart 3 */
        {LIBERO_SETTING_HART3_CSR_PMPCFG0,
        LIBERO_SETTING_HART3_CSR_PMPCFG2,
        LIBERO_SETTING_HART3_CSR_PMPADDR0,
        LIBERO_SETTING_HART3_CSR_PMPADDR1,
        LIBERO_SETTING_HART3_CSR_PMPADDR2,
        LIBERO_SETTING_HART3_CSR_PMPADDR3,
        LIBERO_SETTING_HART3_CSR_PMPADDR4,
        LIBERO_SETTING_HART3_CSR_PMPADDR5,
        LIBERO_SETTING_HART3_CSR_PMPADDR6,
        LIBERO_SETTING_HART3_CSR_PMPADDR7,
        LIBERO_SETTING_HART3_CSR_PMPADDR8,
        LIBERO_SETTING_HART3_CSR_PMPADDR9,
        LIBERO_SETTING_HART3_CSR_PMPADDR10,
        LIBERO_SETTING_HART3_CSR_PMPADDR11,
        LIBERO_SETTING_HART3_CSR_PMPADDR12,
        LIBERO_SETTING_HART3_CSR_PMPADDR13,
        LIBERO_SETTING_HART3_CSR_PMPADDR14,
        LIBERO_SETTING_HART3_CSR_PMPADDR15},
        /* hart 4 */
        {LIBERO_SETTING_HART4_CSR_PMPCFG0,
        LIBERO_SETTING_HART4_CSR_PMPCFG2,
        LIBERO_SETTING_HART4_CSR_PMPADDR0,
        LIBERO_SETTING_HART4_CSR_PMPADDR1,
        LIBERO_SETTING_HART4_CSR_PMPADDR2,
        LIBERO_SETTING_HART4_CSR_PMPADDR3,
        LIBERO_SETTING_HART4_CSR_PMPADDR4,
        LIBERO_SETTING_HART4_CSR_PMPADDR5,
        LIBERO_SETTING_HART4_CSR_PMPADDR6,
        LIBERO_SETTING_HART4_CSR_PMPADDR7,
        LIBERO_SETTING_HART4_CSR_PMPADDR8,
        LIBERO_SETTING_HART4_CSR_PMPADDR9,
        LIBERO_SETTING_HART4_CSR_PMPADDR10,
        LIBERO_SETTING_HART4_CSR_PMPADDR11,
        LIBERO_SETTING_HART4_CSR_PMPADDR12,
        LIBERO_SETTING_HART4_CSR_PMPADDR13,
        LIBERO_SETTING_HART4_CSR_PMPADDR14,
        LIBERO_SETTING_HART4_CSR_PMPADDR15},
};

/***************************************************************************//**
 * MPU_auto_configure()
 * Set MPU's up with configuration from Libero
 *
 *
 * @return
 */
uint8_t pmp_configure(uint8_t hart_id) /* set-up with settings from Libero */
{
	/* make sure enables are off */
    write_csr(pmpcfg0, 0);
    write_csr(pmpcfg2, 0);
	/* set required addressing */
    write_csr(pmpaddr0, pmp_values[hart_id][2]);
    write_csr(pmpaddr1, pmp_values[hart_id][3]);
    write_csr(pmpaddr2, pmp_values[hart_id][4]);
    write_csr(pmpaddr3, pmp_values[hart_id][5]);
    write_csr(pmpaddr4, pmp_values[hart_id][6]);
    write_csr(pmpaddr5, pmp_values[hart_id][7]);
    write_csr(pmpaddr6, pmp_values[hart_id][8]);
    write_csr(pmpaddr7, pmp_values[hart_id][9]);
    write_csr(pmpaddr8, pmp_values[hart_id][10]);
    write_csr(pmpaddr9, pmp_values[hart_id][11]);
    write_csr(pmpaddr10, pmp_values[hart_id][12]);
    write_csr(pmpaddr11, pmp_values[hart_id][13]);
    write_csr(pmpaddr12, pmp_values[hart_id][14]);
    write_csr(pmpaddr13, pmp_values[hart_id][15]);
    write_csr(pmpaddr14, pmp_values[hart_id][16]);
    write_csr(pmpaddr15, pmp_values[hart_id][17]);
	/* now set the enables */
    write_csr(pmpcfg0, pmp_values[hart_id][0]);
    write_csr(pmpcfg2, pmp_values[hart_id][1]);

    return(0);
}
