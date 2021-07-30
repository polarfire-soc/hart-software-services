/******************************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*\!
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>



#include "hss_types.h"

/******************************************************************************************************/
void print_usage(const char *programName)
{
    printf("Usage: %s <input.bin>\n", programName);
}

size_t getFileSize(const char *filename)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    char *filename_input = argv[1];

    printf("%s: opening >>%s<<\n", argv[0], filename_input);
    int fdIn = open(filename_input, O_RDONLY);
    assert(fdIn >= 0);

    size_t fileSize = getFileSize(filename_input);

    struct HSS_BootImage *pBootImage;

    pBootImage = mmap(NULL, fileSize, PROT_READ,
        MAP_PRIVATE | MAP_POPULATE, fdIn, 0);
    assert(pBootImage);

    if (pBootImage->magic != mHSS_BOOT_MAGIC) {
        printf("Warning: does not look like a valid boot image"
            " (expected magic %x, got %x)\n", mHSS_BOOT_MAGIC, pBootImage->magic);
    }

    printf("magic:              %x\n",     pBootImage->magic);
    printf("headerLength:       %lx\n",    pBootImage->headerLength);
    printf("chunkTableOffset:   %lx\n",    pBootImage->chunkTableOffset);
    printf("ziChunkTableOffset: %lx\n",    pBootImage->ziChunkTableOffset);

    for (unsigned int i = 0u; i < NR_CPUs; i++) {
        printf("name[%u]:            >>%s<<\n", i, pBootImage->hart[i].name);
        printf("entryPoint[%u]:      %lx\n",    i, pBootImage->hart[i].entryPoint);
        printf("privMode[%u]:        %u\n",     i, pBootImage->hart[i].privMode);
        printf("firstChunk[%u]       %lu\n",    i, (unsigned long)pBootImage->hart[i].firstChunk);
        printf("lastChunk[%u]        %lu\n",    i, (unsigned long)pBootImage->hart[i].lastChunk);
        printf("numChunks[%u]        %lu\n",    i, (unsigned long)pBootImage->hart[i].numChunks);
    }

    printf("set_name:           >>%s<<\n", pBootImage->set_name);
    printf("bootImageLength:    %lu\n",    (unsigned long)pBootImage->bootImageLength);
    printf("headerCrc:          0x%08x\n", (unsigned int)pBootImage->headerCrc);

    off_t chunkOffset = 0u;
    size_t totalChunkCount = 0u;
    size_t localChunkCount = 0u;
    enum HSSHartId lastOwner = HSS_HART_E51;

    while (1) {
        struct HSS_BootChunkDesc bootChunk;
        memcpy(&bootChunk, ((char *)pBootImage) + pBootImage->chunkTableOffset + chunkOffset, sizeof(struct HSS_BootChunkDesc));

        if (totalChunkCount == 0) {
            lastOwner = bootChunk.owner;
            localChunkCount++;
        } else if (bootChunk.owner == lastOwner) {
            localChunkCount++;
        } else {
            printf(" - %lu chunks found for owner %u\n", (unsigned long)localChunkCount, lastOwner);
            localChunkCount = 1u;
            lastOwner = bootChunk.owner;
        }

        chunkOffset += sizeof(struct HSS_BootChunkDesc);
        totalChunkCount++;
        if (bootChunk.size==0u) { break;}
    }
    printf("Boot Chunks: total of %lu chunk%s found\n", (unsigned long)totalChunkCount, (totalChunkCount != 1u) ? "s":"");

    chunkOffset = 0u;
    totalChunkCount = 0u;
    localChunkCount = 0u;
    lastOwner = HSS_HART_E51;

    while (1) {
        struct HSS_BootZIChunkDesc ziChunk;
        memcpy(&ziChunk, ((char *)pBootImage) + pBootImage->ziChunkTableOffset + chunkOffset, sizeof(struct HSS_BootZIChunkDesc));

        if (totalChunkCount == 0) {
            lastOwner = ziChunk.owner;
            localChunkCount++;
        } else if (ziChunk.owner == lastOwner) {
            localChunkCount++;
        } else {
            printf(" - %lu chunks found for owner %u\n", (unsigned long)localChunkCount, lastOwner);
            localChunkCount = 1u;
            lastOwner = ziChunk.owner;
        }

        chunkOffset += sizeof(struct HSS_BootZIChunkDesc);
        totalChunkCount++;
        if (ziChunk.size==0u) { break;}
    }
    printf("ZI Chunks: total of %lu chunk%s found\n", (unsigned long)totalChunkCount, (totalChunkCount != 1u) ? "s":"");

    // skipping binary fie array

    munmap(pBootImage, fileSize);
    close(fdIn);

    return 0;
}
