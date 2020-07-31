#
# MPFS HSS Embedded Software
#
# Copyright 2019 Microchip Corporation.
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

#
# Confusingly, the ICICLE-KIT-ES is platform MPFS
#
$(info ICICLE-KIT-ES selected)

CONFIG_PLATFORM_MPFS=y
PLATFORM_CFLAGS += -DCONFIG_PLATFORM_MPFS=1

INCLUDES += \
	-Iboards/icicle-kit-es/mpfs_hal_config/\
	-Iboards/icicle-kit-es/soc_config/\
	-Iboards/icicle-kit-es/ \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform \

EXTRA_SRCS-y += \
	boards/icicle-kit-es/hss_uart_init.c \
        boards/icicle-kit-es/uart_helper.c \
        boards/icicle-kit-es/hss_board_init.c \

boards/icicle-kit-es/hss_uart_init.o: boards/icicle-kit-es/hss_uart_init.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<


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
	baremetal/drivers/winbond_w25n01gv/winbond_w25n01gv.c \
	baremetal/drivers/mss_envm/mss_envm.c

#	baremetal/drivers/micron_mt25q/micron_mt25q.c \

baremetal/drivers/winbond_w25n01gv/winbond_w25n01gv.o: baremetal/drivers/winbond_w25n01gv/winbond_w25n01gv.c config.h
	@$(ECHO) " CC        $@";
	@$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/drivers/micron_mt25q/micron_mt25q.o: baremetal/drivers/micron_mt25q/micron_mt25q.c config.h
	@$(ECHO) " CC        $@";
	@$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/drivers/mss_envm/mss_envm.o: baremetal/drivers/mss_envm/mss_envm.c config.h
	@$(ECHO) " CC        $@";
	@$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<


################################################################################################

#
# Extra dependency rules for auto-generated configuration files (from Libero XML)
#

SOC_CONFIG_FILES =  \
	boards/icicle-kit-es/soc_config/clocks/hw_clk_ddr_pll.h \
	boards/icicle-kit-es/soc_config/clocks/hw_clk_mss_cfm.h \
	boards/icicle-kit-es/soc_config/clocks/hw_clk_mss_pll.h \
	boards/icicle-kit-es/soc_config/clocks/hw_clk_sgmii_cfm.h \
	boards/icicle-kit-es/soc_config/clocks/hw_clk_sgmii_pll.h \
	boards/icicle-kit-es/soc_config/clocks/hw_clk_sysreg.h \
	boards/icicle-kit-es/soc_config/clocks/hw_mss_clks.h \
	boards/icicle-kit-es/soc_config/ddr/hw_ddr_io_bank.h \
	boards/icicle-kit-es/soc_config/ddr/hw_ddr_mode.h \
	boards/icicle-kit-es/soc_config/ddr/hw_ddr_off_mode.h \
	boards/icicle-kit-es/soc_config/ddr/hw_ddr_options.h \
	boards/icicle-kit-es/soc_config/ddr/hw_ddr_segs.h \
	boards/icicle-kit-es/soc_config/ddr/hw_ddrc.h \
	boards/icicle-kit-es/soc_config/general/hw_gen_peripherals.h \
	boards/icicle-kit-es/soc_config/hw_platform.h \
	boards/icicle-kit-es/soc_config/io/hw_hsio_mux.h \
	boards/icicle-kit-es/soc_config/io/hw_mssio_mux.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_apb_split.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_cache.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_memory.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_mpu_crypto.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_mpu_fic0.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_mpu_fic1.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_mpu_fic2.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_mpu_gem0.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_mpu_gem1.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_mpu_mmc.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_mpu_scb.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_mpu_trace.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_mpu_usb.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_pmp_hart0.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_pmp_hart1.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_pmp_hart2.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_pmp_hart3.h \
	boards/icicle-kit-es/soc_config/memory_map/hw_pmp_hart4.h \
	boards/icicle-kit-es/soc_config/sgmii/hw_sgmii_tip.h \

$(SOC_CONFIG_FILES): boards/icicle-kit-es/soc_fpga_design/xml/ICICLE_MSS_0.xml
	@$(ECHO) " CFGGEN    $<";
	@$(CMD_PREFIX)python3 tools/polarfire-soc-configuration-generator/mpfs_configuration_generator.py $< boards/icicle-kit-es

$(RISCV_TARGET): $(SOC_CONFIG_FILES)
