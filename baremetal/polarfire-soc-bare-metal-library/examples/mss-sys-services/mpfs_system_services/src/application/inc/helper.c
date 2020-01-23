/*******************************************************************************
 * (c) Copyright 2017 Microsemi SoC Products Group.  All rights reserved.
 *
 * Helper function for PolarFire User Crypto- Cryptography service example.
 *
 * SVN $Revision: 9604 $
 * SVN $Date: 2017-11-30 10:48:06 +0530 (Thu, 30 Nov 2017) $
 */
#include "drivers/mss_uart/mss_uart.h"
#include "inc/helper.h"

static const uint8_t g_separator[] =
"\r\n\
------------------------------------------------------------------------------";

extern mss_uart_instance_t g_mss_uart0_lo;

/*==============================================================================
  Function to clear local variable and array.
 */
static void clear_variable(uint8_t *p_var, uint16_t size)
{
    uint16_t inc;
    
    for(inc = 0; inc < size; inc++)
    {
        *p_var = 0x00;
        p_var++;
    }
}

/*==============================================================================
  Function to get the input data from user.
 */
uint16_t get_input_data
(
    uint8_t* location,
    uint16_t size,
    const uint8_t* msg,
    uint16_t msg_size
)
{
    uint16_t count = 0u;
    
    /* Clear the memory location. */
    clear_variable(location, size);

    /* Read data from UART terminal. */
    count = get_data_from_uart(location, size, msg, msg_size);

    return count;
}

/*==============================================================================
  Function to get the key from user.
 */
void get_key
(
    uint8_t key_type,
    uint8_t* location,
    uint8_t size,
    const uint8_t* msg,
    uint8_t msg_size
)
{
    uint8_t status = 0u;
    const uint8_t invalid_ms[] = "\r\n Invalid key type. ";
    
    if(status == VALID)
    {
        /* Read the 16 bytes of input data from UART terminal. */
        get_input_data(location, size, msg, msg_size);
    }
    else
    {
    	MSS_UART_polled_tx(&g_mss_uart0_lo, invalid_ms, sizeof( invalid_ms));
    }
}

/*==============================================================================
  Convert ASCII value to hex value.
 */
uint8_t convert_ascii_to_hex(uint8_t* dest, const uint8_t* src)
{
    uint8_t error_flag = 0u;

	 if((*src >= '0') && (*src <= '9'))
	{
		*dest = (*src - '0');
	}
	else if((*src >= 'a') && (*src <= 'f'))
	{
		*dest = (*src - 'a') + 10u;
	}
	else if((*src >= 'A') && (*src <= 'F'))
	{
		*dest =  (*src - 'A') + 10u;
	}
	else if(*src != 0x00u)
	{
		MSS_UART_polled_tx(&g_mss_uart0_lo, (const uint8_t *)"\r\n Invalid data.", sizeof("\r\n Invalid data."));
		error_flag = 1u;
	}
	 return error_flag;
}

/*==============================================================================
  Validate the input hex value .
 */
uint8_t validate_input(uint8_t ascii_input)
{
    uint8_t valid_key = 0u;
    
    if(((ascii_input >= 'A') && (ascii_input <= 'F')) ||        \
       ((ascii_input >= 'a') && (ascii_input <= 'f')) ||        \
       ((ascii_input >= '0') && (ascii_input <= '9')))
    {
        valid_key = 1u;
    }
    else
    {
        valid_key = 0u;
    }
    return valid_key;
}

const uint8_t hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                             '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

/*==============================================================================
  Display content of buffer passed as parameter as hex values.
 */
void display_output
(
    uint8_t* in_buffer,
    uint32_t byte_length
)
{
    uint32_t inc;
    uint8_t byte = 0;
    
    MSS_UART_polled_tx(&g_mss_uart0_lo, (const uint8_t*)" ", sizeof(" "));
    for(inc = 0; inc < byte_length; ++inc)
    {
        if((inc > 1u) &&(0u == (inc % 16u)))
        {
            MSS_UART_polled_tx(&g_mss_uart0_lo, (const uint8_t*)"\r\n ", sizeof("\r\n "));
        }
        
        byte = in_buffer[inc];
        MSS_UART_polled_tx(&g_mss_uart0_lo, &hex_chars[((byte & 0xF0) >> 4) ], 1);
        MSS_UART_polled_tx(&g_mss_uart0_lo, &hex_chars[(byte & 0x0F)], 1);
        MSS_UART_polled_tx(&g_mss_uart0_lo, (const uint8_t*)" ", sizeof(" "));
    }

}

