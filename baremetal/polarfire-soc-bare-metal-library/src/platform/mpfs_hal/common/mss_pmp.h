/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */
/*******************************************************************************
 * @file mss_mpu.h
 * @author Microchip-FPGA Embedded Systems Solutions
 * @brief PolarFire SoC MSS MPU driver APIS for configuring access regions for
 * the external masters.
 *
 */
/*=========================================================================*//**

 *//*=========================================================================*/
#ifndef MSS_MPU_H
#define MSS_MPU_H


#ifdef __cplusplus
extern "C" {
#endif


uint8_t pmp_configure(uint8_t hart_id);

#ifdef __cplusplus
}
#endif


#endif /* MSS_MPU_H */
