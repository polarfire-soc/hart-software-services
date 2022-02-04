/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
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
#include "uart_helper.h"
#include "mpfs_reg_map.h"
#include "hss_boot_service.h"
#include "usbdmsc_service.h"

#include "drivers/mss_uart/mss_uart.h"

#if IS_ENABLED(CONFIG_SERVICE_USBDMSC)
#  include "usbdmsc_service.h"
#endif

static void tinycli_init_handler(struct StateMachine * const pMyMachine);
static void tinycli_readline_onEntry(struct StateMachine * const pMyMachine);
static void tinycli_readline_handler(struct StateMachine * const pMyMachine);
static void tinycli_readline_onExit(struct StateMachine * const pMyMachine);
static void tinycli_parseline_handler(struct StateMachine * const pMyMachine);
static void tinycli_usbdmsc_handler(struct StateMachine * const pMyMachine);
static void tinycli_uart_surrender_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief TINYCLI Driver States
 *
 */
enum UartStatesEnum {
    TINYCLI_INITIALIZATION,
    TINYCLI_READLINE,
    TINYCLI_PARSELINE,
    TINYCLI_USBDMSC,
    TINYCLI_UART_SURRENDER,
    TINYCLI_NUM_STATES = TINYCLI_UART_SURRENDER+1
};

/*!
 * \brief TINYCLI Driver State Descriptors
 *
 */
static const struct StateDesc tinycli_state_descs[] = {
    { (const stateType_t)TINYCLI_INITIALIZATION, (const char *)"init",           NULL,                      NULL,                     &tinycli_init_handler },
    { (const stateType_t)TINYCLI_READLINE,       (const char *)"readline",       &tinycli_readline_onEntry, &tinycli_readline_onExit, &tinycli_readline_handler },
    { (const stateType_t)TINYCLI_PARSELINE,      (const char *)"parseline",      NULL,                      NULL,                     &tinycli_parseline_handler },
    { (const stateType_t)TINYCLI_USBDMSC,        (const char *)"usbdmsc",        NULL,                      NULL,                     &tinycli_usbdmsc_handler },
    { (const stateType_t)TINYCLI_UART_SURRENDER, (const char *)"uart_surrender", NULL,                      NULL,                     &tinycli_uart_surrender_handler }
};

/*!
 * \brief TINYCLI Driver State Machine
 *
 */
struct StateMachine tinycli_service = {
    .state             = (stateType_t)TINYCLI_INITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         = (const uint32_t)TINYCLI_NUM_STATES,
    .pMachineName      = (const char *)"tinycli_service",
    .startTime         = 0u,
    .lastExecutionTime = 0u,
    .executionCount    = 0u,
    .pStateDescs       = tinycli_state_descs,
    .debugFlag         = false,
    .priority          = 0u,
    .pInstanceData     = NULL
};

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//

static void tinycli_init_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state = TINYCLI_READLINE;
}

/////////////////

static char myPrevBuffer[HSS_UART_HELPER_MAX_GETLINE];
static char myBuffer[HSS_UART_HELPER_MAX_GETLINE];
const size_t bufferLen = ARRAY_SIZE(myBuffer)-1;
ssize_t readStringLen = 0;

/////////////////

const char* lineHeader = ">> ";

static void tinycli_readline_onEntry(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;

    //myBuffer[0] = '\0';
    readStringLen = 0u;
    mHSS_PUTS(lineHeader);
}

