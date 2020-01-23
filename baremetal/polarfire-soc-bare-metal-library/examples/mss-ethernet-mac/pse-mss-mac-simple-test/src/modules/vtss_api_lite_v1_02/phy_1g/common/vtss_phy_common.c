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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include "vtss_phy_common.h"
#include "vtss_phy_common_prototypes.h"

// #define PHY_PAGE_CHECK_DEBUG 3

#if 0
#undef CONFIG_PHY_DEBUG
#define CONFIG_PHY_DEBUG_ENABLE_PRINTF 5
#define PHY_PAGE_CHECK_DEBUG 3
#else
#undef CONFIG_PHY_DEBUG
#undef CONFIG_PHY_DEBUG_ENABLE_PRINTF
#endif

#if defined(CONFIG_PHY_DEBUG) && (CONFIG_PHY_DEBUG > 0)

#define DPRINTK(level, format, args...)  do { if (CONFIG_PHY_DEBUG >= (level)) \
                                                  printk(VSC_PHY format, ##args); } while (0)
#else

#if defined (CONFIG_PHY_DEBUG_ENABLE_PRINTF) && (CONFIG_PHY_DEBUG_ENABLE_PRINTF > 0)
#define DPRINTK(level, format, args...)  do { if (CONFIG_PHY_DEBUG_ENABLE_PRINTF >= (level)) \
                                                  printf(VSC_PHY_DEBUG format, ##args); } while (0)
#else
#define DPRINTK(level, format, args...)  do {} while (0)
#endif

#endif

int32_t vsc_phy_read(vsc_phy_control_t *cntrl, uint16_t page, uint16_t reg, uint16_t *rd_reg_val)
{
    int32_t       rc=0;
    uint16_t      reg_val=0;
    uint8_t       page_chk = 0;

#if defined(PHY_PAGE_CHECK_DEBUG) && (PHY_PAGE_CHECK_DEBUG > 0)
    page_chk = 1;
#endif

    reg = reg & 0x1f;

    if (page_chk) {
        rc = cntrl->phy_reg_read(cntrl->phy_addr, 0x1f, &reg_val);
        if (rc == VSC_PHY_OK) {
            if (reg_val != page) {
                DPRINTK(3,"<<< PHY RD PAGE ADDR CHK ERROR >>> Phy_Addr: 0x%x, Pg_Reg31: 0x%x - Expected Page:0x%X, Reading reg:0x%X \n", cntrl->phy_addr, reg_val, page, reg);
                //rc = VSC_PHY_ERROR;
                //return(rc);
            }
        }
    }

    if ((rc = cntrl->phy_reg_read(cntrl->phy_addr, reg, rd_reg_val)) == VSC_PHY_OK) {
        DPRINTK(4, "<Phy_Rd> Phy_Addr: 0x%x, Reg:0x%X, Read Val: 0x%04x \n", cntrl->phy_addr, reg, *rd_reg_val);
    } else {
        DPRINTK(2, "<Phy_Rd> PHY Read Error Detected, Phy_Addr: 0x%x, Reg:0x%X  \n", cntrl->phy_addr, reg) ;
    }

    return(rc);
}

int32_t vsc_phy_write(vsc_phy_control_t  *cntrl, uint16_t page, uint16_t reg, uint16_t wr_reg_val, uint16_t mask)
{
    int32_t       rc=0;
    uint16_t      rd_reg_val = 0;
    uint8_t       page_chk = 0;
 
#if defined(PHY_PAGE_CHECK_DEBUG) && (PHY_PAGE_CHECK_DEBUG > 0)
    page_chk = 1;
#endif

    reg = reg & 0x1f;

    if (page_chk) {
        if ((rc = cntrl->phy_reg_read(cntrl->phy_addr, 0x1f, &rd_reg_val)) == VSC_PHY_OK) {
            if (rd_reg_val != page) {
                DPRINTK(3,"<<< PHY WR PAGE ADDR CHK ERROR >>> Phy_Addr: 0x%x, Pg_Reg31: 0x%x - Expected Page:0x%X, Reading reg:0x%X \n", cntrl->phy_addr, rd_reg_val, page, reg);
                //rc = VSC_PHY_ERROR;
                //return(rc);
            }
        } else {
            DPRINTK(2, "<Phy_Rd> PHY Write Error Detected on Page_Chk, Phy_Addr: 0x%x, Reg:0x%X  \n", cntrl->phy_addr, reg) ;
        }
    }

    if (mask != 0xffff) {  // If the MASK is not 0xffff, then key to do Read-Modify-Write
        /* Read-modify-write */
        if ((rc = cntrl->phy_reg_read(cntrl->phy_addr, reg, &rd_reg_val)) == VSC_PHY_OK) {
            if ((rc = cntrl->phy_reg_write(cntrl->phy_addr, reg, (rd_reg_val & ~mask) | (wr_reg_val & mask))) == VSC_PHY_OK) {
                DPRINTK(4, "<Phy_Rd_Mod_Wr> Phy_Addr: 0x%x, Reg:0x%X, Rd value:0x%04x, Write Val: 0x%04x;  Mask: 0x%04x \n", 
                     cntrl->phy_addr, reg, rd_reg_val, (rd_reg_val & ~mask) | (wr_reg_val & mask), mask);
            } else {
                DPRINTK(2, "<Phy_Rd_Mod_Wr> PHY Write Error Detected, Phy_Addr: 0x%x, Reg:0x%X  \n", cntrl->phy_addr, reg) ;
            }
        } else {
            DPRINTK(2, "<Phy_Rd_Mod_Wr> PHY Read Error Detected, Phy_Addr: 0x%x, Reg:0x%X  \n", cntrl->phy_addr, reg) ;
        }

    } else {
        /* Write */
        if ((rc = cntrl->phy_reg_write(cntrl->phy_addr, reg, wr_reg_val)) == VSC_PHY_OK) {
            DPRINTK(4, "<Phy_Wr> Phy_Addr: 0x%x, Reg:0x%X, Write Val: 0x%04x \n", cntrl->phy_addr, reg, wr_reg_val);
        } else {
            DPRINTK(2, "<Phy_Wr> PHY Write Error Detected, Phy_Addr: 0x%x, Reg:0x%X  \n", cntrl->phy_addr, reg) ;
        }
    }

    return(rc);
}

int32_t vsc_phy_wait_for_micro(vsc_phy_control_t *cntrl)
{
    uint16_t      timeout = VSC_PHY_MICRO_TIMEOUT;
    uint16_t      reg_val = 0;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x12, &reg_val));
    while (reg_val & 0x8000 && timeout > 0) {
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x12, &reg_val));
        timeout--;
        cntrl->phy_usleep(1000);
    }

    if (timeout == 0) {
        return VSC_PHY_TIMEOUT;
    }

    return(VSC_PHY_OK);
}

/* Function to set the current micro peek/poke address */
static int32_t vsc_phy_set_micro_set_addr(vsc_phy_control_t *cntrl, const uint16_t    addr)
{
    uint16_t              reg18g;

    switch (cntrl->phy_id.family) {
    case VSC_PHY_FAMILY_VIPER:
        /* This could be an issue as the write to 25E1 should be to Port 0 of the PHY */
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x19, addr, 0xffff));   // use 25E1 as a scratch pad to save data 
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        reg18g = 0xc000;
        break;

        break;

    case VSC_PHY_FAMILY_TESLA:
        if (cntrl->phy_id.revision > VSC_PHY_TESLA_REV_A) {
            if (addr & 0x4000) {
                reg18g = 0xd000 | (addr & 0xfff);
            } else {
                reg18g = 0xc000 | (addr & 0xfff);
            }
            break;
        }

    default:
        DPRINTK(3, "Micro PEEK-Cmd not supported, PHY family %d, revision %u \n", cntrl->phy_id.family, cntrl->phy_id.revision);
        return (VSC_PHY_ERROR);
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff)); // Switch back to micro/GPIO register-page
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, reg18g, 0xffff)); // set micro peek/poke address
    CHK_RC(vsc_phy_wait_for_micro(cntrl));

    return (VSC_PHY_OK);
}


/* Function to peek a byte from the current micro address */
static int32_t vsc_phy_micro_peek(vsc_phy_control_t *cntrl, uint8_t  post_increment, uint8_t  *peek_byte_ptr)
{
    uint16_t           reg18g=0;

    switch (cntrl->phy_id.family) {
    case VSC_PHY_FAMILY_VIPER:
        break;

    case VSC_PHY_FAMILY_TESLA:
        if (cntrl->phy_id.revision > VSC_PHY_TESLA_REV_A) {
            break;
        }

    default:
        DPRINTK(3, "Micro PEEK-Cmd not supported, PHY family %d, revision %u", cntrl->phy_id.family, cntrl->phy_id.revision);
        return (VSC_PHY_ERROR);
    }

    // Setup peek command with or without post-increment
    reg18g = (post_increment) ? 0x9007 : 0x8007;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, reg18g, 0xffff)); // Peek micro memory
    CHK_RC(vsc_phy_wait_for_micro(cntrl));

    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x12, &reg18g));  // read to get peek'ed byte value
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    if (reg18g & 0x4000) {
       *peek_byte_ptr = 0;
        return (VSC_PHY_ERROR);
    }

    *peek_byte_ptr = (reg18g >> 4) & 0xff;

    return (VSC_PHY_OK);
}

/* Function to poke a byte to the current micro address */
static int32_t vsc_phy_micro_poke(vsc_phy_control_t *cntrl, uint8_t  post_increment, uint8_t  poke_byte)
{
    uint16_t           reg18g=0;

    switch (cntrl->phy_id.family) {
    case VSC_PHY_FAMILY_VIPER:
        break;

    case VSC_PHY_FAMILY_TESLA:
        if (cntrl->phy_id.revision > VSC_PHY_TESLA_REV_A) {
            break;
        }

    default:
        DPRINTK(3, "Micro POKE-Cmd not supported, PHY family %d, revision %u", cntrl->phy_id.family, cntrl->phy_id.revision);
        return (VSC_PHY_ERROR);
    }

    // Setup peek command with or without post-increment
    reg18g = ((post_increment) ? 0x9006 : 0x8006) | (((uint16_t)poke_byte) << 4);

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, reg18g, 0xffff)); // Poke byte into micro memory
    CHK_RC(vsc_phy_wait_for_micro(cntrl));

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    return (VSC_PHY_OK);
}

/* Function to wait for the CSR Ring to clear */
static int32_t vsc_phy_wait_for_csr_ring_busy(vsc_phy_control_t  *cntrl, uint32_t  page)
{
    uint16_t val;
    uint8_t  timeout = VSC_PHY_CSR_BUSY_TIMEOUT;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT4_PAGE, 0x1f, VSC_PHY_EXT4_PAGE, 0xffff));

    // Wait for bit 15 to be set (or timeout)
    if (page == 19) {
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT4_PAGE, 0x13, &val));

        while (!(val & 0x8000) && timeout)  {
            cntrl->phy_usleep(1);
            timeout--;
            CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT4_PAGE, 0x13, &val));
        }
    } else if (page == 20) {
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT4_PAGE, 0x14, &val));

        while (!((val & 0x6000) == 0) && timeout)  {
            cntrl->phy_usleep(1);
            timeout--;
            CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT4_PAGE, 0x14, &val));
        }
    } else {
        return(VSC_PHY_ERROR);
    }

    if (timeout == 0) {
        return (VSC_PHY_CSR_TIMEOUT);
    } else {
        return (VSC_PHY_OK);
    }
}

/* Function to get the PHY Chip Port  */
uint16_t vsc_phy_chip_port_get(vsc_phy_control_t  *cntrl)
{
    uint16_t reg_val;
    uint16_t current_page;
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x1f, &current_page));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT1_PAGE, 0x17, &reg_val));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, current_page, 0xffff));
    return (reg_val & 0xF800) >> 11;
}


/* Function to Read from a particular Tgt and Reg Addr from the CSR Ring  */
static int32_t vsc_phy_csr_ring_read(vsc_phy_control_t  *cntrl, 
                                     const uint16_t      target,
                                     const uint32_t      csr_reg_addr,
                                     uint32_t           *value)
{
    uint8_t  macsec_capable = 0;
    uint16_t reg_value = 0;
    uint16_t target_tmp = 0;
    uint16_t mask = 0;
    uint16_t reg_value_lower=0;
    uint16_t reg_value_upper=0;

    if ((cntrl->phy_id.family == VSC_PHY_FAMILY_VIPER) &&
        ((cntrl->phy_id.part_number == VSC_PHY_TYPE_8582) ||
         (cntrl->phy_id.part_number == VSC_PHY_TYPE_8584) ||
         (cntrl->phy_id.part_number == VSC_PHY_TYPE_8564))) {
        macsec_capable = 1;
    }

    if (!macsec_capable && (target == 0x38 || target == 0x3C)) {
        return (VSC_PHY_INVALID_REQ);
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT4_PAGE, 0xffff));

    vsc_phy_wait_for_csr_ring_busy(cntrl, 19);
    target_tmp = target >> 2;
    mask = (0x1 << 4) - 0x1;
    reg_value = ((target_tmp & mask) << 0);
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT4_PAGE, 0x14, reg_value, 0xffff));

    if ((target >> 2) == 1) {
        target_tmp = target & 3; // non-macsec access
    }

    vsc_phy_wait_for_csr_ring_busy(cntrl, 19);

    mask = (0x1 << 14) - 0x1;
    reg_value = (0x1 << 15) | ((target_tmp & 0x3) << 12) | (0x1 << 14) | ((csr_reg_addr & mask) << 0);
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT4_PAGE, 0x13, reg_value, 0xffff));

    vsc_phy_wait_for_csr_ring_busy(cntrl, 19);

    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT4_PAGE, 0x11, &reg_value_lower));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT4_PAGE, 0x12, &reg_value_upper));

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT4_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    *value = (uint32_t)(reg_value_upper << 16) | reg_value_lower;

    return (VSC_PHY_OK);
}

/* Function to Write to a particular Tgt and Reg Addr on the CSR Ring  */
static int32_t vsc_phy_csr_ring_write(vsc_phy_control_t  *cntrl, 
                                      const uint16_t      target,
                                      const uint32_t      csr_reg_addr,
                                      const uint32_t      value)
{

    /* Divide the 32 bit value to [15..0] Bits & [31..16] Bits */
    uint16_t target_tmp = 0;
    uint16_t reg_value_lower = (value & 0xffff);
    uint16_t reg_value_upper = ((value & 0xffff0000) >> 16);
    uint16_t reg_value = 0;
    uint16_t mask = 0;
    uint8_t  macsec_capable = 0;

    if ((cntrl->phy_id.family == VSC_PHY_FAMILY_VIPER) &&
        ((cntrl->phy_id.part_number == VSC_PHY_TYPE_8582) ||
         (cntrl->phy_id.part_number == VSC_PHY_TYPE_8584) ||
         (cntrl->phy_id.part_number == VSC_PHY_TYPE_8564))) {
        macsec_capable = 1;
        DPRINTK(5, "MACSEC capable\n");
    } else {
        DPRINTK(5, "NOT MACSEC capable\n");
    }

    if (!macsec_capable && (target == 0x38 || target == 0x3C)) {
        return (VSC_PHY_INVALID_REQ);
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT4_PAGE, 0xffff));

    vsc_phy_wait_for_csr_ring_busy(cntrl, 19);
    target_tmp = target >> 2;
    mask = (0x1 << 4) - 0x1;
    reg_value = ((target_tmp & mask) << 0);

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT4_PAGE, 0x14, reg_value, 0xffff));

    if (target >> 2 == 1 || target >> 2 == 3) {
        target_tmp = target;
    }

    vsc_phy_wait_for_csr_ring_busy(cntrl, 19);

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT4_PAGE, 0x11, reg_value_lower, 0xffff));
    vsc_phy_wait_for_csr_ring_busy(cntrl, 19);

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT4_PAGE, 0x12, reg_value_upper, 0xffff));
    vsc_phy_wait_for_csr_ring_busy(cntrl, 19);

    mask = (0x1 << 14) - 0x1;
    reg_value = (0x1 << 15) | ((target_tmp & 0x3) << 12) | ((csr_reg_addr & mask) << 0);

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT4_PAGE, 0x13, reg_value, 0xffff));
    vsc_phy_wait_for_csr_ring_busy(cntrl, 19);

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT4_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    return (VSC_PHY_OK);
}

