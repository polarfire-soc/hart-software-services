# Hart Software Services

This is the Hart Software Services (HSS) code.

On PolarFire SoC, this is comprised of two portions:

- A superloop monitor running on the E51 minion processor, which receives requests from the
   individual U54 application processors to perform certain services on their behalf;

- A Machine-Mode software interrupt trap handler, which allows the E51 to send messages to the U54s,
   and request them to perform certain functions for it related to rebooting a U54.

The HSS performs boot and system monitoring functions for PolarFire SoC.

The HSS is compressed (DEFLATE) and stored in eNVM.  On power-up, a small decompressor decompressor wrapper inflates the HSS from eNVM flash to L2-Scratchpad memory and starts the HSS.

## Contributions

Pull requests to this repository must include a per-commit sign-off made by a contributor stating that they agree to the terms published at https://developercertificate.org/ for that particular contribution.

## Source

Source code is found under the `hart-software-services` folder.

    hart-software-services
    ├── application (main function, crt startup, init function)
    ├── baremetal
    │   ├── drivers (local modules)
    │   └── polarfire-soc-bare-metal-library (subtree)
    ├── boards
    │   ├── mpfs-icicle-kit-es (Microchip Icicle Kit)
    │   ├── aries-m100pfsevp (Aries M100PFEVPS Kit)
    │   └── custom-board-design (template for new boards)
    ├── envm-wrapper (helper routines to inflate the HSS to L2-Scratchpad)
    ├── include
    ├── init (system initialization)
    ├── modules
    │   ├── compression
    │   ├── crypto
    │   ├── debug (helper routines for function profiling)
    │   ├── misc (miscellaneous routines)
    │   └── ssmb (secure software message bus)
    │        └── ipi
    ├── services (software service state machines)
    │   ├── beu
    │   ├── boot
    │   ├── crypto
    │   ├── ddr
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
    └── thirdparty
        ├── Kconfiglib (a Kconfig implementation in Python)
        ├── libecc (library for elliptic curves based cryptography (ECC))
        ├── miniz (fast lossless compression library)
        └── opensbi (RISC-V OpenSBI)

## Building

The build is configured using the Kconfig system of selecting build options.

The Hart Software Services includes the Kconfig parsing infrastructure directly as a third-party tool invoked by the build system.

Both Linux and Windows are supported by Kconfiglib.

The HSS currently support PolarFire SoC-based icicle kit (mpfs-icicle-kit-es) as a board build target.

### Building on Linux

The HSS relies on SoftConsole v2021.3 or later to build on Linux. It also needs tkinter installed for the KConfiglib guiconfig tool. To install this on Ubuntu/Debian:

    $ sudo apt install python3-tk

First, ensure that the environment variable `$SC_INSTALL_DIR` is set to the location of SoftCOnsole on your system. For example, assuming that SoftConsole is installed at  `/home/user/Microchip/SoftConsole-v2022.2`:

    $ export SC_INSTALL_DIR=/home/user/Microchip/SoftConsole-v2022.2

For building on Linux from the command line, configure the path appropriately. For example, the following will add the SoftConsole provided Python and RISC-V compiler toolchain to the path:

    $ export PATH=$PATH:$SC_INSTALL_DIR/python3/bin:$SC_INSTALL_DIR/riscv-unknown-elf-gcc/bin

Next, set the environment variable `$FPGENPROG` to the location of the fpgenprog tool installed by Libero -- this is used to program the eNVM ('make program') and also to generate hex files suitable for programming by Libero, or inclusion in the fabric design with Libero. For example, assuming that Libero is installed at `/usr/local/microchip/Libero_SoC_v2023.2/`:

    $ export FPGENPROG=/usr/local/microchip/Libero_SoC_v2023.2/Libero/bin64/fpgenprog

Note: Older versions of Libero are installed at `/usr/local/microsemi/<Libero_Version>`.

To make these persistent upon logging in, these three environment variables - `$SC_INSTALL_DIR`, `$FPGENPROG` and the additions to `$PATH` - should be updated in your user shell initialization file (for example, `/home/user/.bashrc` for those running the bash shell).

You can enter an interactive Kconfiglib configuration selection by running `make BOARD=mpfs-icicle-kit-es config`. This will generate a `.config` file (which is used to configure the Make build system) and a `config.h` header file (which is used to configure the source code):

    $ make BOARD=mpfs-icicle-kit-es config

Alternatively, copy the default config for your board. For example:

    $ cp boards/mpfs-icicle-kit-es/def_config .config

or just use the defconfig target to do the copy:

    $ make BOARD=mpfs-icicle-kit-es defconfig

Once configured, to build, run `make`:

    $ make BOARD=mpfs-icicle-kit-es

In the `build` subdirectory, the standard build will create `hss-envm.elf` and various binary formats (`hss-envm.hex` and `hss-envm.bin`).  Also generated are `output-envm.map`, which is a mapfile for the build, and  `hss-envm.sym`, which is a list of symbols.  (The name `build` is required by SoftConsole for programming purposes. In earlier releases, this was called `Default`.)

Once built, program the HSS to the board:

    $ make program

For programming HSS to PolarFire SoC Video Kit:

    $ make program BOARD=mpfs-video-kit

A variety of alternative build options can be seen by running `make help`:

    $ make help

Verbose builds (which show each individual command) are possible by adding V=1 to the end of the make command. For example:

    $ make V=1

