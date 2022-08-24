#ifndef HSS_U54_STATE_H
#define HSS_U54_STATE_H

/*******************************************************************************
 * Copyright 2022 Microchip FPGA Embedded Systems Solutions.
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
 * Hart Software Services - U54 State
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file U54 State
 * \brief U54 State
 */

enum HSSHartState_t {
    HSS_State_Idle = 0,
    HSS_State_Booting,
    HSS_State_SBIHartInit,
    HSS_State_SBIWaitForColdboot,
    HSS_State_Running,
    HSS_State_Trap,
    HSS_State_Fatal,
};

int HSS_U54_GetState(void);
int HSS_U54_GetState_Ex(int hartId);
char const * HSS_U54_GetStateName(int hartId);

void HSS_U54_SetState(int state);
void HSS_U54_SetState_Ex(int hartId, int state);

void HSS_U54_DumpStates(void);

#ifdef __cplusplus
}
#endif

#endif
