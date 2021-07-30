/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file GUID Partition Table (GPT) Routines
 * \brief GUID Partition Table (GPT) Routines
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"
#include "hss_crc32.h"
#include "gpt.h"

#include <string.h>
#include <assert.h>

#undef GPT_DEBUG
//#define GPT_DEBUG

//
// local modules function pointer types
typedef bool (*ReadBlockFnPtr_t)(void *pDest, size_t srcOffset, size_t byteCount);
typedef bool (*CheckIfGUIDMatchFnPtr_t)(HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry,
    HSS_GPT_GUID_t const * const pGUID2);

static ReadBlockFnPtr_t readBlockFnPtr;

//
// local module function prototypes
static bool CheckIfGUIDMatch_(HSS_GPT_GUID_t const * const pGUID1,
    HSS_GPT_GUID_t const * const pGUID2);
static bool CheckIfTypeIdMatch_(HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry,
    HSS_GPT_GUID_t const * const pGUID);
static bool CheckIfUniqueIdMatch_(HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry,
    HSS_GPT_GUID_t const * const pGUID);
static bool FindPartitionById_(HSS_GPT_t const * const pGpt,
    HSS_GPT_GUID_t const * const pGUID,
    size_t * const pPartitionIndex, CheckIfGUIDMatchFnPtr_t pCheckIfMatchFunc,
    HSS_GPT_PartitionEntry_t const ** ppGptPartitionEntryOut);

//
//

void GPT_RegisterReadBlockFunction(bool (*fnPtr)(void *pDest, size_t srcOffset, size_t byteCount))
{
    assert(fnPtr);

    readBlockFnPtr = fnPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Debug Routines
//

#ifdef GPT_DEBUG
static const HSS_GPT_GUID_t nullGUID = {
    .data1 = 0u,
    .data2 = 0u,
    .data3 = 0u,
    .data4 = 0u
};
#endif


//
//
//
void GPT_DumpHeaderInfo(HSS_GPT_t *pGpt)
{
#ifdef GPT_DEBUG
    HSS_GPT_Header_t const * const pGptHeader = &(pGpt->h.header);
    assert(pGptHeader != NULL);

    mHSS_DEBUG_PRINTF(LOG_STATUS, "Signature:             %c%c%c%c%c%c%c%c (%016lx)" CRLF,
        pGptHeader->s.c[0], pGptHeader->s.c[1], pGptHeader->s.c[2], pGptHeader->s.c[3],
        pGptHeader->s.c[4], pGptHeader->s.c[5], pGptHeader->s.c[6], pGptHeader->s.c[7],
        pGptHeader->s.signature);
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Revision:              0x%08x" CRLF, pGptHeader->revision);
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Header Size:           0x%08x" CRLF, pGptHeader->headerSize);
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Header CRC:            0x%08x" CRLF, pGptHeader->headerCrc32);
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Current LBA:           0x%016lx" CRLF, pGptHeader->currentLBA);
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Backup LBA:            0x%016lx" CRLF, pGptHeader->backupLBA);
    mHSS_DEBUG_PRINTF(LOG_STATUS, "First Usable LBA:      0x%016lx" CRLF, pGptHeader->firstUsableLBA);
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Last Usable LBA:       0x%016lx" CRLF, pGptHeader->lastUsableLBA);
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Disk GUID:             %08x-%04x-%04x-%016lx" CRLF,
        pGptHeader->diskGUID.data1, pGptHeader->diskGUID.data2,
        pGptHeader->diskGUID.data3, __builtin_bswap64(pGptHeader->diskGUID.data4));
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Starting LBA:          0x%016lx" CRLF,
        pGptHeader->partitionEntriesStartingLBA);
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Num. Partitions:       0x%08x" CRLF, pGptHeader->numPartitions);
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Size of Partition:     0x%08x" CRLF,
        pGptHeader->sizeOfPartitionEntry);
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Partition Entries CRC: 0x%08x" CRLF,
        pGptHeader->partitionEntriesArrayCrc32);
#else
    (void)pGpt;
#endif
}

