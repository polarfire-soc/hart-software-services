/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC MSS USB Driver Stack
 *      USB Core Interface Layer (USB-CIFL)
 *          USBD-CIF driver
 *
 * USBD-CIF driver public API.
 *
 */

#ifndef __MSS_USB_DEVICE_CIF_H_
#define __MSS_USB_DEVICE_CIF_H_

#include "mss_usb_core_regs.h"
#include "mss_usb_common_reg_io.h"
#include "mss_usb_device_reg_io.h"
#include "mss_usb_common_cif.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSS_USB_DEVICE_ENABLED

/*******************************************************************************
 Exported APIs from USBD-CIF
 */
void MSS_USBD_CIF_get_hwcore_info(mss_usb_core_info_t* hw_core);

/* DeviceInfo register related APIs */

/*lint -e20 -e522 -e10 -e40 -e63 -e35 -e26 -e78*/

static __INLINE void MSS_USBD_CIF_set_dev_addr(uint8_t addr)
{
    USB->FADDR = addr;
}

static __INLINE uint8_t MSS_USBD_CIF_get_dev_addr(void)
{
    return(USB->FADDR);
}

static __INLINE void MSS_USBD_CIF_force_resume(void)
{
    MSS_USBD_CIF_resume_gen();
}

/*ISO INEP Will wait for SOF packet after TxPktRdy is set*/
static __INLINE void MSS_USBD_CIF_set_isoupdate(void)
{
    USB->POWER |= POWER_REG_ISO_UPDATE_MASK;
}

/*ISO INEP Will NOT wait for SOF packet after TxPktRdy is set*/
static __INLINE void MSS_USBD_CIF_clr_isoupdate(void)
{
    USB->POWER &= ~POWER_REG_ISO_UPDATE_MASK;
}

static __INLINE void MSS_USBD_CIF_rx_ep_clr_stall(mss_usb_ep_num_t ep_num)
{
    MSS_USB_CIF_rx_ep_clr_send_stall_bit(ep_num);

    /*TODO: Confirm...this is required for clear feature command*/
    MSS_USB_CIF_rx_ep_clr_data_tog(ep_num);
}


static __INLINE void MSS_USBD_CIF_rx_ep_stall(mss_usb_ep_num_t ep_num)
{
    MSS_USB_CIF_rx_ep_set_send_stall_bit(ep_num);
}

static __INLINE void MSS_USBD_CIF_tx_ep_clr_stall(mss_usb_ep_num_t ep_num)
{
    MSS_USB_CIF_tx_ep_clr_send_stall_bit(ep_num);

    /*
    TODO: Confirm...this is required for clear feature command
    Lakeview page:62
    */
    MSS_USB_CIF_tx_ep_clr_data_tog(ep_num);
}

static __INLINE void MSS_USBD_CIF_tx_ep_stall(mss_usb_ep_num_t ep_num)
{
    MSS_USB_CIF_tx_ep_set_send_stall_bit(ep_num);
}

/*
 Soft connect -- D+ and D- lines are connected to the bus Valid in Device
 mode only.
 */

static __INLINE void MSS_USBD_CIF_dev_connect(void)
{
    USB->POWER |= POWER_REG_SOFT_CONN_MASK;
}

static __INLINE void MSS_USBD_CIF_dev_disconnect(void)
{
    USB->POWER &= ~POWER_REG_SOFT_CONN_MASK;
}

/*
 In Device mode, only FS or HS are possible.
 In Host mode, becomes valid after reset bit is cleared.
 Remains valid till session ends
*/
static __INLINE uint8_t MSS_USBD_CIF_is_hs_mode(void)
{
    return(((USB->POWER & POWER_REG_HS_MODE_MASK) ?
                                MSS_USB_BOOLEAN_TRUE : MSS_USB_BOOLEAN_FALSE));
}

static __INLINE void MSS_USBD_CIF_cep_end_zdr(void)
{
    USB->INDEXED_CSR.DEVICE_EP0.CSR0 = (CSR0L_DEV_SERVICED_RX_PKT_RDY_MASK |
                                        CSR0L_DEV_DATA_END_MASK);
}

static __INLINE void MSS_USBD_CIF_cep_clr_rxpktrdy(void)
{
    /*Setting SERVICED_RX_PKT_RDY clears RxPktRdy bit*/
    USB->INDEXED_CSR.DEVICE_EP0.CSR0 = CSR0L_DEV_SERVICED_RX_PKT_RDY_MASK;
}

static __INLINE void MSS_USBD_CIF_cep_stall(void)
{
    USB->INDEXED_CSR.DEVICE_EP0.CSR0 = (CSR0L_DEV_SEND_STALL_MASK |
                                         CSR0L_DEV_SERVICED_RX_PKT_RDY_MASK);
}
/*write Data Req, host writing on device (USB OUT) */
static __INLINE void MSS_USBD_CIF_cep_end_wdr(void)
{
    USB->INDEXED_CSR.DEVICE_EP0.CSR0 = (CSR0L_DEV_SERVICED_RX_PKT_RDY_MASK |
                                        CSR0L_DEV_DATA_END_MASK);
}

/*Read Data Req, host reading from device (USB IN)*/
/*Taken care by MSS_USBD_CIF_cep_write_pkt
TODO: Check with Different Optimization levels*/
static __INLINE void MSS_USBD_CIF_cep_end_rdr(void)
{
    USB->INDEXED_CSR.DEVICE_EP0.CSR0 = (CSR0L_DEV_TX_PKT_RDY_MASK |
                                        CSR0L_DEV_DATA_END_MASK);
}

static __INLINE void MSS_USBD_CIF_reset_index_reg(void)
{
    USB->INDEX = 0u;
}

/*lint -restore */
/***************************************************************************//**

*/
void MSS_USBD_CIF_init(mss_usb_device_speed_t speed);

/***************************************************************************//**

*/
void MSS_USBD_CIF_cep_configure(void);

/***************************************************************************//**

*/
void MSS_USBD_CIF_cep_rx_prepare(mss_usb_ep_t* device_ep);

/***************************************************************************//**

*/
void MSS_USBD_CIF_rx_ep_read_prepare(mss_usb_ep_t* device_ep);

/***************************************************************************//**

*/
void MSS_USBD_CIF_cep_read_pkt(mss_usb_ep_t* device_ep);

/***************************************************************************//**

*/
void MSS_USBD_CIF_cep_write_pkt(mss_usb_ep_t* device_ep);

/***************************************************************************//**

*/
void MSS_USBD_CIF_tx_ep_configure(mss_usb_ep_t* device_ep);

/***************************************************************************//**

*/
void MSS_USBD_CIF_rx_ep_configure(mss_usb_ep_t* device_ep);

#endif  //MSS_USB_DEVICE_ENABLED

#ifdef __cplusplus
}
#endif

#endif  /* __MSS_USB_DEVICE_CIF_H_ */
