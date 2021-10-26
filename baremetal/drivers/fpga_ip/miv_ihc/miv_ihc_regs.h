 /******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Register bit offsets and masks definitions for PolarFire SoC MSS MMUART
 *
 */
/*-------------------------------------------------------------------------*//**
  @page

  ==============================================================================
   miv_ihc_regs.h
  ==============================================================================
    The defines in this file are used by the user configuration
    header and the driver files. The header files need to be included in a
    project in the following order to allow over-ride of user settings.
     - miv_ihc_defines.h
     - miv_ihc_config.h    we can over-ride default setting in this file
                         This config file is stored in the boards directory and
                         is called from the mss_sw_config.h file.
     - miv_ihc_regs.h
     - miv_ihc.h

 */

#ifndef MSS_MIV_IHC_REGS_H_
#define MSS_MIV_IHC_REGS_H_

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

 */

/*-------------------------------------------------------------------------*//**

  ## MiV-IHCC register bit definitions

  Register Bit definitions of the control register of the MiV-IHCC.

  | bit position | constant                | description                 |
  |--------------|-------------------------|-----------------------------|
  | 0            | **RMP_MESSAGE_PRESENT** | Remote side message present |
  | 1            | **MP_MESSAGE_PRESENT**  | local side message present  |
  | 2            | **MPIE_EN**             | Enable MP interrupt         |
  | 3            | **ACK_INT**             | Incoming ACK                |
  | 4            | **ACK_CLR**             | Clear ACK                   |
  | 5            | **ACKIE_EN**            | Enable Ack Interrupt        |

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

/*-------------------------------------------------------------------------*//**

  ## Flags return by IHC_tx_message()

  |value|constant|description|
  |-----|-----------------------------| ---------------------------------------|
  | 0   | **MESSAGE_SENT**            | message sent |
  | 1   | **MESSAGE_RX**              | message received |

 */
#define MESSAGE_SENT    0U
#define MP_BUSY         1U


/*-------------------------------------------------------------------------*//**

  ## Flags return by IHC_rx_message()

  |value|constant|description|
  |-----|-----------------------------| ---------------------------------------|
  | 0   | **MESSAGE_RX**              | message sent |
  | 1   | **NO_MESSAGE_RX**           | message received |

 */
#define NO_MESSAGE_RX   0U
#define MESSAGE_RX      1UL


/***************************************************************************//**
  Inter hart communication channel control register.

  The control register of this IP is used to send and indicate packet receipt.

  ___CTL_REG___:
     32 bit register:
      - _RMP_       - Remote Message Present.
      - _MP_        - Message present.
      - _ACK_       - 1 => ACK present
      - _CLR_ACK_   - Set to 0 to clear ACK
      - _ACKIE_     - Enable ACK on reading of MP
      - _reserve28_ - Reserved.
 */
typedef union{      /*!< MIV_IHCC_CTL_REG register definition*/
    __IO  uint32_t              CTL_REG;
    struct
    {
    __IO  uint32_t              RMP                 :1;
    __IO  uint32_t              MP                  :1;
    __IO  uint32_t              MPIE                :1;
    __IO  uint32_t              ACK                 :1;
    __IO  uint32_t              CLR_ACK             :1;
    __IO  uint32_t              ACKIE               :1;
    __IO  uint32_t              reserve28           :26;
    } bitfield;
} MIV_IHCC_CTL_REG_TypeDef;

/*------------ IHC_IP register definition -----------*/
#define RESREVED_ADDRESS_SPACE_IHC		0x80
#define IHC_USED__ADDRESS_SIZE 			64U

/***************************************************************************//**
  Inter hart communication channel registers.

  ___version___:
     IP version, Read only

  ___CTR_REG___:
     Control register

  ___local_hart_id___:
     The hart ID of the connected hart

  ___size_msg___:
     Read only size of the msh buffer in bytes

  ___unused___:
     Spare bytes to keep registers aligned

  ___mesg_in___:
     Message in

  ___mesg_out___:
     Message out

   ___reserved___:
     Gap between each IP blk is defined here
 */
typedef struct IHC_IP_TypeDef_
{
  __IO  uint32_t  version;                  /*!< IP version   */
  __IO  MIV_IHCC_CTL_REG_TypeDef CTR_REG ;  /*!< control reg  */
  __IO  uint32_t local_hart_id;             /*!< local_hart_id: my hart id, set at local init   */
  __I   uint32_t size_msg;                  /*!< Size of msg buffer instantiated in fabric      */
  __I   uint32_t unused[4U];                /*!< not used  */
  __I   uint32_t  mesg_in[IHC_MAX_MESSAGE_SIZE]; 	/*!< message in           */
  __IO  uint32_t  mesg_out[IHC_MAX_MESSAGE_SIZE]; 	/*!< message out          */
  __I   uint8_t  reserved[RESREVED_ADDRESS_SPACE_IHC - IHC_USED__ADDRESS_SIZE];  /*!< reserved address space       */
} IHC_IP_TypeDef;

