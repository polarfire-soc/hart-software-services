/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Support routines for the VTS API for the Microsemi VSC8575 PHY interface
 * to support the peripheral daughter board for the G5 SoC Emulation Platform.
 *
 * SVN $Revision$
 * SVN $Date$
 *
 */
#include <stdio.h>
#include <stdarg.h> /* For va_list */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "drivers/mss_mac/mss_ethernet_mac_user_config.h"

#if MSS_MAC_USE_PHY_VSC8575
#include "vtss_api.h"   /* For BOOL and friends */
#include "vtss_phy_api.h"   /* For PHY API Pre and Post Resets */
#endif

#if MSS_MAC_USE_PHY_VSC8575_LITE
#include "vtss_phy_common.h"
#include "vtss_viper_phy_prototypes.h"
#endif

#include "mss_plic.h"

#include "drivers/mss_mac/mss_ethernet_registers.h"
#include "drivers/mss_mac/mss_ethernet_mac_regs.h"
#include "drivers/mss_mac/mss_ethernet_mac.h"
#include "drivers/mss_mac/phy.h"

#include "drivers/mss_mac/mss_ethernet_mac_types.h"

#ifdef _ZL303XX_FMC_BOARD
/* Only needed if SPI interfaces required */
#include "mss_spi.h"
#include "core_spi.h"
#endif
/* Kernel includes. */
#if defined(USING_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#else
extern volatile uint64_t g_tick_counter;
#endif

/* Uncomment the following to enable logging of MDIO writes to memory */
/* VSC8575_DEBUG_MDIO */

#ifdef __DEBUG_SOCKET__

#ifndef T_E
#define T_E(...) \
    printf("Error: %s, %s, %d, \n", __FILE__, __FUNCTION__, __LINE__); \
    printf(__VA_ARGS__);
#endif

#ifndef T_W
#define T_W(...) \
    printf("Warning: %s, %s, %d, \n", __FILE__, __FUNCTION__, __LINE__); \
    printf(__VA_ARGS__);
#endif

#ifndef T_N
#define T_N(...) \
    printf("Notify: %s, %s, %d, \n", __FILE__, __FUNCTION__, __LINE__); \
    printf(__VA_ARGS__);
#endif

#else

#define T_E(...)
#define T_W(...)
#define T_N(...)
#endif

extern mss_mac_instance_t *g_my_mac;

#if MSS_MAC_USE_PHY_VSC8575
/* ================================================================= *
 *  Trace
 * ================================================================= */

vtss_trace_conf_t vtss_appl_trace_conf = {
    .level = {VTSS_TRACE_LEVEL_ERROR, VTSS_TRACE_LEVEL_ERROR}
};

static void printf_trace_head(const vtss_trace_layer_t layer,
                              const vtss_trace_group_t group,
                              const vtss_trace_level_t level,
                              const char *file,
                              const int line,
                              const char *function,
                              const char *lcont)
{
    time_t  t;
    int     h, m, s;

    (void)group;
    (void)file;
    (void)line;
    t = 0; /* time(NULL); */
    h = (int)(t / 3600 % 24);
    m = (int)(t / 60 % 60);
    s = (int)(t % 60);
    printf("%u:%02u:%02u %s/%s %s%s",
           h, m, s,
           layer == VTSS_TRACE_LAYER_COUNT ? "APPL": layer == VTSS_TRACE_LAYER_AIL ? "AIL" : "CIL",
           level == VTSS_TRACE_LEVEL_ERROR ? "Error" :
           level == VTSS_TRACE_LEVEL_INFO ? "Info " :
           level == VTSS_TRACE_LEVEL_DEBUG ? "Debug" :
           level == VTSS_TRACE_LEVEL_NOISE ? "Noise" : "?????",
           function, lcont);
}


/* Trace callout function */
void vtss_callout_trace_printf(const vtss_trace_layer_t layer,
                               const vtss_trace_group_t group,
                               const vtss_trace_level_t level,
                               const char *file,
                               const int line,
                               const char *function,
                               const char *format,
                               ...)
{
    va_list va;
    printf_trace_head(layer, group, level, file, line, function, ": ");

    va_start(va, format);
    vprintf(format, va);
    va_end(va);
    printf("\n");
}


