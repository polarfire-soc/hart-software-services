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
# Baremetal Drivers and HAL Support

EXTRA_SRCS-y += \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_pdma/mss_pdma.c \

ifdef CONFIG_PLATFORM_MPFS
EXTRA_SRCS-$(CONFIG_SERVICE_QSPI) += \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_qspi/mss_qspi.c \

EXTRA_SRCS-$(CONFIG_SERVICE_MMC) += \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_mmc/mss_mmc.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_mmc/mss_mmc_if.c \

EXTRA_SRCS-$(CONFIG_USE_IHC) += \
	baremetal/drivers/fpga_ip/miv_ihc/miv_ihc.c \

endif

EXTRA_SRCS-$(CONFIG_PLATFORM_MPFS) += \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_uart/mss_uart.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_sys_services/mss_sys_services.c \
	baremetal/drivers/mss_watchdog/mss_watchdog.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_mpu.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_peripherals.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_plic.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_l2_cache.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_util.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_nwc_init.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_ddr.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_ddr_debug.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_ddr_test_pattern.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_io.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_pll.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_pmp.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_sgmii.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/startup_gcc/system_startup.c \

ASM_SRCS += \
 	baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/startup_gcc/mss_utils.S \

	# baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/startup_gcc/mss_mutex.S \

INCLUDES += \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_uart \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_mmc \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_qspi \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_pdma \
        -Ibaremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_sys_services \
	-Ibaremetal/drivers/mss_watchdog \
	-Ibaremetal/drivers/fpga_ip/miv_ihc \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/hal \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/ \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/startup_gcc \

baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_mmc/mss_mmc.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_qspi/mss_qspi.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_uart/mss_uart.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_pdma/mss_pdma.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/drivers/mss_watchdog/mss_watchdog.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/drivers/fpga_ip/miv_ihc/miv_ihc.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/stubs.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/startup_gcc/system_startup.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_nwc_init.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_ddr.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_ddr_debug.o: CFLAGS=$(CFLAGS_GCCEXT) -DHSS
baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_io.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_pll.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/nwc/mss_sgmii.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_l2_cache.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_util.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_sys_services/mss_sys_services.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_mpu.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_peripherals.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_plic.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/common/mss_pmp.o: CFLAGS=$(CFLAGS_GCCEXT)

#################################################################################################
#
# USB

ifdef CONFIG_PLATFORM_MPFS
INCLUDES += \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_gpio \

SRCS-$(CONFIG_SERVICE_USBDMSC) += \
	baremetal/drivers/mss_usb/mss_usb_common_cif.c \
	baremetal/drivers/mss_usb/mss_usb_device.c \
	baremetal/drivers/mss_usb/mss_usb_device_cif.c \
	baremetal/drivers/mss_usb/mss_usb_device_msd.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/hal/hal_irq.c \
	baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_gpio/mss_gpio.c \

endif

baremetal/drivers/mss_usb/mss_usb_common_cif.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/drivers/mss_usb/mss_usb_device.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/drivers/mss_usb/mss_usb_device_cif.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/drivers/mss_usb/mss_usb_device_msd.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/hal/hal_irq.o: CFLAGS=$(CFLAGS_GCCEXT)
baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_gpio/mss_gpio.o: CFLAGS=$(CFLAGS_GCCEXT)
