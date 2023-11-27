#
# MPFS HSS Embedded Software
#
# Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
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

TARGET-l2scratch=hss-l2scratch.elf
TARGET-envm-wrapper=hss-envm-wrapper.elf
RISCV_TARGET=$(TARGET-l2scratch) $(TARGET-envm-wrapper)
TARGET:=$(RISCV_TARGET)

LINKER_SCRIPT-l2scratch=boards/${BOARD}/hss-l2scratch.ld

BOARD_DIR=boards/custom-board-design

INCLUDES += \
	-I$(BOARD_DIR)/mpfs_hal_config/ \
	-I$(BINDIR)/$(BOARD_DIR)/fpga_design_config/ \
	-I$(BINDIR)/$(BOARD_DIR)/ \
	-I$(BOARD_DIR)/ \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform \

EXTRA_SRCS-y += \
	$(BOARD_DIR)/hss_uart_init.c \
        $(BOARD_DIR)/uart_device_list.c \
        $(BOARD_DIR)/hss_board_init.c \

EXTRA_SRCS-$(CONFIG_USE_LOGO) += \
        $(BOARD_DIR)/hss_logo_init.c

$(BOARD_DIR)/hss_uart_init.o: CFLAGS=$(CFLAGS_GCCEXT)

EXTRA_OBJS-$(CONFIG_SERVICE_BOOT_USE_PAYLOAD) += $(BOARD_DIR)/payload.o
$(BOARD_DIR)/payload.o: $(BOARD_DIR)/payload.bin
	$(LD) -r -b binary $< -o $@

################################################################################################
#
# Extra hardware dependency rules for QSPI
#

INCLUDES += \
	-Ibaremetal/ \

ifdef CONFIG_SERVICE_QSPI_WINBOND_W25N01GV
INCLUDES += \
	-Ibaremetal/drivers/winbond_w25n01gv \

EXTRA_SRCS-$(CONFIG_SERVICE_QSPI) += \
	baremetal/drivers/winbond_w25n01gv/winbond_w25n01gv.c

baremetal/drivers/winbond_w25n01gv/winbond_w25n01gv.o: CFLAGS=$(CFLAGS_GCCEXT)
endif

ifdef CONFIG_SERVICE_QSPI_MICRON_MQ25T
INCLUDES += \
	-Ibaremetal/drivers/micron_mt25q \

EXTRA_SRCS-$(CONFIG_SERVICE_QSPI) += \
	baremetal/drivers/micron_mt25q/micron_mt25q.c
baremetal/drivers/micron_mt25q/micron_mt25q.o: CFLAGS=$(CFLAGS_GCCEXT)
endif


################################################################################################
#
# Linker Scripts
#

$(BOARD_DIR)/hss-l2scratch.ld: $(BOARD_DIR)/hss-l2scratch.lds $(CONFIG_H)

################################################################################################

#
# Extra dependency rules for auto-generated configuration files (from Libero XML)
#

SOC_CONFIG_FILES =  \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/clocks/hw_clk_ddr_pll.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/clocks/hw_clk_mss_cfm.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/clocks/hw_clk_mss_pll.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/clocks/hw_clk_sgmii_cfm.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/clocks/hw_clk_sgmii_pll.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/clocks/hw_clk_sysreg.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/clocks/hw_mss_clks.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/ddr/hw_ddr_io_bank.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/ddr/hw_ddr_mode.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/ddr/hw_ddr_off_mode.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/ddr/hw_ddr_options.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/ddr/hw_ddr_segs.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/ddr/hw_ddrc.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/general/hw_gen_peripherals.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/fpga_design_config.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/io/hw_hsio_mux.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/io/hw_mssio_mux.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_apb_split.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_cache.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_memory.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_mpu_crypto.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_mpu_fic0.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_mpu_fic1.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_mpu_fic2.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_mpu_gem0.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_mpu_gem1.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_mpu_mmc.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_mpu_scb.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_mpu_trace.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_mpu_usb.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_pmp_hart0.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_pmp_hart1.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_pmp_hart2.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_pmp_hart3.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/memory_map/hw_pmp_hart4.h \
	$(BINDIR)/$(BOARD_DIR)/fpga_design_config/sgmii/hw_sgmii_tip.h \

ifeq ($(origin CONFIG_SOC_FPGA_DESIGN_XML), undefined)
else
SOC_FPGA_DESIGN_XML_FILE= $(subst $\",,$(CONFIG_SOC_FPGA_DESIGN_XML))
ifeq ("$(wildcard $(SOC_FPGA_DESIGN_XML_FILE))", "")
$(error "XML file $(CONFIG_SOC_FPGA_DESIGN_XML) specified by CONFIG_SOC_FPGA_DESIGN_XML does not exist")
endif
endif

config.h: $(SOC_CONFIG_FILES)
$(SOC_CONFIG_FILES): $(SOC_FPGA_DESIGN_XML_FILE)
	@$(ECHO) " MPFSCFGGEN    $<";
	$(PYTHON) tools/polarfire-soc-configuration-generator/mpfs_configuration_generator.py $< $(BINDIR)/$(BOARD_DIR)

$(RISCV_TARGET): $(SOC_CONFIG_FILES)
