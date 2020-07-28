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

#include "hss_boot_service.h"
#include "hss_boot_init.h"
#include "hss_sys_setup.h"
#include "hss_progress.h"

#ifdef CONFIG_SERVICE_OPENSBI
#  include "opensbi_service.h"
#endif

#ifdef CONFIG_SERVICE_QSPI
//#  include "encoding.h"
//#  include <mss_qspi.h>
#  include "qspi_service.h"
#endif

#ifdef CONFIG_SERVICE_MMC
#  include "mmc_service.h"
#  include "gpt.h"
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


static inline bool verifyMagic_(struct HSS_BootImage const * const pBootImage);

#ifndef CONFIG_SERVICE_BOOT_USE_PAYLOAD
typedef bool (*HSS_BootImageCopyFnPtr_t)(void *pDest, size_t srcOffset, size_t byteCount);
static bool copyBootImageToDDR_(struct HSS_BootImage *pBootImage, char *pDest,
    size_t srcOffset, HSS_BootImageCopyFnPtr_t pCopyFunction);
#endif

#if defined(CONFIG_SERVICE_QSPI)
static bool getBootImageFromQSPI_(struct HSS_BootImage **ppBootImage);
#endif
#if defined(CONFIG_SERVICE_MMC)
static bool getBootImageFromMMC_(struct HSS_BootImage **ppBootImage);
#endif
#if defined(CONFIG_SERVICE_BOOT_USE_PAYLOAD)
static bool getBootImageFromPayload_(struct HSS_BootImage **ppBootImage);
#endif

static void printBootImageDetails_(struct HSS_BootImage const * const pBootImage);
static bool validateCrc_(struct HSS_BootImage *pImage);

//
//

typedef bool (*HSS_GetBootImageFnPtr_t)(struct HSS_BootImage **ppBootImage);

static HSS_GetBootImageFnPtr_t getBootImageFunction =
#if defined(CONFIG_SERVICE_MMC)
    getBootImageFromMMC_;
#elif defined(CONFIG_SERVICE_QSPI)
    getBootImageFromQSPI_;
#elif defined(CONFIG_SERVICE_BOOT_USE_PAYLOAD)
    getBootImageFromPayload_;
#  else
#    error Unable to determine boot mechanism
#endif

bool HSS_BootInit(void)
{
    bool result = false;
    bool decompressedFlag = false;
    struct HSS_BootImage *pBootImage = NULL;

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Initializing Boot Image.." CRLF);

#ifdef CONFIG_SERVICE_BOOT
    result = getBootImageFunction(&pBootImage);
    //
    // check if this image is compressed...
    // if so, decompress it to DDR
    //
    // for now, compression only works with a source already in DDR or XIP-QSPI
#  if defined(CONFIG_COMPRESSION)
    if (result && pBootImage->magic == mHSS_COMPRESSED_MAGIC) {
        decompressedFlag = true;
        if (!result) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "Failed to get boot image, cannot decompress" CRLF);
        } else if (!pBootImage) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "Boot Image NULL, ignoring" CRLF);
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
            mHSS_DEBUG_PRINTF(LOG_ERROR, "Boot Image NULL, ignoring" CRLF);
            result = false;
        } else if (pBootImage->magic != mHSS_BOOT_MAGIC) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "Boot Image magic invalid, ignoring" CRLF);
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
            mHSS_DEBUG_PRINTF(LOG_ERROR, "%s boot image failed CRC" CRLF,
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
    } else {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "Checked HSS_BootImage header CRC (%p->%p): calculated %08x vs expected %08x" CRLF, pImageHdr, (char *)pImageHdr + sizeof(struct HSS_BootImage), headerCrc, originalCrc);
    }

    // restore original headerCrc
    pImageHdr->headerCrc = originalCrc;

    return result;
}

static void printBootImageDetails_(struct HSS_BootImage const * const pBootImage)
{
#ifdef BOOT_DEBUG
    mHSS_DEBUG_PRINTF(LOG_NORMAL, " - set name is >>%s<<" CRLF, pBootImage->set_name);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, " - magic is    %08X" CRLF, pBootImage->magic);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, " - length is   %08X" CRLF, pBootImage->bootImageLength);
#endif
}

#ifndef CONFIG_SERVICE_BOOT_USE_PAYLOAD
static bool copyBootImageToDDR_(struct HSS_BootImage *pBootImage, char *pDest,
    size_t srcOffset, HSS_BootImageCopyFnPtr_t pCopyFunction)
{
    bool result = true;
    char *pOrigDest = pDest;

    printBootImageDetails_(pBootImage);

    // TODO: quickly validate boot image header before a needless copy is
    // performed
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Copying %lu bytes to 0x%X" CRLF,
        pBootImage->bootImageLength, pDest);

    const size_t maxChunkSize = 512u;
    size_t bytesLeft = pBootImage->bootImageLength;
    size_t chunkSize = mMIN(pBootImage->bootImageLength, maxChunkSize);

#ifdef DEBUG_COPY
    const char throbber[] = { '|', '/', '-', '\\' };
    unsigned int state = 0u;
