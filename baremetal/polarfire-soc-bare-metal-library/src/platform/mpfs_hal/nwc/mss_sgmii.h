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

/***************************************************************************//**
  sgmii_off_mode() called in sgmii channels are not being used.

  Example:
  @code

      sgmii_off_mode();

  @endcode

 */
void sgmii_off_mode(void);

/***************************************************************************//**
  sgmii_setup() sets up the SGMII using settings from Libero

  Example:
  @code

      sgmii_setup();

  @endcode

 */
uint32_t sgmii_setup(void);

/***************************************************************************//**
  ddr_pvt_calibration() calibrates DDR I/O using the hardware function

  @return
    This function returns status, see DDR_SS_STATUS enum

  Example:
  @code

      ddr_pvt_calibration();

  @endcode

 */
void
ddr_pvt_calibration
(
    void
);

/***************************************************************************//**
  ddr_pvt_recalibration() recalibrates DDR I/O using the hardware function


  Example:
  @code

      ddr_pvt_recalibration();

  @endcode

 */
void ddr_pvt_recalibration(void);

#ifdef __cplusplus
}
#endif

#endif /* SRC_PLATFORM_MPFS_HAL_NWC_MSS_SGMII_H_ */
