/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
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


//
// local modules function pointer types
typedef bool (*ReadBlockFnPtr_t)(void *pDest, size_t srcOffset, size_t byteCount);
typedef bool (*CheckIfGUIDMatchFnPtr_t)(HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry,
    HSS_GPT_GUID_t const * const pGUID2);

static ReadBlockFnPtr_t readBlockFnPtr;

static const HSS_GPT_GUID_t nullGUID = {
    .data1 = 0u,
    .data2 = 0u,
    .data3 = 0u,
    .data4 = 0u
};


//
// local module function prototypes
static bool CheckIfGUIDMatch_(HSS_GPT_GUID_t const * const pGUID1,
    HSS_GPT_GUID_t const * const pGUID2);
static bool CheckIfTypeIdMatch_(HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry,
    HSS_GPT_GUID_t const * const pGUID);
static bool CheckIfUniqueIdMatch_(HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry,
    HSS_GPT_GUID_t const * const pGUID);
static bool FindPartitionById_(HSS_GPT_Header_t const * const pGptHeader,
    HSS_GPT_GUID_t const * const pGUID, uint8_t *pLBABuffer,
    size_t * const pFirstLBA, size_t * const pLastLBA,
    CheckIfGUIDMatchFnPtr_t checkIfMatchFunc);

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

#define GPT_DEBUG

//
//
//
void GPT_DumpHeaderInfo(HSS_GPT_Header_t *pGptHeader)
{
#ifdef GPT_DEBUG
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
    (void)pGptHeader;
#endif
}

//
//
//
void GPT_DumpPartitionInfo(HSS_GPT_Header_t const * const pGptHeader,
    HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry)
{
#ifdef GPT_DEBUG
    assert(pGptHeader != NULL);
    assert(pGptPartitionEntry != NULL);

    mHSS_DEBUG_PRINTF(LOG_STATUS, "Partition Type GUID:   %08x-%04x-%04x-%016lx" CRLF,
        pGptPartitionEntry->partitionTypeGUID.data1, pGptPartitionEntry->partitionTypeGUID.data2,
        pGptPartitionEntry->partitionTypeGUID.data3, pGptPartitionEntry->partitionTypeGUID.data4);
    mHSS_DEBUG_PRINTF(LOG_STATUS, "Unique Partition GUID: %08x-%04x-%04x-%016lx" CRLF,
        pGptPartitionEntry->uniquePartitionGUID.data1, pGptPartitionEntry->uniquePartitionGUID.data2,
        pGptPartitionEntry->uniquePartitionGUID.data3, pGptPartitionEntry->uniquePartitionGUID.data4);

   mHSS_DEBUG_PRINTF(LOG_STATUS, "First LBA:              %016lx" CRLF, pGptPartitionEntry->firstLBA);
   mHSS_DEBUG_PRINTF(LOG_STATUS, "Last LBA:               %016lx" CRLF, pGptPartitionEntry->lastLBA);
   mHSS_DEBUG_PRINTF(LOG_STATUS, "Attributes:             %016lx" CRLF, pGptPartitionEntry->attributes);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

//
//
//
bool GPT_ValidateHeader(HSS_GPT_Header_t *pGptHeader)
{
    bool result = true;

    assert(pGptHeader != NULL);

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
    }
    return result;
}

//
//
//
bool GPT_ReadHeader(HSS_GPT_Header_t *pGptHeader)
{
    assert(pGptHeader != NULL);
    assert(readBlockFnPtr != NULL);

    bool result = false;

    result = readBlockFnPtr(pGptHeader, 1u * GPT_LBA_SIZE, GPT_LBA_SIZE);
    if (!result) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "Unable to read block for LBA 1" CRLF);
    }

    return result;
}

