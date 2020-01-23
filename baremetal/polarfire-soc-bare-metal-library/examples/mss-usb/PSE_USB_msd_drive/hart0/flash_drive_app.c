/***************************************************************************//**
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * USB MSC Class Storage Device example aplication to demonstrate the
 * PolarFire MSS USB operations in device mode.
 *
 * Drivers used:
 * PolarFire MSS USB Driver stack (inclusive of USBD-MSC class driver).
 * MSS_SPI driver is used to access on board flash at25df641.
 *
 * SVN $Revision: $
 * SVN $Date: $
 */
#include "mss_plic.h"
#include "flash_drive_app.h"

#include "./drivers/mss_usb/mss_usb_device.h"
#include "./drivers/mss_usb/mss_usb_device_msd.h"

/* Include the below file based on the SPI Flash present on Board.*/
#include "./drivers/at25df641/at25df641.h"

extern  uint8_t FLASH_Image[];

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
  Private data structures
*/
/*
 Number of LUNs supported 2. LUN0 and LUN1
 When set to 2, two drives of capacity 4Mb each will appear on Host.
 When set to 1, one drive of capacity 8Mb will appear on Host.

 Note: Make sure to re-format the drive if you change this value.
 */

#define NUMBER_OF_LUNS_ON_DRIVE                                1u

#define FLASH_IMAGE

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
uint32_t usb_flash_media_read(uint8_t lun, uint8_t **buf, uint32_t lba_addr, uint32_t len);
uint8_t* usb_flash_media_acquire_write_buf(uint8_t lun, uint32_t blk_addr, uint32_t *len);
uint32_t usb_flash_media_write_ready(uint8_t lun, uint32_t blk_addr, uint32_t len);
uint8_t usb_flash_media_get_max_lun(void);

/* Implementation of mss_usbd_msc_media_t needed by USB MSD Class Driver*/
mss_usbd_msc_media_t usb_flash_media = {
    usb_flash_media_init,
    usb_flash_media_get_capacity,
    usb_flash_media_is_ready,
    usb_flash_media_is_write_protected,
    usb_flash_media_read,
    usb_flash_media_acquire_write_buf,
    usb_flash_media_write_ready,
    usb_flash_media_get_max_lun,
    usb_flash_media_inquiry
};

extern mss_usbd_user_descr_cb_t flash_drive_descriptors_cb;

/*This buffer is passed to the USB driver. When USB drivers are configured to
use internal DMA, the address of this buffer must be modulo-4.Otherwise DMA
Transfer will fail.*/
#if defined(__GNUC__)
uint8_t  lun0_data_buffer[4096] __attribute__ ((aligned (4)))= {0};
#elif defined(__ICCARM__)
#pragma data_alignment = 4
uint8_t  lun0_data_buffer[4096] = {0};
#elif defined(__CC_ARM)
__align(4) uint8_t  lun0_data_buffer[4096] = {0};
#endif

#if defined(__GNUC__)
//uint8_t lun1_data_buffer[4096] __attribute__ ((aligned (4)))= {0};
#ifndef FLASH_IMAGE
    uint8_t  lun1_data_buffer[4096] = {0};
    uint32_t lun1_start_addr = 0x400000;
#else
    uint8_t  lun1_data_buffer[1] = {0};
    uint32_t lun1_start_addr = 0x400000;
#endif


#elif defined(__ICCARM__)
#pragma data_alignment = 4
uint8_t lun1_data_buffer[4096] = {0};
#elif defined(__CC_ARM)
uint8_t lun1_data_buffer[4096] = {0};
#endif


/*
On board flash AT25DF641 is Controlled through MSS_SPI0.
Size of flash = 8MBytes (128 sectors of 64KBytes each)
SCSI protocol block size = 512
Number of SCSI logical blocks = 0x3FFF
Dividing flash into 2 halves for 2 LUNs
*/
#if (NUMBER_OF_LUNS_ON_DRIVE == 2)
flash_lun_data_t lun_data[NUMBER_OF_LUNS_ON_DRIVE] = {{0x1FFFu, 4096u, 512u},
                                                      {0x1FFFu, 4096u, 512u}};
#elif (NUMBER_OF_LUNS_ON_DRIVE == 1)
	#ifndef FLASH_IMAGE
	flash_lun_data_t lun_data[NUMBER_OF_LUNS_ON_DRIVE] = {{0x3FFFu, 4096u, 512u}};
	#else
	flash_lun_data_t lun_data[NUMBER_OF_LUNS_ON_DRIVE] = {{0x6bu, 4096u, 512u}};
	#endif

#endif

