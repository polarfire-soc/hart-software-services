========================================================================
 PolarFire SoC MSS Ethernet MAC Test program - Dual MAC, Single Queue 
========================================================================

This program implements a test shell for the PolarFire SoC MSS MAC Ethernet
driver which can be used to exercise the driver on the  G5 SoC Emulation
platform with the Peripheral Daughter Board. This program targets designs with
two GEM devices using only the pMAC.

The following project defines are used to configure the system:

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

    MSS_MAC_DESIGN_EMUL_DUAL_INTERNAL - Emulation Platform Dual GEM design with
                                        loopback in fabric
    MSS_MAC_DESIGN_EMUL_DUAL_EXTERNAL - Emulation Platform Dual GEM design with
                                        GEM0 -> VSC, GEM1 -> TI (both GMII)

When working with the G5 SOC Emulation system, the serial port baud rate is
115200.

To use this project you will need a UART terminal configured as below:

    - 115200 baud
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
C - Arms the single packet capture feature for time stamped packets. A hex dump
    of the next received packet will be displayed via the serial port.
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
m - Step through rx TSU time stamp modes. The suported values are: DISABLED, 
    PTP EVENT FRAMES, ALL PTP FRAMES and ALL FRAMES. The initial setting is
    ALL FRAMES.
M - Step through tx TSU time stamp modes. The suported values are: DISABLED, 
    PTP EVENT FRAMES, ALL PTP FRAMES and ALL FRAMES. The initial setting is
    ALL FRAMES.
n - When in loopback mode, toggles adding of an extra byte(s) to the
    re-transmitted packets. This can be useful when examining Wireshark captures
    as it provides a way of identifying the echoed packets.
o - Step through One Step Mode options. The supported values are: disabled,
    replace and update. The default setting is disabled.
p - Toggles promiscuous mode. In promiscuous mode the MAC will receive all
    packets but in normal mode it will only receive broadcast packets and
    packets addressed directly to its MAC address which defaults to
    00:FC:00:12:34:56 for GEM0 and 00:FC:00:12:34:57 for GEM1.
P - Transmit a stream of 110 PTP packets covering the various PTP packet types
    and network protocol types.
q - Toggle Screener Type 1 Filters. When enabled, the filters route UDP packets
    with port 320 to queue 1, packets with a TC/DS value of 15 to queue 2 and
    drop UDP packets with port 1534. All other packets go to queue 0. The 
    default is disabled.
Q - Toggle Screener Type 2 Filters. When enabled, the filters route IP packets
    with data values of 0x9E, 0x48, 0x05, 0x0F at offset 0x48 to queue 3 and
    ARP packets with data values of 12 x 0xFF at offset 0x30 to queue 2. All
    other packets route to queue 0. The default is disabled.
r - Clears the statistics counts.
s - Displays the GEM statistics, some PHY statistics and some internal software
    generated statistics.
t - Transmits sample ARP packet of 128 bytes with 0xFF padding.
T - Transmits sample ARP packet with 0x00 padding that increments after every
    transmit. Packet length varies between 64 and 128 bytes and loops back to 64
    bytes after transmitting a 128 byte packet.
u - Transmits sample Unicast ARP packet of 128 bytes with 0xFF padding.
U - Transmits sample Unicast ARP packet with 0x00 padding that increments after
    every transmit. Packet length varies between 64 and 128 bytes and loops back
    to 64
    bytes after transmitting 128 byte packet.
v - Step through VLAN tag depth options. 0 selects no VLANs, 1 selects standard
    VLAN support and 2 selects stacked VLAN support. The default is 0.
V - Toggle VLAN only mode. When enabled, only VLAN tagged packets are received.
    The default is disabled.
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
= - Toggle opposing address SA filter. This controls the specific address filter
    for the source MAC address of the opposing GEM device i.e. if enabled on
    GEM0, packets from GEM1 will be received. The default is disabled.
# - Insert and delete selection of multicast hashes. This command allows
    insertion of hash address matches for the following multicast addresses one
    by one.
        01:1b:19:00:00:00
        01:00:5e:00:00:6b
        01:00:5e:00:01:81
        33:33:00:00:01:81
        01:00:5e:00:00:16
        01:00:5e:7f:ff:fa
        01:80:c2:00:00:0e
        33:33:ff:bb:59:70
        33:33:00:00:00:02
        33:33:00:00:00:16
        33:33:00:01:00:02
    Once all the address have been inserted, subsequent # commands remove them
    in reverse order.
] - Send single sync packet from P list with extra info appended to the packet 
    for testing one step modes.
> - Send single packet from P list. This allows sending the packets from the
    list one by one.
