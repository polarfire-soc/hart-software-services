/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS Embedded Software
 *
 */


#include "config.h"
#include "hss_types.h"
#include "opensbi_service.h"

#if !IS_ENABLED(CONFIG_OPENSBI)
#  error OPENSBI needed for this module
#endif

#include <sbi/sbi_domain.h>
#include <sbi/sbi_ecall.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_trap.h>
#include <sbi/sbi_version.h>
#include <sbi/riscv_asm.h>
#include <sbi/riscv_barrier.h>

#include <u54_state.h>
#include "hss_trigger.h"
#include "opensbi_suspend_ecall.h"

int sbi_ecall_susp_handler(unsigned long extid, unsigned long funcid,
    const struct sbi_trap_regs *regs, unsigned long *out_val, struct sbi_trap_info *out_trap)
{
    int ret;

    switch (funcid) {
    case SBI_EXT_SUSP_SYSTEM_SUSPEND:
        if (1 == mpfs_domains_get_count()) {
           // Wait for all secondary harts to reach spin_forever (Idle) before touching DDR
           const struct sbi_domain *dom = sbi_domain_thishart_ptr();
           int h;
           sbi_hartmask_for_each_hart(h, dom->possible_harts) {
               if (h == current_hartid()) continue; // boot hart is still running
               while (HSS_U54_GetState_Ex(h) != HSS_State_Idle) {
                   ;
               }
           }

            mpfs_system_suspend();

            mHSS_DEBUG_PRINTF_EX(
                "__        __    _ _   _                 __\n"
                "\\ \\      / /_ _(_) |_(_)_ __   __ _    / _| ___  _ __\n"
                " \\ \\ /\\ / / _` | | __| | '_ \\ / _` |  | |_ / _ \\| '__|\n"
                "  \\ V  V / (_| | | |_| | | | | (_| |  |  _| (_) | |\n"
                "   \\_/\\_/ \\__,_|_|\\__|_|_| |_|\\__, |  |_|  \\___/|_|\n"
                "                              |___/\n"
                "                                           _                   _\n"
                " _ __ ___  ___ _   _ _ __ ___   ___    ___(_) __ _ _ __   __ _| |\n"
                "| '__/ _ \\/ __| | | | '_ ` _ \\ / _ \\  / __| |/ _` | '_ \\ / _` | |\n"
	        "| | |  __/\\__ \\ |_| | | | | | |  __/  \\__ \\ | (_| | | | | (_| | |\n"
		"|_|  \\___||___/\\__,_|_| |_| |_|\\___|  |___/_|\\__, |_| |_|\\__,_|_|\n"
                "                                            |___/\n"
                "\n");

            HSS_Trigger_Clear(EVENT_SYSTEM_SUSPEND_RESUME);
            while (!HSS_Trigger_IsNotified(EVENT_SYSTEM_SUSPEND_RESUME)) {
                ;
            }

            mpfs_system_resume();

            //
            // Clear the suspend trigger now.  Note that secondary harts
            // (U54_2 to U54_4) are still being resumed via mpfs_hart_start() /
            // HSS_Boot_SendResumeGOTO() at this point (~4s per hart).
            //
            // Their watchdogs will have fired during the suspend window and
            // will not be refreshed until Linux is running on each hart again.
            //
            // U54 Watchdog suppression above therefore ends before those harts are back
            // the hardware watchdog fuse values on U54_1 to U54_4 must be large enough
            // to cover the full resume time (~16 s for 4 harts) to avoid a spurious reboot
            // detection.
            //
            HSS_Trigger_Clear(EVENT_SYSTEM_SUSPEND_RESUME); // clear mask of watchdogs
        } else {
            // Suspend is not supported in a meaningful for AMP (can't put DDR into self refresh)...
            // nothing to do if more than one domain => come straight back out ...
        }

        // a0 needs to hold hartid, and a1 needs to hold the resume context - that's passed from a1
        sbi_hart_switch_mode(current_hartid(), regs->a2, regs->a1 /* holds next address */, PRV_S, 0);
        ret = 0;
        *out_val = ret;
        break;

    default:
        ret = SBI_ENOTSUPP;
    };

    return ret;
}

struct sbi_ecall_extension ecall_susp = {
    .extid_start = SBI_EXT_SUSP,
    .extid_end = SBI_EXT_SUSP,
    .handle = sbi_ecall_susp_handler,
};
