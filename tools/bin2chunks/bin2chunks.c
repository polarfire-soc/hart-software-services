/******************************************************************************************
 * Copyright 2019 Microchip Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software - tools/bin2chunks
 *
 * Helper Utility to take a binary file and convert it into a chunks table for
 * MPFS HSS Embedded Software to boot...
 */

#define DEBUG 0

/* the following define is just for debug usage */
#define DEBUG_MEM_OFFSET (0x81200000lu)

#define _FILE_OFFSET_BITS 64


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <libgen.h>


#define mCOMPILE_ASSERT(expr, msg) char msg[expr ? 1: -1]
mCOMPILE_ASSERT(sizeof(void *)==8, fatal_64bit_compiler_required);

#ifdef __MINGW32__
   // x86_64-W64_MING32 has 4-byte long, whereas Linux x64/RV64 compilers have 8-byte long
   // we'll fix up the type defs here to ensure that the structures (including padding)
   // are the same sizes...
   //

#  define __ssize_t_defined
   typedef long long          HSS_ssize_t;
   typedef unsigned long long HSS_size_t;
   typedef unsigned long long HSS_uintptr_t;
   typedef unsigned long long HSS_off_t;
#  define size_t    HSS_size_t
#  define ssize_t   HSS_ssize_t
#  define uintptr_t HSS_uintptr_t
#  define off_t     HSS_off_t
#endif

#define CONFIG_CC_HAS_INTTYPES

#include "hss_types.h"
#include "hss_crc32.h"

#include "../../misc/hss_crc32.c"

/*****************************************************************************************/

#ifndef NR_CPUs
#  define NR_CPUs (4u)
#endif

size_t gChunkSize = 1024u;
#define BYTES_PER_LINE (12u)

#define ENTRY_POINT 0x80000000lu

#define PAD_SIZE  8
#define mPAD(x, pad) (((x + (pad - 1)) / pad) * pad)

#define PRV_U 0
#define PRV_S 1
#define PRV_H 2
#define PRV_M 3

uintptr_t entryPoint[NR_CPUs] = { ENTRY_POINT, ENTRY_POINT, ENTRY_POINT, ENTRY_POINT };
size_t execAddr[NR_CPUs] =   { 0u, 0u, 0u, 0u };
uint8_t privMode[NR_CPUs] = { PRV_S, PRV_S, PRV_S, PRV_S };

size_t bootImageSize = 0u;
size_t bootImagePaddedSize = 0u;
size_t chunkTableSize = 0u;
size_t chunkTablePaddedSize = 0u;
size_t ziChunkTableSize = 0u;
size_t ziChunkTablePaddedSize = 0u;

size_t firstChunkArray[NR_CPUs] = { 0u, 0u, 0u, 0u };
size_t lastChunkArray[NR_CPUs] = { 0u, 0u, 0u, 0u };
size_t chunkCountArray[NR_CPUs] = { 0u, 0u, 0u, 0u };
size_t chunkCount = 0u;
size_t ziChunkCount = 0u;

// self-checks, to ensure tool is generating good boot images
size_t chunkTableOffset = 0u;
size_t ziChunkTableOffset = 0u;
uintptr_t * pChunkOffset;


/*****************************************************************************************/
/*! \brief Pad helper to align structures nicely
 */
static void fileOut_WritePad_(FILE *pFileOut, size_t pad)
{
    size_t i;

    for (i = 0; i < pad; i++) {
        fputc(0, pFileOut);
    }

#ifdef DEBUG
    if (DEBUG) {
        off_t posn = ftello(pFileOut);
        printf("%s(): current location after %lu padding is %lu\n", __func__, (unsigned long)pad, (unsigned long)posn);
    }
#endif
}