//
//
//
static HSS_GPT_PartitionEntry_t const * ReadPartitionEntryIntoBuffer_(
    HSS_GPT_Header_t const * const pGptHeader, uint8_t *pLBABuffer, size_t lbaIndex,
    size_t partitionIndex)
{
    assert(readBlockFnPtr != NULL);
    assert(pGptHeader != NULL);
    assert(pLBABuffer != NULL);

    HSS_GPT_PartitionEntry_t *pResult = NULL;
    bool retVal = false;

    retVal = readBlockFnPtr(pLBABuffer, lbaIndex * GPT_LBA_SIZE, GPT_LBA_SIZE);
    if (!retVal) {
        mHSS_DEBUG_PRINTF(LOG_ERROR,
            "Unable to read block for LBA %lu (partition entry %lu)" CRLF,
            lbaIndex, partitionIndex);
    } else {
        const size_t offset =
            (pGptHeader->sizeOfPartitionEntry * partitionIndex) -
                ((lbaIndex - pGptHeader->partitionEntriesStartingLBA) * GPT_LBA_SIZE);

        if (offset >= GPT_LBA_SIZE) {
            retVal = readBlockFnPtr(pLBABuffer + GPT_LBA_SIZE,
                (lbaIndex + 1u) * GPT_LBA_SIZE, GPT_LBA_SIZE);
            if (!retVal) {
                mHSS_DEBUG_PRINTF(LOG_ERROR,
                    "Unable to read block for LBA %lu (partition entry %lu)" CRLF,
                    lbaIndex, partitionIndex);
            }
        }

        if (retVal) {
            pResult = (HSS_GPT_PartitionEntry_t *)(pLBABuffer + offset);
        }
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
static bool FindPartitionById_(HSS_GPT_Header_t const * const pGptHeader,
    HSS_GPT_GUID_t const * const pGUID, uint8_t *pLBABuffer,
    size_t * const pFirstLBA, size_t * const pLastLBA,
    CheckIfGUIDMatchFnPtr_t checkIfMatchFunc)
{
    assert(pGptHeader != NULL);
    assert(pGUID != NULL);
    assert(pLBABuffer != NULL);
    assert(pFirstLBA != NULL);
    assert(pLastLBA != NULL);

    bool result = false;

    // Read Partition Entries
    for (size_t partitionIndex = 0u; partitionIndex < pGptHeader->numPartitions;
        partitionIndex++) {
        const size_t lbaIndex = pGptHeader->partitionEntriesStartingLBA +
            ((partitionIndex * pGptHeader->sizeOfPartitionEntry) / GPT_LBA_SIZE);

       HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry =
           ReadPartitionEntryIntoBuffer_(pGptHeader, pLBABuffer, lbaIndex, partitionIndex);

       result = checkIfMatchFunc(pGptPartitionEntry, pGUID);

       if (result) {
#ifdef GPT_DEBUG
           mHSS_DEBUG_PRINTF(LOG_NORMAL, "Located partition for GUID %08x-%04x-%04x-%016lx" CRLF,
               pGUID->data1, pGUID->data2, pGUID->data3, __builtin_bswap64(pGUID->data4));
#endif

           *pFirstLBA = pGptPartitionEntry->firstLBA;
           *pLastLBA = pGptPartitionEntry->lastLBA;
           break;
       }
    }

    if (!result) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "Unable to find partition for GUID %08x-%04x-%04x-%016lx" CRLF,
            pGUID->data1, pGUID->data2, pGUID->data3, __builtin_bswap64(pGUID->data4));
    }

    return result;
}

//
//
//
bool GPT_FindPartitionByTypeId(HSS_GPT_Header_t const * const pGptHeader,
    HSS_GPT_GUID_t const * const pGUID, uint8_t *pLBABuffer,
    size_t * const pFirstLBA, size_t * const pLastLBA)
{
    return FindPartitionById_(pGptHeader, pGUID, pLBABuffer, pFirstLBA, pLastLBA, CheckIfTypeIdMatch_);
}

//
//
//
bool GPT_FindPartitionByUniqueId(HSS_GPT_Header_t const * const pGptHeader,
    HSS_GPT_GUID_t const * const pGUID, uint8_t *pLBABuffer,
    size_t * const pFirstLBA, size_t * const pLastLBA)
{
    return FindPartitionById_(pGptHeader, pGUID, pLBABuffer, pFirstLBA, pLastLBA,
        CheckIfUniqueIdMatch_);
}

//
//
//
bool GPT_ValidatePartitionEntries(HSS_GPT_Header_t *pGptHeader, uint8_t *pLBABuffer)
{
    bool result = true;
    uint32_t rollingCrc = 0u;

    assert(pGptHeader != NULL);
    assert(pLBABuffer != NULL);

    //
    // Read Partition Entries
    for (size_t partitionIndex = 0; partitionIndex < pGptHeader->numPartitions;
        partitionIndex++) {
        const size_t lbaIndex = pGptHeader->partitionEntriesStartingLBA +
            ((partitionIndex * pGptHeader->sizeOfPartitionEntry) / GPT_LBA_SIZE);

        HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry =
            ReadPartitionEntryIntoBuffer_(pGptHeader, pLBABuffer, lbaIndex, partitionIndex);
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
            mHSS_DEBUG_PRINTF(LOG_NORMAL, " - Unique Partition GUID: %08x-%04x-%04x-%016lx" CRLF,
                pGUID->data1, pGUID->data2, pGUID->data3, __builtin_bswap64(pGUID->data4));

            pGUID = &(pGptPartitionEntry->partitionTypeGUID);
            mHSS_DEBUG_PRINTF(LOG_NORMAL, " - Partition Type GUID:   %08x-%04x-%04x-%016lx" CRLF,
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
        }
    }

    return result;
}
