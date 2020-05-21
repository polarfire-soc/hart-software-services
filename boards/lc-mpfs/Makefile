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

$(info Board LC-MPFS selected)

#
# Confusingly, the LC-MPFS board is actually the SiFive FU540 platform
#
CONFIG_PLATFORM_FU540=y
PLATFORM_CFLAGS += -DCONFIG_PLATFORM_FU540=1 -DSIFIVE_HIFIVE_UNLEASHED

BOARD_DIR=boards/lc-mpfs

INCLUDES += \
	-I$(BOARD_DIR)/config/hardware \
	-I$(BOARD_DIR)/ \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform \

EXTRA_SRCS-y += \
	$(BOARD_DIR)/hss_uart_init.c \
        $(BOARD_DIR)/uart_helper.c \
        $(BOARD_DIR)/hss_board_init.c \

$(BOARD_DIR)/hss_uart_init.o: $(BOARD_DIR)/hss_uart_init.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

EXTRA_OBJS-$(CONFIG_SERVICE_BOOT_USE_PAYLOAD) += $(BOARD_DIR)/payload.o
$(BOARD_DIR)/payload.o: $(BOARD_DIR)/payload.bin
	$(LD) -r -b binary $< -o $@
