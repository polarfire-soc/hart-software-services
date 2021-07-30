/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Microchip PolarFire SoC MSS USB Driver Stack
 *      USB Core Interface Layer (USB-CIFL)
 *          USB-CIF driver
 *
 *
 * Register bit offset and mask definitions for PolarFire SoC MSS USB.
 *
 * SVN $Revision$
 * SVN $Date$
 */

#ifndef __MSS_USB_CORE_REGS_H_
#define __MSS_USB_CORE_REGS_H_

#include <stdint.h>
#include <stddef.h>

/******************************************************************************
 * Power register
 */
#define POWER_REG_ENABLE_SUSPENDM_MASK                  0x01u
#define POWER_REG_SUSPEND_MODE_MASK                     0x02u
#define POWER_REG_RESUME_SIGNAL_MASK                    0x04u
#define POWER_REG_BUS_RESET_SIGNAL_MASK                 0x08u
#define POWER_REG_HS_MODE_MASK                          0x10u
#define POWER_REG_ENABLE_HS_MASK                        0x20u
#define POWER_REG_SOFT_CONN_MASK                        0x40u
#define POWER_REG_ISO_UPDATE_MASK                       0x80u

/******************************************************************************
 * Soft_reset_mask
 */
#define SOFT_RESET_REG_MASK                             0x03u

/******************************************************************************
 * DevCTL register bit masks
 */
#define DEV_CTRL_SESSION_MASK                           0x01u
#define DEV_CTRL_HOST_REQ_MASK                          0x02u
#define DEV_CTRL_HOST_MODE_MASK                         0x04u
#define DEV_CTRL_VBUS_MASK                              0x18u
#define DEV_CTRL_LS_DEV_MASK                            0x20u
#define DEV_CTRL_FS_DEV_MASK                            0x40u
#define DEV_CTRL_B_DEVICE_MASK                          0x80u

#define VBUS_BELOW_SESSION_END                          0x00u
#define VBUS_ABOVE_SESSION_END                          0x08u
#define VBUS_ABOVE_AVALID                               0x10u
#define VBUS_ABOVE_VBUS_VALID                           0x18u

/******************************************************************************
 * CSR0L bit masks (peripheral mode)
 */
#define CSR0L_DEV_RX_PKT_RDY_MASK                       0x0001u
#define CSR0L_DEV_TX_PKT_RDY_MASK                       0x0002u
#define CSR0L_DEV_STALL_SENT_MASK                       0x0004u
#define CSR0L_DEV_DATA_END_MASK                         0x0008u
#define CSR0L_DEV_SETUP_END_MASK                        0x0010u
#define CSR0L_DEV_SEND_STALL_MASK                       0x0020u
#define CSR0L_DEV_SERVICED_RX_PKT_RDY_MASK              0x0040u
#define CSR0L_DEV_SERVICED_SETUP_END_MASK               0x0080u

/******************************************************************************
 * CSR0H bit masks (peripheral mode)
 */
#define CSR0H_DEV_FLUSH_FIFO_MASK                       0x0100u

/******************************************************************************
 * COUNT0 register masks
 */
#define COUNT0_REG_MASK                                 0x7fu

/******************************************************************************
 * Endpoint TxMAXP register bit masks
 */
#define TX_MAX_P_REG_NUM_USB_PKT_SHIFT                  11u

/******************************************************************************
 * Endpoint TxCSRL register bit masks
 */
#define TxCSRL_REG_EPN_TX_PKT_RDY_MASK                  0x0001u
#define TxCSRL_REG_EPN_TX_FIFO_NE_MASK                  0x0002u
#define TxCSRL_REG_EPN_UNDERRUN_MASK                    0x0004u
#define TxCSRL_REG_EPN_FLUSH_FIFO_MASK                  0x0008u
#define TxCSRL_REG_EPN_SEND_STALL_MASK                  0x0010u
#define TxCSRL_REG_EPN_STALL_SENT_MASK                  0x0020u
#define TxCSRL_REG_EPN_CLR_DATA_TOG_MASK                0x0040u
#define TxCSRL_REG_EPN_ISO_INCOMP_TX_MASK               0x0080u

