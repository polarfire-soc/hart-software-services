/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
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

#include "hss_init.h"
#include "hss_boot_service.h"
#include "hss_boot_init.h"
#include "hss_sys_setup.h"
#include "hss_progress.h"

#if IS_ENABLED(CONFIG_SERVICE_SPI)
#  include <mss_sys_services.h>
#  define SPI_FLASH_BOOT_ENABLED (CONFIG_SERVICE_BOOT_SPI_FLASH_OFFSET != 0xFFFFFFFF)
#else
#  define SPI_FLASH_BOOT_ENABLED 0
#endif

#if IS_ENABLED(CONFIG_SERVICE_OPENSBI)
#  include "opensbi_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_QSPI)
#  include "qspi_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_MMC)
#  include "mmc_service.h"
#  include "gpt.h"
#endif

#if (SPI_FLASH_BOOT_ENABLED)
#  include "mss_sys_services.h"
#endif

#include "hss_state_machine.h"
#include "hss_debug.h"
#include "hss_perfctr.h"

#include <string.h>
#include <assert.h>

#if IS_ENABLED(CONFIG_COMPRESSION)
#  include "hss_decompress.h"
#endif

#if IS_ENABLED(CONFIG_CRYPTO_SIGNING)
#  include "hss_boot_secure.h"
#endif

#include "hss_boot_pmp.h"
#include "hss_atomic.h"

//
// local module functions

#if IS_ENABLED(CONFIG_SERVICE_BOOT)
#  if !IS_ENABLED(CONFIG_SERVICE_BOOT_USE_PAYLOAD)
typedef bool (*HSS_BootImageCopyFnPtr_t)(void *pDest, size_t srcOffset, size_t byteCount);
static bool copyBootImageToDDR_(struct HSS_BootImage *pBootImage, char *pDest,
    size_t srcOffset, HSS_BootImageCopyFnPtr_t pCopyFunction);
#  endif

static void printBootImageDetails_(struct HSS_BootImage const * const pBootImage);
static bool tryBootFunction_(struct HSS_Storage *pStorage, HSS_GetBootImageFnPtr_t getBootImageFunction);
#endif

static bool getBootImageFromQSPI_(struct HSS_Storage *pStorage, struct HSS_BootImage **ppBootImage);
static bool getBootImageFromMMC_(struct HSS_Storage *pStorage, struct HSS_BootImage **ppBootImage);
static bool getBootImageFromSpiFlash_(struct HSS_Storage *pStorage, struct HSS_BootImage **ppBootImage);
static bool getBootImageFromPayload_(struct HSS_Storage *pStorage, struct HSS_BootImage **ppBootImage);


//
//

#if IS_ENABLED(CONFIG_SERVICE_QSPI)
static struct HSS_Storage qspiStorage_ = {
    .name = "QSPI",
    .getBootImage = getBootImageFromQSPI_,
    .init = HSS_CachedQSPIInit,
    .readBlock = HSS_CachedQSPI_ReadBlock,
    .writeBlock = HSS_CachedQSPI_WriteBlock,
    .getInfo = HSS_CachedQSPI_GetInfo,
    .flushWriteBuffer = HSS_CachedQSPI_FlushWriteBuffer
};
#endif
#if IS_ENABLED(CONFIG_SERVICE_MMC)
static struct HSS_Storage mmcStorage_ = {
    .name = "MMC",
    .getBootImage = getBootImageFromMMC_,
    .init = HSS_MMCInit,
    .readBlock = HSS_MMC_ReadBlock,
    .writeBlock = HSS_MMC_WriteBlockSDMA,
    .getInfo = HSS_MMC_GetInfo,
    .flushWriteBuffer = NULL
};
#endif
#if IS_ENABLED(CONFIG_SERVICE_SPI)
static struct HSS_Storage spiStorage_ = {
    .name = "SPI",
    .getBootImage = getBootImageFromSpiFlash_,
    .init = NULL,
    .readBlock = NULL,
    .writeBlock = NULL,
    .getInfo = NULL,
    .flushWriteBuffer = NULL
};
#endif
#if IS_ENABLED(CONFIG_SERVICE_BOOT_USE_PAYLOAD)
static struct HSS_Storage payloadStorage_ = {
    .name = "Payload",
    .getBootImage = getBootImageFromPayload_,
    .init = NULL,
    .readBlock = NULL,
    .writeBlock = NULL,
    .getInfo = NULL,
    .flushWriteBuffer = NULL
};
#endif

