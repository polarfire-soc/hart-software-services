#ifndef HSS_PERF_CTR_H
#define HSS_PERF_CTR_H

/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file Performance Counter
 * \brief Performance Counter
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"
#include "hss_clock.h"

#include <assert.h>

bool HSS_PerfCtr_Allocate(int *pIdx, char const * name) __attribute__((nonnull(1)));
void HSS_PerfCtr_Deallocate(int index);
void HSS_PerfCtr_Start(int index);
void HSS_PerfCtr_Lap(int index);
HSSTicks_t HSS_PerfCtr_GetTime(int index);
void HSS_PerfCtr_DumpAll(void);

#define PERF_CTR_UNINITIALIZED -1

#endif
