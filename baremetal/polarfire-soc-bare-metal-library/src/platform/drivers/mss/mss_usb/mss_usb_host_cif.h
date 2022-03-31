/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Microchip PolarFire SoC MSS USB Driver Stack
 *      USB Core Interface Layer (USB-CIFL)
 *          USBH-CIF driver
 *
 * USBH-CIF driver public API.
 *
 * SVN $Revision$
 * SVN $Date$
 */

#ifndef __MSS_USB_HOST_CIF_H_
#define __MSS_USB_HOST_CIF_H_

#include "mss_usb_common_cif.h"
#include "mss_usb_core_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_HOST_ENABLED

/*-------------------------------------------------------------------------*//**
  Constant values which can be used by class component or the application.
 */
#define USB_HS_BULK_MAX_PKT_SIZE                    512u
#define USB_HS_INTERRUPT_MAX_PKT_SIZE               1024u
#define USB_HS_ISO_MAX_PKT_SIZE                     1024u

#define USB_FS_BULK_MAX_PKT_SIZE                    64u
#define USB_FS_INTERRUPT_MAX_PKT_SIZE               64u
#define USB_FS_ISO_MAX_PKT_SIZE                     1023u

typedef struct {
    void(*usbh_tx_complete)(uint8_t ep_num, uint8_t status);
    void(*usbh_rx)(uint8_t ep_num, uint8_t status);
    void(*usbh_cep)(uint8_t status);
    void(*usbh_sof)(uint32_t frame_number);
    void(*usbh_connect)(mss_usb_device_speed_t target_speed,
                        mss_usb_vbus_level_t vbus_level);

    void(*usbh_disconnect)(void);
    void(*usbh_vbus_error)(mss_usb_vbus_level_t);
    void(*usbh_babble_error)(void);
    void(*usbh_session_request)(void);
    void(*usbh_dma_handler)(mss_usb_ep_num_t ep_num, mss_usb_dma_dir_t dma_dir,
                            uint8_t status, uint32_t dma_addr_val);
} mss_usbh_cb_t;

/*--------------------------------Public APIs---------------------------------*/

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_init()

  @param

  @return

  Example:
  @code
  @endcode
*/
void
MSS_USBH_CIF_init
(
    void
);

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_handle_connect_irq()

  @param

  @return

  Example:
  @code
  @endcode
*/
void
MSS_USBH_CIF_handle_connect_irq
(
    void
);

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_cep_configure()

  @param

  @return

  Example:
  @code
  @endcode
*/
void
MSS_USBH_CIF_cep_configure
(
    mss_usb_ep_t* cep
);

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_tx_ep_configure()

  @param

  @return

  Example:
  @code
  @endcode
*/
void
MSS_USBH_CIF_tx_ep_configure
(
    mss_usb_ep_t* host_ep
);

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_rx_ep_configure()

  @param

  @return

  Example:
  @code
  @endcode
*/
void
MSS_USBH_CIF_rx_ep_configure
(
    mss_usb_ep_t* host_ep
);

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_bus_suspend()

  @param

  @return

  Example:
  @code
  @endcode
*/
void
MSS_USBH_CIF_bus_suspend
(
    uint32_t enable_suspendm
);

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_bus_resume()

  @param

  @return

  Example:
  @code
  @endcode
*/
void
MSS_USBH_CIF_bus_resume
(
    void
);

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_read_vbus_level()

  @param

  @return

  Example:
  @code
  @endcode
*/
mss_usb_vbus_level_t
MSS_USBH_CIF_read_vbus_level
(
    void
);

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_ep_tx_start()

  @param

  @return

  Example:
  @code
  @endcode
*/
void
MSS_USBH_CIF_ep_tx_start
(
    mss_usb_ep_t* host_ep
);

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_tx_ep_mp_configure()

  @param

  @return

  Example:
  @code
  @endcode
*/
uint32_t
MSS_USBH_CIF_tx_ep_mp_configure
(
    uint8_t outpipe_num,
    uint8_t tdev_ep_num,
    uint8_t tdev_addr,
    uint8_t tdev_hub_addr,
    uint8_t tdev_hub_port,
    uint8_t tdev_hub_mtt,
    mss_usb_device_speed_t speed,
    uint32_t tdev_interval,
    mss_usb_xfr_type_t xfr_type
);

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_rx_ep_mp_configure()

  @param

  @return

  Example:
  @code
  @endcode
*/
uint32_t
MSS_USBH_CIF_rx_ep_mp_configure
(
    uint8_t inpipe_num,
    uint8_t tdev_ep_num,
    uint8_t tdev_addr,
    uint8_t tdev_hub_addr,
    uint8_t tdev_hub_port,
    uint8_t tdev_hub_mtt,
    mss_usb_device_speed_t speed,
    uint32_t tdev_interval,
    mss_usb_xfr_type_t xfr_type
);

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_cep_write_pkt()

  @param

  @return

  Example:
  @code
  @endcode
