/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC MSS USB Driver Stack
 *      USB Core Interface Layer (USB-CIFL)
 *          USB-CIF driver
 *
 * USB-CIF driver public API.
 *
 */

#ifndef __MSS_USB_COMMON_CIF_H_
#define __MSS_USB_COMMON_CIF_H_

#include "mss_usb_config.h"
#include "mss_usb_core_regs.h"

#define __INLINE        inline

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*//**
  Constant values internally used by the driver.
 */
#define CEP_MAX_PKT_SIZE                                64u
#define SETUP_PKT_SIZE                                  8u

#define DPB_DISABLE                                     0u
#define DPB_ENABLE                                      1u

#ifdef MSS_USB_HOST_ENABLED
#define SUSPENDM_DISABLE                                0x0000u
#define SUSPENDM_ENABLE                                 0x0001u
#endif

#define MSS_USB_WORD_BIT_0_MASK                         0x0001u
#define MSS_USB_BYTE_BIT_0_MASK                         0x01u

#define MSS_USB_BOOLEAN_FALSE                           0x00u
#define MSS_USB_BOOLEAN_TRUE                            0x01u

#define TX_EP_UNDER_RUN_ERROR                           0x01u
#define TX_EP_STALL_ERROR                               0x02u

#define RX_EP_OVER_RUN_ERROR                            0x01u
#define RX_EP_STALL_ERROR                               0x02u
#define RX_EP_DATA_ERROR                                0x04u
#define RX_EP_PID_ERROR                                 0x06u
#define RX_EP_ISO_INCOMP_ERROR                          0x08u

#define CTRL_EP_SETUP_END_ERROR                         0x01u
#define CTRL_EP_STALL_ERROR                             0x02u

#define DMA_XFR_ERROR                                   0x40u

#define MIN_EP_FIFO_SZ                                  0x0008u
#define EP_FIFO_ADDR_STEP                               0x0008u

#define DMA_DISABLE                                     0u
#define DMA_ENABLE                                      1u

#define NO_ZLP_TO_XFR                                   0u
#define ADD_ZLP_TO_XFR                                  1u

/*-------------------------------------------------------------------------*//**
  INTRUSBE register - USB interrupts masks
 */
#define SUSPEND_IRQ_MASK                                0x01u
#define RESUME_IRQ_MASK                                 0x02u
#define RESET_IRQ_MASK                                  0x04u   /*Device mode*/
#define BABBLE_IRQ_MASK                                 0x04u   /*Host mode*/
#define SOF_IRQ_MASK                                    0x08u
#define CONNECT_IRQ_MASK                                0x10u
#define DISCONNECT_IRQ_MASK                             0x20u
#define SESSION_REQUEST_IRQ_MASK                        0x40u
#define VBUS_ERROR_IRQ_MASK                             0x80u

/*-------------------------------------------------------------------------*//**
  Types which can be used by LDL layer or the application.
 */

typedef enum {
    MSS_USB_XFR_CONTROL,
    MSS_USB_XFR_ISO,
    MSS_USB_XFR_BULK,
    MSS_USB_XFR_INTERRUPT,
    MSS_USB_XFR_HB_INTERRUPT,
    MSS_USB_XFR_HB_ISO
} mss_usb_xfr_type_t;

typedef enum {
    MSS_USB_DEVICE_HS,
    MSS_USB_DEVICE_FS,
    MSS_USB_DEVICE_LS
} mss_usb_device_speed_t;

typedef enum {
    MSS_USB_CEP = 0,
    MSS_USB_TX_EP_1,
    MSS_USB_TX_EP_2,
    MSS_USB_TX_EP_3,
    MSS_USB_TX_EP_4,

    MSS_USB_RX_EP_1 = 1,
    MSS_USB_RX_EP_2,
    MSS_USB_RX_EP_3,
    MSS_USB_RX_EP_4
} mss_usb_ep_num_t;

typedef enum {
    MSS_USB_DMA_CHANNEL1,
    MSS_USB_DMA_CHANNEL2,
    MSS_USB_DMA_CHANNEL3,
    MSS_USB_DMA_CHANNEL4,
    MSS_USB_DMA_CHANNEL_NA
} mss_usb_dma_channel_t;

/*-------------------------------------------------------------------------*//**
  Types which are used internally by the driver.
 */

/* Device mode:  states of the device */
typedef enum {
    MSS_USB_NOT_ATTACHED_STATE,
    MSS_USB_ATTACHED_STATE,
    MSS_USB_POWERED_STATE,
    MSS_USB_DEFAULT_STATE,
    MSS_USB_ADDRESS_STATE,
    MSS_USB_CONFIGURED_STATE,
    MSS_USB_SUSPENDED_STATE
} mss_usb_state_t;

