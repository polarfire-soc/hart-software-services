================================================================================
                    PolarFire SoC MSS I2C Master/Slave example
================================================================================

This example project demonstrates the use of the PolarFire SoC MSS I2C 
peripherals using external Loop back. It demonstrates reading and writing 
data between a pair of MSS I2Cs configured as a master and a slave.

The operation of the MSS I2Cs is controlled via a serial console.

--------------------------------------------------------------------------------
                            How to use this example
--------------------------------------------------------------------------------
Connect the IO pins for the SDA and SCL of the two MSS I2Cs together.

To use this project you will need a UART terminal configured as below:
    - 115200 baud
    - 8 data bits
    - 1 stop bit
    - no parity
    - no flow control

The example project will display instructions over the serial port. A greeting
message and menu instructions are displayed over the UART. Follow the instruction 
to use different menu options provided by the example project.

Use menu option 1 to write between 0 and 32 bytes of data from the master I2C
device to the slave I2C device. Any data received by the slave is written to
the slave_tx_buffer[] array and overwrites some or all of the default contents 
- "<<-------Slave Tx data ------->>". 0 byte transfers are allowed with this
option but not with option 3 below.

Use menu option 2 to read the 32 bytes of data from the Slave I2C 
slave_tx_buffer[] via I2C and display it on the console.

Use menu option 3 to write between 1 and 32 bytes of data to the slave and read
it back in the same operation (uses repeat start between read and write). 

Use menu option 4 to terminate the demo.

To demonstrate the error detection and time out features of the driver, follow
these steps:

1. Error: Disconnect the SDA line. Attempt to write some data to the slave 
   via menu option 1 and observe the "Data Write Failed!" message.
   
2. Time out: Connect the SDA line to gnd. Attempt to write some data to 
   the slave via menu option 1 and observe the "Data Write Timed Out!" message 
   after 3 seconds.
   
--------------------------------------------------------------------------------
                                Target hardware
--------------------------------------------------------------------------------
This example project is targeted PolarFire SoC FPGA family hardware platforms
with the 2 MSS I2C peripherals and MSS MMUART enabled .

There are configurations that needs to be set for this example project. The
configurations are categorized into hardware and software configurations and 
are respectively located in the following folders
    ./mpfs_config/hw_config
    ./mpfs_config/sw_config

The "hw_config" defines the hardware configurations such as clocks. You must 
make sure that the configurations in this example project match the actual 
configurations of your target design you are using to test this example project.

The "sw_config" folder defines the software configurations such as HART 
configurations. These configurations have no dependency on the hardware
configurations that you use. If you change these configurations, you may have 
to change the software code to work according to the new configurations that 
you use.

## Executing project on PolarFire SoC hardware
The MMUART0 must be connected to host PC. The host PC must connect to the 
serial port using a terminal emulator such as Tera Term or PuTTY.

Build the project and launch the debug configuration named 
mpfs-mss-i2c-all-harts Debug.launch which is configured for PolarFire SoC 
hardware platform.

NOTE:
It is a good idea to set the drive current for the I2C I/O to the maximum
allowed so that the fall time for the I2C pins is minimised. This will help to
improve the reliability of the I2C communications.
 
