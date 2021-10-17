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
# Initialization Routines

EXTRA_SRCS-y += \
	init/hss_ddr_init.c \
	init/hss_boot_init.c \
	init/hss_sys_setup.c \

EXTRA_SRCS-$(CONFIG_USE_IHC) += \
	init/hss_ihc_init.c

EXTRA_SRCS-$(CONFIG_USE_PCIE) += \
	init/hss_pcie_init.c

EXTRA_SRCS-$(CONFIG_USE_PDMA) += \
	init/hss_pdma_init.c \

EXTRA_SRCS-$(CONFIG_OPENSBI) += \
        init/hss_opensbi_init.c

EXTRA_SRCS-$(CONFIG_USE_LOGO) += \
        init/hss_logo_init.c

INCLUDES +=\

init/hss_ddr_init.o: CFLAGS=$(CFLAGS_GCCEXT)
init/hss_pdma_init.o: CFLAGS=$(CFLAGS_GCCEXT)
init/hss_boot_init.o: CFLAGS=$(CFLAGS_GCCEXT)
init/hss_opensbi_init.o: CFLAGS=$(CFLAGS_GCCEXT)
init/hss_sys_setup.o: CFLAGS=$(CFLAGS_GCCEXT)
init/hss_pcie_init.o: CFLAGS=$(CFLAGS_GCCEXT)