/*****************************************************************************************/
void calculateChunkCounts_(size_t *binSize, size_t chunkSize)
{
    for (unsigned int i = 0u; i < NR_CPUs; i++) {
        if (binSize[i] == 0u) {
            if (DEBUG) {
                printf("%s: skipping core %u as binSize[%u] is %lu\n", __func__, i, i, (unsigned long)binSize[i]);
            }
            continue;
        }

        size_t idx;
        for (idx = 0u; idx < binSize[i]; idx += chunkSize) {
            size_t thisChunkSize = (binSize[i] - idx);
            if (thisChunkSize > chunkSize) { thisChunkSize = chunkSize; }
            chunkCount++;
        }
    }

    chunkCount++; // to account for terminating sentinel
    ziChunkCount++; // to account for terminating sentinel

    bootImageSize          = sizeof(struct HSS_BootImage);
    bootImagePaddedSize    = mPAD(bootImageSize, PAD_SIZE);

    chunkTableSize         = (sizeof(struct HSS_BootChunkDesc) * chunkCount);
    chunkTablePaddedSize   = mPAD(chunkTableSize, PAD_SIZE);

    ziChunkTableSize       = (sizeof(struct HSS_BootZIChunkDesc) * ziChunkCount);
    ziChunkTablePaddedSize = mPAD(ziChunkTableSize, PAD_SIZE);

    pChunkOffset = calloc(sizeof(uintptr_t), chunkCount);

    printf("Boot Image Size:            %lu\n"
            "Boot Image Padded Size:     %lu\n\n"

            "Number of Chunks:           %lu\n"
            "Chunk Table Size:           %lu\n"
            "Chunk Table Padded Size:    %lu\n\n"

            "Number of ZI Chunks:        %lu\n"
            "ZI Chunk Table Size:        %lu\n"
            "ZI Chunk Table Padded Size: %lu\n\n",

            (unsigned long)bootImageSize, (unsigned long)bootImagePaddedSize,
            (unsigned long)chunkCount, (unsigned long)chunkTableSize, (unsigned long)chunkTablePaddedSize,
            (unsigned long)ziChunkCount, (unsigned long)ziChunkTableSize, (unsigned long)ziChunkTablePaddedSize);
}

/*****************************************************************************************/
/*! \brief Includes binary file as blob with symbols marking start, end and size
 */
void fileOut_WriteBinaryFileArray_(FILE *pFileOut, FILE **ppFileIn, size_t *binSize, size_t chunkSize)
{
    for (unsigned int i = 0u; i < NR_CPUs; i++) {
        FILE *pFileIn = ppFileIn[i];

        if ((pFileIn == NULL) || (binSize == 0)) {
            if (DEBUG) {
                printf("%s: skipping core %u as ppFileIn[%u] is NULL or binSize[%u] is %lu\n", __func__, i, i, i, (unsigned long)binSize[i]);
            }
            continue;
        }

        char byte = fgetc(pFileIn);
        while (!feof(pFileIn)) {
            fputc(byte, pFileOut);
            byte = fgetc(pFileIn);
        }

        size_t binPaddedSize = mPAD(binSize[i], chunkSize);
        fileOut_WritePad_(pFileOut, binPaddedSize - binSize[i]);
    }
}

/*****************************************************************************************/
/*! \brief Generates the code/ro data/rw data chunk table
 */
