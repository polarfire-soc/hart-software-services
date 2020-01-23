/***********************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
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
#include <ctype.h>
#include <inttypes.h>
#include "mpfs_hal/mss_mpu.h"

#include "mpfs_hal/mss_hal.h"

#if PSE
#include "drivers/mss_gpio/mss_gpio.h"
#include "drivers/mss_uart/mss_uart.h"
#else
#include "drivers/FU540_uart/FU540_uart.h"
#endif

#include "mpfs_hal/mss_plic.h"
#include "config/hardware/hw_platform.h"

#include "drivers/mss_mac/mss_ethernet_registers.h"
#include "drivers/mss_mac/mss_ethernet_mac_user_config.h"
#include "drivers/mss_mac/mss_ethernet_mac_regs.h"
#include "drivers/mss_mac/mss_ethernet_mac.h"

#include "drivers/mss_mac/phy.h"

#include "ptp_packets.h"
#if defined(MSS_MAC_USE_DDR) && (MSS_MAC_USE_DDR == MSS_MAC_MEM_CRYPTO)
/*
 * The crypto libraries have been removed from the example projects as they are
 * only required for internal testing. If you want to test this operation you
 * will need to add them back to the project for it to build correctly.
 */

#include "calini.h"
#endif

#define PRINT_STRING(x) MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t *)x);

#define FORCE_10M

/* Lots of references for debug data from MMAC/PHY/SGMII core etc */

#if defined(TARGET_G5_SOC)
extern uint16_t VSC8575_reg_0[32];
extern uint16_t VSC8575_reg_1[16];
extern uint16_t VSC8575_reg_2[16];
extern uint16_t VSC8575_reg_3[20];
extern uint16_t VSC8575_reg_4[16];
extern uint16_t VSC8575_reg_16[32];
extern uint16_t VSC8575_MSS_SGMII_reg16[17];
extern uint32_t VSC8575_MSS_MAC_reg[80];
extern uint32_t VSC8575_MSS_PLIC_REG[80];

extern uint16_t TI_reg_0[32];
extern uint16_t TI_reg_1[25];
extern uint16_t TI_MSS_SGMII_reg[17];

void dump_vsc8575_regs(mss_mac_instance_t * const this_mac);
void dump_ti_regs(mss_mac_instance_t * this_mac);
#endif

/*
 * Align these on an 8 byte boundary as we might be using IEEE 1588 time
 * stamping and that uses b2 of the buffer pointer to indicate that a time stamp
 * is present in this descriptor.
 */
#if !defined(MSS_MAC_SIMPLE_TX_QUEUE)
static uint8_t  g_mac_tx_buffer0[MSS_MAC_TX_RING_SIZE][MSS_MAC_MAX_TX_BUF_SIZE]
                                                  __attribute__ ((aligned (8)));
static uint8_t  g_mac_tx_buffer1[MSS_MAC_TX_RING_SIZE][MSS_MAC_MAX_TX_BUF_SIZE]
                                                  __attribute__ ((aligned (8)));
#endif

#if defined(MSS_MAC_USE_DDR)
static uint8_t *g_mac_rx_buffer0_0 = 0;
static uint8_t *g_mac_rx_buffer1_0 = 0;

#if (MSS_MAC_QUEUE_COUNT >= 2)
static uint8_t *g_mac_rx_buffer0_1 = 0;
static uint8_t *g_mac_rx_buffer1_1 = 0;
#endif

#if (MSS_MAC_QUEUE_COUNT >= 3)
static uint8_t *g_mac_rx_buffer0_2 = 0;
static uint8_t *g_mac_rx_buffer1_2 = 0;
#endif

#if (MSS_MAC_QUEUE_COUNT == 4)
static uint8_t *g_mac_rx_buffer0_3 = 0;
static uint8_t *g_mac_rx_buffer1_3 = 0;
#endif

#else /* defined(MSS_MAC_USE_DDR) */

static uint8_t g_mac_rx_buffer0_0[MSS_MAC_RX_RING_SIZE][MSS_MAC_MAX_RX_BUF_SIZE]
                                                  __attribute__ ((aligned (8)));
static uint8_t g_mac_rx_buffer1_0[MSS_MAC_RX_RING_SIZE][MSS_MAC_MAX_RX_BUF_SIZE]
                                                  __attribute__ ((aligned (8)));

#if (MSS_MAC_QUEUE_COUNT >= 2)
static uint8_t g_mac_rx_buffer0_1[MSS_MAC_RX_RING_SIZE][MSS_MAC_MAX_RX_BUF_SIZE]
                                                  __attribute__ ((aligned (8)));
static uint8_t g_mac_rx_buffer1_1[MSS_MAC_RX_RING_SIZE][MSS_MAC_MAX_RX_BUF_SIZE]
                                                  __attribute__ ((aligned (8)));
#endif

#if (MSS_MAC_QUEUE_COUNT >= 3)
static uint8_t g_mac_rx_buffer0_2[MSS_MAC_RX_RING_SIZE][MSS_MAC_MAX_RX_BUF_SIZE]
                                                  __attribute__ ((aligned (8)));
static uint8_t g_mac_rx_buffer1_2[MSS_MAC_RX_RING_SIZE][MSS_MAC_MAX_RX_BUF_SIZE]
                                                  __attribute__ ((aligned (8)));
#endif

#if (MSS_MAC_QUEUE_COUNT == 4)
static uint8_t g_mac_rx_buffer0_3[MSS_MAC_RX_RING_SIZE][MSS_MAC_MAX_RX_BUF_SIZE]
                                                  __attribute__ ((aligned (8)));
static uint8_t g_mac_rx_buffer1_3[MSS_MAC_RX_RING_SIZE][MSS_MAC_MAX_RX_BUF_SIZE]
                                                  __attribute__ ((aligned (8)));
#endif

/* Single queue for eMAC */
static uint8_t g_emac_rx_buffer0_0[MSS_MAC_RX_RING_SIZE][MSS_MAC_MAX_RX_BUF_SIZE]
                                                   __attribute__ ((aligned (8)));
static uint8_t g_emac_rx_buffer1_0[MSS_MAC_RX_RING_SIZE][MSS_MAC_MAX_RX_BUF_SIZE]
                                                   __attribute__ ((aligned (8)));

#endif /* defined(MSS_MAC_USE_DDR) */

mss_mac_cfg_t g_mac_config;

/*==============================================================================
 * Network configuration globals.
 */


#define TEST_SW_INT 1

#ifdef TEST_SW_INT
volatile uint32_t count_sw_ints_h0 = 0;
extern uint32_t count_sw_ints_h1;
extern uint32_t loop_count_h1;
#endif
extern void init_memory( void);

typedef struct aligned_tx_buf
{
    uint64_t aligner;
    uint8_t packet[MSS_MAC_MAX_PACKET_SIZE];
} ALIGNED_TX_BUF;


ALIGNED_TX_BUF tx_packet0;
ALIGNED_TX_BUF tx_packet1;

uint8_t tx_packet_data[128] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFC,
                               0x00, 0x12, 0x34, 0x56, 0x08, 0x06, 0x00, 0x01,
                               0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0xFC, 0x00,
                               0x12, 0x34, 0x56, 0x0A, 0x02, 0x02, 0x02, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x02,
                               0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint8_t tx_pak_arp[128] =    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFC,
                               0x00, 0x12, 0x34, 0x56, 0x08, 0x06, 0x00, 0x01,
                               0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0xFC, 0x00,
                               0x12, 0x34, 0x56, 0x0A, 0x02, 0x02, 0x02, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x02,
                               0x02, 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static volatile int tx_count0  = 0;
static volatile int tx_count1  = 0;
static volatile int tx_count0e = 0;
static volatile int tx_count1e = 0;

volatile uint32_t g_crc   = 0;    /* CRC pass through control */
volatile int g_loopback0  = 0;    /* Software loopback control */
volatile int g_loopback1  = 0;    /* Software loopback control */
volatile int g_loopback0e = 0;    /* Software loopback control */
volatile int g_loopback1e = 0;    /* Software loopback control */
volatile int g_phy_dump   = 0;    /* PHY Register dump control */
volatile int g_tx_add_1   = 0;    /* Tx length adjustment control to make
                                     loopback packets more visible... */
volatile int g_tx_adjust  = 1;    /* Adjustment to make to the packet length
                                     when enabled by 'n' */
volatile int g_address_swap = 0; /* Control MAC address and IP assignment to
                                    allow for single board GEM0-GEM1 or dual
                                    board GEM0-GEM0 hook up */

/* Receive packet capture variables */
#define PACKET_IDLE      0
#define PACKET_ARMED     1
#define PACKET_ARMED_PTP 2
#define PACKET_DONE      3

#define PACKET_MAX   16384

uint8_t      g_packet_data0[PACKET_MAX];
volatile int g_capture0        = PACKET_IDLE;
volatile int g_packet_length0  = 0;
volatile int g_reload0         = PACKET_IDLE;
volatile int g_queue_no_0      = 0;

uint8_t      g_packet_data0e[PACKET_MAX];
volatile int g_capture0e       = PACKET_IDLE;
volatile int g_packet_length0e = 0;
volatile int g_reload0e        = PACKET_IDLE;
volatile int g_queue_no_0e     = 0;

uint8_t      g_packet_data1[PACKET_MAX];
volatile int g_capture1        = PACKET_IDLE;
volatile int g_packet_length1  = 0;
volatile int g_reload1         = PACKET_IDLE;
volatile int g_queue_no_1      = 0;

uint8_t      g_packet_data1e[PACKET_MAX];
volatile int g_capture1e       = PACKET_IDLE;
volatile int g_packet_length1e = 0;
volatile int g_reload1e        = PACKET_IDLE;
volatile int g_queue_no_1e     = 0;


volatile int g_tx_retry0  = 0;
volatile int g_tx_retry1  = 0;
volatile int g_tx_retry0e = 0;
volatile int g_tx_retry1e = 0;

volatile int g_vlan_tags0  = 0;
volatile int g_vlan_tags1  = 0;
volatile int g_vlan_tags0e = 0;
volatile int g_vlan_tags1e = 0;

volatile int g_hash_index0      = 0;
volatile int g_hash_direction0  = 0;
volatile int g_hash_index1      = 0;
volatile int g_hash_direction1  = 0;
volatile int g_hash_index0e     = 0;
volatile int g_hash_direction0e = 0;
volatile int g_hash_index1e     = 0;
volatile int g_hash_direction1e = 0;

/* Descriptors associated with last tx */
mss_mac_tx_desc_t g_tx_desc0;
mss_mac_tx_desc_t g_tx_desc1;
mss_mac_tx_desc_t g_tx_desc0e;
mss_mac_tx_desc_t g_tx_desc1e;

uint32_t g_tx_ts_count0  = 0;
uint32_t g_tx_ts_count1  = 0;
uint32_t g_tx_ts_count0e = 0;
uint32_t g_tx_ts_count1e = 0;

/* Descriptors associated with last captured rx */
mss_mac_rx_desc_t g_rx_desc0;
mss_mac_rx_desc_t g_rx_desc1;
mss_mac_rx_desc_t g_rx_desc0e;
mss_mac_rx_desc_t g_rx_desc1e;

uint32_t g_rx_ts_count0  = 0;
uint32_t g_rx_ts_count1  = 0;
uint32_t g_rx_ts_count0e = 0;
uint32_t g_rx_ts_count1e = 0;

volatile int g_link_status = 0;

/* Choose which pmac to use */

//mss_mac_instance_t *g_test_mac = &g_mac1;
mss_mac_instance_t *g_test_mac = &g_mac0; /* Start out with MAC 0 */

/* Define this if you are using the other harts... */
/* #define USE_OTHER_HARTS */

int main_first_hart(void)
{
    uint64_t hartid = read_csr(mhartid);
    HLS_DATA* hls;

    if(hartid == MPFS_HAL_FIRST_HART)
    {
#if defined(USE_OTHER_HARTS)
        int hard_idx;
#endif
        init_memory();
        init_bus_error_unit();
        init_mem_protection_unit();

#if defined(USE_OTHER_HARTS)
        /*
         * Start the other harts. They are put in wfi in entry.S
         * When debugging, harts are released from reset separately,
         * so we need to make sure hart is in wfi before we try and release.
        */
        WFI_SM sm_check_thread = INIT_THREAD_PR;
        hard_idx = MPFS_HAL_FIRST_HART + 1U;
        while( hard_idx <= MPFS_HAL_LAST_HART)
        {
            uint32_t wait_count;

            switch(sm_check_thread)
            {
                case INIT_THREAD_PR:
                    hls = (HLS_DATA*)((uint8_t *)&__stack_bottom_h1$
                            + (((uint8_t *)&__stack_top_h1$ -
                                    (uint8_t *)&__stack_bottom_h1$) * hard_idx)
                                - (uint8_t *)(HLS_DEBUG_AREA_SIZE));
                    sm_check_thread = CHECK_WFI;
                    wait_count = 0U;
                    break;

                case CHECK_WFI:
                    if( hls->in_wfi_indicator == HLS_DATA_IN_WFI )
                    {
                        /* Separate state- to add a little delay */
                        sm_check_thread = SEND_WFI;
                    }
                    break;

                case SEND_WFI:
                    raise_soft_interrupt(hard_idx);
                    sm_check_thread = CHECK_WAKE;
                    wait_count = 0UL;
                    break;

                case CHECK_WAKE:
                    if( hls->in_wfi_indicator == HLS_DATA_PASSED_WFI )
                    {
                        sm_check_thread = INIT_THREAD_PR;
                        hard_idx++;
                        wait_count = 0UL;
                    }
                    else
                    {
                        wait_count++;
                        if(wait_count > 0x10U)
                        {
                            if( hls->in_wfi_indicator == HLS_DATA_IN_WFI )
                            {
                                raise_soft_interrupt(hard_idx);
                                wait_count = 0UL;
                            }
                        }
                    }
                    break;
            }
        }
#endif

#if PSE
        SYSREG->SUBBLK_CLOCK_CR = 0xffffffff;       /* all clocks on */
#if defined(G5_SOC_EMU_USE_GEM0)
        MSS_MPU_configure(MSS_MPU_GEM0, MSS_MPU_PMP_REGION0,
                0x0000000000000000ULL, 0x0000000100000000ULL,
                MPU_MODE_READ_ACCESS, MSS_MPU_AM_NAPOT, 0);

        MSS_MPU_configure(MSS_MPU_GEM0, MSS_MPU_PMP_REGION1,
                0x0000000008000000ULL, 0x0000000000200000ULL,
                MPU_MODE_READ_ACCESS | MPU_MODE_WRITE_ACCESS, MSS_MPU_AM_NAPOT, 0);

#if defined(MSS_MAC_USE_DDR)
#if MSS_MAC_USE_DDR == MSS_MAC_MEM_DDR
        MSS_MPU_configure(MSS_MPU_GEM0, MSS_MPU_PMP_REGION2,
                0x00000000C0000000ULL, 0x0000000000200000ULL,
                MPU_MODE_READ_ACCESS | MPU_MODE_WRITE_ACCESS, MSS_MPU_AM_NAPOT, 0);
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_FIC0
        MSS_MPU_configure(MSS_MPU_GEM0, MSS_MPU_PMP_REGION2,
                0x0000000060000000ULL, 0x0000000000200000ULL,
                MPU_MODE_READ_ACCESS | MPU_MODE_WRITE_ACCESS, MSS_MPU_AM_NAPOT, 0);
        MPU[4].CFG[2].pmp = MPU_CFG(0x60000000, 21u);
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_FIC1
        MSS_MPU_configure(MSS_MPU_GEM0, MSS_MPU_PMP_REGION2,
                0x00000000E0000000ULL, 0x0000000000200000ULL,
                MPU_MODE_READ_ACCESS | MPU_MODE_WRITE_ACCESS, MSS_MPU_AM_NAPOT, 0);
        MPU[4].CFG[2].pmp = MPU_CFG(0xE0000000, 21u);
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_CRYPTO
        MSS_MPU_configure(MSS_MPU_GEM0, MSS_MPU_PMP_REGION2,
                0x0000000022002000ULL, 0x0000000000200000ULL,
                MPU_MODE_READ_ACCESS | MPU_MODE_WRITE_ACCESS, MSS_MPU_AM_NAPOT, 0);
#else
#error "bad memory region defined"
#endif
#endif /* defined(MSS_MAC_USE_DDR) */
#endif /* defined(G5_SOC_EMU_USE_GEM0) */

#if defined(G5_SOC_EMU_USE_GEM1)
        MSS_MPU_configure(MSS_MPU_GEM1, MSS_MPU_PMP_REGION0,
                0x0000000000000000ULL, 0x0000000100000000ULL,
                MPU_MODE_READ_ACCESS, MSS_MPU_AM_NAPOT, 0);

        MSS_MPU_configure(MSS_MPU_GEM1, MSS_MPU_PMP_REGION1,
                0x0000000008000000ULL, 0x0000000000200000ULL,
                MPU_MODE_READ_ACCESS | MPU_MODE_WRITE_ACCESS, MSS_MPU_AM_NAPOT, 0);

#if defined(MSS_MAC_USE_DDR)
#if MSS_MAC_USE_DDR == MSS_MAC_MEM_DDR
        MSS_MPU_configure(MSS_MPU_GEM1, MSS_MPU_PMP_REGION2,
                0x00000000C0000000ULL, 0x0000000000200000ULL,
                MPU_MODE_READ_ACCESS | MPU_MODE_WRITE_ACCESS, MSS_MPU_AM_NAPOT, 0);
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_FIC0
        MSS_MPU_configure(MSS_MPU_GEM1, MSS_MPU_PMP_REGION2,
                0x0000000060000000ULL, 0x0000000000200000ULL,
                MPU_MODE_READ_ACCESS | MPU_MODE_WRITE_ACCESS, MSS_MPU_AM_NAPOT, 0);
        MPU[4].CFG[2].pmp = MPU_CFG(0x60000000, 21u);
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_FIC1
        MSS_MPU_configure(MSS_MPU_GEM1, MSS_MPU_PMP_REGION2,
                0x00000000E0000000ULL, 0x0000000000200000ULL,
                MPU_MODE_READ_ACCESS | MPU_MODE_WRITE_ACCESS, MSS_MPU_AM_NAPOT, 0);
        MPU[4].CFG[2].pmp = MPU_CFG(0xE0000000, 21u);
#elif MSS_MAC_USE_DDR == MSS_MAC_MEM_CRYPTO
        MSS_MPU_configure(MSS_MPU_GEM1, MSS_MPU_PMP_REGION2,
                0x0000000022002000ULL, 0x0000000000200000ULL,
                MPU_MODE_READ_ACCESS | MPU_MODE_WRITE_ACCESS, MSS_MPU_AM_NAPOT, 0);
#else
#error "bad memory region defined"
#endif
#endif /* defined(MSS_MAC_USE_DDR) */
#endif /* defined(G5_SOC_EMU_USE_GEM1) */

        SEG[0].CFG[0].offset = -(0x0080000000ll >> 24u);
        SEG[0].CFG[1].offset = -(0x1000000000ll >> 24u);
        SEG[1].CFG[2].offset = -(0x00C0000000ll >> 24u);
        SEG[1].CFG[3].offset = -(0x1400000000ll >> 24u);
        SEG[1].CFG[4].offset = -(0x00D0000000ll >> 24u);
        SEG[1].CFG[5].offset = -(0x1800000000ll >> 24u);
#endif

        main_other_hart();
    }

    /* should never get here */
    while(1)
    {
       static volatile uint64_t counter = 0U;

       /* Added some code as debugger hangs if in loop doing nothing */
       counter = counter + 1;
    }

    return (0);
}


/**============================================================================
 *
 */
static void packet_tx_complete_handler0(/* mss_mac_instance_t*/ void *this_mac,
               uint32_t queue_no, mss_mac_tx_desc_t *cdesc, void * caller_info)
{
    (void)caller_info;
    (void)queue_no;
    (void)this_mac;
    tx_count0++;
    g_tx_desc0 = *cdesc;
#if defined(MSS_MAC_TIME_STAMPED_MODE)
    if(cdesc->status & GEM_TX_DMA_TS_PRESENT)
    {
        g_tx_ts_count0++;
    }
#endif
}

/**============================================================================
 *
 */
static void packet_tx_complete_handler0e(/* mss_mac_instance_t*/ void *this_mac,
                uint32_t queue_no, mss_mac_tx_desc_t *cdesc, void * caller_info)
{
    (void)caller_info;
    (void)queue_no;
    (void)this_mac;
    tx_count0e++;
    g_tx_desc0e = *cdesc;
#if defined(MSS_MAC_TIME_STAMPED_MODE)
    if(cdesc->status & GEM_TX_DMA_TS_PRESENT)
    {
        g_tx_ts_count0e++;
    }
#endif
}

/**============================================================================
 *
 */
static void packet_tx_complete_handler1(/* mss_mac_instance_t*/ void *this_mac,
               uint32_t queue_no, mss_mac_tx_desc_t *cdesc, void * caller_info)
{
    (void)caller_info;
    (void)queue_no;
    (void)this_mac;
    tx_count1++;
    g_tx_desc1 = *cdesc;
#if defined(MSS_MAC_TIME_STAMPED_MODE)
    if(cdesc->status & GEM_TX_DMA_TS_PRESENT)
    {
        g_tx_ts_count1++;
    }
#endif
}

/**============================================================================
 *
 */
static void packet_tx_complete_handler1e(/* mss_mac_instance_t*/ void *this_mac,
                uint32_t queue_no, mss_mac_tx_desc_t *cdesc, void * caller_info)
{
    (void)caller_info;
    (void)queue_no;
    (void)this_mac;
    tx_count1e++;
    g_tx_desc1e = *cdesc;
#if defined(MSS_MAC_TIME_STAMPED_MODE)
    if(cdesc->status & GEM_TX_DMA_TS_PRESENT)
    {
        g_tx_ts_count1e++;
    }
#endif
}


static volatile int rx_count0  = 0;
static volatile int rx_count1  = 0;
static volatile int rx_count0e = 0;
static volatile int rx_count1e = 0;

/**=============================================================================
    Bottom-half of receive packet handler
*/
static void mac_rx_callback0
(
    /* mss_mac_instance_t */ void *this_mac,
    uint32_t queue_no,
    uint8_t * p_rx_packet,
    uint32_t pckt_length,
    mss_mac_rx_desc_t *cdesc,
    void * caller_info
)
{
    (void)caller_info;
    uint8_t tx_status;

     /*
      * Looking for packet so grab a copy
      * May be capture any packet or just capture time stamped packets to
      * assist with PTP testing by filtering out non PTP via the 'm' command.
      */
    if((PACKET_ARMED == g_capture0) ||
            ((PACKET_ARMED_PTP == g_capture0) && (cdesc->addr_low & BIT_02)))
    {
        if(pckt_length > PACKET_MAX)
        {
            pckt_length = PACKET_MAX;
        }

        memcpy(g_packet_data0, p_rx_packet, pckt_length);

        g_packet_length0 = (int)pckt_length;
        g_rx_desc0 = *cdesc;
        g_queue_no_0 = (int)queue_no;
        g_capture0 = PACKET_DONE; /* and say we go it */
    }

#if defined(MSS_MAC_TIME_STAMPED_MODE)
    if(cdesc->addr_low & BIT_02) /* Count the number of time stamps received */
    {
        g_rx_ts_count0++;
    }
#endif

    if(g_loopback0) /* Send what we receive if set to loopback */
    {
        /*
         * We send back any packets we receive (with optional extra bytes to
         * make the returned packets stand out in Wireshark).
         *
         * We may need to wait for the last packet to finish sending as we only
         * have a single transmit queue so 100% back to back operation will not
         * work but... The MSS MAC ISR checks to see if we already handled the
         * current TX so that we don't double dip on the TX handler.
         */

        do
        {
            if(g_tx_add_1)
            {
                tx_status = MSS_MAC_send_pkt(((mss_mac_instance_t *)this_mac),
                            0, p_rx_packet,
                            (pckt_length + (uint32_t)g_tx_adjust) | g_crc, (void *)0);
            }
            else
            {
                tx_status = MSS_MAC_send_pkt(((mss_mac_instance_t *)this_mac),
                            0, p_rx_packet, pckt_length | g_crc, (void *)0);
            }
            if(MSS_MAC_SUCCESS != tx_status) /* Assume we couldn't send this packet as there is one in the pipeline */
            {
                g_tx_retry0++;
                if(((mss_mac_instance_t *)this_mac)->mac_base->INT_STATUS &
                        GEM_TRANSMIT_COMPLETE)
                {
#if 0 /* This is doing it by the book... */
                    if(0 != ((mss_mac_instance_t *)this_mac)->tx_complete_handler)
                    {
                        ((mss_mac_instance_t *)this_mac)->tx_complete_handler((mss_mac_instance_t *)this_mac,
                            ((mss_mac_instance_t *)this_mac)->tx_desc_tab,
                            ((mss_mac_instance_t *)this_mac)->tx_caller_info[0]);
                    }
#else /* This is cutting corners because we know we can in this instance... */
                    tx_count0++;
                    /* Only one tx descriptor matters in our case... */
                    g_tx_desc0 = ((mss_mac_instance_t *)this_mac)->queue[queue_no].tx_desc_tab[0];
#if defined(MSS_MAC_TIME_STAMPED_MODE)
                    if(cdesc->status & GEM_TX_DMA_TS_PRESENT)
                    {
                        g_tx_ts_count0++;
                    }
#endif
#endif
                    ((mss_mac_instance_t *)this_mac)->queue[queue_no].nb_available_tx_desc =
                                       MSS_MAC_TX_RING_SIZE; /* Release transmit queue... */
                    ((mss_mac_instance_t *)this_mac)->mac_base->TRANSMIT_STATUS =
                                                      GEM_STAT_TRANSMIT_COMPLETE;
                    ((mss_mac_instance_t *)this_mac)->mac_base->INT_STATUS =
                                                       GEM_TRANSMIT_COMPLETE;
                }
            }
        } while(tx_status != MSS_MAC_SUCCESS);
    }

    MSS_MAC_receive_pkt((mss_mac_instance_t *)this_mac, queue_no, p_rx_packet,
                        caller_info, MSS_MAC_INT_ENABLE);
    rx_count0++;
}