//
//
//
void GPT_DumpPartitionInfo(HSS_GPT_t const * const pGpt,
    HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry)
{
#ifdef GPT_DEBUG
    assert(pGpt != NULL);
    assert(pGptPartitionEntry != NULL);

    mHSS_DEBUG_PRINTF(LOG_STATUS, "Type GUID:   %08x-%04x-%04x-%016lx" CRLF,
        pGptPartitionEntry->partitionTypeGUID.data1, pGptPartitionEntry->partitionTypeGUID.data2,
        pGptPartitionEntry->partitionTypeGUID.data3, pGptPartitionEntry->partitionTypeGUID.data4);
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Unique GUID: %08x-%04x-%04x-%016lx" CRLF,
        pGptPartitionEntry->uniquePartitionGUID.data1, pGptPartitionEntry->uniquePartitionGUID.data2,
        pGptPartitionEntry->uniquePartitionGUID.data3, pGptPartitionEntry->uniquePartitionGUID.data4);

   mHSS_DEBUG_PRINTF(LOG_STATUS, "First LBA:    %016lx" CRLF, pGptPartitionEntry->firstLBA);
   mHSS_DEBUG_PRINTF(LOG_STATUS, "Last LBA:     %016lx" CRLF, pGptPartitionEntry->lastLBA);
   mHSS_DEBUG_PRINTF(LOG_STATUS, "Attributes:   %016lx" CRLF, pGptPartitionEntry->attributes);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

//
//
//
bool bootPartitionIndexValid = false;
size_t bootPartitionIndex = 0u;
void GPT_SetBootPartitionIndex(HSS_GPT_t *pGpt, size_t index)
{
    assert(pGpt);

    /*pGpt->*/bootPartitionIndexValid = true;
    /*pGpt->*/bootPartitionIndex = index;
}

//
//
//
bool GPT_GetBootPartitionIndex(HSS_GPT_t *pGpt, size_t *pIndex)
{
    assert(pGpt);
    assert(pIndex);

    if (/*pGpt->*/bootPartitionIndexValid) {
        *pIndex = /*pGpt->*/bootPartitionIndex;
    }

    return /*pGpt->*/bootPartitionIndexValid;
}

//
//
//
bool GPT_ValidateHeader(HSS_GPT_t *pGpt)
{
    bool result = true;

    assert(pGpt != NULL);

    HSS_GPT_Header_t * const pGptHeader = &(pGpt->h.header);

    result = (!strncmp(pGptHeader->s.c, (char *)GPT_EXPECTED_SIGNATURE, 8));
    if (!result) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "GPT signature not as expected" CRLF);
    } else {
        result = (pGptHeader->revision == GPT_EXPECTED_REVISION);

        if (!result) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "GPT header revision is %08x vs expected %08x" CRLF,
                pGptHeader->revision, GPT_EXPECTED_REVISION);
        } else {
            uint32_t origChecksum = pGptHeader->headerCrc32;
            pGptHeader->headerCrc32 = 0u;
            uint32_t checksum = CRC32_calculate((const uint8_t *)pGptHeader, pGptHeader->headerSize);
            pGptHeader->headerCrc32 = origChecksum;

            result = (checksum == origChecksum);

            if (!result) {
                mHSS_DEBUG_PRINTF(LOG_ERROR, "GPT header CRC32 is %08x vs expected %08x" CRLF,
                    checksum, origChecksum);
            } else {
                result = (pGptHeader->currentLBA == 1u);

                if (!result) {
                    mHSS_DEBUG_PRINTF(LOG_ERROR, "GPT current LBA is %lu vs expected %u" CRLF,
                        pGptHeader->currentLBA, 1u);
                } else {
                    result = (pGptHeader->partitionEntriesStartingLBA == 2u);

                    if (!result) {
                        mHSS_DEBUG_PRINTF(LOG_ERROR, "GPT starting LBA of array of partition entries is %lu"
                            " vs expected %u" CRLF, pGptHeader->currentLBA, 2u);
                    }
                }
            }
        }
    }

    if (result) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Validated GPT Header ..." CRLF);
        pGpt->headerValid = true;
    }
    return result;
}

//
//
//
bool GPT_ReadHeader(HSS_GPT_t *pGpt)
{
    assert(pGpt != NULL);
    assert(readBlockFnPtr != NULL);

    bool result = false;

    result = readBlockFnPtr(pGpt->h.buffer, 1u * GPT_LBA_SIZE, GPT_LBA_SIZE);
    if (!result) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "Unable to read block for LBA 1" CRLF);
    } else {
            result = GPT_ValidateHeader(pGpt);
    }

    return result;
}

