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
# OpenSPI Service

SRCS-$(CONFIG_SERVICE_OPENSBI) += \
	services/opensbi/opensbi_service.c \
	services/opensbi/platform.c \
	thirdparty/opensbi/platform/common/sys/clint.c \
	thirdparty/opensbi/platform/common/irqchip/plic.c \
	thirdparty/opensbi/platform/common/tinyfdt.c \

#	opensbi/platform/common/libfdt/fdt_rw.c \
#	opensbi/platform/common/libfdt/fdt_ro.c \
#	opensbi/platform/common/libfdt/fdt.c \
#	opensbi/platform/common/libfdt/fdt_overlay.c \
#	opensbi/platform/common/libfdt/fdt_strerror.c \
#	opensbi/platform/common/libfdt/fdt_sw.c \
#	opensbi/platform/common/libfdt/fdt_wip.c \

ifdef CONFIG_OPENSBI
INCLUDES += \
	-I./services/opensbi \
	-I./thirdparty/opensbi/include/sbi \
	-I./thirdparty/opensbi/include \
	-I./thirdparty/opensbi/platform/common \

#	-I./thirdparty/opensbi/platform/common/libfdt
endif


services/opensbi/opensbi_service.o: services/opensbi/opensbi_service.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

services/opensbi/platform.o: services/opensbi/platform.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/platform/common/sys/clint.o: thirdparty/opensbi/platform/common/sys/clint.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/platform/common/irqchip/plic.o: thirdparty/opensbi/platform/common/irqchip/plic.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/platform/common/libfdt/fdt_rw.o: thirdparty/opensbi/platform/common/libfdt/fdt_rw.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/platform/common/libfdt/fdt_ro.o: thirdparty/opensbi/platform/common/libfdt/fdt_ro.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/platform/common/libfdt/fdt.o: thirdparty/opensbi/platform/common/libfdt/fdt.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/platform/common/libfdt/fdt_overlay.o: thirdparty/opensbi/platform/common/libfdt/fdt_overlay.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/platform/common/libfdt/fdt_strerror.o: thirdparty/opensbi/platform/common/libfdt/fdt_strerror.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/platform/common/libfdt/fdt_sw.o: thirdparty/opensbi/platform/common/libfdt/fdt_sw.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/platform/common/libfdt/fdt_wip.o: thirdparty/opensbi/platform/common/libfdt/fdt_wip.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 
