/***********************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Code running on e51.
 *
 * Example project demonstrating the use of PolarFire SoC System Services
 * supported by system controller.
 *
 * Please refer to Readme.txt file for more details
 *
 * SVN $Revision: 9661 $
 * SVN $Date: 2018-01-15 10:43:33 +0000 (Mon, 15 Jan 2018) $
 */

#include <stdio.h>
#include "mpfs_hal/mss_hal.h"
#include "inc/helper.h"
#include "drivers/mss_uart/mss_uart.h"
#include "drivers/mss_sys_services/mss_sys_services.h"

/******************************************************************************
 * Enable this define to use system service driver in interrupt mode. Refer to
 * Readme.txt located in the root folder for more information.
 */
//#define MSS_SYS_INTERRUPT_MODE

/*****************************************************************************
 * MSS_SYS_MAILBOX_DATA_OFFSET
 * Mailbox data offset for all system services.
 */
#define MSS_SYS_MAILBOX_DATA_OFFSET                           0u

/******************************************************************************
 * Private functions.
 */
static void mss_sys_service_interrupt_handler(void);

/******************************************************************************
 * Global variable
 */
volatile uint32_t g_message_interrupt;
uint64_t uart_lock;

/******************************************************************************
 *  Greeting messages displayed over the UART.
 */
const uint8_t g_greeting_msg[] =
"\r\n\r\n\t  ******* PolarFire SoC system services Example *******\n\n\n\r\
Note: Return data from System controller is displayed byte-wise with LSB first\n\n\r\
Note: Input data is provided LSB first. Each ASCII character is one Nibble of data\n\n\r\
Note: This application demonstrates the execution of some of the system services. \n\r\
Most of the services require some infrastructure in the design for successful \r\n\
execution. If the required infrastructure is not available, service will not \n\r\
execute or return error code. Refer readme.txt for more details.\r\n\n\
\r\n1. Serial number service \r\n\
\r\n2. Generate OTP service\r\n\
\r\n3. Terminate debug service \n\r\
\r\n4. SPI copy service \n\r\
\r\n5. Match OTP service \n\r";

const uint8_t iap_msg[] =
"\r\nSelect IAP service option \
\n\r1.IAP Program by Index   \
\r\n2.IAP Verify by index  \
\r\n3.IAP Program by address \
\r\n4.IAP Verify by address  \
\r\n5.IAP Auto-Update";

const uint8_t g_separator[] =
"\r\n----------------------------------------------------------------------\r\n";

/******************************************************************************
 * Maximum receiver buffer size.
 *****************************************************************************/
uint8_t data_buffer [1024];

/******************************************************************************
 * Display greeting message when application is started.
 */
static void display_greeting(void)
{
    MSS_UART_polled_tx_string (&g_mss_uart0_lo, g_greeting_msg);
}

/******************************************************************************
 * Demonstrate serial number service
 */
void execute_serial_number_service(void)
{
    uint8_t status;

    MSS_UART_polled_tx_string (&g_mss_uart0_lo, "\r\nDevice serial number: ");
    status = MSS_SYS_get_serial_number(data_buffer, MSS_SYS_MAILBOX_DATA_OFFSET);

#ifdef MSS_SYS_INTERRUPT_MODE

    while(!g_message_interrupt);
    g_message_interrupt = 0u;
    status = MSS_SYS_read_response();
#endif

    if(MSS_SYS_SUCCESS == status)
    {
       display_output(data_buffer, MSS_SYS_SERIAL_NUMBER_RESP_LEN);
    }
    else
    {
        MSS_UART_polled_tx_string (&g_mss_uart0_lo,
                                  (const uint8_t*)"Service read device serial \
                                          number failed.\r\n");
    }

    MSS_UART_polled_tx_string (&g_mss_uart0_lo, g_separator);
}

/******************************************************************************
 * Demonstrates execute IAP service
 */
