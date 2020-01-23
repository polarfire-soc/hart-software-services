# Title: README.txt
# 
# Linux Kernel / Uboot Modules
#
# This code is provided with the following copyright:
#
#/* ******************************************************************************
#* Copyright (c) 2015 Microsemi, Inc.                                            *
#* Permission is hereby granted, free of charge, to any person obtaining a copy  *
#* of this software and associated documentation files (the "Software"), to deal *
#* in the Software without restriction, including without limitation the rights  *
#* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell     *
#* copies of the Software, and to permit persons to whom the Software is         *
#* furnished to do so, subject to the following conditions:                      *
#* The above copyright notice and this permission notice shall be included in    *
#* all copies or substantial portions of the Software.                           *
#* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    *
#* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,      *
#* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE   *
#* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER        *
#* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
#* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN     *
#* THE SOFTWARE.                                                                 *
#******************************************************************************* */
#
#
# The Directory structure is as follows:
# vtss_api_lite - Top Level Directory
# vtss_api_lite:appl - Sample Application to configure and access PHY using the specified PHY Evaluation Board
# vtss_api_lite:include - Include files that define Public Structures and Interfaces
# vtss_api_lite:phy_1g - 1G PHY Directory containing all PHY Families currently supported
# vtss_api_lite:phy_1g:common - Common PHY Functions (read/write, etc)
# vtss_api_lite:phy_1g:elise - Elise Family, PHY supported functions (VSC8514)
# vtss_api_lite:phy_1g:nano - Nano Family, PHY supported functions (VSC8501/VSC8502) 
# vtss_api_lite:phy_1g:tesla - Tesla Family, PHY supported functions (VSC8574/VSC8572/VSC8504/VSC8552)
# vtss_api_lite:phy_1g:viper - Viper Family, PHY supported functions (VSC8584/VSC8582/VSC8575/VSC8564/VSC8586
# vtss_api_lite:phy_1g:mini  - Mini Family, PHY supported functions (VSC8530/VSC8531/VSC85R40/VSC8541
#
# These modules contain code to do the following:
#    1. initialize_xxxxx_phy()  - Initialize the PHY by running init_scripts to optimize PHY performance and download Micro-code Updates
#    2. reset_xxxxx_phy() - After setting the MAC/Media Interface, a Soft Reset of the PHY is required for it to take effect.
#    3. xxxxx_phy_config_set() - Configuration of the PHY
#    4. User Configuration Step - User configures / Adjusts the configuration of the PHY as necessary
#    5. post_reset_xxxxx_phy(); - Functions to configure the MAC SerDes (SGMII/QSGMII)
#             where xxxxx = PHY Family
#    6. User Releases Coma mode, if applicable
#    7. xxxxx_phy_status_get(), Retrieve the current Status of the PHY
#
#    To compile, See the build scripts in the top level directory.
#    These build scripts create an executable.
#    This executable is a DEMO type applicaiton that can be run on a Linux host connected to a PHY Eval board
#    This DEMO applicaiton was used as a method to exercise the PHY HW on an Eval Board using the VTSS_API_LITE code
#    This DEMO application is not meant to be included as a part of a U-Boot environment, 
#         rather this Demo provides a template/guidance on the function calls sequence and usage.
#        - build_elise_vsc8514ev.sh
#        - build_nano_vsc8502ev.sh
#        - build_tesla_vsc8574ev.sh
#        - build_viper_vsc8584ev.sh
#        - build_mini_vsc8541ev.sh
#
#    The customer Application should be calling the API functions in Phy_1g directory with headers exposed for: 
#        Viper PHY: include/vtss_viper_phy_prototypes.h
#        Tesla PHY: include/vtss_tesla_phy_prototypes.h
#        Elise PHY: include/vtss_elise_phy_prototypes.h
#        NANO  PHY: include/vtss_nano_phy_prototypes.h
#        Mini  PHY: include/vtss_mini_phy_prototypes.h
#
# Brief description of the structure phy_control_t used in the Interface:
#    Each Interface uses the structure "phy_control_t" which is defined in the file: vtss_phy_common.h located in the vtss_api_lite/include directory
#    The "phy_addr" is a value passed through to the MDIO read/write wrapper functions - Selected by User
#    The "user_addr" can be used as a sub-addr - Selected by user, and is a value passed through to the MDIO read/write wrapper functions
#    The "phy_id" is the PHY Identification that is filled in by the function: vsc_get_phy_type() called prior to initialize.
#    The "mac_if" is the PHY MAC i/f selection - Selected by User.  For QSGMII, this is only set for Port 0 of the PHY.
#    The "media_if" is the PHY MEDIA i/f selection - Selected by User.
#    The "phy_usleep" is the function defined for the System Micro-Sleep - Selected by User for specified system.
#    The "phy_read_reg" is the function defined for MDIO Read of PHY Registers, this may be a wrapper function - Selected by User for specified system.
#    The "phy_write_reg" is the function defined for MDIO Write of PHY Registers, this may be a wrapper function - Selected by User for specified system.
#
#
#    The Recommended Sequence of Operation is as follows:
#    1. Create PHY Control structure phy_control_t and configure, hooking up MDIO Read/Write functions, phy_usleep, etc. for specified port
#    2. initialize_xxxxx_phy() - Performed on Port 0 of the PHY 
#    3. reset_xxxxx_phy() - Performed on Each port of the PHY
#    4. xxxxx_phy_config_set() - Performed configuration of the PHY Port, Performed on each port of the PHY
#    5. post_reset_xxxxx_phy() - Performed on Port 0 of the PHY
#
#    The Expected System behavior:
#    1. There is NO locking mechanisms provided.  Uboot is single threaded and locking is not required.
#            The code is expected to be allowed to execute until completion without interruption.
#    2. If used in a Linux a driver, be aware of the phy_usleep(). 
#
#    Sample code for each PHY type can be found in the vtss_api_lite/appl directory.
#    This particular code uses a PHY Eval board as a target platform.
#    The MDIO read/write "wrapper" functions can be found in each Eval Board file which translates the MDIO accesses for the given board.
#
# 
