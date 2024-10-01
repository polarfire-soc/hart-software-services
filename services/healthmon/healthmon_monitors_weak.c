/*******************************************************************************
 * Copyright 2019-2023 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file Health Monitor State Machine
 * \brief E51-Assisted Health Monitor
 */

#include "config.h"
#include "hss_types.h"
#include "healthmon_service.h"

__attribute__((weak)) const struct HealthMonitor monitors[] = { };
__attribute__((weak)) struct HealthMonitor_Status monitor_status[ARRAY_SIZE(monitors)] = { };
__attribute__((weak)) const size_t monitors_array_size = ARRAY_SIZE(monitors);
