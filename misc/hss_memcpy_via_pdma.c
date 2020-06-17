/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file  Boot Driver API
 * \brief API functions for booting firmware service
 *
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#include <assert.h>
#include <string.h>

#ifdef CONFIG_USE_PDMA
#include "drivers/mss_pdma/mss_pdma.h"

static char const * const pdmaErrorTable[] = {
  [ MSS_PDMA_ERROR_INVALID_SRC_ADDR ]        = "Invalid source address",
  [ MSS_PDMA_ERROR_INVALID_DEST_ADDR ]       = "Invalid destination address",
  [ MSS_PDMA_ERROR_TRANSACTION_IN_PROGRESS ] = "Transaction in progress",
  [ MSS_PDMA_ERROR_INVALID_CHANNEL_ID ]      = "Invalid Channel ID",
  [ MSS_PDMA_ERROR_INVALID_NEXTCFG_WSIZE ]   = "Invalid Write Size",
  [ MSS_PDMA_ERROR_INVALID_NEXTCFG_RSIZE ]   = "Invalid Read Size",
  [ MSS_PDMA_ERROR_LAST_ID ]                 = "Last ID"
};
#endif

void *memcpy_via_pdma(void *dest, void *src, size_t num_bytes)
{
    void * result;

    // no overlaps allowed!!
    {
        char *cDest = (char *)dest;
        char *cSrc = (char *)src;

        if (cDest > cSrc) {
            assert((cSrc + num_bytes -1) < cDest);
        } else {
            assert((cDest + num_bytes -1) < cSrc);
        }
    }

    //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Copy from %p to %p (%x bytes)" CRLF, src, dest, num_bytes);


#ifdef CONFIG_USE_PDMA
    // num_bytes must be multiple of 16 or more
    if (!(num_bytes & 0xFu)) {
        uint8_t pdma_error_code = 0u;

        mss_pdma_channel_config_t pdma_config_ch0 = {
            .src_addr = (size_t)src,
            .dest_addr = (size_t)dest,
            .num_bytes = num_bytes,
            .enable_done_int = 0,
            .enable_err_int = 0,
            .force_order = 0,
            .repeat = 0u };

        pdma_error_code = MSS_PDMA_setup_transfer(MSS_PDMA_CHANNEL_0, &pdma_config_ch0);
        if (pdma_error_code == 0) {
            pdma_error_code = MSS_PDMA_start_transfer(MSS_PDMA_CHANNEL_0);
            if (pdma_error_code == 0) {
                while (!MSS_PDMA_get_transfer_complete_status(MSS_PDMA_CHANNEL_0)) {
                    ;
                }
                MSS_PDMA_clear_transfer_complete_status(MSS_PDMA_CHANNEL_0);
            }
        }

        if (pdma_error_code != 0) {
            if (pdma_error_code < mSPAN_OF(pdmaErrorTable)) {
                mHSS_DEBUG_PRINTF(LOG_ERROR, "PDMA Error: %s" CRLF, pdmaErrorTable[pdma_error_code]);
            }
            // fall back to traditional memcpy()
            result = memcpy(dest, src, num_bytes);
        } else {
            result = dest;
        }
    } else {
#endif
        // fall back to traditional memcpy()
        result = memcpy(dest, src, num_bytes);
#ifdef CONFIG_USE_PDMA
    }
#endif

    return result;
}
