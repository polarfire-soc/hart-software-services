/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire and PolarFire SoC PCIe subsystem Core Registers data structures.
 *
 * SVN $Revision$
 * SVN $Date$
 */
#ifndef PF_PCIESS_REGS_H_
#define PF_PCIESS_REGS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------------
  PCS LANE Registers.
 */
typedef struct
{
    volatile    uint32_t    SOFT_RESET;
    volatile    uint32_t    LFWF_R0;
    volatile    uint32_t    LOVR_R0;
    volatile    uint32_t    LPIP_R0;
    volatile    uint32_t    L64_R0;
    volatile    uint32_t    L64_R1;
    volatile    uint32_t    L64_R2;
    volatile    uint32_t    L64_R3;
    volatile    uint32_t    L64_R4;
    volatile    uint32_t    L64_R5;
    volatile    uint32_t    L64_R6;
    volatile    uint32_t    L64_R7;
    volatile    uint32_t    L64_R8;
    volatile    uint32_t    L64_R9;
    volatile    uint32_t    L64_R10;
    volatile    uint32_t    RESERVED0;
    volatile    uint32_t    L8_R0;
    volatile    uint32_t    RESERVED1[3];
    volatile    uint32_t    LNTV_R0;
    volatile    uint32_t    RESERVED2;
    volatile    uint32_t    LCLK_R0;
    volatile    uint32_t    LCLK_R1;
    volatile    uint32_t    RESERVED3[2];
    volatile    uint32_t    LRST_R0;
    volatile    uint32_t    LRST_OPT;
    volatile    uint32_t    RESERVED4[2];
    volatile    uint32_t    OOB_R0;
    volatile    uint32_t    OOB_R1;
    volatile    uint32_t    OOB_R2;
    volatile    uint32_t    OOB_R3;
    volatile    uint32_t    PMA_CTRL_R0;
    volatile    uint32_t    PMA_CTRL_R1;
    volatile    uint32_t    PMA_CTRL_R2;
    volatile    uint32_t    MSTR_CTRL;

 } PCS_LANE_TypeDef;

/*------------------------------------------------------------------------------
  PCS Common Registers.
 */
 typedef struct
{
    volatile    uint32_t    SOFT_RESET;
    volatile    uint32_t    GSSCLK_CTRL;
    volatile    uint32_t    QRST_R0;
    volatile    uint32_t    QDBG_R0;

} PCS_CMN_TypeDef;

/*------------------------------------------------------------------------------
  PMA Lane Registers.
 */
 typedef struct
{
    volatile    uint32_t    SOFT_RESET;
    volatile    uint32_t    DES_CDR_CTRL_1;
    volatile    uint32_t    DES_CDR_CTRL_2;
    volatile    uint32_t    DES_CDR_CTRL_3;
    volatile    uint32_t    DES_DFEEM_CTRL_1;
    volatile    uint32_t    DES_DFEEM_CTRL_2;
    volatile    uint32_t    DES_DFEEM_CTRL_3;
    volatile    uint32_t    RESERVED0;
    volatile    uint32_t    DES_DFE_CTRL_1;
    volatile    uint32_t    DES_DFE_CTRL_2;
    volatile    uint32_t    DES_EM_CTRL_1;
    volatile    uint32_t    DES_EM_CTRL_2;
    volatile    uint32_t    DES_IN_TERM;
    volatile    uint32_t    DES_PKDET;
    volatile    uint32_t    DES_RTL_EM;
    volatile    uint32_t    DES_RTL_LOCK_CTR;
    volatile    uint32_t    DES_RXPLL_DIV;
    volatile    uint32_t    DES_TEST_BUS;
    volatile    uint32_t    DES_CLK_CTRL;
    volatile    uint32_t    DES_RSTPD;
    volatile    uint32_t    DES_RTL_ERR_CHK;
    volatile    uint32_t    DES_PCIE1_2_RXPLL_DIV;
    volatile    uint32_t    DES_SATA1_2_RXPLL_DIV;
    volatile    uint32_t    DES_SATA3_RXPLL_DIV;
    volatile    uint32_t    RESERVED1[4];
    volatile    uint32_t    SER_CTRL;
    volatile    uint32_t    SER_CLK_CTRL;
    volatile    uint32_t    SER_RSTPD;
    volatile    uint32_t    SER_DRV_BYP;
    volatile    uint32_t    SER_RXDET_CTRL;
    volatile    uint32_t    SER_RXDET_OUT;
    volatile    uint32_t    SER_STATIC_LSB;
    volatile    uint32_t    SER_STATIC_MSB;
    volatile    uint32_t    SER_TERM_CTRL;
    volatile    uint32_t    SER_TEST_BUS;
    volatile    uint32_t    SER_DRV_DATA_CTRL;
    volatile    uint32_t    SER_DRV_CTRL;
    volatile    uint32_t    SER_DRV_CTRL_SEL;
    volatile    uint32_t    SER_DRV_CTRL_M0;
    volatile    uint32_t    SER_DRV_CTRL_M1;
    volatile    uint32_t    SER_DRV_CTRL_M2;
    volatile    uint32_t    SER_DRV_CTRL_M3;
    volatile    uint32_t    SER_DRV_CTRL_M4;
    volatile    uint32_t    SER_DRV_CTRL_M5;
    volatile    uint32_t    RESERVED2;
    volatile    uint32_t    SERDES_RTL_CTRL;
    volatile    uint32_t    RESERVED3[3];
    volatile    uint32_t    DES_DFE_CAL_CTRL_0;
    volatile    uint32_t    DES_DFE_CAL_CTRL_1;
    volatile    uint32_t    DES_DFE_CAL_CTRL_2;
    volatile    uint32_t    DES_DFE_CAL_CMD;
    volatile    uint32_t    DES_DFE_CAL_BYPASS;
    volatile    uint32_t    DES_DFE_CAL_EYE_DATA;
    volatile    uint32_t    DES_DFE_CDRH0_MON;
    volatile    uint32_t    DES_DFE_COEFF_MON_0;
    volatile    uint32_t    DES_DFE_COEFF_MON_1;
    volatile    uint32_t    DES_DFE_CAL_OS_MON;
    volatile    uint32_t    DES_DFE_CAL_ST_0;
    volatile    uint32_t    DES_DFE_CAL_ST_1;
    volatile    uint32_t    DES_DFE_CAL_FLAG;

} PMA_LANE_TypeDef;

/*------------------------------------------------------------------------------
  TXPLL_SSC (PMA Common) Registers.
 */
 typedef struct
{
    volatile    uint32_t    SOFT_RESET;
    volatile    uint32_t    TXPLL_CLKBUF;
    volatile    uint32_t    TXPLL_CTRL;
    volatile    uint32_t    TXPLL_CLK_SEL;
    volatile    uint32_t    TXPLL_DIV_1;
    volatile    uint32_t    TXPLL_DIV_2;
    volatile    uint32_t    TXPLL_JA_1;
    volatile    uint32_t    TXPLL_JA_2;
    volatile    uint32_t    TXPLL_JA_3;
    volatile    uint32_t    TXPLL_JA_4;
    volatile    uint32_t    TXPLL_JA_5;
    volatile    uint32_t    TXPLL_JA_6;
    volatile    uint32_t    TXPLL_JA_7;
    volatile    uint32_t    TXPLL_JA_8;
    volatile    uint32_t    TXPLL_JA_9;
    volatile    uint32_t    TXPLL_JA_10;
    volatile    uint32_t    TXPLL_JA_RST;
    volatile    uint32_t    SERDES_SSMOD;
    volatile    uint32_t    RESERVED[2];
    volatile    uint32_t    SERDES_RTERM;
    volatile    uint32_t    SERDES_RTT;

} TXPLL_SSC_TypeDef;