static struct HSS_Storage *pStorages[] =
{
#if IS_ENABLED(CONFIG_SERVICE_QSPI)
	&qspiStorage_,
#endif
#if IS_ENABLED(CONFIG_SERVICE_SPI)
	&spiStorage_,
#endif
#if IS_ENABLED(CONFIG_SERVICE_MMC)
	&mmcStorage_,
#endif
#if IS_ENABLED(CONFIG_SERVICE_BOOT_USE_PAYLOAD)
	&payloadStorage_,
#endif
};

static struct HSS_Storage *pDefaultStorage = NULL;

#if IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_QSPI) || (IS_ENABLED(CONFIG_SERVICE_SPI) && (SPI_FLASH_BOOT_ENABLED))
struct HSS_BootImage bootImage __attribute__((aligned(8)));
#elif IS_ENABLED(CONFIG_SERVICE_BOOT_USE_PAYLOAD)
//
#else
#    error Unable to determine boot mechanism
#endif

struct HSS_Storage *HSS_BootGetActiveStorage(void);
struct HSS_Storage *HSS_BootGetActiveStorage(void)
{
    struct HSS_Storage *pResult = pDefaultStorage;

    if (!pResult) {
        pResult = pStorages[0];
    }

    return pResult;
}

void HSS_BootListStorageProviders(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(pStorages); i++) {
        mHSS_DEBUG_PRINTF_EX(" - %s\n", pStorages[i]->name);
    }
}

bool HSS_BootInit(void)
{
    bool result = true;
#if IS_ENABLED(CONFIG_SERVICE_BOOT)

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Initializing Boot Image ...\n");

    int perf_ctr_index = PERF_CTR_UNINITIALIZED;
    HSS_PerfCtr_Allocate(&perf_ctr_index, "Boot Image Init");

    if (pDefaultStorage) {
        result = pDefaultStorage->init();
        if (result) {
            result = tryBootFunction_(pDefaultStorage, pDefaultStorage->getBootImage);
        }
    } else {
        for (int i = 0; i < ARRAY_SIZE(pStorages); i++) {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Trying to boot via %s ...\n", pStorages[i]->name);

            if (pStorages[i]->init) {
                result = pStorages[i]->init();
            } else {
                result = true;
            }

            if (result) {
                result = tryBootFunction_(pStorages[i], pStorages[i]->getBootImage);
                if (result) { break; }
            }
        }
    }

    HSS_PerfCtr_Lap(perf_ctr_index);
#endif

    return result;
}

#if IS_ENABLED(CONFIG_SERVICE_BOOT)
bool tryBootFunction_(struct HSS_Storage *pStorage, HSS_GetBootImageFnPtr_t const bootImageFunction)
{
    bool result = false;
    (void)pStorage;

    struct HSS_BootImage *pBootImage = NULL;
    bool decompressedFlag = false;

    (void)decompressedFlag;


    result = bootImageFunction(pStorage, &pBootImage);
    //
    // check if this image is compressed...
    // if so, decompress it to DDR
    //
    // for now, compression only works with a source already in DDR
#  if IS_ENABLED(CONFIG_COMPRESSION)
    if (result && pBootImage->magic == mHSS_COMPRESSED_MAGIC) {
        decompressedFlag = true;
        if (!result) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "Failed to get boot image, cannot decompress\n");
        } else if (!pBootImage) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "Boot Image NULL, ignoring\n");
            result = false;
        } else {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Preparing to decompress to DDR ...\n");
            void* const pInput = (void*)pBootImage;
            void * const pOutputInDDR = (void *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR);

            int outputSize = HSS_Decompress(pInput, pOutputInDDR);
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "decompressed %d bytes ...\n", outputSize);

            if (outputSize) {
                pBootImage = (struct HSS_BootImage *)pOutputInDDR;
            } else {
                pBootImage = NULL;
            }
        }
    }
