/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Register bit offsets and masks definitions for PolarFire SoC MSS MAC.
 *
 * SVN $Revision$
 * SVN $Date$
 */
#ifndef MSS_ETHERNET_MAC_REGS_H_
#define MSS_ETHERNET_MAC_REGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define BIT_31  ((uint32_t)(0x80000000UL))
#define BIT_30  ((uint32_t)(0x40000000UL))
#define BIT_29  ((uint32_t)(0x20000000UL))
#define BIT_28  ((uint32_t)(0x10000000UL))
#define BIT_27  ((uint32_t)(0x08000000UL))
#define BIT_26  ((uint32_t)(0x04000000UL))
#define BIT_25  ((uint32_t)(0x02000000UL))
#define BIT_24  ((uint32_t)(0x01000000UL))
#define BIT_23  ((uint32_t)(0x00800000UL))
#define BIT_22  ((uint32_t)(0x00400000UL))
#define BIT_21  ((uint32_t)(0x00200000UL))
#define BIT_20  ((uint32_t)(0x00100000UL))
#define BIT_19  ((uint32_t)(0x00080000UL))
#define BIT_18  ((uint32_t)(0x00040000UL))
#define BIT_17  ((uint32_t)(0x00020000UL))
#define BIT_16  ((uint32_t)(0x00010000UL))
#define BIT_15  ((uint32_t)(0x00008000UL))
#define BIT_14  ((uint32_t)(0x00004000UL))
#define BIT_13  ((uint32_t)(0x00002000UL))
#define BIT_12  ((uint32_t)(0x00001000UL))
#define BIT_11  ((uint32_t)(0x00000800UL))
#define BIT_10  ((uint32_t)(0x00000400UL))
#define BIT_09  ((uint32_t)(0x00000200UL))
#define BIT_08  ((uint32_t)(0x00000100UL))
#define BIT_07  ((uint32_t)(0x00000080UL))
#define BIT_06  ((uint32_t)(0x00000040UL))
#define BIT_05  ((uint32_t)(0x00000020UL))
#define BIT_04  ((uint32_t)(0x00000010UL))
#define BIT_03  ((uint32_t)(0x00000008UL))
#define BIT_02  ((uint32_t)(0x00000004UL))
#define BIT_01  ((uint32_t)(0x00000002UL))
#define BIT_00  ((uint32_t)(0x00000001UL))

#define BITS_30 ((uint32_t)(0x3FFFFFFFUL))
#define BITS_24 ((uint32_t)(0x00FFFFFFUL))
#define BITS_22 ((uint32_t)(0x003FFFFFUL))
#define BITS_18 ((uint32_t)(0x0003FFFFUL))
#define BITS_16 ((uint32_t)(0x0000FFFFUL))
#define BITS_15 ((uint32_t)(0x00007FFFUL))
#define BITS_14 ((uint32_t)(0x00003FFFUL))
#define BITS_13 ((uint32_t)(0x00001FFFUL))
#define BITS_12 ((uint32_t)(0x00000FFFUL))
#define BITS_11 ((uint32_t)(0x000007FFUL))
#define BITS_10 ((uint32_t)(0x000003FFUL))
#define BITS_09 ((uint32_t)(0x000001FFUL))
#define BITS_08 ((uint32_t)(0x000000FFUL))
#define BITS_07 ((uint32_t)(0x0000007FUL))
#define BITS_06 ((uint32_t)(0x0000003FUL))
#define BITS_05 ((uint32_t)(0x0000001FUL))
#define BITS_04 ((uint32_t)(0x0000000FUL))
#define BITS_03 ((uint32_t)(0x00000007UL))
#define BITS_02 ((uint32_t)(0x00000003UL))


/*******************************************************************************
 Register Bit definitions
 */

/* General MAC Network Control register bit definitions */
/* eMAC Network Control register bit definitions */

#define GEM_IFG_EATS_QAV_CREDIT                     BIT_30
#define GEM_TWO_PT_FIVE_GIG                         BIT_29
#define GEM_SEL_MII_ON_RGMII                        BIT_28
#define GEM_OSS_CORRECTION_FIELD                    BIT_27
#define GEM_EXT_RXQ_SEL_EN                          BIT_26
#define GEM_PFC_CTRL                                BIT_25
#define GEM_ONE_STEP_SYNC_MODE                      BIT_24
#define GEM_EXT_TSU_PORT_ENABLE                     BIT_23
#define GEM_STORE_UDP_OFFSET                        BIT_22
#define GEM_ALT_SGMII_MODE                          BIT_21
#define GEM_PTP_UNICAST_ENA                         BIT_20
#define GEM_TX_LPI_EN                               BIT_19
#define GEM_FLUSH_RX_PKT_PCLK                       BIT_18
#define GEM_TRANSMIT_PFC_PRIORITY_BASED_PAUSE_FRAME BIT_17
#define GEM_PFC_ENABLE                              BIT_16
#define GEM_STORE_RX_TS                             BIT_15
#define GEM_TX_PAUSE_FRAME_ZERO                     BIT_12
#define GEM_TX_PAUSE_FRAME_REQ                      BIT_11
#define GEM_TRANSMIT_HALT                           BIT_10
#define GEM_TRANSMIT_START                          BIT_09
#define GEM_BACK_PRESSURE                           BIT_08
#define GEM_STATS_WRITE_EN                          BIT_07
#define GEM_INC_ALL_STATS_REGS                      BIT_06
#define GEM_CLEAR_ALL_STATS_REGS                    BIT_05
#define GEM_MAN_PORT_EN                             BIT_04
#define GEM_ENABLE_TRANSMIT                         BIT_03
#define GEM_ENABLE_RECEIVE                          BIT_02
#define GEM_LOOPBACK_LOCAL                          BIT_01
#define GEM_LOOPBACK                                BIT_00

/* General MAC Network Configuration register bit definitions */
/* eMAC Network Configuration register bit definitions */

#define GEM_UNI_DIRECTION_ENABLE                    BIT_31
#define GEM_IGNORE_IPG_RX_ER                        BIT_30
#define GEM_NSP_CHANGE                              BIT_29
#define GEM_IPG_STRETCH_ENABLE                      BIT_28
#define GEM_SGMII_MODE_ENABLE                       BIT_27
#define GEM_IGNORE_RX_FCS                           BIT_26
#define GEM_EN_HALF_DUPLEX_RX                       BIT_25
#define GEM_RECEIVE_CHECKSUM_OFFLOAD_ENABLE         BIT_24
#define GEM_DISABLE_COPY_OF_PAUSE_FRAMES            BIT_23
#define GEM_DATA_BUS_WIDTH                          (BIT_21 | BIT_22)
#define GEM_MDC_CLOCK_DIVISOR                       (BIT_18 | BIT_19 | BIT_20)
#define GEM_FCS_REMOVE                              BIT_17
#define GEM_LENGTH_FIELD_ERROR_FRAME_DISCARD        BIT_16
#define GEM_RECEIVE_BUFFER_OFFSET                   (BIT_14 | BIT_15)
#define GEM_PAUSE_ENABLE                            BIT_13
#define GEM_RETRY_TEST                              BIT_12
#define GEM_PCS_SELECT                              BIT_11
#define GEM_GIGABIT_MODE_ENABLE                     BIT_10
#define GEM_EXTERNAL_ADDRESS_MATCH_ENABLE           BIT_09
#define GEM_RECEIVE_1536_BYTE_FRAMES                BIT_08
#define GEM_UNICAST_HASH_ENABLE                     BIT_07
#define GEM_MULTICAST_HASH_ENABLE                   BIT_06
#define GEM_NO_BROADCAST                            BIT_05
#define GEM_COPY_ALL_FRAMES                         BIT_04
#define GEM_JUMBO_FRAMES                            BIT_03
#define GEM_DISCARD_NON_VLAN_FRAMES                 BIT_02
#define GEM_FULL_DUPLEX                             BIT_01
#define GEM_SPEED                                   BIT_00

