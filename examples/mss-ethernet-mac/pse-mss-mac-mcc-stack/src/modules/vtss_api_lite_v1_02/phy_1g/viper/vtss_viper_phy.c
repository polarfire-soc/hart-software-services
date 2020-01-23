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
#include "vtss_viper_phy_init_scripts.h"
#include "vtss_viper_phy_micro_patch_9995.h"
#include "vtss_viper_phy_micro_patch_fb48.h"
#include "vtss_phy_common_prototypes.h"
#include "vtss_viper_phy_prototypes.h"


#if 0
#define CONFIG_PHY_VSC8584_DEBUG 0
#define CONFIG_PHY_VSC8584_DEBUG_ENABLE_PRINTF 5
#endif

#if defined(CONFIG_PHY_VSC8584_DEBUG) && (CONFIG_PHY_VSC8584_DEBUG > 0)

#define DPRINTK(level, format, args...)  do { if (CONFIG_PHY_VSC8584_DEBUG >= (level)) \
                                                  printk(VSC_8584_PHY format, ##args); } while (0)
#else

#if defined (CONFIG_PHY_VSC8584_DEBUG_ENABLE_PRINTF) && (CONFIG_PHY_VSC8584_DEBUG_ENABLE_PRINTF > 0)
#define DPRINTK(level, format, args...)  do { printf(VSC_8584_PHY format, ##args); } while (0)
#else
#define DPRINTK(level, format, args...)  do {} while (0)
#endif

#endif

/*< Products: VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
static phy_init_scripts_struct * request_init_scripts_viper(vsc_phy_control_t    *cntrl)
{
    phy_init_scripts_struct * p_iscripts=NULL;

    /*< Products: VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
    // DO NOT USE __KERNEL_ONLY__ option at this time 
#ifdef __KERNEL_ONLY__
    char *name;

    if (cntrl->phy_id.revision == VSC_PHY_VIPER_REV_A) {
        name = "vsc_viper_phy_init_scripts_rev_a";
    } else if (cntrl->phy_id.revision == VSC_PHY_VIPER_REV_B) {
        name = "vsc_viper_phy_init_scripts_rev_b";
    } else { // if later than Rev. B, Use the latest init-scripts available
        name = "vsc_viper_phy_init_scripts_rev_b";
    }

    request_firmware(&p_iscripts, name, (struct dev *)cntrl->phy_addr);
#else
    p_iscripts = (phy_init_scripts_struct *) malloc(sizeof(phy_init_scripts_struct));

    if (cntrl->phy_id.revision == VSC_PHY_VIPER_REV_A) {
        p_iscripts->size = sizeof(init_script_viper_rev_a);
        p_iscripts->data = (uint8_t *)init_script_viper_rev_a;
    } else if (cntrl->phy_id.revision == VSC_PHY_VIPER_REV_B) {
        p_iscripts->size = sizeof(init_script_viper_rev_b);
        p_iscripts->data = (uint8_t *)init_script_viper_rev_b;
    } else { // if later than Rev. B, Use the latest init-scripts available
        p_iscripts->size = sizeof(init_script_viper_rev_b);
        p_iscripts->data = (uint8_t *)init_script_viper_rev_b;
    }
#endif

    return (p_iscripts);
}


/*< Products: VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
static int32_t release_init_scripts_viper(phy_init_scripts_struct * p_iscripts)
{
    if (p_iscripts->data != NULL) {
#ifdef __KERNEL_ONLY__
        release_firmware(p_iscripts);
#else
        free(p_iscripts);
#endif
    } else {
        return (VSC_PHY_ERROR);
    }

    return (VSC_PHY_OK);
}

// init_scripts_viper_a is called on Base Port(0) 
/*< Products: VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
static int32_t init_scripts_viper(vsc_phy_control_t    *cntrl)
{
    uint16_t                           reg, mask;
    uint16_t                           page;
    uint16_t                           *ptr;
    uint16_t                           wr_reg_val, reg_val;
    int32_t                            i;
    phy_init_scripts_struct           *p_init_scripts;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT1_PAGE, 0x17, &reg_val));
    if ((reg_val & 0xf800) >> 11 != 0) {
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        DPRINTK(3,"Init Scripts must be downloaded on Port 0 of PHY, This is NOT PORT 0 of PHY! \n");
        return (VSC_PHY_OK);
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    if (cntrl->phy_id.part_number == VSC_PHY_TYPE_NONE) {
        CHK_RC(vsc_get_phy_type(cntrl));
    }

    if (cntrl->phy_id.family == VSC_PHY_FAMILY_VIPER) {
        p_init_scripts = request_init_scripts_viper(cntrl);
        if (p_init_scripts == NULL) {
            return(VSC_PHY_INVALID_REQ);
        }
    } else {
        return(VSC_PHY_ERROR);
    }

    int32_t elements = p_init_scripts->size / 2;
    for (i=0; i < elements; i+=5) {
        ptr = (uint16_t *)p_init_scripts->data + i;
        if ((*ptr == 0) || (*ptr == 1)) {
            page = *(ptr+1);
            reg = *(ptr+2);
            wr_reg_val = *(ptr+3);
            mask = *(ptr+4);
            CHK_RC(vsc_phy_write(cntrl, page, reg, wr_reg_val, mask));
        } else if ((*ptr == 0xf) && 
                   (*(ptr+1) == 0xffff) &&
                   (*(ptr+2) == 0xff)) {
            DPRINTK(4, "init_script_size: %d ; \n", elements);
            break;
        }
    }

    if (p_init_scripts->data != NULL) {
        release_init_scripts_viper(p_init_scripts);
    }

    return (VSC_PHY_OK);
};
 
// micro_patch_viper_a is called on Base Port(0) 
/*< Products: VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
static phy_micro_patch_struct * request_micro_patch_viper(vsc_phy_control_t    *cntrl)
{
    phy_micro_patch_struct * p_mpatch=NULL;

    if (cntrl->phy_id.revision == VSC_PHY_VIPER_REV_A) {
        // DO NOT USE __KERNEL_ONLY__ option at this time
#ifdef __KERNEL_ONLY__
        char name[] = "vsc_viper_phy_micro_patch_9995";
        request_firmware(&p_mpatch, name, (struct dev *)cntrl->phy_addr);
#else
        p_mpatch = (phy_micro_patch_struct *) malloc(sizeof(phy_micro_patch_struct));
        p_mpatch->size = sizeof(vsc_viper_phy_micro_patch_9995);
        p_mpatch->data = (uint8_t *)vsc_viper_phy_micro_patch_9995;
#endif
    }else if (cntrl->phy_id.revision == VSC_PHY_VIPER_REV_B) {
#ifdef __KERNEL_ONLY__
        char name[] = "vsc_viper_phy_micro_patch_fb48";
        request_firmware(&p_mpatch, name, (struct dev *)cntrl->phy_addr);
#else
        p_mpatch = (phy_micro_patch_struct *) malloc(sizeof(phy_micro_patch_struct));
        p_mpatch->size = sizeof(vsc_viper_phy_micro_patch_fb48);
        p_mpatch->data = (uint8_t *)vsc_viper_phy_micro_patch_fb48;
#endif
    } else {
#ifdef __KERNEL_ONLY__
        char name[] = "vsc_viper_phy_micro_patch_fb48";
        request_firmware(&p_mpatch, name, (struct dev *)cntrl->phy_addr);
#else
        p_mpatch = (phy_micro_patch_struct *) malloc(sizeof(phy_micro_patch_struct));
        p_mpatch->size = sizeof(vsc_viper_phy_micro_patch_fb48);
        p_mpatch->data = (uint8_t *)vsc_viper_phy_micro_patch_fb48;
#endif
    }

    return (p_mpatch);
}

// micro_patch_viper_a is called on Base Port(0) 
/*< Products: VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
static int32_t release_micro_patch_viper(phy_micro_patch_struct * p_mpatch)
{
    if (p_mpatch->data != NULL) {
#ifdef __KERNEL_ONLY__
        release_firmware(p_mpatch);
#else
        free(p_mpatch);
#endif
    }

    return (VSC_PHY_OK);
}

// micro_patch_viper_a is called on Base Port(0) 
/*< Products: VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
static int32_t micro_patch_viper(vsc_phy_control_t    *cntrl)
{
    uint16_t                 reg_val = 0;
    uint16_t                 crc_val = 0;
    uint16_t                 code_size = 0;
    int32_t                  i = 0;
    phy_micro_patch_struct  *p_mpatch;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT1_PAGE, 0x17, &reg_val));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    if ((reg_val & 0xf800) >> 11 != 0) {
        DPRINTK(3,"Micro-Patch must be downloaded on Port 0 of PHY, This is NOT PORT 0 of PHY!; \n");
        return (VSC_PHY_OK);
    }

    if (cntrl->phy_id.part_number == VSC_PHY_TYPE_NONE) {
        CHK_RC(vsc_get_phy_type(cntrl));
    }

    if (cntrl->phy_id.family == VSC_PHY_FAMILY_VIPER) {
        // If p_mpatch=NULL, there is No Patch identified for this PHY Revision
        p_mpatch = request_micro_patch_viper(cntrl);
        if (p_mpatch == NULL) {
            switch (cntrl->phy_id.revision) {
            case VSC_PHY_VIPER_REV_A:
                DPRINTK(2, "Error: PHY Micro-Patch Code Check Failure, Viper rev A \n");
                return(VSC_PHY_INVALID_REQ);
            case VSC_PHY_VIPER_REV_B:
                DPRINTK(2, "Error: PHY Micro-Patch Code Check Failure, Viper rev B \n");
                return (VSC_PHY_OK);
            default:
                DPRINTK(4, "Micro-Patch Code check not applicable for Revision of Viper B or Greater \n");
                return (VSC_PHY_OK);
            }
        }
    } else {
        return(VSC_PHY_ERROR);
    }

    // If p_mpatch!=NULL; Apply the Micro-patch
    code_size = p_mpatch->size + 1;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x800f, 0xffff));

    CHK_RC(vsc_phy_wait_for_micro(cntrl));

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0xc,  0x0000, 0x0800));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x9,  0x005b, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0xa,  0x005b, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0xc,  0x0800, 0x0800));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x800f, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x0,  0x0000, 0x8000));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x0000, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0xc,  0x0000, 0x0800));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x0,  0x7009, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0xc,  0x5002, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0xb,  0x0000, 0xffff));

    for (i = 0; i < code_size; i++) {
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0xc, (0x5000 | p_mpatch->data[i]), 0xffff));
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0xc,  0x0000, 0xffff));

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x0,  0x4018, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x0,  0xc018, 0xffff));

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x19, 0xE800, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1a, code_size, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x8008, 0xffff));

    CHK_RC(vsc_phy_wait_for_micro(cntrl));

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT1_PAGE, 0x19, &crc_val));

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff)); // Done with CRC Ok Check
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x800f, 0xffff));

    CHK_RC(vsc_phy_wait_for_micro(cntrl));

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0xc,  0x0000, 0x0800));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x9,  0x005b, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0xa,  0x005b, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0xc,  0x0800, 0x0800));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x800f, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x0,  0x0000, 0x8000));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x0000, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0xc,  0x0000, 0x0800));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0xc,  0x0000, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x0,  0x4098, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x0,  0xc098, 0xffff));

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    if (p_mpatch->data != NULL) {
        release_micro_patch_viper(p_mpatch);
    }

    switch (cntrl->phy_id.revision) {
    case VSC_PHY_VIPER_REV_A:
        if (crc_val != 0x9995) {
            DPRINTK(3, "ERROR! Micro-Patch Code size: %d;  Expected CRC: 0x9995; Calculated CRC: 0x%X\n", code_size, crc_val);
            return (VSC_PHY_CRC_ERROR);
        }
        break;
    case VSC_PHY_VIPER_REV_B:
        if (crc_val != 0xFB48) {
            DPRINTK(3, "ERROR! Micro-Patch Code size: %d;  Expected CRC: 0xFB48; Calculated CRC: 0x%X\n", code_size, crc_val);
            return (VSC_PHY_CRC_ERROR);
        } 
        break;
    default:  // Default to the Latest Micro-Patch
        if (crc_val != 0xFB48) {
            DPRINTK(3, "ERROR! Micro-Patch Code size: %d;  Expected CRC: 0xFB48; Calculated CRC: 0x%X\n", code_size, crc_val);
            return (VSC_PHY_CRC_ERROR);
        }
    }

    DPRINTK(4, "GOOD CRC! Micro-Patch Code size: %d;  Expected CRC: 0x9995; Calculated CRC: 0x%X\n", code_size, crc_val);
    return (VSC_PHY_OK);
};

// init_viper_phy_mac_if is called on each Port of the PHY
// Must be followed by phy_soft_reset to take effect
/*< Products: VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
static int32_t init_viper_phy_mac_if(vsc_phy_control_t    *cntrl)
{
    uint16_t       port_no = vsc_phy_chip_port_get(cntrl);

    if (port_no != 0) {
        DPRINTK(4, "Init PHY MAC i/f on Port 0 of PHY!  Chip Port No: %d\n", port_no);
        return (VSC_PHY_OK);
    }

    /* Only Init the MAC if once, on Port 0 */
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));

    switch (cntrl->mac_if) {
    case PHY_MAC_IF_MODE_SGMII:
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x13, 0x0000, 0xc000));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x80F0, 0xffff));
        CHK_RC(vsc_phy_wait_for_micro(cntrl));
        break;
    case PHY_MAC_IF_MODE_QSGMII:
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x13, 0x4000, 0xc000));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, 0x80E0, 0xffff));
        CHK_RC(vsc_phy_wait_for_micro(cntrl));
        break;
    default:
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
        return (VSC_PHY_ERROR);
    }

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    return (VSC_PHY_OK);
}

