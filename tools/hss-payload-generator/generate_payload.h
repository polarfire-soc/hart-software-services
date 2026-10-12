#ifndef GENERATE_PAYLOAD_H
#define GENERATE_PAYLOAD_H

/******************************************************************************************
 *
 * MPFS HSS Embedded Software - tools/hss-payload-generator
 *
 * Copyright 2020-2025 Microchip FPGA Embedded Systems Solutions.
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

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#include "hss_types.h"

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


void generate_payload(char const * const filename_output, char const * const private_key_filename, char const * const public_key_filename);
void generate_init(void);

void generate_set_blob_offset(off_t offset);

size_t generate_add_chunk(struct HSS_BootChunkDesc chunk, void *buffer) __attribute__((nonnull));
size_t generate_add_ziChunk(struct HSS_BootZIChunkDesc ziChunk);

#endif