#define GEM_DATA_BUS_WIDTH_SHIFT                    21
#define GEM_MDC_CLOCK_DIVISOR_SHIFT                 18
#define GEM_MDC_CLOCK_DIVISOR_MASK                  BITS_03
#define GEM_RECEIVE_BUFFER_OFFSET_SHIFT             14

/* General MAC Network Status register bit definitions */
/* eMAC Network Status register bit definitions */

#define GEM_LPI_INDICATE_PCLK                       BIT_07
#define GEM_PFC_NEGOTIATE_PCLK                      BIT_06
#define GEM_MAC_PAUSE_TX_EN                         BIT_05
#define GEM_MAC_PAUSE_RX_EN                         BIT_04
#define GEM_MAC_FULL_DUPLEX                         BIT_03
#define GEM_MAN_DONE                                BIT_02
#define GEM_MDIO_IN                                 BIT_01
#define GEM_PCS_LINK_STATE                          BIT_00

/* General MAC User IO register bit definitions */

#define GEM_CODEGROUP_BYPASS                        BIT_05
#define GEM_COMMA_BYPASS                            BIT_04
#define GEM_TSU_CLK_SOURCE                          BIT_00

/* General MAC DMA Config register bit definitions */
/* eMAC DMA Config register bit definitions */

#define GEM_DMA_ADDR_BUS_WIDTH_1                    BIT_30
#define GEM_TX_BD_EXTENDED_MODE_EN                  BIT_29
#define GEM_RX_BD_EXTENDED_MODE_EN                  BIT_28
#define GEM_FORCE_MAX_AMBA_BURST_TX                 BIT_26
#define GEM_FORCE_MAX_AMBA_BURST_RX                 BIT_25
#define GEM_FORCE_DISCARD_ON_ERR                    BIT_24
#define GEM_RX_BUF_SIZE                             (BITS_08 << 16)
#define GEM_CRC_ERROR_REPORT                        BIT_13
#define GEM_INFINITE_LAST_DBUF_SIZE_EN              BIT_12
#define GEM_TX_PBUF_TCP_EN                          BIT_11
#define GEM_TX_PBUF_SIZE                            BIT_10
#define GEM_RX_PBUF_SIZE                            (BIT_08 | BIT_09)
#define GEM_ENDIAN_SWAP_PACKET                      BIT_07
#define GEM_ENDIAN_SWAP_MANAGEMENT                  BIT_06
#define GEM_HDR_DATA_SPLITTING_EN                   BIT_05
#define GEM_AMBA_BURST_LENGTH                       BITS_05

#define GEM_RX_BUF_SIZE_SHIFT                       16
#define GEM_RX_PBUF_SIZE_SHIFT                      8

/* General MAC Transmit Status register bit definitions */
/* eMAC Transmit Status register bit definitions */

#define GEM_TX_DMA_LOCKUP_DETECTED                  BIT_10
#define GEM_TX_MAC_LOCKUP_DETECTED                  BIT_09
#define GEM_TX_RESP_NOT_OK                          BIT_08
#define GEM_LATE_COLLISION_OCCURRED                 BIT_07
#define GEM_STAT_TRANSMIT_UNDER_RUN                 BIT_06
#define GEM_STAT_TRANSMIT_COMPLETE                  BIT_05
#define GEM_STAT_AMBA_ERROR                         BIT_04
#define GEM_TRANSMIT_GO                             BIT_03
#define GEM_RETRY_LIMIT_EXCEEDED                    BIT_02
#define GEM_COLLISION_OCCURRED                      BIT_01
#define GEM_USED_BIT_READ                           BIT_00

/* General MAC Receive Queue Pointer register bit definitions */
/* General MAC Receive Queue 1 Pointer register bit definitions */
/* General MAC Receive Queue 2 Pointer register bit definitions */
/* General MAC Receive Queue 3 Pointer register bit definitions */
/* eMAC Receive Queue Pointer register bit definitions */

#define GEM_DMA_RX_Q_PTR                            (~(BIT_00 | BIT_01))
#define GEM_DMA_RX_DIS_Q                            BIT_00

/* General MAC Transmit Queue Pointer register bit definitions */
/* General MAC Transmit Queue 1 Pointer register bit definitions */
/* General MAC Transmit Queue 2 Pointer register bit definitions */
/* General MAC Transmit Queue 3 Pointer register bit definitions */
/* eMAC Transmit Queue Pointer register bit definitions */

#define GEM_DMA_TX_Q_PTR                            (~(BIT_00 | BIT_01))
#define GEM_DMA_TX_DIS_Q                            BIT_00

/* General MAC Receive Status register bit definitions */
/* eMAC Receive Status register bit definitions */

#define GEM_RX_DMA_LOCKUP_DETECTED                  BIT_05
#define GEM_RX_MAC_LOCKUP_DETECTED                  BIT_04
#define GEM_RX_RESP_NOT_OK                          BIT_03
#define GEM_RECEIVE_OVERRUN                         BIT_02
#define GEM_FRAME_RECEIVED                          BIT_01
#define GEM_BUFFER_NOT_AVAILABLE                    BIT_00

/* General MAC Interrupt Status register bit definitions */
/* General MAC Interrupt Enable register bit definitions */
/* General MAC Interrupt Disable register bit definitions */
/* General MAC Interrupt Mask register bit definitions */
/* General MAC Priority Queue 1 Interrupt Status register bit definitions  - b01 to b11 only */
/* General MAC Priority Queue 2 Interrupt Status register bit definitions  - b01 to b11 only */
/* General MAC Priority Queue 3 Interrupt Status register bit definitions  - b01 to b11 only */
/* General MAC Priority Queue 1 Interrupt Enable register bit definitions  - b01 to b11 only */
/* General MAC Priority Queue 2 Interrupt Enable register bit definitions  - b01 to b11 only */
/* General MAC Priority Queue 3 Interrupt Enable register bit definitions  - b01 to b11 only */
/* General MAC Priority Queue 1 Interrupt Disable register bit definitions  - b01 to b11 only */
/* General MAC Priority Queue 2 Interrupt Disable register bit definitions  - b01 to b11 only */
/* General MAC Priority Queue 3 Interrupt Disable register bit definitions  - b01 to b11 only */
/* General MAC Priority Queue 1 Interrupt Mask register bit definitions  - b01 to b11 only */
/* General MAC Priority Queue 2 Interrupt Mask register bit definitions  - b01 to b11 only */
/* General MAC Priority Queue 3 Interrupt Mask register bit definitions  - b01 to b11 only */
/* eMAC Interrupt Status register bit definitions */
/* eMAC Interrupt Enable register bit definitions */
/* eMAC Interrupt Disable register bit definitions */
/* eMAC Interrupt Mask register bit definitions */