// init_viper_phy_media_if is called on each Port of the PHY
// Must be followed by phy_soft_reset to take effect
/*< Products: VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
static int32_t init_viper_phy_media_if(vsc_phy_control_t    *cntrl)
{
    uint16_t       reg_val = 0;
    uint16_t       cmd_100fx = 0;
    uint8_t        media_op = 0;
    uint8_t        cu_pref = 0;

    CHK_RC(vsc_get_phy_media_if_config(cntrl->media_if, &cmd_100fx, &media_op, &cu_pref));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_EXT1_PAGE, 0xffff));
    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_EXT1_PAGE, 0x17, &reg_val));
    reg_val = (reg_val & 0xf800) >> 11;

    if (cntrl->media_if != PHY_MEDIA_IF_CU) {
        reg_val = reg_val % 4;
        reg_val = 0x80C1 | (0x0100 << reg_val) | cmd_100fx;
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_GPIO_PAGE, 0xffff));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x12, reg_val, 0xffff));
        CHK_RC(vsc_phy_wait_for_micro(cntrl));
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_GPIO_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    } else {
        CHK_RC(vsc_phy_write(cntrl, VSC_PHY_EXT1_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    }

    reg_val  = (media_op & 0x7) << 8;
    reg_val |= (cu_pref ? 0x0800 : 0);
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x17, reg_val, 0x0f00));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));

    return (VSC_PHY_OK);
}

// This function gets called from Port 0, ie. cntrl->portAddr == BasePorti
// The tgt_port_no is the port_no that the operation is to occur upon.
int32_t viper_phy_media_sig_adjust(vsc_phy_control_t         *cntrl, 
                                   vsc_phy_media_interface_t  tgt_port_media_if,
                                   uint16_t                   tgt_port_no)
{
    uint8_t      is_100fx = 0;

    // Note: It is assumed that vsc_get_phy_type() has already been called and the port_cnt is correct for this device!
    // Modulo down to the PHY physical ports
    tgt_port_no = tgt_port_no % cntrl->phy_id.port_cnt;  

    /* Bug# 19146  */
    /* Adjust the 1G SerDes SigDet Input Threshold and Signal Sensitivity for 100FX */
    /* For Tesla, This is done in the Micro-Patch */
    /* For Viper, This is done here in the API */
    if ((tgt_port_media_if != PHY_MEDIA_IF_FI_100FX) || 
        (tgt_port_media_if != PHY_MEDIA_IF_AMS_CU_100FX) ||
        (tgt_port_media_if != PHY_MEDIA_IF_AMS_FI_100FX)) {
        is_100fx = 1;
    } else {
        is_100fx = 0;
    }

    CHK_RC(vsc_phy_sd1g_patch(cntrl, tgt_port_no, is_100fx));
    return (VSC_PHY_OK);
}