static int32_t vsc_phy_sd6g_des_cfg_read(vsc_phy_control_t     *cntrl,
                                         uint8_t               *des_phy_ctrl,
                                         uint8_t               *des_mbtr_ctrl,
                                         uint8_t               *des_bw_hyst,
                                         uint8_t               *des_bw_ana)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x21;
    uint32_t           reg_val = 0;

    CHK_RC(vsc_phy_csr_ring_read(cntrl, tgt, csr_reg_addr, &reg_val)); // des_cfg
    *des_phy_ctrl =  (uint8_t) ((reg_val & 0x0001e000) >> 13);     // bits 16:13
    *des_mbtr_ctrl = (uint8_t) ((reg_val & 0x00001c00) >> 10);     // bits 12:10
    *des_bw_hyst =   (uint8_t) ((reg_val & 0x000000e0) >> 5);      // bits  7:5
    *des_bw_ana =    (uint8_t) ((reg_val & 0x0000000e) >> 1);      // bit   3:1

    return (VSC_PHY_OK);
}

static int32_t vsc_phy_sd6g_des_cfg_write(vsc_phy_control_t     *cntrl,
                                          const uint8_t          des_phy_ctrl,
                                          const uint8_t          des_mbtr_ctrl,
                                          const uint8_t          des_bw_hyst,
                                          const uint8_t          des_bw_ana)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x21;
    uint32_t           reg_val = 0;

    // configurable terms
    reg_val = ((uint32_t)(des_phy_ctrl) << 13) | ((uint32_t)(des_mbtr_ctrl) << 10);
    reg_val |= ((uint32_t)(des_bw_hyst) << 5) | ((uint32_t)(des_bw_ana) << 1);
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));
}

static int32_t vsc_phy_sd6g_inbuf_cfg0_read(vsc_phy_control_t     *cntrl,
                                            uint8_t               *ib_rtrm_adj,
                                            uint8_t               *ib_sig_det_clk_sel,
                                            uint8_t               *ib_reg_pat_sel_offset,
                                            uint8_t               *ib_cal_ena)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x22;
    uint32_t           reg_val = 0;

    CHK_RC(vsc_phy_csr_ring_read(cntrl, tgt, csr_reg_addr, &reg_val)); // ib_cfg0
    *ib_rtrm_adj =           (uint8_t) ((reg_val & 0x1E000000) >> 25);  // bits 28:25
    *ib_sig_det_clk_sel =    (uint8_t) ((reg_val & 0x00070000) >> 16);  // bits 18:16
    *ib_reg_pat_sel_offset = (uint8_t) ((reg_val & 0x00000300) >> 8);   // bits 9:8
    *ib_cal_ena =            (uint8_t) ((reg_val & 0x00000008) >> 3);   // bit 3
    return (VSC_PHY_OK);
}


static int32_t vsc_phy_sd6g_inbuf_cfg0_write(vsc_phy_control_t     *cntrl,
                                             const uint8_t          ib_rtrm_adj,
                                             const uint8_t          ib_sig_det_clk_sel,
                                             const uint8_t          ib_reg_pat_sel_offset,
                                             const uint8_t          ib_cal_ena)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x22;
    uint32_t           reg_val = 0;
    uint32_t           base_val = 0;

    // constant terms
    base_val = (1 << 30) | (1 << 29) | (5 << 21) | (1 << 19) | (1 << 14) | (1 << 12) | (2 << 10) | (1 << 5) | (1 << 4) | 7;
    // configurable terms
    reg_val = base_val;
    reg_val |= ((uint32_t)(ib_rtrm_adj) << 25) | ((uint32_t)(ib_sig_det_clk_sel) << 16) | ((uint32_t)(ib_reg_pat_sel_offset) << 8) | ((uint32_t)(ib_cal_ena) << 3);
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));
}

static int32_t vsc_phy_sd6g_inbuf_cfg1_write(vsc_phy_control_t     *cntrl,
                                             const uint8_t          ib_tjtag,
                                             const uint8_t          ib_tsdet,
                                             const uint8_t          ib_scaly,
                                             const uint8_t          ib_frc_offset)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x23;
    uint32_t           reg_val = 0;
    uint32_t           ib_filt_val = 0;
    uint32_t           ib_frc_val = 0;

    // constant terms
    ib_filt_val = (1 << 7) + (1 << 6) + (1 << 5) + (1 << 4);
    ib_frc_val = (0 << 3) + (0 << 2) + (0 << 1);
    // configurable terms
    reg_val  = ((uint32_t)ib_tjtag << 17) + ((uint32_t)ib_tsdet << 12) + ((uint32_t)ib_scaly << 8) + ib_filt_val + ib_frc_val + ((uint32_t)ib_frc_offset << 0);
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));
}

static int32_t vsc_phy_sd6g_inbuf_cfg2_read(vsc_phy_control_t     *cntrl,
                                            uint8_t               *ib_tinfv,
                                            uint8_t               *ib_tcalv,
                                            uint8_t               *ib_ureg)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x24;
    uint32_t           reg_val = 0;

    CHK_RC(vsc_phy_csr_ring_read(cntrl, tgt, csr_reg_addr, &reg_val));
    *ib_tinfv =  (uint8_t) ((reg_val & 0x70000000) >> 28);  // bits 30:28
    *ib_tcalv =  (uint8_t) ((reg_val & 0x000003e0) >> 5);   // bits  9:5
    *ib_ureg =   (uint8_t) ((reg_val & 0x00000007) >> 0);   // bits  2:0

    return (VSC_PHY_OK);
}

static int32_t vsc_phy_sd6g_inbuf_cfg2_write(vsc_phy_control_t     *cntrl,
                                             const uint8_t          ib_tinfv,
                                             const uint8_t          ib_tcalv,
                                             const uint8_t          ib_ureg)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x24;
    uint32_t           reg_val = 0;
    uint32_t           base_val;

    // constant terms
    base_val = 0x0f878010;
    reg_val = base_val | ((uint32_t)(ib_tinfv) << 28) | ((uint32_t)(ib_tcalv) << 5) | ((uint32_t)(ib_ureg) << 0);
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));
}

static int32_t vsc_phy_sd6g_inbuf_cfg3_write(vsc_phy_control_t     *cntrl,
                                             const uint8_t          ib_ini_hp,
                                             const uint8_t          ib_ini_mid,
                                             const uint8_t          ib_ini_lp,
                                             const uint8_t          ib_ini_offset)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x25;
    uint32_t           reg_val = 0;

    reg_val  = ((uint32_t)ib_ini_hp << 24) + ((uint32_t)ib_ini_mid << 16) + ((uint32_t)ib_ini_lp << 8) + ((uint32_t)ib_ini_offset << 0);
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));
}

static int32_t vsc_phy_sd6g_inbuf_cfg4_write(vsc_phy_control_t     *cntrl,
                                             const uint8_t          ib_max_hp,
                                             const uint8_t          ib_max_mid,
                                             const uint8_t          ib_max_lp,
                                             const uint8_t          ib_max_offset)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x26;
    uint32_t           reg_val = 0;

    reg_val  = ((uint32_t)ib_max_hp << 24) + ((uint32_t)ib_max_mid << 16) + ((uint32_t)ib_max_lp << 8) + ((uint32_t)ib_max_offset << 0);
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));
}

static int32_t vsc_phy_sd6g_outbuf_cfg_write(vsc_phy_control_t     *cntrl,
                                             const uint8_t          ob_ena1v_mode,
                                             const uint8_t          ob_pol,
                                             const uint8_t          ob_post0,
                                             const uint8_t          ob_post1,
                                             const uint8_t          ob_sr_h,
                                             const uint8_t          ob_resistor_ctr,
                                             const uint8_t          ob_sr)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x28;
    uint32_t           reg_val = 0;

    // configurable terms
    reg_val |= (uint32_t)(ob_ena1v_mode) << 30 | (uint32_t)(ob_pol) << 29 | (uint32_t)(ob_post0) << 23 | (uint32_t)(ob_post1) << 18 |
               (uint32_t)(ob_sr_h) << 8 | (uint32_t)(ob_resistor_ctr) << 4 | (uint32_t)(ob_sr) << 0;

    // Modify the Values for ob_post0 and ob_post1
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));
}

static int32_t vsc_phy_sd6g_outbuf_cfg1_write(vsc_phy_control_t     *cntrl, 
                                              const uint8_t          ob_ena_cas,
                                              const uint8_t          ob_lev)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x29;
    uint32_t           reg_val = 0;

    // configurable terms - Modify the Values for ob_ena_cas and ob_lev
    reg_val |= (uint32_t)(ob_ena_cas) << 6 | (uint32_t)(ob_lev) << 0;
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));
}


static int32_t vsc_phy_sd6g_pll_cfg_write(vsc_phy_control_t     *cntrl,
                                          const uint8_t          pll_ena_offs,
                                          const uint8_t          pll_fsm_ctrl_data,
                                          const uint8_t          pll_fsm_ena)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x2b;
    uint32_t           reg_val = 0;

    reg_val  = ((uint32_t)pll_ena_offs << 21) + ((uint32_t)pll_fsm_ctrl_data << 8) + ((uint32_t)pll_fsm_ena << 7);
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));
}

static int32_t vsc_phy_sd6g_common_cfg_write(vsc_phy_control_t     *cntrl,
                                             const uint8_t          sys_rst,
                                             const uint8_t          ena_lane,
                                             const uint8_t          ena_loop,
                                          const uint8_t          qrate,
                                          const uint8_t          if_mode)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x2c;
    uint32_t           reg_val = 0;

    reg_val  = ((uint32_t)sys_rst << 31) + ((uint32_t)ena_lane << 18) + ((uint32_t)ena_loop << 8) + ((uint32_t)qrate << 6) + ((uint32_t)if_mode << 4);
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));
}

static int32_t vsc_phy_sd6g_gp_cfg_write(vsc_phy_control_t     *cntrl,
                                         const uint32_t         gp_cfg_val)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x2e;

    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, gp_cfg_val));
}

static int32_t vsc_phy_sd6g_misc_cfg_write(vsc_phy_control_t     *cntrl,
                                           const uint8_t          lane_rst)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x3b;
    uint32_t           reg_val = lane_rst;

    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));
}

static int32_t vsc_phy_pll5g_cfg0_write(vsc_phy_control_t     *cntrl,
                                        const uint8_t          selbgv820)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x06;
    uint32_t           reg_val = 0x7036f145;

    reg_val |= ((uint32_t)(selbgv820) << 23);
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));
}

static int32_t vsc_phy_pll5g_cfg2_write(vsc_phy_control_t     *cntrl,   
                                        const uint8_t          disable_fsm,
                                     const uint8_t          ena_clk_bypass)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x08;
    uint32_t           reg_val = 0;

    CHK_RC(vsc_phy_csr_ring_read(cntrl, tgt, csr_reg_addr, &reg_val));
    reg_val &= 0xff7ffffd;
    reg_val |= ((uint32_t)disable_fsm << 1) | ((uint32_t)ena_clk_bypass << 23);
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));
}

// Macro for making sure that we don't run forever
#define SD6G_TIMEOUT(timeout_var) if (timeout_var-- == 0) {return (VSC_PHY_SD6G_TIMEOUT);} else {cntrl->phy_usleep(1000);}

// trigger a write to the spcified MCB
static int32_t vsc_phy_mcb_write_trigger(vsc_phy_control_t     *cntrl,
                                         const uint32_t         mcb_reg_addr,
                                         const uint8_t          mcb_slave_num)
{
    const uint16_t     tgt = 0x7;
    uint32_t           rd_dat = 0;
    uint8_t            timeout = 200;

    CHK_RC(vsc_phy_csr_ring_write(cntrl, tgt, mcb_reg_addr, (0x80000000 | (1L << mcb_slave_num))));
    do {
        CHK_RC(vsc_phy_csr_ring_read(cntrl, tgt, mcb_reg_addr, &rd_dat)); // wait for MCB write to complete
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x80000000);   // Wait for Bit 31 to clear
    return (VSC_PHY_OK);
}

// trigger a read to the spcified MCB
static int32_t vsc_phy_mcb_read_trigger(vsc_phy_control_t     *cntrl,
                                        const uint32_t         mcb_reg_addr,
                                        const uint8_t          mcb_slave_num)
{
    const uint16_t     tgt = 0x7;
    uint32_t           rd_dat = 0;
    uint8_t            timeout = 200;

    CHK_RC(vsc_phy_csr_ring_write(cntrl, tgt, mcb_reg_addr, (0x40000000 | (1L << mcb_slave_num))));
    do {
        CHK_RC(vsc_phy_csr_ring_read(cntrl, tgt, mcb_reg_addr, &rd_dat)); // wait for MCB read to complete
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x40000000);   // Wait for Bit 30 to clear
    return (VSC_PHY_OK);
}

static int32_t vsc_phy_sd6g_csr_reg_read(vsc_phy_control_t        *cntrl,
                                         const uint16_t            tgt,
                                         const uint32_t            reg,
                                         uint32_t                 *reg_val)
{
    CHK_RC(vsc_phy_csr_ring_read(cntrl, tgt, reg, reg_val));
    return (VSC_PHY_OK);
}


/*- VIPER & ELISE FAMILY ONLY -- VSC8584 & VSC8514 */
int32_t vsc_phy_sd6g_csr_reg_read_debug(vsc_phy_control_t       *cntrl,
                                        const uint32_t           csr_reg,
                                        uint32_t                *csr_reg_val)
{
    uint32_t   mcb_addr = 0;
    uint16_t   reg_val = 0;
    uint16_t   csr_tgt = 7;
    uint16_t   port_no = 0;
    uint8_t    mac_if;
    uint8_t    mcb_slave_num = 0;

    if (csr_reg > 0x3f) {
        return (VSC_PHY_INVALID_REQ);
    }

    switch (cntrl->phy_id.family) {
    case VSC_PHY_FAMILY_VIPER:
    case VSC_PHY_FAMILY_ELISE:
        break;

    default:
        return (VSC_PHY_OK);
    }

    DPRINTK(5, "vsc_phy_sd6g_csr_reg_read_debug: Phy_Addr: 0x%x\n", cntrl->phy_addr);
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    port_no = vsc_phy_chip_port_get(cntrl);
    DPRINTK(4, "Chip Port No: %d\n", port_no);

    switch (port_no) {
    case 0:
        mcb_slave_num = 0;
        mcb_addr = 0x3f;
        break;
    case 1:
        mcb_slave_num = 1;
        mcb_addr = 0x20;
        DPRINTK(4, "vsc_phy_sd6g_csr_reg_read_debug: <<< WARNING >>> NOT BASE PORT OF PHY!!  Port No: %d\n", port_no);
        break;
    case 2:
        mcb_slave_num = 3;
        mcb_addr = 0x20;
        DPRINTK(4, "vsc_phy_sd6g_csr_reg_read_debug: <<< WARNING >>> NOT BASE PORT OF PHY!!  Port No: %d\n", port_no);
        break;
    case 3:
        mcb_slave_num = 5;
        mcb_addr = 0x20;
        DPRINTK(4, "vsc_phy_sd6g_csr_reg_read_debug: <<< WARNING >>> NOT BASE PORT OF PHY!!  Port No: %d\n", port_no);
        break;
    default:
        return (VSC_PHY_INVALID_REQ);
        break;
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x13, &reg_val));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    mac_if = (reg_val >> 14) & 0x3;

    if (mac_if ==  0x1) {  // QSGMII
        DPRINTK(4, "vsc_phy_sd6g_csr_reg_read_debug: QSGMII mac_if: %x\n", mac_if);
    } else if (mac_if ==  0x0) { // SGMII
        DPRINTK(4, "vsc_phy_sd6g_csr_reg_read_debug: SGMII mac_if: %x\n", mac_if);
    } else {
        DPRINTK(4, "vsc_phy_sd6g_csr_reg_read_debug: Not QSGMII or SGMII; mac_if: %x\n", mac_if);
        // If we get to here, 6G SerDes settings only applies to QSGMII and SGMII, So MAC i/f configured to some other mode
        return (VSC_PHY_OK);
    }

    // Base Port of PHY Only
    CHK_RC(vsc_phy_mcb_read_trigger(cntrl, mcb_addr, mcb_slave_num)); // read 1G or 6G MCB into CSRs
    // Base Port of PHY Only
    CHK_RC(vsc_phy_sd6g_csr_reg_read(cntrl, csr_tgt, csr_reg, csr_reg_val));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    return (VSC_PHY_OK);
}

