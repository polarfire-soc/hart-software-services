/*******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/*******************************************************************************
 * @file mss_ddr_debug.h
 * @author Microchip FPGA Embedded Systems Solutions
 * @brief DDR write and read test functions
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpfs_hal/mss_hal.h"
#include "mss_ddr_debug.h"

/*******************************************************************************
 * Local Defines
 */

/*******************************************************************************
 * External Defines
 */
#ifdef DEBUG_DDR_INIT
extern uint8_t sweep_results[MAX_NUMBER_DPC_V_GEN_SWEEPS]\
    [MAX_NUMBER__BCLK_SCLK_OFFSET_SWEEPS][MAX_NUMBER_ADDR_CMD_OFFSET_SWEEPS];
#endif

/*******************************************************************************
 * External function declarations
 */
extern void delay(uint32_t n);

/*******************************************************************************
 * Local function declarations
 */
static uint32_t ddr_write ( uint32_t *DDR_word_ptr, uint32_t no_of_access,\
        uint8_t data_ptrn );
static uint32_t ddr_read ( volatile uint32_t *DDR_word_ptr, uint32_t no_of_access,\
        uint8_t data_ptrn );


#ifdef DEBUG_DDR_INIT
/***************************************************************************//**
 * Setup serial port if DDR debug required during start-up
 * @param uart Ref to uart you want to use
 * @return
 */

__attribute__((weak))\
        uint32_t setup_ddr_debug_port(mss_uart_instance_t * uart)
{
    /* Turn on UART0 clock */
    SYSREG->SUBBLK_CLOCK_CR |= (SUBBLK_CLOCK_CR_MMUART0_MASK);
    /* Remove soft reset */
    SYSREG->SOFT_RESET_CR   &= ~(SUBBLK_CLOCK_CR_MMUART0_MASK);
    MSS_UART_init( uart,
        MSS_UART_115200_BAUD,
            MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);
    return(0U);
}


/***************************************************************************//**
 * Print in number hex format
 * @param uart
 * @param b
 */

static void dumpbyte(mss_uart_instance_t * uart, uint8_t b)
{
    const char hexchrs[] = { '0','1','2','3','4','5','6','7','8','9','A','B',\
            'C','D','E','F' };
    MSS_UART_polled_tx(uart, &hexchrs[b >> 4u] , 1);
    MSS_UART_polled_tx(uart, &hexchrs[b & 0x0fu] , 1);
}



/***************************************************************************//**
 *
 * @param uart
 * @param msg
 * @param d
 */
__attribute__((weak))\
        void uprint32(mss_uart_instance_t * uart, const char* msg, uint32_t d)
{
    MSS_UART_polled_tx_string(uart, msg);
    for (unsigned i=0; i < 4; i++)
    {
        dumpbyte(uart, (d >> (8*(3-i))) & 0xffu);
    }
}
#endif

/***************************************************************************//**
 * Write data to DDR
 * @param DDR_word_ptr
 * @param no_of_access
 * @param data_ptrn
 * @return
 */
static uint32_t ddr_write
(
    uint32_t *DDR_word_ptr,
    uint32_t no_of_access,
    uint8_t data_ptrn
)
{
    uint32_t i;
    uint32_t DATA, read_data;
    uint32_t error_count = 0U;

    switch (data_ptrn)
    {
        case PATTERN_INCREMENTAL : DATA = 0x00000000; break;
        case PATTERN_WALKING_ONE : DATA = 0x00000001; break;
        case PATTERN_WALKING_ZERO : DATA = 0x01;
                 DATA = ~ DATA; break;
        case PATTERN_RANDOM :
            DATA = rand ( );
            break;
        case PATTERN_0xCCCCCCCC :
            DATA = 0xCCCCCCCC;
            break;
        case PATTERN_0x55555555 :
            DATA = 0x55555555;
            break;
        case PATTERN_ZEROS :
            DATA = 0x00000000;
            break;
        default :  break;
    }

  for( i = 0; i< (no_of_access); i++)
  {
    *DDR_word_ptr = DATA;

    DDR_word_ptr = DDR_word_ptr + 1;
    switch (data_ptrn)
    {
        case PATTERN_INCREMENTAL : DATA = DATA + 0x00000001; break;
        case PATTERN_WALKING_ONE :
            if (DATA == 0x80000000)
                DATA = 0x00000001;
            else
                DATA = (DATA << 1);
            break;
        case PATTERN_WALKING_ZERO :
            DATA = ~DATA;
            if (DATA == 0x80000000)
                DATA = 0x00000001;
            else
            {
                DATA = (DATA << 1);
            }
            DATA = ~DATA;
            break;
        case PATTERN_RANDOM :
            DATA = rand ( );
            break;
        case PATTERN_0xCCCCCCCC :
            DATA = 0xCCCCCCCC;
            break;
        case PATTERN_0x55555555 :
            DATA = 0x55555555;
            break;
        case PATTERN_ZEROS :
            DATA = 0x00000000;
            break;
        default :
            break;
        }
    }
  return error_count;
}

