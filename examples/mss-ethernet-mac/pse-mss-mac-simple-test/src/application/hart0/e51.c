/*******************************************************************************
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
#include "mpfs_hal/mss_mpu.h"
#include "mpfs_hal/system_startup.h"

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
#include "drivers/mss_gpio/mss_gpio.h"

#if defined(MSS_MAC_USE_DDR) && (MSS_MAC_USE_DDR == MSS_MAC_MEM_CRYPTO)
/*
 * The crypto libraries have been removed from the example projects as they are
 * only required for internal testing. If you want to test this operation you
 * will need to add them back to the project for it to build correctly.
 */

#include "calini.h"
#endif

#if defined(TARGET_ALOE)
#define PRINT_STRING(x)MSS_FU540_UART_polled_tx(&g_mss_FU540_uart0, x, strlen(x));
#else
#define PRINT_STRING(x) MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t *)x);
#endif


/* Lots of references for debug data from MMAC/PHY/SGMII core etc */

#if defined(TARGET_G5_SOC)
extern uint16_t VSC8575_reg_0[32];
extern uint16_t VSC8575_reg_1[16];
extern uint16_t VSC8575_reg_3[20];
extern uint16_t VSC8575_MSS_SGMII_reg16[17];
/* Not currently used here but might be of interest... */
#if 0
extern uint16_t VSC8575_reg_2[16];
extern uint16_t VSC8575_reg_4[16];
extern uint16_t VSC8575_reg_16[32];
extern uint32_t VSC8575_MSS_MAC_reg[80];
extern uint32_t VSC8575_MSS_PLIC_REG[80];
#endif

extern uint16_t TI_reg_0[32];
extern uint16_t TI_reg_1[25];
extern uint16_t TI_MSS_SGMII_reg[17];

void dump_vsc8575_regs(mss_mac_instance_t * this_mac);
void dump_ti_regs(mss_mac_instance_t * this_mac);
#endif

#if defined(TARGET_ALOE)
extern uint16_t VSC8541_reg_0[32];
extern uint16_t VSC8541_reg_1[16];
extern uint16_t VSC8541_reg_2[16];
extern uint16_t VSC8541_reg_16[32];
void dump_vsc8541_regs(mss_mac_instance_t * this_mac);
#endif

/*
 * Align these on an 8 byte boundary as we might be using IEEE 1588 time
 * stamping and that uses b2 of the buffer pointer to indicate that a timestamp
 * is present in this descriptor.
 */
#if defined(MSS_MAC_USE_DDR)
static uint8_t *g_mac_rx_buffer = 0;
#else
static uint8_t g_mac_rx_buffer[MSS_MAC_RX_RING_SIZE][MSS_MAC_MAX_RX_BUF_SIZE]
                                                  __attribute__ ((aligned (16)));
#endif

mss_mac_cfg_t g_mac_config;

/*==============================================================================
 * Network configuration globals.
 */


#define TEST_SW_INT 1

#ifdef TEST_SW_INT
volatile uint32_t count_sw_ints_h0 = 0;
#endif
extern void init_memory( void);

typedef struct aligned_tx_buf
{
uint64_t aligner;
uint8_t packet[MSS_MAC_MAX_PACKET_SIZE];
} ALIGNED_TX_BUF;


ALIGNED_TX_BUF tx_packet;
uint8_t tx_packet_data[60] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                               0xFF, 0xFF, 0xFF, 0xFF, 0x08, 0x06, 0x00, 0x01,
                               0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0xEC, 0x08,
                               0x6B, 0xE2, 0xCA, 0x17, 0xC0, 0xA8, 0x80, 0x0F,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xA8,
                               0x80, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00 };

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

static volatile int tx_count = 0;

volatile uint32_t g_crc  = 0; /* CRC pass through control */
volatile int g_loopback  = 0; /* Software loopback control */
volatile int g_phy_dump  = 0; /* PHY Register dump control */
volatile int g_tx_add_1  = 0; /* Tx length adjustment control to make loopback
                                 packets more visible... */
volatile int g_tx_adjust = 1; /* Adjustment to make to the packet length when
                                 enabled by 'n' */

/* Receive packet capture variables */

#define PACKET_IDLE  0
#define PACKET_ARMED 1
#define PACKET_DONE  2

#define PACKET_MAX   16384
volatile int g_capture = PACKET_IDLE;
uint8_t      g_packet_data[PACKET_MAX];
volatile uint32_t g_packet_length = 0;
volatile int g_reload = PACKET_IDLE;

volatile int g_tx_retry = 0;
volatile int g_link_status = 0;

mss_mac_instance_t *g_test_mac = &g_mac0;

/* Define this if you are using the other harts... */
/* #define USE_OTHER_HARTS */

