#!/usr/bin/env python3

"""
fec_secded7264.py — Python implementation of the SECDED (72,64) FEC codec

SEC-DED (Single Error Correct, Double Error Detect) block code operating on
64-bit (8-byte) data symbols encoded to 72-bit (9-byte) codewords.

Based on the C implementation in fec_secded7264.c, which is in turn derived
from liquid-dsp by Joseph Gaeddert and adapted by Sultan Qasim Khan.

References:
    Lin, Shu and Costello, Daniel L. Jr., "Error Control Coding,"
    Prentice Hall, New Jersey, 2nd edition, 2004.

Usage as a command-line tool:
    python3 fec_secded7264.py -e -i input.bin -o encoded.bin   # encode
    python3 fec_secded7264.py -d -i encoded.bin -o output.bin  # decode
"""

#
# MPFS HSS Embedded Software
#
# Copyright 2019-2026 Microchip FPGA Embedded Systems Solutions.
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
import sys

#
# P matrix [8 rows * 8 columns of bytes].
# Row i, column k: secded7264_P[8*i + k]
# Used to compute parity and syndrome bits.
_P = (
    0xFF, 0x0F, 0x0F, 0x0C, 0x68, 0x88, 0x88, 0x80,
    0xF0, 0xFF, 0x00, 0xF3, 0x64, 0x44, 0x44, 0x40,
    0x30, 0xF0, 0xFF, 0x0F, 0x02, 0x22, 0x22, 0x26,
    0xCF, 0x00, 0xF0, 0xFF, 0x01, 0x11, 0x11, 0x16,
    0x68, 0x88, 0x88, 0x80, 0xFF, 0x0F, 0x00, 0xF3,
    0x64, 0x44, 0x44, 0x40, 0xF0, 0xFF, 0x0F, 0x0C,
    0x02, 0x22, 0x22, 0x26, 0xCF, 0x00, 0xFF, 0x0F,
    0x01, 0x11, 0x11, 0x16, 0x30, 0xF0, 0xF0, 0xFF,
)

# Syndrome vectors for single-bit errors (weight-1 error patterns), 72 entries.
# Entry n is the syndrome produced by a single bit error at position n of the
# 72-bit codeword (bit 0 = MSB of parity byte, bit 71 = LSB of data byte 8).
_SYNDROME_W1 = (
    0x0b, 0x3b, 0x37, 0x07, 0x19, 0x29, 0x49, 0x89,
    0x16, 0x26, 0x46, 0x86, 0x13, 0x23, 0x43, 0x83,
    0x1c, 0x2c, 0x4c, 0x8c, 0x15, 0x25, 0x45, 0x85,
    0x1a, 0x2a, 0x4a, 0x8a, 0x0d, 0xcd, 0xce, 0x0e,
    0x70, 0x73, 0xb3, 0xb0, 0x51, 0x52, 0x54, 0x58,
    0xa1, 0xa2, 0xa4, 0xa8, 0x31, 0x32, 0x34, 0x38,
    0xc1, 0xc2, 0xc4, 0xc8, 0x61, 0x62, 0x64, 0x68,
    0x91, 0x92, 0x94, 0x98, 0xe0, 0xec, 0xdc, 0xd0,
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
)

# Pre-compute a syndrome -> bit-position lookup table for O(1) decode.
_SYNDROME_TO_BIT: dict[int, int] = {s: n for n, s in enumerate(_SYNDROME_W1)}


#
# Internal helpers

def _popcount(x: int) -> int:
    """Return the number of set bits in byte x (0..255)."""
    return bin(x).count('1')


def _compute_parity(v: bytes) -> int:
    """Compute the 8-bit parity byte for an 8-byte data symbol.

    Each parity bit i is the XOR of selected data bits chosen by row i of
    the P matrix:  p_i = popcount(P[8*i+0] & v[0]) + ... + popcount(P[8*i+7] & v[7]) mod 2
    The parity byte is assembled MSB-first (i=0 -> bit 7).
    """
    parity = 0
    for i in range(8):
        p = 0
        row = 8 * i
        for k in range(8):
            p += _popcount(_P[row + k] & v[k])
        parity = (parity << 1) | (p & 1)
    return parity


def _compute_syndrome(sym_enc: bytes) -> int:
    """Compute the 8-bit syndrome for a 9-byte encoded symbol.

    sym_enc[0] is the parity byte; sym_enc[1..8] are the 8 data bytes.
    Syndrome bit i = (received_parity_bit_i) XOR (recomputed_parity_bit_i).
    """
    syndrome = 0
    for i in range(8):
        # Received parity bit i (MSB-first from sym_enc[0])
        p = (sym_enc[0] >> (7 - i)) & 1
        # Recomputed parity contribution from data bytes sym_enc[1..8]
        row = 8 * i
        for k in range(8):
            p += _popcount(_P[row + k] & sym_enc[k + 1])
        syndrome = (syndrome << 1) | (p & 1)
    return syndrome