/*==============================================================================
  Function to read data from UART terminal and stored it.
 */
uint16_t get_data_from_uart
(
    uint8_t* src_ptr,
    uint16_t size,
    const uint8_t* msg,
    uint16_t msg_size
)
{
    uint8_t complete = 0u;
    uint8_t rx_buff[1];
    uint8_t rx_size = 0u;
    uint16_t count = 0u;
    uint16_t ret_size = 0u;
    uint8_t first = 0u;
    uint16_t src_ind = 0u;
    uint8_t prev = 0;
    uint8_t curr = 0;
    uint8_t temp = 0;
    uint8_t next_byte = 0;
    uint16_t read_data_size = 0;

    MSS_UART_polled_tx(&g_mss_uart0_lo, g_separator, sizeof(g_separator));
    MSS_UART_polled_tx(&g_mss_uart0_lo, msg, msg_size);

    if(size != 1)
    {
    	read_data_size = size * 2;
    }
    else
    {
    	read_data_size = size;
    }
    
    /* Read the key size sent by user and store it. */
    count = 0u;
    while(!complete)
    {
        rx_size = MSS_UART_get_rx(&g_mss_uart0_lo, rx_buff, sizeof(rx_buff));
        if(rx_size > 0u)
        {
            /* Is it to terminate from the loop */
            if(ENTER == rx_buff[0])
            {
                complete = 1u;
            }
            /* Is entered key valid */
            else if(validate_input(rx_buff[0]) != 1u)
            {
                MSS_UART_polled_tx(&g_mss_uart0_lo, rx_buff, sizeof(rx_buff));
                MSS_UART_polled_tx(&g_mss_uart0_lo, (const uint8_t *)"\r\n Invalid input.",
                          sizeof("\r\n Invalid input."));
                MSS_UART_polled_tx(&g_mss_uart0_lo, msg, msg_size);
                complete = 0u;
                count = 0u;
                first = 0u;
                clear_variable(src_ptr, 4);
            }
            else
            {
            	if(next_byte == 0)
            	{
            		convert_ascii_to_hex(&src_ptr[src_ind], &rx_buff[0]);
            		prev = src_ptr[src_ind];
            		next_byte = 1;
            	}
            	else
            	{
            		convert_ascii_to_hex(&curr, &rx_buff[0]);
            		temp = ((prev << 4) & 0xF0);
            		src_ptr[src_ind] = (temp | curr);
            		next_byte = 0;
            		src_ind++;
            	}

                
                /* Switching to next line after every 8 bytes */
                if(((count % 32u) == 0x00u) && (count > 0x00u) && (complete != 0x01u))
                {
                    MSS_UART_polled_tx(&g_mss_uart0_lo, (const uint8_t *)"\n\r", sizeof("\n\r"));
                    first = 0u;
                }

                if(first == 0u)
                {
                    MSS_UART_polled_tx(&g_mss_uart0_lo, (const uint8_t *)" ", sizeof(" "));
                    first++;
                }
                MSS_UART_polled_tx(&g_mss_uart0_lo, rx_buff, sizeof(rx_buff));
                count++;
                if(read_data_size == count)
                {
                   complete = 1u;
                }
            }
        }
    }
    
    if((count%2) == 0)
    {
    	ret_size = count/2;
    }
    else
    {
    	if(size!=1)
    	{
    		temp = src_ptr[src_ind];
    		src_ptr[src_ind] = ((temp << 4) & 0xF0);

    		ret_size = (count/2)+1;
    	}
    	else
    	{
    		ret_size = 1;
    	}
    }

    return ret_size;
}

/*==============================================================================
  Function to get the key from user.
 */
uint8_t enable_dma
(
    const uint8_t* msg,
    uint8_t msg_size
)
{
    volatile uint8_t invalid_ip = 1u;
    uint8_t dma_enable = 0;
    
    const uint8_t invalid_ms[] = "\r\n Invalid input. ";
    
    while(invalid_ip != 0)
    {
        /* Read the 16 bytes of input data from UART terminal. */
        get_input_data(&dma_enable, 1, msg, msg_size);
    
        if(dma_enable >= 2)
        {
            MSS_UART_polled_tx(&g_mss_uart0_lo, invalid_ms, sizeof( invalid_ms));
        }
        else
        {
          invalid_ip = 0;
        }
    }
    
    return dma_enable;
}
