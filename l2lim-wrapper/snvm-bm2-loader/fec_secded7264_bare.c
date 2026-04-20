/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * ---
 *
 * Adapted for freestanding (bare-metal) build from the original
 * fec_secded7264_encdec tool source by Microchip FPGA Embedded Systems
 * Solutions, 2026.  Changes from the original:
 *
 *   - Removed #include <stdio.h> and #include <stdlib.h>.
 *   - Removed fec_secded7264_encode(), fec_secded7264_encode_symbol() and
 *     fec_secded7264_compute_parity() (encode path not needed at runtime).
 *   - Replaced div(n,8) with integer shift/AND to avoid stdlib dependency.
 *   - Removed fec_secded7264_decode_lazy() (not used by the BM2 loader).
 *   - memset() is provided by snvm-bm2-loader_stubs.c.
 */

/*
 * SEC-DED (72,64) 8/9-rate forward error-correction block code - decoder only
 *
 * References:
 *  [Lin:2004] Lin, Shu and Costello, Daniel L. Jr., "Error Control
 *      Coding," Prentice Hall, New Jersey, 2nd edition, 2004.
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "fec_secded7264_bare.h"

/* number of ones in a byte */
static const unsigned char liquid_c_ones[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

/* P matrix [8 x 64] */
static const unsigned char secded7264_P[64] = {
    0xFF, 0x0F, 0x0F, 0x0C, 0x68, 0x88, 0x88, 0x80,
    0xF0, 0xFF, 0x00, 0xF3, 0x64, 0x44, 0x44, 0x40,
    0x30, 0xF0, 0xFF, 0x0F, 0x02, 0x22, 0x22, 0x26,
    0xCF, 0x00, 0xF0, 0xFF, 0x01, 0x11, 0x11, 0x16,
    0x68, 0x88, 0x88, 0x80, 0xFF, 0x0F, 0x00, 0xF3,
    0x64, 0x44, 0x44, 0x40, 0xF0, 0xFF, 0x0F, 0x0C,
    0x02, 0x22, 0x22, 0x26, 0xCF, 0x00, 0xFF, 0x0F,
    0x01, 0x11, 0x11, 0x16, 0x30, 0xF0, 0xF0, 0xFF
};

/* syndrome vectors for single-bit errors (weight-1) */
static const unsigned char secded7264_syndrome_w1[72] = {
    0x0b, 0x3b, 0x37, 0x07, 0x19, 0x29, 0x49, 0x89,
    0x16, 0x26, 0x46, 0x86, 0x13, 0x23, 0x43, 0x83,
    0x1c, 0x2c, 0x4c, 0x8c, 0x15, 0x25, 0x45, 0x85,
    0x1a, 0x2a, 0x4a, 0x8a, 0x0d, 0xcd, 0xce, 0x0e,
    0x70, 0x73, 0xb3, 0xb0, 0x51, 0x52, 0x54, 0x58,
    0xa1, 0xa2, 0xa4, 0xa8, 0x31, 0x32, 0x34, 0x38,
    0xc1, 0xc2, 0xc4, 0xc8, 0x61, 0x62, 0x64, 0x68,
    0x91, 0x92, 0x94, 0x98, 0xe0, 0xec, 0xdc, 0xd0,
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

/* Compute syndrome on the 9-byte encoded symbol (parity in byte[0]) */
static unsigned char fec_secded7264_compute_syndrome(const unsigned char *_v)
{
    unsigned int i;
    unsigned char syndrome = 0x00;

    for (i = 0; i < 8u; i++) {
        syndrome <<= 1;

        unsigned int p =
            ((_v[0] & (1u << (8u - i - 1u))) ? 1u : 0u) +
            liquid_c_ones[secded7264_P[8u*i+0u] & _v[1]] +
            liquid_c_ones[secded7264_P[8u*i+1u] & _v[2]] +
            liquid_c_ones[secded7264_P[8u*i+2u] & _v[3]] +
            liquid_c_ones[secded7264_P[8u*i+3u] & _v[4]] +
            liquid_c_ones[secded7264_P[8u*i+4u] & _v[5]] +
            liquid_c_ones[secded7264_P[8u*i+5u] & _v[6]] +
            liquid_c_ones[secded7264_P[8u*i+6u] & _v[7]] +
            liquid_c_ones[secded7264_P[8u*i+7u] & _v[8]];

        syndrome |= p & 0x01u;
    }

    return syndrome;
}

/*
 * Estimate error vector.
 * Returns 0 = no error, 1 = single-bit error (corrected), 2 = multiple errors.
 */
static int fec_secded7264_estimate_ehat(const unsigned char *_sym_enc,
                                        unsigned char *_e_hat)
{
    unsigned char s;
    unsigned int ws;
    unsigned int n;

    memset(_e_hat, 0x00, 9u * sizeof(unsigned char));

    s = fec_secded7264_compute_syndrome(_sym_enc);
    ws = liquid_c_ones[s];

    if (ws == 0u) {
        return 0;
    }

    for (n = 0u; n < 72u; n++) {
        if (s == secded7264_syndrome_w1[n]) {
            /* Single error at bit position n.
             * Byte index within the 9-byte symbol: 8 - (n / 8)
             * Bit position within that byte:        n % 8
             */
            unsigned int q = n >> 3;
            unsigned int r = n & 7u;
            _e_hat[8u - q] = (unsigned char)(1u << r);
            return 1;
        }
    }

    return 2;   /* no syndrome match - multiple errors */
}

/* Decode one 9-byte encoded symbol into 8 decoded bytes.
 * Returns 0/1/2 for no error / single error / multiple errors. */
static int fec_secded7264_decode_symbol(const unsigned char *_sym_enc,
                                        unsigned char *_sym_dec)
{
    unsigned char e_hat[9] = {0,0,0,0,0,0,0,0,0};
    int syndrome_flag = fec_secded7264_estimate_ehat(_sym_enc, e_hat);

    _sym_dec[0] = _sym_enc[1] ^ e_hat[1];
    _sym_dec[1] = _sym_enc[2] ^ e_hat[2];
    _sym_dec[2] = _sym_enc[3] ^ e_hat[3];
    _sym_dec[3] = _sym_enc[4] ^ e_hat[4];
    _sym_dec[4] = _sym_enc[5] ^ e_hat[5];
    _sym_dec[5] = _sym_enc[6] ^ e_hat[6];
    _sym_dec[6] = _sym_enc[7] ^ e_hat[7];
    _sym_dec[7] = _sym_enc[8] ^ e_hat[8];

    return syndrome_flag;
}

/*
 * fec_secded7264_decode - decode a block of SECDED 72/64 encoded data.
 *
 *  _enc_msg_len : number of encoded bytes
 *  _msg_enc     : encoded input
 *  _msg_dec     : decoded output
 *  _num_errors  : incremented for each uncorrectable symbol (may be NULL)
 *
 * Returns the number of decoded bytes written.
 */
unsigned int fec_secded7264_decode(unsigned int _enc_msg_len,
                                   const unsigned char *_msg_enc,
                                   unsigned char *_msg_dec,
                                   unsigned int *_num_errors)
{
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int num_errors = 0;
    unsigned int syndrome_flag;
    unsigned int r = _enc_msg_len % 9u;

    for (j = 0u; j < _enc_msg_len - r; j += 9u) {
        syndrome_flag = fec_secded7264_decode_symbol(&_msg_enc[j], &_msg_dec[i]);
        if (syndrome_flag == 2u) {
            num_errors++;
        }
        i += 8u;
    }

    /* Handle partial final symbol (fewer than 9 encoded bytes) */
    if (r != 0u) {
        unsigned char v[9] = {0,0,0,0,0,0,0,0,0};
        unsigned char c[8] = {0,0,0,0,0,0,0,0};
        unsigned int n;

        for (n = 0u; n < r; n++) {
            v[n] = _msg_enc[j + n];
        }

        syndrome_flag = fec_secded7264_decode_symbol(v, c);
        if (syndrome_flag == 2u) {
            num_errors++;
        }

        for (n = 0u; n < r - 1u; n++) {
            _msg_dec[i + n] = c[n];
        }

        i += r - 1u;
    }

    if (_num_errors != NULL) {
        *_num_errors = num_errors;
    }

    return i;
}