int main_first_hart(void)
{
    uint64_t hartid = read_csr(mhartid);
#if defined(USE_OTHER_HARTS)
    HLS_DATA* hls;
#endif
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
static void packet_tx_complete_handler(/* mss_mac_instance_t*/ void *this_mac,
              uint32_t queue_no, mss_mac_tx_desc_t *cdesc, void * caller_info)
{
    (void)caller_info;
    (void)cdesc;
    (void)this_mac;
    (void)queue_no;

    // Unblock the task by releasing the semaphore.
    tx_count++;
}


static volatile int rx_count = 0;

/**=============================================================================
    Bottom-half of receive packet handler
*/
static void mac_rx_callback
(
    /* mss_mac_instance_t */ void *this_mac,
    uint32_t queue_no,
    uint8_t * p_rx_packet,
    uint32_t pckt_length,
    mss_mac_rx_desc_t *cdesc,
    void * caller_info
)
{
    uint8_t tx_status;
    (void)caller_info;
    (void)cdesc;
    (void)queue_no;

    if(PACKET_ARMED == g_capture) /* Looking for packet so grab a copy */
    {
        if(pckt_length > PACKET_MAX)
        {
            pckt_length = PACKET_MAX;
        }

        memcpy(g_packet_data, p_rx_packet, pckt_length);

        g_packet_length = pckt_length;
        g_capture = PACKET_DONE; /* and say we go it */
    }

    if(g_loopback) /* Send what we receive if set to loopback */
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

            if(MSS_MAC_SUCCESS != tx_status)
            {
                g_tx_retry++;
                if(((mss_mac_instance_t *)this_mac)->mac_base->INT_STATUS &
                   GEM_TRANSMIT_COMPLETE)
                {
#if 0 /* This is doing it by the book... */
                    if(0 != ((mss_mac_instance_t *)this_mac)->tx_complete_handler)
                    {
                        ((mss_mac_instance_t *)this_mac)->tx_complete_handler((mss_mac_instance_t *)this_mac,
                                ((mss_mac_instance_t *)this_mac)->tx_caller_info[0]);
                    }
#else /* This is cutting corners because we know we can in this instance... */
                    tx_count++;
#endif

                    ((mss_mac_instance_t *)this_mac)->queue[0].nb_available_tx_desc = MSS_MAC_TX_RING_SIZE; /* Release transmit queue... */
                    ((mss_mac_instance_t *)this_mac)->mac_base->TRANSMIT_STATUS = GEM_STAT_TRANSMIT_COMPLETE;
                    ((mss_mac_instance_t *)this_mac)->mac_base->INT_STATUS = GEM_TRANSMIT_COMPLETE;
                }
            }
        } while(tx_status != MSS_MAC_SUCCESS);
    }

    MSS_MAC_receive_pkt((mss_mac_instance_t *)this_mac, 0, p_rx_packet, 0, 1);
    rx_count++;
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
    uint32_t count;

#if defined(MSS_MAC_USE_DDR)
    g_mac_rx_buffer = g_mss_mac_ddr_ptr;
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

    g_mac_config.speed_duplex_select =  MSS_MAC_ANEG_ALL_SPEEDS;
   /* g_mac_config.interface = GMII; */

    g_mac_config.mac_addr[0] = 0x00;
    g_mac_config.mac_addr[1] = 0xFC;
    g_mac_config.mac_addr[2] = 0x00;
    g_mac_config.mac_addr[3] = 0x12;
    g_mac_config.mac_addr[4] = 0x34;
    g_mac_config.mac_addr[5] = 0x56;

#if MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_DUAL_EX_VTS
    g_mac_config.phy_addr            = PHY_VSC8575_MDIO_ADDR;
    g_mac_config.phy_type            = MSS_MAC_DEV_PHY_VSC8575;
    g_mac_config.pcs_phy_addr        = SGMII_MDIO_ADDR;
    g_mac_config.interface_type      = GMII_SGMII;
    g_mac_config.phy_autonegotiate   = MSS_MAC_VSC8575_phy_autonegotiate;
    g_mac_config.phy_get_link_status = MSS_MAC_VSC8575_phy_get_link_status;
    g_mac_config.phy_init            = MSS_MAC_VSC8575_phy_init;
    g_mac_config.phy_set_link_speed  = MSS_MAC_VSC8575_phy_set_link_speed;

#if MSS_MAC_USE_PHY_DP83867
    g_mac_config.phy_extended_read   = NULL_ti_read_extended_regs;
    g_mac_config.phy_extended_write  = NULL_ti_write_extended_regs;
#endif

    g_test_mac = &g_mac0;
#endif

#if MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_DUAL_EX_TI
    g_mac_config.phy_addr            = PHY_DP83867_MDIO_ADDR;
    g_mac_config.phy_type            = MSS_MAC_DEV_PHY_DP83867;
    g_mac_config.pcs_phy_addr        = SGMII_MDIO_ADDR;
    g_mac_config.interface_type      = GMII_SGMII;
    g_mac_config.phy_autonegotiate   = MSS_MAC_DP83867_phy_autonegotiate;
    g_mac_config.phy_get_link_status = MSS_MAC_DP83867_phy_get_link_status;
    g_mac_config.phy_init            = MSS_MAC_DP83867_phy_init;
    g_mac_config.phy_set_link_speed  = MSS_MAC_DP83867_phy_set_link_speed;

#if MSS_MAC_USE_PHY_DP83867
    g_mac_config.phy_extended_read   = ti_read_extended_regs;
    g_mac_config.phy_extended_write  = ti_write_extended_regs;
#endif

    g_test_mac = &g_mac1;
#endif

#if (MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_TBI) || \
    (MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_TBI_GEM1)
    g_mac_config.phy_addr            = PHY_VSC8575_MDIO_ADDR;
    g_mac_config.phy_type            = MSS_MAC_DEV_PHY_VSC8575;
    g_mac_config.pcs_phy_addr        = SGMII_MDIO_ADDR;
    g_mac_config.interface_type      = TBI;
    g_mac_config.phy_autonegotiate   = MSS_MAC_VSC8575_phy_autonegotiate;
    g_mac_config.phy_get_link_status = MSS_MAC_VSC8575_phy_get_link_status;
    g_mac_config.phy_init            = MSS_MAC_VSC8575_phy_init;
    g_mac_config.phy_set_link_speed  = MSS_MAC_VSC8575_phy_set_link_speed;

#if MSS_MAC_USE_PHY_DP83867
    g_mac_config.phy_extended_read   = NULL_ti_read_extended_regs;
    g_mac_config.phy_extended_write  = NULL_ti_write_extended_regs;
#endif

#if MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_TBI
    g_test_mac = &g_mac0;
#else
    g_test_mac = &g_mac1;
#endif
#endif

#if (MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_TBI_TI)
    g_mac_config.phy_addr            = PHY_DP83867_MDIO_ADDR;
    g_mac_config.phy_type            = MSS_MAC_DEV_PHY_DP83867;
    g_mac_config.pcs_phy_addr        = SGMII_MDIO_ADDR;
    g_mac_config.interface_type      = TBI;
    g_mac_config.phy_autonegotiate   = MSS_MAC_DP83867_phy_autonegotiate;
    g_mac_config.phy_get_link_status = MSS_MAC_DP83867_phy_get_link_status;
    g_mac_config.phy_init            = MSS_MAC_DP83867_phy_init;
    g_mac_config.phy_set_link_speed  = MSS_MAC_DP83867_phy_set_link_speed;
