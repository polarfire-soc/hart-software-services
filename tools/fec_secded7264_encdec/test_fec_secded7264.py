#!/usr/bin/env python3

"""
test_fec_secded7264.py — Unit tests for the Python SECDED 72/64 FEC codec.

Tests exercise:
  - Round-trip encode -> decode for various payload sizes
  - Single-bit error correction
  - Double-bit error detection
  - Cross-validation byte-for-byte against the C reference tool
    (build/tools/fec_secded7264_enc).

Run:
    python3 -m pytest test_fec_secded7264.py -v
  or:
    python3 test_fec_secded7264.py
"""

#
# MPFS HSS Embedded Software
#
# Copyright 2019-2026 Microchip FPGA Embedded Systems Solutions.
#
# SPDX-License-Identifier: MIT
#

import os
import pathlib
import random
import subprocess
import sys
import tempfile
import unittest

# Allow running from any working directory
_HERE = pathlib.Path(__file__).parent
sys.path.insert(0, str(_HERE))

from fec_secded7264 import (  # noqa: E402
    decode,
    decode_symbol,
    encode,
    encode_symbol,
)

# Path to the C reference binary, relative to the repo root.
# The repo root is two levels above tools/fec_secded7264_encdec/.
_REPO_ROOT = _HERE.parent.parent
_C_ENC_BINARY = _REPO_ROOT / 'build' / 'tools' / 'fec_secded7264_enc'
_C_TOOL_AVAILABLE = _C_ENC_BINARY.exists() and os.access(_C_ENC_BINARY, os.X_OK)


def _run_c_tool(in_data: bytes, decode_mode: bool = False) -> bytes:
    """Run the C FEC tool on in_data and return the output bytes."""
    with tempfile.NamedTemporaryFile(delete=False, suffix='.bin') as infile:
        infile.write(in_data)
        in_path = infile.name

    out_path = in_path + '.out'
    try:
        cmd = [str(_C_ENC_BINARY), '-i', in_path, '-o', out_path]
        if decode_mode:
            cmd.append('-d')
        subprocess.run(cmd, check=True, capture_output=True)
        with open(out_path, 'rb') as f:
            return f.read()
    finally:
        os.unlink(in_path)
        if os.path.exists(out_path):
            os.unlink(out_path)


# ---------------------------------------------------------------------------
# Helpers

def _flip_bit(data: bytearray, byte_idx: int, bit: int) -> bytearray:
    """Return a copy of data with bit flipped at data[byte_idx]."""
    result = bytearray(data)
    result[byte_idx] ^= (1 << bit)
    return result


# ---------------------------------------------------------------------------
# Symbol-level tests

class TestEncodeSymbol(unittest.TestCase):

    def test_output_length(self):
        enc = encode_symbol(bytes(8))
        self.assertEqual(len(enc), 9)

    def test_known_zero_input(self):
        """All-zero data should produce all-zero parity (XOR of zeros)."""
        enc = encode_symbol(bytes(8))
        # parity of all-zero data is 0
        self.assertEqual(enc[0], 0x00)
        self.assertEqual(enc[1:], bytes(8))

    def test_data_passthrough(self):
        """Data bytes should appear unchanged in enc[1..8]."""
        data = bytes(range(8))
        enc = encode_symbol(data)
        self.assertEqual(enc[1:], data)

    def test_parity_changes_on_flip(self):
        """Flipping a data bit should change the parity byte."""
        data = bytes(range(8))
        enc0 = encode_symbol(data)
        data_flipped = bytearray(data)
        data_flipped[0] ^= 0x01
        enc1 = encode_symbol(bytes(data_flipped))
        self.assertNotEqual(enc0[0], enc1[0])