#define GEM_TX_LOCKUP_DETECTED                      BIT_31
#define GEM_RX_LOCKUP_DETECTED                      BIT_30
#define GEM_TSU_TIMER_COMPARISON_INTERRUPT          BIT_29
#define GEM_WOL_INTERRUPT                           BIT_28
#define GEM_RX_LPI_INDICATION_STATUS_BIT_CHANGE     BIT_27
#define GEM_TSU_SECONDS_REGISTER_INCREMENT          BIT_26
#define GEM_PTP_PDELAY_RESP_FRAME_TRANSMITTED       BIT_25
#define GEM_PTP_PDELAY_REQ_FRAME_TRANSMITTED        BIT_24
#define GEM_PTP_PDELAY_RESP_FRAME_RECEIVED          BIT_23
#define GEM_PTP_PDELAY_REQ_FRAME_RECEIVED           BIT_22
#define GEM_PTP_SYNC_FRAME_TRANSMITTED              BIT_21
#define GEM_PTP_DELAY_REQ_FRAME_TRANSMITTED         BIT_20
#define GEM_PTP_SYNC_FRAME_RECEIVED                 BIT_19
#define GEM_PTP_DELAY_REQ_FRAME_RECEIVED            BIT_18
#define GEM_PCS_LINK_PARTNER_PAGE_RECEIVED          BIT_17
#define GEM_PCS_AUTO_NEGOTIATION_COMPLETE           BIT_16
#define GEM_EXTERNAL_INTERRUPT                      BIT_15
#define GEM_PAUSE_FRAME_TRANSMITTED                 BIT_14
#define GEM_PAUSE_TIME_ELAPSED                      BIT_13
#define GEM_PAUSE_FRAME_WITH_NON_0_PAUSE_QUANTUM_RX BIT_12
#define GEM_RESP_NOT_OK_INT                         BIT_11
#define GEM_RECEIVE_OVERRUN_INT                     BIT_10
#define GEM_LINK_CHANGE                             BIT_09
#define GEM_TRANSMIT_COMPLETE                       BIT_07
#define GEM_AMBA_ERROR                              BIT_06
#define GEM_RETRY_LIMIT_EXCEEDED_OR_LATE_COLLISION  BIT_05
#define GEM_TRANSMIT_UNDER_RUN                      BIT_04
#define GEM_TX_USED_BIT_READ                        BIT_03
#define GEM_RX_USED_BIT_READ                        BIT_02
#define GEM_RECEIVE_COMPLETE                        BIT_01
#define GEM_MANAGEMENT_FRAME_SENT                   BIT_00

/*
 * General MAC Fatal or Non Fatal Interrupt register bit definitions
 * Note bits 0 to 15 are as per interrupt mask etc registers above.
 */

#define GEM_LOCKUP_DETECTED_INT_TYPE                BIT_22
#define GEM_TSU_TIMER_COMPARISON_INTERRUPT_INT_TYPE BIT_21
#define GEM_WOL_INTERRUPT_INT_TYPE                  BIT_20
#define GEM_RECEIVE_LPI_INT_TYPE                    BIT_19
#define GEM_TSU_SECONDS_REGISTER_INCREMENT_INT_TYPE BIT_18
#define GEM_PTP_FRAME_RECEIVED_INT_TYPE             BIT_17
#define GEM_PCS_INT_TYPE                            BIT_16

/* General MAC Phy Management register bit definitions */
/* eMAC Phy Management register bit definitions */

#define GEM_WRITE0                                  BIT_31
#define GEM_WRITE1                                  BIT_30
#define GEM_OPERATION                               (BIT_28 | BIT_29)
#define GEM_PHY_ADDRESS                             (BITS_05 << 23)
#define GEM_REGISTER_ADDRESS                        (BITS_05 << 18)
#define GEM_WRITE10                                 (BIT_16 | BIT_17)
#define GEM_PHY_WRITE_READ_DATA                     BITS_16

#define GEM_PHY_OP_CL22_WRITE                       ((uint32_t)(1UL))
#define GEM_PHY_OP_CL22_READ                        ((uint32_t)(2UL))

#define GEM_PHY_OP_CL45_ADDRESS                     ((uint32_t)(0UL))
#define GEM_PHY_OP_CL45_WRITE                       ((uint32_t)(1UL))
#define GEM_PHY_OP_CL45_POST_READ_INC               ((uint32_t)(2UL))
#define GEM_PHY_OP_CL45_READ                        ((uint32_t)(3UL))

#define GEM_OPERATION_SHIFT                         28
#define GEM_PHY_ADDRESS_SHIFT                       23
#define GEM_REGISTER_ADDRESS_SHIFT                  18
#define GEM_WRITE10_SHIFT                           16

/* General MAC Pause Time register bit definitions */
/* General MAC Transmit Pause Time register bit definitions */
/* eMAC Pause Time register bit definitions */
/* eMAC Transmit Pause Time register bit definitions */

#define GEM_QUANTUM                                 BITS_16

/* General MAC PBuff TX Cutthru register bit definitions */
/* General MAC PBuff RX Cutthru register bit definitions */
/* eMAC PBuff TX Cutthru register bit definitions */
/* eMAC PBuff RX Cutthru register bit definitions */

#define GEM_DMA_CUTTHRU                             BIT_31
#define GEM_DMA_TX_CUTTHRU_THRESHOLD                BITS_11
#define GEM_DMA_RX_CUTTHRU_THRESHOLD                BITS_10
#define GEM_DMA_EMAC_CUTTHRU_THRESHOLD              BITS_09

/* General MAC Jumbo Max Length register bit definitions */
/* eMAC Jumbo Max Length register bit definitions */

#define GEM_JUMBO_MAX_LENGTH                        BITS_14

/* General MAC AXI Max Pipeline register bit definitions */
/* eMAC AXI Max Pipeline register bit definitions */

#define GEM_USE_AW2B_FILL                           BIT_16
#define GEM_AW2W_MAX_PIPELINE                       (BITS_08 << 8)
#define GEM_AR2R_MAX_PIPELINE                       BITS_08

/* General MAC Int Moderation register bit definitions */
/* eMAC Int Moderation register bit definitions */

#define GEM_TX_INT_MODERATION                       (BITS_08 << 16)
#define GEM_RX_INT_MODERATION                       BITS_08

/* General MAC Sys Wake Time register bit definitions */
/* eMAC Sys Wake Time register bit definitions */

#define GEM_SYS_WAKE_TIME                           BITS_16

/* General MAC Lockup Config register bit definitions */
/* General RX MAC Lockup Time register bit definitions */
/* eMAC Lockup Config register bit definitions */
/* RX eMAC Lockup Time register bit definitions */

#define GEM_TX_DMA_LOCKUP_MON_EN                    BIT_31
#define GEM_TX_MAC_LOCKUP_MON_EN                    BIT_30
#define GEM_RX_DMA_LOCKUP_MON_EN                    BIT_29
#define GEM_RX_MAC_LOCKUP_MON_EN                    BIT_28
#define GEM_LOCKUP_RECOVERY_EN                      BIT_27
#define GEM_LOCKUP_TIME                             BITS_16