//=============================================================================
// 1G Macro setup
//=============================================================================
static int32_t vsc_phy_sd1g_des_cfg_write(vsc_phy_control_t       *cntrl,
                                          const uint8_t            des_phs_ctrl,
                                          const uint8_t            des_mbtr_ctrl)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x12;
    uint32_t           reg_val = 0;

    // read old val
    CHK_RC(vsc_phy_csr_ring_read(cntrl, tgt, csr_reg_addr, &reg_val));  // des_cfg
    // mask off old des_phs_ctrl (16:13) and des_mbtr_ctrl (10:8) values
    reg_val &= ~(0xf << 13); // des_phs_ctrl (16:13)
    reg_val &= ~(0x7 << 8);  // des_mbtr_ctrl (10:8)
    // OR in new values for des_phs_ctrl (16:13) and des_mbtr_ctrl (10:8)
    reg_val = reg_val | ((uint32_t)(des_phs_ctrl) << 13) | ((uint32_t)(des_mbtr_ctrl) << 8);
    // write back with updated values
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));  // des_cfg
}

static int32_t vsc_phy_sd1g_ib_cfg_write(vsc_phy_control_t       *cntrl,
                                         const uint8_t            ib_ena_cmv_term)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x13;
    uint32_t           reg_val = 0;

    // read old val
    CHK_RC(vsc_phy_csr_ring_read(cntrl, tgt, csr_reg_addr, &reg_val));  // ib_cfg
    // mask off old ib_ena_cmv_term (13) value
    reg_val &= ~(1 << 13); // ib_ena_cmv_term (13)
    // OR in new value for ib_ena_cmv_term (13)
    reg_val = reg_val | ((uint32_t)(ib_ena_cmv_term) << 13);
    // write back with updated values
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));  // ib_cfg
}

static int32_t vsc_phy_sd1g_misc_cfg_write(vsc_phy_control_t       *cntrl,  
                                           const uint8_t            des_100fx_cpmd_mode)
{
    const uint16_t     tgt = 0x7;
    const uint32_t     csr_reg_addr = 0x1e;
    uint32_t           reg_val = 0;

    // read old val
    CHK_RC(vsc_phy_csr_ring_read(cntrl, tgt, csr_reg_addr, &reg_val));  // ib_cfg
    // mask off old des_100fx_cpmd_mode (9) value
    reg_val &= ~(1 << 9); // des_100fx_cpmd_mode (9)
    // OR in new value for des_100fx_cpmd_mode (9)
    reg_val = reg_val | ((uint32_t)(des_100fx_cpmd_mode) << 9);
    // write back with updated values
    return (vsc_phy_csr_ring_write(cntrl, tgt, csr_reg_addr, reg_val));  // ib_cfg
}

// This function gets called from Port 0, ie. cntrl->portAddr == BasePorti
// The tgt_port_no is the port_no that the operation is to occur upon.
int32_t vsc_phy_sd1g_patch(vsc_phy_control_t       *cntrl,
                           const uint16_t           tgt_port_no, 
                           const uint8_t            is_100fx)
{
    uint16_t         port_no = 0;
    uint8_t          mcb_slave_addr = 0;
    uint8_t          mcb_addr = 0x20; // MCB for 1G SerDes

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    port_no = vsc_phy_chip_port_get(cntrl);
    DPRINTK(5, "vsc_phy_sd1g_patch: Phy_Addr: 0x%x, <<< Chip Port_no: %x  >>>> \n", cntrl->phy_addr, port_no);
    
    if (port_no != 0) {
        DPRINTK(4, "vsc_phy_sd1g_patch: Exitting - <<< WARNING >>> - NOT BASE PORT OF PHY!! Returning to caller!  Port No: %x\n", port_no);
        return (VSC_PHY_OK);
    }

    switch (tgt_port_no) {
    case 0:
        mcb_slave_addr = 0;
        break;
    case 1:
        mcb_slave_addr = 2;
        break;
    case 2:
        mcb_slave_addr = 4;
        break;
    case 3:
        mcb_slave_addr = 6;
        break;
    default:
        DPRINTK(4, "vsc_phy_sd1g_patch: <<< WARNING >>> NOT BASE PORT OF PHY!!  Port No: %d\n", port_no);
        return (VSC_PHY_INVALID_REQ);
    }

    DPRINTK(4, "Setting 1G");

    CHK_RC(vsc_phy_mcb_read_trigger(cntrl, mcb_addr, mcb_slave_addr));    // read 1G or 6G MCB into CSRs

    if (is_100fx) {
        CHK_RC(vsc_phy_sd1g_ib_cfg_write(cntrl, 0));         // ib_cfg
        CHK_RC(vsc_phy_sd1g_misc_cfg_write(cntrl, 1));       // misc_cfg
        CHK_RC(vsc_phy_sd1g_des_cfg_write(cntrl, 14, 3));    // des_cfg
    } else {
        CHK_RC(vsc_phy_sd1g_ib_cfg_write(cntrl, 1));         // ib_cfg
        CHK_RC(vsc_phy_sd1g_misc_cfg_write(cntrl, 0));       // misc_cfg
        CHK_RC(vsc_phy_sd1g_des_cfg_write(cntrl, 6, 2));     // des_cfg
    }

    CHK_RC(vsc_phy_mcb_write_trigger(cntrl, mcb_addr, mcb_slave_addr));    // read 1G or 6G MCB into CSRs

    DPRINTK(4,"vtss_phy_sd1g_patch: Port: %d  is_100fx: %x;  sd1g patch Complete", port_no, is_100fx);

    return VSC_PHY_OK;
}

//=============================================================================
// End of 1G Macro
//=============================================================================


//=============================================================================
int32_t vsc_phy_sd6g_patch(vsc_phy_control_t    *cntrl)
{
    uint32_t         rd_dat = 0;
    uint16_t         reg_val = 0;
    uint16_t         port_no = 0;

    const uint8_t    ib_sig_det_clk_sel_mm = 7;
    const uint8_t    ib_tsdet_cal = 16;
    const uint8_t    ib_tsdet_mm  = 5;
    uint8_t          timeout = 200;
    uint8_t          mac_if = 0;
    uint8_t          iter = 0;
    uint8_t          viper_rev_a = 0;
    uint8_t          ib_sig_det_clk_sel_cal = 0;
    uint8_t          rcomp = 0;
    uint8_t          ib_rtrm_adj = 0;
    uint8_t          pll_fsm_ctrl_data = 0;
    uint8_t          qrate = 0;
    uint8_t          if_mode = 0;
    uint8_t          des_bw_ana_val = 0;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    port_no = vsc_phy_chip_port_get(cntrl);
    DPRINTK(5, "vsc_phy_sd6g_patch: Phy_Addr: 0x%x, <<< Chip Port_no: %x  >>>> \n", cntrl->phy_addr, port_no);

    if (port_no != 0) {
        DPRINTK(4, "vsc_phy_sd6g_patch: Exitting - WARNING - NOT BASE PORT OF PHY!!  Port No: %x\n", port_no);
        return (VSC_PHY_OK);
    }

    switch (cntrl->phy_id.family) {
    case VSC_PHY_FAMILY_VIPER:
        viper_rev_a = (cntrl->phy_id.revision == VSC_PHY_VIPER_REV_A);
    case VSC_PHY_FAMILY_ELISE:
        break;

    default:
        return (VSC_PHY_OK);
    }

    ib_sig_det_clk_sel_cal = viper_rev_a ? 0 : 7; // 7 for Elise and Viper Rev. B+

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x13, &reg_val));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    mac_if = (reg_val >> 14) & 0x3;
    DPRINTK(5, "vsc_phy_sd6g_patch: mac_if: %x\n", mac_if);

    if (mac_if ==  0x1) {  // QSGMII
        pll_fsm_ctrl_data = 120;
        qrate   = 0;
        if_mode = 3;
        des_bw_ana_val = 5;
    } else if (mac_if ==  0x0) { // SGMII
        pll_fsm_ctrl_data = 60;
        qrate   = 1;
        if_mode = 1;
        des_bw_ana_val = 3;
    } else {
        // If we get to here, 6G SerDes settings only applies to QSGMII and SGMII, So MAC i/f configured to some other mode
        DPRINTK(4, "Exitting: vsc_phy_sd6g_patch: mac_if: %x\n", mac_if);
        return (VSC_PHY_OK);
    }

    DPRINTK(5, "Setting 6G \n");

    // Base Port of PHY Only
    CHK_RC(vsc_phy_mcb_read_trigger(cntrl, 0x3f, 0));
    CHK_RC(vsc_phy_mcb_read_trigger(cntrl, 0x11, 0));
    CHK_RC(vsc_phy_pll5g_cfg0_write(cntrl, 0x4));
    CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x11, 0));

    if (viper_rev_a) {
        // modify RComp value for Viper Rev. A
        CHK_RC(vsc_phy_csr_ring_read(cntrl, 0x7, 0x0f, &rd_dat)); // rcomp_status
        rcomp = rd_dat & 0xf; //~10;
        ib_rtrm_adj = rcomp - 3;
    } else {
        // use trim offset for Viper Rev. B+ and Elise
        ib_rtrm_adj = 16 - 3;
    }

    DPRINTK(5, "Initializing 6G...\n");
    // update des_bw_ana for bug 14948
    CHK_RC(vsc_phy_sd6g_des_cfg_write(cntrl, 0x6, 0x2, 0x5, des_bw_ana_val));
    CHK_RC(vsc_phy_sd6g_inbuf_cfg0_write(cntrl, ib_rtrm_adj, ib_sig_det_clk_sel_cal, 0x0, 0x0));

    if (viper_rev_a) {
        CHK_RC(vsc_phy_sd6g_inbuf_cfg1_write(cntrl, 0x8, ib_tsdet_cal, 0x0, 0x1));
    } else {
        CHK_RC(vsc_phy_sd6g_inbuf_cfg1_write(cntrl, 0x8, ib_tsdet_cal, 0xf, 0x0));
    }
    // update ib_tcalv & ib_ureg for bug 14626
    CHK_RC(vsc_phy_sd6g_inbuf_cfg2_write(cntrl, 0x3, 0xd, 0x5));
    CHK_RC(vsc_phy_sd6g_inbuf_cfg3_write(cntrl, 0x0, 0x1f, 0x1, 0x1f));
    CHK_RC(vsc_phy_sd6g_inbuf_cfg4_write(cntrl, 0x3f, 0x3f, 0x2, 0x3f));

    CHK_RC(vsc_phy_sd6g_pll_cfg_write(cntrl, 0x3, pll_fsm_ctrl_data, 0x0));
    CHK_RC(vsc_phy_sd6g_common_cfg_write(cntrl, 0x0, 0x0, 0x0, qrate, if_mode));
    CHK_RC(vsc_phy_sd6g_misc_cfg_write(cntrl, 0x1));
    CHK_RC(vsc_phy_sd6g_gp_cfg_write(cntrl, 0x300));
    CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x3f, 0));

    DPRINTK(5, "Calibrating PLL...\n");
    CHK_RC(vsc_phy_sd6g_pll_cfg_write(cntrl, 0x3, pll_fsm_ctrl_data, 0x1));
    CHK_RC(vsc_phy_sd6g_common_cfg_write(cntrl, 0x1, 0x1, 0x0, qrate, if_mode));
    CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x3f, 0));

    // wait for PLL cal to complete
    timeout = 200;
    do {
        CHK_RC(vsc_phy_mcb_read_trigger(cntrl, 0x3f, 0));
        CHK_RC(vsc_phy_csr_ring_read(cntrl, 0x7, 0x31, &rd_dat)); // pll_status
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x0001000); // wait for bit 12 to clear

    DPRINTK(5, "Calibrating IB...\n");
    // only for Viper Rev. A
    if (viper_rev_a) {
        // one time with SW clock
        CHK_RC(vsc_phy_sd6g_inbuf_cfg0_write(cntrl, ib_rtrm_adj, ib_sig_det_clk_sel_cal, 0x0, 0x1));
        CHK_RC(vsc_phy_sd6g_misc_cfg_write(cntrl, 0x0));
        CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x3f, 0));
        // 11 cycles w/ SW clock
        for (iter = 0; iter < 11; iter++) {
            CHK_RC(vsc_phy_sd6g_gp_cfg_write(cntrl, 0x301));  // set gp(0)
            CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x3f, 0));
            CHK_RC(vsc_phy_sd6g_gp_cfg_write(cntrl, 0x300));  // clear gp(0)
            CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x3f, 0));
        }
    }
    // auto. cal
    if (viper_rev_a) {
        CHK_RC(vsc_phy_sd6g_inbuf_cfg0_write(cntrl, ib_rtrm_adj, ib_sig_det_clk_sel_cal, 0x0, 0x0));
        CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x3f, 0));
        CHK_RC(vsc_phy_sd6g_inbuf_cfg1_write(cntrl, 0x8, ib_tsdet_cal, 0x0, 0x0));
        CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x3f, 0));
        CHK_RC(vsc_phy_sd6g_inbuf_cfg1_write(cntrl, 0x8, ib_tsdet_cal, 0xf, 0x0));
        CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x3f, 0));
    }

    CHK_RC(vsc_phy_sd6g_inbuf_cfg0_write(cntrl, ib_rtrm_adj, ib_sig_det_clk_sel_cal, 0x0, 0x1));
    CHK_RC(vsc_phy_sd6g_misc_cfg_write(cntrl, 0x0));
    CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x3f, 0));
    // wait for IB cal to complete
    timeout = 200;
    do {
        CHK_RC(vsc_phy_mcb_read_trigger(cntrl, 0x3f, 0));
        CHK_RC(vsc_phy_csr_ring_read(cntrl, 0x7, 0x2f, &rd_dat)); // pll_status
        SD6G_TIMEOUT(timeout);
    } while (~rd_dat & 0x0000100); // wait for bit 8 to set

    DPRINTK(5, "Final settings... \n");
    // update final ib_reg_pat_sel_offset for bug 14626
    CHK_RC(vsc_phy_sd6g_inbuf_cfg0_write(cntrl, ib_rtrm_adj, ib_sig_det_clk_sel_mm, 0x0, 0x1));
    CHK_RC(vsc_phy_sd6g_inbuf_cfg1_write(cntrl, 0x8, ib_tsdet_mm, 0xf, 0x0));
    CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x3f, 0));
    DPRINTK(5, "vsc_phy_sd6g_patch: Phy_Addr: 0x%x  sd6g patch Complete \n", cntrl->phy_addr);

    return (VSC_PHY_OK);
}

//=============================================================================
/*- VIPER, ELISE FAMILY ONLY -- VSC8584 & VSC8514 */
// ob_post0; /*< 6G SerDes Macro, 6 bits, default:0, Coeff for 1st Post Cursor, Customizeable. */
// ob_post1; /*< 6G SerDes Macro, 5 bits, default:0, Coeff for 2nd Post Cursor, Do not change. */
//=============================================================================
int32_t vsc_phy_sd6g_ob_post(vsc_phy_control_t    *cntrl, uint8_t  ob_post0, uint8_t  ob_post1)
{
    uint16_t         reg_val = 0;
    uint16_t         port_no = 0;
    const uint8_t    ob_ena1v_mode = 0x1; /*< Output buffer supply voltage.  1:Set to Nominal 1V, 0:Set to higher Voltage */
    const uint8_t    ob_pol = 0x1; /*< Polarity of Output Signal.  0:Normal, 1:Inverted */
    const uint8_t    ob_resistor_ctr = 0x1; /*< 6G SerDes Macro, 4 bits, default: 0x1, Resistor Control.  Value must be taken from RCOMP_Status.Rcomp */
    uint8_t          ob_sr = 0;    /*< 6G SerDes Macro, 4 bits, default: 0, Driver speed fine adjustment of slew rate 30-60ps if ob_sr_h=0, 60-140ps if ob_sr_h=1 */
    uint8_t          ob_sr_h = 0;  /*< 6G SerDes Macro, 1 bit, default: 0, Half the predriver speed, used for slew rate control */
    uint8_t          mac_if = 0;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    port_no = vsc_phy_chip_port_get(cntrl);
    DPRINTK(5, "vsc_phy_sd6g_patch: Phy_Addr: 0x%x, <<< Chip Port_no: %x  >>>> \n", cntrl->phy_addr, port_no);

    if (port_no != 0) {
        DPRINTK(4, "vsc_phy_sd6g_patch: Exitting - WARNING - NOT BASE PORT OF PHY!!  Port No: %x\n", port_no);
        return (VSC_PHY_OK);
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x13, &reg_val));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    mac_if = (reg_val >> 14) & 0x3;
    DPRINTK(5, "vsc_phy_sd6g_patch: mac_if: %x\n", mac_if);

    if (mac_if ==  0x1) {  // QSGMII
        ob_sr_h = 0;
        ob_sr = 0;
    } else if (mac_if ==  0x0) { // SGMII
        ob_sr_h = 1;
        ob_sr = 7;
    } else {
        // If we get to here, 6G SerDes settings only applies to QSGMII and SGMII, So MAC i/f configured to some other mode
        DPRINTK(4, "Exitting: vsc_phy_sd6g_ob_post: mac_if: %x\n", mac_if);
        return (VSC_PHY_OK);
    }

    // Base Port of PHY Only
    CHK_RC(vsc_phy_mcb_read_trigger(cntrl, 0x3f, 0));
    CHK_RC(vsc_phy_sd6g_outbuf_cfg_write(cntrl, ob_ena1v_mode, ob_pol, ob_post0, ob_post1, ob_sr_h, ob_resistor_ctr, ob_sr));
    CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x3f, 0));
    DPRINTK(5, "vsc_phy_sd6g_ob_post: Phy_Addr: 0x%x  Complete \n", cntrl->phy_addr);
    //printf("vsc_phy_sd6g_ob_post: Phy_Addr: 0x%x;  ob_post0: 0x%x,  ob_post1: 0x%x  Complete \n", cntrl->phy_addr, ob_post0, ob_post1);

    return (VSC_PHY_OK);
}