#if (NUMBER_OF_LUNS_ON_DRIVE == 2)
mss_usbd_msc_scsi_inq_resp_t usb_flash_media_inquiry_data[NUMBER_OF_LUNS_ON_DRIVE] =
{
    {
        0x00u,                                          /* peripheral */
        0x80u,                                          /* removable */
        0x04u,                                          /* version */
        0x02u,                                          /* resp_data_format */
        0x20u,                                          /* additional_length */
        0x00u,                                          /* sccstp */
        0x00u,                                          /* bqueetc */
        0x00u,                                          /* cmd_que */
        "MSCC    ",                                     /* vendor_id[8] */
        "SmartFusion2_msd",                             /* product_id[16] */
        "1234"                                          /* product_rev[4] */
    }
    ,
    {
        0x00u,                                          /* peripheral */
        0x80u,                                          /* removable */
        0x04u,                                          /* version */
        0x02u,                                          /* resp_data_format */
        0x20u,                                          /* additional_length */
        0x00u,                                          /* sccstp */
        0x00u,                                          /* bqueetc */
        0x00u,                                          /* cmd_que */
        "MSCC    ",                                     /* vendor_id[8] */
        "SmartFusion2_msd",                             /* product_id[16] */
        "1234"                                          /* product_rev[4] */
    }
};
#elif (NUMBER_OF_LUNS_ON_DRIVE == 1)
mss_usbd_msc_scsi_inq_resp_t usb_flash_media_inquiry_data[NUMBER_OF_LUNS_ON_DRIVE] =
{{
    0x00u,                                              /* peripheral */
    0x80u,                                              /* removable */
    0x04u,                                              /* version */
    0x02u,                                              /* resp_data_format */
    0x20u,                                              /* additional_length */
    0x00u,                                              /* sccstp */
    0x00u,                                              /* bqueetc */
    0x00u,                                              /* cmd_que */
    "MSCC    ",                                         /* vendor_id[8] */
    "SmartFusion2_msd",                                 /* product_id[16] */
    "1234"                                              /* product_rev[4] */
}};
#endif

/******************************************************************************
  See flash_drive_app.h for details of how to use this function.
*/
void
FLASH_DRIVE_init
(
    void
)
{
    /*Initialize SPI flash. Initialization of mss_spi driver is taken care by this function.*/
    FLASH_init();
    FLASH_global_unprotect();

    /*Initialize USB driver*/
    MSS_USBD_init(MSS_USB_DEVICE_HS);

    /*Assign call-back function Interface needed by USBD driver*/
    MSS_USBD_set_descr_cb_handler(&flash_drive_descriptors_cb);

    /*Assign call-back function handler structure needed by MSD class driver*/
    MSS_USBD_MSC_init(&usb_flash_media, MSS_USB_DEVICE_HS);

}

/******************************************************************************
  Local function definitions
*/
uint8_t*
usb_flash_media_inquiry
(
    uint8_t lun,
    uint32_t *len
)
{
	if(lun != 0 )
	{
		return 0;
	}
    *len = sizeof(usb_flash_media_inquiry_data[lun]);
    return ((uint8_t*)&usb_flash_media_inquiry_data[lun]);
}

uint8_t
usb_flash_media_init
(
    uint8_t lun
)
{
    return 1;
}

uint8_t
usb_flash_media_get_max_lun
(
    void
)
{
    return NUMBER_OF_LUNS_ON_DRIVE;
}

uint8_t
usb_flash_media_get_capacity
(
    uint8_t lun,
    uint32_t *no_of_blocks,
    uint32_t *block_size
)
{
	if(lun != 0 )
	{
		return 0;
	}
    *no_of_blocks = lun_data[lun].number_of_blocks;
    *block_size = lun_data[lun].lba_block_size;

    return 1;
}

static void FLASH_Image_read
(
    uint32_t address,
    uint8_t * rx_buffer,
    size_t size_in_bytes
)
{
   volatile uint32_t  index;

   uint32_t* p_address = (uint32_t*)&FLASH_Image[address];
   uint32_t* p_rx_buffer  = (uint32_t*)&rx_buffer[0];

   for(index = 0; index < size_in_bytes; index+=4)
   {
        *p_rx_buffer = *p_address;
        p_rx_buffer++;
        p_address++;
   }
}

uint32_t
usb_flash_media_read
(
    uint8_t lun,
    uint8_t **buf,
    uint32_t lba_addr,
    uint32_t len
)
{

    *buf = &FLASH_Image[lba_addr];
    if(len > sizeof(lun0_data_buffer))
    {
        len = 32768;
    }

    return (32768);

}

uint8_t*
usb_flash_media_acquire_write_buf
(
    uint8_t lun,
    uint32_t blk_addr,
    uint32_t *len
)
{
    uint32_t block_start_addr;
    uint32_t block_offset;

    *len = 512;


    if(blk_addr > (NO_OF_512_BLOCKS * 512)) //OverFlow Condition
    {
        return((uint8_t*)0);
    }

    if(lun == 0)
    {
      return &FLASH_Image[blk_addr];
    }
    else
    {
        return((uint8_t*)0);
    }
}

#ifdef FLASH_IMAGE
static void FLASH_Image_program
(
    uint32_t address,
    uint8_t * write_buffer,
    size_t size_in_bytes
)
{
   uint32_t* p_FLASH_Image = (uint32_t*)&FLASH_Image[address];
   uint32_t* p_write_buffer = (uint32_t*)&write_buffer[0];

   for(uint16_t index = 0; index < size_in_bytes; index+=4)
   {
        *p_FLASH_Image = *p_write_buffer;
        p_FLASH_Image++;
        p_write_buffer++;
   }
}
#endif

uint32_t
usb_flash_media_write_ready
(
    uint8_t lun,
    uint32_t blk_addr,
    uint32_t len
)
{

    return 1;
}

uint8_t
usb_flash_media_is_ready
(
    uint8_t lun
)
{
    return 1;
}

uint8_t
usb_flash_media_is_write_protected
(
    uint8_t lun
)
{
    return 1;
}

#ifdef __cplusplus
}
#endif