#  endif

    if (result) {
        HSS_Register_Boot_Image(pBootImage);
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s: Boot Image registered ...\n", pStorage->name);
    }

    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void printBootImageDetails_(struct HSS_BootImage const * const pBootImage)
{
#  ifdef BOOT_DEBUG
    mHSS_DEBUG_PRINTF(LOG_NORMAL, " - set name is >>%s<<\n", pBootImage->set_name);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, " - magic is    %08x\n", pBootImage->magic);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, " - length is   %08x\n", pBootImage->bootImageLength);
#  endif
}
#endif

#if IS_ENABLED(CONFIG_SERVICE_BOOT) && !IS_ENABLED(CONFIG_SERVICE_BOOT_USE_PAYLOAD)
static bool copyBootImageToDDR_(struct HSS_BootImage *pBootImage, char *pDest,
    size_t srcOffset, HSS_BootImageCopyFnPtr_t pCopyFunction)
{
    bool result = true;

    printBootImageDetails_(pBootImage);

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Copying %lu bytes to 0x%lx\n",
        pBootImage->bootImageLength, pDest);
    result = pCopyFunction(pDest, srcOffset, pBootImage->bootImageLength);

    return result;
}
#endif

static bool getBootImageFromMMC_(struct HSS_Storage *pStorage, struct HSS_BootImage **ppBootImage)
{
    bool result = false;

#if IS_ENABLED(CONFIG_SERVICE_BOOT) && IS_ENABLED(CONFIG_SERVICE_MMC)
    assert(ppBootImage);

    // if we are using MMC, then we need to do an initial copy of the
    // boot header into our structure, for subsequent use
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Preparing to copy from MMC to DDR ...\n");

    size_t srcLBAOffset = 0u;
    assert(pStorage);

    uint32_t blockSize, eraseSize, blockCount;
    pStorage->getInfo(&blockSize, &eraseSize, &blockCount);

# if (IS_ENABLED(CONFIG_SERVICE_BOOT_MMC_USE_GPT))
    {
        HSS_GPT_t gpt;

        gpt.lbaSize = blockSize;
        GPT_Init(&gpt, pStorage);
        result = GPT_ReadHeader(&gpt);

        if (result) {
            size_t srcIndex = 0u;

            if (GPT_GetBootPartitionIndex(&gpt, &srcIndex)) {
                mHSS_DEBUG_PRINTF(LOG_WARN, "Using manually set partition index\n");
            } else {
                result = GPT_FindBootSectorIndex(&gpt, &srcIndex, NULL);

                if (!result) {
                    mHSS_DEBUG_PRINTF(LOG_ERROR, "GPT_FindBootSectorIndex() failed\n");
                } else {
                    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Boot Partition found at index %lu\n",
                        srcIndex);
                }
            }

            if (result) {
                result = GPT_PartitionIdToLBAOffset(&gpt, srcIndex, &srcLBAOffset);
            }
        }
    }

    if (!result) {
        mHSS_DEBUG_PRINTF(LOG_WARN, "GPT_PartitionIdToLBAOffset() failed - using offset %lu\n", srcLBAOffset);
    } else {
        //mHSS_DEBUG_PRINTF(LOG_WARN, "GPT_PartitionIdToLBAOffset() returned %lu\n", srcLBAOffset);
    }
#endif

    //
    // Even if we have GPT enabled and it fails to find a GPT parttion, we'll still
    // try to boot
    {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Attempting to read image header (%d bytes) ...\n",
            sizeof(struct HSS_BootImage));
        result = HSS_MMC_ReadBlock(&bootImage, srcLBAOffset * blockSize,
            sizeof(struct HSS_BootImage));

        if (!result) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "HSS_MMC_ReadBlock() failed\n");
        } else {
            result = HSS_Boot_VerifyMagic(&bootImage);

            if (!result) {
                mHSS_DEBUG_PRINTF(LOG_ERROR, "HSS_Boot_VerifyMagic() failed\n");
            } else {
                int perf_ctr_index = PERF_CTR_UNINITIALIZED;
                HSS_PerfCtr_Allocate(&perf_ctr_index, "Boot Image MMC Copy");

                result = copyBootImageToDDR_(&bootImage,
                    (char *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR), srcLBAOffset * blockSize,
                    HSS_MMC_ReadBlock);
                *ppBootImage = (struct HSS_BootImage *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR);

                HSS_PerfCtr_Lap(perf_ctr_index);

                if (!result) {
                     mHSS_DEBUG_PRINTF(LOG_ERROR, "copyBootImageToDDR_() failed\n");
                }
            }
        }
    }