typedef enum {
    MSS_USB_CORE_MODE_HOST,
    MSS_USB_CORE_MODE_DEVICE
} mss_usb_core_mode_t;

typedef enum {
    MSS_USB_EP_VALID = 0u,
    MSS_USB_EP_STALLED,
    MSS_USB_EP_NAK,
    MSS_USB_EP_NYET,
    MSS_USB_CEP_IDLE,
    MSS_USB_CEP_SETUP,
    MSS_USB_CEP_TX,
    MSS_USB_CEP_RX,

#ifdef MSS_USB_HOST_ENABLED
    MSS_USB_CEP_STATUS_AFTER_IN,
    MSS_USB_CEP_STATUS_AFTER_OUT,
    MSS_USB_EP_TXN_SUCCESS,
    MSS_USB_EP_NAK_TOUT,
    MSS_USB_EP_NO_RESPONSE,
    MSS_USB_EP_STALL_RCVD,
    MSS_USB_EP_XFR_SUCCESS,
    MSS_USB_EP_ABORTED
#endif  /* MSS_USB_HOST_ENABLED */

} mss_usb_ep_state_t;

typedef enum mss_usb_pkt_type {
    MSS_USB_SETUP_PKT,
    MSS_USB_IN_DATA_PKT,
    MSS_USB_OUT_DATA_PKT,
    MSS_USB_STATUS_PKT_AFTER_IN,
    MSS_USB_STATUS_PKT_AFTER_OUT
}mss_usb_pkt_type_t;

 /*
  Type of device - Detected through DevCTL.D7 register bit depending
  on the type of connector connected to on-board receptacle.
  */
typedef enum mss_usb_device_role {
    MSS_USB_DEVICE_ROLE_DEVICE_A,
    MSS_USB_DEVICE_ROLE_DEVICE_B
} mss_usb_device_role_t;

typedef enum {
    MSS_USB_DMA_WRITE,
    MSS_USB_DMA_READ
} mss_usb_dma_dir_t;

typedef enum
{
    MSS_USB_DMA_MODE0=0,
    MSS_USB_DMA_MODE1=1
} mss_usb_dma_mode_t;

typedef enum {
    MSS_USB_DMA_BURST_MODE0 = 0,
    MSS_USB_DMA_BURST_MODE1,
    MSS_USB_DMA_BURST_MODE2,
    MSS_USB_DMA_BURST_MODE3
} mss_usb_dma_burst_mode_t;

typedef enum {
    VBUS_BLOW_SESSIONEND,
    VBUS_ABV_SESSIONEND_BLOW_AVALID,
    VBUS_ABV_AVALID_BLOW_VB_VALID,
    VBUS_ABV_VB_VALID
} mss_usb_vbus_level_t;

#ifdef MSS_USB_DEVICE_ENABLED
typedef enum {
    MSS_USB_CTRL_EP_IDLE,
    MSS_USB_CTRL_EP_TX,
    MSS_USB_CTRL_EP_RX
} mss_usbd_cep_state_t;
#endif  /* MSS_USB_DEVICE_ENABLED */

/*-------------------------------------------------------------------------*//**
  Data structures of USB-CIFL which are shared with USB-LL.
 */
typedef struct {
    /*EP configuration info*/
    mss_usb_ep_num_t            num;

    uint8_t                     dpb_enable;         /*0 or 1*/
    uint16_t                    fifo_size;          /*number of bytes*/
    uint16_t                    fifo_addr;          /*number of bytes*/
    uint8_t                     dma_enable;
    mss_usb_dma_channel_t       dma_channel;
    uint16_t                    max_pkt_size;       /*Maxpktsize register value*/

    uint8_t                     stall;
    mss_usb_ep_state_t          state;

    /*EP data Transfer related info*/
    mss_usb_xfr_type_t          xfr_type;
    uint32_t                    add_zlp;

    /*
     Number of pkts in one uFrame in case of Interrupt/ISO HB transfers. Number
     of split packets in case of Bulk transfers.Should always be more than 0.
     */
    uint8_t                     num_usb_pkt;
    uint8_t*                    buf_addr;

    /*
     Transfer level info, used mainly for control transfer where the
     total length of data transfer is prior know through setup transaction
     In case of bulk transfer with Autospliting/amalgamation, this value is used
     when length of transfer is bigger than one amalgamated packet.
     */
    uint32_t                    xfr_length;
    uint32_t                    xfr_count;

    /*
     Single packet Transaction level info
     In case of bulk transfer with Autospliting/amalgamation, this value
     represents the amalgamated packet.
     */
    uint32_t                    txn_length;
    uint32_t                    txn_count;

#ifdef MSS_USB_HOST_ENABLED
    /*
    Manual toggle enable is required only when dynamic switching of EP is supported.
    We don't support it.
    HubAddr and HubPortNum registers are needed for talking to LS/FS devices
    connected through Hub - We don't support hub connected devices yet.
    LS/FS device directly connected to SF2 is supported though.
    */
    uint8_t                     cep_data_dir;
    uint8_t*                    cep_cmd_addr;
    uint8_t                     disable_ping;       /*Depends on target speed*/
    uint32_t                    req_pkt_n;          /*No of IN packets*/
    /*
     Interval Must be in terms of frame/uframe. Indicates NAKLIMIT0 register value
     for EP0. TX/RXInterval register value for TX/RX EP.
     */
    uint32_t                    interval;
    /*This index will be used to choose a particular connected device out of the
      Multiple connected devices when Multiple devices are supported.
      Currently we support only one device hence this will always evaluate to 0*/
    uint8_t                     tdev_idx;

#endif  /* MSS_USB_HOST_ENABLED */

}mss_usb_ep_t;

