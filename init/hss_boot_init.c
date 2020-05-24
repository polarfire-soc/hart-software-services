/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS Boot Initalization
 * \brief Boot Initialization
 */

#include "config.h"
#include "hss_types.h"

#ifdef CONFIG_SERVICE_BOOT
#  include "hss_boot_service.h"
#endif
#  include "hss_sys_setup.h"

#ifdef CONFIG_SERVICE_OPENSBI
#  include "opensbi_service.h"
#endif

#ifdef CONFIG_SERVICE_QSPI
//#  include "encoding.h"
#  include "qspi_service.h"
//#  include <mss_qspi.h>
#endif

#ifdef CONFIG_SERVICE_EMMC
#  include "emmc_service.h"
#endif

#include "hss_state_machine.h"
#include "hss_debug.h"
#include "hss_crc32.h"

#include <string.h>
#include <assert.h>

#ifdef CONFIG_COMPRESSION
#  include "hss_decompress.h"
#endif

#include "hss_boot_pmp.h"

//
// local module functions
#ifndef CONFIG_SERVICE_BOOT_USE_PAYLOAD
static bool copyBootImageToDDR_(struct HSS_BootImage *pBootImage, char *pDest,
    size_t srcOffset,
    bool (*pCopyFunction)(void *pDest, size_t srcOffset, size_t byteCount));
#endif
static inline bool verifyMagic_(struct HSS_BootImage const * const pBootImage);
#if defined(CONFIG_SERVICE_QSPI)
static bool getBootImageFromQSPI_(struct HSS_BootImage **ppBootImage);
#endif
#if defined(CONFIG_SERVICE_EMMC)
static bool getBootImageFromEMMC_(struct HSS_BootImage **ppBootImage);
#endif
#if defined(CONFIG_SERVICE_BOOT_USE_PAYLOAD)
static bool getBootImageFromPayload_(struct HSS_BootImage **ppBootImage);
#endif
static void printBootImageDetails_(struct HSS_BootImage const * const pBootImage);
static bool validateCrc_(struct HSS_BootImage *pImage);

//
//

bool HSS_BootInit(void)
{
    bool result = false;
    bool decompressedFlag = false;
    struct HSS_BootImage *pBootImage = NULL;

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Initializing Boot Image.." CRLF);

#ifdef CONFIG_SERVICE_BOOT
#  if defined(CONFIG_SERVICE_QSPI)
    result = getBootImageFromQSPI_(&pBootImage);
#  elif defined(CONFIG_SERVICE_EMMC)
    result = getBootImageFromEMMC_(&pBootImage);
#  elif defined(CONFIG_SERVICE_BOOT_USE_PAYLOAD)
    result = getBootImageFromPayload_(&pBootImage);
#  else
#    error Unable to determine boot mechanism
#  endif

    //
    // check if this image is compressed...
    // if so, decompress it to DDR
    //
    // for now, compression only works with a source already in DDR or XIP-QSPI
#  if defined(CONFIG_COMPRESSION)
    if (result && pBootImage->magic = mHSS_COMPRESSED_MAGIC) {
        decompressedFlag = true;
        if (!result) {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Failed to get boot image, cannot decompress" CRLF);
        } else if (!pBootImage) {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Boot Image NULL, ignoring" CRLF);
            result = false;
        } else {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Preparing to decompress to DDR..." CRLF);
            void* const pInput = (void*)pBootImage;
            void * const pOutputInDDR = (void *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR);

            int outputSize = HSS_Decompress(pInput, pOutputInDDR);
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "decompressed %d bytes..." CRLF, outputSize);

            if (outputSize) {
                pBootImage = (struct HSS_BootImage *)pOutputInDDR;
            } else {
                pBootImage = NULL;
            }
        }
    }
#  endif

    //
    // now have a Boot Image, let's check it is a valid one...
    //
    {
        if (!pBootImage) {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Boot Image NULL, ignoring" CRLF);
            result = false;
        } else if (pBootImage->magic != mHSS_BOOT_MAGIC) {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Boot Image magic invalid, ignoring" CRLF);
            result = false;
        } else if (validateCrc_(pBootImage)) {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s boot image passed CRC" CRLF,
                decompressedFlag ? "decompressed":"");

        // GCC 9.x appears to dislike the pBootImage cast, and sees dereferincing the
        // set name as an out-of-bounds... So we'll disable that warning just for
        // this print...
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Boot image set name: \"%s\"" CRLF, pBootImage->set_name);
#pragma GCC diagnostic pop
            HSS_Register_Boot_Image(pBootImage);
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Boot Image registered..." CRLF);

            if (HSS_Boot_RestartCore(HSS_HART_ALL) == IPI_SUCCESS) {
                result = true;
	    } else {
                result = false;
	    }
        } else {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s boot image failed CRC" CRLF,
                decompressedFlag ? "decompressed":"");
        }
    }
#endif

    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool validateCrc_(struct HSS_BootImage *pImageHdr)
{
    bool result = false;
    uint32_t headerCrc, originalCrc;

    originalCrc = pImageHdr->headerCrc;
    pImageHdr->headerCrc = 0u;

    headerCrc = CRC32_calculate((const uint8_t *)pImageHdr, sizeof(struct HSS_BootImage));

    if (headerCrc == originalCrc) {
        result = true;
    }

    // restore original headerCrc
    pImageHdr->headerCrc = originalCrc;

    return result;
}