#endif

    return result;
}

void HSS_BootSelectSDCARD(void)
{
#if IS_ENABLED(CONFIG_SERVICE_MMC)
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Selecting SDCARD as boot source ...\n");
    pDefaultStorage = &mmcStorage_;
    HSS_MMC_SelectSDCARD();
#else
    (void)getBootImageFromMMC_;
#endif
}

void HSS_BootSelectMMC(void)
{
#if IS_ENABLED(CONFIG_SERVICE_MMC)
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Selecting SDCARD/MMC (fallback) as boot source ...\n");
    pDefaultStorage = &mmcStorage_;
    HSS_MMC_SelectMMC();
#else
    (void)getBootImageFromMMC_;
#endif
}

void HSS_BootSelectEMMC(void)
{
#if IS_ENABLED(CONFIG_SERVICE_MMC)
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Selecting EMMC as boot source ...\n");
    pDefaultStorage = &mmcStorage_;
    HSS_MMC_SelectEMMC();
#else
    (void)getBootImageFromMMC_;
#endif
}

static bool getBootImageFromQSPI_(struct HSS_Storage *pStorage, struct HSS_BootImage **ppBootImage)
{
    bool result = false;

#if IS_ENABLED(CONFIG_SERVICE_BOOT) && IS_ENABLED(CONFIG_SERVICE_QSPI)
    assert(ppBootImage);

    // need to do an initial copy of the boot header into our structure, for subsequent use
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Preparing to copy from QSPI to DDR ...\n");

    size_t srcLBAOffset = 0u;
    assert(pStorage);

    uint32_t blockSize, eraseSize, blockCount;
    pStorage->getInfo(&blockSize, &eraseSize, &blockCount);

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Attempting to read image header (%d bytes) ...\n",
        sizeof(struct HSS_BootImage));
    result = HSS_QSPI_ReadBlock(&bootImage, srcLBAOffset * blockSize,
        sizeof(struct HSS_BootImage));
    if (!result) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "HSS_QSPI_ReadBlock() failed\n");
    } else {
        result = HSS_Boot_VerifyMagic(&bootImage);

        if (!result) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "HSS_Boot_VerifyMagic() failed\n");
        } else {
            int perf_ctr_index = PERF_CTR_UNINITIALIZED;
            HSS_PerfCtr_Allocate(&perf_ctr_index, "Boot Image QSPI Copy");

            result = copyBootImageToDDR_(&bootImage,
                (char *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR), srcLBAOffset * blockSize,
                HSS_QSPI_ReadBlock);
            *ppBootImage = (struct HSS_BootImage *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR);

            HSS_PerfCtr_Lap(perf_ctr_index);

            if (!result) {
                 mHSS_DEBUG_PRINTF(LOG_ERROR, "copyBootImageToDDR_() failed\n");
            }
        }
    }
#endif

    return result;
}

void HSS_BootSelectQSPI(void)
{
#if IS_ENABLED(CONFIG_SERVICE_QSPI)
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Selecting QSPI as boot source ...\n");
    pDefaultStorage = &qspiStorage_;
#else
    (void)getBootImageFromQSPI_;
#endif
}

static bool getBootImageFromPayload_(struct HSS_Storage *pStorage, struct HSS_BootImage **ppBootImage)
{
    bool result = false;
    (void)pStorage;

#if IS_ENABLED(CONFIG_SERVICE_BOOT) && IS_ENABLED(CONFIG_SERVICE_BOOT_USE_PAYLOAD)
    assert(ppBootImage);

    extern struct HSS_BootImage _payload_start;
    *ppBootImage = (struct HSS_BootImage *)&_payload_start;

    result = HSS_Boot_VerifyMagic(*ppBootImage);
    printBootImageDetails_(*ppBootImage);
#endif

    return result;
}

void HSS_BootSelectPayload(void)
{
#if IS_ENABLED(CONFIG_SERVICE_USE_PAYLOAD)
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Selecting Payload as boot source ...\n");
    pDefaultStorage = &payloadStorage_;
#else
    (void)getBootImageFromPayload_;
#endif
}