/*-------------------------------------------------------------------------*//**
  Data structures which are used internally by the driver.
 */
/*Device mode configuration information*/
typedef struct {
    uint8_t                        device_addr;
    uint8_t                        device_total_interfaces;
    uint8_t                        device_total_ep;
    mss_usb_state_t                device_state;
    mss_usb_state_t                device_state_at_suspend;
    uint8_t                        device_status;
    mss_usb_device_speed_t         device_speed;    /*USB speed in Device mode*/
    uint8_t                        active_config_num;      /*SetConfig command*/
    uint8_t                        active_interface_num;/*SetInterface command*/
    uint16_t                       config_feature_status;
    uint8_t                        remote_wakeup;
} mss_usbd_dev_conf_t;

/* Device mode call-back function called from CIF layer */
typedef struct {
    void (*usbd_ep_rx)( mss_usb_ep_num_t num, uint8_t status );
    void (*usbd_ep_tx_complete)( mss_usb_ep_num_t num, uint8_t status );

    void (*usbd_cep_setup)( uint8_t status );
    void (*usbd_cep_rx)( uint8_t status );
    void (*usbd_cep_tx_complete)( uint8_t status );

    void (*usbd_sof)( uint8_t status );
    void (*usbd_reset)( void );
    void (*usbd_suspend)( void );
    void (*usbd_resume)( void );
    void (*usbd_disconnect)( void );
    void (*usbd_dma_handler)(mss_usb_ep_num_t ep_num, mss_usb_dma_dir_t dma_dir,
                             uint8_t status, uint32_t dma_addr_val);
} mss_usbd_cb_t;

/* MSS USB Hardware core information. This is read-only */
typedef struct {
    uint8_t core_max_nbr_of_tx_ep;
    uint8_t core_max_nbr_of_rx_ep;
    uint8_t core_max_nbr_of_dma_chan;
    uint8_t core_ram_bus_width;
    uint8_t core_WTCON;
    uint8_t core_WTID;
    uint8_t core_VPLEN;
    uint8_t core_HS_EOF1;
    uint8_t core_FS_EOF1;
    uint8_t core_LS_EOF1;
    uint8_t core_configdata;
} mss_usb_core_info_t;

/* Internal DMA Configuration data */
typedef struct {
    uint8_t dma_channel;
    uint8_t dma_dir;
    uint8_t dma_assigned_ep;
    uint8_t dma_mode;
    uint8_t dma_burst_mode;
    uint8_t dma_status;
} mss_usb_dma_t;


/*----------------------------------------------------------------------------*/
/*_------------------------USB-CIF Public APIs--------------------------------*/
/*----------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*//**
 */
static __INLINE void MSS_USB_CIF_cep_flush_fifo(void)
{
    USB->INDEXED_CSR.DEVICE_EP0.CSR0 |= CSR0H_DEV_FLUSH_FIFO_MASK;
}

/*-------------------------------------------------------------------------*//**
*/
static __INLINE void MSS_USB_CIF_tx_ep_flush_fifo(mss_usb_ep_num_t ep_num)
{
    USB->ENDPOINT[ep_num].TX_CSR |= TxCSRL_REG_EPN_FLUSH_FIFO_MASK;
}

/*-------------------------------------------------------------------------*//**
*/
static __INLINE void MSS_USB_CIF_rx_ep_flush_fifo(mss_usb_ep_num_t ep_num)
{
    USB->ENDPOINT[ep_num].RX_CSR |= RxCSRL_REG_EPN_FLUSH_FIFO_MASK;
}

