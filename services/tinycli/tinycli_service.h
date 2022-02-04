#ifndef HSS_TINYCLI_SERVICE_H
#define HSS_TINYCLI_SERVICE_H

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
 * Hart Software Services - Tiny CLI Parser
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "ssmb_ipi.h"
#include "hss_state_machine.h"
#include "hss_debug.h"

/**
 * \file Tiny CLI parser
 * \brief Tiny CLI parser
 */

bool HSS_TinyCLI_Parser(void);
size_t HSS_TinyCLI_ParseIntoTokens(char *buffer);
void HSS_TinyCLI_Execute(void);
bool HSS_TinyCLI_IndicatePostInit(void);

void HSS_TinyCLI_RunMonitors(void);
void HSS_TinyCLI_WaitForUSBMSCDDone(void);
void HSS_TinyCLI_SurrenderUART(void);

extern struct StateMachine tinycli_service;

#ifdef __cplusplus
}
#endif

#endif

