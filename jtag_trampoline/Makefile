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
# Boot HSS over JTAG trampoline


OBJS-jtag_trampoline = \
	jtag_trampoline/jtag_trampoline_crt.o \
	jtag_trampoline/jtag_trampoline_funcs.o \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_l2_cache.o \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_nwc_init.o \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_pll.o \
    baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_io.o \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_sgmii.o \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_peripherals.o \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/startup_gcc/mss_utils.o \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss/mss_mmuart/mss_uart.o

EXTRA_OBJS-jtag_trampoline=

LINKER_SCRIPT-jtag_trampoline = jtag_trampoline/jtag_trampoline.ld

jtag_trampoline/jtag_trampoline_crc.o: CFLAGS=$(CFLAGS_GCCEXT)
jtag_trampoline/jtag_trampoline_funcs.o: CFLAGS=$(CFLAGS_GCCEXT)

$(TARGET-jtag_trampoline): LIBS:=
$(TARGET-jtag_trampoline): $(OBJS-jtag_trampoline) $(BINDIR)/run-hss.cfg
	$(call main-build-target,jtag_trampoline)
	@$(ECHO) " HEX       `basename $@ .elf`.hex";
	$(OBJCOPY) -O ihex $(BINDIR)/$@ $(BINDIR)/`basename $@ .elf`.hex
	$(SIZE) $(BINDIR)/$(TARGET-jtag_trampoline) 2>/dev/null

jtag_trampoline_clean:
	-$(RM) $(COMPRESSED_TARGET) $(OBJS-jtag_trampoline) $(BINDIR)/$(TARGET-jtag_trampoline) $(BINDIR)/`basename $(TARGET-jtag_trampoline) .elf`.sym $(BINDIR)/`basename $(TARGET-jtag_trampoline) .elf`.bin

HEX_FILE-jtag_trampoline=$(BINDIR)/$(TARGET-jtag_trampoline:.elf=.hex)
BIN_FILE-jtag_trampoline=$(BINDIR)/$(TARGET-jtag_trampoline:.elf=.bin)

BIN_FILE-jtag_trampoline: $(TARGET-jtag_trampoline)

COPY_CMD := cp

ifeq ($(OS),Windows_NT)
	COPY_CMD := cmd /c copy
endif

$(BINDIR)/run-hss.cfg: jtag_trampoline/run-hss.cfg
	$(SHELL) -c "cp jtag_trampoline/run-hss.cfg $(BINDIR)/run-hss.cfg"
