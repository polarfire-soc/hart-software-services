/*******************************************************************************
 * Copyright 2017-2022 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file Design Version Info
 * \brief Design Version Info
 */

#include "config.h"
#include "hss_types.h"
#include <assert.h>

#include "hss_debug.h"
#include "hss_version.h"

#include "csr_helper.h"

#include "design_version_info.h"
#include "mss_sys_services.h"

/****************************************************************************/

#include <string.h>

static uint8_t design_info_[76];
bool Design_Version_Info_Init(void)
{
    bool result = false;

    MSS_SYS_select_service_mode( MSS_SYS_SERVICE_POLLING_MODE, NULL);
    memset(design_info_, 0, ARRAY_SIZE(design_info_));

    if (MSS_SYS_SUCCESS == MSS_SYS_get_design_info(design_info_, 0u)) {
        mHSS_FANCY_PRINTF(LOG_STATUS, "DESIGNID: ");
        for (int i = 2; i < 32; i++) {
            mHSS_PUTC(design_info_[i]);
        }
        mHSS_PUTC('\n');

        mHSS_FANCY_PRINTF(LOG_STATUS, "DESIGNVER: ");
        mHSS_FANCY_PRINTF_EX("%02x%02x\n", design_info_[32], design_info_[33]);

        mHSS_FANCY_PRINTF(LOG_STATUS, "BACKLEVEL: ");
        mHSS_FANCY_PRINTF_EX("%02x%02x\n", design_info_[34], design_info_[35]);

        result = true;
    } else {
        mHSS_FANCY_PRINTF(LOG_ERROR, "Couldn't read Design Information\n");
    }

    return result;
}

bool Get_Design_Version_Info(uint8_t **ppBuffer, size_t* pLen)
{
    bool result = false;
    assert(ppBuffer);
    assert(pLen);

    *ppBuffer = &(design_info_[0]);
    *pLen = ARRAY_SIZE(design_info_);
    return result;
}