/***************************************************************************//**
 * Reads and compares with what was wriiten
 * @param DDR_word_ptr
 * @param no_of_access
 * @param data_ptrn
 * @return 0 => read backs all expected value, otherwise error count
 */
uint32_t ddr_read
(
    volatile uint32_t *DDR_word_ptr,
    uint32_t no_of_access,
    uint8_t data_ptrn
)
{
    uint32_t i;
    uint32_t DATA, err_cnt, read_data, *GPIO_addr, *DDR_apb_addr;
    volatile uint32_t ddr_data;
    volatile uint32_t *DDR_word_pt_t, *first_DDR_word_pt_t, rand_addr_offset;
    //DDR_word_ptr = (uint32_t *) MSS_BASE_ADD_DRC_NC;
    err_cnt = 0U;
    first_DDR_word_pt_t = DDR_word_ptr;
    switch (data_ptrn)
    {
        case PATTERN_INCREMENTAL : DATA = 0x00000000; break;
        case PATTERN_WALKING_ONE : DATA = 0x00000001; break;
        case PATTERN_WALKING_ZERO : DATA = 0x01;
            DATA = ~ DATA; break;
        case PATTERN_RANDOM :
            DATA = rand ( );
            *DDR_word_ptr = DATA;
        break;
        case PATTERN_0xCCCCCCCC :
            DATA = 0xCCCCCCCC;
        break;
        case PATTERN_0x55555555 :
            DATA = 0x55555555;
        break;
        case PATTERN_ZEROS :
            DATA = 0x00000000;
        break;
        default :  break;
    }
    if (data_ptrn == '4')
    {
        delay(10);
    }
    for( i = 0; i< (no_of_access); i++)
    {
        DDR_word_pt_t = DDR_word_ptr;
        ddr_data = *DDR_word_pt_t;
        //  uprint32(g_mss_uart_debug_pt, "\t READ AT ADDR: 0x ", (uint64_t)(DDR_word_ptr));
        //    uprint32(g_mss_uart_debug_pt, "\n\r JV:: Read data at 0x ", DDR_word_ptr);
        //    uprint32(g_mss_uart_debug_pt, " is 0x ", *DDR_word_ptr);
        //    uprint32(g_mss_uart_debug_pt, " and Expected data is 0x ", DATA);
    if (ddr_data != DATA)
    {
#ifdef DEBUG_DDR_INIT
#ifdef DEBUG_DDR_RD_RW_FAIL
        if (err_cnt <=0xF)
        {

            uprint32(g_mss_uart_debug_pt, "\n\r READ/ WRITE ACCESS FAILED AT ADDR: 0x ", DDR_word_ptr);
            uprint32(g_mss_uart_debug_pt,"\t Expected Data 0x ", DATA);
            uprint32(g_mss_uart_debug_pt,"\t READ DATA: 0x ", ddr_data);
            uprint32(g_mss_uart_debug_pt,"\t READ DATA: 0x ", *DDR_word_ptr);
            uprint32(g_mss_uart_debug_pt,"\t READ DATA: 0x ", *DDR_word_ptr);
        }
#endif
#endif
        err_cnt++;
    }
    else
    {
        //printf("\n\r READ/ WRITE ACCESS passED AT ADDR: 0x%x expected data = 0x%x, Data read 0x%x",DDR_word_ptr, DATA, *DDR_word_ptr);
        //uprint32(g_mss_uart_debug_pt, "\n\r READ/ WRITE ACCESS PASSED AT ADDR: 0x", DDR_word_ptr);
        //uprint32(g_mss_uart_debug_pt,"\t READ DATA: 0x", *DDR_word_ptr);
    }
    DDR_word_ptr = DDR_word_ptr + 1U;
    switch (data_ptrn)
    {
        case PATTERN_INCREMENTAL : DATA = DATA + 0x01; break;
        case PATTERN_WALKING_ONE :
            if (DATA == 0x80000000)
                DATA = 0x00000001;
            else
                DATA = (DATA << 1);
            break;
        case PATTERN_WALKING_ZERO :
            DATA = ~DATA;
            if (DATA == 0x80000000)
            {
                DATA = 0x00000001;
            }
            else
            {
                DATA = (DATA << 1);
            }
            DATA = ~DATA;
            break;
        case PATTERN_RANDOM :
            DATA = rand ( );
            rand_addr_offset = (uint32_t)(rand() & 0xFFFFCUL);
            DDR_word_ptr = first_DDR_word_pt_t + rand_addr_offset;
            *DDR_word_ptr = DATA;
            break;
        case PATTERN_0xCCCCCCCC :
            DATA = 0xCCCCCCCC;
            break;
        case PATTERN_0x55555555 :
            DATA = 0x55555555;
            break;
        case PATTERN_ZEROS :
            DATA = 0x00000000;
            break;
        default :
            break;
      }
    }

  return (err_cnt);
}

