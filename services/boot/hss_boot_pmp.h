#ifndef HSS_BOOT_PMP_H
#define HSS_BOOT_PMP_H

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
 * Hart Software Services - Physical Memory Protections
 *
 */


#ifdef __cplusplus
extern "C" {
#endif

enum PMP_Priv_S {
  PMP_READ  = 1 << 0,
  PMP_WRITE = 1 << 1,
  PMP_EXEC  = 1 << 2,
  PMP_LOCK  = 1 << 7
};

#define MAX_NUM_PMPS 16u

bool HSS_PMP_Init(void);

bool HSS_PMP_CheckWrite(enum HSSHartId target, const ptrdiff_t regionStartAddr, size_t length);
bool HSS_PMP_CheckRead(enum HSSHartId target, const ptrdiff_t regionStartAddr, size_t length);

/* TODO: the following enum could be removed from this header and kept local in scope */
enum AddressMatchingMode
{
    AddressMatchingMode_NULL_REGION = 0,
    AddressMatchingMode_TOR = 1,
    AddressMatchingMode_NA4 = 2,
    AddressMatchingMode_NAPOT = 3
};

/* TODO: the following struct could be removed from this header and kept local in scope */
struct PmpEntry
{
    uint64_t baseAddr;
    uint64_t size;
    enum AddressMatchingMode A;
    unsigned int R:1;
    unsigned int W:1;
    unsigned int X:1;
    unsigned int L:1;
};

// uint64_t pmp_decode_napot_size_encoding(uint64_t addrVal, uint64_t *pMask);
// void pmp_decode(struct PmpEntry *pPmpEntry, struct PmpEntry *pPreviousPmpEntry, uint64_t configVal, uint64_t addrVal);

#ifdef __cplusplus
}
#endif

#endif
