/*******************************************************************************
 * (c) Copyright 2018 Microchip - PRO Embedded Systems Solutions All rights reserved 
 *
 * G5SoC MSS MPU driver
 *
 * SVN $Revision: 10351 $
 * SVN $Date: 2018-09-05 17:02:38 +0530 (Wed, 05 Sep 2018) $
 */
/*=========================================================================*//**
  
 *//*=========================================================================*/
#include "mss_mpu.h"

#ifdef PSE

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
    uint64_t temp = size, cnt=0;
    uint64_t range;
    volatile uint64_t baddr=0;


    /*size must be minimum 4k
      Size must be power of 2
      different masters have different number of regions*/
    if((size >= 4096u) && (0u == (size & (size - 1))) && (pmp_region < num_pmp_lut[master_port]))
    {
        while((0 == (temp & 0x01)))
        {
            cnt++;
            temp >>= 1;
        }

        range = (1ull << (cnt-1u))-1u;

        MSS_MPU(master_port)->PMPCFG[pmp_region].pmp = (base | range) >> 2u;

        MSS_MPU(master_port)->PMPCFG[pmp_region].mode = permission |
                                                        (matching_mode << 3u) |
                                                        (lock_en << 0x7u);

        baddr = (MSS_MPU(master_port)->PMPCFG[pmp_region].mode);
        baddr = (MSS_MPU(master_port)->PMPCFG[pmp_region].pmp);

        return (0);
    }
    else
        return (1);
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
        *matching_mode = (mss_mpu_addrm_t)( reg >> 3);
        *permission = reg & 0x7u;

        return (0);
    }
    else
        return (1);
}

static uint64_t pmp_get_napot_base_and_range(uint64_t reg, uint64_t *range)
{
    /* construct a mask of all bits bar the top bit */
    uint64_t mask = 0u;
    uint64_t base = reg;
    uint64_t numbits = (sizeof(uint64_t) * 8u) + 2u;
    mask = (mask - 1u) >> 1;

    while (mask) {
        if ((reg & mask) == mask) {
            /* this is the mask to use */
            base = reg & ~mask;
            break;
        }
        mask >>= 1;
        numbits--;
    }

    *range = (1lu << numbits);
    return base << 2;
}

#ifdef __cplusplus
}
#endif

#endif
