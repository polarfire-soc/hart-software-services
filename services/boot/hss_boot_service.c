/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file  Boot Service State Machine
 * \brief Boot Service
 */


#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_boot_service.h"
#include "hss_boot_pmp.h"
#include "hss_sys_setup.h"
#include "hss_clock.h"
#include "hss_debug.h"

#include <assert.h>
#include <string.h>

#include "mpfs_reg_map.h"

#ifdef CONFIG_OPENSBI
#  include "sbi/riscv_asm.h"
//#  include "sbi/sbi_bits.h"
#  include "sbi/sbi_bitops.h"
#  include "sbi/sbi_hart.h"
#  include "sbi_init.h"
#  include "sbi_scratch.h"
#  include "sbi_types.h"
#  include "sbi_platform.h"
#else
#  ifdef __riscv
#    include <machine/mtrap.h>
#    include <machine/encoding.h>
#  endif
#endif

#include "hss_atomic.h"

#ifdef CONFIG_PLATFORM_MPFS
#  include "mss_sysreg.h"
#endif
#include "hss_memcpy_via_pdma.h"

/* Timeouts */
#define BOOT_SETUP_PMP_COMPLETE_TIMEOUT (ONE_SEC * 20u)
#define BOOT_WAIT_TIMEOUT               (ONE_SEC / 20u) /* TODO - refactor out */

/*
 * Module Prototypes (states)
 */
static void boot_init_handler(struct StateMachine * const pMyMachine);
static void boot_setup_pmp_onEntry(struct StateMachine * const pMyMachine);
static void boot_setup_pmp_handler(struct StateMachine * const pMyMachine);
static void boot_setup_pmp_complete_onEntry(struct StateMachine * const pMyMachine);
static void boot_setup_pmp_complete_handler(struct StateMachine * const pMyMachine);
//static void boot_setup_l2cache_handler(struct StateMachine * const pMyMachine);
static void boot_download_chunks_onEntry(struct StateMachine * const pMyMachine);
static void boot_download_chunks_handler(struct StateMachine * const pMyMachine);
static void boot_download_chunks_onExit(struct StateMachine * const pMyMachine);
static void boot_wait_onEntry(struct StateMachine * const pMyMachine);
static void boot_wait_handler(struct StateMachine * const pMyMachine);
static void boot_error_handler(struct StateMachine * const pMyMachine);
static void boot_idle_handler(struct StateMachine * const pMyMachine);

static void boot_do_download_chunk(enum HSSHartId target, struct HSS_BootChunkDesc *pChunk);

/*!
 * \brief Boot Driver States
 *
 */
enum BootStatesEnum {
    BOOT_INITIALIZATION,
    BOOT_SETUP_PMP,
    BOOT_SETUP_PMP_COMPLETE,
    BOOT_DOWNLOAD_CHUNKS,
    BOOT_WAIT,
    BOOT_IDLE,
    BOOT_ERROR,
    BOOT_NUM_STATES = BOOT_ERROR+1
};

/*!
 * \brief Boot Driver State Descriptors
 *
 */
static const struct StateDesc boot_state_descs[] = {
    { (const stateType_t)BOOT_INITIALIZATION,     (const char *)"Init",             NULL,                             NULL,                         &boot_init_handler },
    { (const stateType_t)BOOT_SETUP_PMP,          (const char *)"SetupPMP",         &boot_setup_pmp_onEntry,          NULL,                         &boot_setup_pmp_handler },
    { (const stateType_t)BOOT_SETUP_PMP_COMPLETE, (const char *)"SetupPMPComplete", &boot_setup_pmp_complete_onEntry, NULL,                         &boot_setup_pmp_complete_handler },
    { (const stateType_t)BOOT_DOWNLOAD_CHUNKS,    (const char *)"Download",         &boot_download_chunks_onEntry,    &boot_download_chunks_onExit, &boot_download_chunks_handler },
    { (const stateType_t)BOOT_WAIT,               (const char *)"Wait",             &boot_wait_onEntry,               NULL,                         &boot_wait_handler },
    { (const stateType_t)BOOT_IDLE,               (const char *)"Idle",             NULL,                             NULL,                         &boot_idle_handler },
    { (const stateType_t)BOOT_ERROR,              (const char *)"Error",            NULL,                             NULL,                         &boot_error_handler }
};

