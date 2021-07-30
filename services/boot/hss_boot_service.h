#ifndef HSS_BOOT_SERVICE_H
#define HSS_BOOT_SERVICE_H

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
 * Hart Software Services - Boot Service
 *
 */

/*!
 * \file  Boot Boot Service
 * \brief State Machine and API functions for booting firmware service
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "ssmb_ipi.h"
#include "hss_types.h"

enum IPIStatusCode HSS_Boot_IPIHandler(TxId_t transaction_id, enum HSSHartId source,
    uint32_t immediate_arg, void *p_extended_buffer_in_ddr, void *p_ancilliary_buffer_in_ddr);
enum IPIStatusCode HSS_Boot_PMPSetupHandler(TxId_t transaction_id, enum HSSHartId source,
    uint32_t immediate_arg, void *p_extended_buffer_in_ddr, void *p_ancilliary_buffer_in_ddr);
bool HSS_Boot_PMPSetupRequest(enum HSSHartId target, uint32_t *indexOut);
bool HSS_Boot_SBISetupRequest(enum HSSHartId target, uint32_t *indexOut);
enum IPIStatusCode HSS_Boot_RestartCore(enum HSSHartId source);

void HSS_Register_Boot_Image(struct HSS_BootImage *pImage);

bool HSS_Boot_Harts(const union HSSHartBitmask restartHartBitmask);

bool HSS_Boot_Custom(void);

extern struct StateMachine boot_service1;
extern struct StateMachine boot_service2;
extern struct StateMachine boot_service3;
extern struct StateMachine boot_service4;

#ifdef __cplusplus
}
#endif

#endif
