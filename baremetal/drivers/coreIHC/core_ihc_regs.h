 /******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Register bit offsets and masks definitions for PolarFire SoC MSS MMUART
 *
 */

#ifndef MSS_CORE_IHC_REGS_H_
#define MSS_CORE_IHC_REGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __I
#define __I  const volatile
#endif
#ifndef __IO
#define __IO volatile
#endif
#ifndef __O
#define __O volatile
#endif

/*******************************************************************************
 Register Bit definitions
 */

/*
 * Control register bits in the IHC
 */
#define RMP_MESSAGE_PRESENT         (0x01U << 0U)
#define MP_MESSAGE_PRESENT          (0x01U << 1U)
#define MPIE_EN                     (0x01U << 2U)

#define ACK_INT                     (0x01U << 3U)
#define ACK_CLR                     (0x01U << 4U)
#define ACKIE_EN                    (0x01U << 5U)

/*
 * Control register bit MASKS
 */
/* 0 => no message, 1 => we set to 1 when sending a message */
#define RMP_MASK                    (0x01U << 0U)
/* 0 => no message, 1 => we have a message. We set to 0 when message read. */
#define MP_MASK                     (0x01U << 1U)
/* Enable Message present Interrupt ( 0-1 ), we have a message */
#define MPIE_MASK                   (0x01U << 2U)
/* 1 => ACK not sent */
#define ACK_INT_MASK                (0x01U << 3U)

#define IHC_MAX_MESSAGE_SIZE        4U

/*
 * Flags
 */
#define MP_BUSY         1U
#define MESSAGE_SENT    0U
#define MESSAGE_RX      1UL
#define NO_MESSAGE_RX   0U

typedef union{                  /*!< CORE_IHC_CTL_REG register definition*/
    __IO  uint32_t              CTL_REG;
    struct
    {
    __IO  uint32_t              RMP                 :1;
    __IO  uint32_t              MP                  :1;
    __IO  uint32_t              MPIE                :1;
    __IO  uint32_t              ACK                 :1;
    __IO  uint32_t              CLR_ACK             :1;
    __IO  uint32_t              ACKIE               :1;
    __IO   uint32_t             reserve28           :26;
    } bitfield;
} CORE_IHC_CTL_REG_TypeDef;

/*------------ IHC_IP register definition -----------*/
#define RESREVED_ADDRESS_SPACE_IHC		0x80
#define IHC_USED__ADDRESS_SIZE 			64U

typedef struct
{
  __IO  uint32_t  version;                  /*!< IP version   */
  __IO  CORE_IHC_CTL_REG_TypeDef CTR_REG ;  /*!< control reg  */
  __IO  uint32_t local_hart_id;             /*!< local_hart_id: my hart id, set at local init   */
  __I   uint32_t size_msg;                  /*!< Size of msg buffer instantiated in fabric      */
  __I   uint32_t unused[4U];                /*!< not used  */
  __I   uint32_t  mesg_in[IHC_MAX_MESSAGE_SIZE]; 	/*!< message in           */
  __IO  uint32_t  mesg_out[IHC_MAX_MESSAGE_SIZE]; 	/*!< message out          */
  __I   uint8_t  reserved[RESREVED_ADDRESS_SPACE_IHC - IHC_USED__ADDRESS_SIZE];  /*!< reserved address space       */
} IHC_IP_TypeDef;

typedef union{                       /*!< IHCA_IP_MSG_AVAIL_STAT register definition*/
    __IO  uint32_t                        MSG_AVAIL;
    struct
    {
    __I  uint32_t              MP_H0        :1;
    __I  uint32_t              ACK_H0       :1;
    __I  uint32_t              MP_H1        :1;
    __I  uint32_t              ACK_H1       :1;
    __I  uint32_t              MP_H2        :1;
    __I  uint32_t              ACK_H2       :1;
    __I  uint32_t              MP_H3        :1;
    __I  uint32_t              ACK_H3       :1;
    __I  uint32_t              MP_H4        :1;
    __I  uint32_t              ACK_H4       :1;
    __I   uint32_t             reserved     :22;
    } bitfield;
} IHCA_IP_MSG_AVAIL_STAT_TypeDef;

typedef union{                       /*!< IHCA_IP_INT_EN_TypeDef register definition*/
    __IO  uint32_t                        INT_EN;
    struct
    {
    __IO  uint32_t              INT_EN_H0       	:1;
    __IO  uint32_t              INT_EN_H1       	:1;
    __IO  uint32_t              INT_EN_H2       	:1;
    __IO  uint32_t              INT_EN_H3       	:1;
    __IO  uint32_t              INT_EN_H4       	:1;
    __IO   uint32_t             reserved            :27;
    } bitfield;
} IHCA_IP_INT_EN_TypeDef;

typedef struct
{
  __IO  uint32_t  version;
  __IO  IHCA_IP_INT_EN_TypeDef			INT_EN;
  __I   IHCA_IP_MSG_AVAIL_STAT_TypeDef  MSG_AVAIL_STAT;
} IHCA_IP_TypeDef;


/*
 * Type define of upper layer message available handler
 */
typedef uint32_t (*QUEUE_IHC_INCOMING)(uint32_t ,  uint32_t *, uint32_t, bool, uint32_t *);
typedef struct
{
  uint32_t  connected_harts;
  uint32_t  connected_hart_ints;
  uint32_t  context_hart;
  uint32_t  padding;
  QUEUE_IHC_INCOMING  msg_in_handler[5U];
} IPC_DRIVER_HART_INFO_TypeDef;