/* General MAC Specific Address 1 Top register bit definitions */
/* General MAC Specific Address 2 Top register bit definitions */
/* General MAC Specific Address 3 Top register bit definitions */
/* General MAC Specific Address 4 Top register bit definitions */
/* eMAC Specific Address 1 Top register bit definitions */
/* eMAC Specific Address 2 Top register bit definitions */
/* eMAC Specific Address 3 Top register bit definitions */
/* eMAC Specific Address 4 Top register bit definitions */

#define GEM_FILTER_BYTE_MASK                        (BITS_06 << 24)
#define GEM_FILTER_TYPE                             BIT_16
#define GEM_SPEC_ADDRESS                            BITS_16

/* General MAC Specific Address Type 1 register bit definitions */
/* General MAC Specific Address Type 2 register bit definitions */
/* General MAC Specific Address Type 3 register bit definitions */
/* General MAC Specific Address Type 4 register bit definitions */
/* eMAC Specific Address Type 1 register bit definitions */
/* eMAC Specific Address Type 2 register bit definitions */
/* eMAC Specific Address Type 3 register bit definitions */
/* eMAC Specific Address Type 4 register bit definitions */

#define GEM_ENABLE_COPY                             BIT_31
#define GEM_SPEC_ADDR_MATCH                         BITS_16

/* General MAC Wake On LAN register bit definitions */
/* eMAC Wake On LAN register bit definitions */

#define GEM_WOL_MULTICAST_HASH                      BIT_19
#define GEM_WOL_SPEC_ADDRESS_1                      BIT_18
#define GEM_WOL_ARP_REQUEST                         BIT_17
#define GEM_WOL_MAGIC_PACKET                        BIT_16
#define GEM_WOL_ADDRESS                             BITS_16

/* General MAC Stretch Ratio register bit definitions */
/* eMAC Stretch Ratio register bit definitions */

#define GEM_IPG_STRETCH                             BITS_16
#define GEM_IPG_STRETCH_DIV                         (BITS_8 << 8)
#define GEM_IPG_STRETCH_MUL                         BITS_08

#define GEM_IPG_STRETCH_DIV_MASK                    BITS_08
#define GEM_IPG_STRETCH_DIV_SHIFT                   8
#define GEM_IPG_STRETCH_MUL_MASK                    BITS_08

/* General MAC Stacked VLAN register bit definitions */
/* eMAC Stacked VLAN register bit definitions */

#define GEM_ENABLE_PROCESSING                       BIT_31
#define GEM_VLAN_MATCH                              BITS_16
#define GEM_VLAN_C_TAG                              (0x8100U)
#define GEM_VLAN_S_TAG                              (0x88A8U)

/* Valid EtherTypes including VLAN tags must be bigger than this */
#define    GEM_VLAN_ETHERTYPE_MIN                   (1536U)
#define    GEM_VLAN_NO_STACK                        (0U)

/* General MAC Transmit PFC Pause register bit definitions */
/* eMAC Transmit PFC Pause register bit definitions */

#define GEM_VECTOR                                  (BITS_08 << 8)
#define GEM_VECTOR_ENABLE                           BITS_08

/* General MAC Specific Address Type 1 Mask register bit definitions */
/* eMAC Specific Address Type 1 Mask register bit definitions */

#define GEM_SPEC_ADDR_MASK                          BITS_16

/* General MAC Receive DMA Data Buffer Address Mask register bit definitions */
/* eMAC Receive DMA Data Buffer Address Mask register bit definitions */

#define GEM_DMA_DBUF_ADDR_MASK_VALUE                (BITS_04 << 28)
#define GEM_DMA_DBUF_ADDR_MASK_ENABLE               BITS_04

/* General MAC TSU timer comparison value nanosecond register bit definitions */
/* eMAC TSU timer comparison value nanosecond register bit definitions */

#define GEM_NSEC_COMPARISON_VALUE                   BITS_22

/* General MAC TSU timer comparison value seconds 47:32 register bit definitions */
/* General MAC PTP Event Frame Transmitted Seconds Register 47:32 register bit definitions */
/* General MAC PTP Event Frame Received Seconds Register 47:32 register bit definitions */
/* General MAC PTP Peer Event Frame Transmitted Seconds Register 47:32 register bit definitions */
/* General MAC PTP Peer Event Frame Received Seconds Register 47:32 register bit definitions */
/* eMAC TSU timer comparison value seconds 47:32 register bit definitions */
/* eMAC PTP Event Frame Transmitted Seconds Register 47:32 register bit definitions */
/* eMAC PTP Event Frame Received Seconds Register 47:32 register bit definitions */
/* eMAC PTP Peer Event Frame Transmitted Seconds Register 47:32 register bit definitions */
/* eMAC PTP Peer Event Frame Received Seconds Register 47:32 register bit definitions */

#define GEM_SEC_VALUE_UPPER                         BITS_16

/* General MAC DP RAM Fill Debug register bit definitions */
/* eMAC DP RAM Fill Debug register bit definitions */

#define GEM_DMA_TX_RX_FILL_LEVEL                    (BITS_16 << 16)
#define GEM_DMA_TX_Q_FILL_LEVEL_SELECT              (BITS_04 << 4)
#define GEM_DMA_TX_RX_FILL_LEVEL_SELECT             BIT_00

/* General MAC Revision register bit definitions */
/* eMAC Revision register bit definitions */

#define GEM_FIX_NUMBER                              (BITS_04 << 24)
#define GEM_MODULE_IDENTIFICATION_NUMBER            (BITS_12 << 16)
#define GEM_MODULE_REVISION                         BITS_16

/* General MAC Octets Transmitted Top register bit definitions */
/* General MAC Octets Received Top register bit definitions */
/* eMAC Octets Transmitted Top register bit definitions */
/* eMAC Octets Received Top register bit definitions */

#define GEM_UPPER_BITS_OF_48                        BITS_16

/* General MAC Pause Frames Transmitted register bit definitions */
/* General MAC Pause Frames Received register bit definitions */
/* eMAC Pause Frames Transmitted register bit definitions */
/* eMAC Pause Frames Received register bit definitions */

#define GEM_FRAME_COUNT                             BITS_16

/* General MAC Transmit Underruns register bit definitions */
/* eMAC Transmit Underruns register bit definitions */

#define GEM_UNDERRUN_COUNT                          BITS_10

/* General MAC Single Collision register bit definitions */
/* General MAC Multiple Collisions register bit definitions */
/* eMAC Single Collision register bit definitions */
/* eMAC Multiple Collisions register bit definitions */

#define GEM_SM_COLLISION_COUNT                      BITS_18

/* General MAC Late Collisions register bit definitions */
/* eMAC Late Collisions register bit definitions */

#define GEM_LATE_COLLISION_COUNT                    BITS_10

/* General MAC Deferred Frames register bit definitions */
/* eMAC Deferred Frames register bit definitions */

#define GEM_DEFERRED_FRAMES_COUNT                   BITS_18

/* General MAC CRS Errors register bit definitions */
/* eMAC CRS Errors register bit definitions */

#define GEM_CRS_ERROR_COUNT                         BITS_10

/* General MAC Undersize Frames Received register bit definitions */
/* eMAC Undersize Frames Received register bit definitions */

#define GEM_RUNT_FRAME_COUNT                        BITS_10

/* General MAC Oversize Frames Received register bit definitions */
/* eMAC Oversize Frames Received register bit definitions */

#define GEM_OVERSIZE_FRAME_COUNT                    BITS_10

