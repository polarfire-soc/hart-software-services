/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file SSMB Core
 * \brief SSMB Core Definitions
 */

#include "config.h"
#include "hss_types.h"

#ifndef CONFIG_OPENSBI
#  ifdef __riscv
#    include <machine/mtrap.h>
#    include <machine/encoding.h>
#  endif
#else
#endif

#include "ssmb_ipi.h"
#include "hss_debug.h"

#include "mpfs_reg_map.h"
#include "csr_helper.h"

#include "hss_state_machine.h"
#include "hss_registry.h"
#include "hss_atomic.h"

#if IS_ENABLED(CONFIG_HSS_USE_IHC)
#  include "miv_ihc.h"
#endif

#include <string.h>
#include <assert.h>

/////////////////////////////////////////////////////////////////////////////

// IPI queues - to be placed at well known address in memory and PMP protected

#define SIZE_OF_IPI_QUEUES    (sizeof(struct IPI_Outbox_Queue) * IPI_OUTBOX_NUM_QUEUES)
#define SIZE_OF_IPI_COMPLETES (sizeof(struct IPI_Complete) * IPI_MAX_NUM_OUTSTANDING_COMPLETES)


#define IPI_VERSION (0x0101u)

/////////////////////////////////////////////////////////////////////////////

struct IPI_Data {
    uint32_t ipi_version;
    struct IPI_Outbox_Queue ipi_queues[IPI_OUTBOX_NUM_QUEUES];
    struct IPI_Complete ipi_completes[IPI_MAX_NUM_OUTSTANDING_COMPLETES];
    struct {
        TxId_t last[IPI_OUTBOX_NUM_QUEUES];
        TxId_t shadow[IPI_OUTBOX_NUM_QUEUES];
    } txId_per_queue;
    TxId_t my_transaction_id[MAX_NUM_HARTS];
    struct
    {
        size_t message_allocs;
        size_t message_delivers;
        size_t message_frees;
        size_t consume_intents;
        size_t ipi_sends;
        //size_t msg_types[IPI_MSG_NUM_MSG_TYPES];
    } mpfs_ipi_privateData[MAX_NUM_HARTS];
};

#define IPI_SIZE sizeof(struct IPI_Data)

#if IS_ENABLED(CONFIG_IPI_FIXED_BASE_ADDR)
   static struct IPI_Data *ipi_data = (struct IPI_Data *)CONFIG_IPI_FIXED_BASE_ADDR;
#  define IPI_DATA (*ipi_data)
#else
   static struct IPI_Data ipi_data_array;
   static struct IPI_Data *ipi_data = (struct IPI_Data *)&ipi_data_array;
#  define IPI_DATA (*ipi_data)
#endif

#if IS_ENABLED(CONFIG_DEBUG_MSCGEN_IPI)
__extension__ static const char * const hartName[] = { // MAX_NUM_HARTS
    [ HSS_HART_E51 ]   = "E51",
    [ HSS_HART_U54_1 ] = "U54_1",
    [ HSS_HART_U54_2 ] = "U54_2",
    [ HSS_HART_U54_3 ] = "U54_3",
    [ HSS_HART_U54_4 ] = "U54_4"
};

__extension__ static const char * const ipiName[] = { // IPI_MSG_NUM_MSG_TYPES
    [ IPI_MSG_NO_MESSAGE ]        = "IPI_MSG_NO_MESSAGE",
    [ IPI_MSG_BOOT_REQUEST ]      = "IPI_MSG_BOOT_REQUEST",
    [ IPI_MSG_PMP_SETUP ]         = "IPI_MSG_PMP_SETUP",
    [ IPI_MSG_SPI_XFER ]          = "IPI_MSG_SPI_XFER",
    [ IPI_MSG_NET_RXPOLL ]        = "IPI_MSG_NET_RXPOLL",
    [ IPI_MSG_NET_TX ]            = "IPI_MSG_NET_TX",
    [ IPI_MSG_SCATTERGATHER_DMA ] = "IPI_MSG_SCATTERGATHER_DMA",
    [ IPI_MSG_WDOG_INIT ]         = "IPI_MSG_WDOG_INIT",
    [ IPI_MSG_GPIO_SET ]          = "IPI_MSG_GPIO_SET",
    [ IPI_MSG_UART_TX ]           = "IPI_MSG_UART_TX",
    [ IPI_MSG_UART_POLL_RX ]      = "IPI_MSG_UART_POLL_RX",
    [ IPI_MSG_POWERMODE ]         = "IPI_MSG_POWERMODE",
    [ IPI_MSG_FLASHFREEZE ]       = "IPI_MSG_FLASHFREEZE",
    [ IPI_MSG_ACK_PENDING ]       = "IPI_MSG_ACK_PENDING",
    [ IPI_MSG_ACK_COMPLETE ]      = "IPI_MSG_ACK_COMPLETE",
    [ IPI_MSG_HALT ]              = "IPI_MSG_HALT",
    [ IPI_MSG_CONTINUE ]          = "IPI_MSG_CONTINUE",
    [ IPI_MSG_GOTO ]              = "IPI_MSG_GOTO",
    [ IPI_MSG_OPENSBI_INIT ]      = "IPI_MSG_OPENSBI_INIT"
};
#endif