/******************************************************************************
 * Endpoint TxCSRH register bit masks
 */
/*D0,D1 are un-used*/
#define TxCSRH_REG_EPN_DMA_MODE_MASK                    0x0400u
#define TxCSRH_REG_EPN_FRC_DAT_TOG_MASK                 0x0800u
#define TxCSRH_REG_EPN_ENABLE_DMA_MASK                  0x1000u
#define TxCSRH_REG_EPN_TXRX_MODE_MASK                   0x2000u
#define TxCSRH_REG_EPN_ENABLE_ISO_MASK                  0x4000u
#define TxCSRH_REG_EPN_ENABLE_AUTOSET_MASK              0x8000u

/******************************************************************************
 * Endpoint TxMAXP register bit masks
 */
#define RX_MAX_P_REG_NUM_USB_PKT_SHIFT                  11u

/******************************************************************************
 * Endpoint RxCSRL register bit masks
 */
#define RxCSRL_REG_EPN_RX_PKT_RDY_MASK                  0x0001u
#define RxCSRL_REG_EPN_RX_FIFO_FULL_MASK                0x0002u
#define RxCSRL_REG_EPN_OVERRUN_MASK                     0x0004u
#define RxCSRL_REG_EPN_DATA_ERR_MASK                    0x0008u
#define RxCSRL_REG_EPN_FLUSH_FIFO_MASK                  0x0010u
#define RxCSRL_REG_EPN_SEND_STALL_MASK                  0x0020u
#define RxCSRL_REG_EPN_STALL_SENT_MASK                  0x0040u
#define RxCSRL_REG_EPN_CLR_DAT_TOG_MASK                 0x0080u

/******************************************************************************
 * Endpoint RxCSRH register bit masks
 */
#define RxCSRL_REG_EPN_RX_ISO_INCOMP                    0x0100u
/*D1,D2 are unused*/
#define RxCSRL_REG_EPN_DMA_MODE_MASK                    0x0800u

#define RxCSRL_REG_EPN_ISO_PID_ERR_MASK                 0x1000u
#define RxCSRL_REG_EPN_BI_DIS_NYET_MASK                 0x1000u

#define RxCSRL_REG_EPN_ENABLE_DMA_MASK                  0x2000u
#define RxCSRL_REG_EPN_ENABLE_ISO_MASK                  0x4000u
#define RxCSRL_REG_EPN_ENABLE_AUTOCLR_MASK              0x8000u

/******************************************************************************
 * Endpoint DMA_CNTL register bit masks
 */
#define DMA_CNTL_REG_START_XFR_MASK                     0x00000001u
#define DMA_CNTL_REG_DMA_DIR_MASK                       0x00000002u
#define DMA_CNTL_REG_DMA_MODE_MASK                      0x00000004u
#define DMA_CNTL_REG_ENABLE_DMA_IRQ_MASK                0x00000008u
#define DMA_CNTL_REG_DMA_EP_NUM_MASK                    0x000000F0u
#define DMA_CNTL_REG_DMA_BUS_ERR_MASK                   0x00000100u
#define DMA_CNTL_REG_DMA_BURST_MODE_MASK                0x00000600u

#define DMA_CNTL_REG_DMA_BURST_MODE_SHIFT               9u
#define DMA_CNTL_REG_DMA_EP_NUM_SHIFT                   4u
#define DMA_CNTL_REG_DMA_DIR_SHIFT                      1u
#define DMA_CNTL_REG_DMA_MODE_SHIFT                     2u

/******************************************************************************
 * TX Endpoint Fifo size masks
 */
#define TXFIFOSZ_REG_DPB_SHIFT                          4u

