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
# Baremetal Drivers and HAL Support

EXTRA_SRCS-y += \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_pdma/mss_pdma.c \

ifdef CONFIG_PLATFORM_MPFS
EXTRA_SRCS-$(CONFIG_SERVICE_QSPI) += \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_qspi/mss_qspi.c \

EXTRA_SRCS-$(CONFIG_SERVICE_MMC) += \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_mmc/mss_mmc.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_mmc/mss_mmc_if.c \

endif

EXTRA_SRCS-$(CONFIG_PLATFORM_MPFS) += \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_uart/mss_uart.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_sys_services/mss_sys_services.c \
	baremetal/drivers/mss_watchdog/mss_watchdog.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_nwc_init.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_ddr.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_ddr_debug.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_io.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_pll.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_sgmii.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_l2_cache.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_util.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_stubs.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_stubs.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/system_startup.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_mpu.c \

EXTRA_SRCS-$(CONFIG_PLATFORM_FU540) += \
	baremetal/sysreg.c \

ASM_SRCS += \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_mutex.S

INCLUDES += \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_mmc \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_qspi \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_pdma \
        -Ibaremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_sys_services \
	-Ibaremetal/drivers/mss_watchdog \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/hal \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal \

baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_mmc/mss_mmc.o: baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_mmc/mss_mmc.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_qspi/mss_qspi.o: baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_qspi/mss_qspi.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_uart/mss_uart.o: baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_uart/mss_uart.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_pdma/mss_pdma.o: baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_pdma/mss_pdma.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/drivers/mss_watchdog/mss_watchdog.o: baremetal/drivers/mss_watchdog/mss_watchdog.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/stubs.o: baremetal/stubs.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/sysreg.o: baremetal/sysreg.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/system_startup.o: baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/system_startup.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_nwc_init.o: baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_nwc_init.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_ddr.o: baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_ddr.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -DDDR_SANITY_CHECKS_EN -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_ddr_debug.o: baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_ddr_debug.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -DDDR_SANITY_CHECKS_EN -c -DHSS -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_io.o: baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_io.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_pll.o: baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_pll.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_sgmii.o: baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/nwc/mss_sgmii.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_l2_cache.o: baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_l2_cache.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_util.o: baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_util.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_stubs.o: baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_stubs.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_sys_services/mss_sys_services.o: baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_sys_services/mss_sys_services.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_mpu.o: baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_mpu.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

#################################################################################################
#
# USB

ifdef CONFIG_PLATFORM_MPFS
SRCS-$(CONFIG_SERVICE_USBDMSC) += \
	baremetal/drivers/mss_usb/mss_usb_common_cif.c \
	baremetal/drivers/mss_usb/mss_usb_device.c \
	baremetal/drivers/mss_usb/mss_usb_device_cif.c \
	baremetal/drivers/mss_usb/mss_usb_device_msd.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/hal/hal_irq.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_gpio/mss_gpio.c \

endif

baremetal/drivers/mss_usb/mss_usb_common_cif.o: baremetal/drivers/mss_usb/mss_usb_common_cif.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/drivers/mss_usb/mss_usb_device.o: baremetal/drivers/mss_usb/mss_usb_device.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/drivers/mss_usb/mss_usb_device_cif.o: baremetal/drivers/mss_usb/mss_usb_device_cif.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/drivers/mss_usb/mss_usb_device_msd.o: baremetal/drivers/mss_usb/mss_usb_device_msd.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/hal/hal_irq.o: baremetal/polarfire-soc-bare-metal-library/src/platform/hal/hal_irq.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<

baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_gpio/mss_gpio.o: baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_gpio/mss_gpio.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<
