/*******************************************************************************
 * Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
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

#include "gpio_ui_service.h"
#if IS_ENABLED(CONFIG_SERVICE_USBDMSC)
#  include "usbdmsc_service.h"
#endif
#include "mpfs_reg_map.h"
#include "hss_trigger.h"
#if IS_ENABLED(CONFIG_SERVICE_BOOT)
#  include "hss_boot_init.h"
#endif

static void gpio_ui_init_onEntry(struct StateMachine * const pMyMachine);
static void gpio_ui_init_handler(struct StateMachine * const pMyMachine);
static void gpio_ui_preboot_handler(struct StateMachine * const pMyMachine);
static void gpio_ui_usbdmsc_handler(struct StateMachine * const pMyMachine);
static void gpio_ui_idle_handler(struct StateMachine * const pMyMachine);

/*!
 * \brief TINYCLI Driver States
 *
 */
enum UartStatesEnum {
    GPIO_UI_INITIALIZATION,
    GPIO_UI_PREBOOT,
    GPIO_UI_USBDMSC,
    GPIO_UI_IDLE,
    GPIO_UI_NUM_STATES = GPIO_UI_IDLE+1
};

/*!
 * \brief TINYCLI Driver State Descriptors
 *
 */
static const struct StateDesc gpio_ui_state_descs[] = {
    { (const stateType_t)GPIO_UI_INITIALIZATION, (const char *)"init",    &gpio_ui_init_onEntry, NULL, &gpio_ui_init_handler },
    { (const stateType_t)GPIO_UI_PREBOOT,        (const char *)"preboot", NULL,                  NULL, &gpio_ui_preboot_handler },
    { (const stateType_t)GPIO_UI_USBDMSC,        (const char *)"usbdmsc", NULL,                  NULL, &gpio_ui_usbdmsc_handler },
    { (const stateType_t)GPIO_UI_IDLE,           (const char *)"idle",    NULL,                  NULL, &gpio_ui_idle_handler }
};

/*!
 * \brief TINYCLI Driver State Machine
 *
 */
struct StateMachine gpio_ui_service = {
    .state             = (stateType_t)GPIO_UI_INITIALIZATION,
    .prevState         = (stateType_t)SM_INVALID_STATE,
    .numStates         = (const uint32_t)GPIO_UI_NUM_STATES,
    .pMachineName      = (const char *)"gpio_ui_service",
    .startTime         = 0u,
    .lastExecutionTime = 0u,
    .executionCount    = 0u,
    .pStateDescs       = gpio_ui_state_descs,
    .debugFlag         = true,
    .priority          = 0u,
    .pInstanceData     = NULL
};

// --------------------------------------------------------------------------------------------------
// Handlers for each state in the state machine
//

#if IS_ENABLED(CONFIG_SERVICE_USBDMSC)
static bool usbdmsc_requested = false;
static bool check_if_usbdmsc_requested_(void)
{
    if (!usbdmsc_requested) {
        if (HSS_GPIO_UI_Preboot_Check_Button()) {
            HSS_Trigger_Notify(EVENT_USBDMSC_REQUESTED);
            usbdmsc_requested = true;

            mHSS_DEBUG_PRINTF(LOG_WARN, "GPIO_UI: USBDMSC requested\n");
        }
    }

    return usbdmsc_requested;
}
#endif

static void gpio_ui_init_onEntry(struct StateMachine * const pMyMachine)
{
    GPIO_UI_Init();
#if IS_ENABLED(CONFIG_SERVICE_USBDMSC)
    usbdmsc_requested = false;
#endif
}

static void gpio_ui_init_handler(struct StateMachine * const pMyMachine)
{
    if (
#if IS_ENABLED(CONFIG_SERVICE_DDR)
        HSS_Trigger_IsNotified(EVENT_DDR_TRAINED) &&
#endif
        HSS_Trigger_IsNotified(EVENT_STARTUP_COMPLETE)) {
        pMyMachine->state = GPIO_UI_PREBOOT;
#if IS_ENABLED(CONFIG_SERVICE_USBDMSC)
    } else {
        (void)check_if_usbdmsc_requested_();
#endif
    }
}

static void gpio_ui_preboot_handler(struct StateMachine * const pMyMachine)
{
#if IS_ENABLED(CONFIG_SERVICE_USBDMSC)
    if (check_if_usbdmsc_requested_()) {
        pMyMachine->state = GPIO_UI_USBDMSC;
    } else
#endif
    {
#if !IS_ENABLED(CONFIG_SERVICE_TINYCLI)
        if (HSS_BootInit()) { HSS_BootHarts(); } // attempt boot
#endif
        pMyMachine->state = GPIO_UI_IDLE;
    }
}

/////////////////

static void gpio_ui_usbdmsc_handler(struct StateMachine * const pMyMachine)
{
#if IS_ENABLED(CONFIG_SERVICE_USBDMSC)
    if (HSS_GPIO_UI_user_button_pressed() // cancelled by button press
        ||  !USBDMSC_IsActive()) { // cancelled by cable removal
        HSS_Trigger_Notify(EVENT_USBDMSC_FINISHED);
#else
    {
#endif
#if !IS_ENABLED(CONFIG_SERVICE_TINYCLI)
        if (HSS_BootInit()) { HSS_BootHarts(); } // attempt boot
#endif
        pMyMachine->state = GPIO_UI_IDLE;
    }
}

/////////////////

static void gpio_ui_idle_handler(struct StateMachine * const pMyMachine)
{
    (void)pMyMachine;
}
