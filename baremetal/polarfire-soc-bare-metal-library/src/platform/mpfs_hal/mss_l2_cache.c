/*******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */
/*==============================================================================
 * The code in this file is executed before any code/data sections are copied.
 * This code must not rely sdata/data section content. Hence, global variables
 * should not be used unless they are constants.
 */

#include <stdio.h>
#include <string.h>
#include "mss_hal.h"
#include "mss_l2_cache.h"

/*==============================================================================
 * Define describing cache characteristics.
 */
#define MAX_WAY_ENABLE          15
#define NB_SETS                 512
#define NB_BANKS                4
#define CACHE_BLOCK_BYTE_LENGTH 64
#define UINT64_BYTE_LENGTH      8
#define WAY_BYTE_LENGTH         (CACHE_BLOCK_BYTE_LENGTH * NB_SETS * NB_BANKS)

#define ZERO_DEVICE_BOTTOM  0x0A000000ULL
#define ZERO_DEVICE_TOP     0x0C000000ULL

#define CACHE_CTRL_BASE     0x02010000ULL

#define INIT_MARKER     0xC0FFEEBEC0010000ULL

static const uint64_t g_init_marker = INIT_MARKER;

/*==============================================================================
 * Cache controller registers definitions
 */
#define RO  volatile const
#define RW  volatile
#define WO  volatile

typedef struct {
  RO uint8_t  BANKS;
  RO uint8_t  WAYS;
  RO uint8_t  SETS;
  RO uint8_t  BYTES;
} CACHE_CONFIG_typedef;

typedef struct {
  CACHE_CONFIG_typedef CONFIG;
  RO uint32_t RESERVED;
  RW uint8_t  WAY_ENABLE;
  RO uint8_t  RESERVED0[55];

  WO uint32_t ECC_INJECT_ERROR;
  RO uint32_t RESERVED1[47];

  RO uint64_t ECC_DIR_FIX_ADDR;
  RO uint32_t ECC_DIR_FIX_COUNT;
  RO uint32_t RESERVED2[13];

  RO uint64_t ECC_DATA_FIX_ADDR;
  RO uint32_t ECC_DATA_FIX_COUNT;
  RO uint32_t RESERVED3[5];

  RO uint64_t ECC_DATA_FAIL_ADDR;
  RO uint32_t ECC_DATA_FAIL_COUNT;
  RO uint32_t RESERVED4[37];

  WO uint64_t FLUSH64;
  RO uint64_t RESERVED5[7];

  WO uint32_t FLUSH32;
  RO uint32_t RESERVED6[367];

  RW uint64_t WAY_MASK_DMA;

  RW uint64_t WAY_MASK_AXI4_SLAVE_PORT_0;
  RW uint64_t WAY_MASK_AXI4_SLAVE_PORT_1;
  RW uint64_t WAY_MASK_AXI4_SLAVE_PORT_2;
  RW uint64_t WAY_MASK_AXI4_SLAVE_PORT_3;

  RW uint64_t WAY_MASK_E51_DCACHE;
  RW uint64_t WAY_MASK_E51_ICACHE;

  RW uint64_t WAY_MASK_U54_1_DCACHE;
  RW uint64_t WAY_MASK_U54_1_ICACHE;

  RW uint64_t WAY_MASK_U54_2_DCACHE;
  RW uint64_t WAY_MASK_U54_2_ICACHE;

  RW uint64_t WAY_MASK_U54_3_DCACHE;
  RW uint64_t WAY_MASK_U54_3_ICACHE;

  RW uint64_t WAY_MASK_U54_4_DCACHE;
  RW uint64_t WAY_MASK_U54_4_ICACHE;
} CACHE_CTRL_typedef;

#define CACHE_CTRL  ((CACHE_CTRL_typedef *) CACHE_CTRL_BASE)


/*==============================================================================
 * Local functions.
 */
static void config_l2_scratchpad(void);
static void reserve_scratchpad_ways(int nways, uint64_t * scratchpad_start);

/**
 * \brief L2 waymask configuration settings from Libero
 *
 */
const uint64_t    way_mask_values[] = {
    LIBERO_SETTING_WAY_MASK_M0,
    LIBERO_SETTING_WAY_MASK_M1,
    LIBERO_SETTING_WAY_MASK_M2,
    LIBERO_SETTING_WAY_MASK_M3,
    LIBERO_SETTING_WAY_MASK_M4
};


/*==============================================================================
 * This code should only be executed from E51 to be functional.
 * Configure the L2 cache memory:
 *  - Set the number of cache ways used as cache based on the MSS Configurator
 *    settings.
 *  - Configure some of the enabled ways as scratchpad based on linker
 *    configuration.
 */
void config_l2_cache(void)
{
    /*
     * Set the number of ways that will be shared between cache and scratchpad.
     */
    CACHE_CTRL->WAY_ENABLE = LIBERO_SETTING_WAY_ENABLE;

    config_l2_scratchpad();
}


/*==============================================================================
 * Configure the L2 scratchpad based on linker symbols:
 *  __l2_scratchpad_vma_start
 *  __l2_scratchpad_vma_end
 *
 *  These linker symbols specify the start address and length of the scratchpad.
 *  The scratchpad must be located within the Sero Device memory range.
 */