static void tinycli_readline_handler(struct StateMachine * const pMyMachine)
{
    uint8_t cBuf[1];

#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_MONITOR)
    HSS_TinyCLI_RunMonitors();
#endif

    static bool escapeActive = false;

    if (0 != MSS_UART_get_rx(&g_mss_uart0_lo, cBuf, 1)) {
	if (escapeActive) {
		switch (cBuf[0]) {
		case '[':
                    // consume
		    break;

                case 'A': // up arrow
                    readStringLen = strlen(myBuffer);
                    MSS_UART_polled_tx(&g_mss_uart0_lo, (uint8_t const *)"\r", 1);
                    mHSS_PUTS(lineHeader);
                    if (readStringLen) {
                        memcpy(myBuffer, myPrevBuffer, readStringLen);
                        MSS_UART_polled_tx(&g_mss_uart0_lo, (uint8_t const *)myBuffer, readStringLen);
                    } else {
                        myBuffer[0] = '\0';
                    }
		    escapeActive = false;
                    return;

                case 'B': // down arrow
		    escapeActive = false;
                    return;

                case 'C': // right arrow
		    escapeActive = false;
                    if (readStringLen < bufferLen) {
                        readStringLen++;
                        if (myBuffer[readStringLen] == 0) {
                            myBuffer[readStringLen] = ' ';
                        }
                        MSS_UART_polled_tx(&g_mss_uart0_lo, (uint8_t const *)"\033[C", 4u);
                    }
                    return;

                case 'D': // left arrow
                    if (readStringLen) {
                        readStringLen--;
                        MSS_UART_polled_tx(&g_mss_uart0_lo, (uint8_t const *)"\033[D", 4u);
                    }
		    escapeActive = false;
                    return;

                default:
		    escapeActive = false;
                    break;
		}

		if (escapeActive) { return; }
	}

        switch (cBuf[0]) {
        case '\r':
            __attribute__((fallthrough)); // deliberate fallthrough
        case '\n':
            MSS_UART_polled_tx(&g_mss_uart0_lo, cBuf, 1u);
            if (readStringLen < bufferLen) {
                myBuffer[readStringLen] = '\0';
            }
	    if (readStringLen) {
                memcpy(myPrevBuffer, myBuffer, readStringLen+1);
            } else {
                // if just hit enter, as a convenience, reuse last command (a la GDB)
		//mHSS_DEBUG_PRINTF(LOG_WARN, "Convenience: copying >>%s<< into myBuffer" CRLF, myPrevBuffer);
		readStringLen = strlen(myPrevBuffer);
                memcpy(myBuffer, myPrevBuffer, readStringLen+1);
            }
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

        case 0x01u: // ^A
            readStringLen = 0;
            MSS_UART_polled_tx(&g_mss_uart0_lo, (uint8_t const *)"\r", 1);
            mHSS_PUTS(lineHeader);
            break;

        case 0x05u: // ^E
            readStringLen = strlen(myBuffer);
            MSS_UART_polled_tx(&g_mss_uart0_lo, (uint8_t const *)"\r", 1);
            mHSS_PUTS(lineHeader);
            MSS_UART_polled_tx(&g_mss_uart0_lo, (uint8_t const *)myBuffer, readStringLen);
            break;

        case 0x04u: // ^D
            if (readStringLen != 0) {
                break;
            }
            __attribute__((fallthrough)); // deliberate fallthrough
        case 0x03u: // intr - ^C
            readStringLen = -1;
            myBuffer[0] = 0;
            pMyMachine->state = TINYCLI_PARSELINE;
            break;

        case 0x1Bu: // ESC
            escapeActive = true;
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
#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_MONITOR)
    HSS_TinyCLI_RunMonitors();
#endif

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
#  if IS_ENABLED(CONFIG_SERVICE_TINYCLI_MONITOR)
    HSS_TinyCLI_RunMonitors();
#  endif

    bool done = false;
    uint8_t cBuf[1];

    done = !USBDMSC_IsActive();

    if (!done && (0 != MSS_UART_get_rx(&g_mss_uart0_lo, cBuf, 1))) {
        done = (cBuf[0] == '\003') || (cBuf[0] == '\033');
    }

    if (done) {
        USBDMSC_Deactivate();
        pMyMachine->state = TINYCLI_READLINE;
    }
#else
    pMyMachine->state = TINYCLI_READLINE;
#endif
}

/////////////////
void HSS_TinyCLI_WaitForUSBMSCDDone(void)
{
    tinycli_service.state = TINYCLI_USBDMSC;
}

static void tinycli_uart_surrender_handler(struct StateMachine * const pMyMachine)
{
	; // nothing to do here
}

void HSS_TinyCLI_SurrenderUART(void)
{
    tinycli_service.state = TINYCLI_UART_SURRENDER;
}
