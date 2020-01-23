/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MSS QSPI driver example
 *
 */
/******************************************************************************
 *
 * code running on E51
 *
 * SVN $Revision: 9661 $
 * SVN $Date: 2018-01-15 10:43:33 +0000 (Mon, 15 Jan 2018) $
 */

#include "mpfs_hal/mss_hal.h"
#include "helper.h"
#include "drivers/micron_mt25q/micron_mt25q.h"
#include "drivers/mss_uart/mss_uart.h"
#include "drivers/mss_qspi/mss_qspi.h"

#define BUFFER_A_SIZE       71680u
#define FLASH_PAGE_LENGTH   256u

static uint8_t rd_buf[10] __attribute__ ((aligned (4)));
static uint8_t g_flash_wr_buf[BUFFER_A_SIZE] __attribute__ ((aligned (4)));
static uint8_t g_flash_rd_buf[BUFFER_A_SIZE] __attribute__ ((aligned (4)));
static uint8_t verify_write(uint8_t* write_buff, uint8_t* read_buff, uint32_t size);
static mss_qspi_io_format g_io_format = MSS_QSPI_NORMAL;

mss_uart_instance_t *g_uart;
volatile uint32_t xip_read[10] = {0};
uint64_t uart_lock;

/*This function will write data to the QSPI flash, read it back and cross verify
 * the read data with the data that was written.*/
static uint8_t flash_mem_readwrite_test(void)
{
    uint8_t* buf_ptr  = 0;
    uint32_t loop_count;

    Flash_die_erase();

    buf_ptr = &g_flash_wr_buf[0];
    for(loop_count = 0; loop_count < (BUFFER_A_SIZE/ FLASH_PAGE_LENGTH); loop_count++)
    {
        Flash_program(buf_ptr, (loop_count*FLASH_PAGE_LENGTH), FLASH_PAGE_LENGTH);
        buf_ptr += FLASH_PAGE_LENGTH;
    }

    Flash_read(g_flash_rd_buf, 0, BUFFER_A_SIZE);

    return(verify_write(g_flash_wr_buf, g_flash_rd_buf, BUFFER_A_SIZE));
}

/* Main function for the HART0(E51 processor).
 * Application code running on HART0 is placed here.
 */
