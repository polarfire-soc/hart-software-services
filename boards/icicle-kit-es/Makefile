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
