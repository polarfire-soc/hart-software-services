================================================================================
                 PolarFire SoC MSS PDMA Driver example
================================================================================

This example project demonstrates the use of the PolarFire SoC MSS Platform
DMA driver to configure the DMA channel and initiate the transaction between
source and destination memory locations. 
Success and error status in the transactions are reported by respective 
interrupts. 

--------------------------------------------------------------------------------
                            How to use this example
--------------------------------------------------------------------------------
To use this project you will nee a UART terminal configured as below.
    - 115200 baud
    - 8 data bits
    - 1 stop bit
    - no parity
    - no flow control

This is a self contained example project. A greeting message and is displayed
mover the UART terminal indicating the  DMA channels for transaction. 
User need to select the appropriate channel(0 - 3) and the driver will 
configure the selected channel. Once the channel is successfully configured,
transaction will intitate. Error or Success status of the transaction is 
displayed over the UART terminal.
User can repet the process to verify the transactions on different DMA channel. 
--------------------------------------------------------------------------------
                                Target hardware
--------------------------------------------------------------------------------
This example project can be used on PolarFire SoC FPGA family hardware platforms
as well as the PolarFire SoC model on Renode emulation platform. 

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

## Executing project in the Renode emulation

To launch the PolarFire Soc Icicle board emulation model on Renode from 
SoftConsole , launch the preconfigured external tool from
   Run -> External Tools -> "PolarFire-SoC-Renode-emulation-platform". 

This will also launch an UART terminal which works with Renode.

Build the project and launch the debug configuration named 
mpfs_mss_pdma_renode_attach_to_hart0_debug.launch which is configured for Renode. 

Please refer to working_with_renode.md file for more detailed description about
renode configuration.


## Executing project on PolarFire SoC hardware

The same application can be used on PolarFire SoC hardware platform. The MMUART0
must be connected to host PC. The host PC must connect to the serial port using 
a terminal emulator such as Tera Term or PuTTY.

Build the project and launch the debug configuration named 
mpfs_mss_pdma_all_harts_Debug.launch which is configured for PolarFire SoC 
hardware platform.
