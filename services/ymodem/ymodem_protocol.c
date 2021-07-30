/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
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
#include "hss_crc32.h"
#include "hss_crc16.h"
#include "hss_clock.h"
#include "ymodem.h"
#include "drivers/mss_uart/mss_uart.h"
#include "uart_helper.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

#define HSS_XYMODEM_PROTOCOL_XMODEM        1
#define HSS_XYMODEM_PROTOCOL_YMODEM        2

#define HSS_XYMODEM_MAX_FILENAME_LENGTH    64u

#define HSS_XYMODEM_MAX_SYNC_ATTEMPTS      20u
#define HSS_XYMODEM_CAN_COUNT_REQUIRED     2u
#define HSS_XYMODEM_PRE_SYNC_TIMEOUT_SEC   2
#define HSS_XYMODEM_POST_SYNC_TIMEOUT_SEC  1
#define HSS_XYMODEM_BAD_PACKET_RETRIES     10u

#define HSS_XYMODEM_PACKET_HEADER_LEN      3u
#define HSS_XYMODEM_PACKET_TRAILER         2u

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

static int16_t getchar_with_timeout_(int32_t timeout_sec)
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

static void putchar_(uint8_t tx_byte)
{
    MSS_UART_polled_tx(g_my_uart, &tx_byte, 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

struct XYModem_Packet {
    uint8_t startByte;
    uint8_t blkNum;
    uint8_t blkNumOnesComplement;
    char buffer[1024 + HSS_XYMODEM_PACKET_TRAILER];
    //
    //uint8_t padding[3];
    size_t length;
};

struct XYModem_State {
    int protocol;
    union {
        struct {
            int syncFail:1;
            int endOfSession:1;
            int abort:1;
        } s;
        int done;
    } status;
    bool eotReceived;
    uint8_t lastReceivedBlkNum;
    uint8_t expectedBlkNum;
    size_t totalReceivedSize;
    size_t numReceivedPackets;
    size_t numNAKs;
    char filename[HSS_XYMODEM_MAX_FILENAME_LENGTH];
    size_t expectedSize;
    size_t maxSize;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static void XYMODEM_SendReadyChar(struct XYModem_State *pState)
{
    if (pState->protocol == HSS_XYMODEM_PROTOCOL_YMODEM) {
        putchar_(XYMODEM_C); // explicitly request CRC16 mode
    } else {
        pState->numNAKs++;
        putchar_(XYMODEM_NAK);
    }
}

static void XYMODEM_SendCAN(void)
{
    // two consequtive CAN characters without modem errors is transfer abort
    putchar_(XYMODEM_CAN);
    putchar_(XYMODEM_CAN);

    // YAM sends 8, for good measure
    putchar_(XYMODEM_CAN);
    putchar_(XYMODEM_CAN);
    putchar_(XYMODEM_CAN);
    putchar_(XYMODEM_CAN);
    putchar_(XYMODEM_CAN);
    putchar_(XYMODEM_CAN);
}

static void XYMODEM_Purge(int32_t timeout_sec)
{
    // wait for line to clear
    // to prevent infinite loop here, we count down
    uint32_t max_loop_counter = 10u;
    while (getchar_with_timeout_(timeout_sec) != XYMODEM_GETCHAR_TIMEOUT) {
        --max_loop_counter;
        if (0u == max_loop_counter) { break; }
    }
}

static bool XYMODEM_ValidatePacket(struct XYModem_Packet *pPacket, struct XYModem_State *pState)
{
    bool result = true;

    uint16_t crc16 = CRC16_calculate((const unsigned char *)pPacket->buffer,
        pPacket->length + HSS_XYMODEM_PACKET_TRAILER);

    if (crc16 != 0u) { // CRC failure
        result = false;
    } else if (pPacket->blkNum != (pPacket->blkNumOnesComplement ^ 0xFFu)) { // sequence failure
        result = false;
    } else if (pPacket->blkNum != pState->expectedBlkNum) { // sequence failure
        result = false;
    }

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
            int16_t rawStartByte = getchar_with_timeout_(timeout_sec);

            if ((rawStartByte >= 0) && (rawStartByte < 256)) {
                pPacket->startByte = (uint8_t)rawStartByte;
            } else {
                pPacket->startByte = 0u;
            }

            switch (rawStartByte) {
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
                //pState->status.s.endOfSession = true;
                pState->eotReceived= true;
                synced = true;
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
                XYMODEM_SendReadyChar(pState);
                break;
            }
        }

        //
        // if synchronized, extract packet and validate it
        //
        if (pState->status.s.endOfSession) {
            result = true;
        } else if (synced) {
            timeout_sec = HSS_XYMODEM_POST_SYNC_TIMEOUT_SEC;
            pPacket->blkNum = getchar_with_timeout_(timeout_sec);
            pPacket->blkNumOnesComplement = getchar_with_timeout_(timeout_sec);
            ++(pState->numReceivedPackets);

            size_t i = 0u;
            while (i < pPacket->length) {
                pPacket->buffer[i] = getchar_with_timeout_(timeout_sec);
                ++i;
            }

            pPacket->buffer[i] = getchar_with_timeout_(timeout_sec); ++i; //crc_hi
            pPacket->buffer[i] = getchar_with_timeout_(timeout_sec); ++i; //crc_lo

            if (pState->status.done) {
                ;
            } else {
                if (pState->eotReceived)  {
                    ;
                } else if (XYMODEM_ValidatePacket(pPacket, pState)) {
                    pState->lastReceivedBlkNum = pPacket->blkNum;
                    ++(pState->expectedBlkNum);
                } else { // corrupt packet?
                    result = false;
                }
            }
        } else { // not synchronized
            if (pState->status.s.abort) {
                result = true;
            } else {
                result = synced;
            }
            pPacket->length = 0u;
            pState->status.s.syncFail = true;
        }
    }

    //mHSS_DEBUG_PRINTF("%s(): returning %d" CRLF, __func__, result);
    return result;
}

static size_t XYMODEM_GetFileSize(char *pStart, char *pEnd)
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

static size_t XYMODEM_Receive(int protocol, struct XYModem_State *pState, char *buffer, size_t bufferSize)
{
    size_t result;
    uint8_t retries = 0u;

    // initialize state
    pState->status.done = 0;
    pState->lastReceivedBlkNum = 0u;
    pState->expectedBlkNum = 0u;
    pState->totalReceivedSize = 0u;
    pState->numReceivedPackets = 0u;
    pState->expectedSize = 0u;
    pState->maxSize = bufferSize;
    pState->protocol = protocol;

    //
    // Protocol starts with receiver sending a character to indicate to the sender that it is ready...
    //
    XYMODEM_SendReadyChar(pState);
    if (pState->protocol != HSS_XYMODEM_PROTOCOL_YMODEM) {
        pState->expectedBlkNum = 1;
    }

    static struct XYModem_Packet packet; // make this static, as it is contains a large buffer,
                                         //which is not friendly to the stack
    memset(&packet, 0, sizeof(packet));

    //
    // main receive loop
    //
    retries = 0u;
    while (!pState->status.done && (retries < HSS_XYMODEM_BAD_PACKET_RETRIES)) {
        if (XYMODEM_ReadPacket(&packet, pState)) {
            putchar_(XYMODEM_ACK);

            if (!pState->status.done) {
                if ((pState->protocol == HSS_XYMODEM_PROTOCOL_YMODEM) && (pState->lastReceivedBlkNum == 0) && (pState->numReceivedPackets == 1u)) {
                    memcpy(pState->filename, packet.buffer, HSS_XYMODEM_MAX_FILENAME_LENGTH-1);
                    pState->expectedSize = XYMODEM_GetFileSize(packet.buffer, packet.buffer + ARRAY_SIZE(packet.buffer));

                    // if expected file size is known a priori, ensure we have enough buffer
                    // space to receive and abort transfer early
                    if (pState->expectedSize > pState->maxSize) {
                        pState->status.s.abort = true;
                        pState->totalReceivedSize = 0u;
                        XYMODEM_SendCAN();
                        break;
                    }
                } else if (pState->eotReceived) { // end of session
                    pState->status.s.endOfSession = true;
                    putchar_(XYMODEM_ACK);
                } else if ((pState->totalReceivedSize + packet.length) < pState->maxSize) {
                    // dynamically ensure we have enough buffer space to receive, on each received chunk
                    memcpy(buffer + pState->totalReceivedSize, packet.buffer, packet.length);
                    pState->totalReceivedSize += packet.length;
                } else {
                    pState->status.s.abort = true;
                    pState->totalReceivedSize = 0u;
                    XYMODEM_SendCAN();
                    break;
                }
            } else { // transfer is done
                if (pState->status.s.abort) {
                    pState->totalReceivedSize = 0u;
                    XYMODEM_SendCAN();
                    break;
                } else if (pState->status.s.endOfSession) {
                    putchar_(XYMODEM_ACK);
                    XYMODEM_Purge(HSS_XYMODEM_POST_SYNC_TIMEOUT_SEC);
                }
            }
        } else { // bad packet read
           ++retries;

           if (pState->numReceivedPackets) {
               XYMODEM_Purge(HSS_XYMODEM_POST_SYNC_TIMEOUT_SEC);
               pState->numNAKs++;
               putchar_(XYMODEM_NAK);
           }
        }
    }

    if (pState->expectedSize != 0u) {
        result = MIN(pState->expectedSize, pState->totalReceivedSize);
    } else {
        result = pState->totalReceivedSize;
    }

    XYMODEM_Purge(HSS_XYMODEM_POST_SYNC_TIMEOUT_SEC);

    if (retries >= HSS_XYMODEM_BAD_PACKET_RETRIES) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "maximum retries exceeded" CRLF);
    }
    if (pState->status.s.abort) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "Transfer aborted" CRLF);
    }
    return result;
}

size_t ymodem_receive(uint8_t *buffer, size_t bufferSize)
{
    size_t result = 0u;
    struct XYModem_State state = { 0 };
    memset(state.filename, 0, HSS_XYMODEM_MAX_FILENAME_LENGTH);

    result = XYMODEM_Receive(HSS_XYMODEM_PROTOCOL_YMODEM, &state, (char *)buffer, bufferSize);

    if (result != 0) {
        uint32_t crc32 = CRC32_calculate((const unsigned char *)buffer, result);
        mHSS_PRINTF(CRLF CRLF "Received %lu bytes from %s (CRC32 is 0x%08X)" CRLF, result,
            state.filename, crc32);
        //mHSS_PRINTF(CRLF CRLF "Expected %lu bytes in %lu packets (%lu NAKs)" CRLF, state.expectedSize,
        //    state.numReceivedPackets, state.numNAKs);
    }

    return result;
}
