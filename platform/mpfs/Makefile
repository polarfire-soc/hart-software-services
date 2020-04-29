$(info MPFS selected)

CONFIG_PLATFORM_MPFS=y
PLATFORM_CFLAGS += -DCONFIG_PLATFORM_MPFS=1

INCLUDES += \
	-Ibaremetal/polarfire-soc-bare-metal-library/src/platform \
	-I./baremetal/polarfire-soc-bare-metal-library/src/platform/config/hardware

CONFIG_SERVICE_BOOT_PAYLOAD_OBJECT_FILE="platform/mpfs/payload.o"

EXTRA_SRCS-y += \
	platform/mpfs/hss_uart_init.c \
        platform/mpfs/uart_helper.c

platform/mpfs/hss_uart_init.o: platform/mpfs/hss_uart_init.c config.h
	@$(ECHO) " CC        $@";
	$(CMD_PREFIX)$(CC) $(CFLAGS_GCCEXT) $(OPT-y) $(INCLUDES) -c -o $@ $< 