//=============================================================================
/*- VIPER, ELISE FAMILY ONLY -- VSC8584 & VSC8514 */
//=============================================================================
int32_t vsc_phy_sd6g_ob_level(vsc_phy_control_t     *cntrl, uint8_t  ob_level)
{
    uint16_t     reg_val = 0;
    uint16_t     port_no = 0;
    uint8_t      ob_ena_cas = 0;  /*< 6G SerDes Macro, 3 bits, default: 0, Output Skew, Use for skew Adjustment in SGMII mode */
    uint8_t      mac_if = 0;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    port_no = vsc_phy_chip_port_get(cntrl);
    DPRINTK(5, "vsc_phy_sd6g_patch: Phy_Addr: 0x%x, <<< Chip Port_no: %x  >>>> \n", cntrl->phy_addr, port_no);

    if (port_no != 0) {
        DPRINTK(4, "vsc_phy_sd6g_patch: Exitting - WARNING - NOT BASE PORT OF PHY!!  Port No: %x\n", port_no);
        return (VSC_PHY_OK);
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x13, &reg_val));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    mac_if = (reg_val >> 14) & 0x3;
    DPRINTK(5, "vsc_phy_sd6g_patch: mac_if: %x\n", mac_if);

    if (mac_if ==  0x1) {  // QSGMII
        ob_ena_cas = 0;
    } else if (mac_if ==  0x0) { // SGMII
        ob_ena_cas = 2;
    } else {
        // If we get to here, 6G SerDes settings only applies to QSGMII and SGMII, So MAC i/f configured to some other mode
        DPRINTK(4, "Exitting: vsc_phy_sd6g_ob_level: mac_if: %x\n", mac_if);
        return (VSC_PHY_OK);
    }

    // Base Port of PHY Only
    CHK_RC(vsc_phy_mcb_read_trigger(cntrl, 0x3f, 0));
    CHK_RC(vsc_phy_sd6g_outbuf_cfg1_write(cntrl, ob_ena_cas, ob_level));
    CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x3f, 0));
    DPRINTK(3, "vsc_phy_sd6g_ob_level: Phy_Addr: 0x%x  Setting ob_level: 0x%x  Complete \n", cntrl->phy_addr, ob_level);
    //printf("vsc_phy_sd6g_ob_level: Phy_Addr: 0x%x  Setting ob_level: 0x%x  Complete \n", cntrl->phy_addr, ob_level);

    return (VSC_PHY_OK);
}

//=============================================================================

uint8_t  vsc_phy_chk_serdes_init_mac_mode(vsc_phy_control_t   *cntrl)
{
    uint32_t         rd_dat = 0;
    uint16_t         reg_val = 0;
    uint16_t         port_no = 0;

    //uint8_t          viper_rev_a;
    uint8_t          mac_if = 0;
    uint8_t          if_mode = 0;
    uint8_t          qrate = 0;
    uint8_t          hrate = 0;
    uint8_t          ena_lane = 0;
    uint8_t          sys_rst = 0;
    uint8_t          pll_fsm_ena = 0;
    const vsc_phy_mac_interface_t    conf_mac_if = cntrl->mac_if;
    vsc_phy_mac_interface_t          micro_patch_mac_mode = PHY_MAC_IF_MODE_NO_CONNECTION;


    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    port_no = vsc_phy_chip_port_get(cntrl);
    DPRINTK(5, "vsc_phy_chk_serdes_init_mac_mode: Phy_Addr: 0x%x, <<< Chip Port_no: %d  >>>> \n", cntrl->phy_addr, port_no);

    if (port_no != 0) {
        DPRINTK(4, "vsc_phy_chk_serdes_init_mac_mode: <<< WARNING >>> NOT BASE PORT OF PHY!!  Port No: %d\n", port_no);
        return (VSC_PHY_FALSE);
    }

    switch (cntrl->phy_id.family) {
    case VSC_PHY_FAMILY_VIPER:
        //viper_rev_a = (cntrl->phy_id.revision == VSC_PHY_VIPER_REV_A);
    case VSC_PHY_FAMILY_ELISE:
        break;

    default:
        return (VSC_PHY_FALSE);
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x13, &reg_val));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    mac_if = (reg_val >> 14) & 0x3;
    if (mac_if ==  0x1) {  // QSGMII
        if (conf_mac_if != PHY_MAC_IF_MODE_QSGMII) {
            DPRINTK(4, "vsc_phy_chk_serdes_init_mac_mode_private: Phy_Addr: 0x%x, QSGMII MAC i/f Mismatch conf_mac_if: 0x%x  runtime_mac_if: 0x%x\n", 
                    cntrl->phy_addr, conf_mac_if, mac_if);
            return (VSC_PHY_FALSE);
        }
    } else if (mac_if ==  0x0) { // SGMII
        if (conf_mac_if != PHY_MAC_IF_MODE_SGMII) {
            DPRINTK(4, "vsc_phy_chk_serdes_init_mac_mode_private: Phy_Addr: 0x%x, SGMII MAC i/f Mismatch conf_mac_if: 0x%x  runtime_mac_if: 0x%x\n", 
                    cntrl->phy_addr, conf_mac_if, mac_if);
            return (VSC_PHY_FALSE);
        }
    } else {
            DPRINTK(4, "vsc_phy_chk_serdes_init_mac_mode_private: Phy_Addr: 0x%x, MAC i/f Mismatch\n", 
                    cntrl->phy_addr);
        // If we get to here, 6G SerDes settings only applies to QSGMII and SGMII, So MAC i/f configured to some other mode
        return (VSC_PHY_FALSE);
    }

    CHK_RC(vsc_phy_mcb_read_trigger(cntrl, 0x3f, 0));
    CHK_RC(vsc_phy_csr_ring_read(cntrl, 0x7, 0x2b, &rd_dat)); // MACRO_CTRL::SERDES1G_PLL_CFG
    pll_fsm_ena = (uint8_t) ((rd_dat & 0x00000080) >> 7);

    CHK_RC(vsc_phy_mcb_read_trigger(cntrl, 0x3f, 0));
    CHK_RC(vsc_phy_csr_ring_read(cntrl, 0x7, 0x2c, &rd_dat)); // MACRO_CTRL::SERDES1G_CMN_CFG
    sys_rst = (uint8_t) ((rd_dat & 0x80000000) >> 31);
    ena_lane = (uint8_t) ((rd_dat & 0x00040000) >> 18);
    hrate = (uint8_t) ((rd_dat & 0x00000080) >> 7);
    qrate = (uint8_t) ((rd_dat & 0x00000040) >> 6);
    if_mode = (uint8_t) ((rd_dat & 0x00000030) >> 4);
        // Check to see if it's enabled
    if ((sys_rst == 0x1) && (ena_lane == 0x1) && (pll_fsm_ena == 0x1)) {
        if ((if_mode == 0x3) && (qrate == 0) && (hrate == 0)) {
            micro_patch_mac_mode = PHY_MAC_IF_MODE_QSGMII; 
        } else if ((if_mode == 0x1) && (qrate == 1) && (hrate == 0)) {
            micro_patch_mac_mode = PHY_MAC_IF_MODE_SGMII;
        } else {
            DPRINTK(4, "1: sys_rst: 0x%x; ena_lane: 0x%x, pll_fsm_ena: 0x%x;  if_mode: 0x%x, qrate: 0x%x, hrate: 0x%x \n", sys_rst, ena_lane, pll_fsm_ena, if_mode, qrate, hrate );
        }
    } else {
        DPRINTK(4, "2: sys_rst: 0x%x; ena_lane: 0x%x, pll_fsm_ena: 0x%x;  if_mode: 0x%x, qrate: 0x%x, hrate: 0x%x \n", sys_rst, ena_lane, pll_fsm_ena, if_mode, qrate, hrate );
    }

    if ((micro_patch_mac_mode != PHY_MAC_IF_MODE_NO_CONNECTION) &&
        (conf_mac_if == micro_patch_mac_mode)) {
        if (micro_patch_mac_mode == PHY_MAC_IF_MODE_QSGMII) {
            DPRINTK(4, "API Config (%d) IN-SYNC with MAC SerDes HW (%d) == QSGMII \n", conf_mac_if, micro_patch_mac_mode);
        } else if (micro_patch_mac_mode == PHY_MAC_IF_MODE_SGMII) {
            DPRINTK(4, "API Config (%d) IN-SYNC with MAC SerDes HW (%d) == SGMII \n", conf_mac_if, micro_patch_mac_mode);
        }

        return (VSC_PHY_TRUE);
    } else {
        DPRINTK(4, "API Config (%d) NOT-IN-SYNC with MAC SerDes HW (%d); QSGMII != SGMII \n", conf_mac_if, micro_patch_mac_mode);
    }

    DPRINTK(4, "Returning....API Config (%d) NOT-IN-SYNC with MAC SerDes HW (%d); QSGMII != SGMII \n", conf_mac_if, micro_patch_mac_mode);
    return (VSC_PHY_FALSE);
}


uint8_t  vsc_phy_chk_serdes_patch_init(vsc_phy_control_t  *cntrl)
{
    uint16_t         port_no = 0;
    //uint8_t          viper_rev_a = 0;
    uint8_t          ib_sig_det_clk_sel = 0;
    uint8_t          ib_reg_pat_sel_offset = 0;
    uint8_t          ib_cal_ena = 0;
    uint8_t          ib_rtrm_adj = 0;
    uint8_t          des_phy_ctrl = 0;
    uint8_t          des_mbtr_ctrl = 0;
    uint8_t          des_bw_hyst = 0;
    uint8_t          des_bw_ana = 0;
    uint8_t          ib_tinfv = 0;
    uint8_t          ib_tcalv = 0;
    uint8_t          ib_ureg = 0;
    const vsc_phy_mac_interface_t    conf_mac_if = cntrl->mac_if;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    port_no = vsc_phy_chip_port_get(cntrl);
    DPRINTK(5, "vsc_phy_chk_serdes_patch_init: Phy_Addr: 0x%x, <<< Chip Port_no: %d  >>>> \n", cntrl->phy_addr, port_no);

    if (port_no != 0) {
        DPRINTK(4, "vsc_phy_chk_serdes_patch_init: <<< WARNING >>> NOT BASE PORT OF PHY!!  Port No: %d\n", port_no);
        return (VSC_PHY_FALSE);
    }

    switch (cntrl->phy_id.family) {
    case VSC_PHY_FAMILY_VIPER:
        //viper_rev_a = (cntrl->phy_id.revision == VSC_PHY_VIPER_REV_A);
    case VSC_PHY_FAMILY_ELISE:
        break;

    default:
        return (VSC_PHY_FALSE);
    }

    CHK_RC(vsc_phy_mcb_read_trigger(cntrl, 0x3f, 0));
    CHK_RC(vsc_phy_sd6g_inbuf_cfg0_read(cntrl, &ib_rtrm_adj, &ib_sig_det_clk_sel, &ib_reg_pat_sel_offset, &ib_cal_ena));
    CHK_RC(vsc_phy_sd6g_inbuf_cfg2_read(cntrl, &ib_tinfv, &ib_tcalv, &ib_ureg));
    CHK_RC(vsc_phy_sd6g_des_cfg_read(cntrl, &des_phy_ctrl, &des_mbtr_ctrl, &des_bw_hyst, &des_bw_ana));

    DPRINTK(5, "SerDes micro-patch for port: %d, ib_reg_pat_sel_offset: %d, ib_tcalv: %d, ib_ureg: %d, des_bw_ana: %d \n",
               port_no, ib_reg_pat_sel_offset, ib_tcalv, ib_ureg, des_bw_ana);

    if ((ib_reg_pat_sel_offset == 0) &&   // HW default = 1
            (ib_tcalv == 13) &&               // HW default = 12
            (ib_ureg  == 5) &&                // HW default = 4
            (((conf_mac_if == PHY_MAC_IF_MODE_SGMII) && (des_bw_ana == 3)) ||    // SGMII
             ((conf_mac_if == PHY_MAC_IF_MODE_QSGMII) && (des_bw_ana == 5)))) {   // QSGMII
            DPRINTK(4, "SerDes micro-patch Enabled,  Port: %d \n", port_no);
            return (VSC_PHY_TRUE);
    }

    DPRINTK(4, "SerDes micro-patch NOT Enabled: %d \n", port_no);
    return (VSC_PHY_FALSE);
}

//=============================================================================

