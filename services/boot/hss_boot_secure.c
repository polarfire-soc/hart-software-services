/*******************************************************************************
 * Copyright 2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file  Boot Service Image Signing
 * \brief Boot Service
 */


#include "config.h"
#include "hss_types.h"
#include "hss_clock.h"
#include "hss_debug.h"
#include "hss_perfctr.h"
#include "hss_crypto.h"
#include "hss_boot_secure.h"

#include <assert.h>
#include <string.h>

static void __attribute__((__noreturn__)) boot_secure_failure_(void)
{
    // TODO: assert tamper flag?

    mHSS_DEBUG_PRINTF(LOG_ERROR, "failure during signature verification..." CRLF);

    while (1) {
        // spin forever
    };

    // will never be reached
    __builtin_unreachable();
}

static int perf_ctr_index = PERF_CTR_UNINITIALIZED;

bool HSS_Boot_Secure_CheckCodeSigning(struct HSS_BootImage *pBootImage)
{
    bool result = false;

    assert(pBootImage != NULL);

    struct HSS_Signature originalSig __attribute__((aligned)) = pBootImage->signature;
    memset((void *)&(pBootImage->signature), 0, sizeof(struct HSS_Signature));

    HSS_PerfCtr_Allocate(&perf_ctr_index, "SecureBoot");
    HSS_PerfCtr_Start(perf_ctr_index);

    result = HSS_Crypto_Verify_ECDSA_P384(ARRAY_SIZE(originalSig.ecdsaSig), &(originalSig.ecdsaSig[0]),
        pBootImage->bootImageLength, (uint8_t *)pBootImage);

    if (!result) {
        boot_secure_failure_();
    } else {
        mHSS_DEBUG_PRINTF(LOG_STATUS, "ECDSA verification passed" CRLF);
    }

    HSS_PerfCtr_Lap(perf_ctr_index);

    return result;
}
