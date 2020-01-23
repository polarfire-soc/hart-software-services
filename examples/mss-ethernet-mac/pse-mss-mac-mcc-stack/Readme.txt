This program implements a test shell for the PSE MSS MAC Ethernet driver which
can be used to exercise the driver on the G5 SoC Emulation Platform with the
Peripheral Daughter Board or the FU540 based Aloe board.

The following project defines are used to configure the system:

G5_SOC_EMU_USE_GEM0 - Define this to test GEM 0
G5_SOC_EMU_USE_GEM0 - Define this to test GEM 1

TI_PHY - Use G5 SoC Emulation Platform onboard PHY Otherwise VSC8575 PHY is used.

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
TARGET_ALOE      - Define when building for FU540 based Aloe board

SIFIVE_HIFIVE_UNLEASHED=0  - Define when building for FU540 based Aloe board

TARGET_G5_SOC   - Define when building for G5 SoC Emulation Platform board.
PSE=1           - Define as 1 when building for G5 SoC Emulation Platform
                  board 0 otherwise.
                  
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

When working with the G5 SOC Emulation system, the serial port baud rate is 115200.
When working with the Aloe board, the serial port baud rate is 1728000.

The test program responds to the following commands:

a - Initiate a PHY autonegotiation cycle
b - Toggle HW Loopback mode. The default is disabled.
B - Toggle broadcast RX mode. The default is to receive broadcasts.
c - Arms the single packet capture feature. A hex dump of the next received
    packet will be displayed via the serial port.
d - Toggle TX Cutthru. The default is disabled.
D - Toggle RX Cutthru. The default is disabled.
g - Display MSS GPIO 2 input values.
h - Display this help information.
i - Increments the GEM statistics counters using the test feature in the Network
    Control Register.
j - Toggle Jumbo Packet Mode. The default is disabled. If Jumbo packet mode is
    disabled, packets larger than 1536 bytes will not be received.
k - Toggle capture re-trigger mode. The default is disabled. When re-trigger
    mode is enabled, the software will automatically re-enable packet capture
    after displaying a captured packet.
l - Toggles software loopback mode - all received packets are immediately
    re-transmitted. The default is disabled.
L - Toggle SGMII Link Status display mode. When enabled, the display will
    rapidly fill up with repeated + or - characters to show the current link
    status (up or down). The default is disabled.
n - When in loopback mode, toggles adding of an extra byte to the re-transmitted
    packets. This can be useful when examining Wireshark captures as it provides
    a way of identifying the echoed packets.
p - Toggles promiscuous mode. In promiscuous mode the MAC will receive all
    packets but in normal mode it will only receive broadcast packets and
    packets addressed directly to its MAC address which defaults to 
    00:FC:00:12:34:56.
r - Clears the statistics counts.
s - Displays the GEM statistics, some PHY statistics and some internal software
    generated statistics.
t - Transmits sample ARP packet.
T - Transmits sample ARP packet with padding that increments after every
    transmit. Packet length varies between 64 and 128 bytes and loops back to 64
    bytes after transmitting 128 byte packet.
x - Toggles reading of PHY registers. When disabled, the PHY statistics are not
    updated when displaying the statistics. This may help when testing with high
    network loads.
z - Toggle FCS passthrough mode. When FCS passthrough is enabled, the receive
    packet FCS not checked and is copied to memory. For transmit packets, the 
    GEM assumes the packet already has an FCS and does not append one. The
    default is disabled.
' - Transmit a standard Pause Frame.
, - Transmit a Zero Quantum Pause Frame.
. - Transmit a Priority Based Pause Frame.
+/- Increment/Decrement length adjust. Adjusts the additional length added to
    loopbacked packets when the 'n' command is active.

When creating a debug session for this project the following settings are
required to create a debug session for the e51 hart only:

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