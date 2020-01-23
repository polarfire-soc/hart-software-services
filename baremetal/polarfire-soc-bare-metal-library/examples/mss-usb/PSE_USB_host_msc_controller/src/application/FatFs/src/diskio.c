/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "drivers/mss_usb/mss_usb_host_msc.h"

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
/*-----------------------------------------------------------------------*/

#define USB		0
#define MMC		1
#define ATA		2

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
    return 0;   //success
}

/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	return 0;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
    if(0u != drv)
        return(RES_ERROR);

    if(USBH_MSC_DEVICE_READY < MSS_USBH_MSC_get_state())
    {
        return(RES_NOTRDY);
    }
    else if(USBH_MSC_DEVICE_READY == MSS_USBH_MSC_get_state())
    {
        if(0== MSS_USBH_MSC_read(buff, sector, count))
        {
            while(MSS_USBH_MSC_is_scsi_req_complete());
            return(RES_OK);
        }
        else
            return(RES_ERROR);
    }
    else
        return(RES_ERROR);
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
/* The FatFs module will issue multiple sector transfer request
/  (count > 1) to the disk I/O layer. The disk function should process
/  the multiple sector transfer properly Do. not translate it into
/  multiple single sector transfers to the media, or the data read/write
/  performance may be drasticaly decreased. */

#if _READONLY == 0
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
    if(0u != pdrv)
        return(RES_ERROR);

    if(USBH_MSC_DEVICE_READY < MSS_USBH_MSC_get_state())
    {
        return(RES_NOTRDY);
    }
    else if(USBH_MSC_DEVICE_READY == MSS_USBH_MSC_get_state())
    {
        if(0 == MSS_USBH_MSC_write((uint8_t*)buff, sector, count))
        {
            while(MSS_USBH_MSC_is_scsi_req_complete());
            return(RES_OK);
        }
        else
            return(RES_ERROR);
    }
    else
        return(RES_ERROR);
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
uint32_t sect_size1;
uint32_t sect_size2;

/*
 Number of Drives(Logical Units) is always 1.
 _DRIVES must be eualt to 1 in ffconfig.h
 */

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	UINT *result = (UINT *)buff;
    if(0u != pdrv)
        return(RES_ERROR);

	switch (ctrl) {
    case CTRL_SYNC:
    	break;
    case CTRL_POWER:
    	break;
    case CTRL_LOCK:
    	break;
    case CTRL_EJECT:
    	break;
    case GET_SECTOR_COUNT:
        if(USBH_MSC_DEVICE_READY == MSS_USBH_MSC_get_state())
        {
            *result = MSS_USBH_MSC_get_sector_count();
            return RES_OK;
        }
        else if(USBH_MSC_DEVICE_READY < MSS_USBH_MSC_get_state())
        {
            *result = 0u;
            return RES_NOTRDY;
        }
        else
        {
            *result = 0u;
            return RES_ERROR;
        }

    case GET_SECTOR_SIZE:
        if(USBH_MSC_DEVICE_READY == MSS_USBH_MSC_get_state())
        {
            *result = MSS_USBH_MSC_get_sector_size();
            return RES_OK;
        }
        else if(USBH_MSC_DEVICE_READY < MSS_USBH_MSC_get_state())
        {
            *result = 0u;
            return RES_NOTRDY;
        }
        else
        {
            *result = 0u;
            return RES_ERROR;
        }

    case GET_BLOCK_SIZE:
    	*result = 512;/*Erase Block Size */
    	break;
    default:
    	break;
    }
	return (DRESULT)0;

}
