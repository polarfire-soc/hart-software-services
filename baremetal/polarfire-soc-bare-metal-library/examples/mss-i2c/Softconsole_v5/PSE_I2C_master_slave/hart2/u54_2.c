/***********************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * code running on U54 second hart
 *
 * SVN $Revision: 10516 $
 * SVN $Date: 2018-11-08 18:09:23 +0000 (Thu, 08 Nov 2018) $
 */
#include "../mpfs_hal/mss_util.h"
#include "../mpfs_hal/mss_clint.h"
#include "../common/common.h"
#ifndef SIFIVE_HIFIVE_UNLEASHED
#include "../drivers/mss_uart/mss_uart.h"
#else
#include "../drivers/FU540_uart/FU540_uart.h"
#endif

volatile uint64_t dummy_h2 = 0;
volatile uint32_t loop_count_h2 = 0U;

/**
 *
 */
void u54_2(void)
{
	volatile uint64_t * dummy_addr = (volatile uint64_t *)DDR_BASE;
	uint32_t hartid = read_csr(mhartid);
	int8_t info_string[100];
    volatile uint32_t   i = 0U;
    volatile uint32_t   test = 0U;

    clear_soft_interrupt();
    while(( test & MIP_MSIP) == 0U)
    {
        __asm("nop");
        __asm("nop");
        __asm("wfi");          /* wait for interprocess from hart0 to wake up */
        test = read_csr(mip);
    }

	while(1U)
	{
		i++;
		if(i == 0x100000U)
		{
			i = 0U;
			loop_count_h2++;
			sprintf(info_string,"Hart %d\n\r", hartid);
			mss_take_mutex((uint64_t)&uart_lock);
			MSS_UART_polled_tx(&g_mss_uart0_lo, info_string,strlen(info_string));
			mss_release_mutex((uint64_t)&uart_lock);
		}

	}
};





