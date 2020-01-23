/* ******************************************************************************
* Copyright © 2015 Microsemi Corporation                                        *
* Permission is hereby granted, free of charge, to any person obtaining a copy  *
* of this software and associated documentation files (the "Software"), to deal *
* in the Software without restriction, including without limitation the rights  *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell     *
* copies of the Software, and to permit persons to whom the Software is         *
* furnished to do so, subject to the following conditions:                      *
* The above copyright notice and this permission notice shall be included in    *
* all copies or substantial portions of the Software.                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,      *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE   *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER        *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN     *
* THE SOFTWARE.                                                                 *
******************************************************************************* */

#ifndef _VTSS_VIPER_PHY_H_
#define _VTSS_VIPER_PHY_H_

#include "vtss_phy_common.h"

/* function prototypes */
/**
 * \brief PHY Initialization Performed Prior to a PHY Reset, Must be call on Base Port of PHY Only.
 *        Load init scripts and micro-code patch for whole PHY
 *
 * \param cntrl [IN]    PHY port PHY port control struct.
 *
 * \return Return code. VTSS_RC_OK if all Ok
 *                      VTSS_RC_ERROR if and error occurred.
 **/
extern int32_t initialize_viper_phy(vsc_phy_control_t * cntrl);

/**
 * \brief PHY Reset, Set MAC/MEDIA then Reset, Done to each PHY Port.
 *
 * \param cntrl [IN]      PHY port PHY port control struct.
 *
 * \return Return code. VTSS_RC_OK if all Ok
 *                      VTSS_RC_ERROR if and error occurred.
 **/
extern int32_t reset_viper_phy(vsc_phy_control_t * cntrl);

/**
 * \brief PHY Reset, Set MEDIA Only with NO Chg to MAC i/f, then Reset, Done to each PHY Port.
 *        This is a short form if the user desires to switch MEDIA i/f ONLY, after initial bootup
 *
 * \param cntrl [IN]      PHY port PHY port control struct.
 *
 * \return Return code. VTSS_RC_OK if all Ok
 *                      VTSS_RC_ERROR if and error occurred.
 **/
extern int32_t reset_viper_phy_media_chg(vsc_phy_control_t * cntrl);

/**
 * \brief PHY Configuration, Set PHY Config after PHY Reset, Done to each PHY Port.
 *
 * \param cntrl [IN]      PHY port PHY port control struct.
 * \param config [IN]     PHY port config struct.
 *
 * \return Return code. VTSS_RC_OK if all Ok
 *                      VTSS_RC_ERROR if and error occurred.
 **/
extern int32_t viper_phy_config_set(vsc_phy_control_t * cntrl, vsc_phy_conf_t * conf);

/**
 * \brief PHY Post Reset, Must be call after port PHY Reset (reset_viper_phy).
 *        Setup SerDes Init,  Done to Base Port of the PHY, Port 0 only
 *
 * \param cntrl [IN]      PHY port PHY port control struct.
 *
 * \return Return code. VTSS_RC_OK if all Ok
 *                      VTSS_RC_ERROR if and error occurred.
 **/
extern int32_t post_reset_viper_phy(vsc_phy_control_t * cntrl);

/**
 * \brief Function for getting the PHY Status Values.
 *
 * \param cntrl [IN]      PHY port PHY port control struct.
 * \param status [OUT]    PHY Status
 *
 * \return Return code. VTSS_RC_OK if all Ok
 *                      VTSS_RC_ERROR if and error occurred.
 **/
extern int32_t viper_phy_status_get(vsc_phy_control_t * cntrl, vsc_phy_port_status_t * status);

/**
 * \brief Debug function for setting the 6G SerDes ob_level value.
 *
 * \param cntrl [IN]      PHY port PHY port control struct.
 * \param ob_level [IN]   ob_level settings in serdes6g_ob_cfg1 for 6G SerDes Macro (See TN1052), 6 bits, default:0x18, Customizeable Amplitude Control.
 *
 * \return Return code. VTSS_RC_OK if all Ok
 *                      VTSS_RC_ERROR if and error occurred.
 **/
extern int32_t viper_phy_ob_level_set(vsc_phy_control_t  * cntrl, uint8_t  ob_level);

/**
 * \brief Debug function for reading the 6G SerDes ob_level value.
 *
 * \param cntrl [IN]      PHY port PHY port control struct.
 * \param ob_level [OUT]  ob_level settings in serdes6g_ob_cfg1 for 6G SerDes Macro (See TN1052), 6 bits, default:0x18, Customizeable Amplitude Control.
 *
 * \return Return code. VTSS_RC_OK if all Ok
 *                      VTSS_RC_ERROR if and error occurred.
 **/
