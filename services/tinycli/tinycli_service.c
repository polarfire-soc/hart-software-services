/*******************************************************************************
 * Copyright 2019-2021 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file tinycli Driver State Machine
 * \brief Virtualised tinycli Service
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"
#include "hss_clock.h"

#include <string.h> //memset
#include <assert.h>

#include "tinycli_service.h"
#include "mpfs_reg_map.h"
#include "hss_boot_service.h"
#include "usbdmsc_service.h"

#include "drivers/mss_uart/mss_uart.h"

static void tinycli_init_handler(struct StateMachine * const pMyMachine);
static void tinycli_readline_onEntry(struct StateMachine * const pMyMachine);
static void tinycli_readline_handler(struct StateMachine * const pMyMachine);
static void tinycli_readline_onExit(struct StateMachine * const pMyMachine);
static void tinycli_parseline_handler(struct StateMachine * const pMyMachine);
static void tinycli_usbdmsc_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief TINYCLI Driver States
 *
 */
enum UartStatesEnum {
    TINYCLI_INITIALIZATION,
    TINYCLI_READLINE,
    TINYCLI_PARSELINE,
    TINYCLI_USBDMSC,
    TINYCLI_NUM_STATES = TINYCLI_USBDMSC+1
};

/*!
 * \brief TINYCLI Driver State Descriptors
 *
 */
static const struct StateDesc tinycli_state_descs[] = {
    { (const stateType_t)TINYCLI_INITIALIZATION, (const char *)"init",      NULL,                      NULL,                     &tinycli_init_handler },
    { (const stateType_t)TINYCLI_READLINE,       (const char *)"readline",  &tinycli_readline_onEntry, &tinycli_readline_onExit, &tinycli_readline_handler },
    { (const stateType_t)TINYCLI_PARSELINE,      (const char *)"parseline", NULL,                      NULL,                     &tinycli_parseline_handler },
    { (const stateType_t)TINYCLI_USBDMSC,        (const char *)"usbdmsc",   NULL,                      NULL,                     &tinycli_usbdmsc_handler }
};

/*!
 * \brief TINYCLI Driver State Machine
 *
 */
struct StateMachine tinycli_service = {
    (stateType_t)TINYCLI_INITIALIZATION,
    (stateType_t)SM_INVALID_STATE,
    (const uint32_t)TINYCLI_NUM_STATES,
    (const char *)"tinycli_service", 0u, 0u, 0u, tinycli_state_descs, false, 0u, NULL
};

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//

static void tinycli_init_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state = TINYCLI_READLINE;
}

/////////////////

#define HSS_UART_HELPER_MAX_GETLINE 80

static char myBuffer[HSS_UART_HELPER_MAX_GETLINE];
const size_t bufferLen = ARRAY_SIZE(myBuffer);
ssize_t readStringLen = 0;

/////////////////

static void tinycli_readline_onEntry(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;

    memset(myBuffer, 0, bufferLen);
    readStringLen = 0u;
    mHSS_PUTS(">> ");

}

static void tinycli_readline_handler(struct StateMachine * const pMyMachine)
{
    uint8_t cBuf[1];

    if (0 != MSS_UART_get_rx(&g_mss_uart0_lo, cBuf, 1)) {
        switch (cBuf[0]) {
        case '\r':
            MSS_UART_polled_tx(&g_mss_uart0_lo, cBuf, 1u);
            pMyMachine->state = TINYCLI_PARSELINE;
            break;

        case '\n':
            MSS_UART_polled_tx(&g_mss_uart0_lo, cBuf, 1u);
            pMyMachine->state = TINYCLI_PARSELINE;
            break;

        case 0x7Fu: // delete
            if (readStringLen) {
                readStringLen--;
                MSS_UART_polled_tx(&g_mss_uart0_lo, (uint8_t const *)"\033[D \033[D", 7u);
                myBuffer[readStringLen] = 0;
            }
            break;

        case 0x08u: // backspace - ^H
            if (readStringLen) {
                readStringLen--;
                MSS_UART_polled_tx(&g_mss_uart0_lo, (uint8_t const *)" \033[D", 4u);
                myBuffer[readStringLen] = 0;
            }
            break;

        case 0x03u: // intr - ^C
            readStringLen = -1;
            myBuffer[0] = 0;
            pMyMachine->state = TINYCLI_PARSELINE;
            break;

        case 0x1Bu: // ESC
            readStringLen = -1;
            myBuffer[0] = 0;
            pMyMachine->state = TINYCLI_PARSELINE;
            break;

        case 0x04u: // ^D
            if (readStringLen == 0) {
                readStringLen = -1;
                myBuffer[0] = 0;
                pMyMachine->state = TINYCLI_PARSELINE;
            }
            break;

        default:
            if (readStringLen < bufferLen) {
                MSS_UART_polled_tx(&g_mss_uart0_lo, cBuf, 1u);
                myBuffer[readStringLen] = cBuf[0];
                readStringLen++;
            }
            break;
        }
    }
}

static void tinycli_readline_onExit(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;

    const char crlf[] = CRLF;
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, (const uint8_t *)crlf);

    if (readStringLen > 0) {
    } else {
        readStringLen = 0;
        myBuffer[0] = '\0';
    }
}

/////////////////

static void tinycli_parseline_handler(struct StateMachine * const pMyMachine)
{
    if (readStringLen > 0) {

        if (HSS_TinyCLI_ParseIntoTokens(myBuffer)) {
            HSS_TinyCLI_Execute();
        }
    }

    if (pMyMachine->state == TINYCLI_PARSELINE) {
        pMyMachine->state = TINYCLI_READLINE;
    }
}

/////////////////

static void tinycli_usbdmsc_handler(struct StateMachine * const pMyMachine)
{
#if IS_ENABLED(CONFIG_SERVICE_USBDMSC)
    bool done = false;
    uint8_t cBuf[1];

    bool usbdmsc_is_active(void);
    done = !usbdmsc_is_active();

    if (!done && (0 != MSS_UART_get_rx(&g_mss_uart0_lo, cBuf, 1))) {
        done = (cBuf[0] == '\003') || (cBuf[0] == '\033');
    }

    if (done) {
        void usbdmsc_deactivate(void);

        usbdmsc_deactivate();
        pMyMachine->state = TINYCLI_READLINE;
    }
#else
    pMyMachine->state = TINYCLI_READLINE;
#endif
}

/////////////////
void tinycli_wait_for_usbmscd_done(void);
void tinycli_wait_for_usbmscd_done(void)
{
    tinycli_service.state = TINYCLI_USBDMSC;
}