void fileOut_WriteBootChunkTable_(FILE *pFileOut, int *pOwnerArray, size_t *binSize, size_t chunkSize)
{
    off_t posn = ftello(pFileOut);
    assert(chunkTableOffset == (size_t)posn);

    size_t totalChunkCount = 0u;
    size_t totalIdx = 0u;
    size_t idx = 0u;
    for (unsigned int i = 0u; i < NR_CPUs; i++) {
        int owner = pOwnerArray[i];
        if ((owner == 0) || (binSize[i] == 0)) {
            if (DEBUG) {
                printf("%s: skipping core %u as binSize[%u] is %lu\n", __func__, i, i, (unsigned long)binSize[i]);
            }
            continue;
        }

        for (idx = 0u; idx < binSize[i]; idx += chunkSize) {
            struct HSS_BootChunkDesc bootChunk = {
                .owner = owner,
                .loadAddr = (bootImagePaddedSize + chunkTablePaddedSize + ziChunkTablePaddedSize + idx + totalIdx),
                .execAddr = (execAddr[i] + idx),
                //.size = thisCountSize,
                //.crc32 = crc
            };

            if (idx == 0) {
                firstChunkArray[owner-1] = totalChunkCount;
            }

            pChunkOffset[idx/chunkSize] = bootChunk.loadAddr;

            size_t thisChunkSize = (binSize[i] - idx);
            if (thisChunkSize > chunkSize) { thisChunkSize = chunkSize; }

            bootChunk.size = thisChunkSize;
            bootChunk.crc32 = 0u;
            //bootChunk.crc32 = CRC32_calculate(BLAH, thisChunkSize);

#ifdef DEBUG
            if (DEBUG) {
                printf("%s(): %d: Writing chunk %lu: %" PRIx64 "->%" PRIx64
                    " (payload of %lu bytes)\n",  __func__, bootChunk.owner,
                    (unsigned long)(idx/chunkSize),
                    bootChunk.loadAddr + DEBUG_MEM_OFFSET,
                    bootChunk.execAddr, (unsigned long)bootChunk.size);
            }
#endif
            fwrite((char *)&bootChunk, sizeof(struct HSS_BootChunkDesc), 1, pFileOut);
            chunkCountArray[owner-1]++;
            totalChunkCount++;
        }
        totalIdx += idx;
        lastChunkArray[owner-1] = totalChunkCount-1;
    }

    // sentinel
    struct HSS_BootChunkDesc bootChunk = { .owner = 0, .loadAddr = 0, .execAddr = 0, .size = 0, .crc32 = 0 };
    fwrite((char *)&bootChunk, sizeof(struct HSS_BootChunkDesc), 1, pFileOut);

#ifdef DEBUG
    if (DEBUG) {
        printf("%s(): %d: Writing chunk %lu: %" PRIx64 "->%" PRIx64 " (%lu bytes)\n",
            __func__, bootChunk.owner,
           (unsigned long)(idx/chunkSize),
           bootChunk.loadAddr + DEBUG_MEM_OFFSET, bootChunk.execAddr,
           (unsigned long)bootChunk.size);
    }
#endif

    pChunkOffset[idx/chunkSize] = 0;

    fileOut_WritePad_(pFileOut, chunkTablePaddedSize - chunkTableSize);
}

/*****************************************************************************************/
/*! \brief Generate the BSS/Zero Init chunk table
 */
void fileOut_WriteBootZIChunkTable_(FILE *pFileOut, int *pOwnerArray)
{
    // this function is a stub for now...
    struct HSS_BootZIChunkDesc ziChunk = { .owner = 0u, .execAddr = 0x0u, .size = 0u };

    off_t posn = ftello(pFileOut);
    assert(ziChunkTableOffset == (size_t)posn);

    fwrite((char *)&ziChunk, sizeof(struct HSS_BootZIChunkDesc), 1, pFileOut);
    fileOut_WritePad_(pFileOut, ziChunkTablePaddedSize - ziChunkTableSize);
}

/*****************************************************************************************/
/*! \brief Generate the boot image data structure
 */
