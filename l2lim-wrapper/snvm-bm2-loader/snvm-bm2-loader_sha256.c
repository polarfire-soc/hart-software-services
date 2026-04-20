/*******************************************************************************
 * Copyright 2019-2026 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Standalone SHA-256 implementation for the sNVM BM2 loader.
 *
 * No external dependencies beyond <stdint.h> and <stddef.h>.  The message
 * schedule array (m[64]) is declared static to avoid a 256-byte stack
 * allocation inside sha256_transform; the loader is single-threaded so this
 * is safe.
 *
 * Algorithm: FIPS PUB 180-4.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "snvm-bm2-loader_sha256.h"

/* SHA-256 round constants - first 32 bits of the fractional parts of the
 * cube roots of the first 64 primes. */
static const uint32_t K[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

/* Initial hash values - first 32 bits of fractional parts of sqrt of first
 * 8 primes. */
static const uint32_t H0[8] = {
    0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
    0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
};

#define ROTR32(x, n) (((x) >> (n)) | ((x) << (32u - (n))))

#define CH(x, y, z)   (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z)  (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define BSIG0(x)  (ROTR32((x), 2u)  ^ ROTR32((x), 13u) ^ ROTR32((x), 22u))
#define BSIG1(x)  (ROTR32((x), 6u)  ^ ROTR32((x), 11u) ^ ROTR32((x), 25u))
#define SSIG0(x)  (ROTR32((x), 7u)  ^ ROTR32((x), 18u) ^ ((x) >> 3u))
#define SSIG1(x)  (ROTR32((x), 17u) ^ ROTR32((x), 19u) ^ ((x) >> 10u))

/* Expanded message schedule - static to avoid 256-byte stack allocation.
 * Safe because the loader is single-threaded (E51 only). */
static uint32_t msg_sched[64];

static void sha256_transform(sha256_ctx_t *ctx)
{
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t T1, T2;
    unsigned int t;

    /* Prepare message schedule from the 64-byte block in ctx->buf */
    for (t = 0u; t < 16u; t++) {
        msg_sched[t] =
            ((uint32_t)ctx->buf[t * 4u + 0u] << 24u) |
            ((uint32_t)ctx->buf[t * 4u + 1u] << 16u) |
            ((uint32_t)ctx->buf[t * 4u + 2u] <<  8u) |
            ((uint32_t)ctx->buf[t * 4u + 3u]);
    }
    for (t = 16u; t < 64u; t++) {
        msg_sched[t] = SSIG1(msg_sched[t-2u]) + msg_sched[t-7u] +
                       SSIG0(msg_sched[t-15u]) + msg_sched[t-16u];
    }

    /* Initialise working variables */
    a = ctx->h[0]; b = ctx->h[1]; c = ctx->h[2]; d = ctx->h[3];
    e = ctx->h[4]; f = ctx->h[5]; g = ctx->h[6]; h = ctx->h[7];

    /* 64 rounds */
    for (t = 0u; t < 64u; t++) {
        T1 = h + BSIG1(e) + CH(e, f, g) + K[t] + msg_sched[t];
        T2 = BSIG0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + T1;
        d = c; c = b; b = a; a = T1 + T2;
    }

    /* Update hash state */
    ctx->h[0] += a; ctx->h[1] += b; ctx->h[2] += c; ctx->h[3] += d;
    ctx->h[4] += e; ctx->h[5] += f; ctx->h[6] += g; ctx->h[7] += h;
}

void sha256_init(sha256_ctx_t *ctx)
{
    unsigned int i;
    for (i = 0u; i < 8u; i++) {
        ctx->h[i] = H0[i];
    }
    ctx->nbytes = 0u;
}

void sha256_update(sha256_ctx_t *ctx, const uint8_t *data, size_t len)
{
    size_t i;
    for (i = 0u; i < len; i++) {
        uint32_t pos = (uint32_t)(ctx->nbytes & 63u);
        ctx->buf[pos] = data[i];
        ctx->nbytes++;
        if (pos == 63u) {
            sha256_transform(ctx);
        }
    }
}

void sha256_final(sha256_ctx_t *ctx, uint8_t digest[SHA256_DIGEST_LEN])
{
    uint64_t bit_count;
    uint32_t pos;
    unsigned int i;

    pos = (uint32_t)(ctx->nbytes & 63u);
    bit_count = ctx->nbytes * 8u;

    /* Append 0x80 padding byte */
    ctx->buf[pos++] = 0x80u;

    /* If no room for the 8-byte length field in this block, pad and process */
    if (pos > 56u) {
        while (pos < 64u) {
            ctx->buf[pos++] = 0x00u;
        }
        sha256_transform(ctx);
        pos = 0u;
    }

    /* Zero-pad up to byte 56 */
    while (pos < 56u) {
        ctx->buf[pos++] = 0x00u;
    }

    /* Append bit count as big-endian 64-bit integer */
    ctx->buf[56] = (uint8_t)(bit_count >> 56u);
    ctx->buf[57] = (uint8_t)(bit_count >> 48u);
    ctx->buf[58] = (uint8_t)(bit_count >> 40u);
    ctx->buf[59] = (uint8_t)(bit_count >> 32u);
    ctx->buf[60] = (uint8_t)(bit_count >> 24u);
    ctx->buf[61] = (uint8_t)(bit_count >> 16u);
    ctx->buf[62] = (uint8_t)(bit_count >>  8u);
    ctx->buf[63] = (uint8_t)(bit_count       );

    sha256_transform(ctx);

    /* Serialise hash to digest (big-endian) */
    for (i = 0u; i < 8u; i++) {
        digest[i * 4u + 0u] = (uint8_t)(ctx->h[i] >> 24u);
        digest[i * 4u + 1u] = (uint8_t)(ctx->h[i] >> 16u);
        digest[i * 4u + 2u] = (uint8_t)(ctx->h[i] >>  8u);
        digest[i * 4u + 3u] = (uint8_t)(ctx->h[i]       );
    }
}

/*
 * bm2_validate_sha256 - hash [data, data+len) and compare against the
 * build-time-embedded expected digest.
 *
 * Returns true if the digest matches, false otherwise.
 */
bool bm2_validate_sha256(const uint8_t *data, size_t len,
                         const uint8_t expected[SHA256_DIGEST_LEN])
{
    sha256_ctx_t ctx;
    uint8_t computed[SHA256_DIGEST_LEN];
    unsigned int i;
    uint8_t diff = 0u;

    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, computed);

    for (i = 0u; i < SHA256_DIGEST_LEN; i++) {
        diff |= computed[i] ^ expected[i];
    }

    return diff == 0u;
}
