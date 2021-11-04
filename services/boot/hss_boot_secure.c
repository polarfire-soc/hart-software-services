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

static void __attribute__((__noreturn__)) boot_secure_failure_(uint8_t checkpoint)
{
    // TODO: assert tamper flag?

    mHSS_DEBUG_PRINTF(LOG_ERROR, "failure during code signature verification (%u)..." CRLF, checkpoint);

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

    const struct HSS_Signature originalSig = pBootImage->signature;
    memset((void *)&(pBootImage->signature), 0, sizeof(struct HSS_Signature));

    struct HSS_Signature validation __attribute__((aligned));

    HSS_PerfCtr_Allocate(&perf_ctr_index, "SecureBoot");
    HSS_PerfCtr_Start(perf_ctr_index);

    //
    // first, check the signature of the message digest to make sure it is okay ...
    //
    result = HSS_Crypto_Verify_ECDSA_P384(ARRAY_SIZE(validation.ecdsaSig), &(validation.ecdsaSig[0]),
        pBootImage->bootImageLength, (uint8_t *)pBootImage);

    if (!result) {
        boot_secure_failure_(1u);
    } else {
        // as the signature has now validated the message digest value, calculate an equivalent
        // digest over the boot image, and verify against the known good digest value...
        result = HSS_Crypto_SHA384(ARRAY_SIZE(validation.digest), &(validation.digest[0]),
            pBootImage->bootImageLength, (uint8_t *)pBootImage);

        if (!result) {
            HSS_PerfCtr_Lap(perf_ctr_index);
            boot_secure_failure_(2u);
        } else {
            // check digest against boot image
            result = (0 == strncmp((char *)&(validation.digest[0]), (const char * const)&(originalSig.digest[0]),
                ARRAY_SIZE(validation.digest)));

            if (!result) {
                HSS_PerfCtr_Lap(perf_ctr_index);
                boot_secure_failure_(3u);
            } else {
                HSS_PerfCtr_Lap(perf_ctr_index);
                result = true;
            }
        }
    }

    return result;
}
