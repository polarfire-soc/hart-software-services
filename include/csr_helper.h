#ifndef HSS_CSR_HELPER_H
#define HSS_CSR_HELPER_H

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
 * Hart Software Services - CSR Helper
 *
 */

/**
 * \file CSR Helper
 * \brief CSR Helper
 */

#ifdef __cplusplus
extern "C" {
#endif


#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#if IS_ENABLED(CONFIG_OPENSBI)
#  include "sbi/riscv_asm.h"
#  include "sbi/sbi_bitops.h"
#  include "sbi/sbi_hart.h"
#  include "sbi/sbi_hsm.h"
#  include "sbi/sbi_init.h"
#  include "sbi/sbi_scratch.h"
#  define mHSS_CSR_READ csr_read
#  define mHSS_CSR_WRITE csr_write
#else
#  include "mpfs_hal/encoding.h"
#  include "mpfs_hal/bits.h"
#  define mHSS_CSR_READ read_csr
#  define mHSS_CSR_WRITE write_csr
#endif


HSSTicks_t CSR_GetTickCount(void);
HSSTicks_t CSR_GetTime(void);
void CSR_ClearMSIP(void);

#ifdef __cplusplus
}
#endif

#endif
