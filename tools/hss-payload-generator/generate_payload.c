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
 *
 */

#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include "crc32.h"
#include "generate_payload.h"
#include "debug_printf.h"

static_assert(sizeof(void *)==8, "Fatal: this program requires a 64bit compiler");

#ifdef __MINGW32__
	// x86_64-W64_MING32 has 4-byte long, whereas Linux x64/RV64 compilers have 8-byte long
	// we'll fix up the type defs here to ensure that the structures (including padding)
	// are the same sizes...
	//
#	define __ssize_t_defined
	typedef long long	  HSS_ssize_t;
	typedef unsigned long long HSS_size_t;
	typedef unsigned long long HSS_uintptr_t;
	typedef unsigned long long HSS_off_t;
#	define size_t	HSS_size_t
#	define ssize_t   HSS_ssize_t
#	define uintptr_t HSS_uintptr_t
#	define off_t	 HSS_off_t
#endif

#ifndef CONFIG_CC_HAS_INTTYPES
#	define CONFIG_CC_HAS_INTTYPES
#endif

#include "hss_types.h"

#ifdef __riscv
#	error 1
#endif

#if defined(__LP64__) || defined(_LP64)
#	ifndef PRIx64
#		define PRIx64 "lx"
#	endif
# 	ifndef PRIu64
#		define PRIu64 "lu"
#	endif
#else
#	ifndef PRIx64
#		define PRIx64 "I64x"
#	endif
#	ifndef PRIu64
#		define PRIu64 "I64u"
#	endif
#endif


#define PAD_SIZE  8

/************************************************************************************/

static struct chunkTableEntry {
	struct HSS_BootChunkDesc chunk;
	void *pBuffer;
} *chunkTable = NULL;
static struct ziChunkTableEntry {
	struct HSS_BootZIChunkDesc ziChunk;
} *ziChunkTable = NULL;

static size_t numChunks = 0;
static size_t numZIChunks = 0;

off_t bootImagePaddedSize = 0u;
off_t chunkTablePaddedSize = 0u;
off_t ziChunkTablePaddedSize = 0u;

/************************************************************************************/

static size_t calculate_padding(size_t size, size_t pad);
static void write_pad(FILE *pFileOut, size_t pad) __attribute__((nonnull));
static void generate_header(FILE *pFileOut, struct HSS_BootImage *pBootImage) __attribute__((nonnull));
static void generate_chunks(FILE *pFileOut) __attribute__((nonnull));
static void generate_ziChunks(FILE *pFileOut) __attribute__((nonnull));
static void generate_blobs(FILE *pFileOut) __attribute__((nonnull));

extern struct HSS_BootImage bootImage;

/************************************************************************************/

static size_t calculate_padding(size_t size, size_t pad)
{
	assert(pad);

	//
	// given an actual size, and a desired pad, calculate
	// how many additional bytes are required to bring size up
	// to a multiple of the pad size...
	size_t result = (((size + (pad - 1)) / pad) * pad) - size;

	return result;
}

static void write_pad(FILE *pFileOut, size_t pad)
{
	assert(pFileOut);

	size_t i;

	for (i = 0u; i < pad; i++) {
		fputc(0, pFileOut);

		if (ferror(pFileOut) || feof(pFileOut)) {
			perror("fwrite()");
			exit(EXIT_SUCCESS);
		}
	}
}

static void generate_header(FILE *pFileOut, struct HSS_BootImage *pBootImage)
{
	debug_printf(0, "Outputting Payload Header\n");

	assert(pFileOut);
	assert(pBootImage);

	if (fseek(pFileOut, 0, SEEK_SET) != 0) {
		perror("fseek()");
		exit(EXIT_SUCCESS);
	}

	fwrite((char *)pBootImage, sizeof(struct HSS_BootImage), 1, pFileOut);
	if (ferror(pFileOut) || feof(pFileOut)) {
		perror("fwrite()");
		exit(EXIT_SUCCESS);
	}

	write_pad(pFileOut,
		calculate_padding(sizeof(struct HSS_BootImage), PAD_SIZE));


	bootImagePaddedSize = ftello(pFileOut);
}

