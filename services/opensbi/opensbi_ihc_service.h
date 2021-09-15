#ifndef ICC_OPENSBI_SERVICE_H
#define ICC_OPENSBI_SERVICE_H

/*******************************************************************************
 * Copyright 2019-2021 Microchip Corporation.
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
 *
 */


#ifdef __cplusplus
extern "C" {
#endif

#include <string.h> 

#define SBI_EXT_IHC        0x12341234
#define SBI_IHC_CTX_INIT   0x0
#define SBI_EXT_IHC_SEND    0x1
#define SBI_EXT_IHC_RECEIVE    0x2

enum {
	MP_IRQ = 0x0,
	ACK_IRQ = 0x1,
};

struct mpfs_ihc_msg {
	uint32_t msg[IHC_MAX_MESSAGE_SIZE];
};

struct ihc_sbi_rx_msg {
	uint8_t irq_type;
	struct mpfs_ihc_msg ihc_msg;
};

void IHC_SBI_Ecall_Register(void);

#ifdef __cplusplus
}
#endif

#endif
