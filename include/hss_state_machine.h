#ifndef MPFS_HSS_STATE_MACHINE_H
#define MPFS_HSS_STATE_MACHINE_H

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
 * Hart Software Services - State Machine Defines/Types/Declarations
 *
 */


/**
 * \file State Machine
 * \brief State Machine Defines/Types/Declarations
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "hss_clock.h"

struct StateMachine;

typedef int stateType_t;

/**
 * \brief State Descriptor Structure
 *
 * A State Descriptor contains all information about a particular state
 * within a state machine
 */
struct StateDesc {
    stateType_t state;
    const char* const pStateName;
    void (*state_onEntry)(struct StateMachine *pMyMachine);
    void (*state_onExit)(struct StateMachine *pMyMachine);
    void (*state_handler)(struct StateMachine *pMyMachine);
};

/**
 * \brief StateMachine Structure
 *
 * A State Machine is a colection of states and some context, and forms
 * a runnable entity for the superloop state machine engine
 */
struct StateMachine {
    stateType_t state;
    stateType_t prevState;
    const uint32_t numStates;
    const char * const pMachineName;
    HSSTicks_t startTick;
    HSSTicks_t lastExecutionTick;
    HSSTicks_t startTime;
    HSSTicks_t lastExecutionTime;
    HSSTicks_t maxExecutionTime;
    stateType_t maxState;
    HSSTicks_t lastDeltaExecutionTime;
    uint64_t executionCount;
    struct StateDesc const * const pStateDescs;
    bool debugFlag;
    uint8_t priority;
    void *pInstanceData;
};

#define SM_INVALID_STATE ((stateType_t)-1)

void RunStateMachine(struct StateMachine * const pCurrentMachine);
void RunStateMachines(const size_t spanOfPStateMachines, struct StateMachine * const pStateMachines[]);

/**
 * \brief Init Function Type
 */

struct InitFunction {
    const char * const pName;
    bool (*handler)(void);

    int haltOnFailure:1;
    int restartOnFailure:1;
};

/**
 * \brief Get State Machines loop count
 */
uint64_t GetStateMachinesExecutionCount(void);

#ifdef __cplusplus
}
#endif

void RunInitFunctions(const size_t spanOfInitFunctions, const struct InitFunction initFunctions[]);

void DumpStateMachineStats(void);
#endif
