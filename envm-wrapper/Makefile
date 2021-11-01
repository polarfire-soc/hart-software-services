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
	envm-wrapper/envm-wrapper_crt.o \
	envm-wrapper/envm-wrapper_validate_crc.o \
	envm-wrapper/envm-wrapper_stubs.o \
	\
	application/hart0/hss_clock.o \
	init/hss_sys_setup.o \
	modules/misc/hss_crc32.o \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_l2_cache.o \
	modules/misc/csr_helper.o \
	modules/misc/assert.o \
	modules/misc/c_stubs.o \
	modules/misc/stack_guard.o \
	thirdparty/miniz/miniz.o

EXTRA_OBJS-envm-wrapper =

LINKER_SCRIPT-envm-wrapper=envm-wrapper/envm-wrapper.ld

COMPRESSED_TARGET=$(BINDIR)/$(TARGET-l2lim:.elf=.bin.deflate)

envm-wrapper/envm-wrapper_crt.o: $(COMPRESSED_TARGET) 
thirdparty/miniz/miniz.o: CFLAGS=$(CFLAGS_GCCEXT) -DMINIZ_NO_STDIO -DMINIZ_NO_TIME

$(TARGET-envm-wrapper): LIBS:=
$(TARGET-envm-wrapper): $(COMPRESSED_TARGET) $(OBJS-envm-wrapper)
	$(call main-build-target,envm-wrapper)
	@$(ECHO) " BIN       `basename $@ .elf`.bin"
	$(OBJCOPY) -O binary $(BINDIR)/$@ $(BINDIR)/`basename $@ .elf`.bin
	@$(ECHO) " HEX       `basename $@ .elf`.hex";
	$(OBJCOPY) -O ihex $(BINDIR)/$@ $(BINDIR)/`basename $@ .elf`.hex
	$(SIZE) $(BINDIR)/$(TARGET-envm-wrapper) 2>/dev/null

$(COMPRESSED_TARGET): $(BINDIR)/$(TARGET-l2lim:.elf=.bin) $(COMPRESS)
	@$(ECHO) " COMPRESS  $<"
	$(PYTHON) $(COMPRESS) $< $@

envm-wrapper_clean:
	-$(RM) $(COMPRESSED_TARGET) $(OBJS-envm-wrapper) $(BINDIR)/$(TARGET-envm-wrapper) $(BINDIR)/`basename $(TARGET-envm-wrapper) .elf`.sym $(BINDIR)/`basename $(TARGET-envm-wrapper) .elf`.bin

HEX_FILE-envm-wrapper=$(BINDIR)/$(TARGET-envm-wrapper:.elf=.hex)
BIN_FILE-envm-wrapper=$(BINDIR)/$(TARGET-envm-wrapper:.elf=.bin)

BIN_FILE-envm-wrapper: $(TARGET-envm-wrapper)

BOOTMODE?=1
PACKAGE?=FCVG484
DIE?=MPFS250T_ES
FPGENPROG?=/opt/microsemi/Libero_SoC_v12.6/Libero/bin64/fpgenprog
FPGENPROG:=$(FPGENPROG:_bin=)

program: $(BIN_FILE-envm-wrapper)
	-$(RM) -r $(BINDIR)/bootmode1
	[ "$(SC_INSTALL_DIR)" ] || ( echo "SC_INSTALL_DIR environment variable is unset"; exit 1 )
	[ -d $(SC_INSTALL_DIR) ] || ( echo "SoftConsole subdirectory >>$(SC_INSTALL_DIR)<< does not exist."; exit 2 )
	$(SC_INSTALL_DIR)/eclipse/jre/bin/java -jar $(SC_INSTALL_DIR)/extras/mpfs/mpfsBootmodeProgrammer.jar --workdir `pwd`/Default --die $(DIE) --package $(PACKAGE) --bootmode $(BOOTMODE)
