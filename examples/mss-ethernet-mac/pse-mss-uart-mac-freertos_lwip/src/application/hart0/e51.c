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

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"


#include "lwip/sockets.h"
#include "webserver/if_utils.h"

typedef socklen_t SOCKLEN_T;
#include "lwip/def.h"
#include "lwip/inet.h"
/* lwIP includes. */
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"

void http_server_netconn_thread(void *arg);
#if PSE
void dump_vsc8575_regs(mss_mac_instance_t * this_mac);
#endif

static uint8_t g_mac_tx_buffer[MSS_MAC_TX_RING_SIZE][MSS_MAC_MAX_TX_BUF_SIZE] __attribute__ ((aligned (4)));
static uint8_t g_mac_rx_buffer[MSS_MAC_RX_RING_SIZE][MSS_MAC_MAX_RX_BUF_SIZE] __attribute__ ((aligned (4)));


mss_mac_cfg_t g_mac_config;

/*==============================================================================
 * Network configuration globals.
 */

ntm_dhcp_mode_t g_network_addr_mode = NTM_DHCP_FIXED; /* Start off with fixed mode */

ip4_addr_t      g_ip_address; /* Configured fixed/default address */
ip4_addr_t      g_ip_mask;    /* Configured fixed/default mask */
ip4_addr_t      g_ip_gateway; /* Configured fixed/default gateway address */


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
    uint8_t packet[2000];
} ALIGNED_TX_BUF;

ALIGNED_TX_BUF tx_packet;
uint8_t tx_packet_data[60] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x08, 0x06, 0x00, 0x01,
                               0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0xEC, 0x08, 0x6B, 0xE2, 0xCA, 0x17, 0xC0, 0xA8, 0x80, 0x0F,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xA8, 0x80, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

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
       volatile static uint64_t counter = 0U;

       /* Added some code as debugger hangs if in loop doing nothing */
       counter = counter + 1;
    }

    return (0);
}


static volatile int tx_count = 0;
#if 0
/**=============================================================================
 *
 */
static void packet_tx_complete_handler(/* mss_mac_instance_t*/ void *this_mac, void * caller_info)
{
    (void)caller_info;
    // Unblock the task by releasing the semaphore.
    tx_count++;
}
#endif
static volatile int rx_count = 0;

/**=============================================================================
    Bottom-half of receive packet handler
*/
#if 0
void mac_rx_callback
(
    /* mss_mac_instance_t */ void *this_mac,
    uint8_t * p_rx_packet,
    uint32_t pckt_length,
    void * caller_info
)
{
    (void)caller_info;
    MSS_MAC_receive_pkt((mss_mac_instance_t *)this_mac, p_rx_packet, 0, 1);
    rx_count++;
}
#endif
#if 1
/*==============================================================================
 *
 */
/* lwIP MAC configuration. */
struct netif g_EMAC_if;

/*==============================================================================
 *
 */
uint32_t get_ipv4_address(int32_t select)
{
    uint32_t ret_val;

    switch(select)
    {
        case IP4_IP_MASK:
#if (LWIP_IPV4 && LWIP_IPV6)
            ret_val = ((uint32_t)(g_EMAC_if.netmask.u_addr.ip4.addr));
#else
            ret_val = (uint32_t)(g_EMAC_if.netmask.addr);
#endif
            break;

        case IP4_IP_GW:
#if (LWIP_IPV4 && LWIP_IPV6)
            ret_val = ((uint32_t)(g_EMAC_if.gw.u_addr.ip4.addr));
#else
            ret_val = (uint32_t)(g_EMAC_if.gw.addr);
#endif
            break;

        case IP4_IP_ADDR:
        default:
#if (LWIP_IPV4 && LWIP_IPV6)
            ret_val = ((uint32_t)(g_EMAC_if.ip_addr.u_addr.ip4.addr));
#else
            ret_val = (uint32_t)(g_EMAC_if.ip_addr.addr);
#endif
            break;
    }

    return(ret_val);
}


/*==============================================================================
 *
 */
void get_mac_address(uint8_t * mac_addr)
{
    uint32_t inc;

    for(inc = 0; inc < 6; ++inc)
    {
        mac_addr[inc] = g_EMAC_if.hwaddr[inc];
    }
}