// soft_reset_viper_phy is called on each Port of the PHY 
// Normally called after configuring MAC and MEDIA i/f
/*< Products: VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
static int32_t soft_reset_viper_phy(vsc_phy_control_t    *cntrl)
{
    uint16_t       reg_val = 0;
    uint16_t       timeout = 100;

    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x1f, VSC_PHY_STD_PAGE, 0xffff));
    CHK_RC(vsc_phy_write(cntrl, VSC_PHY_STD_PAGE, 0x0, 0x8000, 0xffff));

    CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x0, &reg_val));
    while (reg_val & 0x8000 && timeout > 0) {
        CHK_RC(vsc_phy_read(cntrl, VSC_PHY_STD_PAGE, 0x0, &reg_val));
        timeout--;
        cntrl->phy_usleep(1000);
    }
    if (timeout == 0) {
        return (VSC_PHY_TIMEOUT);
    }

    return (VSC_PHY_OK);
}

/* Load init scripts and micro-code patch for whole PHY */
/*< Products: VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
int32_t initialize_viper_phy(vsc_phy_control_t    *cntrl)
{
    CHK_RC(vsc_get_phy_type(cntrl));
    CHK_RC(vsc_reset_phy_lcpll(cntrl));
    DPRINTK(3, "PHY Resetting LCPLL Complete\n");
    CHK_RC(init_scripts_viper(cntrl));
    CHK_RC(micro_patch_viper(cntrl));

    DPRINTK(3, "PHY Initialize Complete\n");
    return(VSC_PHY_OK);
}

/* Set MAC/MEDIA then Reset, Done to each PHY Port  */
/*< Products: VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
int32_t reset_viper_phy(vsc_phy_control_t    *cntrl)
{
    CHK_RC(init_viper_phy_mac_if(cntrl));
    CHK_RC(init_viper_phy_media_if(cntrl));
    CHK_RC(soft_reset_viper_phy(cntrl));

    DPRINTK(3, "PHY Reset Complete\n");
    return(VSC_PHY_OK);
}

/* Set/Change MEDIA then Reset, Done to each PHY Port  */
/*< Products: VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
int32_t reset_viper_phy_media_chg(vsc_phy_control_t    *cntrl)
{
    CHK_RC(init_viper_phy_media_if(cntrl));
    CHK_RC(soft_reset_viper_phy(cntrl));

    DPRINTK(3, "PHY Reset Media Change Only Complete\n");
    return(VSC_PHY_OK);
}


/* Set MAC/MEDIA then Reset, Done to each PHY Port  */
/*< Products: VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
int32_t post_reset_viper_phy(vsc_phy_control_t    *cntrl)
{
    uint8_t rc;

    /*< Apply the Patch to the Basic SerDes config  */
    CHK_RC(vsc_phy_sd6g_patch(cntrl));

    /*< Check to Make sure that the MAC i/f Basic SerDes config has been programmed */
    if ((rc = vsc_phy_chk_serdes_init_mac_mode(cntrl)) != VSC_PHY_TRUE) {
        return(VSC_PHY_ERROR);
    }

    /*< Check to Make sure that SerDes Patch config has been programmed correctly */
    if ((rc = vsc_phy_chk_serdes_patch_init(cntrl)) != VSC_PHY_TRUE) {
        return(VSC_PHY_ERROR);
    }

    CHK_RC(vsc_phy_coma_mode_set(cntrl, 0x1));

    DPRINTK(3, "PHY Post-Reset Complete\n");
    return (VSC_PHY_OK);
}