static void generate_chunks(FILE *pFileOut)
{
	debug_printf(0, "Outputting Code/Data Chunks\n");

	assert(pFileOut);

	bootImage.chunkTableOffset = (size_t)ftello(pFileOut);

	// sanity check we are were we expected to be, vis-a-vis file padding
	assert(bootImage.chunkTableOffset ==
			sizeof(struct HSS_BootImage)
			+ calculate_padding(sizeof(struct HSS_BootImage), PAD_SIZE));

	size_t cumulativeBlobSize = 0u;

	for (size_t i = 0u; i < numChunks; i++) {
		// calculate offset for chunk blob in file:
		//   = len(header) + len(chunkTable) + len(ziChunkTable) + len(all previous blobs)
		chunkTable[i].chunk.loadAddr =
			bootImage.chunkTableOffset
			+ (numChunks * sizeof(struct HSS_BootChunkDesc))
			+ sizeof(struct HSS_BootChunkDesc) // account for sentinel
			+ calculate_padding(sizeof(struct HSS_BootChunkDesc) * (numChunks + 1), PAD_SIZE)
			+ (numZIChunks * sizeof(struct HSS_BootZIChunkDesc))
			+ sizeof(struct HSS_BootZIChunkDesc) // account for sentinel
			+ calculate_padding(sizeof(struct HSS_BootZIChunkDesc) * (numZIChunks +1), PAD_SIZE)
			+ cumulativeBlobSize
			+ calculate_padding(cumulativeBlobSize, PAD_SIZE);

		cumulativeBlobSize += chunkTable[i].chunk.size
			+ calculate_padding(chunkTable[i].chunk.size, PAD_SIZE);

        	off_t posn = ftello(pFileOut);
		debug_printf(4, "\t- Processing chunk %lu (%lu bytes) at file position %lu "
			"(blob is expected at %lu)\n",
			i, chunkTable[i].chunk.size, posn, chunkTable[i].chunk.loadAddr);

		fwrite((char *)&(chunkTable[i].chunk), sizeof(struct HSS_BootChunkDesc), 1, pFileOut);
		if (ferror(pFileOut) || feof(pFileOut)) {
			perror("fwrite()");
			exit(EXIT_SUCCESS);
		}
	}

	// terminating sentinel
	struct HSS_BootChunkDesc bootChunk = {
		.owner = 0u,
		.loadAddr = 0u,
		.execAddr = 0u,
		.size = 0u,
		.crc32 = 0u
	};

	fwrite((char *)&bootChunk, sizeof(struct HSS_BootChunkDesc), 1, pFileOut);
	if (ferror(pFileOut) || feof(pFileOut)) {
		perror("fwrite()");
		exit(EXIT_SUCCESS);
	}

	write_pad(pFileOut,
		calculate_padding(sizeof(struct HSS_BootChunkDesc) * (numChunks + 1), PAD_SIZE));

	chunkTablePaddedSize = ftello(pFileOut) - (off_t)bootImage.chunkTableOffset;
}

static void generate_ziChunks(FILE *pFileOut)
{
	debug_printf(0, "Outputting ZI Chunks\n");

	assert(pFileOut);

	bootImage.ziChunkTableOffset = (size_t)ftello(pFileOut);

	// sanity check we are were we expected to be, vis-a-vis file padding
	assert(bootImage.ziChunkTableOffset ==
			bootImage.chunkTableOffset
			+ (numChunks * sizeof(struct HSS_BootChunkDesc))
			+ sizeof(struct HSS_BootChunkDesc)
			+ calculate_padding(sizeof(struct HSS_BootChunkDesc) * (numChunks+1), PAD_SIZE));

	for (size_t i = 0u; i < numZIChunks; i++) {
        	off_t posn = ftello(pFileOut);
		debug_printf(4, "\t- Processing ziChunk %lu (%lu bytes) at file position %lu\n",
			i, ziChunkTable[i].ziChunk.size, posn);

		fwrite((char *)&ziChunkTable[i].ziChunk,
			sizeof(struct HSS_BootZIChunkDesc), 1, pFileOut);
		if (ferror(pFileOut) || feof(pFileOut)) {
			perror("fwrite()");
			exit(EXIT_SUCCESS);
		}
	}

	// terminating sentinel
	struct HSS_BootZIChunkDesc ziChunk = {
		.owner = 0u,
		.execAddr = 0u,
		.size = 0u
	};

	fwrite((char *)&ziChunk, sizeof(struct HSS_BootZIChunkDesc), 1, pFileOut);
	if (ferror(pFileOut) || feof(pFileOut)) {
		perror("fwrite()");
		exit(EXIT_SUCCESS);
	}

	write_pad(pFileOut,
		calculate_padding(sizeof(struct HSS_BootZIChunkDesc) * (numZIChunks + 1), PAD_SIZE));

	ziChunkTablePaddedSize = ftello(pFileOut) - (off_t)bootImage.ziChunkTableOffset;
}