#if IS_ENABLED(CONFIG_SERVICE_BOOT) && IS_ENABLED(CONFIG_SERVICE_SPI)
static bool spiFlashReadBlock_(void *dst, size_t offs, size_t count) {
   int retval = MSS_SYS_spi_copy((uintptr_t)dst, offs, count, /* options */ 3, /* mb_offset */ 0);

   if (retval) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "Failed to read 0x%lx bytes from SPI flash @0x%lx (error code %d)!\n", count, offs, retval);
   }

   return (retval == 0);
}
#endif

static bool getBootImageFromSpiFlash_(struct HSS_Storage *pStorage, struct HSS_BootImage **ppBootImage) {
    bool result = false;
    (void)pStorage;

#if IS_ENABLED(CONFIG_SERVICE_BOOT) && IS_ENABLED(CONFIG_SERVICE_SPI)

    assert(ppBootImage);

    size_t srcOffset = CONFIG_SERVICE_BOOT_SPI_FLASH_OFFSET;

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Preparing to copy from SPI Flash +0x%lx to DDR ...\n", srcOffset);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Attempting to read image header (%d bytes) ...\n",
        sizeof(struct HSS_BootImage));

    MSS_SYS_select_service_mode(MSS_SYS_SERVICE_POLLING_MODE, NULL);

    result = spiFlashReadBlock_(&bootImage, srcOffset, sizeof(struct HSS_BootImage));
    if (!result) {
        return false;
    }

    result = HSS_Boot_VerifyMagic(&bootImage);
    if (!result) {
        return false;
    }

    result = copyBootImageToDDR_(&bootImage, (char *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR),
        srcOffset, spiFlashReadBlock_);
    *ppBootImage = (struct HSS_BootImage *)(CONFIG_SERVICE_BOOT_DDR_TARGET_ADDR);
#endif

    return result;
}

void HSS_BootSelectSPI(void)
{
#if IS_ENABLED(CONFIG_SERVICE_SPI)
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Selecting SPI Flash as boot source ...\n");
    pDefaultStorage = &spiStorage_;
#else
    (void)getBootImageFromSpiFlash_;
#endif
}

bool HSS_Storage_Init(void);
bool HSS_Storage_ReadBlock(void *pDest, size_t srcOffset, size_t byteCount);
bool HSS_Storage_WriteBlock(size_t dstOffset, void *pSrc, size_t byteCount);
void HSS_Storage_GetInfo(uint32_t *pBlockSize, uint32_t *pEraseSize, uint32_t *pBlockCount);
void HSS_Storage_FlushWriteBuffer(void);

bool HSS_Storage_Init(void)
{
    bool result = true;

    struct HSS_Storage *pStorage = pDefaultStorage ? pDefaultStorage : pStorages[0];
    assert(pStorage);

    if (pStorage->init) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "initialize %s\n", pStorage->name);
        result = pStorage->init();
    }

    return result;
}

bool HSS_Storage_ReadBlock(void *pDest, size_t srcOffset, size_t byteCount)
{
    bool result = true;

    struct HSS_Storage *pStorage = pDefaultStorage ? pDefaultStorage : pStorages[0];
    assert(pStorage);

    if (pStorage->readBlock) {
        result = pStorage->readBlock(pDest, srcOffset, byteCount);
    }
    return result;
}

bool HSS_Storage_WriteBlock(size_t dstOffset, void *pSrc, size_t byteCount)
{
    struct HSS_Storage *pStorage = pDefaultStorage ? pDefaultStorage : pStorages[0];
    assert(pStorage);

    return pStorage->writeBlock(dstOffset, pSrc, byteCount);
}

void HSS_Storage_GetInfo(uint32_t *pBlockSize, uint32_t *pEraseSize, uint32_t *pBlockCount)
{
    struct HSS_Storage *pStorage = pDefaultStorage ? pDefaultStorage : pStorages[0];
    assert(pStorage);

    if (pStorage->getInfo) {
        pStorage->getInfo(pBlockSize, pEraseSize, pBlockCount);
    }
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s - %u byte pages, %u byte blocks, %u pages\n", pStorage->name, *pBlockSize, *pEraseSize, *pBlockCount);
}

void HSS_Storage_FlushWriteBuffer(void)
{
    struct HSS_Storage *pStorage = pDefaultStorage ? pDefaultStorage : pStorages[0];
    assert(pStorage);

    if (pStorage->flushWriteBuffer) {
        pStorage->flushWriteBuffer();
    }
}