uint32_t fileOut_WriteBootImageHeader_(FILE *pFileOut, int *pOwnerArray, char * pName, char ** pFileNameArray,
    size_t *pNumChunksArray, size_t numZIChunks, size_t bootImageLength, uint32_t headerCrc)
{

    size_t hLen = bootImagePaddedSize + chunkTablePaddedSize + ziChunkTablePaddedSize;

    assert(pFileOut != NULL);
    assert(pOwnerArray != NULL);
    assert(pName != NULL);
    assert(pFileNameArray != NULL);

    struct HSS_BootImage bootImage = {
        .magic = mHSS_BOOT_MAGIC,
        .headerLength = hLen,
        .headerCrc = headerCrc,
        .chunkTableOffset = bootImagePaddedSize,
        .ziChunkTableOffset = bootImagePaddedSize + chunkTablePaddedSize,
        .hart = {
            { .entryPoint = entryPoint[0],
              .privMode = privMode[0],
              .numChunks = pNumChunksArray[0],
              .firstChunk = firstChunkArray[0],
              .lastChunk = lastChunkArray[0],
              .name = "" },
            { .entryPoint = entryPoint[1],
              .privMode = privMode[1],
              .numChunks = pNumChunksArray[1],
              .firstChunk = firstChunkArray[1],
              .lastChunk = lastChunkArray[1],
              .name = "" },
            { .entryPoint = entryPoint[2],
              .privMode = privMode[2],
              .numChunks = pNumChunksArray[2],
              .firstChunk = firstChunkArray[2],
              .lastChunk = lastChunkArray[2],
              .name = "" },
            { .entryPoint = entryPoint[3],
              .privMode = privMode[3],
              .numChunks = pNumChunksArray[3],
              .firstChunk = firstChunkArray[3],
              .lastChunk = lastChunkArray[3],
              .name = "" },
        },
        .set_name = "PolarFireSOC-HSS::",
        .bootImageLength = bootImageLength,
    };

    strncat(bootImage.set_name, pName, BOOT_IMAGE_MAX_NAME_LEN-1);

    for (int i = 0; i < 4; i++) {
        if (pFileNameArray[i]) {
            strncpy(bootImage.hart[pOwnerArray[i]-1].name, pFileNameArray[i], BOOT_IMAGE_MAX_NAME_LEN-1);
        }
    }

    chunkTableOffset = bootImage.chunkTableOffset;
    ziChunkTableOffset = bootImage.ziChunkTableOffset;

#ifdef DEBUG
    if (DEBUG) {
        printf("%s(): bootImage.chunkTableOffset is  %" PRIu64 "\n", __func__, bootImage.chunkTableOffset);
        printf("%s(): bootImage.ziChunkTableOffset is %" PRIu64 "\n", __func__, bootImage.ziChunkTableOffset);
        printf("%s(): bootImage.headerCrc is 0x%08x\n", __func__, bootImage.headerCrc);
    }
#endif

    fwrite((char *)&bootImage, sizeof(struct HSS_BootImage), 1, pFileOut);
    fileOut_WritePad_(pFileOut, bootImagePaddedSize - bootImageSize);

    return CRC32_calculate((const unsigned char *)&bootImage, sizeof(struct HSS_BootImage));
}


/*****************************************************************************************/
static void print_usage_(char *programName)
{
    printf("Usage: %s <0x-entry-point-u54-1> <0x-entry-point-u54-2> <0x-entry-point-u54-3> <0x-entry-point-u54-4>\n"
        "\t<chunkSize> <output.bin>\n"
        "\t<owner-hart> <priv-mode> <input-bin> <exec-addr> [ <owner-hart> <priv-mode> <input-bin> <exec-addr> ]\n", programName);
}

static void validate_owner_(int owner) {
    assert((owner >= 0) && (owner <=4));
}

static void validate_privmode_(int privmode) {
    assert((privmode >= PRV_U) && (privmode <=PRV_S));
}

char imageNameBuf[1024] = "";

