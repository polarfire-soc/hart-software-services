/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software example
 *
 */
/*******************************************************************************
 *
 * Platform definitions
 * Version based on requirements of MPFS MSS
 *
 */
 /*========================================================================*//**
  @mainpage Sample file detailing how hw_cfg_ddr.h should be constructed for
    the MPFS MSS

    @section intro_sec Introduction
    The  hw_cfg_ddr.h is to be located in the project
    /platform/config/hardware/ddr/
    directory.
    Currently this file must be hand crafted when using the MPFS MSS.

    You can use this file as sample.
    hw_platform.h must be copied to
    the /platform/config/hardware/ folder of your project. Then customize it per
    your HW design.

    @section driver_configuration Project configuration Instructions
    1. Change MPFS MSS DDR DEFINITIONS to match design used.

*//*==========================================================================*/

#ifndef HW_CFG_DDR_H_
#define HW_CFG_DDR_H_

#include <stdint.h>

    /*
        DDRMODE              0:3;
            DDR3, DDR3L, DDR4, LPDDR3, LPDDR4 decode:
            000 ddr3
            001 ddr33L
            010 ddr4
            011 LPDDR3
            100 LPDDR4
            111 OFF_MODE
        ECC                  3:1;
        CRC                  4:1;
        Bus_width            5:3;      DDR bus width (16 or 32)  0 => 16, 1 => 32
        DMI_DBI              8:1;
        DQ_drive             9:2;
        DQS_drive            11:2;
        ADD_CMD_drive        13:2;
        Clock_out_drive      15:2;
        DQ_termination       17:2;
        DQS_termination      19:2;
        ADD_CMD_input_pin_termination 21:2;
        preset_odt_clk       23:2;
        Power_down           25:1;
        rank                 26:1;
        Command_Address_Pipe 27:2;
    */
    #define OFF_MODE_DDRPHY_MODE            (0x7U<<0U)
    #define DDR3_USER_INPUT_DDRPHY_MODE     ((1U<<26U)|(1U<<8U)|(1U<<5U)|(1U<<3U)|(1U<<0U))
    #define LPDDR3_USER_INPUT_DDRPHY_MODE   ((1U<<26U)|(1U<<8U)|(1U<<5U)|(0U<<3U)|(3U<<0U))
    #define DDR4_USER_INPUT_DDRPHY_MODE     ((1U<<26U)|(1U<<8U)|(1U<<5U)|(1U<<3U)|(2U<<0U))
    #define LPDDR4_USER_INPUT_DDRPHY_MODE   ((1U<<26U)|(1U<<8U)|(1U<<5U)|(0U<<3U)|(4U<<0U))

	#define	DDR_MODE		DDR3_USER_INPUT_DDRPHY_MODE

    /*
     * DPC_BITS setting
     * Select VS bits for DDR mode selected
     *
     * [19]         dpc_move_en_v   enable dynamic control of vrgen circuit for ADDCMD pins
     * [18]         dpc_vrgen_en_v  enable vref generator for ADDCMD pins
     * [17:12]      dpc_vrgen_v     reference voltage ratio setting for ADDCMD pins
     * [11:11]      dpc_move_en_h   enable dynamic control of vrgen circuit for DQ/DQS pins
     * [10:10]      dpc_vrgen_en_h  enable vref generator for DQ/DQS pins
     * [9:4]        dpc_vrgen_h     reference voltage ratio setting for DQ/DQS pins
     * [3:0]        dpc_vs          bank voltage select for pvt calibration
     */
    #define USER_INPUT_DPC_BITS_INIT            ((0x5U<<0U)|(0x3CU<<4U)|(0U<<10U)|(0U<<11U)|(0x1EU<<12U)|(0U<<18U)|(0U<<19U))

    #define USER_INPUT_PHY_RANKS_TO_TRAIN           0x3U    /*  1 => 1 rank, 3 => 2 ranks */
    #define MAX_POSSIBLE_TIP_TRAININGS              0x05U    /* this is a fixed value, currently only 5 supported in the TIP  */
    #define TRAINING_MASK                           0x1FU   /*  The first five bits represent the currently supported training in the TIP  */
                                                            /*  This value will not change unless more training possibilities are added to the TIP */

    #define    SKIP_BCLKSCLK_TIP_TRAINING        (0x1U<<0U)
    #define    SKIP_addcmd_TIP_TRAINING          (0x1U<<1U)
    #define    SKIP_wrlvl_TIP_TRAINING           (0x1U<<2U)
    #define    SKIP_rdgate_TIP_TRAINING          (0x1U<<3U)
    #define    SKIP_dq_dqs_opt_TIP_TRAINING      (0x1U<<4U)

    #define USER_CONFIG_TRAINING_SKIP_SETTING       SKIP_BCLKSCLK_TIP_TRAINING   /*  Used to pick witch trainings you want */
                                                            /*  0 => no skips, 0x1F => ship all training,  0x01 => skip first training etc */
    #define TRAINING_TEST                           (TRAINING_MASK & ~USER_CONFIG_TRAINING_SKIP_SETTING)

    /*PARAMETERS USED TO DESCRIBE PHYSICAL CONFIGURATION OF LPDDR4 BUS*/
    #define USER_CONFIG_OVRT11      0xF00   /*g5soc_mss_regmap : CFG_DDR_SGMII_PHY : ovrt11, override for ADDCMD lane 1, default 0xF00*/
    #define USER_CONFIG_OVRT12      0xE06   /*g5soc_mss_regmap : CFG_DDR_SGMII_PHY : ovrt12, override for ADDCMD lane 2, default 0xE06*/


    /*
     * parameters used to set value in register
     * tip_cfg_params
     *
     * TIP STATIC PARAMETERS 0
     *
     *  30:22   Number of VCO Phase offsets between BCLK and SCLK
     *  21:13   Number of VCO Phase offsets between BCLK and SCLK
     *  12:6    Number of VCO Phase offsets between BCLK and SCLK
     *  5:3     Number of VCO Phase offsets between BCLK and SCLK
     *  2:0     Number of VCO Phase offsets between REFCLK and ADDCMD bits
     */
    #define TIP_CONFIG_PARAMS_WRLVL_TAP_OFFSET      0x00U
