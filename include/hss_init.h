#ifndef HSS_INIT_H
#define HSS_INIT_H

/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 *
 * Hart Software Services - Toplevel Init Routines
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

void HSS_Init(void);

bool HSS_ZeroTIMs(void);
bool HSS_ZeroDDR(void);
bool HSS_Init_RWDATA_BSS(void);
bool HSS_WakeSleepingHarts(void);
bool HSS_E51_Banner(void);

bool HSS_QueuesInit(void);

#if IS_ENABLED(CONFIG_SERVICE_QSPI)
#  include "qspi_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_MMC)
#  include "mmc_service.h"
#endif

#if IS_ENABLED(CONFIG_OPENSBI)
bool HSS_OpenSBIInit(void);
#endif

bool HSS_DDRInit(void);
bool HSS_DDRPrintSegConfig(void);
bool HSS_DDRPrintL2CacheWaysConfig(void);
bool HSS_DDRPrintL2CacheWayMasks(void);
bool HSS_UARTInit(void);
#if IS_ENABLED(CONFIG_USE_LOGO)
bool HSS_LogoInit(void);
#endif

#if IS_ENABLED(CONFIG_USE_IHC)
bool HSS_IHCInit(void);
void HSS_IHCInit_U54(void);
#endif

#ifdef CONFIG_USE_PCIE
bool HSS_PCIeInit(void);
#endif

bool HSS_PDMAInit(void);

#ifdef __cplusplus
}
#endif

#endif