/**=============================================================================
    Bottom-half of receive packet handler
*/
static void mac_rx_callback0e
(
    /* mss_mac_instance_t */ void *this_mac,
    uint32_t queue_no,
    uint8_t * p_rx_packet,
    uint32_t pckt_length,
    mss_mac_rx_desc_t *cdesc,
    void * caller_info
)
{
    (void)caller_info;
    uint8_t tx_status;

     /*
      * Looking for packet so grab a copy
      * May be capture any packet or just capture time stamped packets to
      * assist with PTP testing by filtering out non PTP via the 'm' command.
      */
    if((PACKET_ARMED == g_capture0e) ||
            ((PACKET_ARMED_PTP == g_capture0e) && (cdesc->addr_low & BIT_02)))
    {
        if(pckt_length > PACKET_MAX)
        {
            pckt_length = PACKET_MAX;
        }

        memcpy(g_packet_data0e, p_rx_packet, pckt_length);

        g_packet_length0e = (int)pckt_length;
        g_rx_desc0e       = *cdesc;
        g_queue_no_0e     = (int)queue_no;
        g_capture0e       = PACKET_DONE; /* and say we go it */
    }

#if defined(MSS_MAC_TIME_STAMPED_MODE)
    if(cdesc->addr_low & BIT_02) /* Count the number of time stamps received */
    {
        g_rx_ts_count0e++;
    }
#endif

    if(g_loopback0e) /* Send what we receive if set to loopback */
    {
        /*
         * We send back any packets we receive (with optional extra bytes to
         * make the returned packets stand out in Wireshark).
         *
         * We may need to wait for the last packet to finish sending as we only
         * have a single transmit queue so 100% back to back operation will not
         * work but... The MSS MAC ISR checks to see if we already handled the
         * current TX so that we don't double dip on the TX handler.
         */

        do
        {
            if(g_tx_add_1)
            {
                tx_status = MSS_MAC_send_pkt(((mss_mac_instance_t *)this_mac),
                        0, p_rx_packet,
                        (pckt_length + (uint32_t)g_tx_adjust) | g_crc, (void *)0);
            }
            else
            {
                tx_status = MSS_MAC_send_pkt(((mss_mac_instance_t *)this_mac),
                        0, p_rx_packet, pckt_length | g_crc, (void *)0);
            }
            /* Assume we couldn't send this packet as there is one in the pipeline */
            if(MSS_MAC_SUCCESS != tx_status)
            {
                g_tx_retry0e++;
                if(((mss_mac_instance_t *)this_mac)->emac_base->INT_STATUS &
                        GEM_TRANSMIT_COMPLETE)
                {
#if 0 /* This is doing it by the book... */
                    if(0 != ((mss_mac_instance_t *)this_mac)->tx_complete_handler)
                    {
                        ((mss_mac_instance_t *)this_mac)->tx_complete_handler((mss_mac_instance_t *)this_mac,
                            ((mss_mac_instance_t *)this_mac)->tx_desc_tab,
                            ((mss_mac_instance_t *)this_mac)->tx_caller_info[0]);
                    }
#else /* This is cutting corners because we know we can in this instance... */
                    tx_count0e++;
                    /* Only one tx descriptor matters in our case... */
                    g_tx_desc0e = ((mss_mac_instance_t *)this_mac)->queue[queue_no].tx_desc_tab[0];
#if defined(MSS_MAC_TIME_STAMPED_MODE)
                    if(cdesc->status & GEM_TX_DMA_TS_PRESENT)
                    {
                        g_tx_ts_count0e++;
                    }
#endif
#endif
                    ((mss_mac_instance_t *)this_mac)->queue[queue_no].nb_available_tx_desc =
                                       MSS_MAC_TX_RING_SIZE; /* Release transmit queue... */
                    ((mss_mac_instance_t *)this_mac)->emac_base->TRANSMIT_STATUS =
                                                       GEM_STAT_TRANSMIT_COMPLETE;
                    ((mss_mac_instance_t *)this_mac)->emac_base->INT_STATUS =
                                                       GEM_TRANSMIT_COMPLETE;
                }
            }
        } while(tx_status != MSS_MAC_SUCCESS);
    }

    MSS_MAC_receive_pkt((mss_mac_instance_t *)this_mac, queue_no, p_rx_packet,
                        caller_info, MSS_MAC_INT_ENABLE);
    rx_count0e++;
}


/**=============================================================================
    Bottom-half of receive packet handler
*/
static void mac_rx_callback1
(
    /* mss_mac_instance_t */ void *this_mac,
    uint32_t queue_no,
    uint8_t * p_rx_packet,
    uint32_t pckt_length,
    mss_mac_rx_desc_t *cdesc,
    void * caller_info
)
{
    (void)caller_info;
    uint8_t tx_status;

     /*
      * Looking for packet so grab a copy
      * May be capture any packet or just capture time stamped packets to
      * assist with PTP testing by filtering out non PTP via the 'm' command.
      */
    if((PACKET_ARMED == g_capture1) ||
            ((PACKET_ARMED_PTP == g_capture1) && (cdesc->addr_low & BIT_02)))
    {
        if(pckt_length > PACKET_MAX)
        {
            pckt_length = PACKET_MAX;
        }

        memcpy(g_packet_data1, p_rx_packet, pckt_length);

        g_packet_length1 = (int)pckt_length;
        g_rx_desc1 = *cdesc;
        g_queue_no_1 = (int)queue_no;
        g_capture1 = PACKET_DONE; /* and say we go it */
    }

#if defined(MSS_MAC_TIME_STAMPED_MODE)
    if(cdesc->addr_low & BIT_02) /* Count the number of time stamps received */
    {
        g_rx_ts_count1++;
    }
#endif

    if(g_loopback1) /* Send what we receive if set to loopback */
    {
        /*
         * We send back any packets we receive (with an optional extra byte to
         * make the returned packets stand out in Wireshark).
         *
         * We may need to wait for the last packet to finish sending as we only
         * have a single transmit queue so 100% back to back operation will not
         * work but... The MSS MAC ISR checks to see if we already handled the
         * current TX so that we don't double dip on the TX handler.
         */

        do
        {
            if(g_tx_add_1)
            {
                tx_status = MSS_MAC_send_pkt(((mss_mac_instance_t *)this_mac),
                        0, p_rx_packet,
                        (pckt_length + (uint32_t)g_tx_adjust) | g_crc, (void *)0);
            }
            else
            {
                tx_status = MSS_MAC_send_pkt(((mss_mac_instance_t *)this_mac),
                        0, p_rx_packet, pckt_length | g_crc, (void *)0);
            }
            /* Assume we couldn't send this packet as there is one in the pipeline */
            if(MSS_MAC_SUCCESS != tx_status)
            {
                g_tx_retry1++;
                if(((mss_mac_instance_t *)this_mac)->mac_base->INT_STATUS & GEM_TRANSMIT_COMPLETE)
                {
#if 0 /* This is doing it by the book... */
                    if(0 != ((mss_mac_instance_t *)this_mac)->tx_complete_handler)
                    {
                        ((mss_mac_instance_t *)this_mac)->tx_complete_handler((mss_mac_instance_t *)this_mac,
                            ((mss_mac_instance_t *)this_mac)->tx_desc_tab,
                            ((mss_mac_instance_t *)this_mac)->tx_caller_info[0]);
                    }
#else /* This is cutting corners because we know we can in this instance... */
                    tx_count1++;
                    /* Only one tx descriptor matters in our case... */
                    g_tx_desc1 = ((mss_mac_instance_t *)this_mac)->queue[queue_no].tx_desc_tab[0];
#if defined(MSS_MAC_TIME_STAMPED_MODE)
                    if(cdesc->status & GEM_TX_DMA_TS_PRESENT)
                    {
                        g_tx_ts_count1++;
                    }
#endif
#endif
                    ((mss_mac_instance_t *)this_mac)->queue[queue_no].nb_available_tx_desc =
                                       MSS_MAC_TX_RING_SIZE; /* Release transmit queue... */
                    ((mss_mac_instance_t *)this_mac)->mac_base->TRANSMIT_STATUS =
                                                      GEM_STAT_TRANSMIT_COMPLETE;
                    ((mss_mac_instance_t *)this_mac)->mac_base->INT_STATUS =
                                                      GEM_TRANSMIT_COMPLETE;
                }
            }
        } while(tx_status != MSS_MAC_SUCCESS);
    }

    MSS_MAC_receive_pkt((mss_mac_instance_t *)this_mac, queue_no, p_rx_packet,
                        caller_info, MSS_MAC_INT_ENABLE);
    rx_count1++;
}


/**=============================================================================
    Bottom-half of receive packet handler
*/
static void mac_rx_callback1e
(
    /* mss_mac_instance_t */ void *this_mac,
    uint32_t queue_no,
    uint8_t * p_rx_packet,
    uint32_t pckt_length,
    mss_mac_rx_desc_t *cdesc,
    void * caller_info
)
{
    (void)caller_info;
    uint8_t tx_status;

     /*
      * Looking for packet so grab a copy
      * May be capture any packet or just capture time stamped packets to
      * assist with PTP testing by filtering out non PTP via the 'm' command.
      */
    if((PACKET_ARMED == g_capture1e) ||
            ((PACKET_ARMED_PTP == g_capture1e) && (cdesc->addr_low & BIT_02)))
    {
        if(pckt_length > PACKET_MAX)
        {
            pckt_length = PACKET_MAX;
        }

        memcpy(g_packet_data1e, p_rx_packet, pckt_length);

        g_packet_length1e = (int)pckt_length;
        g_rx_desc1e       = *cdesc;
        g_queue_no_1e     = (int)queue_no;
        g_capture1e       = PACKET_DONE; /* and say we go it */
    }

#if defined(MSS_MAC_TIME_STAMPED_MODE)
    if(cdesc->addr_low & BIT_02) /* Count the number of time stamps received */
    {
        g_rx_ts_count1e++;
    }
#endif

    if(g_loopback1e) /* Send what we receive if set to loopback */
    {
        /*
         * We send back any packets we receive (with an optional extra byte to
         * make the returned packets stand out in Wireshark).
         *
         * We may need to wait for the last packet to finish sending as we only
         * have a single transmit queue so 100% back to back operation will not
         * work but... The MSS MAC ISR checks to see if we already handled the
         * current TX so that we don't double dip on the TX handler.
         */

        do
        {
            if(g_tx_add_1)
            {
                tx_status = MSS_MAC_send_pkt(((mss_mac_instance_t *)this_mac),
                        0, p_rx_packet,
                        (pckt_length + (uint32_t)g_tx_adjust) | g_crc, (void *)0);
            }
            else
            {
                tx_status = MSS_MAC_send_pkt(((mss_mac_instance_t *)this_mac),
                        0, p_rx_packet, pckt_length | g_crc, (void *)0);
            }
            /* Assume we couldn't send this packet as there is one in the pipeline */
            if(MSS_MAC_SUCCESS != tx_status)
            {
                g_tx_retry1e++;
                if(((mss_mac_instance_t *)this_mac)->emac_base->INT_STATUS &
                        GEM_TRANSMIT_COMPLETE)
                {
#if 0 /* This is doing it by the book... */
                    if(0 != ((mss_mac_instance_t *)this_mac)->tx_complete_handler)
                    {
                        ((mss_mac_instance_t *)this_mac)->tx_complete_handler((mss_mac_instance_t *)this_mac,
                            ((mss_mac_instance_t *)this_mac)->tx_desc_tab,
                            ((mss_mac_instance_t *)this_mac)->tx_caller_info[0]);
                    }
#else /* This is cutting corners because we know we can in this instance... */
                    tx_count1e++;
                    /* Only one tx descriptor matters in our case... */
                    g_tx_desc1e = ((mss_mac_instance_t *)this_mac)->queue[queue_no].tx_desc_tab[0];
#if defined(MSS_MAC_TIME_STAMPED_MODE)
                    if(cdesc->status & GEM_TX_DMA_TS_PRESENT)
                    {
                        g_tx_ts_count1e++;
                    }
#endif
#endif
                    ((mss_mac_instance_t *)this_mac)->queue[queue_no].nb_available_tx_desc =
                                       MSS_MAC_TX_RING_SIZE; /* Release transmit queue... */
                    ((mss_mac_instance_t *)this_mac)->emac_base->TRANSMIT_STATUS =
                                                       GEM_STAT_TRANSMIT_COMPLETE;
                    ((mss_mac_instance_t *)this_mac)->emac_base->INT_STATUS =
                                                       GEM_TRANSMIT_COMPLETE;
                }
            }
        } while(tx_status != MSS_MAC_SUCCESS);
    }

    MSS_MAC_receive_pkt((mss_mac_instance_t *)this_mac, queue_no, p_rx_packet,
                        caller_info, MSS_MAC_INT_ENABLE);
    rx_count1e++;
}


uint8_t mac_addr_store[6];

/*==============================================================================
 *
 */
void get_mac_address(uint8_t * mac_addr)
{
    uint32_t inc;

    for(inc = 0; inc < 6; ++inc)
    {
        mac_addr[inc] = mac_addr_store[inc];
    }
}


/*==============================================================================
 *
 */

void set_mac_address(uint8_t * mac_addr)
{

    /* Update stored copies of mac address */
    memcpy(mac_addr_store,        mac_addr, 6);
    memcpy(g_mac_config.mac_addr, mac_addr, 6);

    /* Reconfigure the actual hardware */
//    MSS_MAC_update_hw_address(&g_mac_config);
}


/**=============================================================================
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(void)
{
    mss_mac_tsu_config_t tsu_cfg;
    int count;

#if defined(MSS_MAC_USE_DDR)
    g_mac_rx_buffer0 = g_mss_mac_ddr_ptr;
    g_mss_mac_ddr_ptr += MSS_MAC_RX_RING_SIZE * MSS_MAC_MAX_RX_BUF_SIZE;
    g_mac_rx_buffer1 = g_mss_mac_ddr_ptr;
    g_mss_mac_ddr_ptr += MSS_MAC_RX_RING_SIZE * MSS_MAC_MAX_RX_BUF_SIZE;
#endif
    /*-------------------------- Initialize the MAC --------------------------*/
    /*
     * Get the default configuration for the Ethernet MAC and change settings
     * to match the system/application. The default values typically changed
     * are:
     *  - interface:
     *      Specifies the interface used to connect the Ethernet MAC to the PHY.
     *      Example choice are MII, GMII, TBI.
     *  - phy_addr:
     *      Specifies the MII management interface address of the external PHY.
     *  - mac_addr:
     *      Specifies the MAC address of the device. This number should be
     *      unique on the network the device is connected to.
     *  - speed_duplex_select:
     *      Specifies the allowed speed and duplex mode for setting up a link.
     *      This can be used to specify the set of allowed speeds and duplex
     *      modes used during auto-negotiation or force the link speed to a
     *      specific speed and duplex mode.
     */
    MSS_MAC_cfg_struct_def_init(&g_mac_config);

    g_mac_config.jumbo_frame_enable = MSS_MAC_JUMBO_FRAME_ENABLE;

    g_mac_config.speed_duplex_select =  MSS_MAC_ANEG_ALL_SPEEDS;
   /* g_mac_config.interface = GMII; */

    g_mac_config.mac_addr[0] = 0x00;
    g_mac_config.mac_addr[1] = 0xFC;
    g_mac_config.mac_addr[2] = 0x00;
    g_mac_config.mac_addr[3] = 0x12;
    g_mac_config.mac_addr[4] = 0x34;
    g_mac_config.mac_addr[5] = 0x56;

#if MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_DUAL_EXTERNAL
    g_mac_config.phy_addr            = PHY_VSC8575_MDIO_ADDR;
    g_mac_config.phy_type            = MSS_MAC_DEV_PHY_VSC8575;
    g_mac_config.pcs_phy_addr        = SGMII_MDIO_ADDR;
    g_mac_config.interface_type      = GMII_SGMII;
    g_mac_config.phy_autonegotiate   = MSS_MAC_VSC8575_phy_autonegotiate;
    g_mac_config.phy_get_link_status = MSS_MAC_VSC8575_phy_get_link_status;
    g_mac_config.phy_init            = MSS_MAC_VSC8575_phy_init;
    g_mac_config.phy_set_link_speed  = MSS_MAC_VSC8575_phy_set_link_speed;
    g_mac_config.phy_extended_read   = NULL_ti_read_extended_regs;
    g_mac_config.phy_extended_write  = NULL_ti_write_extended_regs;
#else  /* MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_DUAL_EXTERNAL */
    g_mac_config.phy_addr            = PHY_NULL_MDIO_ADDR;
    g_mac_config.phy_type            = MSS_MAC_DEV_PHY_NULL;

    g_mac_config.pcs_phy_addr        = SGMII_MDIO_ADDR;
    g_mac_config.interface_type      = GMII;
    g_mac_config.phy_autonegotiate   = MSS_MAC_NULL_phy_autonegotiate;
    g_mac_config.phy_get_link_status = MSS_MAC_NULL_phy_get_link_status;
    g_mac_config.phy_init            = MSS_MAC_NULL_phy_init;
    g_mac_config.phy_set_link_speed  = MSS_MAC_NULL_phy_set_link_speed;

#if MSS_MAC_USE_PHY_DP83867
    g_mac_config.phy_extended_read   = NULL_ti_read_extended_regs;
    g_mac_config.phy_extended_write  = NULL_ti_write_extended_regs;
#endif
#endif /* MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_DUAL_EXTERNAL */

#if defined(MSS_MAC_QUEUES)
//    g_mac_config.queue_enable[0] = MSS_MAC_QUEUE_ENABLE;
//    g_mac_config.queue_enable[1] = MSS_MAC_QUEUE_ENABLE;
//    g_mac_config.queue_enable[2] = MSS_MAC_QUEUE_ENABLE;
#endif
    /*
     * Initialize MAC with specified configuration. The Ethernet MAC is
     * functional after this function returns but still requires transmit and
     * receive buffers to be allocated for communications to take place.
     */
    MSS_MAC_init(g_test_mac, &g_mac_config);

    /*
     * Register MAC interrupt handler listener functions. These functions will
     * be called  by the MAC driver when a packet has been sent or received.
     * These callback functions are intended to help managing transmit and
     * receive buffers by indicating when a transmit buffer can be released or
     * a receive buffer has been filled with an rx packet.
     */

    MSS_MAC_set_tx_callback(g_test_mac, 0, packet_tx_complete_handler0);
    MSS_MAC_set_rx_callback(g_test_mac, 0, mac_rx_callback0);
    MSS_MAC_set_rx_callback(g_test_mac, 1, mac_rx_callback0);
    MSS_MAC_set_rx_callback(g_test_mac, 2, mac_rx_callback0);
    MSS_MAC_set_rx_callback(g_test_mac, 3, mac_rx_callback0);

    /* Now do the eMAC */

    g_mac_config.mac_addr[5] = 0x58; /* Need different MAC address */
    MSS_MAC_init(&g_emac0, &g_mac_config);
    MSS_MAC_set_tx_callback(&g_emac0, 0, packet_tx_complete_handler0e);
    MSS_MAC_set_rx_callback(&g_emac0, 0, mac_rx_callback0e);

    /*
     * Allocate receive buffers.
     *
     * We prime the pump with a full set of packet buffers and then re use them
     * as each packet is handled.
     *
     * This function will need to be called each time a packet is received to
     * hand back the receive buffer to the MAC driver.
     */
    for(count = 0; count < MSS_MAC_RX_RING_SIZE; ++count)
    {
        /*
         * We allocate the buffers with the Ethernet MAC interrupt disabled
         * until we get to the last one. For the last one we ensure the Ethernet
         * MAC interrupt is enabled on return from MSS_MAC_receive_pkt().
         */
#if defined(MSS_MAC_USE_DDR)
        if(count != (MSS_MAC_RX_RING_SIZE - 1))
        {
            MSS_MAC_receive_pkt(&g_mac0,
                    g_mac_rx_buffer0_0 + count * MSS_MAC_MAX_RX_BUF_SIZE,
                    0, MSS_MAC_INT_DISABLE);
#if (MSS_MAC_QUEUE_COUNT >= 2)
            MSS_MAC_receive_pkt(&g_mac0,
                    g_mac_rx_buffer0_1 + count * MSS_MAC_MAX_RX_BUF_SIZE,
                    0, MSS_MAC_INT_DISABLE);
#endif
#if (MSS_MAC_QUEUE_COUNT >= 3)
            MSS_MAC_receive_pkt(&g_mac0,
                    g_mac_rx_buffer0_2 + count * MSS_MAC_MAX_RX_BUF_SIZE,
                    0, MSS_MAC_INT_DISABLE);
#endif
#if (MSS_MAC_QUEUE_COUNT == 4)
            MSS_MAC_receive_pkt(&g_mac0,
                    g_mac_rx_buffer0_3 + count * MSS_MAC_MAX_RX_BUF_SIZE,
                    0, MSS_MAC_INT_DISABLE);
#endif
        }
        else
        {
            MSS_MAC_receive_pkt(&g_mac0,
                    g_mac_rx_buffer0_0 + count * MSS_MAC_MAX_RX_BUF_SIZE,
                    0, MSS_MAC_INT_ARM);
#if (MSS_MAC_QUEUE_COUNT >= 2)
            MSS_MAC_receive_pkt(&g_mac0,
                    g_mac_rx_buffer0_1 + count * MSS_MAC_MAX_RX_BUF_SIZE,
                    0, MSS_MAC_INT_ARM);
#endif
#if (MSS_MAC_QUEUE_COUNT >= 3)
            MSS_MAC_receive_pkt(&g_mac0,
                    g_mac_rx_buffer0_2 + count * MSS_MAC_MAX_RX_BUF_SIZE, 0,
                    MSS_MAC_INT_ARM);
#endif
#if (MSS_MAC_QUEUE_COUNT == 4)
            MSS_MAC_receive_pkt(&g_mac0,
                    g_mac_rx_buffer0_3 + count * MSS_MAC_MAX_RX_BUF_SIZE, 0,
                    MSS_MAC_INT_ARM);
#endif
        }
#else
        if(count != (MSS_MAC_RX_RING_SIZE - 1))
        {
            MSS_MAC_receive_pkt(&g_mac0,  0, g_mac_rx_buffer0_0[count], 0,
                                MSS_MAC_INT_DISABLE);
            MSS_MAC_receive_pkt(&g_emac0, 0, g_emac_rx_buffer0_0[count], 0,
                                MSS_MAC_INT_DISABLE);
#if (MSS_MAC_QUEUE_COUNT >= 2)
            MSS_MAC_receive_pkt(&g_mac0, 1, g_mac_rx_buffer0_1[count], 0,
                                MSS_MAC_INT_DISABLE);
#endif
#if (MSS_MAC_QUEUE_COUNT >= 3)
            MSS_MAC_receive_pkt(&g_mac0, 2, g_mac_rx_buffer0_2[count], 0,
                                MSS_MAC_INT_DISABLE);
#endif
#if (MSS_MAC_QUEUE_COUNT == 4)
            MSS_MAC_receive_pkt(&g_mac0, 3, g_mac_rx_buffer0_3[count], 0,
                                MSS_MAC_INT_DISABLE);
#endif
        }
        else
        {
            MSS_MAC_receive_pkt(&g_mac0,  0, g_mac_rx_buffer0_0[count], 0,
                                MSS_MAC_INT_ARM);
            MSS_MAC_receive_pkt(&g_emac0, 0, g_emac_rx_buffer0_0[count], 0,
                                MSS_MAC_INT_ARM);
#if (MSS_MAC_QUEUE_COUNT >= 2)
            MSS_MAC_receive_pkt(&g_mac0, 1, g_mac_rx_buffer0_1[count], 0,
                                MSS_MAC_INT_ARM);
#endif
#if (MSS_MAC_QUEUE_COUNT >= 3)
            MSS_MAC_receive_pkt(&g_mac0, 2, g_mac_rx_buffer0_2[count], 0,
                                MSS_MAC_INT_ARM);
#endif
#if (MSS_MAC_QUEUE_COUNT == 4)
            MSS_MAC_receive_pkt(&g_mac0, 3, g_mac_rx_buffer0_3[count], 0,
                                MSS_MAC_INT_ARM);
#endif
        }
#endif
    }

    /*
     * Initialize second MAC with specified configuration. The Ethernet MAC is
     * functional after this function returns but still requires transmit and
     * receive buffers to be allocated for communications to take place.
     */

