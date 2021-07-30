#ifndef HSS_CRYPTO_TYPES_H
#define HSS_CRYPTO_TYPES_H

/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
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
 *
 * Hart Software Services - Crypto types
 *
 */

/*!
 * \file Crypto Driver State Machine
 * \brief MSS Crypto Driver
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct SNVMWriteNonAuthPlaintext
{
    uint8_t SVNMADDR;
    uint8_t RESERVED[3];
    uint8_t PT[252];
} __attribute__ ((packed));

struct SNVMWriteAuthPlaintext
{
    uint8_t SVNMADDR;
    uint8_t RESERVED[3];
    uint8_t PT[236];
    uint8_t USK[12];
} __attribute__ ((packed));

struct SNVMWriteAuthCiphertext
{
    uint8_t SVNMADDR;
    uint8_t RESERVED[3];
    uint8_t CT[236];
    uint8_t USK[12];
} __attribute__ ((packed));

#if 0
#ifdef __GNUC__
#define GCC_VERSION ((__GNUC__ * 10000)  + (__GNUC_MINOR__ * 100)  + (__GNUC_PATCHLEVEL__))
#if GCC_VERSION > 40300
// gcc (>4.3) supports C11 static asserts, so use them to ensure that our packing is okay...
//
_Static_assert(sizeof(struct SNVMWriteNonAuthPlaintext)==256, "size of struct SNVMWriteNonAuthPlaintext must be 256 bytes");
_Static_assert(sizeof(struct SNVMWriteAuthPlaintext)==252, "size of struct SNVMWriteNonAuthPlaintext must be 252 bytes");
_Static_assert(sizeof(struct SNVMWriteAuthCiphertext)==252, "size of struct SNVMWriteNonAuthPlaintext must be 252 bytes");
#endif
#endif
#endif

#ifdef __cplusplus
}
#endif


#endif
