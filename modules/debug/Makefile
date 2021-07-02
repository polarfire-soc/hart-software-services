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
# Optional profiling hooks
#

EXTRA_SRCS-y += \
        modules/debug/hss_debug.c \
	modules/debug/hss_perfctr.c \

EXTRA_SRCS-$(CONFIG_DEBUG_PROFILING_SUPPORT) += \
        modules/debug/profiling.c \

OPT-$(CONFIG_DEBUG_PROFILING_SUPPORT) += \
	-finstrument-functions \
	-finstrument-functions-exclude-function-list=g5soc_init_mutex,ee_printf,CSR_GetHartId,CSR_GetTime,ee_vsprintf,uart_send_string_mmuart_ex,g5soc_release_mutex,take_loop,CSR_GetTickCount,ee_puts

INCLUDES +=\
	-Imodules/debug/ \

modules/debug/profiling.o: CFLAGS=$(CFLAGS_GCCEXT)
