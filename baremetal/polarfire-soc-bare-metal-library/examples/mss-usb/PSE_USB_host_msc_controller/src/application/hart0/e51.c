/***************************************************************************//**
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Code running on e51
 *
 * USB MSC Class Host controller example application to demonstrate the
 * PolarFire SoC MSS USB operations in USB Host mode.
 *
 * This project can detect "Memory Stick" category devices and perform file
 * operations on it. To know more about how to use this project please refer
 * README.TXT in this project's folder.
 *
 * SVN $Revision$
 * SVN $Date$
 */

#include <string.h>
#include <stdio.h>
#include "mpfs_hal/mss_hal.h"
#include "drivers/mss_uart/mss_uart.h"
#include "drivers/mss_usb/mss_usb_host.h"
#include "drivers/mss_usb/mss_usb_std_def.h"
#include "mpfs_hal/mss_mpu.h"
#include "ff.h"
#include "../FatFs/src/diskio.h"
#include "drivers/mss_usb/mss_usb_host.h"
#include "drivers/mss_usb/mss_usb_host_msc.h"
#include "drivers/mss_usb/mss_usb_host.h"
#include "drivers/mss_usb/mss_usb_std_def.h"
#include "drivers/mss_uart/mss_uart.h"

/* The buffer size determines the USB read/write transfers.
 * For maximum file/read write performance, the buffer size must be maximum 
 * possible.Due to limited RAM in the debug mode,the buffer size is selected
 * as 4096.
 * In Release mode of this project you may want to increase this value to 
 * extract faster disk read/write performance.
 */
#define BUFFER_SIZE                                     4096
#define MAX_ELEMENT_COUNT                               20u
#define ESC_KEY                                         27u
#define ENTER_KEY                                       13u

FATFS fs;
FRESULT res;
FIL fsrc;
FIL fdest;

/* This buffer is passed to the USB driver. When USB drivers are configured to
 * use internal DMA, the address of this buffer must be modulo-4.Otherwise DMA
 * Transfer will fail.
 */
uint8_t buffer[BUFFER_SIZE] __attribute__ ((aligned (4))) = {0u};
char* file_name[MAX_ELEMENT_COUNT][13] __attribute__ ((aligned (4))) = {{0}};
uint8_t new_fname[13u] __attribute__ ((aligned (4)));
uint8_t g_dev_descr[18] __attribute__ ((aligned (4)))= {0};
uint64_t uart_lock;
char path[] = "0:";

BYTE file_attrib[MAX_ELEMENT_COUNT] = {0};
uint8_t g_copy_file_index = 0;

/* Start node to be scanned (also used as work area) */
FRESULT scan_root_dir (char* path);
FRESULT read_root_dir_file (uint8_t index);
FRESULT copy_root_dir_file (uint8_t index, uint8_t character);

static void display_main_menu(void);
static void display_test_mode_menu(void);
static void goto_main_menu(void);
static void read_file_menu(void);
static void invalid_selection_menu(void);
static void copy_file_menu(void);
static void get_device_descriptor(void);

/* Global variables */
uint8_t volatile g_hid_driver_ready = 0u;
uint8_t volatile g_hid_driver_released = 0u;

void USB_DEV_attached(mss_usb_device_speed_t speed);
void USB_DEV_dettached(void);
void USB_DEV_oc_error(void);
void USB_DEV_enumerated(mss_usbh_tdev_info_t tdev_info);
void USB_DEV_class_driver_assigned(void);
void USB_DEV_not_supported(int8_t error_code);
void USB_DEV_permanent_erro(void);

void MSC_DEV_valid_config(void);
void MSC_DEV_ready(void);
void MSC_DEV_driver_released(void);
void MSC_DEV_error(int8_t error_code);

uint8_t hex_to_char(uint8_t nibble_value)
{
    uint8_t hex_char = '0';

    if (nibble_value < 10u)
    {
        hex_char = nibble_value + '0';
    }
    else if (nibble_value < 16u)
    {
        hex_char = nibble_value + 'A' - 10;
    }

    return hex_char;
}

void display_hex_byte(uint8_t hex_byte)
{
    uint8_t hex_value_msg[2];
    uint8_t upper;
    uint8_t lower;

    upper = (hex_byte >> 4u) & 0x0Fu;
    lower = hex_byte & 0x0Fu;
    hex_value_msg[0] = hex_to_char(upper);
    hex_value_msg[1] = hex_to_char(lower);
    MSS_UART_polled_tx(&g_mss_uart0_lo, hex_value_msg, sizeof(hex_value_msg));
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t *)" ");
}