/*!
 * \brief Boot Driver State Machine
 *
 */
struct HSS_Boot_LocalData {
    enum HSSHartId target;
    struct HSS_BootChunkDesc *pChunk;
    size_t chunkCount;
    uint32_t msgIndex;
#ifdef CONFIG_SERVICE_BOOT_SETS_SUPPORT
    uint32_t msgIndexAux;
#endif
};

#ifdef CONFIG_SERVICE_BOOT_SETS_SUPPORT
#    define mDEFAULT_MSG_INDEX_AUX 0u
#else
#    define mDEFAULT_MSG_INDEX_AUX
#endif

static struct HSS_Boot_LocalData localData[MAX_NUM_HARTS-1] = {
    //{ HSS_HART_E51,   NULL, 0u, IPI_MAX_NUM_OUTSTANDING_COMPLETES, mDEFAULT_MSG_INDEX_AUX }, // unused
    { HSS_HART_U54_1, NULL, 0u, IPI_MAX_NUM_OUTSTANDING_COMPLETES, mDEFAULT_MSG_INDEX_AUX },
    { HSS_HART_U54_2, NULL, 0u, IPI_MAX_NUM_OUTSTANDING_COMPLETES, mDEFAULT_MSG_INDEX_AUX },
    { HSS_HART_U54_3, NULL, 0u, IPI_MAX_NUM_OUTSTANDING_COMPLETES, mDEFAULT_MSG_INDEX_AUX },
    { HSS_HART_U54_4, NULL, 0u, IPI_MAX_NUM_OUTSTANDING_COMPLETES, mDEFAULT_MSG_INDEX_AUX },
};

struct HSS_BootImage *pBootImage = NULL;
static bool pmpSetupFlag[HSS_HART_NUM_PEERS] = { false, false, false, false, false };

/*
 * individual boot machines, one per U54 hart
 */
struct StateMachine boot_service1 = {
    .state             = (stateType_t)BOOT_IDLE, 
    .prevState         = (stateType_t)SM_INVALID_STATE, 
    .numStates         = (const uint32_t)BOOT_NUM_STATES, 
    .pMachineName      = (const char *)"boot_service(u54_1)", 
    .startTime         = 0u, 
    .lastExecutionTime = 0u, 
    .executionCount    = 0u, 
    .pStateDescs       = boot_state_descs, 
    .debugFlag         = true, 
    .priority          = 0u, 
    .pInstanceData     = (void *)&localData[0]
};

struct StateMachine boot_service2 = {
    .state             = (stateType_t)BOOT_IDLE, 
    .prevState         = (stateType_t)SM_INVALID_STATE, 
    .numStates         = (const uint32_t)BOOT_NUM_STATES, 
    .pMachineName      = (const char *)"boot_service(u54_2)", 
    .startTime         = 0u, 
    .lastExecutionTime = 0u, 
    .executionCount    = 0u, 
    .pStateDescs       = boot_state_descs, 
    .debugFlag         = true, 
    .priority          = 0u, 
    .pInstanceData     = (void *)&localData[0]
};

struct StateMachine boot_service3 = {
    .state             = (stateType_t)BOOT_IDLE, 
    .prevState         = (stateType_t)SM_INVALID_STATE, 
    .numStates         = (const uint32_t)BOOT_NUM_STATES, 
    .pMachineName      = (const char *)"boot_service(u54_3)", 
    .startTime         = 0u, 
    .lastExecutionTime = 0u, 
    .executionCount    = 0u, 
    .pStateDescs       = boot_state_descs, 
    .debugFlag         = true, 
    .priority          = 0u, 
    .pInstanceData     = (void *)&localData[0]
};