extern int32_t viper_phy_ob_level_get(vsc_phy_control_t  * cntrl, uint8_t  *ob_level);

/**
 * \brief Debug function for setting the 6G SerDes ob_post0 and ob_post1 values.
 *
 * \param cntrl [IN]     PHY port control struct.
 * \param ob_post0 [IN]  ob_post0 settings in serdes6g_ob_cfg for 6G SerDes Macro (See TN1052), 6 bits, default:0, Customizeable.
 * \param ob_post1 [IN]  ob_post1 settings in serdes6g_ob_cfg for 6G SerDes Macro (See TN1052), 5 bits, default:0, Do not change.
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t viper_phy_ob_post_set(vsc_phy_control_t  * cntrl, uint8_t  ob_post0, uint8_t  ob_post1);

/**
 * \brief Debug function for reading the 6G SerDes ob_post0 and ob_post1 values.
 *
 * \param cntrl [IN]      PHY port control struct.
 * \param ob_post0 [OUT]  ob_post0 settings in serdes6g_ob_cfg for 6G SerDes Macro (See TN1052), 6 bits, default:0, Customizeable.
 * \param ob_post1 [OUT]  ob_post1 settings in serdes6g_ob_cfg for 6G SerDes Macro (See TN1052), 5 bits, default:0, Do not change.
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t viper_phy_ob_post_get(vsc_phy_control_t  * cntrl, uint8_t  *ob_post0, uint8_t  *ob_post1);

/**
 * \brief Function for Configuring the Recovered Clk.
 *        RESTRICTION: This function can ONLY be run on the BASE PORT of the PHY, Port 0
 *
 * \param cntrl [IN]      PHY port control struct.
 * \param clock_port [IN]  Set configuration for this clock port.
 * \param clk_src [IN]     Set the Clock Src for this port.
 * \param conf [IN]        PHY clock configuration.
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t viper_phy_recov_clk_conf_set(vsc_phy_control_t          *cntrl,
                                            vsc_phy_recov_clk_t         clock_port,
                                            vsc_phy_clk_conf_t         *const conf);

/**
 * \brief Function for Getting the Configuration of the Recovered Clk.
 *        RESTRICTION: This function can ONLY be run on the BASE PORT of the PHY, Port 0
 *                                          
 * \param cntrl [IN]       PHY port control struct.
 * \param clock_port [IN]  Retrieve the Configuration for this clock port.
 * \param conf [OUT]       PHY clock configuration.
 * \param clk_src [OUT]    Get the Port number on the PHY that is the Clock Src for this port.
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/                                        
extern int32_t viper_phy_recov_clk_conf_get(vsc_phy_control_t          *cntrl, 
                                            vsc_phy_recov_clk_t         clock_port,
                                            vsc_phy_clk_conf_t         *conf);

/**
 * \brief Function for Updating the Sig Threshold so PHY is more compatable with differnt SFP's.
 *        RESTRICTION: This function can ONLY be run on the BASE PORT of the PHY, Port 0
 *                                          
 * \param cntrl [IN]        PHY port control struct.
 * \param tgt_port_no [IN]  PHY Port No target to be adjusted, since this has to be run on Port 0.
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t viper_phy_media_sig_adjust(vsc_phy_control_t         *cntrl, 
                                          vsc_phy_media_interface_t  tgt_port_media_if,
                                          uint16_t                   tgt_port_no);
 
/**
 * \brief Function for Setting the Configuration For Master/Slave.
 *                                          
 * \param cntrl [IN]    PHY port control struct.
 * \param conf [IN]     PHY configuration.
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t viper_phy_conf_1g_set (vsc_phy_control_t   *cntrl, vsc_phy_conf_1g_t   *conf);


/**
 * \brief Function for Setting the PHY Loopback Configuration
 *                                          
 * \param cntrl [IN]    PHY port control struct.
 * \param loopback [IN] PHY Loopback configuration.
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t viper_phy_loopback_set(vsc_phy_control_t     *cntrl, vsc_phy_loopback_t   *loopback);

/**
 * \brief Function for Setting the PHY Jumbo Packet Mode Configuration
 *                                          
 * \param cntrl [IN]       PHY port control struct.
 * \param pkt_mode [IN]    PHY Jumbo Packet Mode configuration.
 *
 * \return Return code. VSC_PHY_OK if all Ok
 *                      VSC_PHY_ERROR if and error occurred.
 **/
extern int32_t viper_phy_packet_mode_set(vsc_phy_control_t          *cntrl,
                                         vsc_phy_pkt_mode_t          pkt_mode);





#endif