volatile uint32_t g_cnt=0;
#define SYS_TICK_LOAD_VALUE                             25000u  /*For 1ms*/
/**
 *
 */
 mss_usbh_user_cb_t MSS_USBH_user_cb =
{
    USB_DEV_attached,
    USB_DEV_dettached,
    USB_DEV_oc_error,
    USB_DEV_enumerated,
    USB_DEV_class_driver_assigned,
    USB_DEV_not_supported,
    USB_DEV_permanent_erro
};

mss_usbh_msc_user_cb_t MSS_USBH_MSC_user_cb =
{
    MSC_DEV_valid_config,
    MSC_DEV_ready,
    MSC_DEV_driver_released,
    MSC_DEV_error,
};

/* Global variables */
uint8_t volatile g_msc_driver_ready = 0u;
uint8_t volatile g_msc_driver_released = 0u;

void e51(void)
{
    volatile uint32_t i;
    volatile uint32_t info_string[100] = {0};
    uint64_t mcycle_start = 0;
    uint64_t mcycle_end = 0;
    uint64_t delta_mcycle = 0;
    uint32_t num_loops = 1000000;
    uint32_t hartid = read_csr(mhartid);
    static volatile uint64_t dummy_h0 = 0;
    uint8_t rx_buff[1];
    uint32_t rx_idx  = 0;
    uint8_t rx_size = 0;
    uint8_t loop_count = 0;
    uint8_t key;
    uint8_t menu_level = 0u;

    init_memory();
    
    SYSREG->SOFT_RESET_CR &= ~((1u << 16u) |
                               (1u << 0u) | 
                               (1u << 4u) | 
                               (1u << 5u) | 
                               (1u << 19u) | 
                               (1u << 23u) | 
                               (1u << 28u)) ;

    loop_count = MSS_MPU_configure(MSS_MPU_MMC,
                                   MSS_MPU_PMP_REGION1,
                                   0x08000000u,
                                   0x200000,
                                   (MPU_MODE_READ_ACCESS |
                                    MPU_MODE_WRITE_ACCESS |
                                    MPU_MODE_EXEC_ACCESS),
                                    MSS_MPU_AM_NAPOT,
                                    0);

    MSS_UART_init(&g_mss_uart0_lo,
                  MSS_UART_115200_BAUD,
                  (MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | 
                  MSS_UART_ONE_STOP_BIT));

    MSS_UART_polled_tx_string(&g_mss_uart0_lo, "\n\rUSB MSD Host Example.\n\r");

    PLIC_init();
    __enable_irq();

    PLIC_SetPriority(USB_DMA_PLIC, 2);
    PLIC_SetPriority(USB_MC_PLIC, 2);

    /* Initialize SysTick */
    SysTick_Config();

    /* Initialize the USBH driver */
    MSS_USBH_init(&MSS_USBH_user_cb);

    /* Initialize the USBH-MSC driver */
    MSS_USBH_MSC_init(&MSS_USBH_MSC_user_cb);

    /*
     * Get Class driver handle from the USBH-MSC class driver and register it 
     * with the USBH driver.
     * On Device attachment, USBH driver will look for Device information 
     * through descriptors and if match it with the Class driver using this 
     * Handle.
     */
    MSS_USBH_register_class_driver(MSS_USBH_MSC_get_handle());

    while (1)
    {
        if (g_msc_driver_ready == 1u)
        {
            g_msc_driver_ready = 0u;
            g_msc_driver_released = 0u;
            menu_level = 0u;
            key = 0u;
            rx_size = 0u;
            display_main_menu();

            /*
             * Register the FatFS Work area.
             * Note:   Only one Logical unit (drive 0:) is supported.
             * If an Unformatted Drive is connected, this API will take long 
             * time to complete its operation (Depending on drive size).
             */
            f_mount(0, &fs);
        }
        
        rx_size = MSS_UART_get_rx(&g_mss_uart0_lo, &key, 1u);
        if (g_msc_driver_released == 0u)
        {
            switch (menu_level)
            {
                case 0:
                    if (rx_size > 0u)
                    {
                        switch (key)
                        {
                            case '1':/* List elements */
                                menu_level = 1u;
                                scan_root_dir(path);
                                goto_main_menu();
                            break;

                            case '2':/* copy file */
                                menu_level = 2u;
                                scan_root_dir(path);
                                copy_file_menu();
                            break;

                            case '3':/* read file */
                                menu_level = 3u;
                                scan_root_dir(path);
                                read_file_menu();
                            break;

                            case '4':/* test mode */
                                menu_level = 4u;
                                display_test_mode_menu();
                            break;

                            case '5':/* suspend host */
                                menu_level = 5u;
                                MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t *)"\n\n\r Host suspended\n\r");
                                MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\r Press 'Enter' to resume host\n\r");
                                MSS_USBH_suspend();
                            break;

                            case '6':/* Dev Desc */
                                menu_level = 6u;
                                get_device_descriptor();
                            break;

                            default:
                                invalid_selection_menu();
                            break;
                        }
                    }
                break;
                
                case 5:
                    if (rx_size > 0u)
                    {
                        switch (key)
                        {
                            case ENTER_KEY:   /* ESC Key */
                                MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\r Host resumed\n\r");
                                MSS_USBH_resume();
                                display_main_menu();
                                menu_level = 0u;
                            break;

                            default:
                                invalid_selection_menu();
                            break;
                        }
                    }
                break;

                case 3:
                    if (rx_size > 0u)
                    {
                        switch (key)
                        {
                            case ESC_KEY:   /* ESC Key */
                                display_main_menu();
                                menu_level = 0u;
                            break;

                            case '0':
                            case '1':
                            case '2':
                            case '3':
                            case '4':
                            case '5':
                            case '6':
                            case '7':
                            case '8':
                            case '9':
                                read_root_dir_file(key - 48u);
                                goto_main_menu();
                            break;
                            default:
                                invalid_selection_menu();
                            break;
                        }
                    }
                break;

                case 2:
                    if (rx_size > 0u)
                    {
                        switch (key)
                        {
                            case ESC_KEY:   /* ESC Key */
                                display_main_menu();
                                menu_level = 0u;
                            break;

                            case '0':
                            case '1':
                            case '2':
                            case '3':
                            case '4':
                            case '5':
                            case '6':
                            case '7':
                            case '8':
                            case '9':
                                g_copy_file_index = (key - '0');
                                MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\r Enter Alphanumeric character to copy file.\n\r");
                                menu_level = 33u;
                            break;
                            default:
                                invalid_selection_menu();
                            break;
                        }
                    }
                break;
                case 33:
                    if (rx_size > 0u)
                    {
                        if (ESC_KEY == key)   //ESC Key
                        {
                            display_main_menu();
                            menu_level = 0u;
                        }
                        else if ((('0' <= key) & (key <= '9')) ||
                                 (('a' <= key) & (key <= 'z')) ||
                                 (('A' <= key) & (key <= 'Z')))
                        {
                            MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\r copying file...");
                            copy_root_dir_file(g_copy_file_index, key);
                            goto_main_menu();
                        }
                        else
                            invalid_selection_menu();
                    }
                break;
                case 6:
                    display_main_menu();
                    menu_level = 0u;
                break;
                
                case 1:
                    if (rx_size > 0u)
                    {
                        switch (key)
                        {
                            case ESC_KEY:   /* ESC Key */
                                display_main_menu();
                                menu_level = 0u;
                            break;

                            default:
                                invalid_selection_menu();
                            break;
                        }
                    }
                break;
                
                case 4:
                    if(rx_size > 0u)
                    {
                        switch (key)
                        {
                            case '1': /* Test_Packet */
                                MSS_USBH_test_mode(USB_TEST_MODE_SELECTOR_TEST_PACKET);
                                MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r Test-Packet started...");
                            break;

                            case '2':/* Test_J */
                                MSS_USBH_test_mode(USB_TEST_MODE_SELECTOR_TEST_J);
                                MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r Test-J started...");
                            break;

                            case '3':/* Test_K */
                                MSS_USBH_test_mode(USB_TEST_MODE_SELECTOR_TEST_K);
                                MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r Test-K started...");
                            break;

                            case '4':/* Test_SE0_NAK */
                                MSS_USBH_test_mode(USB_TEST_MODE_SELECTOR_TEST_SE0NAK);
                                MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r Test-SE0-NAK started...");
                            break;

                            case '5':/* Force_Host_En */
                                MSS_USBH_test_mode(USB_TEST_MODE_SELECTOR_TEST_FORCE_ENA);
                                MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r Test-Force-Host started...");
                            break;

                            default:
                                invalid_selection_menu();
                            break;
                        }
                    }
                break;
                default:
                break;
            }
            rx_size = 0;
        }
    }
}