### Building on Windows

The HSS relies only on SoftConsole v2021.3 or later to build on Windows.

For more detailed build instructions, particular with regards to using SoftConsole on Windows, see https://mi-v-ecosystem.github.io/redirects/software-development_polarfire-soc-software-tool-flow

First, ensure that the `%SC_INSTALL_DIR%` environment variable is correctly set to the location of SoftConsole.  For example, if SoftConsole is installed in `C:\Microchip\SoftConsole-v2022.2-RISC-V-747`:

    C:\> set SC_INSTALL_DIR=c:\Microchip\SoftConsole-v2022.2-RISC-V-747

For building on Windows from the command line, you must configure the path appropriately to add (Micro) Python, GNU build tools, and the RISC-V compiler toolchain. For example:

    C:\> path %SystemRoot%;%SC_INSTALL_DIR%\build_tools\bin;%SC_INSTALL_DIR%\python3;%SC_INSTALL_DIR%\riscv-unknown-elf-gcc\bin

Ensure the `%SC_INSTALL_DIR%` variable correctly matches your system.

Next, set the environment variable `%FPGENPROG%` to the location of the fpgenprog tool installed by Libero -- this is used to program the eNVM ('make program') and also to generate hex files suitable for programming by Libero, or inclusion in the fabric design with Libero. For example, assuming that Libero is installed at `C:\Microchip\Libero_SoC_v2022.2\Designer\bin64\fpgenprog.exe`:

    C:\> set FPGENPROG=C:\Microchip\Libero_SoC_v2022.2\Designer\bin64\fpgenprog.exe

To make these persistent, set them via the Control Panel or via the Settings App.

#### MicroPython

A MicroPython environment is provided bundled as part of SoftConsole. This includes enough Python support to enable building the HSS. However, it does not support targets `make menuconfig` and `make guiconfig`, as it is missing `windows-curses` (the console library needed for Kconfiglib) and `python3-tk` (the GUI library needed for Kconfiglib).  THese targets are only needed for editing the `.config` configuration.

To work around this, you will need to use an external Python install. This can be done either via the Microsoft Store (preferrred) or manually.

#### Python from Microsoft Store

To do install Python from the Microsoft Store, in a `CMD.EXE` console, type:

    C:\> python3

This should take you to the Microsoft Store to install Python3... Click install.

Make sure that the MicroPython tools are not in your path (e.g. do not have `%SC_INSTALL_DIR%\python3` in your path), and setup your environment as follows:

    C:\work\hart-software-services> path %PATH%;%SC_INSTALL_DIR%\build_tools\bin;%SC_INSTALL_DIR%\riscv-unknown-elf-gcc\bin

Back to the console (no need to restart it if installing Python from the Microsoft Store), and type:

    C:\work\hart-software-services> pip install windows-curses

At this point, windows-curses is now installed. Now `menuconfig` and `guiconfig` work as expected.

#### Python from `python.org`

If you are unable to install Python from the Microsoft Store (due to IT group policy or other enterprise restrictions), you can download it manually and install it from https://www.python.org/downloads/windows/

When you install, please be sure that `Add python.exe to PATH` is selected. Also, ensure that `pip` is being installed (click `Customize installation` and make sure `pip` is selected).  Then click 'Install Now'.  Once the install has completed, close and restart your command prompt. And then type:

    C:\work\hart-software-services> pip install windows-curses

Unlike installs from the Microsoft Store, the `python.org` installer does not add `python3.exe` as an alias of `python.exe`. As a result, if installing Python manually on Windows, you will need to invoke the HSS build commands with `PYTHON=python` as a suffix. For example:

    C>\work\hart-software-services> make menuconfig PYTHON=python
    C>\work\hart-software-services> make guiconfig PYTHON=python

### Debug

The `modules/debug/` subdirectory contains code to enable a number of debug features, including:

 * Logging all state machine transitions to the serial console (MMUART0);
 * Periodic logging of super loop timings to the serial console;
 * Logging of IPI statistics to the serial console;
 * Logging all IPI messages for MSC traces;
 * Function profiling.

### Function Profiling

Function profiling in the HSS can be enabled using the `CONFIG_DEBUG_PROFILING_SUPPORT` Kconfig option.

Function profiling allows capturing of the time spent in each C function (through the use of `__cyg_profile_func_enter` and `__cyg_profile_func_exit`. This information can be logged to the serial console through calling the `HSS_Profile_DumpAll()` function at an appropriate time, depending on what is being debugged, and by using the `debug profile` CLI command.

For more information, please refer to the [HSS Function Profiling documentation](https://mi-v-ecosystem.github.io/redirects/hart-software-services-profiling).

### Scrubbing for ECCs and Non-Coherent Memory.

The HSS includes a scrubbing service (running on the E51) to scrub ECC protected memories such as DDR, L2 Scratchpad, LIM for correctable single-bit errors in an effort to correct them before they propagate to uncorrectable double-bit errors. Note that by default, mpfs-linux is configured such that DDR accesses to/from peripherals such as PCIe, eNVM and USB are directed to uncached memory addresses; with cache synchronisation performed explicitly by the Linux kerne. Enabling scrubbing can result in interfering with this default mpfs-linux behavior, and so it is important to configure mpfs-linux to only used coherent memory when using scrubbing.

As a consequence, scrubbing is currently off by default in provided `def_config` files.

   # CONFIG_SERVICE_SCRUB is not set
