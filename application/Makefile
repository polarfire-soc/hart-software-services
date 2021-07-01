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
#

all: $(TARGETS) config.h
include .config

SRCS-y += \
    application/hart0/hss_state_machine.c \
    application/hart0/hss_clock.c \
    application/hart0/hss_registry.c \

INCLUDES +=\
    -I./include \
    -I.

ASM_SRCS += \
    application/crt.S \


EXTRA_SRCS-y += \
    application/hart0/hss_init.c \
    application/hart0/hss_main.c

application/hart0/hss_main.o: CFLAGS=$(CFLAGS_GCCEXT)
application/hart0/hss_init.o: CFLAGS=$(CFLAGS_GCCEXT)
application/hart0/hss_init.o: include/tool_versions.h
application/hart0/hss_state_machine.o: CFLAGS=$(CFLAGS_GCCEXT)
tools/bin2chunks/bootImage.o: CFLAGS=$(CFLAGS_GCCEXT)

#
# U54 Functionality

EXTRA_SRCS-y += \
        application/hart1-4/u54_handle_ipi.c \

application/hart1-4/u54_handle_ipi.o: CFLAGS=$(CFLAGS_GCCEXT)
