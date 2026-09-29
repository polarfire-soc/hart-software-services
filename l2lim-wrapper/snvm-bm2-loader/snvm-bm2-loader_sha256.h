/*******************************************************************************
 * Copyright 2019-2026 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Standalone SHA-256 API for the sNVM BM2 loader.
 */

#ifndef SNVM_BM2_LOADER_SHA256_H_
#define SNVM_BM2_LOADER_SHA256_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SHA256_DIGEST_LEN  32u

typedef struct {
    uint32_t h[8];      /* current hash state */
    uint8_t  buf[64];   /* partial block buffer */
    uint64_t nbytes;    /* total bytes fed so far */
} sha256_ctx_t;

void sha256_init(sha256_ctx_t *ctx);
void sha256_update(sha256_ctx_t *ctx, const uint8_t *data, size_t len);
void sha256_final(sha256_ctx_t *ctx, uint8_t digest[SHA256_DIGEST_LEN]);

/*
 * bm2_validate_sha256 - compute SHA-256 of [data, data+len) and compare
 * against the expected digest.  Returns true on match.
 */
bool bm2_validate_sha256(const uint8_t *data, size_t len,
                         const uint8_t expected[SHA256_DIGEST_LEN]);

#endif /* SNVM_BM2_LOADER_SHA256_H_ */
