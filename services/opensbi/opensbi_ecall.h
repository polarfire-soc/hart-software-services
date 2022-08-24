#ifndef OPENSBI_ECALL_H
#define OPENSBI_ECALL_H

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
 * Hart Software Services - OpenSBI ECALL Interface
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

// Previously the vendor ID was to be a number allocated by RISC-V International,
// but this duplicates the work of JEDEC in maintaining a manufacturer ID standard.
// SBI ECALLs in the VENDOR region are by MVENDORID as per section 3.1.2 of
// RISC-V Priviliged Architectures spec (2.2-draft or later)
//
// mvendorid[6:0] = JEDEC[6:0]
// mvendorid[XLEN-1:7] = number of 0x7f continuation codes


// Microchip Technology JEDEC Id: Bank 1, 0x29 => MVENDORID = 0x029
#define MICROCHIP_TECHNOLOGY_MVENDOR_ID  0x029

#define SBI_EXT_MICROCHIP_TECHNOLOGY       (SBI_EXT_VENDOR_START | MICROCHIP_TECHNOLOGY_MVENDOR_ID)

#define SBI_EXT_IHC_CTX_INIT   0x00
#define SBI_EXT_IHC_SEND       0x01
#define SBI_EXT_IHC_RECEIVE    0x02

#define SBI_EXT_HSS_REBOOT     0x10

#include <sbi/sbi_ecall.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_trap.h>

int HSS_SBI_ECALL_Handler(long extid, long funcid,
    const struct sbi_trap_regs *regs, unsigned long *out_val, struct sbi_trap_info *out_trap);
int HSS_SBI_Vendor_Ext_Check(long extid);

#ifdef __cplusplus
}
#endif

#endif
