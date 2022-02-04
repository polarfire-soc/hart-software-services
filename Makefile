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

SHELL=/bin/sh
PYTHON?=python3

#
# To build the HSS under SoftConsole on Windows, we need to use SoftConsole-provided
# tools, and potentially to modify paths
#

.ONESHELL:

ifeq ($(OS), Windows_NT)
  #If we need to patch up path for Windows, we could do it here...
  #TOOLPATH:=${SC_INSTALL_DIR}riscv-unknown-elf-gcc\bin
  #export PATH:="$(TOOLPATH);$(PATH)"
  $(info INFO: Windows detected)
  HOST_WINDOWS:=true
else
  SYSTEM:=$(shell uname -s)
  ifneq (, $(findstring Linux, $(SYSTEM)))         # Linux-specific mods
    # Workaround SoftConsole v2021.1 Linux Python limitations by using system python
    export PATH:=/usr/bin:$(PATH)
    $(info INFO: Linux detected)
    HOST_LINUX:=true
  else
    $(error Unsupported build platform $(SYSTEM))
  endif
endif

include application/Makefile
include .config

ifneq ("$(wildcard boards/${BOARD}/Makefile)","")
  include boards/${BOARD}/Makefile
else
  ifndef BOARD
    BOARD:=mpfs-icicle-kit-es
    export BOARD
    $(info INFO: BOARD not specified, defaulting to ${BOARD}) # default to icicle if nothing found
    include boards/${BOARD}/Makefile
  else
    $(error Board >>${BOARD}<< not found)
  endif
endif


MCMODEL=-mcmodel=medany

include application/rules.mk
include application/targets.mk
include init/Makefile
include baremetal/Makefile
include services/Makefile
include modules/Makefile

LIBS =

#$(info $$INCLUDES is [${INCLUDES}])

ifdef CONFIG_CC_USE_MAKEDEP
  DEPENDENCIES=$(SRCS-y:.c=.d) $(EXTRA_SRCS-y:.c=.d) $(TEST_SRCS:.c=.d) $(ASM_SRCS:.S=.d) $(ASM_SRCS-y:.S=.d)
  .PHONY: dep
  dep: $(DEPENDENCIES)

  -include $(DEPENDENCIES)
endif

ifdef CONFIG_DISPLAY_TOOL_VERSIONS
include/tool_versions.h:
	echo \#define CC_VERSION_STRING \"`$(CC) --version | head -n 1`\" > include/tool_versions.h
	echo \#define LD_VERSION_STRING \"`$(LD) --version | head -n 1`\" >> include/tool_versions.h

DEPENDENCIES+=include/tool_versions.h
endif

include envm-wrapper/Makefile

################################################################################################
#
# Main Build Targets
#

OBJS-envm = $(OBJS)
EXTRA_OBJS-envm = $(EXTRA_OBJS)

OBJS-l2lim = $(OBJS)
EXTRA_OBJS-l2lim = $(EXTRA_OBJS)

define main-build-target
	$(ECHO) " LD        $@";
	$(CC) -T $(LINKER_SCRIPT-$(1)) $(CFLAGS_GCCEXT) $(OPT-y) \
		 -static -nostdlib -nostartfiles -nodefaultlibs \
		 -Wl,--build-id -Wl,-Map=$(BINDIR)/output-$(1).map -Wl,--gc-sections \
		 -o $(BINDIR)/$@ $(OBJS-$(1)) $(EXTRA_OBJS-$(1)) $(LIBS)
	$(ECHO) " NM        `basename $@ .elf`.sym";
	$(NM) -n $(BINDIR)/$@ > $(BINDIR)/`basename $@ .elf`.sym
endef

#
# Build Targets
#

$(TARGET-envm): $(OBJS) $(EXTRA_OBJS) config.h  $(DEPENDENCIES) $(LINKER_SCRIPT-envm) $(LIBS)
	$(call main-build-target,envm)
	$(ECHO) " BIN       `basename $@ .elf`.bin"
	$(OBJCOPY) -O binary $(BINDIR)/$@ $(BINDIR)/`basename $@ .elf`.bin
	$(ECHO) " HEX       `basename $@ .elf`.hex";
	$(OBJCOPY) -O ihex $(BINDIR)/$@ $(BINDIR)/`basename $@ .elf`.hex
	$(SIZE) $(BINDIR)/$(TARGET-envm) 2>/dev/null

$(TARGET-l2lim): $(OBJS) $(EXTRA_OBJS) config.h  $(DEPENDENCIES) $(LINKER_SCRIPT-l2lim) $(LIBS)
	$(call main-build-target,l2lim)
	$(ECHO) " BIN       `basename $@ .elf`.bin"
	$(OBJCOPY) -O binary $(BINDIR)/$@ $(BINDIR)/`basename $@ .elf`.bin
	$(SIZE) $(BINDIR)/$(TARGET-l2lim) 2>/dev/null

$(BINDIR)/$(TARGET-envm): $(TARGET-envm)
$(BINDIR)/$(TARGET-l2lim): $(TARGET-l2lim)

$(TARGET-ddr): $(OBJS) $(EXTRA_OBJS) config.h  $(DEPENDENCIES) $(LINKER_SCRIPT-ddr) $(LIBS)
	$(call main-build-target,ddr)
