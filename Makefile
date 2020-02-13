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

SHELL=/bin/bash

#
# To build the HSS under SoftConsole on Windows, we need to spawn the build out to
# MSYS2. If MSYS2, we need to ensure that the path is correctly setup to find
# genconfig.exe
# 
# Therefore, we need to determine whether we are on Linux, or a Unix-ish environment
# such as MSYS2 or Cygwin.  
#
SYSTEM:=$(shell uname -s)
ifneq (, $(findstring Linux, $(SYSTEM)))
# Linux-specific mods
#
# Nothing special needed
else ifneq (, $(findstring MSYS_NT, $(SYSTEM)))
# MSYS2-specific mods
#
# Adjust the path to ensure that we can run kconfiglib (genconfig) from SoftConsole
PATH+=:/usr/bin:/bin
$(info MSYS2 detected, PATH is "$(PATH)")
else ifneq (, $(findstring CYGWIN, $(SYSTEM)))
# Any Cygwin-specific paths
#
# Currently OPENSBI doesn't build on Cygwin without modifications to its Makefile...
#
ifdef CONFIG_OPENSBI
$(warning OPENSBI build may fail on Cygwin due to issues with file paths)
endif
else
endif

RISCV_TARGET=hss.elf

SRCS-y= \
    hss_state_machine.c \
    hss_clock.c \
    hss_registry.c \

INCLUDES=\
    -I./include \
    -I./thirdparty/riscv-pk \
    -I.

ASM_SRCS= \
    crt.S \
    thirdparty/riscv-pk/machine/mentry.S \
    thirdparty/riscv-pk/machine/mpfs_mutex.S
EXTRA_SRCS-y= \
    thirdparty/riscv-pk/machine/mtrap.c \
    hss_init.c \
    hss_main.c


EXTRA_OBJS=$(EXTRA_SRCS-y:.c=.o) $(ASM_SRCS:.S=.o) $(OPENSBI_LIBS) 

MCMODEL=-mcmodel=medany

TARGET:=$(RISCV_TARGET)
include .config
include rules.mk
include targets.mk
include init/Makefile
include misc/Makefile
include baremetal/Makefile
include ssmb/Makefile
include services/Makefile
ifdef CONFIG_COMPRESSION
include compression/Makefile
endif

LIBS =

ifndef CONFIG_SERVICE_QSPI
ifdef CONFIG_COMPRESSION
EXTRA_OBJS += \
        tools/compression/fastlz/bootImageBlob.bin.lz77.o
else
EXTRA_OBJS += \
        tools/bin2chunks/bootImageBlob.bin.o
endif
endif

EXTRA_SRCS-$(CONFIG_CC_STACKPROTECTOR_STRONG) += misc/stack_guard.c


ifdef CONFIG_OPENSBI
OPENSBI_LIBS = thirdparty/opensbi/build/lib/libsbi.a
$(OPENSBI_LIBS):
	+$(CMD_PREFIX)$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) PLATFORM_RISCV_ABI=$(PLATFORM_RISCV_ABI) PLATFORM_RISCV_ISA=$(PLATFORM_RISCV_ISA) -r --no-print-directory -C thirdparty/opensbi V=$(V)
else
OPENSBI_LIBS =
endif

#$(info $$INCLUDES is [${INCLUDES}])

hss_main.o: hss_main.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<
hss_init.o: hss_init.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<
thirdparty/riscv-pk/machine/mtrap.o: thirdparty/riscv-pk/machine/mtrap.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

tools/bin2chunks/bootImage.o: tools/bin2chunks/bootImage.c 
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

config.h: .config

ifdef CONFIG_USE_MAKEDEP
DEPENDENCIES=$(SRCS-y:.c=.d) $(EXTRA_SRCS-y:.c=.d) $(TEST_SRCS:.c=.d) $(ASM_SRCS:.S=.d) 
.PHONY: dep
dep: $(DEPENDENCIES)

-include $(DEPENDENCIES)
endif

$(RISCV_TARGET): $(OBJS) $(EXTRA_OBJS) config.h  $(DEPENDENCIES) $(LINKER_SCRIPT)
	@$(ECHO) " LD        $@";
	+$(CMD_PREFIX)$(CC) -T $(LINKER_SCRIPT)  $(CFLAGS_GCCEXT) $(OPT-y) -static -nostdlib -nostartfiles -nodefaultlibs -Wl,-Map=output.map -o $@ $(OBJS) $(EXTRA_OBJS) $(LIBS)
	@$(ECHO) " NM        `basename $@ .elf`.sym";
	$(CMD_PREFIX)$(NM) -n $@ > `basename $@ .elf`.sym
	@$(ECHO) " BIN       `basename $@ .elf`.bin"
	$(CMD_PREFIX)$(OBJCOPY) -O binary $@ `basename $@ .elf`.bin
	@$(ECHO) " HEX       `basename $@ .elf`.hex";
	$(CMD_PREFIX)$(OBJCOPY) -O ihex $@ `basename $@ .elf`.hex
	$(CMD_PREFIX)$(SIZE) $(TARGET) 2>/dev/null
