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
#  include "qspi_service.h"
#  include <mss_qspi.h>
#endif

#include "hss_state_machine.h"
#include "hss_debug.h"
#include "hss_crc32.h"

#include <string.h>
#include <assert.h>

#ifdef CONFIG_COMPRESSION
#  include "hss_decompress.h"
#endif

#include "hss_memcpy_via_pdma.h"
#include "hss_boot_pmp.h"

static bool validateCrc_(struct HSS_BootImage *pImage)
{
    bool result = false;
    uint32_t headerCrc, originalCrc;
    struct HSS_BootImage header = *pImage;

    originalCrc = header.headerCrc;
    header.headerCrc = 0u;

    headerCrc = CRC32_calculate((const uint8_t *)&header, sizeof(struct HSS_BootImage)); 

    if (headerCrc == originalCrc) {
        result = true;
    }

    return result;
}

bool HSS_BootInit(void)
{
    bool result = false;
    mHSS_DEBUG_PRINTF("\tChecking memory..." CRLF);

    mHSS_DEBUG_PRINTF("\tInitializing Permissions..." CRLF);
    HSS_Setup_PLIC();
    HSS_Setup_MPU();
    HSS_PMP_Init();

    mHSS_DEBUG_PRINTF("\tInitializing Boot Image.." CRLF);

#ifdef CONFIG_SERVICE_BOOT
#  ifdef CONFIG_SERVICE_QSPI
    struct HSS_BootImage *pBootImage = (struct HSS_BootImage *)QSPI_BASE;
#  else
    // assuming that boot image is statically linked with the HSS ELF
    // Note: this method is deprecated and will be disappearing soon!
    //
#      if defined(CONFIG_COMPRESSION)
    extern const char _binary_bootImageBlob_bin_lz77_start;
    struct HSS_BootImage *pBootImage = (struct HSS_BootImage *)&_binary_bootImageBlob_bin_lz77_start;
    mHSS_DEBUG_PRINTF("\tpBootImage is %p, magic is %x" CRLF, pBootImage, pBootImage->magic);
#      else
    extern const char _binary_bootImageBlob_bin_start;
    struct HSS_BootImage *pBootImage = (struct HSS_BootImage *)&_binary_bootImageBlob_bin_start;
#      endif
#  endif

    if (!pBootImage) {
        mHSS_DEBUG_PRINTF("\tBoot Image NULL, ignoring" CRLF);
        result = false;
    } else {
#  if defined(CONFIG_COMPRESSION)
        mHSS_DEBUG_PRINTF("\tPreparing to decompress to DDR..." CRLF);
        void* const pInput = (void*)pBootImage;
        void * const pOutputInDDR = (void *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR);

        int outputSize = HSS_Decompress(pInput, pOutputInDDR);
        mHSS_DEBUG_PRINTF("\tdecompressed %d bytes..." CRLF, outputSize);

        if (outputSize) {
            pBootImage = (struct HSS_BootImage *)pOutputInDDR;
        } else {
            pBootImage = NULL;
        }
#  elif defined(CONFIG_SERVICE_QSPI) && defined(CONFIG_SERVICE_QSPI_COPY_TO_DDR)
        mHSS_DEBUG_PRINTF("\tPreparing to copy from QSPI to DDR..." CRLF);
        // code to copy from QSPI base to DDR to go here...

        // set pDestImageInDDR to an appropriate location in DDR
        void *pDestImageInDDR = (void *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR);

        // TODO: add validation routine to quickly validate boot image header 
        // before a needless copy is performed
        //if (pBootImage->magic != HSS_BOOT_MAGIC) { // causes problems w. RENODE

        mHSS_DEBUG_PRINTF("\tCopying %lu bytes from 0x%X to 0x%X" CRLF, 
            pBootImage->bootImageLength, QSPI_BASE, pDestImageInDDR);

        const size_t maxChunkSize = 4096u * 256u;
        size_t bytesLeft = pBootImage->bootImageLength;
        size_t chunkSize = mMIN(pBootImage->bootImageLength, maxChunkSize);
        char *pSrc = (void *)QSPI_BASE;
        char *pDest = pDestImageInDDR;

        const char throbber[] = { '|', '/', '-', '\\' };
        unsigned int state = 0u;
        while (bytesLeft) {
            state++; state %= 4;
            mHSS_DEBUG_PRINTF_EX("%c %lu bytes (%lu remain) from 0x%X to 0x%X\r", 
                throbber[state], chunkSize, bytesLeft, (void *)pSrc, pDest);

    	    memcpy_via_pdma(pDest, pSrc, chunkSize);

            pSrc += chunkSize;
            pDest += chunkSize;
            bytesLeft -= chunkSize;

            chunkSize = mMIN(bytesLeft, maxChunkSize);
        }
        mHSS_DEBUG_PRINTF_EX("                                                                               \r");
        pBootImage = (struct HSS_BootImage *)pDestImageInDDR;
#  endif
 
        if (!pBootImage) {
            mHSS_DEBUG_PRINTF("\tBoot Image NULL, ignoring" CRLF);
            result = false;
        } else if (validateCrc_(pBootImage)) {
            mHSS_DEBUG_PRINTF("\tdecompressed boot image passed CRC" CRLF);

            mHSS_DEBUG_PRINTF("Boot image set name: \"%s\"" CRLF, pBootImage->set_name);
            HSS_Register_Boot_Image(pBootImage); 
            mHSS_DEBUG_PRINTF("\tBoot Image registered..." CRLF);

            HSS_Boot_RestartCore(HSS_HART_ALL);
            result = true;
        } else {
            mHSS_DEBUG_PRINTF("\tdecompressed boot image failed CRC" CRLF);
        }
    }
#endif

    return result;
}
