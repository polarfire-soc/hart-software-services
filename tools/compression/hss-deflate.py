#!/usr/bin/env python3

"""
MPFS HSS Deflate tool

This script takes an HSS-L2LIM image and compresses it for storage in eNVM
using Deflate

"""

#
#
# MPFS HSS Deflate tool
#
# Copyright 2021-2022 Microchip Corporation.
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
#
#

import argparse
import sys
try:
    import zlib
except:
    print("Unable to import the zlib package, please check your Python installation")
    print("If you are attempting to build the HSS using SoftConsole on Linux there is a known issue when importing the zlib module")
    print("See: https://github.com/orgs/polarfire-soc/discussions/276 for more information")
    sys.exit(1)


def deflate(data: bytes, compression_level: int = 9) -> bytes:
    '''Takes bytes and a compression level, returns compressed bytes'''
    compressor = zlib.compressobj(level=compression_level,
                                  method=zlib.DEFLATED,
                                  wbits=zlib.MAX_WBITS,
                                  memLevel=zlib.DEF_MEM_LEVEL,
                                  strategy=0)
    deflated_bytes = compressor.compress(data) + compressor.flush()
    return deflated_bytes


def inflate(data: bytes) -> bytes:
    '''Takes compressed bytes and a compression level,
    returns decompressed bytes'''
    decompressor = zlib.decompressobj(-zlib.MAX_WBITS)
    inflated_bytes = decompressor.decompress(data) + decompressor.flush()
    return inflated_bytes


def get_script_version() -> str:
    '''returns script version'''
    return "1.0.1"


def get_header_bytes(compressed_crc: int, original_crc: int,
                     compressed_image_len: int,
                     original_image_len: int) -> bytearray:
    '''takes compressed and original CRCs and lengths,
    returns the payload header'''
    #
    # This function must output a header that is in-sync with
    # include/hss-types.h ...
    #
    # #pragma pack(8)
    # struct HSS_CompressedImage {
    #     uint32_t magic;
    #     uint32_t version;
    #     size_t headerLength;
    #     uint32_t headerCrc;
    #     uint32_t compressedCrc;
    #     uint32_t originalCrc;
    #     size_t compressedImageLen;
    #     size_t originalImageLen;
    #     uint8_t hash[32];
    #     uint8_t ecdsaSig[32];
    # };
    # #define mHSS_COMPRESSED_MAGIC    (0xC08B8355u)
    # #define mHSS_COMPRESSED_VERSION_FASTLZ 1u
    # #define mHSS_COMPRESSED_VERSION_MINIZ  2u
    #
    # offsetof(magic):               0
    # offsetof(version):             4
    # offsetof(headerLength):        8
    # offsetof(headerCrc):           16
    # offsetof(compressedCrc):       20
    # offsetof(originalCrc):         24
    # offsetof(compressedImageLen):  32
    # offset_of(originalImageLen):   40
    # offsetof(hash):                48
    # offsetof(ecdsaSig):            80

    header = bytearray(112)
    header[0:3] = 0xC08B8355.to_bytes(4, "little")  # magic
    header[4:8] = 0x2.to_bytes(4, "little")         # version
    header[8:16] = 0x0.to_bytes(8, "little")        # headerLength placeholder
    header[16:20] = 0x0.to_bytes(4, "little")       # headerCrc placeholder
    header[20:24] = compressed_crc.to_bytes(4, "little")
    header[24:32] = original_crc.to_bytes(4, "little")
    header[32:40] = compressed_image_len.to_bytes(8, "little")
    header[40:48] = original_image_len.to_bytes(8, "little")

    header[48:80] = bytearray(32)                   # hash32 placeholder
    header[80:112] = bytearray(32)                  # ecdsaSig32 placeholder

    header_length = len(header)
    header[8:12] = header_length.to_bytes(4, "little")
    header[12:16] = 0x0.to_bytes(4, "little")       # padding

    header_crc = zlib.crc32(header)
    header[16:20] = header_crc.to_bytes(4, "little")

    global args
    if args.verbose:
        print("Header length is %d (0x%08X)" % (header_length, header_length))
        print("Header CRC is 0x%08X" % (header_crc))

    return header


def main():
    '''main function'''
    parser = argparse.ArgumentParser(description='Deflate HSS-L2LIM image')
    parser.add_argument('--verbose', '-v', action='count', default=0)
    parser.add_argument('imageFileIn')
    parser.add_argument('deflatedFileOut')

    global args
    args = parser.parse_args()

    if args.verbose:
        print("Reading image from " + args.imageFileIn)
    with open(args.imageFileIn, 'rb') as file_in:
        image_data = file_in.read()

    image_len = len(image_data)
    if args.verbose:
        print("Reading %d (0x%08X) bytes" % (image_len, image_len))

    image_crc = zlib.crc32(image_data)

    if args.verbose:
        print("Image CRC is 0x%08X" % (image_crc))
        print("Deflating...")

    deflated_data = deflate(image_data)
    deflated_len = len(deflated_data)

    deflated_crc = zlib.crc32(deflated_data)

    if args.verbose:
        print("Deflated CRC is 0x%08X" % (deflated_crc))
        print("Deflated length %d (0x%08X) bytes"
              % (deflated_len, deflated_len))
        print("Outputting to " + args.deflatedFileOut)

    header_data = get_header_bytes(compressed_crc=deflated_crc,
                                   original_crc=image_crc,
                                   compressed_image_len=deflated_len,
                                   original_image_len=image_len)
    with open(args.deflatedFileOut, "wb") as file_out:
        output_size = file_out.write(header_data)
        output_size += file_out.write(deflated_data)
    if args.verbose:
        print("Wrote %d (0x%08X) bytes" % (output_size, output_size))

#
#
#


if __name__ == "__main__":
    main()