#if 1 /* in simulation we need to set this to 2, for hardware it will be dependent on the trace lengths */
 #define TIP_CONFIG_PARAMS_BCLK_VCOPHS_OFFSET       0x2U
#else
    #define TIP_CONFIG_PARAMS_BCLK_VCOPHS_OFFSET    0x4U
#endif
    #define TIP_CONFIG_PARAMS_ADDR_VCOPHS_OFFSET    0x4U
    #define TIP_CONFIG_PARAMS_ADDCMD_WAIT_COUNT     0x1FU
    #define TIP_CONFIG_PARAMS_READ_GATE_MIN_READS   0x1FU

    /* These can be inferred from MODE bus width setting and ECC setting */
    #define USER_DATA_LANES_USED                    4U     /* number of lanes used for data-  does not include ECC */
    #define ALL_DATA_LANES_MASK                     0xFFFFFFFFFF  /* MASK of data lanes, 2 -> 0xFFFF, 4 => 0xFFFFFFFF */
    #define USER_TOTAL_LANES_USED                   5U     /* Total number of lanes used including- ECC */
    #define DEFAULT_WRITE_CALIB_LANE_VALUE          0x03    /* value between 0 and 0xF */

/*
 *
 *
 * settings below are for the DDRC registers
 *
 *
 */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RAS_CFG_RAS                                           0x00000026U     /*tRAS - 7 bit */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RCD_CFG_RCD                                           0x0000000cU     /*tRCD - 7 bit */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RRD_CFG_RRD                                           0x00000007U     /*tRRD - 5 bit */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RP_CFG_RP                                             0x0000000cU     /*tRP - 6bit  ( precharge period) */

    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RC_CFG_RC1                                            0x0000032cU     /*tRC - 8bit  ( active to refresh/active cmd period) */

    #define DDRC_USER_CONFIG_MC_BASE2_CFG_FAW_CFG_FAW                                           0x0000001bU
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RFC_CFG_RFC                                           0x00000076U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RTP_CFG_RTP                                           0x00000008U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_WR_CFG_WR                                             0x00000010U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_WTR_CFG_WTR                                           0x00000008U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_CL_CFG_CL                                             0x0000000cU

    #define DDRC_USER_CONFIG_MC_BASE2_CFG_MRD_CFG_MRD                                           0x00000010U     /* tMRD ( load mode register cmd to active or refresh) */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_REF_PER_CFG_REF_PER                                   0x0000207cU     /* CFG REF PER -- period between refresh cmds */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_STARTUP_DELAY_CFG_STARTUP_DELAY                       0x000000c8U    /* CFG STARTUP DELAY Register 0xd0 was C8 This is startup time 10 */

    #define DDRC_USER_CONFIG_MC_BASE2_CFG_XS_CFG_XS                                             0x00000080U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_XSDLL_CFG_XSDLL                                       0x00000200U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_XPR_CFG_XPR                                           0x00000080U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_CWL_CFG_CWL1                                          0x0000000aU
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_ZQINIT_CAL_DURATION_CFG_ZQINIT_CAL_DURATION           0x000002abU

    #define DDRC_USER_CONFIG_MC_BASE2_CFG_ZQ_CAL_L_DURATION_CFG_ZQ_CAL_L_DURATION               0x00000156U /*tZQO */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_ZQ_CAL_S_DURATION_CFG_ZQ_CAL_S_DURATION               0x00000056U /*tZQCS */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_ZQ_CAL_R_DURATION_CFG_ZQ_CAL_R_DURATION               0x0000000bU /*tCKSRE_PAR when PAR enabled in mem clocks */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_CKSRX_CFG_CKSRX                                       0x0000000bU /*tCKSRX specified in mem clocks */

    #define DDRC_USER_CONFIG_RMW_CFG_DM_EN_CFG_DM_EN                                            0x00000001U     /* RMW ( mask related signals enable to connect memory)  todo: check this computes from address 0x005400 */

    #define DDRC_USER_CONFIG_DFI_CFG_DFI_T_PHY_RDLAT_CFG_DFI_T_PHY_RDLAT                        0x00000006;         /* DFI ( num mem clks dly between dfi_rddata_en to dfi_rddata_vld) */

    #define DDRC_USER_CONFIG_AXI_IF_CFG_AXI_START_ADDRESS_AXI1_0_CFG_AXI_START_ADDRESS_AXI1_0   0x00000000U   /* AXI Device st addr ( vlaid lower axi1-0 addr)  <=> ADDRC_MAP_SEG_BITS parameter */

    #define DDRC_USER_CONFIG_AXI_IF_CFG_AXI_START_ADDRESS_AXI1_1_CFG_AXI_START_ADDRESS_AXI1_1   0x00000000U   /* AXI Device st addr ( vlaid lower axi1-1 addr) */
    #define DDRC_USER_CONFIG_AXI_IF_CFG_AXI_START_ADDRESS_AXI2_0_CFG_AXI_START_ADDRESS_AXI2_0   0x00000000U   /* AXI Device st addr ( vlaid lower axi2-0 addr) */
    #define DDRC_USER_CONFIG_AXI_IF_CFG_AXI_START_ADDRESS_AXI2_1_CFG_AXI_START_ADDRESS_AXI2_1   0x00000000U   /* AXI Device st addr ( vlaid lower axi2-1 addr) */
    #define DDRC_USER_CONFIG_AXI_IF_CFG_AXI_END_ADDRESS_AXI1_0_CFG_AXI_END_ADDRESS_AXI1_0       0xFFFFFFFFU   /* end addr */
    #define DDRC_USER_CONFIG_AXI_IF_CFG_AXI_END_ADDRESS_AXI1_1_CFG_AXI_END_ADDRESS_AXI1_1       0x00000003U   /* end addr */
    #define DDRC_USER_CONFIG_AXI_IF_CFG_AXI_END_ADDRESS_AXI2_0_CFG_AXI_END_ADDRESS_AXI2_0       0xFFFFFFFFU   /* end addr */
    #define DDRC_USER_CONFIG_AXI_IF_CFG_AXI_END_ADDRESS_AXI2_1_CFG_AXI_END_ADDRESS_AXI2_1       0x00000003U   /* end addr */


    #define DDRC_USER_CONFIG_MC_BASE2_CFG_NUM_RANKS_CFG_NUM_RANKS                               0x00000002U   /* nUm memory ranks =2 */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_MEMORY_TYPE_CFG_MEMORY_TYPE                           0x00000008U   /* cfg memory type == DDR3  /*chng */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_MEM_COLBITS_CFG_MEM_COLBITS                           0x0000000aU   /* mem col bits */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_MEM_ROWBITS_CFG_MEM_ROWBITS                           0x0000000eU   /* mem row bits  /*chng */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_MEM_BANKBITS_CFG_MEM_BANKBITS                         0x00000003U   /* bank bits   /*chng */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_REGDIMM_CFG_REGDIMM                                   0x00000000U   /* cfg reg DIMM */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_LRDIMM_CFG_LRDIMM                                     0x00000000U   /* cfg lr DIMM */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RDIMM_LAT_CFG_RDIMM_LAT                               0x00000000U   /* cfg rdimm latency */

    #define DDRC_USER_CONFIG_CFG_NIBBLE_DEVICES                                                 0x00000000U   /* config NIBBLE DEVICE  0x003cc4, */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS0_0                                            0x81881881U   /* BIT MAP INDEX CS0-0 specifies the DQ signals mapping between MC */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS0_1                                            0x00008818U   /* BIT MAP INDEX CS0-1 */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS1_0                                            0xa92a92a9U   /* BIT MAP INDEX CS1-0 */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS1_1                                            0x00002a92U   /* BIT MAP INDEX CS1-0 */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS2_0                                            0xc28c28c2U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS2_1                                            0x00008c28U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS3_0                                            0xea2ea2eaU   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS3_1                                            0x00002ea2U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS4_0                                            0x03903903U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS4_1                                            0x00009039U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS5_0                                            0x2b32b32bU   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS5_1                                            0x000032b3U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS6_0                                            0x44944944U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS6_1                                            0x00009449U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS7_0                                            0x6c36c36cU   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS7_1                                            0x000036c3U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS8_0                                            0x85985985U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS8_1                                            0x00009859U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS9_0                                            0xad3ad3adU   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS9_1                                            0x00003ad3U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS10_0                                           0xc69c69c6U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS10_1                                           0x00009c69U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS11_0                                           0xee3ee3eeU   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS11_1                                           0x00003ee3U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS12_0                                           0x07a07a07U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS12_1                                           0x0000a07aU   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS13_0                                           0x2f42f42fU   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS13_1                                           0x000042f4U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS14_0                                           0x48a48a48U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS14_1                                           0x0000a48aU   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS15_0                                           0x70470470U   /* BIT MAP INX */
    #define DDRC_USER_CONFIG_CFG_BIT_MAP_INDEX_CS15_1                                           0x00004704U  /* BIT MAP INDEX CS15-1 */


    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RCD_CFG_RCD                                           0x0000000cU  /* tRCD */
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_DATA_MASK_CFG_DATA_MASK                               0x00000004U  /* tRRDs for diff bank groUps SDRAM ACT to ACT cmd dly */
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_RRD_S_CFG_RRD_S                                       0x00000004U  /* tRRDs for same bank groUps SDRAM ACT to ACT cmd dly */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RRD_CFG_RRD                                           0x00000007U  /* tRRD */
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_RRD_DLR_CFG_RRD_DLR                                   0x00000004U  /*cfg rrd dlr SDRAM ACTIVATE-to-ACTIVATE command period */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RP_CFG_RP                                             0x0000000cU
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RC_CFG_RC                                             0x00000032U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_FAW_CFG_FAW                                           0x0000001bU
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_FAW_DLR_CFG_FAW_DLR                                   0x00000010U
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_RFC1_CFG_RFC1                                         0x00000036U   /* SDRAM tRFC1 */
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_RFC2_CFG_RFC2                                         0x00000036U   /* SDRAM tRFC2 */
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_RFC4_CFG_RFC4                                         0x00000036U   /* SDRAM tRFC4 */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RFC_CFG_RFC                                           0x00000076U

    #define DDRC_USER_CONFIG_MC_BASE1_CFG_RFC_DLR1_CFG_RFC_DLR1                                 0x00000048U   /* tRFC_dlr1 */
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_RFC_DLR2_CFG_RFC_DLR2                                 0x0000002cU   /* tRFC_dlr1 */
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_RFC_DLR4_CFG_RFC_DLR4                                 0x00000020U   /* tRFC_dlr1 */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RTP_CFG_RTP                                           0x00000008U   /* tRTP */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RAS_CFG_RAS                                           0x00000026U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_WR_CFG_WR                                             0x00000010U
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_WR_CRC_DM_CFG_WR_CRC_DM                               0x00000006U   /* SDRAM tWR_CRC_DM */
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_WTR_S_CFG_WTR_S                                       0x00000003U   /*SDRAM write to read command delay for diï¬€erent bank groUp (tWTR */
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_WTR_L_CFG_WTR_L                                       0x00000003U   /*SDRAM write to read command delay for same bank groUp (tWTR L) */

    #define DDRC_USER_CONFIG_MC_BASE1_CFG_WTR_S_CRC_DM_CFG_WTR_S_CRC_DM                         0x00000003U
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_WTR_L_CRC_DM_CFG_WTR_L_CRC_DM                         0x00000003U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_WTR_CFG_WTR                                           0x00000008U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_WRITE_TO_READ_CFG_WRITE_TO_READ                       0x00000008U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_READ_TO_WRITE_CFG_READ_TO_WRITE                       0x00000003U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_WRITE_TO_WRITE_CFG_WRITE_TO_WRITE                     0x00000003U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_READ_TO_READ_CFG_READ_TO_READ                         0x00000003U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_LOOKAHEAD_ACT_CFG_LOOKAHEAD_ACT                       0x00000001U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_LOOKAHEAD_PCH_CFG_LOOKAHEAD_PCH                       0x00000001U


    #define DDRC_USER_CONFIG_MC_BASE2_CFG_TWO_T_CFG_TWO_T                                       0x00000000U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_TWO_T_SEL_CYCLE_CFG_TWO_T_SEL_CYCLE                   0x00000000U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_MRD_CFG_MRD                                           0x00000010U  /*tMRD */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_MOD_CFG_MOD                                           0x00000010U  /*tMOD */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_MRW_CFG_MRW                                           0x00000010U  /*tMRW */
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_CCD_S_CFG_CCD_S                                       0x00000005U  /* SDRAM tCCD_s */
    #define DDRC_USER_CONFIG_MC_BASE1_CFG_CCD_L_CFG_CCD_L                                       0x00000006U  /* SDRAM tCCD_l */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_XSR_CFG_XSR                                           0x00000036U

    #define DDRC_USER_CONFIG_MC_BASE2_CFG_XP_CFG_XP                                             0x00000003U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_RL_CFG_RL                                             0x0000000cU     /*cfg rl Read latency, speciï¬ed in clocks */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_CL_CFG_CL                                             0x0000000cU     /*cfg cl (CAS REad latency) */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_CWL_CFG_CWL                                           0x00000008U     /*cfg cwl SDRAM CAS write latency, speciï¬ed in clocks */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_WL_CFG_WL                                             0x00000009U     /*cfg wl Write latency, speciï¬ed in clocks_ */


    #define DDRC_USER_CONFIG_MC_BASE2_CFG_AL_MODE_CFG_AL_MODE                                   0x00000000U  	/*cfg al mode Additive latency mode */
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_BL_CFG_BL                                             0x00000000U
    #define DDRC_USER_CONFIG_MC_BASE2_CFG_BL_MODE_CFG_BL_MODE                                   0x00000000U 	/* chng /*cfg bl mode SDRAM burst length mode,cfg bl mode     0x0 - Fi */
    #define DDRC_USER_CONFIG_REORDER_CFG_INTRAPORT_REORDER_EN_CFG_INTRAPORT_REORDER_EN          0x00000000U		/*cfg intraport reorder en Enable intra-port reordering,0 - Disabled */

#endif /* HW_CFG_DDR_H_ */