/*==============================================================================
 *
 */
void str_to_ipv4_address(ip4_addr_t *address, char *string)
{
    int count;
    int index;
    uint8_t raw_addr[4];

    index = 0;
    for(count = 0; count != 4; count++)
    {
        raw_addr[count] = strtol(&string[index], 0, 10); /* Convert 1/4 of address to binary */

        while(isdigit(string[index])) /* Step over current number */
            index++;

        index++; /* Then step over '.' */
    }

    IP4_ADDR( address, raw_addr[0], raw_addr[1], raw_addr[2], raw_addr[3] );
}

/*==============================================================================
 *
 */

void set_mac_address(uint8_t * mac_addr)
{

    /* Update stored copies of mac address */
    memcpy(g_EMAC_if.hwaddr,      mac_addr, 6);
    memcpy(g_mac_config.mac_addr, mac_addr, 6);

    /* Reconfigure the actual hardware */
//    MSS_MAC_update_hw_address(&g_mac_config);
}


/*==============================================================================
 *
 */
void prvEthernetConfigureInterface(void * param)
{
    extern err_t ethernetif_init( struct netif *netif );


    /* Parameters are not used - suppress compiler error. */
    ( void ) param;

    /*
     * Set mac address
     */
    g_mac_config.mac_addr[0] = 0x00;
    g_mac_config.mac_addr[1] = 0xFC;
    g_mac_config.mac_addr[2] = 0x00;
    g_mac_config.mac_addr[3] = 0x12;
    g_mac_config.mac_addr[4] = 0x34;
    g_mac_config.mac_addr[5] = 0x56;

    memcpy(g_EMAC_if.hwaddr, g_mac_config.mac_addr, 6);


    /*
     * Start with a default fixed address so that we can bring the network
     * interface up quickly. Will most likely be the normal mode of operation
     * anyway...
     */
    g_network_addr_mode = NTM_DHCP_FIXED;

    str_to_ipv4_address(&g_ip_address, "10.2.2.20");
    str_to_ipv4_address(&g_ip_gateway, "10.2.2.100");
    str_to_ipv4_address(&g_ip_mask, "255.255.255.0");


    /* Start out with the default address */
    netif_add( &g_EMAC_if, &g_ip_address, &g_ip_mask, &g_ip_gateway, NULL, ethernetif_init, tcpip_input );

    /* bring it up */

    netif_set_up(&g_EMAC_if);

    if(NTM_DHCP_FIXED != g_network_addr_mode)
    {
        dhcp_start(&g_EMAC_if);
    }

    /* make it the default interface */
    netif_set_default(&g_EMAC_if);
}


/*==============================================================================
 *
 */

extern mss_mac_instance_t g_mac_instance;
void ethernetif_tick(void);

mss_mac_speed_t g_net_speed;


void prvLinkStatusTask(void * pvParameters)
{
    (void)pvParameters;
    for(;;)
    {
        volatile uint8_t linkup;
        uint8_t fullduplex;
        mss_mac_speed_t speed;
        uint32_t stats;

#if defined(G5_SOC_EMU_USE_GEM0)
        stats = MSS_MAC_read_stat(&g_mac0, 1);
        /* Run through loop every 500 milliseconds. */
        vTaskDelay(500/ portTICK_RATE_MS);
        linkup = MSS_MAC_get_link_status(&g_mac0, &speed,  &fullduplex);
#else
        stats = MSS_MAC_read_stat(&g_mac1, 1);
        /* Run through loop every 500 milliseconds. */
        vTaskDelay(500/ portTICK_RATE_MS);
        linkup = MSS_MAC_get_link_status(&g_mac1, &speed,  &fullduplex);
#endif
        g_net_speed = speed;
        ethernetif_tick();
    }
}

#endif
void e51_task( void *pvParameters );

volatile int second_task_count = 0;

void e51_second_task( void *pvParameters )
{
    (void)pvParameters;

    for(;;)
    {
        second_task_count++;
        vTaskDelay(100);
    }
}

TaskHandle_t thandle_uart;
TaskHandle_t thandle_link;
TaskHandle_t thandle_web;

