/*******************************************************************************
 * Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * global list of uart devices
 */

#include "config.h"
#include "hss_types.h"

#include <assert.h>

#include "hss_debug.h"

#include "drivers/mss/mss_mmuart/mss_uart.h"
#include <string.h>
#include <stdint.h>

#include "hss_boot_init.h"
#include "uart_helper.h"

#include "fpga_design_config/fpga_design_config.h"
#include "sbi_bitops.h"

#if IS_ENABLED(CONFIG_SERVICE_WDOG)
#    include "wdog_service.h"
#endif

#define mUART_DEV(x) ( LIBERO_SETTING_APBBUS_CR & (BIT(x)) ? &g_mss_uart##x##_hi : &g_mss_uart##x##_lo )

// UART devices list
mss_uart_instance_t * const pUartDeviceList[] = {
    mUART_DEV(0),
    mUART_DEV(1),
    mUART_DEV(2),
    mUART_DEV(3),
    mUART_DEV(4),
};


int uart_putstring(int hartid, char *p)
{
    const uint32_t len = (uint32_t)strlen(p);

    mss_uart_instance_t *pUart = HSS_UART_GetInstance(hartid);

    while (!(MSS_UART_TEMT & MSS_UART_get_tx_status(pUart))) { ; }

    MSS_UART_polled_tx_string(pUart, (const uint8_t *)p);
    // TODO: if hartId is zero (i.e., E51), replace this with non-blocking
    // queue implementation, with HSS_UART state machine consuming from queues...

    return len;
}

void uart_putc(int hartid, const char ch)
{
    uint8_t string[2];
    string[0] = (uint8_t)ch;
    string[1] = 0u;

    mss_uart_instance_t *pUart = HSS_UART_GetInstance(hartid);

    while (!(MSS_UART_TEMT & MSS_UART_get_tx_status(pUart))) { ; }

    MSS_UART_polled_tx_string(pUart, (const uint8_t *)string);
}

ssize_t uart_getline(char **pBuffer, size_t *pBufLen)
{
    ssize_t result = 0;
    bool finished = false;
    static char myBuffer[HSS_UART_HELPER_MAX_GETLINE]; // static to be stack friendly
    const size_t bufferLen = ARRAY_SIZE(myBuffer);

    memset(myBuffer, 0, bufferLen);

    mss_uart_instance_t *pUart = HSS_UART_GetInstance(HSS_HART_E51);

    uint8_t cBuf[1];
    while (!finished) {
        while (0 == MSS_UART_get_rx(pUart, cBuf, 1));

        switch (cBuf[0]) {
        case '\r':
            MSS_UART_polled_tx(pUart, cBuf, 1u);
            finished = true;
            break;

        case '\n':
            MSS_UART_polled_tx(pUart, cBuf, 1u);
            finished = true;
            break;

        case 0x7Fu: // delete
            if (result) {
                result--;
                MSS_UART_polled_tx(pUart, (uint8_t const *)"\033[D \033[D", 7u);
                myBuffer[result] = 0;
            }
            break;

        case 0x08u: // backspace - ^H
            if (result) {
                result--;
                MSS_UART_polled_tx(pUart, (uint8_t const *)" \033[D", 4u);
                myBuffer[result] = 0;
            }
            break;

        case 0x03u: // intr - ^C
            result = -1;
            myBuffer[0] = 0;
            finished = true;
            break;

        case 0x1Bu: // ESC
            result = -1;
            myBuffer[0] = 0;
            finished = true;
            break;

        case 0x04u: // ^D
            if (result == 0) {
                result = -1;
                myBuffer[0] = 0;
                finished = true;
            }
            break;

        default:
            if (result < bufferLen) {
                MSS_UART_polled_tx(pUart, cBuf, 1u);
                myBuffer[result] = cBuf[0];
                result++;
            }
            break;
        }
    }

    const char crlf[] = "\n";
    MSS_UART_polled_tx_string(pUart, (const uint8_t *)crlf);

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
    uint8_t rx_buff[1];
    HSSTicks_t start_time = 0u;
    HSSTicks_t last_sec_time = 0u;

    start_time = last_sec_time = HSS_GetTime();

    const HSSTicks_t timeout_ticks = timeout_sec * TICKS_PER_SEC;

    mss_uart_instance_t *pUart = HSS_UART_GetInstance(HSS_HART_E51);

    while (!done) {
        size_t received = MSS_UART_get_rx(pUart, rx_buff, 1u);
        if (0u != received) {
            done = true;
            if (MSS_UART_NO_ERROR == MSS_UART_get_rx_status(pUart)) {
                *pbuf = rx_buff[0];
                result = true;
                break;
            } else {
                mHSS_DEBUG_PRINTF(LOG_ERROR, "UART error\n");
            }
        }

        if (do_sec_tick && HSS_Timer_IsElapsed(last_sec_time, TICKS_PER_SEC)) {
            const uint8_t dot='.';
            MSS_UART_polled_tx(pUart, &dot, 1);
            last_sec_time = HSS_GetTime();
        }

        if (timeout_sec < 0) {
            ; // blocking until UART data received, so nothing extra to do here...
#if IS_ENABLED(CONFIG_SERVICE_WDOG)
            HSS_Wdog_E51_Tickle();
#endif
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
