/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * 
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS Software Initalization
 * \brief Full System Initialization
 */

#include "config.h"
#include "hss_types.h"
#include <assert.h>

#include <string.h>

#include "hss_debug.h"
#include "hss_crc16.h"
#include "hss_clock.h"
#include "ymodem.h"
#include "drivers/mss_uart/mss_uart.h"
#include "uart_helper.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

#define HSS_XYMODEM_PROTOCOL_XMODEM        1
#define HSS_XYMODEM_PROTOCOL_YMODEM        2

#define HSS_XYMODEM_MAX_FILENAME_LENGTH    64u

#define HSS_XYMODEM_MAX_SYNC_ATTEMPTS      10u
#define HSS_XYMODEM_CAN_COUNT_REQUIRED     2u
#define HSS_XYMODEM_PRE_SYNC_TIMEOUT_SEC   10
#define HSS_XYMODEM_POST_SYNC_TIMEOUT_SEC  1
#define HSS_XYMODEM_BAD_PACKET_RETRIES     10u

enum XYModem_Signals {
    XYMODEM_SOH             = 0x01,
    XYMODEM_STX             = 0x02,
    XYMODEM_ETX             = 0x03,
    XYMODEM_EOT             = 0x04,
    XYMODEM_ACK             = 0x06,
    XYMODEM_NAK             = 0x15,
    XYMODEM_CAN             = 0x18,
    XYMODEM_C               = 0x43,
    XYMODEM_GETCHAR_TIMEOUT = -1
};

/*
 * Each block of the transfer looks like:
 *        <SOH><blk #><255-blk #><--128 data bytes--><crc_hi><crc_lo>
 *  in which:
 *  <SOH>         = 0x01 hex
 *  <blk #>       = binary number, starts at 0x01 increments by 0x01, and
 *                  wraps 0FFH to 00H (not to 0x01)
 *  <255-blk #>   = blk # after going thru 8080 "CMA" instruction, i.e.
 *                  each bit complemented in the 8-bit block number.
 *                  Formally, this is the "ones complement".
 *  <crc_hi>
 *  <crc_lo>
 */

/***************************************************************************/

mss_uart_instance_t *g_my_uart = &g_mss_uart0_lo;

static int16_t ymodem_getchar(int32_t timeout_sec)
{
    uint8_t rx_byte = 0;
    int16_t result = 0;

    bool retval = uart_getchar(&rx_byte, timeout_sec, false);

    if (retval) {
        result = rx_byte; 
    } else {
        result = -1;
    } 

    return result;
}

