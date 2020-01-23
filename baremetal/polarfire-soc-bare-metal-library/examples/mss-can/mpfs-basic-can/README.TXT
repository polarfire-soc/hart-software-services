================================================================================
              PolarFire SoC MSS BasicCAN example
================================================================================
This example project demonstrates using the MSS CAN peripheral to perform CAN
message transmission and reception. The MSS CAN driver has APIs for BasicCAN and
FullCAN Configurations. This project is configured for BasicCAN communication.

The operation of the MSS CAN is controlled via a serial console.

--------------------------------------------------------------------------------
                            How to use this example
--------------------------------------------------------------------------------
To use this project you will nee a UART terminal configured as below:
    - 115200 baud
    - 8 data bits
    - 1 stop bit
    - no parity
    - no flow control
    
This is a self contained example project. A greeting message and is displayed
mover the UART terminal. On startup, the example project requests the user to 
enter in data to send via the CAN Bus. You can enter up to 32 pairs of hex 
digits (no separating spaces) and the data will be sent out in chunks of 8 
bytes at a time in up to 4 CAN packets. You can send less than 32 bytes of 
data by pressing return to terminate the data early.
    
The test program then enters a loop looking for user input to select the next
action to perform. Whilst in this loop, the data portion of any CAN Bus packets
received into the rx buffers is displayed on the console. The following menu
options are available:

    0 - Perform a hardware reset of the MSS CAN peripheral via SYSREG. This 
        shuts down all CAN communications.
    5 - Reinitailze the MSS CAN peripheral then get data from user and send via
        CAN Bus.
    7 - Get data from user and send via CAN Bus.

The following macros modify the behaviour of the program:

    CAN_TX_EXTENDED_ID        - Defining this macro causes CAN messages with
                                with extended 29 bit IDs to be sent instead of
                                the standard 11 bit IDs.
    CAN_TARGET_COCO_PC_ACTIVE - Defining this macro adjusts the baud rate to
                                enable reliable operation with the Kromschroder
                                CoCo PC Active CAN Bus interface.
                                
Note: Because the BasicCAN example program provides an ISR for the CAN_IRQ, the
      following macros need to be defined in the project settings to ensure
      interrupts are fully enabled and the default ISR in mss_can.c is disabled:
      
          MSS_CAN_ENABLE_INTERRUPTS
          MSS_CAN_USER_ISR

--------------------------------------------------------------------------------
                        Test CAN Message Transmission
--------------------------------------------------------------------------------

    1. Enter the data on hyperterminal, which will be received through MSSUART1.
    2. Based on received data bytes, segregate as CAN messages of maximum 8 
       bytes length.
    3. Send the received data in terms of CAN messages.
    4. Observe the CAN messages on CAN Analyzer with message identifier as 0x78.
    5. Compare the data received on CAN Analyzer with the data sent from the 
       hyperterminal data should be same.

--------------------------------------------------------------------------------
                        Test CAN Message Reception
--------------------------------------------------------------------------------

    1. Send the 8 bytes of CAN message from CAN Analyzer with message identifier
        as 0x200.
    2. Read the data using CAN APIs and store it in to RAM buffer.
    3. Transmit the data using MSSUART on to hyper terminal.
    4. Observe the data received on Hyperterminal.
    5. Compare the data sent from CAN Analyzer with the data received on 
       hypertermianl data should be same.
  
--------------------------------------------------------------------------------
                                Target hardware
--------------------------------------------------------------------------------
This example project can be used on the PolarFire SoC FPGA family hardware 
platforms. 

There are configurations that needs to be set for this example project. The
configurations are categorized into hardware and software configurations and 
are respectively located in the following folders
    platform/config/hardware/hw_config
    platform/config/software/mpfs_hal/mss_sw_config

The "hw_config" defines the hardware configurations such as clocks. You must 
make sure that the configurations in this example project match the actual 
configurations of your target design you are using to test this example project.

The "sw_config" folder defines the software configurations such as HART 
configurations. These configurations have no dependency on the hardware
configurations that you use. If you change these configurations, you may have 
to change the software code to work according to the new configurations that 
you use.

## Executing project on PolarFire SoC hardware

This example project can be used on PolarFire SoC hardware platform. The MMUART0
must be connected to host PC. The host PC must connect to the serial port using 
a terminal emulator such as Tera Term or PuTTY.

Build the project and launch the debug configuration named 
mpfs-can-basic-all-harts Debug.launch which is configured for PolarFire SoC 
hardware platform.