/***************************************************************************//**
 *
 * @param DDR_word_ptr  Address
 * @param no_access  Number of addresses
 * @param pattern bit mask with patterns you want to test against
 * @return
 */
#ifdef DEBUG_DDR_INIT
uint32_t ddr_read_write_fn (uint64_t* DDR_word_ptr, uint32_t no_access, uint32_t pattern)
{
    uint32_t error_cnt = 0U;
    for (unsigned i=0; i < 1; i++)
    {
        for (unsigned pattern_mask=0U; pattern_mask < MAX_NO_PATTERNS;\
                                                                pattern_mask++)
        {
            if(pattern & (0x01U << pattern_mask))
            {
                /* write the pattern */
                error_cnt += ddr_write ((uint32_t *)DDR_word_ptr, no_access,\
                        pattern_mask);
                /* read back and verifies */
                error_cnt += ddr_read ((uint32_t *)DDR_word_ptr, no_access,\
                        pattern_mask);
            }
        }
        DDR_word_ptr++; /* increment the address */
    }
    return error_cnt;
}
#endif

/***************************************************************************//**
 *
 * @param error
 * @return
 */
#ifdef DEBUG_DDR_INIT
uint32_t error_status(mss_uart_instance_t *g_mss_uart_debug_pt, uint32_t error)
{
    uprint32(g_mss_uart_debug_pt,  "\n\r ERROR_RESULT: ", error);
    return (0U);
}
#endif

/***************************************************************************//**
 * Calibration status
 * @return
 */
#ifdef DEBUG_DDR_INIT
uint32_t wrcalib_status(mss_uart_instance_t *g_mss_uart_debug_pt)
{
    uprint32(g_mss_uart_debug_pt,  "\n\r WRCALIB_RESULT: ",\
            CFG_DDR_SGMII_PHY->expert_wrcalib.expert_wrcalib);
    return (0U);
}
#endif

#ifdef DEBUG_DDR_INIT
/***************************************************************************//**
 * Prints out DDR status
 * @return
 */
