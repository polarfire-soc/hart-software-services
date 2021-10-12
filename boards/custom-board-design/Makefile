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
# Defines target-specific build-rules variables, extra sources and include paths
#

$(info custom-board-design selected)

BINDIR=Default
TARGET-l2lim=hss-l2lim.elf
TARGET-envm-wrapper=hss-envm-wrapper.elf
RISCV_TARGET=$(TARGET-l2lim) $(TARGET-envm-wrapper)
TARGET:=$(RISCV_TARGET)

LINKER_SCRIPT-l2lim=boards/${BOARD}/hss-l2lim.ld

CONFIG_PLATFORM_MPFS=y
PLATFORM_CFLAGS += -DCONFIG_PLATFORM_MPFS=1
BOARD_DIR=boards/custom-board-design

INCLUDES += \
	-I$(BOARD_DIR)/mpfs_hal_config/\
	-I$(BOARD_DIR)/soc_config/\
	-I$(BOARD_DIR)/ \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform \

EXTRA_SRCS-y += \
	$(BOARD_DIR)/hss_uart_init.c \
        $(BOARD_DIR)/uart_helper.c \
        $(BOARD_DIR)/hss_board_init.c \

$(BOARD_DIR)/hss_uart_init.o: CFLAGS=$(CFLAGS_GCCEXT)

EXTRA_OBJS-$(CONFIG_SERVICE_BOOT_USE_PAYLOAD) += $(BOARD_DIR)/payload.o
$(BOARD_DIR)/payload.o: $(BOARD_DIR)/payload.bin
	$(LD) -r -b binary $< -o $@

################################################################################################
#
# Extra hardware dependency rules for YMODEM
#

INCLUDES += \
	-Ibaremetal/ \
	-Ibaremetal/drivers/winbond_w25n01gv \

EXTRA_SRCS-$(CONFIG_SERVICE_YMODEM) += \
	baremetal/drivers/mss_envm/mss_envm.c

baremetal/drivers/winbond_w25n01gv/winbond_w25n01gv.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/drivers/micron_mt25q/micron_mt25q.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/drivers/mss_envm/mss_envm.o: CFLAGS=$(CFLAGS_GCCEXT)

################################################################################################
#
# Linker Scripts
#

$(BOARD_DIR)/hss-l2lim.ld: $(BOARD_DIR)/hss-l2lim.lds config.h

################################################################################################

#
# Extra dependency rules for auto-generated configuration files (from Libero XML)
#

SOC_CONFIG_FILES =  \
	$(BOARD_DIR)/soc_config/clocks/hw_clk_ddr_pll.h \
	$(BOARD_DIR)/soc_config/clocks/hw_clk_mss_cfm.h \
	$(BOARD_DIR)/soc_config/clocks/hw_clk_mss_pll.h \
	$(BOARD_DIR)/soc_config/clocks/hw_clk_sgmii_cfm.h \
	$(BOARD_DIR)/soc_config/clocks/hw_clk_sgmii_pll.h \
	$(BOARD_DIR)/soc_config/clocks/hw_clk_sysreg.h \
	$(BOARD_DIR)/soc_config/clocks/hw_mss_clks.h \
	$(BOARD_DIR)/soc_config/ddr/hw_ddr_io_bank.h \
	$(BOARD_DIR)/soc_config/ddr/hw_ddr_mode.h \
	$(BOARD_DIR)/soc_config/ddr/hw_ddr_off_mode.h \
	$(BOARD_DIR)/soc_config/ddr/hw_ddr_options.h \
	$(BOARD_DIR)/soc_config/ddr/hw_ddr_segs.h \
	$(BOARD_DIR)/soc_config/ddr/hw_ddrc.h \
	$(BOARD_DIR)/soc_config/general/hw_gen_peripherals.h \
	$(BOARD_DIR)/soc_config/hw_platform.h \
	$(BOARD_DIR)/soc_config/io/hw_hsio_mux.h \
	$(BOARD_DIR)/soc_config/io/hw_mssio_mux.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_apb_split.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_cache.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_memory.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_mpu_crypto.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_mpu_fic0.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_mpu_fic1.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_mpu_fic2.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_mpu_gem0.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_mpu_gem1.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_mpu_mmc.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_mpu_scb.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_mpu_trace.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_mpu_usb.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_pmp_hart0.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_pmp_hart1.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_pmp_hart2.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_pmp_hart3.h \
	$(BOARD_DIR)/soc_config/memory_map/hw_pmp_hart4.h \
	$(BOARD_DIR)/soc_config/sgmii/hw_sgmii_tip.h \

config.h: $(SOC_CONFIG_FILES)
$(SOC_CONFIG_FILES): $(BOARD_DIR)/soc_fpga_design/xml/custom_design_cfg.xml
	@$(ECHO) " CFGGEN    $<";
	$(PYTHON) tools/polarfire-soc-configuration-generator/mpfs_configuration_generator.py $< $(BOARD_DIR)

$(RISCV_TARGET): $(SOC_CONFIG_FILES)
