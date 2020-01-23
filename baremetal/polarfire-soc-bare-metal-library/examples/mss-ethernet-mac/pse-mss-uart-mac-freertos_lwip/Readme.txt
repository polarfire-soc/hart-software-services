=================================================================
 PolarFire SoC MSS Ethernet MAC Test program - Simple web server 
=================================================================

This program implements a simple web server example using FreeRTOS and the
LwIP TCP/IP stack with the PolarFire SoC MSS MAC Ethernet driver. This can be
used to exercise the driver on the G5 SoC Emulation Platform with the Peripheral
Daughter Board or the FU540 based Aloe board.

The software has a fixed IP address of 10.2.2.20/255.255.255.0 If you point
a browser at that address you should see a simple web page display some
information about the system and a count that updates each time the page
auto refreshes.

Note: When testing this functionality, the recommended method is to use a
separate ethernet port on the PC with a fixed IP address in the 10.2.2.x range.
If your PC does not have multiple Ethernet ports, you can modify the IP address
settings of the Firmware to match your network address range or use a USB to
Ethernet adapter to provide a dedicated link for testing - we would recommend
using the USB to Ethernet adapter as this provides isolation from your normal
network.

The IP address for the web server can be changed by editing the
prvEthernetConfigureInterface() function in e51.c.

The following project defines are used to configure the system:

Define these to enable support for FreeRTOS and LwIP in the driver.

USING_FREERTOS
USING_LWIP

LwIP specific defines

LWIP_COMPAT_MUTEX
LWIP_SKIP_PACKING_CHECK
LWIP_PROVIDE_ERRNO

G5_SOC_EMU_USE_GEM0 - Define this to test GEM 0
G5_SOC_EMU_USE_GEM1 - Define this to test GEM 1

Configurations for VTSS API for VSC8575

VTSS_CHIP_CU_PHY
VTSS_FEATURE_SYNCE
VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION
VTSS_FEATURE_SERDES_MACRO_SETTINGS
VTSS_OPT_PORT_COUNT=4
VTSS_OPT_VCORE_III=0
VTSS_PRODUCT_CHIP="PHY"
VTSS_PHY_API_ONLY
VTSS_OPT_TRACE=0
VTSS_OS_BARE_METAL_RV

CMSIS_PROT
TARGET_ALOE         - Define when building for FU540 based Aloe board
SIFIVE_HIFIVE_UNLEASHED=0  - Define as 1 when building for FU540 based Aloe board
                      0 otherwise.

TARGET_G5_SOC   - Define when building for G5 SoC Emulation Platform
PSE=1           - Define as 1 when building for G5 SoC Emulation Platform, 0
                  otherwise.
                  
MSS_MAC_64_BIT_ADDRESS_MODE - Sets address bus width for DMA
MSS_MAC_SIMPLE_TX_QUEUE     - Set this, multipacket TX queue is not implemented
CALCONFIGH=\"config_user.h\"
TEST_H2F_CONTROLLER=0
_ZL303XX_MIV

The MSS_MAC_HW_PLATFORM macro should be defined in mss_ethernet_mac_user_config 
with a value that identifies the hardware platform and interface modes to use.

Valid values are:

    MSS_MAC_DESIGN_ALOE             - ALOE board from Sifive
    MSS_MAC_DESIGN_EMUL_GMII        - G5 Emulation Platform VSC8575 designs with GMII to SGMII bridge on GEM0
    MSS_MAC_DESIGN_EMUL_TBI         - G5 Emulation Platform VSC8575 designs with TBI to SGMII bridge on GEM0
    MSS_MAC_DESIGN_EMUL_TI_GMII     - G5 Emulation Platform DP83867 design with GMII to SGMII bridge
    MSS_MAC_DESIGN_EMUL_DUAL_EX_TI  - G5 Emulation Platform Dual GEM design with external TI PHY on GEM1 (GMII)
    MSS_MAC_DESIGN_EMUL_DUAL_EX_VTS - G5 Emulation Platform Dual GEM design with external Vitess PHY on GEM0 (GMII)
    MSS_MAC_DESIGN_EMUL_GMII_GEM1   - G5 Emulation Platform VSC8575 designs with GMII to SGMII bridge on GEM 1
    MSS_MAC_DESIGN_EMUL_TBI_GEM1    - G5 Emulation Platform VSC8575 designs with TBI to SGMII bridge GEM1
    MSS_MAC_DESIGN_EMUL_TBI_GEM1_TI - G5 Emulation Platform DP83867 designs with TBI to SGMII bridge GEM0

When working with the G5 SOC Emulation system, the serial port baud rate is
115200. When working with the Aloe board, the serial port baud rate is 1728000.

To use this project you will need a UART terminal configured as below:

    - 115200/1728000 baud
    - 8 data bits
    - 1 stop bit
    - no parity
    - no flow control

The serial interface can be used to confirm the application is running as it
will echo any typed characters.
 
When creating a debug session for this project the following settings are required
to create a debug session for the e51 hart only:

For G5 SOC target:

    In Debugger\OpenOCD Setup\Config options

    -c "set DEVICE G5SOC"
    -c "set JTAG_KHZ 6000"
    -d2
    --file board/microsemi-riscv.cfg

    In Debugger\GDB Client Setup\Commands
    
    set $target_riscv=1
    set mem inaccessible-by-default off
    set remotetimeout 15
    
    In Startup\Run/Restart Commands
    
    set *0x02000000=0
    set $pc=0x8000000
    set $mstatus = 0

For Aloe target:

    In Debugger\OpenOCD Setup\Config options

    --file board/microsemi-sifive-hifive-unleashed.cfg

    In Debugger\GDB Client Setup\Commands
    
    set $target_riscv=1
    set mem inaccessible-by-default off
    set remotetimeout 15
    
    In Startup\Run/Restart Commands
    
    set *0x02000000=0
    set *0x02000004=0
    set *0x02000008=0
    set *0x0200000C=0
    set *0x02000010=0
    set $pc=0x8000000
    thread 2
    set $pc=0x8000000
    set $mstatus = 0
    thread 3
    set $pc=0x8000000
    set $mstatus = 0
    thread 4
    set $pc=0x8000000
    set $mstatus = 0
    thread 5
    set $pc=0x8000000
    set $mstatus = 0