//
//
//
static HSS_GPT_PartitionEntry_t const * ReadPartitionEntryIntoBuffer_(HSS_GPT_t const * const pGpt,
    size_t lbaIndex, size_t partitionIndex)
{
    assert(readBlockFnPtr != NULL);
    assert(pGpt!= NULL);

    HSS_GPT_Header_t const * const pGptHeader = &(pGpt->h.header);
    uint8_t const * const pLBABuffer = pGpt->lbaBuffer;

    HSS_GPT_PartitionEntry_t *pResult = NULL;
    bool retVal = false;

    const size_t offset =
        (pGptHeader->sizeOfPartitionEntry * partitionIndex) -
            ((lbaIndex - pGptHeader->partitionEntriesStartingLBA) * GPT_LBA_SIZE);

    retVal = readBlockFnPtr((void *)pLBABuffer, lbaIndex * GPT_LBA_SIZE, GPT_LBA_SIZE);
    if (!retVal) {
        mHSS_DEBUG_PRINTF(LOG_ERROR,
            "Unable to read block for LBA %lu (partition entry %lu)" CRLF,
            lbaIndex, partitionIndex);
    } else {
        if (offset >= GPT_LBA_SIZE) {
            retVal = readBlockFnPtr((void *)(pLBABuffer + GPT_LBA_SIZE),
                (lbaIndex + 1u) * GPT_LBA_SIZE, GPT_LBA_SIZE);
            if (!retVal) {
                mHSS_DEBUG_PRINTF(LOG_ERROR,
                    "Unable to read block for LBA %lu (partition entry %lu)" CRLF,
                    lbaIndex, partitionIndex);
            }
        }
    }

    if (retVal) {
        pResult = (HSS_GPT_PartitionEntry_t *)(pLBABuffer + offset);
    }

    return pResult;;
}

//
//
//
static bool CheckIfGUIDMatch_(HSS_GPT_GUID_t const * const pGUID1,
    HSS_GPT_GUID_t const * const pGUID2)
{
    bool result = false;

    result = ((pGUID1->data1 == pGUID2->data1)
        && (pGUID1->data2 == pGUID2->data2)
        && (pGUID1->data3 == pGUID2->data3)
        && (pGUID1->data4 == pGUID2->data4));

    return result;
}

//
//
//
static bool CheckIfTypeIdMatch_(HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry,
    HSS_GPT_GUID_t const * const pGUID)
{
    bool result = false;

#ifdef GPT_DEBUG
    mHSS_DEBUG_PRINTF(LOG_NORMAL, " - Unique GUID: %08x-%04x-%04x-%016lx" CRLF,
        pGUID->data1, pGUID->data2, pGUID->data3, __builtin_bswap64(pGUID->data4));

    HSS_GPT_GUID_t const * pDbgGUID = &(pGptPartitionEntry->partitionTypeGUID);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, " - Type GUID:   %08x-%04x-%04x-%016lx" CRLF,
        pDbgGUID->data1, pDbgGUID->data2, pDbgGUID->data3, __builtin_bswap64(pDbgGUID->data4));
#endif
    result = CheckIfGUIDMatch_(&(pGptPartitionEntry->partitionTypeGUID), pGUID);
    return result;
}

//
//
//
static bool CheckIfUniqueIdMatch_(HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry,
    HSS_GPT_GUID_t const * const pGUID)
{
    bool result = false;

    result = CheckIfGUIDMatch_(&(pGptPartitionEntry->uniquePartitionGUID), pGUID);
    return result;
}

//
//
//
bool GPT_PartitionIdToLBAOffset(HSS_GPT_t const * const pGpt, size_t partitionIndex, size_t * const pFirstLBA)
{
    bool result = false;

    assert(pGpt != NULL);
    assert(pFirstLBA != NULL);

    HSS_GPT_Header_t const * const pGptHeader = &(pGpt->h.header);

    const size_t lbaIndex = pGptHeader->partitionEntriesStartingLBA +
        ((partitionIndex * pGptHeader->sizeOfPartitionEntry) / GPT_LBA_SIZE);

    HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry =
        ReadPartitionEntryIntoBuffer_(pGpt, lbaIndex, partitionIndex);

    result = partitionIndex < pGptHeader->numPartitions;

    if (result) {
        *pFirstLBA = pGptPartitionEntry->firstLBA;
    }

    return result;
}

