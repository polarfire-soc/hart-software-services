/***************************************************************************//**
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * USB MSC Class Storage Device example application to demonstrate the
 * PolarFire MSS USB operations in device mode.
 *
 * Drivers used:
 * PolarFire MSS USB Driver stack (inclusive of USBD-MSC class driver).
 * mss_mmc driver is used to access on board flash SD or eMMC device.
 *
 */

#include "config.h"

// undefine OPENSBI as we want to use MPFS_HAL types in thie module
#undef CONFIG_OPENSBI
#include "mss_plic.h"
#include "mss_clint.h"
#include <stdbool.h>
#include "flash_drive_app.h"

#include "./drivers/mss_usb/mss_usb_device.h"
#include "./drivers/mss_usb/mss_usb_device_msd.h"
#include "hal/hal.h"
#include "mss_hal.h"
#include "mss_mpu.h"
#include "drivers/mss_mmc/mss_mmc.h"

#ifdef __cplusplus
extern "C" {
#endif

int sbi_printf(const char *format, ...);
bool HSS_MMCInit(void);

#define USE_SDMA_OPERATIONS

/******************************************************************************
 *
 * Private data structures
 *
 */

// Number of LUNs supported 1. LUN0
#define NUMBER_OF_LUNS_ON_DRIVE   1u

/* Single block buffer size */
#define LBA_BLOCK_SIZE            512u
#define NUM_LBA_BLOCKS            0xE90E80   // 15273600 => ~7.28GiB
#define ERROR_INTERRUPT           0x8000u
#define TRANSFER_COMPLETE         0x1u
#define SD_RD_WR_SIZE             32768u

uint32_t g_host_connection_detected = 0u;

/*Type to store information of each LUN*/
typedef struct flash_lun_data {
    uint32_t number_of_blocks;
    uint32_t erase_block_size;
    uint32_t lba_block_size;
} flash_lun_data_t;

/******************************************************************************
  Private function declarations
*/
uint8_t* usb_flash_media_inquiry(uint8_t lun, uint32_t *len);
uint8_t usb_flash_media_init (uint8_t lun);
uint8_t usb_flash_media_get_capacity(uint8_t lun, uint32_t *no_of_blocks, uint32_t *block_size);
uint8_t usb_flash_media_is_ready(uint8_t lun);
uint8_t usb_flash_media_is_write_protected(uint8_t lun);
uint32_t usb_flash_media_read(uint8_t lun, uint8_t **buf, uint64_t lba_addr, uint32_t len);
uint8_t* usb_flash_media_acquire_write_buf(uint8_t lun, uint64_t blk_addr, uint32_t *len);
uint32_t usb_flash_media_write_ready(uint8_t lun, uint64_t blk_addr, uint32_t len);
uint8_t usb_flash_media_release(uint8_t cfgidx);

uint8_t usb_flash_media_get_max_lun(void);

/* Implementation of mss_usbd_msc_media_t needed by USB MSD Class Driver*/
static mss_usbd_msc_media_t usb_flash_media = {
    usb_flash_media_init,
    usb_flash_media_get_capacity,
    usb_flash_media_is_ready,
    usb_flash_media_is_write_protected,
    usb_flash_media_read,
    usb_flash_media_acquire_write_buf,
    usb_flash_media_write_ready,
    usb_flash_media_get_max_lun,
    usb_flash_media_inquiry,
    usb_flash_media_release
};

extern mss_usbd_user_descr_cb_t flash_drive_descriptors_cb;

/*This buffer is passed to the USB driver. When USB drivers are configured to
use internal DMA, the address of this buffer must be modulo-4.Otherwise DMA
Transfer will fail.*/

uint8_t  lun0_data_buffer[SD_RD_WR_SIZE] __attribute__((aligned(8))) = { 0u };

flash_lun_data_t lun_data[NUMBER_OF_LUNS_ON_DRIVE] = {{NUM_LBA_BLOCKS, 4096u, LBA_BLOCK_SIZE}};

static mss_usbd_msc_scsi_inq_resp_t usb_flash_media_inquiry_data[NUMBER_OF_LUNS_ON_DRIVE] =
{
    {
        0x00u,                /* peripheral */
        0x80u,                /* removable */
        0x04u,                /* version */
        0x02u,                /* resp_data_format */
        0x20u,                /* additional_length */
        0x00u,                /* sccstp */
        0x00u,                /* bqueetc */
        0x00u,                /* cmd_que */
        "MSCC    ",           /* vendor_id[8] */
        "PolarFireSoC_msd",   /* product_id[16] */
        "1234"                /* product_rev[4] */
    }
};

/******************************************************************************
  See flash_drive_app.h for details of how to use this function.
*/
bool FLASH_DRIVE_init(void)
{
    bool result = true;

    result = HSS_MMCInit();

    g_host_connection_detected = 0u;
    /*Assign call-back function Interface needed by USBD driver*/
    MSS_USBD_set_descr_cb_handler(&flash_drive_descriptors_cb);

    /*Assign call-back function handler structure needed by MSD class driver*/
    MSS_USBD_MSC_init(&usb_flash_media, MSS_USB_DEVICE_HS);

    /*Initialize USB driver*/
    MSS_USBD_init(MSS_USB_DEVICE_HS);

    return result;
}

/******************************************************************************
  Local function definitions
*/

//#define IS_ENABLED defined
#undef MIN
#include "hss_types.h"

#include "hss_clock.h"
static size_t writeCount = 0u, readCount = 0u;
static size_t lastWriteCount = 0u, lastReadCount = 0u;
HSSTicks_t last_sec_time = 0u;

const char throbber[] = { '/', '-', '\\', '|' };
static size_t throbber_iterator = 0u;

void FLASH_DRIVE_dump_xfer_status(void)
{
    static char activeThrobber = '/';

    if (HSS_Timer_IsElapsed(last_sec_time, 5*TICKS_PER_SEC) ||
        ((lastWriteCount == writeCount) && (lastReadCount == readCount))) {
        activeThrobber = '.';
    } else if (HSS_Timer_IsElapsed(last_sec_time, TICKS_PER_SEC)) {
        activeThrobber = throbber[throbber_iterator];
        throbber_iterator++;
        throbber_iterator%= ARRAY_SIZE(throbber);
    }

    if (HSS_Timer_IsElapsed(last_sec_time, TICKS_PER_SEC)) {
        sbi_printf("\r %c %lu bytes written, %lu bytes read", activeThrobber, writeCount, readCount);
        last_sec_time = HSS_GetTime();
    }
}

static void update_write_count(size_t bytes)
{
    writeCount += bytes;
    FLASH_DRIVE_dump_xfer_status();
}

static void update_read_count(size_t bytes)
{
    readCount += bytes;
    FLASH_DRIVE_dump_xfer_status();
}

uint8_t* usb_flash_media_inquiry(uint8_t lun, uint32_t *len)
{
    if (lun != 0u) {
        return 0u;
    }

    *len = sizeof(usb_flash_media_inquiry_data[lun]);
    return ((uint8_t*)&usb_flash_media_inquiry_data[lun]);
}

uint8_t usb_flash_media_release(uint8_t cfgidx)
{
    (void)cfgidx;

    g_host_connection_detected = 0u;
    return 1u;
}

uint8_t usb_flash_media_init(uint8_t lun)
{
    return 1u;
}

uint8_t usb_flash_media_get_max_lun(void)
{
    return NUMBER_OF_LUNS_ON_DRIVE;
}

uint8_t usb_flash_media_get_capacity(uint8_t lun, uint32_t *no_of_blocks, uint32_t *block_size)
{
    uint8_t result;

    if (lun != 0) {
        result = 0u;
    } else {
        *no_of_blocks = lun_data[lun].number_of_blocks;
        *block_size = lun_data[lun].lba_block_size;

        g_host_connection_detected = 1u;
        result = 1u;
    }

    return result;
}

static void physical_device_read(uint64_t byte_address, uint8_t *p_rx_buffer,
    size_t size_in_bytes)
{
    update_read_count(size_in_bytes);
#if 0 // #ifdef USE_SDMA_OPERATIONS
    uint64_t lba_address = (byte_address / 512);
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;

    // wait for any in-flight transactions to complete
    while (MSS_MMC_get_transfer_status() == MSS_MMC_TRANSFER_IN_PROGRESS) {
        do {
            ret_status = mmc_main_plic_IRQHandler();
        } while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS);
    }

    // now setup next read transaction
    ret_status = MSS_MMC_sdma_read((uint32_t)lba_address, p_rx_buffer, size_in_bytes);
    if (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS) {
        do {
            ret_status = mmc_main_plic_IRQHandler();
        } while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS);
    }
