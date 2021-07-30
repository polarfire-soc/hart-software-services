/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file USB Device Mass Storage Class State Machine
 * \brief USB Device Mass Storage Class
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"
#include "hss_clock.h"

#include <string.h> //memset
#include <assert.h>

#include "usbdmsc_service.h"
#include "mpfs_reg_map.h"
#include "hss_boot_service.h"

#include "drivers/mss_uart/mss_uart.h"

static void usbdmsc_init_handler(struct StateMachine * const pMyMachine);
static void usbdmsc_idle_handler(struct StateMachine * const pMyMachine);
static void usbdmsc_waitForUSBHost_onEntry(struct StateMachine * const pMyMachine);
static void usbdmsc_waitForUSBHost_handler(struct StateMachine * const pMyMachine);
static void usbdmsc_active_handler(struct StateMachine * const pMyMachine);
static void usbdmsc_active_onExit(struct StateMachine * const pMyMachine);

/*!
 * \brief USBDMSC Driver States
 *
 */
enum UartStatesEnum {
    USBDMSC_INITIALIZATION,
    USBDMSC_IDLE,
    USBDMSC_WAIT_FOR_USB_HOST,
    USBDMSC_ACTIVE,
    USBDMSC_NUM_STATES = USBDMSC_ACTIVE+1
};

/*!
 * \brief USBDMSC Driver State Descriptors
 *
 */
static const struct StateDesc usbdmsc_state_descs[] = {
    { (const stateType_t)USBDMSC_INITIALIZATION,    (const char *)"Init",             NULL,                            NULL,                   &usbdmsc_init_handler },
    { (const stateType_t)USBDMSC_IDLE,              (const char *)"Idle",             NULL,                            NULL,                   &usbdmsc_idle_handler },
    { (const stateType_t)USBDMSC_WAIT_FOR_USB_HOST, (const char *)"WaitForUSBHost",   &usbdmsc_waitForUSBHost_onEntry, NULL,                   &usbdmsc_waitForUSBHost_handler },
    { (const stateType_t)USBDMSC_ACTIVE,            (const char *)"Active",           NULL,                            &usbdmsc_active_onExit, &usbdmsc_active_handler },
};

/*!
 * \brief USBDMSC Driver State Machine
 *
 */
struct StateMachine usbdmsc_service = {
    .state             = (stateType_t)USBDMSC_INITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         = (const uint32_t)USBDMSC_NUM_STATES,
    .pMachineName      = (const char *)"usbdmsc_service",
    .startTime         = 0u,
    .lastExecutionTime = 0u,
    .executionCount    = 0u,
    .pStateDescs       = usbdmsc_state_descs,
    .debugFlag         = true,
    .priority          = 0u,
    .pInstanceData     = NULL
};

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//

static void usbdmsc_init_handler(struct StateMachine * const pMyMachine)
{
    pMyMachine->state = USBDMSC_IDLE;
}

/////////////////

static void usbdmsc_idle_handler(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;
}

/////////////////

static void usbdmsc_waitForUSBHost_onEntry(struct StateMachine * const pMyMachine)
{
    USBDMSC_Init();
    mHSS_PUTS("Waiting for USB Host to connect... (CTRL-C to quit)" CRLF);
}

uint32_t FLASH_DRIVE_is_host_connected(void); //TODO: tidy
static void usbdmsc_waitForUSBHost_handler(struct StateMachine * const pMyMachine)
{
    if (FLASH_DRIVE_is_host_connected()) {
        mHSS_PUTS("USB Host connected. Waiting for disconnect... (CTRL-C to quit)" CRLF);
        pMyMachine->state = USBDMSC_ACTIVE;
    }

    USBDMSC_Poll();
}

/////////////////

static void usbdmsc_active_handler(struct StateMachine * const pMyMachine)
{
    if (!FLASH_DRIVE_is_host_connected()) {
        pMyMachine->state = USBDMSC_IDLE;
    }

    USBDMSC_Poll();
}

static void usbdmsc_active_onExit(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;
    mHSS_PUTS(CRLF "USB Host disconnected..." CRLF);
}

/////////////////

void USBDMSC_Activate(void)
{
    usbdmsc_service.state = USBDMSC_WAIT_FOR_USB_HOST;
}

void USBDMSC_Deactivate(void)
{
    if (usbdmsc_service.state != USBDMSC_IDLE) {
        usbdmsc_service.state = USBDMSC_IDLE;
    }
}

bool USBDMSC_IsActive(void)
{
    return (usbdmsc_service.state != USBDMSC_IDLE);
}