#if MSS_MAC_USE_PHY_DP83867
    g_mac_config.phy_extended_read   = ti_read_extended_regs;
    g_mac_config.phy_extended_write  = ti_write_extended_regs;
#endif
    g_test_mac = &g_mac0;
#endif

#if ((MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII) || \
     (MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII_GEM1) || \
     (MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII_LOCAL))

    g_mac_config.phy_addr            = PHY_VSC8575_MDIO_ADDR;
    g_mac_config.phy_type            = MSS_MAC_DEV_PHY_VSC8575;
    g_mac_config.pcs_phy_addr        = SGMII_MDIO_ADDR;
    g_mac_config.interface_type      = GMII_SGMII;
    g_mac_config.phy_autonegotiate   = MSS_MAC_VSC8575_phy_autonegotiate;
    g_mac_config.phy_get_link_status = MSS_MAC_VSC8575_phy_get_link_status;
    g_mac_config.phy_init            = MSS_MAC_VSC8575_phy_init;
    g_mac_config.phy_set_link_speed  = MSS_MAC_VSC8575_phy_set_link_speed;
#if (MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII_LOCAL)
    g_mac_config.use_local_ints      = MSS_MAC_ENABLE;
#endif
#if MSS_MAC_USE_PHY_DP83867
    g_mac_config.phy_extended_read   = NULL_ti_read_extended_regs;
    g_mac_config.phy_extended_write  = NULL_ti_write_extended_regs;
#endif

#if ((MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII) || \
     (MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII_LOCAL))

    g_test_mac = &g_mac0;
#else
    g_test_mac = &g_mac1;
#endif
#endif

#if (MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_TI_GMII)
    g_mac_config.phy_addr            = PHY_DP83867_MDIO_ADDR;
    g_mac_config.phy_type            = MSS_MAC_DEV_PHY_DP83867;
    g_mac_config.pcs_phy_addr        = SGMII_MDIO_ADDR;
    g_mac_config.interface_type      = GMII_SGMII;
    g_mac_config.phy_autonegotiate   = MSS_MAC_DP83867_phy_autonegotiate;
    g_mac_config.phy_get_link_status = MSS_MAC_DP83867_phy_get_link_status;
    g_mac_config.phy_init            = MSS_MAC_DP83867_phy_init;
    g_mac_config.phy_set_link_speed  = MSS_MAC_DP83867_phy_set_link_speed;

#if MSS_MAC_USE_PHY_DP83867
    g_mac_config.phy_extended_read   = ti_read_extended_regs;
    g_mac_config.phy_extended_write  = ti_write_extended_regs;
#endif

    g_test_mac = &g_mac0;
#endif

#if MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_ALOE
g_mac_config.phy_addr            = PHY_VSC8541_MDIO_ADDR;
    g_mac_config.phy_type            = MSS_MAC_DEV_PHY_VSC8541;
    g_mac_config.pcs_phy_addr        = SGMII_MDIO_ADDR;
    g_mac_config.interface_type      = GMII;
    g_mac_config.phy_autonegotiate   = MSS_MAC_VSC8541_phy_autonegotiate;
    g_mac_config.phy_get_link_status = MSS_MAC_VSC8541_phy_get_link_status;
    g_mac_config.phy_init            = MSS_MAC_VSC8541_phy_init;
    g_mac_config.phy_set_link_speed  = MSS_MAC_VSC8541_phy_set_link_speed;

#if MSS_MAC_USE_PHY_DP83867
    g_mac_config.phy_extended_read   = NULL_ti_read_extended_regs;
    g_mac_config.phy_extended_write  = NULL_ti_write_extended_regs;
#endif

    g_test_mac = &g_mac0;
#endif

    /*
     * Initialize MAC with specified configuration. The Ethernet MAC is
     * functional after this function returns but still requires transmit and
     * receive buffers to be allocated for communications to take place.
     */
    MSS_MAC_init(g_test_mac, &g_mac_config);

    /* Allocate all 4 segments to queue 0 as this is our only one... */
    g_test_mac->mac_base->TX_Q_SEG_ALLOC_Q0TO3 = 2;

    /*
     * Register MAC interrupt handler listener functions. These functions will
     * be called  by the MAC driver when a packet has been sent or received.
     * These callback functions are intended to help managing transmit and
     * receive buffers by indicating when a transmit buffer can be released or
     * a receive buffer has been filled with an rx packet.
     */

    MSS_MAC_set_tx_callback(g_test_mac, 0, packet_tx_complete_handler);
    MSS_MAC_set_rx_callback(g_test_mac, 0, mac_rx_callback);

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
            MSS_MAC_receive_pkt(g_test_mac, 0,
                    g_mac_rx_buffer + count * MSS_MAC_MAX_RX_BUF_SIZE, 0, 0);
        }
        else
        {
            MSS_MAC_receive_pkt(g_test_mac, 0,
                    g_mac_rx_buffer + count * MSS_MAC_MAX_RX_BUF_SIZE, 0, -1);
        }
#else
        if(count != (MSS_MAC_RX_RING_SIZE - 1))
        {
            MSS_MAC_receive_pkt(g_test_mac, 0, g_mac_rx_buffer[count], 0, 0);
        }
        else
        {
            MSS_MAC_receive_pkt(g_test_mac, 0, g_mac_rx_buffer[count], 0, -1);
        }
#endif
    }
}


/*==============================================================================
 *
 */

volatile uint64_t g_tick_counter = 0;
uint64_t link_status_timer = 0;

volatile uint8_t g_test_linkup = 0;
uint8_t          g_test_fullduplex = 0;
mss_mac_speed_t  g_test_speed = MSS_MAC_1000MBPS;

