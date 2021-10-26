/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file U54 Handle IPI
 * \brief U54 Handle IPI
 */

#include "config.h"
#include "hss_types.h"
#include "hss_state_machine.h"
#include "hss_debug.h"

#include "ssmb_ipi.h"

#include <assert.h>

#include "csr_helper.h"

#include "ssmb_ipi.h"

#include "goto_service.h"
#include "opensbi_service.h"

#if IS_ENABLED(CONFIG_SERVICE_BOOT)
#  include "hss_boot_pmp.h"
#endif

#include "csr_helper.h"
#include "u54_handle_ipi.h"

#include "hss_atomic.h"

#if IS_ENABLED(CONFIG_HSS_USE_IHC)
#  define set_csr  csr_write
#  define read_csr csr_read
#  include "mss_plic.h"
#  include "miv_ihc.h"
#endif

//
// This routine gets executed when a U54 receives an IPI from E51 in machine mode...
//

#if !IS_ENABLED(CONFIG_HSS_USE_IHC)
static const struct IntentsArray
{
    enum IPIMessagesEnum msg_type;
} intentsArray[] = {
#if IS_ENABLED(CONFIG_SERVICE_BOOT)
    { IPI_MSG_PMP_SETUP },
#endif
#if IS_ENABLED(CONFIG_SERVICE_GOTO)
    { IPI_MSG_GOTO },
#endif
#if IS_ENABLED(CONFIG_SERVICE_OPENSBI)
    { IPI_MSG_OPENSBI_INIT },
#endif
};
#endif

bool HSS_U54_ConsumeIntent(enum IPIMessagesEnum msg_type);
bool HSS_U54_ConsumeIntent(enum IPIMessagesEnum msg_type)
{
    bool result = false;
#if IS_ENABLED(CONFIG_HSS_USE_IHC)
    result = IPI_ConsumeIntent(HSS_HART_E51, msg_type);
#endif
    return result;
}

bool HSS_U54_HandleIPI(void);
bool HSS_U54_HandleIPI(void)
{
    bool result = false;

#if IS_ENABLED(CONFIG_HSS_USE_IHC)
    enum HSSHartId my_hartid = current_hartid();

    __extension__ const uint32_t hartid_to_ihc_ext_interrupt[HSS_HART_NUM_PEERS] = {
        [ HSS_HART_E51 ]   =  0u,
        [ HSS_HART_U54_1 ] =  IHCIA_hart1_INT,
        [ HSS_HART_U54_2 ] =  IHCIA_hart2_INT,
        [ HSS_HART_U54_3 ] =  IHCIA_hart3_INT,
        [ HSS_HART_U54_4 ] =  IHCIA_hart4_INT,
    };

    assert((my_hartid < HSS_HART_NUM_PEERS) && (my_hartid >= 0));

#define MCAUSE64_INT         0x8000000000000000llu
#define MCAUSE64_CAUSE       0x7FFFFFFFFFFFFFFFllu
    volatile uint64_t mcause = csr_read(mcause);
    if (((mcause & MCAUSE64_INT) == MCAUSE64_INT) && ((mcause & MCAUSE64_CAUSE)  == IRQ_M_EXT)) {
        uint32_t my_ihc_interrupt;

        switch (my_hartid) {
        case HSS_HART_U54_1:
            __attribute__((fallthrough)); /* deliberately fallthrough */
        case HSS_HART_U54_2:
            __attribute__((fallthrough)); /* deliberately fallthrough */
        case HSS_HART_U54_3:
            __attribute__((fallthrough)); /* deliberately fallthrough */
        case HSS_HART_U54_4:
            my_ihc_interrupt = hartid_to_ihc_ext_interrupt[my_hartid];

            if (PLIC_pending(my_ihc_interrupt)) {
                int interrupt = PLIC_ClaimIRQ();

                assert(interrupt == my_ihc_interrupt);
                PLIC_CompleteIRQ(my_ihc_interrupt);

                IHC_message_present_poll();
                __sync_synchronize();

                result = true;
            }
            break;

        case HSS_HART_E51:
            __attribute__((fallthrough)); /* deliberately fallthrough */
        default:
            break;
        }
    }
#else
    bool intentFound = false;
    for (int i = 0; i < ARRAY_SIZE(intentsArray); i++) {
        intentFound = IPI_ConsumeIntent(HSS_HART_E51, intentsArray[i].msg_type) | intentFound;
    }

    result = intentFound;
#endif

    // in testing, for harts that aren't in either its domain or the root domain, U-Boot still seems to
    // be generating some unexpected MSIP interrupts which otherwise never get cleared so if not using
    // OpenSBI we could clear explicitly here, but it could affect baremetal SMP...
    // if (
    // #if !IS_ENABLED(CONFIG_HSS_USE_IHC)
    //    intentFound ||
    // #endif
    //    !mpfs_is_hart_using_opensbi(my_hartid)) {
    //    CSR_ClearMSIP();
    // }

    return result;
}


void HSS_U54_Banner(void)
{
    // wait for E51 to setup BSS...
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "u54_%d: Waiting for E51 instruction" CRLF, current_hartid());
}