void e51(void)
{
    BaseType_t rtos_result;

    write_csr(mscratch, 0);
    write_csr(mcause, 0);
    write_csr(mepc, 0);
    init_memory();

    PLIC_init();

    rtos_result = xTaskCreate( e51_task, "e51", 4000, NULL, uartPRIMARY_PRIORITY, NULL );
    if(1 != rtos_result)
    {
        int ix;
        for(;;)
            ix++;
    }

    rtos_result = xTaskCreate( e51_second_task, "e51-2nd", 4000, NULL, uartPRIMARY_PRIORITY + 2, NULL );
    if(1 != rtos_result)
    {
        int ix;
        for(;;)
            ix++;
    }
#if 1
    rtos_result = xTaskCreate(http_server_netconn_thread, (char *) "http_server", 4000, NULL, uartPRIMARY_PRIORITY + 3, &thandle_web );

    if(1 != rtos_result)
    {
        int ix;
        for(;;)
            ix++;
    }

    /* Create the task the Ethernet link status. */
    rtos_result = xTaskCreate(prvLinkStatusTask, (char *) "EthLinkStatus", 4000, NULL, uartPRIMARY_PRIORITY + 1, &thandle_link);
    if(1 != rtos_result)
    {
        int ix;
        for(;;)
            ix++;
    }

    vTaskSuspend(thandle_link);
    vTaskSuspend(thandle_web);
#endif

    /* Start the kernel.  From here on, only tasks and interrupts will run. */
      vTaskStartScheduler();

}

/**
 *
 */
void e51_task( void *pvParameters )
{
    int count;
//    init_memory();
#if defined(SIFIVE_HIFIVE_UNLEASHED)
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

    ( void ) pvParameters;

    PLIC_init();
    __disable_local_irq((int8_t)MMUART0_E51_INT);

    vPortSetupTimer();

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


    PLIC_SetPriority(ethernet_PLIC_53, 7);
    tcpip_init(prvEthernetConfigureInterface, NULL);

    /* hack - must use a semaphore or other synchronisation?
     * Hold off starting these tasks until network is active
     * */
    while(0 == g_mac0.g_mac_available)
    {
        vTaskDelay(1);
    }

    vTaskResume(thandle_web);
    vTaskResume(thandle_link);

#if 0
      MSS_MAC_cfg_struct_def_init(&g_mac_config);

    //g_mac_config.interface = GMII;
    g_mac_config.phy_addr = PHY_MDIO_ADDR;
    g_mac_config.mac_addr[0] = 0x00;
    g_mac_config.mac_addr[1] = 0xFC;
    g_mac_config.mac_addr[2] = 0x00;
    g_mac_config.mac_addr[3] = 0x12;
    g_mac_config.mac_addr[4] = 0x34;
    g_mac_config.mac_addr[5] = 0x56;

    MSS_MAC_init(&g_mac0, &g_mac_config);

    MSS_MAC_set_tx_callback(&g_mac0, packet_tx_complete_handler);
    MSS_MAC_set_rx_callback(&g_mac0, mac_rx_callback);

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
        if(count != (MSS_MAC_RX_RING_SIZE - 1))
        {
            MSS_MAC_receive_pkt(&g_mac0, g_mac_rx_buffer[count], 0, 0);
        }
        else
        {
            MSS_MAC_receive_pkt(&g_mac0, g_mac_rx_buffer[count], 0, -1);
        }
    }

