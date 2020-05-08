# 
# MPFS HSS Embedded Software
#
# Copyright 2019 Microchip Corporation.
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
# Toplevel HSS Makefile
#

CC=gcc
CFLAGS=-Wall --std=gnu11 -O2 -g -DNR_CPUs=4

CWD=$(shell pwd)

all: bin2chunks dump_header

-include ../../.config

bin2chunks: bin2chunks.c ../../misc/hss_crc32.c
	$(CC) $(CFLAGS) -o bin2chunks bin2chunks.c ../../misc/hss_crc32.c -I../../include -I../../ssmb/ipi/ -I../../ -I../../thirdparty/opensbi/include -D__riscv_xlen=64

dump_header: dump_header.c 
	$(CC) $(CFLAGS) -o dump_header dump_header.c -I../../include -I../../ssmb/ipi/ -I../..  -I../../thirdparty/opensbi/include -D__riscv_xlen=64
clean:
	-$(RM) bin2chunks dump_header
