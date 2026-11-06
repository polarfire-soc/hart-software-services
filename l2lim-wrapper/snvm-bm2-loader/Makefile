#
# MPFS HSS Embedded Software
#
# Copyright 2019-2026 Microchip FPGA Embedded Systems Solutions
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
# sNVM Boot Mode 2 (BM2) Loader
#
# This loader is stored in sNVM (Secure NVM).  The PolarFire SoC system
# controller validates its signature, copies it to the 8 KiB E51 DTIM, and
# releases all five harts.  The loader:
#
#   1. FEC-decodes the SECDED 72/64 eNVM contents (l2lim-wrapper + compressed
#      HSS image) to L2LIM
#   2. Validates the SHA-256 digest of the decoded region
#   3. Fires CLINT MSIP[1..4] to release U54 harts (each already points
#      mtvec at L2LIM entry)
#   4. Jumps the E51 to L2LIM entry (l2lim-wrapper _start)
#
# Build artefacts:
#   build/hss-snvm-bm2-loader.elf   - ELF image for debugging
#   build/hss-snvm-bm2-loader.bin   - flat binary image (≤ 8192 bytes)
#   build/hss-envm-l2lim-wrapper.bin.fec - FEC-encoded eNVM blob (program to eNVM)
#
# The generated header build/snvm-bm2-loader/lim_sha256_digest.h embeds the
# SHA-256 digest, decoded byte count, and FEC-encoded byte count of
# l2lim-wrapper.bin into the loader at compile time
#

# The snvm-bm2-loader is only built for boards that define TARGET-l2lim-wrapper
# On boards that use a different boot flow, this entire Makefile is a no-op
ifdef TARGET-l2lim-wrapper

BM2_DIR := l2lim-wrapper/snvm-bm2-loader

