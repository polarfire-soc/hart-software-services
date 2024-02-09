/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
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
#include "hss_trigger.h"

#include "ssmb_ipi.h"
#include <assert.h>
#include <string.h>

#include "hss_memcpy_via_pdma.h"
#include "scrub_service.h"
#include "scrub_types.h"

#include "mss_l2_cache.h"

#ifndef PRIx64
#  define PRIx64 "llu"
#endif

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
    if (HSS_Trigger_IsNotified(EVENT_DDR_TRAINED)) {
        pMyMachine->state++;
    }
}

/////////////////
extern const uint64_t __l2lim_start,      __l2lim_end;
extern const uint64_t __l2_start,         __l2_end;
extern const uint64_t __ddr_start,        __ddr_end;
extern const uint64_t __ddrhi_start,      __ddrhi_end;
extern const uint64_t __ncddrhi_start,    __ncddrhi_end;
extern const uint64_t __dtim_start,       __dtim_end;
extern const uint64_t __e51itim_start,    __e51itim_end;
extern const uint64_t __u54_1_itim_start, __u54_1_itim_end;
extern const uint64_t __u54_2_itim_start, __u54_2_itim_end;
extern const uint64_t __u54_3_itim_start, __u54_3_itim_end;
extern const uint64_t __u54_4_itim_start, __u54_4_itim_end;

const struct {
    uintptr_t baseAddr;
    uintptr_t endAddr;
    bool use_atomic_or;
} rams[] = {
#if IS_ENABLED(CONFIG_SERVICE_SCRUB_L2LIM)
    { (uintptr_t)&__l2lim_start,         (uintptr_t)&__l2lim_end,      true },
#endif
#if IS_ENABLED(CONFIG_SERVICE_SCRUB_L2SCRATCH)
    { (uintptr_t)&__l2_start,            (uintptr_t)&__l2_end,         false },
#endif
#if IS_ENABLED(CONFIG_SERVICE_SCRUB_DTIM)
    { (uintptr_t)&__dtim_start,          (uintptr_t)&__dtim_end,       false },
#endif
#if !IS_ENABLED(CONFIG_SKIP_DDR)
#  if IS_ENABLED(CONFIG_SERVICE_SCRUB_CACHED_DDR)
    { (uintptr_t)&__ddr_start,           (uintptr_t)&__ddr_end,        false },
    { (uintptr_t)&__ddrhi_start,         (uintptr_t)&__ddrhi_end,      false },
#  endif
#  if IS_ENABLED(CONFIG_SERVICE_SCRUB_NONCACHE_DDDR)
    //{ (uintptr_t)&__ncddrhi_start,       (uintptr_t)&__ncddrhi_end,    false },
#  endif
#endif
    //{ (uintptr_t)&__e51itim_start,       (uintptr_t)&__e51itim_end,    false },
    //{ (uintptr_t)&__u54_1_itim_start,    (uintptr_t)&__u54_1_itim_end, false },
    //{ (uintptr_t)&__u54_2_itim_start,    (uintptr_t)&__u54_2_itim_end, false },
    //{ (uintptr_t)&__u54_3_itim_start,    (uintptr_t)&__u54_3_itim_end, false },
    //{ (uintptr_t)&__u54_4_itim_start,    (uintptr_t)&__u54_4_itim_end, false },
};

