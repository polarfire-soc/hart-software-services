# Hart Software Services for HiFive Unleashed (SiFive FU540)

This repository is a port of the Hart Software Services to the HiFive Unleashed/HiFive Unleashed 
Expansion Board.

As the FU540 does not have exactly the same functionality as Microchip PolarFIre SoC, this version is a limited-functionality HSS, which demonstrates using the E51 to boot the U54s. 

It uses an in-built payload of U-Boot as its boot image.

## Building U-Boot

You can use a version of `u-boot.bin` from riscv-yocto or you can build U-Boot directly from git://git.denx.de/u-boot.git

To build a custom version, first checkout a copy from the upstream repository:

    $ git clone git://git.denx.de/u-boot.git
    $ cd u-boot

Edit arch/riscv/dts/hifive-unleashed-a00.dts and modify the memory stanza to

    memory@82000000 {
        device_type = "memory";
        reg = <0x0 0x82000000 0x1 0xfe000000>;
    };

This modification reserves RAM for the HSS.  Later (work in progress) versions of OpenSBI will reserve this space automatically, but for now make the change manually.  Build U-Boot as follows:

    $ export ARCH-riscv
    $ export CROSS_COMPILE=riscv64-linux-gnu-
    $ make sifive_fu540_defconfig
    $ make dtbs
    $ make

We use `riscv64-linux-gnu-` because the `riscv64-unknown-elf-` compiler that comes with SoftConsole produces the following errors:

    riscv64-unknown-elf-ld.bfd: warning: -z nocombreloc ignored
    riscv64-unknown-elf-ld.bfd: -shared not supported
    scripts/Makefile.lib:400: recipe for target 'lib/efi_loader/helloworld_efi.so' failed
    make[2]: *** [lib/efi_loader/helloworld_efi.so] Error 1
    scripts/Makefile.build:432: recipe for target 'lib/efi_loader' failed
    make[1]: *** [lib/efi_loader] Error 2
    Makefile:1728: recipe for target 'lib' failed
    make: *** [lib] Error 2

We will be creating a payload from `u-boot-dtb.bin`. Copy this file to the toplevel `hart-software-services` directory.

## Creating the HSS payload

From the toplevel `hart-software-services` directory, build the bin2chunks tool, which is a sample tool used to create a HSS bootable payload:

    $ make -C tools/bin2chunks

Now, we will create the payload:

    $ ./tools/bin2chunks/bin2chunks 0x80200000 0x80200000 0x80200000 0x80200000 32768 payload.bin 1 u-boot-dtb.bin 0x80200000

The arguments to bin2chunks are the entrypoints for U54s 1 through 4, a chunk size (values of 4096 to 32768 are useful), the output binary name, and then a number of 3-tuples specifying owner U54 hart (in term of PMP memory), input binary, and the initial load-address for this binary.

Next, create a linkable ELF object for this:

    $ riscv64-unknown-elf-ld -r -b binary payload.bin -o payload.o

## Finally, building the HSS image

Now, we can build the HSS:

    $ cp platform/mpfs/def_config .config
    $ make MACHINE=mpfs

This HSS binary can be used instead of U-Boot from riscv-yocto to boot a Linux image. To use it, prepare an SDCARD as usual using the riscv-yocto flow. Next, overwrite the U-Boot partition using:

    $ sudo dd if=hss.bin of=/dev/mmcblk0p2 bs=4096

