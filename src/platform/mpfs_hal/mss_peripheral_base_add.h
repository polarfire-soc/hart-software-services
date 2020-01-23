/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/*******************************************************************************
 *
 * @file mss_peripheral_base_add.h
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief source excel file g5_mss_top-MAS svn rev:92231 comment: Updated
 * MSSIO bank numbers
 *
 * SVN $Revision$
 * SVN $Date$
 */

#ifndef MSS_ADDRESS_MAP_H
#define MSS_ADDRESS_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#define        MSS_BUS_ERROR_UNIT_H0                0x01700000UL
#define        MSS_BUS_ERROR_UNIT_H1                0x01701000UL
#define        MSS_BUS_ERROR_UNIT_H2                0x01702000UL
#define        MSS_BUS_ERROR_UNIT_H3                0x01703000UL
#define        MSS_BUS_ERROR_UNIT_H4                0x01704000UL

#define        ERROR_DEVICE                         0x18000000UL    /* hifive unleashed only */

/* APB slots */
/*
 * By default, all the APB peripherals are connected to AXI-Slave 5 using the AXI to AHB and AHB to APB
 * bridges. This means that the multiple CPUS and Fabric interfaces arbitrate
 * for access to the APB slaves resulting in a variable access latency depended on system activity. This may
 * cause system issues in particular in AMP mode with two separate operating systems running on different CPU’s
 * A second AHB/APB bus system is connected to the AXI slave 6 port using system addresses 0x28000000-0x2FFFFFFF.
 * Each of the APB peripherals can be configured at device start up to be connected the main APB bus
 * (0x20000000-0x203FFFFF) or to the AMP APB bus (0x28000000-0x283FFFFF). This allows two independent access systems
 * from the CPUS to the peripherals. Devices marked as DUAL SLOT in the defines below may be mapped to the second APB
 * bus structure.
 *
 */
#define        MSS_BASE_ADD_MMUART0                 0x20000000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_WDOG0                   0x20001000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_SYSREG_PRIV             0x20002000UL
#define        MSS_BASE_ADD_SYSREG_SCB              0x20003000UL
#define        MSS_BASE_ADD_AXISW_CFG               0x20004000UL
#define        MSS_BASE_ADD_MPUCFG                  0x20005000UL
#define        MSS_BASE_ADD_FMETER                  0x20006000UL
#define        MSS_BASE_ADD_FI_CFG                  0x20007000UL
#define        MSS_BASE_ADD_MMC_CFG                 0x20008000UL
#define        MSS_BASE_ADD_DRC_CFG                 0x20080000UL
#define        MSS_BASE_ADD_MMUART1                 0x20100000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_WDOG1                   0x20101000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_MMUART2                 0x20102000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_WDOG2                   0x20103000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_MMUART3                 0x20104000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_WDOG3                   0x20105000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_MMUART4                 0x20106000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_WDOG4                   0x20107000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_SPI0                    0x20108000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_SPI1                    0x20109000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_I2C0                    0x2010A000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_I2C1                    0x2010B000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_AN0                     0x2010C000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_AN1                     0x2010D000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_MAC0_CFG                0x20110000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_MAC1_CFG                0x20112000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_GPIO0                   0x20120000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_GPIO1                   0x20121000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_GPIO2                   0x20122000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_MSRTC                   0x20124000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_MSTIMER                 0x20125000UL   /* DUAL SLOT */
#define        MSS_BASE_ADD_H2FINT                  0x20126000UL   /* DUAL SLOT */

#define        MSS_BASE_ADD_NVM_CFG                 0x20200000UL
#define        MSS_BASE_ADD_USB_CFG                 0x20201000UL
#define        MSS_BASE_ADD_NVM_DATA                0x20220000UL
#define        MSS_BASE_ADD_QSPI_XIP                0x21000000UL
#define        MSS_BASE_ADD_ATHENA                  0x22000000UL
#define        MSS_BASE_ADD_TRACE_AXIC              0x23000000UL
#define        MSS_BASE_ADD_TRACE_SMB               0x23010000UL
#define        MSS_BASE_ADD_TRACE_VC                0x23020000UL

#define        MSS_BASE_ADD_IOSCB_DATA              0x30000000UL
#define        MSS_BASE_ADD_IOSCB_CFG               0x37080000UL
#define        MSS_BASE_ADD_FIC3_FAB                0x40000000UL

#define        MSS_BASE_ADD_FIC0                    0x60000000UL
#define        MSS_BASE_ADD_DRC_CACHE               0x80000000UL
#define        MSS_BASE_ADD_DRC_NC                  0xC0000000UL
#define        MSS_BASE_ADD_DRC_NC_WCB              0xD0000000UL
#define        MSS_BASE_ADD_FIC1                    0xE0000000UL

#define        MSS_BASE_ADD_DRC_CACHE_AXI_L2        0x1000000000UL
#define        MSS_BASE_ADD_DRC_NC_AXI_NC           0x1400000000UL
#define        MSS_BASE_ADD_DRC_NC_WCB_AXI_NC       0x1800000000UL
#define        MSS_BASE_ADD_FIC0_AXI_F0             0x2000000000UL
#define        MSS_BASE_ADD_FIC1_AXI_F1             0x3000000000UL

#ifdef __cplusplus
}
#endif

#endif /* MSS_ADDRESS_MAP_H */
