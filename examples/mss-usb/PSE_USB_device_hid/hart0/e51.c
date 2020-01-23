/***********************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * code running on e51
 *
 * SVN $Revision: 9661 $
 * SVN $Date: 2018-01-15 10:43:33 +0000 (Mon, 15 Jan 2018) $
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#if PSE
#include "pse_hal.h"
#include "mss_uart.h"
#include "mouse_app.h"
#include "mss_usb_std_def.h"
#include "mss_mpu.h"

#else
#include "../drivers/FU540_uart/FU540_uart.h"
#endif


#ifdef TEST_SW_INT
volatile uint32_t count_sw_ints_h0 = 0;
extern uint32_t count_sw_ints_h1;
#endif

mss_uart_instance_t *g_uart;

/*Global variables*/
uint8_t volatile g_hid_driver_ready = 0u;
uint8_t volatile g_hid_driver_released = 0u;

const uint8_t hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                             '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

volatile uint8_t mouse_update = 0;

typedef struct _HID_MOUSE_Data
{
  uint8_t              x;
  uint8_t              y;
  uint8_t              z;               /* Not Supported */
  uint8_t              button;
}
HID_MOUSE_Data_TypeDef;

HID_MOUSE_Data_TypeDef HID_MOUSE_Data;


static void update_mouse(void)
{
     mouse_update = 1;
}


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
    MSS_UART_polled_tx(g_uart, hex_value_msg, sizeof(hex_value_msg));
    MSS_UART_polled_tx_string(g_uart, (uint8_t *)" ");
}

#define SYS_TICK_LOAD_VALUE                             25000u  /*For 1ms*/
/**
 *
 */
void e51(void)
{
	init_memory();
#if SI_FIVE_UNLEASED
	/*
	 * You should see the following displayed at the terminal window:
Hart 0, 1048576 loops required 423849767 cycles, sw ints h0 = 0, sw ints h1 = 0, mtime = 1 mtimecmp = 1275256922
loop_count_h1 inc
sw int hart0
loop_count_h1 inc
loop_count_h1 inc
loop_count_h1 inc
loop_count_h1 inc
sw int hart1
loop_count_h1 inc
loop_count_h1 inc
loop_count_h1 inc
loop_count_h1 inc
UART interrupt working
loop_count_h1 inc
loop_count_h1 inc
loop_count_h1 inc
loop_count_h1 inc
loop_count_h1 inc
loop_count_h1 inc
loop_count_h1 inc
loop_count_h1 inc
loop_count_h1 inc
Hart 0, 1048576 loops required 580897991 cycles, sw ints h0 = 0, sw ints h1 = 0, mtime = 1 mtimecmp = 1275256922
	 */
	volatile uint32_t i=0;
	uint64_t mcycle_start = 0;
	uint64_t mcycle_end = 0;
	uint64_t delta_mcycle = 0;
	uint32_t num_loops = 0x100000;
	uint32_t hartid = read_csr(mhartid);
	static volatile uint64_t dummy_h0 = 0;
    uint8_t rx_buff[1];
    uint32_t rx_idx  = 0;
    uint8_t rx_size = 0;
    uint8_t info_string[100];


	MSS_FU540_UART_init(&g_mss_FU540_uart0,
			  0,
			  0); /* note- with current boot-loader, serial port baud = 57400 */
	PLIC_SetPriority(USART0_PLIC_4, 1);

	/* Enable UART Interrupt on PLIC */
	PLIC_EnableIRQ(USART0_PLIC_4);

	/*
	 * Start the first U54
	 */
	raise_soft_interrupt((uint32_t)1);

	i=0;
	while(1)
	{
		/* add your code here */
		i++;				/* added some code as debugger hangs if in loop doing nothing */
		if(i == (num_loops >> 0))
		{
			hartid 			= read_csr(mhartid);
			mcycle_end 		= readmcycle();
			delta_mcycle	 = mcycle_end - mcycle_start;

			sprintf(info_string,"Hart %d, %d loops required %ld cycles, sw ints h0 = %d, sw ints h1 = %d, mtime = %d mtimecmp = %d\n\r", hartid,
			num_loops, delta_mcycle, count_sw_ints_h0, count_sw_ints_h1, CLINT->MTIME, CLINT->MTIMECMP[0]);
			MSS_FU540_UART_polled_tx(&g_mss_FU540_uart0, info_string,strlen(info_string));
			raise_soft_interrupt((uint32_t)0);
			i = 0;
		}
		else if(i == (num_loops >> 1))
		{
			MSS_FU540_UART_interrupt_tx(&g_mss_FU540_uart0, "UART interrupt working\n\r",sizeof("UART interrupt working\n\r"));
		}
		else if(i == (num_loops >> 2))
		{
			raise_soft_interrupt((uint32_t)1);
		}
		if(count_sw_ints_h0)
		{
			MSS_FU540_UART_polled_tx(&g_mss_FU540_uart0, "sw int hart0\n\r",sizeof("sw int hart0\n\r"));
			count_sw_ints_h0 = 0;
		}
		if(count_sw_ints_h1)
		{
			MSS_FU540_UART_polled_tx(&g_mss_FU540_uart0, "sw int hart1\n\r",sizeof("sw int hart1\n\r"));
			count_sw_ints_h1 = 0;
		}
		if(loop_count_h1)
		{
			/*
			 * fixme: this iss not working, sw int hart1
			 */
			MSS_FU540_UART_polled_tx(&g_mss_FU540_uart0, "loop_count_h1 inc\n\r",sizeof("loop_count_h1 inc\n\r"));
			loop_count_h1 = 0;
		}
	}

#else
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

    SYSREG->SOFT_RESET_CR |= (1u << 16u);

    for(int i=0; i< 100000; i++);

    SYSREG->SOFT_RESET_CR &= ~( (1u << 16u) |(1u << 0u) | (1u << 4u) | (1u << 5u) | (1u << 19u) | (1u << 23u) | (1u << 28u)) ;       // MMUART0

    SYSREG->IOMUX0_CR = 0xfffffe7f;  // connect MMUART0 to GPIO, QSPI to pads
    SYSREG->IOMUX1_CR = 0x05500000;  // pad5,6 = mux 5 (mmuart 0)

    // IOMUX configurations to allow QSPI pins to the pads
    SYSREG->IOMUX2_CR = 0;
    SYSREG->IOMUX3_CR = 0;
    SYSREG->IOMUX4_CR = 0;
    SYSREG->IOMUX5_CR = 0;

    loop_count = MSS_MPU_configure(MSS_MPU_MMC,
    							   MSS_MPU_PMP_REGION1,
								   0x08000000u,
                                   0x200000,
								   MPU_MODE_READ_ACCESS | MPU_MODE_WRITE_ACCESS | MPU_MODE_EXEC_ACCESS,
								   MSS_MPU_AM_NAPOT, //MSS_MPU_AM_OFF,//MSS_MPU_AM_NAPOT,
                                   0);
								   
/*****************************************************************************************/
#if TEST_H2F_CONTROLLER
    /*GPIO2,MSS_GPIO_1  is loop-backed in the design from H2F UART signal */
    MSS_GPIO_init(GPIO2_LO);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_1, MSS_GPIO_INPUT_MODE);
