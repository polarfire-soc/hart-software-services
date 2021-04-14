# PolarFire SoC Configuration Generator
This is a utility to generate embedded software header files from information supplied by Libero from the Libero design. Libero supplies the information in the form of an xml file. This can be found in the Libero component subdirectory.
e.g: /component/work/PFSOC_MSS_C0/PFSOC_MSS_C0_0

# Table of contents
1. [ Content ](#content)
2. [ Preparation ](#prep)
3. [ Steps to generate embedded software files ](#initial)
4. [ Integrate *soc_config* into Embedded Software Project ](#Int)


## 1. Content <a name="content"></a>
The table below describes the content of the PolarFire SoC Configuration Generator 
directory

| File                                          | Description       |       
| :-------------------------------------------- |:------------------| 
| readme.md                                     | This file.        |
| mpfs_configuration_generator.py               | Python script. Takes .xml and (optional) output directory as arguments. Produces hardware configuration header files inside an soc_config directory. This soc_config directory is created either in the output directory specified as second argument or in the current working directory if no output directory argument is specified.|   

##
~~~
   +---------+
   | root    +-+-->readme.md
   +---------+ +-->mpfs_configuration_generator.py
               |
               |     +-----------------------------------------------+
               |     | +-----------+         Output:                 |
               +------>| soc_config  +-->Created header files          |
                     | +-----------+                                 |
                     +-----------------------------------------------+
~~~

## 2. Preparation <a name="prep"></a>
Python must be present on the computer to run the PolarFire SoC Configuration Generator.
The Python script will run on Python version 2 or 3.

## 3. Steps to generate embedded software files <a name="initial"></a>
Please follow the recommended steps
1. Copy the Libero generated or hand crafted xml file to the ref_xml directory
2. Delete or rename the subdirectory *soc_config* as it will be overwritten
4. The subdirectory *soc_config* will be created containing content for embedded software. 

#### Example generating from a command line in Windows
~~~~
C:\mpfs-bare-metal-sw-config-generator\lib>py -3 mpfs_configuration_generator.py pf_soc_hw_description_reference.xml
generate header files for Embedded software project
pfsoc-baremetal-software-cfg-gen.py
python interpreter details: sys.version_info(major=3, minor=7, micro=4, releaselevel='final', serial=0)
python interpreter running is version 3
output header files created in soc_config/ directory
C:\mpfs-bare-metal-sw-config-generator\lib>
~~~~

#### Example generating from a command line in Linux
~~~~
vagrant@ubuntu-xenial:/home/mpfs-bare-metal-sw-config-generator/lib$ python3 mpfs_configuration_generator.py pf_soc_hw_description_reference.xml
generate header files for Embedded software project
mpfs_configuration_generator.py
python interpreter details: sys.version_info(major=3, minor=5, micro=2, releaselevel='final', serial=0)
python interpreter running is version 3
output header files created in soc_config/ directory
vagrant@ubuntu-xenial:/home/mpfs-bare-metal-sw-config-generator/lib$ 
~~~~

## 4. Integrate *soc_config* folder into Embedded Software Project <a name="Int"></a>

This section describes how to integrate the *soc_config* folder into an embedded software project.

##### Project directory structure, showing where soc_config folder sits.
~~~
   +---------+      +-----------+
   | src     +----->|application|
   +---------+  |   +-----------+
                |                                  
                |   +-----------+     +---------+      +----------+
                +-->|boards     +---->| <board> +----->|soc_config|
                |   +-----------+     +---------+      +----------+
                |
                |   +-----------+
                +-->|middleware |
                |   +-----------+
                |
                |   +-----------+     +---------+
                +-->|platform   +---->|hal      |
                    +-----------+  |  +---------+ 
                                   |
                                   |  +---------+      +----------+
                                   +->|drivers  +----->|mss_uart  |
                                   |  +---------+      +----------+              
                                   |
                                   |  +---------+      +----------+
                                   +->|mpfs_hal +----->|nwc       |
                                      +---------+      +----------+
~~~


Please follow the recommended steps
1. Delete the *platform/config/soc_config* folder in the Embedded Software project.
2. Copy the generated subdirectory *soc_config* into the project *platform/config/* folder


