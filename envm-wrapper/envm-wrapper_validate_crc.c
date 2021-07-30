/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Implementation of validate_crc() for envm-wrapper.
 */

#include "hss_types.h"

#include <assert.h>

#include "hss_debug.h"

#include <mss_uart.h>
#include <string.h>
#include <stdint.h>

#include "uart_helper.h"
#include "hss_sys_setup.h"
#include "hss_crc32.h"

void validate_crc(void);
void validate_crc(void)
{
    extern const struct HSS_CompressedImage hss_l2lim_lz;
    extern const unsigned char __l2lim_start;
    uint32_t crc32 = CRC32_calculate(&__l2lim_start, hss_l2lim_lz.originalImageLen);

    if (hss_l2lim_lz.originalCrc == crc32) {
    } else {
        while (1) { ; } // spin forever
    }
}
