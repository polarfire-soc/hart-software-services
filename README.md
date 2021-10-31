# Hart Software Services

This is the Hart Software Services (HSS) code. 

On PolarFire SoC, this is comprised of two portions:

- A superloop monitor running on the E51 minion processor, which receives requests from the 
   individual U54 application processors to perform certain services on their behalf;

- A Machine-Mode software interrupt trap handler, which allows the E51 to send messages to the U54s, 
   and request them to perform certain functions for it related to rebooting a U54.

The HSS performs boot and system monitoring functions for PolarFire SoC.

The HSS is compressed (DEFLATE) and stored in eNVM.  On power-up, a small decompressor decompressor wrapper inflates the HSS from eNVM flash to L2-LIM memory and starts the HSS.

## Source

Source code is found under the `hart-software-services` folder.
   
    hart-software-services
    ├── application (main function, crt startup, init function)
    ├── baremetal
    │   ├── drivers (local modules)
    │   └── polarfire-soc-bare-metal-library (subtree)
    ├── boards
    │   ├── mpfs-icicle-kit-es (Icicle Kit)
    │   ├── mpfs (Aloe Vera platform)
    │   └── lc-mpfs (Low-cost Aloe Vera platform)
    ├── compression
    ├── debug (helper routines for function profiling)
    ├── envm-wrapper (helper routines to inflate the HSS to L2-LIM)
    ├── include
    ├── init (system initialization)
    ├── misc (miscellaneous routines)
    ├── services (software service state machines)
    │   ├── boot
    │   ├── crypto
    │   ├── ddr
    │   ├── flashfreeze
    │   ├── goto
    │   ├── ipi_poll
    │   ├── mmc
    │   ├── opensbi
    │   ├── powermode
    │   ├── qspi
    │   ├── scrub
    │   ├── sgdma
    │   ├── spi
    │   ├── tinycli
    │   ├── uart
    │   ├── usbdmsc
    │   │    └── flash_drive
    │   ├── wdog
    │   └── ymodem
    ├── ssmb (secure software message bus)
    │   └── ipi
    └── thirdparty
        ├── Kconfiglib (a Kconfig implementation in Python)
        ├── miniz (fast lossless compression library)
        └── opensbi (RISC-V OpenSBI)

## Building

The build is configured using the Kconfig system of selecting build options. 

The Hart Software Services includes the Kconfig parsing infrastructure directly as a third-party tool invoked by the build system.

Both Linux and Windows are supported by Kconfiglib.
 
The HSS currently support PolarFire SoC-based icicle kit (mpfs-icicle-kit-es) as a board build target.

### Building on Linux

For building on Linux from the command line you must configure the path appropriately, e.g.:

    $ export PATH=$PATH:$HOME/Microchip/SoftConsole-v2021.1/python/bin:$HOME/Microchip/SoftConsole-v2021.1/riscv-unknown-elf-gcc/bin

You can enter an interactive Kconfiglib configuration selection by running `make BOARD=mpfs-icicle-kit-es config`. This will generate a `.config` file (which is used to configure the Make build system) and a `config.h` header file (which is used to configure the source code):

    $ make BOARD=mpfs-icicle-kit-es config

Alternatively, copy the default config for your board - e.g.,

    $ cp boards/mpfs-icicle-kit-es/def_config .config

or just use the defconfig target to do the copy:

    $ make BOARD=mpfs-icicle-kit-es defconfig 

Once configured, to build, run `make`:

    $ make BOARD=mpfs-icicle-kit-es

In the `Default` subdirectory, the standard build will create `hss-envm.elf` and various binary formats (`hss-envm.hex` and `hss-envm.bin`).  Also generated are `output-envm.map`, which is a mapfile for the build, and  `hss-envm.sym`, which is a list of symbols.  (The name `Default` is required by SoftConsole for programming purposes.)

A variety of alternative build options can be seen by running `make help`:

    $ make help

Verbose builds (which show each individual command) are possible by adding V=1 to the end of the make command, e.g.:

    $ make V=1

### Building on Windows

The HSS relies only on SoftConsole v2021.1 or later to build on Windows. One workaround that is needed is to copy `python3\bin\python.exe` to `python3\bin\python3.exe` in the SoftConsole v2021.1 installation folder on Windows, e.g.:

    C:\> copy C:\Microchip\SoftConsole-v2021.1\python3\python.exe C:\Microchip\SoftConsole-v2021.1\python3\python3.exe

For more detailed build instructions, particular with regards to using SoftConsole on Windows, see https://github.com/polarfire-soc/polarfire-soc-documentation/blob/master/software-development/polarfire-soc-software-tool-flow.md#build-the-hss.

For building on Windows from the command line one must configure the path appropriately, e.g.:

    C:\> path %SystemRoot%;%SystemRoot%;C:\Microchip\SoftConsole-v2021.1\build_tools\bin;C:\Microchip\SoftConsole-v2021.1\python;C:\Microchip\SoftConsole-v2021.1\riscv-unknown-elf-gcc\bin

### Debug

The `debug/` subdirectory contains code to enable a number of debug features, including:

 * Logging all state machine transitions to the serial console (MMUART0);
 * Periodic logging of super loop timings to the serial console;
 * Logging of IPI statistics to the serial console;
 * Logging all IPI messages for MSC traces;
 * Function profiling.

### Function Profiling

Function profiling allows capturing of the time spent in each C function (through the use of `__cyg_profile_func_enter` and `__cyg_profile_func_exit`. This information can be logged to the serial console through calling the `dump_profile()` function at an appropriate time, depending on what is being debugged.

**NOTE:** *by default, this function is not called, even with profiling enabled.*