/*------------------------------------------------------------------------------
  TXPLL (QAUD - extpll) Registers.
 */
typedef struct
{
    volatile    uint32_t    SOFT_RESET;
    volatile    uint32_t    EXTPLL_CLKBUF;
    volatile    uint32_t    EXTPLL_CTRL;
    volatile    uint32_t    EXTPLL_CLK_SEL;
    volatile    uint32_t    EXTPLL_DIV_1;
    volatile    uint32_t    EXTPLL_DIV_2;
    volatile    uint32_t    EXTPLL_JA_1;
    volatile    uint32_t    EXTPLL_JA_2;
    volatile    uint32_t    EXTPLL_JA_3;
    volatile    uint32_t    EXTPLL_JA_4;
    volatile    uint32_t    EXTPLL_JA_5;
    volatile    uint32_t    EXTPLL_JA_6;
    volatile    uint32_t    EXTPLL_JA_7;
    volatile    uint32_t    EXTPLL_JA_8;
    volatile    uint32_t    EXTPLL_JA_9;
    volatile    uint32_t    EXTPLL_JA_10;
    volatile    uint32_t    EXTPLL_JA_RST;

} TXPLL_TypeDef;

/*------------------------------------------------------------------------------
  PCIESS main Registers.
 */
typedef struct
{
    volatile    uint32_t    SOFT_RESET;
    volatile    uint32_t    OVRLY;
    volatile    uint32_t    MAJOR;
    volatile    uint32_t    INT_PIPE_CLK_CTRL;
    volatile    uint32_t    EXT_PIPE_CLK_CTRL;
    volatile    uint32_t    CLK_CTRL;
    volatile    uint32_t    QMUX_R0;
    volatile    uint32_t    RESERVED0[57];
    volatile    uint32_t    DLL_CTRL0;
    volatile    uint32_t    DLL_CTRL1;
    volatile    uint32_t    DLL_STAT0;
    volatile    uint32_t    DLL_STAT1;
    volatile    uint32_t    DLL_STAT2;
    volatile    uint32_t    TEST_DLL;
    volatile    uint32_t    RESERVED1[29];
    volatile    uint32_t    SPARE;

} PCIESS_MAIN_TypeDef;

/*------------------------------------------------------------------------------
  PCIE Control Registers.
 */
typedef struct
{
    volatile    uint32_t    SOFT_RESET;
    volatile    uint32_t    DEV_CONTROL;
    volatile    uint32_t    CLOCK_CONTROL;
    volatile    uint32_t    RESERVED0;
    volatile    uint32_t    SOFT_RESET_DEBUG_INFO;
    volatile    uint32_t    SOFT_RESET_CTLR;
    volatile    uint32_t    RESERVED1[2];
    volatile    uint32_t    SEC_ERROR_EVENT_CNT;
    volatile    uint32_t    DED_ERROR_EVENT_CNT;
    volatile    uint32_t    SEC_ERROR_INT;
    volatile    uint32_t    SEC_ERROR_INT_MASK;
    volatile    uint32_t    DED_ERROR_INT;
    volatile    uint32_t    DED_ERROR_INT_MASK;
    volatile    uint32_t    ECC_CONTROL;
    volatile    uint32_t    ECC_ERR_LOC;
    volatile    uint32_t    RAM_MARGIN_1;
    volatile    uint32_t    RAM_MARGIN_2;
    volatile    uint32_t    RAM_POWER_CONTROL;
    volatile    uint32_t    RESERVED2;
    volatile    uint32_t    DEBUG_SEL;
    volatile    uint32_t    RESERVED3[2];
    volatile    uint32_t    LTSSM_STATE;
    volatile    uint32_t    PHY_COMMON_INTERFACE;
    volatile    uint32_t    PL_TX_LANEIF_0;
    volatile    uint32_t    PL_RX_LANEIF_0;
    volatile    uint32_t    PL_WAKECLKREQ;
    volatile    uint32_t    RESERVED4[4];
    volatile    uint32_t    PCICONF_PCI_IDS_OVERRIDE;
    volatile    uint32_t    PCICONF_PCI_IDS_31_0;
    volatile    uint32_t    PCICONF_PCI_IDS_63_32;
    volatile    uint32_t    PCICONF_PCI_IDS_95_64;
    volatile    uint32_t    RESERVED5[4];
    volatile    uint32_t    PCIE_PEX_DEV_LINK_SPC2;
    volatile    uint32_t    PCIE_PEX_SPC;
    volatile    uint32_t    RESERVED6[22];
    volatile    uint32_t    PCIE_AXI_MASTER_ATR_CFG0;
    volatile    uint32_t    PCIE_AXI_MASTER_ATR_CFG1;
    volatile    uint32_t    PCIE_AXI_MASTER_ATR_CFG2;
    volatile    uint32_t    RESERVED7[5];
    volatile    uint32_t    AXI_SLAVE_PCIE_ATR_CFG0;
    volatile    uint32_t    AXI_SLAVE_PCIE_ATR_CFG1;
    volatile    uint32_t    AXI_SLAVE_PCIE_ATR_CFG2;
    volatile    uint32_t    RESERVED8[5];
    volatile    uint32_t    PCIE_BAR_01;
    volatile    uint32_t    PCIE_BAR_23;
    volatile    uint32_t    PCIE_BAR_45;
    volatile    uint32_t    PCIE_EVENT_INT;
    volatile    uint32_t    RESERVED9[12];
    volatile    uint32_t    PCIE_BAR_WIN;
    volatile    uint32_t    RESERVED10[703];
    volatile    uint32_t    TEST_BUS_IN_31_0;
    volatile    uint32_t    TEST_BUS_IN_63_32;

} PCIE_CTRL_TypeDef;

/*------------------------------------------------------------------------------
  PCIE Bridge Control Registers.
 */