int32_t vsc_get_phy_type(vsc_phy_control_t   *cntrl)
{
    uint16_t      reg2_val = 0;
    uint16_t      reg3_val = 0;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

#ifdef CONFIG_PHY_DEBUG_ENABLE_PRINTF
    uint16_t     port_no = vsc_phy_chip_port_get(cntrl);
#endif
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x2, &reg2_val));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x3, &reg3_val));

    cntrl->phy_id.oui = ((reg2_val << 6) | ((reg3_val >> 10) & 0x3F));
    cntrl->phy_id.model = ((reg3_val >> 4) & 0x3F);
    cntrl->phy_id.revision = (reg3_val & 0xF);
    DPRINTK(5, "Port: %d,  Reg2: 0x%04x Reg3: 0x%04x \n", port_no, reg2_val, reg3_val);
    DPRINTK(5, "OUI: 0x%04x MODEL: 0x%04x Rev: 0x%x\n", cntrl->phy_id.oui, cntrl->phy_id.model, cntrl->phy_id.revision);

    if (cntrl->phy_id.oui == 0x0001C1) {
        switch (cntrl->phy_id.model) {
        case 0x0a: // VSC8574 Tesla
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8574;
            cntrl->phy_id.family = VSC_PHY_FAMILY_TESLA;
            cntrl->phy_id.port_cnt = 4;
            break;

        case 0x0C : // VSC8504 Tesla
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8504;
            cntrl->phy_id.family = VSC_PHY_FAMILY_TESLA;
            cntrl->phy_id.port_cnt = 4;
            break;

        case 0x0D : // VSC8572 Tesla
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8572;
            cntrl->phy_id.family = VSC_PHY_FAMILY_TESLA;
            cntrl->phy_id.port_cnt = 2;
            break;

        case 0x0E : // VSC8552 Tesla
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8552;
            cntrl->phy_id.family = VSC_PHY_FAMILY_TESLA;
            cntrl->phy_id.port_cnt = 2;
            break;

            //
            // Nano family
            //
        case 0x13 : // VSC8501 Nano 
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8501;
            cntrl->phy_id.family = VSC_PHY_FAMILY_NANO;
            cntrl->phy_id.port_cnt = 1;
            break;

        case 0x23 : // VSC8502 Nano
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8502;
            cntrl->phy_id.family = VSC_PHY_FAMILY_NANO;
            cntrl->phy_id.port_cnt = 2;
            break;

            //
            // ELISE family - Cost optimized VIPER (Without SGMII, FIBER media, 1588, MacSec and Legacy MAC EEE supprt)
            //
        case 0x27 : // VSC8514 Elise
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8514;
            cntrl->phy_id.family = VSC_PHY_FAMILY_ELISE;
            cntrl->phy_id.port_cnt = 4;
            break;

            //
            // Mini family
            //
       case 0x16 : // VSC8530 Mini
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8530;
            cntrl->phy_id.family = VSC_PHY_FAMILY_MINI;
            cntrl->phy_id.port_cnt = 1;
            break;

       case 0x17 : // VSC8531 Mini
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8531;
            cntrl->phy_id.family = VSC_PHY_FAMILY_MINI;
            cntrl->phy_id.port_cnt = 1;
            break;

        case 0x36 : // VSC8540 Mini 
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8540;
            cntrl->phy_id.family = VSC_PHY_FAMILY_MINI;
            cntrl->phy_id.port_cnt = 1;
            break;

        case 0x37 : // VSC8541 Mini 
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8541;
            cntrl->phy_id.family = VSC_PHY_FAMILY_MINI;
            cntrl->phy_id.port_cnt = 1;
            break;

            //
            // Viper family
            //
        case 0x3b: // Dual PHY - VSC8582 Viper - Fully Featured
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8582;
            cntrl->phy_id.family = VSC_PHY_FAMILY_VIPER;
            cntrl->phy_id.port_cnt = 2;
            break;

        case 0x3c: // Quad PHY - VSC8584 Viper - Fully Featured
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8584;
            cntrl->phy_id.family = VSC_PHY_FAMILY_VIPER;
            cntrl->phy_id.port_cnt = 4;
            break;

        case 0x3d: // Quad PHY - VSC8575 Viper - w/1588 but No MACSEC Capability 
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8575;
            cntrl->phy_id.family = VSC_PHY_FAMILY_VIPER;
            cntrl->phy_id.port_cnt = 4;
            break;

        case 0x3e: // Quad PHY - VSC8564 Viper - w/MACSEC but No 1588 Capability 
            cntrl->phy_id.part_number = VSC_PHY_TYPE_8564;
            cntrl->phy_id.family = VSC_PHY_FAMILY_VIPER;
            cntrl->phy_id.port_cnt = 4;
            break;

        default:
            cntrl->phy_id.part_number = VSC_PHY_TYPE_NONE;
            cntrl->phy_id.family = VSC_PHY_FAMILY_NONE;
            cntrl->phy_id.port_cnt = 0;
            break;
        }
    } else {
        cntrl->phy_id.part_number = VSC_PHY_TYPE_NONE;
        cntrl->phy_id.family = VSC_PHY_FAMILY_NONE;
        cntrl->phy_id.port_cnt = 0;
    }

    return(VSC_PHY_OK);
}

int32_t vsc_get_phy_media_if_config(const vsc_phy_media_interface_t media_if, uint16_t *cmd_100fx, uint8_t *media_op, uint8_t *cu_pref)
{
    // Setup media interface
    switch (media_if) {
    case PHY_MEDIA_IF_CU:
        *media_op = 0;
        *cu_pref = 1;
        break;
    case PHY_MEDIA_IF_SFP_PASSTHRU:
        *media_op = 1;
        *cu_pref = 1;
        break;
    case PHY_MEDIA_IF_FI_1000BX:
        *media_op = 2;
        *cu_pref = 0;
        break;
    case PHY_MEDIA_IF_FI_100FX:
        *media_op = 3;
        *cu_pref = 0;
        *cmd_100fx = 1 << 4;
        break;
    case PHY_MEDIA_IF_AMS_CU_PASSTHRU:
        *media_op = 5;
        *cu_pref = 1;
        break;
    case PHY_MEDIA_IF_AMS_FI_PASSTHRU:
        *media_op = 5;
        *cu_pref = 0;
        break;
    case PHY_MEDIA_IF_AMS_CU_1000BX:
        *media_op = 6;
        *cu_pref = 1;
        break;
    case PHY_MEDIA_IF_AMS_FI_1000BX:
        *media_op = 6;
        *cu_pref = 0;
        break;
    case PHY_MEDIA_IF_AMS_CU_100FX:
        *media_op = 7;
        *cu_pref = 1;
        *cmd_100fx = 1 << 4;
        break;
    case PHY_MEDIA_IF_AMS_FI_100FX:
        *media_op = 7;
        *cu_pref = 0;
        *cmd_100fx = 1 << 4;
        break;
    default:
        return (VSC_PHY_ERROR);
    }

    return (VSC_PHY_OK);
}

// Sequence to Reset LCPLL for the PHY
// Note: This would occur PRIOR to calling PHY pre-reset(), reset(), and post-reset() functions
// The Calling application must know the PHY Base Port number.
int32_t vsc_reset_phy_lcpll(vsc_phy_control_t    *cntrl)
{
    int32_t          rc = VSC_PHY_OK;
    uint16_t         reg_val = 0;
    uint16_t         port_no = 0;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    port_no = vsc_phy_chip_port_get(cntrl);
    DPRINTK(5, "vsc_reset_phy_lcpll: Phy_Addr: 0x%x, <<< Chip Port_no: %d  >>>> \n", cntrl->phy_addr, port_no);

    if (port_no != 0) {
        DPRINTK(4, "vsc_reset_phy_lcpll: <<< WARNING >>> NOT BASE PORT OF PHY!!  Port No: %d\n", port_no);
        return (VSC_PHY_ERROR);
    }

    if (cntrl->phy_id.part_number == VSC_PHY_TYPE_NONE) {
        CHK_RC(vsc_get_phy_type(cntrl));
    }

    switch (cntrl->phy_id.family) {
    case VSC_PHY_FAMILY_TESLA:
        rc = VSC_PHY_OK;
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));

        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x8023, 0xffff));
        CHK_RC(vsc_phy_wait_for_micro(cntrl));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0xd7d5, 0xffff));
        CHK_RC(vsc_phy_wait_for_micro(cntrl));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x8d06, 0xffff));
        CHK_RC(vsc_phy_wait_for_micro(cntrl));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x80c0, 0xffff));
        CHK_RC(vsc_phy_wait_for_micro(cntrl));

        cntrl->phy_usleep((10 * 1000)); // 10msec sleep

        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x8506, 0xffff));
        CHK_RC(vsc_phy_wait_for_micro(cntrl));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x80c0, 0xffff));
        CHK_RC(vsc_phy_wait_for_micro(cntrl));

        cntrl->phy_usleep((10 * 1000)); // 10msec sleep

        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_EXT3_PAGE, 0xffff));
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT3_PAGE, 0x14, &reg_val));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        DPRINTK(3, "vsc_reset_phy_lcpll: Tesla PHY - LCPLL Reset Complete \n");
        break;

    case VSC_PHY_FAMILY_VIPER:
    case VSC_PHY_FAMILY_ELISE:
        rc = VSC_PHY_OK;
        //uint8_t          viper_rev_a = 0;
        //viper_rev_a = (cntrl->phy_id.revision == VSC_PHY_VIPER_REV_A);

        // Note: This will be called PRIOR to Phy_Pre_Reset
        // Read LCPLL MCB into tYhe CSR's
        CHK_RC(vsc_phy_mcb_read_trigger(cntrl, 0x11, 0));
        // Reset LCPLL
        CHK_RC(vsc_phy_pll5g_cfg2_write(cntrl, 0x1, 0x0));   
        // WRITE back the LCPLL MCB into the CSR's
        CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x11, 0));
        // 10msec sleep while LCPLL is held in Reset
        cntrl->phy_usleep((10 * 1000));
        // Read LCPLL MCB into tYhe CSR's
        CHK_RC(vsc_phy_mcb_read_trigger(cntrl, 0x11, 0));
        // Release the LCPLL Reset
        CHK_RC(vsc_phy_pll5g_cfg2_write(cntrl, 0x0, 0x0));   
        // WRITE back the LCPLL MCB into the CSR's
        CHK_RC(vsc_phy_mcb_write_trigger(cntrl, 0x11, 0));

        DPRINTK(3, "vsc_reset_phy_lcpll: Viper or Elise PHY - LCPLL Reset Complete \n");
        break;

    default:
        rc = VSC_PHY_ERROR;
        break;
    }

    cntrl->phy_usleep((110 * 1000)); // 110msec sleep to allow re-calibration of LCPLL

    return (rc);
}

/* Get Clause_37 Link Partner Ability and Status - See Reg 19E3 */
/* Function for getting phy lp ability when in pass through mode. */
/* per Application Note : Protocol transfer mode guide - This is used for CuSFP */
int32_t vsc_phy_cl37_lp_ability_get(vsc_phy_control_t          *cntrl,
                                    vsc_phy_cl37_status_t      *status)
{
    uint16_t         reg=0;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
#ifdef CONFIG_PHY_DEBUG_ENABLE_PRINTF
    uint16_t     port_no = vsc_phy_chip_port_get(cntrl);
#endif
    DPRINTK(5, "vsc_phy_cl37_status_get: 0x%x, Chip Port_no: %x  \n", cntrl->phy_addr, port_no);

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT3_PAGE, 0xffff));

    /* Read link partner ability/status from register 19E3 */
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT3_PAGE, 0x13, &reg));  // Read 19E3 - Clause_37 Link Partner Ability

    status->link = (reg & (1 << 15) ? 1 : 0);
    status->fdx =  (reg & (1 << 12) ? 1 : 0);
    switch ((reg >> 10) & 0x3) {
        case 0:
            status->speed = VSC_SPEED_10M;
            break;
        case 1:
            status->speed = VSC_SPEED_100M;
            break;
        case 2:
            status->speed = VSC_SPEED_1G;
            break;
        case 3:
            status->speed = VSC_SPEED_UNDEFINED;
            break;
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    return (VSC_PHY_OK);
}


int32_t vsc_phy_status_get(vsc_phy_control_t    *cntrl, vsc_phy_port_status_t * status)
{
    uint16_t    reg_val = 0;
    uint16_t    reg28_val = 0;
    uint16_t    aneg_enabled = 0;
    uint16_t    pwr_dn_enabled = 0;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    DPRINTK(5, "vsc_phy_status_get: Phy_Addr: 0x%x, Chip Port_no: %d  \n", cntrl->phy_addr, (vsc_phy_chip_port_get(cntrl)));

    /* Read link status from register 1 */
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x01, &reg_val));

    // Link up/down
    status->link                   = (reg_val & (0x01 << 2) ? 0x1 : 0x0);
    status->link_down              = (reg_val & (0x01 << 2) ? 0x0 : 0x1);
    status->remote_fault           = (reg_val & (0x01 << 4) ? 0x1 : 0x0);
    status->aneg_complete          = (reg_val & (0x01 << 5) ? 0x1 : 0x0);
    status->unidir_able            = (reg_val & (0x01 << 7) ? 0x1 : 0x0);
    status->ext_status_ena         = (reg_val & (0x01 << 8) ? 0x1 : 0x0);


    if (status->link_down) {
        /* Read status again if link down (latch low field) */
        /* Read link status again from register 1 */
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x01, &reg_val));
        status->link =      (reg_val & (0x01 << 2) ? 0x1 : 0x0);
        status->link_down = (reg_val & (0x01 << 2) ? 0x0 : 0x1);
        DPRINTK(3, "status->link = %d, status->link_down = %d, PHY port = 0x%x, reg_val = 0x%x \n", status->link, status->link_down, cntrl->phy_addr, reg_val);
    } else {
        status->link = 0x1;
    }

    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x00, &reg_val));
    aneg_enabled = (reg_val & (0x01 << 12) ? 0x1 : 0x0);
    pwr_dn_enabled = (reg_val & (0x01 << 11) ? 0x1 : 0x0);
//    forced_speed_sel = ((reg_val & (0x1 << 6) ? 0x1 : 0x0) << 1) | ((reg_val & (0x01 << 12) ? 0x1 : 0x0) << 0);

    DPRINTK(4, "PHY port = %d, link: %d, reg0_val = 0x%x, forced_speed_sel: 0x%x, aneg_enabled: 0x%0x, pwr_dn_enabled: 0x%x \n", 
            vsc_phy_chip_port_get(cntrl), status->link, reg_val, 
            (((reg_val & (0x1 << 6) ? 0x1 : 0x0) << 1) | ((reg_val & (0x01 << 12) ? 0x1 : 0x0) << 0)), 
            ((reg_val & (0x01 << 12) ? 0x1 : 0x0)), 
            ((reg_val & (0x01 << 11) ? 0x1 : 0x0)));

    /* Status = Link is UP! */
    if (status->link) {
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x1C, &reg28_val));

        if (pwr_dn_enabled) {        /* VSC_PHY_MODE_POWER_DOWN Mode */
            /* do Nothing */
            DPRINTK (5, "phy_addr:0x%x, POWER-DN!  \n", cntrl->phy_addr);
        } else if (aneg_enabled) {   /* VSC_PHY_MODE_ANEG Mode */
            DPRINTK (5, "phy_addr:0x%x, ANEG Enabled!  \n", cntrl->phy_addr);

            if ((status->aneg_complete) == 0) {   /* ANEG NOT Complete ? */
                status->mdi_cross = ((reg28_val & (0x01 << 13)) ? VSC_PHY_TRUE : VSC_PHY_FALSE);
                if ((reg28_val & 0x1b) != 0xa) {
                    /* Auto negotiation not complete, link considered down */
                    status->link = 0;
                } else {
                    status->speed = VSC_SPEED_100M;
                    status->fdx = 1;
                }
            }

            /* Link Status could have changed if ANEG not complete */
            if (status->link) {
                uint8_t   sym_pause;
                uint8_t   asym_pause;
                uint8_t   lp_sym_pause;
                uint8_t   lp_asym_pause;

                /* Use register 4 to determine the local ANEG Advertisements */
                CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x04, &reg_val));
                asym_pause = (reg_val & (0x01 << 11) ? 0x1 : 0x0);
                sym_pause =  (reg_val & (0x01 << 10) ? 0x1 : 0x0);
                /* Use register 5 to determine the link partner ANEG Advertisements */
                CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x05, &reg_val));
                lp_asym_pause = (reg_val & (1 << 11) ? 0x1 : 0x0);
                lp_sym_pause =  (reg_val & (1 << 10) ? 0x1 : 0x0);

                /* Using info from Reg 04 and 05, determine flow control result */
                status->aneg_obey_pause = (sym_pause && (lp_sym_pause || (asym_pause && lp_asym_pause)) ? 1 : 0);
                status->aneg_generate_pause = (lp_sym_pause && (sym_pause || (asym_pause && lp_asym_pause)) ? 1 : 0);
                DPRINTK (5, "PHY Addr:0x%x, status->aneg_generate_pause:%d, status->aneg_generate_pause:%d, lp_asym_pause:%d, lp_sym_pause:%d, sym_pause:%d, asym_pause:%d \n",
                    cntrl->phy_addr, status->aneg_generate_pause, status->aneg_generate_pause, lp_asym_pause, lp_sym_pause, sym_pause, asym_pause);

                if (cntrl->phy_id.family != VSC_PHY_FAMILY_NONE) {
                    /* Vitesse PHY, use register 28 to determine speed/duplex */
                    switch ((reg_val >> 3) & 0x3) {
                    case 0:
                        status->speed = VSC_SPEED_10M;
                        break;
                    case 1:
                        status->speed = VSC_SPEED_100M;
                        break;
                    case 2:
                        status->speed = VSC_SPEED_1G;
                        break;
                    case 3:
                        status->speed = VSC_SPEED_UNDEFINED;
                        break;
                    }
                    /* Vitesse PHY, use register 28 to determine speed/duplex/mdix */
                    status->fdx = (reg28_val & (0x1 << 5) ? 0x1 : 0x0);
                    status->mdi_cross = ((reg28_val & (0x1 << 13)) ? 0x1 : 0x0);

                    DPRINTK (5, "PHY_Addr:0x%x, ANEG Mode: LINK-UP! link: %d, speed:%d, fdx:%d, mdi_cross:%d, pwr_dn:%d, aneg:%d \n", 
                        cntrl->phy_addr, status->link, status->speed, status->fdx, status->mdi_cross, pwr_dn_enabled, aneg_enabled);

                    /* If we are in SFP_PASSTHRU Mode, then we need to get the status from the CuSFP to get the actual Link Status */
                    if (cntrl->media_if == PHY_MEDIA_IF_SFP_PASSTHRU) {
                        CHK_RC(vsc_phy_cl37_lp_ability_get(cntrl, &status->cl37_status));
                    } else {
                        status->cl37_status.speed = VSC_SPEED_UNDEFINED;
                        status->cl37_status.fdx = 0;
                        status->cl37_status.link = 0;
                    }
                }
            } else {
                /* Link Down Mode */
                DPRINTK (6, "Phy_Addr:0x%x, LINK-DN! ANEG Enabled, ANEG NOT Complete! \n", cntrl->phy_addr);
            }
        } else {
            DPRINTK (5, "PHY_Addr:0x%x, FORCED Mode!  \n", cntrl->phy_addr);
            /* VSC_PHY_MODE_FORCED: Not Power-Down and ANEG Not Enabled, Therefore in Forced Mode */
            if (cntrl->phy_id.family != VSC_PHY_FAMILY_NONE) {

                switch ((reg28_val >> 3) & 0x3) {
                case 0:
                    status->speed = VSC_SPEED_10M;
                    break;
                case 1:
                    status->speed = VSC_SPEED_100M;
                    break;
                case 2:
                    status->speed = VSC_SPEED_1G;
                    break;
                case 3:
                    status->speed = VSC_SPEED_UNDEFINED;
                    break;
                }
                /* Vitesse PHY, use register 28 to determine speed/duplex/mdix */
                status->fdx = (reg28_val & (0x1 << 5) ? 0x1 : 0x0);
                status->mdi_cross = ((reg28_val & (0x1 << 13)) ? 0x1 : 0x0);
            }

            DPRINTK (5, "PHY_Addr:0x%x, FORCED Mode: LINK-UP! link: %d, speed:%d, fdx:%d, mdi_cross:%d, pwr_dn:%d, aneg:%d \n", 
              cntrl->phy_addr, status->link, status->speed, status->fdx, status->mdi_cross, pwr_dn_enabled, aneg_enabled);
        }
    }

    /* Handle link down event */
    if ((!status->link || status->link_down)) {
        if (status->link_down) {
            /* Handle link up event */
            if (status->link) {
                DPRINTK(4, "Link UP Event on phy_addr 0x%x, status->link = %d, status->link_down = %d \n", cntrl->phy_addr, status->link, status->link_down);
                if (status->speed == VSC_SPEED_1G) {
                   CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_TR_PAGE, 0xffff));
                   CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TR_PAGE, 0x10, 0xaff0, 0xffff));
                   CHK_RC(vsc_phy_read(cntrl, VSC_PHY_TR_PAGE, 0x11, &reg_val));
                   CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TR_PAGE, 0x1f, VSC_PHY_TEST_PAGE, 0xffff));
                   CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TEST_PAGE, 0x0C, 0x0000, 0x0300));
                   CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TEST_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
                   DPRINTK(4, "VSC_PHY_LINK_UP_FULL_PWR \n");
                }
            } else {
                DPRINTK(4, "LINK DOWN Event, Link is Now DOWN on phy_addr 0x%x, status->link = %d, status->link_down = %d \n", cntrl->phy_addr, status->link, status->link_down);
            }
        } else {
            DPRINTK(4, "LINK DOWN on phy_addr 0x%x, status->link = %d, status->link_down = %d \n", cntrl->phy_addr, status->link, status->link_down);
        }

        // Determine if it is a fiber or CU port
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT1_PAGE, 0x14, &reg_val));
        status->fiber = (((reg_val >> 6) & 0x3) == 2 ? 0x1 : 0x0);
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        DPRINTK (4, "phy_addr:0x%x, link: %d, speed:%d, fdx:%d, mdi_cross:%d, aneg:%d, fiber_port:%d \n", 
              cntrl->phy_addr, status->link, status->speed, status->fdx, status->mdi_cross, aneg_enabled, status->fiber);
    }

    return VSC_PHY_OK;
}


