#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../../../include/hss_types.h"
#include "miniz.h"


int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <input>\n\n", argv[0]);
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

	char *pBuffer = malloc(inputSize);
	if (!pBuffer) {
		perror("malloc()");
		exit(-4);
	}

	struct HSS_CompressedImage imgHdr;
	size_t bytesProcessed = fread(pBuffer, 1, inputSize, pFileIn);
	if (bytesProcessed != inputSize) {
		perror("fread()");
		exit(-5);
	}
	printf("%s: fread okay for input buffer of %lu bytes\n", argv[0], inputSize);
	memcpy(&imgHdr, pBuffer, sizeof(imgHdr));

	fclose(pFileIn);

	printf("Header Size:			%lu\n", sizeof(struct HSS_CompressedImage));

	printf("Magic:				0x%08X (expected 0x%08X)\n", imgHdr.magic, mHSS_COMPRESSED_MAGIC);
	printf("Version:			0x%08X (expected 0x%08X)\n", imgHdr.version,
		mHSS_COMPRESSED_VERSION_MINIZ);
	printf("Header Length:			%lu\n",	imgHdr.headerLength);
	printf("Header CRC:			0x%08X\n", imgHdr.headerCrc);
	printf("Compressed CRC:			0x%08X\n", imgHdr.compressedCrc);
	printf("Original CRC:			0x%08X\n", imgHdr.originalCrc);
	printf("Compressed Image Length:	0x%08lX\n", imgHdr.compressedImageLen);
	printf("Original Image Length:		0x%08lX\n", imgHdr.originalImageLen);

	size_t upperBound;
	upperBound = imgHdr.originalImageLen;
	char *pOriginalBuffer = malloc(upperBound);
	if (!pOriginalBuffer) {
		perror("malloc()");
		exit(-6);
	}

	printf("Skipping %lu bytes, where I find ... ", imgHdr.headerLength);
	for (int i = 0; i < 8; i++) {
		printf("%02x ", (unsigned char)pBuffer[imgHdr.headerLength + i]);
	}
	printf("\n\n");

	int status = uncompress(pOriginalBuffer, &upperBound, pBuffer + imgHdr.headerLength, imgHdr.compressedImageLen);
	if (status != MZ_OK) {
		fprintf(stderr, "uncompress() returned %d\n", status);
		exit(-7);
	}

	printf("Decompressed okay. Decompressed size is %lu\n", upperBound);

	FILE *fileOut = fopen("out.bin", "wb");
	if (!fileOut) {
		perror("fopen()");
		exit(-8);
	}

	size_t outSize = fwrite(pOriginalBuffer, 1, upperBound, fileOut);
	if (outSize != upperBound) {
		fprintf(stderr, "Mismatch on size written (%lu) vs size of buffer (%lu)\n", outSize, upperBound);
		exit(-9);
	}

	fclose(fileOut);

	free(pBuffer);
	free(pOriginalBuffer);

	return 0;
}