typedef struct
{
    volatile    uint32_t    BRIDGE_VER;
    volatile    uint32_t    BRIDGE_BUS;
    volatile    uint32_t    BRIDGE_IMPL_IF;
    volatile    uint32_t    RESERVED;
    volatile    uint32_t    PCIE_IF_CONF;
    volatile    uint32_t    PCIE_BASIC_CONF;
    volatile    uint32_t    PCIE_BASIC_STATUS;
    volatile    uint32_t    RESERVED0[2];
    volatile    uint32_t    AXI_SLVL_CONF;
    volatile    uint32_t    RESERVED1[2];
    volatile    uint32_t    AXI_MST0_CONF;
    volatile    uint32_t    AXI_SLV0_CONF;
    volatile    uint32_t    RESERVED2[18];
    volatile    uint32_t    GEN_SETTINGS;
    volatile    uint32_t    PCIE_CFGCTRL;
    volatile    uint32_t    PCIE_PIPE_DW0;
    volatile    uint32_t    PCIE_PIPE_DW1;
    volatile    uint32_t    PCIE_VC_CRED_DW0;
    volatile    uint32_t    PCIE_VC_CRED_DW1;
    volatile    uint32_t    PCIE_PCI_IDS_DW0;
    volatile    uint32_t    PCIE_PCI_IDS_DW1;
    volatile    uint32_t    PCIE_PCI_IDS_DW2;
    volatile    uint32_t    PCIE_PCI_LPM;
    volatile    uint32_t    PCIE_PCI_IRQ_DW0;
    volatile    uint32_t    PCIE_PCI_IRQ_DW1;
    volatile    uint32_t    PCIE_PCI_IRQ_DW2;
    volatile    uint32_t    PCIE_PCI_IOV_DW0;
    volatile    uint32_t    PCIE_PCI_IOV_DW1;
    volatile    uint32_t    RESERVED3;
    volatile    uint32_t    PCIE_PEX_DEV;
    volatile    uint32_t    PCIE_PEX_DEV2;
    volatile    uint32_t    PCIE_PEX_LINK;
    volatile    uint32_t    PCIE_PEX_SLOT;
    volatile    uint32_t    PCIE_PEX_ROOT_VC;
    volatile    uint32_t    PCIE_PEX_SPC;
    volatile    uint32_t    PCIE_PEX_SPC2;
    volatile    uint32_t    PCIE_PEX_NFTS;
    volatile    uint32_t    PCIE_PEX_L1SS;
    volatile    uint32_t    PCIE_BAR_01_DW0;
    volatile    uint32_t    PCIE_BAR_01_DW1;
    volatile    uint32_t    PCIE_BAR_23_DW0;
    volatile    uint32_t    PCIE_BAR_23_DW1;
    volatile    uint32_t    PCIE_BAR_45_DW0;
    volatile    uint32_t    PCIE_BAR_45_DW1;
    volatile    uint32_t    PCIE_BAR_WIN;
    volatile    uint32_t    PCIE_EQ_PRESET_DW0;
    volatile    uint32_t    PCIE_EQ_PRESET_DW1;
    volatile    uint32_t    PCIE_EQ_PRESET_DW2;
    volatile    uint32_t    PCIE_EQ_PRESET_DW3;
    volatile    uint32_t    PCIE_EQ_PRESET_DW4;
    volatile    uint32_t    PCIE_EQ_PRESET_DW5;
    volatile    uint32_t    PCIE_EQ_PRESET_DW6;
    volatile    uint32_t    PCIE_EQ_PRESET_DW7;
    volatile    uint32_t    PCIE_SRIOV_DW0;
    volatile    uint32_t    PCIE_SRIOV_DW1;
    volatile    uint32_t    PCIE_SRIOV_DW2;
    volatile    uint32_t    PCIE_SRIOV_DW3;
    volatile    uint32_t    PCIE_SRIOV_DW4;
    volatile    uint32_t    PCIE_SRIOV_DW5;
    volatile    uint32_t    PCIE_SRIOV_DW6;
    volatile    uint32_t    PCIE_SRIOV_DW7;
    volatile    uint32_t    PCIE_CFGNUM;
    volatile    uint32_t    RESERVED4[12];
    volatile    uint32_t    PM_CONF_DW0;
    volatile    uint32_t    PM_CONF_DW1;
    volatile    uint32_t    PM_CONF_DW2;
    volatile    uint32_t    IMASK_LOCAL;
    volatile    uint32_t    ISTATUS_LOCAL;
    volatile    uint32_t    IMASK_HOST;
    volatile    uint32_t    ISTATUS_HOST;
    volatile    uint32_t    IMSI_ADDR;
    volatile    uint32_t    ISTATUS_MSI;
    volatile    uint32_t    ICMD_PM;
    volatile    uint32_t    ISTATUS_PM;
    volatile    uint32_t    ATS_PRI_REPORT;
    volatile    uint32_t    LTR_VALUES;
    volatile    uint32_t    RESERVED5[2];
    volatile    uint32_t    ISTATUS_DMA0;
    volatile    uint32_t    ISTATUS_DMA1;
    volatile    uint32_t    RESERVED6[8];
    volatile    uint32_t    ISTATUS_P_ADT_WIN0;
    volatile    uint32_t    ISTATUS_P_ADT_WIN1;
    volatile    uint32_t    ISTATUS_A_ADT_SLV0;
    volatile    uint32_t    ISTATUS_A_ADT_SLV1;
    volatile    uint32_t    ISTATUS_A_ADT_SLV2;
    volatile    uint32_t    ISTATUS_A_ADT_SLV3;
    volatile    uint32_t    RESERVED7[4];
    volatile    uint32_t    ROUTING_RULES_R_DW0;
    volatile    uint32_t    ROUTING_RULES_R_DW1;
    volatile    uint32_t    ROUTING_RULES_R_DW2;
    volatile    uint32_t    ROUTING_RULES_R_DW3;
    volatile    uint32_t    ROUTING_RULES_R_DW4;
    volatile    uint32_t    ROUTING_RULES_R_DW5;
    volatile    uint32_t    ROUTING_RULES_R_DW6;
    volatile    uint32_t    ROUTING_RULES_R_DW7;
    volatile    uint32_t    ROUTING_RULES_R_DW8;
    volatile    uint32_t    ROUTING_RULES_R_DW9;
    volatile    uint32_t    ROUTING_RULES_R_DW10;
    volatile    uint32_t    ROUTING_RULES_R_DW11;
    volatile    uint32_t    ROUTING_RULES_R_DW12;
    volatile    uint32_t    ROUTING_RULES_R_DW13;
    volatile    uint32_t    ROUTING_RULES_R_DW14;
    volatile    uint32_t    ROUTING_RULES_R_DW15;
    volatile    uint32_t    ROUTING_RULES_W_DW0;
    volatile    uint32_t    ROUTING_RULES_W_DW1;
    volatile    uint32_t    ROUTING_RULES_W_DW2;
    volatile    uint32_t    ROUTING_RULES_W_DW3;
    volatile    uint32_t    ROUTING_RULES_W_DW4;
    volatile    uint32_t    ROUTING_RULES_W_DW5;
    volatile    uint32_t    ROUTING_RULES_W_DW6;
    volatile    uint32_t    ROUTING_RULES_W_DW7;
    volatile    uint32_t    ROUTING_RULES_W_DW8;
    volatile    uint32_t    ROUTING_RULES_W_DW9;
    volatile    uint32_t    ROUTING_RULES_W_DW10;
    volatile    uint32_t    ROUTING_RULES_W_DW11;
    volatile    uint32_t    ROUTING_RULES_W_DW12;
    volatile    uint32_t    ROUTING_RULES_W_DW13;
    volatile    uint32_t    ROUTING_RULES_W_DW14;
    volatile    uint32_t    ROUTING_RULES_W_DW15;
    volatile    uint32_t    ARBITRATION_RULES_DW0;
    volatile    uint32_t    ARBITRATION_RULES_DW1;
    volatile    uint32_t    ARBITRATION_RULES_DW2;
    volatile    uint32_t    ARBITRATION_RULES_DW3;
    volatile    uint32_t    ARBITRATION_RULES_DW4;
    volatile    uint32_t    ARBITRATION_RULES_DW5;
    volatile    uint32_t    ARBITRATION_RULES_DW6;
    volatile    uint32_t    ARBITRATION_RULES_DW7;
    volatile    uint32_t    ARBITRATION_RULES_DW8;
    volatile    uint32_t    ARBITRATION_RULES_DW9;
    volatile    uint32_t    ARBITRATION_RULES_DW10;
    volatile    uint32_t    ARBITRATION_RULES_DW11;
    volatile    uint32_t    ARBITRATION_RULES_DW12;
    volatile    uint32_t    ARBITRATION_RULES_DW13;
    volatile    uint32_t    ARBITRATION_RULES_DW14;
    volatile    uint32_t    ARBITRATION_RULES_DW15;
    volatile    uint32_t    PRIORITY_RULES_DW0;
    volatile    uint32_t    PRIORITY_RULES_DW1;
    volatile    uint32_t    PRIORITY_RULES_DW2;
    volatile    uint32_t    PRIORITY_RULES_DW3;
    volatile    uint32_t    PRIORITY_RULES_DW4;
    volatile    uint32_t    PRIORITY_RULES_DW5;
    volatile    uint32_t    PRIORITY_RULES_DW6;
    volatile    uint32_t    PRIORITY_RULES_DW7;
    volatile    uint32_t    PRIORITY_RULES_DW8;
    volatile    uint32_t    PRIORITY_RULES_DW9;
    volatile    uint32_t    PRIORITY_RULES_DW10;
    volatile    uint32_t    PRIORITY_RULES_DW11;
    volatile    uint32_t    PRIORITY_RULES_DW12;
    volatile    uint32_t    PRIORITY_RULES_DW13;
    volatile    uint32_t    PRIORITY_RULES_DW14;
    volatile    uint32_t    PRIORITY_RULES_DW15;
    volatile    uint32_t    RESERVED8[48];
    volatile    uint32_t    P2A_TC_QOS_CONV;
    volatile    uint32_t    P2A_ATTR_CACHE_CONV;
    volatile    uint32_t    P2A_NC_BASE_ADDR_DW0;
    volatile    uint32_t    P2A_NC_BASE_ADDR_DW1;
    volatile    uint32_t    RESERVED9[12];
    volatile    uint32_t    DMA0_SRC_PARAM;
    volatile    uint32_t    DMA0_DESTPARAM;
    volatile    uint32_t    DMA0_SRCADDR_LDW;
    volatile    uint32_t    DMA0_SRCADDR_UDW;
    volatile    uint32_t    DMA0_DESTADDR_LDW;
    volatile    uint32_t    DMA0_DESTADDR_UDW;
    volatile    uint32_t    DMA0_LENGTH;
    volatile    uint32_t    DMA0_CONTROL;
    volatile    uint32_t    DMA0_STATUS;
    volatile    uint32_t    DMA0_PRC_LENGTH;
    volatile    uint32_t    DMA0_SHARE_ACCESS;
    volatile    uint32_t    RESERVED10[5];
    volatile    uint32_t    DMA1_SRC_PARAM;
    volatile    uint32_t    DMA1_DESTPARAM;
    volatile    uint32_t    DMA1_SRCADDR_LDW;
    volatile    uint32_t    DMA1_SRCADDR_UDW;
    volatile    uint32_t    DMA1_DESTADDR_LDW;
    volatile    uint32_t    DMA1_DESTADDR_UDW;
    volatile    uint32_t    DMA1_LENGTH;
    volatile    uint32_t    DMA1_CONTROL;
    volatile    uint32_t    DMA1_STATUS;
    volatile    uint32_t    DMA1_PRC_LENGTH;
    volatile    uint32_t    DMA1_SHARE_ACCESS;
    volatile    uint32_t    RESERVED11[101];
    volatile    uint32_t    ATR0_PCIE_WIN0_SRCADDR_PARAM;
    volatile    uint32_t    ATR0_PCIE_WIN0_SRC_ADDR;
    volatile    uint32_t    ATR0_PCIE_WIN0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR0_PCIE_WIN0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR0_PCIE_WIN0_TRSL_PARAM;
    volatile    uint32_t    RESERVED12;
    volatile    uint32_t    ATR0_PCIE_WIN0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR0_PCIE_WIN0_TRSL_MASK_DW1;
    volatile    uint32_t    ATR1_PCIE_WIN0_SRCADDR_PARAM;
    volatile    uint32_t    ATR1_PCIE_WIN0_SRC_ADDR;
    volatile    uint32_t    ATR1_PCIE_WIN0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR1_PCIE_WIN0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR1_PCIE_WIN0_TRSL_PARAM;
    volatile    uint32_t    RESERVED13;
    volatile    uint32_t    ATR1_PCIE_WIN0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR1_PCIE_WIN0_TRSL_MASK_DW1;
    volatile    uint32_t    ATR2_PCIE_WIN0_SRCADDR_PARAM;
    volatile    uint32_t    ATR2_PCIE_WIN0_SRC_ADDR;
    volatile    uint32_t    ATR2_PCIE_WIN0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR2_PCIE_WIN0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR2_PCIE_WIN0_TRSL_PARAM;
    volatile    uint32_t    RESERVED14;
    volatile    uint32_t    ATR2_PCIE_WIN0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR2_PCIE_WIN0_TRSL_MASK_DW1;
    volatile    uint32_t    ATR3_PCIE_WIN0_SRCADDR_PARAM;
    volatile    uint32_t    ATR3_PCIE_WIN0_SRC_ADDR;
    volatile    uint32_t    ATR3_PCIE_WIN0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR3_PCIE_WIN0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR3_PCIE_WIN0_TRSL_PARAM;
    volatile    uint32_t    RESERVED15;
    volatile    uint32_t    ATR3_PCIE_WIN0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR3_PCIE_WIN0_TRSL_MASK_DW1;
    volatile    uint32_t    ATR4_PCIE_WIN0_SRCADDR_PARAM;
    volatile    uint32_t    ATR4_PCIE_WIN0_SRC_ADDR;
    volatile    uint32_t    ATR4_PCIE_WIN0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR4_PCIE_WIN0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR4_PCIE_WIN0_TRSL_PARAM;
    volatile    uint32_t    RESERVED16;
    volatile    uint32_t    ATR4_PCIE_WIN0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR4_PCIE_WIN0_TRSL_MASK_DW1;
    volatile    uint32_t    ATR5_PCIE_WIN0_SRCADDR_PARAM;
    volatile    uint32_t    ATR5_PCIE_WIN0_SRC_ADDR;
    volatile    uint32_t    ATR5_PCIE_WIN0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR5_PCIE_WIN0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR5_PCIE_WIN0_TRSL_PARAM;
    volatile    uint32_t    RESERVED17;
    volatile    uint32_t    ATR5_PCIE_WIN0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR5_PCIE_WIN0_TRSL_MASK_DW1;
    volatile    uint32_t    ATR6_PCIE_WIN0_SRCADDR_PARAM;
    volatile    uint32_t    ATR6_PCIE_WIN0_SRC_ADDR;
    volatile    uint32_t    ATR6_PCIE_WIN0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR6_PCIE_WIN0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR6_PCIE_WIN0_TRSL_PARAM;
    volatile    uint32_t    RESERVED18;
    volatile    uint32_t    ATR6_PCIE_WIN0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR6_PCIE_WIN0_TRSL_MASK_DW1;
    volatile    uint32_t    ATR7_PCIE_WIN0_SRCADDR_PARAM;
    volatile    uint32_t    ATR7_PCIE_WIN0_SRC_ADDR;
    volatile    uint32_t    ATR7_PCIE_WIN0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR7_PCIE_WIN0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR7_PCIE_WIN0_TRSL_PARAM;
    volatile    uint32_t    RESERVED19;
    volatile    uint32_t    ATR7_PCIE_WIN0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR7_PCIE_WIN0_TRSL_MASK_DW1;

    volatile    uint32_t    ATR0_PCIE_WIN1_SRCADDR_PARAM;
    volatile    uint32_t    ATR0_PCIE_WIN1_SRC_ADDR;
    volatile    uint32_t    ATR0_PCIE_WIN1_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR0_PCIE_WIN1_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR0_PCIE_WIN1_TRSL_PARAM;
    volatile    uint32_t    RESERVED20;
    volatile    uint32_t    ATR0_PCIE_WIN1_TRSL_MASK_DW0;
    volatile    uint32_t    ATR0_PCIE_WIN1_TRSL_MASK_DW1;
    volatile    uint32_t    ATR1_PCIE_WIN1_SRCADDR_PARAM;
    volatile    uint32_t    ATR1_PCIE_WIN1_SRC_ADDR;
    volatile    uint32_t    ATR1_PCIE_WIN1_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR1_PCIE_WIN1_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR1_PCIE_WIN1_TRSL_PARAM;
    volatile    uint32_t    RESERVED21;
    volatile    uint32_t    ATR1_PCIE_WIN1_TRSL_MASK_DW0;
    volatile    uint32_t    ATR1_PCIE_WIN1_TRSL_MASK_DW1;
    volatile    uint32_t    ATR2_PCIE_WIN1_SRCADDR_PARAM;
    volatile    uint32_t    ATR2_PCIE_WIN1_SRC_ADDR;
    volatile    uint32_t    ATR2_PCIE_WIN1_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR2_PCIE_WIN1_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR2_PCIE_WIN1_TRSL_PARAM;
    volatile    uint32_t    RESERVED22;
    volatile    uint32_t    ATR2_PCIE_WIN1_TRSL_MASK_DW0;
    volatile    uint32_t    ATR2_PCIE_WIN1_TRSL_MASK_DW1;
    volatile    uint32_t    ATR3_PCIE_WIN1_SRCADDR_PARAM;
    volatile    uint32_t    ATR3_PCIE_WIN1_SRC_ADDR;
    volatile    uint32_t    ATR3_PCIE_WIN1_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR3_PCIE_WIN1_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR3_PCIE_WIN1_TRSL_PARAM;
    volatile    uint32_t    RESERVED23;
    volatile    uint32_t    ATR3_PCIE_WIN1_TRSL_MASK_DW0;
    volatile    uint32_t    ATR3_PCIE_WIN1_TRSL_MASK_DW1;
    volatile    uint32_t    ATR4_PCIE_WIN1_SRCADDR_PARAM;
    volatile    uint32_t    ATR4_PCIE_WIN1_SRC_ADDR;
    volatile    uint32_t    ATR4_PCIE_WIN1_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR4_PCIE_WIN1_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR4_PCIE_WIN1_TRSL_PARAM;
    volatile    uint32_t    RESERVED24;
    volatile    uint32_t    ATR4_PCIE_WIN1_TRSL_MASK_DW0;
    volatile    uint32_t    ATR4_PCIE_WIN1_TRSL_MASK_DW1;
    volatile    uint32_t    ATR5_PCIE_WIN1_SRCADDR_PARAM;
    volatile    uint32_t    ATR5_PCIE_WIN1_SRC_ADDR;
    volatile    uint32_t    ATR5_PCIE_WIN1_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR5_PCIE_WIN1_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR5_PCIE_WIN1_TRSL_PARAM;
    volatile    uint32_t    RESERVED25;
    volatile    uint32_t    ATR5_PCIE_WIN1_TRSL_MASK_DW0;
    volatile    uint32_t    ATR5_PCIE_WIN1_TRSL_MASK_DW1;
    volatile    uint32_t    ATR6_PCIE_WIN1_SRCADDR_PARAM;
    volatile    uint32_t    ATR6_PCIE_WIN1_SRC_ADDR;
    volatile    uint32_t    ATR6_PCIE_WIN1_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR6_PCIE_WIN1_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR6_PCIE_WIN1_TRSL_PARAM;
    volatile    uint32_t    RESERVED26;
    volatile    uint32_t    ATR6_PCIE_WIN1_TRSL_MASK_DW0;
    volatile    uint32_t    ATR6_PCIE_WIN1_TRSL_MASK_DW1;
    volatile    uint32_t    ATR7_PCIE_WIN1_SRCADDR_PARAM;
    volatile    uint32_t    ATR7_PCIE_WIN1_SRC_ADDR;
    volatile    uint32_t    ATR7_PCIE_WIN1_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR7_PCIE_WIN1_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR7_PCIE_WIN1_TRSL_PARAM;
    volatile    uint32_t    RESERVED27;
    volatile    uint32_t    ATR7_PCIE_WIN1_TRSL_MASK_DW0;
    volatile    uint32_t    ATR7_PCIE_WIN1_TRSL_MASK_DW1;

    volatile    uint32_t    ATR0_AXI4_SLV0_SRCADDR_PARAM;
    volatile    uint32_t    ATR0_AXI4_SLV0_SRC_ADDR;
    volatile    uint32_t    ATR0_AXI4_SLV0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR0_AXI4_SLV0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR0_AXI4_SLV0_TRSL_PARAM;
    volatile    uint32_t    RESERVED28;
    volatile    uint32_t    ATR0_AXI4_SLV0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR0_AXI4_SLV0_TRSL_MASK_DW1;
    volatile    uint32_t    ATR1_AXI4_SLV0_SRCADDR_PARAM;
    volatile    uint32_t    ATR1_AXI4_SLV0_SRC_ADDR;
    volatile    uint32_t    ATR1_AXI4_SLV0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR1_AXI4_SLV0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR1_AXI4_SLV0_TRSL_PARAM;
    volatile    uint32_t    RESERVED29;
    volatile    uint32_t    ATR1_AXI4_SLV0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR1_AXI4_SLV0_TRSL_MASK_DW1;
    volatile    uint32_t    ATR2_AXI4_SLV0_SRCADDR_PARAM;
    volatile    uint32_t    ATR2_AXI4_SLV0_SRC_ADDR;
    volatile    uint32_t    ATR2_AXI4_SLV0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR2_AXI4_SLV0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR2_AXI4_SLV0_TRSL_PARAM;
    volatile    uint32_t    RESERVED30;
    volatile    uint32_t    ATR2_AXI4_SLV0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR2_AXI4_SLV0_TRSL_MASK_DW1;
    volatile    uint32_t    ATR3_AXI4_SLV0_SRCADDR_PARAM;
    volatile    uint32_t    ATR3_AXI4_SLV0_SRC_ADDR;
    volatile    uint32_t    ATR3_AXI4_SLV0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR3_AXI4_SLV0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR3_AXI4_SLV0_TRSL_PARAM;
    volatile    uint32_t    RESERVED31;
    volatile    uint32_t    ATR3_AXI4_SLV0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR3_AXI4_SLV0_TRSL_MASK_DW1;
    volatile    uint32_t    ATR4_AXI4_SLV0_SRCADDR_PARAM;
    volatile    uint32_t    ATR4_AXI4_SLV0_SRC_ADDR;
    volatile    uint32_t    ATR4_AXI4_SLV0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR4_AXI4_SLV0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR4_AXI4_SLV0_TRSL_PARAM;
    volatile    uint32_t    RESERVED32;
    volatile    uint32_t    ATR4_AXI4_SLV0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR4_AXI4_SLV0_TRSL_MASK_DW1;
    volatile    uint32_t    ATR5_AXI4_SLV0_SRCADDR_PARAM;
    volatile    uint32_t    ATR5_AXI4_SLV0_SRC_ADDR;
    volatile    uint32_t    ATR5_AXI4_SLV0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR5_AXI4_SLV0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR5_AXI4_SLV0_TRSL_PARAM;
    volatile    uint32_t    RESERVED33;
    volatile    uint32_t    ATR5_AXI4_SLV0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR5_AXI4_SLV0_TRSL_MASK_DW1;
    volatile    uint32_t    ATR6_AXI4_SLV0_SRCADDR_PARAM;
    volatile    uint32_t    ATR6_AXI4_SLV0_SRC_ADDR;
    volatile    uint32_t    ATR6_AXI4_SLV0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR6_AXI4_SLV0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR6_AXI4_SLV0_TRSL_PARAM;
    volatile    uint32_t    RESERVED34;
    volatile    uint32_t    ATR6_AXI4_SLV0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR6_AXI4_SLV0_TRSL_MASK_DW1;
    volatile    uint32_t    ATR7_AXI4_SLV0_SRCADDR_PARAM;
    volatile    uint32_t    ATR7_AXI4_SLV0_SRC_ADDR;
    volatile    uint32_t    ATR7_AXI4_SLV0_TRSL_ADDR_LSB;
    volatile    uint32_t    ATR7_AXI4_SLV0_TRSL_ADDR_UDW;
    volatile    uint32_t    ATR7_AXI4_SLV0_TRSL_PARAM;
    volatile    uint32_t    RESERVED35;
    volatile    uint32_t    ATR7_AXI4_SLV0_TRSL_MASK_DW0;
    volatile    uint32_t    ATR7_AXI4_SLV0_TRSL_MASK_DW1;

} PCIE_BRIDGE_TypeDef;

