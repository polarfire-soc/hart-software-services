/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Implementation of uart_putstring/g(). 
 * This is function is intended to be used from ee_printf().
 */

#include "config.h"
#include "hss_types.h"

#include <assert.h>

#include "hss_debug.h"

#include <mss_uart.h>
#include <string.h>
#include <stdint.h>

#include "uart_helper.h"

int uart_putstring(int hartid, char *p)
{
    const uint32_t len = (uint32_t)strlen(p);

    mss_uart_instance_t *pUart;

    switch (hartid) {
    default:
        pUart = &g_mss_uart0_lo;
        break;
    case HSS_HART_E51:
        pUart = &g_mss_uart0_lo;
        break;
    case HSS_HART_U54_1:
        pUart = &g_mss_uart1_lo;
        break;
    case HSS_HART_U54_2:
        pUart = &g_mss_uart2_lo;
        break;
    case HSS_HART_U54_3:
        pUart = &g_mss_uart3_lo;
        break;
    case HSS_HART_U54_4:
        pUart = &g_mss_uart4_lo;
        break;
    }
    MSS_UART_polled_tx_string(pUart, (const uint8_t *)p); 
    // TODO: if hartId is zero (i.e., E51), replace this with non-blocking
    // queue implementation, with HSS_UART state machine consuming from queues...
    return len;
}

#define HSS_UART_HELPER_MAX_GETLINE 80
ssize_t uart_getline(char **pBuffer, size_t *pBufLen)
{
    ssize_t result = 0;
    bool finished = false;
    static char myBuffer[HSS_UART_HELPER_MAX_GETLINE]; // static to be stack friendly
    const size_t bufferLen = mSPAN_OF(myBuffer);

    memset(myBuffer, 0, bufferLen);

    uint8_t c;
    while (!finished) {
        while (0 == MSS_UART_get_rx(&g_mss_uart0_lo, &c, 1));

        switch (c) {
        case '\r':
            mHSS_PUTC(c);
            finished = true;
            break;

        case '\n':
            mHSS_PUTC(c);
            finished = true;
            break;

        case 0x7f: // delete
            if (result) {
                result--;
                mHSS_PUTS("\033[D \033[D");
                myBuffer[result] = 0;
            }
            break;

        case 8: // backspace - ^H
            if (result) {
                result--;
                mHSS_PUTS(" \033[D");
                myBuffer[result] = 0;
            }
            break;

        case 3: // intr - ^C
            result = -1;
            myBuffer[0] = 0;
            finished = true;
            break;

        case 27: // ESC
            result = -1;
            myBuffer[0] = 0;
            finished = true;
            break;

        case 4: // ^D
            if (result == 0) {
                finished = true;
            }
            break;

        default:
            if (result < bufferLen) {
                mHSS_PUTC(c);
                myBuffer[result] = c;
                result++;
            }
            break;
        }
    }
    mHSS_PUTS(CRLF);

    if (result > 0) {
        *pBuffer = myBuffer;
        *pBufLen = (size_t)result;
    } else {
        *pBuffer = NULL;
        *pBufLen = 0u;
    }

    return result;
}

bool uart_getchar(uint8_t *pbuf, int32_t timeout_sec, bool do_sec_tick)
{
    bool result = false;
    bool done = false;
    uint8_t rx_byte;
    HSSTicks_t start_time = 0u;
    HSSTicks_t last_sec_time = 0u;

    const HSSTicks_t timeout_ticks = timeout_sec * TICKS_PER_SEC;

    if (timeout_sec > 0) {
        start_time = last_sec_time = HSS_GetTime();
    }

    while (!done) {
        size_t received = MSS_UART_get_rx(&g_mss_uart0_lo, &rx_byte, 1u);
        if (0u != received) {
            done = true;
            if (MSS_UART_NO_ERROR == MSS_UART_get_rx_status(&g_mss_uart0_lo)) {
                *pbuf = rx_byte;
                result = true;
                break;
            } else {
                mHSS_DEBUG_PRINTF("UART error" CRLF);
            }
        }

        if (do_sec_tick && HSS_Timer_IsElapsed(last_sec_time, TICKS_PER_SEC)) {
            mHSS_PUTC('.');
            last_sec_time = HSS_GetTime();
        }

        if (timeout_sec < 0) {
            ; // blocking until UART data received, so nothing extra to do here...
        } else if (timeout_sec > 0) {
            // time limited
            done = HSS_Timer_IsElapsed(start_time, timeout_ticks);
        } else /* timeout == 0 */ {
            // one-shot
            break;
        }
    }

    return result;
}