OBJS-snvm-bm2-loader := \
	$(BINDIR)/$(BM2_DIR)/snvm-bm2-loader_crt.o \
	$(BINDIR)/$(BM2_DIR)/snvm-bm2-loader.o \
	$(BINDIR)/$(BM2_DIR)/snvm-bm2-loader_sha256.o \
	$(BINDIR)/$(BM2_DIR)/fec_secded7264_bare.o \
	$(BINDIR)/$(BM2_DIR)/snvm-bm2-loader_stubs.o \
	\
	$(BINDIR)/application/hart0/hss_clock.o \
	$(BINDIR)/init/hss_sys_setup.o \
	$(BINDIR)/modules/misc/assert.o \
	$(BINDIR)/modules/misc/c_stubs.o \
	$(BINDIR)/modules/misc/stack_guard.o \
	$(BINDIR)/modules/misc/uart_helper.o \
	$(BINDIR)/$(BOARD_DIR)/uart_device_list.o \
	$(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_l2_cache.o \
	$(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_nwc_init.o \
	$(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss/mss_mmuart/mss_uart.o \
	$(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_pll.o \
	$(BINDIR)/envm-wrapper/envm-wrapper_stubs.o \
	$(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_io.o \
	$(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/startup_gcc/mss_utils.o \
	$(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_sgmii.o \

LINKER_SCRIPT-snvm-bm2-loader = $(BM2_DIR)/snvm-bm2-loader.ld

TARGET-snvm-bm2-loader = hss-snvm-bm2-loader.elf

# The DTIM hard limit (bytes)
SNVM_BM2_MAX_BIN_BYTES := 8192

# Python FEC encoder script (in-tree; no C host tool needed)
FEC_PY_TOOL := tools/fec_secded7264_encdec/fec_secded7264.py

# Generated header that embeds SHA-256 digest, decoded size and encoded size
BM2_DIGEST_HDR    := $(BINDIR)/snvm-bm2-loader/lim_sha256_digest.h
GEN_LIM_DIGEST    := tools/snvm-bm2-loader/gen-lim-sha256-digest.py

#
# CRT: needs the opensbi headers for CSR name macros
# Add the path explicitly so the build works even without CONFIG_SERVICE_OPENSBI
$(BINDIR)/$(BM2_DIR)/snvm-bm2-loader_crt.o: CFLAGS += -Ithirdparty/opensbi/include

# All snvm-bm2-loader objects: local includes + generated digest header dir
$(BINDIR)/$(BM2_DIR)/snvm-bm2-loader.o: CFLAGS += \
	-I$(BM2_DIR) -I$(BINDIR)/snvm-bm2-loader
$(BINDIR)/$(BM2_DIR)/snvm-bm2-loader_sha256.o: CFLAGS += -I$(BM2_DIR)
$(BINDIR)/$(BM2_DIR)/fec_secded7264_bare.o: CFLAGS += -I$(BM2_DIR)
$(BINDIR)/$(BM2_DIR)/snvm-bm2-loader_stubs.o: CFLAGS += -I$(BM2_DIR)

# The main C file includes the generated digest header. Ensure it is built first
$(BINDIR)/$(BM2_DIR)/snvm-bm2-loader.o: $(BM2_DIGEST_HDR)

#
# Generate the SHA-256 digest header from l2lim-wrapper.bin
# Also records the decoded byte count and FEC-encoded byte count so that
# snvm-bm2-loader.c knows exactly how many bytes to pass to the decoder
#$(BM2_DIGEST_HDR): $(TARGET-l2lim-wrapper)
$(BM2_DIGEST_HDR): $(BIN_FILE-l2lim-wrapper) $(GEN_LIM_DIGEST)
	$(ECHO) " GEN       $@"
	$(PYTHON) $(GEN_LIM_DIGEST) $< $@

#
# FEC-encoded eNVM blob: encode l2lim-wrapper.bin with the Python FEC tool
#$(BINDIR)/hss-envm-l2lim-wrapper.bin.fec: $(TARGET-l2lim-wrapper)
$(BINDIR)/hss-envm-l2lim-wrapper.bin.fec: $(BIN_FILE-l2lim-wrapper) $(FEC_PY_TOOL)
	$(ECHO) " FEC-ENC   $@"
	$(PYTHON) $(FEC_PY_TOOL) -e -i $< -o $@

#
# Link the BM2 loader ELF
$(TARGET-snvm-bm2-loader): private LIBS :=
$(TARGET-snvm-bm2-loader): $(OBJS-snvm-bm2-loader) $(LINKER_SCRIPT-snvm-bm2-loader) $(BM2_DIGEST_HDR)
	$(call main-build-target,snvm-bm2-loader)
	$(ECHO) " BIN       `basename $@ .elf`.bin"
	$(OBJCOPY) -O binary $(BINDIR)/$@ $(BINDIR)/`basename $@ .elf`.bin
	@BIN_SIZE=$$(wc -c < $(BINDIR)/`basename $@ .elf`.bin); \
	if [ $$BIN_SIZE -gt $(SNVM_BM2_MAX_BIN_BYTES) ]; then \
		echo "ERROR: `basename $@ .elf`.bin ($$BIN_SIZE bytes) exceeds 8 KiB DTIM limit ($(SNVM_BM2_MAX_BIN_BYTES) bytes)"; \
		exit 1; \
	else \
		echo " SIZECHECK `basename $@ .elf`.bin: $$BIN_SIZE / $(SNVM_BM2_MAX_BIN_BYTES) bytes (8 KiB DTIM) => OK"; \
	fi
	$(SIZE) $(BINDIR)/$(TARGET-snvm-bm2-loader) 2>/dev/null

# Also build the FEC-encoded eNVM blob when the loader is built
$(TARGET-snvm-bm2-loader): $(BINDIR)/hss-envm-l2lim-wrapper.bin.fec

# Add the snvm-bm2-loader to the default 'all' build target
# Make accumulates prerequisites from multiple rules for the same target
all: $(TARGET-snvm-bm2-loader)

clean: snvm-bm2-loader_clean
snvm-bm2-loader_clean:
	-$(RM) $(OBJS-snvm-bm2-loader) \
		$(BINDIR)/$(TARGET-snvm-bm2-loader) \
		$(BINDIR)/`basename $(TARGET-snvm-bm2-loader) .elf`.bin \
		$(BINDIR)/`basename $(TARGET-snvm-bm2-loader) .elf`.sym \
		$(BINDIR)/hss-snvm-bm2-loader-ubli.bin \
		$(BINDIR)/hss-snvm-bm2-loader-ubli.hex \
		$(BINDIR)/hss-envm-l2lim-wrapper.bin.fec \
		$(BINDIR)/hss-envm-l2lim-wrapper.bin.fec.padded \
		$(BINDIR)/hss-envm-l2lim-wrapper.bin.fec.hex \
		$(BM2_DIGEST_HDR)

# BM2 programming: dryrun / program / verify (single JTAG session, direct fpgenprog).

FEC_FILE-l2lim-wrapper       = $(BINDIR)/hss-envm-l2lim-wrapper.bin.fec

# Prepend a 256-byte zero header so the FEC payload lands at 0x20220100
ENVM_HEADER_BYTES            := 256
FEC_PADDED-l2lim-wrapper     = $(BINDIR)/hss-envm-l2lim-wrapper.bin.fec.padded
$(FEC_PADDED-l2lim-wrapper): $(FEC_FILE-l2lim-wrapper)
	$(ECHO) " PAD       `basename $@`"
	{ head -c $(ENVM_HEADER_BYTES) /dev/zero; cat $<; } > $@

# Convert to Intel HEX at the eNVM base LMA (0x20220000)
ENVM_BASE_ADDR               := 20220000
FEC_HEX_FILE-l2lim-wrapper   = $(BINDIR)/hss-envm-l2lim-wrapper.bin.fec.hex
$(FEC_HEX_FILE-l2lim-wrapper): $(FEC_PADDED-l2lim-wrapper)
	$(ECHO) " HEX       `basename $@`"
	$(OBJCOPY) -I binary -O ihex --change-section-lma '*+0x$(ENVM_BASE_ADDR)' $< $@

# sNVM boot client: a plain-text UBLI. gen-snvm-ubli.py wraps the flat binary in
# the 60-byte UBLI header (reset vector = DTIM entry 0x01000000, payload SHA-256).
BIN_FILE-snvm-bm2-loader       = $(BINDIR)/$(TARGET-snvm-bm2-loader:.elf=.bin)
GEN_SNVM_UBLI                  := tools/snvm-bm2-loader/gen-snvm-ubli.py
# Reset vector = loader ELF entry point (E51 DTIM, 0x01000000).
UBLI_RESETVEC                  := 01000000
SNVM_UBLI_BIN-snvm-bm2-loader  = $(BINDIR)/hss-snvm-bm2-loader-ubli.bin
SNVM_HEX_FILE-snvm-bm2-loader  = $(BINDIR)/hss-snvm-bm2-loader-ubli.hex

$(SNVM_UBLI_BIN-snvm-bm2-loader): $(BIN_FILE-snvm-bm2-loader) $(GEN_SNVM_UBLI)
	$(ECHO) " UBLI      `basename $@`"
	$(PYTHON) $(GEN_SNVM_UBLI) $(BIN_FILE-snvm-bm2-loader) $@ $(UBLI_RESETVEC)

$(SNVM_HEX_FILE-snvm-bm2-loader): $(SNVM_UBLI_BIN-snvm-bm2-loader)
	$(ECHO) " HEX       `basename $@`"
	$(OBJCOPY) -I binary -O ihex $< $@

# Prepare the fpgenprog project (no hardware access): new_project, snvm_client,
# mss_boot_info, envm_client, generate_bitstream.
define setup-fpgenprog-project-l2lim
	-$(RM) -r $(BINDIR)/bootmode$(BOOTMODE)
	mkdir -p $(BINDIR)/bootmode$(BOOTMODE)
	$(FPGENPROG) new_project \
		--location `pwd`/$(BINDIR)/bootmode$(BOOTMODE)/fpgenprogProject \
		--target_die $(DIE) --target_package $(PACKAGE)
	$(FPGENPROG) snvm_client \
		--location `pwd`/$(BINDIR)/bootmode$(BOOTMODE)/fpgenprogProject \
		--client_name snvm_bm2_loader \
		--content_file_format intel-hex \
		--content_file `pwd`/$(SNVM_HEX_FILE-snvm-bm2-loader) \
		--mem_file_base_address $(UBLI_RESETVEC) \
		--start_page $(BM2_SNVM_PAGE) \
		--number_of_bytes $(shell wc -c < $(SNVM_UBLI_BIN-snvm-bm2-loader) | awk '{print $$1}') \
		--fabric_access_read 1 --fabric_access_write 1 \
		--mss_access_read  1  --mss_access_write  1
	$(FPGENPROG) mss_boot_info \
		--location `pwd`/$(BINDIR)/bootmode$(BOOTMODE)/fpgenprogProject \
		--u_mss_bootmode $(BOOTMODE) \
		--u_mss_bootcfg $(BM2_BOOTCFG)
	$(FPGENPROG) envm_client \
		--location `pwd`/$(BINDIR)/bootmode$(BOOTMODE)/fpgenprogProject \
		--client_name envm_l2lim_wrapper \
		--content_file_format intel-hex \
		--content_file `pwd`/$(FEC_HEX_FILE-l2lim-wrapper) \
		--mem_file_base_address $(ENVM_BASE_ADDR) \
		--start_page 0 \
		--number_of_bytes $(shell wc -c < $(FEC_PADDED-l2lim-wrapper) | awk '{print $$1}')
	$(FPGENPROG) generate_bitstream \
		--location `pwd`/$(BINDIR)/bootmode$(BOOTMODE)/fpgenprogProject
endef

# Programs or verifies sNVM and eNVM in a single JTAG session.
# $(1) = fpgenprog action: PROGRAM or VERIFY
define boot-mode-programmer-l2lim
	$(call setup-fpgenprog-project-l2lim)
	$(FPGENPROG) run_action \
		--location `pwd`/$(BINDIR)/bootmode$(BOOTMODE)/fpgenprogProject \
		--action $(1)
endef

dryrun: $(SNVM_HEX_FILE-snvm-bm2-loader) $(FEC_HEX_FILE-l2lim-wrapper)
	$(call setup-fpgenprog-project-l2lim)

program: $(SNVM_HEX_FILE-snvm-bm2-loader) $(FEC_HEX_FILE-l2lim-wrapper)
	$(call boot-mode-programmer-l2lim,PROGRAM)

verify: $(SNVM_HEX_FILE-snvm-bm2-loader) $(FEC_HEX_FILE-l2lim-wrapper)
	$(call boot-mode-programmer-l2lim,VERIFY)

endif  # TARGET-l2lim-wrapper