/*------------------------------------------------------------------------------
  PCIE Configuration Type 0 Space Registers.
 */
typedef struct
{
/*======================= Information registers ======================*/
    /**
      Information register: vendor_id & device_id
      bits [15:0]   vendor_id
      bits [31:16]  device_id
     */
    /* 0x000 */
    volatile    uint32_t    VID_DEVID;

    /**
      PCI Express Control & Status Register: cfg_prmscr
     */
    /* 0x004 */
    volatile    uint32_t    CFG_PRMSCR;

    /**
      Information register: class_code
     */
    /* 0x008 */
    volatile    uint32_t    CLASS_CODE;

     /**
      BIST, Header, master latency timer, cache : BIST_HEADER
     */
         /* 0x00C */
    volatile    uint32_t    BIST_HEADER;

    /**
      Bridge Configuration Register: bar0
     */
    /* 0x010 */
    volatile    uint32_t    BAR0;

    /**
      Bridge Configuration Register: bar1
     */
    /* 0x014 */
    volatile    uint32_t    BAR1;

    /**
      Bridge Configuration Register: bar2
     */
    /* 0x018 */
    volatile    uint32_t    BAR2;

    /**
      Bridge Configuration Register: bar3
     */
    /* 0x01C */
    volatile    uint32_t    BAR3;

    /**
      Bridge Configuration Register: bar4
     */
    /* 0x020 */
    volatile    uint32_t    BAR4;

    /**
      Bridge Configuration Register: bar5
     */
    /* 0x024 */
    volatile    uint32_t    BAR5;


    volatile    uint32_t    RESERVED1;

    /**
      Information register: subsystem_id
     */
    /* 0x02C */
    volatile    uint32_t    SUBSYSTEM_ID;

    /**
      Expansion ROM Base Address Register: expansion_rom
     */
    /* 0x030 */
    volatile    uint32_t    EXPAN_ROM;

    /**
      Capability pointer register: capab_pointer
     */
    /* 0x034 */
    volatile    uint32_t    CAPAB_POINTER;

    /**
      Expansion ROM Base Address register: expansion_rom_base
     */
    /* 0x038 */
    volatile    uint32_t    EXPAN_ROM_BASE;

    /**
      Interrupt Line and Pin register: int_line_pin
     */
    /* 0x03C */
    volatile    uint32_t    INT_LINE_PIN;

    /* 0x40 to 0x7C */
    volatile    uint32_t     RESERVED2[16];

    /* PCIe Capability structure register */

     /**
      PCIe Capability list register: CAPAB_LIST
     */
    /* 0x080 */
    volatile    uint32_t  CAPAB_LIST;

     /**
      Device Capabilities register: device_capab
     */
    /* 0x084 */
    volatile    uint32_t  DEVICE_CAPAB;

     /**
      Device Control and status register: device_ctrl_stat
     */
    /* 0x088 */
    volatile    uint32_t  DEVICE_CTRL_STAT;

     /**
      Link Capabilities register: link_capab
     */
    /* 0x08C */
    volatile    uint32_t  LINK_CAPAB;

     /**
      Link Control and status register: link_ctrl_stat
     */
    /* 0x090 */
    volatile    uint32_t  LINK_CTRL_STAT;

     /**
      Slot capabilities register: slot_capab
     */
    /* 0x094 */
    volatile    uint32_t  SLOT_CAPAB;

     /**
      Slot Control and status register: slot_ctrl_stat
     */
    /* 0x098 */
    volatile    uint32_t  SLOT_CTRL_STAT;

     /**
      Root control register: root_ctrl
     */
    /* 0x09C */
    volatile    uint32_t  ROOT_CTRL;

     /**
      Root status register: root_stat
     */
    /* 0x0A0 */
    volatile    uint32_t  ROOT_STAT;

     /**
      Device 2 Capabilities register: device2_capab
     */
    /* 0x0A4 */
    volatile    uint32_t  DEVICE2_CAPAB;

     /**
      Device 2 Control and status register: device2_ctrl_stat
     */
    /* 0x0A8 */
    volatile    uint32_t  DEVICE2_CTRL_STAT;

     /**
      Link Capabilities 2 register: link2_capab
     */
    /* 0x0AC */
    volatile    uint32_t  LINK2_CAPAB;

     /**
      Link Control and status 2register: link2_ctrl_stat
     */
    /* 0x0B0 */
    volatile    uint32_t  LINK2_CTRL_STAT;

     /**
      Slot 2 capabilities register: slot2_capab
     */
    /* 0x0B4 */
    volatile    uint32_t  SLOT2_CAPAB;

     /**
      Slot 2 Control and status register: slot2_ctrl_stat
     */
    /* 0x0B8 */
    volatile    uint32_t  SLOT2_CTRL_STAT;

    /* 0xBC to 0xCC */
    volatile    uint32_t     RESERVED3[5];

    /* MSI-X Capability (optional)*/

    /**
      MSI-X capability and control register: msi_x_capab_ctrl
     */
    /* 0x0D0 */
    volatile    uint32_t  MSI_X_CAPAB_CTRL;

    /**
      MSI-X table register: msi_x_table
     */
    /* 0x0D4 */
    volatile    uint32_t  MSI_X_TABLE;

    /**
      MSI-X PBA register: msi_x_pba
     */
    /* 0x0D8 */
    volatile    uint32_t  MSI_X_PBA;

    /* 0xDC */
    volatile    uint32_t     RESERVED4;

    /**
      MSI capability id and message control register: msi_capab_ctrl
     */
    /* 0x0E0 */
    volatile    uint32_t  MSI_CAPAB_CTRL;

    /**
      MSI message lower address register: msi_lower address
     */
    /* 0x0E4 */
    volatile    uint32_t  MSI_LOWER_ADDRESS;

    /**
      MSI message upper address register: msi_upper address
     */
    /* 0x0E8 */
    volatile    uint32_t  MSI_UPPER_ADDRESS;

    /* 0xEC */
    volatile    uint32_t     RESERVED5;

    /**
      MSI message data register: msi_data
     */
    /* 0x0F0 */
    volatile    uint32_t  MSI_DATA;
    /* 0xF4 */
    volatile    uint32_t     RESERVED6;

    /* Power Management Capability Structure */
    /**
       Power Management Capability register: power_mngm_capab
     */
    /* 0x0F8 */
    volatile    uint32_t  POWER_MNGM_CAPAB;

    /**
       Power Management control and status register: power_ctrl_stat
     */
    /* 0x0FC */
    volatile    uint32_t  POWER_CTRL_STAT;

} PCIE_END_CONF_TypeDef;