/////////////////////////////////////////////////////////////////////////////

//
// @brief Dump IPI Debug Statistics and Counters
//
void IPI_DebugDumpStats(void)
{
#if IS_ENABLED(CONFIG_DEBUG_IPI_STATS) || IS_ENABLED(CONFIG_SERVICE_TINYCLI)
    enum HSSHartId myHartId = current_hartid();
    for (myHartId = HSS_HART_E51; myHartId < HSS_HART_NUM_PEERS; myHartId++) {
        mHSS_DEBUG_PRINTF(LOG_STATUS, "hartId:           %u" CRLF, myHartId);
        mHSS_DEBUG_PRINTF(LOG_STATUS, "message_allocs:   %" PRIu64 CRLF,
            IPI_DATA.mpfs_ipi_privateData[myHartId].message_allocs);
        mHSS_DEBUG_PRINTF(LOG_STATUS, "message_delivers: %" PRIu64 CRLF,
            IPI_DATA.mpfs_ipi_privateData[myHartId].message_delivers);
        mHSS_DEBUG_PRINTF(LOG_STATUS, "message_frees:    %" PRIu64 CRLF,
            IPI_DATA.mpfs_ipi_privateData[myHartId].message_allocs);
        mHSS_DEBUG_PRINTF(LOG_STATUS, "consume_intents:  %" PRIu64 CRLF,
            IPI_DATA.mpfs_ipi_privateData[myHartId].consume_intents);
        mHSS_DEBUG_PRINTF(LOG_STATUS, "ipi_sends:        %" PRIu64 CRLF CRLF,
            IPI_DATA.mpfs_ipi_privateData[myHartId].ipi_sends);
    }
#endif
}

//
// @brief Given a source hart and target hart, calculate the queue index which
// represents its message traffic.
//
// The calculation logic here may be a little confusing, but in essence what it is
// doing is mapping queue direction to index as follows:
//
// @verbatim
//   E51->U54_1 => 0,  E51->U54_2   => 1,  E51->U54_3   => 2,  E51->U54_4   => 3
//   U54_1->E51 => 4,  U54_1->U54_2 => 5,  U54_1->U54_3 => 6,  U54_1->U54_4 => 7
//   U54_2->E51 => 8,  U54_2->U54_1 => 9,  U54_2->U54_3 => 10, U54_2->U54_4 => 11
//   U54_3->E51 => 12, U54_3->U54_1 => 13, U54_3->U54_2 => 14, U54_3->U54_4 => 15
//   U54_4->E51 => 16, U54_4->U54_1 => 17, U54_4->U54_2 => 18, U54_4->U53_4 => 19
// @endverbatim
//
// @param source [in] source hart from where the traffic will originate
// @param target [in] target hart to where the traffic will be sent
// @return uint32_t representing queue index
//
inline uint32_t IPI_CalculateQueueIndex(enum HSSHartId source, enum HSSHartId target)
{
     uint32_t index = (source * 4u) + target;
     if (target > source) {
         index--;
     }
     return index;
}

//
// @brief Get pending count of IPI messages on a particular queue
// @param queueIndex [in] target queue
// @return uint32_t representing the count of active messages
//
uint32_t IPI_GetQueuePendingCount(uint32_t queueIndex)
{
    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "called - queueIndex is %u, count is %u" CRLF, queueIndex,
    //    IPI_DATA.ipi_queues[queueIndex].count);
    return (IPI_DATA.ipi_queues[queueIndex].count);
}

// @brief Send an IPI to a particular target hart
// @param target [in] target hart to send an IPI to
// @return bool indicating success
//

