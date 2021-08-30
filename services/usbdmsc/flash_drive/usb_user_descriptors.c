/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * USB MSC Class Storage Device example application to demonstrate the
 * PolarFire MSS USB operations in USB Device mode.
 *
 * This file provides the Device Descriptor for the implemented USB Device.
 * This file implements Application call-back Interface structure type provided
 * by USBD driver.
 *
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#include "./drivers/mss_usb/mss_usb_device.h"
#include "drivers/mss_usb/mss_usb_std_def.h"

#include "device_serial_number.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Constant definitions
 */
#define  USB_MAX_STRING_DESCRIPTOR_SIZE                     64u

/* String Descriptor Indexes */
#define  USB_STRING_DESCRIPTOR_IDX_LANGID                   0x00
#define  USB_STRING_DESCRIPTOR_IDX_MANUFACTURER             0x01
#define  USB_STRING_DESCRIPTOR_IDX_PRODUCT                  0x02
#define  USB_STRING_DESCRIPTOR_IDX_SERIAL                   0x03
#define  USB_STRING_DESCRIPTOR_IDX_CONFIG                   0x04
#define  USB_STRING_DESCRIPTOR_IDX_INTERFACE                0x05

#define  USB_STRING_MANUFACTURER                            "Microchip Inc"
#define  USB_STRING_PRODUCT                                 "PolarFireSoc-FlashDrive"
#define  USB_STRING_SERIAL                                  "123456789ABCDEF151411111"
#define  USB_STRING_CONFIG                                  "CFG-HS"
#define  USB_STRING_INTERFACE                               "Interface-MSD"

/*******************************************************************************
 * Local functions.
 */
uint8_t* flash_drive_device_descriptor(uint32_t* length);
uint8_t* flash_drive_device_qual_descriptor(mss_usb_device_speed_t speed, uint32_t* length);
uint8_t* flash_drive_string_descriptor(uint8_t index, uint32_t* length);
uint8_t flash_drive_get_string(uint8_t const * const string, uint8_t* dest);

/***************************************************************************//**
  Device descriptor.
 */
uint8_t device_descriptor[USB_STD_DEVICE_DESCR_LEN] = {
    USB_STD_DEVICE_DESCR_LEN,                           /* bLength */
    USB_DEVICE_DESCRIPTOR_TYPE,                         /* bDescriptorType */
    0x00u,                                              /* bcdUSB LSB */
    0x02u,                                              /* bcdUSB MSB */
    0x00u,                                              /* bDeviceClass */
    0x00u,                                              /* bDeviceSubClass */
    0x00u,                                              /* bDeviceProtocol */
    0x40u,                                              /* bMaxPacketSize0 */
    0x14u,                                              /* idVendor LSB */
    0x15u,                                              /* idVendor MSB */
    0x01u,                                              /* idProduct LSB */
    0x00u,                                              /* idProduct MSB */
    0x00u,                                              /* bcdDevice LSB */
    0x30u,                                              /* bcdDevice MSB */
    USB_STRING_DESCRIPTOR_IDX_MANUFACTURER,             /* iManufacturer */
    USB_STRING_DESCRIPTOR_IDX_PRODUCT,                  /* iProduct */
    USB_STRING_DESCRIPTOR_IDX_SERIAL,                   /* iSerialNumber */
    0x01u                                               /* bNumConfigurations */
};

/***************************************************************************//**
  Device qualifiers.
 */
uint8_t hs_dev_qualifier_descriptor[USB_STD_DEV_QUAL_DESCR_LENGTH] = {
    USB_STD_DEV_QUAL_DESCR_LENGTH,                      /* bLength */
    USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE,               /* bDescriptorType */
    0x00u,                                              /* bcdUSB LSB */
    0x02u,                                              /* bcdUSB MSB */
    0x00u,                                              /* bDeviceClass */
    0x00u,                                              /* bDeviceSubClass */
    0x00u,                                              /* bDeviceProtocol */
    0x40u,                                              /* bMaxPacketSize0 */
    0x01u,                                              /* bNumConfigurations */
    0x00u                                               /* Reserved */
};

uint8_t fs_dev_qualifier_descriptor[USB_STD_DEV_QUAL_DESCR_LENGTH] = {
    USB_STD_DEV_QUAL_DESCR_LENGTH,                      /* bLength */
    USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE,               /* bDescriptorType */
    0x00u,                                              /* bcdUSB LSB */
    0x02u,                                              /* bcdUSB MSB */
    0x00u,                                              /* bDeviceClass */
    0x00u,                                              /* bDeviceSubClass */
    0x00u,                                              /* bDeviceProtocol */
    0x40u,                                              /* bMaxPacketSize0 */
    0x01u,                                              /* bNumConfigurations */
    0x00u                                               /* Reserved */
};