int32_t viper_phy_config_set(vsc_phy_control_t    *cntrl, vsc_phy_conf_t *conf)
{
    CHK_RC(vsc_phy_conf_set(cntrl, conf));

    DPRINTK(3, "PHY Addr 0x%x, Config Complete\n", cntrl->phy_addr);
    return(VSC_PHY_OK);
}

int32_t viper_phy_status_get(vsc_phy_control_t    *cntrl, vsc_phy_port_status_t  *status)
{
    CHK_RC(vsc_phy_status_get(cntrl, status));

    DPRINTK(3, "PHY Addr 0x%x, Get PHY Status Complete\n", cntrl->phy_addr);
    return(VSC_PHY_OK);
}

int32_t viper_phy_ob_level_set(vsc_phy_control_t    *cntrl, uint8_t  ob_level)
{
    CHK_RC(vsc_phy_sd6g_ob_level(cntrl, ob_level));
    DPRINTK(3, "PHY Addr 0x%x, Ob_Level Set Complete\n", cntrl->phy_addr);
    return(VSC_PHY_OK);
}

int32_t viper_phy_ob_level_get(vsc_phy_control_t    *cntrl, uint8_t  *ob_level)
{
    int32_t                rc = VSC_PHY_OK;      
    uint32_t               csr_reg = 0x29;  // ob_cfg1
    uint32_t               csr_reg_val = 0;

    *ob_level = 0;

    rc = vsc_phy_sd6g_csr_reg_read_debug(cntrl, csr_reg, &csr_reg_val);
    if (rc == VSC_PHY_OK) {
        *ob_level = csr_reg_val & 0x0000003f;   /* bits 5:0 */
    }

    DPRINTK(3, "PHY Addr 0x%x, Reading Ob_Level: 0x%x \n", cntrl->phy_addr, *ob_level);
    return(rc);
}