static void printBootImageDetails_(struct HSS_BootImage const * const pBootImage)
{
    mHSS_DEBUG_PRINTF(LOG_NORMAL, " - set name is >>%s<<" CRLF, pBootImage->set_name);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, " - magic is    %08X" CRLF, pBootImage->magic);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, " - length is   %08X" CRLF, pBootImage->bootImageLength);
}

#ifndef CONFIG_SERVICE_BOOT_USE_PAYLOAD
static bool copyBootImageToDDR_(struct HSS_BootImage *pBootImage, char *pDest,
    size_t srcOffset,
    bool (*pCopyFunction)(void *pDest, size_t srcOffset, size_t count))
{
    bool result = true;

    printBootImageDetails_(pBootImage);

    // TODO: quickly validate boot image header before a needless copy is
    // performed
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Copying %lu bytes to 0x%X" CRLF,
        pBootImage->bootImageLength, pDest);

    const size_t maxChunkSize = 512u;
    size_t bytesLeft = pBootImage->bootImageLength;
    size_t chunkSize = mMIN(pBootImage->bootImageLength, maxChunkSize);

    const char throbber[] = { '|', '/', '-', '\\' };
    unsigned int state = 0u;
    while (bytesLeft) {
        state++; state %= 4;
        mHSS_DEBUG_PRINTF_EX("%c %lu bytes (%lu remain) to 0x%X\r",
            throbber[state], chunkSize, bytesLeft, pDest);

        result = pCopyFunction(pDest, srcOffset, chunkSize);

         if (!result) { break; }

        srcOffset += chunkSize;
        pDest += chunkSize;
        bytesLeft -= chunkSize;

        chunkSize = mMIN(bytesLeft, maxChunkSize);
    }

    // clear copy output to console by printing an empty string
    mHSS_DEBUG_PRINTF_EX("                                                                      \r");

    return result;
}
#endif

static inline bool verifyMagic_(struct HSS_BootImage const * const pBootImage)
{
    bool result = false;

    if ((pBootImage->magic == mHSS_BOOT_MAGIC) || (pBootImage->magic == mHSS_COMPRESSED_MAGIC)) {
        result = true;
    }

    return result;
}

#if defined(CONFIG_SERVICE_EMMC) || defined(CONFIG_SERVICE_QSPI)
struct HSS_BootImage bootImage __attribute__((aligned(8)));
#endif

#ifdef CONFIG_SERVICE_EMMC
static bool getBootImageFromEMMC_(struct HSS_BootImage **ppBootImage)
{
    bool result = false;

    assert(ppBootImage);

    // if we are using EMMC, then we need to do an initial copy of the
    // boot header into our structure, for subsequent use
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Preparing to copy from EMMC to DDR ..." CRLF);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Attempting to read image header (%d bytes) ..." CRLF,
        sizeof(struct HSS_BootImage));

    size_t srcOffset = 0u; // assuming zero as sector/block offset for now
    HSS_EMMC_ReadBlock(&bootImage, srcOffset, sizeof(struct HSS_BootImage));

    result = verifyMagic_(&bootImage);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, " after verify" CRLF);

    if (result) {
        result = copyBootImageToDDR_(&bootImage,
            (char *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR),
            srcOffset, HSS_EMMC_ReadBlock);
        *ppBootImage = (struct HSS_BootImage *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR);
    }

    return result;
}
#endif

#ifdef CONFIG_SERVICE_QSPI
static bool getBootImageFromQSPI_(struct HSS_BootImage **ppBootImage)
{
    bool result = false;

    assert(ppBootImage);

#  ifndef CONFIG_SERVICE_QSPI_USE_XIP
    // if we are not using XIP, then we need to do an initial copy of the
    // boot header into our structure, for subsequent use
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Preparing to copy from QSPI to DDR ..." CRLF);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Attempting to read image header (%d bytes) ..." CRLF,
        sizeof(struct HSS_BootImage));

    size_t srcOffset = 0u; // assuming zero as sector/block offset for now
    HSS_QSPI_ReadBlock(&bootImage, srcOffset, sizeof(struct HSS_BootImage));

    result = verifyMagic_(&bootImage);

    if (result) {
        result = copyBootImageToDDR_(&bootImage,
            (char *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR),
            srcOffset, HSS_QSPI_ReadBlock);
        *ppBootImage = (struct HSS_BootImage *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR);
    }

#  else
    *ppBootImage = (struct HSS_BootImage *)QSPI_BASE;
    result = verifyMagic_(**ppBootImage);
#  endif

    return result;
}
#endif

#ifdef CONFIG_SERVICE_BOOT_USE_PAYLOAD
static bool getBootImageFromPayload_(struct HSS_BootImage **ppBootImage)
{
    bool result = false;

    assert(ppBootImage);

    extern struct HSS_BootImage _payload_start;
    *ppBootImage = (struct HSS_BootImage *)&_payload_start;

    result = verifyMagic_(*ppBootImage);
    printBootImageDetails_(*ppBootImage);

    return result;
}
#endif