static bool clint_set_MSIP(enum HSSHartId const target, uint32_t value)
{
    bool result = true;

    switch (target) {
    case HSS_HART_E51:
        mHSS_WriteRegU32(CLINT, MSIP_E51_0, value);
        break;

    case HSS_HART_U54_1:
        mHSS_WriteRegU32(CLINT, MSIP_U54_1, value);
        break;

    case HSS_HART_U54_2:
        mHSS_WriteRegU32(CLINT, MSIP_U54_2, value);
        break;

    case HSS_HART_U54_3:
        mHSS_WriteRegU32(CLINT, MSIP_U54_3, value);
        break;

    case HSS_HART_U54_4:
        mHSS_WriteRegU32(CLINT, MSIP_U54_4, value);
        break;

    default:
        assert((target >= HSS_HART_E51) && (target <= HSS_HART_U54_4));
        result = false;
    }

    return result;
}

bool CLINT_Raise_MSIP(enum HSSHartId const target)
{
    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "sending IPI to %u" CRLF, target);
    bool result = clint_set_MSIP(target, 1u);

    if (result) {
        IPI_DATA.mpfs_ipi_privateData[target].ipi_sends++;
    }

    return result;
}

// @brief Clear the MSIP bit of a particular target hart
// @param target [in] target hart to send an IPI to
//
void CLINT_Clear_MSIP(enum HSSHartId const target)
{
    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "clearing IPI on %u" CRLF, target);
    (void)clint_set_MSIP(target, 0u);
}

//
// @brief Given a source hart and target hart, find a pointer to the first message in its queue
// @param source [in] source hart from where the traffic will originate
// @param target [in] target hart to where the traffic will be sent
// @return IPI_Outbox_Msg * pointer to first message in the queue
//
inline struct IPI_Outbox_Msg *IPI_DirectionToFirstMsgInQueue(enum HSSHartId source,
        enum HSSHartId target)
{
    assert(target < HSS_HART_NUM_PEERS);
    assert(target != source);

    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Resolved (%d, %d) to index %u" CRLF, source, target, index);
    //mHSS_DEBUG_PRINTF(WARN, "WARNING: queue %u is %u" CRLF, index,
    //    IPI_DATA.ipi_queues[index].count);

    uint32_t index = IPI_CalculateQueueIndex(source, target);
    struct IPI_Outbox_Msg *pMsgResult = &(IPI_DATA.ipi_queues[index].msgQ[0]);

    return pMsgResult;
}

//
// @brief Given a source hart and target hart, find a pointer to the first message in its queue
// @param target [in] target hart to where the traffic will be sent
// @param message [in] HSS Message Type
// @param transaction_id [in] Transaction Identifier
// @param immediate_arg [in] Optional uint32_t immediate argument
// @param p_extended_buffer_in_ddr [in] Optional pointer to extended buffer in memory
// @param p_ancilliar_buffer_in_ddr [in] Optional pointer to ancilliary buffer in memory
// @return bool indicating message was sent okay
//

bool IPI_Send(enum HSSHartId target, enum IPIMessagesEnum message, TxId_t transaction_id,
        uint32_t immediate_arg, void const *p_extended_buffer_in_ddr,
        void const *p_ancilliary_buffer_in_ddr) {
    bool result = false;
    uint32_t i = 0u;

    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "called with message type of %u to %d" CRLF, message, target);

    // find where to put the message
    uint32_t index = IPI_CalculateQueueIndex(current_hartid(), target);
    struct IPI_Outbox_Msg *pMsg = &(IPI_DATA.ipi_queues[index].msgQ[0]);
    bool space_available = false;

    assert(pMsg != NULL);

    for (i = 0u; i < IPI_MAX_NUM_QUEUE_MESSAGES; i++) {
        if (pMsg->msg_type == IPI_MSG_NO_MESSAGE) {
            space_available = true;
            break;
        } else {
            //mHSS_DEBUG_PRINTF(LOG_NORMAL, "pMsg: %p, index: %u, msg_type: %u" CRLF, pMsg, i, pMsg->msg_type);
        }
        pMsg++;
    }

    if (space_available) {
        pMsg->msg_type = message;
        pMsg->transaction_id = transaction_id;
        pMsg->p_extended_buffer_in_ddr = (void *)p_extended_buffer_in_ddr;
        pMsg->p_ancilliary_buffer_in_ddr = (void *)p_ancilliary_buffer_in_ddr;
        pMsg->immediate_arg = immediate_arg;

        IPI_DATA.txId_per_queue.last[index] = transaction_id;

#if IS_ENABLED(CONFIG_HSS_USE_IHC)
        const uint32_t hss_message[] = { (uint32_t)message, (uint32_t)transaction_id, 0x0, 0x0 };
        uint32_t tx_status = IHC_tx_message((IHC_CHANNEL)target, (uint32_t *)&hss_message);
        if (tx_status == MESSAGE_SENT) {
            result = true;
	}
#else
        result = CLINT_Raise_MSIP(target);
#endif
    } else {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "No space in queue!!!!!!" CRLF);
    }

