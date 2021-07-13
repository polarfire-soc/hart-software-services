===============================================================================
# mpfs_hal
===============================================================================

The PolarFire-SoC MSS HAL provides the initial boot code, interrupt handling, 
and hardware access methods for the MPFS MSS. The terms PolarFire-SoC HAL and 
MPFS HAL are used interchangeably but in the main the term PolarFire-SoC MSS HAL 
is preferred.
The PolarFire-SoC MSS hal is a combination of C and assembly source code.

The mpfs_hal folder is included in an PolarFire Embedded project under the 
platform directory.

It contains :

* Start-up code executing from reset
* Interrupt handling support
* Exception handling support
* Memory protection configuration, PMP and MPU
* DDR configuration
* SGMII configuration
* MSSIO setup

## Inputs to the mss_hal
There are two configuration sources. 

1. Libero design
   
   Libero input through header files located in the config/hardware under the 
   platform directory. These files are generated using the PF SoC embedded 
   software configuration generator. It takes an xml file generated in the Libero 
   design flow and produces header files based on the xml content in a suitable 
   form for consumption by the hal.
  
2. Software configuration
   Software configuration settings are located in the mpfs_hal_config folder.


### Example Project directory structure, showing where mpfs_hal folder sits.

~~~~

  +---------+
  | project |
  +----+----+ +---------+      +-----------+
       +-----+| src     |----->|application|
              +---------+  |   +-----------+
                           |
                           |   +-----------+
                           +-->|boards     |
                           +   +----+------+
                           |        |          +---------------+
                           |        +---------+|icicle-kit-es  |
                           |                   +---+-----------+
                           |                       |
                           |                       |  +---------------+
                           |                       +->|platform_config|
                           |                       |  +---------------+
                           |                       |
                           |                       |         +---------------+
                           |                       |---------|drivers_config |
                           |                       |         +---------------+
                           |                       |
                           |                       |         +---------------+
                           |                       |---------|linker         |
                           |                       |         +---------------+
                           |                       |
                           |                       |         +---------------+
                           |                       |---------|mpfs_hal_config|
                           |                       |         +---------------+
                           |                       |
                           |                       |
                           |                       | +---------------+
                           |                       +>|soc_config     |
                           |                       | +---+-----------+
                           |                       |     |
                           |                       |     |  +---------------+------------------------+
                           |                       |     +->|multiple folders with fpga config for sw|
                           |                       |        +----------------------------------------+
                           |                       |
                           |                       |
                           |                       |
                           |                       | +---------------+
                           |                       +>|soc_fpga_design|
                           |                         +--+------------+
                           |                            |
                           |                            |   +---------------+
                           |                            +-->|libero_tcl     |
                           |                            |   +---------------+
                           |                            |
                           |   +-----------+            |   +---------------+
                           +--+|middleware +            +-->|xml            |
                           |   +-----------+                +---------------+
                           |
                           +
                           |   +-----------+
                           +--+|platform   |
                               +----+------+
                                    |          +---------------+
                                    +---------+|drivers        |
                                    |          +---------------+
                                    |
                                    |          +---------------+
                                    +---------+|hal            |
                                    |          +---------------+
                                    |
                                    |          +---------------+
                                    +---------+|mpfs_hal       |
                                    |          +---------------+
                                    |
                                    |          +-------------------------+
                                    +---------+|platform_config_reference|
                                    |          +-------------------------+
                                    |
                                    |          +---------------------+
                                    +---------+|soc_config_generator |
                                               +---------------------+
~~~~
  
Please see the user guide for further details on 
use.
