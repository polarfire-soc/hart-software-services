/******************************************************************************************
 *
 * MPFS HSS Embedded Software - tools/hss-payload-generator
 *
 * Copyright 2020-2021 Microchip Corporation.
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

static void process_blob(void *pBuffer, uintptr_t exec_addr, size_t size, size_t owner);

/////////////////////////////////////////////////////////////////////////////
//
// Local Functions
//

static void process_blob(void *pBuffer, uintptr_t exec_addr, size_t size, size_t owner)
{
	assert(pBuffer);

	struct HSS_BootChunkDesc chunk = {
    		.owner = owner,
    		.loadAddr = 0u,
    		.execAddr = (uintptr_t)exec_addr,
    		.size = size,
    		.crc32 = CRC32_calculate(pBuffer, size)
	};

	numChunks = generate_add_chunk(chunk, pBuffer);
}

/////////////////////////////////////////////////////////////////////////////
//
// Module Inteface
//

void blob_handler_init(void)
{
	; // nothing to do
}

bool blob_handler(char const * const filename, uintptr_t exec_addr, size_t owner)
{
	bool result = true;

	assert(filename);

	debug_printf(1, "\nProcessing blob >>%s<<\n", filename);
	if (!bootImage.hart[owner-1].firstChunk) {
		bootImage.hart[owner-1].firstChunk = numChunks;
	}

	FILE *fileIn = fopen(filename, "r+");
	if (!fileIn) {
		perror("fopen()");
		exit(EXIT_FAILURE);
	}

        fseeko(fileIn, 0, SEEK_END);
	size_t size = (size_t)ftello(fileIn);

	void *pBuffer = malloc(size);
	assert(pBuffer);

        fseeko(fileIn, 0, SEEK_SET);
	if (fread(pBuffer, 1, size, fileIn) != size) {
		perror("fread()");
		exit(EXIT_FAILURE);
	}

	process_blob(pBuffer, exec_addr, size, owner);

	bootImage.hart[owner-1].lastChunk = numChunks - 1u;
	bootImage.hart[owner-1].numChunks += 1u;

	fclose(fileIn);

	return result;
}
