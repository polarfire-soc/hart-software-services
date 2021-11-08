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
# hss-payload-generator Makefile
#

SHELL=/bin/bash
CC = gcc
ECHO = echo

ifeq ($(V), 1)
else
.SILENT:
endif

build_dir?=$(CURDIR)
ifneq ($(O),)
	build_dir:=$(O)
endif

CFLAGS= -g3 -ggdb -std=gnu11 -O3 \
	-Wall -Werror -Wshadow -fno-builtin-printf \
	-fomit-frame-pointer -Wredundant-decls -Wall -Wundef -Wwrite-strings -fno-strict-aliasing \
	-fno-common -Wendif-labels -Wmissing-include-dirs -Wempty-body -Wformat=2 -Wformat-security \
	-Wformat-y2k -Winit-self -Wignored-qualifiers -Wold-style-declaration -Wold-style-definition \
	-Wtype-limits -Wstrict-prototypes -Wimplicit-fallthrough=5 \
	-Wmissing-prototypes -Wconversion -ftrapv \
	-Wno-stringop-truncation \
	-fstack-protector-strong

INCLUDES=\
	-I. \
	-I../../include \

LIBS=\
	-lyaml \
	-lelf \
	-lz \

SRCS=\
	main.c \
	yaml_parser.c \
	blob_handler.c \
	elf_parser.c \
	elf_strings.c \
	crc32.c \
	generate_payload.c \
	dump_payload.c \
	debug_printf.c \

OBJS := $(patsubst %.c,$(build_dir)/%.o,$(SRCS))

################################################################################
#
# Build Rules
#

$(build_dir)/%.o: %.c
	@$(ECHO) " CC        $@";
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

################################################################################
#
# Targets
#

TARGET := $(build_dir)/hss-payload-generator
all: $(TARGET)

.PHONY: clean cppcheck

$(TARGET): $(OBJS)
	@$(ECHO) " LD        $@";
	$(CC) $(CFLAGS) $(LDFLAGS) -static -o $@ $(OBJS) $(LIBS)

README.docx: README.md
	pandoc -o README.docx README.md

.phony: clean
clean:
	@$(ECHO) " RM      $(TARGET) $(OBJS)"
	$(RM) $(TARGET) $(OBJS) cppcheck.log README.docx

cppcheck: $(SRCS)
	cppcheck --suppress=missingIncludeSystem -v --enable=all --inconclusive --std=posix \
                 $(INCLUDES) --force . >cppcheck.log 2>&1
