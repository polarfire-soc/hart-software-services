========================================================================
 PolarFire SoC MSS Ethernet MAC Test program - Single MAC, Single Queue 
========================================================================

This program implements a test shell for the PolarFire SoC MSS MAC Ethernet
driver which can be used to exercise the driver on the G5 SoC Emulation Platform
with the Peripheral Daughter Board or the FU540 based Aloe board.

The following project defines are used to configure the system:

G5_SOC_EMU_USE_GEM0 - Define this to test GEM 0
G5_SOC_EMU_USE_GEM1 - Define this to test GEM 1

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

The MSS_MAC_HW_PLATFORM macro should be defined in 
mss_ethernet_mac_user_config.h with a value that identifies the hardware
platform and interface modes to use.

Valid values are:

    MSS_MAC_DESIGN_ALOE             - ALOE board from Sifive
    MSS_MAC_DESIGN_EMUL_GMII        - G5 Emulation Platform VSC8575 designs with
                                      GMII to SGMII bridge on GEM0
    MSS_MAC_DESIGN_EMUL_TBI         - G5 Emulation Platform VSC8575 designs with
                                      TBI to SGMII bridge on GEM0
    MSS_MAC_DESIGN_EMUL_TI_GMII     - G5 Emulation Platform DP83867 design with
                                      GMII to SGMII bridge
    MSS_MAC_DESIGN_EMUL_DUAL_EX_TI  - G5 Emulation Platform Dual GEM design with
                                      external TI PHY on GEM1 (GMII)
    MSS_MAC_DESIGN_EMUL_DUAL_EX_VTS - G5 Emulation Platform Dual GEM design with
                                      external Vitess PHY on GEM0 (GMII)
    MSS_MAC_DESIGN_EMUL_GMII_GEM1   - G5 Emulation Platform VSC8575 designs with
                                      GMII to SGMII bridge on GEM 1
    MSS_MAC_DESIGN_EMUL_TBI_GEM1    - G5 Emulation Platform VSC8575 designs with
                                      TBI to SGMII bridge GEM1
    MSS_MAC_DESIGN_EMUL_TBI_GEM1_TI - G5 Emulation Platform DP83867 designs with
                                      TBI to SGMII bridge GEM0

When working with the G5 SOC Emulation system, the serial port baud rate is
115200. When working with the Aloe board, the serial port baud rate is 1728000.

To use this project you will need a UART terminal configured as below:

    - 115200/1728000 baud
    - 8 data bits
    - 1 stop bit
    - no parity
    - no flow control

The test program responds to the following single key commands via the serial
console:

a - Initiate a PHY autonegotiation cycle
b - Toggle HW Loopback mode. The default is disabled.
B - Toggle broadcast RX mode. The default is to receive broadcasts.
c - Arms the single packet capture feature. A hex dump of the next received
    packet will be displayed via the serial port.
d - Toggle TX Cutthru. The default is disabled.
D - Toggle RX Cutthru. The default is disabled.
g - Display MSS GPIO 2 input values.
h - Display command help information.
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
n - When in loopback mode, toggles adding of an extra byte(s) to the
    re-transmitted packets. This can be useful when examining Wireshark captures
    as it provides a way of identifying the echoed packets.
p - Toggles promiscuous mode. In promiscuous mode the MAC will receive all
    packets but in normal mode it will only receive broadcast packets and
    packets addressed directly to its MAC address - which defaults to 
    00:FC:00:12:34:56.
r - Clears the statistics counts.
s - Displays the GEM statistics, some PHY statistics and some internal software
    generated statistics.
t - Transmits sample ARP packet.
T - Transmits sample ARP packet with padding that increments after every
    transmit. Packet length varies between 64 and 128 bytes and loops back to 64
    bytes after transmitting a 128 byte packet.
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

This test program responds to the above single key commands and will display
responses as appropriate. At any time the list of available commands can be
viewed by pressing'h'. Thh help display will also show the current settings of
most of the options.

For example, typing the 4 command sequence "spkc" displays the current
statistic information, turns on promiscuous mode, turns on capture auto 
re-trigger mode and enables packet capture so that any received packets are
dumped to the console. This should produce a display something like this:

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PolarFire MSS Ethernet MAC Test program
Polling method for TXRX. Typed characters will be echoed.
Link is currently Up, Duplex = Full, Speed = 1Gb
RX 60 (1 pkts), TX 0 (0 pkts)
RX Over Flow 0, TX Retries 0
TX Pause 0, RX Pause 0, Pause Elapsed 0
HRESP not ok 0 RX Restarts 0

