/*******************************************************************************
 * Copyright 2023 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file Reboot Handling
 * \brief Handling of reboot requests
 */
#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"
#include "hss_clock.h"

#include <assert.h>

#include "hss_boot_service.h"
#include "opensbi_service.h"
#include "reboot_service.h"
#include "wdog_service.h"
#include "csr_helper.h"

#include "sbi/riscv_encoding.h"
#include "sbi/sbi_ecall_interface.h"

#include "mss_watchdog.h"
#include "mss_sysreg.h"
#include "mss_sys_services.h"

#include "mpfs_fabric_reg_map.h"

#define AUTO_UPGRADE_IMAGE_INDEX 1u

static void __attribute__((__noreturn__, unused)) do_srst_ecall(void)
{
    register uintptr_t a7 asm ("a7") = SBI_EXT_SRST;
    register uintptr_t a6 asm ("a6") = SBI_EXT_SRST_RESET;
    register uintptr_t a0 asm ("a0") = SBI_SRST_RESET_TYPE_WARM_REBOOT;
    register uintptr_t a1 asm ("a1") = SBI_SRST_RESET_REASON_NONE;
    asm volatile ("ecall" : "+r" (a0), "+r" (a1) : "r" (a6), "r" (a7) : "memory");

    csr_write(CSR_MIE, MIP_MSIP);
    while (1) { asm("wfi"); }
}

static int HSS_reboot_auto_upgrade(void)
{
    uint16_t ret;

    // the ISR is set to NULL, as interrupt mode is broken (doesn't interrupt if
    // the service fails), and therefore we must use polling mode. Polling mode
    // should be the default, but set it explicitly just in case!
    MSS_SYS_select_service_mode(MSS_SYS_SERVICE_POLLING_MODE, NULL);

    // the image must be authenticated before attempting Auto Upgrade, as
    // invalid images may cause the system controller to hang
    ret = MSS_SYS_authenticate_iap_image(AUTO_UPGRADE_IMAGE_INDEX);
    if (ret) {
        if (ret == 24) {
            mHSS_DEBUG_PRINTF(LOG_NORMAL,
                              "reboot: Auto Upgrade image is not an upgrade: %d\n", ret);
        } else {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "reboot: No valid Auto Upgrade image found: %d\n", ret);
        }

        return ret;
    }

    // for Auto Update, only the 0th argument is used
    ret = MSS_SYS_execute_iap(MSS_SYS_IAP_AUTOUPDATE_CMD, 0, 0);
    if (!ret) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "reboot: Auto Upgrade in progress\n");
        while (1) { ; }
    }

    mHSS_DEBUG_PRINTF(LOG_ERROR, "reboot: Auto Upgrade failed: %d\n", ret);

    return ret;
}

static void HSS_reboot_cold_all(void)
{
    if (IS_ENABLED(CONFIG_COLDREBOOT_TRY_AUTO_UPGRADE)) {
        if(HSS_reboot_auto_upgrade())
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "reboot: Using fallback reboot provider\n");
    }

    if (IS_ENABLED(CONFIG_COLDREBOOT_FULL_FPGA_RESET)) {
        // Writing a 1 to the reset register of the tamper macro triggers a
        // full reset of the FPGA.
        mHSS_WriteRegU32(TAMPER, RESET, 1u);
    } else {
        SYSREG->MSS_RESET_CR = 0xDEAD;
    }
}

void HSS_reboot_cold(enum HSSHartId target)
{
    switch (target) {
    case HSS_HART_E51:
        MSS_WD_force_reset(MSS_WDOG0_LO);
        break;

    case HSS_HART_U54_1:
        MSS_WD_force_reset(MSS_WDOG1_LO);
        break;

    case HSS_HART_U54_2:
        MSS_WD_force_reset(MSS_WDOG2_LO);
        break;

    case HSS_HART_U54_3:
        MSS_WD_force_reset(MSS_WDOG3_LO);
        break;

    case HSS_HART_U54_4:
        MSS_WD_force_reset(MSS_WDOG4_LO);
        break;

    case HSS_HART_ALL:
        HSS_reboot_cold_all();

        while (1) { ; }

        break;

    default:
        assert(1 == 0); // should never reach here!! LCOV_EXCL_LINE
        break;
    }
}

void HSS_reboot(uint32_t wdog_status)
{
    uint32_t restart_mask = 0u;

    if (IS_ENABLED(CONFIG_ALLOW_COLDREBOOT_ALWAYS))
        HSS_reboot_cold(HSS_HART_ALL);

    // watchdog timer has triggered for a monitored hart.
    // ensure OpenSBI housekeeping is in order for requesting reboots.
    // if any of these harts are allowed permission to force a cold reboot
    // it will happen here also...

    for (enum HSSHartId source = HSS_HART_U54_1; source < HSS_HART_NUM_PEERS; source++) {
        if (!(wdog_status & (1u << source)))
            continue;

        mHSS_DEBUG_PRINTF(LOG_ERROR, "u54_%d: Watchdog has fired\n", source);

        if (IS_ENABLED(CONFIG_ALLOW_COLDREBOOT) && mpfs_is_cold_reboot_allowed(source)) {
            HSS_reboot_cold(HSS_HART_ALL);
        }

        // a cold reboot is not possible, treat it as a warm reboot now
        // potentially add a check for mpfs_is_warm_reboot_allowed(source) here
        restart_mask |= (1 << source);

        for (enum HSSHartId peer = HSS_HART_U54_1; peer < HSS_HART_NUM_PEERS; peer++) {
            if (peer == source)
                continue;

            if (mpfs_are_harts_in_same_domain(peer, source)) {
                restart_mask |= (1 << peer);
            }
        }
    }

    // if we reached here, nobody triggered a cold reboot, so
    // now trigger warm restarts as needed...
    if (restart_mask) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "reboot: Watchdog triggering reboot of ");
        for (enum HSSHartId peer = HSS_HART_U54_1; peer < HSS_HART_NUM_PEERS; peer++) {
            if (!(restart_mask & (1u << peer)))
                    continue;

            mHSS_DEBUG_PRINTF_EX("[u54_%d] ", peer);
#if IS_ENABLED(CONFIG_SERVICE_BOOT)
            // Restart core using SRST mechanism
            IPI_Send(peer, IPI_MSG_GOTO, 0u, PRV_M, do_srst_ecall, NULL);
            HSS_Wdog_Init_Time(peer);
            HSS_SpinDelay_MilliSecs(50u);
#endif
        }
        mHSS_DEBUG_PUTS("\n");

    }
}
