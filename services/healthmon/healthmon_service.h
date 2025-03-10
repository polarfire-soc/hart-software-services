#ifndef HSS_HEALTHMON_SERVICE_H
#define HSS_HEALTHMON_SERVICE_H

/*******************************************************************************
 * Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
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
 * Hart Software Services - Health Monitoring Service API
 *
 */


/*!
 * \file Health Monitoring API
 * \brief Health Monitoring State Machine API function declarations
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "hss_state_machine.h"
#include "hss_debug.h"

extern struct StateMachine healthmon_service;

void HSS_Health_DumpStats(void);
void healthmon_nop_trigger(uintptr_t pAddr);

enum HealthMon_CheckType
{
    ABOVE_THRESHOLD,
    BELOW_THRESHOLD,
    ABOVE_OR_BELOW_THRESHOLD,
    EQUAL_TO_VALUE,
    NOT_EQUAL_TO_VALUE,
    CHANGED_SINCE_LAST,
};

struct HealthMonitor
{
    char const * const pName;
    uintptr_t pAddr;
    enum HealthMon_CheckType checkType;
    uint32_t maxValue;
    uint32_t minValue;
    uint8_t shift; // shift applied first...
    uint64_t mask; // then mask
    void (*triggerCallback)(uintptr_t pAddr);
    uint32_t throttleScale; // times 1sec, to throttle console messages
};

struct HealthMonitor_Status
{
    HSSTicks_t throttle_startTime;
    uint32_t lastValue;
    size_t count;
    bool initialized;
};
#ifdef __cplusplus
}
#endif

#endif