#if IS_ENABLED(CONFIG_DEBUG_MSCGEN_IPI)
    {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "::mscgen: %s->%s %s %u %u %p %p" CRLF,
            hartName[current_hartid()], hartName[target],
            ipiName[message], transaction_id, immediate_arg,
            p_extended_buffer_in_ddr, p_ancilliary_buffer_in_ddr);
    }
#endif

    return result;
}

//
// @brief Polled incoming IPI queues and counts messages
// @param target [in] target hart to where the traffic will be sent
// @param message [in] HSS Message Type
// @param transaction_id [in] Transaction Identifier
// @param immediate_arg [in] Optional uint32_t immediate argument
// @param p_extended_buffer_in_ddr [in] Optional pointer to extended buffer in memory
// @return bool indicating queue messages have changed
//
bool IPI_PollReceive(union HSSHartBitmask hartMask)
{
    bool result = false;
    uint32_t i;

    (void)hartMask;
    enum HSSHartId const myHartId = current_hartid();

    for (i = 0u; i < MAX_NUM_HARTS; i++) {
#if IS_ENABLED(CONFIG_HSS_USE_IHC)
        IHC_message_present_poll();
        __sync_synchronize();
#endif

        if (i == myHartId) { continue; } // don't handle messages if to my own hartid
        if (!((1u << i) & hartMask.uint)) { continue; } // only look at selected harts

        // myHartId => target, i => source
        uint32_t const index = IPI_CalculateQueueIndex(i, myHartId);
        if (IPI_DATA.txId_per_queue.shadow[index] == IPI_DATA.txId_per_queue.last[index]) {
            // nothing new since last time, so continue
            continue;
        } else {
            IPI_DATA.txId_per_queue.shadow[index] = IPI_DATA.txId_per_queue.last[index];

            uint32_t countPerQueue = 0u;
            uint32_t j;

            for (j = 0u; j < IPI_MAX_NUM_QUEUE_MESSAGES; j++) {
                if (IPI_DATA.ipi_queues[index].msgQ[j].msg_type != IPI_MSG_NO_MESSAGE) {
                    result = true; // incoming message to handle
                    countPerQueue++;

                    //if (result) { mHSS_DEBUG_PRINTF(NLOG_ORMAL, "q(%u)/msg(%u): received IPI (type %d)"
                    //     CRLF, index, j, IPI_DATA.ipi_queues[index].msgQ[j].msg_type); }
                }
            }

            IPI_DATA.ipi_queues[index].count = countPerQueue;
            result = true;
        }
    }

    return result;
}

