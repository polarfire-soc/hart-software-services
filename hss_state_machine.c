/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
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
#include "hss_clock.h"
#include "hss_debug.h"
#include <assert.h>

#include "ssmb_ipi.h"

#ifndef CONFIG_SERVICE_IPI_POLL
#  include "csr_helper.h"
#endif
#ifdef    CONFIG_DEBUG_PROFILING_SUPPORT
#  include "profiling.h"
#endif

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
static HSSTicks_t maxLoopTime = 0u;
static uint64_t loopCount = 0u;
void RunStateMachines(const size_t spanOfPStateMachines, struct StateMachine *const pStateMachines[]) 
{
    HSSTicks_t const startTicks = HSS_GetTickCount();
    HSSTicks_t endTicks;

#ifndef CONFIG_SERVICE_IPI_POLL
    // poll IPIs each iteration for new messages
    {
        const union HSSHartBitmask hartBitmask = { .uint = mHSS_BITMASK_ALL_U54 };
        bool status = IPI_PollReceive(hartBitmask);
        if (status) {
            enum HSSHartId const myHartId = current_hartid();
            size_t i;

            for (i = 0u; i < MAX_NUM_HARTS; i++) {
                if (mUNLIKELY(i == myHartId)) { continue; } // don't handle messages if to myself

                uint32_t index = IPI_CalculateQueueIndex(i, myHartId);

                if (IPI_GetQueuePendingCount(index)) {
                    IPI_ConsumeIntent(i, IPI_MSG_ACK_COMPLETE); // gobble up any ACK completes
                    IPI_ConsumeIntent(i, IPI_MSG_ACK_PENDING);  // gobble up any ACK pendings
                }
            }
        }
    }
#endif

    {
        size_t i = 0u;

        for (i = 0; i < spanOfPStateMachines; i++) {
            struct StateMachine * const pCurrentMachine = pStateMachines[i];
            const char *pMachineName = "<error-unnamed>";

            assert(pCurrentMachine != NULL);
            if (pCurrentMachine->pMachineName != NULL) {
                pMachineName = (const char *)pCurrentMachine->pMachineName;
            }

            //mHSS_DEBUG_PRINTF("running machine %s" CRLF, pMachineName);
            {
                stateType_t prevState = pCurrentMachine->prevState;
                stateType_t currentState = pCurrentMachine->state;

                struct StateDesc const * const pCurrentStateDesc =
                    &(pCurrentMachine->pStateDescs[currentState]);

                assert(pCurrentStateDesc != NULL); // mandatory handler
                assert(pCurrentStateDesc->state == currentState);

                if (prevState != currentState) {
                    if (IsValidState(pCurrentMachine, prevState)) {
                        struct StateDesc const * const pLastStateDesc =
                            &(pCurrentMachine->pStateDescs[prevState]);

                        assert(pLastStateDesc->state == prevState);

                        if (pLastStateDesc->state_onExit) { // optional handler
                            pLastStateDesc->state_onExit(pCurrentMachine);
                        }
                    }

                    if (pCurrentStateDesc->state_onEntry) { // optional handler
                        pCurrentStateDesc->state_onEntry(pCurrentMachine);
                    }

                    pCurrentMachine->startTime = HSS_GetTickCount();
                    pCurrentMachine->executionCount = 0;
                    pCurrentMachine->prevState = pCurrentMachine->state;
                }

                //if (mLIKELY(pCurrentStateDesc->state_handler != NULL)) {
                pCurrentStateDesc->state_handler(pCurrentMachine);
                pCurrentMachine->lastExecutionTime = HSS_GetTickCount();
                pCurrentMachine->executionCount++;
                //}

#ifdef CONFIG_DEBUG_LOG_STATE_TRANSITIONS
                // debug print any state transitions...
                {
                    const char *pLastStateName;

                    // refresh states
                    prevState =    pCurrentMachine->prevState;
                    currentState = pCurrentMachine->state;

                    pLastStateName =
                        (pCurrentMachine->pStateDescs[prevState]).pStateName;
                    // should be valid at this stage

                    {
                        const char *pCurrentStateName =
                            (pCurrentMachine->pStateDescs[currentState]).pStateName;

                        if (prevState != currentState) {
                            mHSS_FANCY_STATE_TRANSITION_TEXT;

                            mHSS_DEBUG_PRINTF("%s::%s -> %s::%s" CRLF, pMachineName,
                                pLastStateName, pMachineName, pCurrentStateName);

                            mHSS_FANCY_NORMAL_TEXT;
                        }
                    }
                }
#else
		(void)pMachineName;
#endif
            }
        }
    }

    loopCount++;
    endTicks = HSS_GetTickCount();
#if defined(CONFIG_DEBUG_LOOP_TIMES) || defined(CONFIG_DEBUG_IPI_STATS)
    {
        HSSTicks_t const delta = endTicks - startTicks;

        bool dump_flag = false;
        bool max_exceeded_flag = false;

        if (mUNLIKELY(delta > maxLoopTime)) { 
            maxLoopTime = delta; 
            max_exceeded_flag = true; 
       }

        if (mUNLIKELY((loopCount % (unsigned long)CONFIG_DEBUG_LOOP_TIMES_THRESHOLD) == 0u)) {
            dump_flag = true;
        }

        if (mUNLIKELY(dump_flag || max_exceeded_flag)) {
#ifdef    CONFIG_DEBUG_PROFILING_SUPPORT
            dump_profile();
#endif
            if (dump_flag) {
                mHSS_FANCY_STATUS_TEXT;
            } else /* if (max_exceeded_flag) */ {
                mHSS_FANCY_WARN_TEXT;
            }

#ifdef   CONFIG_DEBUG_IPI_STATS
            IPI_DebugDumpStats();
#endif
#ifdef   CONFIG_DEBUG_LOOP_TIMES
            mHSS_DEBUG_PRINTF(" loop %" PRIu64
                " took %" PRIu64 " tick%s"
                " (max %" PRIu64 " tick%s)" CRLF, loopCount,
                delta, delta == 1u ? "" : "s",
                maxLoopTime, maxLoopTime == 1u ? "" : "s");
#endif

            mHSS_FANCY_NORMAL_TEXT;

        }
    }
#else
    (void)startTicks;
    (void)endTicks;
    (void)loopCount;
    (void)maxLoopTime;
#endif
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
    for (i = 0u; i < spanOfInitFunctions; i++) {
        assert(initFunctions[i].handler);

        //mHSS_DEBUG_PRINTF("Running %d of %d: %s()" CRLF, i, spanOfInitFunctions, 
        //    initFunctions[i].pName);

        bool result = (initFunctions[i].handler)();

        if (!result) {
            mHSS_FANCY_ERROR_TEXT;
            mHSS_DEBUG_PRINTF("%s() returned %d" CRLF, initFunctions[i].pName, result);
            mHSS_FANCY_NORMAL_TEXT;

            if (initFunctions[i].haltOnFailure) {
                while (true) { ; } // HALT on failure 
            }
        }
    }
}
