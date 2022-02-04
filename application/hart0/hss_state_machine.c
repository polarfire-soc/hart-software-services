/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file State Machine Engine
 *
 * \brief Registered state machine super-loop
 */

#include "config.h"
#include "hss_types.h"

#include "hss_state_machine.h"
#include "hss_progress.h"
#include "hss_clock.h"
#include "hss_debug.h"
#include <assert.h>

#include "ssmb_ipi.h"

#include "csr_helper.h"
#include "profiling.h"

#include "hss_registry.h"

/**
 * \brief Ensure that state is valid for given state machine
 */
static inline bool IsValidState(struct StateMachine *pStateMachine, int state)
{
    bool result = false;

    if ((state >= 0) && (state < (int)pStateMachine->numStates)) {
        result = true;
    }

    return result;
}

/**
 * \brief Run all registered state machines
 */
void RunStateMachine(struct StateMachine *const pCurrentMachine)
{
    assert(pCurrentMachine != NULL);

    const char *pMachineName = "<error-unnamed>";
    if (pCurrentMachine->pMachineName != NULL) {
        pMachineName = (const char *)pCurrentMachine->pMachineName;
    }

    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "running machine %s" CRLF, pMachineName);
    {
        stateType_t prevState = pCurrentMachine->prevState;
        stateType_t currentState = pCurrentMachine->state;

        struct StateDesc const * const pCurrentStateDesc =
            &(pCurrentMachine->pStateDescs[currentState]);

        assert(pCurrentStateDesc != NULL); // mandatory handler
        assert(pCurrentStateDesc->state == currentState);

        HSSTicks_t lastEntry = HSS_GetTime();

        if (!pCurrentMachine->startTime) {
            pCurrentMachine->startTime = lastEntry;
        }
        pCurrentMachine->lastExecutionTime = lastEntry;

        if (prevState != currentState) {
            if (IsValidState(pCurrentMachine, prevState)) {
                struct StateDesc const * const pLastStateDesc =
                    &(pCurrentMachine->pStateDescs[prevState]);

                assert(pLastStateDesc->state == prevState);

                if (pLastStateDesc->state_onExit) { // optional onExit handler
                    pLastStateDesc->state_onExit(pCurrentMachine);
                }
            }

            if (pCurrentStateDesc->state_onEntry) { // optional onEntry handler
                pCurrentStateDesc->state_onEntry(pCurrentMachine);
            }

            pCurrentMachine->executionCount = 0;
            pCurrentMachine->prevState = pCurrentMachine->state;
        }

        if (likely(pCurrentStateDesc->state_handler != NULL)) {
            pCurrentStateDesc->state_handler(pCurrentMachine);
        }

        ++pCurrentMachine->executionCount;

        HSSTicks_t lastExit = HSS_GetTime();
        pCurrentMachine->lastDeltaExecutionTime = lastExit - lastEntry;

        if (pCurrentMachine->lastDeltaExecutionTime > pCurrentMachine->maxExecutionTime) {
            pCurrentMachine->maxExecutionTime = pCurrentMachine->lastDeltaExecutionTime;
            pCurrentMachine->maxState = pCurrentMachine->prevState;
        }

        if (IS_ENABLED(CONFIG_DEBUG_LOG_STATE_TRANSITIONS)) {
            // debug print any state transitions...
            if (pCurrentMachine->debugFlag) {

                // refresh states
                prevState =    pCurrentMachine->prevState;
                currentState = pCurrentMachine->state;

#ifdef DEBUG
                {
                    const char *pLastStateName =
                        (pCurrentMachine->pStateDescs[prevState]).pStateName;
                        // should be valid at this stage
                    const char *pCurrentStateName =
                        (pCurrentMachine->pStateDescs[currentState]).pStateName;

                    if (prevState != currentState) {
                        mHSS_DEBUG_PRINTF(LOG_STATE_TRANSITION, "%s :: %s -> %s" CRLF, pMachineName,
                            pLastStateName, pCurrentStateName);
                    }
                }
#endif
            }
        } else {
            (void)pMachineName;
        }
    }
}

