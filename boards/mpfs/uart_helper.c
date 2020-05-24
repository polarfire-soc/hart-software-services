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

#include <string.h>
#include <stdint.h>

#include "uart_helper.h"


int uart_putstring(int hartid, char *p)
{
    uint32_t base = 0x10010000u;

    switch (hartid) {
    case 0:
        base = 0x10010000u;
        //return 0;
        break;
    case 1:
        base = 0x10011000u;
        return 0;
        break;
    case 2:
        base = 0x10011000u;
        return 0;
        break;
    case 3:
        base = 0x10011000u;
        return 0;
        break;
    case 4:
        base = 0x10011000u;
        return 0;
        break;
    default:
        return 0;
        break;
    }

    int n = 0;
    while (*p) {
        volatile uint32_t full = *(volatile uint32_t *)((uint64_t)base) & 0x80000000u;

        if (!full) {
            *(volatile uint32_t *)((uint64_t)base) = (uint32_t)(*p);
            p++;
            n++;
        } else {
            //volatile uint32_t delay = 100lu;
            //while (delay) { delay--; }
        }
    }
    return n;
}

void uart_putc(int hartid, const char ch)
{
    uint32_t base = 0x10010000u;

    switch (hartid) {
    case 0:
        base = 0x10010000u;
        //return;
        break;
    case 1:
        base = 0x10011000u;
        return;
        break;
    case 2:
        base = 0x10011000u;
        return;
        break;
    case 3:
        base = 0x10011000u;
        return;
        break;
    case 4:
        base = 0x10011000u;
        return;
        break;
    default:
        return;
        break;
    }

    volatile uint32_t full;
    do {
        full = *(volatile uint32_t *)((uint64_t)base) & 0x80000000u;

        if (full) {
            //volatile int delay = 1000u;
            //while (delay) { delay--; }
        }
    } while (full);
    *(volatile uint32_t *)((uint64_t)base) = (uint32_t)(ch);
}

#define HSS_UART_HELPER_MAX_GETLINE 80
ssize_t uart_getline(char **pBuffer, size_t *pBufLen)
{
    ssize_t result = 0;

    static char myBuffer[HSS_UART_HELPER_MAX_GETLINE]; // static to be stack friendly
    const size_t bufferLen = mSPAN_OF(myBuffer);

    memset(myBuffer, 0, bufferLen);

    void sbi_gets(char *s, int maxwidth, char endchar);

    sbi_gets(myBuffer, bufferLen, '\n');

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
    bool result = true;

    assert(pbuf);

    int sifive_uart_getc(void);
    int rx_byte = sifive_uart_getc();

    if (timeout_sec == 0) {
        if (rx_byte >= 0) {
            *pbuf = rx_byte;
        }
    } else if (timeout_sec > 0) {
        if (rx_byte >= 0) {
            *pbuf = rx_byte;
        }
    } else /* if (timeout_sec < 0) */ {
    }

    return result;
}
