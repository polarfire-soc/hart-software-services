/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file BEU Monitor State Machine
 * \brief E51-Assisted BEU Monitor
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
#include "beu_service.h"
#include "sbi_bitops.h"

#include "mss_hart_ints.h"

#ifndef BIT
#  define BIT(nr)			(1UL << (nr))
#endif

static void beu_init_handler(struct StateMachine * const pMyMachine);
static void beu_monitoring_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief BEU Driver States
 */
enum BeuStatesEnum {
    BEU_INITIALIZATION,
    BEU_MONITORING,
    BEU_NUM_STATES = BEU_MONITORING+1
};

/*!
 * \brief BEU Driver State Descriptors
 */
static const struct StateDesc beu_state_descs[] = {
    { (const stateType_t)BEU_INITIALIZATION, (const char *)"init",      NULL, NULL, &beu_init_handler },
    { (const stateType_t)BEU_MONITORING,      (const char *)"monitoring", NULL, NULL, &beu_monitoring_handler },
};

/*!
 * \brief BEU Driver State Machine
 */
struct StateMachine beu_service = {
    .state             = (stateType_t)BEU_INITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         = (const uint32_t)BEU_NUM_STATES,
    .pMachineName      = (const char *)"beu_service",
    .startTime         = 0u,
    .lastExecutionTime = 0u,
    .executionCount    = 0u,
    .pStateDescs       = beu_state_descs,
    .debugFlag         = true,
    .priority          = 0u,
    .pInstanceData     = NULL
};

// BEU Events:
//
// +-------+-----------------------------------------------------|
// | Cause | Meaning                                             |
// +-------+-----------------------------------------------------|
// | 0     | No error                                            |
// | 1     | Reserved                                            |
// | 2     | Instruction cache or ITM correctable error          |
// | 3     | ITIM uncorrectable ECC error                        |
// | 4     | Reserved                                            |
// | 5     | Load or store TileLink bus error                    |
// | 6     | Data cache correctable ECC error                    |
// | 7     | Data cache uncorrectalbe ECC error                  |
// +-------+-----------------------------------------------------|
//
enum BEU_Event_Cause {
    BEU_EVENT_NO_ERROR                 = 0,
    BEU_EVENT_RESEVERD1                = 1,
    BEU_EVENT_ITIM_CORRECTABLE         = 2,
    BEU_EVENT_ITIM_UNCORRECTABLE       = 3,
    BEU_EVENT_RESERVED2                = 4,
    BEU_EVENT_TILELINK_BUS_ERROR       = 5,
    BEU_EVENT_DATA_CACHE_CORRECTABLE   = 6,
    BEU_EVENT_DATA_CACHE_UNCORRECTABLE = 7,
    MAX_BEU_CAUSES                     = BEU_EVENT_DATA_CACHE_UNCORRECTABLE + 1
};

char const * const BEU_Event_Name[] = {
    [BEU_EVENT_NO_ERROR]                 = "No error",
    [BEU_EVENT_RESEVERD1]                = "Reserved",
    [BEU_EVENT_ITIM_CORRECTABLE]         = "Instruction cache or ITM correctable error",
    [BEU_EVENT_ITIM_UNCORRECTABLE]       = "ITIM uncorrectable ECC error",
    [BEU_EVENT_RESERVED2]                = "Reserved",
    [BEU_EVENT_TILELINK_BUS_ERROR]       = "Load or store TileLink bus error",
    [BEU_EVENT_DATA_CACHE_CORRECTABLE]   = "Data cache correctable ECC error",
    [BEU_EVENT_DATA_CACHE_UNCORRECTABLE] = "Data cache uncorrectable ECC error"
};

const uint64_t BEU_ENABLE_MASK = (BIT(BEU_EVENT_ITIM_CORRECTABLE) | 
    BIT(BEU_EVENT_ITIM_UNCORRECTABLE) | 
    BIT(BEU_EVENT_TILELINK_BUS_ERROR) |
    BIT(BEU_EVENT_DATA_CACHE_CORRECTABLE) | 
    BIT(BEU_EVENT_DATA_CACHE_UNCORRECTABLE));

const uint64_t BEU_ENABLE_UNCORRECTABLE_MASK = (BIT(BEU_EVENT_ITIM_UNCORRECTABLE) | 
    BIT(BEU_EVENT_DATA_CACHE_UNCORRECTABLE));

static struct {
  const enum BEU_Event_Cause bit_position;
  char const * const pName; 
  size_t counter;
} beu_stats_[] = {
  { BEU_EVENT_ITIM_CORRECTABLE, BEU_Event_Name[BEU_EVENT_ITIM_CORRECTABLE], 0llu },
  { BEU_EVENT_ITIM_UNCORRECTABLE, BEU_Event_Name[BEU_EVENT_ITIM_UNCORRECTABLE], 0llu },
  { BEU_EVENT_TILELINK_BUS_ERROR, BEU_Event_Name[BEU_EVENT_TILELINK_BUS_ERROR], 0llu },
  { BEU_EVENT_DATA_CACHE_CORRECTABLE, BEU_Event_Name[BEU_EVENT_DATA_CACHE_CORRECTABLE], 0llu },
  { BEU_EVENT_DATA_CACHE_UNCORRECTABLE, BEU_Event_Name[BEU_EVENT_DATA_CACHE_UNCORRECTABLE], 0llu }
};

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void beu_init_handler(struct StateMachine * const pMyMachine)
{
    for (enum HSSHartId hartid = HSS_HART_U54_1; hartid <= HSS_HART_U54_4; hartid++)
    {
        BEU->regs[hartid].ACCRUED = 0llu;
        BEU->regs[hartid].VALUE = 0llu;
        BEU->regs[hartid].ENABLE = (unsigned long long)BEU_ENABLE_MASK;
    }

    pMyMachine->state++;
}

/////////////////
static void beu_monitoring_handler(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;
    static uint64_t shadow_accrued_[MAX_NUM_HARTS] = { 0llu, };
    static uint64_t shadow_value_[MAX_NUM_HARTS] = { 0llu, };

    for (enum HSSHartId hartid = HSS_HART_U54_1; hartid <= HSS_HART_U54_4; hartid++)
    {
        uint64_t accrued = BEU->regs[hartid].ACCRUED;
        uint64_t value = BEU->regs[hartid].VALUE;

        if (accrued & BEU_ENABLE_MASK) {
            if (accrued & BEU_ENABLE_UNCORRECTABLE_MASK) {
                if ((BEU->regs[hartid].ENABLE) && (value == shadow_value_[hartid])) {
                    mHSS_DEBUG_PRINTF(LOG_ERROR, "Uncorrectable errors: hart %d - error %llu at %p" CRLF, hartid, accrued, value);
                }

                // hart has experienced fatal error, so stop checking for BEU errors for this hart...
                BEU->regs[hartid].ENABLE = 0llu; 
            } else {
                (void)(shadow_accrued_[hartid]); // reference to avoid compiler warning...
            }

            for (size_t i = 0u; i < ARRAY_SIZE(beu_stats_); i++) {
                if (accrued & BIT(beu_stats_[i].bit_position)) {
                    beu_stats_[i].counter++;
                }
            }

            shadow_accrued_[hartid] = accrued;
            shadow_value_[hartid] = value;
        }
    }
}
/////////////////
void HSS_BEU_DumpStats(void)
{
    for (size_t i = 0u; i < ARRAY_SIZE(beu_stats_); i++) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "% 45s:  %" PRIu64 CRLF,
            beu_stats_[i].pName, beu_stats_[i].counter);
    }
}