struct StateMachine boot_service4 = {
    .state             = (stateType_t)BOOT_IDLE, 
    .prevState         = (stateType_t)SM_INVALID_STATE, 
    .numStates         = (const uint32_t)BOOT_NUM_STATES, 
    .pMachineName      = (const char *)"boot_service(u54_4)", 
    .startTime         = 0u, 
    .lastExecutionTime = 0u, 
    .executionCount    = 0u, 
    .pStateDescs       = boot_state_descs, 
    .debugFlag         = true, 
    .priority          = 0u, 
    .pInstanceData     = (void *)&localData[0]
};

/*
 * table of individual boot machines, for convenience
 */
const struct {
    struct StateMachine* const pMachine;
    const enum HSSHartId hartId;
} bootMachine[] = {
    { &boot_service1, HSS_HART_U54_1 },
    { &boot_service2, HSS_HART_U54_2 },
    { &boot_service3, HSS_HART_U54_3 },
    { &boot_service4, HSS_HART_U54_4 },
};

// ----------------------------------------------------------------------------
/*!
 * \brief Helper Functions
 */

/*!
 * \brief Download Chunks
 *
 * The E51 places chunks from the boot image to their correct location in
 * physical system memory *before* releasing the U54s from WFI.
 *
 * \warning It is vitally important that we permissions check the bounds of each of
 * these reads/writes w.r.t. the U54 that owns it to make sure that is a permitted
 * write. Otherwise, a boot image could be constructed to circumvent these
 * protections.
 *
 */
static void boot_do_download_chunk(enum HSSHartId target, struct HSS_BootChunkDesc *pChunk)
{
    assert(pChunk->size);

    if (HSS_PMP_CheckWrite(target, (void *)pChunk->execAddr, pChunk->size)) {
        memcpy_via_pdma((void *)pChunk->execAddr,
            (void*)((size_t)pBootImage + (size_t)pChunk->loadAddr), pChunk->size);
    } else {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "Target %d is skipping chunk %p due to invalid permissions" CRLF,
            target, pChunk);
    }
}

static void free_msg_index(struct HSS_Boot_LocalData * const pInstanceData)
{
    if (pInstanceData->msgIndex != IPI_MAX_NUM_OUTSTANDING_COMPLETES) {
        IPI_MessageFree(pInstanceData->msgIndex);
        pInstanceData->msgIndex = IPI_MAX_NUM_OUTSTANDING_COMPLETES;
    }
}

static void free_msg_index_aux(struct HSS_Boot_LocalData * const pInstanceData)
{
#ifdef CONFIG_SERVICE_BOOT_SETS_SUPPORT
    if (pInstanceData->msgIndexAux != IPI_MAX_NUM_OUTSTANDING_COMPLETES) {
        IPI_MessageFree(pInstanceData->msgIndexAux);
        pInstanceData->msgIndexAux = IPI_MAX_NUM_OUTSTANDING_COMPLETES;
    }
#endif
}


static bool check_for_ipi_acks(struct StateMachine * const pMyMachine)
{
    struct HSS_Boot_LocalData * const pInstanceData = pMyMachine->pInstanceData;
    bool result = true;

#ifdef CONFIG_SERVICE_BOOT_SETS_SUPPORT
    // TODO: this works for 2 cores, but needs to be extended for up to 4...
    //
    if (pInstanceData->msgIndexAux != IPI_MAX_NUM_OUTSTANDING_COMPLETES) {
        result = IPI_MessageCheckIfComplete(pInstanceData->msgIndexAux);

        if (result) {
            free_msg_index_aux(pInstanceData);
        }
    }
#endif

    if (pInstanceData->msgIndex != IPI_MAX_NUM_OUTSTANDING_COMPLETES) {
        result = result & IPI_MessageCheckIfComplete(pInstanceData->msgIndex);

        if (result) {
            free_msg_index(pInstanceData);
        }
    }

    return result;
}


// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//
static void boot_init_handler(struct StateMachine * const pMyMachine)
{
    if (pBootImage) {
        //mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s::\tstarting boot" CRLF, pMyMachine->pMachineName);

        pMyMachine->state = BOOT_SETUP_PMP;
    } else {
        // unexpected error state
        if (!pBootImage) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "%s::\tNo Boot Image registered" CRLF, pMyMachine->pMachineName);
        }
        pMyMachine->state = BOOT_ERROR;
    }
}

/////////////////

static void boot_setup_pmp_onEntry(struct StateMachine * const pMyMachine)
{
    struct HSS_Boot_LocalData * const pInstanceData = pMyMachine->pInstanceData;

    pInstanceData->msgIndex = IPI_MAX_NUM_OUTSTANDING_COMPLETES;
#ifdef CONFIG_SERVICE_BOOT_SETS_SUPPORT
    pInstanceData->msgIndexAux = IPI_MAX_NUM_OUTSTANDING_COMPLETES;
#endif
}

static void boot_setup_pmp_handler(struct StateMachine * const pMyMachine)
{
    struct HSS_Boot_LocalData * const pInstanceData = pMyMachine->pInstanceData;
    enum HSSHartId const target = pInstanceData->target;

    bool result = false;

    if (pInstanceData->msgIndex == IPI_MAX_NUM_OUTSTANDING_COMPLETES) {
        result = HSS_Boot_PMPSetupRequest(target, &(pInstanceData->msgIndex));
    } else {
        result = true;
    }

    if (result) {
        pMyMachine->state = BOOT_SETUP_PMP_COMPLETE;
    }
}

static void boot_setup_pmp_complete_onEntry(struct StateMachine * const pMyMachine)
{
    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s::Checking for IPI ACKs: - -" CRLF, pMyMachine->pMachineName);
}

static void boot_setup_pmp_complete_handler(struct StateMachine * const pMyMachine)
{
    if (HSS_Timer_IsElapsed(pMyMachine->startTime, BOOT_SETUP_PMP_COMPLETE_TIMEOUT)) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "%s::Timeout after %" PRIu64 " iterations" CRLF, pMyMachine->pMachineName,
            pMyMachine->executionCount);

        struct HSS_Boot_LocalData * const pInstanceData = pMyMachine->pInstanceData;
#ifdef CONFIG_SERVICE_BOOT_SETS_SUPPORT
        free_msg_index_aux(pInstanceData);
#endif
        free_msg_index(pInstanceData);

        pMyMachine->state = BOOT_ERROR;
    } else {
        // need to free as received, not all at once...

        if (check_for_ipi_acks(pMyMachine)) {
            //mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s::Checking for IPI ACKs: ACK/IDLE ACK" CRLF, pMyMachine->pMachineName);
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s::PMP setup completed" CRLF, pMyMachine->pMachineName);

            pMyMachine->state = BOOT_DOWNLOAD_CHUNKS;
        }
    }
}

/////////////////

static void boot_download_chunks_onEntry(struct StateMachine * const pMyMachine)
{
    struct HSS_Boot_LocalData * const pInstanceData = pMyMachine->pInstanceData;
    enum HSSHartId const target = pInstanceData->target;

    assert(pBootImage != NULL);

    if (pBootImage->hart[target-1].numChunks) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s::Processing boot image:" CRLF "  \"%s\"" CRLF,
            pMyMachine->pMachineName, pBootImage->hart[target-1].name);
        pInstanceData->pChunk =
            (struct HSS_BootChunkDesc *)((char *)pBootImage + pBootImage->chunkTableOffset);

#ifdef CONFIG_DEBUG_CHUNK_DOWNLOADS
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s::firstChunk is %u" CRLF, pMyMachine->pMachineName, pBootImage->hart[target-1].firstChunk);
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s::lastChunk is %u" CRLF, pMyMachine->pMachineName, pBootImage->hart[target-1].lastChunk);
#endif

        pInstanceData->chunkCount = 0u;
        pInstanceData->pChunk += pBootImage->hart[target-1].firstChunk;
    } else {
        // nothing to do for this machine, numChunks is zero...
    }
}