/ - Toggle address swap mode. Each time the command is executed, the GEM0 and
    GEM1 MAC addresses are swapped. This is useful when testing with two boards
    as it allows unicast communications between the two boards.
+/- Increment/Decrement length adjust. Adjusts the additional length added to
    loopbacked packets when the 'n' command is active.
! - Display TSU count for current GEM.
0/1 Switch active GEM to 0 or 1. This changes the GEM device selected. Most of
    the commands only operate on the currently selected device. The default is
    GEM0.
4 - Toggle IPv4 filter. When enabled, any IPv4 packets are automatically
    received. The default is disabled.
6 - Toggle IPv6 filter. When enabled, any IPv6 packets are automatically
    received. The default is disabled.

This test program responds to the above single key commands and will display
responses as appropriate. At any time the list of available commands can be
viewed by pressing'h'. The help display will also show the current settings of
most of the options.

For example, typing the 4 command sequence "spkc" displays the current
statistic information, turns on promiscuous mode, turns on capture auto 
re-trigger mode and enables packet capture so that any received packets are
dumped to the console. This should produce a display something like this:

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
PolarFire MSS Ethernet Dual MAC Test program
Polling method for TXRX. Typed characters will be echoed.
Current MAC is GEM0
Link 0 is currently Up, Duplex = Full, Speed = 1Gb
Link 1 is currently Up, Duplex = Full, Speed = 10Mb
RX 3906 (63 pkts), TX 0 (0 pkts)
RX Over Flow 0, TX Retries 0
RX TS 63, TX TS 0
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
RX_OCTETS_LOW                 4158  RX_OCTETS_HIGH                   0
RX_FRAMES_OK                    63  RX_BCAST_FRAMES_OK              63
RX_MCAST_FRAMES_OK               0  RX_PAUSE_FRAMES_OK               0
RX_64_BYTE_FRAMES_OK            54  RX_65_BYTE_FRAMES_OK             9
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
143 byte packet captured on GEM0 - queue 0
0000 01 00 5E 7F FF FA 00 E0 4C 68 39 02 08 00 45 00 ..^.....Lh9...E.
0010 00 81 0C 18 00 00 04 11 AD F3 0A 02 02 65 EF FF .............e..
0020 FF FA EE EC 07 6C 00 6D 85 3F 4D 2D 53 45 41 52 .....l.m.?M-SEAR
0030 43 48 20 2A 20 48 54 54 50 2F 31 2E 31 0D 0A 48 CH * HTTP/1.1..H
0040 6F 73 74 3A 20 32 33 39 2E 32 35 35 2E 32 35 35 ost: 239.255.255
0050 2E 32 35 30 3A 31 39 30 30 0D 0A 53 54 3A 20 75 .250:1900..ST: u
0060 70 6E 70 3A 72 6F 6F 74 64 65 76 69 63 65 0D 0A pnp:rootdevice..
0070 4D 61 6E 3A 20 22 73 73 64 70 3A 64 69 73 63 6F Man: "ssdp:disco
0080 76 65 72 22 0D 0A 4D 58 3A 20 33 0D 0A 0D 0A    ver"..MX: 3....
RX Descriptor details:
Global all ones broadcast  - N     Multicast hash match       - N
Unicast hash match         - N     External address match     - N
Specific address reg match - N (1) Type ID register match     - N (1)
VLAN tag detected          - N     Priority tag detected      - N (1)
Start of frame             - Y     End of frame               - Y
Time stamp present         - Y     Time stamp = 722:932203892
Destination Address Hash = 37
60 byte packet captured on GEM0 - queue 0
0000 FF FF FF FF FF FF 00 E0 4C 68 39 02 08 00 45 00 ........Lh9...E.
0010 00 24 5A 0B 00 00 80 11 C7 56 0A 02 02 65 0A 02 .$Z......V...e..
0020 02 FF 05 FE 05 FE 00 10 3B F5 54 43 46 32 04 00 ........;.TCF2..
0030 00 00 00 00 00 00 00 00 00 00 00 00             ............
RX Descriptor details:
Global all ones broadcast  - Y     Multicast hash match       - N
Unicast hash match         - N     External address match     - N
Specific address reg match - N (1) Type ID register match     - N (1)
VLAN tag detected          - N     Priority tag detected      - N (1)
Start of frame             - Y     End of frame               - Y
Time stamp present         - Y     Time stamp = 730:668061572
Destination Address Hash = 0
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Most commands work only on the currently selected MAC and you can switch between
active MACs using the '0' and '1' commands. The 'h' command will indicate the
currently selected MAC.

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
