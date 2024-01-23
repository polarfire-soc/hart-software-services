/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
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
#include "hss_boot_init.h"
#include "hss_progress.h"

#include <string.h> //memset
#include <assert.h>

#include "tinycli_service.h"
#include "uart_helper.h"
#include "mpfs_reg_map.h"
#include "hss_boot_service.h"
#include "usbdmsc_service.h"

#include "drivers/mss/mss_mmuart/mss_uart.h"

static void tinycli_init_handler(struct StateMachine * const pMyMachine);
static void tinycli_preboot_handler(struct StateMachine * const pMyMachine);
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
    TINYCLI_PREBOOT,
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
    { (const stateType_t)TINYCLI_PREBOOT,        (const char *)"preboot",        NULL,                      NULL,                     &tinycli_preboot_handler },
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
    pMyMachine->state = TINYCLI_PREBOOT;
}

/////////////////

static char myPrevBuffer[HSS_UART_HELPER_MAX_GETLINE];
static char myBuffer[HSS_UART_HELPER_MAX_GETLINE];
const size_t bufferLen = ARRAY_SIZE(myBuffer)-1;
ssize_t readStringLen = 0;

/////////////////

const char* lineHeader = ">> ";
#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_ENABLE_PREBOOT_TIMEOUT)
static HSSTicks_t readlineIdleTime = 0u;
#endif

static void tinycli_preboot_handler(struct StateMachine * const pMyMachine)
{
   bool keyPressedFlag = false;
   uint8_t rcv_buf;

   keyPressedFlag = HSS_ShowTimeout("Press a key to enter CLI, ESC to skip\n",
       CONFIG_SERVICE_TINYCLI_TIMEOUT, &rcv_buf);

    if (!keyPressedFlag) {
        mHSS_FANCY_PUTS(LOG_NORMAL, "CLI boot interrupt timeout\n");
        HSS_BootHarts();
    } else {
        mHSS_FANCY_PUTS(LOG_NORMAL, "Type HELP for list of commands\n");
#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_ENABLE_PREBOOT_TIMEOUT)
        readlineIdleTime = HSS_GetTime();
#endif
    }

    pMyMachine->state = TINYCLI_READLINE;
}

static void tinycli_readline_onEntry(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;

#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_ENABLE_PREBOOT_TIMEOUT)
#  define PREBOOT_IDLE_TIMEOUT (ONE_SEC * CONFIG_SERVICE_TINYCLI_PREBOOT_TIMEOUT)
    if (!HSS_BootInit_IsPostInit()) {
        if (HSS_Timer_IsElapsed(readlineIdleTime, PREBOOT_IDLE_TIMEOUT)) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "***** Timeout on Pre-Boot TinyCLI *****\n");
            HSS_SpinDelay_Secs(5u);
            tinyCLI_Reset_();
        }
    }
#endif

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

    if (uart_getchar(cBuf, 0, false)) {
	if (escapeActive) {
		switch (cBuf[0]) {
		case '[':
                    // consume
		    break;

                case 'A': // up arrow
                    readStringLen = strlen(myBuffer);
                    uart_putc(HSS_HART_E51, '\r');
                    uart_putstring(HSS_HART_E51, (char *)lineHeader);
                    if (readStringLen) {
                        memcpy(myBuffer, myPrevBuffer, readStringLen);
                        uart_putstring(HSS_HART_E51, (char *)myBuffer);
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
                        uart_putstring(HSS_HART_E51, (char *)"\033[C");
                    }
                    return;

                case 'D': // left arrow
                    if (readStringLen) {
                        readStringLen--;
                        uart_putstring(HSS_HART_E51, (char *)"\033[D");
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
            uart_putc(HSS_HART_E51, cBuf[0]);
            if (readStringLen < bufferLen) {
                myBuffer[readStringLen] = '\0';
            }
	    if (readStringLen) {
                memcpy(myPrevBuffer, myBuffer, readStringLen+1);
            } else {
                // if just hit enter, as a convenience, reuse last command (a la GDB)
		readStringLen = strlen(myPrevBuffer);
                memcpy(myBuffer, myPrevBuffer, readStringLen+1);
            }
            pMyMachine->state = TINYCLI_PARSELINE;
            break;

        case 0x7Fu: // delete
            if (readStringLen) {
                readStringLen--;
                uart_putstring(HSS_HART_E51, (char *)"\033[D \033[D");
                myBuffer[readStringLen] = 0;
            }
            break;

        case 0x08u: // backspace - ^H
            if (readStringLen) {
                readStringLen--;
                uart_putstring(HSS_HART_E51, (char *)"\033[D");
                myBuffer[readStringLen] = 0;
            }
            break;

        case 0x01u: // ^A
            readStringLen = 0;
            uart_putc(HSS_HART_E51, '\r');
            mHSS_PUTS(lineHeader);
            break;

        case 0x05u: // ^E
            readStringLen = strlen(myBuffer);
            uart_putc(HSS_HART_E51, '\r');
            uart_putstring(HSS_HART_E51, (char *)lineHeader);
            uart_putstring(HSS_HART_E51, (char *)myBuffer);
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
                uart_putc(HSS_HART_E51, cBuf[0]);
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

    const char crlf[] = "\n";
    uart_putstring(HSS_HART_E51, (char *)crlf);

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

    if (!done && (uart_getchar(cBuf, 0, false))) {
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
