/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file  Boot Service State Machine
 * \brief Boot Service
 */

#include "config.h"
#include "hss_types.h"

#include <assert.h>

#include "csr_helper.h"

#include "hss_state_machine.h"
#include "hss_boot_service.h"
#include "hss_boot_pmp.h"
#include "hss_debug.h"
#include "hss_boot_pmp.h"

#define XLEN 64u

///////////////////////////////////////////////////////////////////////////////////////////
//


///////////////////////////////////////////////////////////////////////////////////////////

static struct PmpEntry pmpEntry[HSS_HART_NUM_PEERS][MAX_NUM_PMPS];

static uint64_t pmp_decode_napot_size_encoding(uint64_t addrVal, uint64_t *pMask);
static void pmp_decode(struct PmpEntry *pPmpEntry, struct PmpEntry *pPreviousPmpEntry, uint8_t configVal, uint64_t addrVal);

static inline uint8_t pmp_getConfigVal(size_t index);
static inline uint64_t pmp_getAddrVal(size_t index);

bool HSS_PMP_Init(void)
{
    bool result = true;
    enum HSSHartId target;
    unsigned int pmpIndex;

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Initializing PMPs" CRLF);

    for (target = HSS_HART_U54_1; target <HSS_HART_NUM_PEERS; target++) {
        for (pmpIndex = 0u; pmpIndex < MAX_NUM_PMPS; pmpIndex++) {
#if 0
#if IS_ENABLED(CONFIG_PLATFORM_MPFS)
            const uint64_t configVal = pmp_getConfigVal(pmpIndex);
            const uint64_t addrVal = pmp_getAddrVal(pmpIndex);

            struct PmpEntry *pCurrentEntry = &(pmpEntry[target][pmpIndex]);
            struct PmpEntry *pPreviousEntry = NULL;

            if (pmpIndex) {
                pPreviousEntry = &(pmpEntry[target][pmpIndex-1]);
            }

            pmp_decode(pCurrentEntry, pPreviousEntry, configVal, addrVal);
#endif
#else
            (void)pmp_decode;
            //(void)configVal;
            //(void)addrVal;
            //(void)pCurrentEntry;
            //(void)pPreviousEntry;
#endif
        }
    }

    return result;
}

bool HSS_PMP_CheckWrite(enum HSSHartId target, const ptrdiff_t regionStartAddr, size_t length)
{
    bool result = true;
    unsigned int pmpIndex;

    const ptrdiff_t regionEndAddr = regionStartAddr + length;

    for (pmpIndex = 0u; pmpIndex < MAX_NUM_PMPS; pmpIndex++) {
        struct PmpEntry *pPmpEntry = &(pmpEntry[target][pmpIndex]);

        if (!pPmpEntry->A) { continue; } // inactive PMP

        const ptrdiff_t pmpStartAddr = pPmpEntry->baseAddr;
        const ptrdiff_t pmpEndAddr = pmpStartAddr + pPmpEntry->size;

        if ((pmpStartAddr <= regionStartAddr) && (pmpEndAddr > regionEndAddr)) {
            if (pPmpEntry->W == 0) {
                result = false; // found match disallowing region
                break;
            } else if (pPmpEntry->W != 0) {
                result = true; // found match allowing region
                break;
            }
        }
    }

    return result;
}