void execute_iap(void)
{
    uint8_t cmd = 0u;
    uint32_t spiaddr = 0u;
    uint8_t uart_input[8]={0};
    uint32_t* ptr = 0u;

     /* Read FF timeout value*/
    get_input_data(uart_input, (sizeof(cmd)),
                  (const uint8_t*)iap_msg,
                  sizeof(iap_msg));

    ptr = ((uint32_t*)&uart_input);

    switch(*ptr)
    {
        case 1:
            cmd = MSS_SYS_IAP_PROGRAM_BY_SPIIDX_CMD;
            break;
        case 2:
            cmd = MSS_SYS_IAP_VERIFY_BY_SPIIDX_CMD;
            break;
        case 3:
            cmd = MSS_SYS_IAP_PROGRAM_BY_SPIADDR_CMD;
            break;
        case 4:
            cmd = MSS_SYS_IAP_VERIFY_BY_SPIADDR_CMD;
            break;
        case 5:
            cmd = MSS_SYS_IAP_AUTOUPDATE_CMD;
        break;

        default: MSS_UART_polled_tx_string (&g_mss_uart0_lo, (const uint8_t*)
                "Invalid input.\r\n");
    }

    get_input_data(uart_input,
                   (sizeof(spiaddr)),
                   (const uint8_t*)"\r\nEnter Index for option 1,2.\
                    \r\nEnter address for options 3,4\
                    \r\nPress 'Enter' for option 5",
                    sizeof("\r\nEnter Index for option 1,2.\
                            \r\nEnter address for options 3,4\
                            \r\nPress 'Enter' for option 5"));

    ptr = ((uint32_t*)&uart_input);
    spiaddr = *ptr;

    MSS_SYS_execute_iap(cmd, spiaddr);
}

/******************************************************************************
 * Demonstrate design info service
 */
void execute_designinfo_service(void)
{
    uint8_t status;

    MSS_UART_polled_tx_string (&g_mss_uart0_lo, (const uint8_t*)"Design ID: ");
    status = MSS_SYS_get_design_info(data_buffer, MSS_SYS_MAILBOX_DATA_OFFSET);

    if(MSS_SYS_SUCCESS == status)
    {
       display_output(data_buffer, 32);

       MSS_UART_polled_tx_string (&g_mss_uart0_lo, (const uint8_t*)"\r\nDesign \
                                                    Version: ");
       display_output((data_buffer + 32), 2);

       MSS_UART_polled_tx_string (&g_mss_uart0_lo, (const uint8_t*)"\r\nDesign \
                                                    Back-Level: ");
       display_output((data_buffer + 34), 2);
    }
    else
    {
        MSS_UART_polled_tx_string (&g_mss_uart0_lo, (const uint8_t*)"DesignInfo \
                Service failed.\r\n");
    }

    MSS_UART_polled_tx_string (&g_mss_uart0_lo,  g_separator);
}

/******************************************************************************
 * Demonstrate SPI copy service
 */
void execute_spi_copy_service(void)
{
    uint8_t status;
    uint32_t n_bytes = 8u;
    uint64_t mss_addr = 0x1400000000u;
    uint32_t spi_addr = 0x0000u;
    uint8_t frequency = 3u;
    uint32_t count = 0u;
    uint8_t byte;

    uint8_t l_mss_addr[8] = {0};
    uint8_t l_spi_addr[4] = {0};
    uint8_t l_n_bytes[4] = {0};

    MSS_UART_polled_tx_string (&g_mss_uart0_lo, (const uint8_t*)"\r\n ");
    MSS_UART_polled_tx_string (&g_mss_uart0_lo, (const uint8_t*)"\n SPI copy service\r\n ");

    status = MSS_SYS_spi_copy(mss_addr , spi_addr, n_bytes,
                                       frequency, MSS_SYS_MAILBOX_DATA_OFFSET);
#ifdef MSS_SYS_INTERRUPT_MODE

    while(!g_message_interrupt);
    g_message_interrupt = 0u;
    status = MSS_SYS_read_response();

#endif

    /* An external flash device must be connected and design must include the
     * connections to that external flash to execute this service successfully*/
     if (MSS_SYS_SUCCESS == status)
     {
         MSS_UART_polled_tx_string (&g_mss_uart0_lo, (const uint8_t*)"SPI Copy test\r\n");

         for (count = 0; count < n_bytes; count++)
         {
             sprintf(data_buffer, (uint8_t*) "%x   ", *((uint8_t *) 0x1400000000U + count));
             MSS_UART_polled_tx_string(&g_mss_uart0_lo,(const uint8_t*) data_buffer);
         }

         MSS_UART_polled_tx_string (&g_mss_uart0_lo, (const uint8_t*)"\r\n ");
         MSS_UART_polled_tx_string (&g_mss_uart0_lo, (const uint8_t*)"\r\n ");
     }
     else if (status == MSS_SYS_SPI_MASTER_MODE_ERR)
     {
         MSS_UART_polled_tx_string (&g_mss_uart0_lo,
                 (const uint8_t*)"Device not configured for master mode\r\n");
     }
     else if (status == MSS_SYS_SPI_AXI_ERR)
     {
         for (count = 0; count < n_bytes; count++)
         {
             sprintf(data_buffer, (uint8_t*) "%x   ", *((uint8_t *) 0x1400000000U + count));
             MSS_UART_polled_tx_string(&g_mss_uart0_lo,(const uint8_t*) data_buffer);
         }

         MSS_UART_polled_tx_string (&g_mss_uart0_lo, (const uint8_t*)"\r\n ");
         MSS_UART_polled_tx_string (&g_mss_uart0_lo, (const uint8_t*)"\nAXI error\r\n");
         MSS_UART_polled_tx_string (&g_mss_uart0_lo, (const uint8_t*)"\r\n ");
     }

     MSS_UART_polled_tx_string(&g_mss_uart0_lo, g_separator);
}

