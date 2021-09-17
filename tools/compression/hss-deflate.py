#!/usr/bin/env python3

#==============================================================================
#
# MPFS HSS Deflate Stage
#
# Copyright 2021 Microchip Corporation.
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
# This script takes an HSS-L2LIM image and compresses it for storage in eNVM
# using Deflate
#
#==============================================================================

import argparse
import zlib
import sys

import base64

def deflate(data, compressionLevel=4):
	compressor = zlib.compressobj(level=compressionLevel, method=zlib.DEFLATED,
                                     wbits=zlib.MAX_WBITS, memLevel=zlib.DEF_MEM_LEVEL,
                                     strategy=0)
	deflatedBytes = compressor.compress(data) + compressor.flush()
	return deflatedBytes

def inflate(data):
	decompressor = zlib.decompressobj(-zlib.MAX_WBITS)
	inflatedBytes = decompressor.decompress(data) + decompressor.flush()
	return inflatedBytes

def get_script_version():
	return "1.0.0"

def get_header_bytes(compressedCrc, originalCrc, compressedImageLen,
		originalImageLen):
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
	# #define mHSS_COMPRESSED_MAGIC	(0xC08B8355u)
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
	header[0:3] = 0xC08B8355.to_bytes(4, "little")			# magic
	header[4:8] = 0x2.to_bytes(4, "little")				# version
	header[8:16] = 0x0.to_bytes(8, "little")			# headerLength placeholder
	header[16:20]= 0x0.to_bytes(4, "little")			# headerCrc placeholder
	header[20:24] = compressedCrc.to_bytes(4, "little") 		# compressedCrc
	header[24:32] = originalCrc.to_bytes(4, "little")		# originalCrc
	header[32:40] = compressedImageLen.to_bytes(8, "little")	# compressedImageLen
	header[40:48] = originalImageLen.to_bytes(8, "little")		# originalImageLen

	header[48:80] = bytearray(32)					# hash32 placeholder
	header[80:112] = bytearray(32)					# ecdsaSig32 placeholder

	headerLength = len(header)
	header[8:12] = headerLength.to_bytes(4, "little")		# headerLength
	header[12:16] = 0x0.to_bytes(4, "little")			# padding

	headerCrc = zlib.crc32(header)
	header[16:20] = headerCrc.to_bytes(4, "little")			# headerCrc

	if (args.verbose):
		print("Header length is %d (0x%08X)" %(headerLength, headerLength) )
		print("Header CRC is 0x%08X" %(headerCrc) )

	return header

def main():
	parser = argparse.ArgumentParser(description = 'Deflate HSS-L2LIM image')
	parser.add_argument('--verbose', '-v', action='count', default=0)
	parser.add_argument('imageFileIn')
	parser.add_argument('deflatedFileOut')
	global args
	args = parser.parse_args()

	if (args.verbose):
		print("Reading image from " + args.imageFileIn)
	with open(args.imageFileIn, 'rb') as fileIn:
		imageData = fileIn.read()

	imageLen = len(imageData)
	if (args.verbose):
		print("Reading %d (0x%08X) bytes" %(imageLen, imageLen) )

	imageCrc = zlib.crc32(imageData)

	if (args.verbose):
		print("Image CRC is 0x%08X" %(imageCrc) )
		print("Deflating...")

	deflatedData = deflate(imageData)
	deflatedLen = len(deflatedData)

	deflatedCrc = zlib.crc32(deflatedData)

	if (args.verbose):
		print("Deflated CRC is 0x%08X" %(deflatedCrc) )
		print("Deflated length %d (0x%08X) bytes" %(deflatedLen, deflatedLen) )
		print("Outputting to " + args.deflatedFileOut)

	headerData = get_header_bytes(compressedCrc=deflatedCrc, originalCrc=imageCrc,
		compressedImageLen=deflatedLen, originalImageLen=imageLen)
	with open(args.deflatedFileOut, "wb") as fileOut:
		outputSize = fileOut.write(headerData)
		outputSize += fileOut.write(deflatedData)
	if (args.verbose):
		print("Wrote %d (0x%08X) bytes" %(outputSize, outputSize) )
#
#
#

if __name__ == "__main__":
	main()
