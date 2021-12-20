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
#include <string.h>
#include <sys/types.h>
#include <mss_sys_services.h>

#include "hss_debug.h"


#include <stdint.h>

static uint8_t nonce_buffer[32];

int get_random(unsigned char *buf, unsigned short len);

int get_random(unsigned char *buf, unsigned short len)
{
    int result = 0;
    uint16_t retVal;
    off_t offset = 0u;

    MSS_SYS_select_service_mode(MSS_SYS_SERVICE_POLLING_MODE, NULL);
    memset(nonce_buffer, 0, ARRAY_SIZE(nonce_buffer));

    while (len > 32u) {
        retVal = MSS_SYS_nonce_service(nonce_buffer, 0u);
        if (MSS_SYS_SUCCESS == retVal) {
            memcpy(&(buf[offset]), nonce_buffer, 32);
	    offset += 32u;
            len -= 32u;
        } else {
            mHSS_FANCY_PRINTF(LOG_ERROR, "Couldn't read Serial Number (%u)" CRLF, retVal);
	    result = 1;
            break;
        }
    }

    if (!result && len) {
        retVal = MSS_SYS_nonce_service(nonce_buffer, 0u);
        if (MSS_SYS_SUCCESS == retVal) {
            memcpy(&(buf[offset]), nonce_buffer, len);
            len = 0u;
        } else {
            mHSS_FANCY_PRINTF(LOG_ERROR, "Couldn't read Serial Number (%u)" CRLF, retVal);
	    result = 1;
        }
    }

    return result;
}