static void generate_blobs(FILE *pFileOut)
{
	debug_printf(0, "Outputting Binary Data\n");

	// sanity check we are were we expected to be, vis-a-vis file padding
	assert(chunkTable[0].chunk.loadAddr ==
			bootImage.ziChunkTableOffset
			+ (numZIChunks * sizeof(struct HSS_BootZIChunkDesc))
			+ sizeof(struct HSS_BootZIChunkDesc) // account for sentinel
			+ calculate_padding(sizeof(struct HSS_BootZIChunkDesc) * (numZIChunks +1), PAD_SIZE));

	for (size_t i = 0u; i < numChunks; i++) {
        	off_t posn = ftello(pFileOut);
		debug_printf(4, "\t- Processing blob %lu (%lu bytes) at file position %lu\n",
			i, chunkTable[i].chunk.size, posn);
		debug_printf(4, "\t\tCRC32: %x\n",
			CRC32_calculate((uint8_t *)chunkTable[i].pBuffer, chunkTable[i].chunk.size));
		fflush(stdout);

		fwrite((char *)chunkTable[i].pBuffer, chunkTable[i].chunk.size, 1, pFileOut);
		if (ferror(pFileOut) || feof(pFileOut)) {
			perror("fwrite()");
			exit(EXIT_SUCCESS);
		}

		free(chunkTable[i].pBuffer);

		write_pad(pFileOut,
			calculate_padding(chunkTable[i].chunk.size, PAD_SIZE));
	}
	assert(pFileOut);
}

void generate_payload(char const * const filename_output)
{
	assert(filename_output);

	printf("Output filename is >>%s<<\n", filename_output);

	FILE *pFileOut = fopen(filename_output, "w+");
	if (!pFileOut) {
		perror("fopen()");
		exit(EXIT_FAILURE);
	}

	generate_header(pFileOut, &bootImage);
	generate_chunks(pFileOut);
	generate_ziChunks(pFileOut);

	bootImage.headerLength = (size_t)ftello(pFileOut);
	assert(bootImage.headerLength ==
		bootImagePaddedSize + chunkTablePaddedSize + ziChunkTablePaddedSize);
	debug_printf(4, "End of header is %lu\n", bootImage.headerLength);

	generate_blobs(pFileOut);
	bootImage.bootImageLength = (size_t)ftello(pFileOut);

    	bootImage.headerCrc =
		CRC32_calculate((const unsigned char *)&bootImage, sizeof(struct HSS_BootImage));
	generate_header(pFileOut, &bootImage); // rewrite header...

	if (fclose(pFileOut) != 0) {
		perror("fclose()");
		exit(EXIT_FAILURE);
	}
}

size_t generate_add_chunk(struct HSS_BootChunkDesc chunk, void *pBuffer)
{
	assert(pBuffer);
	numChunks++;

	debug_printf(6, "\nAttempting to realloc %lu at %p",
		numChunks * sizeof(struct chunkTableEntry), chunkTable);
	void *tmpPtr = realloc(chunkTable, numChunks * sizeof(struct chunkTableEntry));
	debug_printf(6, " => %p\n", tmpPtr);
	if (!tmpPtr) {
		perror("realloc()");
		exit(EXIT_FAILURE);
	} else {
		chunkTable = tmpPtr;
	}

	memset(&chunkTable[numChunks-1], 0, sizeof(struct chunkTableEntry));
	chunkTable[numChunks-1].chunk = chunk;
	chunkTable[numChunks-1].pBuffer = pBuffer;

	debug_printf(4, "chunk: execAddr = 0x%.16" PRIx64 ", size = 0x%.16" PRIx64 ", CRC32=%x\n",
		chunk.execAddr, chunk.size,
		CRC32_calculate((const unsigned char *)pBuffer, chunk.size));

	return numChunks;
}

size_t generate_add_ziChunk(struct HSS_BootZIChunkDesc ziChunk)
{
	numZIChunks++;

	debug_printf(6, "\nAttempting to realloc %lu at %p",
		numZIChunks * sizeof(struct ziChunkTableEntry), ziChunkTable);
	void *tmpPtr = realloc(ziChunkTable, numZIChunks * sizeof(struct ziChunkTableEntry));
	debug_printf(6, " => %p\n", tmpPtr);
	if (!tmpPtr) {
		perror("realloc()");
		exit(EXIT_FAILURE);
	} else {
		ziChunkTable = tmpPtr;
	}

	ziChunkTable[numZIChunks-1].ziChunk = ziChunk;

	debug_printf(4, "ziChunk: execAddr = 0x%.16" PRIx64 ", size = 0x%.16" PRIx64 "\n",
		ziChunk.execAddr, ziChunk.size);

	return numZIChunks;
}

void generate_init(void)
{
	bootImage.magic = mHSS_BOOT_MAGIC;
}
