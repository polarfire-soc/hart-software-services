#ifndef OPENSBI_CRYPTO_ECALL_H
#define OPENSBI_CRYPTO_ECALL_H

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
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "opensbi_ecall.h"

#ifndef BIT
#  define BIT(nr)		(1UL << (nr))
#endif

struct mchp_crypto_info {
	uint32_t services;
#define CRYPTO_SERVICE_AES		BIT(0)
#define CRYPTO_SERVICE_AEAD		BIT(1)
#define CRYPTO_SERVICE_HASH		BIT(2)
#define CRYPTO_SERVICE_MAC		BIT(3)
#define CRYPTO_SERVICE_RSA		BIT(4)
#define CRYPTO_SERVICE_DSA		BIT(5)
#define CRYPTO_SERVICE_ECDSA		BIT(6)
#define CRYPTO_SERVICE_NRBG		BIT(7)
	uint64_t cipher_algo;
#define CRYPTO_ALG_AES_ECB		BIT(0)
#define CRYPTO_ALG_AES_CBC		BIT(1)
#define CRYPTO_ALG_AES_OFB		BIT(2)
#define CRYPTO_ALG_AES_CFB		BIT(3)
#define CRYPTO_ALG_AES_CTR		BIT(4)
	uint32_t aead_algo;
	uint32_t hash_algo;
	uint64_t mac_algo;
	uint64_t akcipher_algo;
	uint32_t nrbg_algo;
};

struct mchp_crypto_aes_req {
	uint64_t src;
	uint64_t iv;
	uint64_t key;
	uint64_t dst;
	uint64_t size;
};

int sbi_ecall_crypto_handler(unsigned long extid, unsigned long funcid,
			     const struct sbi_trap_regs *regs,
			     unsigned long *out_val,
			     struct sbi_trap_info *out_trap);

#ifdef __cplusplus
}
#endif

#endif
