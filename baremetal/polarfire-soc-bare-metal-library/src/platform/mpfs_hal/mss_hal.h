/*******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/*******************************************************************************
 * @file mss_hal.h
 * @author Microchip-FPGA Embedded Systems Solutions
 * @brief MPFS HAL include file. This is the file intended for application to
 * include so that all the other MPFS files are then accessible to it.
 *
 */

#ifndef MSS_HAL_H
#define MSS_HAL_H

#include "mpfs_hal/nwc/mss_ddr.h"
#include "mpfs_hal/nwc/mss_ddr_SGMII_regs.h"
#include "mpfs_hal/nwc/mss_io_config.h"
#include "mpfs_hal/nwc/mss_pll.h"
#include "mpfs_hal/nwc/mss_scb_nwc_regs.h"
#include "mpfs_hal/nwc/mss_scb_nwc_regs.h"
#include "config/software/mpfs_hal/mss_sw_config.h"
/*
 * The hw_platform.h is included here only. It must be included after
 * mss_sw_config.h. This allows defines in hw_platform.h be overload from
 * mss_sw_config.h if necessary.
 * */
#include "config/hardware/hw_platform.h"
#include "atomic.h"
#include "bits.h"
#include "encoding.h"
#include "mss_clint.h"
#include "mss_coreplex.h"
#include "mss_h2f.h"
#include "mss_hart_ints.h"
#include "mss_ints.h"
#include "mss_mpu.h"
#include "mss_peripheral_base_add.h"
#include "mss_plic.h"
#include "mss_prci.h"
#include "mss_seg.h"
#include "mss_sysreg.h"
#include "mss_util.h"
#include "mtrap.h"
#include "mss_l2_cache.h"
#include "nwc/mss_ddr.h"
#include "nwc/mss_sgmii.h"
#include "system_startup.h"
#ifdef SIMULATION_TEST_FEEDBACK
#include "nwc/simulation.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint32_t SysTick_Config(void);
void disable_systick(void);

#ifdef __cplusplus
}
#endif

#endif /* MSS_HAL_H */