/* General MAC Jabbers Received register bit definitions */
/* eMAC Jabbers Received register bit definitions */

#define GEM_JABBER_COUNT                            BITS_10

/* General MAC FCS Error register bit definitions */
/* eMAC FCS Error register bit definitions */

#define GEM_FCS_ERROR_COUNT                         BITS_10

/* General MAC Length Field Frame Errors register bit definitions */
/* eMAC Length Field Frame Errors register bit definitions */

#define GEM_LENGTH_ERROR_COUNT                      BITS_10

/* General MAC Receive Symbol Errors register bit definitions */
/* eMAC Receive Symbol Errors register bit definitions */

#define GEM_SYMBOL_ERROR_COUNT                      BITS_10

/* General MAC Receive Alignment Errors register bit definitions */
/* eMAC Receive Alignment Errors register bit definitions */

#define GEM_ALIGNMENT_ERROR_COUNT                   BITS_10

/* General MAC Receive Resource Error register bit definitions */
/* eMAC Receive Resource Error register bit definitions */

#define GEM_RESOURCE_ERROR_COUNT                    BITS_10

/* General MAC Receive Overrun register bit definitions */
/* eMAC Receive Overrun register bit definitions */

#define GEM_OVERRUN_COUNT                           BITS_10

/* General MAC IP Checksum Error register bit definitions */
/* General MAC TCP Checksum Error register bit definitions */
/* General MAC UDP Checksum Error register bit definitions */
/* eMAC IP Checksum Error register bit definitions */
/* eMAC TCP Checksum Error register bit definitions */
/* eMAC UDP Checksum Error register bit definitions */

#define GEM_IP_CHECKSUM_ERROR_COUNT                 BITS_08

/* General MAC Auto Flushed Packets register bit definitions */
/* eMAC Auto Flushed Packets register bit definitions */

#define GEM_AUTO_FLUSHED_COUNT                      BITS_16

/* General MAC TSU Timer Increment Sub Nanoseconds register bit definitions */
/* eMAC TSU Timer Increment Sub Nanoseconds register bit definitions */

#define GEM_SUB_NS_INCR_LSB                         (BITS_08 << 24)
#define GEM_SUB_NS_INCR                             BITS_16

/* General MAC TSU Timer Seconds MSB register bit definitions */
/* General MAC TSU Strobe Seconds MSB register bit definitions */

#define GEM_TSU_SECONDS_MSB                         BITS_16

/* General MAC TSU Timer Sync Strobe Nanoseconds register bit definitions */
/* General MAC TSU Timer Nanoseconds register bit definitions */
/* General MAC TSU Timer Adjust register bit definitions */
/* General MAC PTP Event Frame Transmitted Nanoseconds register bit definitions */
/* General MAC PTP Event Frame Received Nanoseconds register bit definitions */
/* General MAC PTP Peer Event Frame Transmitted Nanoseconds register bit definitions */
/* General MAC PTP Peer Event Frame Received Nanoseconds register bit definitions */
/* eMAC TSU Timer Sync Strobe Nanoseconds register bit definitions */
/* eMAC TSU Timer Nanoseconds register bit definitions */
/* eMAC TSU Timer Adjust register bit definitions */
/* eMAC PTP Event Frame Transmitted Nanoseconds register bit definitions */
/* eMAC PTP Event Frame Received Nanoseconds register bit definitions */
/* eMAC PTP Peer Event Frame Transmitted Nanoseconds register bit definitions */
/* eMAC PTP Peer Event Frame Received Nanoseconds register bit definitions */

#define GEM_ADD_SUBTRACT                            BIT_31 /* Adjust register only... */
#define GEM_TSU_NANOSECONDS                         BITS_30

/* General MAC TSU Timer Adjust register bit definitions */
/* eMAC TSU Timer Adjust register bit definitions */

#define GEM_NUM_INCS                                (BITS_08 << 16)
#define GEM_ALT_NS_INC                              (BITS_08 << 8)
#define GEM_NS_INCREMENT                            BITS_08

/* General MAC PCS Control register bit definitions */

#define GEM_PCS_SOFTWARE_RESET                      BIT_15
#define GEM_LOOPBACK_MODE                           BIT_14
#define GEM_SPEED_SELECT_BIT_1                      BIT_13
#define GEM_ENABLE_AUTO_NEG                         BIT_12
#define GEM_RESTART_AUTO_NEG                        BIT_09
#define GEM_MAC_DUPLEX_STATE                        BIT_08
#define GEM_COLLISION_TEST                          BIT_07
#define GEM_SPEED_SELECT_BIT_0                      BIT_06

/* General MAC PCS Status register bit definitions */

#define GEM_BASE_100_T4                             BIT_15
#define GEM_BASE_100_X_FULL_DUPLEX                  BIT_14
#define GEM_BASE_100_X_HALF_DUPLEX                  BIT_13
#define GEM_MBPS_10_FULL_DUPLEX                     BIT_12
#define GEM_MBPS_10_HALF_DUPLEX                     BIT_11
#define GEM_BASE_100_T2_FULL_DUPLEX                 BIT_10
#define GEM_BASE_100_T2_HALF_DUPLEX                 BIT_09
#define GEM_EXTENDED_STATUS                         BIT_08
#define GEM_AUTO_NEG_COMPLETE                       BIT_05
#define GEM_REMOTE_FAULT                            BIT_04
#define GEM_AUTO_NEG_ABILITY                        BIT_03
#define GEM_LINK_STATUS                             BIT_02
#define GEM_EXTENDED_CAPABILITIES                   BIT_00

/* General MAC PCS PHY Top ID register bit definitions */
/* General MAC PCS PHY Bottom ID register bit definitions */

#define GEM_ID_CODE                                 BITS_16

/* General MAC PCS Autonegotiation Advertisment register bit definitions */

#define GEM_AN_AV_NEXT_PAGE                         BIT_15
#define GEM_AN_AV_REMOTE_FAULT                      (BIT_12 | BIT_13)
#define GEM_AN_AV_PAUSE                             (BIT_08 | BIT_07)
#define GEM_AN_AV_HALF_DUPLEX                       BIT_06
#define GEM_AN_AV_FULL_DUPLEX                       BIT_05

/* General MAC PCS Autonegotiation Link Partner Base register bit definitions */

#define GEM_LINK_PARTNER_NEXT_PAGE_STATUS           BIT_15
#define GEM_LINK_PARTNER_ACKNOWLEDGE                BIT_14
#define GEM_LINK_PARTNER_REMOTE_FAULT_DUPLEX_MODE   (BIT_12 | BIT_13)
#define GEM_LINK_PARTNER_SPEED                      (BIT_10 | BIT_11)
#define GEM_LINK_PARTNER_PAUSE                      (BIT_07 | BIT_08)
#define GEM_LINK_PARTNER_HALF_DUPLEX                BIT_06
#define GEM_LINK_PARTNER_FULL_DUPLEX                BIT_05

/* General MAC PCS Autonegotiation Next Page Ability register bit definitions */

#define GEM_NEXT_PAGE_CAPABILITY                    BIT_02
#define GEM_PAGE_RECEIVED                           BIT_01

/* General MAC PCS Autonegotiation Next Page Transmit register bit definitions */
/* General MAC PCS Autonegotiation Next Page Receive register bit definitions */

