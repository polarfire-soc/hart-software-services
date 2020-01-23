/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * code running on e51
 *
 * PolarFire SoC MSS PDMA Driver example project
 *
 * SVN $Revision$
 * SVN $Date$
 */

#include <stdio.h>
#include <string.h>
#include "mpfs_hal/mss_hal.h"
#include "drivers/mss_pdma/mss_pdma.h"
#include "drivers/mss_uart/mss_uart.h"

/* Local buffers to store the destination and source address data.
 */
uint8_t g_src_arr[1024] = { 0 };
uint8_t g_dst_arr[1024] = { 0 };

/* Global variable to store the error code in PDMA channel configuration.
 */
uint8_t g_pdma_error_code = 0;

/* This function will ensure that the configuration of PDMA channel is success.
 * In case of error, appropriate UART message will be generated.
 */
static uint8_t verify(uint8_t* g_dst_arr,uint8_t* g_src_arr,uint32_t num_loops);

/* This function will compare the data in source and destination address. Any
 * mismatch in the data is reported to the main function.
 */
static void check_pdma_error(uint8_t g_pdma_error_code);

/* Debug mask to track interrupt processing. */
uint8_t g_done_int_processed = 0;
uint8_t g_err_int_processed = 0;

uint64_t uart_lock;

/******************************************************************************
 *  Greeting messages displayed over the UART.
 */
const uint8_t g_greeting_msg[] =
"\r\n\r\n\t  ******* PolarFire SoC PDMA Driver Example *******\n\n\n\r\
Select options below to select the DMA channel for transactions. After\r\n\
selecting the channel, the following messages on terminal will notify about \r\n\
success / failure about the DMA transactions \r\n\
\n\n\r Press 0--> Initiate PDMA CH0 \n\r\
\n\n\r Press 1--> Initiate PDMA CH1 \n\r\
\n\n\r Press 2--> Initiate PDMA CH2 \n\r\
\n\n\r Press 3--> Initiate PDMA CH3 \n\r\
";

/* Main function for the HART0(E51 processor).
 * Application code running on HART0 is placed here.
 */