uint32_t tip_register_status (mss_uart_instance_t *g_mss_uart_debug_pt)
{

    uint32_t t_status;
    uint32_t MSS_DDR_APB_ADDR;
    uint32_t lane_sel;
    uint32_t ddr_lane_sel;
    uint32_t dq0_dly, dq1_dly, dq2_dly, dq3_dly, dq4_dly, dq5_dly, dq6_dly, dq7_dly;
    uint64_t uart_lock;

    /*  MSS_UART_polled_tx_string(g_mss_uart_debug_pt, "\n\n\r TIP register status \n");
    delay(1000);*/
    uprint32(g_mss_uart_debug_pt,  "\n\r\n\r training status = ", CFG_DDR_SGMII_PHY->training_status.training_status);
    uprint32(g_mss_uart_debug_pt,  "\n\r PCODE = ", (CFG_DDR_SGMII_PHY->IOC_REG2.IOC_REG2 & 0x7F));
    uprint32(g_mss_uart_debug_pt,  "\n\r NCODE = ", (((CFG_DDR_SGMII_PHY->IOC_REG2.IOC_REG2) >> 7) & 0x7F));

    MSS_UART_polled_tx_string(g_mss_uart_debug_pt, "\n\n\r lane_select \t gt_err_comb \t gt_txdly \t gt_steps_180 \t gt_state \t wl_delay_0 \t dqdqs_err_done \t dqdqs_state \t delta0 \t delta1");

    for (ddr_lane_sel=0U; ddr_lane_sel < LIBERO_SETTING_DATA_LANES_USED; ddr_lane_sel++)
    {
        CFG_DDR_SGMII_PHY->lane_select.lane_select = ddr_lane_sel; uprint32(g_mss_uart_debug_pt, "\n\r ", CFG_DDR_SGMII_PHY->lane_select.lane_select);
        delay(1000);
        MSS_DDR_APB_ADDR = CFG_DDR_SGMII_PHY->gt_err_comb.gt_err_comb;
        uprint32(g_mss_uart_debug_pt, "\t ", MSS_DDR_APB_ADDR);
        t_status = t_status | MSS_DDR_APB_ADDR;

        MSS_DDR_APB_ADDR = CFG_DDR_SGMII_PHY->gt_txdly.gt_txdly;
        uprint32(g_mss_uart_debug_pt, "\t ", MSS_DDR_APB_ADDR);

        if((MSS_DDR_APB_ADDR & 0xFF) == 0)    t_status = 1;
        if((MSS_DDR_APB_ADDR & 0xFF00) == 0)  t_status = 1;
        if((MSS_DDR_APB_ADDR & 0xFF0000) == 0) t_status = 1;
        if((MSS_DDR_APB_ADDR & 0xFF000000) == 0) t_status = 1;

        uprint32(g_mss_uart_debug_pt, "\t ", CFG_DDR_SGMII_PHY->gt_steps_180.gt_steps_180);
        uprint32(g_mss_uart_debug_pt, "\t ", CFG_DDR_SGMII_PHY->gt_state.gt_state);
        uprint32(g_mss_uart_debug_pt, "\t ", CFG_DDR_SGMII_PHY->wl_delay_0.wl_delay_0);
        uprint32(g_mss_uart_debug_pt, "\t ", CFG_DDR_SGMII_PHY->dq_dqs_err_done.dq_dqs_err_done);
        t_status = t_status | (MSS_DDR_APB_ADDR  != 8);

        uprint32(g_mss_uart_debug_pt, "\t\t ", CFG_DDR_SGMII_PHY->dqdqs_state.dqdqs_state);
        uprint32(g_mss_uart_debug_pt, "\t ", CFG_DDR_SGMII_PHY->delta0.delta0);
        dq0_dly = (MSS_DDR_APB_ADDR & 0xFF);
        dq1_dly = (MSS_DDR_APB_ADDR & 0xFF00) >> 8;
        dq2_dly = (MSS_DDR_APB_ADDR & 0xFF0000) >> 16;
        dq3_dly = (MSS_DDR_APB_ADDR & 0xFF000000) >> 24;
        uprint32(g_mss_uart_debug_pt, "\t ", CFG_DDR_SGMII_PHY->delta1.delta1);
        dq4_dly = (MSS_DDR_APB_ADDR & 0xFF);
        dq5_dly = (MSS_DDR_APB_ADDR & 0xFF00) >> 8;
        dq2_dly = (MSS_DDR_APB_ADDR & 0xFF0000) >> 16;
        dq3_dly = (MSS_DDR_APB_ADDR & 0xFF000000) >> 24;
    }

    MSS_UART_polled_tx_string(g_mss_uart_debug_pt, "\n\r\n\r lane_select\t rdqdqs_status2\t addcmd_status0\t addcmd_status1\t addcmd_answer1\t dqdqs_status1\n\r");
    for (ddr_lane_sel=0U; ddr_lane_sel < LIBERO_SETTING_DATA_LANES_USED; ddr_lane_sel++)
    {
        CFG_DDR_SGMII_PHY->lane_select.lane_select = ddr_lane_sel; uprint32(g_mss_uart_debug_pt, "\n\r ", CFG_DDR_SGMII_PHY->lane_select.lane_select);
        delay(1000);

        if(dq0_dly > 20) t_status = 1;
        if(dq1_dly > 20) t_status = 1;
        if(dq2_dly > 20) t_status = 1;
        if(dq3_dly > 20) t_status = 1;
        if(dq4_dly > 20) t_status = 1;
        if(dq5_dly > 20) t_status = 1;
        if(dq6_dly > 20) t_status = 1;
        if(dq7_dly > 20) t_status = 1;

        //    MSS_DDR_APB_ADDR = CFG_DDR_SGMII_PHY->dqdqs_status0.dqdqs_status0;
        //    uprint32(g_mss_uart_debug_pt, "\t ", MSS_DDR_APB_ADDR);
        //    MSS_DDR_APB_ADDR = CFG_DDR_SGMII_PHY->dqdqs_status1.dqdqs_status1;
        //    uprint32(g_mss_uart_debug_pt, "\t ", MSS_DDR_APB_ADDR);
        uprint32(g_mss_uart_debug_pt, "\t ", CFG_DDR_SGMII_PHY->dqdqs_status2.dqdqs_status2);
        //    MSS_DDR_APB_ADDR = CFG_DDR_SGMII_PHY->dqdqs_status3.dqdqs_status3;
        //    uprint32(g_mss_uart_debug_pt, "\t ", MSS_DDR_APB_ADDR);
        //    MSS_DDR_APB_ADDR = CFG_DDR_SGMII_PHY->dqdqs_status4.dqdqs_status4;
        //    uprint32(g_mss_uart_debug_pt, "\t ", MSS_DDR_APB_ADDR);
        //    MSS_DDR_APB_ADDR = CFG_DDR_SGMII_PHY->dqdqs_status5.dqdqs_status5;
        //    uprint32(g_mss_uart_debug_pt, "\t ", MSS_DDR_APB_ADDR);
        //    MSS_DDR_APB_ADDR = CFG_DDR_SGMII_PHY->dqdqs_status6.dqdqs_status6;
        //    uprint32(g_mss_uart_debug_pt, "\t ", MSS_DDR_APB_ADDR);
        uprint32(g_mss_uart_debug_pt, "\t ", CFG_DDR_SGMII_PHY->addcmd_status0.addcmd_status0);
        uprint32(g_mss_uart_debug_pt, "\t ", CFG_DDR_SGMII_PHY->addcmd_status1.addcmd_status1);
        uprint32(g_mss_uart_debug_pt, "\t ", CFG_DDR_SGMII_PHY->addcmd_answer.addcmd_answer);
        uprint32(g_mss_uart_debug_pt, "\t ", CFG_DDR_SGMII_PHY->dqdqs_status1.dqdqs_status1);

    }
    return(t_status);
}
#endif