// Function for configuring MDI for a given port
static int32_t vsc_phy_mdi_conf(vsc_phy_control_t    *cntrl, vsc_phy_conf_t * conf)
{
    vsc_phy_port_status_t   phy_status;
    uint16_t                mask = 0;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    DPRINTK(5, "vsc_phy_mdi_conf: Phy_Addr: 0x%x, Chip Port_no: %d  \n", cntrl->phy_addr, vsc_phy_chip_port_get(cntrl));

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    switch (conf->mdi) {
    case VSC_PHY_AUTO_MDIX:
        DPRINTK(4, "VSC_PHY_AUTO_MDIX  \n");
        // Enable  HP AUto-MDIX, and en pair swap correction.
        // Cu media forced normal HP-AUTO-MDIX operation, See datasheet
        mask = (0x1 << 7) | (0x1 << 5);
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x12, 0x0000, mask));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
        mask = 0x3 << 2;
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x13, 0x0000, mask));
        break;

    case VSC_PHY_MDIX:
        DPRINTK(4, "VSC_PHY_MDIX \n");
        CHK_RC(vsc_phy_status_get(cntrl, &phy_status));

        mask = (0x1 << 7);
        if (phy_status.speed == VSC_SPEED_1G) {
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x12, 0x0000, mask));
            DPRINTK(5,"speed:%d", phy_status.speed);
        } else {
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x12, mask, mask));
        }

        mask = (0x1 << 5);
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x12, 0x0000, mask));

        // Cu media forced MDI-X, See datasheet
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
        mask = 0x3 << 2;
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x13, 0x000C, mask));
        break;

    case VSC_PHY_MDI:
        DPRINTK(4, "VSC_PHY_MDI\n");
        CHK_RC(vsc_phy_status_get(cntrl, &phy_status));

        mask = (0x1 << 7);
        if (phy_status.speed == VSC_SPEED_1G) {
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x12, 0x0000, mask));
            DPRINTK(5,"speed:%d", phy_status.speed);
        } else {
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x12, mask, mask));
        }

        mask = (0x1 << 5);
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x12, 0x0000, mask));

        // Cu media forced MDI, See datasheet
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
        mask = 0x3 << 2;
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x13, 0x0008, mask));
        break;

    default:
        DPRINTK(5, "Unknown mdi mode:%d \n", conf->mdi);

    }
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    return VSC_PHY_OK;
}

//Function for suspending / resuming the 8051 patch.
//
// In : port_no - Any port within the chip where to supend 8051 patch
//      suspend - True if 8051 patch shall be suspended, else patch is resumed.
// Return : VTSS_RC_OK if patch was suspended else error code.
static int32_t vsc_phy_micro_patch_suspend(vsc_phy_control_t    *cntrl, uint8_t  suspend)
{
    uint16_t     reg_val = 0;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    DPRINTK(5, "vsc_phy_micro_patch_suspend: PHY Addr: 0x%x, PHY Chip Port_no: %d  \n", cntrl->phy_addr, vsc_phy_chip_port_get(cntrl));

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));

    if (suspend) {
        // Suspending 8051 patch
        // See comment below.
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x12, &reg_val));
        if (!(reg_val & 0x4000)) {
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x800f, 0xffff));
        }
        CHK_RC(vsc_phy_wait_for_micro(cntrl));
    } else {
        // Resuming 8051 patch
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x12, &reg_val));
        if (reg_val & 0x4000) {
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x8009, 0xffff));
        }
        CHK_RC(vsc_phy_wait_for_micro(cntrl));
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff)); // Change to standard page

    return VSC_PHY_OK;
}

/* Set Clause_37 Ability - See Reg 18E3 */
// Function for setting phy in pass through mode per Application Note : Protocol transfer mode guide
static int32_t vsc_phy_pass_through_speed_mode(vsc_phy_control_t    *cntrl, vsc_phy_conf_t * conf)
{
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    DPRINTK(5, "vsc_phy_pass_through_speed_mode: PHY Addr: 0x%x, PHY Chip Port_no: %d  \n", cntrl->phy_addr, vsc_phy_chip_port_get(cntrl));

    switch (cntrl->phy_id.family) {
    case VSC_PHY_FAMILY_TESLA:
    case VSC_PHY_FAMILY_VIPER:
        DPRINTK(4, "Phy_Addr:0x%x: Pass through mode setting mode:%d, speed:%d \n", cntrl->phy_addr, conf->mode, conf->forced.port_speed);

        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        //Protocol Transfer mode Guide : Section 4.1.1 - Aneg must be enabled
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x00, (0x1 << 12), (0x1 << 12)));

        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT3_PAGE, 0xffff));
        // Set 16E3.7, MAC SerDes ANEG Enable and Default clear 16E3: "force advertise ability" bit as well
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x10, (0x1 << 7), ((0x1 << 11) | (0x1 << 7))));

        // Protocol Transfer mode Guide : Section 4.1.3
        if (conf->mode == VSC_PHY_MODE_FORCED) {
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x10, (0x1 << 11), (0x1 << 11)));

            switch (conf->forced.port_speed) {
            case VSC_SPEED_100M:
                /* 18E3 */
                CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x12, 0x8401, 0xffff));
                break;

            case VSC_SPEED_10M:
                CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x12, 0x8001, 0xffff));
                break;

            case VSC_SPEED_1G:
                CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x12, 0x8801, 0xffff));
                break;

            default:
                DPRINTK(4, "Unexpected port speed:%d defaulting to 1G \n", conf->forced.port_speed);
                CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x12, 0x8801, 0xffff));
                break;
            }
        }
        break;
    default:
        DPRINTK(6, "Phy_Addr:0x%x: All other PHYs don't need this or are not supporting SFP pass through mode. \n", cntrl->phy_addr);
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    return VSC_PHY_OK;
}


static int32_t vsc_phy_ena_mdix_in_forced(vsc_phy_control_t   *cntrl, vsc_phy_mode_t  mode)
{
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    // Suspend Micro-Patch
    CHK_RC(vsc_phy_micro_patch_suspend( cntrl, 0x1));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_TEST_PAGE, 0xffff)); // Change to TEST page
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TEST_PAGE, 0x8, 0x8000, 0x8000)); //Ensure RClk125 enabled even in powerdown
    // Clear Cl40AutoCrossover in forced-speed mode, but set it in non-forced modes
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TEST_PAGE, 0x1f, VSC_PHY_TR_PAGE, 0xffff)); // Change to TR page
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TR_PAGE, 0x10, 0xa7fa, 0xffff)); // issue token-ring read request
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TR_PAGE, 0x11, ((mode == VSC_PHY_MODE_FORCED) ? 0x0000 : 0x1000), 0x1000)); // issue token-ring read request
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TR_PAGE, 0x10, 0x87fa, 0xffff)); // issue token-ring write request
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TR_PAGE, 0x10, 0xaf82, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TR_PAGE, 0x11, 0x0002, 0x000f));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TR_PAGE, 0x10, 0x8f82, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TR_PAGE, 0x1f, VSC_PHY_TEST_PAGE, 0xffff)); // Change to TEST page
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TEST_PAGE, 0x8, 0x0000, 0x8000)); //Restore RClk125 gating
    // Resume Micro-Patch
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_TEST_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    CHK_RC(vsc_phy_micro_patch_suspend( cntrl, 0x0));
    // Enable HP Auto-MDIX in forced-mode (by clearing disable bit)
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x12, 0x0000, (0x1 << 7)));

    return VSC_PHY_OK;
}

/* Setting Master/Slave */
/* This Applies to 1G Forced Mode, Manual selection of Master/Slave */
int32_t vsc_phy_conf_1g_set(vsc_phy_control_t *cntrl, vsc_phy_conf_1g_t   *conf_1g)
{
    uint16_t              reg_val = 0;
    uint16_t              reg_cntrl = 0;
    uint16_t              reg_status = 0;

    /* Save setup */
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    DPRINTK(5, "vsc_phy_conf_1g_set: Phy_Addr: 0x%x, <<< Chip Port_no: %d  >>>> \n", cntrl->phy_addr, vsc_phy_chip_port_get(cntrl));

    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x09, &reg_cntrl));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x0A, &reg_status));

    reg_val  = ((conf_1g->master.cfg ? 1 : 0) << 12) | ((conf_1g->master.val ? 1 : 0) << 11);

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x09, reg_val, 0x1800));

    /* Re-start Auto-Neg if Master/Slave Manual Config/Value changed */
    if ((reg_cntrl ^ reg_val) & 0x1800) {
        if ((reg_val & 0x1000) || (reg_status & 0x8000)) { /* Only start Auto-neg if Manuel is selected or master/slave resolution failed */
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x00, 0x0200, 0x0200));
        }
    }

    return VSC_PHY_OK;
}

int32_t  vsc_phy_packet_mode_set(vsc_phy_control_t *cntrl, vsc_phy_pkt_mode_t   pkt_mode)
{
    uint16_t    reg_mask = 0;
    uint16_t    reg_val = 0;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    DPRINTK(5, "vsc_phy_packet_mode_set: Phy_Addr: 0x%x, <<< Chip Port_no: %d  >>>> \n", cntrl->phy_addr, vsc_phy_chip_port_get(cntrl));

    switch (pkt_mode) {
    case VSC_PHY_PKT_MODE_IEEE_1_5_KB:
    case VSC_PHY_PKT_MODE_JUMBO_9_KB:
    case VSC_PHY_PKT_MODE_JUMBO_12_KB:
        reg_mask = 0x3 << 4;
        reg_val = pkt_mode << 4;
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x18, reg_val, reg_mask));
        break;
    default:
        break;
    }

    return VSC_PHY_OK;
}