/******************************************************************************
 * RX Endpoint Fifo size masks
 */
#define RXFIFOSZ_REG_DPB_SHIFT                          4u

/******************************************************************************
 * TX_IRQ_ENABLE register masks
 */
#define TX_IRQ_ENABLE_REG_CEP_MASK                      0x0001u


/******************************************************************************
* Host Side register definitions
*/

/******************************************************************************
 * CSR0L bit masks
 */
#define CSR0L_HOST_RX_PKT_RDY_MASK                      0x0001u
#define CSR0L_HOST_TX_PKT_RDY_MASK                      0x0002u
#define CSR0L_HOST_STALL_RCVD_MASK                      0x0004u
#define CSR0L_HOST_SETUP_PKT_MASK                       0x0008u
#define CSR0L_HOST_RETRY_ERR_MASK                       0x0010u
#define CSR0L_HOST_IN_PKT_REQ_MASK                      0x0020u
#define CSR0L_HOST_STATUS_PKT_MASK                      0x0040u
#define CSR0L_HOST_NAK_TIMEOUT_MASK                     0x0080u

/******************************************************************************
 * CSR0H bit masks
 */
#define CSR0H_HOST_FLUSH_FIFO_MASK                      0x0100u/*Self Clearing*/
#define CSR0H_HOST_DATA_TOG_MASK                        0x0200u
#define CSR0H_HOST_DATA_TOG_WE_MASK                     0x0400u/*Self Clearing*/
#define CSR0H_HOST_DISABLE_PING_MASK                    0x0800u

/******************************************************************************
* Type0 register bit masks
*/
#define TYPE0_HOST_MP_TARGET_SPEED_MASK                 0xC0u

#define TYPE0_HOST_MP_TARGET_SPEED_HIGH                 0x40u
#define TYPE0_HOST_MP_TARGET_SPEED_FULL                 0x80u
#define TYPE0_HOST_MP_TARGET_SPEED_LOW                  0xC0u
#define TYPE0_HOST_MP_TARGET_SPEED_SELF                 0x00u

#define TYPE0_HOST_MP_TARGET_SPEED_SHIFT                6u

/******************************************************************************
* NAKLIMIT0 register bit masks
*/
#define NAKLIMIT0_REG_MASK                              0x00u

/******************************************************************************
 * Endpoint TxCSRL register bit masks
 */
#define TxCSRL_HOST_EPN_TX_PKT_RDY_MASK                 0x0001u
#define TxCSRL_HOST_EPN_TX_FIFO_NE_MASK                 0x0002u
#define TxCSRL_HOST_EPN_RESPONSE_ERR_MASK               0x0004u
#define TxCSRL_HOST_EPN_FLUSH_FIFO_MASK                 0x0008u
#define TxCSRL_HOST_EPN_SETUP_PKT_MASK                  0x0010u
#define TxCSRL_HOST_EPN_STALL_RCVD_MASK                 0x0020u
#define TxCSRL_HOST_EPN_CLR_DATA_TOG_MASK               0x0040u
#define TxCSRL_HOST_EPN_NAK_TIMEOUT_MASK                0x0080u

/******************************************************************************
 * Endpoint TxCSRH register bit masks
 */
#define TxCSRH_HOST_EPN_DATA_TOG_MASK                   0x0100u
#define TxCSRH_HOST_EPN_DATA_TOG_WE_MASK                0x0200u
#define TxCSRH_HOST_EPN_DMA_MODE_MASK                   0x0400u
#define TxCSRH_HOST_EPN_FRC_DATA_TOG_MASK               0x0800u
#define TxCSRH_HOST_EPN_ENABLE_DMA_MASK                 0x1000u
#define TxCSRH_HOST_EPN_TXRX_MODE_MASK                  0x2000u
/*D6 is unused*/
#define TxCSRH_HOST_EPN_ENABLE_AUTOSET_MASK             0x8000u

