/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */
/*******************************************************************************
 * @file mss_pmp.h
 * @author Microchip-FPGA Embedded Systems Solutions
 * @brief PolarFire SoC MSS PMP configuration using MSS configurator values.
 *
 */
/*=========================================================================*//**

 *//*=========================================================================*/
#ifndef MSS_PMP_H
#define MSS_PMP_H


#ifdef __cplusplus
extern "C" {
#endif

#if !defined (LIBERO_SETTING_MEM_CONFIGS_ENABLED)
#define LIBERO_SETTING_MEM_CONFIGS_ENABLED      0ULL
/* Enabled when bit set to 1                               */
/* PMP                               [0:0]   RW value= 0x0 */
/* MPU                               [1:0]   RW value= 0x0 */
#endif
#define PMP_ENABLED_MASK      1UL
#define MPU_ENABLED_MASK      2UL


uint8_t pmp_configure(uint8_t hart_id);
void pmp_master_configs(uint8_t hart_id, uint64_t * pmp0cfg);

#ifdef __cplusplus
}
#endif


#endif /* MSS_PMP_H */