*/
void
MSS_USBH_CIF_cep_write_pkt
(
    mss_usb_ep_t* cep
);

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_cep_start_xfr()

  @param

  @return

  Example:
  @code
  @endcode
*/
void
MSS_USBH_CIF_cep_start_xfr
(
    mss_usb_ep_state_t state
);

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_cep_abort_xfr()

  @param

  @return

  Example:
  @code
  @endcode
*/
void
MSS_USBH_CIF_cep_abort_xfr
(
    void
);

/*-------------------------------------------------------------------------*//**
  MSS_USBH_CIF_cep_read_pkt()

  @param

  @return

  Example:
  @code
  @endcode
*/
void
MSS_USBH_CIF_cep_read_pkt
(
    mss_usb_ep_t* hcep
);

/*-------------------------------------------------------------------------*//**
  Static __Inline functions
 */
static __INLINE void MSS_USBH_CIF_assert_bus_reset(void)
{
    USB->POWER |= POWER_REG_BUS_RESET_SIGNAL_MASK;
}

static __INLINE void MSS_USBH_CIF_clr_bus_reset(void)
{
    USB->POWER &= ~POWER_REG_BUS_RESET_SIGNAL_MASK;
}

static __INLINE void MSS_USBH_CIF_assert_bus_resume(void)
{
    USB->POWER |= POWER_REG_RESUME_SIGNAL_MASK;
    /*clear after min. 20ms*/
}

static __INLINE void MSS_USBH_CIF_clr_bus_resume(void)
{
    USB->POWER &= ~POWER_REG_RESUME_SIGNAL_MASK;
}

static __INLINE void MSS_USBH_CIF_enable_suspendm_out(void)
{
    USB->POWER |= POWER_REG_ENABLE_SUSPENDM_MASK;
}

static __INLINE void MSS_USBH_CIF_disable_suspendm_out(void)
{
    USB->POWER &= ~POWER_REG_ENABLE_SUSPENDM_MASK;
}

static __INLINE void MSS_USBH_CIF_assert_suspend_bus(void)
{
    USB->POWER |= POWER_REG_SUSPEND_MODE_MASK;
}

static __INLINE void MSS_USBH_CIF_clr_suspend_bus(void)
{
    USB->POWER &= ~POWER_REG_SUSPEND_MODE_MASK;
}

