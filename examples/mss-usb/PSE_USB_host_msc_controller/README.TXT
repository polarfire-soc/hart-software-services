================================================================================
         PolarFire SoC MSS USB Mass storage Class Host Controller
================================================================================
This project demonstrates the use of PolarFire SoC MSS USB driver stack in USB 
Host mode.

This example emulates the USB Mass storage class host functionality. MSS USB
will act as a MSC class host which can read/write files from the Mass storage
device(e.g. pen drive) connected to PolarFire SoC device through USB.

This Example project and involved drivers pass the USB-IF USB2.0 compliance
test for High speed Embedded Host.

--------------------------------------------------------------------------------
                    File System support
--------------------------------------------------------------------------------
This example uses FatFs file system library. The supported features configured
for this project are listed below.

File systems        :   FAT12, FAT16, FAT32.
Sector size         :   512
Logical Units       :   1
File Names          :   Short File Names
Character code set  :   ANSI/OEM
Partition           :   1

To know more about the library configurations in this example refer .\FatFs\ffconf.h

To read more about the FatFs Refer - http://elm-chan.org/fsw/ff/00index_e.html

--------------------------------------------------------------------------------
                    How to use this project
--------------------------------------------------------------------------------
To use this project you will need a UART terminal configured as below:
    - 115200 baud
    - 8 data bits
    - 1 stop bit
    - no parity
    - no flow control

This application uses FatFs file system library to access the files on the 
targeted Flash drive. The application only needs to use the APIs provided by 
FatFS to access the Flash drive. To achieve this the discio.c (required by 
FatFs.) is implemented by this application. The discio.c internally uses the 
MSS USB Host driver which in turn accesses the connected Flash drive.

This example initializes the MSS USB Host driver in USB Host mode. Initializes
the USB MSC host class driver and registers it with the MSS USB Host driver.
As the FatFs application Interface APIs are used, the corresponding MSS
USB MSC class Driver APIs will be invoked through discio.c(Disc I/O interface)

Note:  It is preferred to connect a pre-formatted Flash drive to PolarFire SoC.
If unformatted Flash drive is connected, f_mount API can be used to clean the 
Flash Drive. But this takes long time to complete(depending on the size of the
Flash drive.). This long time might give a false appearance that the PolarFire 
SoC system is not work.

The UI appears on UART. The UI is self explanatory. Follow the steps as per the 
messages appearing on the console.

--------------------------------------------------------------------------------
                    File Read/Write performance
--------------------------------------------------------------------------------
The speed of the file read/write operation depends on the application, FatFs
and the MSS USB driver. For maximum file read/write performance the application 
should configure the FatFs to extract its maximum performance. To know more 
about the efficient configuration of FatFs, please refer:
http://elm-chan.org/fsw/ff/en/appnote.html#limits.

To achieve maximum performance, the application should also make sure that the 
data transfers should be sector aligned. One way of achieving this is by 
selecting the data buffer sizes and transfer sizes which are integer multiple of
sector size i.e. 512 bytes.

The MSS USB driver supports multi packet Bulk transfers. The application should
make sure that the transfer size is selected as the maximum transfer size 
supported by FatFs, to make sure that the maximum possible bandwidth with multi
packet Bulk transfer is utilized fully.

The MSS USB driver supports Bulk transfers using the DMA inside MSS USB or
without using the internal DMA. It is advised that the MSS USB driver is 
configured to use the internal DMA in order to free up the application from
transferring data to/from MSS USB hardware block.
   
--------------------------------------------------------------------------------
                                Target hardware
--------------------------------------------------------------------------------
This example project can be used on the PolarFire SoC model on Renode emulation 
platform as well as PolarFire SoC FPGA family hardware platforms. 

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

## Executing project in the Renode emulation

To launch the PolarFire Soc board emulation model on Renode from 
SoftConsole , launch the preconfigured external tool from
   Run -> External Tools -> "mpfs-mss-usb-host-msc Renode-emulation-platform". 

This will also launch an UART terminal which works with Renode.

Build the project and launch the debug configuration named 
mpfs-mss-usb-host-msc Renode Debug.launch which is configured for Renode. 

For more information, refer Working_with_Renode.md copied in the root folder of
this project.

## Executing project on PolarFire SoC hardware

The same application can be used on PolarFire SoC hardware platform. The MMUART0
must be connected to host PC. The host PC must connect to the serial port using 
a terminal emulator such as Tera Term or PuTTY.

Build the project and launch the debug configuration named 
mpfs-mss-usb-host-msc-all-harts Debug.launch which is configured for PolarFire 
SoC hardware platform.