int32_t vsc_phy_conf_set(vsc_phy_control_t * cntrl, vsc_phy_conf_t * conf)
{
    uint16_t              port_no;
    uint16_t              cur_reg_val;
    uint16_t              cur_status_val;
    uint16_t              new_reg_val;
    uint16_t              prev_mdi_val;
    uint16_t              reg_bit_mask = 0;
    uint8_t               restart_aneg = VSC_PHY_FALSE;

    /* Save setup */
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    DPRINTK(5, "vsc_phy_conf_set: Phy_Addr: 0x%x, <<< Chip Port_no: %d  >>>> \n", cntrl->phy_addr, vsc_phy_chip_port_get(cntrl));

    if (conf->mode != VSC_PHY_MODE_ANEG) {
        // Setting AUTO NEG advertisement to 0 in order to make sure that ANEG is restarted when we returns to aneg mode.
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x04, 0x0, 0xffff));
    }

    // Read the current MDI Settings before changing any config
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT1_PAGE, 0x13, &prev_mdi_val));
    prev_mdi_val &= 0x000C;

    // Ensure that we are on the Std Page Registers before we start programming
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    // If in forced mdi mode and ANEG, we have to make sure to set MDI mode before ANEG restart
    CHK_RC(vsc_phy_mdi_conf(cntrl, conf));

    switch (conf->mode) {
    case VSC_PHY_MODE_ANEG:
        /* Setup register 4 */
        new_reg_val = (((conf->aneg.tx_remote_fault ? 1 : 0) << 13) |
                         ((conf->aneg.asymmetric_pause ? 1 : 0) << 11) |
                         ((conf->aneg.symmetric_pause ? 1 : 0) << 10) |
                         ((conf->aneg.speed_100m_fdx ? 1 : 0) << 8) |
                         ((conf->aneg.speed_100m_hdx ? 1 : 0) << 7) |
                         ((conf->aneg.speed_10m_fdx ? 1 : 0) << 6) |
                         ((conf->aneg.speed_10m_hdx ? 1 : 0) << 5) |
                         (1 << 0));

        if (cntrl->media_if == PHY_MEDIA_IF_SFP_PASSTHRU) {
            reg_bit_mask = 0xac1f; // In Pass though mode the advertisement is done in the cu SFP.
        } else {
            reg_bit_mask = 0xbfff;// Bit 14 is reserved.
        }

        /* Read Current ANEG Advertisements */
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x04, &cur_reg_val));
        if (cur_reg_val != new_reg_val) {
            restart_aneg = VSC_PHY_TRUE;
        }

        /* Normally in MDIX_AUTO. */
        /* However, If in Forced MDI or MDIX mode, In order to assure that we are in the correct mode, restart ANEG */
        /* In some situations in 1G mode, the chip tries to maintain the link-up regardless of forced MDI settings */
        switch (conf->mdi) {
        case VSC_PHY_AUTO_MDIX:
            if (prev_mdi_val != 0x0) {
                restart_aneg = VSC_PHY_TRUE;
            }
            break;
        case VSC_PHY_MDIX:
            if (prev_mdi_val != 0xC) {
                restart_aneg = VSC_PHY_TRUE;
            }
        case VSC_PHY_MDI:
            if (prev_mdi_val != 0x8) {
                restart_aneg = VSC_PHY_TRUE;
            }
            break;
        default:
            break;
        }

        DPRINTK( 4, "VSC_PHY_DEVICE_AUTONEG_ADVERTISEMENT, port:%d,  mdi: 0x%x, Curr RegVal: 0x%x, New RegVal: 0x%x,  Mask: 0x%x \n",
                   vsc_phy_chip_port_get(cntrl), conf->mdi, cur_reg_val, new_reg_val, reg_bit_mask);

        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x04, new_reg_val, reg_bit_mask));

        /* Setup register 9 for 1G advertisement */
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x09, &cur_reg_val));
        new_reg_val = 0;
        if (conf->aneg.speed_1g_fdx) {
            new_reg_val |= 0x1 << 9;
        }
        /* For 1000BaseT-HDX, although allowed per 802.3, is not supported in industry */
        /* We are unaware of anyone making a Gigabit Ethernet hub, so testing 1000BT hdx with any vendor’s L2 device may be problematic. */
        if (conf->aneg.speed_1g_hdx) {
            new_reg_val |= 0x1 << 8;
        }

        reg_bit_mask = (0x1 << 9) | (0x1 << 8);  // FDX and HDX
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x09, new_reg_val, reg_bit_mask));

        if ((cur_reg_val & ((0x1 << 9) | (0x1 << 8))) != new_reg_val) {
            restart_aneg = VSC_PHY_TRUE;
        }

        CHK_RC(vsc_phy_ena_mdix_in_forced(cntrl, conf->mode));

        /* Now configure for Master/Slave Manual Config */
        /* Note, there is a Seperate function used in Forced Mode, that is because we don't want to restart ANEG multiple times */
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x09, &cur_reg_val));
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x0A, &cur_status_val));
        reg_bit_mask = 0x1800;   // bits Reg9.12:11
        new_reg_val  = ((conf->conf_1g.master.cfg ? 1 : 0) << 12) | ((conf->conf_1g.master.val ? 1 : 0) << 11);
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x09, new_reg_val, reg_bit_mask));
        //printf("vsc_phy_conf_set: Phy_Addr: 0x%x, Manual_Master_Slave:%x, Master:%x \n", 
        //      cntrl->phy_addr, conf->conf_1g.master.cfg, conf->conf_1g.master.val);
        DPRINTK(5, "vsc_phy_conf_set: Phy_Addr: 0x%x, Manual_Master_Slave:%x, Master:%x \n", 
              cntrl->phy_addr, conf->conf_1g.master.cfg, conf->conf_1g.master.val);

        /* Re-start Auto-Neg if Master/Slave Manual Config/Value changed */
        if ((cur_reg_val ^ new_reg_val) & 0x1800) {
            if ((new_reg_val & 0x1000) || (cur_status_val & 0x8000)) { /* Only start Auto-neg if Manuel is selected or master/slave resolution failed */
                restart_aneg = VSC_PHY_TRUE;
            }
        }

        /* If required, Use register 0 to restart auto negotiation */
        /* If Changing Restarting ANEG, Make sure PHY is not in Power Dn State. */
        if (restart_aneg) {
            new_reg_val = (0x1 << 12) | (0x1 << 9);
            reg_bit_mask = (0x1 << 12) | (0x1 << 11) | (0x1 << 9);
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x00, new_reg_val, reg_bit_mask));

            DPRINTK(4, "VSC_PHY Change Detected: Restart ANEG, PHY port:%d, RegVal: 0x%x, Mask: 0x%x  \n", vsc_phy_chip_port_get(cntrl), new_reg_val, reg_bit_mask);

        } else {
            new_reg_val = (0x1 << 12);
            reg_bit_mask = (0x1 << 12) | (0x1 << 11);
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x00, new_reg_val, reg_bit_mask));
        }
        DPRINTK(4, "VSC_PHY ANEG, PHY port:%d, RegVal: 0x%x, Mask: 0x%x \n", vsc_phy_chip_port_get(cntrl), new_reg_val, reg_bit_mask);
        break;

    case VSC_PHY_MODE_FORCED:
        /* Setup register 0 */
        reg_bit_mask = 0xffff;
        new_reg_val = (((conf->forced.port_speed == VSC_SPEED_100M ? 1 : 0) << 13) | (0 << 12) |
                       ((conf->forced.fdx ? 1 : 0) << 8) |
                       ((conf->forced.port_speed == VSC_SPEED_1G ? 1 : 0) << 6) |
                       ((conf->unidir == VSC_PHY_UNIDIR_ENABLE ? 1 : 0) << 5));

        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x00, new_reg_val, reg_bit_mask));

        DPRINTK(4, "VSC_PHY_MODE_FORCED, port:%d, mode_control regVal:0x%X  \n", vsc_phy_chip_port_get(cntrl), new_reg_val);

        if (conf->forced.port_speed != VSC_SPEED_1G) {
            /* Enable Auto MDI/MDI-X in forced 10/100 mode */
            CHK_RC(vsc_phy_ena_mdix_in_forced(cntrl, conf->mode));
        }

        if (conf->forced.port_speed == VSC_SPEED_1G) {
            DPRINTK(4, "VSC_PHY_MODE_FORCED, port:%d, Setting Master/Slave Config  \n", vsc_phy_chip_port_get(cntrl));
            CHK_RC(vsc_phy_conf_1g_set(cntrl, &conf->conf_1g));
        }
        break;

    case VSC_PHY_MODE_POWER_DOWN:
        /* Setup register 0 */
        new_reg_val = 0x1 << 11;
        reg_bit_mask = 0x1 << 11;
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x00, new_reg_val, reg_bit_mask));
        break;
    default:
        DPRINTK( 5, "port_no %u, unknown mode %d  \n", vsc_phy_chip_port_get(cntrl), conf->mode);
        return VSC_PHY_ERROR;
    }

    // If in Not in ANEG mode, need to set mdi after vsc_phy_ena_mdix_in_forced() because it gets cleared
    CHK_RC(vsc_phy_mdi_conf(cntrl, conf));

    if (cntrl->media_if == PHY_MEDIA_IF_SFP_PASSTHRU) {
        CHK_RC(vsc_phy_pass_through_speed_mode(cntrl, conf));
    } else {
        switch (cntrl->phy_id.family) {
        case VSC_PHY_FAMILY_TESLA:
        case VSC_PHY_FAMILY_VIPER:
        case VSC_PHY_FAMILY_ELISE:
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT3_PAGE, 0xffff));
            // Setup Reg16E3
            new_reg_val = 0;
            new_reg_val = (((conf->mac_if_pcs.disable ? 1 : 0) << 15) |
                                 ((conf->mac_if_pcs.restart ? 1 : 0) << 14) |
                                 ((conf->mac_if_pcs.pd_enable ? 1 : 0) << 13) |
                                 ((conf->mac_if_pcs.aneg_restart ? 1 : 0) << 12) |
                                 ((conf->mac_if_pcs.force_adv_ability ? 1 : 0) << 11) |
                                 (conf->mac_if_pcs.sgmii_in_pre << 9) |
                                 ((conf->mac_if_pcs.sgmii_out_pre ? 1 : 0) << 8) |
                                 ((conf->mac_if_pcs.serdes_aneg_ena ? 1 : 0) << 7) |
                                 ((conf->mac_if_pcs.serdes_pol_inv_in ? 1 : 0) << 6) |
                                 ((conf->mac_if_pcs.serdes_pol_inv_out ? 1 : 0) << 5) |
                                 ((conf->mac_if_pcs.fast_link_stat_ena ? 1 : 0) << 4) |
                                 ((conf->mac_if_pcs.inhibit_odd_start ? 1 : 0) << 2));

            // Recommended by chip designers - Setting VTSS_F_MAC_SERDES_PCS_CONTROL_MAC_IF_PD_ENA,
            // This should allow link-up if the MAC is not doing auto-neg.
            // Enable "MAC interface autonegotiation parallel detect",
            //    else data flow is stopped for the CU ports if PHY has MAC ANEG enabled and the switch is connected to isn't
			/* HH:  SF2 is SGMII ANEG platform so skip this */
            /* new_reg_val |= 0x1 << 13; */
            /* If the bits are ON by Default, leave them ON - We don't want to break default behavior */
            /* If clearing bit is desired, Clear the bit in the Register either before or after this Write */
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x10, 0xffff, new_reg_val));

            // Setup Reg23E3
            new_reg_val = 0;
            new_reg_val = ((conf->media_if_pcs.remote_fault << 14) |
                                 ((conf->media_if_pcs.aneg_pd_detect ? 1 : 0) << 13) |
                                 ((conf->media_if_pcs.force_adv_ability ? 1 : 0) << 11) |
                                 ((conf->media_if_pcs.serdes_pol_inv_in ? 1 : 0) << 6) |
                                 ((conf->media_if_pcs.serdes_pol_inv_out ? 1 : 0) << 5) |
                                 ((conf->media_if_pcs.inhibit_odd_start ? 1 : 0) << 4) |
                                 ((conf->media_if_pcs.force_hls ? 1 : 0) << 2) |
                                 ((conf->media_if_pcs.force_fefi ? 1 : 0) << 1) |
                                 ((conf->media_if_pcs.force_fefi_value ? 1 : 0)));

            /* If the bits are ON by Default, leave them ON - We don't want to break default behavior */
            /* If clearing bit is desired, Clear the bit in the Register either before or after this Write */
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x17, 0xffff, new_reg_val));
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
            break;

        default :
            break;
        }
    }
    /* Set Sigdet pin polarity active high/low. Reg19E1.0 */
    switch (conf->sigdet) {
    case VSC_PHY_SIGDET_POLARITY_ACT_LOW:
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x13, (0x1 << 0), (0x1 << 0)));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        break;
    case VSC_PHY_SIGDET_POLARITY_ACT_HIGH:
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x13, (0x0 << 0), (0x1 << 0)));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        break;
    default:
        /* Default is to keep the sigdet polarity active high */
        break;
    }

    /* Set Fast Link Fail Pin config. Reg19E1.4 */
    /* Set Fast Link Fail_2 config. Reg20E2.15 */
    switch (conf->flf.flf) {
    case VSC_PHY_FAST_LINK_FAIL_DISABLE:
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        port_no = vsc_phy_chip_port_get(cntrl);
        DPRINTK(5, "vsc_phy_conf_set: Port 0x%x, Chip Port_no: %x  Setting FLF \n", cntrl->phy_addr, port_no);

        if (port_no == 0) {
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x13, 0x0, (0x1 << 4)));   // Reg19E1.4 = 0
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

            // if FLF is disabled, then FLF_2 is disabled too.  FLF2 Cannot be Enabled when FLF is not enabled
            switch (cntrl->phy_id.family) {
            case VSC_PHY_FAMILY_MINI:
                CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT2_PAGE, 0xffff));
                CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT2_PAGE, 0x14, 0x0, (0x1 << 15)));   // Reg20E2.15 = 0
                CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT2_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
                break;

            default:
                break;
            }
        } else {
            DPRINTK(3, "vsc_phy_conf_set: VSC_PHY_FAST_LINK_FAIL_DISABLE - NOT Port 0 of PHY, Addr 0x%x, Chip Port_no: %x  \n", cntrl->phy_addr, port_no);
        }
        break;

    case VSC_PHY_FAST_LINK_FAIL_ENABLE:
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        port_no = vsc_phy_chip_port_get(cntrl);
        DPRINTK(5, "vsc_phy_conf_set: Port 0x%x, Chip Port_no: %x  Setting FLF \n", cntrl->phy_addr, port_no);

        if (port_no == 0) {
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x13, (0x1 << 4), (0x1 << 4)));   // Reg19E1.4 = 1
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

            // if FLF is enabled, then FLF_2 can be enabled.  FLF2 Cannot be Enabled when FLF is not enabled
            switch (cntrl->phy_id.family) {
            case VSC_PHY_FAMILY_MINI:
                CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT2_PAGE, 0xffff));

                if (conf->flf.flf_2 == VSC_PHY_FAST_LINK_FAIL_ENABLE) {
                    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT2_PAGE, 0x14, (0x1 << 15), (0x1 << 15)));   // Reg20E2.15 = 1
                } else if (conf->flf.flf_2 == VSC_PHY_FAST_LINK_FAIL_DISABLE) {
                    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT2_PAGE, 0x14, 0x0, (0x1 << 15)));   // Reg20E2.15 = 0
                } else {
                    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT2_PAGE, 0x14, 0x0, (0x1 << 15)));   // Reg20E2.15 = 0
                }

                CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT2_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
                break;

            default:
                break;
            }
        } else {
            DPRINTK(5, "vsc_phy_conf_set: VSC_PHY_FAST_LINK_FAIL_ENABLE - NOT Port 0 of PHY, Addr 0x%x, Chip Port_no: %x  \n", cntrl->phy_addr, port_no);
        }
    default:
        break;
    }

    /* Set Packet Mode config. Reg24.5:4 */
    CHK_RC(vsc_phy_packet_mode_set(cntrl, conf->pkt_mode));
    DPRINTK(4, "vsc_phy_conf_set: Config PHY Jumbo Pkt Mode: phy_addr: 0x%x, Pkt_Mode: 0x%x \n", cntrl->phy_addr, conf->pkt_mode);

    return VSC_PHY_OK;
}

//  Function for pulling the coma mode pin high or low 
// (The coma mode pin MUST be pulled high by an external pull up resistor)
//
// In : cntrl : Phy port control struct (Any port within the PHY chip to pull down coma mode pin).
//      pull_low : True to pull coma mode pin low (disable coma mode - ports are powered up)
// Return :  VSC_PHY_OK is setup was preformed correct else VSC_PHY_ERROR

int32_t vsc_phy_coma_mode_set(vsc_phy_control_t * cntrl, uint8_t pull_low)
{
    uint16_t              reg_bit_mask = 0;

    /* Save setup */
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    DPRINTK(5, "vsc_phy_coma_mode_set: Phy_Addr: 0x%x, Chip Port_no: %d  \n", cntrl->phy_addr, vsc_phy_chip_port_get(cntrl));

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));
    if (pull_low) {
        reg_bit_mask = (0x1 << 13) | (0x1 << 12);
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x0E, 0x0, reg_bit_mask));
    } else {
        reg_bit_mask = 0x1 << 13;
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x0E, reg_bit_mask, reg_bit_mask));
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    return VSC_PHY_OK;
}

// Function for enabling/disabling squelch work around.
//    : enable  - TRUE = enable squelch workaround, FALSE = Disable squelch workaround
// Return - VTSS_RC_OK - Workaround was enabled/disable. VTSS_RC_ERROR - Squelch workaround patch not loaded
static int32_t vsc_phy_squelch_workaround(vsc_phy_control_t     *cntrl, uint8_t enable)
{
    uint16_t    reg_val;
 
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    switch (cntrl->phy_id.family) {
    case VSC_PHY_FAMILY_TESLA:
    case VSC_PHY_FAMILY_VIPER:
        /* Squelch Work-Around doesn't apply to Rev. B Silicon of Viper */
        if (cntrl->phy_id.family == VSC_PHY_FAMILY_VIPER) {
           if (cntrl->phy_id.revision != VSC_PHY_VIPER_REV_A) {
               break;
           }
        }

        //Enable workaround by writing command to 18G
        //Command 18G instruction
        // switch to micro page (global to all ports of PHY)
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));
        if (enable) {
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x801d, 0xffff));
        } else {
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x800d, 0xffff));
        }
        //After writing to register 18G wait for bit 15 to equal 0.
        CHK_RC(vsc_phy_wait_for_micro(cntrl));

        // Still on GPIO Page after wait_for_micro.
        // Check bit register 18G bit 14, if equal 1 there was an error.
        // If there was an error then most likely the squelch patch is not loaded.
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x12, &reg_val));
        if (reg_val & (1 << 14)) {
            DPRINTK(2, "vsc_phy_squelch_workaround: Squelch-Workaround NOT_Loaded");
            CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
            return VSC_PHY_ERROR;
        }

        DPRINTK(3, "vsc_phy_squelch_workaround: Squelch-Workaround Loaded: setup reg_value = 0x%X, enable:%d", reg_val, enable);
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        break;

    default:
        break;
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    return VSC_PHY_OK;
}