/* Trace hex-dump callout function */
void vtss_callout_trace_hex_dump(const vtss_trace_layer_t layer,
                                 const vtss_trace_group_t group,
                                 const vtss_trace_level_t level,
                                 const char               *file,
                                 const int                line,
                                 const char               *function,
                                 const unsigned char      *byte_p,
                                 const int                byte_cnt)
{
    int i;

    printf_trace_head(layer, group, level, file, line, function, "\n");

    for (i= 0; i < byte_cnt; i += 16) {
        int j = 0;
        printf("%04x:", i);
        while (j+i < byte_cnt && j < 16) {
            printf(" %02x", byte_p[i+j]);
            j++;
        }
        putchar('\n');
    }
}

vtss_rc miim_read(const vtss_inst_t    inst,
                  const vtss_port_no_t phy_port,
                  const u8             phy_reg,
                  u16                  *const value);

vtss_rc miim_write(const vtss_inst_t    inst,
                   const vtss_port_no_t phy_port,
                   const u8             phy_reg,
                   const u16            value);

int32_t viper_fmc_board_init(vtss_init_conf_t *target);



/* Function for initializing the hardware board. */
int32_t viper_fmc_board_init(vtss_init_conf_t *target)
{
#ifdef _ZL303XX_FMC_BOARD
    /* 0x30200000U is the base address of the CoreSPI in the fabric */
    SPI_init(&g_vsc_spi, 0x30200000U , 32); /* Now is probably a good time to take care of this... */
    /*
     * Note: for the FMC board, the APB clock frequency is 83MHz and the maximum
     * allowed clock frequency for the 1588 SPI interface is 25MHz. We can only choose
     * even clock divisors and a divisor of 4 used in the design gives us a 20.75MHz SPI clock.
     */
    SPI_configure_master_mode(&g_vsc_spi); /* Motorola Mode 0, 8 bits selected in design */
    SPI_set_slave_select(&g_vsc_spi, SPI_SLAVE_0);

    target->spi_read_write = spi_read_write; /* Set pointer to SPI interface r/w function for this board */
#endif

    target->miim_read      = miim_read;      /* Set pointer to the MIIM read function for this board. */
    target->miim_write     = miim_write;     /* Set pointer to the MIIM write function for this board. */
    return 0;
}


void vtss_callout_lock(const vtss_api_lock_t *const lock)
{
    (void)lock;
}


void vtss_callout_unlock(const vtss_api_lock_t *const lock)
{
    (void)lock;
}


/**
 * brief SPI read/write function
 *
 * param inst [IN] Vitesse API instance.
 * param port_no [IN] Port number.
 * param bitsize [IN] Size (in bytes) of bitstream following this parameter.
 * param data [IN|OUT] Pointer to the data to be written to SPI Slave, if doing write operation.
 *                      Pointer to the data read from SPI Slave, if doing read operation.
 *
 * return Return code.
 **/
/* G5 SOC Emulation platform has no SPI interface at the moment... */
#ifdef _ZL303XX_FMC_BOARD

vtss_rc spi_read_write(const vtss_inst_t inst,
                       const vtss_port_no_t port_no,
                       const u8 bitsize,
                       u8 *const bitstream);
vtss_rc spi_read_write(const vtss_inst_t inst,
                       const vtss_port_no_t port_no,
                       const u8 bitsize,
                       u8 *const bitstream)
{
    (void)inst;
    (void)port_no;
    /*
     * The VTSS API transfers 32 bit values using this function.
     *
     * We use the bitsize parameter to determine read vs write as
     * it will be 7 for writes and 10 for reads.
     *
     * The first 3 bytes are the R/W status and the register address
     * information.
     *
     * When writing, the next 4 values are the frame to write,
     * When reading, the next 3 values are padding and the last 4
     * bytes are the read data.
     */
    if(7 == bitsize) /* Write operation */
    {
        SPI_transfer_block_vsc(&g_vsc_spi, bitstream, bitsize, 0, 0);
    }
    else
    {
        SPI_transfer_block_vsc(&g_vsc_spi, bitstream, 6, &bitstream[6], 4);
    }
    return VTSS_RC_OK;
}
#endif /* _ZL303XX_FMC_BOARD */



/* ================================================================= *
 *  Misc. functions
 * ================================================================= */
