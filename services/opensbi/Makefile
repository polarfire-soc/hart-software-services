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

SRCS-$(CONFIG_SERVICE_OPENSBI) += \
	services/opensbi/opensbi_service.c \
	thirdparty/opensbi/lib/utils/sys/clint.c \
	thirdparty/opensbi/lib/utils/irqchip/plic.c \
	thirdparty/opensbi/lib/utils/libfdt/fdt.c \
	thirdparty/opensbi/lib/utils/libfdt/fdt_ro.c \
	thirdparty/opensbi/lib/utils/libfdt/fdt_rw.c \
	thirdparty/opensbi/lib/utils/libfdt/fdt_wip.c \
	thirdparty/opensbi/lib/utils/libfdt/fdt_addresses.c \
	thirdparty/opensbi/lib/utils/fdt/fdt_fixup.c \
	thirdparty/opensbi/lib/utils/fdt/fdt_helper.c \
	thirdparty/opensbi/lib/sbi/riscv_asm.c \
	thirdparty/opensbi/lib/sbi/riscv_atomic.c \
	thirdparty/opensbi/lib/sbi/riscv_locks.c \
	thirdparty/opensbi/lib/sbi/sbi_bitmap.c \
	thirdparty/opensbi/lib/sbi/sbi_bitops.c \
	thirdparty/opensbi/lib/sbi/sbi_console.c \
	thirdparty/opensbi/lib/sbi/sbi_domain.c \
	thirdparty/opensbi/lib/sbi/sbi_ecall_base.c \
	thirdparty/opensbi/lib/sbi/sbi_ecall.c \
	thirdparty/opensbi/lib/sbi/sbi_ecall_hsm.c \
	thirdparty/opensbi/lib/sbi/sbi_ecall_legacy.c \
	thirdparty/opensbi/lib/sbi/sbi_ecall_replace.c \
	thirdparty/opensbi/lib/sbi/sbi_ecall_vendor.c \
	thirdparty/opensbi/lib/sbi/sbi_emulate_csr.c \
	thirdparty/opensbi/lib/sbi/sbi_fifo.c \
	thirdparty/opensbi/lib/sbi/sbi_hart.c \
	thirdparty/opensbi/lib/sbi/sbi_hsm.c \
	thirdparty/opensbi/lib/sbi/sbi_illegal_insn.c \
	thirdparty/opensbi/lib/sbi/sbi_init.c \
	thirdparty/opensbi/lib/sbi/sbi_ipi.c \
	thirdparty/opensbi/lib/sbi/sbi_math.c \
	thirdparty/opensbi/lib/sbi/sbi_misaligned_ldst.c \
	thirdparty/opensbi/lib/sbi/sbi_platform.c \
	thirdparty/opensbi/lib/sbi/sbi_scratch.c \
	thirdparty/opensbi/lib/sbi/sbi_string.c \
	thirdparty/opensbi/lib/sbi/sbi_system.c \
	thirdparty/opensbi/lib/sbi/sbi_timer.c \
	thirdparty/opensbi/lib/sbi/sbi_tlb.c \
	thirdparty/opensbi/lib/sbi/sbi_trap.c \
	thirdparty/opensbi/lib/sbi/sbi_unpriv.c \

SRCS-$(CONFIG_SERVICE_OPENSBI_IHC) += \
	services/opensbi/opensbi_ihc_ecall.c \

ASM_SRCS-$(CONFIG_SERVICE_OPENSBI) += \
	thirdparty/opensbi/lib/sbi/riscv_hardfp.S \
	thirdparty/opensbi/lib/sbi/sbi_expected_trap.S \
	thirdparty/opensbi/lib/sbi/sbi_hfence.S \

SRCS-$(CONFIG_SERVICE_OPENSBI) += \
	services/opensbi/platform.c \
	services/opensbi/opensbi_ecall.c

ifdef CONFIG_OPENSBI
INCLUDES += \
	-I./services/opensbi \
	-I./thirdparty/opensbi/include/sbi \
	-I./thirdparty/opensbi/include \
	-I./thirdparty/opensbi/lib/utils/libfdt
endif

ifdef CONFIG_PROVIDE_DTB
ifdef CONFIG_DEFAULT_DEVICE_TREE
EXTRA_OBJS += services/opensbi/mpfs_dtb.o
services/opensbi/mpfs_dtb.o: $(CONFIG_DEFAULT_DEVICE_TREE:"%"=%).dtb
	$(CROSS_COMPILE)ld -r -b binary -o $@ $<
endif
endif

services/opensbi/opensbi_service.o: CFLAGS=$(CFLAGS_GCCEXT)
services/opensbi/opensbi_ihc_ecall.o: CFLAGS=$(CFLAGS_GCCEXT)
services/opensbi/platform.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/utils/sys/clint.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/utils/irqchip/plic.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/utils/libfdt/fdt_rw.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/utils/libfdt/fdt_ro.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/utils/libfdt/fdt.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/utils/libfdt/fdt_overlay.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/utils/libfdt/fdt_strerror.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/utils/libfdt/fdt_sw.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/utils/libfdt/fdt_addresses.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/utils/libfdt/fdt_wip.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/utils/serial/sifive-uart.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/platform/sifive/fu540/platform.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/riscv_asm.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/riscv_atomic.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/riscv_locks.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_bitmap.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_bitops.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_console.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_domain.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_ecall_base.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_ecall.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_ecall_hsm.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_ecall_legacy.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_ecall_replace.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_ecall_vendor.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_emulate_csr.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_fifo.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_hart.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_hsm.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_illegal_insn.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_init.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_ipi.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_math.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_misaligned_ldst.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_platform.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_scratch.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_string.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_system.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_timer.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_tlb.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_trap.o: CFLAGS=$(CFLAGS_GCCEXT)
thirdparty/opensbi/lib/sbi/sbi_unpriv.o: CFLAGS=$(CFLAGS_GCCEXT)
