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
# eNVM Compression Wrapper

COMPRESS=tools/compression/hss-deflate.py

OBJS-envm-wrapper = \
	$(BINDIR)/envm-wrapper/envm-wrapper_crt.o \
	$(BINDIR)/envm-wrapper/envm-wrapper_validate_crc.o \
	$(BINDIR)/envm-wrapper/envm-wrapper_stubs.o \
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
OBJS-envm-wrapper += \
       $(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_nwc_init.o \
       $(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_pll.o \
       $(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_io.o \
       $(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_sgmii.o \
       $(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_peripherals.o \
       $(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/startup_gcc/mss_utils.o \
       $(BINDIR)/baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss/mss_mmuart/mss_uart.o \

#endif


EXTRA_OBJS-envm-wrapper =

LINKER_SCRIPT-envm-wrapper = envm-wrapper/envm-wrapper.ld

COMPRESSED_TARGET=$(BINDIR)/$(TARGET-l2scratch:.elf=.bin.deflate)

$(BINDIR)/envm-wrapper/envm-wrapper_crt.o: $(COMPRESSED_TARGET) 
$(BINDIR)/thirdparty/miniz/miniz.o: CFLAGS=$(CFLAGS_GCCEXT) -DMINIZ_NO_STDIO -DMINIZ_NO_TIME
$(BINDIR)/envm-wrapper/envm-wrapper_validate_crc.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(BOARD_DIR)/uart_device_list.o: CFLAGS=$(CFLAGS_GCCEXT)

define common-boot-mode-programmer
	-$(RM) -r $(BINDIR)/bootmode$(BOOTMODE)
	[ "$(SC_INSTALL_DIR)" ] || ( echo "SC_INSTALL_DIR environment variable is unset"; exit 1 )
	[ -d $(SC_INSTALL_DIR) ] || ( echo "SoftConsole subdirectory >>$(SC_INSTALL_DIR)<< does not exist."; exit 2 )
	$(MPFS_BOOTMODE_PROGRAMMER) --workdir `pwd`/$(BINDIR) --die $(DIE) --package $(PACKAGE) --bootmode $(BOOTMODE) $(BOOTMODE_ARGS) $(1) $(2)
endef


MPFS_BOOTMODE_PROGRAMMER_VERSION := $(findstring v3.6,$(shell $(MPFS_BOOTMODE_PROGRAMMER) --help))
EXPECTED_MPFS_BOOTMODE_PROGRAMMER_VERSION := v3.6
ifneq ($(MPFS_BOOTMODE_PROGRAMMER_VERSION),$(EXPECTED_MPFS_BOOTMODE_PROGRAMMER_VERSION))
  $(info INFO: Expected mpfsBootmodeProgrammer.jar version $(EXPECTED_MPFS_BOOTMODE_PROGRAMMER_VERSION) but found $(MPFS_BOOTMODE_PROGRAMMER_VERSION))
  $(info INFO: This version of the HSS relies on SoftConsole v2021.3 or later)
endif

$(TARGET-envm-wrapper): LIBS:=
$(TARGET-envm-wrapper): $(COMPRESSED_TARGET) $(OBJS-envm-wrapper)
	$(call main-build-target,envm-wrapper)
	@$(ECHO) " BIN       `basename $@ .elf`.bin"
	$(OBJCOPY) -O binary $(BINDIR)/$@ $(BINDIR)/`basename $@ .elf`.bin
	@$(ECHO) " HEX       `basename $@ .elf`.hex";
	$(call common-boot-mode-programmer,--dryrun,`basename $@`)
	@$(CP) $(BINDIR)/bootmode$(BOOTMODE)/hss-envm-wrapper-bm$(BOOTMODE)-p0.hex $(BINDIR)/`basename $@ .elf`.${BOARD}.hex
	$(SIZE) $(BINDIR)/$(TARGET-envm-wrapper) 2>/dev/null

$(COMPRESSED_TARGET): $(BINDIR)/$(TARGET-l2scratch:.elf=.bin) $(COMPRESS)
	@$(ECHO) " COMPRESS  $<"
	$(PYTHON) $(COMPRESS) $< $@

envm-wrapper_clean:
	-$(RM) $(COMPRESSED_TARGET) $(OBJS-envm-wrapper) $(BINDIR)/$(TARGET-envm-wrapper) $(BINDIR)/`basename $(TARGET-envm-wrapper) .elf`.sym $(BINDIR)/`basename $(TARGET-envm-wrapper) .elf`.bin

HEX_FILE-envm-wrapper=$(BINDIR)/$(TARGET-envm-wrapper:.elf=.hex)
BIN_FILE-envm-wrapper=$(BINDIR)/$(TARGET-envm-wrapper:.elf=.bin)

BIN_FILE-envm-wrapper: $(TARGET-envm-wrapper)

BOOTMODE?=1
PACKAGE?=FCVG484
DIE?=MPFS250T
FPGENPROG?=/opt/microsemi/Libero_SoC_v2021.2/Libero/bin64/fpgenprog
FPGENPROG:=$(FPGENPROG:_bin=)

program: $(BIN_FILE-envm-wrapper)
	$(call common-boot-mode-programmer,,$(TARGET-envm-wrapper))

verify: $(BIN_FILE-envm-wrapper)
	$(call common-boot-mode-programmer,--verify,$(TARGET-envm-wrapper))
