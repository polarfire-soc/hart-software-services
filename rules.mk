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
# Defines Rules such as compiler and tool choice/settings, build flags, and 
# basic build rules (.c to .o, etc)
#

CROSS_COMPILE?=riscv64-unknown-elf-
CC=$(CROSS_COMPILE)gcc
SIZE=$(CROSS_COMPILE)size
GPROF=$(CROSS_COMPILE)gprof
OBJCOPY=$(CROSS_COMPILE)objcopy
OBJDUMP=$(CROSS_COMPILE)objdump
READELF=$(CROSS_COMPILE)readelf
NM=$(CROSS_COMPILE)nm
ECHO=echo
MAKE=make
MAKEDEP=makedepend

PLATFORM_RISCV_ABI=lp64
PLATFORM_RISCV_ISA=rv64imac

CORE_CFLAGS+=$(MCMODEL) -mstrict-align

ifdef CONFIG_WITH_ARCH
  CORE_CFLAGS+=-mabi=$(PLATFORM_RISCV_ABI) -march=$(PLATFORM_RISCV_ISA) 
endif

CORE_CFLAGS+=-g -Wall -Werror -Wshadow -DDEBUG -ffast-math -fno-builtin-printf -fomit-frame-pointer
CORE_CFLAGS+=-Wredundant-decls -Wall -Wundef -Wwrite-strings -fno-strict-aliasing -fno-common \
  -Wendif-labels -Wmissing-include-dirs -Wempty-body -Wformat-security -Wformat-y2k -Winit-self \
 -Wignored-qualifiers -Wold-style-declaration -Wold-style-definition -Wtype-limits -Wstrict-prototypes 
#-ffreestanding

# TODO - introduce the following prototype warnings...
#CORE_CFLAGS+=-Wmissing-prototypes
# TODO - introduce the following conversion warnings... Currently fails on drivers and OpenSBI
#CORE_CFLAGS+=-Wconversion 
# TODO - introduce trapv for integer overflows etc... Currently missing primitives
#CORE_CFLAGS+=-ftrapv

CFLAGS=-std=c11 $(CORE_CFLAGS) $(PLATFORM_CFLAGS) -Wmissing-prototypes

# separate flags for C files that need GCC Extensions...
CFLAGS_GCCEXT=$(CORE_CFLAGS) $(PLATFORM_CFLAGS) 
#OPT-y=-O2
#OPT-y+=-Os -funroll-loops -fpeel-loops -fgcse-sm -fgcse-las
OPT-y+=-Os -fno-strict-aliasing


LINKER_SCRIPT=platform/${MACHINE}/hss.ld

#
# for some reason, -flto isn't playing nicely currently with -fstack-protector-strong...
# Stack protection is really useful, but if it is enabled, for now disabling LTO optimisation
# 
ifdef CONFIG_CC_STACKPROTECTOR_STRONG
  $(warning Not enabling -flto as stack protector enabled)
  CORE_CFLAGS+=-fstack-protector-strong
else
  $(warning NOTICE: enabling -flto (which means stack protection is disabled))
  OPT-y+=-flto=auto -ffat-lto-objects -fcompare-debug -fno-stack-protector
endif

all: config.h $(TARGET)

##############################################################################
#
# Build Rules
#

# Check if verbosity is ON for build process
CMD_PREFIX_DEFAULT := @
ifeq ($(V), 1)
  CMD_PREFIX :=
else
  CMD_PREFIX := $(CMD_PREFIX_DEFAULT)
endif

OBJS = $(SRCS-y:.c=.o)

%.S: %.c config.h
	@$(ECHO) " CC -S     $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -Wa,-adhln -g  $<  > $@

%.e: %.c config.h
	@$(ECHO) " CC -E     $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS) $(OPT-y) $(INCLUDES) -c -E -o $@ $<

ifdef CONFIG_USE_MAKEDEP
  %.o: %.c config.h %.d
else
  %.o: %.c config.h
endif
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS) $(OPT-y) $(INCLUDES) -c -o $@ $<

%.o: %.S config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS) $(defs) -D__ASSEMBLY__=1 -c $(INCLUDES) $< -o $@

%.hex: %.elf
	@$(ECHO) " HEX       $@";
	$(CMD_PREFIX)$(OBJCOPY) -O ihex $< $@

%.lss: %.elf
	@$(ECHO) " LSS       $@";
	$(CMD_PREFIX)$(OBJDUMP) -h -S -z $< > $@

%.sym: %.elf
	@$(ECHO) " NM        $@";
	$(CMD_PREFIX)$(NM) -n $< > $@

%.bin: %.elf
	@$(ECHO) " BIN       $@";
	$(CMD_PREFIX)$(OBJCOPY) -O binary $< $@

#
%.d: %.c 
	@$(ECHO) " MAKEDEP   $@";
	$(CMD_PREFIX)$(MAKEDEP) -f - $(INCLUDES) $< 2>/dev/null | sed 's,\($*\.o\)[ :]*\(.*\),$@ : $$\(wildcard \2\)\n\1 : \2,g' > $*.d 

%.d: %.S 
	@$(ECHO) " MAKEDEP   $@";
	$(CMD_PREFIX)$(MAKEDEP) -f - $(INCLUDES) $< 2>/dev/null | sed 's,\($*\.o\)[ :]*\(.*\),$@ : $$\(wildcard \2\)\n\1 : \2,g' > $*.d 
