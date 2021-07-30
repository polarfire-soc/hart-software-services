/******************************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software - tools/bin2chunks
 *
 * Helper Utility to take a binary file and convert it into a chunks table for
 * MPFS HSS Embedded Software to boot...
 */

#define DEBUG 1

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

#ifndef CONFIG_CC_HAS_INTTYPES
#    define CONFIG_CC_HAS_INTTYPES
#endif

#include "hss_types.h"

#ifdef __riscv
#error 1
#endif

#if defined(__LP64__) || defined(_LP64)
#  ifndef PRIx64
#    define PRIx64 "lx"
#  endif
#  ifndef PRIu64
#    define PRIu64 "lu"
#  endif
#else
#  ifndef PRIx64
#    define PRIx64 "I64x"
#  endif
#  ifndef PRIu64
#    define PRIu64 "I64u"
#  endif
#endif


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

/*****************************************************************************************
 * CRC32 calculation
 */

static const uint32_t precalcTable_[256] = {
    0x00000000u, 0x77073096u, 0xEE0E612Cu, 0x990951BAu, 0x076DC419u, 0x706AF48Fu,
    0xE963A535u, 0x9E6495A3u, 0x0EDB8832u, 0x79DCB8A4u, 0xE0D5E91Eu, 0x97D2D988u,
    0x09B64C2Bu, 0x7EB17CBDu, 0xE7B82D07u, 0x90BF1D91u, 0x1DB71064u, 0x6AB020F2u,
    0xF3B97148u, 0x84BE41DEu, 0x1ADAD47Du, 0x6DDDE4EBu, 0xF4D4B551u, 0x83D385C7u,
    0x136C9856u, 0x646BA8C0u, 0xFD62F97Au, 0x8A65C9ECu, 0x14015C4Fu, 0x63066CD9u,
    0xFA0F3D63u, 0x8D080DF5u, 0x3B6E20C8u, 0x4C69105Eu, 0xD56041E4u, 0xA2677172u,
    0x3C03E4D1u, 0x4B04D447u, 0xD20D85FDu, 0xA50AB56Bu, 0x35B5A8FAu, 0x42B2986Cu,
    0xDBBBC9D6u, 0xACBCF940u, 0x32D86CE3u, 0x45DF5C75u, 0xDCD60DCFu, 0xABD13D59u,
    0x26D930ACu, 0x51DE003Au, 0xC8D75180u, 0xBFD06116u, 0x21B4F4B5u, 0x56B3C423u,
    0xCFBA9599u, 0xB8BDA50Fu, 0x2802B89Eu, 0x5F058808u, 0xC60CD9B2u, 0xB10BE924u,
    0x2F6F7C87u, 0x58684C11u, 0xC1611DABu, 0xB6662D3Du, 0x76DC4190u, 0x01DB7106u,
    0x98D220BCu, 0xEFD5102Au, 0x71B18589u, 0x06B6B51Fu, 0x9FBFE4A5u, 0xE8B8D433u,
    0x7807C9A2u, 0x0F00F934u, 0x9609A88Eu, 0xE10E9818u, 0x7F6A0DBBu, 0x086D3D2Du,
    0x91646C97u, 0xE6635C01u, 0x6B6B51F4u, 0x1C6C6162u, 0x856530D8u, 0xF262004Eu,
    0x6C0695EDu, 0x1B01A57Bu, 0x8208F4C1u, 0xF50FC457u, 0x65B0D9C6u, 0x12B7E950u,
    0x8BBEB8EAu, 0xFCB9887Cu, 0x62DD1DDFu, 0x15DA2D49u, 0x8CD37CF3u, 0xFBD44C65u,
    0x4DB26158u, 0x3AB551CEu, 0xA3BC0074u, 0xD4BB30E2u, 0x4ADFA541u, 0x3DD895D7u,
    0xA4D1C46Du, 0xD3D6F4FBu, 0x4369E96Au, 0x346ED9FCu, 0xAD678846u, 0xDA60B8D0u,
    0x44042D73u, 0x33031DE5u, 0xAA0A4C5Fu, 0xDD0D7CC9u, 0x5005713Cu, 0x270241AAu,
    0xBE0B1010u, 0xC90C2086u, 0x5768B525u, 0x206F85B3u, 0xB966D409u, 0xCE61E49Fu,
    0x5EDEF90Eu, 0x29D9C998u, 0xB0D09822u, 0xC7D7A8B4u, 0x59B33D17u, 0x2EB40D81u,
    0xB7BD5C3Bu, 0xC0BA6CADu, 0xEDB88320u, 0x9ABFB3B6u, 0x03B6E20Cu, 0x74B1D29Au,
    0xEAD54739u, 0x9DD277AFu, 0x04DB2615u, 0x73DC1683u, 0xE3630B12u, 0x94643B84u,
    0x0D6D6A3Eu, 0x7A6A5AA8u, 0xE40ECF0Bu, 0x9309FF9Du, 0x0A00AE27u, 0x7D079EB1u,
    0xF00F9344u, 0x8708A3D2u, 0x1E01F268u, 0x6906C2FEu, 0xF762575Du, 0x806567CBu,
    0x196C3671u, 0x6E6B06E7u, 0xFED41B76u, 0x89D32BE0u, 0x10DA7A5Au, 0x67DD4ACCu,
    0xF9B9DF6Fu, 0x8EBEEFF9u, 0x17B7BE43u, 0x60B08ED5u, 0xD6D6A3E8u, 0xA1D1937Eu,
    0x38D8C2C4u, 0x4FDFF252u, 0xD1BB67F1u, 0xA6BC5767u, 0x3FB506DDu, 0x48B2364Bu,
    0xD80D2BDAu, 0xAF0A1B4Cu, 0x36034AF6u, 0x41047A60u, 0xDF60EFC3u, 0xA867DF55u,
    0x316E8EEFu, 0x4669BE79u, 0xCB61B38Cu, 0xBC66831Au, 0x256FD2A0u, 0x5268E236u,
    0xCC0C7795u, 0xBB0B4703u, 0x220216B9u, 0x5505262Fu, 0xC5BA3BBEu, 0xB2BD0B28u,
    0x2BB45A92u, 0x5CB36A04u, 0xC2D7FFA7u, 0xB5D0CF31u, 0x2CD99E8Bu, 0x5BDEAE1Du,
    0x9B64C2B0u, 0xEC63F226u, 0x756AA39Cu, 0x026D930Au, 0x9C0906A9u, 0xEB0E363Fu,
    0x72076785u, 0x05005713u, 0x95BF4A82u, 0xE2B87A14u, 0x7BB12BAEu, 0x0CB61B38u,
    0x92D28E9Bu, 0xE5D5BE0Du, 0x7CDCEFB7u, 0x0BDBDF21u, 0x86D3D2D4u, 0xF1D4E242u,
    0x68DDB3F8u, 0x1FDA836Eu, 0x81BE16CDu, 0xF6B9265Bu, 0x6FB077E1u, 0x18B74777u,
    0x88085AE6u, 0xFF0F6A70u, 0x66063BCAu, 0x11010B5Cu, 0x8F659EFFu, 0xF862AE69u,
    0x616BFFD3u, 0x166CCF45u, 0xA00AE278u, 0xD70DD2EEu, 0x4E048354u, 0x3903B3C2u,
    0xA7672661u, 0xD06016F7u, 0x4969474Du, 0x3E6E77DBu, 0xAED16A4Au, 0xD9D65ADCu,
    0x40DF0B66u, 0x37D83BF0u, 0xA9BCAE53u, 0xDEBB9EC5u, 0x47B2CF7Fu, 0x30B5FFE9u,
    0xBDBDF21Cu, 0xCABAC28Au, 0x53B39330u, 0x24B4A3A6u, 0xBAD03605u, 0xCDD70693u,
    0x54DE5729u, 0x23D967BFu, 0xB3667A2Eu, 0xC4614AB8u, 0x5D681B02u, 0x2A6F2B94u,
    0xB40BBE37u, 0xC30C8EA1u, 0x5A05DF1Bu, 0x2D02EF8Du
};