void prvLinkStatusTask(void);
void prvLinkStatusTask(void)
{
    if(g_tick_counter >= link_status_timer)
    {
        /* Run through loop every 500 milliseconds. */
        g_test_linkup = MSS_MAC_get_link_status(g_test_mac, &g_test_speed,
                                                &g_test_fullduplex);
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
#if MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_GMII_LOCAL
    volatile int ix;
#endif
    write_csr(mscratch, 0);
    write_csr(mcause, 0);
    write_csr(mepc, 0);

    PLIC_init();

#if MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_GMII_LOCAL
    SYSREG->FAB_INTEN_U54_2 = 0x000001F8;
    SYSREG->FAB_INTEN_MISC  = 0x00000002;
    SysTick_Config();  /* Let hart 0 run the timer */

    CLINT->MSIP[2] = 1; /* Kick start hart 2 */
    __enable_irq();
    while(1)
    {
        ix++;
    }
#else
    e51_task(0);
#endif
}


#if (MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII_LOCAL)

void u54_2(void)
{
    volatile int ix;

    write_csr(mscratch, 0);
    write_csr(mcause, 0);
    write_csr(mepc, 0);

    PLIC_init();
    e51_task(0);

    while(1)
    {
        ix++;
    }
}

#endif


/*==============================================================================
 *
 */
static void print_help(void)
{
    char info_string[200];
    uint32_t temp_cutthru;

    sprintf(info_string,"The following single key commands are accepted:\n\r\n\r");
    PRINT_STRING(info_string);

    sprintf(info_string,"a - Initiate a PHY autonegotiation cycle\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"b - Toggle HW Loopback mode ---------------(%s)\n\r",
            0 != (g_test_mac->mac_base->NETWORK_CONTROL & GEM_LOOPBACK_LOCAL) ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    sprintf(info_string,"B - Toggle broadcast RX mode --------------(%s)\n\r",
            0 != (g_test_mac->mac_base->NETWORK_CONFIG & GEM_NO_BROADCAST) ? "disabled" : "enabled");
    PRINT_STRING(info_string);
    sprintf(info_string,"c - Capture and dump next packet\n\r");
    PRINT_STRING(info_string);
    temp_cutthru = MSS_MAC_get_tx_cutthru(g_test_mac);
    sprintf(info_string,"d - Toggle TX Cutthru ---------------------(%s)\n\r",
            temp_cutthru ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    temp_cutthru = MSS_MAC_get_rx_cutthru(g_test_mac);
    sprintf(info_string,"D - Toggle RX Cutthru ---------------------(%s)\n\r",
            temp_cutthru ? "enabled" : "disabled");
    PRINT_STRING(info_string);
#if defined(TARGET_G5_SOC)
    sprintf(info_string,"g - Display MSS GPIO 2 input values\n\r");
    PRINT_STRING(info_string);
#endif
    sprintf(info_string,"h - Display this help information\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"i - Increment all GEM stats counter registers\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"j - Toggle Jumbo Packet Mode---------------(%s)\n\r",
            0 != (g_test_mac->mac_base->NETWORK_CONFIG & GEM_JUMBO_FRAMES) ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    sprintf(info_string,"k - Toggle capture re-trigger mode --------(%s)\n\r",
            g_reload ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    sprintf(info_string,"l - Toggle SW Loopback mode ---------------(%s)\n\r",
            g_loopback ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    sprintf(info_string,"L - Toggle SGMII Link Status display mode--(%s)\n\r",
            g_link_status ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    sprintf(info_string,"n - Toggle TX response length adjust mode -(%s)\n\r",
            g_tx_add_1 ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    sprintf(info_string,"p - Toggle promiscuous receive mode -------(%s)\n\r",
            0 != (g_test_mac->mac_base->NETWORK_CONFIG & GEM_COPY_ALL_FRAMES) ? "enabled" : "disabled");
    PRINT_STRING(info_string);
    sprintf(info_string,"r - Reset statistics counts\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"s - Show statistics\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"t - Transmit sample ARP packet\n\r");
    PRINT_STRING(info_string);
    sprintf(info_string,"T - Transmit sample ARP packet - 60+ bytes 0x00 padded\n\r");
    PRINT_STRING(info_string);
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
    sprintf(info_string,"+/- Increment/Decrement length adjust -----(%d)\n\r",
            g_tx_adjust);
    PRINT_STRING(info_string);
}


/**
 *MSS_MAC_TX_OCTETS_LOW,                * 32-bit *
 *MSS_MAC_TX_OCTETS_HIGH,               * 16-bit *
 *MSS_MAC_TX_FRAMES_OK,                 * 32-bit *
 *MSS_MAC_TX_BCAST_FRAMES_OK,           * 32-bit *
 *MSS_MAC_TX_MCAST_FRAMES_OK,           * 32-bit *
 *MSS_MAC_TX_PAUSE_FRAMES_OK,           * 32-bit *
 *MSS_MAC_TX_64_BYTE_FRAMES_OK,         * 32-bit *
 *MSS_MAC_TX_65_BYTE_FRAMES_OK,         * 32-bit *
 *MSS_MAC_TX_128_BYTE_FRAMES_OK,        * 32-bit *
 *MSS_MAC_TX_256_BYTE_FRAMES_OK,        * 32-bit *
 *MSS_MAC_TX_512_BYTE_FRAMES_OK,        * 32-bit *
 *MSS_MAC_TX_1024_BYTE_FRAMES_OK,       * 32-bit *
 *MSS_MAC_TX_1519_BYTE_FRAMES_OK,       * 32-bit *
 *MSS_MAC_TX_UNDERRUNS,                 * 10-bit *
 *MSS_MAC_TX_SINGLE_COLLISIONS,         * 18-bit *
 *MSS_MAC_TX_MULTIPLE_COLLISIONS,       * 18-bit *
 *MSS_MAC_TX_EXCESSIVE_COLLISIONS,      * 10-bit *
 *MSS_MAC_TX_LATE_COLLISIONS,           * 10-bit *
 *MSS_MAC_TX_DEFERRED_FRAMES,           * 18-bit *
 *MSS_MAC_TX_CRS_ERRORS,                * 10-bit *
 *
 *MSS_MAC_RX_OCTETS_LOW,                * 32-bit *
 *MSS_MAC_RX_OCTETS_HIGH,               * 16-bit *
 *MSS_MAC_RX_FRAMES_OK,                 * 32-bit *
 *MSS_MAC_RX_BCAST_FRAMES_OK,           * 32-bit *
 *MSS_MAC_RX_MCAST_FRAMES_OK,           * 32-bit *
 *MSS_MAC_RX_PAUSE_FRAMES_OK,           * 32-bit *
 *MSS_MAC_RX_64_BYTE_FRAMES_OK,         * 32-bit *
 *MSS_MAC_RX_65_BYTE_FRAMES_OK,         * 32-bit *
 *MSS_MAC_RX_128_BYTE_FRAMES_OK,        * 32-bit *
 *MSS_MAC_RX_256_BYTE_FRAMES_OK,        * 32-bit *
 *MSS_MAC_RX_512_BYTE_FRAMES_OK,        * 32-bit *
 *MSS_MAC_RX_1024_BYTE_FRAMES_OK,       * 32-bit *
 *MSS_MAC_RX_1519_BYTE_FRAMES_OK,       * 32-bit *
 *MSS_MAC_RX_UNDERSIZE_FRAMES_OK,       * 10-bit *
 *MSS_MAC_RX_OVERSIZE_FRAMES_OK,        * 10-bit *
 *MSS_MAC_RX_JABBERS,                   * 10-bit *
 *MSS_MAC_RX_FCS_ERRORS,                * 10-bit *
 *MSS_MAC_RX_LENGTH_ERRORS,             * 10-bit *
 *MSS_MAC_RX_SYMBOL_ERRORS,             * 10-bit *
 *MSS_MAC_RX_ALIGNMENT_ERRORS,          * 10-bit *
 *MSS_MAC_RX_RESOURCE_ERRORS,           * 18-bit *
 *MSS_MAC_RX_OVERRUNS,                  * 10-bit *
 *MSS_MAC_RX_IP_CHECKSUM_ERRORS,        *  8-bit *
 *MSS_MAC_RX_TCP_CHECKSUM_ERRORS,       *  8-bit *
 *MSS_MAC_RX_UDP_CHECKSUM_ERRORS,       *  8-bit *
 *MSS_MAC_RX_AUTO_FLUSHED_PACKETS,      * 16-bit *
 *
 *  MSS_MAC_LAST_STAT
 *
 */

static uint32_t stats[MSS_MAC_LAST_STAT];
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
#if defined(TARGET_ALOE)
        dump_vsc8541_regs(g_test_mac);

        copper_rx_good      += VSC8541_reg_1[2] & 0x7FFF;
        phy_rx_err          += VSC8541_reg_0[19];
        phy_false_carrier   += VSC8541_reg_0[20];
        phy_link_disconnect += VSC8541_reg_0[21];
#endif

#if defined (TARGET_G5_SOC)
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
#endif
    }

#if defined(TARGET_ALOE)
    sprintf(info_string,"PHY Statistics\n\r");
    PRINT_STRING(info_string);

    sprintf(info_string,"PHY CU RX GOOD          % 10U  ", copper_rx_good);
    PRINT_STRING(info_string);

    sprintf(info_string,"PHY CU RX ERRORS        % 10U\n\r", phy_rx_err);
    PRINT_STRING(info_string);

    sprintf(info_string,"PHY FALSE CARRIER ERR   % 10U  ", phy_false_carrier);
    PRINT_STRING(info_string);

    sprintf(info_string,"PHY LINK DISCONNECTS    % 10U\n\r\n\r", phy_link_disconnect);
    PRINT_STRING(info_string);
#endif

#if defined(TARGET_G5_SOC)
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
#endif

    /* Grab the stats up front to minimise skew */
    for(count = MSS_MAC_TX_OCTETS_LOW; count != MSS_MAC_LAST_STAT; count++)
    {
        stats[count] += MSS_MAC_read_stat(g_test_mac, count);
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

void packet_dump(void);
void packet_dump(void)
{
    char info_string[200];
    int dump_address = 0;
    int count;
    char temp_string[10];

    g_capture = PACKET_IDLE;

    sprintf(info_string,"%d byte packet captured\n\r", g_packet_length);
    PRINT_STRING(info_string);

    while((g_packet_length - (uint32_t)dump_address) >= 16)
    {
        sprintf(info_string,"%04X ", dump_address);
        for(count = 0; count < 16; count++)
        {
            sprintf(temp_string,"%02X ",
                    (int)g_packet_data[dump_address + count] & 255);
            strcat(info_string, temp_string);
        }

        for(count = 0; count < 16; count++)
        {
            if((g_packet_data[dump_address + count] >= 32) &&
                    (g_packet_data[dump_address + count] < 127))
            {
                strncat(info_string,
                        (char *)&g_packet_data[dump_address + count], 1);
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

    if((g_packet_length - (uint32_t)dump_address) > 0) /* Finish off partial end line */
    {
        sprintf(info_string,"%04X ", dump_address);
        for(count = 0; count < ((int)g_packet_length - dump_address); count++)
        {
            sprintf(temp_string,"%02X ",
                    (int)g_packet_data[dump_address + count] & 255);
            strcat(info_string, temp_string);
        }

        strncat(info_string, "                                                ",
                (size_t)((16 - count) * 3)); /* Crude but effective space padding... */
        for(count = 0; count < ((int)g_packet_length - dump_address); count++)
        {
            if((g_packet_data[dump_address + count] >= 32) &&
                    (g_packet_data[dump_address + count] < 127))
            {
                strncat(info_string,
                        (char *)&g_packet_data[dump_address + count], 1);
            }
            else
            {
                strcat(info_string, ".");
            }
        }
        strcat(info_string, "\n\r");
        PRINT_STRING(info_string);
    }

    /* Finally see if we need to reload the capture mechanism... */
    if(g_reload)
    {
        g_capture = g_reload;
    }
}


/*==============================================================================
 *
 */

#define ATHENA_CR ((volatile uint32_t *) (0x20127000u))
#define ATHENA_CR_CSRMERRS ((volatile uint32_t *) (0x2200600C))

void e51_task( void *pvParameters )
{
    static uint32_t add_on = 0;
    //init_memory();
    char info_string[200];
    uint8_t rx_buff[1];
    size_t rx_size = 0;
    volatile uint64_t delay_count;
    volatile uint32_t gpio_inputs;

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

#if (MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_TI_GMII)
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

/******************************************************************************/
#if MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_DUAL_EX_TI
    MSS_GPIO_init(GPIO1_LO);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_0, MSS_GPIO_OUTPUT_MODE); /* DP83867 PHY reset - active low here as it is inverted in the bit-file */
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_1, MSS_GPIO_OUTPUT_MODE); /* SGMII core reset - active high */
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_2, MSS_GPIO_OUTPUT_MODE); /* SGMII core MDIO address b0-b4 */
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_3, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_4, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_5, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_6, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_7, MSS_GPIO_INOUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_8, MSS_GPIO_INOUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_9, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_10, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_11, MSS_GPIO_INPUT_MODE);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_12, MSS_GPIO_INPUT_MODE);
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

    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_0, 0); /* Force VSC8575 into reset */
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
 *   delay_count++;
 */
#endif


#if MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_DUAL_EX_VTS
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
#if 0
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_2, 1); /* Set address to 16 */
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_3, 1);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_4, 0);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_5, 0);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_6, 0);
#else
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_2, 0); /* Set address to 16 */
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_3, 0);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_4, 0);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_5, 0);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_6, 1);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_8, 1);
    MSS_GPIO_set_output(GPIO2_LO, MSS_GPIO_9, 1);