#endif
//    while(1)
//        i++;

    i=0;
    while(1)
    {
        uint8_t mac_ret;
        mss_mac_speed_t speed;
        uint8_t         fullduplex;
        int old_rx_count;

//        mac_ret = MSS_MAC_get_link_status(&g_mac0, &speed, &fullduplex);
//        sprintf(info_string,"Speed %d, Duplex %d, RX Count %d, TX Count %d, task counter %d\n\r", (int)speed, (int)fullduplex, (int)rx_count, (int)tx_count, second_task_count);
//        MSS_FU540_UART_polled_tx(&g_mss_FU540_uart0, info_string,strlen(info_string));
//        dump_vsc_regs(&g_mac0);
        if(old_rx_count != rx_count)
        {
            old_rx_count = rx_count;
            if(0 == (rx_count % 4))
            {
                memcpy(tx_packet.packet, tx_packet_data, 60);
//                MSS_MAC_send_pkt(&g_mac0, tx_packet.packet, 60, 0);
            }
        }
        /* add your code here */
        i++;                /* added some code as debugger hangs if in loop doing nothing */
        if(i == (num_loops >> 0))
        {
            hartid             = read_csr(mhartid);
            mcycle_end         = readmcycle();
            delta_mcycle     = mcycle_end - mcycle_start;

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

        vTaskDelay(10);
    }

#else
    volatile int i;
    int8_t info_string[100];
    uint64_t mcycle_start = 0;
    uint64_t mcycle_end = 0;
    uint64_t delta_mcycle = 0;
    uint32_t num_loops = 1000000;
    uint32_t hartid = read_csr(mhartid);
    static volatile uint64_t dummy_h0 = 0;
    uint8_t rx_buff[1];
    uint32_t rx_idx  = 0;
    uint8_t rx_size = 0;
    volatile uint64_t delay_count;
    volatile uint32_t gpio_inputs;

    SYSREG->SOFT_RESET_CR &= ~( (1u << 0u) | (1u << 4u) | (1u << 5u) | (1u << 17u) | (1u << 19u) | (1u << 23u) | (1u << 24u) | (1u << 25u) | (1u << 26u) | (1u << 27u) | (1u << 28u) ) ;        // MMUART0
//    SYSREG->SOFT_RESET_CR &= ~( (1u << 0u) | (1u << 4u) | (1u << 5u) | (1u << 19u) | (1u << 23u) | (1u << 28u)) ;       // MMUART0

    SYSREG->IOMUX0_CR = 0xfffffe7f;  // connect MMUART0 to GPIO, QSPI to pads
    SYSREG->IOMUX1_CR = 0x05500000;  // pad5,6 = mux 5 (mmuart 0)

    // IOMUX configurations to allow QSPI pins to the pads
    SYSREG->IOMUX2_CR = 0;
    SYSREG->IOMUX3_CR = 0;
    SYSREG->IOMUX4_CR = 0;
    SYSREG->IOMUX5_CR = 0;


#if defined(TARGET_G5_SOC)
#if defined(MSS_MAC_USE_DDR) && (MSS_MAC_USE_DDR == MSS_MAC_MEM_CRYPTO)
    SYSREG->SOFT_RESET_CR &= ~( (1u << 16u) | (1u << 0u) | (1u << 4u) | (1u << 5u) | (1u << 17u) | (1u << 19u) | (1u << 23u) | (1u << 24u) | (1u << 25u) | (1u << 26u) | (1u << 27u) | (1u << 28u) ) ;        // MMUART0
#else
    SYSREG->SOFT_RESET_CR &= ~( (1u << 0u) | (1u << 4u) | (1u << 5u) | (1u << 17u) | (1u << 19u) | (1u << 23u) | (1u << 24u) | (1u << 25u) | (1u << 26u) | (1u << 27u) | (1u << 28u) ) ;        // MMUART0
#endif


#if MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_KEN_GMII
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


#if (MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_TBI) || (MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_TBI_GEM1) || (MSS_MAC_HW_PLATFORM  == MSS_MAC_DESIGN_EMUL_TBI_TI)
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

#if ((MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII) || (MSS_MAC_HW_PLATFORM == MSS_MAC_DESIGN_EMUL_GMII_GEM1))
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



    PLIC_init();
    __disable_local_irq((int8_t)MMUART0_E51_INT);

    vPortSetupTimer();

    MSS_UART_init( &g_mss_uart0_lo,
                MSS_UART_115200_BAUD,
                MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

//    PRINT_STRING("PolarFire MSS Ethernet Dual eMAC/pMAC Test program\n\r");

//    PRINT_STRING("Polling method for TXRX. Typed characters will be echoed.\n\r");
    __enable_irq();

#if defined(MSS_MAC_USE_DDR) && (MSS_MAC_USE_DDR == MSS_MAC_MEM_CRYPTO)
    *ATHENA_CR = SYSREG_ATHENACR_RESET | SYSREG_ATHENACR_RINGOSCON;
    *ATHENA_CR = SYSREG_ATHENACR_RINGOSCON;
    CALIni();
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, "CALIni() done..\n\r");
#endif

    PLIC_SetPriority(MAC0_INT_PLIC,  7);
    PLIC_SetPriority(MAC1_INT_PLIC,  7);
    PLIC_SetPriority(MAC0_EMAC_PLIC, 7);
    PLIC_SetPriority(MAC1_EMAC_PLIC, 7);
    PLIC_SetPriority(MAC0_MMSL_PLIC, 7);
    PLIC_SetPriority(MAC1_MMSL_PLIC, 7);

#if defined MSS_MAC_QUEUES
    PLIC_SetPriority(MAC0_QUEUE1_PLIC, 7);
    PLIC_SetPriority(MAC0_QUEUE2_PLIC, 7);
    PLIC_SetPriority(MAC0_QUEUE3_PLIC, 7);
    PLIC_SetPriority(MAC1_QUEUE1_PLIC, 7);
    PLIC_SetPriority(MAC1_QUEUE2_PLIC, 7);
    PLIC_SetPriority(MAC1_QUEUE3_PLIC, 7);
#endif

//    low_level_init();

    tcpip_init(prvEthernetConfigureInterface, NULL);

    /* hack - must use a semaphore or other synchronisation?
     * Hold off starting these tasks until network is active
     * */
    while(MSS_MAC_AVAILABLE != g_mac0.mac_available)
    {
        vTaskDelay(1);
    }

    vTaskResume(thandle_web);
    vTaskResume(thandle_link);

    /*
     * Startup the other harts
     */
#ifdef TEST_SW_INT
    raise_soft_interrupt((uint32_t)1); /* get hart1 out of wfi */
#endif

    while(1)
    {
#ifdef TEST_SW_INT
        raise_soft_interrupt((uint32_t)1);
#endif

#if 0
#if defined(TI_PHY)
        dump_ti_regs(&g_mac0);
#else
        dump_vsc8575_regs(&g_mac0);
#endif
#endif
        rx_size = MSS_UART_get_rx(&g_mss_uart0_lo, rx_buff, sizeof(rx_buff));
        if(rx_size > 0)
        {
            if(rx_buff[0] == '1')
            {
                hartid             = read_csr(mhartid);
                mcycle_end         = readmcycle();
                delta_mcycle     = mcycle_end - mcycle_start;

                sprintf(info_string,"Hart %d, %d loops required %ld cycles, sw ints h0 = %d, sw ints h1 = %d, mtime = %d mtimecmp = %d\n\r", hartid,
                        num_loops, delta_mcycle, count_sw_ints_h0, count_sw_ints_h1, CLINT->MTIME, CLINT->MTIMECMP[0]);
                MSS_UART_polled_tx_string (&g_mss_uart0_lo, info_string);

            }
            else if(rx_buff[0] == '2')
            {
                raise_soft_interrupt((uint32_t)0);
                MSS_UART_polled_tx_string (&g_mss_uart0_lo, "Raise sw int hart 0\n\r");

            }
            else if(rx_buff[0] == '3')
            {
                /*
                 * FIXME: This does not raise an interrupt
                 * Much the same code does on the unleashed-
                 */
                raise_soft_interrupt((uint32_t)1);
                MSS_UART_polled_tx_string (&g_mss_uart0_lo, "Raise sw int hart 1\n\r");

            }
            else
            {
                /* echo the rx char */
                MSS_UART_polled_tx(&g_mss_uart0_lo, rx_buff, rx_size);
            }
        }
        vTaskDelay(10);
    }
#endif
}


#ifdef TEST_SW_INT
/**
 *
 */
void Software_h0_IRQHandler(void)
{
    uint32_t hart_id = read_csr(mhartid);
    if(hart_id == 0)
    {
        count_sw_ints_h0++;
    }
}
#endif




/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c or heap_2.c are used, then the size of the
    heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
    FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
    to query the size of free heap space that remains (although it does not
    provide information on how the remaining heap might be fragmented). */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
    task.  It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()).  If the application makes use of the
    vTaskDelete() API function (as this demo application does) then it is also
    important that vApplicationIdleHook() is permitted to return to its calling
    function, because it is the responsibility of the idle task to clean up
    memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
