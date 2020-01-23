/***********************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
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