#define GEM_NEXT_PAGE_TO_TRANSMIT                   BIT_15
#define GEM_NEXT_PAGE_TO_RECEIVE                    BIT_15
#define GEM_ACKNOWLEDGE                             BIT_14
#define GEM_MESSAGE_PAGE_INDICATOR                  BIT_13
#define GEM_ACKNOWLEDGE_2                           BIT_12
#define GEM_TOGGLE                                  BIT_11
#define GEM_AN_MESSAGE                              BITS_11

/* General MAC PCS Autonegotiation Extended Status register bit definitions */

#define GEM_FULL_DUPLEX_1000BASE_X                  BIT_15
#define GEM_HALF_DUPLEX_1000BASE_X                  BIT_14
#define GEM_FULL_DUPLEX_1000BASE_T                  BIT_13
#define GEM_HALF_DUPLEX_1000BASE_T                  BIT_12

/* General MAC Received LPI Transitions register bit definitions */
/* General MAC Transmitted LPI Transitions register bit definitions */
/* eMAC Received LPI Transitions register bit definitions */
/* eMAC Transmitted LPI Transitions register bit definitions */

#define GEM_LPI_COUNT                               BITS_16

/* General MAC Received LPI Time register bit definitions */
/* General MAC Transmitted LPI Time register bit definitions */
/* eMAC Received LPI Time register bit definitions */
/* eMAC Transmitted LPI Time register bit definitions */

#define GEM_LPI_TIME                                BITS_24

/* General MAC Design Configuration Debug 1 register bit definitions */
/* eMAC Design Configuration Debug 1 register bit definitions */

#define GEM_AXI_CACHE_VALUE                         (BITS_04 << 28)
#define GEM_DMA_BUS_WIDTH                           (BIT_25 | BIT_26 | BIT 27)
#define GEM_EXCLUDE_CBS                             BIT_24
#define GEM_IRQ_READ_CLEAR                          BIT_23
#define GEM_NO_SNAPSHOT                             BIT_22
#define GEM_NO_STATS                                BIT_21
#define GEM_USER_IN_WIDTH                           (BITS_05 << 15)
#define GEM_USER_OUT_WIDTH                          (BITS_05 << 10)
#define GEM_USER_IO                                 BIT_09
#define GEM_EXT_FIFO_INTERFACE                      BIT_06
#define GEM_INT_LOOPBACK                            BIT_04
#define GEM_EXCLUDE_QBV                             BIT_01
#define GEM_NO_PCS                                  BIT_00

/* General MAC Design Configuration Debug 2 register bit definitions */
/* eMAC Design Configuration Debug 2 register bit definitions */

#define GEM_SPRAM                                   BIT_31
#define GEM_AXI                                     BIT_30
#define GEM_TX_PBUF_ADDR                            (BITS_04 << 26)
#define GEM_RX_PBUF_ADDR                            (BITS_04 << 22)
#define GEM_TX_PKT_BUFFER                           BIT_21
#define GEM_RX_PKT_BUFFER                           BIT_20
#define GEM_HPROT_VALUE                             (BITS_04 << 16)
#define GEM_JUMBO_MAX_LENGTH                        BITS_14

/* General MAC Design Configuration Debug 3 register bit definitions */
/* eMAC Design Configuration Debug 3 register bit definitions */

#define GEM_NUM_SPEC_ADD_FILTERS                   (BITS_06 << 24)

/* General MAC Design Configuration Debug 5 register bit definitions */
/* eMAC Design Configuration Debug 5 register bit definitions */

#define GEM_AXI_PROT_VALUE                          (BIT_29 | BIT_30 | BIT_31)
#define GEM_TSU_CLK                                 BIT_28
#define GEM_RX_BUFFER_LENGTH_DEF                    (BITS_08 << 20)
#define GEM_TX_PBUF_SIZE_DEF                        BIT_19
#define GEM_RX_PBUF_SIZE_DEF                        (BIT_17 | BIT_18)
#define GEM_ENDIAN_SWAP_DEF                         (BIT_15 | BIT_16)
#define GEM_MDC_CLOCK_DIV                           (BIT_12 | BIT_13 | BIT_14)
#define GEM_DMA_BUS_WIDTH_DEF                       (BIT_10 | BIT_11)
#define GEM_PHY_IDENT                               BIT_09
#define GEM_TSU                                     BIT_08
#define GEM_TX_FIFO_CNT_WIDTH                       (BITS_04  << 4)
#define GEM_RX_FIFO_CNT_WIDTH                       BITS_04

/* General MAC Design Configuration Debug 6 register bit definitions */
/* eMAC Design Configuration Debug 6 register bit definitions */

#define GEM_PBUF_LSO                                BIT_27
#define GEM_PBUF_RSC                                BIT_26
#define GEM_PBUF_CUTTHRU                            BIT_25
#define GEM_PFC_MULTI_QUANTUM                       BIT_24
#define GEM_DMA_ADDR_WIDTH_IS_64B                   BIT_23
#define GEM_HOST_IF_SOFT_SEL                        BIT_22
#define GEM_TX_ADD_FIFO_IF                          BIT_21
#define GEM_EXT_TSU_TIMER                           BIT_20
#define GEM_TX_PBUF_QUEUE_SEGMENT_SIZE              (BITS_04 << 16)
#define GEM_DMA_PRIORITY_QUEUE15                    BIT_15
#define GEM_DMA_PRIORITY_QUEUE14                    BIT_14
#define GEM_DMA_PRIORITY_QUEUE13                    BIT_13
#define GEM_DMA_PRIORITY_QUEUE12                    BIT_12
#define GEM_DMA_PRIORITY_QUEUE11                    BIT_11
#define GEM_DMA_PRIORITY_QUEUE10                    BIT_10
#define GEM_DMA_PRIORITY_QUEUE9                     BIT_09
#define GEM_DMA_PRIORITY_QUEUE8                     BIT_08
#define GEM_DMA_PRIORITY_QUEUE7                     BIT_07
#define GEM_DMA_PRIORITY_QUEUE6                     BIT_06
#define GEM_DMA_PRIORITY_QUEUE5                     BIT_05
#define GEM_DMA_PRIORITY_QUEUE4                     BIT_04
#define GEM_DMA_PRIORITY_QUEUE3                     BIT_03
#define GEM_DMA_PRIORITY_QUEUE2                     BIT_02
#define GEM_DMA_PRIORITY_QUEUE1                     BIT_01

/* General MAC Design Configuration Debug 7 register bit definitions */
/* eMAC Design Configuration Debug 7 register bit definitions */

#define GEM_TX_PBUF_NUM_SEGMENTS_Q7                 (BITS_04 << 28)
#define GEM_TX_PBUF_NUM_SEGMENTS_Q6                 (BITS_04 << 24)
#define GEM_TX_PBUF_NUM_SEGMENTS_Q5                 (BITS_04 << 20)
#define GEM_TX_PBUF_NUM_SEGMENTS_Q4                 (BITS_04 << 16)
#define GEM_TX_PBUF_NUM_SEGMENTS_Q3                 (BITS_04 << 12)
#define GEM_TX_PBUF_NUM_SEGMENTS_Q2                 (BITS_04 << 8)
#define GEM_TX_PBUF_NUM_SEGMENTS_Q1                 (BITS_04 << 4)
#define GEM_TX_PBUF_NUM_SEGMENTS_Q0                 BITS_04

