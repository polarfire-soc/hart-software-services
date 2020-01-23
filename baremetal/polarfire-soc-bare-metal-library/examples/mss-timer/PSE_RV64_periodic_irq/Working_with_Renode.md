# Working with Renode-emulation-platform

The PolarFire SoC HAL for bare metal software runs the startup code and brings 
up all 5 harts of the PolarFire SoC CoreComplex. It hands over control to the 
Application software such that all harts have their own main():

- **void e51(void)**   in hart0/e51.c
- **void u54_1(void)** in hart1/u54_1.c
- **void u54_2(void)** in hart2/u54_2.c
- **void u54_3(void)** in hart3/u54_3.c
- **void u54_4(void)** in hart4/u54_4.c

User is expected to write the application code for particular hart in it's 
respective function. 

You can download the PolarFire SoC bare metal firmware example projects from 
firmware catalog.

## Executing example in the Renode emulation
Renode is an emulation platform used to model the PolarFire SoC hardware
functionality. Renode is integrated with the SoftConsole Tool. To execute an 
example firmware project on Renode you must first launch the PolarFire SoC model 
Renode model from SoftConsole and then run the executable on it.
This can be done in two ways.

### Run Renode as external tool
* To launch the PolarFire Soc Icicle board emulation model on Renode from 
  SoftConsole , launch the preconfigured external tool from   
       Run -> External Tools -> "PolarFire-SoC-Icicle-Renode-emulation-platform". 
       
* Create and launch a debug configuration to download the executable and 
  connect to one of the harts for running and step debugging the code.   
        Run -> Debug Configurations -> GDB OpenOCD Debugging
        
### Create launch group
* Create a **Launch Group** configuration. This configuration will combine the 
  two steps i.e. launching of the Renode tool and launching of the debug 
  config to download and debug the code into one launch group configuration.
            Run -> Debug Configurations -> Launch group
    
  Examples of each are provided in the Firmware catalog example projects.

SoftConsole uses GDB to communicate with GDB Server for remote target debugging
over TCP/IP connection port.
For PolarFire SoC Renode model target each debug launcher can attach to only 
one hart over its own dedicated GDB port:

- hart 0 e51   at port 3333
- hart 1 u54_1 at port 3334
- hart 2 u54_2 at port 3335
- hart 3 u54_3 at port 3336
- hart 4 u54_4 at port 3337

For example, you can build the project and run **Launch Group** to start the 
emulation and attach to hart0. 
Though we use individual GDB port to connect to individual hart, we have a single 
ELF file which contains the code that is executed on all harts. Only one of the 
five harts needs to reset the MSS and download the executable and connect to
that hart. Rest of the Debug configs just need to start GDB session and connect 
to the other harts (no reset and download required).

When loading the ELF file using debug config for a specific hart the PCs for 
other harts will not be set correctly and therefore the first Debug launcher 
needs to call the monitor commands to set up the initial value in the PC of all
other harts.
 
In the example below the reset and code download is done by the E51's
Debug Launcher and it sets all the u54 hart's PCs to the PC of the E51 hart :

    monitor sysbus.u54_1 PC `sysbus.e51 PC`
    monitor sysbus.u54_2 PC `sysbus.e51 PC`
    monitor sysbus.u54_3 PC `sysbus.e51 PC`
    monitor sysbus.u54_4 PC `sysbus.e51 PC`

It is possible to duplicate the launcher and connect to a different ports/harts,
You can change the above example commands per your need.

If you don't want to use all the harts then you may halt the harts that you don't 
want to use. This can be achieved by using renode commands   
e.g `monitor u54_4 IsHalted true` in the "initialization commands" tab of the 
Debug Config Launcher.

### Troubleshooting
When troubleshooting applications which interact with the peripherals, then 
increased verbosity can improve the visibility into the peripheral. Increasing 
log level in the debug launcher can show events happening in the peripheral. 
For increasing PLIC and GPIO log verbosity the following commands has to be 
added to the   
    Launcher -> Startup -> Initialization commands:
```
    monitor logLevel -1 sysbus.gpio1
    monitor logLevel -1 sysbus.plic
```

The log levels are:

    - NOISY (-1)
    - DEBUG (0)
    - INFO (1)
    - WARNING (2)
    - ERROR (3)

Any of the applicable Renode command can be added to the "initialization commands"
or Run/Restart command tab in the Debug Config. Note that you must use the prefix
<monitor> to the Renode command for it to work successfully.   
e.g.    
Renode command      
    logLevel -1 sysbus.gpio1   
Write it as mentioned below in the "initialization commands" tab      
    monitor logLevel -1 sysbus.gpio1   
   
Alternatively, These commands can be typed in the Renode's monitor windows 
(without monitor prefix).

For more information and useful commands read the Renode's
[documentation](https://renode.readthedocs.io/en/latest/)


### Connecting to other harts

Example launchers are provided for other harts which will connect to the 
platform set the PCs as the hart0 launcher would do (in this case the PC is 
read from hart1), they are just using different gdb port to select the desired 
hart. This is useful when doing single hart debugging. While being able to debug
a specific hart the other harts are executing code as they normally would, 
just the breakpoints on their code will not work. Breaking/suspending connected 
hart will suspend the execution of the other harts as well.

### Connecting to multiple harts at the same time

This feature is not fully supported yet, but sometimes a workaround can be used.
***Use it at own risk as the workaround is not stable yet.***

Start Renode and connect to the hart0 with the available group launcher. Let the 
hart0 running (do not try this with halted hart0) and configure a new launcher 
as follows:

- Debugger tab:
    - Leave the **Start OpenOCD locally** unchecked
    - Select new port to connect to (3333 to 3337 for hart0 - 4)

- Startup tab:
    - Uncheck **Initial Reset**
    - Uncheck **Load executable**
    - Uncheck **Set breakpoint at**
    - Remove monitor commands if they are not needed (do not set the PCs as they 
    were set when the first launcher connected)
    - Add new commands as required, for example showing new terminal 
    `monitor showAnalyzer sysbus.mmuart1`

Run the second launcher connecting to the same Renode platform instance.


### Work in progress
The Renode is work in progress and some SoftConsole's commands (such as RTOS 
queries) are not supported yet. This can result in an increased number of 
warning or error messages. 

For the PolarFire SoC the PCs for all harts need to be set. And if the following
error is repeated on each instruction step then there is an issue with the PCs.
If the error is mentioned only once on startup then it should be safe to ignore it.

```
10:22:49.0443 [ERROR] u54_1: CPU abort [PC=0x1000]: Trying to execute code outside RAM or ROM at 0x0000000000001000.
10:22:49.0444 [ERROR] u54_4: CPU abort [PC=0x1000]: Trying to execute code outside RAM or ROM at 0x0000000000001000.
10:22:49.0444 [ERROR] u54_3: CPU abort [PC=0x1000]: Trying to execute code outside RAM or ROM at 0x0000000000001000.
10:22:49.0444 [ERROR] u54_2: CPU abort [PC=0x1000]: Trying to execute code outside RAM or ROM at 0x0000000000001000.
```