/***************************************************************************//**
  Inter hart communication agregator

  The control register of this IP is used to send and indicate packet receipt.

  ___MSG_AVAIL___:
     32 bit register:
      - _MP_H0_     - Enable interrupt from Message Present from hart 0
      - _ACK_H0_    - Enable interrupt from ACK coming from hart 0
      - _MP_H1_     - Enable interrupt from Message Present from hart 1
      - _ACK_H1_    - Enable interrupt from ACK coming from hart 1
      - _MP_H2_     - Enable interrupt from Message Present from hart 2
      - _ACK_H2_    - Enable interrupt from ACK coming from hart 2
      - _MP_H3_     - Enable interrupt from Message Present from hart 3
      - _ACK_H3_    - Enable interrupt from ACK coming from hart 3
      - _MP_H4_     - Enable interrupt from Message Present from hart 4
      - _ACK_H4_    - Enable interrupt from ACK coming from hart 4
      - _reserved_ - Reserved.
 */
typedef union IHCA_IP_MSG_AVAIL_STAT_TypeDef_{    /*!< IHCA_IP_MSG_AVAIL_STAT register definition*/
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

/***************************************************************************//**
  Inter hart communication agregator

  The control register of this IP is used to send and indicate packet receipt.

  ___INT_EN___:
     32 bit register:
      - _MP_H0_     - Enable interrupt from Message Present from hart 0
      - _ACK_H0_    - Enable interrupt from ACK coming from hart 0
      - _MP_H1_     - Enable interrupt from Message Present from hart 1
      - _ACK_H1_    - Enable interrupt from ACK coming from hart 1
      - _MP_H2_     - Enable interrupt from Message Present from hart 2
      - _ACK_H2_    - Enable interrupt from ACK coming from hart 2
      - _MP_H3_     - Enable interrupt from Message Present from hart 3
      - _ACK_H3_    - Enable interrupt from ACK coming from hart 3
      - _MP_H4_     - Enable interrupt from Message Present from hart 4
      - _ACK_H4_    - Enable interrupt from ACK coming from hart 4
      - _reserved_ - Reserved.
 */

typedef union IHCA_IP_INT_EN_TypeDef_ {                       /*!< IHCA_IP_INT_EN_TypeDef register definition*/
    __IO  uint32_t                        INT_EN;
    struct
    {
        __I  uint32_t              MP_H0_EN     :1;
        __I  uint32_t              ACK_H0_EN    :1;
        __I  uint32_t              MP_H1_EN     :1;
        __I  uint32_t              ACK_H1_EN    :1;
        __I  uint32_t              MP_H2_EN     :1;
        __I  uint32_t              ACK_H2_EN    :1;
        __I  uint32_t              MP_H3_EN     :1;
        __I  uint32_t              ACK_H3_EN    :1;
        __I  uint32_t              MP_H4_EN     :1;
        __I  uint32_t              ACK_H4_EN    :1;
        __I   uint32_t             reserved     :22;
    } bitfield;
} IHCA_IP_INT_EN_TypeDef;


/***************************************************************************//**
  Inter hart communication agregator core registers.

  The agregator takes intputs from the IHCC cores and end generates an output
  which is connected to one of the interrupts on selected hart.

  ___version___:
     IP version, Read only

  ___INT_EN___:
     enable the interrupts you require

  ___MSG_AVAIL_STAT___:
     Register used to see what inputs are high.

     Gap between each IP blk is defined here
 */
typedef struct IHCIA_IP_TypeDef_
{
  __IO  uint32_t  version;
  __IO  IHCA_IP_INT_EN_TypeDef			INT_EN;
  __I   IHCA_IP_MSG_AVAIL_STAT_TypeDef  MSG_AVAIL_STAT;
} IHCIA_IP_TypeDef;


/***************************************************************************//**
  QUEUE_IHC_INCOMING

  Type define of upper layer message available handler. The upper layer
  registers a function of this type when initialing the driver for each channel
  registered.

 */
typedef uint32_t (*QUEUE_IHC_INCOMING)(uint32_t ,  uint32_t *, uint32_t, bool, uint32_t *);


/***************************************************************************//**
  IPC_DRIVER_HART_INFO_TypeDef

  Structure used to hold information related to a hart

  ___connected_harts___:
     Stores info on which harts are comnnected ot out hart instance

  ___connected_hart_ints___:
  What remotte harts will generate an interrupt

  ___context_hart___:
     Out local hart ID.

 ___padding__:
     Keeps members aligned

___msg_in_handler[5]___:
   Seperate pointer for each remote hart incomimg handler

 */
typedef struct IPC_DRIVER_HART_INFO_TypeDef_
{
  uint32_t  connected_harts;
  uint32_t  connected_hart_ints;
  uint32_t  context_hart;
  uint32_t  padding;
  QUEUE_IHC_INCOMING  msg_in_handler[5U];
} IPC_DRIVER_HART_INFO_TypeDef;


/***************************************************************************//**
  IHC cluster definition

  Structure holds pointers to IHC cluster for a hart. This structure is
  Initialised on startup with the base addresses from from the
  miv_ihc_add_mapping.h file which is populated with base addresses from the
  Libero design if non-default.  otherwise the base addresses from this file
  will be used if mivihc_add_mapping.h is not present

  ___HART_IHC[5]___:
     A IHC for each remote hart

  ___interrupt_concentrator___:
  One concentrator/agregator for each hart

 */

typedef struct IHC_TypeDef_
{
    __IO  IHC_IP_TypeDef *HART_IHC[5U];           /*!< hart0-4 ihc registers  */
    __IO  IHCIA_IP_TypeDef *interrupt_concentrator;
    IPC_DRIVER_HART_INFO_TypeDef local_h_setup;
} IHC_TypeDef;

/*-------------------------------------------------------------------------*//**

  ## Base address definitions

  Default base address defines. These are used if the file
  miv_ihc_add_mapping.h does not exist.

  |local hart | base address     |  Name                                       |
  |-----------|------------------| --------------------------------------------|
  | 0         |  0x50000000UL    | **IHC_LOCAL_H0_REMOTE_H1**                  |
  | 0         |  0x50000100UL    | **IHC_LOCAL_H0_REMOTE_H2**                  |
  | 0         |  0x50000200UL    | **IHC_LOCAL_H0_REMOTE_H3**                  |
  | 0         |  0x50000300UL    | **IHC_LOCAL_H0_REMOTE_H4**                  |
  | 0         |  0x50000400UL    | **IHCA_LOCAL_H0**                           |
  |-----------|----------------  | --------------------------------------------|
  | 1         |  0x50000500UL    | **IHC_LOCAL_H1_REMOTE_H0**                  |
  | 1         |  0x50000600UL    | **IHC_LOCAL_H1_REMOTE_H2**                  |
  | 1         |  0x50000700UL    | **IHC_LOCAL_H1_REMOTE_H3**                  |
  | 1         |  0x50000800UL    | **IHC_LOCAL_H1_REMOTE_H4**                  |
  | 1         |  0x50000900UL    | **IHCA_LOCAL_H1**                           |
  |-----------|----------------  | --------------------------------------------|
  | 2         |  0x50000A00UL    | **IHC_LOCAL_H2_REMOTE_H0**                  |
  | 2         |  0x50000B00UL    | **IHC_LOCAL_H2_REMOTE_H1**                  |
  | 2         |  0x50000C00UL    | **IHC_LOCAL_H2_REMOTE_H3**                  |
  | 2         |  0x50000D00UL    | **IHC_LOCAL_H2_REMOTE_H4**                  |
  | 2         |  0x50000E00UL    | **IHCA_LOCAL_H2**                           |
  |-----------|----------------  | --------------------------------------------|
  | 3         |  0x50000F00UL    | **IHC_LOCAL_H3_REMOTE_H0**                  |
  | 3         |  0x50001000UL    | **IHC_LOCAL_H3_REMOTE_H1**                  |
  | 3         |  0x50002000UL    | **IHC_LOCAL_H3_REMOTE_H2**                  |
  | 3         |  0x50003000UL    | **IHC_LOCAL_H3_REMOTE_H4**                  |
  | 3         |  0x50004000UL    | **IHCA_LOCAL_H3**                           |
  |-----------|----------------  | --------------------------------------------|
  | 4         |  0x50005000UL    | **IHC_LOCAL_H4_REMOTE_H0**                  |
  | 4         |  0x50006000UL    | **IHC_LOCAL_H4_REMOTE_H1**                  |
  | 4         |  0x50007000UL    | **IHC_LOCAL_H4_REMOTE_H2**                  |
  | 4         |  0x50008000UL    | **IHC_LOCAL_H4_REMOTE_H3**                  |
  | 4         |  0x50009000UL    | **IHCA_LOCAL_H4**                           |
 */

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

#endif /* MSS_MIV_IHC_REGS_H_ */
