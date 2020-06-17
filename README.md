# Hart Software Services

This is the Hardware Software Services code. 

On PolarFire SoC, this is comprised of two portions:

- A superloop monitor running on the E51 minion processor, which receives requests from the 
   individual U54 application processors to perform certain services on their behalf;

- A Machine-Mode software interrupt trap handler, which allows the E51 to send messages to the U54s, 
   and request them to perform certain functions for it related to rebooting a U54.

## Source

Source code is found under the `hart-software-services` folder.
     
    hart-software-services
    ├── baremetal
    │   ├── drivers (legacy)
    │   └── polarfire-soc-bare-metal-library (subtree)
    ├── compression
    ├── debug (helper routines for function profiling)
    ├── include
    ├── init (system initialization)
    ├── misc
    ├── services (software service state machines)
    │   ├── boot
    │   ├── crypto
    │   ├── ddr
    │   ├── flashfreeze
    │   ├── goto
    │   ├── ipi_poll
    │   ├── opensbi
    │   ├── powermode
    │   ├── qspi
    │   ├── sgdma
    │   ├── spi
    │   ├── uart
    │   ├── wdog
    │   └── ymodem
    ├── ssmb (secure software message bus)
    │   └── ipi
    └── thirdparty
        ├── fastlz (fast lossless compression library)
        ├── opensbi (RISC-V OpenSBI)
        └── riscv-pk (RISC-V Proxy Kernel - legacy)
            └── machine

## Building

The build is configured using the Kconfig system of selecting build options. 

Due to potential licensing issues, the Hart Software Services do not include Kconfig parsing infrastructure directly. Instead, install [Kconfiglib](https://github.com/ulfalizer/Kconfiglib) - available at [https://github.com/ulfalizer/Kconfiglib](https://github.com/ulfalizer/Kconfiglib).

Both Linux and Windows are supported by Kconfiglib.  Kconfiglib is easily installed with pip from the Linux command line:
 
    $ pip install kconfiglib

The HSS supports a number of board targets - currently including PolarFireSoC-based boards (icicle-kit-es), and SiFive FU540-based boards (lc-mpfs and mpfs).

Once Kconfiglib is installed,  you can enter an interactive selection by running `make BOARD=icicle-kit-es config`. This will generate a `.config` file (which is used to configure the Make build system) and a `config.h` header file (which is used to configure the source code):

    $ make BOARD=icicle-kit-es config

Alternatively, copy the default config for your board - e.g.,

    $ cp boards/icicle-kit-es/def_config .config

Once configured, to build, run `make`:

    $ make BOARD=icicle-kit-es

The standard build will create `hss.elf` and various binary formats (`hss.hex` and `hss.bin`).  Also generated are `output.map`, which is a mapfile for the build, and  `hss.sym`, which is a list of symbols.

A variety of alternative build options can be seen by running `make help`:

    $ make help

Verbose builds (which show each individual command) are possible by adding V=1 to the end of the make command, e.g.:

    $ make V=1

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