/*------------------------------------------------------------------------------
  PCIE Configuration Type 1 Space Registers.
 */
typedef struct
{
/*======================= Information registers ======================*/
    /**
      Information register: vendor_id & device_id
      bits [15:0]   vendor_id
      bits [31:16]  device_id
     */
    /* 0x000 */
    volatile    uint32_t    VID_DEVID;

    /**
      PCI Express Control & Status Register: cfg_prmscr
     */
    /* 0x004 */
    volatile    uint32_t    CFG_PRMSCR;

    /**
      Information register: class_code
     */
    /* 0x008 */
    volatile    uint32_t    CLASS_CODE;

     /**
      BIST, Header, master latency timer, cache : BIST_HEADER
     */
         /* 0x00C */
    volatile    uint32_t    BIST_HEADER;

    /**
      Bridge Configuration Register: bar0
     */
    /* 0x010 */
    volatile    uint32_t    BAR0;

    /**
      Bridge Configuration Register: bar1
     */
    /* 0x014 */
    volatile    uint32_t    BAR1;

    /**
      Secondary Latency timer, Subordinate bus number,
      Secondary bus Number, primary bus number Register: prim_sec_bus_num
     */
    /* 0x018 */
    volatile    uint32_t    PRIM_SEC_BUS_NUM;

    /**
      Secondary status, I/O limit, I/O base Register: io_limit_base
     */
    /* 0x01C */
    volatile    uint32_t    IO_LIMIT_BASE;

    /**
      memory limit, memory base Register: mem_limit_base
     */
    /* 0x020 */
    volatile    uint32_t    MEM_LIMIT_BASE;

    /**
      prefetchable memory limit, memory base Register: pref_mem_limit_base
     */
    /* 0x024 */
    volatile    uint32_t    PREF_MEM_LIMIT_BASE;

    /**
      prefetchable base upper Register: pref_base_upper
     */
    /* 0x028 */
    volatile    uint32_t    PREF_BASE_UPPER;

    /**
      prefetchable limit upper Register: pref_limit_upper
     */
    /* 0x02C */
    volatile    uint32_t    PREF_LIMIT_UPPER;

    /**
      i/o base, limit upper Register: io_limit_base_upper
     */
    /* 0x030 */
    volatile    uint32_t    IO_LIMIT_BASE_UPPER;

    /**
      Capability pointer register: capab_pointer
     */
    /* 0x034 */
    volatile    uint32_t    CAPAB_POINTER;

    /**
      Expansion ROM Base Address register: expansion_rom_base
     */
    /* 0x038 */
    volatile    uint32_t    EXPAN_ROM_BASE;

    /**
      Interrupt Line and Pin register: int_line_pin
     */
    /* 0x03C */
    volatile    uint32_t    INT_LINE_PIN;

    /* 0x40 to 0x7C */
    volatile    uint32_t     RESERVED2[16];

    /* PCIe Capability structure register */

     /**
      PCIe Capability list register: CAPAB_LIST
     */
    /* 0x080 */
    volatile    uint32_t  CAPAB_LIST;

     /**
      Device Capabilities register: device_capab
     */
    /* 0x084 */
    volatile    uint32_t  DEVICE_CAPAB;

     /**
      Device Control and status register: device_ctrl_stat
     */
    /* 0x088 */
    volatile    uint32_t  DEVICE_CTRL_STAT;

     /**
      Link Capabilities register: link_capab
     */
    /* 0x08C */
    volatile    uint32_t  LINK_CAPAB;

     /**
      Link Control and status register: link_ctrl_stat
     */
    /* 0x090 */
    volatile    uint32_t  LINK_CTRL_STAT;

     /**
      Slot capabilities register: slot_capab
     */
    /* 0x094 */
    volatile    uint32_t  SLOT_CAPAB;

     /**
      Slot Control and status register: slot_ctrl_stat
     */
    /* 0x098 */
    volatile    uint32_t  SLOT_CTRL_STAT;

     /**
      Root control register: root_ctrl
     */
    /* 0x09C */
    volatile    uint32_t  ROOT_CTRL;

     /**
      Root status register: root_stat
     */
    /* 0x0A0 */
    volatile    uint32_t  ROOT_STAT;

     /**
      Device 2 Capabilities register: device2_capab
     */
    /* 0x0A4 */
    volatile    uint32_t  DEVICE2_CAPAB;

     /**
      Device 2 Control and status register: device2_ctrl_stat
     */
    /* 0x0A8 */
    volatile    uint32_t  DEVICE2_CTRL_STAT;

     /**
      Link Capabilities 2 register: link2_capab
     */
    /* 0x0AC */
    volatile    uint32_t  LINK2_CAPAB;

     /**
      Link Control and status 2register: link2_ctrl_stat
     */
    /* 0x0B0 */
    volatile    uint32_t  LINK2_CTRL_STAT;

     /**
      Slot 2 capabilities register: slot2_capab
     */
    /* 0x0B4 */
    volatile    uint32_t  SLOT2_CAPAB;

     /**
      Slot 2 Control and status register: slot2_ctrl_stat
     */
    /* 0x0B8 */
    volatile    uint32_t  SLOT2_CTRL_STAT;

    /* 0xBC to 0xCC */
    volatile    uint32_t     RESERVED3[5];

    /* MSI-X Capability (optional)*/

    /**
      MSI-X capability and control register: msi_x_capab_ctrl
     */
    /* 0x0D0 */
    volatile    uint32_t  MSI_X_CAPAB_CTRL;

    /**
      MSI-X table register: msi_x_table
     */
    /* 0x0D4 */
    volatile    uint32_t  MSI_X_TABLE;

    /**
      MSI-X PBA register: msi_x_pba
     */
    /* 0x0D8 */
    volatile    uint32_t  MSI_X_PBA;

    /* 0xDC */
    volatile    uint32_t     RESERVED4;

    /**
      MSI capability id and message control register: msi_capab_ctrl
     */
    /* 0x0E0 */
    volatile    uint32_t  MSI_CAPAB_CTRL;

    /**
      MSI message lower address register: msi_lower address
     */
    /* 0x0E4 */
    volatile    uint32_t  MSI_LOWER_ADDRESS;

    /**
      MSI message upper address register: msi_upper address
     */
    /* 0x0E8 */
    volatile    uint32_t  MSI_UPPER_ADDRESS;

    /* 0xEC */
    volatile    uint32_t     RESERVED5;

    /**
      MSI message data register: msi_data
     */
    /* 0x0F0 */
    volatile    uint32_t  MSI_DATA;
    /* 0xF4 */
    volatile    uint32_t     RESERVED6;

    /* Power Management Capability Structure */
    /**
       Power Management Capability register: power_mngm_capab
     */
    /* 0x0F8 */
    volatile    uint32_t  POWER_MNGM_CAPAB;

    /**
       Power Management control and status register: power_ctrl_stat
     */
    /* 0x0FC */
    volatile    uint32_t  POWER_CTRL_STAT;

} PCIE_ROOT_CONF_TypeDef;