class TestDecodeSymbol(unittest.TestCase):

    def test_roundtrip(self):
        data = bytes(range(8))
        enc = encode_symbol(data)
        flag, dec = decode_symbol(enc)
        self.assertEqual(flag, 0)
        self.assertEqual(dec, data)

    def test_no_error_flag(self):
        data = bytes([0xFF] * 8)
        enc = encode_symbol(data)
        flag, _ = decode_symbol(enc)
        self.assertEqual(flag, 0)

    def test_single_bit_error_in_data(self):
        """A single flipped bit in a data byte should be corrected (flag=1)."""
        data = bytes(range(8))
        enc = bytearray(encode_symbol(data))
        enc[3] ^= 0x04  # flip one bit in data byte 2 (enc[3])
        flag, dec = decode_symbol(bytes(enc))
        self.assertEqual(flag, 1)
        self.assertEqual(dec, data)

    def test_single_bit_error_in_parity(self):
        """A single flipped bit in the parity byte should be detected (flag=1)
        and the data output should still be correct."""
        data = bytes(range(8))
        enc = bytearray(encode_symbol(data))
        enc[0] ^= 0x80  # flip parity MSB
        flag, dec = decode_symbol(bytes(enc))
        self.assertEqual(flag, 1)
        self.assertEqual(dec, data)

    def test_double_bit_error_detected(self):
        """Two flipped bits should be detected as uncorrectable (flag=2)."""
        data = bytes(range(8))
        enc = bytearray(encode_symbol(data))
        enc[1] ^= 0x01
        enc[2] ^= 0x01
        flag, _ = decode_symbol(bytes(enc))
        self.assertEqual(flag, 2)

    def test_all_single_bit_errors_corrected(self):
        """Every single-bit position in the 72-bit codeword should be
        correctable."""
        data = bytes([0xA5, 0x5A, 0xFF, 0x00, 0x12, 0x34, 0x56, 0x78])
        enc = encode_symbol(data)
        for byte_idx in range(9):
            for bit in range(8):
                corrupted = bytearray(enc)
                corrupted[byte_idx] ^= (1 << bit)
                flag, dec = decode_symbol(bytes(corrupted))
                self.assertEqual(flag, 1,
                    f'Expected single-error flag for byte={byte_idx} bit={bit}')
                self.assertEqual(dec, data,
                    f'Wrong correction at byte={byte_idx} bit={bit}')


# ---------------------------------------------------------------------------
# Block-level tests