static uintptr_t offset = 0u;
static size_t idx = 0u;
static size_t entryCount = 0u;
static void scrub_scrubbing_handler(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;

    if (ARRAY_SIZE(rams)) {
        if (!entryCount) {
            if ((rams[idx].baseAddr + offset)  >= rams[idx].endAddr) {
                idx = (idx + 1u) % ARRAY_SIZE(rams);
                // mHSS_DEBUG_PRINTF(LOG_NORMAL, "Scrubbing %p to %p\n", rams[idx].baseAddr, rams[idx].endAddr);
                offset = 0u;
            }

            const uintptr_t length = rams[idx].endAddr - rams[idx].baseAddr;

            if (length) {
                const size_t chunkSize = MIN(CONFIG_SERVICE_SCRUB_MAX_SIZE_PER_LOOP_ITER, length-offset);

                const uintptr_t chunkStartAddr = (uintptr_t)(rams[idx].baseAddr) + offset;
                const uintptr_t chunkEndAddr =   (uintptr_t)(chunkStartAddr) + chunkSize;

                for (uint64_t *pMem = (uint64_t*)chunkStartAddr; pMem < (uint64_t*)chunkEndAddr; pMem++) {
                    if (unlikely(rams[idx].use_atomic_or)) {
                        __atomic_or_fetch((volatile uint64_t *)pMem, (uint64_t)0u, __ATOMIC_RELAXED);
                    } else {
                        *(volatile uint64_t *)pMem;
                    }
                }
                offset = offset + chunkSize;
            }
        }
    }

#if IS_ENABLED(CONFIG_SERVICE_SCRUB_CACHES)
    static size_t trigger_cache_flush = 0u;
    static enum HSSHartId last_peer = HSS_HART_U54_1;

    if (trigger_cache_flush > CONFIG_SERVICE_SCRUB_CACHES_TRIGGER_SETPOINT) {
        // flush local E51 I$
        __asm("fence.i"); // flush I$

        // flush remote U54 I$ and TLBs

        IPI_Send(last_peer, IPI_MSG_SCRUB, 0u, 0u, NULL, NULL);

        trigger_cache_flush = 0u;
        last_peer++;

        if (last_peer >= HSS_HART_NUM_PEERS) {
            // flush L2 Cache, way-by-way
            // see https://forums.sifive.com/t/flush-invalidate-l1-l2-on-the-u54-mc/4483/9
            //
            // the thing to be wary of is the policy is random replacement by way,
            // so there must be only 1 way enabled at a time...
            // then for each way individually, still need to go through ~2MiB
            // (2MiB/16 per way) of reads to safely ensure the L2 is cleared...
            //

            for (int wayMaskN = 0; wayMaskN < LIBERO_SETTING_WAY_ENABLE; wayMaskN++) {
                // disable evictions from all but WayMaskN
               __atomic_store_8(&CACHE_CTRL->WAY_MASK_E51_DCACHE, wayMaskN, __ATOMIC_RELAXED);

                // read 2MiB/16 from L2 zero device
                for (int i = 0; i < 131*1024; i+=8) { (void)*(volatile uint64_t *)(ZERO_DEVICE_BOTTOM + i); };
            }

            // restore WayMask values...
            __atomic_store_8(&CACHE_CTRL->WAY_MASK_E51_DCACHE, LIBERO_SETTING_WAY_MASK_E51_DCACHE, __ATOMIC_RELAXED);

            last_peer = HSS_HART_U54_1;
        }
    } else {
        trigger_cache_flush++;
    }
#endif

#if defined(CONFIG_SERVICE_SCRUB_RUN_EVERY_X_SUPERLOOPS)
    entryCount = (entryCount + 1u) % CONFIG_SERVICE_SCRUB_RUN_EVERY_X_SUPERLOOPS;
#else
    entryCount = 0u;
#endif
}

enum IPIStatusCode Scrub_IPIHandler(TxId_t transaction_id, enum HSSHartId source,
    uint32_t immediate_arg, void *p_extended_buffer_in_ddr, void *p_ancilliary_buffer_in_ddr)
{
    (void)transaction_id;
    (void)source;
    (void)immediate_arg;
    (void)p_extended_buffer_in_ddr;
    (void)p_ancilliary_buffer_in_ddr;

    __asm("fence.i"); // flush I$
    __asm("sfence.vma x0, x0"); // flush any local hardware caches related to address translation. (CIP-1200)

    // could consider reading 2*D$ size here to flush D$ ?

    // no need to report back that we're done...
    return IPI_IDLE;
}


void scrub_dump_stats(void)
{
    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "idx:      0x%" PRIx64 "\n", idx);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "mem base:   0x%" PRIx64 "\n", rams[idx].baseAddr);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "offset:     0x%" PRIx64 "\n", offset);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "entryCount: 0x%" PRIx64 "\n", entryCount);
}