#if MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_DUAL_EXTERNAL
    g_mac_config.phy_addr            = PHY_DP83867_MDIO_ADDR;
    g_mac_config.phy_type            = MSS_MAC_DEV_PHY_DP83867;
    g_mac_config.pcs_phy_addr        = SGMII_MDIO_ADDR;
    g_mac_config.interface_type      = GMII_SGMII;
    g_mac_config.phy_autonegotiate   = MSS_MAC_DP83867_phy_autonegotiate;
    g_mac_config.phy_get_link_status = MSS_MAC_DP83867_phy_get_link_status;
    g_mac_config.phy_init            = MSS_MAC_DP83867_phy_init;
    g_mac_config.phy_set_link_speed  = MSS_MAC_DP83867_phy_set_link_speed;
    g_mac_config.phy_extended_read   = ti_read_extended_regs;
    g_mac_config.phy_extended_write  = ti_write_extended_regs;
#endif

/* Can only use 10MB FD for GEM1 in current dual GEM designs... */
#if 1
    g_mac_config.speed_duplex_select =  MSS_MAC_ANEG_10M_FD;
#endif

    g_mac_config.mac_addr[5] = 0x57; /* Need different MAC address */

    g_mac_config.use_hi_address = 1; /* Use alternate address for GEM1 */
    MSS_MAC_init(&g_mac1, &g_mac_config);

    /*
     * Register MAC interrupt handler listener functions. These functions will
     * be called  by the MAC driver when a packet has been sent or received.
     * These callback functions are intended to help managing transmit and
     * receive buffers by indicating when a transmit buffer can be released or
     * a receive buffer has been filled with an rx packet.
     */

    MSS_MAC_set_tx_callback(&g_mac1, 0, packet_tx_complete_handler1);
    MSS_MAC_set_rx_callback(&g_mac1, 0, mac_rx_callback1);
    MSS_MAC_set_rx_callback(&g_mac1, 1, mac_rx_callback1);
    MSS_MAC_set_rx_callback(&g_mac1, 2, mac_rx_callback1);
    MSS_MAC_set_rx_callback(&g_mac1, 3, mac_rx_callback1);

    /* Now do the eMAC */

    g_mac_config.mac_addr[5] = 0x59; /* Need different MAC address */
    MSS_MAC_init(&g_emac1, &g_mac_config);
    MSS_MAC_set_tx_callback(&g_emac1, 0, packet_tx_complete_handler1e);
    MSS_MAC_set_rx_callback(&g_emac1, 0, mac_rx_callback1e);

    /*
     * Allocate receive buffers.
     *
     * We prime the pump with a full set of packet buffers and then re use them
     * as each packet is handled.
     *
     * This function will need to be called each time a packet is received to
     * hand back the receive buffer to the MAC driver.
     */
    for(count = 0; count < MSS_MAC_RX_RING_SIZE; ++count)
    {
        /*
         * We allocate the buffers with the Ethernet MAC interrupt disabled
         * until we get to the last one. For the last one we ensure the Ethernet
         * MAC interrupt is enabled on return from MSS_MAC_receive_pkt().
         */
#if defined(MSS_MAC_USE_DDR)
        if(count != (MSS_MAC_RX_RING_SIZE - 1))
        {
            MSS_MAC_receive_pkt(&g_mac1,
                    g_mac_rx_buffer1_0 + count * MSS_MAC_MAX_RX_BUF_SIZE, 0,
                    MSS_MAC_INT_DISABLE);
#if (MSS_MAC_QUEUE_COUNT >= 2)
            MSS_MAC_receive_pkt(&g_mac1,
                    g_mac_rx_buffer1_1 + count * MSS_MAC_MAX_RX_BUF_SIZE, 0,
                    MSS_MAC_INT_DISABLE);
#endif
#if (MSS_MAC_QUEUE_COUNT >= 3)
            MSS_MAC_receive_pkt(&g_mac1,
                    g_mac_rx_buffer1_2 + count * MSS_MAC_MAX_RX_BUF_SIZE, 0,
                    MSS_MAC_INT_DISABLE);
#endif
#if (MSS_MAC_QUEUE_COUNT == 4)
            MSS_MAC_receive_pkt(&g_mac1,
                    g_mac_rx_buffer1_3 + count * MSS_MAC_MAX_RX_BUF_SIZE, 0,
                    MSS_MAC_INT_DISABLE);
#endif
        }
        else
        {
            MSS_MAC_receive_pkt(&g_mac1,
                    g_mac_rx_buffer1_0 + count * MSS_MAC_MAX_RX_BUF_SIZE, 0,
                    MSS_MAC_INT_ARM);
#if (MSS_MAC_QUEUE_COUNT >= 2)
            MSS_MAC_receive_pkt(&g_mac1,
                    g_mac_rx_buffer1_1 + count * MSS_MAC_MAX_RX_BUF_SIZE, 0,
                    MSS_MAC_INT_ARM);
#endif
#if (MSS_MAC_QUEUE_COUNT >= 3)
            MSS_MAC_receive_pkt(&g_mac1,
                    g_mac_rx_buffer1_2 + count * MSS_MAC_MAX_RX_BUF_SIZE, 0,
                    MSS_MAC_INT_ARM);
#endif
#if (MSS_MAC_QUEUE_COUNT == 4)
            MSS_MAC_receive_pkt(&g_mac1,
                    g_mac_rx_buffer1_3 + count * MSS_MAC_MAX_RX_BUF_SIZE, 0,
                    MSS_MAC_INT_ARM);
#endif
        }
#else
        if(count != (MSS_MAC_RX_RING_SIZE - 1))
        {
            MSS_MAC_receive_pkt(&g_mac1,  0, g_mac_rx_buffer1_0[count], 0,
                    MSS_MAC_INT_DISABLE);
            MSS_MAC_receive_pkt(&g_emac1, 0, g_emac_rx_buffer1_0[count], 0,
                    MSS_MAC_INT_DISABLE);
#if (MSS_MAC_QUEUE_COUNT >= 2)
            MSS_MAC_receive_pkt(&g_mac1, 1, g_mac_rx_buffer1_1[count], 0,
                    MSS_MAC_INT_DISABLE);
#endif
#if (MSS_MAC_QUEUE_COUNT >= 3)
            MSS_MAC_receive_pkt(&g_mac1, 2, g_mac_rx_buffer1_2[count], 0,
                    MSS_MAC_INT_DISABLE);
#endif
#if (MSS_MAC_QUEUE_COUNT == 4)
            MSS_MAC_receive_pkt(&g_mac1, 3, g_mac_rx_buffer1_3[count], 0,
                    MSS_MAC_INT_DISABLE);
#endif
        }
        else
        {
            MSS_MAC_receive_pkt(&g_mac1,  0, g_mac_rx_buffer1_0[count],  0,
                    MSS_MAC_INT_ARM);
            MSS_MAC_receive_pkt(&g_emac1, 0, g_emac_rx_buffer1_0[count], 0,
                    MSS_MAC_INT_ARM);
#if (MSS_MAC_QUEUE_COUNT >= 2)
            MSS_MAC_receive_pkt(&g_mac1, 1, g_mac_rx_buffer1_1[count], 0,
                    MSS_MAC_INT_ARM);
#endif
#if (MSS_MAC_QUEUE_COUNT >= 3)
            MSS_MAC_receive_pkt(&g_mac1, 2, g_mac_rx_buffer1_2[count], 0,
                    MSS_MAC_INT_ARM);
#endif
#if (MSS_MAC_QUEUE_COUNT == 4)
            MSS_MAC_receive_pkt(&g_mac1, 3, g_mac_rx_buffer1_3[count], 0,
                    MSS_MAC_INT_ARM);
#endif
        }
#endif
    }

    tsu_cfg.nanoseconds = 500;
    tsu_cfg.secs_lsb    = 0x123;
    tsu_cfg.secs_msb    = 0x0000;
    tsu_cfg.ns_inc      = 8;
    tsu_cfg.sub_ns_inc  = 0;

    MSS_MAC_init_TSU(&g_mac0, &tsu_cfg);
    MSS_MAC_init_TSU(&g_emac0, &tsu_cfg);

    /* Apply a gross offset between TSUs */
    tsu_cfg.secs_lsb    += 1000;

    MSS_MAC_init_TSU(&g_mac1, &tsu_cfg);
    MSS_MAC_init_TSU(&g_emac1, &tsu_cfg);

    /* GEM0 is 10.1.1.2 and GEM1 is 10.1.1.3 for our exercises */
    MSS_MAC_set_TSU_unicast_addr(&g_mac0, MSS_MAC_TSU_UNICAST_RX, 0x0A010102);
    MSS_MAC_set_TSU_unicast_addr(&g_mac0, MSS_MAC_TSU_UNICAST_TX, 0x0A010103);
    MSS_MAC_set_TSU_unicast_addr(&g_mac1, MSS_MAC_TSU_UNICAST_RX, 0x0A010103);
    MSS_MAC_set_TSU_unicast_addr(&g_mac1, MSS_MAC_TSU_UNICAST_TX, 0x0A010102);

    MSS_MAC_set_TSU_unicast_addr(&g_emac0, MSS_MAC_TSU_UNICAST_RX, 0x0A010104);
    MSS_MAC_set_TSU_unicast_addr(&g_emac0, MSS_MAC_TSU_UNICAST_TX, 0x0A010105);
    MSS_MAC_set_TSU_unicast_addr(&g_emac1, MSS_MAC_TSU_UNICAST_RX, 0x0A010105);
    MSS_MAC_set_TSU_unicast_addr(&g_emac1, MSS_MAC_TSU_UNICAST_TX, 0x0A010104);
}


/*==============================================================================
 *
 */
volatile uint64_t g_tick_counter = 0;
uint64_t link_status_timer = 0;

volatile uint8_t g_test_linkup0 = 0;
uint8_t          g_test_fullduplex0 = 0;
mss_mac_speed_t  g_test_speed0 = MSS_MAC_1000MBPS;

volatile uint8_t g_test_linkup1 = 0;
uint8_t          g_test_fullduplex1 = 0;
mss_mac_speed_t  g_test_speed1 = MSS_MAC_1000MBPS;

void prvLinkStatusTask(void);
void prvLinkStatusTask(void)
{
    if(g_tick_counter >= link_status_timer)
    {
        /* Run through loop every 500 milliseconds. */
        g_test_linkup0 = MSS_MAC_get_link_status(&g_mac0, &g_test_speed0,
                                                 &g_test_fullduplex0);
        g_test_linkup1 = MSS_MAC_get_link_status(&g_mac1, &g_test_speed1,
                                                 &g_test_fullduplex1);
        link_status_timer = g_tick_counter + 250;
    }
}


/*==============================================================================
 *
 */
void SysTick_Handler(uint32_t hart_id)
{
    (void)hart_id;
    g_tick_counter++;
}


/*==============================================================================
 *
 */

void e51_task( void *pvParameters );

void e51(void);
void e51(void)
{
    write_csr(mscratch, 0);
    write_csr(mcause, 0);
    write_csr(mepc, 0);
//    init_memory();

    PLIC_init();
    PLIC_init_on_reset();

    e51_task(0);
}


/*==============================================================================
 *
 */

uint8_t *tsu_mode_string(mss_mac_tsu_mode_t mode);
uint8_t *tsu_mode_string(mss_mac_tsu_mode_t mode)
{
    char *ret_val;

    if(mode >= MSS_MAC_TSU_MODE_END)
    {
        ret_val = "INVALID";
    }
    else if(MSS_MAC_TSU_MODE_DISABLED == mode)
    {
        ret_val = "DISABLED";
    }
    else if(MSS_MAC_TSU_MODE_PTP_EVENT == mode)
    {
        ret_val = "PTP EVENT FRAMES";
    }
    else if(MSS_MAC_TSU_MODE_PTP_ALL == mode)
    {
        ret_val = "ALL PTP FRAMES";
    }
    else if(MSS_MAC_TSU_MODE_ALL == mode)
    {
        ret_val = "ALL FRAMES";
    }
    else
    {
        ret_val = "INVALID";
    }

    return((uint8_t *)ret_val);
}
/*==============================================================================
 *
 */
