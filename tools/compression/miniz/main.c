/*
 * Software tool to compress boot image using miniz
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "miniz.h"
#include "hss_types.h"
#include "hss_crc32.h"


int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input> <output>\n\n", argv[0]);
        exit(-1);
    }

    FILE *pFileIn = fopen(argv[1], "r");
    if (!pFileIn) {
        perror("fopen()");
        exit(-2);
    }

    fseeko(pFileIn, 0, SEEK_END);
    off_t inputSize = (size_t)ftello(pFileIn);

    if (inputSize < 0) {
        perror("ftello()");
        exit(-3);
    }

    fseeko(pFileIn, 0, SEEK_SET);

    printf("%s: input size is %lu bytes\n", argv[0], inputSize);

    char *pInput = malloc(inputSize * sizeof(char));
    if (!pInput) {
        perror("malloc()");
        exit(-4);
    }
    //printf("%s: malloc okay for input buffer of %lu bytes\n", argv[0], inputSize);

    off_t outputSize = compressBound(inputSize * sizeof(char));
    char *pOutput = malloc(outputSize);
    if (!pOutput) {
        perror("malloc()");
        exit(-5);
    }
    //printf("%s: malloc okay for output buffer of %lu bytes\n", argv[0], inputSize);

    size_t bytesProcessed = fread(pInput, 1, inputSize, pFileIn);
    if (bytesProcessed != (size_t)inputSize) {
        perror("fread()");
        exit(-6);
    }
    //printf("%s: fread okay for input buffer of %lu bytes\n", argv[0], inputSize);

    fclose(pFileIn);

    printf("%s: about to compress...\n", argv[0]);
    int cmp_status = compress(pOutput, &outputSize, (const unsigned char *)pInput, inputSize);
    if (cmp_status != MZ_OK) {
        fprintf(stderr, "compress() failed - %d\n", cmp_status);
        exit(-7);
    }

    printf("%s: output size is %lu bytes\n", argv[0], outputSize);

    FILE *pFileOut = fopen(argv[2], "w+");
    if (!pFileOut) {
        perror("fopen()");
        exit(-7);
    }

    struct HSS_CompressedImage imgHdr = {
        .magic= mHSS_COMPRESSED_MAGIC,
        .version= mHSS_COMPRESSED_VERSION_MINIZ,
        .headerLength = sizeof(struct HSS_CompressedImage),
        .headerCrc = 0u,
        .compressedCrc = 0u,
        .originalCrc = 0u,
        .compressedImageLen = outputSize,
        .originalImageLen = inputSize,
    };

    imgHdr.headerCrc = CRC32_calculate((const uint8_t *)&imgHdr, sizeof(struct HSS_CompressedImage));
    imgHdr.compressedCrc = CRC32_calculate((const uint8_t *)pOutput, outputSize);
    imgHdr.originalCrc = CRC32_calculate((const uint8_t *)pInput, inputSize);

    bytesProcessed = fwrite((const void*)&imgHdr, 1, sizeof(struct HSS_CompressedImage), pFileOut);
    if (bytesProcessed != (size_t)sizeof(struct HSS_CompressedImage)) {
        perror("fwrite()");
        exit(-8);
    }

    bytesProcessed = fwrite((const void *)pOutput, 1, outputSize, pFileOut);
    if (bytesProcessed != (size_t)outputSize) {
        perror("fwrite()");
        exit(-9);
    }

    fclose(pFileOut);

    //printf("magic: %lu\n", offsetof(struct HSS_CompressedImage, magic));
    //printf("version: %lu\n", offsetof(struct HSS_CompressedImage, version));
    //printf("headerLength: %lu\n", offsetof(struct HSS_CompressedImage, headerLength));
    //printf("headerCrc: %lu\n", offsetof(struct HSS_CompressedImage, headerCrc));
    //printf("compressedCrc: %lu\n", offsetof(struct HSS_CompressedImage, compressedCrc));
    //printf("originalCrc: %lu\n", offsetof(struct HSS_CompressedImage, originalCrc));
    //printf("compressedImageLen: %lu\n", offsetof(struct HSS_CompressedImage, compressedImageLen));
    //printf("originalImageLen: %lu\n", offsetof(struct HSS_CompressedImage, originalImageLen));
    //printf("hash: %lu\n", offsetof(struct HSS_CompressedImage, hash));
    //printf("ecdsaSig: %lu\n", offsetof(struct HSS_CompressedImage, ecdsaSig));

    return 0;
}
