/***********************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * code running on U54 fourth hart
 *
 * SVN $Revision: 9661 $
 * SVN $Date: 2018-01-15 10:43:33 +0000 (Mon, 15 Jan 2018) $
 */

#include "mpfs_hal/mss_util.h"

void u54_4(void)
{
    volatile int ix = 0u;
    uint32_t hartid = read_csr(mhartid);

    /* add code here */
    while(1)
        ix++;
}
