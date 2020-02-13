/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * 
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file CSR Helper
 * \brief CSR Helper
 */

#include "config.h"
#include "hss_types.h"

#include "csr_helper.h"
#include "ssmb_ipi.h"
#include "hss_debug.h"

#include "mpfs_reg_map.h"


#include <assert.h>

#ifdef CONFIG_SUPERLOOP_IN_U_MODE
static struct {
    unsigned int hartID;
    unsigned int vendorID;
    unsigned int archID;
    unsigned int misa;
} csrHelperPrivateData;

void CSR_Init(void)
{
    csrHelperPrivateData.hartID = mHSS_CSR_READ(mhartid);
    csrHelperPrivateData.vendorID = mHSS_CSR_READ(mvendorid);
    csrHelperPrivateData.archID = mHSS_CSR_READ(marchid);
    csrHelperPrivateData.misa = mHSS_CSR_READ(misa);
}
#endif

__attribute__((weak)) enum HSSHartId CSR_GetHartId(void) 
{
    enum HSSHartId result;

#ifdef __riscv
#  ifdef CONFIG_SUPERLOOP_IN_U_MODE
    result = csrHelperPrivateData.hartID;
#  else
    result = mHSS_CSR_READ(mhartid);
#  endif
#else
    result = 0;
#endif

    assert((result >= HSS_HART_E51) && (result < HSS_HART_NUM_PEERS));
    return result;
}

unsigned long CSR_GetVendorID(void)
{
    unsigned long result;

#ifdef CONFIG_SUPERLOOP_IN_U_MODE
    result = csrHelperPrivateData.vendorID;
#else
    result = mHSS_CSR_READ(mvendorid);
#endif

    return result;
}

unsigned long CSR_GetArchID(void)
{
    unsigned long result;

#ifdef CONFIG_SUPERLOOP_IN_U_MODE
    result = csrHelperPrivateData.archID;
#else
    result = mHSS_CSR_READ(marchid);
#endif

    return result;
}

unsigned long CSR_GetMISA(void)
{
    unsigned long result;

#ifdef CONFIG_SUPERLOOP_IN_U_MODE
    result = csrHelperPrivateData.misa;
#else
    result = mHSS_CSR_READ(misa);
#endif

    return result;
}

HSSTicks_t CSR_GetTickCount(void)
{
    HSSTicks_t tickCount;

#ifdef CONFIG_SUPERLOOP_IN_U_MODE
    tickCount = mHSS_CSR_READ(cycle);
#else
    tickCount = mHSS_CSR_READ(mcycle);
#endif

    return tickCount;
}

HSSTicks_t CSR_GetTime(void)
{
    HSSTicks_t time;

#ifdef CONFIG_SUPERLOOP_IN_U_MODE
    time = mHSS_CSR_READ(time);
#else
    time = mHSS_ReadRegU64(CLINT, MTIME);
#endif

    return time;
}