//
//
//
bool GPT_ReadPartitionEntryByIndex(HSS_GPT_t const * const pGpt,
    size_t const partitionIndex, HSS_GPT_PartitionEntry_t const ** ppGptPartitionEntryOut)
{
    bool result = false;

    assert(pGpt != NULL);
    assert(ppGptPartitionEntryOut != NULL);

    // Read Partition Entries
    HSS_GPT_Header_t const * const pGptHeader = &(pGpt->h.header);
    const size_t lbaIndex = pGptHeader->partitionEntriesStartingLBA +
        ((partitionIndex * pGptHeader->sizeOfPartitionEntry) / GPT_LBA_SIZE);

    *ppGptPartitionEntryOut =
        ReadPartitionEntryIntoBuffer_(pGpt, lbaIndex, partitionIndex);

    result = (*ppGptPartitionEntryOut != NULL);

    return result;
}

//
//
//
static bool FindPartitionById_(HSS_GPT_t const * const pGpt,
    HSS_GPT_GUID_t const * const pGUID,
    size_t * const pPartitionIndex, CheckIfGUIDMatchFnPtr_t pCheckIfMatchFunc,
    HSS_GPT_PartitionEntry_t const ** ppGptPartitionEntryOut)
{
    bool result = false;

    assert(pGpt != NULL);
    assert(pGUID != NULL);
    assert(pPartitionIndex != NULL);
    assert(pCheckIfMatchFunc != NULL);

    HSS_GPT_Header_t const * const pGptHeader = &(pGpt->h.header);

    //
    // Read Partition Entries
    for (size_t partitionIndex = 0u; partitionIndex < pGptHeader->numPartitions;
        partitionIndex++) {
        HSS_GPT_PartitionEntry_t const * pPartitionEntry;
	assert(GPT_ReadPartitionEntryByIndex(pGpt, partitionIndex, &pPartitionEntry));

        // if we've passed the starting LBA of this parameter into the search, we already know
        // about it so look for another...
        if (partitionIndex < *pPartitionIndex) {
#ifdef GPT_DEBUG
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Skipping partition %lu" CRLF, partitionIndex);
#endif
            continue;
	}

        result = pCheckIfMatchFunc(pPartitionEntry, pGUID);

        if (result) {
#ifdef GPT_DEBUG
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Located partition for GUID %08x-%04x-%04x-%016lx" CRLF,
                pGUID->data1, pGUID->data2, pGUID->data3, __builtin_bswap64(pGUID->data4));
#endif

            *pPartitionIndex = partitionIndex;
            if (ppGptPartitionEntryOut) {
                *ppGptPartitionEntryOut = pPartitionEntry;
            }

            break;
        }
    }

    if (!result && (!*pPartitionIndex)) { // didn't find a partition and we were searching for the second or subsequent
        mHSS_DEBUG_PRINTF(LOG_ERROR, "Unable to find partition for GUID %08x-%04x-%04x-%016lx" CRLF,
            pGUID->data1, pGUID->data2, pGUID->data3, __builtin_bswap64(pGUID->data4));
    }

    return result;
}

//
//
//
bool GPT_FindPartitionByTypeId(HSS_GPT_t const * const pGpt,
    HSS_GPT_GUID_t const * const pGUID, size_t * const pPartitionIndex,
    HSS_GPT_PartitionEntry_t const ** ppGptPartitionEntryOut)
{
    return FindPartitionById_(pGpt, pGUID, pPartitionIndex, CheckIfTypeIdMatch_, ppGptPartitionEntryOut);
}

//
//
//
bool GPT_FindPartitionByUniqueId(HSS_GPT_t const * const pGpt,
    HSS_GPT_GUID_t const * const pGUID, size_t * const pPartitionIndex,
    HSS_GPT_PartitionEntry_t const ** ppGptPartitionEntryOut)
{
    return FindPartitionById_(pGpt, pGUID, pPartitionIndex, CheckIfUniqueIdMatch_, ppGptPartitionEntryOut);
}