bool HSS_PMP_CheckRead(enum HSSHartId target, const ptrdiff_t regionStartAddr, size_t length)
{
    bool result = false;
    unsigned int pmpIndex;

    const ptrdiff_t regionEndAddr = regionStartAddr + length;

    for (pmpIndex = 0u; pmpIndex < MAX_NUM_PMPS; pmpIndex++) {
        struct PmpEntry *pPmpEntry = &(pmpEntry[target][pmpIndex]);

        if (!pPmpEntry->A) { continue; } // inactive PMP

        const ptrdiff_t pmpStartAddr = pPmpEntry->baseAddr;
        const ptrdiff_t pmpEndAddr = pmpStartAddr + pPmpEntry->size;

        if ((pmpStartAddr <= regionStartAddr) && (pmpEndAddr > regionEndAddr)) {
            if (pPmpEntry->R == 0) {
                result = false; // found match disallowing region
                break;
            } else if (pPmpEntry->R != 0) {
                result = true; // found match allowing region
                break;
            }
        }
    }

    return result;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
// Decode Functions
//
static uint64_t pmp_decode_napot_size_encoding(uint64_t addrVal, uint64_t *pMask)
{
    uint64_t mask = 0u;
    uint64_t result = XLEN + 2u;

    mask = (mask - 1u) >> 1;

    while (mask)
    {
        if ((addrVal & mask) == mask)
        {
            *pMask = mask;
            break;
        }

        result = result - 1u;
        mask = mask >> 1;
    }

    return (result);
}

static void pmp_decode(struct PmpEntry *pPmpEntry, struct PmpEntry *pPreviousPmpEntry, uint8_t configVal, uint64_t addrVal)
{
    uint64_t mask = 0u;
    uint64_t range = 0u;

    assert(pPmpEntry != NULL);

    pPmpEntry->A = ((configVal & 0x18) >> 3);

    if (pPmpEntry->A)
    {
        pPmpEntry->R = (configVal & PMP_READ)  ? 1u : 0u;
        pPmpEntry->W = (configVal & PMP_WRITE) ? 1u : 0u;
        pPmpEntry->X = (configVal & PMP_EXEC)  ? 1u : 0u;
        pPmpEntry->L = (configVal & PMP_LOCK)  ? 1u : 0u;
    }

    switch (pPmpEntry->A)
    {
    case AddressMatchingMode_NULL_REGION:
        break;

    case AddressMatchingMode_TOR:
        assert(pPreviousPmpEntry != NULL);
        addrVal <<= 2;                // shift up from [53:0] to [55:2]
        if (pPreviousPmpEntry != NULL) {
            pPmpEntry->baseAddr = pPreviousPmpEntry->baseAddr + pPreviousPmpEntry->size;
        } else {
            pPmpEntry->baseAddr = 0u;
        }
        pPmpEntry->size = addrVal - pPmpEntry->baseAddr;
        break;

    case AddressMatchingMode_NA4:
        addrVal <<= 2;                // shift up from [53:0] to [55:2]
        pPmpEntry->baseAddr = addrVal;
        pPmpEntry->size = 4u;
        break;

    case AddressMatchingMode_NAPOT:
        range = pmp_decode_napot_size_encoding(addrVal, &mask);
        addrVal = addrVal & (~mask);  // remove the size encoding
        addrVal <<= 2;                // shift up from [53:0] to [55:2]
        pPmpEntry->baseAddr = addrVal;
        pPmpEntry->size = (1lu << range);
        break;

    default:
        break;
    }
}

static inline uint8_t pmp_getConfigVal(size_t index)
{
    uint64_t result;

    switch (index) {
    case 0:  result = (uint64_t)mHSS_CSR_READ(pmpcfg0); break;
    case 1:  result = (uint64_t)mHSS_CSR_READ(pmpcfg0); break;
    case 2:  result = (uint64_t)mHSS_CSR_READ(pmpcfg0); break;
    case 3:  result = (uint64_t)mHSS_CSR_READ(pmpcfg0); break;
    case 4:  result = (uint64_t)mHSS_CSR_READ(pmpcfg0); break;
    case 5:  result = (uint64_t)mHSS_CSR_READ(pmpcfg0); break;
    case 6:  result = (uint64_t)mHSS_CSR_READ(pmpcfg0); break;
    case 7:  result = (uint64_t)mHSS_CSR_READ(pmpcfg0); break;

    case 8:  result = (uint64_t)mHSS_CSR_READ(pmpcfg1); break;
    case 9:  result = (uint64_t)mHSS_CSR_READ(pmpcfg1); break;
    case 10: result = (uint64_t)mHSS_CSR_READ(pmpcfg1); break;
    case 11: result = (uint64_t)mHSS_CSR_READ(pmpcfg1); break;
    case 12: result = (uint64_t)mHSS_CSR_READ(pmpcfg1); break;
    case 13: result = (uint64_t)mHSS_CSR_READ(pmpcfg1); break;
    case 14: result = (uint64_t)mHSS_CSR_READ(pmpcfg1); break;
    case 15: result = (uint64_t)mHSS_CSR_READ(pmpcfg1); break;
    default: result = 0u;                break;
    }

    if (index < 8u) {
        result = result >> (index * 8u);
    } else if (index < 15) {
        result = result >> ((index - 8u) * 8u);
    }

    return (uint8_t)(result & 0xFFu);
}

static inline uint64_t pmp_getAddrVal(size_t index)
{
    uint64_t result;

    switch (index) {
    case 0:  result = (uint64_t)mHSS_CSR_READ(pmpaddr0);  break;
    case 1:  result = (uint64_t)mHSS_CSR_READ(pmpaddr1);  break;
    case 2:  result = (uint64_t)mHSS_CSR_READ(pmpaddr2);  break;
    case 3:  result = (uint64_t)mHSS_CSR_READ(pmpaddr3);  break;
    case 4:  result = (uint64_t)mHSS_CSR_READ(pmpaddr4);  break;
    case 5:  result = (uint64_t)mHSS_CSR_READ(pmpaddr5);  break;
    case 6:  result = (uint64_t)mHSS_CSR_READ(pmpaddr6);  break;
    case 7:  result = (uint64_t)mHSS_CSR_READ(pmpaddr7);  break;
    case 8:  result = (uint64_t)mHSS_CSR_READ(pmpaddr8);  break;
    case 9:  result = (uint64_t)mHSS_CSR_READ(pmpaddr9);  break;
    case 10: result = (uint64_t)mHSS_CSR_READ(pmpaddr10); break;
    case 11: result = (uint64_t)mHSS_CSR_READ(pmpaddr11); break;
    case 12: result = (uint64_t)mHSS_CSR_READ(pmpaddr12); break;
    case 13: result = (uint64_t)mHSS_CSR_READ(pmpaddr13); break;
    case 14: result = (uint64_t)mHSS_CSR_READ(pmpaddr14); break;
    case 15: result = (uint64_t)mHSS_CSR_READ(pmpaddr15); break;
    default: result = 0u;                 break;
    }

    return result;
}