#endif
    while (bytesLeft) {
#ifdef DEBUG_COPY
        state++; state %= 4;
        mHSS_DEBUG_PRINTF_EX("%c %lu bytes (%lu remain) to 0x%X\r",
            throbber[state], chunkSize, bytesLeft, pDest);
#else
        HSS_ShowProgress(pBootImage->bootImageLength, bytesLeft);
#endif

        result = pCopyFunction(pDest, srcOffset, chunkSize);

         if (!result) { break; }

        srcOffset += chunkSize;
        pDest += chunkSize;
        bytesLeft -= chunkSize;

        chunkSize = mMIN(bytesLeft, maxChunkSize);
    }

#ifdef DEBUG_COPY
    // clear copy output to console by printing an empty string
    mHSS_DEBUG_PRINTF_EX("                                                                      \r");
#else
    HSS_ShowProgress(pBootImage->bootImageLength, 0u);
#endif

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Calculated CRC32 of image in DDR is %08x" CRLF,
        CRC32_calculate((const uint8_t *)pOrigDest, pBootImage->bootImageLength));

    return result;
}
#endif

static inline bool verifyMagic_(struct HSS_BootImage const * const pBootImage)
{
    bool result = false;

    if ((pBootImage->magic == mHSS_BOOT_MAGIC) || (pBootImage->magic == mHSS_COMPRESSED_MAGIC)) {
        result = true;
    } else {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "magic is %08x vs expected %08x or %08x" CRLF,
            pBootImage->magic, mHSS_BOOT_MAGIC, mHSS_COMPRESSED_MAGIC);
    }

    return result;
}

#if defined(CONFIG_SERVICE_MMC) || defined(CONFIG_SERVICE_QSPI)
struct HSS_BootImage bootImage __attribute__((aligned(8)));
#endif

#ifdef CONFIG_SERVICE_MMC
static bool getBootImageFromMMC_(struct HSS_BootImage **ppBootImage)
{
    bool result = true;

    assert(ppBootImage);

    // if we are using MMC, then we need to do an initial copy of the
    // boot header into our structure, for subsequent use
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Preparing to copy from MMC to DDR ..." CRLF);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Attempting to read image header (%d bytes) ..." CRLF,
        sizeof(struct HSS_BootImage));

    size_t srcOffset = 0u;

#ifdef CONFIG_SERVICE_BOOT_MMC_USE_GPT
    // For now, GPT needs a 3-sector buffer (~1.5KB) - one for GPT header,
    // and two for partition entity
    char lbaBuffer[3][GPT_LBA_SIZE] __attribute__((aligned(8)));
    HSS_GPT_Header_t *pGptHeader = (HSS_GPT_Header_t *)lbaBuffer[0];

    GPT_RegisterReadBlockFunction(HSS_MMC_ReadBlock);

    result = GPT_ReadHeader(pGptHeader);

    if (result) {
        result = GPT_ValidateHeader(pGptHeader);

        if (result) {
            result = GPT_ValidatePartitionEntries(pGptHeader, (uint8_t *)lbaBuffer[1]);

            if (!result) {
             mHSS_DEBUG_PRINTF(LOG_ERROR, "GPT_ValidatePartitionEntries() failed" CRLF);
            }
        }
    }

    if (result) {
        const HSS_GPT_GUID_t diskGUID = {
            .data1 = 0x21686148u,
            .data2 = 0x6449u,
            .data3 = 0x6E6Fu,
            .data4 = 0x4946456465654e74u
        };

        size_t firstLBA, lastLBA;
        result = GPT_FindPartitionByTypeId(pGptHeader, &diskGUID, (uint8_t *)lbaBuffer[1],
            &firstLBA, &lastLBA);

        if (!result) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "GPT_FindPartitionByUniqueId() failed" CRLF);
        } else {
            srcOffset = firstLBA;
        }
    }
#endif
    //
    // Even if we have GPT enabled and it fails to find a GPT parttion, we'll still
    // try to boot
    /*if (result)*/ {
        result = HSS_MMC_ReadBlock(&bootImage, srcOffset * GPT_LBA_SIZE,
            sizeof(struct HSS_BootImage));

        if (!result) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "HSS_MMC_ReadBlock() failed" CRLF);
        } else {
            result = verifyMagic_(&bootImage);

            if (!result) {
                mHSS_DEBUG_PRINTF(LOG_ERROR, "verifyMagic_() failed" CRLF);
            } else {
                result = copyBootImageToDDR_(&bootImage,
                    (char *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR), srcOffset * GPT_LBA_SIZE,
                    HSS_MMC_ReadBlock);
                *ppBootImage = (struct HSS_BootImage *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR);

               if (!result) {
                   mHSS_DEBUG_PRINTF(LOG_ERROR, "copyBootImageToDDR_() failed" CRLF);
               }
            }
        }
    }

    return result;
}

void HSS_BootSelectMMC(void)
{
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Selecting MMC as boot source ..." CRLF);
    getBootImageFunction = getBootImageFromMMC_;
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

void HSS_BootSelectQSPI(void)
{
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Selecting QSPI as boot source ..." CRLF);
    getBootImageFunction = getBootImageFromQSPI_;
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

void HSS_BootSelectPayload(void)
{
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Selecting Payload as boot source ..." CRLF);
    getBootImageFunction = getBootImageFromPayload_;
}
#endif