/******************************************************************************
 * Demonstrates generate OTP service
 */
void execute_generate_otp_service(void)
{
    uint8_t status;
    uint8_t user_nonce[16] = {0x31, 0x14, 0xb4, 0xfa, 0x0c, 0xfa, 0xd6, 0x05,
                              0x08, 0x7b, 0x30, 0x62, 0x0c, 0xbe, 0xc8, 0x5e};

    MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t*)"\r\nGENERATE OTP SERVICE\r\n");

    status = MSS_SYS_otp_generate(MSS_SYS_KM_USER_KEY1, user_nonce,
                                       data_buffer, MSS_SYS_MAILBOX_DATA_OFFSET,
                                       20u);
#ifdef MSS_SYS_INTERRUPT_MODE

    while(!g_message_interrupt);
    g_message_interrupt = 0u;
    status = MSS_SYS_read_response();

#endif

    if(MSS_SYS_SUCCESS == status)
    {
        MSS_UART_polled_tx_string(&g_mss_uart0_lo,
                (uint8_t*)"\r\n\nGENERATE OTP SERVICE RESPONSE\r\n\n");
         MSS_UART_polled_tx_string (&g_mss_uart0_lo, (const uint8_t*)"\r\n ");

         display_output(data_buffer, MSS_SYS_GENERATE_OTP_RESP_LEN);
    }

    else if(MSS_SYS_GENERATE_OTP_SECERR == status)
    {
        MSS_UART_polled_tx_string (&g_mss_uart0_lo,
                (const uint8_t*)"Operation blocked by device security.\r\n");
    }

    else if(MSS_SYS_GENERATE_OTP_PROTOCOLERR == status)
    {
        MSS_UART_polled_tx_string (&g_mss_uart0_lo,
                (const uint8_t*)"Invalid keymode specified.\r\n");

         display_output(data_buffer, MSS_SYS_GENERATE_OTP_RESP_LEN);
    }

    MSS_UART_polled_tx_string (&g_mss_uart0_lo, g_separator);
}

/******************************************************************************
 * Demonstrates match OTP service
 */
void execute_match_otp_service(void)
{
    uint8_t status;
    uint8_t one_time_passcode[32] = {0};
    uint8_t user_id[16] = {0};

    /*NOTE: The parameter 'validator' is parameter is initialized to {0} here
     * for demonstration.
     * You must initialize it to a Proper validator value per application need
     * for the service to execute successfully.
     * */
    uint8_t validator[32] = {0};


    MSS_UART_polled_tx_string(&g_mss_uart0_lo,
            (const uint8_t*)"\r\nMatch OTP service\r\n" );


    /* The UID parameter is only used if the KEYMODE used for the GENERATE OTP
     * service was KEYMODE_FACTORY_KEY and the passcode was not the Factory
     * Passcode.
     */

    status = MSS_SYS_otp_match(user_id, validator, one_time_passcode,
                                       MSS_SYS_MAILBOX_DATA_OFFSET,
                                       0u);

#ifdef MSS_SYS_INTERRUPT_MODE

    while(!g_message_interrupt);
    g_message_interrupt = 0u;
    status = MSS_SYS_read_response();

#endif
    if(MSS_SYS_SUCCESS == status)
    {
        MSS_UART_polled_tx_string(&g_mss_uart0_lo,
                (uint8_t*)"\r\n\nMATCH OTP success \r\n\n");
    }

    else if(MSS_SYS_MATCH_OTP_PROTOCOLERR == status)
    {
        MSS_UART_polled_tx_string (&g_mss_uart0_lo,
                (const uint8_t*)"\r\n Generate OTP service not executed or "
                                      "KEYMODE specified is invalid\r\n");
    }

    else if(MSS_SYS_MATCH_OTP_MISMATCHERR == status)
    {
        MSS_UART_polled_tx_string (&g_mss_uart0_lo,
                (const uint8_t*)"\r\n Calculated validator does not agree \r\n");
    }

    MSS_UART_polled_tx_string (&g_mss_uart0_lo, g_separator);
}

