#ifndef GPT_H
#define GPT_H

/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Hart Software Services - GUID Partition Table (GPT) Types and Routines.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#define GPT_LBA_SIZE 512u

typedef struct HSS_GPT_GUID_s {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint64_t data4;
} HSS_GPT_GUID_t;

typedef struct HSS_GPT_Header_s {
     union {
         uint64_t signature;
         char     c[8];
         uint8_t  u[8];
     } s;
     uint32_t revision;
     uint32_t headerSize;
     uint32_t headerCrc32;
     uint32_t reserved;
     uint64_t currentLBA;
     uint64_t backupLBA;
     uint64_t firstUsableLBA;
     uint64_t lastUsableLBA;
     HSS_GPT_GUID_t diskGUID;
     uint64_t partitionEntriesStartingLBA;
     uint32_t numPartitions;
     uint32_t sizeOfPartitionEntry;
     uint32_t partitionEntriesArrayCrc32;
} HSS_GPT_Header_t;

typedef struct HSS_GPT_s {
    union {
        HSS_GPT_Header_t header;
        uint8_t buffer[GPT_LBA_SIZE] __attribute__((aligned(8)));
    } h;
    // For now, GPT needs a 2-sector buffer (~1KB) - for partition entity
    uint8_t lbaBuffer[2 * GPT_LBA_SIZE] __attribute__((aligned(8)));
    size_t bootPartitionIndex;

    int headerValid:1;
    int partitionEntriesValid:1;
    int bootPartitionIndexValid:1;
} HSS_GPT_t;


#define GPT_EXPECTED_SIGNATURE "EFI PART"
#define GPT_EXPECTED_REVISION 0x00010000u

void GPT_RegisterReadFunction(bool (*fnPtr)(void *pDest, size_t srcOffset, size_t byteCount));

void GPT_Init(HSS_GPT_t *pGpt);
bool GPT_ReadHeader(HSS_GPT_t *pGpt);
void GPT_DumpHeaderInfo(HSS_GPT_t *pGpt);
bool GPT_ValidateHeader(HSS_GPT_t *pGpt);

typedef struct HSS_GPT_PartitionEntry_s {
    HSS_GPT_GUID_t partitionTypeGUID;
    HSS_GPT_GUID_t uniquePartitionGUID;
    uint64_t firstLBA;
    uint64_t lastLBA;
    uint64_t attributes;
    union {
        uint16_t utf16le[36];
        char     c[72];
        uint8_t  u[72];
    };
} HSS_GPT_PartitionEntry_t;

bool GPT_ValidatePartitionEntries(HSS_GPT_t *pGpt);

void GPT_DumpPartitionInfo(HSS_GPT_t const * const pGpt,
    HSS_GPT_PartitionEntry_t const * const pGptPartitionEntry);

bool GPT_PartitionIdToLBAOffset(HSS_GPT_t const * const pGpt,
    size_t partitionIndex, size_t * const pFirstLBA);

bool GPT_FindPartitionByTypeId(HSS_GPT_t const * const pGpt,
    HSS_GPT_GUID_t const * const pGUID, size_t * const pPartitionIndex,
    HSS_GPT_PartitionEntry_t const ** ppGptPartitionEntryOut);

bool GPT_FindPartitionByUniqueId(HSS_GPT_t const * const pGpt,
    HSS_GPT_GUID_t const * const pGUID, size_t * const pPartitionIndex,
    HSS_GPT_PartitionEntry_t const ** ppGptPartitionEntryOut);

void GPT_RegisterReadBlockFunction(bool (*fnPtr)(void *pDest, size_t srcOffset, size_t byteCount));

bool GPT_FindBootSectorIndex(HSS_GPT_t *pGpt, size_t *srcIndex,
    HSS_GPT_PartitionEntry_t const ** ppGptPartitionEntryOut);

void GPT_SetBootPartitionIndex(HSS_GPT_t *pGpt, size_t index);
bool GPT_GetBootPartitionIndex(HSS_GPT_t *pGpt, size_t *pIndex);
bool GPT_ReadPartitionEntryByIndex(HSS_GPT_t const * const pGpt,
    size_t const partitionIndex, HSS_GPT_PartitionEntry_t const ** ppGptPartitionEntryOut);

#ifdef __cplusplus
}
#endif

#endif