void e51(void)
{
    uint8_t rx_buff[1];
    uint8_t rx_size = 0u;
    uint32_t errors;
    uint32_t mem_idx;

    SYSREG->SOFT_RESET_CR &= ~((1u << 0u) | (1u << 4u) | (1u << 5u) |
                              (1u << 19u) | (1u << 23u) | (1u << 28u)) ;// MMUART0

    PLIC_init();
    __enable_irq();

    PLIC_SetPriority(DMA_CH0_DONE_IRQn, 1u);
    PLIC_SetPriority(DMA_CH0_ERR_IRQn, 1u);
    PLIC_SetPriority(DMA_CH1_DONE_IRQn, 1u);
    PLIC_SetPriority(DMA_CH1_ERR_IRQn, 1u);
    PLIC_SetPriority(DMA_CH2_DONE_IRQn, 1u);
    PLIC_SetPriority(DMA_CH2_ERR_IRQn, 1u);
    PLIC_SetPriority(DMA_CH3_DONE_IRQn, 1u);
    PLIC_SetPriority(DMA_CH3_ERR_IRQn, 1u);

    /* Enable PDMA Interrupts. */
    PLIC_EnableIRQ(DMA_CH0_DONE_IRQn);
    PLIC_EnableIRQ(DMA_CH0_ERR_IRQn);
    PLIC_EnableIRQ(DMA_CH1_DONE_IRQn);
    PLIC_EnableIRQ(DMA_CH1_ERR_IRQn);
    PLIC_EnableIRQ(DMA_CH2_DONE_IRQn);
    PLIC_EnableIRQ(DMA_CH2_ERR_IRQn);
    PLIC_EnableIRQ(DMA_CH3_DONE_IRQn);
    PLIC_EnableIRQ(DMA_CH3_ERR_IRQn);

    MSS_UART_init( &g_mss_uart0_lo,
            MSS_UART_115200_BAUD,
            MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    MSS_UART_polled_tx_string (&g_mss_uart0_lo, g_greeting_msg);

    g_done_int_processed = 0u;
    g_err_int_processed = 0u;

    /* Example MEMORY 2 MEMORY DMA Transaction. */
    for(mem_idx = 0; mem_idx < 1024; mem_idx++)
    {
        g_src_arr[mem_idx] = (mem_idx & 0xFF);
    }

    /* Setup the channel configuration structure.
     * Simple memory to memory transaction.
     * */
    mss_pdma_channel_config_t pdma_config_ch0;
    pdma_config_ch0.src_addr  = (uint64_t) &g_src_arr;
    pdma_config_ch0.dest_addr = (uint64_t)&g_dst_arr;
    pdma_config_ch0.num_bytes = 1024;
    pdma_config_ch0.enable_done_int = 1u;
    pdma_config_ch0.enable_err_int = 1u;
    pdma_config_ch0.force_order = 0u;
    pdma_config_ch0.repeat = 0u;

    while(1)
    {
        rx_size = MSS_UART_get_rx(&g_mss_uart0_lo, rx_buff, sizeof(rx_buff));
        if (rx_size > 0)
        {
            MSS_UART_polled_tx_string (&g_mss_uart0_lo, "\n\r");
            memset(g_dst_arr, 0x00, sizeof(g_dst_arr));

            // Every Loop set correct Source and Destination
            pdma_config_ch0.src_addr  = (uint64_t) &g_src_arr;
            pdma_config_ch0.dest_addr = (uint64_t)&g_dst_arr;

            if (rx_buff[0] == '0')
            {
                g_pdma_error_code = MSS_PDMA_setup_transfer(MSS_PDMA_CHANNEL_0,
                                                          &pdma_config_ch0);
                if (g_pdma_error_code != 0)
                {
                    check_pdma_error(g_pdma_error_code);
                }

                g_pdma_error_code = MSS_PDMA_start_transfer(MSS_PDMA_CHANNEL_0);
                if (g_pdma_error_code != 0)
                {
                    check_pdma_error(g_pdma_error_code);
                }

                MSS_UART_polled_tx_string (&g_mss_uart0_lo, "DMA CH '0' ");
            }
            else if (rx_buff[0] == '1')
            {
                MSS_PDMA_setup_transfer(MSS_PDMA_CHANNEL_1, &pdma_config_ch0);
                MSS_PDMA_start_transfer(MSS_PDMA_CHANNEL_1);

                MSS_UART_polled_tx_string (&g_mss_uart0_lo, "DMA CH '1' ");
            }
            else if (rx_buff[0] == '2')
            {
                MSS_PDMA_setup_transfer(MSS_PDMA_CHANNEL_2, &pdma_config_ch0);
                MSS_PDMA_start_transfer(MSS_PDMA_CHANNEL_2);

                MSS_UART_polled_tx_string (&g_mss_uart0_lo, "DMA CH '2' ");

            }
            else if (rx_buff[0] == '3')
            {
                MSS_PDMA_setup_transfer(MSS_PDMA_CHANNEL_3, &pdma_config_ch0);
                MSS_PDMA_start_transfer(MSS_PDMA_CHANNEL_3);

                MSS_UART_polled_tx_string (&g_mss_uart0_lo, "DMA CH '3' ");
            }
            else
            {
                MSS_UART_polled_tx_string (&g_mss_uart0_lo, "Please Select Correct Channel  \n\r");
            }
        }

        /* Poll to get to see did we get the done interrupt flag, and report status. */
        /* g_done_int_processed: Bit0 for Channel 0
         * g_done_int_processed: Bit1 for Channel 1
         * g_done_int_processed: Bit2 for Channel 2
         * g_done_int_processed: Bit3 for Channel 3
         */
        g_done_int_processed |= MSS_PDMA_clear_transfer_complete_status(MSS_PDMA_CHANNEL_0)
                                                              << MSS_PDMA_CHANNEL_0;
        g_done_int_processed |= MSS_PDMA_clear_transfer_complete_status(MSS_PDMA_CHANNEL_1)
                                                              << MSS_PDMA_CHANNEL_1;
        g_done_int_processed |= MSS_PDMA_clear_transfer_complete_status(MSS_PDMA_CHANNEL_2)
                                                              << MSS_PDMA_CHANNEL_2;
        g_done_int_processed |= MSS_PDMA_clear_transfer_complete_status(MSS_PDMA_CHANNEL_3)
                                                              << MSS_PDMA_CHANNEL_3;

        /* UART Reporting and debug messages. */
         if (g_done_int_processed)
         {
             /* UART Reporting and debug messages. */
             errors = verify((uint8_t*)&g_dst_arr[0],(uint8_t*)&g_src_arr[0],
                              sizeof(g_dst_arr));

             if (errors == 0u)
                 MSS_UART_polled_tx_string (&g_mss_uart0_lo, "\n\rMemory to Memory DMA Transaction successful \n\r");
            else
                MSS_UART_polled_tx_string (&g_mss_uart0_lo, "\n\rMemory to Memory DMA Transaction failed \n\r");

             if (g_done_int_processed & (0x01 << MSS_PDMA_CHANNEL_0))
             {
                 g_done_int_processed &= (~(g_done_int_processed & (0x01 << MSS_PDMA_CHANNEL_0)));
                 MSS_UART_polled_tx_string (&g_mss_uart0_lo, "PDMA CH0 Done Int\n\r");
             }
             else if (g_done_int_processed & (0x01 << MSS_PDMA_CHANNEL_1))
             {
                 g_done_int_processed &= (~(g_done_int_processed & (0x01 << MSS_PDMA_CHANNEL_1)));
                 MSS_UART_polled_tx_string (&g_mss_uart0_lo, "PDMA CH1 Done Int\n\r");
             }
             else if (g_done_int_processed & (0x01 << MSS_PDMA_CHANNEL_2))
             {
                g_done_int_processed &= (~(g_done_int_processed & (0x01 << MSS_PDMA_CHANNEL_2)));
                MSS_UART_polled_tx_string (&g_mss_uart0_lo, "PDMA CH2 Done Int\n\r");
             }
             else if (g_done_int_processed & (0x01 << MSS_PDMA_CHANNEL_3))
             {
                g_done_int_processed &= (~(g_done_int_processed & (0x01 << MSS_PDMA_CHANNEL_3)));
                MSS_UART_polled_tx_string (&g_mss_uart0_lo, "PDMA CH3 Done Int\n\r");
             }
         }

         /* Poll to get to see did we get the error interrupt flag, and report status. */
         /* g_err_int_processed: Bit0 for Channel 0
          * g_err_int_processed: Bit1 for Channel 1
          * g_err_int_processed: Bit2 for Channel 2
          * g_err_int_processed: Bit3 for Channel 3
          */
         g_err_int_processed |= MSS_PDMA_clear_transfer_error_status(MSS_PDMA_CHANNEL_0)
                                                              << MSS_PDMA_CHANNEL_0;
         g_err_int_processed |= MSS_PDMA_clear_transfer_error_status(MSS_PDMA_CHANNEL_1)
                                                              << MSS_PDMA_CHANNEL_1;
         g_err_int_processed |= MSS_PDMA_clear_transfer_error_status(MSS_PDMA_CHANNEL_2)
                                                              << MSS_PDMA_CHANNEL_2;
         g_err_int_processed |= MSS_PDMA_clear_transfer_error_status(MSS_PDMA_CHANNEL_3)
                                                              << MSS_PDMA_CHANNEL_3;

         /* UART Reporting and debug messages. */
         if (g_err_int_processed)
         {
             if (g_err_int_processed & (0x01 << MSS_PDMA_CHANNEL_0))
             {
                 g_err_int_processed &= (~(g_err_int_processed & (0x01 << MSS_PDMA_CHANNEL_0)));
                 MSS_UART_polled_tx_string (&g_mss_uart0_lo, "PDMA CH0 Err Int\n\r");
             }
             else if (g_err_int_processed & (0x01 << MSS_PDMA_CHANNEL_1))
             {
                 g_err_int_processed &= (~(g_err_int_processed & (0x01 << MSS_PDMA_CHANNEL_1)));
                 MSS_UART_polled_tx_string (&g_mss_uart0_lo, "PDMA CH1 Err Int\n\r");
             }
             else if (g_err_int_processed & (0x01 << MSS_PDMA_CHANNEL_2))
             {
                 g_err_int_processed &= (~(g_err_int_processed & (0x01 << MSS_PDMA_CHANNEL_2)));
                MSS_UART_polled_tx_string (&g_mss_uart0_lo, "PDMA CH2 Err Int\n\r");
             }
             else if (g_err_int_processed & (0x01 << MSS_PDMA_CHANNEL_3))
             {
                 g_err_int_processed &= (~(g_err_int_processed & (0x01 << MSS_PDMA_CHANNEL_3)));
                MSS_UART_polled_tx_string (&g_mss_uart0_lo, "PDMA CH3 Err Int\n\r");
             }
         }
    }
}

/* Local function to verify the PDMA transaction.
 */
static uint8_t
verify
(
    uint8_t* g_dst_arr,
    uint8_t* g_src_arr,
    uint32_t num_loops
)
{
    uint8_t error = 0u;
    uint32_t idx = 0u;
    while (num_loops != 0)
    {
        if (g_dst_arr[idx] != g_src_arr[idx])
        {
            error = 1;
            break;
        }
        idx++;
        num_loops--;
    }

    return error;
}

/*
 * Local function to check the errors in PDMA channel configuration
 */
static void
check_pdma_error
(
    uint8_t g_pdma_error_code
)
{
    if (g_pdma_error_code == 1)
    {
        MSS_UART_polled_tx_string (&g_mss_uart0_lo, "Invalid source address\n\r");
    }
    else if (g_pdma_error_code == 2)
    {
        MSS_UART_polled_tx_string (&g_mss_uart0_lo, "Invalid destination address\n\r");
    }
    else if (g_pdma_error_code == 3)
    {
        MSS_UART_polled_tx_string (&g_mss_uart0_lo, "Transaction in progress\n\r");
    }
    else if (g_pdma_error_code == 4)
    {
        MSS_UART_polled_tx_string (&g_mss_uart0_lo, "Invalid Channel ID\n\r");
    }
    else if (g_pdma_error_code == 5)
    {
        MSS_UART_polled_tx_string (&g_mss_uart0_lo, "Invalid write size\n\r");
    }
    else if (g_pdma_error_code == 6)
    {
        MSS_UART_polled_tx_string (&g_mss_uart0_lo, "Invalid Read size\n\r");
    }
    else if (g_pdma_error_code == 7)
    {
        MSS_UART_polled_tx_string (&g_mss_uart0_lo, "Last ID\n\r");
    }
}