static __INLINE uint8_t MSS_USBH_CIF_is_host_suspended(void)
{
    return (((USB->POWER & POWER_REG_SUSPEND_MODE_MASK) ? MSS_USB_BOOLEAN_TRUE
                                                        : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void MSS_USBH_CIF_end_session(void)
{
    USB->DEV_CTRL &= ~DEV_CTRL_SESSION_MASK;
}
/*-------------------------------------------------------------------------*//**
  DEVCTL register related APIs
 */
static __INLINE uint8_t MSS_USBH_CIF_is_target_ls(void)
{
    return (((USB->DEV_CTRL & DEV_CTRL_LS_DEV_MASK) ? MSS_USB_BOOLEAN_TRUE
                                                    : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE uint8_t MSS_USBH_CIF_is_target_fs(void)
{
    return (((USB->DEV_CTRL & DEV_CTRL_FS_DEV_MASK) ? MSS_USB_BOOLEAN_TRUE
                                                    : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE uint8_t MSS_USBH_CIF_is_hs_mode(void)
{
    return(((USB->POWER & POWER_REG_HS_MODE_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

/*-------------------------------------------------------------------------*//**
  CSR0L register related APIs
 */
static __INLINE void
MSS_USBH_CIF_cep_clr_rxpktrdy(void)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR &= ~CSR0L_HOST_RX_PKT_RDY_MASK;
}

static __INLINE void MSS_USBH_CIF_cep_set_request_in_pkt(void)
{
    /* Cleared when RxPktRdy is cleared */
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR |= CSR0L_HOST_IN_PKT_REQ_MASK;
}

static __INLINE void MSS_USBH_CIF_cep_set_setuppktonly(void)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR |= (CSR0L_HOST_SETUP_PKT_MASK);
}
static __INLINE void MSS_USBH_CIF_cep_set_setuppktrdy(void)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR |= (CSR0L_HOST_SETUP_PKT_MASK |
                                              CSR0L_HOST_TX_PKT_RDY_MASK);
}

static __INLINE void MSS_USBH_CIF_cep_set_statuspktrdy_after_out(void)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR |= (CSR0L_HOST_STATUS_PKT_MASK |
                                              CSR0L_HOST_IN_PKT_REQ_MASK);
}

static __INLINE void MSS_USBH_CIF_cep_set_statuspktrdy_after_in(void)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR |= (CSR0L_HOST_STATUS_PKT_MASK |
                                              CSR0L_HOST_TX_PKT_RDY_MASK);
}

static __INLINE void MSS_USBH_CIF_cep_clr_statusRxpktrdy(void)
{
    if(CSR0L_HOST_RX_PKT_RDY_MASK | USB->ENDPOINT[MSS_USB_CEP].TX_CSR)
    {
        USB->ENDPOINT[MSS_USB_CEP].TX_CSR &= ~(CSR0L_HOST_STATUS_PKT_MASK |
                                                  CSR0L_HOST_RX_PKT_RDY_MASK);
    }
    else
    {
        USB->ENDPOINT[MSS_USB_CEP].TX_CSR &= ~(CSR0L_HOST_STATUS_PKT_MASK);
    }
}

static __INLINE uint16_t MSS_USBH_CIF_cep_read_csr_reg(void)
{
    return(USB->ENDPOINT[MSS_USB_CEP].TX_CSR);
}

static __INLINE void MSS_USBH_CIF_cep_set_txpktrdy(void)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR |= CSR0L_HOST_TX_PKT_RDY_MASK;
}

static __INLINE void
MSS_USBH_CIF_cep_clr_data_tog
(
    void
)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR &= ~CSR0H_HOST_DATA_TOG_MASK;
}

static __INLINE void
MSS_USBH_CIF_cep_set_data_tog_we
(
    void
)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR &= ~CSR0H_HOST_DATA_TOG_WE_MASK;
}

static __INLINE void
MSS_USBH_CIF_cep_clr_data_tog_we
(
    void
)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR &= ~CSR0H_HOST_DATA_TOG_WE_MASK;
}

static __INLINE void
MSS_USBH_CIF_cep_set_data_tog
(
    void
)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_CSR |= CSR0H_HOST_DATA_TOG_MASK;
}

/*-------------------------------------------------------------------------*//**
  FIFOx register related APIs
 */
static __INLINE void MSS_USBH_CIF_load_tx_fifo(mss_usb_ep_num_t ep_num,
                                               void * in_data,
                                               uint32_t length)
{
    uint32_t idx;
    uint32_t* temp;
    uint8_t* temp_8bit;

    uint16_t words = length / 4;
    temp =in_data;
    temp_8bit = in_data;

    for(idx = 0u; idx < words; ++idx)
    {
        USB->FIFO[ep_num].WORD.VALUE = (uint32_t)temp[idx];
    }

    for(idx = (length - (length % 4)); idx < length; ++idx)
    {
        USB->FIFO[ep_num].BYTE.VALUE = (uint8_t)temp_8bit[idx];
    }
}

static __INLINE void MSS_USBH_CIF_read_rx_fifo(mss_usb_ep_num_t ep_num,
                                               void * out_data,
                                               uint32_t length)
{
    uint32_t idx;
    uint32_t* temp;
    uint8_t* temp_8bit;

    uint16_t words = length / 4;
    temp = out_data;
    temp_8bit = out_data;

    for(idx = 0u; idx < words; ++idx)
    {
        temp[idx] = USB->FIFO[ep_num].WORD.VALUE;
    }

    for(idx = (length - (length % 4u)); idx < length; ++idx)
    {
        temp_8bit[idx] = USB->FIFO[ep_num].BYTE.VALUE;
    }
}

/*-------------------------------------------------------------------------*//**
  TXFUNCADDR register related APIs
 */
static __INLINE void
MSS_USBH_CIF_tx_ep_set_target_func_addr
(
    mss_usb_ep_num_t ep_num,
    uint8_t addr
)
{
    /*
     * Device number of the target - initially zero, then determined by
     * enumeration process.
     */
    USB->TAR[ep_num].TX_FUNC_ADDR = (addr & 0x7Fu);
}

/*-------------------------------------------------------------------------*//**
  TYPE0 register related APIs
 */
static __INLINE void
MSS_USBH_CIF_cep_set_type0_reg
(
    mss_usb_device_speed_t speed
)
{
    USB->ENDPOINT[MSS_USB_CEP].TX_TYPE = (speed << TYPE0_HOST_MP_TARGET_SPEED_SHIFT);
}

static __INLINE mss_usb_device_speed_t
MSS_USBH_CIF_cep_get_type0_reg
(
    void
)
{
    return ((mss_usb_device_speed_t)(USB->ENDPOINT[MSS_USB_CEP].TX_TYPE >>
                                     TYPE0_HOST_MP_TARGET_SPEED_SHIFT));
}

static __INLINE void
MSS_USBH_CIF_rx_ep_set_reqpkt
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR |= RXCSRL_HOST_EPN_IN_PKT_REQ_MASK;
}

static __INLINE void
MSS_USBH_CIF_rx_ep_clr_reqpkt
(
    mss_usb_ep_num_t ep_num
)
{
    USB->ENDPOINT[ep_num].RX_CSR &= ~RXCSRL_HOST_EPN_IN_PKT_REQ_MASK;
}

static __INLINE uint8_t
MSS_USBH_CIF_rx_ep_is_reqpkt
(
    mss_usb_ep_num_t ep_num
)
{
    return((USB->ENDPOINT[ep_num].RX_CSR &RXCSRL_HOST_EPN_IN_PKT_REQ_MASK)? MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE);
}
#endif /* MSS_USB_HOST_ENABLED */

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_HOST_CIF_H_ */