int32_t vsc_phy_clock_conf_get(vsc_phy_control_t          *cntrl,
                               const vsc_phy_recov_clk_t   clock_port,
                               vsc_phy_clk_conf_t         *conf)
{
    uint16_t           reg_val = 0;
    uint16_t           port_no = 0;
    uint16_t           src = 0;
    uint16_t           freq = 0;
    uint16_t           squelch = 0;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    port_no = vsc_phy_chip_port_get(cntrl);
    DPRINTK(5, "vsc_phy_clock_conf_set: 0x%x, Chip Port_no: %x  \n", cntrl->phy_addr, port_no);

    if (port_no != 0) {
        DPRINTK(3, "vsc_phy_clock_conf_set: Exitting - WARNING - NOT BASE PORT OF PHY!!  Port No: %x\n", port_no);
        DPRINTK(2, "WARNING: RECOVERED_CLK_CONFIG_NOT_ACCESSIBLE: RECOVERED Clocks config can ONLY be accessed from Base Port of PHY!! \n");
        return (VSC_PHY_ERROR);
    }

    switch (cntrl->phy_id.family) {
    // These families  support clock recovery, but need a squelch work-around
    case VSC_PHY_FAMILY_VIPER:
    case VSC_PHY_FAMILY_TESLA:
        break;

    case VSC_PHY_FAMILY_ELISE:
        // Only Certain chip Dash Numbers support clock configuration. The chip detection is determined by register 18E2
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT2_PAGE, 0xffff));
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT2_PAGE, 0x12, &reg_val));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT2_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        reg_val &= 0x3f;
        if ((reg_val == 0x10) || (reg_val == 0x13)) {    // VSC8514-10 or -11 and VSC8514-13 or -14 do not support SyncE
            return (VSC_PHY_ERR_RECOV_CLK_CONF_NOT_SUPPORTED);
        }
        break;

    case VSC_PHY_FAMILY_NANO :
        // Only Certain chip Dash Numbers support clock configuration. The chip detection is determined by register 18E2
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT2_PAGE, 0xffff));
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT2_PAGE, 0x12, &reg_val));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT2_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        reg_val &= 0x3f;
        if (reg_val == 0x13) {    // VSC8502-13 does not support SyncE
            return (VSC_PHY_ERR_RECOV_CLK_CONF_NOT_SUPPORTED);
        }
        break;

    case VSC_PHY_FAMILY_MINI :
        // Only Certain chips support clock configuration, the 8x8 pkg.
        if ((cntrl->phy_id.part_number = VSC_PHY_TYPE_8540) ||
            (cntrl->phy_id.part_number = VSC_PHY_TYPE_8541)) {
            //printf("Recovered Clk: Supported \n");
        } else {
            return (VSC_PHY_ERR_RECOV_CLK_CONF_NOT_SUPPORTED);
        }
        break;


    default:
        // All other families don't support clock recovery
        DPRINTK(3, "clock recovery not supported on port %u, Chip family:0x%x,  Part_Number: %d \n",
                   vsc_phy_chip_port_get(cntrl), cntrl->phy_id.family, cntrl->phy_id.part_number);
        return (VSC_PHY_ERR_RECOV_CLK_CONF_NOT_SUPPORTED);
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));

    if (clock_port == VSC_PHY_RECOV_CLK1) {
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x17, &reg_val));  // Reg 23G PHY_RECOVERED_CLOCK_1_CONTROL
    } else if (clock_port == VSC_PHY_RECOV_CLK2) {
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x18, &reg_val));  // Reg 24G, PHY_RECOVERED_CLOCK_2_CONTROL
    } else {
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        return (VSC_PHY_ERR_RECOV_CLK_CONF_NOT_SUPPORTED);
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    conf->clk_src_sel = (reg_val & 7000) >> 11;

    src = reg_val & 0x0007; 

    DPRINTK(4, "Recovered Clks: Clk Selection for port %u, Chip family:0x%x,  Part_Number: %d,  Clk_Src: 0x%x \n", 
                   vsc_phy_chip_port_get(cntrl), cntrl->phy_id.family, cntrl->phy_id.part_number, src );

    switch (src) {
    case 0x0:
        conf->src = VSC_PHY_SERDES_MEDIA;
        break;
    case 0x1:
        conf->src = VSC_PHY_COPPER_MEDIA;
        break;
    case 0x2:
        conf->src = VSC_PHY_TCLK_OUT;
        break;
    case 0x3:
        conf->src = VSC_PHY_LOCAL_XTAL;
        break;

    default:
        DPRINTK(3, "Recovered Clks: Clk Selection INVALID for port %u, Chip family:0x%x,  Part_Number: %d Src:0x%x \n", 
                   vsc_phy_chip_port_get(cntrl), cntrl->phy_id.family, cntrl->phy_id.part_number, src);
        break;
    }

    // Disable over-rides the Clk Selection
    if ((reg_val & 0x8000) == 0) {
        conf->src = VSC_PHY_CLK_DISABLED;
    }

    freq = (reg_val & 0x0700) >> 8; 

    switch (freq) {
    case 0x0:
        conf->freq = VSC_PHY_FREQ_25M;
        break;
    case 0x1:
        conf->freq = VSC_PHY_FREQ_125M;
        break;
    case 0x2:
        conf->freq = VSC_PHY_FREQ_3125M;
        break;
    default:
        DPRINTK(3, "Recovered Clks freq INVALID for port %u, Chip family:0x%x,  Part_Number: %d \n", 
                   vsc_phy_chip_port_get(cntrl), cntrl->phy_id.family, cntrl->phy_id.part_number);
        break;
    }

    squelch = (reg_val & 0x0030) >> 4; 

    switch (squelch) {     
    case 0x0:
        conf->squelch = VSC_PHY_CLK_SQUELCH_MAX;
        break;
    case 0x1:
        conf->squelch = VSC_PHY_CLK_SQUELCH_MED;
        break;
    case 0x2:
        conf->squelch = VSC_PHY_CLK_SQUELCH_MIN;
        break;
    case 0x3:
        conf->squelch = VSC_PHY_CLK_SQUELCH_NONE;
        break;
    default:
        DPRINTK(3, "Recovered Clks squelch INVALID for port %u, Chip family:0x%x,  Part_Number: %d \n", 
                   vsc_phy_chip_port_get(cntrl), cntrl->phy_id.family, cntrl->phy_id.part_number);
        break;
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    return VSC_PHY_OK;
}


/* Configure 23G/24G (Recovered Clock1/Recovered Clock2) */
int32_t vsc_phy_clock_conf_set(vsc_phy_control_t          *cntrl,
                               const vsc_phy_recov_clk_t   clock_port,
                               const vsc_phy_clk_conf_t   *const conf)
{
    uint16_t              reg_val = 0;
    uint16_t              port_no = 0;
    uint16_t              save_squelch_ctrl_addr = 0;
    uint8_t               use_squelch_workaround = VSC_PHY_FALSE;
    uint8_t               other_port_save_squelch_ctrl = 0;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    port_no = vsc_phy_chip_port_get(cntrl);
    DPRINTK(5, "vsc_phy_clock_conf_set: 0x%x, Chip Port_no: %x  \n", cntrl->phy_addr, port_no);

    if (port_no != 0) {
        DPRINTK(3, "vsc_phy_clock_conf_set: Exitting - WARNING - NOT BASE PORT OF PHY!!  Port No: %x\n", port_no);
        DPRINTK(2, "WARNING: RECOVERED_CLK_NOT_CONFIGURED: RECOVERED Clocks can ONLY be configured from Base Port of PHY!! \n");
        return (VSC_PHY_ERROR);
    }

    switch (cntrl->phy_id.family) {
    // These families  support clock recovery, but need a squelch work-around
    case VSC_PHY_FAMILY_VIPER:
        if (cntrl->phy_id.revision == VSC_PHY_VIPER_REV_A) {
            use_squelch_workaround = VSC_PHY_TRUE;
        } else {
            use_squelch_workaround = VSC_PHY_FALSE;
        }
        break;

    case VSC_PHY_FAMILY_TESLA:
        use_squelch_workaround = VSC_PHY_TRUE;
        break;

    case VSC_PHY_FAMILY_ELISE:
        // Only Certain chip Dash Numbers support clock configuration. The chip detection is determined by register 18E2
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT2_PAGE, 0xffff));
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT2_PAGE, 0x12, &reg_val));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT2_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        reg_val &= 0x3f;
        if ((reg_val == 0x10) || (reg_val == 0x13)) {    // VSC8514-10 or -11 and VSC8514-13 or -14 do not support SyncE
            return (VSC_PHY_ERR_RECOV_CLK_CONF_NOT_SUPPORTED);
        }
        use_squelch_workaround = VSC_PHY_FALSE;
        break;

    case VSC_PHY_FAMILY_NANO :
        // Only Certain chip Dash Numbers support clock configuration. The chip detection is determined by register 18E2
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT2_PAGE, 0xffff));
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT2_PAGE, 0x12, &reg_val));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT2_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        reg_val &= 0x3f;
        if (reg_val == 0x13) {    // VSC8502-13 does not support SyncE
            return (VSC_PHY_ERR_RECOV_CLK_CONF_NOT_SUPPORTED);
        }
        use_squelch_workaround = VSC_PHY_FALSE;
        break;

    case VSC_PHY_FAMILY_MINI :
        // Only Certain chips support clock configuration, the 8x8 pkg.
        if ((cntrl->phy_id.part_number = VSC_PHY_TYPE_8540) ||
            (cntrl->phy_id.part_number = VSC_PHY_TYPE_8541)) {
            //printf("Recovered Clk: Supported \n");
        } else {
            return (VSC_PHY_ERR_RECOV_CLK_CONF_NOT_SUPPORTED);
        }
        use_squelch_workaround = VSC_PHY_FALSE;
        break;

    default:
        // All other families don't support clock recovery
        DPRINTK(3, "clock recovery setup not supported on port %u, Chip family:0x%x,  Part_Number: %d \n", 
                   vsc_phy_chip_port_get(cntrl), cntrl->phy_id.family, cntrl->phy_id.part_number);
        return (VSC_PHY_ERR_RECOV_CLK_CONF_NOT_SUPPORTED);
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));

    if (clock_port == VSC_PHY_RECOV_CLK1) {
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x17, &reg_val));  // Reg 23G PHY_RECOVERED_CLOCK_0_CONTROL
    } else if (clock_port == VSC_PHY_RECOV_CLK2) {
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_GPIO_PAGE, 0x18, &reg_val));  // Reg 24G, PHY_RECOVERED_CLOCK_1_CONTROL
    } else {
        return (VSC_PHY_ERR_RECOV_CLK_CONF_NOT_SUPPORTED);
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    /* return if already disabled and new source is disable (due to the 'weired' disable) */
    if (((reg_val & 0x8000) == 0) && (conf->src == VSC_PHY_CLK_DISABLED)) {
        return (VSC_PHY_OK);
    }

    DPRINTK(4, "Set Recovered Clks: Clk Selection for port %u, Chip family:0x%x,  Part_Number: %d,  Clk_Src: 0x%x \n", 
                   vsc_phy_chip_port_get(cntrl), cntrl->phy_id.family, cntrl->phy_id.part_number, conf->src );

    switch (conf->src) {
    case VSC_PHY_SERDES_MEDIA:
        reg_val = 0x8000;
        break;
    case VSC_PHY_COPPER_MEDIA:
        reg_val = 0x8001;
        break;
    case VSC_PHY_TCLK_OUT:
        reg_val = 0x8002;
        break;
    case VSC_PHY_LOCAL_XTAL:
        reg_val = 0x8003;
        break;
    case VSC_PHY_CLK_DISABLED:
        reg_val = (reg_val + 1) & 0x0001;
        break; /* Know this is weird - have to set media wrong to get clock out disabled */
    default:
        return (VSC_PHY_OK);
    }

    switch (cntrl->phy_id.family) {
    case VSC_PHY_FAMILY_TESLA:
    case VSC_PHY_FAMILY_VIPER:
    case VSC_PHY_FAMILY_ELISE:
    case VSC_PHY_FAMILY_NANO :
    case VSC_PHY_FAMILY_MINI :
        /* This uses the current port_no of the PHY  */
        reg_val |= ((((conf->clk_src_sel % 4) & 0xF) << 11) | (conf->freq << 8) | (conf->squelch << 4));
        break;

    default:
        // All other families don't support clock recovery
        DPRINTK(3, "clock recovery setup not supported on port %u, Chip family:0x%x,  Part_Number: %d \n", 
                   vsc_phy_chip_port_get(cntrl), cntrl->phy_id.family, cntrl->phy_id.part_number);
        return (VSC_PHY_ERR_RECOV_CLK_CONF_NOT_SUPPORTED);
    }

    if (use_squelch_workaround) {
        CHK_RC(vsc_phy_squelch_workaround(cntrl, VSC_PHY_FALSE));

        // Set address to save squelch control for the other recovered-clock port
        if (cntrl->phy_id.revision == VSC_PHY_TESLA_REV_A) {
            save_squelch_ctrl_addr = (clock_port == VSC_PHY_RECOV_CLK1) ? 0x00BE : 0x00BD;
        } else {
            save_squelch_ctrl_addr = (clock_port == VSC_PHY_RECOV_CLK1) ? 0x0026 : 0x0025;
        }

        // This function requires access to Port 0
        CHK_RC(vsc_phy_set_micro_set_addr(cntrl, save_squelch_ctrl_addr));

        // Peek saved squelch control for the other recovered-clock port
        CHK_RC(vsc_phy_micro_peek(cntrl, 0x0, &other_port_save_squelch_ctrl));
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));

    if (clock_port == VSC_PHY_RECOV_CLK1) {
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x17, reg_val, 0xffff));
    } else {
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x18, reg_val, 0xffff));
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    if (use_squelch_workaround) {
        CHK_RC(vsc_phy_squelch_workaround(cntrl, VSC_PHY_TRUE));

        // Poke back saved squelch control for the other recovered-clock port
        CHK_RC(vsc_phy_micro_poke(cntrl, 0x0, other_port_save_squelch_ctrl));
    }

    return VSC_PHY_OK;
}

int32_t  vsc_phy_loopback_set(vsc_phy_control_t *cntrl, vsc_phy_loopback_t   *loopback)
{
    uint16_t              reg_val = 0;
    uint16_t              reg_mask = 0;
    uint16_t              reg_22E3 = 0;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    DPRINTK(5, "vsc_phy_conf_1g_set: Phy_Addr: 0x%x, <<< Chip Port_no: %d  >>>> \n", cntrl->phy_addr, vsc_phy_chip_port_get(cntrl));


    // FAR-End Loopback
    // Enable Far End Loopback, Set Reg23.3=1
    reg_mask = 0x1 << 3;
    reg_val = loopback->far_end_enable ? reg_mask : 0;  // Set or Clear Reg23.3 

    // Viper Rev. B work-around
    if (loopback->far_end_enable &&
        (cntrl->phy_id.family == VSC_PHY_FAMILY_VIPER) &&
        (cntrl->phy_id.revision == VSC_PHY_VIPER_REV_B)) {
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT3_PAGE, 0xffff));
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT3_PAGE, 0x16, &reg_22E3));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x16, 0x0, (0x1 << 13)));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x17, reg_val, reg_mask));

        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT3_PAGE, 0xffff));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x16, reg_22E3, 0xffff));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT3_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    } else {
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x17, reg_val, reg_mask));
    }

    // NEAR-End Loopback
    // Enable Near End Loopback, Set Reg0.14=1
    reg_mask = 0x1 << 14;
    reg_val = loopback->near_end_enable ? reg_mask : 0;  // Set or Clear Reg0.14 
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x00, reg_val, reg_mask));

    // CONNECTOR Loopback
    // Enable 1000BaseT connector Loopback, Set Reg24.0=1
    reg_mask = 0x1;
    reg_val = loopback->connector_enable ? reg_mask : 0;  // Set or Clear Reg24.0 
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x18, reg_val, reg_mask));

    // CONNECTOR Loopback
    // Disable Pair Swap Correction, Set Reg18.5=1
    reg_mask = 0x1 << 5;
    reg_val = loopback->connector_enable ? reg_mask : 0;  // Set or Clear Reg18.5 
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x12, reg_val, reg_mask));

    return VSC_PHY_OK;
}




