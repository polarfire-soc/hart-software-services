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
    baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_qspi/mss_qspi.c \
    baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_mmuart/mss_uart.c \
    baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_pdma/mss_pdma.c \
    baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_util.c \
    baremetal/drivers/mss_watchdog/mss_watchdog.c \

INCLUDES += \
    -I./baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_qspi \
    -I./baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_mmuart \
    -I./baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_pdma \
    -I./baremetal/drivers/mss_watchdog \
    -I./baremetal/polarfire-soc-bare-metal-library/src/platform/ \
    -I./baremetal/polarfire-soc-bare-metal-library/src/platform/hal \
    -I./baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/ \
    -I./baremetal/polarfire-soc-bare-metal-library/src/platform/config/software/mpfs_hal/ \
    -I./baremetal/polarfire-soc-bare-metal-library/src/platform/config/hardware/

baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_qspi/mss_qspi.o: baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_qspi/mss_qspi.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

# TODO : for now, passing MSS clock in via define to HAL... This hardcode needs to be removed
baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_mmuart/mss_uart.o: baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_mmuart/mss_uart.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<  

baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_pdma/mss_pdma.o: baremetal/polarfire-soc-bare-metal-library/src/platform/drivers/mss_pdma/mss_pdma.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $<  

baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_util.o: baremetal/polarfire-soc-bare-metal-library/src/platform/mpfs_hal/mss_util.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

baremetal/drivers/mss_watchdog/mss_watchdog.o: baremetal/drivers/mss_watchdog/mss_watchdog.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

baremetal/stubs.o: baremetal/stubs.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

#
# Extra hardware dependency rules for YMODEM
#
INCLUDES += \
    -I./baremetal/drivers/mss_envm \

SRCS-$(CONFIG_SERVICE_YMODEM) += \
    baremetal/drivers/micron_mt25q/micron_mt25q.c \
    baremetal/drivers/mss_envm/mss_envm.c \

baremetal/drivers/micron_mt25q/micron_mt25q.o: baremetal/drivers/micron_mt25q/micron_mt25q.c config.h 
	@$(ECHO) " CC        $@";
	@$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

baremetal/drivers/mss_envm/mss_envm.o: baremetal/drivers/mss_envm/mss_envm.c config.h
	@$(ECHO) " CC        $@";
	@$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 