#if 0 /* PMCS: Remove for now */
/* Function defining the port interface. */
static vtss_port_interface_t port_interface(vtss_port_no_t port_no)
{
    return VTSS_PORT_INTERFACE_SGMII;
}

/* Function defining the port interface. */
static void viper_phy_pre_reset(void)
{
    vtss_rc   rc;
    rc = vtss_phy_pre_reset (NULL, 0);
    return;
}

/* Function defining the port interface. */
static vtss_rc viper_phy_post_reset(void)
{
    return (vtss_phy_post_reset (NULL, 0));
}
#endif


/*
 * Each board can have it own way of communicating with the chip. The miim read and write function are called by the API
 * when the API needs to do register access.
 *
 * Miim read access specific for this board.
 * In : port_no - The port to access.
 *      addr    - The address to access
 *
 * In/Out:  value   - Pointer to the value to be returned
 */


vtss_rc miim_read(const vtss_inst_t    inst,
                  const vtss_port_no_t phy_port,
                  const u8             phy_reg,
                  u16                  *const value)
{
#ifdef __DEBUG_SOCKET__
    const uint16_t   port_no = (uint16_t) phy_port;
    uint8_t   addr = (uint8_t)phy_reg & 0xff;
#endif
    (void)inst;
    if((void *)0 != g_my_mac)
    {
        *value = MSS_MAC_read_phy_reg(g_my_mac, (uint8_t)(phy_port + g_my_mac->phy_addr), (uint8_t)phy_reg); /* TBD: PMCS Warning only works for single MAC/VSC8575 combination */
    }

    T_N("miim read port_no = %d, addr = %d, value = 0x%X", port_no, addr, *value);

    return VTSS_RC_OK;
}


#if defined(VSC8575_DEBUG_MDIO)
/* Store all in 32 bit values so mem dump can view them neatly on 16 byte boundaries... */
typedef struct mii_debug_data
{
#if defined(USING_FREERTOS)
    TickType_t time;
#else
    uint64_t     time;
#endif
    /* vtss_port_no_t */ uint32_t page;
    /* u8             */ uint32_t reg;
    /*u16             */ uint32_t data;
} mii_debug_data_t;

mii_debug_data_t mii_data[1000];
uint32_t mii_data_index = 0;
uint32_t mii_page;
#endif
/*
 * Miim write access specific for this board.
 * In : port_no - The port to access.
 *      addr    - The address to access
 *      value   - The value to written
 */
vtss_rc miim_write(const vtss_inst_t    inst,
                   const vtss_port_no_t phy_port,
                   const u8             phy_reg,
                   const u16            value)
{
#ifdef __DEBUG_SOCKET__
    const uint16_t   port_no = (uint16_t) phy_port;
    uint8_t   addr = phy_reg & 0xff;
#endif

    (void)inst;
#if defined(VSC8575_DEBUG_MDIO)
    if(0x1f == phy_reg)
    {
        mii_page = value;
    }
    else
    {
#if defined(USING_FREERTOS)
        mii_data[mii_data_index].time = xTaskGetTickCount();
#else
        mii_data[mii_data_index].time = g_tick_counter;
#endif
        mii_data[mii_data_index].page = mii_page;
        mii_data[mii_data_index].reg  = phy_reg;
        mii_data[mii_data_index].data = value;
        mii_data_index++;
        if(1000 == mii_data_index)
        {
            mii_data_index = 0;
        }
    }
#endif
    T_N("miim_writes port_no = %d, addr = %d, value = 0x%X", port_no, addr ,value);
    if((void *)0 != g_my_mac)
    {
        MSS_MAC_write_phy_reg(g_my_mac, (uint8_t)(phy_port + g_my_mac->phy_addr), (uint8_t)phy_reg, value); /* TBD: PMCS Warning only works for single MAC/VSC8575 combination */
    }

    return VTSS_RC_OK;
}
#endif /* MSS_MAC_USE_PHY_VSC8575 */
#if MSS_MAC_USE_PHY_VSC8575_LITE
int32_t miim_read(const uint32_t phy_port, const uint16_t phy_reg, uint16_t *const value);
int32_t miim_write(const uint32_t phy_port, const uint16_t phy_reg, const uint16_t value);
int32_t usleep(uint32_t usecs);
int32_t viper_fmc_board_init(struct phy_control_t   *cntrl);