#endif
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


#if (MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_TBI) || \
    (MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_TBI_GEM1) || \
    (MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_TBI_TI)

    MSS_GPIO_init(GPIO2_LO);
    MSS_GPIO_config(GPIO2_LO, MSS_GPIO_0, MSS_GPIO_INOUT_MODE);  /* PHY Reset - active low / gtwiz_reset_rx_cdr_stable_out */
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

#if ((MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII) || \
     (MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII_GEM1) || \
     (MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII_LOCAL))

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
//    PLIC_init();
    __disable_local_irq((int8_t)MMUART0_E51_INT);

#if !(MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII_LOCAL)
    SysTick_Config();
#endif

#if defined(TARGET_ALOE)
    MSS_FU540_UART_init(&g_mss_FU540_uart0,
      0,
      0); /* note- with current boot-loader, serial port baud = 57400 */
    PLIC_SetPriority(USART0_PLIC_4, 1);
    /* Enable UART Interrupt on PLIC */
PLIC_EnableIRQ(USART0_PLIC_4);
#else
    MSS_UART_init( &g_mss_uart0_lo,
                    MSS_UART_115200_BAUD,
                    MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);
#endif

    PRINT_STRING("PolarFire MSS Ethernet MAC Test program\n\r");

    PRINT_STRING("Polling method for TXRX. Typed characters will be echoed.\n\r");
    __enable_irq();