bool IPI_ConsumeIntent(enum HSSHartId source, enum IPIMessagesEnum msg_type)
{
    bool intentFound = false;
    enum HSSHartId const myHartId = current_hartid();

    //uint32_t index = IPI_CalculateQueueIndex(source, myHartId);
    //if (IPI_GetQueuePendingCount(index)) {

    {
        // find appropriate starting point
        uint32_t const index = IPI_CalculateQueueIndex(source, myHartId);
        struct IPI_Outbox_Msg *pMsg = &(IPI_DATA.ipi_queues[index].msgQ[0]);
        IPI_handlerFunction pHandler = NULL;

        assert(pMsg != NULL);

        // search for handler function...
        uint32_t j=0u;

        // direct look-up for speed into the table
        // to make this safer, we'll ensure its within range, and we'll also
        // assert that it is the correct handler type...
        {
            assert(msg_type < spanOfIpiRegistry);
            assert(msg_type == ipiRegistry[msg_type].msg_type);

            if ((msg_type < spanOfIpiRegistry) && (msg_type == ipiRegistry[msg_type].msg_type)) {
                pHandler = ipiRegistry[msg_type].handler;
            }
        }

        // check the queue for a message of the required type
        for (j = 0u; j < IPI_MAX_NUM_QUEUE_MESSAGES; j++) {
            if (pMsg->msg_type == msg_type) {
#if IS_ENABLED(CONFIG_DEBUG_MSCGEN_IPI)
                mHSS_DEBUG_PRINTF(LOG_NORMAL, "::mscgen: %s->%s %s %u %u %p %p" CRLF,
                    hartName[source], hartName[current_hartid()],
                    ipiName[msg_type], pMsg->transaction_id, pMsg->immediate_arg,
                        pMsg->p_extended_buffer_in_ddr, pMsg->p_ancilliary_buffer_in_ddr);
#endif
                if (!pHandler) { // ensure we don't fill up queue with unhandlable messages
                    mHSS_DEBUG_PRINTF(LOG_ERROR, "no handler found for IPI of type %u, force clearing" CRLF,
                        msg_type);
                    pMsg->msg_type = IPI_MSG_NO_MESSAGE;
                } else {
                    intentFound = true;
                    break;
                }
            }
            pMsg++;
        }

        // if we found the intent we were looking for, and also have a valid handler for it
        // process the intent and generate any return ACK packets to the peer if necessary
        //
        if (pHandler && intentFound) {
            enum IPIStatusCode result;

            assert(pHandler != NULL);
            IPI_DATA.mpfs_ipi_privateData[current_hartid()].consume_intents++;
            result = (*pHandler)(pMsg->transaction_id, source,
                pMsg->immediate_arg, pMsg->p_extended_buffer_in_ddr, pMsg->p_ancilliary_buffer_in_ddr);

            switch (pMsg->msg_type) {
            case IPI_MSG_ACK_COMPLETE:
                break;
            case IPI_MSG_ACK_PENDING:
                break;
            default:
                switch (result) {
                case IPI_SUCCESS:
                    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "sending ACK_COMPLETE on txId %u" CRLF,
                    //    pMsg->transaction_id);
                    IPI_Send(source, IPI_MSG_ACK_COMPLETE, pMsg->transaction_id, IPI_SUCCESS, NULL, NULL);
                    break;

                case IPI_PENDING:
                    IPI_Send(source, IPI_MSG_ACK_PENDING, pMsg->transaction_id, IPI_PENDING, NULL, NULL);
                    break;

                default:
                case IPI_FAIL:
                    IPI_Send(source, IPI_MSG_ACK_COMPLETE, pMsg->transaction_id, IPI_FAIL, NULL, NULL);
                    break;

                case IPI_IDLE:
                    // nothing to do - for example, state machine might want to send response
                    // itself, or no response
                    break;
                }
            }

            pMsg->msg_type = IPI_MSG_NO_MESSAGE;
        }
    }

    if (!intentFound) {
        //mHSS_DEBUG_PRINTF(LOG_NORMAL, "did not find IPI of type %u" CRLF, msg_type);
    }

    return intentFound;
}

#if IS_ENABLED(CONFIG_IPI_FIXED_BASE_ADDR)
extern unsigned long _hss_start, _hss_end;
static bool check_if_ipi_queues_overlap_hss(void)
{
    return (((char *)CONFIG_IPI_FIXED_BASE_ADDR + IPI_SIZE) >= (char *)&_hss_start)
        && ((char *)CONFIG_IPI_FIXED_BASE_ADDR <= (char *)&_hss_end);
}
#endif

bool IPI_QueuesInit(void)
{
#if IS_ENABLED(CONFIG_DEBUG_MSCGEN_IPI)
    assert(ARRAY_SIZE(hartName) == MAX_NUM_HARTS);
    assert(ARRAY_SIZE(ipiName) == IPI_MSG_NUM_MSG_TYPES);
#endif

#if IS_ENABLED(CONFIG_IPI_FIXED_BASE_ADDR)
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "IPI Queues: %p to %p" CRLF, (char *)CONFIG_IPI_FIXED_BASE_ADDR,
        (char*)CONFIG_IPI_FIXED_BASE_ADDR + IPI_SIZE);
    assert(check_if_ipi_queues_overlap_hss() == false);
#endif

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Initializing IPI Queues (%lu bytes @ %p)..." CRLF,
        sizeof(struct IPI_Data), (void *)&IPI_DATA);

    memset((void *)ipi_data, 0, sizeof(struct IPI_Data));

    for (unsigned int i = 0u; i < MAX_NUM_HARTS; i++) {
        IPI_DATA.my_transaction_id[i] = 1u;
    }

    IPI_DATA.ipi_version = IPI_VERSION;

    return true;
}