void SysTick_Handler(uint32_t hard_id)
{
    /*
     * This function must be called. This function provides the time stamp
     * which is used to keep track of activities such as USB Reset, Settling 
     * time etc. in Enumeration process.
     * The USBH driver and USBH-MSC class driver housekeeping task is also run 
     * based on this timestamp.
     *
     * This avoids the application to wait for the enumeration process to 
     * complete which might take time in 100s of miliseconds depending on 
     * target device.
     * You may choose a general purpose timer instead of Systick.
     */
    MSS_USBH_task();
    MSS_USBH_MSC_task();
    MSS_USBH_1ms_tick();
}


FRESULT scan_root_dir (
    char* path        /* Start node to be scanned (also used as work area) */
)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    int elements = 0u;

    /* This function is assuming non-Unicode cfg. */
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\rElements in Root Directory (Maximum 10):\n\r");

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK)
    {
        while (elements < MAX_ELEMENT_COUNT)
        {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (fno.fname[0] == '.') continue;             /* Ignore dot entry */

            if (res != FR_OK)
            {
                MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\r Error reading root directory\n\r");
                break;
            }
            else
            {
                if (fno.fname[0] == 0)
                {
                    if (elements == 0u)
                    {
                        MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\rRoot directory is empty\n\r");
                    }
                    break;
                }
                else
                {
                    uint8_t character = 0;
                    strncpy((char*)file_name[elements], fno.fname,13); /* store file names and attributes for using it in File read menu. */
                    file_attrib[elements] = fno.fattrib;
                    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"Testing\n\r ");
                    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\r ");
                    character = elements + '0';
                    MSS_UART_polled_tx(&g_mss_uart0_lo,(uint8_t *)&character, 1u);
                    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)" -   ");
                    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)fno.fname);
                }
            }
            elements++;
        }
    }
    return res;
}