//
//
//
bool GPT_ValidatePartitionEntries(HSS_GPT_t *pGpt)
{
    bool result = true;
    uint32_t rollingCrc = 0u;

    assert(pGpt != NULL);

    HSS_GPT_Header_t const * const pGptHeader = &(pGpt->h.header);

    //
    // Read Partition Entries
    for (size_t partitionIndex = 0; partitionIndex < pGptHeader->numPartitions;
        partitionIndex++) {
        const size_t lbaIndex = pGptHeader->partitionEntriesStartingLBA +
            ((partitionIndex * pGptHeader->sizeOfPartitionEntry) / GPT_LBA_SIZE);

        HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry =
            ReadPartitionEntryIntoBuffer_(pGpt, lbaIndex, partitionIndex);
        if (!pGptPartitionEntry) {
            result = false;
            break;
        }

#ifdef GPT_DEBUG
        if ((CheckIfGUIDMatch_(&nullGUID, &(pGptPartitionEntry->uniquePartitionGUID)))
            || (CheckIfGUIDMatch_(&nullGUID, &(pGptPartitionEntry->uniquePartitionGUID)))) {
            // skip debug output if null
        } else {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Found partition:" CRLF);
            HSS_GPT_GUID_t const * pGUID = &(pGptPartitionEntry->uniquePartitionGUID);
            mHSS_DEBUG_PRINTF(LOG_NORMAL, " - Unique GUID: %08x-%04x-%04x-%016lx" CRLF,
                pGUID->data1, pGUID->data2, pGUID->data3, __builtin_bswap64(pGUID->data4));

            pGUID = &(pGptPartitionEntry->partitionTypeGUID);
            mHSS_DEBUG_PRINTF(LOG_NORMAL, " - Type GUID:   %08x-%04x-%04x-%016lx" CRLF,
                pGUID->data1, pGUID->data2, pGUID->data3, __builtin_bswap64(pGUID->data4));
        }
#endif
        rollingCrc = CRC32_calculate_ex(rollingCrc, (uint8_t const *)pGptPartitionEntry,
            pGptHeader->sizeOfPartitionEntry);
    }

    if (result) {
        result = (rollingCrc == pGptHeader->partitionEntriesArrayCrc32);

        if (!result) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "CRC32 of partition entries is %08x, vs expected %08x" CRLF,
                rollingCrc, pGptHeader->partitionEntriesArrayCrc32);
        } else {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Validated GPT Partition Entries ..." CRLF);
            pGpt->partitionEntriesValid = true;
        }
    }

    return result;
}

//
//
//
bool GPT_FindBootSectorIndex(HSS_GPT_t *pGpt, size_t *srcIndex,
    HSS_GPT_PartitionEntry_t const ** ppGptPartitionEntryOut)
{
    bool result = false;

    assert(srcIndex != NULL);
    assert(readBlockFnPtr != NULL);
    assert(pGpt != NULL);

    result = pGpt->headerValid;
    if (!result) {
        result = GPT_ValidateHeader(pGpt);
    }

    result = pGpt->partitionEntriesValid;
    if (!result) {
        result = GPT_ValidatePartitionEntries(pGpt);
    }

    if (!result) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "GPT_ValidatePartitionEntries() failed" CRLF);
    }

    if (result) {
        const HSS_GPT_GUID_t diskGUID = {
            .data1 = 0x21686148u,
            .data2 = 0x6449u,
            .data3 = 0x6E6Fu,
            .data4 = 0x4946456465654e74u
        };

        size_t partitionIndex = *srcIndex;
        result = GPT_FindPartitionByTypeId(pGpt, &diskGUID, &partitionIndex, ppGptPartitionEntryOut);

        if (result) {
            *srcIndex = partitionIndex;
        }
    }

    return result;
}

//
//
//
void GPT_Init(HSS_GPT_t *pGpt)
{
    assert(pGpt);

    pGpt->headerValid = false;
    pGpt->partitionEntriesValid = false;
    pGpt->bootPartitionIndexValid = false;
    pGpt->bootPartitionIndex = 0;
}