void e51(void)
{
	int8_t info_string[100];
    uint8_t rx_buff[1];
    uint8_t rx_size = 0;
    uint32_t loop_count;
    uint8_t error=0;

    SYSREG->SOFT_RESET_CR &= ~((1u << 0u) | (1u << 4u) | (1u << 5u) |
                               (1u << 19u) | (1u << 23u) | (1u << 28u));

    PLIC_init();
    __enable_irq();

    g_uart = &g_mss_uart0_lo;
    MSS_UART_init( g_uart,
                   MSS_UART_115200_BAUD,
                   MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    MSS_UART_polled_tx_string(g_uart, "\r\n\r\n\r\n********PolarFire SoC QSPI Example*********\r\n");
    MSS_UART_polled_tx_string(g_uart, "\r\nAccessing Micron on-board Flash \
memory in NORMAL, DUAL and QUAD formats\r\n");

    PLIC_SetPriority(QSPI_PLIC,2);
    PLIC_EnableIRQ(QSPI_PLIC);
    MSS_QSPI_enable();

    /* Ideally, this function call is not required. User must know what is the default
     * mode of the Flash Memory that he is using. and configure the QSPI controller
     * accordingly. You may call this function if you are not sure what is the
     * default mode the Flash memory is operating in and force it to normal mode.
     * At the end of this function the QSPI controller and flash memory both shall
     * be in NORMAL mode.*/
/*    Flash_force_normal_mode(); */

    g_io_format = Flash_probe_io_format();
    MSS_UART_polled_tx_string (g_uart, "\r\n\r\nFlash memory current IO format:\
0=Normal, 6=Dual, 7= Quad\r\n\r\n");

    display_output((uint8_t*)&g_io_format, 1);

    if(MSS_QSPI_NORMAL != g_io_format)
    {
        Flash_force_normal_mode();
        MSS_UART_polled_tx_string (g_uart, "\r\nGot the Flash memory to \
normal mode now.\r\n");
    }
    /*--------------------------------------------------------------------------
     * Initialize the write and read Buffers
    */
    for(loop_count = 0; loop_count < (BUFFER_A_SIZE); loop_count++)
    {
        g_flash_wr_buf[loop_count] = 0x15 + loop_count;
        g_flash_rd_buf[loop_count] = 0x00;
    }

    MSS_UART_polled_tx_string (g_uart, "\r\n\r\n\r\nUse Normal IO format\r\n");
    Flash_init(MSS_QSPI_NORMAL);
    MSS_UART_polled_tx_string (g_uart, "\r\nREADID\r\n");
    Flash_readid(rd_buf);
    display_output(rd_buf, 1);
    MSS_UART_polled_tx_string (g_uart, "\r\nNORMAL IO format read write test\r\n");
    MSS_UART_polled_tx_string (g_uart, "\r\nREAD flash result 0=PASS, 1=FAIL\r\n");
    error = flash_mem_readwrite_test();
    display_output((uint8_t*)&error, 1);
    MSS_UART_polled_tx_string (g_uart, "\r\n\r\nDATA\r\n");
    display_output(g_flash_rd_buf, 10);


    /*--------------------------------------------------------------------------
     * Initialize the write and read Buffers
    */
    for(loop_count = 0; loop_count < (BUFFER_A_SIZE); loop_count++)
    {
        g_flash_wr_buf[loop_count] = 0x25 + loop_count;
        g_flash_rd_buf[loop_count] = 0x00;
    }

    MSS_UART_polled_tx_string (g_uart, "\r\n\r\n\r\nEnter DUAL FULL IO format\r\n");
    Flash_init(MSS_QSPI_DUAL_FULL);
    MSS_UART_polled_tx_string (g_uart, "\r\nREADID\r\n");
    Flash_readid(rd_buf);
    display_output(rd_buf, 1);
    MSS_UART_polled_tx_string (g_uart, "\r\nDual Full Mode read write test\r\n");
    MSS_UART_polled_tx_string (g_uart, "\r\nREAD flash result 0=PASS, 1=FAIL\r\n");
    error = flash_mem_readwrite_test();
    display_output((uint8_t*)&error, 1);
    MSS_UART_polled_tx_string (g_uart, "\r\n\r\nDATA\r\n");
    display_output(g_flash_rd_buf, 10);

    /*--------------------------------------------------------------------------
     * Initialize the write and read Buffers
    */
    for(loop_count = 0; loop_count < (BUFFER_A_SIZE); loop_count++)
    {
        g_flash_wr_buf[loop_count] = 0x35 + loop_count;
        g_flash_rd_buf[loop_count] = 0x00;
    }

    MSS_UART_polled_tx_string (g_uart, "\r\n\r\n\r\nEnter QUAD FULL IO format\r\n");
    Flash_init(MSS_QSPI_QUAD_FULL);
    MSS_UART_polled_tx_string (g_uart, "\r\nREADID\r\n");
    Flash_readid(rd_buf);
    display_output(rd_buf, 1);
    MSS_UART_polled_tx_string (g_uart, "\r\nQUAD Full IO format read write test\r\n");
    MSS_UART_polled_tx_string (g_uart, "\r\nREAD flash result 0=PASS, 1=FAIL\r\n");
    error = flash_mem_readwrite_test();
    display_output((uint8_t*)&error, 1);
    MSS_UART_polled_tx_string (g_uart, "\r\n\r\nDATA\r\n");
    display_output(g_flash_rd_buf, 10);

    MSS_UART_polled_tx_string (g_uart, "\r\nEnter XIP\r\n");
    /*Enter XIP mode*/
    Flash_enter_xip();

    /*As we are in XIP mode, the AHB access to the QSPI memory space will now
     * return the FLASH memory data which was previously written by Flash_program()
     * Monitor the values of xip_read[]. Note that we are in 3 byte mode so only
     * first 3 bytes of the 32 but address will be fetched from FLASH*/
    xip_read[0] = *(uint32_t*)0x21000000;
    xip_read[1] = *(uint32_t*)0x21000004;
    MSS_UART_polled_tx_string (g_uart, "\r\nXIP Data\r\n");
    display_output((uint8_t*)&xip_read, 8);

    /*Exit XIP mode*/
    Flash_exit_xip();
    MSS_UART_polled_tx_string (g_uart, "\r\nExit XIP\r\n");

    /*Now we are back to normal mode. Now the AHB access will return the QSPI
     * register values.*/
    xip_read[0] = *(uint32_t*)0x21000000;
    xip_read[1] = *(uint32_t*)0x21000004;
    MSS_UART_polled_tx_string (g_uart, "\r\nAHB Access Data\r\n");
    display_output((uint8_t*)&xip_read, 8);

    while(1)
    {
        rx_size = MSS_UART_get_rx(&g_mss_uart0_lo, rx_buff, sizeof(rx_buff));
        if(rx_size > 0)
        {
        	/* echo the rx char */
        	MSS_UART_polled_tx(&g_mss_uart0_lo, rx_buff, rx_size);
        }
    }
}

/***************************************************************************//**
 * Read the date from SPI FLASH and compare the same with write buffer.
 */
static uint8_t verify_write(uint8_t* write_buff, uint8_t* read_buff, uint32_t size)
{
    uint8_t error = 0;
    uint32_t index = 0;

    while(size != 0)
    {
        if(write_buff[index] != read_buff[index])
        {
            error = 1;
            break;
        }
        index++;
        size--;
    }

    return error;
}

uint8_t maintenance_u51_local_IRQHandler_0(void)
{
    return(0);
}