def _estimate_ehat(sym_enc: bytes) -> tuple[int, list[int]]:
    """Estimate the 72-bit error vector for a 9-byte encoded symbol.

    Returns (syndrome_flag, e_hat) where:
      syndrome_flag = 0 — no error detected
      syndrome_flag = 1 — single error detected and corrected in e_hat
      syndrome_flag = 2 — uncorrectable (double or higher) error
    e_hat is a list of 9 integers (byte masks).
    """
    e_hat = [0] * 9
    s = _compute_syndrome(sym_enc)

    if s == 0:
        return 0, e_hat

    bit_pos = _SYNDROME_TO_BIT.get(s)
    if bit_pos is not None:
        # Single error at codeword bit position bit_pos.
        # The 72 codeword bits are laid out as: byte[8] byte[7] ... byte[0],
        # MSB-first within each byte (matching the C div(n,8) indexing):
        #   byte_index = 9 - bit_pos//8 - 1
        #   bit_mask   = 1 << (bit_pos % 8)
        byte_idx = 9 - bit_pos // 8 - 1
        e_hat[byte_idx] = 1 << (bit_pos % 8)
        return 1, e_hat

    # No weight-1 match — multiple errors
    return 2, e_hat


#
# Symbol-level encode / decode

def encode_symbol(data8: bytes) -> bytes:
    """Encode an 8-byte data symbol to a 9-byte codeword.

    Returns bytes: [parity, data[0], ..., data[7]]
    """
    if len(data8) != 8:
        raise ValueError(f'encode_symbol expects 8 bytes, got {len(data8)}')
    parity = _compute_parity(data8)
    return bytes([parity]) + bytes(data8)


def decode_symbol(sym_enc: bytes) -> tuple[int, bytes]:
    """Decode a 9-byte codeword to an 8-byte data symbol.

    Returns (syndrome_flag, data8):
      syndrome_flag = 0 — no error
      syndrome_flag = 1 — single-bit error corrected
      syndrome_flag = 2 — uncorrectable error (output may be wrong)
    """
    if len(sym_enc) != 9:
        raise ValueError(f'decode_symbol expects 9 bytes, got {len(sym_enc)}')
    flag, e_hat = _estimate_ehat(sym_enc)
    data8 = bytes(sym_enc[k + 1] ^ e_hat[k + 1] for k in range(8))
    return flag, data8


#
# Block-level encode / decode

def encode(data: bytes) -> bytes:
    """Encode arbitrary bytes with SECDED 72/64.

    Each full 8-byte block encodes to 9 bytes.  A partial final block of
    r (1..7) bytes encodes to r+1 bytes: [parity, data[0], ..., data[r-1]].
    """
    n = len(data)
    full, r = divmod(n, 8)
    out = bytearray()

    for i in range(full):
        out += encode_symbol(data[8 * i: 8 * i + 8])

    if r:
        # Pad to 8 bytes, compute parity, then emit only parity + the r real bytes.
        v = data[8 * full:] + bytes(8 - r)
        parity = _compute_parity(v)
        out += bytes([parity]) + data[8 * full:]

    return bytes(out)


def decode(enc_data: bytes) -> tuple[bytes, int]:
    """Decode SECDED 72/64 encoded bytes.

    Returns (decoded_data, num_uncorrectable_errors).

    num_uncorrectable_errors > 0 means at least one double-bit error was
    detected; the corresponding output bytes are unreliable.
    """
    n = len(enc_data)
    r = n % 9
    full_chunks = (n - r) // 9
    out = bytearray()
    num_errors = 0

    for i in range(full_chunks):
        flag, dec8 = decode_symbol(enc_data[9 * i: 9 * i + 9])
        if flag == 2:
            num_errors += 1
        out += dec8

    if r:
        # Partial final symbol: pad to 9 bytes, decode, keep first r-1 bytes.
        v = bytes(enc_data[9 * full_chunks:]) + bytes(9 - r)
        flag, dec8 = decode_symbol(v)
        if flag == 2:
            num_errors += 1
        out += dec8[:r - 1]   # r-1 == original data bytes in this partial block

    return bytes(out), num_errors


#
# Command-line interface

def _parse_args(argv=None):
    parser = argparse.ArgumentParser(
        description='SECDED 72/64 FEC encoder/decoder')
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument('-e', '--encode', action='store_true',
                      help='Encode input file (default)')
    mode.add_argument('-d', '--decode', action='store_true',
                      help='Decode input file')
    parser.add_argument('-i', '--input', required=True,
                        metavar='FILE', help='Input binary file')
    parser.add_argument('-o', '--output', required=True,
                        metavar='FILE', help='Output binary file')
    parser.add_argument('-v', '--verbose', action='store_true')
    return parser.parse_args(argv)


def main(argv=None):
    args = _parse_args(argv)

    with open(args.input, 'rb') as f:
        data = f.read()

    if args.decode:
        result, num_errors = decode(data)
        if args.verbose:
            print(f'Decoded {len(data)} -> {len(result)} bytes '
                  f'({num_errors} uncorrectable errors)', file=sys.stderr)
        if num_errors:
            print(f'WARNING: {num_errors} uncorrectable error(s) detected',
                  file=sys.stderr)
        with open(args.output, 'wb') as f:
            f.write(result)
    else:
        result = encode(data)
        if args.verbose:
            print(f'Encoded {len(data)} -> {len(result)} bytes', file=sys.stderr)
        with open(args.output, 'wb') as f:
            f.write(result)


if __name__ == '__main__':
    main()