/*==============================================================================
 * emulate the Unix usleep function using the taskdelay functionality of
 * FreeRTOS. It is not very close as our example system currently uses a 1mS
 * tick but all the instances of usleep() being called in the VTSS API Lite are
 * for 1000uS so it should do...
 */
int32_t usleep(uint32_t usecs)
{
#if defined(USING_FREERTOS)
    uint32_t ustick = portTICK_PERIOD_MS * 1000U; /* calculate microseconds per tick */

    /* convert uS to ticks, rounding up to the nearest tick */
    usecs = (usecs + (ustick - 1)) / ustick;

    vTaskDelay(usecs);
#else
    /* Assumes 1mS tick... */
    volatile uint64_t timeout = g_tick_counter + (((uint64_t)usecs + 999ULL) / 1000ULL);
    volatile uint64_t index = 0U;

    while(g_tick_counter <= timeout)
    {
        index++; /* Stop debugger from locking up... */
    }
#endif

    return(0);
}


/*
 * Each board can have it own way of communicating with the chip. The miim read and write function are called by the API
 * when the API needs to do register access.
 *
 * Miim read access specific for this board.
 * In : port_no - The port to access.
 *      addr    - The address to access
 *
 * In/Out:  value   - Pointer to the value to be returned
 */
int32_t miim_read( const uint32_t          phy_port,
               const uint16_t          phy_reg,
               uint16_t         *const value)
{
#ifdef __DEBUG_SOCKET__
    const uint16_t   port_no = (uint16_t) phy_port;
    uint8_t   addr = (uint8_t)phy_reg & 0xff;
#endif

    if((void *)0 != g_my_mac)
    {
        *value = MSS_MAC_read_phy_reg(g_my_mac, (uint8_t)(phy_port + g_my_mac->phy_addr), (uint8_t)phy_reg); /* TBD: PMCS Warning only works for single MAC/VSC8575 combination */
    }

    T_N("miim read port_no = %d, addr = %d, value = 0x%X", port_no, addr, *value);

    return 0;
}


/*
 * Miim write access specific for this board.
 * In : port_no - The port to access.
 *      addr    - The address to access
 *      value   - The value to written
 *
 * Store all in 32 bit values so mem dump can view them neatly on 16 byte boundaries...
 */
#if defined(VSC8575_DEBUG_MDIO)
typedef struct mii_debug_data
{
    TickType_t     time;
    /* vtss_port_no_t */ uint32_t page;
    /* u8             */ uint32_t reg;
    /*u16             */ uint32_t data;
} mii_debug_data_t;

mii_debug_data_t mii_data[1000];
uint32_t mii_data_index = 0;
uint32_t mii_page = 0;
#endif
int32_t miim_write( const uint32_t      phy_port,
                const uint16_t          phy_reg,
                const uint16_t          value)
{
#ifdef __DEBUG_SOCKET__
    const uint16_t   port_no = (uint16_t) phy_port;
    uint8_t   addr = phy_reg & 0xff;
#endif

#if defined(VSC8575_DEBUG_MDIO)
    if(0 == phy_port)
    {
        if(0x1f == phy_reg)
        {
            mii_page = value;
        }
        else
        {
            mii_data[mii_data_index].time = xTaskGetTickCount();
            mii_data[mii_data_index].page = mii_page;
            mii_data[mii_data_index].reg  = phy_reg;
            mii_data[mii_data_index].data = value;
            mii_data_index++;
            if(1000 == mii_data_index)
            {
                mii_data_index = 0;
            }
        }
    }
#endif

    T_N("miim_writes port_no = %d, addr = %d, value = 0x%X", port_no, addr ,value);
    if((void *)0 != g_my_mac)
    {
        MSS_MAC_write_phy_reg(g_my_mac, (uint8_t)(phy_port + g_my_mac->phy_addr), (uint8_t)phy_reg, value); /* TBD: PMCS Warning only works for single MAC/VSC8575 combination */
    }

    return 0;
}


/* Function for initializing the hardware board. */
int32_t viper_fmc_board_init(struct phy_control_t   *cntrl)
{
    cntrl->phy_reg_read =  miim_read;  /* Set pointer to the MIIM read function for this board. */
    cntrl->phy_reg_write = miim_write; /* Set pointer to the MIIM write function for this board. */

    return 0;
}
#endif /* MSS_MAC_USE_PHY_VSC8575_LITE */