/******************************************************************************
 * Endpoint RxCSRL register bit masks
 */
#define RXCSRL_HOST_EPN_RX_PKT_RDY_MASK                 0x0001u
#define RXCSRL_HOST_EPN_RX_FIFO_FULL_MASK               0x0002u
#define RXCSRL_HOST_EPN_RESPONSE_ERR_MASK               0x0004u
#define RXCSRL_HOST_EPN_NAK_TIMEOUT_ERR_MASK            0x0008u
#define RXCSRL_HOST_EPN_FLUSH_FIFO_MASK                 0x0010u
#define RXCSRL_HOST_EPN_IN_PKT_REQ_MASK                 0x0020u
#define RXCSRL_HOST_EPN_STALL_RCVD_MASK                 0x0040u
#define RXCSRL_HOST_EPN_CLR_DATA_TOG_MASK               0x0080u

/******************************************************************************
 * Endpoint RxCSRH register bit masks
 */
#define RXCSRH_HOST_EPN_RX_ISO_INCOMP                   0x0100u
#define RXCSRH_HOST_EPN_DATA_TOG_MASK                   0x0200u
#define RXCSRH_HOST_EPN_DATA_TOG_WE_MASK                0x0400u
#define RXCSRH_HOST_EPN_DMA_MODE_MASK                   0x0800u
#define RXCSRH_HOST_EPN_PID_ERR_MASK                    0x1000u
#define RXCSRH_HOST_EPN_ENABLE_DMA_MASK                 0x2000u
#define RXCSRH_HOST_EPN_ENABLE_AUTOREQ_MASK             0x4000u
#define RXCSRH_HOST_EPN_ENABLE_AUTOCLR_MASK             0x8000u

/******************************************************************************
* TXType register bit masks
*/
#define TXTYPE_HOST_TARGET_EP_NUM_MASK                  0x0Fu
#define TXTYPE_HOST_TARGET_EP_PROTOCOL_MASK             0x30u
#define TXTYPE_HOST_TARGET_EP_SPEED_MASK                0xC0u

#define TXTYPE_HOST_TARGET_EP_NUM_SHIFT                 0u
#define TXTYPE_HOST_TARGET_EP_PROTOCOL_SHIFT            4u
#define TXTYPE_HOST_TARGET_EP_SPEED_SHIFT               6u

/******************************************************************************
 TXINTERVAL register bit masks
*/
#define TXINTERVAL_HOST_REG_MASK                        0x00

/******************************************************************************
 TXType register bit masks
 */
#define RXTYPE_HOST_TARGET_EP_NUM_MASK                  0x0Fu
#define RXTYPE_HOST_TARGET_EP_PROTOCOL_MASK             0x30u
#define RXTYPE_HOST_TARGET_EP_SPEED_MASK                0xC0u

#define RXTYPE_HOST_TARGET_EP_NUM_SHIFT                 0u
#define RXTYPE_HOST_TARGET_EP_PROTOCOL_SHIFT            4u
#define RXTYPE_HOST_TARGET_EP_SPEED_SHIFT               6u

/******************************************************************************
 RXINTERVAL register bit masks
 */
#define RXINTERVAL_HOST_REG_MASK                        0x00u

/******************************************************************************
 TX/RXFUNCTIONADDR register bit masks
 */
#define TARGET_DEVICE_ADDR_MASK                         0x7Fu

/******************************************************************************
 TX/RXHUBADDR register bit masks
 */
#define TARGET_DEVICE_HUB_ADDR_MASK                     0x7Fu
#define TARGET_DEVICE_HUB_MT_MASK                       0x10u

#define TARGET_DEVICE_HUB_MT_SHIFT                      7u

/******************************************************************************
 TX/RXHUBPORT register bit masks
 */
#define TARGET_DEVICE_HUB_PORT_MASK                     0x7Fu

/******************************************************************************
 TESTMODE register bit masks
 */
