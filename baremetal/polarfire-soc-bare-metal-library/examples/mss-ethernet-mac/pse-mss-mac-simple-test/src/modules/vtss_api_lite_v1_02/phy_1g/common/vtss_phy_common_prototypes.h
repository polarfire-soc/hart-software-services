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
#ifndef _VTSS_PHY_COMMON_PROTO_H_
#define _VTSS_PHY_COMMON_PROTO_H_
#include "vtss_phy_common.h"

extern int32_t vsc_phy_read(vsc_phy_control_t *cntrl, uint16_t page, uint16_t reg, uint16_t *rd_reg_val);
extern int32_t vsc_phy_write(vsc_phy_control_t *cntrl, uint16_t page, uint16_t reg, uint16_t wr_reg_val, uint16_t mask);
extern int32_t vsc_phy_wait_for_micro(vsc_phy_control_t *cntrl);
extern int32_t vsc_get_phy_type(vsc_phy_control_t *cntrl);
extern uint16_t vsc_phy_chip_port_get(vsc_phy_control_t *cntrl);

extern int32_t vsc_get_phy_media_if_config(const vsc_phy_media_interface_t media_if, uint16_t *cmd_100fx, uint8_t *media_op, uint8_t *cu_pref);

extern int32_t vsc_phy_sd6g_patch(vsc_phy_control_t *cntrl);
extern int32_t vsc_phy_sd6g_csr_reg_read_debug(vsc_phy_control_t *cntrl, const uint32_t csr_reg, uint32_t *csr_reg_val);

extern uint8_t  vsc_phy_chk_serdes_patch_init(vsc_phy_control_t  *cntrl);
extern uint8_t  vsc_phy_chk_serdes_init_mac_mode(vsc_phy_control_t  *cntrl);

extern int32_t vsc_phy_coma_mode_set(vsc_phy_control_t *cntrl, uint8_t pull_low);


#endif