#define CRC32_MASK (0xFFFFFFFFu)
#define	CRC32_SEED (CRC32_MASK)

static inline uint32_t CRC32_updateByte(uint32_t crc32, const uint8_t byte)
{
    uint32_t index;

    index = (crc32 ^ byte) & 0xFF;
    crc32 = (crc32 >> 8) ^ precalcTable_[index];

    return crc32 & CRC32_MASK;
}

static uint32_t CRC32_calculate_ex(uint32_t seed, uint8_t const *pInput, size_t numBytes)
{
    uint32_t crc32 = ~seed;

    if (pInput != NULL) {
        while (numBytes--) {
            crc32 = CRC32_updateByte(crc32, *pInput);
            ++pInput;
        }
    }

    return ~crc32;
}

static uint32_t CRC32_calculate(uint8_t const *pInput, size_t numBytes)
{
    uint32_t crc32 = 0u;

    return CRC32_calculate_ex(crc32, pInput, numBytes);
}



/*****************************************************************************************/
/*! \brief Pad helper to align structures nicely
 */
static void fileOut_WritePad_(FILE *pFileOut, size_t pad)
{
    size_t i;

    for (i = 0; i < pad; i++) {
        fputc(0, pFileOut);
    }

#if DEBUG
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

#if DEBUG
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

#if DEBUG
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

#if DEBUG
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
