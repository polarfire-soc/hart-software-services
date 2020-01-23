/***********************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * code running on U54 first hart
 *
 * SVN $Revision: 9661 $
 * SVN $Date: 2018-01-15 10:43:33 +0000 (Mon, 15 Jan 2018) $
 */

#include "mpfs_hal/mss_util.h"
#include "mpfs_hal/mss_clint.h"

volatile uint32_t count_sw_ints_h1 = 0;
volatile uint32_t loop_count_h1 = 0;
volatile uint64_t dummy_h1 = 0;

void u54_1(void)
{
    volatile int i;
    int8_t  info_string[100];
    static uint64_t mcycle_start = 0;
    static uint64_t mcycle_end = 0;
    static uint64_t delta_mcycle = 0;
    uint32_t num_loops = 100000;
    uint32_t hartid = read_csr(mhartid);

    while(1)
    {

        mcycle_start = readmcycle();

        for(i = 0; i < num_loops; i++)
        {
            dummy_h1 = i;
        }

        hartid       = read_csr(mhartid);
        mcycle_end   = readmcycle();
        delta_mcycle = mcycle_end - mcycle_start;

        loop_count_h1++;
    }

  /* never return */
}


/**
 *
 */
void Software_h1_IRQHandler(void)
{
    uint32_t hart_id = read_csr(mhartid);
    if(hart_id == 1)
    {
        count_sw_ints_h1++;
    }
}