/******************************************************************************
 * Demonstrates terminate debug service
 */
void execute_terminate_debug_service(void)
{
    uint8_t status;

    MSS_UART_polled_tx_string(&g_mss_uart0_lo,
            (const uint8_t*)"\r\nDebug terminate service. \r\n\
Executing this service terminates the debug operation which was previously  \r\n\
requested using one of the debug services.\r\n" );

    status = MSS_SYS_debug_terminate(MSS_SYS_MAILBOX_DATA_OFFSET,
                                             0u);

#ifdef MSS_SYS_INTERRUPT_MODE

    while(!g_message_interrupt);
    g_message_interrupt = 0u;
    status = MSS_SYS_read_response();

#endif

    if(status == MSS_SYS_SUCCESS)
    {
        MSS_UART_polled_tx_string(&g_mss_uart0_lo ,
                (const uint8_t*)"\r\nTerminate debug success!!!\r\n");
    }

    MSS_UART_polled_tx_string (&g_mss_uart0_lo, g_separator);

}

static void mss_sys_service_interrupt_handler
(
    void
)
{
    g_message_interrupt = 1u;
}

/******************************************************************************
 * main function.
 *****************************************************************************/
void e51(void)
{
    uint8_t rx_buff[1];
    uint8_t rx_size = 0;
    uint32_t mpu_status;

    SYSREG->SOFT_RESET_CR &= ~( (1u << 0u) | (1u << 4u) | (1u << 5u) |
            (1u << 19u) | (1u << 23u) | (1u << 28u)) ;       // MMUART0

    MSS_UART_init(&g_mss_uart0_lo,
                  MSS_UART_115200_BAUD,
                  MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT
                  );

    /*Configuring MPU for spi copy service*/
    mpu_status = MSS_MPU_configure(MSS_MPU_SCB,
                                   MSS_MPU_PMP_REGION0,
                                   0x1400000000,
                                   0x7FFFFFFFF+1,
                                   MPU_MODE_READ_ACCESS | MPU_MODE_WRITE_ACCESS |
                                   MPU_MODE_EXEC_ACCESS,
                                   MSS_MPU_AM_NAPOT,
                                   0);

#ifdef MSS_SYS_INTERRUPT_MODE
    PLIC_init();
    __enable_irq();

    /*Message interrupt enable for service completion signal*/
    PLIC_EnableIRQ(g5c_MESSAGE_PLIC);
    PLIC_SetPriority(g5c_MESSAGE_PLIC, 2);

    /*Select service mode for service execution*/
    MSS_SYS_select_service_mode(MSS_SYS_SERVICE_INTERRUPT_MODE,
                                mss_sys_service_interrupt_handler);
#else
    /*Select service mode for service execution*/
    MSS_SYS_select_service_mode(MSS_SYS_SERVICE_POLLING_MODE,
                                mss_sys_service_interrupt_handler);
#endif

    display_greeting();

    while(1)
    {
        /* Start command line interface if any key is pressed. */
        rx_size = MSS_UART_get_rx(&g_mss_uart0_lo, rx_buff, sizeof(rx_buff));
        if(rx_size > 0)
        {
            MSS_UART_polled_tx_string(&g_mss_uart0_lo, g_separator);
            switch(rx_buff[0])
            {
            case '1':
                execute_serial_number_service();
            break;

            case '2':
                execute_generate_otp_service();
            break;

            case '3':
                execute_terminate_debug_service();
            break;

            case '4':
                execute_spi_copy_service();
            break;

            case '5':
                execute_match_otp_service();
            break;
            }
            display_greeting();
        }
    }
}

/*
 * Interrupt service routine
 */
uint8_t maintenance_u51_local_IRQHandler_0(void)
{
    /* Read MPU violation SR to know which MASTER violated.
     * When first bit is set, it means FIC0 caused violation.
     */
    if (SYSREG->MPU_VIOLATION_SR)
    {
        /*by writing 1, clear the status failed bit*/
        SYSREG->MPU_VIOLATION_SR |= 0x01;
    }

    return(0);
}