static void boot_download_chunks_handler(struct StateMachine * const pMyMachine)
{
    struct HSS_Boot_LocalData * const pInstanceData = pMyMachine->pInstanceData;
    enum HSSHartId const target = pInstanceData->target;

    if (pBootImage->hart[target-1].numChunks) {
        //
        // end of image is denoted by sentinel chunk with zero size...
        // so if we're not on the sentinel chunk
        if ((pInstanceData->chunkCount <= pBootImage->hart[target-1].lastChunk)
            && (pInstanceData->pChunk->size)) {
            //
            // and it is for us, then download it
            if (pInstanceData->pChunk->owner == target) {
#ifdef CONFIG_DEBUG_CHUNK_DOWNLOADS
                mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s::%d:chunk@0x%X->0x%X, %u bytes" CRLF, pMyMachine->pMachineName,
                    pInstanceData->chunkCount, (uint64_t)pInstanceData->pChunk->loadAddr,
                    (uint64_t)pInstanceData->pChunk->execAddr, pInstanceData->pChunk->size);
#endif
                // check each hart to see if it wants to transmit
                boot_do_download_chunk(pInstanceData->pChunk->owner, pInstanceData->pChunk);
                pInstanceData->chunkCount++;
                pInstanceData->pChunk++;
            } else {
                pInstanceData->pChunk++;
            }
            //
            // either way, with a non-sentinel chunk,
            // move onto next chunk, which will be handled by next time into state machine...
        } else {
            //
            // otherwise we are on a sentinel chunk and thus we are
            // finished processing the image
            pMyMachine->state = BOOT_WAIT;
        }
    } else {
        pMyMachine->state = BOOT_IDLE;
    }
}

static void boot_download_chunks_onExit(struct StateMachine * const pMyMachine)
{
    struct HSS_Boot_LocalData * const pInstanceData = pMyMachine->pInstanceData;
    enum HSSHartId const target = pInstanceData->target;

    // otherwise if it has a valid entry point, allocate a message for it and send a GOTO IPI
    if (pBootImage->hart[target-1].entryPoint) {
        bool result;

        if (pBootImage->hart[target-1].numChunks) {
#ifdef CONFIG_SERVICE_BOOT_SETS_SUPPORT
            // in interrupts-always-enabled world of the HSS, it would appear less
            // racey to boot secondary cores first and have them all wait...
            for (unsigned int i = 0u; i < mSPAN_OF(bootMachine); i++) {
                enum HSSHartId peer = bootMachine[i].hartId;

                if (peer == target) { continue; } // skip myself

                if (pBootImage->hart[peer-1].entryPoint == pBootImage->hart[target-1].entryPoint) {
                    // found another hart in same boot set as me...

                    mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s::u54_%u:sbi_init %p" CRLF, pMyMachine->pMachineName,
                        peer, pBootImage->hart[peer-1].entryPoint);
                    result = IPI_MessageAlloc(&(pInstanceData->msgIndexAux));
                    assert(result);

                    mb();
                    mb_i();

                    result = IPI_MessageDeliver(pInstanceData->msgIndexAux, peer,
#if 0
                        IPI_MSG_GOTO,
#else
                        IPI_MSG_OPENSBI_INIT,
#endif
                        pBootImage->hart[peer-1].privMode,
                        &(pBootImage->hart[peer-1].entryPoint));
                    assert(result);

                    //HSS_SpinDelay_MilliSecs(1);
                }
            }
#endif
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s::u54_%u:sbi_init %p" CRLF, pMyMachine->pMachineName,
                target, pBootImage->hart[target-1].entryPoint);
            result = IPI_MessageAlloc(&(pInstanceData->msgIndex));
            assert(result);

            mb();
            mb_i();

            //HSS_SpinDelay_MilliSecs(50); //TBD

            result = IPI_MessageDeliver(pInstanceData->msgIndex, target,
#if 0
                IPI_MSG_GOTO,
#else
                IPI_MSG_OPENSBI_INIT,
#endif
                pBootImage->hart[target-1].privMode, &(pBootImage->hart[target-1].entryPoint));
            assert(result);
        }
    } else {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s::target is %u, pBootImage is %p, skipping sbi_init %p" CRLF,
            pMyMachine->pMachineName, target, pBootImage, pBootImage->hart[target-1].entryPoint);
    }
}

