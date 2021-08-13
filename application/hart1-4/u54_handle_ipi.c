/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file U54 Handle IPI
 * \brief U54 Handle IPI
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"

#include "ssmb_ipi.h"

#include <assert.h>

#include "csr_helper.h"

#include "ssmb_ipi.h"

#include "goto_service.h"
#include "opensbi_service.h"

#if IS_ENABLED(CONFIG_SERVICE_BOOT)
#  include "hss_boot_pmp.h"
#endif

#include "mpfs_reg_map.h"
#include "csr_helper.h"
#include "u54_handle_ipi.h"

#include "hss_atomic.h"

//
// This routine gets executed when a U54 receives an IPI from E51 in machine mode...
//

static const struct IntentsArray
{
    bool enabled;
    enum IPIMessagesEnum msg_type;
} intentsArray[] = {
    { IS_ENABLED(CONFIG_SERVICE_BOOT),   	IPI_MSG_PMP_SETUP },
    { IS_ENABLED(CONFIG_SERVICE_GOTO),   	IPI_MSG_GOTO },
    { IS_ENABLED(CONFIG_SERVICE_OPENSBI),	IPI_MSG_OPENSBI_INIT }
};


bool HSS_U54_HandleIPI(void);
bool HSS_U54_HandleIPI(void)
{
    //mHSS_DEBUG_PRINTF(">>" CRLF);
    bool intentFound = false;
    enum HSSHartId myHartId = current_hartid();

    mb();

    for (int i = 0; i < ARRAY_SIZE(intentsArray); i++) {
        if (intentsArray[i].enabled) { intentFound = IPI_ConsumeIntent(HSS_HART_E51, intentsArray[i].msg_type) | intentFound; }
    }

    // in testing, for harts that aren't in either its domain or the root domain, U-Boot still seems to be generating some
    // unexpected MSIP interrupts which otherwise never get cleared so if not using OpenSBI we'll clear explicitly here...
    if (intentFound || !mpfs_is_hart_using_opensbi(myHartId)) {
        CSR_ClearMSIP();
    }

    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "<<" CRLF);

    return intentFound;
}

void HSS_U54_Banner(void)
{
    // wait for E51 to setup BSS...
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "u54_%d: Waiting for E51 instruction" CRLF, current_hartid());
}