/* General MAC Design Configuration Debug 8 register bit definitions */
/* eMAC Design Configuration Debug 8 register bit definitions */

#define GEM_NUM_TYPE1_SCREENERS                     (BITS_08 << 24)
#define GEM_NUM_TYPE2_SCREENERS                     (BITS_08 << 16)
#define GEM_NUM_SCR2_ETHTYPE_REGS                   (BITS_08 << 8)
#define GEM_NUM_SCR2_COMPARE_REGS                   BITS_08

/* General MAC Design Configuration Debug 9 register bit definitions */
/* eMAC Design Configuration Debug 9 register bit definitions */

#define GEM_TX_PBUF_NUM_SEGMENTS_Q15                (BITS_04 << 28)
#define GEM_TX_PBUF_NUM_SEGMENTS_Q14                (BITS_04 << 24)
#define GEM_TX_PBUF_NUM_SEGMENTS_Q13                (BITS_04 << 20)
#define GEM_TX_PBUF_NUM_SEGMENTS_Q12                (BITS_04 << 16)
#define GEM_TX_PBUF_NUM_SEGMENTS_Q11                (BITS_04 << 12)
#define GEM_TX_PBUF_NUM_SEGMENTS_Q10                (BITS_04 << 8)
#define GEM_TX_PBUF_NUM_SEGMENTS_Q9                 (BITS_04 << 4)
#define GEM_TX_PBUF_NUM_SEGMENTS_Q8                 BITS_04

/* General MAC Design Configuration Debug 10 register bit definitions */
/* eMAC Design Configuration Debug 10 register bit definitions */

#define GEM_EMAC_BUS_WIDTH                          (BITS_04 << 28)
#define GEM_TX_PBUF_DATA                            (BITS_04 << 24)
#define GEM_RX_PBUF_DATA                            (BITS_04 << 20)
#define GEM_AXI_ACCESS_PIPELINE_BITS                (BITS_04 << 16)
#define GEM_AXI_TX_DESCR_RD_BUFF_BITS               (BITS_04 << 12)
#define GEM_AXI_RX_DESCR_RD_BUFF_BITS               (BITS_04 << 8)
#define GEM_AXI_TX_DESCR_WR_BUFF_BITS               (BITS_04 << 4)
#define GEM_AXI_RX_DESCR_WR_BUFF_BITS               BITS_04

/* General MAC Design Configuration Debug 11 register bit definitions */
/* eMAC Design Configuration Debug 11 register bit definitions */

#define GEM_PROTECT_DESCR_ADDR                      BIT_04
#define GEM_PROTECT_TSU                             BIT_03
#define GEM_ADD_CSR_PARITY                          BIT_02
#define GEM_ADD_DP_PARITY                           BIT_01
#define GEM_ADD_ECC_DPRAM                           BIT_00

/* General MAC Design Configuration Debug 12 register bit definitions */
/* eMAC Design Configuration Debug 12 register bit definitions */

#define GEM_GEM_HAS_802P3_BR                        BIT_25
#define GEM_EMAC_TX_PBUF_ADDR                       (BITS_04 << 21)
#define GEM_EMAC_RX_PBUF_ADDR                       (BITS_04 << 17)
#define GEM_GEM_HAS_CB                              BIT_16
#define GEM_GEM_CB_HISTORY_LEN                      (BITS_08 << 8)
#define GEM_GEM_NUM_CB_STREAMS                      BITS_08

/* General MAC Queue 1 DMA Receive Buffer Size register bit definitions */
/* General MAC Queue 2 DMA Receive Buffer Size register bit definitions */
/* General MAC Queue 3 DMA Receive Buffer Size register bit definitions */

#define GEM_DMA_RX_Q_BUF_SIZE                       BITS_08

/* General MAC CBS Control register bit definitions */
/* eMAC CBS Control register bit definitions */

#define GEM_CBS_ENABLE_QUEUE_B                      BIT_01
#define GEM_CBS_ENABLE_QUEUE_A                      BIT_00

/* General MAC TX BD Control register bit definitions */
/* General MAC RX BD Control register bit definitions */
/* eMAC TX BD Control register bit definitions */
/* eMAC RX BD Control register bit definitions */

#define GEM_BD_TS_MODE                              (BIT_04 | BIT_05)
#define GEM_BD_TS_MODE_SHIFT                        4

/* General MAC WD Counter register bit definitions */

#define GEM_RX_BD_REREAD_TIMER                      BITS_04

/* General MAC AXI TX Full Threshold 0 register bit definitions */

#define GEM_AXI_TX_FULL_ADJ_0                       (BITS_11 << 16)
#define GEM_AXI_TX_FULL_ADJ_1                       BITS_11

/* General MAC AXI TX Full Threshold 1 register bit definitions */

#define GEM_AXI_TX_FULL_ADJ_2                       (BITS_11 << 16)
#define GEM_AXI_TX_FULL_ADJ_3                       BITS_11

/* General Screening Type 1 Register register bit definitions */

#define GEM_DROP_ON_MATCH                           BIT_30
#define GEM_UDP_PORT_MATCH_ENABLE                   BIT_29
#define GEM_DSTC_ENABLE                             BIT_28
#define GEM_UDP_PORT_MATCH                          (BITS_16 << 12)
#define GEM_DSTC_MATCH                              (BITS_08 << 4)
#define GEM_QUEUE_NUMBER                            BITS_04

#define GEM_UDP_PORT_MATCH_SHIFT                    12
#define GEM_DSTC_MATCH_SHIFT                        4

/* General Screening Type 2 Register register bit definitions */

#define GEM_T2_DROP_ON_MATCH                        BIT_31
#define GEM_COMPARE_C_ENABLE                        BIT_30
#define GEM_COMPARE_C                               (BITS_05 << 25)
#define GEM_COMPARE_B_ENABLE                        BIT_24
#define GEM_COMPARE_B                               (BITS_05 << 19)
#define GEM_COMPARE_A_ENABLE                        BIT_18
#define GEM_COMPARE_A                               (BITS_05 << 13)
#define GEM_ETHERTYPE_ENABLE                        BIT_12
#define GEM_ETHERTYPE_REG_INDEX                     (BITS_03 << 9)
#define GEM_VLAN_ENABLE                             BIT_08
#define GEM_VLAN_PRIORITY                           (BITS_03 << 4)

#define GEM_COMPARE_C_SHIFT                         25
#define GEM_COMPARE_B_SHIFT                         19
#define GEM_COMPARE_A_SHIFT                         13
#define GEM_ETHERTYPE_REG_INDEX_SHIFT               9
#define GEM_VLAN_PRIORITY_SHIFT                     4

/* General MAC TX Schedule Control register bit definitions */

#define GEM_TX_SCHED_Q3                             (BIT_06 | BIT_07)
#define GEM_TX_SCHED_Q2                             (BIT_04 | BIT_05)
#define GEM_TX_SCHED_Q1                             (BIT_02 | BIT_03)
#define GEM_TX_SCHED_Q0                             (BIT_00 | BIT_01)

/* General MAC TX Bandwidth Rate Limit Queue 0 to 3 register bit definitions */

