#
# OS Portability Abstraction
#
# Detect if running under:
#   Windows/Command Line
#   Windows/SoftConsole
#   Linux/Command Line (in Desktop, including with/without SoftConsole)
#   Linux/Command Line (via terminal)
#

ifeq ($(OS), Windows_NT)
  #If we need to patch up path for Windows, we could do it here...
  #TOOLPATH:=${SC_INSTALL_DIR}riscv-unknown-elf-gcc\bin
  #export PATH:="$(TOOLPATH);$(PATH)"
  $(info INFO: Windows detected)
  HOST_WINDOWS:=true
  PYTHON?=python.exe
  MPFS_BOOTMODE_PROGRAMMER = $(SC_INSTALL_DIR)\eclipse\jre\bin\java.exe -jar $(SC_INSTALL_DIR)\extras\mpfs\mpfsBootmodeProgrammer.jar
  MPFS_BOOTMODE_PROGRAMMER:=$(subst \,/,$(MPFS_BOOTMODE_PROGRAMMER))
  export SC_INSTALL_DIR:=$(subst \,/,$(SC_INSTALL_DIR))
else
  SYSTEM:=$(shell uname -s)
  ifneq (, $(findstring Linux, $(SYSTEM)))         # Linux-specific mods
    # Workaround SoftConsole v2021.1 Linux Python limitations by using system python
    export PATH:=/usr/bin:$(PATH)
    $(info INFO: Linux detected)
    HOST_LINUX:=true
    PYTHON?=python3
    MPFS_BOOTMODE_PROGRAMMER = $(SC_INSTALL_DIR)/eclipse/jre/bin/java -jar $(SC_INSTALL_DIR)/extras/mpfs/mpfsBootmodeProgrammer.jar
    ifneq ($(origin XDG_SESSION_DESKTOP),undefined)
      HOST_LINUX_DESKTOP:=true
      $(info INFO: Linux Desktop detected)
    endif
    #
    # We could detect if running in SoftConsole on Linux, but we don't need to
    # as just detected a Desktop environment allows us run the guiconfig tool
    #ifeq ($(origin XDG_SESSION_CLASS),undefined)
    #  HOST_LINUX_SOFTCONSOLE:=true
    #endif
  else
    $(error Unsupported build platform $(SYSTEM))
  endif
endif

#$(info INFO: MPFS_BOOTMODE_PROGRAMMER is $(MPFS_BOOTMODE_PROGRAMMER))
#$(info INFO: SC_INSTALL_DIR is $(SC_INSTALL_DIR))