/*-------------------------------------------------------------------------*//**
*/
static __INLINE uint8_t MSS_USB_CIF_rx_ep_is_fifo_full(mss_usb_ep_num_t ep_num)
{
    return(((USB->ENDPOINT[ep_num].RX_CSR & RxCSRL_REG_EPN_RX_FIFO_FULL_MASK) ?
                                    MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

/*-------------------------------------------------------------------------*//**
 * Enables USB interrupts.
 */
static __INLINE void MSS_USB_CIF_enable_usbirq(uint8_t irq_mask)
{
    USB->USB_ENABLE |= (irq_mask);
}

/*-------------------------------------------------------------------------*//**
  Disables USB interrupts.
 */
static __INLINE void MSS_USB_CIF_disable_usbirq(uint8_t irq_mask)
{
    USB->USB_ENABLE &= ~(irq_mask);
}
/*-------------------------------------------------------------------------*//**
  Indicates that there is at least one byte available to be transmitted from
  TX FIFO
 */
static __INLINE uint8_t MSS_USB_CIF_is_txepfifo_notempty(mss_usb_ep_num_t ep_num)
{
    return(((USB->ENDPOINT[ep_num].TX_CSR & TxCSRL_REG_EPN_TX_FIFO_NE_MASK)
            ? 1u : 0u));
}

/*-------------------------------------------------------------------------*//**
 */
static __INLINE void
MSS_USB_CIF_cep_enable_irq
(
    void
)
{
    USB->TX_IRQ_ENABLE |= (uint16_t)(TX_IRQ_ENABLE_REG_CEP_MASK);
}

/*-------------------------------------------------------------------------*//**
 */
static __INLINE void
MSS_USB_CIF_cep_disable_irq
(
    void
)
{
    USB->TX_IRQ_ENABLE &= (uint16_t)(~TX_IRQ_ENABLE_REG_CEP_MASK);
}

/*-------------------------------------------------------------------------*//**
  INDEX register related APIs
 */
static __INLINE void
MSS_USB_CIF_set_index_reg
(
    uint8_t index
)
{
    USB->INDEX = index;
}

static __INLINE void MSS_USB_CIF_start_testse0nak(void)
{
    USB->TEST_MODE = TESTMODE_SE0NAK_MASK;
}

static __INLINE void MSS_USB_CIF_start_testj(void)
{
    USB->TEST_MODE = TESTMODE_TESTJ_MASK;
}

static __INLINE void MSS_USB_CIF_start_testk(void)
{
    USB->TEST_MODE = TESTMODE_TESTK_MASK;
}

static __INLINE void MSS_USB_CIF_start_testpacket_bit(void)
{
    USB->TEST_MODE = TESTMODE_TESTPACKET_MASK;
}

static __INLINE void MSS_USB_CIF_start_forcehost_ena(void)
{
    USB->TEST_MODE = TESTMODE_FORCEHOST_MASK | TESTMODE_FORCEHS_MASK;
}

static __INLINE void MSS_USB_CIF_end_testmode(void)
{
    USB->TEST_MODE = 0x00U;
}

/*-------------------------------------------------------------------------*//**
 */
void
MSS_USB_CIF_configure_ep_dma
(
    mss_usb_dma_channel_t channel,
    mss_usb_dma_dir_t dir,
    mss_usb_dma_mode_t dma_mode,
    mss_usb_dma_burst_mode_t burst_mode,
    mss_usb_ep_num_t ep_num,
    uint32_t buf_addr
);

/*-------------------------------------------------------------------------*//**
 */
void
MSS_USB_CIF_tx_ep_configure
(
    mss_usb_ep_t* core_ep
);

/*-------------------------------------------------------------------------*//**
 */
void
MSS_USB_CIF_rx_ep_configure
(
    mss_usb_ep_t* core_ep
);

/*-------------------------------------------------------------------------*//**
  Prepares the RX EP for receiving data as per parameters provided by upper
  layer
 */
void
MSS_USB_CIF_rx_ep_read_prepare
(
    mss_usb_ep_num_t ep_num,
    uint8_t* buf_addr,
    uint8_t dma_enable,
    mss_usb_dma_channel_t dma_channel,
    mss_usb_xfr_type_t xfr_type,
    uint32_t xfr_length
);

/*-------------------------------------------------------------------------*//**
 */
void
MSS_USB_CIF_ep_write_pkt
(
    mss_usb_ep_num_t ep_num,
    uint8_t* buf_addr,
    uint8_t dma_enable,
    mss_usb_dma_channel_t dma_channel,
    mss_usb_xfr_type_t xfr_type,
    uint32_t xfr_length,
    uint32_t txn_length
);


/*-------------------------------------------------------------------------*//**
  USB2.0 test mode functions
*/
void
MSS_USB_CIF_start_testpacket
(
    void
);

#ifdef __cplusplus
}
#endif

#endif /*__MSS_USB_COMMON_CIF_H_*/
