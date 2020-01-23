/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
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

#ifdef CONFIG_SERVICE_BOOT
#  include "hss_boot_pmp.h"
#endif

#include "mpfs_reg_map.h"
#include "csr_helper.h"
#include "u54_handle_ipi.h"

#include "hss_atomic.h"

//
// This routine gets executed when a U54 receives an IPI from E51 in machine mode...
//

bool U54_HandleIPI(void)
{
    mHSS_DEBUG_PRINTF(">>" CRLF);
    volatile bool intentFound = false;

    mb();

#ifdef CONFIG_SERVICE_BOOT
    /*if (!intentFound)*/ 
    { intentFound = IPI_ConsumeIntent(HSS_HART_E51, IPI_MSG_PMP_SETUP); }
#endif

#ifdef CONFIG_SERVICE_GOTO
    if (!intentFound) { intentFound =  IPI_ConsumeIntent(HSS_HART_E51, IPI_MSG_GOTO); }
#endif

#ifdef CONFIG_SERVICE_OPENSBI
    if (!intentFound) { intentFound =  IPI_ConsumeIntent(HSS_HART_E51, IPI_MSG_OPENSBI_INIT); }
#endif

     CLINT_Clear_MSIP(CSR_GetHartId());

#ifdef CONFIG_DEBUG_IPI_STATS
    {
        enum HSSHartId myHartId = CSR_GetHartId();
        static size_t count[5]; count[myHartId]++; 

        mHSS_DEBUG_STATUS_TEXT;
        mHSS_DEBUG_PRINTF(" ipi_interrupts: %" PRIu64 " (%d)" CRLF, count[myHartId], intentFound);
        IPI_DebugDumpStats();
        mHSS_DEBUG_NORMAL_TEXT;
    }
#endif

    //mb();
    // if not for me, pass to S-mode ...
    mHSS_DEBUG_PRINTF("<<" CRLF);

    return intentFound; 
}

void u54_banner(void);

void u54_banner(void)
{
    // wait for E51 to setup BSS...
    int msip = MIP_MSIP;
    asm volatile("wfi\n\
    	    csrc mip, %0" : : "r"(msip));

    CLINT_Clear_MSIP(CSR_GetHartId());

    asm volatile("csrw mstatus, %0" : : "r"(msip));

    mHSS_DEBUG_PRINTF("u54_%d: Waiting for E51 instruction" CRLF, CSR_GetHartId());
}
