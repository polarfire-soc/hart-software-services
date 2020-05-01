/*
 * Software tool to compress boot image using FastLZ
 * Ivan Griffin (ivan.griffin@microchip.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "fastlz.h"
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
    printf("%s: malloc okay for input buffer of %lu bytes\n", argv[0], inputSize);

    char *pOutput = malloc(inputSize * 2 * sizeof(char));
    if (!pOutput) {
        perror("malloc()");
        exit(-5);
    }
    printf("%s: malloc okay for output buffer of %lu bytes\n", argv[0], inputSize);

    size_t bytesProcessed = fread(pInput, 1, inputSize, pFileIn);
    if (bytesProcessed != (size_t)inputSize) {
        perror("fread()");
        exit(-6);
    }
    printf("%s: fread okay for input buffer of %lu bytes\n", argv[0], inputSize);

    fclose(pFileIn);

    printf("%s: about to compress...\n", argv[0]);
    off_t outputSize = fastlz_compress((const void *)pInput, (int)inputSize, (void *)pOutput);
    printf("%s: output size is %lu bytes\n", argv[0], outputSize);

    FILE *pFileOut = fopen(argv[2], "w+");
    if (!pFileOut) {
        perror("fopen()");
        exit(-7);
    }

    struct HSS_CompressedImage imgHdr = {
        .magic= HSS_COMPRESSED_MAGIC,
        .headerLength = sizeof(struct HSS_CompressedImage),
        .headerCrc = 0u, 
        .compressedCrc = 0u,
        .compressedImageLen = outputSize,
        .originalImageLen = inputSize,
    };

    imgHdr.headerCrc = CRC32_calculate((const uint8_t *)&imgHdr, sizeof(struct HSS_CompressedImage));
   

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
    return 0;
}
