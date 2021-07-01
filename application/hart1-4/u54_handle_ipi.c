/*******************************************************************************
 * Copyright 2019-2021 Microchip Corporation.
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

bool HSS_U54_HandleIPI(void);
bool HSS_U54_HandleIPI(void)
{
    //mHSS_DEBUG_PRINTF(">>" CRLF);
    bool intentFound = false;

    mb();

    if (IS_ENABLED(CONFIG_SERVICE_BOOT)) { intentFound = IPI_ConsumeIntent(HSS_HART_E51, IPI_MSG_PMP_SETUP); }

    if (IS_ENABLED(CONFIG_SERVICE_GOTO) && (!intentFound)) { intentFound =  IPI_ConsumeIntent(HSS_HART_E51, IPI_MSG_GOTO); }

    if (IS_ENABLED(CONFIG_SERVICE_OPENSBI) && (!intentFound)) { intentFound =  IPI_ConsumeIntent(HSS_HART_E51, IPI_MSG_OPENSBI_INIT); }

    if (IS_ENABLED(CONFIG_DEBUG_IPI_STATS)) {
        enum HSSHartId myHartId = current_hartid();
        static size_t count[5]; count[myHartId]++;

        mHSS_DEBUG_PRINTF(LOG_STATUS, " ipi_interrupts: %" PRIu64 " (%d)" CRLF, count[myHartId],
            intentFound);
        IPI_DebugDumpStats();
    }

    //mb();
    // if not for me, pass to S-mode ...
    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "MTVEC is now %p" CRLF, mHSS_CSR_READ(mtvec));
    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "<<" CRLF);

    return intentFound;
}

void HSS_U54_Banner(void)
{
    // wait for E51 to setup BSS...
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "u54_%d: Waiting for E51 instruction" CRLF, current_hartid());
}