#if defined(MSS_MAC_USE_DDR) && (MSS_MAC_USE_DDR == MSS_MAC_MEM_CRYPTO)
    *ATHENA_CR = SYSREG_ATHENACR_RESET | SYSREG_ATHENACR_RINGOSCON;
    *ATHENA_CR = SYSREG_ATHENACR_RINGOSCON;
    CALIni();
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, "CALIni() done..\n\r");
#endif

    low_level_init();

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

        if(PACKET_DONE == g_capture)
        {
            packet_dump();
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
                reg = MSS_MAC_read_phy_reg(g_test_mac,
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

#if defined(TARGET_ALOE)
        rx_size = MSS_FU540_UART_get_rx(&g_mss_FU540_uart0,
                                        rx_buff, sizeof(rx_buff));
#else
        rx_size = MSS_UART_get_rx(&g_mss_uart0_lo, rx_buff,
                                  (uint8_t)sizeof(rx_buff));
#endif
        if(rx_size > 0)
        {
            if(rx_buff[0] == 'a')
            {
                PRINT_STRING("Starting autonegotiation\n\r");
                g_test_mac->phy_autonegotiate(g_test_mac);
                PRINT_STRING("Finished autonegotiation\n\r");
            }
            else if(rx_buff[0] == 'b')
            {
                /* Note RX and TX must be disabled when changing loopback setting */
                if(0 == (g_test_mac->mac_base->NETWORK_CONTROL & GEM_LOOPBACK_LOCAL))
                {
                    PRINT_STRING("Hardware loopback enabled\n\r");
                    g_test_mac->mac_base->NETWORK_CONTROL &=
                        (uint32_t)(~(GEM_ENABLE_TRANSMIT | GEM_ENABLE_RECEIVE));

                    g_test_mac->mac_base->NETWORK_CONTROL |=
                            GEM_LOOPBACK_LOCAL | GEM_LOOPBACK;

                    g_test_mac->mac_base->NETWORK_CONTROL |=
                            GEM_ENABLE_TRANSMIT | GEM_ENABLE_RECEIVE;
                }
                else
                {
                    PRINT_STRING("Hardware loopback disabled\n\r");
                    g_test_mac->mac_base->NETWORK_CONTROL &=
                        (uint32_t)(~(GEM_ENABLE_TRANSMIT | GEM_ENABLE_RECEIVE));

                    g_test_mac->mac_base->NETWORK_CONTROL &=
                        (uint32_t)(~(GEM_LOOPBACK_LOCAL | GEM_LOOPBACK));

                    g_test_mac->mac_base->NETWORK_CONTROL |=
                        GEM_ENABLE_TRANSMIT | GEM_ENABLE_RECEIVE;
                }
            }
            else if(rx_buff[0] == 'B')
            {
                if(0 == (g_test_mac->mac_base->NETWORK_CONFIG & GEM_NO_BROADCAST))
                {
                    PRINT_STRING("Broadcast reception disabled\n\r");
                    g_test_mac->mac_base->NETWORK_CONFIG |= GEM_NO_BROADCAST;
                }
                else
                {
                    PRINT_STRING("Broadcast reception enabled\n\r");
                    g_test_mac->mac_base->NETWORK_CONFIG &=
                            (uint32_t)(~GEM_NO_BROADCAST);
                }
            }
            else if(rx_buff[0] == 'c')
            {
                PRINT_STRING("Packet capture armed\n\r");
                g_capture = PACKET_ARMED;
                if(g_reload)
                {
                    g_reload = PACKET_ARMED;
                }
            }
            else if(rx_buff[0] == 'd')
            {
                uint32_t temp_cutthru;

                temp_cutthru = MSS_MAC_get_tx_cutthru(g_test_mac);
                if(0 == temp_cutthru)
                {
                    PRINT_STRING("TX Cutthru enabled\n\r");
                    temp_cutthru = g_test_mac->is_emac ? 0x80000180ul : 0x80000180ul; /* May need to tweak... */
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
                    temp_cutthru = g_test_mac->is_emac ? 0x20ul : 0x20ul; /* May need to tweak... */
                    MSS_MAC_set_rx_cutthru(g_test_mac, temp_cutthru);
                }
                else
                {
                    PRINT_STRING("RX Cutthru disabled\n\r");
                    MSS_MAC_set_rx_cutthru(g_test_mac, 0);
                }
            }

#if defined(TARGET_G5_SOC)
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
#endif /* defined(TARGET_G5_SOC) */
            else if(rx_buff[0] == 'h')
            {
                print_help();
            }
            else if(rx_buff[0] == 'i')
            {
                PRINT_STRING("Incrementing stats counters\n\r");
                g_test_mac->mac_base->NETWORK_CONTROL |= 0x40;
            }
            else if(rx_buff[0] == 'j')
            {
                if(0 == (g_test_mac->mac_base->NETWORK_CONFIG & GEM_JUMBO_FRAMES))
                {
                    PRINT_STRING("Jumbo packets enabled\n\r");
                    g_test_mac->mac_base->NETWORK_CONFIG |= GEM_JUMBO_FRAMES;
                }
                else
                {
                    PRINT_STRING("Jumbo packets disabled\n\r");
                    g_test_mac->mac_base->NETWORK_CONFIG &=
                            (uint32_t)(~GEM_JUMBO_FRAMES);
                }
            }
            else if(rx_buff[0] == 'k')
            {
                if(g_reload != PACKET_IDLE)
                {
                    g_reload = PACKET_IDLE;
                }
                else
                {
                    g_reload = PACKET_ARMED;
                }

                sprintf(info_string, "Capture reload is %s\n\r",
                        g_reload ? "enabled" : "disabled");
                PRINT_STRING(info_string);
            }
            else if(rx_buff[0] == 'l')
            {
                g_loopback = !g_loopback;
                if(g_loopback)
                {
                    PRINT_STRING("SW Loopback on\n\r");
                }
                else
                {
                    PRINT_STRING("SW Loopback off\n\r");
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
            else if(rx_buff[0] == 'p')
            {
                if(0 == (g_test_mac->mac_base->NETWORK_CONFIG & GEM_COPY_ALL_FRAMES))
                {
                    g_test_mac->mac_base->NETWORK_CONFIG |= GEM_COPY_ALL_FRAMES;
                    PRINT_STRING("Promiscuous mode on\n\r");
                }
                else
                {
                    g_test_mac->mac_base->NETWORK_CONFIG &=
                            (uint32_t)(~GEM_COPY_ALL_FRAMES);
                    PRINT_STRING("Promiscuous mode off\n\r");
                }
            }
            else if(rx_buff[0] == 'r')
            {
                PRINT_STRING("Stats reset\n\r");
                memset(stats, 0, sizeof(stats));
                MSS_MAC_clear_statistics(g_test_mac);
                g_test_mac->queue[0].ingress     = 0;
                g_test_mac->queue[0].egress      = 0;
                g_test_mac->queue[0].hresp_error = 0;
                g_test_mac->queue[0].rx_restart  = 0;
                g_test_mac->queue[0].rx_overflow = 0;
                g_test_mac->tx_pause             = 0;
                g_test_mac->rx_pause             = 0;
                g_test_mac->pause_elapsed        = 0;

                g_tx_retry              = 0;
                tx_count                = 0;
                rx_count                = 0;
                copper_rx_good          = 0;
                phy_rx_err              = 0;
#if defined (TARGET_G5_SOC)
                mac_rx_good             = 0;
                mac_rx_err              = 0;
                mac_tx_good             = 0;
                mac_tx_err              = 0;
#endif
                phy_false_carrier       = 0;
                phy_link_disconnect     = 0;

#if defined(TARGET_G5_SOC)
                if(MSS_MAC_DEV_PHY_VSC8575 == g_test_mac->phy_type)
                {
                    /* This clears some stats in phy as they are reset on read... */
                    dump_vsc8575_regs(g_test_mac);
                }
#endif
#if defined(TARGET_ALOE)
                /* This clears some stats in phy as they are reset on read... */
                dump_vsc8541_regs(g_test_mac);
#endif
            }
            else if(rx_buff[0] == 's')
            {
                sprintf(info_string, "Link is currently %s, Duplex = %s, ",
                        g_test_linkup ? "Up" : "Down", g_test_fullduplex ? "Full" : "Half");
                PRINT_STRING(info_string);
                if(MSS_MAC_1000MBPS == g_test_speed)
                {
                    PRINT_STRING("Speed = 1Gb\n\r");
                }
                else if(MSS_MAC_100MBPS == g_test_speed)
                {
                    PRINT_STRING("Speed = 100Mb\n\r");
                }
                else if(MSS_MAC_10MBPS == g_test_speed)
                {
                    PRINT_STRING("Speed = 10Mb\n\r");
                }
                else
                {
                    PRINT_STRING("Speed = Unknown\n\r");
                }

                sprintf(info_string,"RX %U (%U pkts), TX %U (%U pkts)\n\rRX Over Flow %U, TX Retries %U\n\r",
                        g_test_mac->queue[0].ingress, rx_count,
                        g_test_mac->queue[0].egress, tx_count,
                        g_test_mac->queue[0].rx_overflow, g_tx_retry);
                PRINT_STRING(info_string);
                sprintf(info_string,"TX Pause %U, RX Pause %U, Pause Elapsed %U\n\r",
                        g_test_mac->tx_pause, g_test_mac->rx_pause,
                        g_test_mac->pause_elapsed);
                PRINT_STRING(info_string);

                sprintf(info_string,"HRESP not ok %U RX Restarts %U\n\r\n\r",
                        g_test_mac->queue[0].hresp_error,
                        g_test_mac->queue[0].rx_restart);
                PRINT_STRING(info_string);
                stats_dump();
            }
            else if(rx_buff[0] == 't')
            {
                uint8_t tx_status;

                add_on = 0; /* Reset the count for 'T' command */
                memcpy(&tx_pak_arp[6], g_test_mac->mac_addr, 6);
                tx_status = MSS_MAC_send_pkt(g_test_mac, 0, tx_pak_arp,
                        sizeof(tx_pak_arp) | g_crc, (void *)0);

                sprintf(info_string,"TX status %d\n\r", (int)tx_status);
                PRINT_STRING(info_string);
            }
            else if(rx_buff[0] == 'T')
            {
                uint8_t tx_status;
                volatile uint32_t *ctrl_reg;

                memcpy(&tx_pak_arp[6], g_test_mac->mac_addr, 6);
                tx_status = MSS_MAC_send_pkt(g_test_mac, 0,
                        tx_packet_data, ((60 + add_on) | g_crc), (void *)0);
                sprintf(info_string,"TX status %d, size %d\n\r",
                        (int)tx_status, (int)(add_on + 60));
                PRINT_STRING(info_string);
                ctrl_reg = &g_test_mac->mac_base->NETWORK_CONFIG;
                /* Coarse adjust for jumbo frame mode */
                if(0 != (*ctrl_reg & GEM_JUMBO_FRAMES))
                {
                    add_on += 100;
                    if(add_on > 10440) /* Allow a little extra for testing 10K upper limit */
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
                    g_test_mac->mac_base->NETWORK_CONFIG &=
                            (uint32_t)(~GEM_FCS_REMOVE);
                    g_test_mac->mac_base->NETWORK_CONFIG |= GEM_IGNORE_RX_FCS;
                }
                else
                {
                    PRINT_STRING("CRC passthrough disabled\n\r");
                    g_test_mac->mac_base->NETWORK_CONFIG |= GEM_FCS_REMOVE;
                    g_test_mac->mac_base->NETWORK_CONFIG &=
                            (uint32_t)(~GEM_IGNORE_RX_FCS);
                }
            }
            else if(rx_buff[0] == '.')
            {
                MSS_MAC_tx_enable(g_test_mac); /* Just in case we have not already enabled it... */

                g_test_mac->mac_base->TX_PAUSE_QUANTUM  = 0x00020001ul;
                g_test_mac->mac_base->TX_PAUSE_QUANTUM1 = 0x00040003ul;
                g_test_mac->mac_base->TX_PAUSE_QUANTUM2 = 0x00060005ul;
                g_test_mac->mac_base->TX_PAUSE_QUANTUM3 = 0x00080007ul;
                g_test_mac->mac_base->TX_PFC_PAUSE      = 0x000000FFul;

                g_test_mac->mac_base->NETWORK_CONFIG  |= GEM_PAUSE_ENABLE;
                g_test_mac->mac_base->NETWORK_CONTROL |=
                     GEM_TRANSMIT_PFC_PRIORITY_BASED_PAUSE_FRAME | GEM_PFC_CTRL;

                PRINT_STRING("Transmit Priority Based Pause Frame\n\r");
            }
            else if(rx_buff[0] == ',')
            {
                MSS_MAC_tx_enable(g_test_mac); /* Just in case we have not already enabled it... */
                g_test_mac->mac_base->NETWORK_CONTROL &=
                        (uint32_t)(~GEM_PFC_CTRL);
                g_test_mac->mac_base->NETWORK_CONFIG  &=
                        (uint32_t)(~GEM_PAUSE_ENABLE);
                g_test_mac->mac_base->NETWORK_CONTROL |=
                        GEM_TX_PAUSE_FRAME_ZERO;

                PRINT_STRING("Transmit Zero Quantum Pause Frame\n\r");
            }
            else if(rx_buff[0] == '\'')
            {
                MSS_MAC_tx_enable(g_test_mac); /* Just in case we have not already enabled it... */
                g_test_mac->mac_base->NETWORK_CONTROL &=
                        (uint32_t)(~GEM_PFC_CTRL);
                g_test_mac->mac_base->NETWORK_CONFIG  &=
                        (uint32_t)(~GEM_PAUSE_ENABLE);
                g_test_mac->mac_base->NETWORK_CONTROL |=
                        GEM_TX_PAUSE_FRAME_REQ;

                PRINT_STRING("Transmit Pause Frame\n\r");
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
            else
            {
                /* echo the rx char */
#if defined(TARGET_ALOE)
                MSS_FU540_UART_polled_tx(&g_mss_FU540_uart0, rx_buff, rx_size);
#else
                MSS_UART_polled_tx(&g_mss_uart0_lo, rx_buff, (uint32_t)rx_size);
#endif
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