/////////////////

static void boot_wait_onEntry(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s::Checking for IPI ACKs: - -" CRLF, pMyMachine->pMachineName);
}

static void boot_wait_handler(struct StateMachine * const pMyMachine)
{
    struct HSS_Boot_LocalData * const pInstanceData = pMyMachine->pInstanceData;
    enum HSSHartId const target = pInstanceData->target;

    if (!pBootImage->hart[target-1].entryPoint) {
        // nothing for me to do, not expecting GOTO ack...
        pMyMachine->state = BOOT_IDLE;
    } else if (HSS_Timer_IsElapsed(pMyMachine->startTime, BOOT_WAIT_TIMEOUT)) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "%s::Timeout after %" PRIu64 " iterations" CRLF,
            pMyMachine->pMachineName, pMyMachine->executionCount);

#ifdef CONFIG_SERVICE_BOOT_SETS_SUPPORT
        free_msg_index_aux(pInstanceData);
#endif
        free_msg_index(pInstanceData);

        pMyMachine->state = BOOT_ERROR;
    } else {
        // need to free as received, not all at once...
        if (check_for_ipi_acks(pMyMachine)) {
#ifdef CONFIG_PLATFORM_MPFS
            // turn appropriate bit on in SYSREGSCB:MSS_STATUS:BOOT_STATUS to indicate it is up
            // note: this bit is a status indicator to SW only, and is not functional/does not have side effects
            mHSS_ReadModWriteRegU32(SYSREGSCB, MSS_STATUS, 0xFFFFu, 1u << (target-1));
#endif

            mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s::Checking for IPI ACKs: ACK/IDLE ACK" CRLF,
                pMyMachine->pMachineName);
            pMyMachine->state = BOOT_IDLE;
        }
    }
}

/////////////////

static void boot_error_handler(struct StateMachine * const pMyMachine)
{
    mHSS_DEBUG_PRINTF(LOG_ERROR, "%s::" CRLF
        "*******************************************************************" CRLF
        "* WARNING: Boot Error - transitioning to IDLE                     *" CRLF
        "*******************************************************************" CRLF,
        pMyMachine->pMachineName);

    pMyMachine->state = BOOT_IDLE;
}


/////////////////

static void boot_idle_handler(struct StateMachine * const pMyMachine)
{
    struct HSS_Boot_LocalData const * const pInstanceData = pMyMachine->pInstanceData;
    IPI_ConsumeIntent(pInstanceData->target, IPI_MSG_BOOT_REQUEST); // check for boot requests
}



// ----------------------------------------------------------------------------
// PUBLIC API
//

bool HSS_Boot_Harts(enum HSSHartId const source)
{
    bool result = false;

    // TODO: should it restart all, or one the non-busy boot state machines??
    //
    for (unsigned int i = 0u; i < mSPAN_OF(bootMachine); i++) {
        if ((source == HSS_HART_ALL) || (source == bootMachine[i].hartId)) {
            struct StateMachine * const pMachine = bootMachine[i].pMachine;

#if 1
            if (pMachine->state == BOOT_SETUP_PMP_COMPLETE) { // TBD
               pMachine->state = (stateType_t)BOOT_INITIALIZATION;
               result = true;
            } else
#endif
            if ((pMachine->state == BOOT_INITIALIZATION) || (pMachine->state == BOOT_IDLE)) {
               pMachine->state = (stateType_t)BOOT_INITIALIZATION;
               result = true;
            } else {
               result = false;
               mHSS_DEBUG_PRINTF(LOG_ERROR, "invalid hart state %d for hart %u" CRLF, pMachine->state, i);
            }
        }
    }

    return result;
}


