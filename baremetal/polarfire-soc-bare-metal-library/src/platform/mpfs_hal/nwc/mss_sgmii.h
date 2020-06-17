/*******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/*******************************************************************************
 * @file mss_sgmii.h
 * @author Microchip-FPGA Embedded Systems Solutions
 * @brief SGMII defines
 *
 */

#ifndef SRC_PLATFORM_MPFS_HAL_NWC_MSS_SGMII_H_
#define SRC_PLATFORM_MPFS_HAL_NWC_MSS_SGMII_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TX_RX_CH_EN_MASK    0xFU
#define TX_RX_CH_EN_OFFSET  0x4U

/*
 *
 */
void sgmii_off_mode(void);
uint32_t sgmii_setup(void);

#ifdef __cplusplus
}
#endif

#endif /* SRC_PLATFORM_MPFS_HAL_NWC_MSS_SGMII_H_ */