/*------------ IHC cluster definition -------------------*/
typedef struct IHC_TypeDef_
{
    __IO  IHC_IP_TypeDef *HART_IHC[5U];           /*!< hart0-4 ihc registers  */
    __IO  IHCA_IP_TypeDef *interrupt_concentrator;
    IPC_DRIVER_HART_INFO_TypeDef local_h_setup;
} IHC_TypeDef;


#ifndef COMMON_AHB_BASE_ADD
#define COMMON_AHB_BASE_ADD     0x50000000UL
#endif
#ifndef IHC_HO_BASE_OFFSET
#define IHC_HO_BASE_OFFSET      0x00000000UL
#endif
#ifndef IHC_H1_BASE_OFFSET
#define IHC_H1_BASE_OFFSET      0x00000500UL
#endif
#ifndef IHC_H2_BASE_OFFSET
#define IHC_H2_BASE_OFFSET      0x00000A00UL
#endif
#ifndef IHC_H3_BASE_OFFSET
#define IHC_H3_BASE_OFFSET      0x00000F00UL
#endif
#ifndef IHC_H4_BASE_OFFSET
#define IHC_H4_BASE_OFFSET      0x00001400UL
#endif

/************** My Hart 0 ************/

#ifndef IHC_LOCAL_H0_REMOTE_H1
#define IHC_LOCAL_H0_REMOTE_H1      0x50000000
#endif

#ifndef IHC_LOCAL_H0_REMOTE_H2
#define IHC_LOCAL_H0_REMOTE_H2      0x50000100
#endif

#ifndef IHC_LOCAL_H0_REMOTE_H3
#define IHC_LOCAL_H0_REMOTE_H3      0x50000200
#endif

#ifndef IHC_LOCAL_H0_REMOTE_H4
#define IHC_LOCAL_H0_REMOTE_H4      0x50000300
#endif

#ifndef IHCA_LOCAL_H0
#define IHCA_LOCAL_H0               0x50000400
#endif

/************** My Hart 1 ************/

#ifndef IHC_LOCAL_H1_REMOTE_H0
#define IHC_LOCAL_H1_REMOTE_H0      0x50000500
#endif

#ifndef IHC_LOCAL_H1_REMOTE_H2
#define IHC_LOCAL_H1_REMOTE_H2      0x50000600
#endif

#ifndef IHC_LOCAL_H1_REMOTE_H3
#define IHC_LOCAL_H1_REMOTE_H3      0x50000700
#endif

#ifndef IHC_LOCAL_H1_REMOTE_H4
#define IHC_LOCAL_H1_REMOTE_H4      0x50000800
#endif

#ifndef IHCA_LOCAL_H1
#define IHCA_LOCAL_H1               0x50000900
#endif

/************** My Hart 2 ************/

#ifndef IHC_LOCAL_H2_REMOTE_H0
#define IHC_LOCAL_H2_REMOTE_H0      0x50000A00
#endif

#ifndef IHC_LOCAL_H2_REMOTE_H1
#define IHC_LOCAL_H2_REMOTE_H1      0x50000B00
#endif

#ifndef IHC_LOCAL_H2_REMOTE_H3
#define IHC_LOCAL_H2_REMOTE_H3      0x50000C00
#endif

#ifndef IHC_LOCAL_H2_REMOTE_H4
#define IHC_LOCAL_H2_REMOTE_H4      0x50000D00
#endif

#ifndef IHCA_LOCAL_H2
#define IHCA_LOCAL_H2               0x50000E00
#endif

/************** My Hart 3 ************/

#ifndef IHC_LOCAL_H3_REMOTE_H0
#define IHC_LOCAL_H3_REMOTE_H0      0x50000F00
#endif

#ifndef IHC_LOCAL_H3_REMOTE_H1
#define IHC_LOCAL_H3_REMOTE_H1      0x50001000
#endif

#ifndef IHC_LOCAL_H3_REMOTE_H2
#define IHC_LOCAL_H3_REMOTE_H2      0x50001100
#endif

#ifndef IHC_LOCAL_H3_REMOTE_H4
#define IHC_LOCAL_H3_REMOTE_H4      0x50001200
#endif

#ifndef IHCA_LOCAL_H3
#define IHCA_LOCAL_H3               0x50001300
#endif

/************** My Hart 4 ************/

#ifndef IHC_LOCAL_H4_REMOTE_H0
#define IHC_LOCAL_H4_REMOTE_H0      0x50001400
#endif

#ifndef IHC_LOCAL_H4_REMOTE_H1
#define IHC_LOCAL_H4_REMOTE_H1      0x50001500
#endif

#ifndef IHC_LOCAL_H4_REMOTE_H2
#define IHC_LOCAL_H4_REMOTE_H2      0x50001600
#endif

#ifndef IHC_LOCAL_H4_REMOTE_H3
#define IHC_LOCAL_H4_REMOTE_H3      0x50001700
#endif

#ifndef IHCA_LOCAL_H4
#define IHCA_LOCAL_H4               0x50001800
#endif

#ifdef __cplusplus
}
#endif

#endif /* MSS_CORE_IHC_REGS_H_ */
