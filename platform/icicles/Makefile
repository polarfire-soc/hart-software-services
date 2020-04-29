$(info ICICLE selected)

CONFIG_PLATFORM_POLARFIRESOC=y
PLATFORM_CFLAGS += -DCONFIG_PLATFORM_POLARFIRESOC=1

INCLUDES += \
	-Iplatform/icicles/config/hardware \
	-Iplatform/icicles/ \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform \
	-I./baremetal/polarfire-soc-bare-metal-library/src/platform/config/hardware

EXTRA_SRCS-y += \
	platform/icicles/hss_uart_init.c \
        platform/icicles/uart_helper.c 

platform/icicles/hss_uart_init.o: platform/icicles/hss_uart_init.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 

