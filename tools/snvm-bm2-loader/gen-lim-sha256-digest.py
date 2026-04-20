#!/usr/bin/env python3

"""
gen-lim-sha256-digest.py — generate lim_sha256_digest.h for the sNVM BM2 loader

Reads a flat binary (l2lim-wrapper.bin), computes its SHA-256 digest and the
SECDED 72/64 FEC-encoded byte count, then writes a C header containing:

    static const uint8_t  lim_expected_sha256[32];  -- SHA-256 of the decoded image
    static const size_t   LIM_DECODED_SIZE;          -- byte count of input binary
    static const size_t   LIM_FEC_ENCODED_SIZE;      -- byte count after FEC encoding

The header is consumed by snvm-bm2-loader.c at compile time so the loader can
validate the L2LIM image after FEC-decoding it from eNVM at run time.
"""

#
# MPFS HSS Embedded Software
#
# Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
#
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#

import argparse
import hashlib
import os
import sys


def fec_secded7264_encoded_size(decoded_size: int) -> int:
    """Return the byte count produced by fec_secded7264_encode() for decoded_size bytes.

    SECDED 72/64 encodes 8 data bytes into 9 bytes (1 parity + 8 data).
    A partial final block of r data bytes encodes to r+1 bytes.
    This matches the formula in fec_secded7264.c exactly.
    """
    full_symbols, remainder = divmod(decoded_size, 8)
    return full_symbols * 9 + (remainder + 1 if remainder else 0)


def make_header(data: bytes) -> str:
    """Return the content of lim_sha256_digest.h as a string."""
    digest = hashlib.sha256(data).digest()
    decoded_size = len(data)
    encoded_size = fec_secded7264_encoded_size(decoded_size)

    hex_bytes = ', '.join(hex(b) for b in digest)

    lines = [
        '#ifndef LIM_SHA256_DIGEST_H_',
        '#define LIM_SHA256_DIGEST_H_',
        '#include <stdint.h>',
        '#include <stddef.h>',
        f'static const uint8_t lim_expected_sha256[32] = {{{hex_bytes}}};',
        f'static const size_t LIM_DECODED_SIZE = {decoded_size}u;',
        f'static const size_t LIM_FEC_ENCODED_SIZE = {encoded_size}u;',
        '#endif /* LIM_SHA256_DIGEST_H_ */',
    ]
    return '\n'.join(lines) + '\n'


def main():
    parser = argparse.ArgumentParser(
        description='Generate lim_sha256_digest.h for the sNVM BM2 loader')
    parser.add_argument('--verbose', '-v', action='store_true')
    parser.add_argument('imageFileIn',
                        help='Input binary (l2lim-wrapper.bin)')
    parser.add_argument('headerFileOut',
                        help='Output C header (lim_sha256_digest.h)')
    args = parser.parse_args()

    with open(args.imageFileIn, 'rb') as f:
        data = f.read()

    if args.verbose:
        print(f'Read {len(data)} bytes from {args.imageFileIn}',
              file=sys.stderr)

    header = make_header(data)

    os.makedirs(os.path.dirname(os.path.abspath(args.headerFileOut)),
                exist_ok=True)
    with open(args.headerFileOut, 'w') as f:
        f.write(header)

    if args.verbose:
        digest_hex = hashlib.sha256(data).hexdigest()
        enc_sz = fec_secded7264_encoded_size(len(data))
        print(f'SHA-256:           {digest_hex}', file=sys.stderr)
        print(f'LIM_DECODED_SIZE:  {len(data)}', file=sys.stderr)
        print(f'LIM_FEC_ENC_SIZE:  {enc_sz}', file=sys.stderr)
        print(f'Written {args.headerFileOut}', file=sys.stderr)


if __name__ == '__main__':
    main()