void USB_DEV_attached(mss_usb_device_speed_t speed)
{
    if (speed == MSS_USB_DEVICE_HS)
        MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\rHS ");
    if (speed == MSS_USB_DEVICE_FS)
        MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\rFS ");
    if (speed == MSS_USB_DEVICE_LS)
        MSS_UART_polled_tx_string(  &g_mss_uart0_lo,(uint8_t *)"\n\rLS ");

    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"Device detected\n\r");
}
void USB_DEV_dettached(void)
{
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rUSB Device removed\n");
}
void USB_DEV_oc_error(void)
{
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rOverCurrent error");
}
void USB_DEV_enumerated(mss_usbh_tdev_info_t tdev_info)
{
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rEnumeration successful\n");
}
void USB_DEV_class_driver_assigned(void)
{
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rMSC Class driver assigned");
}
void USB_DEV_not_supported(int8_t error_code)
{
    switch(error_code)
    {
        case FS_DEV_NOT_SUPPORTED:
            MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\rFS Device Not Supported");
            break;
        case LS_DEV_NOT_SUPPORTED:
            MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\rLS Device Not Supported");
            break;
        case HUB_CLASS_NOT_SUPPORTED:
            MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\rHUB Not Supported");
            break;
        case CLASS_DRIVER_NOT_MATCHED:
            MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\rClass Not Supported");
            break;
        case CLASS_DRIVER_NOT_FREE:
            MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\rClass driver not free");
            break;
        case CONF_DESC_POWER_LIM_EXCEED:
            MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\rDevice needs more power");
            break;
        case CONF_DESC_WRONG_DESC_TYPE:
        case CONF_DESC_WRONG_LENGTH:
        case DEV_DESC_LS_MAXPKTSZ0_NOT8:
        case DEV_DESC_HS_MAXPKTSZ0_NOT64:
        case DEV_DESC_HS_USBBCD_NOT200:
        case DEV_DESC_WRONG_MAXPKTSZ0:
        case DEV_DESC_WRONG_USBBCD:
        case DEV_DESC_WRONG_DESC_TYPE:
        case DEV_DESC_WRONG_LENGTH:
            MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"Device not supported");
            break;
        default:
            break;
    }
}

void USB_DEV_permanent_erro(void)
{
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\rPermanent error, try reseting SF2...");
}

void MSC_DEV_valid_config(void)
{
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rConfiguring MSC device...");
}

void MSC_DEV_ready(void)
{
   MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rMSC device ready for transfers");
   g_msc_driver_ready = 1u;
}