uint8_t lang_string_descriptor[] = {
    0x04u,                                              /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,                         /* bDescriptorType */
    0x09u,                                              /* LangID-LSB */
    0x04u                                               /* LangID-MSB */
};

mss_usbd_user_descr_cb_t flash_drive_descriptors_cb = {
    flash_drive_device_descriptor,
    flash_drive_device_qual_descriptor,
    flash_drive_string_descriptor,
};

uint8_t g_string_descriptor[USB_MAX_STRING_DESCRIPTOR_SIZE];

uint8_t* flash_drive_device_descriptor(uint32_t* length)
{
   *length = sizeof(device_descriptor);
   return (device_descriptor);
}

uint8_t* flash_drive_device_qual_descriptor(mss_usb_device_speed_t speed,
    uint32_t* length)
{
    if (speed == MSS_USB_DEVICE_HS) {
        *length = sizeof(fs_dev_qualifier_descriptor);
         return (fs_dev_qualifier_descriptor);
    } else {
        *length = sizeof(hs_dev_qualifier_descriptor);
         return (hs_dev_qualifier_descriptor);
    }
}

static uint8_t nibble_to_hexchar_(uint8_t nibble)
{
    uint8_t result;

    switch (nibble) {
    case 0u ... 9u:
        result = '0' + nibble;
        break;

    case 10u ... 15u:
        result = 'A' + (nibble - 10u);
        break;

    default:
        result = '?';
        break;
    }

    return result;
}

static uint8_t* get_device_serial_(void)
{
    //
    // for CI test purposes, we'll use the device serial number to give each
    // board a unique USBDMSC iSerial number...
    static uint8_t string_device_serial[USB_MAX_STRING_DESCRIPTOR_SIZE] = "";
    static uint8_t* result = NULL;

    if (!result) {
        uint8_t* p_device_number_buffer;
        size_t serial_length;

        if (Get_Device_Serial_Number(&p_device_number_buffer, &serial_length)) {
            const size_t limit = MIN(MIN(USB_MAX_STRING_DESCRIPTOR_SIZE/2, serial_length), sizeof(USB_STRING_SERIAL)/2);

            for (size_t i = 0u; i < limit; i++) {
                string_device_serial[i*2u] = nibble_to_hexchar_(p_device_number_buffer[i] >> 4);
                string_device_serial[(i*2u) + 1u] = nibble_to_hexchar_(p_device_number_buffer[i] & 0xF);
            }

            result = string_device_serial;
        } else {
	    result = (uint8_t*)USB_STRING_SERIAL;
        }
    }

    return result;
}
uint8_t* flash_drive_string_descriptor(uint8_t index, uint32_t* length)
{
    switch (index) {
        case USB_STRING_DESCRIPTOR_IDX_LANGID:
            *length = sizeof(lang_string_descriptor);
            break;

        case USB_STRING_DESCRIPTOR_IDX_MANUFACTURER:
            *length = flash_drive_get_string((uint8_t*)USB_STRING_MANUFACTURER, g_string_descriptor);
            break;

        case USB_STRING_DESCRIPTOR_IDX_PRODUCT:
            *length = flash_drive_get_string((uint8_t*)USB_STRING_PRODUCT, g_string_descriptor);
            break;

        case USB_STRING_DESCRIPTOR_IDX_SERIAL:
            *length = flash_drive_get_string(get_device_serial_(), g_string_descriptor);
            break;

        case USB_STRING_DESCRIPTOR_IDX_CONFIG:
            *length = flash_drive_get_string((uint8_t*)USB_STRING_CONFIG, g_string_descriptor);
            break;

        case USB_STRING_DESCRIPTOR_IDX_INTERFACE:
            *length = flash_drive_get_string((uint8_t*)USB_STRING_INTERFACE, g_string_descriptor);
            break;

        default:
            /*Raise error*/
            *length = 0;
            break;
    }

    if (USB_STRING_DESCRIPTOR_IDX_LANGID == index) {
        return (lang_string_descriptor);
    }

    return (g_string_descriptor);
}

uint8_t flash_drive_get_string(uint8_t const * const src_string, uint8_t* dest)
{
    const uint8_t *idx = src_string ;
    uint8_t *cp_dest;

    cp_dest = dest;

    if ((dest != NULL) && (src_string != NULL)) {
        for (; *(idx); ++idx) {
            *(dest + 2u) = *(idx);
            dest++;

            *(dest + 2u) = 0x00u;
            dest++;
        }
        *cp_dest = ((idx - src_string) * 2u) + 2u;               /*bLength*/
        *(cp_dest + 1u) = USB_STRING_DESCRIPTOR_TYPE;        /*bDesriptorType*/
    }

    return (((idx - src_string) * 2u) + 2u);
}

#ifdef __cplusplus
}
#endif
