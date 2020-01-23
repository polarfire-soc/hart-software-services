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
#include <stdbool.h>
#include "mpfs_hal/mss_plic.h"
#include "drivers/mss_mac/mss_ethernet_registers.h"
#include "drivers/mss_mac/mss_ethernet_mac_user_config.h"
#include "drivers/mss_mac/mss_ethernet_mac_regs.h"
#include "drivers/mss_mac/mss_ethernet_mac.h"

/* 
 * Enable this and add your own application specific code if you wish to
 * override the default HAL code.
 */
#if 0
void u54_4(void)
{
    volatile int ix;
    uint32_t hartid = read_csr(mhartid);

    /* add code here */
    while(1)
        ix++;
}
#endif