#endif
    PLIC_init();
  //  __disable_local_irq((int8_t)MMUART0_U51_INT);
    __enable_irq();

    MOUSE_init();
    PLIC_SetPriority(USB_DMA_PLIC, 2);
    PLIC_SetPriority(USB_MC_PLIC, 2);

    g_uart = &g_mss_uart0_lo;
    MSS_UART_init( g_uart,
                   MSS_UART_115200_BAUD,
                   MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT,
				   MSS_UART0_INTR_PLIC);

  //  MSS_UART_irq_tx(&g_mss_uart0_lo, "PolarFire MSS UART Interrupt TX\n\r", sizeof("PolarFire MSS UART Interrupt TX\n\r"));

    MSS_UART_polled_tx_string (g_uart, "\n\rUSB HID Host Example. Displaying mouse co-ordinates on UART port\n\r");


#if	TEST_H2F_CONTROLLER
    /* enable output to fabric, we will read GPIO in interrupt and check if H2F output is high as expected */
    /* Note: We need to have H2F output looped back to GPIO2 in the hardware design  */
    enable_h2f_int_output(MMUART0_PLIC_77);
#endif
//    __enable_irq();

    /*
     * Startup the other harts
     */
#ifdef TEST_SW_INT
    raise_soft_interrupt((uint32_t)1); /* get hart1 out of wfi */
#endif

    while(1)
    {
        MOUSE_task();
#ifdef TEST_SW_INT
    	raise_soft_interrupt((uint32_t)1);
#endif
    }
#endif
}

volatile uint32_t g_cnt=0;
void SysTick_Handler(void)
{
    /*
    This function must be called. This function provides the time stamp
    which is used to keep track of activities such as USB Reset, Settling time etc.
    in Enumeration process.
    The USBH driver and USBH-MSC class driver housekeeping task is also run based
    on this timestamp.

    This avoids the application to wait for the enumeration process to complete
    which might take time in 100s of miliseconds depending on target device.

    You may choose a general purpose timer instead of Systick.
    */
    g_cnt++;
}

