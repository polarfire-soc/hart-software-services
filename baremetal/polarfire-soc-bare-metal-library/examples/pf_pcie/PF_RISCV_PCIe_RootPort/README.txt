================================================================================
                    PolarFire PCIe Read and Write Application
================================================================================

This example project demonstrates the PolarFire PCIe root port data read and 
write operation with the PCIe endpoint. It is targeted at the PolarFire
Eval Kit(MPF300) for PCIe RootPort.

--------------------------------------------------------------------------------
                            Mi-V Soft Processor
--------------------------------------------------------------------------------
This example uses a Mi-V Soft processor MiV_RV32IMA_L1_AHB. The design is 
built for debugging MiV_RV32IMA_L1_AHB through the PolarFire FPGA programming 
JTAG port using a FlashPro5. To achieve this the CoreJTAGDebug IP is used to 
connect to the JTAG port of the MiV_RV32IMA_L1_AHB.

All the platform/design specific definitions such as peripheral base addresses,
system clock frequency etc. are included in hw_platform.h. The hw_platform.h is 
located at the root folder of this project.

The MiV_RV32IMA_L1_AHB firmware projects needs the riscv_hal and the hal firmware
(RISC-V HAL).

The RISC-V HAL is available through Firmware catalog as well as the link below:
    https://github.com/RISCV-on-Microsemi-FPGA/riscv-hal
--------------------------------------------------------------------------------
                            How to use this example
--------------------------------------------------------------------------------
Run the example project using a debugger. Place watches on the pointer p_pcie_end_point.
You will then be able to observe the content of p_pcie_end_point being written into
PCIe endpoint memory and read back from PCIe endpoint memory.
Open a debugger memory window to view the content of p_pcie_end_point address.

--------------------------------------------------------------------------------
                                Target hardware
--------------------------------------------------------------------------------
This example project is targeted at a Mi-V design running on the PolarFire
Eval Kit(MPF300). The design should have an instance of PCI Express located at 
address 0x60000000UL. The example project is built using a clock frequency of
50MHz.

This example project can be used with another design using a different clock
configuration. This can be achieved by overwriting the content of this example
project's "hw_platform.h" file with the correct data from your Libero design.

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                          Silicon revision dependencies
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
This example is tested on MPF300T device.