static void print_help(void)
{
    char info_string[200];
    mss_mac_tsu_mode_t temp_mode;
    mss_mac_type_1_filter_t filter1;
    int temp;
    uint32_t temp_cutthru;
    mss_mac_mmsl_config_t mmsl_cfg;

    sprintf(info_string,"The following single key commands are accepted:\n\r\n\r");
    PRINT_STRING(info_string);

    sprintf(info_string,"a - Initiate a PHY autonegotiation cycle\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"A - Toggle Pause Frame Copy to Memory------(%s)\n\r",
            MSS_MAC_get_pause_frame_copy_to_mem(g_test_mac) ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    if(g_test_mac == &g_mac0)
    {
        sprintf(info_string,"b - Toggle HW Loopback mode ---------------(%s)\n\r",
                0 != (g_test_mac->emac_base->NETWORK_CONTROL & GEM_LOOPBACK_LOCAL) ? "enabled" : "disabled");
    }
    else
    {
        sprintf(info_string,"b - Toggle HW Loopback mode ---------------(%s)\n\r",
                0 != (g_test_mac->mac_base->NETWORK_CONTROL & GEM_LOOPBACK_LOCAL) ? "enabled" : "disabled");
    }
    PRINT_STRING(info_string);
    if(g_test_mac->is_emac)
    {
        sprintf(info_string,"B - Toggle broadcast RX mode --------------(%s)\n\r",
                0 != (g_test_mac->emac_base->NETWORK_CONFIG & GEM_NO_BROADCAST) ? "disabled" : "enabled");
    }
    else
    {
        sprintf(info_string,"B - Toggle broadcast RX mode --------------(%s)\n\r",
                0 != (g_test_mac->mac_base->NETWORK_CONFIG & GEM_NO_BROADCAST) ? "disabled" : "enabled");
    }
    PRINT_STRING(info_string);
    sprintf(info_string,"c - Capture and dump next packet\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"C - Capture and dump next time stamped packet\n\r");
    PRINT_STRING(info_string);
    temp_cutthru = MSS_MAC_get_tx_cutthru(g_test_mac);
    sprintf(info_string,"d - Toggle TX Cutthru ---------------------(%s)\n\r",
            temp_cutthru ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    temp_cutthru = MSS_MAC_get_rx_cutthru(g_test_mac);
    sprintf(info_string,"D - Toggle RX Cutthru ---------------------(%s)\n\r",
            temp_cutthru ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    if((&g_mac0 == g_test_mac) || (&g_emac0 == g_test_mac))
    {
        MSS_MAC_get_mmsl_mode(&g_mac0, &mmsl_cfg);
    }
    else
    {
        MSS_MAC_get_mmsl_mode(&g_mac1, &mmsl_cfg);
    }
    sprintf(info_string,"e - Toggle RX all to pMAC -----------------(%s)\n\r",
            mmsl_cfg.use_pmac ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    sprintf(info_string,"E - Toggle preemption mode ----------------(%s)\n\r",
            mmsl_cfg.preemption ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    PRINT_STRING("g - Display MSS GPIO 2 input values\n\r");
    PRINT_STRING("h - Display this help information\n\r");
    PRINT_STRING("i - Increment all GEM stats counter registers\n\r");
    PRINT_STRING("I - Display MMSL statistics\n\r");
    sprintf(info_string,"j - Toggle Jumbo Packet Mode --------------(%s)\n\r",
            MSS_MAC_get_jumbo_frames_mode(g_test_mac) ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    if(g_test_mac == &g_mac0)
    {
        sprintf(info_string,"k - Toggle capture re-trigger mode --------(%s)\n\r",
                g_reload0 ? "enabled" : "disabled");
    }
    else if(g_test_mac == &g_emac0)
    {
        sprintf(info_string,"k - Toggle capture re-trigger mode --------(%s)\n\r",
                g_reload0e ? "enabled" : "disabled");
    }
    else if(g_test_mac == &g_mac1)
    {
        sprintf(info_string,"k - Toggle capture re-trigger mode --------(%s)\n\r",
                g_reload1 ? "enabled" : "disabled");
    }
    else
    {
        sprintf(info_string,"k - Toggle capture re-trigger mode --------(%s)\n\r",
                g_reload1e ? "enabled" : "disabled");
    }
    PRINT_STRING(info_string);

    if(g_test_mac == &g_mac0)
    {
        sprintf(info_string,"l - Toggle SW Loopback mode ---------------(%s)\n\r",
                g_loopback0 ? "enabled" : "disabled");
    }
    else if(g_test_mac == &g_emac0)
    {
        sprintf(info_string,"l - Toggle SW Loopback mode ---------------(%s)\n\r",
                g_loopback0e ? "enabled" : "disabled");
    }
    else if(g_test_mac == &g_mac1)
    {
        sprintf(info_string,"l - Toggle SW Loopback mode ---------------(%s)\n\r",
                g_loopback1 ? "enabled" : "disabled");
    }
    else
    {
        sprintf(info_string,"l - Toggle SW Loopback mode ---------------(%s)\n\r",
                g_loopback1e ? "enabled" : "disabled");
    }
    PRINT_STRING(info_string);
    sprintf(info_string,"L - Toggle SGMII Link Status display mode -(%s)\n\r",
            g_link_status ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    temp_mode = MSS_MAC_get_TSU_rx_mode(g_test_mac);
    sprintf(info_string,"m - Step through rx TSU time stamp modes --(%s)\n\r",
            tsu_mode_string(temp_mode));
    PRINT_STRING(info_string);
    temp_mode = MSS_MAC_get_TSU_tx_mode(g_test_mac);
    sprintf(info_string,"M - Step through tx TSU time stamp modes --(%s)\n\r",
            tsu_mode_string(temp_mode));
    PRINT_STRING(info_string);
    sprintf(info_string,"n - Toggle TX response length adjust mode -(%s)\n\r",
            g_tx_add_1 ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    sprintf(info_string,"o - Step through One Step Mode options ----");
    PRINT_STRING(info_string);
    if(MSS_MAC_OSS_MODE_DISABLED == MSS_MAC_get_TSU_oss_mode(g_test_mac))
    {
        PRINT_STRING("(disabled)\n\r");
    }
    else if(MSS_MAC_OSS_MODE_REPLACE == MSS_MAC_get_TSU_oss_mode(g_test_mac))
    {
        PRINT_STRING("(replace)\n\r");
    }
    else if(MSS_MAC_OSS_MODE_ADJUST == MSS_MAC_get_TSU_oss_mode(g_test_mac))
    {
        PRINT_STRING("(adjust)\n\r");
    }
    else if(MSS_MAC_OSS_MODE_INVALID == MSS_MAC_get_TSU_oss_mode(g_test_mac))
    {
        PRINT_STRING("(invalid)\n\r");
    }
    else
    {
        PRINT_STRING("(unknown)\n\r");
    }
    if(g_test_mac->is_emac)
    {
        sprintf(info_string,"p - Toggle promiscuous receive mode -------(%s)\n\r",
                0 != (g_test_mac->emac_base->NETWORK_CONFIG & GEM_COPY_ALL_FRAMES) ? "enabled" : "disabled");
    }
    else
    {
        sprintf(info_string,"p - Toggle promiscuous receive mode -------(%s)\n\r",
                0 != (g_test_mac->mac_base->NETWORK_CONFIG & GEM_COPY_ALL_FRAMES) ? "enabled" : "disabled");
    }
    PRINT_STRING(info_string);
    sprintf(info_string,"P - Transmit stream of PTP packets\n\r");
    PRINT_STRING(info_string);
    MSS_MAC_get_type_1_filter(g_test_mac, 3, &filter1);
    if(0 == filter1.udp_port)
    {
        PRINT_STRING("q - Toggle Screener Type 1 Filters  -------(disabled)\n\r");
    }
    else
    {
        PRINT_STRING("q - Toggle Screener Type 1 Filters  -------(enabled)\n\r");
    }
    if(0 == MSS_MAC_get_type_2_ethertype(g_test_mac, 0))
    {
        PRINT_STRING("Q - Toggle Screener Type 2 Filters  -------(disabled)\n\r");
    }
    else
    {
        PRINT_STRING("Q - Toggle Screener Type 2 Filters  -------(enabled)\n\r");
    }
    sprintf(info_string,"r - Reset statistics counts\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"s - Show statistics\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"S - Show subset of statistics\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"t - Transmit sample ARP packet - 128 bytes 0xFF padded\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"T - Transmit sample ARP packet - 60+ bytes 0x00 padded\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"u - Transmit sample Unicast ARP packet - 128 bytes 0xFF padded\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"U - Transmit sample Unicast ARP packet - 60+ bytes 0x00 padded\n\r");
    PRINT_STRING(info_string);

    if(g_test_mac == &g_mac0)
    {
        temp = g_vlan_tags0;
    }
    else if(g_test_mac == &g_emac0)
    {
        temp = g_vlan_tags0e;
    }
    else if(g_test_mac == &g_mac1)
    {
        temp = g_vlan_tags1;
    }
    else
    {
        temp = g_vlan_tags1e;
    }
    sprintf(info_string,"v - Step through VLAN tag depth options ---(%d)\n\r",
            temp);
    PRINT_STRING(info_string);
    sprintf(info_string,"V - Toggle VLAN only mode -----------------(%s)\n\r",
            MSS_MAC_get_VLAN_only_mode(g_test_mac) ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    PRINT_STRING("w - TX frame on pMAC and eMAC with no delay\n\r");
    PRINT_STRING("W - TX frame on pMAC and eMAC with short delay\n\r");
    sprintf(info_string,"x - Toggle PHY register dump mode ---------(%s)\n\r",
            g_phy_dump ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    sprintf(info_string,"z - Toggle FCS passthrough mode -----------(%s)\n\r",
            g_crc ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    sprintf(info_string,"\' - Transmit Pause Frame\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,", - Transmit Zero Quantum Pause Frame\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,". - Transmit Priority Based Pause Frame\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"= - Toggle opposing address SA filter -----(%s)\n\r",
            (MSS_MAC_get_sa_filter(g_test_mac, 2, 0) & MSS_MAC_SA_FILTER_SOURCE) ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    sprintf(info_string,"# - Insert and delete selection of multicast hashes\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"] - Send single sync packet from P list with\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"    extra info for testing one step modes\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"> - Send single packet from P list\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"/ - Toggle address swap mode --------------(%s)\n\r",
            g_address_swap ? "swapped" : "normal");
    PRINT_STRING(info_string);
    sprintf(info_string,"+/- Increment/Decrement length adjust -----(%d)\n\r",
            g_tx_adjust);
    PRINT_STRING(info_string);
    sprintf(info_string,"! - Display TSU count for current GEM\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"0/1 Switch active (e)GEM to 0 or 1 --------(Currently ");
    PRINT_STRING(info_string);
    if(g_test_mac == &g_mac0)
    {
        PRINT_STRING("GEM0)");
    }
    else if(g_test_mac == &g_emac0)
    {
        PRINT_STRING("eGEM0)");
    }
    else if(g_test_mac == &g_mac1)
    {
        PRINT_STRING("GEM1)");
    }
    else
    {
        PRINT_STRING("eGEM1)");
    }

    sprintf(info_string,"\n\r4 - Toggle IPv4 filter --------------------(%s)\n\r",
            MSS_MAC_get_type_filter(g_test_mac, 3) ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    sprintf(info_string,"6 - Toggle IPv6 filter --------------------(%s)\n\r",
            MSS_MAC_get_type_filter(g_test_mac, 4) ? "enabled" : "disabled");
    PRINT_STRING(info_string);
}


/**
 *    MSS_MAC_TX_OCTETS_LOW,                * 32-bit *
 *    MSS_MAC_TX_OCTETS_HIGH,               * 16-bit *
 *    MSS_MAC_TX_FRAMES_OK,                 * 32-bit *
 *    MSS_MAC_TX_BCAST_FRAMES_OK,           * 32-bit *
 *    MSS_MAC_TX_MCAST_FRAMES_OK,           * 32-bit *
 *    MSS_MAC_TX_PAUSE_FRAMES_OK,           * 32-bit *
 *    MSS_MAC_TX_64_BYTE_FRAMES_OK,         * 32-bit *
 *    MSS_MAC_TX_65_BYTE_FRAMES_OK,         * 32-bit *
 *    MSS_MAC_TX_128_BYTE_FRAMES_OK,        * 32-bit *
 *    MSS_MAC_TX_256_BYTE_FRAMES_OK,        * 32-bit *
 *    MSS_MAC_TX_512_BYTE_FRAMES_OK,        * 32-bit *
 *    MSS_MAC_TX_1024_BYTE_FRAMES_OK,       * 32-bit *
 *    MSS_MAC_TX_1519_BYTE_FRAMES_OK,       * 32-bit *
 *    MSS_MAC_TX_UNDERRUNS,                 * 10-bit *
 *    MSS_MAC_TX_SINGLE_COLLISIONS,         * 18-bit *
 *    MSS_MAC_TX_MULTIPLE_COLLISIONS,       * 18-bit *
 *    MSS_MAC_TX_EXCESSIVE_COLLISIONS,      * 10-bit *
 *    MSS_MAC_TX_LATE_COLLISIONS,           * 10-bit *
 *    MSS_MAC_TX_DEFERRED_FRAMES,           * 18-bit *
 *    MSS_MAC_TX_CRS_ERRORS,                * 10-bit *
 *
 *    MSS_MAC_RX_OCTETS_LOW,                * 32-bit *
 *    MSS_MAC_RX_OCTETS_HIGH,               * 16-bit *
 *    MSS_MAC_RX_FRAMES_OK,                 * 32-bit *
 *    MSS_MAC_RX_BCAST_FRAMES_OK,           * 32-bit *
 *    MSS_MAC_RX_MCAST_FRAMES_OK,           * 32-bit *
 *    MSS_MAC_RX_PAUSE_FRAMES_OK,           * 32-bit *
 *    MSS_MAC_RX_64_BYTE_FRAMES_OK,         * 32-bit *
 *    MSS_MAC_RX_65_BYTE_FRAMES_OK,         * 32-bit *
 *    MSS_MAC_RX_128_BYTE_FRAMES_OK,        * 32-bit *
 *    MSS_MAC_RX_256_BYTE_FRAMES_OK,        * 32-bit *
 *    MSS_MAC_RX_512_BYTE_FRAMES_OK,        * 32-bit *
 *    MSS_MAC_RX_1024_BYTE_FRAMES_OK,       * 32-bit *
 *    MSS_MAC_RX_1519_BYTE_FRAMES_OK,       * 32-bit *
 *    MSS_MAC_RX_UNDERSIZE_FRAMES_OK,       * 10-bit *
 *    MSS_MAC_RX_OVERSIZE_FRAMES_OK,        * 10-bit *
 *    MSS_MAC_RX_JABBERS,                   * 10-bit *
 *    MSS_MAC_RX_FCS_ERRORS,                * 10-bit *
 *    MSS_MAC_RX_LENGTH_ERRORS,             * 10-bit *
 *    MSS_MAC_RX_SYMBOL_ERRORS,             * 10-bit *
 *    MSS_MAC_RX_ALIGNMENT_ERRORS,          * 10-bit *
 *    MSS_MAC_RX_RESOURCE_ERRORS,           * 18-bit *
 *    MSS_MAC_RX_OVERRUNS,                  * 10-bit *
 *    MSS_MAC_RX_IP_CHECKSUM_ERRORS,        *  8-bit *
 *    MSS_MAC_RX_TCP_CHECKSUM_ERRORS,       *  8-bit *
 *    MSS_MAC_RX_UDP_CHECKSUM_ERRORS,       *  8-bit *
 *    MSS_MAC_RX_AUTO_FLUSHED_PACKETS,      * 16-bit *
 *
 *  MSS_MAC_LAST_STAT
 *
 */
static uint32_t stats0[MSS_MAC_LAST_STAT];
static uint32_t stats1[MSS_MAC_LAST_STAT];
static uint32_t stats0e[MSS_MAC_LAST_STAT];
static uint32_t stats1e[MSS_MAC_LAST_STAT];
uint32_t *stats = stats0;

static uint64_t copper_rx_good      = 0; /* P1 reg 18 */
static uint32_t phy_rx_err          = 0; /* P0 reg 19 */
static uint32_t phy_false_carrier   = 0; /* P0 reg 20 */
static uint32_t phy_link_disconnect = 0; /* P0 reg 21 */
#if defined (TARGET_G5_SOC)
static uint64_t mac_rx_good         = 0; /* P3 reg 28 */
static uint32_t mac_rx_err          = 0; /* P3 reg 29 */
static uint64_t mac_tx_good         = 0; /* P3 reg 15 */
static uint32_t mac_tx_err          = 0; /* P3 reg 16 */
#endif

/*==============================================================================
 *
 */

void stats_dump(void);
void stats_dump(void)
{
    char info_string[200];
    mss_mac_stat_t count;

    if(g_phy_dump) /* Only do if enabled as it can impact comms response times */
    {
        if(MSS_MAC_DEV_PHY_VSC8575 == g_test_mac->phy_type)
            {
            dump_vsc8575_regs(g_test_mac);

            copper_rx_good      += VSC8575_reg_1[2] & 0x7FFF;
            phy_rx_err          += VSC8575_reg_0[19];
            phy_false_carrier   += VSC8575_reg_0[20];
            phy_link_disconnect += VSC8575_reg_0[21];

            mac_rx_good         += VSC8575_reg_3[16] & 0x3FFF;
            mac_rx_err          += VSC8575_reg_3[17] & 0x00FF;
            mac_tx_good         += VSC8575_reg_3[18] & 0x3FFF;
            mac_tx_err          += VSC8575_reg_3[19] & 0x00FF;
            }

        if(MSS_MAC_DEV_PHY_DP83867 == g_test_mac->phy_type)
        {
            dump_ti_regs(g_test_mac);
        }
    }

    if(MSS_MAC_DEV_PHY_VSC8575 == g_test_mac->phy_type)
    {
        sprintf(info_string,"PHY Statistics\n\r");
        PRINT_STRING(info_string);

        sprintf(info_string,"PHY CU RX GOOD          % 10U  ", copper_rx_good);
        PRINT_STRING(info_string);

        sprintf(info_string,"PHY CU RX ERRORS        % 10U\n\r", phy_rx_err);
        PRINT_STRING(info_string);

        sprintf(info_string,"PHY MAC RX GOOD         % 10U  ", mac_rx_good);
        PRINT_STRING(info_string);

        sprintf(info_string,"PHY MAC RX ERRORS       % 10U\n\r", mac_rx_err);
        PRINT_STRING(info_string);
        sprintf(info_string,"PHY MAC TX GOOD         % 10U  ", mac_tx_good);
        PRINT_STRING(info_string);

        sprintf(info_string,"PHY MAC TX ERRORS       % 10U\n\r", mac_tx_err);
        PRINT_STRING(info_string);

        sprintf(info_string,"PHY FALSE CARRIER ERR   % 10U  ",
                phy_false_carrier);
        PRINT_STRING(info_string);

        sprintf(info_string,"PHY LINK DISCONNECTS    % 10U\n\r\n\r",
                phy_link_disconnect);
        PRINT_STRING(info_string);

        if(GMII_SGMII == g_test_mac->interface_type)
        {
            sprintf(info_string,"SGMII Registers\n\r");
            PRINT_STRING(info_string);

            sprintf(info_string,"SGMII Control Register  %04X\n\r",
                    (unsigned int)VSC8575_MSS_SGMII_reg16[0]);
            PRINT_STRING(info_string);

            sprintf(info_string,"SGMII Status Register   %04X\n\r",
                    (unsigned int)VSC8575_MSS_SGMII_reg16[1]);
            PRINT_STRING(info_string);

            sprintf(info_string,"SGMII AN Advertisement  %04X\n\r\n\r",
                    (unsigned int)VSC8575_MSS_SGMII_reg16[4]);
            PRINT_STRING(info_string);
        }
    }

    if(MSS_MAC_DEV_PHY_DP83867 == g_test_mac->phy_type)
    {
        sprintf(info_string,"PHY Registers\n\r");
        PRINT_STRING(info_string);

        sprintf(info_string,"Basic Mode Control Register                    %04X\n\r",
                (unsigned int)TI_reg_0[0]);
        PRINT_STRING(info_string);

        sprintf(info_string,"Basic Mode Status Register                     %04X\n\r",
                (unsigned int)TI_reg_0[1]);
        PRINT_STRING(info_string);

        sprintf(info_string,"Auto-Negotiation Advertisement Register        %04X\n\r",
                (unsigned int)TI_reg_0[4]);
        PRINT_STRING(info_string);

        sprintf(info_string,"Auto-Negotiation Link Partner Ability Register %04X\n\r",
                (unsigned int)TI_reg_0[5]);
        PRINT_STRING(info_string);

        sprintf(info_string,"Auto-Negotiate Expansion Register              %04X\n\r",
                (unsigned int)TI_reg_0[6]);
        PRINT_STRING(info_string);

        sprintf(info_string,"Auto-Negotiation Next Page Transmit Register   %04X\n\r",
                (unsigned int)TI_reg_0[7]);
        PRINT_STRING(info_string);

        sprintf(info_string,"Auto-Negotiation Next Page Receive Register    %04X\n\r",
                (unsigned int)TI_reg_0[8]);
        PRINT_STRING(info_string);

        sprintf(info_string,"1000BASE-T Configuration Register              %04X\n\r",
                (unsigned int)TI_reg_0[9]);
        PRINT_STRING(info_string);

        sprintf(info_string,"Status Register 1                              %04X\n\r",
                (unsigned int)TI_reg_0[10]);
        PRINT_STRING(info_string);

        sprintf(info_string,"1000BASE-T Status Register                     %04X\n\r",
                (unsigned int)TI_reg_0[15]);
        PRINT_STRING(info_string);

        sprintf(info_string,"PHY Control Register                           %04X\n\r",
                (unsigned int)TI_reg_0[16]);
        PRINT_STRING(info_string);

        sprintf(info_string,"PHY Status Register                            %04X\n\r",
                (unsigned int)TI_reg_0[17]);
        PRINT_STRING(info_string);

        sprintf(info_string,"Interrupt Status Register                      %04X\n\r",
                (unsigned int)TI_reg_0[19]);
        PRINT_STRING(info_string);

        sprintf(info_string,"Configuration Register 2                       %04X\n\r",
                (unsigned int)TI_reg_0[20]);
        PRINT_STRING(info_string);

        sprintf(info_string,"Configuration Register 4                       %04X\n\r",
                (unsigned int)TI_reg_1[2]);
        PRINT_STRING(info_string);

        sprintf(info_string,"SGMII Auto-Negotiation Status                  %04X\n\r",
                (unsigned int)TI_reg_1[5]);
        PRINT_STRING(info_string);

        sprintf(info_string,"Skew FIFO Status                               %04X\n\r",
                (unsigned int)TI_reg_1[7]);
        PRINT_STRING(info_string);

        sprintf(info_string,"10M SGMII Configuration                        %04X\n\r",
                (unsigned int)TI_reg_1[18]);
        PRINT_STRING(info_string);

        sprintf(info_string,"MMD3 PCS Control Register                      %04X\n\r\n\r",
                (unsigned int)TI_reg_1[23]);
        PRINT_STRING(info_string);

        if(GMII_SGMII == g_test_mac->interface_type)
        {
            sprintf(info_string,"SGMII Registers\n\r");
            PRINT_STRING(info_string);

            sprintf(info_string,"SGMII Control Register                         %04X\n\r",
                    (unsigned int)TI_MSS_SGMII_reg[0]);
            PRINT_STRING(info_string);

            sprintf(info_string,"SGMII Status Register                          %04X\n\r",
                    (unsigned int)TI_MSS_SGMII_reg[1]);
            PRINT_STRING(info_string);

            sprintf(info_string,"SGMII AN Advertisement                         %04X\n\r\n\r",
                    (unsigned int)TI_MSS_SGMII_reg[4]);
            PRINT_STRING(info_string);
        }
    }

    if(MSS_MAC_DEV_PHY_NULL == g_test_mac->phy_type)
    {
        sprintf(info_string,"NULL PHY connection\n\r\n\r");
        PRINT_STRING(info_string);
    }
        /* Grab the stats up front to minimise skew */
    if(g_test_mac == &g_mac0)
    {
        for(count = MSS_MAC_TX_OCTETS_LOW; count != MSS_MAC_LAST_STAT; count++)
        {
            stats0[count] += MSS_MAC_read_stat(&g_mac0, count);
        }

        stats = stats0;
    }
    else if(g_test_mac == &g_emac0)
    {
        for(count = MSS_MAC_TX_OCTETS_LOW; count != MSS_MAC_LAST_STAT; count++)
        {
            stats0e[count] += MSS_MAC_read_stat(&g_emac0, count);
        }

        stats = stats0e;
    }
    else if(g_test_mac == &g_mac1)
    {
        for(count = MSS_MAC_TX_OCTETS_LOW; count != MSS_MAC_LAST_STAT; count++)
        {
            stats1[count] += MSS_MAC_read_stat(&g_mac1, count);
        }

        stats = stats1;
    }
    else
    {
        for(count = MSS_MAC_TX_OCTETS_LOW; count != MSS_MAC_LAST_STAT; count++)
        {
            stats1e[count] += MSS_MAC_read_stat(&g_emac1, count);
        }

        stats = stats1e;
    }


    sprintf(info_string,"GEM Statistics\n\r");
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_OCTETS_LOW           % 10U  ",
            (uint64_t)stats[MSS_MAC_TX_OCTETS_LOW]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_OCTETS_HIGH          % 10U\n\r",
            (uint64_t)stats[MSS_MAC_TX_OCTETS_HIGH]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_FRAMES_OK            % 10U  ",
            (uint64_t)stats[MSS_MAC_TX_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_BCAST_FRAMES_OK      % 10U\n\r",
            (uint64_t)stats[MSS_MAC_TX_BCAST_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_MCAST_FRAMES_OK      % 10U  ",
            (uint64_t)stats[MSS_MAC_TX_MCAST_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_PAUSE_FRAMES_OK      % 10U\n\r",
            (uint64_t)stats[MSS_MAC_TX_PAUSE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_64_BYTE_FRAMES_OK    % 10U  ",
            (uint64_t)stats[MSS_MAC_TX_64_BYTE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_65_BYTE_FRAMES_OK    % 10U\n\r",
            (uint64_t)stats[MSS_MAC_TX_65_BYTE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_128_BYTE_FRAMES_OK   % 10U  ",
            (uint64_t)stats[MSS_MAC_TX_128_BYTE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_256_BYTE_FRAMES_OK   % 10U\n\r",
            (uint64_t)stats[MSS_MAC_TX_256_BYTE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_512_BYTE_FRAMES_OK   % 10U  ",
            (uint64_t)stats[MSS_MAC_TX_512_BYTE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_1024_BYTE_FRAMES_OK  % 10U\n\r",
            (uint64_t)stats[MSS_MAC_TX_1024_BYTE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_1519_BYTE_FRAMES_OK  % 10U  ",
            (uint64_t)stats[MSS_MAC_TX_1519_BYTE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_UNDERRUNS            % 10U\n\r",
            (uint64_t)stats[MSS_MAC_TX_UNDERRUNS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_SINGLE_COLLISIONS    % 10U  ",
            (uint64_t)stats[MSS_MAC_TX_SINGLE_COLLISIONS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_MULTIPLE_COLLISIONS  % 10U\n\r",
            (uint64_t)stats[MSS_MAC_TX_MULTIPLE_COLLISIONS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_EXCESSIVE_COLLISIONS % 10U  ",
            (uint64_t)stats[MSS_MAC_TX_EXCESSIVE_COLLISIONS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_LATE_COLLISIONS      % 10U\n\r",
            (uint64_t)stats[MSS_MAC_TX_LATE_COLLISIONS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_DEFERRED_FRAMES      % 10U  ",
            (uint64_t)stats[MSS_MAC_TX_DEFERRED_FRAMES]);
    PRINT_STRING(info_string);

    sprintf(info_string,"TX_CRS_ERRORS           % 10U\n\r",
            (uint64_t)stats[MSS_MAC_TX_CRS_ERRORS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_OCTETS_LOW           % 10U  ",
            (uint64_t)stats[MSS_MAC_RX_OCTETS_LOW]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_OCTETS_HIGH          % 10U\n\r",
            (uint64_t)stats[MSS_MAC_RX_OCTETS_HIGH]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_FRAMES_OK            % 10U  ",
            (uint64_t)stats[MSS_MAC_RX_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_BCAST_FRAMES_OK      % 10U\n\r",
            (uint64_t)stats[MSS_MAC_RX_BCAST_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_MCAST_FRAMES_OK      % 10U  ",
            (uint64_t)stats[MSS_MAC_RX_MCAST_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_PAUSE_FRAMES_OK      % 10U\n\r",
            (uint64_t)stats[MSS_MAC_RX_PAUSE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_64_BYTE_FRAMES_OK    % 10U  ",
            (uint64_t)stats[MSS_MAC_RX_64_BYTE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_65_BYTE_FRAMES_OK    % 10U\n\r",
            (uint64_t)stats[MSS_MAC_RX_65_BYTE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_128_BYTE_FRAMES_OK   % 10U  ",
            (uint64_t)stats[MSS_MAC_RX_128_BYTE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_256_BYTE_FRAMES_OK   % 10U\n\r",
            (uint64_t)stats[MSS_MAC_RX_256_BYTE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_512_BYTE_FRAMES_OK   % 10U  ",
            (uint64_t)stats[MSS_MAC_RX_512_BYTE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_1024_BYTE_FRAMES_OK  % 10U\n\r",
            (uint64_t)stats[MSS_MAC_RX_1024_BYTE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_1519_BYTE_FRAMES_OK  % 10U  ",
            (uint64_t)stats[MSS_MAC_RX_1519_BYTE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_UNDERSIZE_FRAMES_OK  % 10U\n\r",
            (uint64_t)stats[MSS_MAC_RX_UNDERSIZE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_OVERSIZE_FRAMES_OK   % 10U  ",
            (uint64_t)stats[MSS_MAC_RX_OVERSIZE_FRAMES_OK]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_JABBERS              % 10U\n\r",
            (uint64_t)stats[MSS_MAC_RX_JABBERS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_FCS_ERRORS           % 10U  ",
            (uint64_t)stats[MSS_MAC_RX_FCS_ERRORS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_LENGTH_ERRORS        % 10U\n\r",
            (uint64_t)stats[MSS_MAC_RX_LENGTH_ERRORS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_SYMBOL_ERRORS        % 10U  ",
            (uint64_t)stats[MSS_MAC_RX_SYMBOL_ERRORS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_ALIGNMENT_ERRORS     % 10U\n\r",
            (uint64_t)stats[MSS_MAC_RX_ALIGNMENT_ERRORS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_RESOURCE_ERRORS      % 10U  ",
            (uint64_t)stats[MSS_MAC_RX_RESOURCE_ERRORS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_OVERRUNS             % 10U\n\r",
            (uint64_t)stats[MSS_MAC_RX_OVERRUNS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_IP_CHECKSUM_ERRORS   % 10U  ",
            (uint64_t)stats[MSS_MAC_RX_IP_CHECKSUM_ERRORS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_TCP_CHECKSUM_ERRORS  % 10U\n\r",
            (uint64_t)stats[MSS_MAC_RX_TCP_CHECKSUM_ERRORS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_UDP_CHECKSUM_ERRORS  % 10U  ",
            (uint64_t)stats[MSS_MAC_RX_UDP_CHECKSUM_ERRORS]);
    PRINT_STRING(info_string);

    sprintf(info_string,"RX_AUTO_FLUSHED_PACKETS % 10U\n\r\n\r",
            (uint64_t)stats[MSS_MAC_RX_AUTO_FLUSHED_PACKETS]);
    PRINT_STRING(info_string);
}


/*==============================================================================
 *
 */

void packet_dump(int gem);
void packet_dump(int gem)
{
    char info_string[200];
    int dump_address = 0;
    int count;
    char temp_string[10];
    uint8_t *packet_data;
    int packet_length;
    mss_mac_rx_desc_t *cdesc;
    int vlan_offset = 0;
    int is_ptp = 0;
    int ptp_offset = 0;
    int queue_no;

    if(0 == gem)
    {
        g_capture0 = PACKET_IDLE;

        packet_length = g_packet_length0;
        packet_data   = g_packet_data0;
        cdesc         = &g_rx_desc0;
        queue_no = g_queue_no_0;
        sprintf(info_string,"%d byte packet captured on GEM0 - queue %d\n\r",
                packet_length, queue_no);
        PRINT_STRING(info_string);
    }
    else if(1 == gem)
    {
        g_capture1 = PACKET_IDLE;

        packet_length = g_packet_length1;
        packet_data   = g_packet_data1;
        cdesc         = &g_rx_desc1;
        queue_no = g_queue_no_1;
        sprintf(info_string,"%d byte packet captured on GEM1 - queue %d\n\r",
                packet_length, queue_no);
        PRINT_STRING(info_string);
    }
    else if(2 == gem)
    {
        g_capture0e = PACKET_IDLE;

        packet_length = g_packet_length0e;
        packet_data   = g_packet_data0e;
        cdesc         = &g_rx_desc0e;
        queue_no = g_queue_no_0e;
        sprintf(info_string,"%d byte packet captured on eGEM0 - queue %d\n\r",
                packet_length, queue_no);
        PRINT_STRING(info_string);
    }
    else
    {
        g_capture1e = PACKET_IDLE;

        packet_length = g_packet_length1e;
        packet_data   = g_packet_data1e;
        cdesc         = &g_rx_desc1e;
        queue_no = g_queue_no_1e;
        sprintf(info_string,"%d byte packet captured on eGEM1 - queue %d\n\r",
                packet_length, queue_no);
        PRINT_STRING(info_string);
    }

    while((packet_length - dump_address) >= 16)
    {
        sprintf(info_string,"%04X ", dump_address);
        for(count = 0; count < 16; count++)
        {
            sprintf(temp_string,"%02X "
                    , (int)packet_data[dump_address + count] & 255);
            strcat(info_string, temp_string);
        }

        for(count = 0; count < 16; count++)
        {
            if((packet_data[dump_address + count] >= 32) &&
                    (packet_data[dump_address + count] < 127))
            {
                strncat(info_string,
                        (char *)&packet_data[dump_address + count], 1);
            }
            else
            {
                strcat(info_string, ".");
            }
        }

        strcat(info_string, "\n\r");
        PRINT_STRING(info_string);

        dump_address += 16;
    }

    if((packet_length - dump_address) > 0) /* Finish off partial end line */
    {
        sprintf(info_string,"%04X ", dump_address);
        for(count = 0; count < (packet_length - dump_address); count++)
        {
            sprintf(temp_string,"%02X ",
                    (int)packet_data[dump_address + count] & 255);
            strcat(info_string, temp_string);
        }

        strncat(info_string, "                                                                        ",
                (size_t)(16 - count) * 3); /* Crude but effective space padding... */
        for(count = 0; count < (packet_length - dump_address); count++)
        {
            if((packet_data[dump_address + count] >= 32) &&
                    (packet_data[dump_address + count] < 127))
            {
                strncat(info_string,
                        (char *)&packet_data[dump_address + count], 1);
            }
            else
            {
                strcat(info_string, ".");
            }
        }
        strcat(info_string, "\n\r");
        PRINT_STRING(info_string);
    }

    /* Check for stacked VALN 1 first */
    if((0x88 == packet_data[12]) && (0xA8 == packet_data[13]) &&
            (0x00 == packet_data[14]) && (0x01 == packet_data[15]))
    {
        vlan_offset = 4;
    }

    /* Check for VALN 2 next - stacked or otherwise */
    if((0x81 == packet_data[12 + vlan_offset]) &&
       (0x00 == packet_data[13 + vlan_offset]) &&
       (0x00 == packet_data[14 + vlan_offset]) &&
       (0x02 == packet_data[15 + vlan_offset]))
    {
        vlan_offset += 4;
    }

    /* Is it a raw Ethernet PTP packet? */
    if((0x88 == packet_data[12 + vlan_offset]) &&
       (0xF7 == packet_data[13 + vlan_offset]))
    {
        is_ptp = 1;
        ptp_offset = vlan_offset + 0x0E;
        PRINT_STRING("Ethernet\n\r");
    }

    /* Is it IPV4 PTP packet ? */
    if((0x08 == packet_data[12 + vlan_offset]) &&
       (0x00 == packet_data[13 + vlan_offset]) &&
       (0x11 == packet_data[0x17 + vlan_offset])) /* IP V4 UDP*/
    {
        /* Simple check for 319 or 320 destination port... */
        if((0x01 == packet_data[0x24 + vlan_offset]) &&
          ((0x3F == packet_data[0x25 + vlan_offset]) ||
           (0x40 == packet_data[0x25 + vlan_offset])))
        {
            is_ptp = 1;
            ptp_offset = vlan_offset + 0x2A;
            PRINT_STRING("IPv4\n\r");
        }
    }

    /* Is it IPV6 PTP packet ? */
    if((0x86 == packet_data[12 + vlan_offset]) &&
       (0xDD == packet_data[13 + vlan_offset]) &&
       (0x11 == packet_data[0x14 + vlan_offset])) /* IP V6 UDP*/
    {
        /* Simple check for 319 or 320 destination port... */
        if((0x01 == packet_data[0x38 + vlan_offset]) &&
          ((0x3F == packet_data[0x39 + vlan_offset]) ||
           (0x40 == packet_data[0x39 + vlan_offset])))
        {
            is_ptp = 1;
            ptp_offset = vlan_offset + 0x3E;
            PRINT_STRING("IPv6\n\r");
        }
    }

    if(is_ptp)
    {
        uint8_t *ptp;
        int      has_ts   = 0;
        uint32_t sub_ns;
        uint64_t ns;
        uint64_t secs;
        uint32_t nsecs;

        ptp = &packet_data[ptp_offset];
        if(1 == (ptp[1] & 0x0f)) /* PTP V1 packet */
        {
            if(0 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V1 Sync\n\r");
                has_ts   = 1;
            }
            else if(1 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V1 Delay_Req\n\r");
                has_ts   = 1;
            }
            else if(8 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V1 Follow_Up\n\r");
                has_ts   = 1;
            }
            else if(9 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V1 Delay_Resp\n\r");
                has_ts   = 1;
            }
            else if(11 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V1 Announce\n\r");
            }
            else if(12 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V1 Signaling\n\r");
            }
            else if(13 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V1 Management\n\r");
            }
            else
            {
                PRINT_STRING("PTP V1 Unknown\n\r");
            }

            if(has_ts)
            {
                int ts_index;
                if(8 == (ptp[0] &0x0f))
                {
                    ts_index = 44;
                }
                else
                {
                    ts_index = 40;
                }

                nsecs = get_ptp_packet_uint(&ptp[ts_index + 4]);

                secs = (uint64_t)get_ptp_packet_uint(&ptp[ts_index]);
                sprintf(info_string,"Packet Time Stamp:       %llu:%09u\n\r",
                        secs, nsecs);
                PRINT_STRING(info_string);

                if(packet_length == (256 + vlan_offset))
                {
                    ts_info_t ts_blob;

                    memcpy(&ts_blob,
                           &packet_data[packet_length - (int)sizeof(ts_blob)],
                           sizeof(ts_blob));

                    secs = (uint64_t)ts_blob.secs_lsb;
                    sprintf(info_string,"Stored Time Stamp:       %llu:%09u\n\r",
                            secs, ts_blob.nanoseconds);
                    PRINT_STRING(info_string);
                }
            }
        }
        else if(2 == (ptp[1] & 0x0f)) /* PTP V2 packet */
        {
            if(0 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V2 Sync\n\r");
                has_ts   = 1;
            }
            else if(1 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V2 Delay_Req\n\r");
                has_ts   = 1;
            }
            else if(2 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V2 PDelay_Req\n\r");
                has_ts   = 1;
            }
            else if(3 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V2 PDelay_Resp\n\r");
                has_ts   = 1;
            }
            else if(8 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V2 Follow_Up\n\r");
                has_ts   = 1;
            }
            else if(9 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V2 Delay_Resp\n\r");
                has_ts   = 1;
            }
            else if(10 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V2 PDelay_Resp_Follow_Up\n\r");
                has_ts   = 1;
            }
            else if(11 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V2 Announce\n\r");
            }
            else if(12 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V2 Signaling\n\r");
            }
            else if(13 == (ptp[0] &0x0f))
            {
                PRINT_STRING("PTP V2 Management\n\r");
            }
            else
            {
                PRINT_STRING("PTP V2 Unknown\n\r");
            }

            sub_ns = (uint32_t)get_ptp_packet_short_uint(&ptp[14]);

            ns = ((uint64_t)get_ptp_packet_short_uint(&ptp[8])) << 32;
            ns |= (uint64_t)get_ptp_packet_uint(&ptp[10]);

            sprintf(info_string,"Packet Correction Field: %"PRIu64":%u\n\r",
                    ns, sub_ns);
            PRINT_STRING(info_string);
            if(has_ts)
            {
                nsecs = get_ptp_packet_uint(&ptp[40]);

                secs = ((uint64_t)get_ptp_packet_short_uint(&ptp[34])) << 32;
                secs |= (uint64_t)get_ptp_packet_uint(&ptp[36]);
                sprintf(info_string,"Packet Time Stamp:       %llu:%09u\n\r",
                        secs, nsecs);
                PRINT_STRING(info_string);
            }
            if(packet_length == (256 + vlan_offset))
            {
                ts_info_t ts_blob;

                memcpy(&ts_blob,
                       &packet_data[packet_length - (int)sizeof(ts_blob)],
                       sizeof(ts_blob));

                ns = ((uint64_t)ts_blob.nsecs_msb) << 32;
                ns |= (uint64_t)ts_blob.nsecs_lsb;

                sprintf(info_string,
                        "Stored Correction Field: %llu:%u\n\r", ns,
                        ts_blob.sub_nanoseconds);
                PRINT_STRING(info_string);

                if(has_ts)
                {
                    secs = ((uint64_t)ts_blob.secs_msb) << 32;
                    secs |= (uint64_t)ts_blob.secs_lsb;
                    sprintf(info_string,
                            "Stored Time Stamp:       %llu:%09u\n\r", secs,
                            ts_blob.nanoseconds);
                    PRINT_STRING(info_string);
                }
            }
        }
    }

    PRINT_STRING("RX Descriptor details:\n\r");
    sprintf(info_string,"Global all ones broadcast  - %s     ",
            cdesc->status & GEM_RX_DMA_BCAST ? "Y" : "N");
    PRINT_STRING(info_string);
    sprintf(info_string,"Multicast hash match       - %s\n\r",
            cdesc->status & GEM_RX_DMA_MULTICAST_HASH ? "Y" : "N");
    PRINT_STRING(info_string);
    sprintf(info_string,"Unicast hash match         - %s     ",
            cdesc->status & GEM_RX_DMA_UNICAST_HASH ? "Y" : "N");
    PRINT_STRING(info_string);
    sprintf(info_string,"External address match     - %s\n\r",
            cdesc->status & GEM_RX_DMA_EXT_ADDR_MATCH ? "Y" : "N");
    PRINT_STRING(info_string);
    sprintf(info_string,"Specific address reg match - %s (%d) ",
            cdesc->status & GEM_RX_DMA_SPECIFIC_ADDR ? "Y" : "N", (int)((cdesc->status & GEM_RX_DMA_ADDR_REGISTER) >> 25) + 1);
    PRINT_STRING(info_string);
    sprintf(info_string,"Type ID register match     - %s (%d)\n\r",
            cdesc->status & GEM_RX_DMA_TYPE_ID_MATCH ? "Y" : "N", (int)((cdesc->status & GEM_RX_DMA_TYPE_ID) >> 22) + 1);
    PRINT_STRING(info_string);
    sprintf(info_string,"VLAN tag detected          - %s     ",
            cdesc->status & GEM_RX_DMA_VLAN_TAG ? "Y" : "N");
    PRINT_STRING(info_string);
    sprintf(info_string,"Priority tag detected      - %s (%d)\n\r",
            cdesc->status & GEM_RX_DMA_PRIORITY_TAG ? "Y" : "N", (int)((cdesc->status & GEM_RX_DMA_VLAN_PRIORITY) >> 17) + 1);
    PRINT_STRING(info_string);
    sprintf(info_string,"Start of frame             - %s     ",
            cdesc->status & GEM_RX_DMA_START_OF_FRAME ? "Y" : "N");
    PRINT_STRING(info_string);
    sprintf(info_string,"End of frame               - %s\n\r",
            cdesc->status & GEM_RX_DMA_END_OF_FRAME ? "Y" : "N");
    PRINT_STRING(info_string);
#if defined(MSS_MAC_TIME_STAMPED_MODE)
    sprintf(info_string,"Time stamp present         - %s     ",
            cdesc->addr_low & BIT_02 ? "Y" : "N");
    PRINT_STRING(info_string);
    if(cdesc->addr_low & BIT_02)
    {
        uint32_t ts_seconds;

        ts_seconds = (cdesc->seconds << 2) + ((cdesc->nano_seconds >> 30) & 3);
        sprintf(info_string,"Time stamp = %d:%d\n\r", ts_seconds,
                (int)(cdesc->nano_seconds & 0x3FFFFFFFUL));
        PRINT_STRING(info_string);
    }
    else
    {
        PRINT_STRING("\n\r");
    }

#endif
    sprintf(info_string,"Destination Address Hash = %d\n\r",
            calc_gem_hash_index(packet_data));
    PRINT_STRING(info_string);

    /* Finally see if we need to reload the capture mechanism... */
    if((0 == gem) && (g_reload0))
    {
        g_capture0 = g_reload0;
    }

    if((2 == gem) && (g_reload0e))
    {
        g_capture0e = g_reload0e;
    }

    if((1 == gem) && (g_reload1))
    {
        g_capture1 = g_reload1;
    }

    if((3 == gem) && (g_reload1e))
    {
        g_capture1e = g_reload1e;
    }
}


/*==============================================================================
 *
 */

void print_tx_info(int gem);
void print_tx_info(int gem)
{
    char info_string[200];
    mss_mac_tx_desc_t *cdesc;

    if(0 == gem)
    {
        cdesc         = &g_tx_desc0;
    }
    else if(1 == gem)
    {
        cdesc         = &g_tx_desc1;
    }
    else if(2 == gem)
    {
        cdesc         = &g_tx_desc0e;
    }
    else
    {
        cdesc         = &g_tx_desc1e;
    }

    PRINT_STRING("TX Descriptor details:\n\r");
    sprintf(info_string,"Retry limit exceeded       - %s\n\r",
            cdesc->status & GEM_TX_DMA_RETRY_ERROR ? "Y" : "N");
    PRINT_STRING(info_string);
    sprintf(info_string,"Transmit underrun          - %s\n\r",
            cdesc->status & GEM_TX_DMA_UNDERRUN ? "Y" : "N");
    PRINT_STRING(info_string);
    sprintf(info_string,"AXI bus error              - %s\n\r",
            cdesc->status & GEM_TX_DMA_BUS_ERROR ? "Y" : "N");
    PRINT_STRING(info_string);
    sprintf(info_string,"Late collision error       - %s\n\r",
            cdesc->status & GEM_TX_DMA_LATE_COL_ERROR ? "Y" : "N");
    PRINT_STRING(info_string);
    sprintf(info_string,"FCS gen offload error      - %d\n\r",
            (int)(cdesc->status & GEM_TX_DMA_OFFLOAD_ERRORS) >> 20);
    PRINT_STRING(info_string);
#if defined(MSS_MAC_TIME_STAMPED_MODE)
    sprintf(info_string,"Time stamp present         - %s\n\r",
            cdesc->status & GEM_TX_DMA_TS_PRESENT ? "Y" : "N");
    PRINT_STRING(info_string);
    if(cdesc->status & GEM_TX_DMA_TS_PRESENT)
    {
        uint32_t ts_seconds;

        ts_seconds = (cdesc->seconds << 2) + ((cdesc->nano_seconds >> 30) & 3);
        sprintf(info_string,"Time stamp = %d:%d\n\r", ts_seconds,
                (int)(cdesc->nano_seconds & 0x3FFFFFFFUL));
        PRINT_STRING(info_string);
    }
#endif
}


/*==============================================================================
 *
 */

#define ATHENA_CR ((volatile uint32_t *) (0x20127000u))
#define ATHENA_CR_CSRMERRS ((volatile uint32_t *) (0x2200600C))

void e51_task( void *pvParameters )
{
    static uint32_t add_on = 0;
    int pkt_index = 1;
    int sync_pkt_index = 1;
    char info_string[200];
    uint8_t rx_buff[1];
    size_t rx_size = 0;
    volatile uint32_t gpio_inputs;
    volatile uint64_t delay_count;

    (void)pvParameters;
#if defined(TARGET_G5_SOC)
#if defined(MSS_MAC_USE_DDR) && (MSS_MAC_USE_DDR == MSS_MAC_MEM_CRYPTO)
    SYSREG->SOFT_RESET_CR &= ~( (1u << 16u) | (1u << 0u) | (1u << 4u) |
            (1u << 5u) | (1u << 17u) | (1u << 19u) | (1u << 23u) | (1u << 24u) |
            (1u << 25u) | (1u << 26u) | (1u << 27u) | (1u << 28u) );
#else
    SYSREG->SOFT_RESET_CR &= ~( (1u << 0u) | (1u << 4u) | (1u << 5u) |
            (1u << 17u) | (1u << 19u) | (1u << 23u) | (1u << 24u) |
            (1u << 25u) | (1u << 26u) | (1u << 27u) | (1u << 28u) );
#endif


#if MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_TI_GMII
    SYSREG->IOMUX0_CR = 0xfffffe3f;  // connect MMUART0 to GPIO, QSPI to pads
    SYSREG->IOMUX1_CR = 0xf55fffff;  // pad5,6 = mux 5 (mmuart 0)
#else
    SYSREG->IOMUX0_CR = 0xfffffe7f;  // connect MMUART0 to GPIO, QSPI to pads
    SYSREG->IOMUX1_CR = 0x05500000;  // pad5,6 = mux 5 (mmuart 0)
#endif

    /* IOMUX configurations to allow QSPI pins to the pads */
    SYSREG->IOMUX2_CR = 0;
    SYSREG->IOMUX3_CR = 0;
    SYSREG->IOMUX4_CR = 0;
    SYSREG->IOMUX5_CR = 0;

/*****************************************************************************************/
#if (MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_TBI) || \
    (MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_TBI_TI)

    MSS_GPIO_init(GPIO2_LO);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_0, MSS_GPIO_INOUT_MODE);  /* VSC8575 Reset - active low / gtwiz_reset_rx_cdr_stable_out */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_1, MSS_GPIO_INPUT_MODE);  /* gtwiz_reset_tx_done_out */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_2, MSS_GPIO_INPUT_MODE);  /* gtwiz_reset_rx_done_out */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_3, MSS_GPIO_INPUT_MODE);  /* resetdone */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_4, MSS_GPIO_INPUT_MODE);  /* txpmaresetdone_out */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_5, MSS_GPIO_INPUT_MODE);  /* rxpmaresetdone_out */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_6, MSS_GPIO_INPUT_MODE);  /* rxcommadet_out */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_7, MSS_GPIO_INPUT_MODE);  /* rxbyterealign_out */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_8, MSS_GPIO_INPUT_MODE);  /* rxbyteisaligned_out */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_9, MSS_GPIO_INPUT_MODE);  /* gtwiz_buffbypass_rx_error_out */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_10, MSS_GPIO_INPUT_MODE); /* gtwiz_buffbypass_rx_done_out */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_11, MSS_GPIO_INPUT_MODE); /* gtpowergood_out */

    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_0, 0); /* Force VSC8575 into reset */

    for(delay_count = 0; delay_count != 1000;)
        delay_count++;

    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_0, 1); /* Release reset line */

    for(delay_count = 0; delay_count != 1000000;)
        delay_count++;
#endif /* MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_TBI */

#if MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_0, MSS_GPIO_OUTPUT_MODE); /* VSC8575 Reset - active low */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_1, MSS_GPIO_OUTPUT_MODE); /* SGMII core reset - active high */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_2, MSS_GPIO_OUTPUT_MODE); /* SGMII core MDIO address b0-b4 */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_3, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_4, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_5, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_6, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_7, MSS_GPIO_INOUT_MODE);  /* SGMII core reset done input and CRS output*/
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_8, MSS_GPIO_INOUT_MODE);  /* Part of status vector input and COL output */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_9, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_10, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_11, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_12, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_13, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_14, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_15, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_16, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_17, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_18, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_19, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_20, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_21, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_22, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_23, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_24, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_25, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_26, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_27, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_28, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_29, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_30, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_31, MSS_GPIO_INPUT_MODE);

    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_2, 0); /* Set address to 16 */
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_3, 0);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_4, 0);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_5, 0);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_6, 1);

    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_0, 0); /* Force VSC8575 into reset */
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_1, 1); /* Force SGMII core into reset */

    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_7, 0); /* Assert CRS */
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_8, 0); /* Deassert COL */


    for(delay_count = 0; delay_count != 1000;)
        delay_count++;
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_0, 1); /* Release reset line */
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_1, 0); /* Release reset line */
    for(delay_count = 0; delay_count != 1000000;)
        delay_count++;

    while(0 == ((gpio_inputs = MSS_GPIO_get_inputs(GPIO2_LO)) & 0x80))
        delay_count++;

#endif /* MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII */
#endif /* defined(TARGET_G5_SOC) */

#if MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_DUAL_EXTERNAL
    MSS_GPIO_init(GPIO1_LO);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_0, MSS_GPIO_OUTPUT_MODE); /* DP83867 PHY reset - active low here as it is inverted in the bit-file */
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_1, MSS_GPIO_OUTPUT_MODE); /* SGMII core reset - active high */
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_2, MSS_GPIO_OUTPUT_MODE); /* SGMII core MDIO address b0-b4 */
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_3, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_4, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_5, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_6, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_7, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_8, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_9, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_10, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_11, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_12, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_13, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_14, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_15, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_16, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_17, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_18, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_19, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_20, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_21, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_22, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_23, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_24, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_25, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_26, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_27, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_28, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_29, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_30, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_31, MSS_GPIO_INPUT_MODE);

    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_2, 0); /* Set address to 16 */
    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_3, 0);
    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_4, 0);
    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_5, 0);
    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_6, 1);

    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_7, 1);
    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_8, 1);
    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_9, 1);
    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_10, 1);
    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_11, 1);
    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_12, 1);

    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_0, 0); /* Force DP83867 into reset */
    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_1, 1); /* Force SGMII core into reset */

    for(delay_count = 0; delay_count != 1000;)
        delay_count++;
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_0, 1); /* Release reset line */
    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_1, 0); /* Release reset line */
    for(delay_count = 0; delay_count != 1000000;)
        delay_count++;
/*
 * GEM 1 to TI PHY does not have reset complete line...
 *
 *   while(0 == ((gpio_inputs = MSS_GPIO_get_inputs(GPIO1_LO)) & 0x80))
 *       delay_count++;
 */

    MSS_GPIO_init(GPIO2_LO);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_0, MSS_GPIO_OUTPUT_MODE); /* VSC8575 Reset - active low */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_1, MSS_GPIO_OUTPUT_MODE); /* SGMII core reset - active high */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_2, MSS_GPIO_OUTPUT_MODE); /* SGMII core MDIO address b0-b4 */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_3, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_4, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_5, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_6, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_7, MSS_GPIO_INOUT_MODE);  /* SGMII core reset done input and CRS output*/
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_8, MSS_GPIO_INOUT_MODE);  /* Part of status vector input and COL output */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_9, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_10, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_11, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_12, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_13, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_14, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_15, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_16, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_17, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_18, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_19, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_20, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_21, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_22, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_23, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_24, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_25, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_26, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_27, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_28, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_29, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_30, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_31, MSS_GPIO_INPUT_MODE);

    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_2, 0); /* Set address to 16 */
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_3, 0);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_4, 0);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_5, 0);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_6, 1);

    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_8, 1);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_9, 1);

    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_0, 0); /* Force VSC8575 into reset */
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_1, 1); /* Force SGMII core into reset */

    for(delay_count = 0; delay_count != 1000;)
        delay_count++;
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_0, 1); /* Release reset line */
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_1, 0); /* Release reset line */
    for(delay_count = 0; delay_count != 1000000;)
        delay_count++;

    while(0 == ((gpio_inputs = MSS_GPIO_get_inputs(GPIO2_LO)) & 0x80))
        delay_count++;
