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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <elf.h>
#include <gelf.h>
#include <libelf.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "blob_handler.h"
#include "elf_strings.h"
#include "hss_types.h"
#include "debug_printf.h"
#include "generate_payload.h"
#include "crc32.h"


extern bool wide_output;
extern struct HSS_BootImage bootImage;

static size_t numChunks = 0u;

/////////////////////////////////////////////////////////////////////////////
//
// Local Function Prototypes
//

static void process_blob(void *pBuffer, uintptr_t exec_addr, size_t size, size_t owner, bool is_ancilliary_data);

/////////////////////////////////////////////////////////////////////////////
//
// Local Functions
//

static void process_blob(void *pBuffer, uintptr_t exec_addr, size_t size, size_t owner, bool is_ancilliary_data)
{
	assert(pBuffer);

	struct HSS_BootChunkDesc chunk = {
		.owner = owner | (is_ancilliary_data ? BOOT_FLAG_ANCILLIARY_DATA : 0u),
			.loadAddr = 0u,
			.execAddr = (uintptr_t)exec_addr,
			.size = size,
			.crc32 = CRC32_calculate(pBuffer, size)
	};

	numChunks = generate_add_chunk(chunk, pBuffer); // deliberately orphaning pBuffer for simplicity
}

/////////////////////////////////////////////////////////////////////////////
//
// Module Inteface
//

void blob_handler_init(void)
{
	; // nothing to do
}

bool blob_handler(char const * const filename, uintptr_t exec_addr, size_t owner, bool ancilliary_data_present, char const * const ancilliary_filename)
{
	bool result = true;

	assert(filename);

	if (ancilliary_data_present) {
		assert(ancilliary_filename);
	}

	debug_printf(1, "\nProcessing blob >>%s<< - placing at %p\n", filename, exec_addr);
	if (!bootImage.hart[owner-1].firstChunk) {
		bootImage.hart[owner-1].firstChunk = numChunks;
	}

	size_t size;

	// main blob
	{
		FILE *fileIn = fopen(filename, "r+");
		if (!fileIn) {
			fprintf(stderr, "%s(): File: %s -", __func__, filename);
			perror("fopen()");
			exit(EXIT_FAILURE);
		}

			fseeko(fileIn, 0, SEEK_END);
		size = (size_t)ftello(fileIn);

		void *pBuffer = malloc(size);
		assert(pBuffer);

		fseeko(fileIn, 0, SEEK_SET);
		if (fread(pBuffer, 1, size, fileIn) != size) {
			perror("fread()");
			exit(EXIT_FAILURE);
		}

		process_blob(pBuffer, exec_addr, size, owner, false); // deliberately orphaning pBuffer for simplicity

		bootImage.hart[owner-1].lastChunk = numChunks - 1u;
		bootImage.hart[owner-1].numChunks += 1u;
		debug_printf(1, "lastChunk is %d, numChunks is %d\n", bootImage.hart[owner-1].lastChunk, bootImage.hart[owner-1].numChunks);

		fclose(fileIn);
	}

	// ancilliary data (e.g., a DTB)
	if (ancilliary_data_present) {
		exec_addr += size; // increment past main blob
		debug_printf(1, "\nProcessing blob >>%s<< - placing at %p\n", ancilliary_filename, exec_addr);

		FILE *fileIn = fopen(ancilliary_filename, "r+");
		if (!fileIn) {
			fprintf(stderr, "%s(): File: %s -", __func__, ancilliary_filename);
			perror("fopen()");
			exit(EXIT_FAILURE);
		}

		fseeko(fileIn, 0, SEEK_END);
		size = (size_t)ftello(fileIn);

		void *pBuffer = malloc(size);
		assert(pBuffer);

		fseeko(fileIn, 0, SEEK_SET);
		if (fread(pBuffer, 1, size, fileIn) != size) {
			perror("fread()");
			exit(EXIT_FAILURE);
		}

		process_blob(pBuffer, exec_addr, size, owner, true); // deliberately orphaning pBuffer for simplicity

		bootImage.hart[owner-1].lastChunk = numChunks - 1u;
		bootImage.hart[owner-1].numChunks += 1u;
		debug_printf(1, "lastChunk is %d, numChunks is %d\n", bootImage.hart[owner-1].lastChunk, bootImage.hart[owner-1].numChunks);

		fclose(fileIn);
	}

	return result;
}
