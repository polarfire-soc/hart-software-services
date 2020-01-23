================================================================================
                    PolarFire SoC MSS System Services example
================================================================================
This example project demonstrates the use of the PolarFire SoC MSS System 
service driver to execute the services supported by system controller.  

--------------------------------------------------------------------------------
                            How to use this example
--------------------------------------------------------------------------------
To use this project you will need a UART terminal configured as below:
    - 115200 baud
    - 8 data bits
    - 1 stop bit
    - no parity
    - no flow control

The PolarFire SoC MSS system service driver can be configured to execute service in
interrupt mode and polling mode. Enable the macro MSS_SYS_INTERRUPT_MODE in 
e51.c to execute the service in interrupt mode. User need to select the mode of 
operation by configuring the driver with appropriate service mode macros. If 
application does not select any mode, dafault is polling mode. 
The application will display the list of the services implemented in this
example. User can select the desired service for execution from the list 
displayed on UART terminal.
Every service will respond with SUCCESS or ERROR message, which can be observed 
on the UART terminal. 

Note: 
This application demonstrates the execution of some of the system services. 
Most of the services require some infrastructure in the design for successful
execution. If the required infrastructure is not available, service will not
execute or return error code. Refer readme.txt for more details.
For example:
To execute usercode service, the usercode must be programmed in the design. 

To execute device certificate service, the certificate has to be stored
in device pNVM.

For SPI copy service, an external flash memory is required with the existing 
setup and the design must have the SPI connections to the external flash device.  

Match OTP service requires a proper validator input for it to execute successfully.
--------------------------------------------------------------------------------
                                Target hardware
--------------------------------------------------------------------------------
This example project is targeted PolarFire SoC FPGA family hardware platforms
with MSS MMUART enabled .

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
mpfs_system_services_all_harts_Debug.launch which is configured for PolarFire 
SoC hardware platform.

 