#endif
#if MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_DUAL_INTERNAL

    MSS_GPIO_init(GPIO2_LO);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_0, MSS_GPIO_INOUT_MODE);  /* GEM0 clock */
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_1, MSS_GPIO_INOUT_MODE);  /* GEM1 clock */

    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_0, 1); /*  */
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_1, 1); /*  */
    for(delay_count = 0; delay_count != 1000;)
        delay_count++;
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_0, 0); /*  */
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_1, 0); /*  */

#endif /* MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_DUAL_INTERNAL */

    __disable_local_irq((int8_t)MMUART0_E51_INT);

    SysTick_Config();

    MSS_UART_init( &g_mss_uart0_lo,
                MSS_UART_115200_BAUD,
                MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    PRINT_STRING("PolarFire MSS Ethernet Dual eMAC/pMAC Test program\n\r");

    PRINT_STRING("Polling method for TXRX. Typed characters will be echoed.\n\r");
    __enable_irq();

#if defined(MSS_MAC_USE_DDR) && (MSS_MAC_USE_DDR == MSS_MAC_MEM_CRYPTO)
    *ATHENA_CR = SYSREG_ATHENACR_RESET | SYSREG_ATHENACR_RINGOSCON;
    *ATHENA_CR = SYSREG_ATHENACR_RINGOSCON;
    CALIni();
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, "CALIni() done..\n\r");
#endif

    low_level_init();

    set_csr(mie, MIP_MSIP);
    /*
     * Startup the other harts
     */
#ifdef TEST_SW_INT
    raise_soft_interrupt((uint32_t)1); /* get hart1 out of wfi */
