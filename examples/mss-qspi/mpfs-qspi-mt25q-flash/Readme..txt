================================================================================
                    PolarFire SoC MSS QSPI Flash example
================================================================================
This example project demonstrates the use of the PolarFire MSS QSPI hardware
block. It reads and writes the content of an external QSPI flash device.
All modes of operations including XIP are demonstrated.
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
mover the UART terminal. It configures the QSPI flash controller and the
QSPI flash memory in the Normal SPI mode and does the write, read operations
on it in that sequence. It then cross-checks the content read from the memory 
with the contents that were written to it. A pass or fail message is displayed 
as per the results.

This program then does the same operations in Dual and Quad modes of operations.
The MSS QSPI driver can carry out blocking transfers using polling method or it 
can do the same operations in a non-blocking way using interrupt. Wether to use
interrupt or not is decided by the application. In this example polling mode
is used by default. To enable interrupt mode define a constant USE_QSPI_INTERRUPT 
in micron_mt25.c.

At the end it will configure the the XIP mode and display the data. It will then
exit the XIP mode and show the normal register access data.

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
Choose the Debug or Release configuration of the project, Build the project and 
launch the launch the debug launcher for that configuration to download and
execute the project on PolarFire SoC hardware platform.