int main(int argc, char **argv)
{
    printf("sizeof(long) is %lu\n", (unsigned long)sizeof(long));
    printf("sizeof(size_t) is %lu\n", (unsigned long)sizeof(size_t));

    if (argc != 11) {
        if ((argc < 11) || ((argc - 11) % 4)) {
            print_usage_(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    int argIndex = 1;
    // first core is E51 - skipped here...
    for (unsigned int i = 0u; i < NR_CPUs; i++) {
        entryPoint[i] = strtoul(argv[argIndex++], NULL, 16);
        printf("entryPoint[%u] set to %" PRIx64 "\n", i, entryPoint[i]);
    }

    gChunkSize = strtoul(argv[argIndex++], NULL, 10);
    printf("chunkSize set to %lu\n", (unsigned long)gChunkSize);

    char * filename_output = argv[argIndex++];
    printf("output file set to >>%s<<\n", filename_output);

    FILE *pFileOut = fopen(filename_output, "wb");
    assert(pFileOut != NULL);

    FILE *ppFileIn[NR_CPUs];
    size_t binSize[NR_CPUs];
    int ownerArray[NR_CPUs];
    char *pFileNameArray[NR_CPUs];

    for (unsigned int i = 0u; i < NR_CPUs; i++) {
        ppFileIn[i] = NULL;
        binSize[i] = 0u;
        ownerArray[i] = 0;
        pFileNameArray[i] = NULL;
        firstChunkArray[i] = 0u;
        lastChunkArray[i] = 0u;
        chunkCountArray[i] = 0u;
    }

    //for each image...
    for (unsigned int i = 0u; i < NR_CPUs; i++) {
        if (argIndex >= argc) {
            break;
        }

        printf(" - processing image %u\n", i+1);

        ownerArray[i] = strtol(argv[argIndex++], NULL, 10);
        validate_owner_(ownerArray[i]);
        printf(" - hart owner is >>%d<<\n", ownerArray[i]);

        privMode[i] = strtol(argv[argIndex++], NULL, 10);
        validate_privmode_(ownerArray[i]);
        printf(" - privMode >>%d<<\n", privMode[i]);

        pFileNameArray[i] = argv[argIndex++];
        printf(" - input file is >>%s<<\n", pFileNameArray[i]);


        if (strlen(imageNameBuf)) {
            strncat(imageNameBuf, "+", BOOT_IMAGE_MAX_NAME_LEN-1);
        }
        strncat(imageNameBuf, pFileNameArray[i], BOOT_IMAGE_MAX_NAME_LEN-1);

        execAddr[i] = strtoul(argv[argIndex++], NULL, 16);
        printf("execAddr[%u] set to %" PRIx64 "\n", i, execAddr[i]);

        ppFileIn[i] = fopen(pFileNameArray[i], "rb");
        assert(ppFileIn[i] != NULL);

        fseeko(ppFileIn[i], 0, SEEK_END);
        binSize[i] = (size_t)ftello(ppFileIn[i]);
        fseeko(ppFileIn[i], 0, SEEK_SET);
        printf(" - binSize[%u] is %lu\n", i, (unsigned long)binSize[i]);
    }

    calculateChunkCounts_(binSize, gChunkSize);
    // writing test header
    (void)fileOut_WriteBootImageHeader_(pFileOut, ownerArray, imageNameBuf, &pFileNameArray[0], chunkCountArray, ziChunkCount, 0u, 0u);
    fileOut_WriteBootChunkTable_(pFileOut, ownerArray, binSize, gChunkSize);
    fileOut_WriteBootZIChunkTable_(pFileOut, ownerArray);
    fileOut_WriteBinaryFileArray_(pFileOut, ppFileIn, binSize, gChunkSize);

    off_t posn = ftello(pFileOut);
    printf("%s: %" PRIu64 " bytes written to >>%s<<\n", argv[0], posn, filename_output);

    // rewrite header to correct it
    uint32_t headerCrc = 0;
    fseeko(pFileOut, 0, SEEK_SET);
    headerCrc = fileOut_WriteBootImageHeader_(pFileOut, ownerArray, imageNameBuf, &pFileNameArray[0], chunkCountArray, ziChunkCount, posn, headerCrc);
    fseeko(pFileOut, 0, SEEK_SET);
    (void) fileOut_WriteBootImageHeader_(pFileOut, ownerArray, imageNameBuf, &pFileNameArray[0], chunkCountArray, ziChunkCount, posn, headerCrc);
    printf("%s: headerCrc is 0x%08X\n", argv[0], headerCrc);

    // correct image length...

    fclose(pFileOut);
    for (unsigned int i = 0u; i < NR_CPUs; i++) {
        if (ppFileIn[i]) {
            fclose(ppFileIn[i]);
        }
    }

    return EXIT_SUCCESS;
}