PHY Statistics
PHY CU RX GOOD                   0  PHY CU RX ERRORS                 0
PHY MAC RX GOOD                  0  PHY MAC RX ERRORS                0
PHY MAC TX GOOD                  0  PHY MAC TX ERRORS                0
PHY FALSE CARRIER ERR            0  PHY LINK DISCONNECTS             0

SGMII Registers
SGMII Control Register  0000
SGMII Status Register   0000
SGMII AN Advertisement  0000

GEM Statistics
TX_OCTETS_LOW                    0  TX_OCTETS_HIGH                   0
TX_FRAMES_OK                     0  TX_BCAST_FRAMES_OK               0
TX_MCAST_FRAMES_OK               0  TX_PAUSE_FRAMES_OK               0
TX_64_BYTE_FRAMES_OK             0  TX_65_BYTE_FRAMES_OK             0
TX_128_BYTE_FRAMES_OK            0  TX_256_BYTE_FRAMES_OK            0
TX_512_BYTE_FRAMES_OK            0  TX_1024_BYTE_FRAMES_OK           0
TX_1519_BYTE_FRAMES_OK           0  TX_UNDERRUNS                     0
TX_SINGLE_COLLISIONS             0  TX_MULTIPLE_COLLISIONS           0
TX_EXCESSIVE_COLLISIONS          0  TX_LATE_COLLISIONS               0
TX_DEFERRED_FRAMES               0  TX_CRS_ERRORS                    0
RX_OCTETS_LOW                   64  RX_OCTETS_HIGH                   0
RX_FRAMES_OK                     1  RX_BCAST_FRAMES_OK               1
RX_MCAST_FRAMES_OK               0  RX_PAUSE_FRAMES_OK               0
RX_64_BYTE_FRAMES_OK             1  RX_65_BYTE_FRAMES_OK             0
RX_128_BYTE_FRAMES_OK            0  RX_256_BYTE_FRAMES_OK            0
RX_512_BYTE_FRAMES_OK            0  RX_1024_BYTE_FRAMES_OK           0
RX_1519_BYTE_FRAMES_OK           0  RX_UNDERSIZE_FRAMES_OK           0
RX_OVERSIZE_FRAMES_OK            0  RX_JABBERS                       0
RX_FCS_ERRORS                    0  RX_LENGTH_ERRORS                 0
RX_SYMBOL_ERRORS                 0  RX_ALIGNMENT_ERRORS              0
RX_RESOURCE_ERRORS               0  RX_OVERRUNS                      0
RX_IP_CHECKSUM_ERRORS            0  RX_TCP_CHECKSUM_ERRORS           0
RX_UDP_CHECKSUM_ERRORS           0  RX_AUTO_FLUSHED_PACKETS          0

Promiscuous mode on
Capture reload is enabled
Packet capture armed
143 byte packet captured
0000 01 00 5E 7F FF FA 00 E0 4C 68 39 02 08 00 45 00 ..^.....Lh9...E.
0010 00 81 09 05 00 00 04 11 B1 06 0A 02 02 65 EF FF .............e..
0020 FF FA EE EC 07 6C 00 6D 85 3F 4D 2D 53 45 41 52 .....l.m.?M-SEAR
0030 43 48 20 2A 20 48 54 54 50 2F 31 2E 31 0D 0A 48 CH * HTTP/1.1..H
0040 6F 73 74 3A 20 32 33 39 2E 32 35 35 2E 32 35 35 ost: 239.255.255
0050 2E 32 35 30 3A 31 39 30 30 0D 0A 53 54 3A 20 75 .250:1900..ST: u
0060 70 6E 70 3A 72 6F 6F 74 64 65 76 69 63 65 0D 0A pnp:rootdevice..
0070 4D 61 6E 3A 20 22 73 73 64 70 3A 64 69 73 63 6F Man: "ssdp:disco
0080 76 65 72 22 0D 0A 4D 58 3A 20 33 0D 0A 0D 0A    ver"..MX: 3....
60 byte packet captured
0000 FF FF FF FF FF FF 00 E0 4C 68 39 02 08 00 45 00 ........Lh9...E.
0010 00 24 57 AD 00 00 80 11 C9 B4 0A 02 02 65 0A 02 .$W..........e..
0020 02 FF C3 13 05 FE 00 10 7E DF 54 43 46 32 04 00 ........~.TCF2..
0030 00 00 00 00 00 00 00 00 00 00 00 00             ............
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

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