#define TESTMODE_SE0NAK_MASK                            0x01u
#define TESTMODE_TESTJ_MASK                             0x02u
#define TESTMODE_TESTK_MASK                             0x04u
#define TESTMODE_TESTPACKET_MASK                        0x08u
#define TESTMODE_FORCEHS_MASK                           0x10u
#define TESTMODE_FORCEFS_MASK                           0x20u
#define TESTMODE_FIFOACCESS_MASK                        0x40u/*Self Clearing*/
#define TESTMODE_FORCEHOST_MASK                         0x80u


typedef struct
{
    volatile uint16_t   TX_MAX_P;
    volatile uint16_t   TX_CSR;
    volatile uint16_t   RX_MAX_P;
    volatile uint16_t   RX_CSR;
    volatile uint16_t   RX_COUNT;
    volatile uint8_t    TX_TYPE;
    volatile uint8_t    TX_INTERVAL;
    volatile uint8_t    RX_TYPE;
    volatile uint8_t    RX_INTERVAL;
    volatile uint8_t    RESERVED;
    volatile uint8_t    FIFO_SIZE;
} USB_endpoint_regs_t;

typedef struct
{
    volatile uint8_t    TX_FUNC_ADDR;
    volatile uint8_t    UNUSED0;
    volatile uint8_t    TX_HUB_ADDR;
    volatile uint8_t    TX_HUB_PORT;
    volatile uint8_t    RX_FUNC_ADDR;
    volatile uint8_t    UNUSED1;
    volatile uint8_t    RX_HUB_ADDR;
    volatile uint8_t    RX_HUB_PORT;
} USB_tar_t;

typedef union
{
    struct
    {
        volatile uint32_t   VALUE;
    } WORD;

    struct
    {
        volatile uint8_t    VALUE;
        volatile uint8_t    RESERVED1;
        volatile uint8_t    RESERVED2;
        volatile uint8_t    RESERVED3;
    } BYTE;

    struct
    {
        volatile uint16_t   VALUE;
        volatile uint16_t   RESERVED;
    } HALFWORD;
} USB_fifo_t;

typedef union
{
    struct
    {
        volatile uint16_t   TX_MAX_P;
        volatile uint16_t   CSR0;
        volatile uint16_t   RX_MAX_P;
        volatile uint16_t   RX_CSR;
        volatile uint16_t   COUNT0;
        volatile uint8_t    RESERVED0;
        volatile uint8_t    RESERVED1;
        volatile uint8_t    RESERVED2;
        volatile uint8_t    RESERVED3;
        volatile uint8_t    RESERVED4;
        volatile uint8_t    CONFIG_DATA;
    } DEVICE_EP0;

    struct
    {
        volatile uint16_t   TX_MAX_P;
        volatile uint16_t   TX_CSR;
        volatile uint16_t   RX_MAX_P;
        volatile uint16_t   RX_CSR;
        volatile uint16_t   RX_COUNT;
        volatile uint8_t    RESERVED0;
        volatile uint8_t    RESERVED1;
        volatile uint8_t    RESERVED2;
        volatile uint8_t    RESERVED3;
        volatile uint8_t    RESERVED4;
        volatile uint8_t    FIFO_SIZE;
    } DEVICE_EPN;

    struct
    {
        volatile uint16_t   TX_MAX_P;
        volatile uint16_t   CSR0;
        volatile uint16_t   RX_MAX_P;
        volatile uint16_t   RX_CSR;
        volatile uint16_t   COUNT0;
        volatile uint8_t    TYPE0;
        volatile uint8_t    NAK_LIMIT0;
        volatile uint8_t    RX_TYPE;
        volatile uint8_t    RX_INTERVAL;
        volatile uint8_t    RESERVED0;
        volatile uint8_t    CONFIG_DATA;
    } HOST_EP0;

    struct
    {
        volatile uint16_t   TX_MAX_P;
        volatile uint16_t   TX_CSR;
        volatile uint16_t   RX_MAX_P;
        volatile uint16_t   RX_CSR;
        volatile uint16_t   RX_COUNT;
        volatile uint8_t    TX_TYPE;
        volatile uint8_t    TX_INTERVAL;
        volatile uint8_t    RX_TYPE;
        volatile uint8_t    RX_INTERVAL;
        volatile uint8_t    RESERVED0;
        volatile uint8_t    FIFO_SIZE;
    } HOST_EPN;

} USB_indexed_csr_t;

