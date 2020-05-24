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
# Miscellaneous Functionality

EXTRA_SRCS-y += \
        misc/assert.c \
        misc/csr_helper.c \
        misc/c_stubs.c \
        misc/ee_printf.c \
        misc/u54_handle_ipi.c \
        misc/vsnprintf.c \
        misc/hss_crc16.c \
        misc/hss_crc32.c \
        misc/hss_memcpy_via_pdma.c \
        misc/hss_progress.c \

EXTRA_SRCS-$(CONFIG_TINYCLI) += \
        misc/hss_tinycli.c \

EXTRA_SRCS-$(CONFIG_MEMTEST) += \
        misc/hss_memtest.c \

INCLUDES +=\

misc/c_stubs.o: misc/c_stubs.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

misc/u54_handle_ipi.o: misc/u54_handle_ipi.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

misc/vsnprintf.o: misc/vsnprintf.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

misc/assert.o: misc/assert.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

misc/ee_printf.o: misc/ee_printf.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

misc/csr_helper.o: misc/csr_helper.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

misc/hss_memcpy_via_pdma.o: misc/hss_memcpy_via_pdma.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

misc/hss_memtest.o: misc/hss_memtest.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

misc/stack_guard.o: misc/stack_guard.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

###################################################################################
#
# Optional OPENSBI support
#

EXTRA_SRCS-$(CONFIG_OPENSBI) += \
        thirdparty/opensbi/lib/sbi/sbi_string.c
ifdef CONFIG_OPENSBI
INCLUDES += -I./thirdparty/opensbi/include/sbi
endif

