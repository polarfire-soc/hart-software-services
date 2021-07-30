/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file RAM Scrubbing Driver State Machine
 * \brief E51-Assisted RAM Scrubbing
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"
#include "hss_boot_pmp.h"

#include "ssmb_ipi.h"
#include <assert.h>
#include <string.h>

#include "hss_memcpy_via_pdma.h"
#include "scrub_service.h"
#include "scrub_types.h"

static void scrub_init_handler(struct StateMachine * const pMyMachine);
static void scrub_scrubbing_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief SCRUB Driver States
 */
enum ScrubStatesEnum {
    SCRUB_INITIALIZATION,
    SCRUB_SCRUBBING,
    SCRUB_NUM_STATES = SCRUB_SCRUBBING+1
};

/*!
 * \brief SCRUB Driver State Descriptors
 */
static const struct StateDesc scrub_state_descs[] = {
    { (const stateType_t)SCRUB_INITIALIZATION, (const char *)"init",      NULL, NULL, &scrub_init_handler },
    { (const stateType_t)SCRUB_SCRUBBING,      (const char *)"scrubbing", NULL, NULL, &scrub_scrubbing_handler },
};

/*!
 * \brief SCRUB Driver State Machine
 */
struct StateMachine scrub_service = {
    .state             = (stateType_t)SCRUB_INITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         = (const uint32_t)SCRUB_NUM_STATES,
    .pMachineName      = (const char *)"scrub_service",
    .startTime         = 0u,
    .lastExecutionTime = 0u,
    .executionCount    = 0u,
    .pStateDescs       = scrub_state_descs,
    .debugFlag         = true,
    .priority          = 0u,
    .pInstanceData     = NULL
};


// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void scrub_init_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state++;
}

/////////////////
#define MAX_SCRUB_SIZE_PER_LOOP_ITER (4096u)
extern const uint64_t __l2lim_start,         __l2lim_end;
extern const uint64_t __l2_scratchpad_start, __l2_scratchpad_end;
extern const uint64_t __ddr_start,           __ddr_end;
extern const uint64_t __ddrhi_start,         __ddrhi_end;
extern const uint64_t __dtim_start,          __dtim_end;
extern const uint64_t __e51itim_start,       __e51itim_end;
extern const uint64_t __u54_1_itim_start,    __u54_1_itim_end;
extern const uint64_t __u54_2_itim_start,    __u54_2_itim_end;
extern const uint64_t __u54_3_itim_start,    __u54_3_itim_end;
extern const uint64_t __u54_4_itim_start,    __u54_4_itim_end;

const struct {
	uintptr_t baseAddr;
	uintptr_t endAddr;
} rams[] = {
    { (uintptr_t)&__l2lim_start,         (uintptr_t)&__l2lim_end },
    { (uintptr_t)&__l2_scratchpad_start, (uintptr_t)&__l2_scratchpad_end },
    { (uintptr_t)&__ddr_start,           (uintptr_t)&__ddr_end },
    { (uintptr_t)&__ddrhi_start,         (uintptr_t)&__ddrhi_end },
    //{ (uintptr_t)&__dtim_start,          (uintptr_t)&__dtim_end },
    //{ (uintptr_t)&__e51itim_start,       (uintptr_t)&__e51itim_end },
    //{ (uintptr_t)&__u54_1_itim_start,    (uintptr_t)&__u54_1_itim_end },
    //{ (uintptr_t)&__u54_2_itim_start,    (uintptr_t)&__u54_2_itim_end },
    //{ (uintptr_t)&__u54_3_itim_start,    (uintptr_t)&__u54_3_itim_end },
    //{ (uintptr_t)&__u54_4_itim_start,    (uintptr_t)&__u54_4_itim_end },
};

static size_t offset = 0u;
static size_t index = 0u;
static size_t entryCount = 0u;
static void scrub_scrubbing_handler(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;

    if (ARRAY_SIZE(rams)) {
        if (!entryCount) {
            if ((rams[index].baseAddr + offset)  >= rams[index].endAddr) {
                index = (index + 1u) % ARRAY_SIZE(rams);
                mHSS_DEBUG_PRINTF(LOG_NORMAL, "Scrubbing %p to %p" CRLF, rams[index].baseAddr, rams[index].endAddr);
		offset = 0u;
            }

            const uintptr_t length = rams[index].endAddr - rams[index].baseAddr;

            if (length) {
                const size_t chunkSize = MIN(MAX_SCRUB_SIZE_PER_LOOP_ITER, length-offset);

                const uint64_t *pStart = (uint64_t *)(rams[index].baseAddr + offset);
                const uint64_t *pEnd = (uint64_t *)(pStart + chunkSize);

                for (uint64_t *pMem = (uint64_t *)pStart; pMem < pEnd; pMem++) {
                    *(volatile uint64_t *)pMem;
                }
                offset = offset + chunkSize;
            }
        }
    }

    //entryCount = (entryCount + 1u) % 0x1000;
    entryCount = 0;
}

void scrub_dump_stats(void)
{
    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "index:      0x%" PRIx64 CRLF, index);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Mem base:   0x%" PRIx64 CRLF, rams[index].baseAddr);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "offset:     0x%" PRIx64 CRLF, offset);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "entryCount: 0x%" PRIx64 CRLF, entryCount);
}
