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
#include "mss_spi.h"
#if PSE
#include "pse_hal.h"
#include "mss_uart.h"
#include "mss_mpu.h"
#include "mss_usb_std_def.h"
#include "flash_drive_app.h"
#include "mss_usb_device_msd.h"
#else
#include "../drivers/FU540_uart/FU540_uart.h"
#endif


#ifdef TEST_SW_INT
volatile uint32_t count_sw_ints_h0 = 0;
extern uint32_t count_sw_ints_h1;
#endif

volatile uint8_t* p_USB = 0x20201000;
volatile uint32_t* p1_USB = 0x20201218;
volatile uint32_t Channel_1_Addr,Channel_2_Addr, count_channl_1, count_channl_2;
volatile uint8_t a_USB[31] = {0x00};
mss_uart_instance_t *g_uart;

#define SYS_TICK_LOAD_VALUE                             25000u  /*For 1ms*/
/**
 *
 */
volatile uint32_t* g_ptr;
void e51(void)
{
    volatile uint32_t i;
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
    volatile uint8_t* temp;

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

    SYSREG->mpu_violation_inten_cr = 0x1FF; //Enable MPU violation interrupts for all 9 masters.
    loop_count = MSS_MPU_configure(MSS_MPU_MMC,
                                   MSS_MPU_PMP_REGION1,
                                   0x08000000u,
                                   0x200000,
                                   MPU_MODE_READ_ACCESS | MPU_MODE_WRITE_ACCESS | MPU_MODE_EXEC_ACCESS,
                                   MSS_MPU_AM_NAPOT,
                                   0);

    loop_count = MSS_MPU_configure(MSS_MPU_MMC,
        							   MSS_MPU_PMP_REGION2,
									   0x081C2000,
                                       0x200000,
    								   MPU_MODE_READ_ACCESS | MPU_MODE_WRITE_ACCESS | MPU_MODE_EXEC_ACCESS,
    								   MSS_MPU_AM_NAPOT,
                                       0);
    PLIC_init();

    PLIC_SetPriority(USB_DMA_PLIC, 2);
    PLIC_SetPriority(USB_MC_PLIC, 2);

    FLASH_DRIVE_init();
    __enable_irq();


    while(1)
    {
        ;
    }
}

