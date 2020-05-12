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
	thirdparty/opensbi/lib/utils/sys/clint.c \
	thirdparty/opensbi/lib/utils/irqchip/plic.c \
	thirdparty/opensbi/lib/utils/libfdt/fdt.c \
	thirdparty/opensbi/lib/utils/libfdt/fdt_ro.c \
	thirdparty/opensbi/lib/utils/libfdt/fdt_rw.c \
	thirdparty/opensbi/lib/utils/libfdt/fdt_wip.c \
	thirdparty/opensbi/lib/utils/libfdt/fdt_addresses.c \

ifdef CONFIG_PLATFORM_MPFS
SRCS-$(CONFIG_SERVICE_OPENSBI) += \
	services/opensbi/platform.c
endif

ifdef CONFIG_PLATFORM_FU540
SRCS-$(CONFIG_SERVICE_OPENSBI) += \
	thirdparty/opensbi/lib/utils/serial/sifive-uart.c \
	thirdparty/opensbi/platform/sifive/fu540/platform.c
endif


ifdef CONFIG_OPENSBI
INCLUDES += \
	-I./services/opensbi \
	-I./thirdparty/opensbi/include/sbi \
	-I./thirdparty/opensbi/include \
	-I./thirdparty/opensbi/lib/utils/libfdt \

endif

ifdef CONFIG_PROVIDE_DTB
ifdef CONFIG_DEFAULT_DEVICE_TREE
EXTRA_OBJS += services/opensbi/mpfs_dtb.o
services/opensbi/mpfs_dtb.o: $(CONFIG_DEFAULT_DEVICE_TREE:"%"=%).dtb
	$(CMD_PREFIX)$(CROSS_COMPILE)ld -r -b binary -o $@ $<
endif
endif

services/opensbi/opensbi_service.o: services/opensbi/opensbi_service.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

services/opensbi/platform.o: services/opensbi/platform.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/lib/utils/sys/clint.o: thirdparty/opensbi/lib/utils/sys/clint.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/lib/utils/irqchip/plic.o: thirdparty/opensbi/lib/utils/irqchip/plic.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/lib/utils/libfdt/fdt_rw.o: thirdparty/opensbi/lib/utils/libfdt/fdt_rw.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/lib/utils/libfdt/fdt_ro.o: thirdparty/opensbi/lib/utils/libfdt/fdt_ro.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/lib/utils/libfdt/fdt.o: thirdparty/opensbi/lib/utils/libfdt/fdt.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/lib/utils/libfdt/fdt_overlay.o: thirdparty/opensbi/lib/utils/libfdt/fdt_overlay.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/lib/utils/libfdt/fdt_strerror.o: thirdparty/opensbi/lib/utils/libfdt/fdt_strerror.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/lib/utils/libfdt/fdt_sw.o: thirdparty/opensbi/lib/utils/libfdt/fdt_sw.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/lib/utils/libfdt/fdt_addresses.o: thirdparty/opensbi/lib/utils/libfdt/fdt_addresses.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/lib/utils/libfdt/fdt_wip.o: thirdparty/opensbi/lib/utils/libfdt/fdt_wip.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/lib/utils/serial/sifive-uart.o: thirdparty/opensbi/lib/utils/serial/sifive-uart.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

thirdparty/opensbi/platform/sifive/fu540/platform.o: thirdparty/opensbi/platform/sifive/fu540/platform.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 
