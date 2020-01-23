# PolarFire SoC Bare Metal Library
This folder contains the PolarFire SoC Bare Metal Library.

The PolarFire SoC Bare Metal Library includes:
- Source code for start-up code and Hardware Abstraction Layer (HAL) for the PolarFire SoC RISC-V processor complex
- Source code for the PolarFire SoC Microprocessor Subsystem (MSS) peripehral drivers
- Documentation for the HAL and peripheral drivers
- SoftConsole example projects demonstrating the use of the various PolarFire SoC peripherals

## Source
Source code is found under the "src" folder.

    polarfire-soc-bare-metal-library
        |
        |--- docs
        |--- examples
        |--- src
              |--- platform
                    |--- config
                    |       |--- hardware
                    |       |--- linker
                    |       |--- software
                    |
                    |--- drivers
                    |--- hal
                    |--- mpfs_hal
                    
### src/platform
The src/platform folder contains the entire source code specific to PolarFire SoC. This entire
folder is intended to be copied into a bare metal software project.

#### src/platform/config
The src/platform/config folder contains configuration files applying to the hardware design, software configuration and linker scripts.
- src/platform/config/hardware folder contains files describing the configuration of the PolarFire SoC specific to a Libero design/board.
- src/platform/config/linker contains linker scripts
- src/platform/config/software contains configuration parameters relevant to the Hardware Abstraction Layer (HAL) and drivers. 

##### src/platform/config/hardware
The content of this folder is expected to be generated from a Libero design. The content of this
folder is not intended to be manually modified but instead should be regenerated from a Libero design
meta-data description whenever required.
The content of this folder may need to be updated when reprogramming PolarFire SoC hardware with a
new design.
If you need to modify a parameter coming from the Libero flow please follow the method described in
any of the header files contained in <platform/config/hardware>

#### src/platform/drivers
The src/platform/drivers folder contains the source code for the MSS peripherals and DirectCore soft IP. The content of this folder is not intended to be modified.

#### src/platform/hal
The src/platform/hal folder contains the Hardware Abstraction Layer (HAL). This part of the HAL is intended to be generic across all SoC-FPGA devices.
It is mainly used by DirectCore FPGA IP cores' drivers. The content of this folder is not intended to be modified.

#### src/platform/mpfs_hal
The src/platform/mpfs_hal folder contains the part of the HAL specific to PolarFire SoC. It contains start-up code and MSS peripheral register descriptions.
The content of this folder is not intended to be modified.

## Documentation
Documentation for the HAL and MSS peripheral drivers can be found in the "docs" folder.

## Examples
The "examples" folder contains SoftConsole example projects demonstrating the use of the HAL and MSS peripheral drivers.
Some of these examples include Renode development virtual platform debug configurations providing an execution platform for these examples in the absence of hardware.
Please refer to the [Readme.md](examples/Readme.md) file in the "examples" folder for details on how to use these examples.