/*------------------------------------------------------------------------------
 PCIESS sub system registers

typedef struct
{
    PCS_LANE_TypeDef        pcs_lane[4];
    PCS_CMN_TypeDef         pcs_cmn;
    PMA_LANE_TypeDef        pma_lane[4];
    TXPLL_SSC_TypeDef       txpll_ssc;
    TXPLL_TypeDef           txpll[2];
    PCIESS_MAIN_TypeDef     pciess_main;
    PCIE_BRIDGE_TypeDef     pcie_bridge[2];
    PCIE_END_CONF_TypeDef   pcie_end_config_space;
    PCIE_ROOT_CONF_TypeDef  pcie_root_config_space;
    PCIE_CTRL_TypeDef       pcie_ctrl[2];

} PCIESS_TypeDef;
*/

/*----------------------------------------------------------------------------*/
#define PCS_LANE           PCS_LANE_TypeDef
#define PCS_CMN            PCS_CMN_TypeDef
#define PMA_LANE           PMA_LANE_TypeDef
#define TXPLL_SSC          TXPLL_SSC_TypeDef
#define TXPLL              TXPLL_TypeDef
#define PCIESS_MAIN        PCIESS_MAIN_TypeDef
#define PCIE_BRIDGE        PCIE_BRIDGE_TypeDef
#define PCIE_END_CONF      PCIE_END_CONF_TypeDef
#define PCIE_ROOT_CONF     PCIE_ROOT_CONF_TypeDef
#define PCIE_CTRL          PCIE_CTRL_TypeDef

