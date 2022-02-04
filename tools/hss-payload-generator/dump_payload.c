/******************************************************************************************
 *
 * MPFS HSS Embedded Software - tools/hss-payload-generator
 *
 * Copyright 2020-2021 Microchip FPGA Embedded Systems Solutions.
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
 */

#define _FILE_OFFSET_BITS 64
#define NR_CPUs 4

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
#include "debug_printf.h"
#include "dump_payload.h"

#define PRV_U (0u)
#define PRV_S (1u)
#define PRV_M (3u)


/*
 * Local function prototypes
 */
static size_t getFileSize(const char *filename) __attribute__((nonnull));

static size_t getFileSize(const char *filename)
{
	struct stat st;
	stat(filename, &st);
	return (size_t)st.st_size;
}

static char const * privModeToString(uint8_t privMode)
{
	char const * result = "??";
	switch (privMode) {
	case PRV_U:
		result = "PRV_U";
		break;

	case PRV_S:
		result = "PRV_S";
		break;

	case PRV_M:
		result = "PRV_M";
		break;

	default:
		result = "Unknown";
		break;
	}

	return result;
}

void dump_payload(const char *filename_input)
{
	printf("opening >>%s<<\n", filename_input);
	int fdIn = open(filename_input, O_RDONLY);
	assert(fdIn >= 0);

	size_t fileSize = getFileSize(filename_input);

	struct HSS_BootImage *pBootImage;

	pBootImage = mmap(NULL, fileSize, PROT_READ,
		MAP_PRIVATE, fdIn, 0);
	assert(pBootImage);

	if (pBootImage->magic != mHSS_BOOT_MAGIC) {
		printf("Warning: does not look like a valid boot image"
			" (expected magic %x, got %x)\n", mHSS_BOOT_MAGIC, pBootImage->magic);
	}

	printf("magic:	            0x%x\n",	 pBootImage->magic);
	printf("headerLength:       0x%lx\n",	pBootImage->headerLength);
	printf("chunkTableOffset:   0x%lx\n",	pBootImage->chunkTableOffset);
	printf("ziChunkTableOffset: 0x%lx\n",	pBootImage->ziChunkTableOffset);

	for (unsigned int i = 0u; i < NR_CPUs; i++) {
		printf("name[%u]:            >>%s<<\n",  i, pBootImage->hart[i].name);
		printf("entryPoint[%u]:      0x%lx\n",	 i, pBootImage->hart[i].entryPoint);
		printf("privMode[%u]:        %u (%s)\n", i, pBootImage->hart[i].privMode,
			privModeToString(pBootImage->hart[i].privMode));
		printf("flags[%u]:           %x\n",	 i, pBootImage->hart[i].flags);

		if (pBootImage->hart[i].flags) {
			printf("\t");
			if (pBootImage->hart[i].flags & BOOT_FLAG_ANCILLIARY_DATA) {
				printf(" ANCILLIARY_DATA");
			}
			if (pBootImage->hart[i].flags & BOOT_FLAG_SKIP_OPENSBI) {
				printf(" SKIP_OPENSBI");
			}
			printf("\n");
		}

		printf("firstChunk[%u]       %lu\n",	i,
			(unsigned long)pBootImage->hart[i].firstChunk);
		printf("lastChunk[%u]        %lu\n",	i,
			(unsigned long)pBootImage->hart[i].lastChunk);
		printf("numChunks[%u]        %lu\n",	i,
			(unsigned long)pBootImage->hart[i].numChunks);
	}

	printf("set_name            >>%s<<\n", pBootImage->set_name);
	printf("bootImageLength:    %lu\n",	(unsigned long)pBootImage->bootImageLength);
	printf("headerCrc:          0x%08x\n", (unsigned int)pBootImage->headerCrc);

	off_t chunkOffset = 0u;
	size_t totalChunkCount = 0u;
	size_t localChunkCount = 0u;
	enum HSSHartId lastOwner = HSS_HART_E51;

	while (1) {
		struct HSS_BootChunkDesc bootChunk;
		memcpy(&bootChunk, ((char *)pBootImage) + pBootImage->chunkTableOffset + chunkOffset,
			sizeof(struct HSS_BootChunkDesc));

		if (totalChunkCount == 0) {
			lastOwner = bootChunk.owner;
			localChunkCount++;
		} else if (bootChunk.owner == lastOwner) {
			localChunkCount++;
		} else {
			printf(" - %lu chunk%s found for owner %u\n",
				(unsigned long)localChunkCount, (localChunkCount != 1) ? "s":"",
				lastOwner);
			localChunkCount = 1u;
			lastOwner = bootChunk.owner;
		}

		debug_printf(3, "\t%d / %lx / %lx / %lx / %x\n",
			bootChunk.owner, bootChunk.loadAddr, bootChunk.execAddr,
			bootChunk.size, bootChunk.crc32);

		chunkOffset += (off_t)sizeof(struct HSS_BootChunkDesc);
		totalChunkCount++;
		if (bootChunk.size==0u) { break;}
	}
	printf("Boot Chunks: total of %lu chunk%s found\n", (unsigned long)totalChunkCount,
		(totalChunkCount != 1u) ? "s":"");

	chunkOffset = 0u;
	totalChunkCount = 0u;
	localChunkCount = 0u;
	lastOwner = HSS_HART_E51;

	while (1) {
		struct HSS_BootZIChunkDesc ziChunk;
		memcpy(&ziChunk, ((char *)pBootImage) + pBootImage->ziChunkTableOffset + chunkOffset,
			sizeof(struct HSS_BootZIChunkDesc));

		if (totalChunkCount == 0) {
			lastOwner = ziChunk.owner;
			localChunkCount++;
		} else if (ziChunk.owner == lastOwner) {
			localChunkCount++;
		} else {
			printf(" - %lu ZI chunk%s found for owner %u\n",
				(unsigned long)localChunkCount, (localChunkCount != 1) ? "s":"",
				lastOwner);
			localChunkCount = 1u;
			lastOwner = ziChunk.owner;
		}

		debug_printf(0, "\t%d / %lx / %lx\n", ziChunk.owner, ziChunk.execAddr, ziChunk.size);

		chunkOffset += (off_t) sizeof(struct HSS_BootZIChunkDesc);
		totalChunkCount++;
		if (ziChunk.size==0u) { break;}
	}
	printf("ZI Chunks: total of %lu chunk%s found\n", (unsigned long)totalChunkCount,
		(totalChunkCount != 1u) ? "s":"");

	// skipping binary fie array

	munmap(pBootImage, fileSize);
	close(fdIn);
}
