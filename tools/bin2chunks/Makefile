#
# MPFS HSS Embedded Software
#
# Copyright 2019-2021 Microchip Corporation.
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
CORE_CFLAGS+=-Wall -Werror -Wshadow -fno-builtin-printf \
   -fomit-frame-pointer -Wredundant-decls -Wall -Wundef -Wwrite-strings -fno-strict-aliasing \
   -fno-common -Wendif-labels -Wmissing-include-dirs -Wempty-body -Wformat=2 -Wformat-security \
   -Wformat-y2k -Winit-self -Wignored-qualifiers -Wold-style-declaration -Wold-style-definition \
   -Wtype-limits -Wstrict-prototypes

#  -Wimplicit-fallthrough=5 # not supported in GCC 5.4 (Ubuntu 16.04)

CFLAGS=--std=gnu11 -O1 -g3 $(CORE_CFLAGS) -DNR_CPUs=4 -D__riscv_xlen=64 -D_FORTIFY_SOURCE=2 -mno-ms-bitfields

all: bin2chunks dump_header

-include ../../.config

INCLUDES=\
	-I. \
	-I../../include \
	-I../../modules/ssmb/ipi/ \
	-I../../ \
	-I../../thirdparty/opensbi/include \
	

bin2chunks: bin2chunks.c
	$(CC) $(CFLAGS) $(INCLUDES) -o bin2chunks bin2chunks.c

dump_header: dump_header.c
	$(CC) $(CFLAGS) $(INCLUDES) -o dump_header dump_header.c

clean:
	-$(RM) bin2chunks dump_header
