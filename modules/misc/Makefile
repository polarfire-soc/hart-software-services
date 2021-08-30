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
# Miscellaneous Functionality

EXTRA_SRCS-y += \
        modules/misc/assert.c \
        modules/misc/csr_helper.c \
        modules/misc/c_stubs.c \
        modules/misc/vsnprintf.c \
        modules/misc/hss_crc16.c \
        modules/misc/hss_crc32.c \
        modules/misc/hss_memcpy_via_pdma.c \
        modules/misc/hss_progress.c \
        modules/misc/device_serial_number.c \

#        modules/misc/ee_printf.c \

EXTRA_SRCS-$(CONFIG_TINYCLI) += \
        modules/misc/hss_tinycli.c \

EXTRA_SRCS-$(CONFIG_MEMTEST) += \
        modules/misc/hss_memtest.c \

EXTRA_SRCS-$(CONFIG_CC_STACKPROTECTOR_STRONG) += \
	 modules/misc/stack_guard.c

INCLUDES +=\

modules/misc/c_stubs.o: CFLAGS=$(CFLAGS_GCCEXT)
modules/misc/vsnprintf.o: CFLAGS=$(CFLAGS_GCCEXT)
modules/misc/assert.o: CFLAGS=$(CFLAGS_GCCEXT)
modules/misc/ee_printf.o: CFLAGS=$(CFLAGS_GCCEXT)
modules/misc/csr_helper.o: CFLAGS=$(CFLAGS_GCCEXT)
modules/misc/hss_memcpy_via_pdma.o: CFLAGS=$(CFLAGS_GCCEXT)
modules/misc/stack_guard.o: CFLAGS=$(CFLAGS_GCCEXT)