#define GEM_DWRR_ETS_WEIGHT_Q3                      (BITS_08 << 24)
#define GEM_DWRR_ETS_WEIGHT_Q2                      (BITS_08 << 16)
#define GEM_DWRR_ETS_WEIGHT_Q1                      (BITS_08 << 8)
#define GEM_DWRR_ETS_WEIGHT_Q0                      BITS_08

/* General MAC TX Queue Segment Alloc Queue 0 to 3 register bit definitions */

#define GEM_SEGMENT_ALLOC_Q3                        (BIT_12 | BIT_13 | BIT_14)
#define GEM_SEGMENT_ALLOC_Q2                        (BIT_08 | BIT_09 | BIT_10)
#define GEM_SEGMENT_ALLOC_Q1                        (BIT_04 | BIT_05 | BIT_06)
#define GEM_SEGMENT_ALLOC_Q0                        (BIT_00 | BIT_01 | BIT_02)

/* General MAC Screening Type 2 Ethertype Reg 0 register bit definitions */

#define GEM_COMPARE_VALUE                           BITS_16

/* General MAC Type 2 Compare 0 Word 0 register bit definitions */
/* eMAC Type 2 Compare 0 Word 0 register bit definitions */
/* eMAC Type 2 Compare 1 Word 0 register bit definitions */
/* eMAC Type 2 Compare 2 Word 0 register bit definitions */
/* eMAC Type 2 Compare 3 Word 0 register bit definitions */
/* eMAC Type 2 Compare 4 Word 0 register bit definitions */
/* eMAC Type 2 Compare 5 Word 0 register bit definitions */

#define GEM_W0_COMPARE_VALUE                        (BITS_16 << 16)
#define GEM_W0_MASK_VALUE                           BITS_16

/* General MAC Type 2 Compare 0 Word 1 register bit definitions */
/* eMAC Type 2 Compare 0 Word 1 register bit definitions */
/* eMAC Type 2 Compare 1 Word 1 register bit definitions */
/* eMAC Type 2 Compare 2 Word 1 register bit definitions */
/* eMAC Type 2 Compare 3 Word 1 register bit definitions */
/* eMAC Type 2 Compare 4 Word 1 register bit definitions */
/* eMAC Type 2 Compare 5 Word 1 register bit definitions */

#define GEM_COMPARE_VLAN_ID                         BIT_10
#define GEM_DISABLE_MASK                            BIT_09
#define GEM_COMPARE_OFFSET                          (BIT_07 | BIT_08)
#define GEM_COMPARE_S_TAG                           BIT_07
#define GEM_OFFSET_VALUE                            BITS_07

#define GEM_COMPARE_OFFSET_SHIFT                    7

/* General MAC Enst Start Time Queue 0 register bit definitions */
/* General MAC Enst Start Time Queue 1 register bit definitions */
/* General MAC Enst Start Time Queue 2 register bit definitions */
/* General MAC Enst Start Time Queue 3 register bit definitions */
/* eMAC Enst Start Time register bit definitions */

#define GEM_START_TIME_SEC                          (BIT_30 | BIT_31)
#define GEM_START_TIME_NSEC                         BITS_30

/* General MAC Enst Start Time Queue 0 register bit definitions */
/* General MAC Enst Start Time Queue 1 register bit definitions */
/* General MAC Enst Start Time Queue 2 register bit definitions */
/* General MAC Enst Start Time Queue 3 register bit definitions */
/* General MAC Enst Off Time Queue 0 register bit definitions */
/* General MAC Enst Off Time Queue 1 register bit definitions */
/* General MAC Enst Off Time Queue 2 register bit definitions */
/* General MAC Enst Off Time Queue 3 register bit definitions */
/* eMAC Enst Start Time register bit definitions */
/* eMAC Enst Off Time register bit definitions */

#define GEM_ON_OFF_TIME                             BITS_17

/* General MAC Enst Control register bit definitions */
/* eMAC Enst Control register bit definitions */

#define GEM_ENST_DISABLE_Q_3                        BIT_19
#define GEM_ENST_DISABLE_Q_2                        BIT_18
#define GEM_ENST_DISABLE_Q_1                        BIT_17
#define GEM_ENST_DISABLE_Q_0                        BIT_16
#define GEM_ENST_ENABLE_Q_3                         BIT_03
#define GEM_ENST_ENABLE_Q_2                         BIT_02
#define GEM_ENST_ENABLE_Q_1                         BIT_01
#define GEM_ENST_ENABLE_Q_0                         BIT_00

/* General MAC MMSL Control register bit definitions */

#define GEM_MMSL_DEBUG_MODE                         BIT_06
#define GEM_ROUTE_RX_TO_PMAC                        BIT_05
#define GEM_RESTART_VER                             BIT_04
#define GEM_PRE_ENABLE                              BIT_03
#define GEM_VERIFY_DISABLE                          BIT_02
#define GEM_ADD_FRAG_SIZE                           (BIT_00 | BIT_01)

/* General MAC MMSL Status register bit definitions */

#define GEM_SMD_ERROR                               BIT_10
#define GEM_FRER_COUNT_ERR                          BIT_09
#define GEM_SMDC_ERROR                              BIT_08
#define GEM_SMDS_ERROR                              BIT_07
#define GEM_RCV_V_ERROR                             BIT_06
#define GEM_RCV_R_ERROR                             BIT_05
#define GEM_VERIFY_STATUS                           (BIT_02 | BIT_03 | BIT_04)
#define GEM_RESPOND_STATUS                          BIT_01
#define GEM_PRE_ACTIVE                              BIT_00

#define GEM_VERIFY_STATUS_SHIFT                     2

#define GEM_VERIFY_INIT                             ((uint32_t)(0x00UL))
#define GEM_VERIFY_IDLE                             ((uint32_t)(0x01UL))
#define GEM_VERIFY_SEND                             ((uint32_t)(0x02UL))
#define GEM_VERIFY_WAIT                             ((uint32_t)(0x03UL))
#define GEM_VERIFY_DONE_OK                          ((uint32_t)(0x04UL))
#define GEM_VERIFY_DONE_FAIL                        ((uint32_t)(0x05UL))

/* General MAC MMSL Error Stats register bit definitions */

#define GEM_SMD_ERR_COUNT                           (BITS_08 << 16)
#define GEM_ASS_ERR_COUNT                           BITS_08

#define GEM_SMD_ERR_COUNT_SHIFT                     16

/* General MAC MMSL Ass OK Count register bit definitions */

#define GEM_ASS_OK_COUNT                            BITS_17

/* General MAC MMSL Frag Count RX register bit definitions */
/* General MAC MMSL Frag Count TX register bit definitions */

#define GEM_FRAG_COUNT                              BITS_17

/* General MAC MMSL Interrupt Status register bit definitions */
/* General MAC MMSL Interrupt Enable register bit definitions */
/* General MAC MMSL Interrupt Disable register bit definitions */
/* General MAC MMSL Interrupt Mask register bit definitions */

#define GEM_INT_SMD_ERROR                           BIT_05
#define GEM_INT_FRER_COUNT_ERR                      BIT_04
#define GEM_INT_SMDC_ERROR                          BIT_03
#define GEM_INT_SMDS_ERROR                          BIT_02
#define GEM_INT_RCV_V_ERROR                         BIT_01
#define GEM_INT_RCV_R_ERROR                         BIT_00

#ifdef __cplusplus
}
#endif

#endif /* MSS_ETHERNET_MAC_REGS_H_ */