int32_t viper_phy_ob_post_set(vsc_phy_control_t    *cntrl, uint8_t  ob_post0, uint8_t  ob_post1)
{
    CHK_RC(vsc_phy_sd6g_ob_post(cntrl, ob_post0, ob_post1));
    DPRINTK(3, "PHY Addr 0x%x, Ob_Post Set Complete\n", cntrl->phy_addr);
    return(VSC_PHY_OK);
}

int32_t viper_phy_ob_post_get(vsc_phy_control_t    *cntrl, uint8_t  *ob_post0, uint8_t  *ob_post1)
{
    int32_t                rc = VSC_PHY_OK;
    uint32_t               csr_reg = 0x28;  // ob_cfg
    uint32_t               csr_reg_val = 0;

    *ob_post0 = 0;
    *ob_post1 = 0;

    rc = vsc_phy_sd6g_csr_reg_read_debug(cntrl, csr_reg, &csr_reg_val);
    if (rc == VSC_PHY_OK) {
        *ob_post0 = (csr_reg_val & 0x1f800000) >> 23;   /* bits 28:23 */
        *ob_post1 = (csr_reg_val & 0x007c0000) >> 18;   /* bits 18:22 */
    }

    DPRINTK(3, "PHY Addr 0x%x, Reading Ob_Post0: 0x%x,  Ob_Post1: 0x%x \n", cntrl->phy_addr, *ob_post0, *ob_post1);
    return(rc);
}

