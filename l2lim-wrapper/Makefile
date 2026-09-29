#
# MPFS HSS Embedded Software
#
# Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
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
# L2LIM Wrapper
#
# This wrapper is FEC-encoded and stored in eNVM for radiation-tolerant targets.
# The sNVM boot code FEC-decodes it to L2LIM, then all harts jump to L2LIM entry.
# The wrapper then decompresses the embedded DEFLATE-compressed HSS to L2Scratchpad.
#
# Unlike envm-wrapper (which executes directly from eNVM), this wrapper is linked
# to run from L2LIM (0x08000000). It shares the same compressed HSS payload
# (build/hss-l2scratch.bin.deflate) produced by envm-wrapper/Makefile.
#

OBJS-l2lim-wrapper = \
	$(BINDIR)/l2lim-wrapper/l2lim-wrapper_crt.o \
	$(BINDIR)/l2lim-wrapper/l2lim-wrapper_validate_crc.o \
	$(BINDIR)/l2lim-wrapper/l2lim-wrapper_stubs.o \
	\
	$(BINDIR)/application/hart0/hss_clock.o \
	$(BINDIR)/init/hss_sys_setup.o \
	$(BINDIR)/modules/misc/hss_crc32.o \
	$(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_l2_cache.o \
	$(BINDIR)/modules/misc/csr_helper.o \
	$(BINDIR)/modules/misc/assert.o \
	$(BINDIR)/modules/misc/c_stubs.o \
	$(BINDIR)/modules/misc/stack_guard.o \
	$(BINDIR)/thirdparty/miniz/miniz.o \
	$(BINDIR)/modules/misc/uart_helper.o \
        $(BINDIR)/$(BOARD_DIR)/uart_device_list.o \

#ifndef CONFIG_CODE_SIGNING
OBJS-l2lim-wrapper += \
       $(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_nwc_init.o \
       $(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_pll.o \
       $(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_io.o \
       $(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_sgmii.o \
       $(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_peripherals.o \
       $(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/startup_gcc/mss_utils.o \
       $(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss/mss_mmuart/mss_uart.o \

#endif


EXTRA_OBJS-l2lim-wrapper =

LINKER_SCRIPT-l2lim-wrapper = l2lim-wrapper/l2lim-wrapper.ld

# eNVM budget for the l2lim-wrapper binary.
#
# The l2lim-wrapper.bin is SECDED 72/64 FEC-encoded before being stored in eNVM:
# every 8 data bytes are stored as 9 bytes. The encoded blob follows a 256-byte
# zero header so that it lands at 0x20220100 (see snvm-bm2-loader/Makefile),
# leaving 128 KiB - 256 bytes for the encoded image.
# Maximum usable payload = floor((128 KiB - 256) / 9) × 8 = 116,280 bytes.
L2LIM_WRAPPER_ENVM_SIZE_BYTES   := 131072
L2LIM_WRAPPER_ENVM_HEADER_BYTES := 256
L2LIM_WRAPPER_MAX_BIN_BYTES     := $(shell expr \( $(L2LIM_WRAPPER_ENVM_SIZE_BYTES) - $(L2LIM_WRAPPER_ENVM_HEADER_BYTES) \) / 9 \* 8)

# The compressed HSS payload is shared with envm-wrapper; the build rule lives
# in envm-wrapper/Makefile (included before this file). We declare it as a
# prerequisite only — no duplicate build rule here.
COMPRESSED_TARGET_L2LIM = $(BINDIR)/$(TARGET-l2scratch:.elf=.bin.deflate)

$(BINDIR)/l2lim-wrapper/asm_offsets.h: l2lim-wrapper/asm_offsets.c
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	$(ECHO) " OFFSETS   $@"
	$(CC) $(CFLAGS_GCCEXT) $(INCLUDES) -I. -S -o - $< | \
		grep "^->" | \
		awk '{ gsub(/^\$$/, "", $$3); print "#define", $$3, $$2 }' > $@

$(BINDIR)/l2lim-wrapper/l2lim-wrapper_crt.o: $(COMPRESSED_TARGET_L2LIM) $(BINDIR)/l2lim-wrapper/asm_offsets.h
$(BINDIR)/l2lim-wrapper/l2lim-wrapper_crt.o: CFLAGS += -I$(BINDIR)/l2lim-wrapper
$(BINDIR)/l2lim-wrapper/l2lim-wrapper_validate_crc.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(BOARD_DIR)/uart_device_list.o: CFLAGS=$(CFLAGS_GCCEXT)

$(TARGET-l2lim-wrapper): private LIBS:=
$(TARGET-l2lim-wrapper): $(COMPRESSED_TARGET_L2LIM) $(OBJS-l2lim-wrapper)
	$(call main-build-target,l2lim-wrapper)
	@$(ECHO) " BIN       `basename $@ .elf`.bin"
	$(OBJCOPY) -O binary $(BINDIR)/$@ $(BINDIR)/`basename $@ .elf`.bin
	@BIN_SIZE=$$(wc -c < $(BINDIR)/`basename $@ .elf`.bin); \
	if [ $$BIN_SIZE -gt $(L2LIM_WRAPPER_MAX_BIN_BYTES) ]; then \
		echo "ERROR: `basename $@ .elf`.bin ($$BIN_SIZE bytes) exceeds SECDED 72/64 max payload for 128KiB eNVM less $(L2LIM_WRAPPER_ENVM_HEADER_BYTES)-byte header ($(L2LIM_WRAPPER_MAX_BIN_BYTES) bytes)"; \
		exit 1; \
	else \
		echo " SIZECHECK `basename $@ .elf`.bin: $$BIN_SIZE / $(L2LIM_WRAPPER_MAX_BIN_BYTES) bytes (SECDED 72/64, 128KiB eNVM less $(L2LIM_WRAPPER_ENVM_HEADER_BYTES)-byte header) => OK"; \
	fi
	$(SIZE) $(BINDIR)/$(TARGET-l2lim-wrapper) 2>/dev/null

l2lim-wrapper_clean:
	-$(RM) $(COMPRESSED_TARGET_L2LIM) $(OBJS-l2lim-wrapper) \
		$(BINDIR)/$(TARGET-l2lim-wrapper) \
		$(BINDIR)/`basename $(TARGET-l2lim-wrapper) .elf`.sym \
		$(BINDIR)/`basename $(TARGET-l2lim-wrapper) .elf`.bin \
		$(BINDIR)/l2lim-wrapper/asm_offsets.h

HEX_FILE-l2lim-wrapper=$(BINDIR)/$(TARGET-l2lim-wrapper:.elf=.hex)
BIN_FILE-l2lim-wrapper=$(BINDIR)/$(TARGET-l2lim-wrapper:.elf=.bin)

$(BIN_FILE-l2lim-wrapper): $(TARGET-l2lim-wrapper)
