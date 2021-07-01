#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../../../include/hss_types.h"


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

	struct HSS_CompressedImage imgHdr;
	size_t bytesProcessed = fread(&imgHdr, 1, sizeof(struct HSS_CompressedImage), pFileIn);
	if (bytesProcessed != sizeof(struct HSS_CompressedImage)) {
		perror("fread()");
		exit(-4);
	}
	printf("%s: fread okay for input buffer of %lu bytes\n", argv[0], inputSize);

	fclose(pFileIn);

	printf("Header Size:			%lu\n", sizeof(struct HSS_CompressedImage));

	printf("offsetof(magic):		%lu\n", offsetof(struct HSS_CompressedImage, magic));
	printf("offsetof(version):		%lu\n", offsetof(struct HSS_CompressedImage, version));
	printf("offsetof(headerLength):		%lu\n", offsetof(struct HSS_CompressedImage, headerLength));
	printf("offsetof(headerCrc):		%lu\n", offsetof(struct HSS_CompressedImage, headerCrc));
	printf("offsetof(compressedCrc):	%lu\n", offsetof(struct HSS_CompressedImage, compressedCrc));
	printf("offsetof(originalCrc):		%lu\n", offsetof(struct HSS_CompressedImage, originalCrc));
	printf("offsetof(compressedImageLen):	%lu\n", offsetof(struct HSS_CompressedImage, compressedImageLen));
	printf("offsetof(originalImageLen):	%lu\n", offsetof(struct HSS_CompressedImage, originalImageLen));
	printf("offsetof(hash):			%lu\n", offsetof(struct HSS_CompressedImage, hash));
	printf("offsetof(ecdsaSig):		%lu\n", offsetof(struct HSS_CompressedImage, ecdsaSig));

	printf("Magic:				0x%08X (expected 0x%08X)\n", imgHdr.magic, mHSS_COMPRESSED_MAGIC);
	printf("Version:			0x%08X (expected 0x%08X)\n", imgHdr.version,
		mHSS_COMPRESSED_VERSION_DEFLATE);
	printf("Header Length:			%lu\n",	imgHdr.headerLength);
	printf("Header CRC:			0x%08X\n", imgHdr.headerCrc);
	printf("Compressed CRC:			0x%08X\n", imgHdr.compressedCrc);
	printf("Original CRC:			0x%08X\n", imgHdr.originalCrc);
	printf("Compressed Image Length:	0x%08lX\n", imgHdr.compressedImageLen);
	printf("Original Image Length:		0x%08lX\n", imgHdr.originalImageLen);

	return 0;
}