bool IPI_MessageAlloc(uint32_t *indexOut)
{
    uint32_t index;
    bool result = false;

    assert(indexOut != NULL);

    for (index = 0u; index < IPI_MAX_NUM_OUTSTANDING_COMPLETES; index++) {
        if (!IPI_DATA.ipi_completes[index].used) {
            IPI_DATA.ipi_completes[index].used = true;
            IPI_DATA.ipi_completes[index].transaction_id =
                IPI_DATA.my_transaction_id[current_hartid()];;

            IPI_DATA.ipi_completes[index].status = IPI_PENDING;
            IPI_DATA.my_transaction_id[current_hartid()]++;

            result = true;
            *indexOut = index;
            break;
        }
    }

    if (result) {
        IPI_DATA.mpfs_ipi_privateData[current_hartid()].message_allocs++;
    }

    return result;
}

bool IPI_MessageDeliver(uint32_t index, enum HSSHartId target, enum IPIMessagesEnum message,
                 uint32_t immediate_arg, void const *p_extended_buffer_in_ddr,
                 void const *p_ancilliary_buffer_in_ddr)
{
    bool result = false;

    assert(index < IPI_MAX_NUM_OUTSTANDING_COMPLETES);

    if ((index < IPI_MAX_NUM_OUTSTANDING_COMPLETES) && (IPI_DATA.ipi_completes[index].used)) {
        result = IPI_Send(target, message, IPI_DATA.ipi_completes[index].transaction_id,
                          immediate_arg, p_extended_buffer_in_ddr, p_ancilliary_buffer_in_ddr);
    }

    if (result) {
        IPI_DATA.mpfs_ipi_privateData[current_hartid()].message_delivers++;
    }

    return result;
}

bool IPI_MessageCheckIfComplete(uint32_t index)
{
    bool result = false;

    assert(index <= IPI_MAX_NUM_OUTSTANDING_COMPLETES);

    if (index == IPI_MAX_NUM_OUTSTANDING_COMPLETES) {
        result = true;
    } else if ((index < IPI_MAX_NUM_OUTSTANDING_COMPLETES) && (IPI_DATA.ipi_completes[index].used)) {
        switch (IPI_DATA.ipi_completes[index].status) {
        case IPI_PENDING:
            result = false;
            break;

        case IPI_SUCCESS:
            __attribute__((fallthrough)); // deliberate fallthrough
        case IPI_IDLE:
            __attribute__((fallthrough)); // deliberate fallthrough
        case IPI_FAIL:
            __attribute__((fallthrough)); // deliberate fallthrough
        default:
            result = true;
            break;
        }
    }

    return result;
}

bool IPI_MessageUpdateStatus(TxId_t transaction_id, enum IPIStatusCode status)
{
    bool result = false;
    uint32_t index;

    for (index = 0u; index < IPI_MAX_NUM_OUTSTANDING_COMPLETES; index++) {
        if (IPI_DATA.ipi_completes[index].transaction_id == transaction_id) {
            //mHSS_DEBUG_PRINTF(LOG_NORMAL, "index is %u, TxId is %u, status is %d" CRLF,
            //    index, transaction_id, status);
            IPI_DATA.ipi_completes[index].status = status;

            result = true;
            break;
        }
    }

    return result;
}

void IPI_MessageFree(uint32_t index)
{
    assert(IPI_DATA.ipi_completes[index].used);
    IPI_DATA.ipi_completes[index].used = false;

    IPI_DATA.mpfs_ipi_privateData[current_hartid()].message_frees++;

    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "index is %u, TxId is %u" CRLF, index,
    //    IPI_DATA.ipi_completes[index].transaction_id);
}

TxId_t IPI_DebugGetTxId(void)
{
    return IPI_DATA.my_transaction_id[current_hartid()];
}

enum IPIStatusCode IPI_ACK_IPIHandler(TxId_t transaction_id, enum HSSHartId source,
    uint32_t immediate_arg, void *p_extended_buffer_in_ddr, void *p_ancilliary_buffer_in_ddr)
{
    (void)source;
    (void)immediate_arg;
    (void)p_extended_buffer_in_ddr;
    (void)p_ancilliary_buffer_in_ddr;

    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "freeing IPI" CRLF);

    IPI_MessageUpdateStatus(transaction_id, IPI_IDLE); // free the IPI

    return IPI_SUCCESS;
}