#else
    bool HSS_MMC_ReadBlock(void * pDest, size_t srcOffset, size_t byteCount);
    (void) HSS_MMC_ReadBlock((void *)p_rx_buffer, (size_t)byte_address, size_in_bytes);
#endif
}

uint32_t usb_flash_media_read(uint8_t lun, uint8_t **buf, uint64_t lba_addr, uint32_t len)
{
    if (lun == 0) {
        if (len > SD_RD_WR_SIZE) {
            len = SD_RD_WR_SIZE;
        }

        physical_device_read(lba_addr, lun0_data_buffer, len);

        *buf = lun0_data_buffer;
    }

    return len;
}

uint8_t* usb_flash_media_acquire_write_buf(uint8_t lun, uint64_t blk_addr, uint32_t *len)
{
    uint8_t *result = NULL;
    *len = 0u;

    if ((blk_addr <= ((uint64_t)NUM_LBA_BLOCKS * LBA_BLOCK_SIZE)) && (lun == 0u)) {
        *len = SD_RD_WR_SIZE;
        result = lun0_data_buffer;
    }

    return result;
}

static void physical_device_program(uint64_t byte_address, uint8_t * p_write_buffer,
    uint32_t size_in_bytes)
{
    update_write_count(size_in_bytes);

#ifdef USE_SDMA_OPERATIONS
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    uint64_t lba_address = byte_address / LBA_BLOCK_SIZE;

    // wait for any in-flight transactions to complete
    while (MSS_MMC_get_transfer_status() == MSS_MMC_TRANSFER_IN_PROGRESS) {
        do {
            ret_status = mmc_main_plic_IRQHandler();
        } while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS);
    }

    // setup new transaction...
    ret_status = MSS_MMC_sdma_write(p_write_buffer, (uint32_t)lba_address, size_in_bytes);
    if (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS) {
        do {
            ret_status = mmc_main_plic_IRQHandler();
        } while (ret_status == MSS_MMC_TRANSFER_IN_PROGRESS);
    }
#else
    bool HSS_MMC_WriteBlock(size_t dstOffset, void * pSrc, size_t byteCount);
    (void)HSS_MMC_WriteBlock((size_t)byte_address, (void *)p_write_buffer,
        (size_t) size_in_bytes);
#endif
}

uint32_t usb_flash_media_write_ready(uint8_t lun, uint64_t blk_addr, uint32_t len)
{
    uint32_t result = 0u;
    if (lun == 0u) {
        if (len > SD_RD_WR_SIZE) {
            len = SD_RD_WR_SIZE;
        }

        physical_device_program(blk_addr, lun0_data_buffer, len);
        result = 1u;
    }

    return result;

}

uint8_t usb_flash_media_is_ready(uint8_t lun)
{
    (void)lun;
    return 1u;
}

uint8_t usb_flash_media_is_write_protected(uint8_t lun)
{
    (void)lun;
    return 1u;
}

uint32_t FLASH_DRIVE_is_host_connected(void)
{
    return(g_host_connection_detected);
}

#ifdef __cplusplus
}
#endif
