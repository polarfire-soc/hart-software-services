/*******************************************************************************
 * Copyright 2017-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file Device Serial Number
 * \brief Device Serial Number
 */

#include "config.h"
#include "hss_types.h"
#include <assert.h>

#include "hss_debug.h"
#include "hss_version.h"

#include "csr_helper.h"

#include "device_serial_number.h"
#include "mss_sys_services.h"

/****************************************************************************/

#include <string.h>

static uint8_t serial_num_buffer[50];
static bool result = false;

bool Device_Serial_Number_Init(void)
{
    MSS_SYS_select_service_mode( MSS_SYS_SERVICE_POLLING_MODE, NULL);
    memset(serial_num_buffer, 0, ARRAY_SIZE(serial_num_buffer));
    if (MSS_SYS_SUCCESS == MSS_SYS_get_serial_number(serial_num_buffer, 0u /*mb_offset??*/)) {
        mHSS_FANCY_PRINTF(LOG_STATUS, "Serial Number: " CRLF); // move to boards...
        for (int i = 0; i < ARRAY_SIZE(serial_num_buffer); i++) {
            mHSS_PRINTF("%02x", serial_num_buffer[i]);
        }
        mHSS_PRINTF(CRLF);
        result = true;
    } else {
        mHSS_FANCY_PRINTF(LOG_ERROR, "Couldn't read Serial Number" CRLF);
    }

    return result;
}

bool Get_Device_Serial_Number(uint8_t **ppBuffer, size_t* pLen)
{
    assert(ppBuffer);
    assert(pLen);

    *ppBuffer = &(serial_num_buffer[0]);
    *pLen = ARRAY_SIZE(serial_num_buffer);
    return result;
}
