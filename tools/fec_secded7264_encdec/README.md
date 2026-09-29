# SECDED 72/64 FEC encoder/decoder

Python implementation of the Hamming (72,64) SEC-DED (Single Error Correct,
Double Error Detect) block code used to protect the eNVM contents on
RTPFS-class PolarFire SoC boards.

## Usage — command line

    # Encode
    python3 fec_secded7264.py -e -i input.bin -o output.fec

    # Decode
    python3 fec_secded7264.py -d -i input.fec -o output.bin

## Usage — as a library

```python
from fec_secded7264 import encode, decode, encode_symbol, decode_symbol

enc = encode(data)              # bytes -> bytes
dec, num_errors = decode(enc)   # bytes -> (bytes, int)
```

## Tests

    python3 -m pytest test_fec_secded7264.py -v

The test suite includes round-trip, single-bit error correction, double-bit
error detection, and byte-for-byte cross-validation against the C reference
tool (when `build/tools/fec_secded7264_enc` is present).
