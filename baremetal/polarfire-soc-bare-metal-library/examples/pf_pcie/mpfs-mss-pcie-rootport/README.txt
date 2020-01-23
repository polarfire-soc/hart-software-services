================================================================================
                    PolarFire SoC PCIe Read and Write Application
================================================================================

This example project demonstrates the PolarFire SoC PCIe root port data read and 
write operation with the PCIe endpoint. It is targeted at the Renode model for 
PCIe RootPort and Endpoint.
--------------------------------------------------------------------------------
                    Executing example in the Renode emulation
--------------------------------------------------------------------------------

Renode is an emulation platform used to model the PolarFire SoC hardware
functionality. Renode is integrated with the SoftConsole Tool. To execute an 
example firmware project on Renode you must first launch the PolarFire SoC 
Renode model from SoftConsole and then run the executable on it.

* Create a **Launch Group** configuration. This configuration will combine the 
  two steps i.e. launching of the Renode tool and launching of the debug 
  config to download and debug the code into one launch group configuration.
            Run -> Debug Configurations -> Launch group

For more information, refer "Working_with_Renode.md" copied in the root folder 
of this project.
--------------------------------------------------------------------------------
                            How to use this example
--------------------------------------------------------------------------------
Run the example project using a debugger on e51(HART0). Place watches on the 
pointer p_pcie_end_point. You will then be able to observe the content of 
p_pcie_end_point being written into PCIe endpoint memory and read back from
PCIe endpoint memory. Open a debugger memory window to view the content of 
p_pcie_end_point address.

--------------------------------------------------------------------------------
                                Target hardware
--------------------------------------------------------------------------------
This example project is targeted at a PolarFire SoC Renode platform.

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                          Silicon revision dependencies
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
none known.

