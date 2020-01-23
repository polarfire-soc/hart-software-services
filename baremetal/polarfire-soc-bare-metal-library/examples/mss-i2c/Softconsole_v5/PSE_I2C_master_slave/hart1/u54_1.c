/***********************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * code running on U54 first hart
 *
 * SVN $Revision: 10516 $
 * SVN $Date: 2018-11-08 18:09:23 +0000 (Thu, 08 Nov 2018) $
 */
 
#include "../mpfs_hal/mss_util.h"
#include "../mpfs_hal/mss_clint.h"
#include "../common/common.h"
#ifndef SIFIVE_HIFIVE_UNLEASHED
#include "../drivers/mss_uart/mss_uart.h"
#include "../mpfs_hal/mss_ints.h"
#else
#include "../drivers/FU540_uart/FU540_uart.h"
#endif

volatile uint32_t loop_count_h1 = 0U, count_sw_ints_h1 = 0U;
volatile uint64_t dummy_h1 = 0U;


void u54_1(void)
{
  int8_t info_string[100];
  uint64_t mcycle_start = 0U;
  uint64_t mcycle_end = 0U;
  uint64_t delta_mcycle = 0U;
  uint64_t num_loops = 100000U;
  uint32_t hartid = read_csr(mhartid);
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

  while(1)
  {

    mcycle_start = readmcycle();

    for(i = 0U; i < num_loops; i++)
    {
        dummy_h1 = i;
    }

    sprintf(info_string,"Hart %ld, %ld delta_mcycle \n\r", hartid, delta_mcycle);
    mss_take_mutex((uint64_t)&uart_lock);
	MSS_UART_polled_tx(&g_mss_uart0_lo, info_string,strlen(info_string));
	mss_release_mutex((uint64_t)&uart_lock);

    hartid 			= read_csr(mhartid);
    mcycle_end 		= readmcycle();
    delta_mcycle	= mcycle_end - mcycle_start;

    loop_count_h1++;
  }

  /* never return */
}



void Software_h1_IRQHandler(void)
{
	uint32_t hart_id = read_csr(mhartid);
	if(hart_id == 0U)
	{
		count_sw_ints_h1++;
	}

}
