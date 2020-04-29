/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
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

/*
 *
 */
void sgmii_off_mode(void);
uint32_t sgmii_setup(void);
void ddr_pvt_recalibration(void);
void ddr_pvt_calibration(void);

#ifdef __cplusplus
}
#endif

#endif /* SRC_PLATFORM_MPFS_HAL_NWC_MSS_SGMII_H_ */
