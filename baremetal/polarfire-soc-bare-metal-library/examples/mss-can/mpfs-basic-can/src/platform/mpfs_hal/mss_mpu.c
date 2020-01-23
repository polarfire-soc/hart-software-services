/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */
/*******************************************************************************
 * @file mss_mpu.c
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief PolarFire SoC MSS MPU driver for configuring access regions for the
 * external masters.
 *
 * SVN $Revision: 11867 $
 * SVN $Date: 2019-07-29 23:56:04 +0530 (Mon, 29 Jul 2019) $
 */
/*=========================================================================*//**
  
 *//*=========================================================================*/
#include "mss_mpu.h"

#ifndef SIFIVE_HIFIVE_UNLEASHED

#ifdef __cplusplus
extern "C" {
#endif

static uint64_t pmp_get_napot_base_and_range(uint64_t reg, uint64_t *range);

uint8_t num_pmp_lut[10] = {16,16,8,4,8,8,4,4,8,2};

/***************************************************************************//**
*/
uint8_t MSS_MPU_configure(mss_mpu_mport_t master_port,
                                        mss_mpu_pmp_region_t pmp_region,
                                        uint64_t base,
                                        uint64_t size,
                                        uint8_t permission,
                                        mss_mpu_addrm_t matching_mode,
                                        uint8_t lock_en)
{
    uint64_t temp = size, cnt=0ULL;
    uint64_t range;
    volatile uint64_t baddr=0ULL;


    /*size must be minimum 4k
      Size must be power of 2
      different masters have different number of regions*/
    if((size >= 4096ULL) && (0u == (size & (size - 1))) && (pmp_region < num_pmp_lut[master_port]))
    {
        while((0 == (temp & 0x01)))
        {
            cnt++;
            temp >>= 1;
        }

        range = (1ull << (cnt-1u))-1u;

        MSS_MPU(master_port)->PMPCFG[pmp_region].pmp = (base | range) >> 2U;

        MSS_MPU(master_port)->PMPCFG[pmp_region].mode = permission |
                                                        (matching_mode << 3U) |
                                                        (lock_en << 0x7U);

        baddr = (MSS_MPU(master_port)->PMPCFG[pmp_region].mode);
        baddr = (MSS_MPU(master_port)->PMPCFG[pmp_region].pmp);

        return ((uint8_t)0);
    }
    else
        return ((uint8_t)1);
}

uint8_t MSS_MPU_get_config(mss_mpu_mport_t master_port,
                           mss_mpu_pmp_region_t pmp_region,
                           uint64_t* base,
                           uint64_t* size,
                           uint8_t* permission,
                           mss_mpu_addrm_t* matching_mode,
                           uint8_t* lock_en)
{
    uint64_t reg;

    /*All AXI external masters dont have same number of PMP regions*/
    if(pmp_region < num_pmp_lut[master_port])
    {
        reg = MSS_MPU(master_port)->PMPCFG[pmp_region].pmp;
        *base = pmp_get_napot_base_and_range(reg, size);

        reg = MSS_MPU(master_port)->PMPCFG[pmp_region].mode;
        *lock_en = ( reg >> 0x7u) & 0x1u;
        *matching_mode = (mss_mpu_addrm_t)( (reg >> 3ULL) & 0x3U);
        *permission = reg & 0x7u;

        return ((uint8_t)0);
    }
    else
        return ((uint8_t)1);
}

static uint64_t pmp_get_napot_base_and_range(uint64_t reg, uint64_t *range)
{
    /* construct a mask of all bits bar the top bit */
    uint64_t mask = 0U;
    uint64_t base = reg;
    uint64_t numbits = (sizeof(uint64_t) * 8U) + 2U;
    mask = (mask - 1U) >> 1U;

    while (mask) {
        if ((reg & mask) == mask) {
            /* this is the mask to use */
            base = reg & ~mask;
            break;
        }
        mask >>= 1U;
        numbits--;
    }

    *range = (1LU << numbits);
    return (base << 2U);
}

#ifdef __cplusplus
}
#endif

#endif