/*----------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
  PCIE Configuration Type 0 Space Registers offset definitions.
 */
#define DEVICE_VID_DEVID            0x000u
#define DEVICE_CFG_PRMSCR           0x004u
#define DEVICE_CLASS_CODE           0x008u
#define DEVICE_BIST_HEADER          0x00Cu
#define DEVICE_BAR0                 0x010u
#define DEVICE_BAR1                 0x014u
#define DEVICE_BAR2                 0x018u
#define DEVICE_BAR3                 0x01Cu
#define DEVICE_BAR4                 0x020u
#define DEVICE_BAR5                 0x024u
#define DEVICE_SUBSYSTEM_ID         0x02Cu
#define DEVICE_EXPAN_ROM            0x030u
#define DEVICE_CAPAB_POINTER        0x034u
#define DEVICE_EXPAN_ROM_BASE       0x038u
#define DEVICE_INT_LINE_PIN         0x03Cu
#define DEVICE_CAPAB_LIST           0x080u
#define DEVICE_DEVICE_CAPAB         0x084u
#define DEVICE_DEVICE_CTRL_STAT     0x088u
#define DEVICE_LINK_CAPAB           0x08Cu
#define DEVICE_LINK_CTRL_STAT       0x090u
#define DEVICE_SLOT_CAPAB           0x094u
#define DEVICE_SLOT_CTRL_STAT       0x098u
#define DEVICE_ROOT_CTRL            0x09Cu
#define DEVICE_ROOT_STAT            0x0A0u
#define DEVICE_DEVICE2_CAPAB        0x0A4u
#define DEVICE_DEVICE2_CTRL_STAT    0x0A8u
#define DEVICE_LINK2_CAPAB          0x0ACu
#define DEVICE_LINK2_CTRL_STAT      0x0B0u
#define DEVICE_SLOT2_CAPAB          0x0B4u
#define DEVICE_SLOT2_CTRL_STAT      0x0B8u
#define DEVICE_MSI_X_CAPAB_CTRL     0x0D0u
#define DEVICE_MSI_X_TABLE          0x0D4u
#define DEVICE_MSI_X_PBA            0x0D8u
#define DEVICE_MSI_CAPAB_CTRL       0x0E0u
#define DEVICE_MSI_LOWER_ADDRESS    0x0E4u
#define DEVICE_MSI_UPPER_ADDRESS    0x0E8u
#define DEVICE_MSI_DATA             0x0F0u
#define DEVICE_POWER_MNGM_CAPAB     0x0F8u
#define DEVICE_POWER_CTRL_STAT      0x0FCu

