/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * This file contains the type definitions for the GEM Ethernet MAC as
 * implemented for the PolarFire SoC. This also covers the subset implemented for
 * the FU540 on the Aloe board with the provisio that many of the registers will
 * not be present on that device.
 *
 * We use separate MAC and eMAC definitions even though the eMAC is a subset
 * of the MAC as it helps to catch errors if we try to program registers not
 * present on the eMAC.
 *
 * SVN $Revision$
 * SVN $Date$
 */
#ifndef MSS_ETHERNET_MAC_REGISTERS_H_
#define MSS_ETHERNET_MAC_REGISTERS_H_

#ifdef __cplusplus
extern "C" {
#endif
/*----------------------------------------------------------------------------*/
/*----------------------------------- MAC -----------------------------------*/
/*----------------------------------------------------------------------------*/
#define __I  const volatile
#define __O  volatile
#define __IO volatile

typedef struct
{
    __IO uint32_t  NETWORK_CONTROL;                     /* 0x0000 */
    __IO uint32_t  NETWORK_CONFIG;                      /* 0x0004 */
    __I  uint32_t  NETWORK_STATUS;                      /* 0x0008 */
    __IO uint32_t  USER_IO;                             /* 0x000C */
    __IO uint32_t  DMA_CONFIG;                          /* 0x0010 */
    __IO uint32_t  TRANSMIT_STATUS;                     /* 0x0014 */
    __IO uint32_t  RECEIVE_Q_PTR;                       /* 0x0018 */
    __IO uint32_t  TRANSMIT_Q_PTR;                      /* 0x001C */
    __IO uint32_t  RECEIVE_STATUS;                      /* 0x0020 */
    __IO uint32_t  INT_STATUS;                          /* 0x0024 */
    __IO uint32_t  INT_ENABLE;                          /* 0x0028 */
    __IO uint32_t  INT_DISABLE;                         /* 0x002C */
    __IO uint32_t  INT_MASK;                            /* 0x0030 */
    __IO uint32_t  PHY_MANAGEMENT;                      /* 0x0034 */
    __IO uint32_t  PAUSE_TIME;                          /* 0x0038 */
    __IO uint32_t  TX_PAUSE_QUANTUM;                    /* 0x003C */
    __IO uint32_t  PBUF_TXCUTTHRU;                      /* 0x0040 */
    __IO uint32_t  PBUF_RXCUTTHRU;                      /* 0x0044 */
    __IO uint32_t  JUMBO_MAX_LENGTH;                    /* 0x0048 */
         uint32_t  reserved1;                           /* 0x004C */
         uint32_t  reserved2;                           /* 0x0050 */
    __IO uint32_t  AXI_MAX_PIPELINE;                    /* 0x0054 */
         uint32_t  reserved3;                           /* 0x0058 */
    __IO uint32_t  INT_MODERATION;                      /* 0x005C */
    __IO uint32_t  SYS_WAKE_TIME;                       /* 0x0060 */
    __IO uint32_t  FATAL_OR_NON_FATAL_INT_SEL;          /* 0x0064 */
    __IO uint32_t  LOCKUP_CONFIG;                       /* 0x0068 */
    __IO uint32_t  RX_MAC_LOCKUP_TIME;                  /* 0x006C */
         uint32_t  reserved4;                           /* 0x0070 */
         uint32_t  reserved5;                           /* 0x0074 */
         uint32_t  reserved6;                           /* 0x0078 */
         uint32_t  reserved7;                           /* 0x007C */
    __IO uint32_t  HASH_BOTTOM;                         /* 0x0080 */
    __IO uint32_t  HASH_TOP;                            /* 0x0084 */
    __IO uint32_t  SPEC_ADD1_BOTTOM;                    /* 0x0088 */
    __IO uint32_t  SPEC_ADD1_TOP;                       /* 0x008C */
    __IO uint32_t  SPEC_ADD2_BOTTOM;                    /* 0x0090 */
    __IO uint32_t  SPEC_ADD2_TOP;                       /* 0x0094 */
    __IO uint32_t  SPEC_ADD3_BOTTOM;                    /* 0x0098 */
    __IO uint32_t  SPEC_ADD3_TOP;                       /* 0x009C */
    __IO uint32_t  SPEC_ADD4_BOTTOM;                    /* 0x00A0 */
    __IO uint32_t  SPEC_ADD4_TOP;                       /* 0x00A4 */
    __IO uint32_t  SPEC_TYPE1;                          /* 0x00A8 */
    __IO uint32_t  SPEC_TYPE2;                          /* 0x00AC */
    __IO uint32_t  SPEC_TYPE3;                          /* 0x00B0 */
    __IO uint32_t  SPEC_TYPE4;                          /* 0x00B4 */
    __IO uint32_t  WOL_REGISTER;                        /* 0x00B8 */
    __IO uint32_t  STRETCH_RATIO;                       /* 0x00BC */
    __IO uint32_t  STACKED_VLAN;                        /* 0x00C0 */
    __IO uint32_t  TX_PFC_PAUSE;                        /* 0x00C4 */
    __IO uint32_t  MASK_ADD1_BOTTOM;                    /* 0x00C8 */
    __IO uint32_t  MASK_ADD1_TOP;                       /* 0x00CC */
    __IO uint32_t  DMA_ADDR_OR_MASK;                    /* 0x00D0 */
    __IO uint32_t  RX_PTP_UNICAST;                      /* 0x00D4 */
    __IO uint32_t  TX_PTP_UNICAST;                      /* 0x00D8 */
    __IO uint32_t  TSU_NSEC_CMP;                        /* 0x00DC */
    __IO uint32_t  TSU_SEC_CMP;                         /* 0x00E0 */
    __IO uint32_t  TSU_MSB_SEC_CMP;                     /* 0x00E4 */
    __IO uint32_t  TSU_PTP_TX_MSB_SEC_CMP;              /* 0x00E8 */
    __IO uint32_t  TSU_PTP_RX_MSB_SEC_CMP;              /* 0x00EC */
    __IO uint32_t  TSU_PEER_TX_MSB_SEC_CMP;             /* 0x00F0 */
    __IO uint32_t  TSU_PEER_RX_MSB_SEC_CMP;             /* 0x00F4 */
    __IO uint32_t  DPRAM_FILL_DBG;                      /* 0x00F8 */
    __IO uint32_t  REVISION_REG;                        /* 0x00FC */
    __IO uint32_t  OCTETS_TXED_BOTTOM;                  /* 0x0100 */
    __IO uint32_t  OCTETS_TXED_TOP;                     /* 0x0104 */
    __IO uint32_t  FRAMES_TXED_OK;                      /* 0x0108 */
    __IO uint32_t  BROADCAST_TXED;                      /* 0x010C */
    __IO uint32_t  MULTICAST_TXED;                      /* 0x0110 */
    __IO uint32_t  PAUSE_FRAMES_TXED;                   /* 0x0114 */
    __IO uint32_t  FRAMES_TXED_64;                      /* 0x0118 */
    __IO uint32_t  FRAMES_TXED_65;                      /* 0x011C */
    __IO uint32_t  FRAMES_TXED_128;                     /* 0x0120 */
    __IO uint32_t  FRAMES_TXED_256;                     /* 0x0124 */
    __IO uint32_t  FRAMES_TXED_512;                     /* 0x0128 */
    __IO uint32_t  FRAMES_TXED_1024;                    /* 0x012C */
    __IO uint32_t  FRAMES_TXED_1519;                    /* 0x0130 */
    __IO uint32_t  TX_UNDERRUNS;                        /* 0x0134 */
    __IO uint32_t  SINGLE_COLLISIONS;                   /* 0x0138 */
    __IO uint32_t  MULTIPLE_COLLISIONS;                 /* 0x013C */
    __IO uint32_t  EXCESSIVE_COLLISIONS;                /* 0x0140 */
    __IO uint32_t  LATE_COLLISIONS;                     /* 0x0144 */
    __IO uint32_t  DEFERRED_FRAMES;                     /* 0x0148 */
    __IO uint32_t  CRS_ERRORS;                          /* 0x014C */
    __IO uint32_t  OCTETS_RXED_BOTTOM;                  /* 0x0150 */
    __IO uint32_t  OCTETS_RXED_TOP;                     /* 0x0154 */
    __IO uint32_t  FRAMES_RXED_OK;                      /* 0x0158 */
    __IO uint32_t  BROADCAST_RXED;                      /* 0x015C */
    __IO uint32_t  MULTICAST_RXED;                      /* 0x0160 */
    __IO uint32_t  PAUSE_FRAMES_RXED;                   /* 0x0164 */
    __IO uint32_t  FRAMES_RXED_64;                      /* 0x0168 */
    __IO uint32_t  FRAMES_RXED_65;                      /* 0x016C */
    __IO uint32_t  FRAMES_RXED_128;                     /* 0x0170 */
    __IO uint32_t  FRAMES_RXED_256;                     /* 0x0174 */
    __IO uint32_t  FRAMES_RXED_512;                     /* 0x0178 */
    __IO uint32_t  FRAMES_RXED_1024;                    /* 0x017C */
    __IO uint32_t  FRAMES_RXED_1519;                    /* 0x0180 */
    __IO uint32_t  UNDERSIZE_FRAMES;                    /* 0x0184 */
    __IO uint32_t  EXCESSIVE_RX_LENGTH;                 /* 0x0188 */
    __IO uint32_t  RX_JABBERS;                          /* 0x018C */
    __IO uint32_t  FCS_ERRORS;                          /* 0x0190 */
    __IO uint32_t  RX_LENGTH_ERRORS;                    /* 0x0194 */
    __IO uint32_t  RX_SYMBOL_ERRORS;                    /* 0x0198 */
    __IO uint32_t  ALIGNMENT_ERRORS;                    /* 0x019C */
    __IO uint32_t  RX_RESOURCE_ERRORS;                  /* 0x01A0 */
    __IO uint32_t  RX_OVERRUNS;                         /* 0x01A4 */
    __IO uint32_t  RX_IP_CK_ERRORS;                     /* 0x01A8 */
    __IO uint32_t  RX_TCP_CK_ERRORS;                    /* 0x01AC */
    __IO uint32_t  RX_UDP_CK_ERRORS;                    /* 0x01B0 */
    __IO uint32_t  AUTO_FLUSHED_PKTS;                   /* 0x01B4 */
         uint32_t  reserved8;                           /* 0x01B8 */
    __IO uint32_t  TSU_TIMER_INCR_SUB_NSEC;             /* 0x01BC */
    __IO uint32_t  TSU_TIMER_MSB_SEC;                   /* 0x01C0 */
    __IO uint32_t  TSU_STROBE_MSB_SEC;                  /* 0x01C4 */
    __IO uint32_t  TSU_STROBE_SEC;                      /* 0x01C8 */
    __IO uint32_t  TSU_STROBE_NSEC;                     /* 0x01CC */
    __IO uint32_t  TSU_TIMER_SEC;                       /* 0x01D0 */
    __IO uint32_t  TSU_TIMER_NSEC;                      /* 0x01D4 */
    __IO uint32_t  TSU_TIMER_ADJUST;                    /* 0x01D8 */
    __IO uint32_t  TSU_TIMER_INCR;                      /* 0x01DC */
    __IO uint32_t  TSU_PTP_TX_SEC;                      /* 0x01E0 */
    __IO uint32_t  TSU_PTP_TX_NSEC;                     /* 0x01E4 */
    __IO uint32_t  TSU_PTP_RX_SEC;                      /* 0x01E8 */
    __IO uint32_t  TSU_PTP_RX_NSEC;                     /* 0x01EC */
    __IO uint32_t  TSU_PEER_TX_SEC;                     /* 0x01F0 */
    __IO uint32_t  TSU_PEER_TX_NSEC;                    /* 0x01F4 */
    __IO uint32_t  TSU_PEER_RX_SEC;                     /* 0x01F8 */
    __IO uint32_t  TSU_PEER_RX_NSEC;                    /* 0x01FC */
    __IO uint32_t  PCS_CONTROL;                         /* 0x0200 */
    __IO uint32_t  PCS_STATUS;                          /* 0x0204 */
    __IO uint32_t  PCS_PHY_TOP_ID;                      /* 0x0208 */
    __IO uint32_t  PCS_PHY_BOT_ID;                      /* 0x020C */
    __IO uint32_t  PCS_AN_ADV;                          /* 0x0210 */
    __IO uint32_t  PCS_AN_LP_BASE;                      /* 0x0214 */
    __IO uint32_t  PCS_AN_EXP;                          /* 0x0218 */
    __IO uint32_t  PCS_AN_NP_TX;                        /* 0x021C */
    __IO uint32_t  PCS_AN_LP_NP;                        /* 0x0220 */
         uint32_t  reserved9[6];                        /* 0x0224 - 0x0238 */
    __IO uint32_t  PCS_AN_EXT_STATUS;                   /* 0x023C */
         uint32_t  reserved10[8];                       /* 0x0240 - 0x025C */
    __IO uint32_t  TX_PAUSE_QUANTUM1;                   /* 0x0260 */
    __IO uint32_t  TX_PAUSE_QUANTUM2;                   /* 0x0264 */
    __IO uint32_t  TX_PAUSE_QUANTUM3;                   /* 0x0268 */
    __IO uint32_t  PFC_STATUS;                          /* 0x026C */
    __IO uint32_t  RX_LPI;                              /* 0x0270 */
    __IO uint32_t  RX_LPI_TIME;                         /* 0x0274 */
    __IO uint32_t  TX_LPI;                              /* 0x0278 */
    __IO uint32_t  TX_LPI_TIME;                         /* 0x027C */
    __IO uint32_t  DESIGNCFG_DEBUG1;                    /* 0x0280 */
    __IO uint32_t  DESIGNCFG_DEBUG2;                    /* 0x0284 */
    __IO uint32_t  DESIGNCFG_DEBUG3;                    /* 0x0288 */
    __IO uint32_t  DESIGNCFG_DEBUG4;                    /* 0x028C */
    __IO uint32_t  DESIGNCFG_DEBUG5;                    /* 0x0290 */
    __IO uint32_t  DESIGNCFG_DEBUG6;                    /* 0x0294 */
    __IO uint32_t  DESIGNCFG_DEBUG7;                    /* 0x0298 */
    __IO uint32_t  DESIGNCFG_DEBUG8;                    /* 0x029C */
    __IO uint32_t  DESIGNCFG_DEBUG9;                    /* 0x02A0 */
    __IO uint32_t  DESIGNCFG_DEBUG10;                   /* 0x02A4 */
    __IO uint32_t  DESIGNCFG_DEBUG11;                   /* 0x02A8 */
    __IO uint32_t  DESIGNCFG_DEBUG12;                   /* 0x02AC */
         uint32_t  reserved11[84];                      /* 0x02B0 - 0x03FC */
    __IO uint32_t  INT_Q1_STATUS;                       /* 0x0400 */
    __IO uint32_t  INT_Q2_STATUS;                       /* 0x0404 */
    __IO uint32_t  INT_Q3_STATUS;                       /* 0x0408 */
         uint32_t  reserved12[13];                      /* 0x040C - 0x043C */
    __IO uint32_t  TRANSMIT_Q1_PTR;                     /* 0x0440 */
    __IO uint32_t  TRANSMIT_Q2_PTR;                     /* 0x0444 */
    __IO uint32_t  TRANSMIT_Q3_PTR;                     /* 0x0448 */
         uint32_t  reserved13[13];                      /* 0x044C - 0x047C */
    __IO uint32_t  RECEIVE_Q1_PTR;                      /* 0x0480 */
    __IO uint32_t  RECEIVE_Q2_PTR;                      /* 0x0484 */
    __IO uint32_t  RECEIVE_Q3_PTR;                      /* 0x0488 */
         uint32_t  reserved14[5];                       /* 0x048C - 0x049C */
    __IO uint32_t  DMA_RXBUF_SIZE_Q1;                   /* 0x04A0 */
    __IO uint32_t  DMA_RXBUF_SIZE_Q2;                   /* 0x04A4 */
    __IO uint32_t  DMA_RXBUF_SIZE_Q3;                   /* 0x04A8 */
         uint32_t  reserved15[4];                       /* 0x04AC - 0x04B8 */
    __IO uint32_t  CBS_CONTROL;                         /* 0x04BC */
    __IO uint32_t  CBS_IDLESLOPE_Q_A;                   /* 0x04C0 */
    __IO uint32_t  CBS_IDLESLOPE_Q_B;                   /* 0x04C4 */
    __IO uint32_t  UPPER_TX_Q_BASE_ADDR;                /* 0x04C8 */
    __IO uint32_t  TX_BD_CONTROL;                       /* 0x04CC */
    __IO uint32_t  RX_BD_CONTROL;                       /* 0x04D0 */
    __IO uint32_t  UPPER_RX_Q_BASE_ADDR;                /* 0x04D4 */
         uint32_t  reserved16[5];                       /* 0x04D8 - 0x04E8 */
    __IO uint32_t  WD_COUNTER;                          /* 0x04EC */
         uint32_t  reserved17[2];                       /* 0x04F0 - 0x04F4 */
    __IO uint32_t  AXI_TX_FULL_THRESH0;                 /* 0x04F8 */
    __IO uint32_t  AXI_TX_FULL_THRESH1;                 /* 0x04FC */
    __IO uint32_t  SCREENING_TYPE_1_REGISTER_0;         /* 0x0500 */
    __IO uint32_t  SCREENING_TYPE_1_REGISTER_1;         /* 0x0504 */
    __IO uint32_t  SCREENING_TYPE_1_REGISTER_2;         /* 0x0508 */
    __IO uint32_t  SCREENING_TYPE_1_REGISTER_3;         /* 0x050C */
         uint32_t  reserved18[12];                      /* 0x0510 - 0x053C */
    __IO uint32_t  SCREENING_TYPE_2_REGISTER_0;         /* 0x0540 */
    __IO uint32_t  SCREENING_TYPE_2_REGISTER_1;         /* 0x0544 */
    __IO uint32_t  SCREENING_TYPE_2_REGISTER_2;         /* 0x0548 */
    __IO uint32_t  SCREENING_TYPE_2_REGISTER_3;         /* 0x054C */
         uint32_t  reserved18b[12];                     /* 0x0550 - 0x057C */
    __IO uint32_t  TX_SCHED_CTRL;                       /* 0x0580 */
         uint32_t  reserved19[3];                       /* 0x0584 - 0x058C */
    __IO uint32_t  BW_RATE_LIMIT_Q0TO3;                 /* 0x0590 */
         uint32_t  reserved20[3];                       /* 0x0594 - 0x059C */
    __IO uint32_t  TX_Q_SEG_ALLOC_Q0TO3;                /* 0x05A0 */
         uint32_t  reserved21[23];                      /* 0x05A4 - 0x05FC */
    __IO uint32_t  INT_Q1_ENABLE;                       /* 0x0600 */
    __IO uint32_t  INT_Q2_ENABLE;                       /* 0x0604 */
    __IO uint32_t  INT_Q3_ENABLE;                       /* 0x0608 */
         uint32_t  reserved22[5];                       /* 0x060C - 0x061C */
    __IO uint32_t  INT_Q1_DISABLE;                      /* 0x0620 */
    __IO uint32_t  INT_Q2_DISABLE;                      /* 0x0624 */
    __IO uint32_t  INT_Q3_DISABLE;                      /* 0x0628 */
         uint32_t  reserved23[5];                       /* 0x062C - 0x063C */
    __IO uint32_t  INT_Q1_MASK;                         /* 0x0640 */
    __IO uint32_t  INT_Q2_MASK;                         /* 0x0644 */
    __IO uint32_t  INT_Q3_MASK;                         /* 0x0648 */
         uint32_t  reserved24[37];                      /* 0x064C - 0x06DC */
    __IO uint32_t  SCREENING_TYPE_2_ETHERTYPE_REG_0;    /* 0x06E0 */
    __IO uint32_t  SCREENING_TYPE_2_ETHERTYPE_REG_1;    /* 0x06E4 */
    __IO uint32_t  SCREENING_TYPE_2_ETHERTYPE_REG_2;    /* 0x06E8 */
    __IO uint32_t  SCREENING_TYPE_2_ETHERTYPE_REG_3;    /* 0x06EC */
         uint32_t  reserved25[4];                       /* 0x06F0 - 0x06FC */
    __IO uint32_t  TYPE2_COMPARE_0_WORD_0;              /* 0x0700 */
    __IO uint32_t  TYPE2_COMPARE_0_WORD_1;              /* 0x0704 */
    __IO uint32_t  TYPE2_COMPARE_1_WORD_0;              /* 0x0708 */
    __IO uint32_t  TYPE2_COMPARE_1_WORD_1;              /* 0x070C */
    __IO uint32_t  TYPE2_COMPARE_2_WORD_0;              /* 0x0710 */
    __IO uint32_t  TYPE2_COMPARE_2_WORD_1;              /* 0x0714 */
    __IO uint32_t  TYPE2_COMPARE_3_WORD_0;              /* 0x0718 */
    __IO uint32_t  TYPE2_COMPARE_3_WORD_1;              /* 0x071C */
         uint32_t  reserved26[56];                      /* 0x0720 - 0x07FC */
    __IO uint32_t  ENST_START_TIME_Q0;                  /* 0x0800 */
    __IO uint32_t  ENST_START_TIME_Q1;                  /* 0x0804 */
    __IO uint32_t  ENST_START_TIME_Q2;                  /* 0x0808 */
    __IO uint32_t  ENST_START_TIME_Q3;                  /* 0x080C */
         uint32_t  reserved27[4];                       /* 0x0810 - 0x081C */
    __IO uint32_t  ENST_ON_TIME_Q0;                     /* 0x0820 */
    __IO uint32_t  ENST_ON_TIME_Q1;                     /* 0x0824 */
    __IO uint32_t  ENST_ON_TIME_Q2;                     /* 0x0828 */
    __IO uint32_t  ENST_ON_TIME_Q3;                     /* 0x082C */
         uint32_t  reserved28[4];                       /* 0x0830 - 0x083C */
    __IO uint32_t  ENST_OFF_TIME_Q0;                    /* 0x0840 */
    __IO uint32_t  ENST_OFF_TIME_Q1;                    /* 0x0844 */
    __IO uint32_t  ENST_OFF_TIME_Q2;                    /* 0x0848 */
    __IO uint32_t  ENST_OFF_TIME_Q3;                    /* 0x084C */
    __IO uint32_t  ENST_CONTROL;                        /* 0x0850 */
         uint32_t  reserved29[427];                     /* 0x0854 - 0x0EFC */
    __IO uint32_t  MMSL_CONTROL;                        /* 0x0F00 */
    __IO uint32_t  MMSL_STATUS;                         /* 0x0F04 */
    __IO uint32_t  MMSL_ERR_STATS;                      /* 0x0F08 */
    __IO uint32_t  MMSL_ASS_OK_COUNT;                   /* 0x0F0C */
    __IO uint32_t  MMSL_FRAG_COUNT_RX;                  /* 0x0F10 */
    __IO uint32_t  MMSL_FRAG_COUNT_TX;                  /* 0x0F14 */
    __IO uint32_t  MMSL_INT_STATUS;                     /* 0x0F18 */
    __IO uint32_t  MMSL_INT_ENABLE;                     /* 0x0F1C */
    __IO uint32_t  MMSL_INT_DISABLE;                    /* 0x0F20 */
    __IO uint32_t  MMSL_INT_MASK;                       /* 0x0F24 */
         uint32_t  reserved30[54];                      /* 0x0F28 - 0x0FFC */
} MAC_TypeDef;

typedef struct
{
    __IO uint32_t  NETWORK_CONTROL;                /* 0x1000 */
    __IO uint32_t  NETWORK_CONFIG;                 /* 0x1004 */
    __IO uint32_t  NETWORK_STATUS;                 /* 0x1008 */
         uint32_t  reserved31;                     /* 0x100C */
    __IO uint32_t  DMA_CONFIG;                     /* 0x1010 */
    __IO uint32_t  TRANSMIT_STATUS;                /* 0x1014 */
    __IO uint32_t  RECEIVE_Q_PTR;                  /* 0x1018 */
    __IO uint32_t  TRANSMIT_Q_PTR;                 /* 0x101C */
    __IO uint32_t  RECEIVE_STATUS;                 /* 0x1020 */
    __IO uint32_t  INT_STATUS;                     /* 0x1024 */
    __IO uint32_t  INT_ENABLE;                     /* 0x1028 */
    __IO uint32_t  INT_DISABLE;                    /* 0x102C */
    __IO uint32_t  INT_MASK;                       /* 0x1030 */
    __IO uint32_t  PHY_MANAGEMENT;                 /* 0x1034 */
    __IO uint32_t  PAUSE_TIME;                     /* 0x1038 */
    __IO uint32_t  TX_PAUSE_QUANTUM;               /* 0x103C */
    __IO uint32_t  PBUF_TXCUTTHRU;                 /* 0x1040 */
    __IO uint32_t  PBUF_RXCUTTHRU;                 /* 0x1044 */
    __IO uint32_t  JUMBO_MAX_LENGTH;               /* 0x1048 */
         uint32_t  reserved32[2];                  /* 0x104C - 0x1050 */
    __IO uint32_t  AXI_MAX_PIPELINE;               /* 0x1054 */
         uint32_t  reserved33;                     /* 0x1058 */
    __IO uint32_t  INT_MODERATION;                 /* 0x105C */
    __IO uint32_t  SYS_WAKE_TIME;                  /* 0x1060 */
    __IO uint32_t  FATAL_OR_NON_FATAL_INT_SEL;     /* 0x1064 */
    __IO uint32_t  LOCKUP_CONFIG;                  /* 0x1068 */
    __IO uint32_t  RX_MAC_LOCKUP_TIME;             /* 0x106C */
         uint32_t  reserved34[4];                  /* 0x1070 - 0x107C */
    __IO uint32_t  HASH_BOTTOM;                    /* 0x1080 */
    __IO uint32_t  HASH_TOP;                       /* 0x1084 */
    __IO uint32_t  SPEC_ADD1_BOTTOM;               /* 0x1088 */
    __IO uint32_t  SPEC_ADD1_TOP;                  /* 0x108C */
    __IO uint32_t  SPEC_ADD2_BOTTOM;               /* 0x1090 */
    __IO uint32_t  SPEC_ADD2_TOP;                  /* 0x1094 */
    __IO uint32_t  SPEC_ADD3_BOTTOM;               /* 0x1098 */
    __IO uint32_t  SPEC_ADD3_TOP;                  /* 0x109C */
    __IO uint32_t  SPEC_ADD4_BOTTOM;               /* 0x10A0 */
    __IO uint32_t  SPEC_ADD4_TOP;                  /* 0x10A4 */
    __IO uint32_t  SPEC_TYPE1;                     /* 0x10A8 */
    __IO uint32_t  SPEC_TYPE2;                     /* 0x10AC */
    __IO uint32_t  SPEC_TYPE3;                     /* 0x10B0 */
    __IO uint32_t  SPEC_TYPE4;                     /* 0x10B4 */
    __IO uint32_t  WOL_REGISTER;                   /* 0x10B8 */
    __IO uint32_t  STRETCH_RATIO;                  /* 0x10BC */
    __IO uint32_t  STACKED_VLAN;                   /* 0x10C0 */
    __IO uint32_t  TX_PFC_PAUSE;                   /* 0x10C4 */
    __IO uint32_t  MASK_ADD1_BOTTOM;               /* 0x10C8 */
    __IO uint32_t  MASK_ADD1_TOP;                  /* 0x10CC */
    __IO uint32_t  DMA_ADDR_OR_MASK;               /* 0x10D0 */
    __IO uint32_t  RX_PTP_UNICAST;                 /* 0x10D4 */
    __IO uint32_t  TX_PTP_UNICAST;                 /* 0x10D8 */
    __IO uint32_t  TSU_NSEC_CMP;                   /* 0x10DC */
    __IO uint32_t  TSU_SEC_CMP;                    /* 0x10E0 */
    __IO uint32_t  TSU_MSB_SEC_CMP;                /* 0x10E4 */
    __IO uint32_t  TSU_PTP_TX_MSB_SEC;             /* 0x10E8 */
    __IO uint32_t  TSU_PTP_RX_MSB_SEC;             /* 0x10EC */
    __IO uint32_t  TSU_PEER_TX_MSB_SEC;            /* 0x10F0 */
    __IO uint32_t  TSU_PEER_RX_MSB_SEC;            /* 0x10F4 */
    __IO uint32_t  DPRAM_FILL_DBG;                 /* 0x10F8 */
    __IO uint32_t  REVISION_REG;                   /* 0x10FC */
    __IO uint32_t  OCTETS_TXED_BOTTOM;             /* 0x1100 */
    __IO uint32_t  OCTETS_TXED_TOP;                /* 0x1104 */
    __IO uint32_t  FRAMES_TXED_OK;                 /* 0x1108 */
    __IO uint32_t  BROADCAST_TXED;                 /* 0x110C */
    __IO uint32_t  MULTICAST_TXED;                 /* 0x1110 */
    __IO uint32_t  PAUSE_FRAMES_TXED;              /* 0x1114 */
    __IO uint32_t  FRAMES_TXED_64;                 /* 0x1118 */
    __IO uint32_t  FRAMES_TXED_65;                 /* 0x111C */
    __IO uint32_t  FRAMES_TXED_128;                /* 0x1120 */
    __IO uint32_t  FRAMES_TXED_256;                /* 0x1124 */
    __IO uint32_t  FRAMES_TXED_512;                /* 0x1128 */
    __IO uint32_t  FRAMES_TXED_1024;               /* 0x112C */
    __IO uint32_t  FRAMES_TXED_1519;               /* 0x1130 */
    __IO uint32_t  TX_UNDERRUNS;                   /* 0x1134 */
    __IO uint32_t  SINGLE_COLLISIONS;              /* 0x1138 */
    __IO uint32_t  MULTIPLE_COLLISIONS;            /* 0x113C */
    __IO uint32_t  EXCESSIVE_COLLISIONS;           /* 0x1140 */
    __IO uint32_t  LATE_COLLISIONS;                /* 0x1144 */
    __IO uint32_t  DEFERRED_FRAMES;                /* 0x1148 */
    __IO uint32_t  CRS_ERRORS;                     /* 0x114C */
    __IO uint32_t  OCTETS_RXED_BOTTOM;             /* 0x1150 */
    __IO uint32_t  OCTETS_RXED_TOP;                /* 0x1154 */
    __IO uint32_t  FRAMES_RXED_OK;                 /* 0x1158 */
    __IO uint32_t  BROADCAST_RXED;                 /* 0x115C */
    __IO uint32_t  MULTICAST_RXED;                 /* 0x1160 */
    __IO uint32_t  PAUSE_FRAMES_RXED;              /* 0x1164 */
    __IO uint32_t  FRAMES_RXED_64;                 /* 0x1168 */
    __IO uint32_t  FRAMES_RXED_65;                 /* 0x116C */
    __IO uint32_t  FRAMES_RXED_128;                /* 0x1170 */
    __IO uint32_t  FRAMES_RXED_256;                /* 0x1174 */
    __IO uint32_t  FRAMES_RXED_512;                /* 0x1178 */
    __IO uint32_t  FRAMES_RXED_1024;               /* 0x117C */
    __IO uint32_t  FRAMES_RXED_1519;               /* 0x1180 */
    __IO uint32_t  UNDERSIZE_FRAMES;               /* 0x1184 */
    __IO uint32_t  EXCESSIVE_RX_LENGTH;            /* 0x1188 */
    __IO uint32_t  RX_JABBERS;                     /* 0x118C */
    __IO uint32_t  FCS_ERRORS;                     /* 0x1190 */
    __IO uint32_t  RX_LENGTH_ERRORS;               /* 0x1194 */
    __IO uint32_t  RX_SYMBOL_ERRORS;               /* 0x1198 */
    __IO uint32_t  ALIGNMENT_ERRORS;               /* 0x119C */
    __IO uint32_t  RX_RESOURCE_ERRORS;             /* 0x11A0 */
    __IO uint32_t  RX_OVERRUNS;                    /* 0x11A4 */
    __IO uint32_t  RX_IP_CK_ERRORS;                /* 0x11A8 */
    __IO uint32_t  RX_TCP_CK_ERRORS;               /* 0x11AC */
    __IO uint32_t  RX_UDP_CK_ERRORS;               /* 0x11B0 */
    __IO uint32_t  AUTO_FLUSHED_PKTS;              /* 0x11B4 */
         uint32_t  reserved35;                     /* 0x10B8 */
    __IO uint32_t  TSU_TIMER_INCR_SUB_NSEC;        /* 0x11BC */
    __IO uint32_t  TSU_TIMER_MSB_SEC;              /* 0x11C0 */
    __IO uint32_t  TSU_STROBE_MSB_SEC;             /* 0x11C4 */
    __IO uint32_t  TSU_STROBE_SEC;                 /* 0x11C8 */
    __IO uint32_t  TSU_STROBE_NSEC;                /* 0x11CC */
    __IO uint32_t  TSU_TIMER_SEC;                  /* 0x11D0 */
    __IO uint32_t  TSU_TIMER_NSEC;                 /* 0x11D4 */
    __IO uint32_t  TSU_TIMER_ADJUST;               /* 0x11D8 */
    __IO uint32_t  TSU_TIMER_INCR;                 /* 0x11DC */
    __IO uint32_t  TSU_PTP_TX_SEC;                 /* 0x11E0 */
    __IO uint32_t  TSU_PTP_TX_NSEC;                /* 0x11E4 */
    __IO uint32_t  TSU_PTP_RX_SEC;                 /* 0x11E8 */
    __IO uint32_t  TSU_PTP_RX_NSEC;                /* 0x11EC */
    __IO uint32_t  TSU_PEER_TX_SEC;                /* 0x11F0 */
    __IO uint32_t  TSU_PEER_TX_NSEC;               /* 0x11F4 */
    __IO uint32_t  TSU_PEER_RX_SEC;                /* 0x11F8 */
    __IO uint32_t  TSU_PEER_RX_NSEC;               /* 0x11FC */
         uint32_t  reserved36[24];                 /* 0x1200 - 0x125C */
    __IO uint32_t  TX_PAUSE_QUANTUM1;              /* 0x1260 */
    __IO uint32_t  TX_PAUSE_QUANTUM2;              /* 0x1264 */
    __IO uint32_t  TX_PAUSE_QUANTUM3;              /* 0x1268 */
    __IO uint32_t  PFC_STATUS;                     /* 0x126C */
    __IO uint32_t  RX_LPI;                         /* 0x1270 */
    __IO uint32_t  RX_LPI_TIME;                    /* 0x1274 */
    __IO uint32_t  TX_LPI;                         /* 0x1278 */
    __IO uint32_t  TX_LPI_TIME;                    /* 0x127C */
    __IO uint32_t  DESIGNCFG_DEBUG1;               /* 0x1280 */
    __IO uint32_t  DESIGNCFG_DEBUG2;               /* 0x1284 */
    __IO uint32_t  DESIGNCFG_DEBUG3;               /* 0x1288 */
    __IO uint32_t  DESIGNCFG_DEBUG4;               /* 0x128C */
    __IO uint32_t  DESIGNCFG_DEBUG5;               /* 0x1290 */
    __IO uint32_t  DESIGNCFG_DEBUG6;               /* 0x1294 */
    __IO uint32_t  DESIGNCFG_DEBUG7;               /* 0x1298 */
    __IO uint32_t  DESIGNCFG_DEBUG8;               /* 0x129C */
    __IO uint32_t  DESIGNCFG_DEBUG9;               /* 0x12A0 */
    __IO uint32_t  DESIGNCFG_DEBUG10;              /* 0x12A4 */
    __IO uint32_t  DESIGNCFG_DEBUG11;              /* 0x12A8 */
    __IO uint32_t  DESIGNCFG_DEBUG12;              /* 0x12AC */
         uint32_t  reserved37[131];                /* 0x12B0 - 0x14B8 */
    __IO uint32_t  CBS_CONTROL;                    /* 0x14BC */
    __IO uint32_t  CBS_IDLESLOPE_Q_A;              /* 0x14C0 */
    __IO uint32_t  CBS_IDLESLOPE_Q_B;              /* 0x14C4 */
    __IO uint32_t  UPPER_TX_Q_BASE_ADDR;           /* 0x14C8 */
    __IO uint32_t  TX_BD_CONTROL;                  /* 0x14CC */
    __IO uint32_t  RX_BD_CONTROL;                  /* 0x14D0 */
    __IO uint32_t  UPPER_RX_Q_BASE_ADDR;           /* 0x14D4 */
         uint32_t  reserved38[10];                 /* 0x14D8 - 0x14FC */
    __IO uint32_t  SCREENING_TYPE_1_REGISTER_0;    /* 0x1500  - TBD PMCS Remove this and look for other possible additional registers near end of eMAC that we might be missing... */
         uint32_t  reserved39[15];                 /* 0x1504 - 0x153C */
    __IO uint32_t  SCREENING_TYPE_2_REGISTER_0;    /* 0x1540 */
    __IO uint32_t  SCREENING_TYPE_2_REGISTER_1;    /* 0x1544 */
         uint32_t  reserved40[110];                /* 0x1548 - 0x16FC */
    __IO uint32_t  TYPE2_COMPARE_0_WORD_0;         /* 0x1700 */
    __IO uint32_t  TYPE2_COMPARE_0_WORD_1;         /* 0x1704 */
    __IO uint32_t  TYPE2_COMPARE_1_WORD_0;         /* 0x1708 */
    __IO uint32_t  TYPE2_COMPARE_1_WORD_1;         /* 0x170C */
    __IO uint32_t  TYPE2_COMPARE_2_WORD_0;         /* 0x1710 */
    __IO uint32_t  TYPE2_COMPARE_2_WORD_1;         /* 0x1714 */
    __IO uint32_t  TYPE2_COMPARE_3_WORD_0;         /* 0x1718 */
    __IO uint32_t  TYPE2_COMPARE_3_WORD_1;         /* 0x171C */
    __IO uint32_t  TYPE2_COMPARE_4_WORD_0;         /* 0x1720 */
    __IO uint32_t  TYPE2_COMPARE_4_WORD_1;         /* 0x1724 */
    __IO uint32_t  TYPE2_COMPARE_5_WORD_0;         /* 0x1728 */
    __IO uint32_t  TYPE2_COMPARE_5_WORD_1;         /* 0x172C */
         uint32_t  reserved41[52];                 /* 0x1730 - 0x17FC */
    __IO uint32_t  ENST_START_TIME;                /* 0x1800 */
         uint32_t  reserved42[7];                  /* 0x1804 - 0x181C */
    __IO uint32_t  ENST_ON_TIME;                   /* 0x1820 */
         uint32_t  reserved43[7];                  /* 0x1824 - 0x183C */
    __IO uint32_t  ENST_OFF_TIME;                  /* 0x1840 */
         uint32_t  reserved44[15];                 /* 0x1844 - 0x187C */
    __IO uint32_t  ENST_CONTROL;                   /* 0x1820 */
} eMAC_TypeDef;

#ifdef __cplusplus
}
#endif

#endif /* MSS_ETHERNET_MAC_REGISTERS_H_ */