class TestEncodeDecode(unittest.TestCase):

    def _roundtrip(self, n: int, seed: int = 42):
        rng = random.Random(seed)
        data = bytes([rng.randint(0, 255) for _ in range(n)])
        enc = encode(data)
        dec, num_errors = decode(enc)
        self.assertEqual(num_errors, 0)
        self.assertEqual(dec, data, f'Mismatch for size={n}')
        return data, enc

    def test_sizes(self):
        """Round-trip across a range of sizes including partial blocks."""
        for n in [0, 1, 7, 8, 9, 14, 15, 16, 17, 63, 64, 65, 100, 255, 256, 1000]:
            with self.subTest(n=n):
                self._roundtrip(n)

    def test_empty(self):
        self.assertEqual(encode(b''), b'')
        dec, errs = decode(b'')
        self.assertEqual(dec, b'')
        self.assertEqual(errs, 0)

    def test_encoded_length_full_blocks(self):
        """16 data bytes -> 18 encoded bytes (2 × 9)."""
        enc = encode(bytes(16))
        self.assertEqual(len(enc), 18)

    def test_encoded_length_partial_block(self):
        """3 data bytes -> 4 encoded bytes (parity + 3 data)."""
        enc = encode(bytes(3))
        self.assertEqual(len(enc), 4)

    def test_encoded_length_formula(self):
        """Verify the encoded length matches the formula for all residues."""
        for base in range(0, 64, 8):
            for r in range(0, 8):
                n = base + r
                data = bytes(n)
                enc = encode(data)
                expected = (n // 8) * 9 + (r + 1 if r else 0)
                self.assertEqual(len(enc), expected, f'len mismatch for n={n}')

    def test_single_bit_errors_in_stream(self):
        """Flip every bit in the encoded stream, decode should still succeed."""
        rng = random.Random(0xDEAD)
        data = bytes([rng.randint(0, 255) for _ in range(32)])
        enc = bytearray(encode(data))
        for byte_idx in range(len(enc)):
            for bit in range(8):
                corrupted = bytearray(enc)
                corrupted[byte_idx] ^= (1 << bit)
                dec, num_errors = decode(bytes(corrupted))
                self.assertEqual(num_errors, 0,
                    f'Expected 0 uncorrectable errors for byte={byte_idx} bit={bit}')
                self.assertEqual(dec, data,
                    f'Wrong decode for byte={byte_idx} bit={bit}')

    def test_double_bit_errors_detected(self):
        """Two flipped bits within the same 9-byte symbol are detected."""
        data = bytes(range(8))  # exactly one symbol
        enc = bytearray(encode(data))
        enc[0] ^= 0x01
        enc[1] ^= 0x01
        _, num_errors = decode(bytes(enc))
        self.assertGreater(num_errors, 0)


# ---------------------------------------------------------------------------
# Cross-validation against the C reference tool

@unittest.skipUnless(_C_TOOL_AVAILABLE,
                     f'C tool not found at {_C_ENC_BINARY} — build first with make')
class TestCrossValidation(unittest.TestCase):

    def _cross_encode(self, n: int, seed: int = 99):
        """Python-encoded output must match C-encoded output byte for byte."""
        rng = random.Random(seed)
        data = bytes([rng.randint(0, 255) for _ in range(n)])
        py_enc = encode(data)
        c_enc = _run_c_tool(data, decode_mode=False)
        self.assertEqual(py_enc, c_enc,
            f'Encode mismatch for n={n}: '
            f'Python={py_enc.hex()} C={c_enc.hex()}')
        return data, py_enc

    def _cross_decode(self, n: int, seed: int = 99):
        """Python-decoded output must match C-decoded output byte for byte."""
        data, enc = self._cross_encode(n, seed)
        py_dec, _ = decode(enc)
        c_dec = _run_c_tool(enc, decode_mode=True)
        self.assertEqual(py_dec, c_dec,
            f'Decode mismatch for n={n}: '
            f'Python={py_dec.hex()} C={c_dec.hex()}')
        self.assertEqual(py_dec, data)

    def test_cross_encode_full_block(self):
        self._cross_encode(8)

    def test_cross_encode_partial_block_1(self):
        self._cross_encode(1)

    def test_cross_encode_partial_block_7(self):
        self._cross_encode(7)

    def test_cross_encode_multi_block(self):
        self._cross_encode(100)

    def test_cross_encode_large(self):
        self._cross_encode(1024)

    def test_cross_decode_full_block(self):
        self._cross_decode(8)

    def test_cross_decode_partial_1(self):
        self._cross_decode(1)

    def test_cross_decode_partial_7(self):
        self._cross_decode(7)

    def test_cross_decode_multi(self):
        self._cross_decode(100)

    def test_cross_decode_large(self):
        self._cross_decode(1024)

    def test_cross_all_sizes_0_to_63(self):
        """Every size from 0 to 63 encodes and decodes identically to C."""
        rng = random.Random(0xBEEF)
        for n in range(64):
            data = bytes([rng.randint(0, 255) for _ in range(n)])
            py_enc = encode(data)
            c_enc = _run_c_tool(data, decode_mode=False)
            with self.subTest(n=n, phase='encode'):
                self.assertEqual(py_enc, c_enc)
            if n > 0:
                py_dec, _ = decode(py_enc)
                c_dec = _run_c_tool(py_enc, decode_mode=True)
                with self.subTest(n=n, phase='decode'):
                    self.assertEqual(py_dec, c_dec)
                    self.assertEqual(py_dec, data)


# ---------------------------------------------------------------------------

if __name__ == '__main__':
    unittest.main(verbosity=2)
