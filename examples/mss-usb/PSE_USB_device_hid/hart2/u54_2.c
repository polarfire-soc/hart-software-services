/***********************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * code running on U54 second hart
 *
 * SVN $Revision: 9661 $
 * SVN $Date: 2018-01-15 10:43:33 +0000 (Mon, 15 Jan 2018) $
 */

#include "../pse_hal/pse_util.h"

volatile uint64_t dummy_h2 = 0;

void u54_2(void)
{

  volatile uint64_t * dummy_addr = (volatile uint64_t *)DDR_BASE;
  uint32_t hartid = read_csr(mhartid);

/* add code here */
  while(1);
};