typedef struct {
    volatile uint32_t   IRQ;
    volatile uint32_t   CNTL;
    volatile uint32_t   ADDR;
    volatile uint32_t   COUNT;
} USB_DMA_channel;

typedef struct
{
    /*
     * Common USB Registers
     */
    volatile uint8_t    FADDR;
    volatile uint8_t    POWER;
    volatile uint16_t   TX_IRQ;
    volatile uint16_t   RX_IRQ;
    volatile uint16_t   TX_IRQ_ENABLE;
    volatile uint16_t   RX_IRQ_ENABLE;
    volatile uint8_t    USB_IRQ;
    volatile uint8_t    USB_ENABLE;
    volatile uint16_t   FRAME;
    volatile uint8_t    INDEX;
    volatile uint8_t    TEST_MODE;

    /*
     * Indexed CSR
     */
    USB_indexed_csr_t   INDEXED_CSR;

    /*
     * Endpoint FIFOs
     */
    USB_fifo_t      FIFO[16];

    /*
     * OTG, dynamic FIFO and version
     */
    volatile uint8_t    DEV_CTRL;
    volatile uint8_t    MISC;
    volatile uint8_t    TX_FIFO_SIZE;
    volatile uint8_t    RX_FIFO_SIZE;
    volatile uint16_t   TX_FIFO_ADDR;
    volatile uint16_t   RX_FIFO_ADDR;
    volatile uint32_t   VBUS_CSR;
    volatile uint16_t   HW_VERSION;
    volatile uint16_t   RESERVED;

    /*
     * ULPI and configuration registers
     */
    volatile uint8_t    ULPI_VBUS_CTRL;
    volatile uint8_t    ULPI_CARKIT_CTRL;
    volatile uint8_t    ULPI_IRQ_MASK;
    volatile uint8_t    ULPI_IRQ_SRC;
    volatile uint8_t    ULPI_DATA_REG;
    volatile uint8_t    ULPI_ADDR_REG;
    volatile uint8_t    ULPI_CTRL_REG;
    volatile uint8_t    ULPI_RAW_DATA;
    volatile uint8_t    EP_INFO;
    volatile uint8_t    RAM_INFO;
    volatile uint8_t    LINK_INFO;
    volatile uint8_t    VP_LEN;
    volatile uint8_t    HS_EOF1;
    volatile uint8_t    FS_EOF1;
    volatile uint8_t    LS_EOF1;
    volatile uint8_t    SOFT_RST;

    /*
     * Target Address registers
     */
    USB_tar_t       TAR[16];

    /*
     * Endpoints CSR
     */
    USB_endpoint_regs_t ENDPOINT[16];

    /*
     * DMA
     */
    USB_DMA_channel DMA_CHANNEL[8];

    volatile uint32_t   RESERVED_EXT[32];
    volatile uint32_t   RQ_PKT_CNT[16];
    volatile uint16_t   RX_DPBUF_DIS;
    volatile uint16_t   TX_DPBUF_DIS;
    volatile uint16_t   C_T_UCH;
    volatile uint16_t   C_T_HHSRTN;
    volatile uint16_t   C_T_HSBT;

} MSS_USB_TypeDef;

#define USB                     ((MSS_USB_TypeDef *) USB_BASE)
#define USB_BASE                0x20201000u

#endif /*__MSS_USB_CORE_REGS_H_*/