/*------------------------------------------------------------------------------
The table below shows the base address and available address space for each of
the leaf instances in the pciess system.

Instance Name                       Base Address    Range

pcs_lane_0                          0x004 1000      4 KBytes
pcs_lane_1                          0x004 2000      4 KBytes
pcs_lane_2                          0x004 4000      4 KBytes
pcs_lane_3                          0x004 8000      4 KBytes
pcs_cmn                             0x005 0000      4 KBytes
pma_lane_0                          0x104 1000      4 KBytes
pma_lane_1                          0x104 2000      4 KBytes
pma_lane_2                          0x104 4000      4 KBytes
pma_lane_3                          0x104 8000      4 KBytes
txpll_ssc(pma_cmn)                  0x105 0000      4 KBytes
tx(quad)_pll_0                      0x204 4000      4 KBytes
tx(quad)_pll_1                      0x204 8000      4 KBytes
pciess_main                         0x205 0000      4 KBytes

pcie_top_0__g5_xpressrich3_bridge   0x300 4000      8 KBytes
pcie_top_0__pcie_ctrl               0x300 6000      4 KBytes
pcie_top_1__g5_xpressrich3_bridge   0x300 8000      8 KBytes
pcie_top_1__pcie_ctrl               0x300 A000      4 KBytes
------------------------------------------------------------------------------*/

#define PCIESS_PCS_LANE0_PHY_ADDR_OFFSET            0x00041000u
#define PCIESS_PCS_LANE1_PHY_ADDR_OFFSET            0x00042000u
#define PCIESS_PCS_LANE2_PHY_ADDR_OFFSET            0x00044000u
#define PCIESS_PCS_LANE3_PHY_ADDR_OFFSET            0x00048000u

#define PCIESS_PCS_CMN_PHY_ADDR_OFFSET              0x00050000u

#define PCIESS_PMA_LANE0_PHY_ADDR_OFFSET            0x01041000u
#define PCIESS_PMA_LANE1_PHY_ADDR_OFFSET            0x01042000u
#define PCIESS_PMA_LANE2_PHY_ADDR_OFFSET            0x01044000u
#define PCIESS_PMA_LANE3_PHY_ADDR_OFFSET            0x01048000u

#define PCIESS_TXPLL_SSC_PHY_ADDR_OFFSET            0x01050000u

#define PCIESS_TXPLL0_PHY_ADDR_OFFSET                0x02044000u
#define PCIESS_TXPLL1_PHY_ADDR_OFFSET                0x02048000u

#define PCIESS_MAIN_PHY_ADDR_OFFSET                 0x02050000u

#define PCIE0_BRIDGE_PHY_ADDR_OFFSET                0x03004000u
#define PCIE0_CRTL_PHY_ADDR_OFFSET                  0x03006000u

#define PCIE1_BRIDGE_PHY_ADDR_OFFSET                0x03008000u
#define PCIE1_CRTL_PHY_ADDR_OFFSET                  0x0300A000u

#ifdef __cplusplus
}
#endif

#endif /* PF_PCIESS_REGS_H_ */
