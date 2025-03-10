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
 *
 */

#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include "crc32.h"
#include "debug_printf.h"
#include "verify_payload.h"

#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/obj_mac.h>
#include <openssl/sha.h>
#include <openssl/bn.h>

#include <openssl/opensslv.h>
#include <openssl/err.h>

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
#	define CONFIG_CC_HAS_INTTYPES 1
#endif
#ifndef CONFIG_CRYPTO_SIGNING
#	define CONFIG_CRYPTO_SIGNING 1
#endif

#include "hss_types.h"
#include "generate_payload.h"

#if defined(__LP64__) || defined(_LP64)
#	ifndef PRIx64
#		define PRIx64 "lx"
#	endif
#	ifndef PRIu64
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

#define ECDSA_P384_SIG_LEN ((384u/8)*2)
static bool Verify_ECDSA_P384(const size_t sigLen, uint8_t sigBuffer[sigLen],
    const size_t dataBufSize, uint8_t dataBuf[dataBufSize],
    char const * public_key_filename)
{
	bool result = false;

	// read in entire payload to calculate signature...
	uint8_t *pEntirePayloadBuffer = (uint8_t *)&dataBuf[0];
	uint8_t *pSignatureBuffer = &sigBuffer[0];

	//
	// read in the public key, and convert to an EC key
	//
	FILE *pubKeyFileIn = fopen(public_key_filename, "r");
	assert(pubKeyFileIn != NULL);

	// Get the size of the public key file
	fseeko(pubKeyFileIn, 0, SEEK_END);
	size_t pubKeyFileSize = (size_t)ftello(pubKeyFileIn);
	fseeko(pubKeyFileIn, 0, SEEK_SET);

	// Allocate buffer to hold the DER data
	uint8_t *pubKeyBuffer = (uint8_t *)malloc(pubKeyFileSize);
	assert(pubKeyBuffer != NULL);

	// Read the DER-encoded public key into the buffer
	assert(0 < fread(pubKeyBuffer, 1, pubKeyFileSize, pubKeyFileIn));
	fclose(pubKeyFileIn);

	//
	// convert the DER buffer to an EVP_PKEY structure
	//
	const uint8_t *pTemp = pubKeyBuffer;
	EVP_PKEY *pPubKey = NULL;
	assert(d2i_PUBKEY(&pPubKey, &pTemp, (long)pubKeyFileSize) != NULL);
	free(pubKeyBuffer);  // Free the buffer after converting to EVP_PKEY

	// print key to verify it is correct...
#if 0
	{
		BIO *out = BIO_new_fp(stdout, BIO_NOCLOSE);
		if (out) {
			EVP_PKEY_print_params(out, pPubKey, 0, NULL);
			EVP_PKEY_print_public(out, pPubKey, 0, NULL);
			BIO_free(out);
		} else {
			printf("Error: unable to create BIO for output\n");
		}
	}
#endif

	//
	// create the verification context by using SHA384 digest and the SECP384r1 public key
	//
	EVP_MD_CTX *pCtx = EVP_MD_CTX_new();
	assert(pCtx != NULL);
	
	assert(EVP_DigestVerifyInit(pCtx, NULL, EVP_sha384(), NULL, pPubKey) == 1);

	// create digest of the payload data
	assert(EVP_DigestVerifyUpdate(pCtx, pEntirePayloadBuffer, dataBufSize) == 1);

	{
		EVP_MD_CTX *hash_ctx = EVP_MD_CTX_new();
		unsigned char digest[EVP_MAX_MD_SIZE];
		unsigned int digest_len = 0;

		assert(1 == EVP_DigestInit_ex(hash_ctx, EVP_sha384(), NULL));
		assert(1 == EVP_DigestUpdate(hash_ctx, pEntirePayloadBuffer, dataBufSize));
		assert(1 == EVP_DigestFinal_ex(hash_ctx, digest, &digest_len));

		EVP_MD_CTX_free(hash_ctx);
	}


	//
	// verify the signature against the signed digest
	//
	int verify_result = EVP_DigestVerifyFinal(pCtx, pSignatureBuffer, sigLen);

	if (verify_result != 1) {
		ERR_print_errors_fp(stderr);
	} else {
		// signature is valid
		result = true;
	}	
	
	// Clean up
	EVP_MD_CTX_free(pCtx);
	EVP_PKEY_free(pPubKey);

	return result;
}

bool HSS_Boot_Secure_CheckCodeSigning(struct HSS_BootImage *pBootImage, char const * public_key_filename)
{
	bool result = false;

	struct HSS_BootImage *pRWBootImage = malloc(pBootImage->bootImageLength);
	assert(pRWBootImage);
	memcpy(pRWBootImage, (void *)pBootImage, pBootImage->bootImageLength);

	struct HSS_Signature originalSig __attribute__((aligned)) = pRWBootImage->signature;
	memset((void *)&(pRWBootImage->signature), 0, sizeof(struct HSS_Signature));

	uint8_t *pSig = (uint8_t *)&originalSig;
	uint8_t *r_bytes = pSig + 48;
	uint8_t *s_bytes = pSig + 96;

	BIGNUM *r = BN_bin2bn(r_bytes, 48, NULL); assert(r);
	BIGNUM *s = BN_bin2bn(s_bytes, 48, NULL); assert(s);

	ECDSA_SIG *sig = ECDSA_SIG_new(); assert(sig);
	ECDSA_SIG_set0(sig, r, s); // transfers ownership of r and s
	uint8_t *der = NULL;
	int der_len = i2d_ECDSA_SIG(sig, &der); assert(der_len >= 0);

	ECDSA_SIG_free(sig);

	assert(ARRAY_SIZE(pRWBootImage->signature.digest) == SHA384_DIGEST_LENGTH);

	result = Verify_ECDSA_P384((size_t)der_len, der, pRWBootImage->bootImageLength, (uint8_t *)pRWBootImage, public_key_filename);

	return result;
	OPENSSL_free(der);
}
