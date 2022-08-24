/*******************************************************************************
 * Copyright 2017-2022 Microchip FPGA Embedded Systems Solutions.
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

#define NONCE_CHUNK_SIZE (32u)
static uint8_t nonce_buffer[NONCE_CHUNK_SIZE];

static int get_random_chunk_(unsigned char *buf, unsigned short len)
{
    uint16_t retVal;
    int result = 0;

    retVal = MSS_SYS_nonce_service(nonce_buffer, 0u);
    if (MSS_SYS_SUCCESS == retVal) {
        memcpy(buf, nonce_buffer, len);
    } else {
        mHSS_FANCY_PRINTF(LOG_ERROR, "Couldn't read random number (%u)\n", retVal);
	result = 1;
    }

    return result;
}


int get_random(unsigned char *buf, unsigned short len);
int get_random(unsigned char *buf, unsigned short len)
{
    int result = 0;
    off_t offset = 0u;

    MSS_SYS_select_service_mode(MSS_SYS_SERVICE_POLLING_MODE, NULL);
    memset(nonce_buffer, 0, ARRAY_SIZE(nonce_buffer));

    while (!result && (len > NONCE_CHUNK_SIZE)) {
        result = get_random_chunk_(&buf[offset], NONCE_CHUNK_SIZE);
        len = len - NONCE_CHUNK_SIZE;
    }

    if (!result && len) {
        result = get_random_chunk_(&buf[offset], len);
    }

    return result;
}
