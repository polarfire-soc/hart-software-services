Project compile settings can be modified to compile for Unleashed or PSE (Xilinx Emulation platform)
SoftConsole used to test:
Version: 5.3.0.18 unleached
Build id: 5.3.0.18-20180411-112251

Project currently configured for Xilinx Emulation platform
PSE=1
SI_FIVE_UNLEASED=0
TEST_H2F_CONTROLLER=1

Project set to show F2H working.
BitFile with loopback required.
This is just an example which can vbe used to test all H2F interrupts.

To use this program with unleashed board, change the define setting id SoftConsole in the 
settings/Prepocessor TAB:
PSE=0
SI_FIVE_UNLEASED=1
