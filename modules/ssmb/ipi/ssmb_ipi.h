#ifndef HSS_SSMB_IPI_H
#define HSS_SSMB_IPI_H

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
 * Hart Software Services - Secure Software Message Bus IPI
 *
 */

/**
 * \file Secure Software Message Bus IPI
 * \brief SSMB Core Types/Defines/Declarations
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include "hss_types.h"

#define IPI_MAX_NUM_QUEUE_MESSAGES ((unsigned long)CONFIG_IPI_MAX_NUM_QUEUE_MESSAGES)
#define IPI_MAX_NUM_OUTSTANDING_COMPLETES (IPI_MAX_NUM_QUEUE_MESSAGES * (HSS_HART_NUM_PEERS-1u))

/**
 * \brief IPI Outbox Enumeration
 */
enum IPIOutboxEnum {
    IPI_OUTBOX_E51_TO_U54_1,
    IPI_OUTBOX_E51_TO_U54_2,
    IPI_OUTBOX_E51_TO_U54_3,
    IPI_OUTBOX_E51_TO_U54_4,

    IPI_OUTBOX_U54_1_TO_E51,
    IPI_OUTBOX_U54_1_TO_U54_2,
    IPI_OUTBOX_U54_1_TO_U54_3,
    IPI_OUTBOX_U54_1_TO_U54_4,

    IPI_OUTBOX_U54_2_TO_E51,
    IPI_OUTBOX_U54_2_TO_U54_1,
    IPI_OUTBOX_U54_2_TO_U54_3,
    IPI_OUTBOX_U54_2_TO_U54_4,

    IPI_OUTBOX_U54_3_TO_E51,
    IPI_OUTBOX_U54_3_TO_U54_1,
    IPI_OUTBOX_U54_3_TO_U54_2,
    IPI_OUTBOX_U54_3_TO_U54_4,

    IPI_OUTBOX_U54_4_TO_E51,
    IPI_OUTBOX_U54_4_TO_U54_1,
    IPI_OUTBOX_U54_4_TO_U54_2,
    IPI_OUTBOX_U54_4_TO_U54_3,

    IPI_OUTBOX_NUM_QUEUES
};

#define IPI_PEER_CORE_TO_OFFSET(x) (x - HSS_HART_U54_1)

/**
 * \brief IPI Message Types Enumeration
 */
enum IPIMessagesEnum {
    IPI_MSG_NO_MESSAGE,
    IPI_MSG_BOOT_REQUEST,
    IPI_MSG_PMP_SETUP,
    IPI_MSG_SPI_XFER,
    IPI_MSG_NET_RXPOLL,
    IPI_MSG_NET_TX,
    IPI_MSG_SCATTERGATHER_DMA,
    IPI_MSG_WDOG_INIT,
    IPI_MSG_GPIO_SET,
    IPI_MSG_UART_TX,
    IPI_MSG_UART_POLL_RX,
    IPI_MSG_POWERMODE,
    IPI_MSG_FLASHFREEZE,
    IPI_MSG_ACK_PENDING,
    IPI_MSG_ACK_COMPLETE,
    IPI_MSG_HALT,
    IPI_MSG_CONTINUE,
    IPI_MSG_GOTO,
    IPI_MSG_OPENSBI_INIT,
    IPI_MSG_NUM_MSG_TYPES,
};

/**
 * \brief IPI Return Codes Type Enumeration
 */
enum IPIStatusCode {
    IPI_FAIL,
    IPI_SUCCESS,
    IPI_PENDING,
    IPI_IDLE,
};

/**
 * \brief IPI Transaction Id Type
 */
typedef uint32_t TxId_t;

typedef enum IPIStatusCode (*IPI_handlerFunction)(TxId_t transaction_id, enum HSSHartId source, uint32_t immediate_arg,
                                                  void *p_extended_buffer_in_ddr, void *p_ancilliary_buffer_in_ddr);

/**
 * \brief IPI Outbox Message Structure
 */
struct IPI_Outbox_Msg {
    enum IPIMessagesEnum msg_type;
    TxId_t transaction_id;
    uint32_t immediate_arg;
    void *p_extended_buffer_in_ddr;
    void *p_ancilliary_buffer_in_ddr;
};

/**
 * \brief IPI Outbox Queue Structure
 */
struct IPI_Outbox_Queue {
    struct IPI_Outbox_Msg msgQ[IPI_MAX_NUM_QUEUE_MESSAGES];
    uint32_t count;
};

struct IPI_Complete {
    bool used;
    TxId_t transaction_id;
    enum IPIStatusCode status;
};


/**
 * \brief IPI Handler Descriptor
 *
 * Maps message type to handler function pointer
 */
struct IPI_Handler {
    enum IPIMessagesEnum msg_type;
    IPI_handlerFunction handler;
};

// ------------------------------------------------------------------------
// PUBLIC API
//

bool IPI_Send(enum HSSHartId target, enum IPIMessagesEnum message, TxId_t transaction_id, uint32_t immediate_arg,
        void const *p_extended_buffer_in_ddr, void const *p_ancilliary_buffer_in_ddr);
bool IPI_PollReceive(union HSSHartBitmask hartMask);
bool IPI_QueuesInit(void);
bool IPI_ConsumeIntent(enum HSSHartId source, enum IPIMessagesEnum msg_type);
uint32_t IPI_GetQueuePendingCount(uint32_t queueIndex);

bool IPI_MessageAlloc(uint32_t *indexOut);
bool IPI_MessageDeliver(uint32_t index, enum HSSHartId target, enum IPIMessagesEnum message,
        uint32_t immediate_arg, void const *p_extended_buffer_in_ddr,
        void const *p_ancilliary_buffer_in_ddr);
bool IPI_MessageUpdateStatus(TxId_t transaction_id, enum IPIStatusCode status);
bool IPI_MessageCheckIfComplete(uint32_t index);
void IPI_MessageFree(uint32_t index);

TxId_t IPI_DebugGetTxId(void);
void IPI_DebugDumpStats(void);

struct IPI_Outbox_Msg *IPI_DirectionToFirstMsgInQueue(enum HSSHartId source, enum HSSHartId target);
uint32_t IPI_CalculateQueueIndex(enum HSSHartId source, enum HSSHartId target);

/* handler for ACKs and PENDINGs */
enum IPIStatusCode IPI_ACK_IPIHandler(TxId_t transaction_id, enum HSSHartId source,
    uint32_t immediate_arg, void *p_extended_buffer_in_ddr, void *p_ancilliary_buffer_in_ddr);

/* helpers for setting and clearing MSIP */
bool CLINT_Raise_MSIP(enum HSSHartId const target);
void CLINT_Clear_MSIP(enum HSSHartId const target);

#ifdef __cplusplus
}
#endif

#endif
