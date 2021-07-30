#ifndef HSS_REGISTRY_H
#define HSS_REGISTRY_H

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
 * Hart Software Services - Registry of Tables (State Machines, IPI handlers, Init
 * Functions)
 *
 */

/*!
 * \file HSS Registry
 * \brief Initalization for any registered tables.
 */

#ifdef __cplusplus
extern "C" {
#endif

extern const struct IPI_Handler  /*@null@*/ ipiRegistry[];
extern struct StateMachine /*@null@*/ * const pGlobalStateMachines[];
extern const struct InitFunction /*@null@*/ globalInitFunctions[];

extern const size_t spanOfIpiRegistry;
extern const size_t spanOfPGlobalStateMachines;
extern const size_t spanOfGlobalInitFunctions;

#ifdef __cplusplus
}
#endif

#endif