int32_t viper_phy_recov_clk_conf_set(vsc_phy_control_t          *cntrl, 
                                     const vsc_phy_recov_clk_t   clock_port,
                                     vsc_phy_clk_conf_t         *const conf)
{
    CHK_RC(vsc_phy_clock_conf_set(cntrl, clock_port, conf));

    return(VSC_PHY_OK);
}

int32_t viper_phy_recov_clk_conf_get(vsc_phy_control_t          *cntrl, 
                                     const vsc_phy_recov_clk_t   clock_port,
                                     vsc_phy_clk_conf_t         *conf)
{

    CHK_RC(vsc_phy_clock_conf_get(cntrl, clock_port, conf));

    return(VSC_PHY_OK);
}


int32_t viper_phy_conf_1g_set (vsc_phy_control_t          *cntrl,
                               vsc_phy_conf_1g_t          *conf)
{
    CHK_RC(vsc_phy_conf_1g_set(cntrl, conf));

    return(VSC_PHY_OK);
}

int32_t viper_phy_loopback_set(vsc_phy_control_t          *cntrl,
                               vsc_phy_loopback_t         *loopback)
{
    CHK_RC(vsc_phy_loopback_set (cntrl, loopback));

    return(VSC_PHY_OK);
}

int32_t viper_phy_packet_mode_set(vsc_phy_control_t          *cntrl,
                                 vsc_phy_pkt_mode_t          pkt_mode)
{
    CHK_RC(vsc_phy_packet_mode_set(cntrl, pkt_mode));
    return(VSC_PHY_OK);
}