static HSSTicks_t maxLoopTime = 0u;
static uint64_t loopCount = 0u;
void RunStateMachines(const size_t spanOfPStateMachines, struct StateMachine *const pStateMachines[])
{
    HSSTicks_t const startTicks = HSS_GetTickCount();
    HSSTicks_t endTicks;

    if (!IS_ENABLED(CONFIG_SERVICE_IPI_POLL)) {
        // poll IPIs each iteration for new messages
        const union HSSHartBitmask hartBitmask = { .uint = mHSS_BITMASK_ALL_U54 };
        bool status = IPI_PollReceive(hartBitmask);
        if (status) {
            enum HSSHartId const myHartId = current_hartid();
            size_t i;

            for (i = 0u; i < MAX_NUM_HARTS; ++i) {
                if (unlikely(i == myHartId)) { continue; } // don't handle messages if to myself

                uint32_t index = IPI_CalculateQueueIndex(i, myHartId);

                if (IPI_GetQueuePendingCount(index)) {
                    IPI_ConsumeIntent(i, IPI_MSG_ACK_COMPLETE); // gobble up any ACK completes
                    IPI_ConsumeIntent(i, IPI_MSG_ACK_PENDING);  // gobble up any ACK pendings
                }
            }
        }
    }

    {
        size_t i = 0u;

        for (i = 0; i < spanOfPStateMachines; ++i) {
            struct StateMachine * const pCurrentMachine = pStateMachines[i];

            RunStateMachine(pCurrentMachine);
        }
    }

    ++loopCount;
    endTicks = HSS_GetTickCount();
    if (IS_ENABLED(CONFIG_DEBUG_LOOP_TIMES) || IS_ENABLED(CONFIG_DEBUG_IPI_STATS)) {
        HSSTicks_t const delta = endTicks - startTicks;

        bool dump_flag = false;
        bool max_exceeded_flag = false;

        if (unlikely(delta > maxLoopTime)) {
            maxLoopTime = delta;
            max_exceeded_flag = true;
       }

#if IS_ENABLED(CONFIG_DEBUG_LOOP_TIMES)
        if (unlikely((loopCount % (unsigned long)CONFIG_DEBUG_LOOP_TIMES_THRESHOLD) == 0u)) {
            dump_flag = true;
        }
#endif

        if (unlikely(dump_flag || max_exceeded_flag)) {
            if (IS_ENABLED(CONFIG_DEBUG_PROFILING_SUPPORT)) {
                dump_profile();
            }

            if (IS_ENABLED(CONFIG_DEBUG_IPI_STATS)) {
                IPI_DebugDumpStats();
            }

            if (IS_ENABLED(CONFIG_DEBUG_LOOP_TIMES)) {
                if (dump_flag) {
                    mHSS_DEBUG_PRINTF(LOG_STATUS, " loop %" PRIu64
                        " took %" PRIu64 " tick%s"
                        " (max %" PRIu64 " tick%s)" CRLF, loopCount,
                        delta, delta == 1u ? "" : "s",
                        maxLoopTime, maxLoopTime == 1u ? "" : "s");
                } else /* if (max_exceeded_flag) */ {
                    mHSS_DEBUG_PRINTF(LOG_WARN, " loop %" PRIu64
                        " took %" PRIu64 " tick%s"
                        " (max %" PRIu64 " tick%s)" CRLF, loopCount,
                        delta, delta == 1u ? "" : "s",
                        maxLoopTime, maxLoopTime == 1u ? "" : "s");
                }
            }
        }
    } else {
        (void)startTicks;
        (void)endTicks;
        (void)loopCount;
        (void)maxLoopTime;
    }
}

/**
 * \brief Get State Machines loop count
 */
uint64_t GetStateMachinesExecutionCount(void)
{
    return loopCount;
}


/**
 * \brief Run through array of InitFunctions
 */
void RunInitFunctions(const size_t spanOfInitFunctions, const struct InitFunction initFunctions[])
{
    size_t i;
    for (i = 0u; i < spanOfInitFunctions; ++i) {
        assert(initFunctions[i].handler);

        //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Running %d of %d: %s()" CRLF, i, spanOfInitFunctions,
        //    initFunctions[i].pName);

        bool result = (initFunctions[i].handler)();

        if (!result) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "%s() returned %d" CRLF, initFunctions[i].pName, result);

            if (initFunctions[i].haltOnFailure) {
                while (true) { ; } // HALT on failure
            } else if (initFunctions[i].restartOnFailure) {
                uint8_t rcvBuf;

                bool keyPressedFlag = HSS_ShowTimeout("Init failed, press a key to prevent restart" CRLF, 5u, &rcvBuf);

                if (IS_ENABLED(CONFIG_SERVICE_TINYCLI) && keyPressedFlag) {
                    bool HSS_TinyCLI_Parser(void);

                    (void)HSS_TinyCLI_Parser();
                } else {
                    void _start(void);
                    _start();
                }
            }
        }
    }
}

void DumpStateMachineStats(void)
{
    mHSS_DEBUG_PRINTF(LOG_STATUS, " State Machine Name: Max Exec Time / State : Last Delta Time / Current State" CRLF);

    for (size_t i = 0u; i < spanOfPGlobalStateMachines; i++) {
        mHSS_DEBUG_PRINTF(LOG_STATUS, "%19s: % 13" PRIu64 " /    % 2" PRIu64 " : % 15" PRIu64 " /    % 2" PRIu64 CRLF,
            pGlobalStateMachines[i]->pMachineName,
            pGlobalStateMachines[i]->maxExecutionTime,
            pGlobalStateMachines[i]->maxState,
            pGlobalStateMachines[i]->lastDeltaExecutionTime,
            pGlobalStateMachines[i]->state);
    }
}