static void config_l2_scratchpad(void)
{
    extern char __l2_scratchpad_vma_start;
    extern char __l2_scratchpad_vma_end;
    int n_scratchpad_ways;
    const uint64_t end = (const uint64_t)&__l2_scratchpad_vma_end;
    const uint64_t start = (const uint64_t)&__l2_scratchpad_vma_start;
    uint64_t modulo;

    /*
     * Figure out how many cache ways will be required from linker script
     * symbols.
     */
    n_scratchpad_ways = (end - start) / WAY_BYTE_LENGTH;
    modulo = (end - start) % WAY_BYTE_LENGTH;
    if(modulo > 0)
    {
        ++n_scratchpad_ways;
    }

    if(n_scratchpad_ways > 0)
    {
        reserve_scratchpad_ways(n_scratchpad_ways, (uint64_t *)start);
    }
}

/*==============================================================================
 * Reserve a number of cache ways to be used as scratchpad memory.
 *
 * @param nways
 *  Number of ways to be used as scratchpad. One way is 128Kbytes.
 *
 * @param scratchpad_start
 *  Start address within the Zero Device memory range in which the scratchpad
 *  will be located.
 */
static void reserve_scratchpad_ways(int nways, uint64_t * scratchpad_start)
{
    uint64_t way_enable;
    uint64_t available_ways = 1;
    uint64_t scratchpad_ways = 0;
    uint64_t non_scratchpad_ways;
    int inc;
    int ways_inc;

    ASSERT(scratchpad_start >= (uint64_t *)ZERO_DEVICE_BOTTOM);
    ASSERT(scratchpad_start < (uint64_t *)ZERO_DEVICE_TOP);

    /*
     * Ensure at least one way remains available as cache.
     */
    way_enable = CACHE_CTRL->WAY_ENABLE;
    ASSERT(nways <= way_enable);
    if(nways <= way_enable)
    {
        /*
         * Compute the mask used to specify ways that will be used by the
         * scratchpad.
         */

        for(inc = 0; inc < way_enable; ++inc)
        {
            available_ways = (available_ways << 1) | (uint64_t)0x01;
            if(inc < nways)
            {
                scratchpad_ways = (scratchpad_ways << 1) | (uint64_t)0x01;
            }
        }

        /*
         * Prevent other masters from evicting cache lines from scratchpad ways.
         * Only allow E51 to evict from scratchpad ways.
         */
        non_scratchpad_ways = available_ways &  ~scratchpad_ways;

        CACHE_CTRL->WAY_MASK_DMA = non_scratchpad_ways;

        CACHE_CTRL->WAY_MASK_AXI4_SLAVE_PORT_0 = non_scratchpad_ways;
        CACHE_CTRL->WAY_MASK_AXI4_SLAVE_PORT_1 = non_scratchpad_ways;
        CACHE_CTRL->WAY_MASK_AXI4_SLAVE_PORT_2 = non_scratchpad_ways;
        CACHE_CTRL->WAY_MASK_AXI4_SLAVE_PORT_3 = non_scratchpad_ways;

        CACHE_CTRL->WAY_MASK_E51_ICACHE = non_scratchpad_ways;

        CACHE_CTRL->WAY_MASK_U54_1_DCACHE = non_scratchpad_ways;
        CACHE_CTRL->WAY_MASK_U54_1_ICACHE = non_scratchpad_ways;

        CACHE_CTRL->WAY_MASK_U54_2_DCACHE = non_scratchpad_ways;
        CACHE_CTRL->WAY_MASK_U54_2_ICACHE = non_scratchpad_ways;

        CACHE_CTRL->WAY_MASK_U54_3_DCACHE = non_scratchpad_ways;
        CACHE_CTRL->WAY_MASK_U54_3_ICACHE = non_scratchpad_ways;

        CACHE_CTRL->WAY_MASK_U54_4_DCACHE = non_scratchpad_ways;
        CACHE_CTRL->WAY_MASK_U54_4_ICACHE = non_scratchpad_ways;

        /*
         * Assign ways to Zero Device
         */
        uint64_t * p_scratchpad = scratchpad_start;

        uint64_t current_way = 1;
        for(ways_inc = 0; ways_inc < nways; ++ways_inc)
        {
            /*
             * Populate the scratchpad memory one way at a time.
             */
            CACHE_CTRL->WAY_MASK_E51_DCACHE = current_way;
            /*
             * Write to the first 64-bit location of each cache block.
             */
            for(inc = 0; inc < (WAY_BYTE_LENGTH / CACHE_BLOCK_BYTE_LENGTH); ++inc)
            {
                *p_scratchpad = g_init_marker + inc;
                p_scratchpad += CACHE_BLOCK_BYTE_LENGTH / UINT64_BYTE_LENGTH;
            }
            current_way = current_way << 1U;
            mb();
        }

        /*
         * Prevent E51 from evicting from scratchpad ways.
         */
        CACHE_CTRL->WAY_MASK_E51_DCACHE = non_scratchpad_ways;
    }
}
