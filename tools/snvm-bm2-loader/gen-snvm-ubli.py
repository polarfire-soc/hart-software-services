#!/usr/bin/env python3

"""
gen-snvm-ubli.py - build a UBLI (User Boot Loader Image) for BM2 sNVM boot

Wraps the flat sNVM BM2 loader binary in the 60-byte User Boot Loader Image
header the PolarFire SoC system controller reads when booting in Boot Mode 2
(user secure boot from sNVM), then writes it out as an Intel HEX at address 0
so fpgenprog snvm_client --mem_file_base_address 00000000 places the whole
UBLI at the sNVM start page with no offset applied.

Because the client is plain-text (non-authenticated), payload integrity is
provided by the UBLI's own SHA-256 (bytes 28-59), which the system controller
verifies directly from sNVM - no USK/SMK is required.

UBLI header layout (60 bytes):

    bytes  0- 3:  reset_vector (LE)        - all five harts jump here
    bytes  4- 7:  payload_size_words (LE)  - payload size in 32-bit words
    bytes  8-27:  reset_vector x5 (LE)    - one entry per hart (E51 + U54x4)
    bytes 28-59:  SHA-256 digest of payload
    bytes 60+:    binary payload (snvm-bm2-loader code, <= 8 KiB)

The reset vector is the loader ELF entry point (0x01000000, the E51 DTIM);
the SC copies the payload there and releases the harts.

Usage:
    gen-snvm-ubli.py <input.bin> <output-ubli.bin> <reset_vector_hex>

    <reset_vector_hex>  hex address without 0x prefix, e.g. 01000000

#
# MPFS HSS Embedded Software
#
# Copyright 2019-2026 Microchip FPGA Embedded Systems Solutions.
#
# SPDX-License-Identifier: MIT
#
"""

import hashlib
import struct
import sys


def build_ubli(payload, reset_vector):
    size_words = (len(payload) + 3) // 4
    digest = hashlib.sha256(payload).digest()

    header = struct.pack("<I", reset_vector)
    header += struct.pack("<I", size_words)
    header += struct.pack("<I", reset_vector) * 5
    header += digest
    assert len(header) == 60, f"UBLI header must be 60 bytes, got {len(header)}"
    return header + payload


def main():
    if len(sys.argv) != 4:
        sys.exit(f"usage: {sys.argv[0]} <input.bin> <output-ubli.bin> "
                 "<reset_vector_hex>")

    in_bin, out_bin, resetvec_hex = sys.argv[1], sys.argv[2], sys.argv[3]
    reset_vector = int(resetvec_hex, 16)

    with open(in_bin, "rb") as f:
        payload = f.read()

    ubli = build_ubli(payload, reset_vector)

    with open(out_bin, "wb") as f:
        f.write(ubli)

    print(f"gen-snvm-ubli: reset_vector=0x{reset_vector:08x} "
          f"payload={len(payload)} bytes -> {out_bin} ({len(ubli)} bytes)")


if __name__ == "__main__":
    main()
