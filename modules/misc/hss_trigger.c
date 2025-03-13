/*******************************************************************************
 * Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file Triggers
 * \brief Triggers
 */

#include "config.h"
#include "hss_types.h"
#include "assert.h"

#include "hss_debug.h"
#include "hss_trigger.h"
#include "riscv_atomic.h"

#if IS_ENABLED(CONFIG_SERVICE_TINYCLI)
#  include "tinycli_service.h"
#endif
#if IS_ENABLED(CONFIG_OPENSBI)
#  include "opensbi_service.h"
#endif

#define CONFIG_DEBUG_TRIGGERS 0

struct HSS_Triggers
{
    atomic_t opensbi_initialized;
    atomic_t ipi_initialized;
    atomic_t ddr_trained;
    atomic_t startup_complete;
    atomic_t usbdmsc_requested;
    atomic_t post_boot;
    atomic_t boot_complete;
    atomic_t hart_state_changed;
    atomic_t healthmon;
} triggerStatus = {
    .opensbi_initialized = ATOMIC_INITIALIZER(0),
    .ipi_initialized =      ATOMIC_INITIALIZER(0),
    .ddr_trained =         ATOMIC_INITIALIZER(0),
    .startup_complete =    ATOMIC_INITIALIZER(0),
    .usbdmsc_requested =   ATOMIC_INITIALIZER(0),
    .post_boot =           ATOMIC_INITIALIZER(0),
    .boot_complete =       ATOMIC_INITIALIZER(0),
    .hart_state_changed =  ATOMIC_INITIALIZER(0),
    .healthmon =           ATOMIC_INITIALIZER(0),
};

void HSS_Trigger_Notify(enum HSS_Event event)
{
#if IS_ENABLED(CONFIG_DEBUG_TRIGGERS)
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

    char const * const triggerNames[] = {
        [EVENT_OPENSBI_INITIALIZED] = "OpenSBI Initialized",
        [EVENT_IPI_INITIALIZED] =     "IPI Initialized",
        [EVENT_DDR_TRAINED] =         "DDR Trained",
        [EVENT_STARTUP_COMPLETE] =    "Initial Startup Complete",
        [EVENT_USBDMSC_REQUESTED] =   "USBDMSC Request",
        [EVENT_POST_BOOT] =           "Post First Boot",
        [EVENT_BOOT_COMPLETE] =       "Boot Complete",
        [EVENT_HART_STATE_CHANGED] =  "Hart State Changed",
        [EVENT_HEALTHMON] =           "Healthmon Event",
    };

    assert(event < ARRAY_SIZE(triggerNames));
    mHSS_DEBUG_PRINTF(LOG_WARN, "*** TRIGGER: >>%s<<\n", (char *)triggerNames[event]);
#endif

    switch (event) {
    case EVENT_OPENSBI_INITIALIZED:
        atomic_write(&triggerStatus.opensbi_initialized, 1);
        break;

    case EVENT_IPI_INITIALIZED:
        atomic_write(&triggerStatus.ipi_initialized, 1);
        break;

    case EVENT_DDR_TRAINED:
        atomic_write(&triggerStatus.ddr_trained, 1);
        break;

    case EVENT_STARTUP_COMPLETE:
        atomic_write(&triggerStatus.startup_complete, 1);
        break;

    case EVENT_USBDMSC_REQUESTED:
        atomic_write(&triggerStatus.usbdmsc_requested, 1);
//mHSS_DEBUG_PRINTF(LOG_WARN, "Notifying Trigger USBDMSC_REQUESTED\n");
        break;

    case EVENT_POST_BOOT:
        atomic_write(&triggerStatus.post_boot, 1);
#if IS_ENABLED(CONFIG_SERVICE_BOOT) // TODO - move all this into uart_helper function
#  if IS_ENABLED(CONFIG_UART_SURRENDER)
#    if IS_ENABLED(CONFIG_SERVICE_TINYCLI)
        HSS_TinyCLI_SurrenderUART();
#    endif
#    if IS_ENABLED(CONFIG_OPENSBI)
        mpfs_uart_surrender();
#    endif
#  endif
#endif
        break;

    case EVENT_BOOT_COMPLETE:
        atomic_write(&triggerStatus.boot_complete, 1);
        break;

    case EVENT_HART_STATE_CHANGED:
        break;

    case EVENT_HEALTHMON:
        break;

    default:
        break;
    }
}

bool HSS_Trigger_IsNotified(enum HSS_Event event)
{
    bool result = false;

    switch (event) {
    case EVENT_OPENSBI_INITIALIZED:
        result = atomic_read(&triggerStatus.opensbi_initialized) ? true : false;
        break;

    case EVENT_IPI_INITIALIZED:
        result = atomic_read(&triggerStatus.ipi_initialized) ? true : false;
        break;

    case EVENT_DDR_TRAINED:
        // DDR is considered notified as "trained" if training has completed,
        // or if DDR service is not enabled
        result = atomic_read(&triggerStatus.ddr_trained) ? true : !IS_ENABLED(CONFIG_SERVICE_DDR);
        break;

    case EVENT_STARTUP_COMPLETE:
        result = atomic_read(&triggerStatus.startup_complete) ? true : false;
        break;

    case EVENT_USBDMSC_REQUESTED:
        result = atomic_read(&triggerStatus.usbdmsc_requested) ? true : false;
        break;

    case EVENT_POST_BOOT:
        result = atomic_read(&triggerStatus.post_boot) ? true : false;
        break;

    case EVENT_BOOT_COMPLETE:
        result = atomic_read(&triggerStatus.boot_complete) ? true : false;
        break;

    case EVENT_HART_STATE_CHANGED:
        break;

    case EVENT_HEALTHMON:
        break;

    default:
        break;
    }

    return result;
}

void HSS_Trigger_Clear(enum HSS_Event event)
{
    switch (event) {
    case EVENT_OPENSBI_INITIALIZED:
        atomic_write(&triggerStatus.opensbi_initialized, 0);
        break;

    case EVENT_IPI_INITIALIZED:
        atomic_write(&triggerStatus.ipi_initialized, 0);
        break;

    case EVENT_DDR_TRAINED:
        atomic_write(&triggerStatus.ddr_trained, 0);
        break;

    case EVENT_STARTUP_COMPLETE:
        atomic_write(&triggerStatus.startup_complete, 0);
        break;

    case EVENT_USBDMSC_REQUESTED:
        atomic_write(&triggerStatus.usbdmsc_requested, 0);
//mHSS_DEBUG_PRINTF(LOG_WARN, "Clearing Trigger USBDMSC_REQUESTED\n");
        break;

    case EVENT_POST_BOOT:
        atomic_write(&triggerStatus.post_boot, 0);
        break;

    case EVENT_BOOT_COMPLETE:
        atomic_write(&triggerStatus.boot_complete, 0);
        break;

    case EVENT_HART_STATE_CHANGED:
        break;

    case EVENT_HEALTHMON:
        break;

    default:
        break;
    }
}