/***************************************************************************//**
 * display sweep results
 *
 * @param g_mss_uart_debug_pt
 */
#ifdef DEBUG_DDR_INIT
void sweep_status (mss_uart_instance_t *g_mss_uart_debug_pt)
{

    uint32_t t_status;
    uint8_t cmd_index;
    uint8_t bclk_sclk_index;
    uint8_t dpc_vgen_index;

    MSS_UART_polled_tx_string(g_mss_uart_debug_pt, "\n\n\r dpc_vgen \t bclk_sclk");
    for (cmd_index=0U; cmd_index < MAX_NUMBER_ADDR_CMD_OFFSET_SWEEPS; cmd_index++)
    {
        uprint32(g_mss_uart_debug_pt, "\t ", cmd_index + LIBERO_SETTING_MIN_ADDRESS_CMD_OFFSET);
    }
    MSS_UART_polled_tx_string(g_mss_uart_debug_pt,\
            "\n\r------------------------------------------------------------------------");

    for (dpc_vgen_index=0U; dpc_vgen_index < MAX_NUMBER_DPC_V_GEN_SWEEPS; dpc_vgen_index++)
    {
        for (bclk_sclk_index=0U; bclk_sclk_index < MAX_NUMBER__BCLK_SCLK_OFFSET_SWEEPS; bclk_sclk_index++)
        {
            uprint32(g_mss_uart_debug_pt, "\n\r ", dpc_vgen_index + LIBERO_SETTING_MIN_DPC_V_GEN);
            uprint32(g_mss_uart_debug_pt, "\t ", bclk_sclk_index + LIBERO_SETTING_MIN_ADDRESS_BCLK_SCLK_OFFSET);
            for (cmd_index=0U; cmd_index < MAX_NUMBER_ADDR_CMD_OFFSET_SWEEPS; cmd_index++)
            {
                if(sweep_results[dpc_vgen_index][bclk_sclk_index][cmd_index] ==\
                        CALIBRATION_PASSED)
                {
                    MSS_UART_polled_tx_string(g_mss_uart_debug_pt, "\t\t pass");
                }
                else
                {
                    /*
                     * easier to see if not printed
                     * todo: may add detail to fail
                     * */
                    MSS_UART_polled_tx_string(g_mss_uart_debug_pt, "\t\t F");
                }

            }
        }
    }
}
#endif

