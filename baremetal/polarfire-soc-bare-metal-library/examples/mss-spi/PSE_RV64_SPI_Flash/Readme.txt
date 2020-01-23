================================================================================
                    PolarFire SoC SPI Flash example
================================================================================
This example project demonstrates the use of the PolarFire MSS SPI hardware
block. It reads and writes the content of an external SPI flash device. The 
PolarFire SoC is configured as a master and SPI Flash acts as a slave for this 
particular example project.
--------------------------------------------------------------------------------
                            How to use this example
--------------------------------------------------------------------------------
The example project is targeted to PolarFire SoC hardware platform. The SPI0 is 
configured in master mode whereas SPI flash is the slave. The data is then 
transferred between SPI0 and SPI Flash.
Run the example project using a debugger. Place watches on buffers
g_flash_wr_buf and g_flash_rd_buf. You will then be able to observe the content
of g_flash_wr_buf being written into external flash and read back into the
g_flash_rd_buf buffer.

NOTE: In Release mode, Debugging level is set to default(-g) so that user 
can put the breakpoint and check the buffer.
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

Build the project and launch the debug configuration named 
PSE_RV64_SPI_FLASH hw emulation all harts Debug which is configured for 
PolarFire SoC hardware platform.
