/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS Debug UART Initalization
 * \brief Debug UART Initialization
 */

#include "config.h"
#include "hss_types.h"
#include "hss_init.h"
#include "csr_helper.h"

#include <assert.h>

#include "hss_debug.h"
#include "hss_types.h"
#include "ssmb_ipi.h"

#include "mss_plic.h"
#include "miv_ihc.h"

enum {
    IHC_CHAN_ID_H0_H1,
    IHC_CHAN_ID_H0_H2,
    IHC_CHAN_ID_H0_H3,
    IHC_CHAN_ID_H0_H4,
    IHC_CHAN_ID_H0_HX_MAX = IHC_CHAN_ID_H0_H4,
    IHC_CHAN_ID_H1_H0 = 5,
    IHC_CHAN_ID_H2_H0 = 10,
    IHC_CHAN_ID_H3_H0 = 15,
    IHC_CHAN_ID_H4_H0 = 20,
    IHC_CHAN_ID_HX_H0_MAX = IHC_CHAN_ID_H4_H0,
};

#if IS_ENABLED(CONFIG_HSS_USE_IHC)
static uint32_t hss_ihc_incoming_(uint32_t remote_hartid, uint32_t *p_message_in, uint32_t message_size, bool is_ack_required, uint32_t *p_message_storage)
{
    return 0u;
}

static uint32_t u54_ihc_incoming_(uint32_t remote_hartid, uint32_t *p_message_in, uint32_t message_size, bool is_ack_required, uint32_t *p_message_storage)
{
    (void)remote_hartid;
    (void)message_size;
    (void)is_ack_required;
    (void)p_message_storage;

    assert(p_message_in);

    bool HSS_U54_ConsumeIntent(enum IPIMessagesEnum msg_type);
    HSS_U54_ConsumeIntent((enum IPIMessagesEnum)p_message_in[0]);

    return 0u;
}
#endif

bool HSS_IHCInit(void)
{
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Initializing Mi-V IHC\n");

    for (uint32_t chan_id = IHC_CHAN_ID_H0_H1; chan_id <= IHC_CHAN_ID_H0_HX_MAX; chan_id++) {
        IHC_init(chan_id);
#if IS_ENABLED(CONFIG_HSS_USE_IHC)
        IHC_config_mp_callback_handler(ch, hss_ihc_incoming_);
#endif
    }

    for (uint32_t chan_id = IHC_CHAN_ID_H1_H0; chan_id <= IHC_CHAN_ID_HX_H0_MAX; chan_id+=5) {
        IHC_init(chan_id);
#if IS_ENABLED(CONFIG_HSS_USE_IHC)
        IHC_config_mp_callback_handler(chan_id, u54_ihc_incoming_);
        IHC_enable_mp_interrupt(chan_id);
#endif
    }

    return true;
}

void HSS_IHCInit_U54(void)
{
#if IS_ENABLED(CONFIG_HSS_USE_IHC)
    const enum HSSHartId local_hartid = current_hartid();

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Initializing PLIC (Mi-V IHC) for u54_%d\n", local_hartid);

    // enable PLIC interrupt for IHC
    // the E51 will be calling PLIC_init_on_reset(), so we'll delay here to allow that to complete...
    // 50mS is sufficient, and won't impact overall boot time as U54s are waiting for E51 anyway...
    HSS_SpinDelay_MilliSecs(50u);
    PLIC_init();

    __extension__ const PLIC_IRQn_Type ihcia_hart_to_int_table[] = {
        [ HSS_HART_E51]   = 0,
        [ HSS_HART_U54_1] = IHCIA_hart1_INT,
        [ HSS_HART_U54_2] = IHCIA_hart2_INT,
        [ HSS_HART_U54_3] = IHCIA_hart3_INT,
        [ HSS_HART_U54_4] = IHCIA_hart4_INT,
    };

    switch(local_hartid) {
    case HSS_HART_U54_1:
        __attribute__((fallthrough)); /* deliberately fallthrough */
    case HSS_HART_U54_2:
        __attribute__((fallthrough)); /* deliberately fallthrough */
    case HSS_HART_U54_3:
        __attribute__((fallthrough)); /* deliberately fallthrough */
    case HSS_HART_U54_4:
        PLIC_SetPriority(ihcia_hart_to_int_table[local_hartid], 7);
        PLIC_EnableIRQ(ihcia_hart_to_int_table[local_hartid]);
        break;

    case HSS_HART_E51:
        __attribute__((fallthrough)); /* deliberately fallthrough */
    default:
        break;
    }
#endif
}