static void ymodem_putchar(uint8_t tx_byte)
{
    MSS_UART_polled_tx(g_my_uart, &tx_byte, 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

struct XYModem_Packet {
    int16_t startByte;
    uint8_t blkNum;
    uint8_t blkNumOnesComplement;
    char buffer[1024];
    uint8_t crc_hi;
    uint8_t crc_lo;
    //
    size_t length;
};

struct XYModem_State {
    int protocol;
    union {
        struct {
            int eot:1;
            int abort:1;
        } s;
        int done;
    } status;
    uint8_t lastReceivedBlkNum;
    uint8_t expectedBlkNum;
    size_t totalReceivedSize;
    size_t numReceivedPackets;
    char filename[HSS_XYMODEM_MAX_FILENAME_LENGTH];
    size_t expectedSize;
    size_t maxSize;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static void XYMODEM_SendACK(void)
{
    ymodem_putchar(XYMODEM_ACK);
}

static void XYMODEM_SendCAN(void)
{
    // two consequtive CAN characters without modem errors is transfer abort
    ymodem_putchar(XYMODEM_CAN);
    ymodem_putchar(XYMODEM_CAN);

    // YAM sends 8, for good measure
    ymodem_putchar(XYMODEM_CAN);
    ymodem_putchar(XYMODEM_CAN);
    ymodem_putchar(XYMODEM_CAN);
    ymodem_putchar(XYMODEM_CAN);
    ymodem_putchar(XYMODEM_CAN);
    ymodem_putchar(XYMODEM_CAN);
}

static void XYMODEM_PurgeAndSendNAK(void)
{
    // wait for line to clear
    // to prevent infinite loop here, we count down
    uint32_t max_loop_counter = 10u;
    while (ymodem_getchar(HSS_XYMODEM_POST_SYNC_TIMEOUT_SEC) != XYMODEM_GETCHAR_TIMEOUT) {
        --max_loop_counter;
        if (0u == max_loop_counter) { break; }
    }

    ymodem_putchar(XYMODEM_NAK);
}

static bool XYMODEM_ValidatePacket(struct XYModem_Packet *pPacket, struct XYModem_State *pState)
{
    bool result = true;

    // CRC failure
    uint16_t crc16 = CRC16_calculate((const unsigned char *)pPacket->buffer, pPacket->length + 2u);
    if (crc16 != 0u) { result = false; }

    //
    // sequence failure
    //
    if ((result) && (pPacket->blkNum != (pPacket->blkNumOnesComplement ^ 0xFFu))) { result = false; }
    
    if ((result) && (pPacket->blkNum != pState->expectedBlkNum)) { result = false; }

    return result;
}

static bool XYMODEM_ReadPacket(struct XYModem_Packet *pPacket, struct XYModem_State *pState)
{
    bool result = true;
    bool synced = false;

    unsigned int syncAttempt = 0u;
    int timeout_sec =  HSS_XYMODEM_PRE_SYNC_TIMEOUT_SEC;
    unsigned int can_rx_count = 0u;

    if (pState->status.done) {
        result = false;
    } else {
        //
        // Attempt to synchronize up to HSS_XYMODEM_MAX_SYNC_ATTEMPTS times
        // 
        while (!synced && (syncAttempt < HSS_XYMODEM_MAX_SYNC_ATTEMPTS)) {
            pPacket->startByte = ymodem_getchar(timeout_sec);
            switch (pPacket->startByte) {
            case XYMODEM_SOH:
                can_rx_count = 0u;
                pPacket->length = 128u;
                synced = true;
                break;

            case XYMODEM_STX:
                can_rx_count = 0u;
                pPacket->length = 1024u;
                synced = true;
                break;

            case XYMODEM_EOT:
                can_rx_count = 0u;
                pPacket->length = 0u;
                pState->status.s.eot = true;
                synced = false;
                syncAttempt = HSS_XYMODEM_MAX_SYNC_ATTEMPTS;
                break;

            case XYMODEM_ETX: // interactive CTRL-C
                pPacket->length = 0u;
                pState->status.s.abort = true;
                synced = false;
                syncAttempt = HSS_XYMODEM_MAX_SYNC_ATTEMPTS;
                break;

            case XYMODEM_CAN:
                // check again
                ++can_rx_count;
                if (can_rx_count == HSS_XYMODEM_CAN_COUNT_REQUIRED) {
                    pPacket->length = 0u;
                    pState->status.s.abort = true;
                    synced = false;
                    syncAttempt = HSS_XYMODEM_MAX_SYNC_ATTEMPTS;
                } else {
                    ++syncAttempt;
                }
                break;

            case XYMODEM_GETCHAR_TIMEOUT:
                __attribute__((fallthrough)); // deliberate fallthrough
            default:
                //mHSS_DEBUG_PRINTF("%s(): %d: char is %0x" CRLF, __func__, syncAttempt, 
                //    pPacket->startByte);
                can_rx_count = 0u;
                ++syncAttempt;
                synced = false;
                break;
            }
        }

        //
        // if synchronized, extract packet and validate it
        //
        if (synced) {
            timeout_sec = HSS_XYMODEM_POST_SYNC_TIMEOUT_SEC;
            pPacket->blkNum = ymodem_getchar(timeout_sec);
            pPacket->blkNumOnesComplement = ymodem_getchar(timeout_sec);
            ++(pState->numReceivedPackets);
   
            size_t i = 0u; 
            while (i < pPacket->length) {
                pPacket->buffer[i] = ymodem_getchar(timeout_sec);
                ++i;
            }

            pPacket->crc_hi = ymodem_getchar(timeout_sec);
            pPacket->crc_lo = ymodem_getchar(timeout_sec);

            if (pState->status.done) {
                ;
            } else {
                if (XYMODEM_ValidatePacket(pPacket, pState)) {
                    pState->lastReceivedBlkNum = pPacket->blkNum;
                    ++(pState->expectedBlkNum);
                } else {
                    //mHSS_DEBUG_PRINTF("%s(): validate failure" CRLF, __func__);
                    result = false; 
                }
            }
        } else {
            //mHSS_DEBUG_PRINTF("%s(): sync failure" CRLF, __func__);
            result = synced;
            pPacket->length = 0u;
            pState->status.s.eot = true;
            pState->status.s.abort = true;
        }
    }

    //mHSS_DEBUG_PRINTF("%s(): returning %d" CRLF, __func__, result);
    return result;
}

size_t XYMODEM_GetFileSize(char *pStart, char *pEnd)
{
    char *pChar = pStart;
    bool hunting = true;
    bool finished = false;
    size_t fileSize = 0u;

    // need to be careful not to go outside bounds
    while ((pChar < pEnd) && (!finished)) {
        switch (*pChar) {
        case 0:
            if (!hunting) {
                finished = true;
            }
            hunting = false; // found end of filename
            break;

        case '0' ... '9':
            if (!hunting) {
                fileSize = (fileSize * 10u) + (uint8_t)(*pChar - '0');
            }
            break;

        case 32:
            if (!hunting) {
                finished = true;
            }
            break;
       
        default:
            if (!hunting) {
                finished = true;
            }
            break;
        }

        ++pChar;
    }

    return fileSize;
}

static size_t XYMODEM_Receive(int protocol, char *buffer, size_t bufferSize)
{
    size_t result;
    uint8_t retries = 0u;
   
    struct XYModem_State state;
    {
        // initialize state
        state.status.done = 0;
        state.lastReceivedBlkNum = 0u;
        state.expectedBlkNum = 0u;
        state.totalReceivedSize = 0u;
        state.numReceivedPackets = 0u;
        state.expectedSize = 0u;
        state.maxSize = bufferSize;
        memset(state.filename, 0, mSPAN_OF(state.filename));
        state.protocol = protocol;
    }

    //
    // Protocol starts with receiver sending a character to indicate to the sender that it is ready...
    //
    if (state.protocol == HSS_XYMODEM_PROTOCOL_YMODEM) {
        ymodem_putchar(XYMODEM_C); // explicitly request CRC16 mode
        state.expectedBlkNum = 0;
    } else {
        ymodem_putchar(XYMODEM_NAK); // 
        state.expectedBlkNum = 1;
    }

    static struct XYModem_Packet packet; // make this static, as it is contains a large buffer, 
                                         //which is not friendly to the stack
    memset(&packet, 0, sizeof(packet));

    // 
    // main receive loop
    //
    retries = 0u;
    while (!state.status.done && (retries < HSS_XYMODEM_BAD_PACKET_RETRIES)) {
        if (XYMODEM_ReadPacket(&packet, &state)) {
            XYMODEM_SendACK();

            if (!state.status.done) {
                if ((state.protocol == HSS_XYMODEM_PROTOCOL_YMODEM) && (state.lastReceivedBlkNum == 0)) {
                    memcpy(state.filename, packet.buffer, mSPAN_OF(state.filename)-1); 
                    state.expectedSize = XYMODEM_GetFileSize(packet.buffer, packet.buffer+1024);
                  
                    // if expected file size is known a priori, ensure we have enough buffer 
                    // space to receive and abort transfer early
                    if (state.expectedSize > state.maxSize) { 
                        state.status.done = 1;
                        state.totalReceivedSize = 0;
                        XYMODEM_SendCAN();
                        break;
                    } else {
                        //mHSS_DEBUG_PRINTF("FILENAME: %s, expected size %lu" CRLF CRLF, 
                        //                   state.filename, state.expectedSize);
                    }
                } else {
                    // dynamnically ensure we have enough buffer space to receive, on each 
                    // received chunk
                    if ((state.totalReceivedSize + packet.length) < state.maxSize) { 
                        memcpy(buffer + state.totalReceivedSize, packet.buffer, packet.length);
                        state.totalReceivedSize += packet.length;
                    } else {
                        state.status.done = 1;
                        state.totalReceivedSize = 0;
                        XYMODEM_SendCAN();
                        break;
                    }
                }
            } else {
                // transfer was aborted
                if (state.status.s.abort) {
                    state.totalReceivedSize = 0;
                    XYMODEM_SendCAN();
                    break;
                }
            }
        } else {
           ++retries;
           if (state.numReceivedPackets) {
               XYMODEM_PurgeAndSendNAK(); 
           }
        }
    }

    if (state.expectedSize != 0u) {
        result = mMIN(state.expectedSize, state.totalReceivedSize);
    } else {
        result = state.totalReceivedSize;
    }

    return result;
}

size_t ymodem_receive(uint8_t *buffer, size_t bufferSize)
{
    return XYMODEM_Receive(HSS_XYMODEM_PROTOCOL_YMODEM, (char *)buffer, bufferSize);
}