enum IPIStatusCode HSS_Boot_RestartCore(enum HSSHartId source)
{
    enum IPIStatusCode result = IPI_FAIL;

    if (source != HSS_HART_ALL) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "called for hart %u" CRLF, source);
    } else {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "called for all harts" CRLF);
    }

    if (HSS_Boot_Harts(source)) {
        result = IPI_SUCCESS;
    }

    return result;
}

enum IPIStatusCode HSS_Boot_IPIHandler(TxId_t transaction_id, enum HSSHartId source,
    uint32_t immediate_arg, void *p_extended_buffer_in_ddr)
{
    (void)transaction_id;
    (void)source;
    (void)immediate_arg;
    (void)p_extended_buffer_in_ddr;

    // boot strap IPI received from one of the U54s...
    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "called" CRLF);

    return HSS_Boot_RestartCore(source);
}

void HSS_Register_Boot_Image(struct HSS_BootImage *pImage)
{
    pBootImage = pImage;
}

/*!
 * \brief PMP Setup Handler
 *
 * Handle request to U54 from E51 to setup PMPs
 *
 */
enum IPIStatusCode HSS_Boot_PMPSetupHandler(TxId_t transaction_id, enum HSSHartId source,
    uint32_t immediate_arg, void *p_extended_buffer_in_ddr)
{
    enum IPIStatusCode result = IPI_FAIL;
    (void)transaction_id;
    (void)source;
    (void)immediate_arg;
    (void)p_extended_buffer_in_ddr;

    // request to setup PMP by E51 received
    enum HSSHartId myHartId = current_hartid();

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Hart%u ", myHartId);

    if (!pmpSetupFlag[myHartId]) {
        pmpSetupFlag[myHartId] = true; // PMPs can be setup only once without reboot....

        HSS_Setup_PMP();

        mHSS_DEBUG_PRINTF_EX("setup complete" CRLF);
        result =  IPI_SUCCESS;
    } else {
        mHSS_DEBUG_PRINTF_EX("PMPs already configured" CRLF);
        result =  IPI_SUCCESS;
    }

    return result;
}

/*!
 * \brief PMP Setup Request
 *
 * Make request to U54 from E51 to setup PMPs
 *
 */
bool HSS_Boot_PMPSetupRequest(enum HSSHartId target, uint32_t *indexOut)
{
    // setup PMP for each hart ...
    bool result;

    assert(target != HSS_HART_ALL); // need to setup PMPs on each Hart individually

    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "called for hart %u" CRLF, target);

    result = IPI_MessageAlloc(indexOut);
    assert(result);

    if (result) {
        result = IPI_MessageDeliver(*indexOut, target, IPI_MSG_PMP_SETUP, 0u, NULL);

        // couldn't send message, so free up resources...
        if (!result) {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "hart %u: failed to send message, so freeing" CRLF, target); //TODO
            IPI_MessageFree(*indexOut);
        }
    }

    return result;
}

/*!
 * \brief SBI Setup Request
 *
 * Make request to U54 from E51 to setup SBIs
 *
 */
bool HSS_Boot_SBISetupRequest(enum HSSHartId target, uint32_t *indexOut)
{
    // setup SBI for each hart ...
    bool result;

    assert(target != HSS_HART_ALL); // need to setup SBIs on each Hart individually

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "called for hart %u" CRLF, target);

    result = IPI_MessageAlloc(indexOut);
    assert(result);

    if (result) {
        result = IPI_MessageDeliver(*indexOut, target, IPI_MSG_OPENSBI_INIT, 0u, NULL);

        // couldn't send message, so free up resources...
        if (!result) {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "hart %u: failed to send message, so freeing" CRLF, target); //TODO
            IPI_MessageFree(*indexOut);
        }
    }

    return result;
}