#endif

    while(1)
    {
        prvLinkStatusTask();
#ifdef TEST_SW_INT
        raise_soft_interrupt((uint32_t)1);
#endif

        if(PACKET_DONE == g_capture0)
        {
            packet_dump(0);
        }

        if(PACKET_DONE == g_capture1)
        {
            packet_dump(1);
        }

        if(PACKET_DONE == g_capture0e)
        {
            packet_dump(2);
        }

        if(PACKET_DONE == g_capture1e)
        {
            packet_dump(3);
        }

        if(g_link_status)
        {
            volatile uint16_t reg = 0;
            if(TBI == g_test_mac->interface_type)
            {
                reg = (uint16_t)g_test_mac->mac_base->PCS_STATUS;
            }

            if(GMII_SGMII == g_test_mac->interface_type)
            {
                reg = (uint16_t)MSS_MAC_read_phy_reg(g_test_mac,
                        (uint8_t)g_test_mac->phy_addr, 1);
            }

            if(reg & 4)
            {
                PRINT_STRING("+");
            }
            else
            {
                PRINT_STRING("-");
            }
        }

        rx_size = MSS_UART_get_rx(&g_mss_uart0_lo, rx_buff, sizeof(rx_buff));
        if(rx_size > 0)
        {
            if(rx_buff[0] == 'a')
            {
                PRINT_STRING("Starting autonegotiation\n\r");
                g_test_mac->phy_autonegotiate(g_test_mac);
                PRINT_STRING("Finished autonegotiation\n\r");
            }
            else if(rx_buff[0] == 'A')
            {
                if(MSS_MAC_get_pause_frame_copy_to_mem(g_test_mac))
                {
                    PRINT_STRING("Disabling Pause Frame copy to memory\n\r");
                    MSS_MAC_set_pause_frame_copy_to_mem(g_test_mac, 0);
                }
                else
                {
                    PRINT_STRING("Enabling Pause Frame copy to memory\n\r");
                    MSS_MAC_set_pause_frame_copy_to_mem(g_test_mac, 1);
                }
            }

            else if(rx_buff[0] == 'b')
            {
                volatile uint32_t *ctrl_reg;
                if(g_test_mac->is_emac)
                {
                    ctrl_reg = (volatile uint32_t *)g_test_mac->emac_base;
                }
                else
                {
                    ctrl_reg = (volatile uint32_t *)g_test_mac->mac_base;
                }

                /* Note RX and TX must be disabled when changing loopback setting */
                if(0 == (*ctrl_reg & GEM_LOOPBACK_LOCAL))
                {
                    PRINT_STRING("Hardware loopback enabled\n\r");
                    *ctrl_reg &= (uint32_t)(~(GEM_ENABLE_TRANSMIT |
                            GEM_ENABLE_RECEIVE));
                    *ctrl_reg |= GEM_LOOPBACK_LOCAL | GEM_LOOPBACK;
                    *ctrl_reg |= GEM_ENABLE_TRANSMIT | GEM_ENABLE_RECEIVE;
                }
                else
                {
                    PRINT_STRING("Hardware loopback disabled\n\r");
                    *ctrl_reg &= (uint32_t)(~(GEM_ENABLE_TRANSMIT | GEM_ENABLE_RECEIVE));
                    *ctrl_reg &= (uint32_t)(~(GEM_LOOPBACK_LOCAL | GEM_LOOPBACK));
                    *ctrl_reg |= GEM_ENABLE_TRANSMIT | GEM_ENABLE_RECEIVE;
                }
            }
            else if(rx_buff[0] == 'B')
            {
                if(g_test_mac->is_emac)
                {
                    if(0 == (g_test_mac->emac_base->NETWORK_CONFIG & GEM_NO_BROADCAST))
                    {
                        PRINT_STRING("Broadcast reception disabled\n\r");
                        g_test_mac->emac_base->NETWORK_CONFIG |= GEM_NO_BROADCAST;
                    }
                    else
                    {
                        PRINT_STRING("Broadcast reception enabled\n\r");
                        g_test_mac->emac_base->NETWORK_CONFIG &=
                                (uint32_t)(~GEM_NO_BROADCAST);
                    }
                }
                else
                {
                    if(0 == (g_test_mac->mac_base->NETWORK_CONFIG & GEM_NO_BROADCAST))
                    {
                        PRINT_STRING("Broadcast reception disabled\n\r");
                        g_test_mac->mac_base->NETWORK_CONFIG |=
                                GEM_NO_BROADCAST;
                    }
                    else
                    {
                        PRINT_STRING("Broadcast reception enabled\n\r");
                        g_test_mac->mac_base->NETWORK_CONFIG &=
                                (uint32_t)(~GEM_NO_BROADCAST);
                    }
                }
            }
            else if(rx_buff[0] == 'c')
            {
                PRINT_STRING("Packet capture armed\n\r");
                if(&g_mac0 == g_test_mac)
                {
                    g_capture0 = PACKET_ARMED;
                    if(g_reload0)
                    {
                        g_reload0 = PACKET_ARMED;
                    }
                }
                else if(&g_emac0 == g_test_mac)
                {
                    g_capture0e = PACKET_ARMED;
                    if(g_reload0e)
                    {
                        g_reload0e = PACKET_ARMED;
                    }
                }
                else if(&g_emac1 == g_test_mac)
                {
                    g_capture1e = PACKET_ARMED;
                    if(g_reload1e)
                    {
                        g_reload1e = PACKET_ARMED;
                    }
                }
                else
                {
                    g_capture1 = PACKET_ARMED;
                    if(g_reload1)
                    {
                        g_reload1 = PACKET_ARMED;
                    }
                }
            }
            else if(rx_buff[0] == 'C')
            {
                PRINT_STRING("Timestamped only packet capture armed\n\r");
                if(&g_mac0 == g_test_mac)
                {
                    g_capture0 = PACKET_ARMED_PTP;
                    if(g_reload0)
                    {
                        g_reload0 = PACKET_ARMED_PTP;
                    }
                }
                else if(&g_emac0 == g_test_mac)
                {
                    g_capture0e = PACKET_ARMED_PTP;
                    if(g_reload0e)
                    {
                        g_reload0e = PACKET_ARMED_PTP;
                    }
                }
                else if(&g_emac1 == g_test_mac)
                {
                    g_capture1e = PACKET_ARMED_PTP;
                    if(g_reload1e)
                    {
                        g_reload1e = PACKET_ARMED_PTP;
                    }
                }
                else
                {
                    g_capture1 = PACKET_ARMED_PTP;
                    if(g_reload1)
                    {
                        g_reload1 = PACKET_ARMED_PTP;
                    }
                }
            }
            else if(rx_buff[0] == 'd')
            {
                uint32_t temp_cutthru;

                temp_cutthru = MSS_MAC_get_tx_cutthru(g_test_mac);
                if(0 == temp_cutthru)
                {
                    PRINT_STRING("TX Cutthru enabled\n\r");
                    /* May need to tweak... */
                    temp_cutthru = g_test_mac->is_emac ? 0x80000180ul : 0x80000180ul;
                    MSS_MAC_set_tx_cutthru(g_test_mac, temp_cutthru);
                }
                else
                {
                    PRINT_STRING("TX Cutthru disabled\n\r");
                    MSS_MAC_set_tx_cutthru(g_test_mac, 0);
                }
            }
            else if(rx_buff[0] == 'D')
            {
                uint32_t temp_cutthru;

                temp_cutthru = MSS_MAC_get_rx_cutthru(g_test_mac);
                if(0 == temp_cutthru)
                {
                    PRINT_STRING("RX Cutthru enabled\n\r");
                    /* May need to tweak... */
                    temp_cutthru = g_test_mac->is_emac ? 0x20ul : 0x20ul;
                    MSS_MAC_set_rx_cutthru(g_test_mac, temp_cutthru);
                }
                else
                {
                    PRINT_STRING("RX Cutthru disabled\n\r");
                    MSS_MAC_set_rx_cutthru(g_test_mac, 0);
                }
            }
            else if(rx_buff[0] == 'e')
            {
                mss_mac_mmsl_config_t mmsl_cfg;

                if((&g_mac0 == g_test_mac) || (&g_emac0 == g_test_mac))
                {
                    MSS_MAC_get_mmsl_mode(&g_mac0, &mmsl_cfg);
                    mmsl_cfg.use_pmac = !mmsl_cfg.use_pmac;
                    mmsl_cfg.verify_disable = 1;
                    MSS_MAC_set_mmsl_mode(&g_mac0, &mmsl_cfg);
                }
                else
                {
                    MSS_MAC_get_mmsl_mode(&g_mac1, &mmsl_cfg);
                    mmsl_cfg.use_pmac = !mmsl_cfg.use_pmac;
                    mmsl_cfg.verify_disable = 1;
                    MSS_MAC_set_mmsl_mode(&g_mac1, &mmsl_cfg);
                }

                sprintf(info_string, "RX all to pMAC %s\n\r",
                        mmsl_cfg.use_pmac ? "enabled" : "disabled");
                PRINT_STRING(info_string);
            }
            else if(rx_buff[0] == 'E')
            {
                mss_mac_mmsl_config_t mmsl_cfg;

                if((&g_mac0 == g_test_mac) || (&g_emac0 == g_test_mac))
                {
                    MSS_MAC_get_mmsl_mode(&g_mac0, &mmsl_cfg);
                    mmsl_cfg.preemption = !mmsl_cfg.preemption;
                    mmsl_cfg.verify_disable = 1;
                    MSS_MAC_set_mmsl_mode(&g_mac0, &mmsl_cfg);
                }
                else
                {
                    MSS_MAC_get_mmsl_mode(&g_mac1, &mmsl_cfg);
                    mmsl_cfg.preemption = !mmsl_cfg.preemption;
                    mmsl_cfg.verify_disable = 1;
                    MSS_MAC_set_mmsl_mode(&g_mac1, &mmsl_cfg);
                }

                sprintf(info_string, "Preemption %s\n\r",
                        mmsl_cfg.preemption ? "enabled" : "disabled");
                PRINT_STRING(info_string);
            }
            else if(rx_buff[0] == 'f')
            {
                /* We will fill this using big words for speed */
                static uint64_t big_packet[10240/4];
                uint64_t big_data = 0;
                int count;
                uint8_t tx_status;

                big_packet[0] = 0xFFFFFFFFFFFFFFFFULL;
                big_packet[1] = 0xFFFFFFFFFFFFFFFFULL;

                /* Fill with incrementing 16 bit values */
                for(count = 2; count != (10240/4); ++count)
                {
                    big_packet[count] = big_data;
                    big_data += 0x0001000100010001;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10234, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                /* Fill with incrementing 16 bit values */
                for(count = 2; count != (10240/4); ++count)
                {
                    big_packet[count] = big_data;
                    big_data += 0x0001000100010001;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10234, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                /* Fill with incrementing 16 bit values */
                for(count = 2; count != (10240/4); ++count)
                {
                    big_packet[count] = big_data;
                    big_data += 0x0001000100010001;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10234, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                /* Fill with incrementing 16 bit values */
                for(count = 2; count != (10240/4); ++count)
                {
                    big_packet[count] = big_data;
                    big_data += 0x0001000100010001;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10234, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                /* Fill with incrementing 16 bit values */
                for(count = 2; count != (10240/4); ++count)
                {
                    big_packet[count] = big_data;
                    big_data += 0x0001000100010001;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10234, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                /* Fill with incrementing 16 bit values */
                for(count = 2; count != (10240/4); ++count)
                {
                    big_packet[count] = big_data;
                    big_data += 0x0001000100010001;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10234, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                /* Fill with incrementing 16 bit values */
                for(count = 2; count != (10240/4); ++count)
                {
                    big_packet[count] = big_data;
                    big_data += 0x0001000100010001;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10234, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                /* Fill with incrementing 16 bit values */
                for(count = 2; count != (10240/4); ++count)
                {
                    big_packet[count] = big_data;
                    big_data += 0x0001000100010001;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10234, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                /* Fill with incrementing 16 bit values */
                for(count = 2; count != (10240/4); ++count)
                {
                    big_packet[count] = big_data;
                    big_data += 0x0001000100010001;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10234, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                /* Fill with incrementing 16 bit values */
                for(count = 2; count != (10240/4); ++count)
                {
                    big_packet[count] = big_data;
                    big_data += 0x0001000100010001;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10234, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

            }
            else if(rx_buff[0] == 'F')
            {
                static uint8_t big_packet[10240] =
                {0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x10, 0x94, 0x00,
                 0x00, 0x02, 0x08, 0x00, 0x45, 0x00, 0x02, 0x0F, 0x00, 0x00,
                 0x00, 0x00, 0xFF, 0xFD, 0x37, 0x99, 0xC0, 0x55, 0x01, 0x02,
                 0xC0, 0x00, 0x00, 0x01};
                uint8_t little_data = 0;
                int count;
                uint8_t tx_status;
                volatile int spinner;


                /* Fill with incrementing 16 bit values */
                for(count = 34; count != 10240; ++count)
                {
                    big_packet[count] = little_data;
                    little_data++;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10240, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                while(g_test_mac->queue[0].nb_available_tx_desc !=
                        MSS_MAC_TX_RING_SIZE)
                {
                    spinner++;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10240, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                while(g_test_mac->queue[0].nb_available_tx_desc !=
                        MSS_MAC_TX_RING_SIZE)
                {
                    spinner++;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10240, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                while(g_test_mac->queue[0].nb_available_tx_desc !=
                        MSS_MAC_TX_RING_SIZE)
                {
                    spinner++;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10240, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                while(g_test_mac->queue[0].nb_available_tx_desc !=
                        MSS_MAC_TX_RING_SIZE)
                {
                    spinner++;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10240, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                while(g_test_mac->queue[0].nb_available_tx_desc !=
                        MSS_MAC_TX_RING_SIZE)
                {
                    spinner++;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10240, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                while(g_test_mac->queue[0].nb_available_tx_desc !=
                        MSS_MAC_TX_RING_SIZE)
                {
                    spinner++;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10240, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                while(g_test_mac->queue[0].nb_available_tx_desc !=
                        MSS_MAC_TX_RING_SIZE)
                {
                    spinner++;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10240, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                while(g_test_mac->queue[0].nb_available_tx_desc !=
                        MSS_MAC_TX_RING_SIZE)
                {
                    spinner++;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10240, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);

                while(g_test_mac->queue[0].nb_available_tx_desc !=
                        MSS_MAC_TX_RING_SIZE)
                {
                    spinner++;
                }

                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        (uint8_t *)big_packet, 10240, (void *)0);
                sprintf(info_string,"Jumbo TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);
            }
            else if(rx_buff[0] == 'g')
            {
                volatile uint32_t temp_in;

                temp_in = MSS_GPIO_get_inputs(GPIO2_LO);
                sprintf(info_string,"MSS GPIO 2 Input = %08X\n\r", temp_in);
                PRINT_STRING(info_string);
                if(TBI == g_test_mac->interface_type)
                {
                    sprintf(info_string,"    gtwiz_reset_rx_cdr_stable_out = %s\n\r",
                            temp_in & 0x0001 ? "active" : "inactive");
                    PRINT_STRING(info_string);
                    sprintf(info_string,"    gtwiz_reset_tx_done_out       = %s\n\r",
                            temp_in & 0x0002 ? "active" : "inactive");
                    PRINT_STRING(info_string)
                    sprintf(info_string,"    gtwiz_reset_rx_done_out       = %s\n\r",
                            temp_in & 0x0004 ? "active" : "inactive");
                    PRINT_STRING(info_string);
                    sprintf(info_string,"    resetdone                     = %s\n\r",
                            temp_in & 0x0008 ? "active" : "inactive");
                    PRINT_STRING(info_string);
                    sprintf(info_string,"    txpmaresetdone_out            = %s\n\r",
                            temp_in & 0x0010 ? "active" : "inactive");
                    PRINT_STRING(info_string);
                    sprintf(info_string,"    rxpmaresetdone_out            = %s\n\r",
                            temp_in & 0x0020 ? "active" : "inactive");
                    PRINT_STRING(info_string);
                    sprintf(info_string,"    rxcommadet_out                = %s\n\r",
                            temp_in & 0x0040 ? "active" : "inactive");
                    PRINT_STRING(info_string);
                    sprintf(info_string,"    rxbyterealign_out             = %s\n\r",
                            temp_in & 0x0080 ? "active" : "inactive");
                    PRINT_STRING(info_string);
                    sprintf(info_string,"    rxbyteisaligned_out           = %s\n\r",
                            temp_in & 0x0100 ? "active" : "inactive");
                    PRINT_STRING(info_string);
                    sprintf(info_string,"    gtwiz_buffbypass_rx_error_out = %s\n\r",
                            temp_in & 0x0200 ? "active" : "inactive");
                    PRINT_STRING(info_string);
                    sprintf(info_string,"    gtwiz_buffbypass_rx_done_out  = %s\n\r",
                            temp_in & 0x0400 ? "active" : "inactive");
                    PRINT_STRING(info_string);
                    sprintf(info_string,"    gtpowergood_out               = %s\n\r\n\r",
                            temp_in & 0x0800 ? "active" : "inactive");
                    PRINT_STRING(info_string);
                }
            }
            else if(rx_buff[0] == 'h')
            {
                print_help();
            }
            else if(rx_buff[0] == 'i')
            {
                PRINT_STRING("Incrementing stats counters\n\r");
                if(g_test_mac->is_emac)
                {
                    g_test_mac->emac_base->NETWORK_CONTROL |= 0x40;
                }
                else
                {
                    g_test_mac->mac_base->NETWORK_CONTROL |= 0x40;
                }
            }
            else if(rx_buff[0] == 'I')
            {
                mss_mac_mmsl_stats_t mmsl_stats;

                MSS_MAC_get_mmsl_stats(g_test_mac, &mmsl_stats);
                PRINT_STRING("MMSL Statistics:\n\r");
                sprintf(info_string, "SMD Error Count          - %d\n\r",
                        (int)mmsl_stats.smd_err_count);
                PRINT_STRING(info_string);
                sprintf(info_string, "Reassembly Error Count   - %d\n\r",
                        (int)mmsl_stats.ass_err_count);
                PRINT_STRING(info_string);
                sprintf(info_string, "Reassembled Frames Count - %d\n\r",
                        (int)mmsl_stats.ass_ok_count);
                PRINT_STRING(info_string);
                sprintf(info_string, "Transmit Fragment Count  - %d\n\r",
                        (int)mmsl_stats.frag_count_tx);
                PRINT_STRING(info_string);
                sprintf(info_string, "Receive Fragment Count   - %d\n\r",
                        (int)mmsl_stats.frag_count_rx);
                PRINT_STRING(info_string);
            }
            else if(rx_buff[0] == 'j')
            {
                if(0 == MSS_MAC_get_jumbo_frames_mode(g_test_mac))
                {
                    PRINT_STRING("Jumbo packets enabled\n\r");
                    MSS_MAC_set_jumbo_frames_mode(g_test_mac, 1);
                }
                else
                {
                    PRINT_STRING("Jumbo packets disabled\n\r");
                    MSS_MAC_set_jumbo_frames_mode(g_test_mac, 0);
                }
            }
            else if(rx_buff[0] == 'k')
            {
                if(&g_mac0 == g_test_mac)
                {
                    if(g_reload0 != PACKET_IDLE)
                    {
                        g_reload0 = PACKET_IDLE;
                    }
                    else
                    {
                        if(PACKET_ARMED == g_capture0)
                        {
                            g_reload0 = PACKET_ARMED;
                        }
                        else if(PACKET_ARMED_PTP == g_capture0)
                        {
                            g_reload0 = PACKET_ARMED_PTP;
                        }
                        else
                        {
                            g_reload0 = PACKET_ARMED;
                        }
                    }
                    sprintf(info_string, "Capture reload is %s\n\r",
                            g_reload0 ? "enabled" : "disabled");
                    PRINT_STRING(info_string);
                }
                else if(&g_emac0 == g_test_mac)
                {
                    if(g_reload0e != PACKET_IDLE)
                    {
                        g_reload0e = PACKET_IDLE;
                    }
                    else
                    {
                        if(PACKET_ARMED == g_capture0e)
                        {
                            g_reload0e = PACKET_ARMED;
                        }
                        else if(PACKET_ARMED_PTP == g_capture0e)
                        {
                            g_reload0e = PACKET_ARMED_PTP;
                        }
                        else
                        {
                            g_reload0e = PACKET_ARMED;
                        }
                    }
                    sprintf(info_string, "Capture reload is %s\n\r",
                            g_reload0e ? "enabled" : "disabled");
                    PRINT_STRING(info_string);
                }
                else if(&g_emac1 == g_test_mac)
                {
                    if(g_reload1e != PACKET_IDLE)
                    {
                        g_reload1e = PACKET_IDLE;
                    }
                    else
                    {
                        if(PACKET_ARMED == g_capture1e)
                        {
                            g_reload1e = PACKET_ARMED;
                        }
                        else if(PACKET_ARMED_PTP == g_capture1e)
                        {
                            g_reload1e = PACKET_ARMED_PTP;
                        }
                        else
                        {
                            g_reload1e = PACKET_ARMED;
                        }
                    }
                    sprintf(info_string, "Capture reload is %s\n\r",
                            g_reload1e ? "enabled" : "disabled");
                    PRINT_STRING(info_string);
                }
                else
                {
                    if(g_reload1 != PACKET_IDLE)
                    {
                        g_reload1 = PACKET_IDLE;
                    }
                    else
                    {
                        if(PACKET_ARMED == g_capture0)
                        {
                            g_reload1 = PACKET_ARMED;
                        }
                        else if(PACKET_ARMED_PTP == g_capture1)
                        {
                            g_reload1 = PACKET_ARMED_PTP;
                        }
                        else
                        {
                            g_reload1 = PACKET_ARMED;
                        }
                    }
                    sprintf(info_string, "Capture reload is %s\n\r",
                            g_reload1 ? "enabled" : "disabled");
                    PRINT_STRING(info_string);
                }
            }
            else if(rx_buff[0] == 'l')
            {
                if(g_test_mac == &g_mac0)
                {
                    g_loopback0 = !g_loopback0;
                    if(g_loopback0)
                    {
                        PRINT_STRING("SW Loopback on\n\r");
                    }
                    else
                    {
                        PRINT_STRING("SW Loopback off\n\r");
                    }
                }
                else if(g_test_mac == &g_emac0)
                {
                    g_loopback0e = !g_loopback0e;
                    if(g_loopback0e)
                    {
                        PRINT_STRING("SW Loopback on\n\r");
                    }
                    else
                    {
                        PRINT_STRING("SW Loopback off\n\r");
                    }
                }
                else if(g_test_mac == &g_emac1)
                {
                    g_loopback1e = !g_loopback1e;
                    if(g_loopback1e)
                    {
                        PRINT_STRING("SW Loopback on\n\r");
                    }
                    else
                    {
                        PRINT_STRING("SW Loopback off\n\r");
                    }
                }
                else
                {
                    g_loopback1 = !g_loopback1;
                    if(g_loopback1)
                    {
                        PRINT_STRING("SW Loopback on\n\r");
                    }
                    else
                    {
                        PRINT_STRING("SW Loopback off\n\r");
                    }
                }
            }
            else if(rx_buff[0] == 'L')
            {
                g_link_status = !g_link_status;
                if(g_link_status)
                {
                    PRINT_STRING("Link status display on\n\r");
                }
                else
                {
                    PRINT_STRING("Link status display off\n\r");
                }
            }
            else if(rx_buff[0] == 'm') /* RX mode step */
            {
                mss_mac_tsu_mode_t temp_mode;

                temp_mode = MSS_MAC_get_TSU_rx_mode(g_test_mac) + 1;
                if(temp_mode >= MSS_MAC_TSU_MODE_END)
                {
                    temp_mode = MSS_MAC_TSU_MODE_DISABLED;
                }

                MSS_MAC_set_TSU_rx_mode(g_test_mac, temp_mode);
                sprintf(info_string,"RX TSU time stamp mode = %s\n\r",
                        tsu_mode_string(temp_mode));
                PRINT_STRING(info_string);
            }
            else if(rx_buff[0] == 'M') /* TX mode step */
            {
                mss_mac_tsu_mode_t temp_mode;

                temp_mode = MSS_MAC_get_TSU_tx_mode(g_test_mac) + 1;
                if(temp_mode >= MSS_MAC_TSU_MODE_END)
                {
                    temp_mode = MSS_MAC_TSU_MODE_DISABLED;
                }

                MSS_MAC_set_TSU_tx_mode(g_test_mac, temp_mode);
                sprintf(info_string,"TX TSU time stamp mode = %s\n\r",
                        tsu_mode_string(temp_mode));
                PRINT_STRING(info_string);
            }
            else if(rx_buff[0] == 'n')
            {
                g_tx_add_1 = !g_tx_add_1;
                if(g_tx_add_1)
                {
                    PRINT_STRING("TX loopback length increment enabled\n\r");
                }
                else
                {
                    PRINT_STRING("TX loopback length increment disabled\n\r");
                }
            }
            else if(rx_buff[0] == 'o')
            {
                mss_mac_oss_mode_t oss_mode;

                oss_mode = MSS_MAC_get_TSU_oss_mode(g_test_mac) + 1;
                if(oss_mode >= MSS_MAC_OSS_MODE_INVALID)
                {
                    oss_mode = MSS_MAC_OSS_MODE_DISABLED;
                }

                MSS_MAC_set_TSU_oss_mode(g_test_mac, oss_mode);
                sprintf(info_string,"OSS mode changed to ");
                PRINT_STRING(info_string);
                if(MSS_MAC_OSS_MODE_DISABLED == oss_mode)
                {
                    PRINT_STRING("(disabled)\n\r");
                }
                else if(MSS_MAC_OSS_MODE_REPLACE == oss_mode)
                {
                    PRINT_STRING("(replace)\n\r");
                }
                else if(MSS_MAC_OSS_MODE_ADJUST == oss_mode)
                {
                    PRINT_STRING("(adjust)\n\r");
                }
                else if(MSS_MAC_OSS_MODE_INVALID == oss_mode)
                    /* Shouldn't get this far... */
                {
                    PRINT_STRING("(invalid)\n\r");
                }
                else
                {
                    PRINT_STRING("(unknown)\n\r");
                }
            }
            else if(rx_buff[0] == 'p')
            {
                if(g_test_mac->is_emac)
                {
                    if(0 == (g_test_mac->emac_base->NETWORK_CONFIG &
                            GEM_COPY_ALL_FRAMES))
                    {
                        g_test_mac->emac_base->NETWORK_CONFIG |=
                                GEM_COPY_ALL_FRAMES;
                        PRINT_STRING("Promiscuous mode on\n\r");
                    }
                    else
                    {
                        g_test_mac->emac_base->NETWORK_CONFIG &=
                                (uint32_t)(~GEM_COPY_ALL_FRAMES);
                        PRINT_STRING("Promiscuous mode off\n\r");
                    }
                }
                else
                {
                    if(0 == (g_test_mac->mac_base->NETWORK_CONFIG &
                            GEM_COPY_ALL_FRAMES))
                    {
                        g_test_mac->mac_base->NETWORK_CONFIG |=
                                GEM_COPY_ALL_FRAMES;
                        PRINT_STRING("Promiscuous mode on\n\r");
                    }
                    else
                    {
                        g_test_mac->mac_base->NETWORK_CONFIG &=
                                (uint32_t)(~GEM_COPY_ALL_FRAMES);
                        PRINT_STRING("Promiscuous mode off\n\r");
                    }
                }
            }
            else if(rx_buff[0] == 'P')
            {
                /* Reset stats next time '>' is used as this is going to invalidate them... */
                pkt_index = 1;
                sync_pkt_index = 1;
                send_ptp_stream(g_test_mac, (void *)0, &pkt_index);
            }
            else if(rx_buff[0] == 'q')
            {
                mss_mac_type_1_filter_t filter1;

                MSS_MAC_get_type_1_filter(g_test_mac, 3, &filter1);
                if(0 == filter1.udp_port)
                {
                    PRINT_STRING("Enabling Screening Type 1 Filters.\n\r");
                    PRINT_STRING("    UDP port 320 routes to Q 1\n\r");
                    PRINT_STRING("    TC/DS 15 routes to Q 2\n\r");
                    PRINT_STRING("    UDP port 1534 is dropped\n\r");

                    memset(&filter1, 0, sizeof(filter1));
                    filter1.udp_port = 320;
                    filter1.udp_port_enable = 1;
                    filter1.queue_no = 1;     /* All 320 frames go to queue 1 */

                    MSS_MAC_set_type_1_filter(g_test_mac, 3, &filter1);

                    memset(&filter1, 0, sizeof(filter1));
                    filter1.dstc = 0x0F;
                    filter1.dstc_enable = 1;
                    filter1.queue_no = 2; /* All 0x0F ds/tc frames to queue 2 */

                    MSS_MAC_set_type_1_filter(g_test_mac, 2, &filter1);

                    memset(&filter1, 0, sizeof(filter1));
                    filter1.udp_port = 1534;
                    filter1.udp_port_enable = 1;
                    filter1.drop_on_match = 1;
                    filter1.queue_no = 3;      /* All 1534 frames get dropped */

                    MSS_MAC_set_type_1_filter(g_test_mac, 1, &filter1);
                }
                else
                {
                    PRINT_STRING("Disabling Screening Type 1 Filters.\n\r");
                    memset(&filter1, 0, sizeof(filter1));
                    MSS_MAC_set_type_1_filter(g_test_mac, 0, &filter1);
                    MSS_MAC_set_type_1_filter(g_test_mac, 1, &filter1);
                    MSS_MAC_set_type_1_filter(g_test_mac, 2, &filter1);
                    MSS_MAC_set_type_1_filter(g_test_mac, 3, &filter1);
                }
            }
            else if(rx_buff[0] == 'Q')
            {
                mss_mac_type_2_filter_t filter;
                mss_mac_type_2_compare_t compare;

                if(0 == MSS_MAC_get_type_2_ethertype(g_test_mac, 0))
                {
                    PRINT_STRING("Enabling Screening Type 2 Filters.\n\r");
                    PRINT_STRING("    IP packet with 0x9E, 0x48, 0x05, 0x0F at offset 0x48 routes to Q 3\n\r");
                    PRINT_STRING("    ARP packet with 12 x 0xFF at offset 0x30 routes to Q 2\n\r");

                    MSS_MAC_set_type_2_ethertype(g_test_mac, 0, 0x0800); /* IPv4 */

                    compare.disable_mask = 1;
                    compare.data = 0x0F05489EUL;
                    compare.compare_offset = MSS_MAC_T2_OFFSET_FRAME;
                    compare.offset_value = 0x48;
                    MSS_MAC_set_type_2_compare(g_test_mac, 0, &compare);

                    memset(&filter, 0, sizeof(filter));
                    filter.compare_a_enable = 1;
                    filter.compare_a_index  = 0;
                    filter.ethertype_enable = 1;
                    filter.ethertype_index  = 0;
                    filter.queue_no         = 3;

                    MSS_MAC_set_type_2_filter(g_test_mac, 0, &filter);
                    compare.disable_mask = 1;
                    compare.data = 0xFFFFFFFFUL;
                    compare.compare_offset = MSS_MAC_T2_OFFSET_FRAME;
                    compare.offset_value = 0x30;
                    MSS_MAC_set_type_2_compare(g_test_mac, 1, &compare);
                    compare.offset_value = 0x34;
                    MSS_MAC_set_type_2_compare(g_test_mac, 2, &compare);
                    compare.offset_value = 0x38;
                    MSS_MAC_set_type_2_compare(g_test_mac, 3, &compare);
                    MSS_MAC_set_type_2_ethertype(g_test_mac, 1, 0x0806); /* ARP */

                    memset(&filter, 0, sizeof(filter));
                    filter.compare_a_enable = 1;
                    filter.compare_a_index  = 1;
                    filter.compare_b_enable = 1;
                    filter.compare_b_index  = 2;
                    filter.compare_c_enable = 1;
                    filter.compare_c_index  = 3;
                    filter.ethertype_enable = 1;
                    filter.ethertype_index  = 1;
                    filter.queue_no         = 2;

                    MSS_MAC_set_type_2_filter(g_test_mac, 1, &filter);
                }
                else
                {
                    PRINT_STRING("Disabling Screening Type 2 Filters.\n\r");
                    memset(&filter, 0, sizeof(filter));
                    MSS_MAC_set_type_2_filter(g_test_mac, 0, &filter);
                    MSS_MAC_set_type_2_filter(g_test_mac, 1, &filter);
                    MSS_MAC_set_type_2_filter(g_test_mac, 2, &filter);
                    MSS_MAC_set_type_2_filter(g_test_mac, 3, &filter);
                    memset(&compare, 0, sizeof(compare));
                    MSS_MAC_set_type_2_compare(g_test_mac, 0, &compare);
                    MSS_MAC_set_type_2_compare(g_test_mac, 1, &compare);
                    MSS_MAC_set_type_2_compare(g_test_mac, 2, &compare);
                    MSS_MAC_set_type_2_compare(g_test_mac, 3, &compare);
                    MSS_MAC_set_type_2_ethertype(g_test_mac, 0, 0x0000);
                    MSS_MAC_set_type_2_ethertype(g_test_mac, 1, 0x0000);
                    MSS_MAC_set_type_2_ethertype(g_test_mac, 2, 0x0000);
                    MSS_MAC_set_type_2_ethertype(g_test_mac, 3, 0x0000);
                }
            }
            else if(rx_buff[0] == 'r')
            {
                int q_index;

                PRINT_STRING("Stats reset\n\r");
                memset(stats, 0, sizeof(stats0));
                MSS_MAC_clear_statistics(g_test_mac);
                for(q_index = 0; q_index != 4; q_index++)
                {
                    g_test_mac->queue[q_index].ingress     = 0;
                    g_test_mac->queue[q_index].egress      = 0;
                    g_test_mac->queue[q_index].hresp_error = 0;
                    g_test_mac->queue[q_index].rx_restart  = 0;
                    g_test_mac->queue[q_index].rx_overflow = 0;
                }

                g_test_mac->tx_pause      = 0;
                g_test_mac->rx_pause      = 0;
                g_test_mac->pause_elapsed = 0;

                if(g_test_mac == &g_mac0)
                {
                    g_tx_retry0    = 0;
                    tx_count0      = 0;
                    rx_count0      = 0;
#if defined(MSS_MAC_TIME_STAMPED_MODE)
                    g_rx_ts_count0 = 0;
                    g_tx_ts_count0 = 0;
#endif
                }
                else if(g_test_mac == &g_emac0)
                {
                    g_tx_retry0e    = 0;
                    tx_count0e      = 0;
                    rx_count0e      = 0;
#if defined(MSS_MAC_TIME_STAMPED_MODE)
                    g_rx_ts_count0e = 0;
                    g_tx_ts_count0e = 0;
#endif
                }
                else if(g_test_mac == &g_emac1)
                {
                    g_tx_retry1e    = 0;
                    tx_count1e      = 0;
                    rx_count1e      = 0;
#if defined(MSS_MAC_TIME_STAMPED_MODE)
                    g_rx_ts_count1e = 0;
                    g_tx_ts_count1e = 0;
#endif
                }
                else
                {
                    g_tx_retry1    = 0;
                    tx_count1      = 0;
                    rx_count1      = 0;
#if defined(MSS_MAC_TIME_STAMPED_MODE)
                    g_rx_ts_count1 = 0;
                    g_tx_ts_count1 = 0;
#endif
                }

                copper_rx_good      = 0;
                phy_rx_err          = 0;
                mac_rx_good         = 0;
                mac_rx_err          = 0;
                mac_tx_good         = 0;
                mac_tx_err          = 0;
                phy_false_carrier   = 0;
                phy_link_disconnect = 0;

                if(MSS_MAC_DEV_PHY_VSC8575 == g_test_mac->phy_type)
                {
                    /* This clears some stats in phy as they are reset on read... */
                    dump_vsc8575_regs(g_test_mac);
                }
            }
            else if((rx_buff[0] == 's') || (rx_buff[0] == 'S'))
            {
                uint64_t rx_bytes;
                int64_t rx_packets;
                uint64_t tx_bytes;
                int64_t tx_packets;
                uint64_t rx_overflows;
                int64_t tx_retries;
                uint64_t rx_ts;
                uint64_t tx_ts;
                uint64_t tx_pause;
                uint64_t rx_pause;
                uint64_t pause_elapsed;
                uint64_t hresp_nok;
                uint64_t rx_restart;


                if(&g_mac0 == g_test_mac)
                {
                    PRINT_STRING("Current MAC is GEM0\n\r");
                }
                else if(&g_emac0 == g_test_mac)
                {
                    PRINT_STRING("Current MAC is eGEM0\n\r");
                }
                else if(&g_emac1 == g_test_mac)
                {
                    PRINT_STRING("Current MAC is eGEM1\n\r");
                }
                else
                {
                    PRINT_STRING("Current MAC is GEM1\n\r");
                }
                sprintf(info_string, "Link 0 is currently %s, Duplex = %s, ",
                        g_test_linkup0     ? "Up"   : "Down",
                        g_test_fullduplex0 ? "Full" : "Half");
                PRINT_STRING(info_string);
                if(MSS_MAC_1000MBPS == g_test_speed0)
                {
                    PRINT_STRING("Speed = 1Gb\n\r");
                }
                else if(MSS_MAC_100MBPS == g_test_speed0)
                {
                    PRINT_STRING("Speed = 100Mb\n\r");
                }
                else if(MSS_MAC_10MBPS == g_test_speed0)
                {
                    PRINT_STRING("Speed = 10Mb\n\r");
                }
                else
                {
                    PRINT_STRING("Speed = Unknown\n\r");
                }

                sprintf(info_string, "Link 1 is currently %s, Duplex = %s, ",
                        g_test_linkup1     ? "Up"   : "Down",
                        g_test_fullduplex1 ? "Full" : "Half");
                PRINT_STRING(info_string);
                if(MSS_MAC_1000MBPS == g_test_speed1)
                {
                    PRINT_STRING("Speed = 1Gb\n\r");
                }
                else if(MSS_MAC_100MBPS == g_test_speed1)
                {
                    PRINT_STRING("Speed = 100Mb\n\r");
                }
                else if(MSS_MAC_10MBPS == g_test_speed1)
                {
                    PRINT_STRING("Speed = 10Mb\n\r");
                }
                else
                {
                    PRINT_STRING("Speed = Unknown\n\r");
                }

                rx_bytes       = g_test_mac->queue[0].ingress +
                                 g_test_mac->queue[1].ingress +
                                 g_test_mac->queue[2].ingress +
                                 g_test_mac->queue[3].ingress;
                tx_bytes       = g_test_mac->queue[0].egress +
                                 g_test_mac->queue[1].egress +
                                 g_test_mac->queue[2].egress +
                                 g_test_mac->queue[3].egress;
                rx_overflows   = g_test_mac->queue[0].rx_overflow +
                                 g_test_mac->queue[1].rx_overflow +
                                 g_test_mac->queue[2].rx_overflow +
                                 g_test_mac->queue[3].rx_overflow;
                hresp_nok      = g_test_mac->queue[0].hresp_error +
                                 g_test_mac->queue[1].hresp_error +
                                 g_test_mac->queue[2].hresp_error +
                                 g_test_mac->queue[3].hresp_error;
                rx_restart     = g_test_mac->queue[0].rx_restart +
                                 g_test_mac->queue[1].rx_restart +
                                 g_test_mac->queue[2].rx_restart +
                                 g_test_mac->queue[3].rx_restart;
                tx_pause       = g_test_mac->tx_pause;
                rx_pause       = g_test_mac->rx_pause;
                pause_elapsed  = g_test_mac->pause_elapsed;

                if(g_test_mac == &g_mac0)
                {
                    rx_packets     = rx_count0;
                    tx_packets     = tx_count0;
                    tx_retries     = g_tx_retry0;
                    rx_ts          = g_rx_ts_count0;
                    tx_ts          = g_tx_ts_count0;
                }
                else if(g_test_mac == &g_emac0)
                {
                    rx_packets     = rx_count0e;
                    tx_packets     = tx_count0e;
                    tx_retries     = g_tx_retry0e;
                    rx_ts          = g_rx_ts_count0e;
                    tx_ts          = g_tx_ts_count0e;
                }
                else if(g_test_mac == &g_mac1)
                {
                    rx_packets     = rx_count1;
                    tx_packets     = tx_count1;
                    tx_retries     = g_tx_retry1;
                    rx_ts          = g_rx_ts_count1;
                    tx_ts          = g_tx_ts_count1;
                }
                else
                {
                    rx_packets     = rx_count1e;
                    tx_packets     = tx_count1e;
                    tx_retries     = g_tx_retry1e;
                    rx_ts          = g_rx_ts_count1e;
                    tx_ts          = g_tx_ts_count1e;
                }

                sprintf(info_string,
                        "RX %U (%U pkts), TX %U (%U pkts)\n\rRX Over Flow %U, TX Retries %U\n\r",
                        rx_bytes, rx_packets, tx_bytes, tx_packets, rx_overflows, tx_retries);
                PRINT_STRING(info_string);
#if defined(MSS_MAC_TIME_STAMPED_MODE)
                sprintf(info_string,
                        "RX TS %U, TX TS %U\n\rTX Pause %U, RX Pause %U, Pause Elapsed %U\n\r",
                        rx_ts, tx_ts, tx_pause, rx_pause, pause_elapsed);
                PRINT_STRING(info_string);
#endif
                sprintf(info_string,"HRESP not ok %U RX Restarts %U\n\r\n\r",
                        hresp_nok, rx_restart);
                PRINT_STRING(info_string);

                if(rx_buff[0] == 's')
                {
                    stats_dump();
                }
            }
            else if(rx_buff[0] == 't')
            {
                uint8_t tx_status;

                add_on = 0; /* Reset the count for 'T' command */
                memcpy(&tx_pak_arp[6], g_test_mac->mac_addr, 6);
                tx_status = MSS_MAC_send_pkt(g_test_mac, 0, tx_pak_arp,
                        (sizeof(tx_pak_arp)) | g_crc, (void *)0);

                sprintf(info_string,"TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);
                if(1 == tx_status)
                {
                    if(&g_mac0 == g_test_mac)
                    {
                        print_tx_info(0);
                    }
                    else if(&g_emac0 == g_test_mac)
                    {
                        print_tx_info(2);
                    }
                    else if(&g_emac1 == g_test_mac)
                    {
                        print_tx_info(3);
                    }
                    else
                    {
                        print_tx_info(1);
                    }
                }
            }
            else if(rx_buff[0] == 'T')
            {
                uint8_t tx_status;
                volatile uint32_t *ctrl_reg = 0;

                memcpy(&tx_pak_arp[6], g_test_mac->mac_addr, 6);
                tx_status = MSS_MAC_send_pkt(g_test_mac, 0, tx_packet_data,
                        (60 + add_on) | g_crc, (void *)0);
                sprintf(info_string,"TX status %d, size %d\n\r",
                        (int)tx_status, (int)(add_on + 60));
                PRINT_STRING(info_string);
                if(1 == tx_status)
                {
                    if(&g_mac0 == g_test_mac)
                    {
                        print_tx_info(0);
                        ctrl_reg = &g_test_mac->mac_base->NETWORK_CONFIG;
                    }
                    else if(&g_emac0 == g_test_mac)
                    {
                        print_tx_info(2);
                        ctrl_reg = &g_test_mac->emac_base->NETWORK_CONFIG;
                    }
                    else if(&g_emac1 == g_test_mac)
                    {
                        print_tx_info(3);
                        ctrl_reg = &g_test_mac->emac_base->NETWORK_CONFIG;
                    }
                    else
                    {
                        print_tx_info(1);
                        ctrl_reg = &g_test_mac->mac_base->NETWORK_CONFIG;
                    }
                }
                /* Coarse adjust for jumbo frame mode */
                if(0 != (*ctrl_reg & GEM_JUMBO_FRAMES))
                {
                    /* Allow a little extra for testing 10K upper limit */
                    add_on += 100;
                    if(add_on > 10440)
                    {
                        add_on = 0;
                    }
                }
                else
                {
                    add_on++;
                    if(add_on > 68)
                    {
                        add_on = 0;
                    }
                }
            }
            else if(rx_buff[0] == 'u')
            {
                uint8_t tx_status;
                uint8_t temp_pkt[128];

                memcpy(temp_pkt, tx_pak_arp, 128);
                if(g_test_mac == &g_mac0) /* Select correct to/from combination */
                {
                    memcpy(&temp_pkt[0], g_mac1.mac_addr, 6);
                    memcpy(&temp_pkt[6], g_mac0.mac_addr, 6);
                }
                else if(g_test_mac == &g_emac0) /* Select correct to/from combination */
                {
                    memcpy(&temp_pkt[0], g_emac1.mac_addr, 6);
                    memcpy(&temp_pkt[6], g_emac0.mac_addr, 6);
                }
                else if(g_test_mac == &g_emac1) /* Select correct to/from combination */
                {
                    memcpy(&temp_pkt[0], g_emac0.mac_addr, 6);
                    memcpy(&temp_pkt[6], g_emac1.mac_addr, 6);
                }
                else
                {
                    memcpy(&temp_pkt[0], g_mac0.mac_addr, 6);
                    memcpy(&temp_pkt[6], g_mac1.mac_addr, 6);
                }
                add_on = 0; /* Reset the count for 'T' or 'U' command */
                tx_status = MSS_MAC_send_pkt(g_test_mac, 0, temp_pkt,
                        (sizeof(tx_pak_arp)) | g_crc, (void *)0);

                sprintf(info_string,"Unicast TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);
                if(1 == tx_status)
                {
                    if(&g_mac0 == g_test_mac)
                    {
                        print_tx_info(0);
                    }
                    else if(&g_mac0 == g_test_mac)
                    {
                        print_tx_info(2);
                    }
                    else if(&g_mac0 == g_test_mac)
                    {
                        print_tx_info(3);
                    }
                    else
                    {
                        print_tx_info(1);
                    }
                }
            }
            else if(rx_buff[0] == 'U')
            {
                uint8_t tx_status;
                uint8_t temp_pkt[128];

                memcpy(temp_pkt, tx_packet_data, 128);
                if(g_test_mac == &g_mac0) /* Select correct to/from combination */
                {
                    memcpy(&temp_pkt[0], g_mac1.mac_addr, 6);
                    memcpy(&temp_pkt[6], g_mac0.mac_addr, 6);
                }
                else if(g_test_mac == &g_emac0) /* Select correct to/from combination */
                {
                    memcpy(&temp_pkt[0], g_emac1.mac_addr, 6);
                    memcpy(&temp_pkt[6], g_emac0.mac_addr, 6);
                }
                else if(g_test_mac == &g_emac1) /* Select correct to/from combination */
                {
                    memcpy(&temp_pkt[0], g_emac0.mac_addr, 6);
                    memcpy(&temp_pkt[6], g_emac1.mac_addr, 6);
                }
                else
                {
                    memcpy(&temp_pkt[0], g_mac0.mac_addr, 6);
                    memcpy(&temp_pkt[6], g_mac1.mac_addr, 6);
                }
                tx_status = MSS_MAC_send_pkt(g_test_mac, 0, temp_pkt,
                        (60 + add_on) | g_crc, (void *)0);
                add_on++;
                if(add_on > 68)
                {
                    add_on = 0;
                }

                sprintf(info_string,"Unicast TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);
                if(1 == tx_status)
                {
                    if(&g_mac0 == g_test_mac)
                    {
                        print_tx_info(0);
                    }
                    else if(&g_mac0 == g_test_mac)
                    {
                        print_tx_info(2);
                    }
                    else if(&g_mac0 == g_test_mac)
                    {
                        print_tx_info(3);
                    }
                    else
                    {
                        print_tx_info(1);
                    }
                }
            }
            else if(rx_buff[0] == 'v')
            {
                if(&g_mac0 == g_test_mac)
                {
                    g_vlan_tags0++;
                    if(g_vlan_tags0 >= 3)
                    {
                        g_vlan_tags0 = 0;
                    }

                    sprintf(info_string,"VLAN tag level = %d\n\r", g_vlan_tags0);
                    if(2 == g_vlan_tags0)
                    {
                        MSS_MAC_set_stacked_VLAN(g_test_mac, GEM_VLAN_S_TAG);
                    }
                    else
                    {
                        MSS_MAC_set_stacked_VLAN(g_test_mac, GEM_VLAN_NO_STACK);
                    }
                }
                else if(&g_emac0 == g_test_mac)
                {
                    g_vlan_tags0e++;
                    if(g_vlan_tags0e >= 3)
                    {
                        g_vlan_tags0e = 0;
                    }

                    sprintf(info_string,"VLAN tag level = %d\n\r", g_vlan_tags0e);
                    if(2 == g_vlan_tags0e)
                    {
                        MSS_MAC_set_stacked_VLAN(g_test_mac, GEM_VLAN_S_TAG);
                    }
                    else
                    {
                        MSS_MAC_set_stacked_VLAN(g_test_mac, GEM_VLAN_NO_STACK);
                    }
                }
                else if(&g_emac1 == g_test_mac)
                {
                    g_vlan_tags1e++;
                    if(g_vlan_tags1e >= 3)
                    {
                        g_vlan_tags1e = 0;
                    }

                    sprintf(info_string,"VLAN tag level = %d\n\r", g_vlan_tags1e);
                    if(2 == g_vlan_tags1e)
                    {
                        MSS_MAC_set_stacked_VLAN(g_test_mac, GEM_VLAN_S_TAG);
                    }
                    else
                    {
                        MSS_MAC_set_stacked_VLAN(g_test_mac, GEM_VLAN_NO_STACK);
                    }
                }
                else
                {
                    g_vlan_tags1++;
                    if(g_vlan_tags1 >= 3)
                    {
                        g_vlan_tags1 = 0;
                    }

                    sprintf(info_string,"VLAN tag level = %d\n\r", g_vlan_tags1);
                    if(2 == g_vlan_tags1)
                    {
                        MSS_MAC_set_stacked_VLAN(g_test_mac, GEM_VLAN_S_TAG);
                    }
                    else
                    {
                        MSS_MAC_set_stacked_VLAN(g_test_mac, GEM_VLAN_NO_STACK);
                    }
                }

                PRINT_STRING(info_string);
            }
            else if(rx_buff[0] == 'V')
            {
                bool vlan_only_mode;

                vlan_only_mode = !MSS_MAC_get_VLAN_only_mode(g_test_mac);
                MSS_MAC_set_VLAN_only_mode(g_test_mac, vlan_only_mode);
                sprintf(info_string,"VLAN only mode %s\n\r",
                        vlan_only_mode ? "enabled" : "disabled");
                PRINT_STRING(info_string);
            }
            else if(rx_buff[0] == 'x')
            {
                g_phy_dump = !g_phy_dump;
                if(g_phy_dump)
                {
                    PRINT_STRING("Phy register dump enabled\n\r");
                }
                else
                {
                    PRINT_STRING("Phy register dump disabled\n\r");
                }
            }
            else if(rx_buff[0] == 'z')
            {
                if(g_crc)
                {
                    g_crc = 0;
                }
                else
                {
                    g_crc = 0x80000000UL;
                }

                if(g_crc)
                {
                    PRINT_STRING("CRC passthrough enabled\n\r");
                    if(g_test_mac->is_emac)
                    {
                        g_test_mac->emac_base->NETWORK_CONFIG &=
                                (uint32_t)(~GEM_FCS_REMOVE);
                        g_test_mac->emac_base->NETWORK_CONFIG |=
                                GEM_IGNORE_RX_FCS;
                    }
                    else
                    {
                        g_test_mac->mac_base->NETWORK_CONFIG &=
                                (uint32_t)(~GEM_FCS_REMOVE);
                        g_test_mac->mac_base->NETWORK_CONFIG |=
                                GEM_IGNORE_RX_FCS;
                    }
                }
                else
                {
                    PRINT_STRING("CRC passthrough disabled\n\r");
                    if(g_test_mac->is_emac)
                    {
                        g_test_mac->emac_base->NETWORK_CONFIG |= GEM_FCS_REMOVE;
                        g_test_mac->emac_base->NETWORK_CONFIG &=
                                (uint32_t)(~GEM_IGNORE_RX_FCS);
                    }
                    else
                    {
                        g_test_mac->mac_base->NETWORK_CONFIG |= GEM_FCS_REMOVE;
                        g_test_mac->mac_base->NETWORK_CONFIG &=
                                (uint32_t)(~GEM_IGNORE_RX_FCS);
                    }
                }
            }
            else if((rx_buff[0] == 'w') || (rx_buff[0] == 'W'))
            {
                uint8_t tx_status_p;
                uint8_t tx_status_e;
                volatile uint32_t delay;

                if(rx_buff[0] == 'w')
                {
                    delay = 0;
                    PRINT_STRING("Delay Factor   0\n\r");
                }
                else
                {
                    PRINT_STRING("Delay Factor   20\n\r");
                    /* This works for 100Mb design and 1Gb design with 25MHz clock, YMMV... */
                    delay = 20;
                }

                if((&g_mac0 == g_test_mac) || (&g_emac0 == g_test_mac))
                {
                    memcpy(&tx_pak_arp[6], g_mac0.mac_addr, 6);
                    tx_status_p = MSS_MAC_send_pkt(&g_mac0, 0,
                            tx_pak_arp, 3870, (void *)0);
                    memcpy(&tx_pak_arp[6], g_emac0.mac_addr, 6);
                    while(delay)
                    {
                        --delay;
                    };
                    tx_status_e = MSS_MAC_send_pkt(&g_emac0, 0,
                            tx_pak_arp, 60, (void *)0);
                }
                else
                {
                    memcpy(&tx_pak_arp[6], g_mac1.mac_addr, 6);
                    tx_status_p = MSS_MAC_send_pkt(&g_mac1, 0,
                            tx_pak_arp, 3870, (void *)0);
                    memcpy(&tx_pak_arp[6], g_emac1.mac_addr, 6);
                    while(delay)
                    {
                        --delay;
                    };
                    tx_status_e = MSS_MAC_send_pkt(&g_emac1, 0,
                            tx_pak_arp, 60, (void *)0);
                }

                sprintf(info_string,"pMAC TX status %d\n\r", (int)tx_status_p);
                PRINT_STRING(info_string);
                sprintf(info_string,"eMAC TX status %d\n\r", (int)tx_status_e);
                PRINT_STRING(info_string);
            }
            else if(rx_buff[0] == '/')
            {
                g_address_swap = !g_address_swap;
                if(g_address_swap)
                {
                    PRINT_STRING("MAC and IP swap enabled\n\r");
                    /* GEM0 is 10.1.1.3 and GEM1 is 10.1.1.2 for opposing board in dual board setup */
                    MSS_MAC_set_TSU_unicast_addr(&g_mac1,
                            MSS_MAC_TSU_UNICAST_RX, 0x0A010102);
                    MSS_MAC_set_TSU_unicast_addr(&g_mac1,
                            MSS_MAC_TSU_UNICAST_TX, 0x0A010103);
                    MSS_MAC_set_TSU_unicast_addr(&g_mac0,
                            MSS_MAC_TSU_UNICAST_RX, 0x0A010103);
                    MSS_MAC_set_TSU_unicast_addr(&g_mac0,
                            MSS_MAC_TSU_UNICAST_TX, 0x0A010102);

                    MSS_MAC_set_TSU_unicast_addr(&g_emac1,
                            MSS_MAC_TSU_UNICAST_RX, 0x0A010104);
                    MSS_MAC_set_TSU_unicast_addr(&g_emac1,
                            MSS_MAC_TSU_UNICAST_TX, 0x0A010105);
                    MSS_MAC_set_TSU_unicast_addr(&g_emac0,
                            MSS_MAC_TSU_UNICAST_RX, 0x0A010105);
                    MSS_MAC_set_TSU_unicast_addr(&g_emac0,
                            MSS_MAC_TSU_UNICAST_TX, 0x0A010104);

                    g_mac_config.mac_addr[5] = 0x57;
                    MSS_MAC_update_hw_address(&g_mac0, &g_mac_config);
                    g_mac_config.mac_addr[5] = 0x56;
                    MSS_MAC_update_hw_address(&g_mac1, &g_mac_config);
                    g_mac_config.mac_addr[5] = 0x59;
                    MSS_MAC_update_hw_address(&g_emac0, &g_mac_config);
                    g_mac_config.mac_addr[5] = 0x58;
                    MSS_MAC_update_hw_address(&g_emac1, &g_mac_config);
                }
                else
                {
                    PRINT_STRING("MAC and IP swap disabled\n\r");
                    /* GEM0 is 10.1.1.2 and GEM1 is 10.1.1.3 for our exercises */
                    MSS_MAC_set_TSU_unicast_addr(&g_mac0,
                            MSS_MAC_TSU_UNICAST_RX, 0x0A010102);
                    MSS_MAC_set_TSU_unicast_addr(&g_mac0,
                            MSS_MAC_TSU_UNICAST_TX, 0x0A010103);
                    MSS_MAC_set_TSU_unicast_addr(&g_mac1,
                            MSS_MAC_TSU_UNICAST_RX, 0x0A010103);
                    MSS_MAC_set_TSU_unicast_addr(&g_mac1,
                            MSS_MAC_TSU_UNICAST_TX, 0x0A010102);

                    MSS_MAC_set_TSU_unicast_addr(&g_emac0,
                            MSS_MAC_TSU_UNICAST_RX, 0x0A010104);
                    MSS_MAC_set_TSU_unicast_addr(&g_emac0,
                            MSS_MAC_TSU_UNICAST_TX, 0x0A010105);
                    MSS_MAC_set_TSU_unicast_addr(&g_emac1,
                            MSS_MAC_TSU_UNICAST_RX, 0x0A010105);
                    MSS_MAC_set_TSU_unicast_addr(&g_emac1,
                            MSS_MAC_TSU_UNICAST_TX, 0x0A010104);

                    g_mac_config.mac_addr[5] = 0x56;
                    MSS_MAC_update_hw_address(&g_mac0, &g_mac_config);
                    g_mac_config.mac_addr[5] = 0x57;
                    MSS_MAC_update_hw_address(&g_mac1, &g_mac_config);
                    g_mac_config.mac_addr[5] = 0x58;
                    MSS_MAC_update_hw_address(&g_emac0, &g_mac_config);
                    g_mac_config.mac_addr[5] = 0x59;
                    MSS_MAC_update_hw_address(&g_emac1, &g_mac_config);
                }
            }
            else if(rx_buff[0] == '=')
            {
                uint16_t filter;

                filter = MSS_MAC_get_sa_filter(g_test_mac, 2, 0);
                if(g_test_mac == &g_mac0)
                {
                    if(filter & MSS_MAC_SA_FILTER_SOURCE)
                    {
                        PRINT_STRING("Opposing GEM Source Address match disabled\n\r");
                        MSS_MAC_set_sa_filter(g_test_mac, 2,
                                MSS_MAC_SA_FILTER_DISABLE, g_mac1.mac_addr);
                    }
                    else
                    {
                        PRINT_STRING("Opposing GEM Source Address match enabled\n\r");
                        MSS_MAC_set_sa_filter(g_test_mac, 2,
                                MSS_MAC_SA_FILTER_SOURCE, g_mac1.mac_addr);
                    }
                }
                else if(g_test_mac == &g_emac0)
                {
                    if(filter & MSS_MAC_SA_FILTER_SOURCE)
                    {
                        PRINT_STRING("Opposing GEM Source Address match disabled\n\r");
                        MSS_MAC_set_sa_filter(g_test_mac, 2,
                                MSS_MAC_SA_FILTER_DISABLE, g_emac1.mac_addr);
                    }
                    else
                    {
                        PRINT_STRING("Opposing GEM Source Address match enabled\n\r");
                        MSS_MAC_set_sa_filter(g_test_mac, 2,
                                MSS_MAC_SA_FILTER_SOURCE, g_emac1.mac_addr);
                    }
                }
                else if(g_test_mac == &g_emac1)
                {
                    if(filter & MSS_MAC_SA_FILTER_SOURCE)
                    {
                        PRINT_STRING("Opposing GEM Source Address match disabled\n\r");
                        MSS_MAC_set_sa_filter(g_test_mac, 2,
                                MSS_MAC_SA_FILTER_DISABLE, g_emac0.mac_addr);
                    }
                    else
                    {
                        PRINT_STRING("Opposing GEM Source Address match enabled\n\r");
                        MSS_MAC_set_sa_filter(g_test_mac, 2,
                                MSS_MAC_SA_FILTER_SOURCE, g_emac0.mac_addr);
                    }
                }
                else
                {
                    if(filter & MSS_MAC_SA_FILTER_SOURCE)
                    {
                        PRINT_STRING("Opposing GEM Source Address match disabled\n\r");
                        MSS_MAC_set_sa_filter(g_test_mac, 2,
                                MSS_MAC_SA_FILTER_DISABLE, g_mac0.mac_addr);
                    }
                    else
                    {
                        PRINT_STRING("Opposing GEM Source Address match enabled\n\r");
                        MSS_MAC_set_sa_filter(g_test_mac, 2,
                                MSS_MAC_SA_FILTER_SOURCE, g_mac0.mac_addr);
                    }
                }
            }
            else if(rx_buff[0] == '#')
            {
                uint64_t temp_hash;
                uint64_t current_hash;
                int32_t index;
                int this_index;
                int this_direction;

                if(g_test_mac == &g_mac0)
                {
                    this_index = g_hash_index0;
                    this_direction = g_hash_direction0;
                }
                else if(g_test_mac == &g_emac0)
                {
                    this_index = g_hash_index0e;
                    this_direction = g_hash_direction0e;
                }
                else if(g_test_mac == &g_emac1)
                {
                    this_index = g_hash_index1e;
                    this_direction = g_hash_direction1e;
                }
                else
                {
                    this_index = g_hash_index1;
                    this_direction = g_hash_direction1;
                }

                index = calc_gem_hash_index(mac_address_list[this_index].octets);
                temp_hash = (uint64_t)(1ll << index);
                current_hash = MSS_MAC_get_hash(g_test_mac);
                if(0 != this_direction)
                {
                    sprintf(info_string,"Removing hash for %02x:%02x:%02x:%02x:%02x:%02x\n\r",
                            mac_address_list[this_index].octets[0],
                            mac_address_list[this_index].octets[1],
                            mac_address_list[this_index].octets[2],
                            mac_address_list[this_index].octets[3],
                            mac_address_list[this_index].octets[4],
                            mac_address_list[this_index].octets[5]
                            );
                    PRINT_STRING(info_string);
                    current_hash = current_hash & ~temp_hash;
                }
                else
                {
                    if(current_hash & temp_hash) /* Hash bit already set so collision... */
                    {
                        PRINT_STRING("Note. hash bit already set.\n\r");
                    }
                    sprintf(info_string,
                            "Inserting hash for %02x:%02x:%02x:%02x:%02x:%02x\n\r",
                            mac_address_list[this_index].octets[0],
                            mac_address_list[this_index].octets[1],
                            mac_address_list[this_index].octets[2],
                            mac_address_list[this_index].octets[3],
                            mac_address_list[this_index].octets[4],
                            mac_address_list[this_index].octets[5]
                            );
                    PRINT_STRING(info_string);
                    current_hash = current_hash | temp_hash;
                }

                MSS_MAC_set_hash_mode(g_test_mac, MSS_MAC_HASH_MULTICAST);
                MSS_MAC_set_hash(g_test_mac, current_hash);
                if(0 == this_direction)
                {
                    this_index++;
                    if((0 == mac_address_list[this_index].octets[0]) &&
                       (0 == mac_address_list[this_index].octets[1]) &&
                       (0 == mac_address_list[this_index].octets[2]) &&
                       (0 == mac_address_list[this_index].octets[3]) &&
                       (0 == mac_address_list[this_index].octets[4]) &&
                       (0 == mac_address_list[this_index].octets[5]))
                    {
                        this_index--;
                        this_direction = 1;
                        PRINT_STRING("All hashes inserted\n\r");
                    }
                }
                else
                {
                    if(0 == this_index)
                    {
                        this_direction = 0;
                        PRINT_STRING("All hashes removed\n\r");
                    }
                    else
                    {
                    this_index--;
                    }
                }

                if(g_test_mac == &g_mac0)
                {
                    g_hash_index0 = this_index;
                    g_hash_direction0 = this_direction;
                }
                else if(g_test_mac == &g_emac0)
                {
                    g_hash_index0e = this_index;
                    g_hash_direction0e = this_direction;
                }
                else if(g_test_mac == &g_emac1)
                {
                    g_hash_index1e = this_index;
                    g_hash_direction1e = this_direction;
                }
                else
                {
                    g_hash_index1 = this_index;
                    g_hash_direction1 = this_direction;
                }

            }
            else if(rx_buff[0] == '.')
            {
                MSS_MAC_tx_enable(g_test_mac); /* Just in case we have not already enabled it... */

                if(g_test_mac->is_emac)
                {
                    g_test_mac->emac_base->TX_PAUSE_QUANTUM  = 0x00020001ul;
                    g_test_mac->emac_base->TX_PAUSE_QUANTUM1 = 0x00040003ul;
                    g_test_mac->emac_base->TX_PAUSE_QUANTUM2 = 0x00060005ul;
                    g_test_mac->emac_base->TX_PAUSE_QUANTUM3 = 0x00080007ul;
                    g_test_mac->emac_base->TX_PFC_PAUSE      = 0x000000FFul;

                    g_test_mac->emac_base->NETWORK_CONFIG  |= GEM_PAUSE_ENABLE;
                    g_test_mac->emac_base->NETWORK_CONTROL |=
                        GEM_TRANSMIT_PFC_PRIORITY_BASED_PAUSE_FRAME | GEM_PFC_CTRL;
                }
                else
                {
                    g_test_mac->mac_base->TX_PAUSE_QUANTUM  = 0x00020001ul;
                    g_test_mac->mac_base->TX_PAUSE_QUANTUM1 = 0x00040003ul;
                    g_test_mac->mac_base->TX_PAUSE_QUANTUM2 = 0x00060005ul;
                    g_test_mac->mac_base->TX_PAUSE_QUANTUM3 = 0x00080007ul;
                    g_test_mac->mac_base->TX_PFC_PAUSE      = 0x000000FFul;

                    g_test_mac->mac_base->NETWORK_CONFIG  |= GEM_PAUSE_ENABLE;
                    g_test_mac->mac_base->NETWORK_CONTROL |=
                        GEM_TRANSMIT_PFC_PRIORITY_BASED_PAUSE_FRAME | GEM_PFC_CTRL;
                }
                PRINT_STRING("Transmit Priority Based Pause Frame\n\r");
            }
            else if(rx_buff[0] == ',')
            {
                /* Just in case we have not already enabled it... */
                MSS_MAC_tx_enable(g_test_mac);
                if(g_test_mac->is_emac)
                {
                    g_test_mac->emac_base->NETWORK_CONTROL &=
                            (uint32_t)(~GEM_PFC_CTRL);
                    g_test_mac->emac_base->NETWORK_CONFIG  &=
                            (uint32_t)(~GEM_PAUSE_ENABLE);
                    g_test_mac->emac_base->NETWORK_CONTROL |=
                            GEM_TX_PAUSE_FRAME_ZERO;
                }
                else
                {
                    g_test_mac->mac_base->NETWORK_CONTROL &=
                            (uint32_t)(~GEM_PFC_CTRL);
                    g_test_mac->mac_base->NETWORK_CONFIG  &=
                            (uint32_t)(~GEM_PAUSE_ENABLE);
                    g_test_mac->mac_base->NETWORK_CONTROL |=
                            GEM_TX_PAUSE_FRAME_ZERO;
                }
                PRINT_STRING("Transmit Zero Quantum Pause Frame\n\r");
            }
            else if(rx_buff[0] == '\'')
            {
                /* Just in case we have not already enabled it... */
                MSS_MAC_tx_enable(g_test_mac);
                if(g_test_mac->is_emac)
                {
                    g_test_mac->emac_base->NETWORK_CONTROL &=
                            (uint32_t)(~GEM_PFC_CTRL);
                    g_test_mac->emac_base->NETWORK_CONFIG  &=
                            (uint32_t)(~GEM_PAUSE_ENABLE);
                    g_test_mac->emac_base->NETWORK_CONTROL |=
                            GEM_TX_PAUSE_FRAME_REQ;
                }
                else
                {
                    g_test_mac->mac_base->NETWORK_CONTROL &=
                            (uint32_t)(~GEM_PFC_CTRL);
                    g_test_mac->mac_base->NETWORK_CONFIG  &=
                            (uint32_t)(~GEM_PAUSE_ENABLE);
                    g_test_mac->mac_base->NETWORK_CONTROL |=
                            GEM_TX_PAUSE_FRAME_REQ;
                }
                PRINT_STRING("Transmit Pause Frame\n\r");
            }
            else if(rx_buff[0] == '>')
            {
                send_ptp_stream(g_test_mac, ptp_packets, &pkt_index);
            }
            else if(rx_buff[0] == ']')
            {
                send_ptp_stream(g_test_mac, ptp_sync_packets, &sync_pkt_index);
            }
            else if(rx_buff[0] == '+')
            {
                g_tx_adjust++;
                   PRINT_STRING("TX loopback length adjust incremented\n\r");
            }
            else if(rx_buff[0] == '-')
            {
                if(1 != g_tx_adjust)
                {
                    g_tx_adjust--;
                    PRINT_STRING("TX loopback length adjust decremented\n\r");
                }
            }
            else if(rx_buff[0] == '!')
            {
                mss_mac_tsu_time_t tsu_val;

                MSS_MAC_read_TSU(g_test_mac, &tsu_val);

                sprintf(info_string,"TSU = %u, %u, %u\n\r", tsu_val.secs_msb,
                        tsu_val.secs_lsb, tsu_val.nanoseconds);
                PRINT_STRING(info_string);
            }
            else if(rx_buff[0] == '0')
            {
                if(g_test_mac == &g_mac0)
                {
                    PRINT_STRING("Selecting eGEM0\n\r");
                    g_test_mac = &g_emac0;
                    stats = stats0e;
                }
                else
                {
                    PRINT_STRING("Selecting GEM0\n\r");
                    g_test_mac = &g_mac0;
                    stats = stats0;
                }
            }
            else if(rx_buff[0] == '1')
            {
                if(g_test_mac == &g_mac1)
                {
                    PRINT_STRING("Selecting eGEM1\n\r");
                    g_test_mac = &g_emac1;
                    stats = stats1e;
                }
                else
                {
                    PRINT_STRING("Selecting GEM1\n\r");
                    g_test_mac = &g_mac1;
                    stats = stats1;
                }
            }
            else if(rx_buff[0] == '4')
            {
                if(0 == MSS_MAC_get_type_filter(&g_mac0, 3))
                {
                    PRINT_STRING("Enabling IPv4 packets on Specific Type 3 filter\n\r");
                    MSS_MAC_set_type_filter(&g_mac0, 3, 0x0800);
                }
                else
                {
                    PRINT_STRING("Disabling IPv4 packets on Specific Type 3 filter\n\r");
                    MSS_MAC_set_type_filter(&g_mac0, 3, 0x0000);
                }
            }
            else if(rx_buff[0] == '6')
            {
                if(0 == MSS_MAC_get_type_filter(&g_mac0, 4))
                {
                    PRINT_STRING("Enabling IPv6 packets on Specific Type 4 filter\n\r");
                    MSS_MAC_set_type_filter(&g_mac0, 4, 0x86DD);
                }
                else
                {
                    PRINT_STRING("Disabling IPv6 packets on Specific Type 4 filter\n\r");
                    MSS_MAC_set_type_filter(&g_mac0, 4, 0x0000);
                }
            }
            else
            {
                /* echo the rx char */
                MSS_UART_polled_tx(&g_mss_uart0_lo, rx_buff, (uint32_t)rx_size);
            }
        }
    }
}


#ifdef TEST_SW_INT
/*==============================================================================
 *
 */

void Software_h0_IRQHandler(void);
void Software_h0_IRQHandler(void)
{
    uint32_t hart_id = read_csr(mhartid);
    if(hart_id == 0)
    {
        count_sw_ints_h0++;
    }
}
#endif