void MSC_DEV_driver_released(void)
{
    g_msc_driver_released = 1u;
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rUSBH MSC driver released\n\r");
}

void MSC_DEV_error(int8_t error_code)
{
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rMSC device error");
}

static void display_main_menu(void)
{
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rSelect Menu");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r   1) List Root-Directory elements");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r   2) Copy file");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r   3) Read file (First 512 bytes)");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r   4) Compliance test mode");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r   5) Suspend Host");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r   6) Get Device Descriptor");
}

static void display_test_mode_menu(void)
{
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rSelect test    (Board reset required to resume normal functionality)");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r   1) Test_Packet");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r   2) Test_J");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r   3) Test_K");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r   4) Test_SE0_NAK");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r   5) Test_Force_Host");
}

static void goto_main_menu(void)
{
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\n\rPress 'ESC' to go back to main menu");
}

static void read_file_menu(void)
{
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r Choose file out of listed files by its index ('0' onwards)\n\r");
}

static void copy_file_menu(void)
{
    MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\r Choose file out of listed files by its index ('0' onwards)\n\r");
}

static void invalid_selection_menu(void)
{
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t *)"\n\r\n\rInvalid selection. Try again\n\r");
}

FRESULT read_root_dir_file (uint8_t index)
{
    FRESULT res;
    uint32_t bytes_read = 0u;

    if (file_attrib[index] & AM_DIR)
    {
        MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rAccesing directory is not supported");
        return (FR_INVALID_PARAMETER);
    }
    else
    {
        res = f_open(&fsrc, (char*)file_name[index], FA_OPEN_EXISTING | FA_READ);
        if(FR_OK == res)
        {
            res = f_read(&fsrc, buffer, sizeof(buffer), (UINT*)&bytes_read);
            MSS_UART_polled_tx(&g_mss_uart0_lo,(uint8_t *)buffer, sizeof(buffer));
        }
        else
        {
            MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rFile read error");
        }

        f_close(&fsrc);
    }
    return(res);
}

FRESULT copy_root_dir_file (uint8_t index, uint8_t character)
{
    FRESULT res;
    uint32_t bytes_read = 0u;
    uint32_t bytes_written = 0u;

    if (file_attrib[index] & AM_DIR)
    {
        MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rAccesing directory is not supported");
        return (FR_INVALID_PARAMETER);
    }
    else
    {
        res = f_open(&fsrc, (char*)file_name[index], FA_OPEN_EXISTING | FA_READ);
        if(res)
        {
            MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rError while opening selected file");
        }

        strncpy((char*)new_fname, (char*)file_name[index], 13);

        new_fname[0] = character;    /* Replace First character of filename */

        res = f_open(&fdest, (char*)new_fname, FA_CREATE_ALWAYS  | FA_WRITE);
        if (FR_OK == res)
        {
             /* Copy source to destination */
            for (;;)
            {
                res = f_read(&fsrc, buffer, sizeof buffer, (UINT*)&bytes_read);/* Read a chunk of src file */

                MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)".");
                if (res || bytes_read == 0)
                    break; /* error or eof */

                res = f_write(&fdest, buffer, bytes_read, (UINT*)&bytes_written);/* Write it to the dst file */

                if (res || bytes_written < bytes_read)
                    break; /* error or disk full */
            }

            if (res)
                MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rCopy error");
            else if (bytes_read == 0)
            {
                MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rCopy success \n\r New file name - ");
                MSS_UART_polled_tx(&g_mss_uart0_lo,(uint8_t *)new_fname, 13u);
            }
            else if (bytes_written < bytes_read)
                MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rDisk space not sufficient");
        }
        else if (FR_EXIST == res)
        {
            MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rTrying to create ");
            MSS_UART_polled_tx(&g_mss_uart0_lo,(uint8_t *)new_fname, sizeof(new_fname));
            MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rFile already exists");
        }

        f_close(&fsrc);
        f_close(&fdest);
        return(res);
    }
}

static void get_device_descriptor(void)
{
    uint8_t idx = 0u;

    if (0u == MSS_USBH_get_std_dev_descr(g_dev_descr))
    {
        MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rDevice Descriptor:");
        do
        {
            display_hex_byte(g_dev_descr[idx]);
            idx++;
        }while (idx < 8);
    }
    else
    {
        MSS_UART_polled_tx_string(&g_mss_uart0_lo,(uint8_t *)"\n\n\rError reading descriptor");
    }
}
