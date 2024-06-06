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
# OpenSPI Service

export OPENSBI_SRC_DIR=thirdparty/opensbi
SRCS-$(CONFIG_SERVICE_OPENSBI) += \
	services/opensbi/opensbi_service.c \
	$(OPENSBI_SRC_DIR)/lib/utils/irqchip/plic.c \
	$(OPENSBI_SRC_DIR)/lib/utils/libfdt/fdt.c \
	$(OPENSBI_SRC_DIR)/lib/utils/libfdt/fdt_ro.c \
	$(OPENSBI_SRC_DIR)/lib/utils/libfdt/fdt_rw.c \
	$(OPENSBI_SRC_DIR)/lib/utils/libfdt/fdt_wip.c \
	$(OPENSBI_SRC_DIR)/lib/utils/libfdt/fdt_addresses.c \
	$(OPENSBI_SRC_DIR)/lib/utils/fdt/fdt_fixup.c \
	$(OPENSBI_SRC_DIR)/lib/utils/fdt/fdt_helper.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/riscv_asm.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/riscv_atomic.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/riscv_locks.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_bitmap.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_bitops.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_console.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_domain.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_emulate_csr.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_fifo.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_hart.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_hsm.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_illegal_insn.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_init.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_ipi.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_irqchip.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_math.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_misaligned_ldst.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_platform.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_pmu.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_scratch.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_string.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_system.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_timer.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_tlb.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_trap.c \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_unpriv.c \
	$(OPENSBI_SRC_DIR)/lib/utils/timer/aclint_mtimer.c \
	$(OPENSBI_SRC_DIR)/lib/utils/ipi/aclint_mswi.c \

ifdef CONFIG_SERVICE_OPENSBI
EXTRA_OBJS += $(BINDIR)/services/opensbi/opensbi_ecall_exts.o
endif

SRCS-$(CONFIG_SERVICE_OPENSBI) += $(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_base.c
SRCS-$(CONFIG_SBI_ECALL_HSM) += $(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_hsm.c
SRCS-$(CONFIG_SBI_ECALL_IPI) += $(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_ipi.c
SRCS-$(CONFIG_SBI_ECALL_LEGACY) += $(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_legacy.c
SRCS-$(CONFIG_SBI_ECALL_PMU) += $(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_pmu.c
SRCS-$(CONFIG_SBI_ECALL_RFENCE) += $(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_rfence.c
SRCS-$(CONFIG_SBI_ECALL_SRST) += $(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_srst.c
SRCS-$(CONFIG_SBI_ECALL_TIME) += $(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_time.c
SRCS-$(CONFIG_SBI_ECALL_VENDOR) += $(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_vendor.c

carray-sbi_ecall_exts-y += ecall_base
carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_HSM) += ecall_hsm
carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_IPI) += ecall_ipi
carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_LEGACY) += ecall_legacy
carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_PMU) += ecall_pmu
carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_RFENCE) += ecall_rfence
carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_SRST) += ecall_srst
carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_TIME) += ecall_time
carray-sbi_ecall_exts-$(CONFIG_SBI_ECALL_VENDOR) += ecall_vendor

$(BINDIR)/services/opensbi/opensbi_ecall_exts.c: $(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_exts.carray $(DOT_CONFIG)
	echo " CARRAY    $<"
	if [ ! -d `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(OPENSBI_SRC_DIR)/scripts/carray.sh -i $< -l "$(carray-sbi_ecall_exts-y)" > $@

$(BINDIR)/$(BINDIR)/services/opensbi/opensbi_ecall_exts.d: $(BINDIR)/services/opensbi/opensbi_ecall_exts.c
	touch $<

ifdef CONFIG_USE_IHC
SRCS-$(CONFIG_SERVICE_OPENSBI_IHC) += \
	services/opensbi/opensbi_ihc_ecall.c \

SRCS-$(CONFIG_SERVICE_OPENSBI_RPROC) += \
    services/opensbi/opensbi_rproc_ecall.c \

endif

ifdef CONFIG_USE_USER_CRYPTO
SRCS-$(CONFIG_SERVICE_OPENSBI_CRYPTO) += \
	services/opensbi/opensbi_crypto_ecall.c
endif

ASM_SRCS-$(CONFIG_SERVICE_OPENSBI) += \
	$(OPENSBI_SRC_DIR)/lib/sbi/riscv_hardfp.S \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_expected_trap.S \
	$(OPENSBI_SRC_DIR)/lib/sbi/sbi_hfence.S \

SRCS-$(CONFIG_SERVICE_OPENSBI) += \
	services/opensbi/platform.c \
	services/opensbi/opensbi_ecall.c

ifdef CONFIG_OPENSBI
INCLUDES += \
	-I./services/opensbi \
	-I./$(OPENSBI_SRC_DIR)/include/sbi \
	-I./$(OPENSBI_SRC_DIR)/include \
	-I./$(OPENSBI_SRC_DIR)/lib/utils/libfdt
endif

ifdef CONFIG_PROVIDE_DTB
ifdef CONFIG_DEFAULT_DEVICE_TREE
CONFIG_DEFAULT_DEVICE_TREE_UNQUOTED:=$(CONFIG_DEFAULT_DEVICE_TREE:"%"=%)
EXTRA_OBJS += $(BINDIR)/services/opensbi/mpfs_dtb.o
$(BINDIR)/services/opensbi/mpfs_dtb.o: $(CONFIG_DEFAULT_DEVICE_TREE_UNQUOTED)
	cp $(CONFIG_DEFAULT_DEVICE_TREE_UNQUOTED) $(BINDIR)/services/opensbi/mpfs.dtb
	$(CROSS_COMPILE)ld -r -b binary -o $@ $(BINDIR)/services/opensbi/mpfs.dtb
	rm $(BINDIR)/services/opensbi/mpfs.dtb

$(CONFIG_DEFAULT_DEVICE_TREE_UNQUOTED): $(CONFIG_DEFAULT_DEVICE_TREE_UNQUOTED:%.dtb=%.dts)
	$(error Error:: Rebuilding the DTB is not supported... Please rebuild manually)
endif
endif

$(BINDIR)/services/opensbi/opensbi_service.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/services/opensbi/opensbi_ihc_ecall.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/services/opensbi/platform.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/utils/irqchip/plic.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/utils/libfdt/fdt_rw.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/utils/libfdt/fdt_ro.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/utils/libfdt/fdt.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/utils/libfdt/fdt_overlay.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/utils/libfdt/fdt_strerror.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/utils/libfdt/fdt_sw.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/utils/libfdt/fdt_addresses.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/utils/libfdt/fdt_wip.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/utils/serial/sifive-uart.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/platform/sifive/fu540/platform.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/riscv_asm.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/riscv_atomic.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/riscv_locks.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_bitmap.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_bitops.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_console.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_domain.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_base.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_hsm.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_legacy.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_replace.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_vendor.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_emulate_csr.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_fifo.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_hart.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_hsm.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_illegal_insn.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_init.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_ipi.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_math.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_misaligned_ldst.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_platform.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_pmu.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_scratch.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_string.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_system.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_timer.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_tlb.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_trap.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_unpriv.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/utils/timer/aclint_mtimer.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/utils/ipi/aclint_mswi.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_pmu.o: CFLAGS=$(CFLAGS_GCCEXT)
$(BINDIR)/$(OPENSBI_SRC_DIR)/lib/sbi/sbi_ecall_srst.o: CFLAGS=$(CFLAGS_GCCEXT)
