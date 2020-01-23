/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC MSS eMMC SD bare metal driver implementation.
 *
 * SVN $Revision: 12579 $
 * SVN $Date: 2019-12-04 16:41:30 +0530 (Wed, 04 Dec 2019) $
 */
#include "mss_mmc.h"
#include "mss_mmc_if.h"
#include "mss_mmc_regs.h"
#include "mss_mmc_types.h"
#include "hal/hal_assert.h"
#include "mpfs_hal/mss_hal.h"
#include "mpfs_hal/mss_sysreg.h"
#include "mpfs_hal/mss_plic.h"

#ifdef MSS_MMC_INTERNAL_APIS
#include "mss_mmc_internal_api.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************************//**
 *  Macros
 */
#define WORD_SIZE                       4u
#define BYTE_MASK                       0xFFu
#define BLK_SIZE                        512u
#define BLOCK_COUNT_ENABLE_SHIFT        16u
#define BUFF_EMPTY                      0u
#define RCA_VALUE                       0x0001u
#define STUFF_BITS                      0x0u
#define BUSY_BIT_MASK                   0x80000000u
#define SECT_SIZE_CSD_MASK              0x03C000u
#define SECT_SIZE_CSD_SHIFT             14u
#define SECTOR_ACCESS_MODE_MASK         0x60000000u
#define SECTOR_SHIFT                    29u
#define RESET_ARG                       0x00000000u
#define MMC_DW_CSD                      0x03B70000u
#define MMC_LEGACY_MODE                 0x03B90000u
#define MMC_HS_MODE                     0x03B90100u
#define MMC_HS200_MODE                  0x03B90200u
#define MMC_HS400_MODE                  0x03B90300u
#define MMC_HS_MODE_DEFAULT             0x03B90000u
#define MMC_HPI_ENABLE                  0x03A10100u

#define MMC_CQ_ENABLE                   0x030F0100u
#define CQ_IDLE_TIME                    0x1000u

#define MMC_DEVICE_LOW_POWER            0x80u
#define MMC_DEVICE_LOW_VOLT_SET         0x40000080u
#define MMC_DEVICE_3_3V_VOLT_SET        0x40300000u

#define MMC_CLEAR                       0x0u
#define MMC_SET                         0x1u
#define MMC_STATUS_CLEAR                0xFFFFFFFFu
#define MMC_64BIT_UPPER_ADDR_SHIFT      32u
#define MMC_SRS03_COMMAND_SHIFT         24u
#define DELAY_COUNT                     0xFFFFu
#define DATA_TIMEOUT_VALUE              500000u
#define HRS0_SOFTWARE_RESET             0x00000001u
#define MMC_SOFTWARE_RESET_SHIFT        0x3u
#define DEBOUNCING_TIME                 0x300000u
#define MMC_RESET_DATA_CMD_LINE         0x06000000u
/* SDMA boundary is 512k */
#define SIZE_32MB                       0x02000000u
#define SIZE_64KB                       0x00010000u
#define SIZE_1GB                        0x40000000u

#define EXT_CSD_SECTOR_COUNT_OFFSET     212u
#define EXT_CSD_CARD_TYPE_OFFSET        196u
#define EXT_CSD_REVISION_OFFSET         192u
#define EXT_CSD_HS_TIMING_OFFSET        185u
#define EXT_CSD_ES_SUPPORT_OFFSET       184u
#define EXT_CSD_CQ_SUPPORT_OFFSET       308u
/* CMD Queuing Depth */
#define EXT_CSD_CQ_DEPTH_OFFSET         307u
#define EXT_CSD_CQ_MODE_EN_OFFSET       15u

#define READ_SEND_SCR                   0xFFFFFFFEu
#define SCR_REG_DATA_SIZE               8u

#define IF_COND_27V_33V                 (1u << 8u)

#define RCA_SHIFT_BIT                   16u
#define SIZE_4KB                        4096u
#define COMMANDS_TIMEOUT                3000u
/* HS200/400 1.8v support */
#define DEVICE_SUPPORT_HS400            0x40u
#define DEVICE_SUPPORT_HS200            0x10u

#define DEVICE_SUPPORT_DDR              0x04u
#define DEVICE_SUPPORT_SDR              0x03u
#define DEVICE_SUPPORT_SDR_50MHZ        0x02u
#define DEVICE_SUPPORT_SDR_25MHZ        0x01u
#define DEVICE_SUPPORT_LEGACY           0x00u

#define MMC_ES_DUAL_DATA_WIDTH_8BIT     0x86u
#define MMC_DUAL_DATA_WIDTH_8BIT        0x6u
#define MMC_DUAL_DATA_WIDTH_4BIT        0x5u

#define MAX_CURRENT_MA                  150u
#define SHIFT_2BIT                      0x2u
#define SHIFT_4BIT                      0x4u
#define SHIFT_8BIT                      0x8u
#define SHIFT_9BIT                      9u
#define SHIFT_16BIT                     16u
#define SHIFT_24BIT                     24u
#define SHIFT_30BIT                     30u
#define SHIFT_31BIT                     31u
#define SHIFT_32BIT                     32u

#define MASK_32BIT                      0xFFFFFFFFu
#define MASK_24BIT                      0xFFFFFFu
#define MASK_16BIT                      0xFFFFu
#define MASK_8BIT                       0xFFu
#define MASK_4BIT                       0xFu

#define BYTES_128                       128u
#define BYTES_64                        64u
#define BYTES_40                        40u
#define BYTES_7                         7u
#define BYTES_6                         6u
#define BYTES_5                         5u
#define BYTES_4                         4u
#define BYTES_3                         3u
#define BYTES_2                         2u
#define BYTES_1                         1u

#define BYTE_3_MASK                     0xFF0000u
#define CQ_TDLBA_ALIGN_MASK            ((1u << 10u) - 1u)

#define CARD_INT_STATUS_MASK            0xFEu
#define SDCARD_BUSWIDTH_MASK            0xFCu

#define DEVICE_HPI_SUPPORT              0x01u
#define HPI_WITH_CMD12_SUPPORT          0x02u
/* Number outside of normal range- used to tell read
 block routine to read extended CSD register */
#define READ_SEND_EXT_CSD               0xFFFFFFFFu

/* Number of temporary sub-buffers, used by to split data
 bigger than 64KB for smaller parts. Split is made by ADMA module. */
#define SDIO_CFG_SDIO_BUFFERS_COUNT     0x4000u

#define NULL_POINTER                            ((void *)0u)
/*******************************************************************************
 *  Global variable file scope
 */
/* Relative card address (eMMC/SD) */
static uint32_t sdcard_RCA;
/* ADMA descriptor table */
static uint32_t adma_descriptor_table[SIZE_64KB];
static uint8_t *g_desc_addr = NULL_POINTER;
static uint8_t g_mmc_init_complete;
static uint8_t g_mmc_cq_init_complete;
static uint8_t g_mmc_is_multi_blk = MMC_CLEAR;
static uint8_t g_sdio_fun_num;
static uint8_t g_device_hpi_suport;

#ifdef MSS_MMC_INTERNAL_APIS
static uint8_t g_device_hpi_set = MMC_CLEAR;
static uint32_t g_trans_remain_size = MMC_CLEAR;
static uint8_t * g_trans_src_addr = MMC_CLEAR;
static uint32_t g_trans_dest_addr = MMC_CLEAR;
#endif /* MSS_MMC_INTERNAL_APIS */

static uint8_t g_cq_task_id = MMC_CLEAR;
/******************************************************************************/
struct mmc_trans
{
    volatile mss_mmc_status_t state;
};
static struct mmc_trans g_mmc_trs_status;
/******************************************************************************/
struct phydelayaddresses
{
    uint8_t address;
    MSS_MMC_phydelay phydelaytype;
};
/*******************************************************************************
 * Private functions
 */
static mss_mmc_status_t mmccard_oper_config(const mss_mmc_cfg_t * cfg);
static mss_mmc_status_t sdcard_oper_config(const mss_mmc_cfg_t * cfg);
static mss_mmc_status_t sdio_oper_config(const mss_mmc_cfg_t * cfg);

static uint8_t set_host_sdclk(uint32_t frequencyKHz);
static mss_mmc_status_t set_data_timeout(uint32_t timeout_val_us);
static mss_mmc_status_t  set_sdhost_power(uint32_t voltage);
static void get_phy_addr(MSS_MMC_phydelay phydelaytype, uint8_t *phySetAddr);
static void phy_training_mmc(uint8_t delay_type, uint32_t clk_rate);
static void phy_write_set(uint8_t delay_type, uint8_t delay_value);
static mss_mmc_status_t device_set_hs_timing
(
    uint32_t hs_mode,
    const mss_mmc_cfg_t * cfg
);
static mss_mmc_status_t set_device_hs400_mode(const mss_mmc_cfg_t *cfg);
static void mmc_delay(uint32_t value);
static uint8_t is_uhsi_supported(void);
static mss_mmc_status_t cmd6_single_block_read
(
    uint32_t src_addr,
    uint32_t * dst_addr,
    uint32_t size
);
static mss_mmc_status_t set_sd_host_device_bus_mode(const mss_mmc_cfg_t * cfg);
static mss_mmc_status_t set_host_uhsi_mode
(
    uint8_t access_mode,
    uint8_t driver_strengh
);
static mss_mmc_status_t sd_host_process_switch_function
(
    uint8_t function_num,
    uint8_t group_num
);
static mss_mmc_status_t sd_host_exec_cmd6_command
(
    uint32_t argument,
    uint8_t *buffer,
    uint8_t function_num,
    uint8_t group_num
);
static mss_mmc_status_t sd_card_uhsi_supported(void);
static mss_mmc_status_t adma2_create_descriptor_table
(
    const uint8_t *data_src,
    uint32_t data_sz
);

static mss_mmc_status_t change_sdio_device_bus_mode(const mss_mmc_cfg_t * cfg);
static void sdio_host_access_cccr
(
    uint8_t transfer_direction,
    void *data,
    uint8_t data_size,
    uint8_t register_address
);
static void sdio_host_access_fbr
(
    uint8_t transfer_direction,
    void *data,
    uint8_t data_size,
    uint32_t register_address,
        uint8_t fun_num
);
static void sdio_host_get_tuple_from_cis
(
    uint32_t tuple_address,
    uint8_t tuple_code,
    uint8_t *buffer,
    uint8_t buffer_size
);
static const uint32_t* calc_write_pattern(const uint8_t bus_width);
static void host_mmc_tune(uint8_t value);
static mss_mmc_status_t sd_tuning(void);
static uint8_t calc_longest_valid_delay_chain_val(const uint8_t* pattern_ok);
static mss_mmc_status_t read_tune_block
(
    uint32_t *read_data,
    uint32_t size,
    uint8_t cmd
);
static mss_mmc_status_t execute_tunning_mmc(uint8_t data_width);

static mss_mmc_handler_t g_transfer_complete_handler_t;
/*****************************************************************************/

/*-------------------------------------------------------------------------*//**
 * See "mss_mmc.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_init
(
    const mss_mmc_cfg_t * cfg
)
{
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    cif_response_t response_status;
    uint32_t reg, cap, srs10, hrs6;
    uint32_t csd_reg[BLK_SIZE/WORD_SIZE];
    uint8_t *pcsd_reg = NULL_POINTER;

    uint32_t hw_sec_count;
    uint8_t hw_ext_csd_rev;
    uint8_t hw_device_type;
    uint8_t hw_strobe_suport;
    uint8_t hw_hs_timing;
    
    /* Reset MMC */
    SYSREG->SOFT_RESET_CR &= ~(MMC_SET << MMC_SOFTWARE_RESET_SHIFT);
    /* Disable MMC interrupt */
    PLIC_DisableIRQ(MMC_main_PLIC);

    g_transfer_complete_handler_t = NULL_POINTER;
    g_mmc_init_complete = MMC_CLEAR;
    g_sdio_fun_num = MMC_CLEAR;
    g_mmc_trs_status.state = MSS_MMC_NOT_INITIALISED;
    /* Set RCA default value */
    sdcard_RCA = RCA_VALUE;
    /* Reset host controller*/
    MMC->HRS00 |= HRS0_SOFTWARE_RESET;
    mmc_delay(DELAY_COUNT);

    do
    {
       reg = MMC->HRS00; 
    }while ((reg & HRS0_SOFTWARE_RESET) != MMC_CLEAR);

    /* Set de-bounce time */
    MMC->HRS01 = DEBOUNCING_TIME;
    /* eMMC Mode select in Host controller */
    hrs6 = MMC->HRS06;
    hrs6 &= ~MSS_MMC_MODE_MASK;
    /* Check card type SD or MMC */
    if (MSS_MMC_CARD_TYPE_MMC == cfg->card_type)
    {
        hrs6 |= MSS_MMC_MODE_LEGACY;
    }
    else
    {
        hrs6 |= MSS_MMC_MODE_SDCARD;
    }
    MMC->HRS06 = hrs6;
    /* Clear error/interrupt status */
    MMC->SRS12 = MMC_STATUS_CLEAR;
    
    reg = MMC->SRS15;
    cap = MMC->SRS16;
    /* Check DMA 64 bit support */
    if ((cap & SRS16_64BIT_SUPPORT) != MMC_CLEAR)
    {
        reg |= SRS15_64_BIT_ADDRESSING;
        reg |= SRS15_HOST_4_ENABLE;
        MMC->SRS15 = reg;
    }
    /* Enable error/interrupt status */
    MMC->SRS13 = SRS13_STATUS_EN;
    /* Disable error/interrupt */
    MMC->SRS14 = MMC_CLEAR;
    /* Set 500ms data timeout */
    ret_status = set_data_timeout(DATA_TIMEOUT_VALUE);
    if (MSS_MMC_NO_ERROR != ret_status)
    {
        ret_status = MSS_MMC_NOT_INITIALISED;
    }
    /* Turn-off Host Controller Power */
    ret_status = set_sdhost_power(MMC_CLEAR);
    if (MSS_MMC_NO_ERROR != ret_status)
    {
        ret_status = MSS_MMC_NOT_INITIALISED;
    }
    reg = MMC->SRS09;
    /* If card stable is not set it means that something is wrong */
    if (((reg & SRS9_CARD_STATE_STABLE) == MMC_CLEAR) &&
                                    (MSS_MMC_NO_ERROR == ret_status))
    {
        ret_status = MSS_MMC_CARD_STATE_STABLE_ERR;
    }
    /* if card is not inserted to SD socked */
    if (MSS_MMC_CARD_TYPE_SD == cfg->card_type)
    {
        if (((reg & SRS9_CARD_INSERTED) == MMC_CLEAR) &&
                                    (MSS_MMC_NO_ERROR == ret_status))
        {
            ret_status = MSS_MMC_CARD_INSERTED_ERR;
        }
    }
    /* Set 1-bit bus mode */
    srs10 = MMC->SRS10;
    srs10 &= ~SRS10_DATA_WIDTH_4BIT;
    srs10 &= ~SRS10_EXTENDED_DATA_TRANSFER_WIDTH;
    MMC->SRS10 = srs10;

    if (((MSS_MMC_MODE_HS200 == cfg->bus_speed_mode) ||
            (MSS_MMC_MODE_HS400_ES == cfg->bus_speed_mode) ||
            ( MSS_MMC_MODE_HS400 == cfg->bus_speed_mode)) &&
                    (MSS_MMC_NO_ERROR == ret_status))
    {
        /* Set Host Controller power to 1.8v */
        ret_status = set_sdhost_power(SRS10_SET_1_8V_BUS_VOLTAGE);
        if (ret_status != MSS_MMC_NO_ERROR)
        {
            ret_status = MSS_MMC_NOT_INITIALISED;
        }
    }
    else
    {
        if (MSS_MMC_NO_ERROR == ret_status)
        {
            /* Set Host Controller power to 3.3v */
            ret_status = set_sdhost_power(SRS10_SET_3_3V_BUS_VOLTAGE);
            if (ret_status != MSS_MMC_NO_ERROR)
            {
                ret_status = MSS_MMC_NOT_INITIALISED;
            }
        }
    }

    if (MSS_MMC_NO_ERROR == ret_status)
    {
        /* Set 400khz clock */
        set_host_sdclk(MSS_MMC_CLOCK_400KHZ);
    }
    mmc_delay(DELAY_COUNT);
    /* MMC memory */
    if ((MSS_MMC_CARD_TYPE_MMC == cfg->card_type) && (MSS_MMC_NO_ERROR == ret_status))
    {
        response_status = cif_send_cmd(RESET_ARG, MMC_CMD_0_GO_IDLE_STATE,
                                                    MSS_MMC_RESPONSE_NO_RESP);
        mmc_delay(DELAY_COUNT);
        /* Reset CMD is success */
        if (TRANSFER_IF_SUCCESS == response_status)
        {
            ret_status = mmccard_oper_config(cfg);
            if (MSS_MMC_CARD_SELECT_SUCCESS == ret_status)
            {
                 /* mmc init success */
                g_mmc_init_complete = MMC_SET;
                /* Set Block length in device */
                response_status = cif_send_cmd(BLK_SIZE, SD_CMD_16,
                                                        MSS_MMC_RESPONSE_R1);
                if (TRANSFER_IF_SUCCESS == response_status)
                {
                    /* Read EXT_CSD */
                    ret_status = MSS_MMC_single_block_read(READ_SEND_EXT_CSD, csd_reg);
                    if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
                    {
                        pcsd_reg = ((uint8_t *)csd_reg);
                        hw_device_type   =  pcsd_reg[EXT_CSD_CARD_TYPE_OFFSET];
                        hw_hs_timing   = pcsd_reg[EXT_CSD_HS_TIMING_OFFSET];
                        hw_strobe_suport = pcsd_reg [EXT_CSD_ES_SUPPORT_OFFSET];
                        g_device_hpi_suport = pcsd_reg[EXT_CSD_ES_SUPPORT_OFFSET];
    
                        if ((g_device_hpi_suport & DEVICE_HPI_SUPPORT) == DEVICE_HPI_SUPPORT)
                        {
                            /* enable HPI in device */
                            response_status = cif_send_cmd(MMC_HPI_ENABLE,
                                                            MMC_CMD_6_SWITCH,
                                                            MSS_MMC_RESPONSE_R1B);
                            /*
                            * Writing to the EXT CSD register takes significant time,
                            * so function must not return until the 'READY FOR DATA'
                            * bit is set in the Card Status Register.
                            */
                            while (DEVICE_BUSY == response_status)
                            {
                                response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                                            MMC_CMD_13_SEND_STATUS,
                                                            MSS_MMC_RESPONSE_R1);
                            }
                        }
                        switch (cfg->bus_speed_mode)
                        {
                            case MSS_MMC_MODE_HS400_ES:
    
                                if (((hw_device_type & DEVICE_SUPPORT_HS400) == DEVICE_SUPPORT_HS400)
                                                && (hw_strobe_suport == MMC_SET))
                                {
                                    ret_status =  set_device_hs400_mode(cfg);
                                    if (MSS_MMC_TRANSFER_SUCCESS != ret_status)
                                    {
                                        ret_status = MSS_MMC_HS400_MODE_SETUP_FAILURE;
                                    }
                                }
                                else
                                {
                                    ret_status = MSS_MMC_DEVICE_NOT_SUPPORT_HS400;
                                }
                                break;
    
                            case MSS_MMC_MODE_HS400:
                        
                                if ((hw_device_type & DEVICE_SUPPORT_HS400) == DEVICE_SUPPORT_HS400)
                                {
                                    ret_status =  set_device_hs400_mode(cfg);
                                    if (MSS_MMC_TRANSFER_SUCCESS != ret_status)
                                    {
                                        ret_status = MSS_MMC_HS400_MODE_SETUP_FAILURE;
                                    }
                                }
                                else
                                {
                                    ret_status = MSS_MMC_DEVICE_NOT_SUPPORT_HS400;
                                }
                                break;
                                
                            case MSS_MMC_MODE_HS200:
                            
                                if ((hw_device_type & DEVICE_SUPPORT_HS200) == DEVICE_SUPPORT_HS200)
                                {
                                    ret_status = device_set_hs_timing(DEVICE_SUPPORT_HS200, cfg);
                                    if (MSS_MMC_TRANSFER_SUCCESS != ret_status)
                                    {
                                        ret_status = MSS_MMC_MODE_NOT_SUPPORT_DATAWIDTH;
                                    }
                                    else
                                    {
                                        ret_status = execute_tunning_mmc(cfg->data_bus_width);
                                        if (MSS_MMC_TRANSFER_SUCCESS != ret_status)
                                        {
                                            ret_status = MSS_MMC_TRANSFER_FAIL;
                                        }
                                    }
                                }
                                else
                                {
                                    ret_status = MSS_MMC_DEVICE_NOT_SUPPORT_HS200;
                                }
                                break;
                                
                            case MSS_MMC_MODE_DDR:
                        
                                if ((hw_device_type & DEVICE_SUPPORT_DDR) == DEVICE_SUPPORT_DDR)
                                {
                                    ret_status = device_set_hs_timing(DEVICE_SUPPORT_DDR, cfg);
                                    if (MSS_MMC_TRANSFER_SUCCESS != ret_status)
                                    {
                                        ret_status = MSS_MMC_MODE_NOT_SUPPORT_DATAWIDTH;
                                    }
                                    /* Enable Host High Speed */
                                    MMC->SRS10 |= (MMC_SET << SHIFT_2BIT);
                                }
                                else
                                {
                                    ret_status = MSS_MMC_DEVICE_NOT_SUPPORT_DDR;
                                }
                                break;
    
                            case MSS_MMC_MODE_SDR:
                        
                                if ((hw_device_type & DEVICE_SUPPORT_SDR_50MHZ) == DEVICE_SUPPORT_SDR_50MHZ)
                                {
                                    ret_status = device_set_hs_timing(DEVICE_SUPPORT_SDR, cfg);
                                    if (MSS_MMC_TRANSFER_SUCCESS != ret_status)
                                    {
                                        ret_status = MSS_MMC_DWIDTH_ERR;
                                    }
                                    /* Enable Host High Speed */
                                    MMC->SRS10 |= (MMC_SET << SHIFT_2BIT);
                                }
                                else if ((hw_device_type & DEVICE_SUPPORT_SDR_25MHZ) == DEVICE_SUPPORT_SDR_25MHZ)
                                {
                                    ret_status = device_set_hs_timing(DEVICE_SUPPORT_SDR, cfg);
                                    if (MSS_MMC_TRANSFER_SUCCESS != ret_status)
                                    {
                                        ret_status = MSS_MMC_DWIDTH_ERR;
                                    }
                                }
                                else
                                {
                                    ret_status = MSS_MMC_DEVICE_NOT_SUPPORT_SDR;
                                }
                                break;
    
                            case MSS_MMC_MODE_LEGACY:
                                ret_status = device_set_hs_timing(DEVICE_SUPPORT_LEGACY, cfg);
                                if (MSS_MMC_TRANSFER_SUCCESS != ret_status)
                                {
                                    ret_status = MSS_MMC_CLK_DIV_ERR;
                                }
                                break;
                            default:
                                ret_status = MSS_MMC_INIT_FAILURE;
                                break;
                        }
                        if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
                        {
                            ret_status = MSS_MMC_single_block_read(READ_SEND_EXT_CSD, csd_reg);
                            if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
                            {
                                pcsd_reg = (uint8_t *)csd_reg;
                                /* offsets defined in JESD84-B51 extended CSD */
                                hw_sec_count   = csd_reg[EXT_CSD_SECTOR_COUNT_OFFSET/WORD_SIZE];
                                hw_ext_csd_rev = pcsd_reg[EXT_CSD_REVISION_OFFSET] & BYTE_MASK;
                                hw_hs_timing   = pcsd_reg[EXT_CSD_HS_TIMING_OFFSET];
    
                                if ((MMC_CLEAR == hw_hs_timing) && 
                                    (cfg->bus_speed_mode != MSS_MMC_MODE_LEGACY))
                                {
                                    ret_status = MSS_MMC_CLK_DIV_ERR;
                                }
                            }
                        }
                        if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
                        {
                            ret_status = MSS_MMC_INIT_SUCCESS;
                            g_mmc_trs_status.state = MSS_MMC_INIT_SUCCESS;
                        }
                        else
                        {
                            g_mmc_init_complete = MMC_CLEAR;
                            ret_status = MSS_MMC_INIT_FAILURE;
                            g_mmc_trs_status.state = MSS_MMC_INIT_FAILURE;
                        }
                    }
                    else
                    {
                        g_mmc_init_complete = MMC_CLEAR;
                        ret_status = MSS_MMC_INIT_FAILURE;
                        g_mmc_trs_status.state = MSS_MMC_INIT_FAILURE;
                    }
                }
                else
                {
                    g_mmc_init_complete = MMC_CLEAR;
                    ret_status = MSS_MMC_INIT_FAILURE;
                    g_mmc_trs_status.state = MSS_MMC_INIT_FAILURE;
                }
            }
        }
        else
        {
            ret_status = MSS_MMC_RESET_ERR;
        }
    }
    /* SD Memory card */
    else if ((MSS_MMC_CARD_TYPE_SD == cfg->card_type) && (MSS_MMC_NO_ERROR == ret_status))
    {
        response_status = cif_send_cmd(RESET_ARG, MMC_CMD_0_GO_IDLE_STATE,
                                                    MSS_MMC_RESPONSE_NO_RESP);
        mmc_delay(DELAY_COUNT);
    
        if (TRANSFER_IF_SUCCESS == response_status)
        {
            /* Specify the operating conditions to the device */
            ret_status = sdcard_oper_config(cfg);

            if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
            {
                ret_status = MSS_MMC_INIT_SUCCESS;
                g_mmc_trs_status.state = MSS_MMC_INIT_SUCCESS;
            }
            else
            {
                g_mmc_init_complete = MMC_CLEAR;
                ret_status = MSS_MMC_INIT_FAILURE;
                g_mmc_trs_status.state = MSS_MMC_INIT_FAILURE;
            }
        }
        else
        {
            g_mmc_init_complete = MMC_CLEAR;
            ret_status = MSS_MMC_INIT_FAILURE;
            g_mmc_trs_status.state = MSS_MMC_INIT_FAILURE;
        }
    }
    /* SDIO card */
    else if ((MSS_MMC_CARD_TYPE_SDIO == cfg->card_type) && (MSS_MMC_NO_ERROR == ret_status))
    {
        ret_status = sdio_oper_config(cfg);
        
        if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
        {
            ret_status = MSS_MMC_INIT_SUCCESS;
            g_mmc_trs_status.state = MSS_MMC_INIT_SUCCESS;
        }
        else
        {
            g_mmc_init_complete = MMC_CLEAR;
            ret_status = MSS_MMC_INIT_FAILURE;
            g_mmc_trs_status.state = MSS_MMC_INIT_FAILURE;
        }
    }
    else
    {
        ret_status = MSS_MMC_NOT_INITIALISED;
        g_mmc_init_complete = MMC_CLEAR;
        g_mmc_trs_status.state = MSS_MMC_NOT_INITIALISED;
    }
    /* Clear interrupts */
    MMC->SRS12 = ~(SRS12_CURRENT_LIMIT_ERROR
                            | SRS12_CARD_INTERRUPT
                            | SRS12_CARD_REMOVAL
                            | SRS12_CARD_INSERTION);
    return(ret_status);
}

/*-------------------------------------------------------------------------*//**
 * See "mss_mmc.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_single_block_read
(
    uint32_t src_addr,
    uint32_t * dst_addr
)
{
    uint32_t isr_errors;
    uint32_t blk_read, srs03_data, srs9;
    uint16_t word_cnt = (BLK_SIZE/WORD_SIZE);
    uint32_t idx_cnt;
    cif_response_t response_status;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    
    if (g_mmc_init_complete == MMC_SET)
    {
        do
        {
            response_status = cif_send_cmd(sdcard_RCA << SHIFT_16BIT,
                                        MMC_CMD_13_SEND_STATUS,
                                        MSS_MMC_RESPONSE_R1);
        } while (DEVICE_BUSY == response_status);
    
        /* Reset Data and cmd line */
        MMC->SRS11 |= MMC_RESET_DATA_CMD_LINE;
        /* Block length and count*/
        MMC->SRS01 = (BLK_SIZE | (MMC_SET << BLOCK_COUNT_ENABLE_SHIFT));
    
        /* DPS, Data transfer direction - read */
        srs03_data = (uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_READ
                    |SRS3_BLOCK_COUNT_ENABLE | SRS3_RESP_ERR_CHECK_EN
                    | SRS3_RESP_INTER_DISABLE | SRS3_RESPONSE_CHECK_TYPE_R1
                    | SRS3_RESP_LENGTH_48 | SRS3_CRC_CHECK_EN
                    | SRS3_INDEX_CHECK_EN);
        /* Check cmd and data line busy */
        do
        {
            srs9 = MMC->SRS09;
        }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT)) != MMC_CLEAR);
        /* eMMC EXT_CSD read */
        if (READ_SEND_EXT_CSD == src_addr)
        {
                /* Command argument */
                MMC->SRS02 = MMC_CLEAR;
                /* execute command */
                MMC->SRS03 = (uint32_t)((MMC_CMD_8_SEND_EXT_CSD << MMC_SRS03_COMMAND_SHIFT) | srs03_data);
        }/* SD card ACMD51 read */
        else if (READ_SEND_SCR == src_addr)
        {
            MMC->SRS01 = (SCR_REG_DATA_SIZE | (MMC_SET << BLOCK_COUNT_ENABLE_SHIFT));
            word_cnt = SHIFT_2BIT;
            response_status = cif_send_cmd(SCR_REG_DATA_SIZE,
                                        SD_CMD_16,
                                        MSS_MMC_RESPONSE_R1);
    
            response_status = cif_send_cmd(sdcard_RCA << SHIFT_16BIT,
                                        SD_CMD_55,
                                        MSS_MMC_RESPONSE_R1);
            /* Command argument */
            MMC->SRS02 = MMC_CLEAR;
            /* execute command */
            MMC->SRS03 = (uint32_t)((SD_ACMD_51 << MMC_SRS03_COMMAND_SHIFT) | srs03_data);
        }
        else
        {
            /* Command argument */
            MMC->SRS02 = src_addr;
            /* execute command */
            MMC->SRS03 = (uint32_t)((MMC_CMD_17_READ_SINGLE_BLOCK << MMC_SRS03_COMMAND_SHIFT) | srs03_data);
        }
        
        mmc_delay(DELAY_COUNT);
        
        idx_cnt = MMC_CLEAR;
        blk_read = MMC->SRS12;
        /* Read in the contents of the Buffer */
        while (((blk_read & SRS12_BUFFER_READ_READY) != MMC_CLEAR) && (word_cnt > (BUFF_EMPTY)))
        {
            dst_addr[idx_cnt] = MMC->SRS08;
            ++idx_cnt;
            --word_cnt;
            blk_read = MMC->SRS12;
        }

        isr_errors = MMC->SRS12;
        /* Abort if any errors*/
        if ((SRS12_ERROR_STATUS_MASK & isr_errors) == MMC_CLEAR)
        {
                /*
                * Ensure no error fields in Card Status register are set and that
                * the device is idle before this function returns.
                */
                do
                {
                    response_status = cif_send_cmd(sdcard_RCA << SHIFT_16BIT,
                                                MMC_CMD_13_SEND_STATUS,
                                                MSS_MMC_RESPONSE_R1);
                } while (DEVICE_BUSY == response_status);
    
                /* Reset Data and cmd line */
                MMC->SRS11 |= MMC_RESET_DATA_CMD_LINE;

                if (TRANSFER_IF_SUCCESS == response_status)
                {
                    ret_status = MSS_MMC_TRANSFER_SUCCESS;
                }
                else
                {
                    ret_status = MSS_MMC_TRANSFER_FAIL;
                }
        }
        else
        {
    
            ret_status = MSS_MMC_ERR_INTERRUPT;
        }
        /* Clear all status interrupts except:
        *  current limit error, card interrupt, card removal, card insertion */
        MMC->SRS12 = ~(SRS12_CURRENT_LIMIT_ERROR
                                | SRS12_CARD_INTERRUPT
                                | SRS12_CARD_REMOVAL
                                | SRS12_CARD_INSERTION);
    }
    else
    {
        ret_status = MSS_MMC_NOT_INITIALISED;
    }
    return (ret_status);
}
/*******************************************************************************
*************************** READ APIs *****************************************
*******************************************************************************/
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_sdma_read
(
    uint32_t src,
    uint8_t *dest,
    uint32_t size
)
{
    uint32_t blockcount;
    uint32_t argument;
    uint32_t blocklen;
    uint32_t tmp, srs03_data, srs9;
    cif_response_t response_status;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;

    blocklen = BLK_SIZE;
    argument = src;

    if (g_mmc_init_complete == MMC_SET)
    {
        if (MSS_MMC_TRANSFER_IN_PROGRESS == g_mmc_trs_status.state)
        {
            ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
        }
        else
        {
            /* Size should be divided by 512, not greater than (32MB - 512) */
            if (((size % blocklen) != MMC_CLEAR) || (size > (SIZE_32MB - BLK_SIZE))
                    || (size == MMC_CLEAR) || (dest == NULL_POINTER))
            {
                ret_status = MSS_MMC_INVALID_PARAMETER;
                g_mmc_trs_status.state = MSS_MMC_INVALID_PARAMETER;
            }
            else
            {
                /* Disable PLIC interrupt for MMC */
                PLIC_DisableIRQ(MMC_main_PLIC);
                /* Calculate block count */
                blockcount = ((size - MMC_SET) / blocklen) + MMC_SET;
                /* check eMMC/SD device is busy */
                do
                {
                    response_status = cif_send_cmd(sdcard_RCA << SHIFT_16BIT,
                                            MMC_CMD_13_SEND_STATUS,
                                            MSS_MMC_RESPONSE_R1);
                } while (DEVICE_BUSY == response_status);

                /* select SDMA */
                tmp = MMC->SRS10;
                tmp = (tmp & (~SRS10_DMA_SELECT_MASK));
                MMC->SRS10 = (tmp | SRS10_DMA_SELECT_SDMA);
                /* SDMA setup */
                MMC->SRS22 = ((uint32_t)((uintptr_t)dest));
                MMC->SRS23 = ((uint32_t)(((uint64_t)((uintptr_t)dest)) >> MMC_64BIT_UPPER_ADDR_SHIFT));
                /* Block length and count SDMA buffer boundary */
                MMC->SRS01 = (blocklen | (blockcount << BLOCK_COUNT_ENABLE_SHIFT)
                                       |  SRS1_DMA_BUFF_SIZE_512KB);
            
                /* Enable interrupts */
                MMC->SRS14 = (SRS14_COMMAND_COMPLETE_SIG_EN |
                              SRS14_TRANSFER_COMPLETE_SIG_EN |
                              SRS14_DMA_INTERRUPT_SIG_EN |
                              SRS14_DATA_TIMEOUT_ERR_SIG_EN);

                PLIC_EnableIRQ(MMC_main_PLIC);

                /* Multi Block read */
                if (blockcount > MMC_SET)
                {
                        /* DPS, Data transfer direction - read */
                        srs03_data = ((uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_READ
                                            | SRS3_MULTI_BLOCK_SEL | SRS3_BLOCK_COUNT_ENABLE
                                            | SRS3_RESPONSE_CHECK_TYPE_R1 | SRS3_RESP_LENGTH_48
                                            | SRS3_CRC_CHECK_EN | SRS3_INDEX_CHECK_EN
                                            | SRS3_DMA_ENABLE));
                        /* Check cmd and data line busy */
                        do
                        {
                            srs9 = MMC->SRS09;
                        }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT) != MMC_CLEAR));
                        /* multi block transfer */
                        g_mmc_is_multi_blk = MMC_SET;
                        /* Command argument */
                        MMC->SRS02 = argument;
                        /* execute command */
                        MMC->SRS03 = ((uint32_t)((MMC_CMD_18_READ_MULTIPLE_BLOCK << MMC_SRS03_COMMAND_SHIFT) | srs03_data));
                }
                else /* single block read */
                {
                    /* DPS, Data transfer direction - read */
                    srs03_data = ((uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_READ
                                            | SRS3_BLOCK_COUNT_ENABLE
                                            | SRS3_RESPONSE_CHECK_TYPE_R1
                                            | SRS3_RESP_LENGTH_48
                                            | SRS3_CRC_CHECK_EN | SRS3_INDEX_CHECK_EN
                                            | SRS3_DMA_ENABLE));
                    do
                    {
                        srs9 = MMC->SRS09;
                    }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT) != MMC_CLEAR));
                    /* single block transfer */
                    g_mmc_is_multi_blk = MMC_CLEAR;
                    /* Command argument */
                    MMC->SRS02 = argument;
                    /* execute command */
                    MMC->SRS03 = ((uint32_t)((MMC_CMD_17_READ_SINGLE_BLOCK << MMC_SRS03_COMMAND_SHIFT) | srs03_data));
                }
            }
            g_mmc_trs_status.state = MSS_MMC_TRANSFER_IN_PROGRESS;
            ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
        }
    }
    else
    {
        ret_status = MSS_MMC_NOT_INITIALISED;
    }
    return ret_status;
}
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_adma2_read
(
    uint32_t src,
    uint8_t *dest,
    uint32_t size
)
{
    uint32_t blockcount;
    uint32_t argument;
    uint32_t blocklen;
    uint32_t tmp, srs03_data, srs9;
    cif_response_t response_status;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;

    blocklen = BLK_SIZE;
    argument = src;
    
    if (g_mmc_init_complete == MMC_SET)
    {
        if (MSS_MMC_TRANSFER_IN_PROGRESS == g_mmc_trs_status.state)
        {
            ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
        }
        else
        {
            /* Size should be divided by 512, not greater than (32MB - 512) */
            if (((size % blocklen) != MMC_CLEAR) || (size > (SIZE_32MB - BLK_SIZE))
                    || (size == MMC_CLEAR) || (dest == NULL_POINTER))
            {
                ret_status = MSS_MMC_INVALID_PARAMETER;
            }
            else
            {
                /* Disable PLIC interrupt for MMC */
                PLIC_DisableIRQ(MMC_main_PLIC);
                /* Calculate block count */
                blockcount = ((size - MMC_SET) / blocklen) + MMC_SET;

                /* Create ADMA2 descriptor table */
                ret_status = adma2_create_descriptor_table(dest, size);
                if (ret_status != MSS_MMC_INVALID_PARAMETER)
                {
                    /* Check eMMC/SD device is busy */
                    do
                    {
                        response_status = cif_send_cmd(sdcard_RCA << SHIFT_16BIT,
                                                MMC_CMD_13_SEND_STATUS,
                                                MSS_MMC_RESPONSE_R1);
                    } while (DEVICE_BUSY == response_status);
    
                    /* ADMA setup */
                    MMC->SRS22 = (uint32_t)(uintptr_t)adma_descriptor_table;
                    MMC->SRS23 = (uint32_t)(((uint64_t)(uintptr_t)adma_descriptor_table) >> MMC_64BIT_UPPER_ADDR_SHIFT);
                    /* Select ADMA2 */
                    tmp = MMC->SRS10;
                    tmp = (tmp & (~SRS10_DMA_SELECT_MASK));
                    MMC->SRS10 = (tmp | SRS10_DMA_SELECT_ADMA2);
        
                    /* Block length and count */
                    MMC->SRS01 = (blocklen | (blockcount << BLOCK_COUNT_ENABLE_SHIFT));
        
                    /* Enable interrupts */
                    MMC->SRS14 = (SRS14_COMMAND_COMPLETE_SIG_EN |
                                    SRS14_TRANSFER_COMPLETE_SIG_EN  |
                                    SRS14_DATA_TIMEOUT_ERR_SIG_EN |
                                    SRS14_ADMA_ERROR_SIG_EN);
                    PLIC_EnableIRQ(MMC_main_PLIC);
                    /* Multi Block read */
                    if (blockcount > MMC_SET)
                    {
                            /* DPS, Data transfer direction - read */
                            srs03_data = (uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_READ
                                                | SRS3_MULTI_BLOCK_SEL | SRS3_BLOCK_COUNT_ENABLE
                                                | SRS3_RESPONSE_CHECK_TYPE_R1 | SRS3_RESP_LENGTH_48
                                                | SRS3_CRC_CHECK_EN | SRS3_INDEX_CHECK_EN
                                                | SRS3_DMA_ENABLE);
                            /* Check cmd and data line busy */
                            do
                            {
                                srs9 = MMC->SRS09;
                            }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT) != MMC_CLEAR));
                            /* multi block transfer */
                            g_mmc_is_multi_blk = MMC_SET;
                            /* Command argument */
                            MMC->SRS02 = argument;
                            /* execute command */
                            MMC->SRS03 = (uint32_t)((MMC_CMD_18_READ_MULTIPLE_BLOCK << MMC_SRS03_COMMAND_SHIFT) | srs03_data);
                    }
                    else /* single block read */
                    {
                        /* DPS, Data transfer direction - read */
                        srs03_data = (uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_READ | SRS3_BLOCK_COUNT_ENABLE
                                                | SRS3_RESPONSE_CHECK_TYPE_R1 | SRS3_RESP_LENGTH_48
                                                | SRS3_CRC_CHECK_EN | SRS3_INDEX_CHECK_EN
                                                | SRS3_DMA_ENABLE);
                        do
                        {
                            srs9 = MMC->SRS09;
                        }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT) != MMC_CLEAR));
                        /* single block transfer */
                        g_mmc_is_multi_blk = MMC_CLEAR;
                        /* Command argument */
                        MMC->SRS02 = argument;
                        /* execute command */
                        MMC->SRS03 = (uint32_t)((MMC_CMD_17_READ_SINGLE_BLOCK << MMC_SRS03_COMMAND_SHIFT) | srs03_data);
                    }
                    g_mmc_trs_status.state = MSS_MMC_TRANSFER_IN_PROGRESS;
                    ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
                }
            }
        }
    }
    else
    {
        ret_status = MSS_MMC_NOT_INITIALISED;
    }
    return ret_status;
}
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_sdio_single_block_read
(
    uint8_t function_num,
    uint32_t src_addr,
    uint8_t *dst_addr
)
{
    uint32_t isr_errors;
    uint32_t blk_read, srs03_data, srs9;
    uint16_t word_cnt = (BLK_SIZE/WORD_SIZE);
    uint32_t arguement;
    uint32_t idx_cnt;
    uint8_t fun_num = MMC_SET << function_num;
    uint16_t blck_size = BLK_SIZE;
    cif_response_t response_status;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;

    if (g_mmc_init_complete == MMC_SET)
    {
        if ((g_sdio_fun_num & fun_num) == MMC_CLEAR)
        {
            g_sdio_fun_num |= fun_num;
            sdio_host_access_cccr(SDIOHOST_CCCR_WRITE, &fun_num,
                                   sizeof(fun_num),
                                   MSS_MMC_CCCR_IO_ENABLE);
            sdio_host_access_cccr(SDIOHOST_CCCR_READ, &fun_num,
                                   sizeof(fun_num),
                                   MSS_MMC_CCCR_IO_READY);
            while (fun_num == MMC_CLEAR)
            {
                sdio_host_access_cccr(SDIOHOST_CCCR_READ, &fun_num,
                                       sizeof(fun_num),
                                       MSS_MMC_CCCR_IO_READY);
            }
            sdio_host_access_fbr(SDIOHOST_CCCR_WRITE,  &blck_size, BYTES_2, MSS_MMC_FBR_BLOCK_SIZE, function_num);
        }
        /* Reset Data and cmd line */
        MMC->SRS11 |= MMC_RESET_DATA_CMD_LINE;
        /* Block length and count*/
        MMC->SRS01 = (BLK_SIZE | (MMC_SET << BLOCK_COUNT_ENABLE_SHIFT));

        /* DPS, Data transfer direction - read */
        srs03_data = (uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_READ |SRS3_BLOCK_COUNT_ENABLE
                            | SRS3_RESP_ERR_CHECK_EN | SRS3_RESP_INTER_DISABLE
                            | SRS3_RESPONSE_CHECK_TYPE_R5 | SRS3_RESP_LENGTH_48
                            | SRS3_CRC_CHECK_EN | SRS3_INDEX_CHECK_EN);
        /* Check cmd and data line busy */
        do
        {
            srs9 = MMC->SRS09;
        }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT)) != MMC_CLEAR);

        arguement = (function_num << SHIFT_31BIT) | (src_addr << SHIFT_9BIT)| BLK_SIZE;
            /* Command argument */
        MMC->SRS02 = arguement;
            /* execute command */
        MMC->SRS03 = (uint32_t)((SDIO_CMD_53_IO_RW_EXTENDED << MMC_SRS03_COMMAND_SHIFT) | srs03_data);

        mmc_delay(DELAY_COUNT);

        idx_cnt = MMC_CLEAR;
        blk_read = MMC->SRS12;
        /* Read in the contents of the Buffer */
        while (((blk_read & SRS12_BUFFER_READ_READY) == SRS12_BUFFER_READ_READY) && (word_cnt > (BUFF_EMPTY)))
        {
            dst_addr[idx_cnt] = MMC->SRS08;
            ++idx_cnt;
            --word_cnt;
            blk_read = MMC->SRS12;
        }

        isr_errors = MMC->SRS12;
        /* Abort if any errors*/
        if ((SRS12_ERROR_STATUS_MASK & isr_errors) == MMC_CLEAR)
        {
              ret_status = MSS_MMC_TRANSFER_FAIL;
        }
        else
        {
            ret_status = MSS_MMC_ERR_INTERRUPT;
        }
        /* Clear all status interrupts except:
        *  current limit error, card interrupt, card removal, card insertion */
        MMC->SRS12 = ~(SRS12_CURRENT_LIMIT_ERROR
                                | SRS12_CARD_INTERRUPT
                                | SRS12_CARD_REMOVAL
                                | SRS12_CARD_INSERTION);
    }
    else
    {
        ret_status = MSS_MMC_NOT_INITIALISED;
    }
    return (ret_status);
}
/*******************************************************************************
*************************** WRITE APIs *****************************************
*******************************************************************************/
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_single_block_write
(
    const uint32_t * src_addr,
    uint32_t dst_addr
)
{
    cif_response_t response_status;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    uint16_t word_cnt = MMC_CLEAR;
    uint8_t blk_tran_err;
    uint8_t blk_write;
    uint32_t srs03_data, srs9;
    
    if (g_mmc_init_complete == MMC_SET)
    {
        do
        {
            response_status = cif_send_cmd(sdcard_RCA << SHIFT_16BIT,
                                        MMC_CMD_13_SEND_STATUS,
                                        MSS_MMC_RESPONSE_R1);
        } while (DEVICE_BUSY == response_status);
    
        /* Reset Data and cmd line */
        MMC->SRS11 |= MMC_RESET_DATA_CMD_LINE;

        /* Block length and count*/
        MMC->SRS01 = (BLK_SIZE | (MMC_SET << BLOCK_COUNT_ENABLE_SHIFT));
    
        /* DPS, Data transfer direction - read */
        srs03_data = (uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_WRITE |SRS3_BLOCK_COUNT_ENABLE
                            | SRS3_RESP_ERR_CHECK_EN | SRS3_RESP_INTER_DISABLE
                            | SRS3_RESPONSE_CHECK_TYPE_R1 | SRS3_RESP_LENGTH_48
                            | SRS3_CRC_CHECK_EN | SRS3_INDEX_CHECK_EN);
        /* Check cmd and data line busy */
        do
        {
            srs9 = MMC->SRS09;
        }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT)) != MMC_CLEAR);
    
        /* Command argument */
        MMC->SRS02 = dst_addr;
        /* execute command */
        MMC->SRS03 = (uint32_t)((MMC_CMD_24_WRITE_SINGLE_BLOCK << MMC_SRS03_COMMAND_SHIFT) | srs03_data);
    
        do
        {
            blk_write = MMC->SRS12;
        } while (MMC_CLEAR == (blk_write & SRS12_BUFFER_WRITE_READY));
        /*
        * Load the block of data into the
        * buffer, one byte at a time through the Buffer Data Register.
        */
        while ((BLK_SIZE/WORD_SIZE) > word_cnt)
        {
            MMC->SRS08 = src_addr[word_cnt];
            ++word_cnt;
        }
        /* Check is block write complete */
        do
        {
            blk_write = MMC->SRS12;
        } while (MMC_CLEAR == (blk_write & SRS12_TRANSFER_COMPLETE));

        blk_tran_err = MMC->SRS12;
        /* Abort if any errors*/
        if ((SRS12_ERROR_STATUS_MASK & blk_tran_err) == MMC_CLEAR)
        {
            /*
            * Ensure no error fields in Card Status register are set and that
            * the device is idle before this function returns.
            */
            do
            {
                response_status = cif_send_cmd(sdcard_RCA << SHIFT_16BIT,
                                            MMC_CMD_13_SEND_STATUS,
                                            MSS_MMC_RESPONSE_R1);
            } while (DEVICE_BUSY == response_status);
    
            /* Reset Data and cmd line */
            MMC->SRS11 |= MMC_RESET_DATA_CMD_LINE;

            if (TRANSFER_IF_SUCCESS == response_status)
            {
                ret_status = MSS_MMC_TRANSFER_SUCCESS;
            }
            else
            {
                ret_status = MSS_MMC_TRANSFER_FAIL;
            }
        }
        else
        {
            ret_status = MSS_MMC_ERR_INTERRUPT;
        }
    
        MMC->SRS12 = ~(SRS12_CURRENT_LIMIT_ERROR
                                | SRS12_CARD_INTERRUPT
                                | SRS12_CARD_REMOVAL
                                | SRS12_CARD_INSERTION);
    }
    else
    {
        ret_status = MSS_MMC_NOT_INITIALISED;
    }
    return (ret_status);
}
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_sdma_write
(
    const uint8_t *src,
    uint32_t dest,
    uint32_t size
)
{
    uint32_t blockcount;
    uint32_t argument;
    uint32_t blocklen;
    uint32_t tmp, srs03_data, srs9;
    cif_response_t response_status;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;

    blocklen = BLK_SIZE;
    argument = dest;
    
    if (g_mmc_init_complete == MMC_SET)
    {
        if (MSS_MMC_TRANSFER_IN_PROGRESS == g_mmc_trs_status.state)
        {
            ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
        }
        else
        {
            /* Size should be divided by 512, not greater than (32MB - 512) */
            if (((size % blocklen) != MMC_CLEAR) || (size > (SIZE_32MB - BLK_SIZE))
                    || (size == MMC_CLEAR) || (src == NULL_POINTER))
            {
                ret_status = MSS_MMC_INVALID_PARAMETER;
            }
            else
            {
                /* disable PCLI interrupt for MMC */
                PLIC_DisableIRQ(MMC_main_PLIC);
                
                /* Calculate block count */
                blockcount = ((size - MMC_SET) / blocklen) + MMC_SET;
                /* check MMC device is busy */
                do
                {
                    response_status = cif_send_cmd(sdcard_RCA << SHIFT_16BIT,
                                            MMC_CMD_13_SEND_STATUS,
                                            MSS_MMC_RESPONSE_R1);
                } while (DEVICE_BUSY == response_status);
    
                tmp = MMC->SRS10;
                tmp = (tmp & (~SRS10_DMA_SELECT_MASK));
                MMC->SRS10 = (tmp | SRS10_DMA_SELECT_SDMA);
    
                /* SDMA setup */
                MMC->SRS22 = (uint32_t)(uintptr_t)src;
                MMC->SRS23 = (uint32_t)(((uint64_t)(uintptr_t)src) >> MMC_64BIT_UPPER_ADDR_SHIFT);
            
                /* Block length and count SDMA buffer boundary */
                MMC->SRS01 = (blocklen | (blockcount << BLOCK_COUNT_ENABLE_SHIFT) | SRS1_DMA_BUFF_SIZE_512KB);
            
                /* enable interrupts */
                MMC->SRS14 = (SRS14_COMMAND_COMPLETE_SIG_EN |
                                SRS14_TRANSFER_COMPLETE_SIG_EN |
                                SRS14_DMA_INTERRUPT_SIG_EN |
                                SRS14_DATA_TIMEOUT_ERR_SIG_EN);
                PLIC_EnableIRQ(MMC_main_PLIC);
    
                /* Multi Block write */
                if (blockcount > MMC_SET)
                {
                        /* DPS, Data transfer direction - write */
                        srs03_data = (uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_WRITE
                                            | SRS3_MULTI_BLOCK_SEL | SRS3_BLOCK_COUNT_ENABLE
                                            | SRS3_RESPONSE_CHECK_TYPE_R1 | SRS3_RESP_LENGTH_48
                                            | SRS3_CRC_CHECK_EN | SRS3_INDEX_CHECK_EN
                                            | SRS3_DMA_ENABLE);
                        /* check data line busy */
                        do
                        {
                            srs9 = MMC->SRS09;
                        }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT) != MMC_CLEAR));
                        /* multi block transfer */
                        g_mmc_is_multi_blk = MMC_SET;

                        MMC->SRS02 = argument;
                        /* execute command */
                        MMC->SRS03 = (uint32_t)((MMC_CMD_25_WRITE_MULTI_BLOCK << MMC_SRS03_COMMAND_SHIFT) | srs03_data);
                }
                else /* single block write */
                {
                    /* DPS, Data transfer direction - write */
                    srs03_data = (uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_WRITE
                                            | SRS3_BLOCK_COUNT_ENABLE
                                            | SRS3_RESPONSE_CHECK_TYPE_R1
                                            | SRS3_RESP_LENGTH_48
                                            | SRS3_CRC_CHECK_EN
                                            | SRS3_INDEX_CHECK_EN
                                            | SRS3_DMA_ENABLE);
                    do
                    {
                        srs9 = MMC->SRS09;
                    }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT) != MMC_CLEAR));
                    /* single block transfer */
                    g_mmc_is_multi_blk = MMC_CLEAR;
                    /* execute command */
                    MMC->SRS02 = argument;
                    MMC->SRS03 = (uint32_t)((MMC_CMD_24_WRITE_SINGLE_BLOCK << MMC_SRS03_COMMAND_SHIFT) | srs03_data);
                }
            }
                g_mmc_trs_status.state = MSS_MMC_TRANSFER_IN_PROGRESS;
                ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
        }
    }
    else
    {
        ret_status = MSS_MMC_NOT_INITIALISED;
    }
    return ret_status;
}
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_adma2_write
(
    const uint8_t *src,
    uint32_t dest,
    uint32_t size
)
{
    uint32_t blockcount;
    uint32_t argument;
    uint32_t blocklen;
    uint32_t tmp, srs03_data, srs9;
    cif_response_t response_status;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;

    blocklen = BLK_SIZE;
    argument = dest;

    if (g_mmc_init_complete == MMC_SET)
    {
        if (MSS_MMC_TRANSFER_IN_PROGRESS == g_mmc_trs_status.state)
        {
            ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
        }
        else
        {
            /* Size should be divided by 512, not greater than (32MB - 512) */
            if (((size % blocklen) != MMC_CLEAR) || (size > (SIZE_32MB - BLK_SIZE))
                    || (size == MMC_CLEAR) || (src == NULL_POINTER))
            {
                ret_status = MSS_MMC_INVALID_PARAMETER;
            }
            else
            {
                /* disable PCLI interrupt for MMC */
                PLIC_DisableIRQ(MMC_main_PLIC);
    
                /* Calculate block count */
                blockcount = ((size - MMC_SET) / blocklen) + MMC_SET;
                /* ADMA2 table create */
                ret_status = adma2_create_descriptor_table(src, size);
                if (ret_status != MSS_MMC_INVALID_PARAMETER)
                {
                    /* check MMC device is busy */
                    do
                    {
                        response_status = cif_send_cmd(sdcard_RCA << SHIFT_16BIT,
                                            MMC_CMD_13_SEND_STATUS,
                                            MSS_MMC_RESPONSE_R1);
                    } while (DEVICE_BUSY == response_status);
    
                    /* ADMA setup */
                    MMC->SRS22 = (uint32_t)(uintptr_t)adma_descriptor_table;
                    MMC->SRS23 = (uint32_t)(((uint64_t)(uintptr_t)adma_descriptor_table) >> MMC_64BIT_UPPER_ADDR_SHIFT);
    
                    tmp = MMC->SRS10;
                    tmp = (tmp & (~SRS10_DMA_SELECT_MASK));
                    MMC->SRS10 = (tmp | SRS10_DMA_SELECT_ADMA2);
    
                    /* Block length and count */
                    MMC->SRS01 = (blocklen | (blockcount << BLOCK_COUNT_ENABLE_SHIFT));
    
                    /* enable interrupts */
                    MMC->SRS14 = (SRS14_COMMAND_COMPLETE_SIG_EN | SRS14_TRANSFER_COMPLETE_SIG_EN
                                        | SRS14_DATA_TIMEOUT_ERR_SIG_EN | SRS14_ADMA_ERROR_SIG_EN);
                    PLIC_EnableIRQ(MMC_main_PLIC);
    
                    /* Multi Block write */
                    if (blockcount > MMC_SET)
                    {
                            /* DPS, Data transfer direction - write */
                            srs03_data = (uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_WRITE
                                            | SRS3_MULTI_BLOCK_SEL | SRS3_BLOCK_COUNT_ENABLE
                                            | SRS3_RESPONSE_CHECK_TYPE_R1 | SRS3_RESP_LENGTH_48
                                            | SRS3_CRC_CHECK_EN | SRS3_INDEX_CHECK_EN
                                            | SRS3_DMA_ENABLE);
                        /* check data line busy */
                        do
                        {
                            srs9 = MMC->SRS09;
                        }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT) != MMC_CLEAR));
                        /* multi block transfer */
                        g_mmc_is_multi_blk = MMC_SET;
                        MMC->SRS02 = argument;
                        /* execute command */
                        MMC->SRS03 = (uint32_t)((MMC_CMD_25_WRITE_MULTI_BLOCK << MMC_SRS03_COMMAND_SHIFT) | srs03_data);
                    }
                    else /* single block write */
                    {
                        /* DPS, Data transfer direction - write */
                        srs03_data = (uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_WRITE | SRS3_BLOCK_COUNT_ENABLE
                                            | SRS3_RESPONSE_CHECK_TYPE_R1 | SRS3_RESP_LENGTH_48
                                            | SRS3_CRC_CHECK_EN| SRS3_INDEX_CHECK_EN
                                            | SRS3_DMA_ENABLE);
                        do
                        {
                            srs9 = MMC->SRS09;
                        }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT) != MMC_CLEAR));
                        /* single block transfer */
                        g_mmc_is_multi_blk = MMC_CLEAR;
                        /* execute command */
                        MMC->SRS02 = argument;
                        MMC->SRS03 = (uint32_t)((MMC_CMD_24_WRITE_SINGLE_BLOCK << MMC_SRS03_COMMAND_SHIFT) | srs03_data);
                    }
                    g_mmc_trs_status.state = MSS_MMC_TRANSFER_IN_PROGRESS;
                    ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
                }
            }
        }
    }
    else
    {
        ret_status = MSS_MMC_NOT_INITIALISED;
    }
    return ret_status;
}

/*-------------------------------------------------------------------------*//**
 * See "mss_mmc.h" for details of how to use this function.
 */
mss_mmc_status_t MSS_MMC_get_transfer_status(void)
{
    return g_mmc_trs_status.state;
}
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc.h" for details of how to use this function.
 */
void MSS_MMC_set_handler( mss_mmc_handler_t handler)
{
    g_transfer_complete_handler_t = handler;
}
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc.h" for details of how to use this function.
 */
mss_mmc_status_t MSS_MMC_cq_init(void)
{
    uint32_t csd_reg[BLK_SIZE/WORD_SIZE];
    uint8_t  *pcsd_reg;
    uint64_t desc_addr64;
    uint32_t reg;
    uint8_t  hw_cq_mode_en;
    uint8_t  hw_cq_depth;
    uint8_t  hw_cq_support;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    cif_response_t response_status;
    static uint8_t g_cq_task_desc[SIZE_4KB];

    g_mmc_cq_init_complete = MMC_CLEAR;
    
    if (g_mmc_init_complete == MMC_SET)
    {
        /* task descriptor size configuration */
        reg = MMC->CQRS02;
        reg &= ~(uint32_t)CQRS02_TASK_DESCRIPTOR_SIZE_MASK;
        reg |= (uint32_t)CQRS02_TASK_DESCRIPTOR_SIZE_128;
        MMC->CQRS02 = reg;
    
        g_desc_addr = (uint8_t *)g_cq_task_desc;
    
        if (((uintptr_t)g_desc_addr & CQ_TDLBA_ALIGN_MASK) != MMC_CLEAR)
        {
            g_desc_addr += (CQ_TDLBA_ALIGN_MASK + MMC_SET) - ((uintptr_t)g_desc_addr & CQ_TDLBA_ALIGN_MASK);
        }
        /* Task Descriptor List address configuration */
        desc_addr64 = (uintptr_t)g_desc_addr;
        MMC->CQRS08 = (uint32_t)(desc_addr64 & MASK_32BIT);
        MMC->CQRS09 = (uint32_t)((desc_addr64 >> SHIFT_32BIT) & MASK_32BIT);
    
        ret_status = MSS_MMC_single_block_read(READ_SEND_EXT_CSD, csd_reg);
        if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
        {
            pcsd_reg = (uint8_t *)csd_reg;
            /* offsets defined in JESD84-B51 extended CSD */
            hw_cq_mode_en = pcsd_reg[EXT_CSD_CQ_MODE_EN_OFFSET];
            hw_cq_depth = pcsd_reg[EXT_CSD_CQ_DEPTH_OFFSET] + MMC_SET;
            hw_cq_support = pcsd_reg[EXT_CSD_CQ_SUPPORT_OFFSET];
    
            if (hw_cq_support == MMC_SET)
            {
                /* Enable CQ mode in the device */
                response_status = cif_send_cmd(MMC_CQ_ENABLE,
                                            MMC_CMD_6_SWITCH,
                                            MSS_MMC_RESPONSE_R1B);
    
                if ((TRANSFER_IF_SUCCESS == response_status)||(DEVICE_BUSY == response_status))
                {
                    do
                    {
                        response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                                        MMC_CMD_13_SEND_STATUS,
                                                        MSS_MMC_RESPONSE_R1);
                    } while (DEVICE_BUSY == response_status);
                }
    
                if (TRANSFER_IF_SUCCESS == response_status)
                {
                    /* check device is enabled with CQ or not */
                    ret_status = MSS_MMC_single_block_read(READ_SEND_EXT_CSD, csd_reg);
                    if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
                    {
                        pcsd_reg = (uint8_t *)csd_reg;
                        hw_cq_mode_en = pcsd_reg[EXT_CSD_CQ_MODE_EN_OFFSET];
                        if (MMC_SET == hw_cq_mode_en)
                        {
                            /* Enable CQ in Host controller */
                            /* set blk count and idle timer */
                            MMC->CQRS16 = (MMC_SET << SHIFT_16BIT) |(CQ_IDLE_TIME);
                            /* enable signaling command queuing interrupts */
                            reg = (uint32_t)(CQRS05_TASK_CLEARED_STAT_EN
                                                    | CQRS05_RESP_ERR_STAT_EN
                                                    | CQRS05_TASK_COMPLETE_STAT_EN);
                            MMC->CQRS05 = reg;
                            /* clear all interrupts */
                            MMC->CQRS04 = MASK_32BIT;
                            /* set 16-bit RCA value which is send as an argument in SEND_QUEUE_STATUS */
                            MMC->CQRS17 = sdcard_RCA;
                            /* enable command queuing engine in SD host controller */
                            reg = MMC->CQRS02;
                            reg |= (uint32_t)CQRS02_COMMAND_QUEUING_ENABLE;
                            MMC->CQRS02 = reg;
                            /* enable task complete interrupt */
                            reg = (uint32_t)(CQRS06_TASK_COMPLETE_INT_SIG_EN | CQRS06_RESP_ERR_INT_SIG_EN);
                            MMC->CQRS06 = reg;
                            /* check device CQ status */
                            do
                            {
                                response_status = cif_send_cq_direct_command(g_desc_addr,
                                                                sdcard_RCA,
                                                            MMC_CMD_13_SEND_STATUS,
                                                            MSS_MMC_RESPONSE_R1,
                                                            CQ_DCMD_TASK_ID);
                            } while (DEVICE_BUSY == response_status);
    
                            if (TRANSFER_IF_SUCCESS == response_status)
                            {
                                g_mmc_cq_init_complete = MMC_SET;
                                ret_status = MSS_MMC_INIT_SUCCESS;
                            }
                            else
                            {
                                g_mmc_cq_init_complete = MMC_CLEAR;
                                ret_status = MSS_MMC_CQ_INIT_FAILURE;
                            }
                        }
                        else
                        {
                            ret_status = MSS_MMC_CQ_INIT_FAILURE;
                        }
                    }
                }
                else
                {
                    ret_status = MSS_MMC_CQ_INIT_FAILURE;
                }
            }
            else
            {
                ret_status = MSS_MMC_DEVICE_NOT_SUPPORT_CQ;
            }
        }
        else
        {
            ret_status = MSS_MMC_CQ_INIT_FAILURE;
        }
    }
    else
    {
        ret_status = MSS_MMC_CQ_INIT_FAILURE;
    }
    return ret_status;
}
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_cq_write
(
    const uint8_t *src,
    uint32_t dest,
    uint32_t size
)
{
    uint32_t reg;
    uint32_t *dcmdTaskDesc = NULL_POINTER;
    uint32_t flags;
    uint32_t desc_offset;
    volatile uint32_t trans_status_isr;
    uint32_t cmd_response;
    uint32_t blockcount;
    uint32_t argument;
    uint32_t blocklen;
    uint32_t blk_cnt;
    uint8_t task_id;
    uint32_t trans_size;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    cif_response_t response_status;

    blocklen = BLK_SIZE;
    argument = dest;
    trans_size = size;

    if (g_mmc_cq_init_complete == MMC_SET)
    {
        if (MSS_MMC_TRANSFER_IN_PROGRESS == g_mmc_trs_status.state)
        {
            ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
        }
        else
        {
            /* Size should be divided by 512, not greater than (1GB - 16KB) */
            if (((size % blocklen) != MMC_CLEAR) || (size > (SIZE_1GB - (CQ_HOST_NUMBER_OF_TASKS * BLK_SIZE)))
                    || (size == MMC_CLEAR) || (src == NULL_POINTER))
            {
                ret_status = MSS_MMC_INVALID_PARAMETER;
            }
            else
            {
                /* disable PCLI interrupt for MMC */
                PLIC_DisableIRQ(MMC_main_PLIC);

                /* Calculate block count */
                blockcount = ((size - MMC_SET) / blocklen) + MMC_SET;

                /* check MMC device is busy */
                do
                {
                    response_status = cif_send_cq_direct_command(g_desc_addr,
                                                    sdcard_RCA,
                                                    MMC_CMD_13_SEND_STATUS,
                                                    MSS_MMC_RESPONSE_R1,
                                                    CQ_DCMD_TASK_ID);
                } while (DEVICE_BUSY == response_status);

                reg = MMC_CLEAR;

                for(task_id = MMC_CLEAR; ((task_id < CQ_HOST_NUMBER_OF_TASKS) && (blockcount != MMC_CLEAR)); ++task_id)
                {
                    desc_offset = CQ_HOST_NUMBER_OF_TASKS * task_id;
                    dcmdTaskDesc = (uint32_t *)(g_desc_addr + desc_offset);

                    flags = (uint32_t)(CQ_DESC_VALID |  CQ_DESC_END | CQ_DESC_ACT_TASK | CQ_DESC_INT);
                    flags |= CQ_DESC_SET_CONTEXT_ID(MMC_SET) | CQ_DESC_DATA_DIR_WRITE;
#if 0
                    flags |= (uint16_t)CQ_DESC_PRIORITY_HIGH;
                    flags |= (uint16_t)CQ_DESC_QUEUE_BARRIER;
                    flags |= (uint16_t)CQ_DESC_FORCE_PROG;
                    flags |= (uint16_t)CQ_DESC_TAG_REQUEST;
                    flags |= (uint16_t)CQ_DESC_RELIABLE_WRITE;
                    flags |= (uint16_t)CQ_DESC_INT;
#endif
                    if (blockcount > MASK_16BIT)
                    {
                        blk_cnt = MASK_16BIT;
                        blockcount = blockcount - MASK_16BIT;
                    }
                    else
                    {
                        blk_cnt = blockcount;
                        blockcount = MMC_CLEAR;
                    }
                    dcmdTaskDesc[MMC_CLEAR] = flags | (blk_cnt << SHIFT_16BIT);
                    dcmdTaskDesc[BYTES_1] = argument;
                    dcmdTaskDesc[BYTES_2] = MMC_CLEAR;
                    dcmdTaskDesc[BYTES_3] = MMC_CLEAR;

                    if (trans_size > MASK_16BIT)
                    {
                        /* data buffer size is 64KB */
                        size = MMC_CLEAR;
                    }
                    else
                    {
                        size = trans_size;
                    }
                    /* supports only 64KB */
                    dcmdTaskDesc[BYTES_4] = (uint32_t)((CQ_DESC_VALID | CQ_DESC_ACT_TRAN |  CQ_DESC_END) | (size << SHIFT_16BIT));
                    /* Data buffer address in host memory, lower part */
                    dcmdTaskDesc[BYTES_5] = (uint32_t)(uintptr_t)src;
                    /* Data buffer address in host memory, higher part */
                    dcmdTaskDesc[BYTES_6] = (uint32_t)(((uint64_t)(uintptr_t)src) >> MMC_64BIT_UPPER_ADDR_SHIFT);
                    dcmdTaskDesc[BYTES_7] = MMC_CLEAR;

                    if (trans_size > (SIZE_32MB - BLK_SIZE))
                    {
                        trans_size = trans_size - (SIZE_32MB - BLK_SIZE);
                        src = src + (SIZE_32MB - - BLK_SIZE);
                        argument = argument + blockcount;
                    }
                   reg |= (MMC_SET << task_id);
                }

                g_cq_task_id = task_id;
                        /* Enable interrupts */
                MMC->SRS14 = (SRS14_CMD_QUEUING_SIG_EN | SRS14_COMMAND_TIMEOUT_ERR_SIG_EN
                                    | SRS14_DATA_TIMEOUT_ERR_SIG_EN);
                PLIC_EnableIRQ(MMC_main_PLIC);
               /* Set doorbell to start processing descriptors by controller */
                MMC->CQRS10 = reg;

                g_mmc_trs_status.state = MSS_MMC_TRANSFER_IN_PROGRESS;
                ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
            }
        }
    }
    else
    {
        ret_status = MSS_MMC_CQ_NOT_INITIALISED;
    }
    return ret_status;
}
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_cq_read
(
    uint32_t src,
    uint8_t *dest,
    uint32_t size
)
{
    uint32_t reg;
    uint32_t *dcmdTaskDesc = NULL_POINTER;
    uint32_t flags;
    uint32_t desc_offset;
    volatile uint32_t trans_status_isr;
    uint32_t cmd_response;
    uint32_t blockcount;
    uint32_t argument;
    uint32_t blocklen;
    uint32_t blk_cnt;
    uint8_t task_id;
    uint32_t trans_size;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    cif_response_t response_status;

    blocklen = BLK_SIZE;
    argument = src;
    trans_size = size;

    if (g_mmc_cq_init_complete == MMC_SET)
    {
       if (MSS_MMC_TRANSFER_IN_PROGRESS == g_mmc_trs_status.state)
       {
           ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
       }
       else
       {
            /* Size should be divided by 512, not greater than (1GB - 16KB) */
            if (((size % blocklen) != MMC_CLEAR) || (size > (SIZE_1GB - (CQ_HOST_NUMBER_OF_TASKS * BLK_SIZE)))
                    || (size == MMC_CLEAR) || (dest == NULL_POINTER))
            {
                ret_status = MSS_MMC_INVALID_PARAMETER;
            }
            else
            {
                /* disable PCLI interrupt for MMC */
                PLIC_DisableIRQ(MMC_main_PLIC);

                /* Calculate block count */


                blockcount = ((size - MMC_SET) / blocklen) + MMC_SET;
                

                /* check MMC device is busy */
                do
                {
                   response_status = cif_send_cq_direct_command(g_desc_addr,
                                                       sdcard_RCA,
                                                       MMC_CMD_13_SEND_STATUS,
                                                       MSS_MMC_RESPONSE_R1,
                                                       CQ_DCMD_TASK_ID);
                } while (DEVICE_BUSY == response_status);

                reg = MMC_CLEAR;

                for(task_id = MMC_CLEAR; ((task_id < CQ_HOST_NUMBER_OF_TASKS) && (blockcount != MMC_CLEAR)); ++task_id)
                {
                    desc_offset = CQ_HOST_NUMBER_OF_TASKS * task_id;
                    dcmdTaskDesc = (uint32_t *)(g_desc_addr + desc_offset);

                    flags = (uint32_t)(CQ_DESC_VALID |  CQ_DESC_END | CQ_DESC_ACT_TASK | CQ_DESC_INT);
                    flags |= CQ_DESC_SET_CONTEXT_ID(MMC_CLEAR) | CQ_DESC_DATA_DIR_READ;
#if 0
                    flags |= (uint16_t)CQ_DESC_PRIORITY_HIGH;
                    flags |= (uint16_t)CQ_DESC_QUEUE_BARRIER;
                    flags |= (uint16_t)CQ_DESC_FORCE_PROG;
                    flags |= (uint16_t)CQ_DESC_TAG_REQUEST;
                    flags |= (uint16_t)CQ_DESC_RELIABLE_WRITE;
                    flags |= (uint16_t)CQ_DESC_INT;
#endif
                    if (blockcount > MASK_16BIT)
                    {
                        blk_cnt = MASK_16BIT;
                        blockcount = blockcount - MASK_16BIT;
                    }
                    else
                    {
                        blk_cnt = blockcount;
                        blockcount = MMC_CLEAR;
                    }
                    dcmdTaskDesc[MMC_CLEAR] = flags | (blk_cnt << SHIFT_16BIT);
                    dcmdTaskDesc[BYTES_1] = argument;
                    dcmdTaskDesc[BYTES_2] = MMC_CLEAR;
                    dcmdTaskDesc[BYTES_3] = MMC_CLEAR;

                    if (trans_size > MASK_16BIT)
                    {
                        /* data buffer size is 64KB */
                        size = MMC_CLEAR;
                    }
                    else
                    {
                        size = trans_size;
                    }
                    /* supports only 64KB */
                    dcmdTaskDesc[BYTES_4] = (uint32_t)((CQ_DESC_VALID | CQ_DESC_ACT_TRAN |  CQ_DESC_END) | (size << SHIFT_16BIT));
                    /* Data buffer address in host memory, lower part */
                    dcmdTaskDesc[BYTES_5] = (uint32_t)(uintptr_t)dest;
                    /* Data buffer address in host memory, higher part */
                    dcmdTaskDesc[BYTES_6] = (uint32_t)(((uint64_t)(uintptr_t)dest) >> MMC_64BIT_UPPER_ADDR_SHIFT);
                    dcmdTaskDesc[BYTES_7] = MMC_CLEAR;

                    if (trans_size > (SIZE_32MB - BLK_SIZE))
                    {
                        trans_size = trans_size - (SIZE_32MB - BLK_SIZE);
                        src = src + (SIZE_32MB - - BLK_SIZE);
                        argument = argument + blockcount;
                    }
                   reg |= (MMC_SET << task_id);
                }

                g_cq_task_id = task_id;
                        /* Enable interrupts */
                MMC->SRS14 = (SRS14_CMD_QUEUING_SIG_EN | SRS14_COMMAND_TIMEOUT_ERR_SIG_EN
                                    | SRS14_DATA_TIMEOUT_ERR_SIG_EN);
                PLIC_EnableIRQ(MMC_main_PLIC);
               /* Set doorbell to start processing descriptors by controller */
                MMC->CQRS10 = reg;

                g_mmc_trs_status.state = MSS_MMC_TRANSFER_IN_PROGRESS;
                ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
            }
        }
    }
    else
    {
        ret_status = MSS_MMC_CQ_NOT_INITIALISED;
    }
    return ret_status;
}

/******************************************************************************
  MMC ISR
*******************************************************************************/
uint8_t  mmc_main_plic_IRQHandler(void)
{
    uint32_t trans_status_isr;
    uint32_t response_reg, inttoclear;
    uintptr_t address;
    uintptr_t highaddr;
    uint64_t address64;
    static uint8_t transfer_complete  = MMC_CLEAR;

    trans_status_isr = MMC->SRS12;

    /* Error interrupt */
    if ((trans_status_isr & SRS12_ERROR_INTERRUPT) != MMC_CLEAR)
    {
        MMC->SRS12 = trans_status_isr;
        MMC->SRS14 = MMC_CLEAR;
        g_mmc_trs_status.state = MSS_MMC_TRANSFER_FAIL;
        if (g_transfer_complete_handler_t != NULL_POINTER)
        {
            g_transfer_complete_handler_t(trans_status_isr);
        }
    }
    /* Command completion */
    else if ((trans_status_isr & SRS12_COMMAND_COMPLETE) != MMC_CLEAR)
    {
        MMC->SRS12 = SRS12_COMMAND_COMPLETE;
        response_reg = MMC->SRS04;
        if ((response_reg & CARD_STATUS_ALL_ERRORS_MASK) != MMC_CLEAR)
        {
            g_mmc_trs_status.state = MSS_MMC_RESPONSE_ERROR;
        }
        /* stop command success */
        if (transfer_complete == MMC_SET)
        {
            g_mmc_trs_status.state = MSS_MMC_TRANSFER_SUCCESS;
            transfer_complete = MMC_CLEAR;
            if (g_transfer_complete_handler_t != NULL_POINTER)
            {
                g_transfer_complete_handler_t(trans_status_isr);
            }
        }
    }
    /* Transfer Completion */
    else if ((trans_status_isr & SRS12_TRANSFER_COMPLETE) != MMC_CLEAR)
    {
        MMC->SRS12 = trans_status_isr;

        if (g_mmc_is_multi_blk == MMC_CLEAR)
        {
            g_mmc_trs_status.state = MSS_MMC_TRANSFER_SUCCESS;
            transfer_complete = MMC_CLEAR;
            if (g_transfer_complete_handler_t != NULL_POINTER)
            {
                g_transfer_complete_handler_t(trans_status_isr);
            }
        }
        else
        {
            /* multi block transfer */
            transfer_complete = MMC_SET;
            send_mmc_cmd(sdcard_RCA << SHIFT_16BIT, MMC_CMD_12_STOP_TRANSMISSION,
                                    MSS_MMC_RESPONSE_R1, CHECK_IF_CMD_SENT_INT);
        }
    }
    /* DMA interrupt */
    else if ((trans_status_isr & SRS12_DMA_INTERRUPT) != MMC_CLEAR)
    {
        address = MMC->SRS22;
        highaddr = MMC->SRS23;

        address64 = address | ((uint64_t)highaddr << MMC_64BIT_UPPER_ADDR_SHIFT);
        /* Increase address(512kb) and re-write new address in DMA buffer */

        address = (uint32_t)address64;
        highaddr = (uint32_t)(address64 >> MMC_64BIT_UPPER_ADDR_SHIFT);

        MMC->SRS22 = address;
        MMC->SRS23 = highaddr;
        MMC->SRS12 = SRS12_DMA_INTERRUPT;
    }
    else if ((trans_status_isr & SRS12_CMD_QUEUING_INT) != MMC_CLEAR)
    {
        MMC->SRS12 = trans_status_isr;
        inttoclear = MMC->CQRS04;
        MMC->CQRS04 = inttoclear;

        if ((inttoclear & CQRS04_RESP_ERR_INT) != MMC_CLEAR)
        {
           g_mmc_trs_status.state = MSS_MMC_TRANSFER_FAIL;
        }

        if ((inttoclear & CQRS04_TASK_COMPLETE_INT) != MMC_CLEAR)
        {
            inttoclear = MMC->CQRS11;
            /* clear all caught notifications */
            MMC->CQRS11 = inttoclear;
            --g_cq_task_id;
            if (g_cq_task_id == MMC_CLEAR)
            {
                g_mmc_trs_status.state = MSS_MMC_TRANSFER_SUCCESS;
                if (g_transfer_complete_handler_t != NULL_POINTER)
                {
                    g_transfer_complete_handler_t(trans_status_isr);
                }
            }
        }
    }
    else
    {
        MMC->SRS12 = trans_status_isr;
    }
    return MMC_CLEAR;
}

#ifdef MSS_MMC_INTERNAL_APIS
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc_internal_test.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_pause_sdma_write_hpi
(
    const uint8_t *src,
    uint32_t dest,
    uint32_t size
)
{
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    cif_response_t response_status;
    uint32_t command_information;
    uint32_t trans_status_isr, response;
    uint32_t total_sectors = size/BLK_SIZE;
    uint32_t remaining_sectors;
    uint32_t csd_reg[BLK_SIZE/WORD_SIZE];
    uint8_t  *pcsd_reg;
    uint32_t prog_sector_num = MMC_CLEAR;


    g_device_hpi_set = MMC_CLEAR;

    if (MSS_MMC_TRANSFER_IN_PROGRESS == g_mmc_trs_status.state)
    {
        if ((g_device_hpi_suport & DEVICE_HPI_SUPPORT) == DEVICE_HPI_SUPPORT)
        {
            /* Disable PLIC interrupt for MMC */
            PLIC_DisableIRQ(MMC_main_PLIC);
            if ((g_device_hpi_suport & HPI_WITH_CMD12_SUPPORT) != MMC_CLEAR)
            {

                response_status = cif_send_cmd((uint32_t)((sdcard_RCA << RCA_SHIFT_BIT) | DEVICE_HPI_SUPPORT),
                                        MMC_CMD_12_STOP_TRANSMISSION,
                                        MSS_MMC_RESPONSE_R1B);
            }
            else
            {
                response_status = cif_send_cmd((uint32_t)((sdcard_RCA << RCA_SHIFT_BIT) | DEVICE_HPI_SUPPORT),
                                        MMC_CMD_13_SEND_STATUS,
                                        MSS_MMC_RESPONSE_R1);
                do
                {
                    response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                            MMC_CMD_13_SEND_STATUS,
                                            MSS_MMC_RESPONSE_R1);
                }while (DEVICE_BUSY == response_status);

                response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                        MMC_CMD_12_STOP_TRANSMISSION,
                                        MSS_MMC_RESPONSE_R1B);
            }

            do
            {
                command_information = (uint32_t)(SRS3_ABORT_CMD | SRS3_RESPONSE_CHECK_TYPE_R1 | SRS3_RESP_LENGTH_48
                              | SRS3_CRC_CHECK_EN | SRS3_INDEX_CHECK_EN);
                MMC->SRS02 = (sdcard_RCA << RCA_SHIFT_BIT) | DEVICE_HPI_SUPPORT;
                MMC->SRS03 = (uint32_t)((MMC_CMD_13_SEND_STATUS << CMD_SHIFT) | command_information);

                do
                {
                    trans_status_isr = MMC->SRS12;
                }while (((SRS12_COMMAND_COMPLETE | SRS12_ERROR_INTERRUPT) & trans_status_isr) == MMC_CLEAR);

                if (SRS12_COMMAND_COMPLETE == (trans_status_isr & SRS12_COMMAND_COMPLETE))
                {
                    /* If the response is an R1/B response */
                    response = MMC->SRS04;
                    if ((CARD_STATUS_ALL_ERRORS_MASK & response) == MMC_CLEAR) /* no error */
                    {
                        if ((CARD_STATUS_READY_FOR_DATA & response) != MMC_CLEAR)
                        {
                            response_status = TRANSFER_IF_SUCCESS;
                        }
                        else
                        {
                            response_status = DEVICE_BUSY;
                        }
                    }
                    else if (SRS12_ERROR_INTERRUPT == (SRS12_ERROR_INTERRUPT & trans_status_isr))
                    {
                        response_status = TRANSFER_IF_FAIL;
                    }
                    else
                    {
                        response_status = TRANSFER_IF_FAIL;
                    }
                }
            }while (response_status == DEVICE_BUSY);

            if (TRANSFER_IF_SUCCESS == response_status)
            {
                ret_status = MSS_MMC_single_block_read(READ_SEND_EXT_CSD, csd_reg);
                if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
                {
                    pcsd_reg = (uint8_t *)csd_reg;
                    prog_sector_num = (uint32_t)(pcsd_reg[245] << SHIFT_24BIT);
                    prog_sector_num |= (uint32_t)(pcsd_reg[244] << SHIFT_16BIT);
                    prog_sector_num |= (uint32_t)(pcsd_reg[243] << SHIFT_8BIT);
                    prog_sector_num |= (uint32_t)pcsd_reg[242];

                    remaining_sectors = total_sectors - prog_sector_num;

                    g_trans_remain_size = remaining_sectors * BLK_SIZE;
                    g_trans_dest_addr = (dest + total_sectors) - remaining_sectors;
                    g_trans_src_addr = (uint8_t *)(src + (prog_sector_num * BLK_SIZE));

                    g_device_hpi_set = MMC_SET;
                }
                else
                {
                    ret_status = MSS_MMC_TRANSFER_FAIL;
                }
            }
            else
            {
                ret_status = MSS_MMC_TRANSFER_FAIL;
            }
        }
        else
        {
            ret_status = MSS_MMC_DEVICE_NOT_SUPPORT_HPI;
        }
    }
    else
    {
        ret_status = g_mmc_trs_status.state;
    }
    return (ret_status);
}
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc_internal_test.h" for details of how to use this function.
 */
mss_mmc_status_t MSS_MMC_resume_sdma_write_hpi(void)
{
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;

    if (g_device_hpi_set == MMC_SET)
    {
        g_device_hpi_set = MMC_CLEAR;
        if (g_trans_remain_size != MMC_CLEAR)
        {
            g_mmc_trs_status.state = MSS_MMC_INIT_SUCCESS;
            ret_status = MSS_MMC_sdma_write(g_trans_src_addr, g_trans_dest_addr, g_trans_remain_size);
        }
        else
        {
            ret_status = MSS_MMC_TRANSFER_SUCCESS;
        }
    }
    else
    {
        ret_status = MSS_MMC_DEVICE_IS_NOT_IN_HPI_MODE;
    }
    return ret_status;
}
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc_internal_test.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_packed_write
(
    const uint8_t *src,
    uint32_t dest,
    uint32_t size
)
{
    uint32_t blockcount;
    uint32_t argument;
    uint32_t blocklen;
    uint32_t tmp, srs03_data, srs9;
    cif_response_t response_status;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;

    blocklen = BLK_SIZE;
    argument = dest;

    if (g_mmc_init_complete == MMC_SET)
    {
        if (MSS_MMC_TRANSFER_IN_PROGRESS == g_mmc_trs_status.state)
        {
            ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
        }
        else
        {
            /* Size should be divided by 512, not greater than (32MB - 512) */
            if (((size % blocklen) != MMC_CLEAR) || (size > (SIZE_32MB - BLK_SIZE))
                    || (size == MMC_CLEAR) || (src == NULL_POINTER))
            {
                ret_status = MSS_MMC_INVALID_PARAMETER;
            }
            else
            {
                /* disable PCLI interrupt for MMC */
                PLIC_DisableIRQ(MMC_main_PLIC);

                /* Calculate block count */
                blockcount = ((size - MMC_SET) / blocklen) + MMC_SET;
                /* Multi Block write */
                if (blockcount > MMC_SET)
                {
                    tmp = MMC->SRS10;
                    tmp = (tmp & (~SRS10_DMA_SELECT_MASK));
                    MMC->SRS10 = (tmp | SRS10_DMA_SELECT_SDMA);

                    /* SDMA setup */
                    MMC->SRS22 = (uint32_t)(uintptr_t)src;
                    MMC->SRS23 = (uint32_t)(((uint64_t)(uintptr_t)src) >> MMC_64BIT_UPPER_ADDR_SHIFT);

                    /* DPS, Data transfer direction - write */
                    srs03_data = (uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_WRITE
                                        | SRS3_MULTI_BLOCK_SEL | SRS3_BLOCK_COUNT_ENABLE
                                        | SRS3_RESPONSE_CHECK_TYPE_R1 | SRS3_RESP_LENGTH_48
                                        | SRS3_CRC_CHECK_EN | SRS3_INDEX_CHECK_EN
                                        | SRS3_DMA_ENABLE);
                    /* Block length and count SDMA buffer boundary */
                    MMC->SRS01 = (blocklen | (blockcount << BLOCK_COUNT_ENABLE_SHIFT) | SRS1_DMA_BUFF_SIZE_512KB);
                    /* multi block transfer */
                    g_mmc_is_multi_blk = MMC_CLEAR;
                    /* Set Block count and packed command in device using CMD23 */
                    response_status = cif_send_cmd(blockcount | (MMC_SET << SHIFT_30BIT),
                                                MMC_CMD_23_SET_BLOCK_COUNT,
                                                MSS_MMC_RESPONSE_R1);
                    /* enable interrupts */
                    MMC->SRS14 = (SRS14_COMMAND_COMPLETE_SIG_EN | SRS14_TRANSFER_COMPLETE_SIG_EN
                                    | SRS14_DMA_INTERRUPT_SIG_EN | SRS14_DATA_TIMEOUT_ERR_SIG_EN);
                    PLIC_EnableIRQ(MMC_main_PLIC);

                    /* check data line busy */
                    do
                    {
                        srs9 = MMC->SRS09;
                    }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT) != MMC_CLEAR));

                    MMC->SRS02 = argument;
                    /* execute command */
                    MMC->SRS03 = (uint32_t)((MMC_CMD_25_WRITE_MULTI_BLOCK << MMC_SRS03_COMMAND_SHIFT) | srs03_data);
                }
                else
                {
                    g_mmc_trs_status.state = MSS_MMC_TRANSFER_FAIL;
                    ret_status = MSS_MMC_DATA_SIZE_IS_NOT_MULTI_BLOCK;
                }

            }
            g_mmc_trs_status.state = MSS_MMC_TRANSFER_IN_PROGRESS;
            ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
        }
    }
    else
    {
        ret_status = MSS_MMC_NOT_INITIALISED;
    }
    return ret_status;
}
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc_internal_test.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_packed_read
(
    uint32_t src,
    uint8_t *dest,
    uint32_t *packed_header,
    uint32_t size
)
{
    uint32_t blockcount;
    uint32_t argument;
    uint32_t blocklen;
    uint32_t tmp, srs03_data, srs9;
    cif_response_t response_status;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    uint16_t word_cnt = MMC_CLEAR;
    uint8_t blk_tran_err;
    uint8_t blk_write;

    blocklen = BLK_SIZE;
    argument = src;

    if (g_mmc_init_complete == MMC_SET)
    {
        if (MSS_MMC_TRANSFER_IN_PROGRESS == g_mmc_trs_status.state)
        {
            ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
        }
        else
        {
            /* Size should be divided by 512, not greater than (32MB - 512) */
            if (((size % blocklen) != MMC_CLEAR) || (size > (SIZE_32MB - BLK_SIZE))
                    || (size == MMC_CLEAR) || (dest == NULL_POINTER)
                    || (packed_header == NULL_POINTER))
            {
                ret_status = MSS_MMC_INVALID_PARAMETER;
            }
            else
            {
                /* Disable PCLI interrupt for MMC */
                PLIC_DisableIRQ(MMC_main_PLIC);
                /* Calculate block count */
                blockcount = ((size - MMC_SET) / blocklen) + MMC_SET;

                if (blockcount > MMC_SET)
                {
                    /* Block length and count*/
                    MMC->SRS01 = (BLK_SIZE | (MMC_SET << BLOCK_COUNT_ENABLE_SHIFT));

                    /* DPS, Data transfer direction - read */
                    srs03_data = (uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_WRITE |SRS3_BLOCK_COUNT_ENABLE
                                        | SRS3_RESP_ERR_CHECK_EN | SRS3_RESP_INTER_DISABLE
                                        | SRS3_RESPONSE_CHECK_TYPE_R1 | SRS3_RESP_LENGTH_48
                                        | SRS3_CRC_CHECK_EN | SRS3_INDEX_CHECK_EN);

                    /* Set Block count with packed in device using CMD23 */
                    response_status = cif_send_cmd(MMC_SET | (MMC_SET << SHIFT_30BIT),
                                                MMC_CMD_23_SET_BLOCK_COUNT,
                                                MSS_MMC_RESPONSE_R1);

                    /* Check cmd and data line busy */
                    do
                    {
                        srs9 = MMC->SRS09;
                    }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT)) != MMC_CLEAR);

                    /* Command argument */
                    MMC->SRS02 = argument;
                    /* execute command */
                    MMC->SRS03 = (uint32_t)((MMC_CMD_25_WRITE_MULTI_BLOCK << MMC_SRS03_COMMAND_SHIFT) | srs03_data);

                    do
                    {
                        blk_write = MMC->SRS12;
                    } while (MMC_CLEAR == (blk_write & SRS12_BUFFER_WRITE_READY));
                    /* Load the block of data into the
                    * buffer, one byte at a time through the Buffer Data Register.
                    */
                    while ((BLK_SIZE/WORD_SIZE) > word_cnt)
                    {
                        MMC->SRS08 = packed_header[word_cnt];
                        ++word_cnt;
                    }
                    /* Check is block write complete */
                    do
                    {
                        blk_write = MMC->SRS12;
                    }while (MMC_CLEAR == (blk_write & SRS12_TRANSFER_COMPLETE));

                    blk_tran_err = MMC->SRS12;
                    /* Abort if any errors*/
                    if ((SRS12_ERROR_STATUS_MASK & blk_tran_err) == MMC_CLEAR)
                    {
                        ret_status = MSS_MMC_TRANSFER_SUCCESS;
                    }
                    else
                    {
                        ret_status = MSS_MMC_ERR_INTERRUPT;
                    }
                    MMC->SRS12 = blk_tran_err;

                    if (ret_status == MSS_MMC_TRANSFER_SUCCESS)
                    {
                        tmp = MMC->SRS10;
                        tmp = (tmp & (~SRS10_DMA_SELECT_MASK));
                        MMC->SRS10 = (tmp | SRS10_DMA_SELECT_SDMA);

                        /* SDMA setup */
                        MMC->SRS22 = (uint32_t)(uintptr_t)dest;
                        MMC->SRS23 = (uint32_t)(((uint64_t)(uintptr_t)dest) >> MMC_64BIT_UPPER_ADDR_SHIFT);
                        /* Block length and count SDMA buffer boundary */
                        MMC->SRS01 = (blocklen | (blockcount << BLOCK_COUNT_ENABLE_SHIFT) | SRS1_DMA_BUFF_SIZE_512KB);
                        /* DPS, Data transfer direction - read */
                        srs03_data = (uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_READ
                                            | SRS3_MULTI_BLOCK_SEL | SRS3_BLOCK_COUNT_ENABLE
                                            | SRS3_RESPONSE_CHECK_TYPE_R1 | SRS3_RESP_LENGTH_48
                                            | SRS3_CRC_CHECK_EN | SRS3_INDEX_CHECK_EN
                                            | SRS3_DMA_ENABLE);
                        /* multi block transfer */
                        g_mmc_is_multi_blk = MMC_CLEAR;

                        /* Set Block count with packed in device using CMD23 */
                        response_status = cif_send_cmd(blockcount | (MMC_SET << SHIFT_30BIT),
                                                MMC_CMD_23_SET_BLOCK_COUNT,
                                                MSS_MMC_RESPONSE_R1);
                        /* Enable interrupts */
                        MMC->SRS14 = (SRS14_COMMAND_COMPLETE_SIG_EN | SRS14_TRANSFER_COMPLETE_SIG_EN
                                            | SRS14_DMA_INTERRUPT_SIG_EN | SRS14_DATA_TIMEOUT_ERR_SIG_EN);
                        PLIC_EnableIRQ(MMC_main_PLIC);
                        /* Multi Block read */

                        /* Check cmd and data line busy */
                        do
                        {
                            srs9 = MMC->SRS09;
                        }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT) != MMC_CLEAR));

                        /* Command argument */
                        MMC->SRS02 = argument;
                        /* execute command */
                        MMC->SRS03 = (uint32_t)((MMC_CMD_18_READ_MULTIPLE_BLOCK << MMC_SRS03_COMMAND_SHIFT) | srs03_data);

                        g_mmc_trs_status.state = MSS_MMC_TRANSFER_IN_PROGRESS;
                        ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
                    }
                    else
                    {
                        g_mmc_trs_status.state = MSS_MMC_TRANSFER_FAIL;
                    }
                }
                else
                {
                    g_mmc_trs_status.state = MSS_MMC_TRANSFER_FAIL;
                    ret_status = MSS_MMC_DATA_SIZE_IS_NOT_MULTI_BLOCK;
                }
            }
        }
    }
    else
    {
        ret_status = MSS_MMC_NOT_INITIALISED;
    }
    return ret_status;
}

/*-------------------------------------------------------------------------*//**
 * See "mss_mmc_internal_test.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_cq_single_task_write
(
    const uint8_t *src,
    uint32_t dest,
    uint8_t task_id,
    uint32_t size
)
{
    uint32_t reg;
    uint32_t *dcmdTaskDesc = NULL_POINTER;
    uint32_t flags;
    uint32_t desc_offset;
    volatile uint32_t trans_status_isr;
    uint32_t cmd_response;
    uint32_t blockcount;
    uint32_t argument;
    uint32_t blocklen;

    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    cif_response_t response_status = TRANSFER_IF_FAIL;

    desc_offset = CQ_HOST_NUMBER_OF_TASKS * task_id;

    dcmdTaskDesc = (uint32_t *)(g_desc_addr + desc_offset);
    g_cq_task_id = MMC_CLEAR;
    blocklen = BLK_SIZE;
    argument = dest;

    if(g_mmc_cq_init_complete == MMC_SET)
    {
        if(MSS_MMC_TRANSFER_IN_PROGRESS == g_mmc_trs_status.state)
        {
            ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
        }
        else
        {
            /* Size should be divided by 512, not greater than (32MB - 512) */
            if (((size % blocklen) != MMC_CLEAR) || (size > (SIZE_32MB - BLK_SIZE))
                    || (size == MMC_CLEAR) || (src == NULL_POINTER))
            {
                ret_status = MSS_MMC_INVALID_PARAMETER;
            }
            else
            {
                /* disable PCLI interrupt for MMC */
                PLIC_DisableIRQ(MMC_main_PLIC);

                /* Calculate block count */
                blockcount = ((size - MMC_SET) / blocklen) + MMC_SET;

                /* check MMC device is busy */
                do
                {
                    response_status = cif_send_cq_direct_command(g_desc_addr, sdcard_RCA,
                     MMC_CMD_13_SEND_STATUS, MSS_MMC_RESPONSE_R1, CQ_DCMD_TASK_ID);
                } while(DEVICE_BUSY == response_status);


                flags = (uint32_t)(CQ_DESC_VALID |  CQ_DESC_END | CQ_DESC_ACT_TASK | CQ_DESC_INT);
                flags |= CQ_DESC_SET_CONTEXT_ID(MMC_SET) | CQ_DESC_DATA_DIR_WRITE;
#if 0
                flags |= (uint16_t)CQ_DESC_PRIORITY_HIGH;
                flags |= (uint16_t)CQ_DESC_QUEUE_BARRIER;
                flags |= (uint16_t)CQ_DESC_FORCE_PROG;
                flags |= (uint16_t)CQ_DESC_TAG_REQUEST;
                flags |= (uint16_t)CQ_DESC_RELIABLE_WRITE;
                flags |= (uint16_t)CQ_DESC_INT;
#endif
                dcmdTaskDesc[MMC_CLEAR] = flags | (blockcount << SHIFT_16BIT);
                dcmdTaskDesc[BYTES_1] = argument;
                dcmdTaskDesc[BYTES_2] = MMC_CLEAR;
                dcmdTaskDesc[BYTES_3] = MMC_CLEAR;

                /* supports only 64KB */
                dcmdTaskDesc[BYTES_4] = (uint32_t)((CQ_DESC_VALID | CQ_DESC_ACT_TRAN |  CQ_DESC_END) | (size << SHIFT_16BIT));
                /* Data buffer address in host memory, lower part */
                dcmdTaskDesc[BYTES_5] = (uint32_t)(uintptr_t)src;
                /* Data buffer address in host memory, higher part */
                dcmdTaskDesc[BYTES_6] = (uint32_t)(((uint64_t)(uintptr_t)src) >> MMC_64BIT_UPPER_ADDR_SHIFT);
                dcmdTaskDesc[BYTES_7] = MMC_CLEAR;

                g_cq_task_id = MMC_SET;
                /* Enable interrupts */
                MMC->SRS14 = (SRS14_CMD_QUEUING_SIG_EN | SRS14_COMMAND_TIMEOUT_ERR_SIG_EN
                                     | SRS14_DATA_TIMEOUT_ERR_SIG_EN);
                PLIC_EnableIRQ(MMC_main_PLIC);
                /* set doorbell to start processing descriptors by controller */
                reg = MMC_SET << task_id;
                MMC->CQRS10 = reg;

                g_mmc_trs_status.state = MSS_MMC_TRANSFER_IN_PROGRESS;
                ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
            }
        }
    }
    else
    {
        ret_status = MSS_MMC_CQ_NOT_INITIALISED;
    }
    return ret_status;
}
/*-------------------------------------------------------------------------*//**
 * See "mss_mmc_internal_test.h" for details of how to use this function.
 */
mss_mmc_status_t
MSS_MMC_cq_single_task_read
(
    uint32_t src,
    uint8_t *dest,
    uint8_t task_id,
    uint32_t size
)
{
    uint32_t reg;
    uint32_t *dcmdTaskDesc = NULL_POINTER;
    uint32_t flags;
    uint32_t desc_offset;
    volatile uint32_t trans_status_isr;
    uint32_t cmd_response;
    uint32_t blockcount;
    uint32_t argument;
    uint32_t blocklen;

    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    cif_response_t response_status = TRANSFER_IF_FAIL;

    desc_offset = CQ_HOST_NUMBER_OF_TASKS * task_id;

    dcmdTaskDesc = (uint32_t *)(g_desc_addr + desc_offset);
    g_cq_task_id = MMC_CLEAR;
    blocklen = BLK_SIZE;
    argument = src;

    if(g_mmc_cq_init_complete == MMC_SET)
    {
       if(MSS_MMC_TRANSFER_IN_PROGRESS == g_mmc_trs_status.state)
       {
           ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
       }
       else
       {
            /* Size should be divided by 512, not greater than (32MB - 512) */
            if (((size % blocklen) != MMC_CLEAR) || (size > (SIZE_32MB - BLK_SIZE))
                    || (size == MMC_CLEAR) || (dest == NULL_POINTER))
            {
                ret_status = MSS_MMC_INVALID_PARAMETER;
            }
            else
            {
                /* disable PCLI interrupt for MMC */
                PLIC_DisableIRQ(MMC_main_PLIC);

                /* Calculate block count */
                blockcount = ((size - MMC_SET) / blocklen) + MMC_SET;
                /* check MMC device is busy */
                do
                {
                   response_status = cif_send_cq_direct_command(g_desc_addr, sdcard_RCA,
                    MMC_CMD_13_SEND_STATUS, MSS_MMC_RESPONSE_R1, CQ_DCMD_TASK_ID);
                } while(DEVICE_BUSY == response_status);

                flags = (uint32_t)(CQ_DESC_VALID |  CQ_DESC_END | CQ_DESC_ACT_TASK | CQ_DESC_INT);
                flags |= CQ_DESC_SET_CONTEXT_ID(MMC_CLEAR) | CQ_DESC_DATA_DIR_READ;
#if 0
                flags |= (uint16_t)CQ_DESC_PRIORITY_HIGH;
                flags |= (uint16_t)CQ_DESC_QUEUE_BARRIER;
                flags |= (uint16_t)CQ_DESC_FORCE_PROG;
                flags |= (uint16_t)CQ_DESC_TAG_REQUEST;
                flags |= (uint16_t)CQ_DESC_RELIABLE_WRITE;
                flags |= (uint16_t)CQ_DESC_INT;
#endif

                dcmdTaskDesc[MMC_CLEAR] = flags | (blockcount << SHIFT_16BIT);
                dcmdTaskDesc[BYTES_1] = argument;
                dcmdTaskDesc[BYTES_2] = MMC_CLEAR;
                dcmdTaskDesc[BYTES_3] = MMC_CLEAR;

                /* supports only 64KB */
                dcmdTaskDesc[BYTES_4] = (uint32_t)((CQ_DESC_VALID | CQ_DESC_ACT_TRAN |  CQ_DESC_END) | (size << SHIFT_16BIT));
                /* Data buffer address in host memory, lower part */
                dcmdTaskDesc[BYTES_5] = (uint32_t)(uintptr_t)dest;
                /* Data buffer address in host memory, higher part */
                dcmdTaskDesc[BYTES_6] = (uint32_t)(((uint64_t)(uintptr_t)dest) >> MMC_64BIT_UPPER_ADDR_SHIFT);
                dcmdTaskDesc[BYTES_7] = MMC_CLEAR;

                g_cq_task_id = MMC_SET;
                /* Enable interrupts */
                MMC->SRS14 = (SRS14_CMD_QUEUING_SIG_EN | SRS14_COMMAND_TIMEOUT_ERR_SIG_EN
                                     | SRS14_DATA_TIMEOUT_ERR_SIG_EN);
                PLIC_EnableIRQ(MMC_main_PLIC);

                /* set doorbell to start processing descriptors by controller */
                reg = MMC_SET << task_id;
                MMC->CQRS10 = reg;
                g_mmc_trs_status.state = MSS_MMC_TRANSFER_IN_PROGRESS;
                ret_status = MSS_MMC_TRANSFER_IN_PROGRESS;
            }
        }
    }
    else
    {
        ret_status = MSS_MMC_CQ_NOT_INITIALISED;
    }
    return ret_status;
}

#endif /* MSS_MMC_INTERNAL_APIS */
/*******************************************************************************
****************************** Private Functions *******************************
*******************************************************************************/
static void phy_training_mmc(uint8_t delay_type, uint32_t clk_rate)
{

    uint32_t max_delay;
    uint16_t i;
    uint8_t status;
    uint8_t pos, length, currLength;
    uint8_t new_delay;
    uint8_t rx_buff[BLK_SIZE];
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;

    max_delay = (MSS_MMC_CLOCK_200MHZ /clk_rate) * BYTES_2;

    pos = length = currLength = MMC_CLEAR;
    /* Reset Data and cmd line */
    MMC->SRS11 |= MMC_RESET_DATA_CMD_LINE;
    for (i = MMC_CLEAR; i < max_delay; i++)
    {
        phy_write_set(delay_type, i);

        ret_status = read_tune_block((uint32_t *)rx_buff, BLK_SIZE, MMC_CMD_17_READ_SINGLE_BLOCK);
        if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
        {
            currLength++;
            if (currLength > length)
            {
                pos = i - length;
                length++;
                /* Reset Data and cmd line */
                 MMC->SRS11 |= MMC_RESET_DATA_CMD_LINE;
            }
        }
        else
        {
            /* Reset Data and cmd line */
            MMC->SRS11 |= MMC_RESET_DATA_CMD_LINE;
            currLength = MMC_CLEAR;
        }
    }
    new_delay = (pos + length) / BYTES_2;
    phy_write_set(delay_type, new_delay);

    ret_status = read_tune_block((uint32_t *)rx_buff, BLK_SIZE, MMC_CMD_17_READ_SINGLE_BLOCK);
    /* Reset Data and cmd line */
    MMC->SRS11 |= MMC_RESET_DATA_CMD_LINE;
}
/******************************************************************************/
static void phy_write_set(uint8_t delay_type, uint8_t delay_value)
{
    uint8_t phyaddr;
    uint32_t reg, phycfg;

    /* Phy delay set for eMMC modes */
    get_phy_addr(delay_type, &phyaddr);

    do
    {
        reg = MMC->HRS04;
    }while ((reg & HRS_PHY_ACKNOWLEDGE_REQUEST) != MMC_CLEAR);

    phycfg = ((uint32_t)phyaddr | (delay_value << SHIFT_8BIT));

    /* set data and address */
    MMC->HRS04 = phycfg;
    /* send write request */
    MMC->HRS04 |= (uint32_t)HRS_PHY_WRITE_REQUEST;
    /* wait for acknowledge */
    do
    {
        reg = MMC->HRS04;
    }while ((reg & HRS_PHY_ACKNOWLEDGE_REQUEST) == MMC_CLEAR);

    phycfg &= ~(uint32_t)HRS_PHY_WRITE_REQUEST;
    /* clear write request */
    MMC->HRS04 = phycfg;
    MMC->HRS04 = MMC_CLEAR;
}
/******************************************************************************/
static void get_phy_addr(MSS_MMC_phydelay phydelaytype, uint8_t *phySetAddr)
{
    uint8_t i;

    static const struct phydelayaddresses phydelayaddr[] = {
        { UIS_ADDR_DEFAULT_SPEED, MSS_MMC_PHY_DELAY_INPUT_DEFAULT_SPEED},
        { UIS_ADDR_HIGH_SPEED, MSS_MMC_PHY_DELAY_INPUT_HIGH_SPEED},
        { UIS_ADDR_UHSI_SDR12, MSS_MMC_PHY_DELAY_INPUT_SDR12},
        { UIS_ADDR_UHSI_SDR25, MSS_MMC_PHY_DELAY_INPUT_SDR25},
        { UIS_ADDR_UHSI_SDR50, MSS_MMC_PHY_DELAY_INPUT_SDR50},
        { UIS_ADDR_UHSI_DDR50, MSS_MMC_PHY_DELAY_INPUT_DDR50},
        { UIS_ADDR_MMC_LEGACY, MSS_MMC_PHY_DELAY_INPUT_MMC_LEGACY},
        { UIS_ADDR_MMC_SDR, MSS_MMC_PHY_DELAY_INPUT_MMC_SDR},
        { UIS_ADDR_MMC_DDR, MSS_MMC_PHY_DELAY_INPUT_MMC_DDR},
        { UIS_ADDR_SDCLK, MSS_MMC_PHY_DELAY_DLL_SDCLK},
        { UIS_ADDR_HS_SDCLK, MSS_MMC_PHY_DELAY_DLL_HS_SDCLK},
        { UIS_ADDR_DAT_STROBE, MSS_MMC_PHY_DELAY_DLL_DAT_STROBE},
    };

    for (i = MMC_CLEAR; i < (sizeof(phydelayaddr) / sizeof(phydelayaddr[MMC_CLEAR])); i++)
    {
        if (phydelayaddr[i].phydelaytype == phydelaytype)
        {
            *phySetAddr = phydelayaddr[i].address;
            break;
        }
    }
}
/******************************************************************************/
static mss_mmc_status_t adma2_create_descriptor_table
(
        const uint8_t *data_src,
        uint32_t data_sz
)
{
    uint32_t datasize = data_sz;
    uint32_t i = MMC_CLEAR;
    uint32_t j = MMC_CLEAR;
    uintptr_t buf_address = (uintptr_t)data_src;
    uint32_t offset = MMC_CLEAR;
    mss_mmc_status_t status = MSS_MMC_NO_ERROR;

    if (datasize >= SIZE_64KB)
    {
        uint32_t current_subsize = SIZE_64KB;
        uint32_t size = datasize;

        while (size > MMC_CLEAR)
        {
            if (i >= SDIO_CFG_SDIO_BUFFERS_COUNT)
            {
                /* There is not enough buffer count to create descriptors.
                 You need to increase SDIO_CFG_SDIO_BUFFERS_COUNT parameter */
                status = MSS_MMC_INVALID_PARAMETER;
                break;
            }

            if (size < current_subsize)
            {
                current_subsize = size;
            }
            adma_descriptor_table[j++] = (ADMA2_DESCRIPTOR_TYPE_TRAN
                                                | ADMA2_DESCRIPTOR_VAL | ADMA2_DESCRIPTOR_INT
                                                | ((current_subsize & MASK_16BIT) << SHIFT_16BIT));
            adma_descriptor_table[j++] = ((uint32_t)buf_address & MASK_32BIT);
            adma_descriptor_table[j++] = (uint32_t)((uint64_t)buf_address >> MMC_64BIT_UPPER_ADDR_SHIFT);
            adma_descriptor_table[j] = MMC_CLEAR;
            j++;

            size -= current_subsize;
            buf_address += current_subsize;
            i++;
        }
    }
    else
    {
        adma_descriptor_table[j++] = (ADMA2_DESCRIPTOR_TYPE_TRAN
                                    | ADMA2_DESCRIPTOR_VAL | ADMA2_DESCRIPTOR_INT
                                    | ((datasize & MASK_16BIT) << SHIFT_16BIT));
        adma_descriptor_table[j++] = ((uint32_t)buf_address & MASK_32BIT);
        adma_descriptor_table[j++] = (uint32_t)((uint64_t)buf_address >> MMC_64BIT_UPPER_ADDR_SHIFT);
        adma_descriptor_table[j] = MMC_CLEAR;
        i = MMC_SET;
    }
    /* last descriptor finishes transmission */
    offset  = (i * WORD_SIZE) - WORD_SIZE;
    adma_descriptor_table[offset] |= ADMA2_DESCRIPTOR_END;

    return (status);
}
/******************************************************************************/
static mss_mmc_status_t mmccard_oper_config(const mss_mmc_cfg_t * cfg)
{
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    cif_response_t response_status;
    uint32_t response_reg;
    uint32_t power_up_status;
    uint32_t access_mode;
    uint32_t csd_max_sector_lwr;
    uint32_t max_sector_len;
    uint16_t sector_size;
    uint32_t oper_cond = MMC_CLEAR;

    response_status = cif_send_cmd(STUFF_BITS, MMC_CMD_1_SEND_OP_COND,
                                                MSS_MMC_RESPONSE_R3);
    if (TRANSFER_IF_SUCCESS == response_status)
    {
        response_reg = MMC->SRS04;
        if ((cfg->bus_speed_mode == MSS_MMC_MODE_HS400) ||(cfg->bus_speed_mode == MSS_MMC_MODE_HS200))
        {
            if ((response_reg & MMC_DEVICE_LOW_POWER) == MMC_DEVICE_LOW_POWER)
            {
                oper_cond = MMC_DEVICE_LOW_VOLT_SET;
            }
            else
            {
                ret_status = MSS_MMC_DEVICE_NOT_SUPPORT_LOW_POWER;
            }
        }
        else
        {
            oper_cond = MMC_DEVICE_3_3V_VOLT_SET;
        }
    }

    do
    {
        response_status = cif_send_cmd(oper_cond,
                                       MMC_CMD_1_SEND_OP_COND,
                                       MSS_MMC_RESPONSE_R3);
        response_reg = MMC->SRS04;
        power_up_status = BUSY_BIT_MASK & response_reg;

    } while ((BUSY_BIT_MASK != power_up_status) && (TRANSFER_IF_FAIL != response_status));

    if (TRANSFER_IF_SUCCESS == response_status)
    {
        mmc_delay(DELAY_COUNT);

        response_reg = MMC->SRS04;;
        /* If access_mode = 2 then sector address, if access = 0 then byte mode */
        access_mode = ((SECTOR_ACCESS_MODE_MASK & response_reg) >> SECTOR_SHIFT);

        /* read CID register from device */
        response_status = cif_send_cmd(STUFF_BITS, MMC_CMD_2_ALL_SEND_CID,
                                                    MSS_MMC_RESPONSE_R2);

        do
        {
             /* Assign a RCA to the device */
            response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                   MMC_CMD_3_SET_RELATIVE_ADDR,
                                   MSS_MMC_RESPONSE_R1);
        }while (TRANSFER_IF_SUCCESS != response_status);

        if (TRANSFER_IF_SUCCESS == response_status)
        {
            /* Configure the data transfer */


            /* Read CSD register from device */
            response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                            MMC_CMD_9_SEND_CSD,
                                            MSS_MMC_RESPONSE_R2);
            /* Sector size from "Max. write data block length"  of CSD[25:22]*/
            csd_max_sector_lwr  = MMC->SRS04 & SECT_SIZE_CSD_MASK;
            max_sector_len = csd_max_sector_lwr >> SECT_SIZE_CSD_SHIFT;
            sector_size = (uint16_t)((uint32_t)MMC_SET << max_sector_len);

            /* Select device */
            response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                            MMC_CMD_7_SELECT_DESELECT_CARD,
                                            MSS_MMC_RESPONSE_R1);

            if (TRANSFER_IF_SUCCESS == response_status)
            {
                /* Set MMC data bus width 1-bit */
                response_status = cif_send_cmd( MMC_DW_CSD |(MSS_MMC_DATA_WIDTH_1BIT << SHIFT_8BIT),
                                                MMC_CMD_6_SWITCH,
                                                MSS_MMC_RESPONSE_R1B);
                /*
                * Writing to the EXT CSD register takes significant time, so function must
                * not return until the 'READY FOR DATA' bit is set in the Card Status
                * Register.
                */
                while (DEVICE_BUSY == response_status)
                {
                    response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                                MMC_CMD_13_SEND_STATUS,
                                                MSS_MMC_RESPONSE_R1);
                }
                /* Up the Host MMC clock frequency */
                set_host_sdclk(MSS_MMC_CLOCK_12_5MHZ);
                /* Set Phy delay for select MMC mode */
                g_mmc_init_complete = MMC_SET;
                phy_training_mmc(MSS_MMC_PHY_DELAY_INPUT_MMC_LEGACY, MSS_MMC_CLOCK_12_5MHZ);
                /* Data timeout 500ms */
                ret_status = set_data_timeout(DATA_TIMEOUT_VALUE);
                if (ret_status == MSS_MMC_NO_ERROR)
                {
                    ret_status =  MSS_MMC_CARD_SELECT_SUCCESS;
                }
            }
            else
            {
                ret_status = MSS_MMC_CARD_SELECT_ERROR;
            }
        }
        else
        {
            ret_status = MSS_MMC_RCA_ERROR;
        }
    }
    else
    {
        ret_status = MSS_MMC_OP_COND_ERR;
    }
    return ret_status;
}
/******************************************************************************/
static mss_mmc_status_t sdcard_oper_config(const mss_mmc_cfg_t * cfg)
{
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    cif_response_t response_status;
    uint32_t temp;
    uint32_t scr_reg[SHIFT_2BIT];
    uint32_t bus_width;
    uint32_t argument;

    uint32_t S18A = MMC_CLEAR;
    uint8_t S18R = MMC_CLEAR;
    uint8_t XPC = MMC_CLEAR;
    uint32_t tmp, CCS;
    uint32_t max_current_3_3V;
    uint32_t max_current_1_8V;

    uint32_t card_ocr_reg = MASK_24BIT;
    uint32_t card_volt = MMC_CLEAR;
    uint32_t controller_volt = MMC_CLEAR;

    uint32_t csd_max_sector_lwr;
    uint32_t max_sector_len;
    uint16_t sector_size;

    response_status = cif_send_cmd(IF_COND_27V_33V,
                                    SD_CMD_8_SEND_IF_COND,
                                    MSS_MMC_RESPONSE_R7);
    if (TRANSFER_IF_SUCCESS == response_status)
    {
        tmp = MMC->SRS18;
        max_current_3_3V = ((tmp & MASK_8BIT) * SHIFT_4BIT);
        max_current_1_8V = (((tmp & BYTE_3_MASK) >> SHIFT_16BIT) * SHIFT_4BIT);
        /* flag used for SDXC cards
           if card is initialized with XPC=1
           then it is operating less than 150mA
        */
         XPC = ((max_current_1_8V >= MAX_CURRENT_MA) && (max_current_3_3V >= MAX_CURRENT_MA)) ? MMC_SET : MMC_CLEAR;
        /* check if UHS-I is supported by SDIO host controller, then set to 1 */
         S18R = (is_uhsi_supported() != MMC_CLEAR) ? MMC_SET : MMC_CLEAR;

        response_status = cif_send_cmd(MMC_CLEAR,
                                        SD_CMD_55,
                                        MSS_MMC_RESPONSE_R1);
        if (TRANSFER_IF_SUCCESS == response_status)
        {
            response_status = cif_send_cmd(MMC_CLEAR,
                                        SD_ACMD_41_SEND_OP_COND,
                                        MSS_MMC_RESPONSE_R3);
            if (TRANSFER_IF_SUCCESS == response_status)
            {
                 /* Read OCR register */
                card_ocr_reg = MMC->SRS04;

                if ((cfg->bus_speed_mode == MSS_SDCARD_MODE_DEFAULT_SPEED) ||
                        (cfg->bus_speed_mode == MSS_SDCARD_MODE_HIGH_SPEED))
                {
                    S18R = MMC_CLEAR;
                }
                /* check the voltage capabilities of the SDIO host controller and a card
                      to set appropriate voltage */
                if ((card_ocr_reg & SDCARD_REG_OCR_3_3_3_4) != MMC_CLEAR)
                {
                    card_volt = (uint32_t)SDCARD_REG_OCR_3_3_3_4;
                    controller_volt = SRS10_SET_3_3V_BUS_VOLTAGE;
                }
                else if ((card_ocr_reg & SDCARD_REG_OCR_3_2_3_3) != MMC_CLEAR)
                {
                    card_volt = (uint32_t)SDCARD_REG_OCR_3_2_3_3;
                    controller_volt = SRS10_SET_3_3V_BUS_VOLTAGE;
                }
                else if ((card_ocr_reg & SDCARD_REG_OCR_3_0_3_1) != MMC_CLEAR)
                {
                    card_volt = (uint32_t)SDCARD_REG_OCR_3_0_3_1;
                    controller_volt = SRS10_SET_3_0V_BUS_VOLTAGE;
                }
                else if ((card_ocr_reg & SDCARD_REG_OCR_2_9_3_0) != MMC_CLEAR)
                {
                    card_volt = (uint32_t)SDCARD_REG_OCR_2_9_3_0;
                    controller_volt = SRS10_SET_3_0V_BUS_VOLTAGE;
                }
                else
                {
                    /* default values */
                    card_volt = (uint32_t)SDCARD_REG_OCR_3_3_3_4;
                    controller_volt = SRS10_SET_3_3V_BUS_VOLTAGE;
                }

                /* if the voltage is different to 3.3v */
                if (controller_volt != SRS10_SET_3_3V_BUS_VOLTAGE)
                {
                    set_sdhost_power(controller_volt);
                    response_status = cif_send_cmd(RESET_ARG,
                                                MMC_CMD_0_GO_IDLE_STATE,
                                                MSS_MMC_RESPONSE_NO_RESP);

                    if (TRANSFER_IF_SUCCESS == response_status)
                    {
                        mmc_delay(DELAY_COUNT);
                        response_status = cif_send_cmd(IF_COND_27V_33V,
                                                        SD_CMD_8_SEND_IF_COND,
                                                        MSS_MMC_RESPONSE_R7);
                        if (TRANSFER_IF_SUCCESS == response_status)
                        {
                            temp = MMC->SRS04;
                        }
                        else
                        {
                            return MSS_MMC_SDCARD_NOT_SUPPORT_VOLTAGE;
                        }
                    }
                }
                argument = card_volt;
                /* if card sent response on CMD8 command
                   set host high capacity flag to 1 */
                argument |= (uint32_t)SDCARD_ACMD41_HCS;
                if (S18R != MMC_CLEAR)
                {
                    argument |= (uint32_t)SDCARD_REG_OCR_S18R;
                }

                if (XPC != MMC_CLEAR)
                {
                    argument |= (uint32_t)SDCARD_REG_OCR_XPC;
                }
                ret_status = MSS_MMC_TRANSFER_SUCCESS;

                /* Read OCR register from SD card*/
                do
                {
                    mmc_delay(DELAY_COUNT);
                    response_status = cif_send_cmd(MMC_CLEAR,
                                        SD_CMD_55,
                                        MSS_MMC_RESPONSE_R1);
                    if (TRANSFER_IF_SUCCESS == response_status)
                    {
                        mmc_delay(DELAY_COUNT);
                        response_status = cif_send_cmd(argument,
                                        SD_ACMD_41_SEND_OP_COND,
                                        MSS_MMC_RESPONSE_R3);
                    }
                    tmp = MMC->SRS04;
                }while ((tmp & SDCARD_REG_OCR_READY) == MMC_CLEAR);
                /* If CCS = 0, SDSC card < 2gb. If CCS = 1, SDHC/XC >2gb */
                CCS = MMC->SRS04 & SDCARD_REG_OCR_CCS;
                S18A = MMC->SRS04 & SDCARD_REG_OCR_S18A;
                /* UHI -I Support */
                if (S18A != MMC_CLEAR)
                {
                    if (cfg->data_bus_width == MSS_MMC_DATA_WIDTH_4BIT)
                    {
                        MMC->SRS10 |= SRS10_DATA_WIDTH_4BIT;
                    }
                    else
                    {
                        return MSS_MMC_MODE_NOT_SUPPORT_DATAWIDTH;
                    }
                    const uint32_t dat_level = (SRS9_DAT0_SIGNAL_LEVEL | SRS9_DAT1_SIGNAL_LEVEL
                                              | SRS9_DAT2_SIGNAL_LEVEL | SRS9_DAT3_SIGNAL_LEVEL);

                    response_status = cif_send_cmd(MMC_CLEAR,
                                                SD_CMD_11_VOLAGE_SWITCH,
                                                MSS_MMC_RESPONSE_R1);
                    if (TRANSFER_IF_SUCCESS == response_status)
                    {
                        /* disable sd clock */
                        temp = MMC->SRS11;
                        temp &= ~(uint32_t)SRS11_SD_CLOCK_ENABLE;
                        MMC->SRS11 = temp;
                        /* if dat line is not 0000b return error */
                        tmp = MMC->SRS09;
                        if ((tmp & dat_level) != MMC_CLEAR)
                        {
                            ret_status = MSS_MMC_ERR_SWITCH_VOLTAGE_FAILED;
                        }
                        else
                        {
                            /* 1.8V signal enable */
                            tmp = MMC->SRS15;
                            tmp |= SRS15_18V_ENABLE;
                            MMC->SRS15 = tmp;

                            /* wait 5ms */
                            mmc_delay(DELAY_COUNT);
                            mmc_delay(DELAY_COUNT);
                            /* if 1.8V signal enable is cleared by host return with error */
                            tmp = MMC->SRS15;
                            if ((tmp & SRS15_18V_ENABLE) == MMC_CLEAR)
                            {
                                ret_status = MSS_MMC_ERR_SWITCH_VOLTAGE_FAILED;
                            }
                            else
                            {
                                /* internal clock enable */
                                tmp = MMC->SRS11;
                                tmp |= (uint32_t)SRS11_INT_CLOCK_ENABLE;
                                MMC->SRS11 = tmp;
                                while (MMC_CLEAR == (MMC->SRS11 & SRS11_INT_CLOCK_STABLE));

                                /* enable sd clock */
                                temp = MMC->SRS11;
                                temp |= (uint32_t)SRS11_SD_CLOCK_ENABLE;
                                MMC->SRS11 = temp;
                                /* wait 1ms */
                                mmc_delay(DELAY_COUNT);
                                /* if dat line is not 1111b return error */
                                tmp = MMC->SRS09;
                                if ((tmp & dat_level) != dat_level)
                                {
                                    ret_status = MSS_MMC_ERR_SWITCH_VOLTAGE_FAILED;
                                    response_status = TRANSFER_IF_FAIL;
                                }
                                else
                                {
                                    ret_status = MSS_MMC_TRANSFER_SUCCESS;
                                }
                            }
                        }
                    }
                }
                
                if ((TRANSFER_IF_SUCCESS == response_status) && (ret_status == MSS_MMC_TRANSFER_SUCCESS))
                {
                    response_status = cif_send_cmd(MMC_CLEAR,
                                                    MMC_CMD_2_ALL_SEND_CID,
                                                    MSS_MMC_RESPONSE_R2);
                    if (TRANSFER_IF_SUCCESS == response_status)
                    {

                        response_status = cif_send_cmd(MMC_CLEAR,
                                                    MMC_CMD_3_SET_RELATIVE_ADDR,
                                                    MSS_MMC_RESPONSE_R6);

                        sdcard_RCA = ((MMC->SRS04 >> SHIFT_16BIT) & MASK_16BIT);

                        if ((S18A == MMC_CLEAR) && (S18R != MMC_CLEAR))
                        {
                            sd_card_uhsi_supported();
                        }
                        /* Read CSD register from device */
                        response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                                        MMC_CMD_9_SEND_CSD,
                                                        MSS_MMC_RESPONSE_R2);
                        /* Sector size from "Max. write data block length"  of CSD[25:22]*/
                        csd_max_sector_lwr  = MMC->SRS04 & SECT_SIZE_CSD_MASK;
                        max_sector_len = csd_max_sector_lwr >> SECT_SIZE_CSD_SHIFT;
                        sector_size = (uint16_t)((uint32_t)MMC_SET << max_sector_len);

                        response_status = cif_send_cmd(sdcard_RCA << SHIFT_16BIT,
                                                    MMC_CMD_7_SELECT_DESELECT_CARD,
                                                    MSS_MMC_RESPONSE_R1B);
                        if (TRANSFER_IF_SUCCESS == response_status)
                        {


                            /* disable host normal interrupts for masking  incorrect interrupts
                                that may occur while changing the bus width.*/
                            tmp = MMC->SRS13;
                            MMC->SRS13 = tmp & ~(uint32_t)SRS13_CARD_INTERRUPT_STAT_EN;
                            mmc_delay(DELAY_COUNT);
                            response_status = cif_send_cmd(sdcard_RCA << SHIFT_16BIT,
                                                                SD_CMD_55,
                                                            MSS_MMC_RESPONSE_R1);
                            if (TRANSFER_IF_SUCCESS == response_status)
                            {
                                if (cfg->data_bus_width == MSS_MMC_DATA_WIDTH_4BIT)
                                {
                                    /* 4-bit bus data width */
                                    bus_width = SHIFT_2BIT;
                                }
                                else
                                {
                                    /* 1-bit bus data width */
                                    bus_width = MMC_CLEAR;
                                }
                                /* set bus with in the card 4-bit -2, 1-bit - 0 */
                                response_status = cif_send_cmd(bus_width,
                                                                SD_ACMD_6,
                                                           MSS_MMC_RESPONSE_R1);
                                if (TRANSFER_IF_SUCCESS == response_status)
                                {
                                    if (cfg->data_bus_width == MSS_MMC_DATA_WIDTH_4BIT)
                                    {
                                        MMC->SRS10 |= SRS10_DATA_WIDTH_4BIT;
                                    }
                                    MMC->SRS13 = tmp;
                                    /* mmc init success */
                                    g_mmc_init_complete = MMC_SET;
                                    /* get SCR register */
                                    ret_status = MSS_MMC_single_block_read(READ_SEND_SCR, scr_reg);
                                    if (ret_status == MSS_MMC_TRANSFER_SUCCESS)
                                    {
                                        ret_status = set_sd_host_device_bus_mode(cfg);
                                        if (ret_status == MSS_MMC_TRANSFER_SUCCESS)
                                        {
                                            /* up the Host MMC clock frequency */
                                            set_host_sdclk(cfg->clk_rate);

                                            switch(cfg->bus_speed_mode)
                                            {

                                                case MSS_SDCARD_MODE_SDR12:
                                                    phy_training_mmc(MSS_MMC_PHY_DELAY_INPUT_SDR12, cfg->clk_rate);
                                                    break;
                                                case MSS_SDCARD_MODE_SDR25:
                                                    phy_training_mmc(MSS_MMC_PHY_DELAY_INPUT_SDR25, cfg->clk_rate);
                                                    break;
                                                case MSS_SDCARD_MODE_SDR50:
                                                    phy_training_mmc(MSS_MMC_PHY_DELAY_INPUT_SDR50, cfg->clk_rate);
                                                    tmp = MMC->SRS17;
                                                    if ((tmp & SRS17_USE_TUNING_SDR50) != MMC_CLEAR)
                                                    {
                                                        ret_status = sd_tuning();
                                                        if (ret_status != MSS_MMC_TRANSFER_SUCCESS)
                                                        {
                                                            response_status = TRANSFER_IF_FAIL;
                                                        }
                                                    }
                                                    break;
                                                case MSS_SDCARD_MODE_SDR104:
                                                    ret_status = sd_tuning();
                                                    if (ret_status != MSS_MMC_TRANSFER_SUCCESS)
                                                    {
                                                        response_status = TRANSFER_IF_FAIL;
                                                    }
                                                    break;
                                                case MSS_SDCARD_MODE_DDR50:
                                                    phy_training_mmc(MSS_MMC_PHY_DELAY_INPUT_DDR50, cfg->clk_rate);
                                                    break;
                                                default:
                                                    phy_training_mmc(MSS_MMC_PHY_DELAY_INPUT_HIGH_SPEED, cfg->clk_rate);
                                                    break;
                                            }
                                        }
                                        else
                                        {
                                            response_status = TRANSFER_IF_FAIL;
                                        }
                                    }
                                    else
                                    {
                                        response_status = TRANSFER_IF_FAIL;
                                    }
                                }
                                else
                                {
                                    response_status = TRANSFER_IF_FAIL;
                                }
                            }
                            else
                            {
                                MMC->SRS13 = tmp;
                            }
                        }
                    }
                }
            }
        }
    }

    if (TRANSFER_IF_SUCCESS == response_status)
    {
        ret_status = MSS_MMC_TRANSFER_SUCCESS;
    }
    else
    {
        ret_status = MSS_MMC_TRANSFER_FAIL;
    }
    return(ret_status);
}
/*****************************SDIO START***************************************/
/******************************************************************************/
static mss_mmc_status_t sdio_oper_config(const mss_mmc_cfg_t * cfg)
{
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    cif_response_t response_status;
    uint32_t temp;
    uint32_t power_up_status;
    uint32_t scr_reg[BYTES_2];
    uint32_t bus_width;
    uint32_t tmp, argument;
    
    uint32_t S18A;
    uint8_t S18R;

    uint32_t card_ocr_reg = MASK_24BIT;
    uint32_t card_volt = MMC_CLEAR;
    uint32_t controller_volt = MMC_CLEAR;

    /* check if UHS-I is supported by SDIO host controller, then set to 1 */
    S18R = (is_uhsi_supported() != MMC_CLEAR) ? MMC_SET : MMC_CLEAR;

    response_status = cif_send_cmd(MMC_CLEAR, SD_CMD_5, MSS_MMC_RESPONSE_R4);
    if (TRANSFER_IF_SUCCESS == response_status)
    {
         /* Read OCR register */
        card_ocr_reg = MMC->SRS04;

        if ((cfg->bus_speed_mode == MSS_SDCARD_MODE_DEFAULT_SPEED) ||
                    (cfg->bus_speed_mode == MSS_SDCARD_MODE_HIGH_SPEED))
        {
            S18R = MMC_CLEAR;
        }
        /* check the voltage capabilities of the SDIO host controller and a card
           to set appropriate voltage */
        if ((card_ocr_reg & SDCARD_REG_OCR_3_3_3_4) != MMC_CLEAR)
        {
            card_volt = (uint32_t)SDCARD_REG_OCR_3_3_3_4;
            controller_volt = SRS10_SET_3_3V_BUS_VOLTAGE;
        }
        else if ((card_ocr_reg & SDCARD_REG_OCR_3_2_3_3) != MMC_CLEAR)
        {
            card_volt = (uint32_t)SDCARD_REG_OCR_3_2_3_3;
            controller_volt = SRS10_SET_3_3V_BUS_VOLTAGE;
        }
        else if ((card_ocr_reg & SDCARD_REG_OCR_3_0_3_1) != MMC_CLEAR)
        {
            card_volt = (uint32_t)SDCARD_REG_OCR_3_0_3_1;
            controller_volt = SRS10_SET_3_0V_BUS_VOLTAGE;
        }
        else if ((card_ocr_reg & SDCARD_REG_OCR_2_9_3_0) != MMC_CLEAR)
        {
            card_volt = (uint32_t)SDCARD_REG_OCR_2_9_3_0;
            controller_volt = SRS10_SET_3_0V_BUS_VOLTAGE;
        }
        else
        {
            card_volt = (uint32_t)SDCARD_REG_OCR_3_3_3_4;
            controller_volt = SRS10_SET_3_3V_BUS_VOLTAGE;
        }
        /* if the voltage is different to 3.3v */
        if (controller_volt != SRS10_SET_3_3V_BUS_VOLTAGE)
        {
            set_sdhost_power(controller_volt);
        }

        argument = card_volt;

        if (S18R != MMC_CLEAR)
        {
            argument |= (uint32_t)SDCARD_REG_OCR_S18R;
        }

        ret_status = MSS_MMC_TRANSFER_SUCCESS;

        /* Read OCR register from SD card*/
        do
        {
            mmc_delay(DELAY_COUNT);
            response_status = cif_send_cmd(argument, SD_CMD_5, MSS_MMC_RESPONSE_R4);

        }while ((MMC->SRS04 & SDCARD_REG_OCR_READY) == MMC_CLEAR);

        S18A = MMC->SRS04 & SDCARD_REG_OCR_S18A;
        /* UHI -I Support */
        if (S18A != MMC_CLEAR)
        {
            if (cfg->data_bus_width == MSS_MMC_DATA_WIDTH_4BIT)
            {
                MMC->SRS10 |= SRS10_DATA_WIDTH_4BIT;
            }
            else
            {
                return MSS_MMC_MODE_NOT_SUPPORT_DATAWIDTH;
            }
            const uint32_t dat_level = (SRS9_DAT0_SIGNAL_LEVEL | SRS9_DAT1_SIGNAL_LEVEL
                                      | SRS9_DAT2_SIGNAL_LEVEL | SRS9_DAT3_SIGNAL_LEVEL);

            response_status = cif_send_cmd(MMC_CLEAR,
                                        SD_CMD_11_VOLAGE_SWITCH,
                                        MSS_MMC_RESPONSE_R1);
            if (TRANSFER_IF_SUCCESS == response_status)
            {
                /* disable sd clock */
                temp = MMC->SRS11;
                temp &= ~(uint32_t)SRS11_SD_CLOCK_ENABLE;
                MMC->SRS11 = temp;
                /* if dat line is not 0000b return error */
                tmp = MMC->SRS09;
                if ((tmp & dat_level) != MMC_CLEAR)
                {
                    ret_status = MSS_MMC_ERR_SWITCH_VOLTAGE_FAILED;
                }
                else
                {
                    /* 1.8V signal enable */
                    tmp = MMC->SRS15;
                    tmp |= SRS15_18V_ENABLE;
                    MMC->SRS15 = tmp;
    
                    /* wait 5ms */
                    mmc_delay(DELAY_COUNT);
                    mmc_delay(DELAY_COUNT);
                    mmc_delay(DELAY_COUNT);
                    mmc_delay(DELAY_COUNT);
                    /* if 1.8V signal enable is cleared by host return with error */
                    tmp = MMC->SRS15;
                    if ((tmp & SRS15_18V_ENABLE) == MMC_CLEAR)
                    {
                        ret_status = MSS_MMC_ERR_SWITCH_VOLTAGE_FAILED;
                    }
                    else
                    {
                        /* internal clock enable */
                        tmp = MMC->SRS11;
                        tmp |= (uint32_t)SRS11_INT_CLOCK_ENABLE;
                        MMC->SRS11 = tmp;
                        while (MMC_CLEAR == (MMC->SRS11 & SRS11_INT_CLOCK_STABLE));
    
                        /* enable sd clock */
                        temp = MMC->SRS11;
                        temp |= (uint32_t)SRS11_SD_CLOCK_ENABLE;
                        MMC->SRS11 = temp;
                        /* wait 1ms */
                        mmc_delay(DELAY_COUNT);
                        mmc_delay(DELAY_COUNT);
                        /* if dat line is not 1111b return error */
                        tmp = MMC->SRS09;
                        if ((tmp & dat_level) != dat_level)
                        {
                            ret_status = MSS_MMC_ERR_SWITCH_VOLTAGE_FAILED;
                            response_status = TRANSFER_IF_FAIL;
                        }
                        else
                        {
                            ret_status = MSS_MMC_TRANSFER_SUCCESS;
                        }
                    }
                }
            }
            
        }
        if ((TRANSFER_IF_SUCCESS == response_status) &&
                (ret_status == MSS_MMC_TRANSFER_SUCCESS))
        {
            
            response_status = cif_send_cmd(MMC_CLEAR,
                                MMC_CMD_3_SET_RELATIVE_ADDR,
                                MSS_MMC_RESPONSE_R6);

            sdcard_RCA = ((MMC->SRS04 >> SHIFT_16BIT) & MASK_16BIT);
    
            if ((S18A == MMC_CLEAR) && (S18R != MMC_CLEAR))
            {
                sd_card_uhsi_supported();
            }
            response_status = cif_send_cmd(sdcard_RCA << SHIFT_16BIT,
                                        MMC_CMD_7_SELECT_DESELECT_CARD,
                                        MSS_MMC_RESPONSE_R1B);
            if (TRANSFER_IF_SUCCESS == response_status)
            {
                uint8_t dum_tmp;
                uint8_t tmp_1;
                uint8_t card_int_stat;
            /* disable host normal interrupts for masking  incorrect interrupts
                    that may occur while changing the bus width.*/
                dum_tmp = MMC->SRS13;
                MMC->SRS13 = dum_tmp & ~(uint32_t)SRS13_CARD_INTERRUPT_STAT_EN;
                mmc_delay(DELAY_COUNT);

                /* reset SDIO card */
                tmp_1 = SHIFT_8BIT;
                sdio_host_access_cccr(SDIOHOST_CCCR_WRITE, &tmp_1,
                                             sizeof(tmp_1), MSS_MMC_CCCR_ABORT);

                /* read status of the card interrupt register */
                sdio_host_access_cccr(SDIOHOST_CCCR_READ, &card_int_stat,
                                sizeof(card_int_stat), MSS_MMC_CCCR_INT_ENABLE);

                /* clear interrupt enable master bit */
                if ((card_int_stat & CARD_INT_STATUS_MASK) != MMC_CLEAR)
                {
                    tmp_1 = card_int_stat & CARD_INT_STATUS_MASK;
                    /* disable all card interrupts */
                    sdio_host_access_cccr(SDIOHOST_CCCR_WRITE, &tmp_1,
                                         sizeof(tmp_1), MSS_MMC_CCCR_INT_ENABLE);
                }

                /* read bus interface control */
                sdio_host_access_cccr(SDIOHOST_CCCR_READ, &tmp_1,
                                        sizeof(tmp_1), MSS_MMC_CCCR_BUS_CONTROL);

                if (cfg->data_bus_width == MSS_MMC_DATA_WIDTH_1BIT)
                {
                    tmp_1 = (tmp_1 & SDCARD_BUSWIDTH_MASK) | SDCARD_BIS_BUS_WIDTH_1BIT;
                }
                else
                {
                    tmp_1 = (tmp_1 & SDCARD_BUSWIDTH_MASK) | SDCARD_BIS_BUS_WIDTH_4BIT;
                }

                /* set bus width in the SDIO card */
                sdio_host_access_cccr(SDIOHOST_CCCR_WRITE, &tmp_1,
                                         sizeof(tmp_1), MSS_MMC_CCCR_BUS_CONTROL);


                if (cfg->data_bus_width == MSS_MMC_DATA_WIDTH_4BIT)
                {
                    MMC->SRS10 |= SRS10_DATA_WIDTH_4BIT;
                }
                /* check if the interrupts was enabled before */
                if ((card_int_stat & CARD_INT_STATUS_MASK) != MMC_CLEAR)
                {
                    /* resume card interrupts state */
                    sdio_host_access_cccr(SDIOHOST_CCCR_WRITE, &card_int_stat,
                                sizeof(card_int_stat), MSS_MMC_CCCR_INT_ENABLE);
                }
                uint16_t blck_size = BLK_SIZE;
                /* set block size 512 in the SDIO card for Function 0 */
                sdio_host_access_cccr(SDIOHOST_CCCR_WRITE, &blck_size,
                                 sizeof(blck_size), MSS_MMC_CCCR_FN0_BLOCK_SIZE);

                MMC->SRS13 = dum_tmp;
                /* mmc init success */
                g_mmc_init_complete = MMC_SET;

                uint16_t manufacturerCode, manufacturer_information;
                uint8_t buffer[BYTES_4];
                uint32_t common_cis_address;
                /* Get CIS pointer */
                sdio_host_access_cccr(SDIOHOST_CCCR_READ,
                                        &common_cis_address,
                                        sizeof(common_cis_address),
                                        MSS_MMC_CCCR_CIS_POINTER);
                /* Get manufacturer ID and manufacturer informations */
                sdio_host_get_tuple_from_cis(common_cis_address,
                                            MSS_MMC_TUPLE_CISTPL_MANFID,
                                            buffer, sizeof(buffer));

                manufacturerCode = buffer[MMC_CLEAR] | ((uint16_t)buffer[BYTES_1] << SHIFT_8BIT);
                manufacturer_information = buffer[BYTES_2] | ((uint16_t)buffer[BYTES_3] << SHIFT_8BIT);
                
                ret_status = change_sdio_device_bus_mode(cfg);
                if (ret_status == MSS_MMC_TRANSFER_SUCCESS)
                {
                    /* up the Host MMC clock frequency */
                    set_host_sdclk(cfg->clk_rate);
                }
                else
                {
                    response_status = TRANSFER_IF_FAIL;
                }
            }
        }
    }

    if (TRANSFER_IF_SUCCESS == response_status)
    {    
        ret_status = MSS_MMC_TRANSFER_SUCCESS;
    }
    else
    {
        ret_status = MSS_MMC_TRANSFER_FAIL;
    }
    return(ret_status);
}

/******************************************************************************/
static mss_mmc_status_t change_sdio_device_bus_mode(const mss_mmc_cfg_t * cfg)
{
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    uint8_t high_speed_reg, uhsi_support;
    uint8_t enable_high_speed = MMC_CLEAR;
    uint32_t tmp;
    /* get high speed register from a card */
    sdio_host_access_cccr(SDIOHOST_CCCR_READ, &high_speed_reg,
                                 sizeof(high_speed_reg), MSS_MMC_CCCR_HIGH_SPEED);

    /* clear all access mode settings */
    high_speed_reg &= ~SDIO_CCCR_13_BSS_MASK;

    if (cfg->bus_speed_mode > MSS_SDCARD_MODE_SDR25)
    {
        sdio_host_access_cccr(SDIOHOST_CCCR_READ, &uhsi_support,
                                     sizeof(uhsi_support),
                                     MSS_MMC_CCCR_UHSI_SUPPORT);
    }

    switch (cfg->bus_speed_mode)
    {
    case MSS_SDCARD_MODE_SDR12:
    case MSS_SDCARD_MODE_DEFAULT_SPEED:
        break;
    case MSS_SDCARD_MODE_SDR25:
    case MSS_SDCARD_MODE_HIGH_SPEED:
        /* check if card supports SDR25 mode (high speed) */
        if (MMC_CLEAR == (high_speed_reg & SDIO_CCCR_13_SHS))
        {
            return MSS_MMC_SDIO_ERR_BUS_SPEED_UNSUPP;
        }
        high_speed_reg |= SDIO_CCCR_13_EHS;
        enable_high_speed = MMC_SET;
        break;
    case MSS_SDCARD_MODE_SDR50:
        /* check if card supports SDR50 access mode */
        if (MMC_CLEAR == (uhsi_support & SDIO_CCCR_14_SSDR50))
        {
            return MSS_MMC_SDIO_ERR_BUS_SPEED_UNSUPP;
        }
        high_speed_reg |= SDIO_CCCR_13_BSS_SDR50;
        enable_high_speed = MMC_SET;
        break;
    case MSS_SDCARD_MODE_SDR104:
        /* check if card supports SDR104 access mode */
        if (MMC_CLEAR == (uhsi_support & SDIO_CCCR_14_SSDR104))
        {
            return MSS_MMC_SDIO_ERR_BUS_SPEED_UNSUPP;
        }
        high_speed_reg |= SDIO_CCCR_13_BSS_SDR104;
        enable_high_speed = MMC_SET;
        break;
    case MSS_SDCARD_MODE_DDR50:
        /* check if card supports DDR50 access mode */
        if (MMC_CLEAR == (uhsi_support & SDIO_CCCR_14_SDDR50))
        {
            return MSS_MMC_SDIO_ERR_BUS_SPEED_UNSUPP;
        }
        high_speed_reg |= SDIO_CCCR_13_BSS_DDR50;
        enable_high_speed = MMC_SET;
        break;
    default: /* nothing */
        break;

    }

    /* apply access bus mode to sdio card */
    sdio_host_access_cccr(SDIOHOST_CCCR_WRITE, &high_speed_reg,
                               sizeof(high_speed_reg),
                               MSS_MMC_CCCR_HIGH_SPEED);
                               
    if (enable_high_speed != MMC_CLEAR)
    {
        /* enable high-speed in the SDIO host controller */
        tmp = MMC->SRS10;
        tmp |= SRS10_HIGH_SPEED_ENABLE;
        MMC->SRS10 = tmp;
    }
    else
    {
        /* disable high-speed in the SDIO host controller */
        tmp = MMC->SRS10;
        tmp &= ~SRS10_HIGH_SPEED_ENABLE;
        MMC->SRS10 = tmp;
    }
        
    return MSS_MMC_TRANSFER_SUCCESS;
}
/******************************************************************************/
#if 0
static void SDIOHost_ReadCCCR(uint8_t *data)
{
    /* for SDIO or combo cards read CCCR to get information about supported bus widths */
    uint8_t CardCapability;

    /* read bus card capabilities */
    sdio_host_access_cccr(SDIOHOST_CCCR_READ,
                                 &CardCapability, sizeof(CardCapability),
                                 MSS_MMC_CCCR_CARD_CAPABILITY);

    if ((CardCapability & SDCARD_CCR_LSC) != MMC_CLEAR
        && ((CardCapability & SDCARD_CCR_4BLS)) == MMC_CLEAR) {
       *data =  SDIO_BUS_WIDTH_1;
    }
    else {
        *data = SDIO_BUS_WIDTH_1 | SDIO_BUS_WIDTH_4;
    }
}
#endif
/******************************************************************************/
static void sdio_host_access_cccr
(
    uint8_t transfer_direction,
    void *data,
    uint8_t data_size,
    uint8_t register_address
)
{

    uint8_t number_of_bytes, i = MMC_CLEAR;
    uint32_t argument, response, tmp = MMC_CLEAR;
    cif_response_t response_status = TRANSFER_IF_SUCCESS;
    /* set numbers of bytes to transfer
      and check if the data parameter has appropriate size */
    switch (register_address) {
    case MSS_MMC_CCCR_CIS_POINTER:
        number_of_bytes = BYTES_3;
        if (data_size < BYTES_4) {
        }
        break;
    case MSS_MMC_CCCR_FN0_BLOCK_SIZE:
        if (data_size < BYTES_2) {
        }
        number_of_bytes = BYTES_2;
        break;
    default:
        if (data_size < BYTES_1) {
        }
        number_of_bytes = BYTES_1;
        break;
    }

    /* read data from the CCCR register */
    if (transfer_direction == MMC_CLEAR)
    {
        for (i = MMC_CLEAR; i < number_of_bytes; i++)
        {
            /* set address of register to read */
            argument = (uint32_t)((register_address + i) << SHIFT_9BIT)
                               | (uint32_t)(transfer_direction << SHIFT_31BIT);

            /* execute command CMD52 to read data /from CCCR register */
            response_status = cif_send_cmd(argument,
                                            SDIO_CMD_52_IO_RW_DIRECT,
                                            MSS_MMC_RESPONSE_R5);
            response = MMC->SRS04;

            tmp |= (response & MASK_8BIT) << (i * SHIFT_8BIT);
        }

        switch (register_address)
        {
        case MSS_MMC_CCCR_CIS_POINTER:
            *((uint32_t *)data) = (uint32_t)tmp;
            break;
        case MSS_MMC_CCCR_FN0_BLOCK_SIZE:
            *((uint16_t *)data) = (uint16_t)tmp;
            break;
        default:
            *((uint8_t *)data) = (uint8_t)tmp;
            break;
        }
    }
    /* write data to the CCCR register */
    else
    {
        uint8_t *data_byte = data;
        for (i = MMC_CLEAR; i < number_of_bytes; i++)
        {
            /* set address of register to write */
            argument = (uint32_t)((register_address + i) << SHIFT_9BIT)
                               | (uint32_t)(transfer_direction << SHIFT_31BIT)
                               | *data_byte++;

            /* execute command CMD52 to write data to CCCR register */
            response_status = cif_send_cmd(argument,
                                        SDIO_CMD_52_IO_RW_DIRECT,
                                        MSS_MMC_RESPONSE_R5);
        }
    }
}
/******************************************************************************/
static void sdio_host_access_fbr
(
    uint8_t transfer_direction,
    void *data,
    uint8_t data_size,
    uint32_t register_address,
    uint8_t fun_num
)
{

    uint8_t number_of_bytes, i = MMC_CLEAR;
    uint32_t argument, response, tmp = MMC_CLEAR;
    cif_response_t response_status = TRANSFER_IF_SUCCESS;
    /* set numbers of bytes to transfer
      and check if the data parameter has appropriate size */
    switch (register_address) {
    case MSS_MMC_FBR_ADDR_CIS:
    case MSS_MMC_FBR_ADDR_CSA:
        number_of_bytes = BYTES_3;
        if (data_size < BYTES_4) {
        }
        break;
    case MSS_MMC_FBR_BLOCK_SIZE:
        if (data_size < BYTES_2) {
        }
        number_of_bytes = BYTES_2;
        break;
    default:
        if (data_size < BYTES_1) {
        }
        number_of_bytes = BYTES_1;
        break;
    }

    register_address = register_address + (100u * fun_num);
    /* read data from the CCCR register */
    if (transfer_direction == MMC_CLEAR)
    {
        for (i = MMC_CLEAR; i < number_of_bytes; i++)
        {
            /* set address of register to read */
            argument = (uint32_t)((register_address + i) << SHIFT_9BIT)
                               | (uint32_t)(transfer_direction << SHIFT_31BIT);

            /* execute command CMD52 to read data /from CCCR register */
            response_status = cif_send_cmd(argument,
                                            SDIO_CMD_52_IO_RW_DIRECT,
                                            MSS_MMC_RESPONSE_R5);
            response = MMC->SRS04;

            tmp |= (response & MASK_8BIT) << (i * SHIFT_8BIT);
        }

        switch (register_address)
        {
        case MSS_MMC_FBR_ADDR_CIS:
        case MSS_MMC_FBR_ADDR_CSA:
            *((uint32_t *)data) = (uint32_t)tmp;
            break;
        case MSS_MMC_FBR_BLOCK_SIZE:
            *((uint16_t *)data) = (uint16_t)tmp;
            break;
        default:
            *((uint8_t *)data) = (uint8_t)tmp;
            break;
        }
    }
    /* write data to the CCCR register */
    else
    {
        uint8_t *data_byte = data;
        for (i = MMC_CLEAR; i < number_of_bytes; i++)
        {
            /* set address of register to write */
            argument = (uint32_t)((register_address + i) << SHIFT_9BIT)
                               | (uint32_t)(transfer_direction << SHIFT_31BIT)
                               | *data_byte++;

            /* execute command CMD52 to write data to CCCR register */
            response_status = cif_send_cmd(argument,
                                        SDIO_CMD_52_IO_RW_DIRECT,
                                        MSS_MMC_RESPONSE_R5);
        }
    }
}
/******************************************************************************/
static void sdio_host_get_tuple_from_cis
(
    uint32_t tuple_address,
    uint8_t tuple_code,
    uint8_t *buffer,
    uint8_t buffer_size
)
{
    uint32_t argument, response;
    uint8_t i;
    uint8_t next_tuple_offset;
    cif_response_t response_status = TRANSFER_IF_SUCCESS;

    argument = (tuple_address << SHIFT_9BIT);
    while (MMC_SET)
    {
        uint8_t read_tuple_code;
        /* Read tuple code name */
        response_status = cif_send_cmd(argument,
                                        SDIO_CMD_52_IO_RW_DIRECT,
                                        MSS_MMC_RESPONSE_R5);
        response = MMC->SRS04;
        if ((response & MASK_8BIT) == MSS_MMC_TUPLE_CISTPL_END)
        {
           /* SDIO ERR TUPLE NOT FOUND */
        }
        read_tuple_code = response & MASK_8BIT;

        tuple_address++;
        argument = tuple_address << SHIFT_9BIT;
        /* Read link to next tuple */
        response_status = cif_send_cmd(argument,
                                        SDIO_CMD_52_IO_RW_DIRECT,
                                        MSS_MMC_RESPONSE_R5);
        response = MMC->SRS04;
        if ((response & MASK_8BIT) == MSS_MMC_TUPLE_CISTPL_END)
        {
            /* SDIO ERR TUPLE NOT FOUND */
        }

        /* save size of tuple body */
        next_tuple_offset = response & MASK_8BIT;
        tuple_address++;

        if (read_tuple_code == tuple_code)
        {
            if (buffer_size < next_tuple_offset)
            {
                /* to small buffer for data */
            }

            for (i = MMC_CLEAR; i < next_tuple_offset; i++)
            {
                argument = (tuple_address + i) << SHIFT_9BIT;
                /* Read tuple body */
                response_status = cif_send_cmd(argument,
                                                SDIO_CMD_52_IO_RW_DIRECT,
                                                MSS_MMC_RESPONSE_R5);
                response = MMC->SRS04;

                buffer[i] = (uint8_t)(response & MASK_8BIT);
            }
            break;
        }
        else
        {
            tuple_address += next_tuple_offset;
            argument = tuple_address << SHIFT_9BIT;
        }
    }

}
/*****************************SDIO END*****************************************/
/******************************************************************************/
static mss_mmc_status_t set_sd_host_device_bus_mode(const mss_mmc_cfg_t * cfg)
{
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    uint8_t card_access_mode;
    uint8_t card_driver_strength = SDCARD_SWITCH_DRIVER_STRENGTH_TYPE_B;
    switch (cfg->bus_speed_mode)
    {
    case (uint8_t)MSS_SDCARD_MODE_SDR12:
    case (uint8_t)MSS_SDCARD_MODE_DEFAULT_SPEED:
        card_access_mode = SDCARD_SWITCH_ACCESS_MODE_SDR12;
        break;
    case (uint8_t)MSS_SDCARD_MODE_HIGH_SPEED:
    case (uint8_t)MSS_SDCARD_MODE_SDR25:
        card_access_mode = SDCARD_SWITCH_ACCESS_MODE_SDR25;
        break;
    case (uint8_t)MSS_SDCARD_MODE_SDR50:
        card_access_mode = SDCARD_SWITCH_ACCESS_MODE_SDR50;
        break;
    case (uint8_t)MSS_SDCARD_MODE_SDR104:
        card_access_mode = SDCARD_SWITCH_ACCESS_MODE_SDR104;
        break;
    case (uint8_t)MSS_SDCARD_MODE_DDR50:
        card_access_mode = SDCARD_SWITCH_ACCESS_MODE_DDR50;
        break;
    default:
        ret_status = MSS_MMC_INVALID_PARAMETER;
        break;
    }
    if (ret_status == MSS_MMC_NO_ERROR)
    {
        ret_status= sd_host_process_switch_function(card_access_mode, SDCARD_SWITCH_GROUP_NR_1);
        if (ret_status == MSS_MMC_TRANSFER_SUCCESS)
        {
            ret_status = set_host_uhsi_mode(cfg->bus_speed_mode, card_driver_strength);
        }
    }
    return ret_status;
}
/******************************************************************************/
static mss_mmc_status_t set_host_uhsi_mode
(
    uint8_t access_mode,
    uint8_t driver_strengh
)
{
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    uint32_t tmp;
    uint32_t uhs_mode;
    uint8_t enable_high_speed = MMC_CLEAR;

    switch (access_mode) {
    case (uint8_t)MSS_SDCARD_MODE_SDR12:
    case (uint8_t)MSS_SDCARD_MODE_DEFAULT_SPEED:
        uhs_mode = SRS15_UHS_MODE_SDR12;
        break;
    case (uint8_t)MSS_SDCARD_MODE_SDR25:
    case (uint8_t)MSS_SDCARD_MODE_HIGH_SPEED:
        uhs_mode = SRS15_UHS_MODE_SDR25;
        enable_high_speed = MMC_SET;
        break;
    case (uint8_t)MSS_SDCARD_MODE_SDR50:
        uhs_mode = SRS15_UHS_MODE_SDR50;
        enable_high_speed = MMC_SET;
        break;
    case (uint8_t)MSS_SDCARD_MODE_SDR104:
        uhs_mode = SRS15_UHS_MODE_SDR104;
        enable_high_speed = MMC_SET;
        break;
    case (uint8_t)MSS_SDCARD_MODE_DDR50:
        uhs_mode = SRS15_UHS_MODE_DDR50;
        enable_high_speed = MMC_SET;
        break;
    default:
        ret_status = MSS_MMC_INVALID_PARAMETER;
        break;
    }

    if (ret_status == MSS_MMC_NO_ERROR)
    {
        uint32_t SRS15 = MMC->SRS15;

        SRS15 &= ~SRS15_DRIVER_TYPE_MASK;

        switch (driver_strengh) {
        case SDCARD_SWITCH_DRIVER_STRENGTH_TYPE_A:
            SRS15 |= SRS15_DRIVER_TYPE_A;
            break;
        case SDCARD_SWITCH_DRIVER_STRENGTH_TYPE_B:
            SRS15 |= SRS15_DRIVER_TYPE_B;
            break;
        case SDCARD_SWITCH_DRIVER_STRENGTH_TYPE_C:
            SRS15 |= SRS15_DRIVER_TYPE_C;
            break;
        case SDCARD_SWITCH_DRIVER_STRENGTH_TYPE_D:
            SRS15 |= SRS15_DRIVER_TYPE_D;
            break;
        default:
            /* nothing */
            break;
        }
        
        MMC->SRS15 = SRS15;

        if (enable_high_speed != MMC_CLEAR)
        {
            /* enable high-speed in the SDIO host controller */
            tmp = MMC->SRS10;
            tmp |= SRS10_HIGH_SPEED_ENABLE;
            MMC->SRS10 = tmp;
        }
        else
        {
            /* disable high-speed in the SDIO host controller */
            tmp = MMC->SRS10;
            tmp &= ~SRS10_HIGH_SPEED_ENABLE;
            MMC->SRS10 = tmp;
        }

        tmp = MMC->SRS15;
        tmp &= ~SRS15_UHS_MODE_MASK;
        tmp |= uhs_mode;
        MMC->SRS15 = tmp;
        ret_status = MSS_MMC_TRANSFER_SUCCESS;
    }

    return (ret_status);
}
/******************************************************************************/
static mss_mmc_status_t sd_host_process_switch_function
(
    uint8_t function_num,
    uint8_t group_num
)
{
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    uint8_t tmp_buffer[BLK_SIZE];
    uint32_t argument;
    uint32_t argumentBase = MMC_CLEAR;

    const uint32_t shift = (((uint32_t)group_num - MMC_SET) * SHIFT_4BIT);

    if (shift < SHIFT_32BIT)
    {
        argumentBase = (uint32_t)(MASK_24BIT & ~(MASK_4BIT << shift))
                       | ((uint32_t)function_num << shift);
    }

    argument = argumentBase | (uint32_t)SDCARD_SWITCH_FUNC_MODE_CHECK;
    /* first check if function is supported */
    ret_status = sd_host_exec_cmd6_command(argument, tmp_buffer, function_num, group_num);
    if (ret_status != MSS_MMC_TRANSFER_SUCCESS)
    {
        ret_status = MSS_MMC_SDCARD_NOT_SUPPORT_BUS_MODE;
    }
    else if (SDCARD_SWICH_FUNC_GET_STAT_CODE(tmp_buffer, group_num) == MASK_4BIT)
    {
        /* if status code for function is 0xF
         then function is not supported by a card */
        ret_status = MSS_MMC_SDCARD_NOT_SUPPORT_BUS_MODE;
    }
    else
    {
        argument = argumentBase | (uint32_t)SDCARD_SWITCH_FUNC_MODE_SWITCH;
        /* execute command to switch function */
        ret_status = sd_host_exec_cmd6_command(argument, tmp_buffer, function_num, group_num);
        if (ret_status != MSS_MMC_TRANSFER_SUCCESS)
        {
            ret_status = MSS_MMC_SDCARD_NOT_SUPPORT_BUS_MODE;
        }
        else if (SDCARD_SWICH_FUNC_GET_STAT_CODE(tmp_buffer, group_num) != function_num) {
            /* Status indicates the same function number as specified in the argument,
               which means Supported function successful function change. */

            ret_status = MSS_MMC_SDCARD_CMD6_SWITCH_ERROR;
        } else {
            ret_status = MSS_MMC_TRANSFER_SUCCESS;
        }
    }
    return (ret_status);
}
/******************************************************************************/
static mss_mmc_status_t sd_tuning(void)
{
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    uint8_t repeat_count;
    uint32_t tmp;
    uint32_t read_pattern[BYTES_128];
    tmp = MMC->SRS15;

    /* reset tuning unit */
    tmp &= ~SRS15_SAMPLING_CLOCK_SELECT;
    /* start of tuning */
    tmp |= SRS15_EXECUTE_TUNING;
    MMC->SRS15 = tmp;

    repeat_count = BYTES_40;
    while ((tmp & SRS15_EXECUTE_TUNING) != MMC_CLEAR)
    {
        ret_status = read_tune_block(read_pattern, BYTES_64, SD_CMD_19_SEND_TUNING_BLK);

        if ((repeat_count == MMC_CLEAR) || (ret_status != MSS_MMC_TRANSFER_SUCCESS))
        {
            break;
        }

        tmp = MMC->SRS15;
        --repeat_count;
    }

    if ((ret_status == MSS_MMC_TRANSFER_SUCCESS) && ((tmp & SRS15_SAMPLING_CLOCK_SELECT) == MMC_CLEAR))
    {
        ret_status = MSS_MMC_SDCARD_TUNING_FAILED;
    }

    /* tuning completed */
    return (ret_status);
}
/******************************************************************************/
static mss_mmc_status_t sd_host_exec_cmd6_command
(
    uint32_t argument,
    uint8_t *buffer,
    uint8_t function_num,
    uint8_t group_num
)
{

    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    uint32_t timeout = COMMANDS_TIMEOUT;
    uint8_t do_continue = MMC_SET;
    do 
    {
        ret_status = cmd6_single_block_read(argument, (uint32_t *)buffer, BYTES_64);
        if (MSS_MMC_TRANSFER_SUCCESS != ret_status)
        {
            break;
        }
        /* check data structure version
          if it is 1 then checking the busy status is possible */
        if (buffer[17u] == MMC_SET)
        {
            /* if function is busy continue loop
            wait until function will be ready */
            if (SDCARD_SWICH_FUNC_GET_BUSY_STAT(buffer, group_num, function_num) == MMC_CLEAR)
            {
                do_continue = MMC_CLEAR;
            }
        } 
        else
        {
            do_continue = MMC_CLEAR;
        }
        --timeout;
    } while ((timeout != MMC_CLEAR) && (do_continue != MMC_CLEAR));

    if (timeout == MMC_CLEAR)
    {
        ret_status = MSS_MMC_TRANSFER_FAIL;
    } else {
        ret_status = MSS_MMC_TRANSFER_SUCCESS;
    }
    return ret_status;
}
/******************************************************************************/
static mss_mmc_status_t sd_card_uhsi_supported(void)
{
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    const uint8_t group_num = MMC_SET;
    uint8_t function_num = SDCARD_SWITCH_ACCESS_MODE_SDR50;
    uint32_t data_c6[BYTES_128];
    uint8_t *tmp_buffer = (uint8_t *)data_c6;
    uint32_t tmp;
    uint32_t argument = MMC_CLEAR;
    const uint32_t shift = ((group_num - MMC_SET) * SHIFT_4BIT);

    if (shift < SHIFT_32BIT)
    {
        argument = (uint32_t)(MASK_24BIT & ~(MASK_4BIT << shift))
                   | ((uint32_t)function_num << shift)
                   | (uint32_t)SDCARD_SWITCH_FUNC_MODE_CHECK;
    }
    ret_status = cmd6_single_block_read(argument, data_c6, BYTES_64);
    if (ret_status != MSS_MMC_TRANSFER_SUCCESS)
    {
        ret_status = MSS_MMC_SDCARD_NOT_SUPPORT_BUS_MODE;
    }
    else if (SDCARD_SWICH_FUNC_GET_STAT_CODE(tmp_buffer, group_num) == MASK_24BIT)
    {
        /* if status code for function is 0xF
         then function is not supported by a card */
        ret_status = MSS_MMC_SDCARD_NOT_SUPPORT_BUS_MODE;
    }
    else if (SDCARD_SWICH_FUNC_GET_STAT_CODE(tmp_buffer, group_num) == function_num)
    {
        MMC->SRS10 |= SRS10_DATA_WIDTH_4BIT;

        /* disable sd clock */
        tmp = MMC->SRS11;
        tmp &= ~(uint32_t)SRS11_SD_CLOCK_ENABLE;
        MMC->SRS11 = tmp;
        /* 1.8V signal enable */
        tmp = MMC->SRS15;
        tmp |= SRS15_18V_ENABLE;
        MMC->SRS15 = tmp;

        /* wait 5ms */
        mmc_delay(DELAY_COUNT);
        mmc_delay(DELAY_COUNT);

        /* if 1.8V signal enable is cleared by host return with error */
        tmp = MMC->SRS15;
        if ((tmp & SRS15_18V_ENABLE) == MMC_CLEAR)
        {
             ret_status = MSS_MMC_ERR_SWITCH_VOLTAGE_FAILED;
        }
        else
        {
            /* internal clock enable */
            tmp = MMC->SRS11;
            tmp |= (uint32_t)SRS11_INT_CLOCK_ENABLE;
            MMC->SRS11 = tmp;
            while (MMC_CLEAR == (MMC->SRS11 & SRS11_INT_CLOCK_STABLE));

            /* enable sd clock */
            tmp = MMC->SRS11;
            tmp |= (uint32_t)SRS11_SD_CLOCK_ENABLE;
            MMC->SRS11 = tmp;
            /* wait 1ms */
            mmc_delay(DELAY_COUNT);

            ret_status = MSS_MMC_TRANSFER_SUCCESS;
        }
    }
    else
    {
        ret_status = MSS_MMC_TRANSFER_FAIL;
    }
    return (ret_status);
}

/******************************************************************************/
static uint8_t is_uhsi_supported(void)
{
    uint8_t result = MMC_SET;
    uint32_t tmp;

    tmp = MMC->SRS16;

    if ((tmp & SRS16_VOLTAGE_1_8V_SUPPORT) == MMC_CLEAR)
    {
        result = MMC_CLEAR;
    }
    else
    {
        tmp = MMC->SRS17;

        uint32_t uhsi_modes = (uint32_t)(SRS17_SDR50_SUPPORTED | SRS17_SDR104_SUPPORTED
                                        | SRS17_DDR50_SUPPORTED);
        if ((tmp & uhsi_modes) == MMC_CLEAR)
        {
            result = MMC_CLEAR;
        }
    }
    return (result);
}
/******************************************************************************/
static mss_mmc_status_t set_device_hs400_mode(const mss_mmc_cfg_t *cfg)
{
    cif_response_t response_status = TRANSFER_IF_SUCCESS;
    uint32_t srs10, hrs6;
    uint32_t hs_timing = MMC_HS200_MODE;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    uint32_t csd_reg[BLK_SIZE/WORD_SIZE];
    uint8_t  *pcsd_reg;

    uint32_t hw_sec_count;
    uint8_t  hw_ext_csd_rev;
    uint8_t  hw_device_type;
    uint8_t  hw_hs_timing;

    uint32_t data_width = cfg->data_bus_width;

    if (MSS_MMC_DATA_WIDTH_8BIT != data_width)
    {
        ret_status = MSS_MMC_TRANSFER_FAIL;
        return ret_status;
    }

    /* set 8-bit bus mode */
    srs10 = MMC->SRS10;
    srs10 |= SRS10_EXTENDED_DATA_TRANSFER_WIDTH;
    MMC->SRS10 = srs10;
    /* do HS400 Mode Operation, not HS400_ES  */
    if (MSS_MMC_MODE_HS400 == cfg->bus_speed_mode)
    {
        /* Set MMC data bus width */
        response_status = cif_send_cmd(MMC_DW_CSD | (data_width << SHIFT_8BIT),
                                MMC_CMD_6_SWITCH,
                                MSS_MMC_RESPONSE_R1B);
    /*
     * Writing to the EXT CSD register takes significant time, so function must
     * not return until the 'READY FOR DATA' bit is set in the Card Status
     * Register.
     */
        while (DEVICE_BUSY == response_status)
        {
            response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                MMC_CMD_13_SEND_STATUS,
                                MSS_MMC_RESPONSE_R1);
        }

        hs_timing = MMC_HS200_MODE;

        response_status = cif_send_cmd(hs_timing,
                                   MMC_CMD_6_SWITCH,
                                   MSS_MMC_RESPONSE_R1B);

        if ((TRANSFER_IF_SUCCESS == response_status)||(DEVICE_BUSY == response_status))
        {
            do
            {
                response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                            MMC_CMD_13_SEND_STATUS,
                                            MSS_MMC_RESPONSE_R1);
            } while (DEVICE_BUSY == response_status);
        }

        if (TRANSFER_IF_SUCCESS == response_status)
        {
            /* eMMC Mode select in Host controller */
            hrs6 = MMC->HRS06;
            hrs6 &= ~MSS_MMC_MODE_MASK;
            hrs6 |= MSS_MMC_MODE_HS200;
            MMC->HRS06 = hrs6;
            /* Up the Host MMC clock frequency */
            set_host_sdclk(cfg->clk_rate);
            /* HS200 tuning */
            ret_status = execute_tunning_mmc(MSS_MMC_DATA_WIDTH_8BIT);
    
            ret_status = MSS_MMC_single_block_read(READ_SEND_EXT_CSD, csd_reg);
            if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
            {
                pcsd_reg = (uint8_t *)csd_reg;
                /* offsets defined in JESD84-B51 extended CSD */
                hw_sec_count   = csd_reg[EXT_CSD_SECTOR_COUNT_OFFSET/WORD_SIZE];
                hw_ext_csd_rev = pcsd_reg[EXT_CSD_REVISION_OFFSET] & BYTE_MASK;
                hw_hs_timing   = pcsd_reg[EXT_CSD_HS_TIMING_OFFSET];
            }
            else
            {
                ret_status = MSS_MMC_TRANSFER_FAIL;
                return ret_status;
            }
        }
        if (TRANSFER_IF_SUCCESS != response_status)
        {
            ret_status = MSS_MMC_TRANSFER_FAIL;
            return ret_status;
        }
    }

    /* High Speed Mode set */
    hs_timing = MMC_HS_MODE;
    
    response_status = cif_send_cmd(hs_timing,
                                MMC_CMD_6_SWITCH,
                                MSS_MMC_RESPONSE_R1B);
    
    if ((TRANSFER_IF_SUCCESS == response_status)||(DEVICE_BUSY == response_status))
    {
        do
        {
            response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                            MMC_CMD_13_SEND_STATUS,
                                            MSS_MMC_RESPONSE_R1);
        } while (DEVICE_BUSY == response_status);
    }

    if (TRANSFER_IF_SUCCESS == response_status)
    {
        set_host_sdclk(MSS_MMC_CLOCK_50MHZ);
        /* Enable Host High Speed */
        MMC->SRS10 |= (MMC_SET << SHIFT_2BIT);
        
        if (MSS_MMC_MODE_HS400 == cfg->bus_speed_mode)
        {
            data_width = MMC_DUAL_DATA_WIDTH_8BIT;
        }
        else if (MSS_MMC_MODE_HS400_ES == cfg->bus_speed_mode)
        {
            /* Enable strobe support*/
            data_width = MMC_ES_DUAL_DATA_WIDTH_8BIT;
        }
        else
        {
            data_width = MMC_DUAL_DATA_WIDTH_8BIT;
        }
        /* Set MMC data bus width */
        response_status = cif_send_cmd(MMC_DW_CSD | (data_width << SHIFT_8BIT),
                                        MMC_CMD_6_SWITCH,
                                        MSS_MMC_RESPONSE_R1B);
        /*
        * Writing to the EXT CSD register takes significant time, so function must
        * not return until the 'READY FOR DATA' bit is set in the Card Status
        * Register.
        */
        while (DEVICE_BUSY == response_status)
        {
            response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                        MMC_CMD_13_SEND_STATUS,
                                        MSS_MMC_RESPONSE_R1);
        }
        /* eMMC Mode select in Host controller */
        hrs6 = MMC->HRS06;
        hrs6 &= ~MSS_MMC_MODE_MASK;
        hrs6 |= MSS_MMC_MODE_DDR;
        MMC->HRS06 = hrs6;

        set_host_sdclk(MSS_MMC_CLOCK_50MHZ);

        ret_status = MSS_MMC_single_block_read(READ_SEND_EXT_CSD, csd_reg);
        if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
        {
            pcsd_reg = (uint8_t *)csd_reg;
            /* offsets defined in JESD84-B51 extended CSD */
            hw_device_type   =  pcsd_reg[EXT_CSD_CARD_TYPE_OFFSET];
            hw_sec_count   = csd_reg[EXT_CSD_SECTOR_COUNT_OFFSET/WORD_SIZE];
            hw_ext_csd_rev = pcsd_reg[EXT_CSD_REVISION_OFFSET] & BYTE_MASK;
            hw_hs_timing   = pcsd_reg[EXT_CSD_HS_TIMING_OFFSET];
        }

        /* Set HS400 mode */
        hs_timing = MMC_HS400_MODE;
        response_status = cif_send_cmd(hs_timing,
                                    MMC_CMD_6_SWITCH,
                                    MSS_MMC_RESPONSE_R1B);

        if ((TRANSFER_IF_SUCCESS == response_status)||(DEVICE_BUSY == response_status))
        {
            do
            {
                response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                                MMC_CMD_13_SEND_STATUS,
                                                MSS_MMC_RESPONSE_R1);
            } while (DEVICE_BUSY == response_status);
        }

        if (TRANSFER_IF_SUCCESS == response_status)
        {
            /* eMMC Mode select in Host controller */
            hrs6 = MMC->HRS06;
            hrs6 &= ~MSS_MMC_MODE_MASK;
            if (MSS_MMC_MODE_HS400 == cfg->bus_speed_mode)
            {
                hrs6 |= MSS_MMC_MODE_HS400;
            }
            else if (MSS_MMC_MODE_HS400_ES == cfg->bus_speed_mode)
            {
                hrs6 |= MSS_MMC_MODE_HS400_ES;
            }
            else
            {
                hrs6 = MMC_CLEAR;
            }

            MMC->HRS06 = hrs6;
            /* Up the Host MMC clock frequency */
            set_host_sdclk(cfg->clk_rate);
            ret_status = MSS_MMC_single_block_read(MMC_CLEAR, (uint32_t *)csd_reg);
            if (MSS_MMC_TRANSFER_SUCCESS != ret_status)
            {
                ret_status = MSS_MMC_TRANSFER_FAIL;
            }
        }
    }
    if (TRANSFER_IF_SUCCESS != response_status)
    {
        ret_status = MSS_MMC_TRANSFER_FAIL;
    }
    return ret_status;
}
/******************************************************************************/
static mss_mmc_status_t device_set_hs_timing
(
    uint32_t hs_mode,
    const mss_mmc_cfg_t * cfg
)
{
    cif_response_t response_status = TRANSFER_IF_SUCCESS;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;
    uint32_t srs10, hrs6;
    uint32_t hs_timing = MMC_HS_MODE_DEFAULT;
    uint8_t phy_delay_type;

    uint32_t data_width = cfg->data_bus_width;

    /* set 4/8-bit bus mode */
    srs10 = MMC->SRS10;
    if (data_width == MSS_MMC_DATA_WIDTH_4BIT)
    {
        srs10 |= SRS10_DATA_WIDTH_4BIT;
    }
    else if (data_width == MSS_MMC_DATA_WIDTH_8BIT)
    {
        srs10 |= SRS10_EXTENDED_DATA_TRANSFER_WIDTH;
    }
    else
    {
        srs10 |= SRS10_DATA_WIDTH_4BIT;
    }
    MMC->SRS10 = srs10;

    switch (hs_mode)
    {
        case DEVICE_SUPPORT_LEGACY:
            hs_timing = MMC_LEGACY_MODE;
            phy_delay_type = MSS_MMC_PHY_DELAY_INPUT_MMC_LEGACY;
            if (cfg->clk_rate > MSS_MMC_CLOCK_26MHZ)
            {
                response_status = TRANSFER_IF_FAIL;
            }
            break;
        case DEVICE_SUPPORT_SDR:
            hs_timing = MMC_HS_MODE;
            phy_delay_type = MSS_MMC_PHY_DELAY_INPUT_MMC_SDR;
            break;
        case DEVICE_SUPPORT_DDR:
            hs_timing = MMC_HS_MODE;
            phy_delay_type = MSS_MMC_PHY_DELAY_INPUT_MMC_DDR;
            if (MSS_MMC_DATA_WIDTH_8BIT == data_width)
            {
                data_width = MMC_DUAL_DATA_WIDTH_8BIT;
            }
            else if (MSS_MMC_DATA_WIDTH_4BIT == data_width)
            {
                data_width = MMC_DUAL_DATA_WIDTH_4BIT;
            }
            else
            {
                response_status = TRANSFER_IF_FAIL;
            }
            break;
        case DEVICE_SUPPORT_HS200:
            hs_timing = MMC_HS200_MODE;
            phy_delay_type = MSS_MMC_PHY_DELAY_DLL_HS_SDCLK;
            if (MSS_MMC_DATA_WIDTH_1BIT == data_width)
            {
                response_status = TRANSFER_IF_FAIL;
            }
            break;
        default:
            response_status = TRANSFER_IF_FAIL;
            break;
    }
    if (TRANSFER_IF_SUCCESS == response_status)
    {
        if (DEVICE_SUPPORT_DDR == hs_mode)
        {
            response_status = cif_send_cmd(hs_timing,
                                           MMC_CMD_6_SWITCH,
                                           MSS_MMC_RESPONSE_R1B);

            if ((TRANSFER_IF_SUCCESS == response_status)||(DEVICE_BUSY == response_status))
            {
                do
                {
                    response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                                    MMC_CMD_13_SEND_STATUS,
                                                    MSS_MMC_RESPONSE_R1);
                } while (DEVICE_BUSY == response_status);
            }
            /* Set MMC data bus width */
            response_status = cif_send_cmd(MMC_DW_CSD | (data_width << SHIFT_8BIT),
                                            MMC_CMD_6_SWITCH,
                                            MSS_MMC_RESPONSE_R1B);
            /*
            * Writing to the EXT CSD register takes significant time, so function must
            * not return until the 'READY FOR DATA' bit is set in the Card Status
            * Register.
            */
            while (DEVICE_BUSY == response_status)
            {
                response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                            MMC_CMD_13_SEND_STATUS,
                                            MSS_MMC_RESPONSE_R1);
            }

        }
        else if (DEVICE_SUPPORT_LEGACY == hs_mode)
        {
            /* Set MMC data bus width */
            response_status = cif_send_cmd(MMC_DW_CSD | (data_width << SHIFT_8BIT),
                                        MMC_CMD_6_SWITCH,
                                        MSS_MMC_RESPONSE_R1B);
            /*
             * Writing to the EXT CSD register takes significant time, so function must
             * not return until the 'READY FOR DATA' bit is set in the Card Status
             * Register.
             */
            while (DEVICE_BUSY == response_status)
            {
                response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                        MMC_CMD_13_SEND_STATUS,
                                        MSS_MMC_RESPONSE_R1);
            }
        }
        else
        {
            /* Set MMC data bus width */
            response_status = cif_send_cmd(MMC_DW_CSD | (data_width << SHIFT_8BIT),
                                        MMC_CMD_6_SWITCH,
                                        MSS_MMC_RESPONSE_R1B);
            /*
             * Writing to the EXT CSD register takes significant time, so function must
             * not return until the 'READY FOR DATA' bit is set in the Card Status
             * Register.
             */
            while (DEVICE_BUSY == response_status)
            {
                response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                        MMC_CMD_13_SEND_STATUS,
                                        MSS_MMC_RESPONSE_R1);
            }
            response_status = cif_send_cmd(hs_timing,
                                           MMC_CMD_6_SWITCH,
                                           MSS_MMC_RESPONSE_R1B);

            if ((TRANSFER_IF_SUCCESS == response_status)||(DEVICE_BUSY == response_status))
            {
                do
                {
                    response_status = cif_send_cmd(sdcard_RCA << RCA_SHIFT_BIT,
                                                    MMC_CMD_13_SEND_STATUS,
                                                    MSS_MMC_RESPONSE_R1);
                } while (DEVICE_BUSY == response_status);
            }
        }

        if (TRANSFER_IF_SUCCESS == response_status)
        {
            /* eMMC Mode select in Host controller */
            hrs6 = MMC->HRS06;
            hrs6 &= ~MSS_MMC_MODE_MASK;
            hrs6 |= cfg->bus_speed_mode;
            MMC->HRS06 = hrs6;
            /* Up the Host MMC clock frequency */
            set_host_sdclk(cfg->clk_rate);
            /* Set Phy delay for select MMC mode */
            phy_training_mmc(phy_delay_type, cfg->clk_rate);
            ret_status = MSS_MMC_TRANSFER_SUCCESS;
        }
    }
    if (TRANSFER_IF_SUCCESS != response_status)
    {
        ret_status = MSS_MMC_TRANSFER_FAIL;
    }
    return ret_status;
}
/******************************************************************************/
static uint8_t set_host_sdclk(uint32_t frequencyKHz)
{
    uint32_t temp, temp1;
    uint32_t baseclkkHz;
    uint32_t i;
    uint32_t setFreqKhz;

    /* disable sd clock */
    temp = MMC->SRS11;
    temp &= ~(uint32_t)SRS11_SD_CLOCK_ENABLE;
    MMC->SRS11 = temp;
    /* read base clock frequency for SD clock in kilo herz */
    temp = MMC->SRS16;
    baseclkkHz = ((temp & 0x0000FF00u) >> 8U) * 1000u;

    if (baseclkkHz == 0u)
    {
        return 1u;
    }
    /* read current value of SRS11 register */
    temp = MMC->SRS11;
    /* clear old frequency base settings */
    temp &= ~(uint32_t)SRS11_SEL_FREQ_BASE_MASK;

    /* 10-bit Divider Clock Mode */
    for (i = 1u; i < 2046u; i++)
    {
        if (((baseclkkHz / i) < frequencyKHz)
            || (((baseclkkHz / i) == frequencyKHz) && ((baseclkkHz % i) == 0u)))
        {
            break;
        }
    }
    temp1 = ((i / 2u) << 8);
    /* Set SDCLK Frequency Select and Internal Clock Enable */
    temp |= (temp1 & 0xFF00u) | ((temp1 & 0x30000u) >> 10)
            | (uint32_t)SRS11_INT_CLOCK_ENABLE;
    temp &= ~(uint32_t)SRS11_CLOCK_GENERATOR_SELECT;
    setFreqKhz = baseclkkHz / i;

    MMC->SRS11 = temp;

    while ((MMC->SRS11 & SRS11_INT_CLOCK_STABLE) == 0u);

    /* enable sd clock */
    temp = MMC->SRS11;
    temp |= (uint32_t)SRS11_SD_CLOCK_ENABLE;
    MMC->SRS11 = temp;
    return 0;
}
/******************************************************************************/
static mss_mmc_status_t set_data_timeout(uint32_t timeout_val_us)
{
    mss_mmc_status_t status = MSS_MMC_NO_ERROR;
    uint32_t temp, sdmclk_khz, sdmclk_Mhz, timeout_interval;
    uint8_t j;
    uint32_t sdmclk, timeoutVal;

    temp = MMC->SRS16;
    sdmclk_khz = (temp & 0x0000003Fu);/* 0x4u; - 0x4 is dummy -> 50Mhz * 4 = 200Mhz, */

    if (((temp & SRS16_TIMEOUT_CLOCK_UNIT_MHZ) == 0u) && (timeout_val_us < 1000u))
    {

        status = MSS_MMC_INVALID_PARAMETER;
    }
    else if (sdmclk_khz == 0u)
    {
        status = MSS_MMC_BASE_CLK_IS_ZERO_ERR;
    }
    else
    {
        if ((temp & SRS16_TIMEOUT_CLOCK_UNIT_MHZ) != 0u)
        {
            sdmclk_khz *= 1000u;
        }
        sdmclk_Mhz = sdmclk_khz / 1000u;

        if (sdmclk_Mhz == 0u)
        {
            sdmclk = sdmclk_khz;
            timeoutVal = timeout_val_us / 1000u;
        }
        else
        {
            sdmclk = sdmclk_Mhz;
            timeoutVal = timeout_val_us;
        }
        /* calculate data Timeout Counter Value */
        timeout_interval = 8192u/*2 ^ 13*/;
        for (j = 0u; j < 15u; j++)
        {
            /* if  (timeoutVal < ((1 / sdmclk) * timeout_interval)) */
            if (timeoutVal < (timeout_interval / sdmclk))
            {
                break;
            }
            timeout_interval *= 2u;
        }
        timeout_interval = (uint32_t)j << 16u;

        temp = MMC->SRS11;
        temp &= (uint32_t)~SRS11_TIMEOUT_MASK;
        temp |= timeout_interval;
        MMC->SRS11 = temp;
    }
    return status;
}
/******************************************************************************/
static mss_mmc_status_t  set_sdhost_power(uint32_t voltage)
{
    uint32_t temp, srs16,delay;
    mss_mmc_status_t status = MSS_MMC_NO_ERROR;

        /* disable SD bus power */
    temp = MMC->SRS10;
    temp &= ~SRS10_SD_BUS_POWER;
    MMC->SRS10 = temp;

    /*clear current voltage settings*/
    temp &= ~SRS10_BUS_VOLTAGE_MASK;
    srs16 = MMC->SRS16;

    /* if Voltage == 0
     * disable bus power
     * power is disabled so do nothing, return no error
     */
    if (voltage != MMC_CLEAR)
    {
        switch (voltage)
        {
            case SRS10_SET_3_3V_BUS_VOLTAGE:
                if ((srs16 & SRS16_VOLTAGE_3_3V_SUPPORT) == MMC_CLEAR)
                {
                   status = MSS_MMC_INVALID_PARAMETER;
                    break;
                }
                /* set new voltage value */
                temp |= (SRS10_SET_3_3V_BUS_VOLTAGE | SRS10_SD_BUS_POWER);
                break;
            case SRS10_SET_3_0V_BUS_VOLTAGE:
                if ((srs16 & SRS16_VOLTAGE_3_0V_SUPPORT) == MMC_CLEAR)
                {
                    status = MSS_MMC_INVALID_PARAMETER;
                    break;
                }
                /* set new voltage value */
                temp |= (SRS10_SET_3_0V_BUS_VOLTAGE | SRS10_SD_BUS_POWER);
                break;
            case SRS10_SET_1_8V_BUS_VOLTAGE:
                if ((srs16 & SRS16_VOLTAGE_1_8V_SUPPORT) == MMC_CLEAR)
                {
                    status = MSS_MMC_INVALID_PARAMETER;
                    break;
                }
                /* set new voltage value */
                temp |= (SRS10_SET_1_8V_BUS_VOLTAGE | SRS10_SD_BUS_POWER);
                break;
            default:
                status = MSS_MMC_INVALID_PARAMETER;
                break;
        }

        if (status == MSS_MMC_NO_ERROR)
        {
            MMC->SRS10 = temp;
        }
        mmc_delay(DELAY_COUNT);
    }
    return (status);
}
/******************************************************************************/
static void mmc_delay(uint32_t value)
{
    while (value--);
}
/******************************************************************************/
static mss_mmc_status_t cmd6_single_block_read
(
    uint32_t src_addr,
    uint32_t * dst_addr,
    uint32_t size
)
{
    uint32_t isr_errors;
    uint32_t blk_read, srs03_data, srs9;
    uint16_t word_cnt = (BLK_SIZE/WORD_SIZE);
    uint32_t idx_cnt = MMC_CLEAR;

    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;

    /* Block length and count*/
    MMC->SRS01 = (size | (MMC_SET << BLOCK_COUNT_ENABLE_SHIFT));

    /* DPS, Data transfer direction - read */
    srs03_data = (uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_READ |SRS3_BLOCK_COUNT_ENABLE
                        | SRS3_RESP_ERR_CHECK_EN | SRS3_RESP_INTER_DISABLE
                        | SRS3_RESPONSE_CHECK_TYPE_R1 | SRS3_RESP_LENGTH_48
                        | SRS3_CRC_CHECK_EN | SRS3_INDEX_CHECK_EN);
    /* Check cmd and data line busy */
    do
    {
        srs9 = MMC->SRS09;
    }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT)) != MMC_CLEAR);

    word_cnt = (size/WORD_SIZE);
            /* Command argument */
    MMC->SRS02 = src_addr;
   /* execute command */
    MMC->SRS03 = (uint32_t)((SD_CMD_6 << MMC_SRS03_COMMAND_SHIFT) | srs03_data);

    mmc_delay(DELAY_COUNT);

    idx_cnt = MMC_CLEAR;
    blk_read = MMC->SRS12;
    /* Read in the contents of the Buffer */
    while (((blk_read & SRS12_BUFFER_READ_READY) != MMC_CLEAR) && (word_cnt > (BUFF_EMPTY)))
    {
        dst_addr[idx_cnt] = MMC->SRS08;
        ++idx_cnt;
        --word_cnt;
        blk_read = MMC->SRS12;
    }
    /* Check a block read is complete */
    do
    {
        blk_read = MMC->SRS12;
    } while (MMC_CLEAR == (blk_read & SRS12_TRANSFER_COMPLETE));

    isr_errors = MMC->SRS12;
    /* Abort if any errors*/
    if ((SRS12_ERROR_STATUS_MASK & isr_errors) == 0u)
    {
        ret_status = MSS_MMC_TRANSFER_SUCCESS;
    }
    else
    {

        ret_status = MSS_MMC_ERR_INTERRUPT;
    }
    /* Clear all status interrupts except:
    *  current limit error, card interrupt, card removal, card insertion */
    MMC->SRS12 = ~(SRS12_CURRENT_LIMIT_ERROR
                            | SRS12_CARD_INTERRUPT
                            | SRS12_CARD_REMOVAL
                            | SRS12_CARD_INSERTION);
    return (ret_status);
}
/******************************************************************************/
static mss_mmc_status_t execute_tunning_mmc(uint8_t data_width)
{
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;

    uint32_t ReadPattern[128u];
    uint8_t i, j, PatternOk[40u];

    uint8_t BufferSize =  (data_width == MSS_MMC_DATA_WIDTH_4BIT)? 64u:128u;
    uint32_t const *WritePattern = calc_write_pattern(data_width);

    for (j = 0u; j < 40u; j++)
    {
        host_mmc_tune(j);
        mmc_delay(0xFFu);
        for (i = 0u; i < (BufferSize / 4u); i++)
        {
            ReadPattern[i] = 0u;
        }
        ret_status = read_tune_block(ReadPattern, BufferSize, MMC_CMD_21_SEND_TUNE_BLK);
        if (MSS_MMC_TRANSFER_SUCCESS == ret_status)
        {
            /* compare data with pattern */
            PatternOk[j] = 1u;
            for (i = 0u; i < (BufferSize / 4u); i++)
            {
                if (WritePattern[i] != ReadPattern[i])
                {
                    PatternOk[j] = 0u;
                    /* Reset Data and cmd line */
                    MMC->SRS11 |= MMC_RESET_DATA_CMD_LINE;
                    /* read pattern is not correct - exit loop */
                    break;
                }
            }
        }
        else
        {
            /* Reset Data and cmd line */
            MMC->SRS11 |= MMC_RESET_DATA_CMD_LINE;
            PatternOk[j] = 0u;
        }
    }
    uint8_t Pos = calc_longest_valid_delay_chain_val(PatternOk);
    /* Delay value set to Pos */
    host_mmc_tune(Pos);
    ret_status = read_tune_block(ReadPattern, BufferSize, MMC_CMD_21_SEND_TUNE_BLK);
    return ret_status;
}
/******************************************************************************/
static mss_mmc_status_t read_tune_block
(
    uint32_t *read_data,
    uint32_t size,
    uint8_t cmd
)
{

    uint32_t isr_errors;
    uint32_t blk_read, srs03_data, srs9;
    uint16_t word_cnt = (BLK_SIZE/WORD_SIZE);
    uint32_t idx_cnt = MMC_CLEAR;
    mss_mmc_status_t ret_status = MSS_MMC_NO_ERROR;

    /* Clear all status interrupts */
    MMC->SRS12 = MMC_STATUS_CLEAR;
    /* Block length and count*/
    MMC->SRS01 = (size | (MMC_SET << BLOCK_COUNT_ENABLE_SHIFT));
    /* DPS, Data transfer direction - read */
    srs03_data = (uint32_t)(SRS3_DATA_PRESENT | SRS3_TRANS_DIRECT_READ |SRS3_BLOCK_COUNT_ENABLE
                        | SRS3_RESP_ERR_CHECK_EN | SRS3_RESP_INTER_DISABLE
                        | SRS3_RESPONSE_CHECK_TYPE_R1 | SRS3_RESP_LENGTH_48
                        | SRS3_CRC_CHECK_EN | SRS3_INDEX_CHECK_EN);
    /* Check cmd and data line busy */
    do
    {
        srs9 = MMC->SRS09;
    }while ((srs9 & (SRS9_CMD_INHIBIT_CMD | SRS9_CMD_INHIBIT_DAT)) != MMC_CLEAR);
    
    word_cnt = size/WORD_SIZE;
    /* Command argument */
    MMC->SRS02 = MMC_CLEAR;
    /* execute command */
    MMC->SRS03 = (uint32_t)((cmd << MMC_SRS03_COMMAND_SHIFT) | srs03_data);
    mmc_delay(DELAY_COUNT);
    
    idx_cnt = MMC_CLEAR;
    blk_read = MMC->SRS12;
    /* Read in the contents of the Buffer */
    while (((blk_read & SRS12_BUFFER_READ_READY) != MMC_CLEAR) && (word_cnt > (BUFF_EMPTY)))
    {
        read_data[idx_cnt] = MMC->SRS08;
        ++idx_cnt;
        --word_cnt;
        blk_read = MMC->SRS12;
    }
    isr_errors = MMC->SRS12;
    /* Abort if any errors*/
    if ((SRS12_ERROR_STATUS_MASK & isr_errors) == MMC_CLEAR)
    {
        ret_status = MSS_MMC_TRANSFER_SUCCESS;
    }
    else
    {
    
        ret_status = MSS_MMC_ERR_INTERRUPT;
    }
    /* Clear all status interrupts */
      MMC->SRS12 = MMC_STATUS_CLEAR;
    return (ret_status);
}
/******************************************************************************/
static const uint32_t* calc_write_pattern(const uint8_t bus_width)
{
    static const uint32_t emmc_pattern4b[64 / 4] = {
        0x00ff0fffU,
        0xccc3ccffU,
        0xffcc3cc3U,
        0xeffefffeU,
        0xddffdfffU,
        0xfbfffbffU,
        0xff7fffbfU,
        0xefbdf777U,
        0xf0fff0ffU,
        0x3cccfc0fU,
        0xcfcc33ccU,
        0xeeffefffU,
        0xfdfffdffU,
        0xffbfffdfU,
        0xfff7ffbbU,
        0xde7b7ff7U
    };

    static const uint32_t emmc_pattern8b[128 / 4] = {
        0xff00ffffU,
        0x0000ffffU,
        0xccccffffU,
        0xcccc33ccU,
        0xcc3333ccU,
        0xffffccccU,
        0xffffeeffU,
        0xffeeeeffU,
        0xffddffffU,
        0xddddffffU,
        0xbbffffffU,
        0xbbffffffU,
        0xffffffbbU,
        0xffffff77U,
        0x77ff7777U,
        0xffeeddbbU,
        0x00ffffffU,
        0x00ffffffU,
        0xccffff00u,
        0xcc33ccccU,
        0x3333ccccU,
        0xffccccccU,
        0xffeeffffU,
        0xeeeeffffU,
        0xddffffffU,
        0xddffffffU,
        0xffffffddU,
        0xffffffbbU,
        0xffffbbbbU,
        0xffff77ffU,
        0xff7777ffU,
        0xeeddbb77U
    };

    const uint32_t* write_pattern = NULL_POINTER;

    if ((uint8_t)MSS_MMC_DATA_WIDTH_8BIT == bus_width) {
        write_pattern = (const uint32_t*)emmc_pattern8b;
    }
    if ((uint8_t)MSS_MMC_DATA_WIDTH_4BIT == bus_width) {
        write_pattern = (const uint32_t*)emmc_pattern4b;
    }

    return (write_pattern);
}
/******************************************************************************/
static void host_mmc_tune(uint8_t value)
{
    uint32_t hrs6, read_data;

    hrs6 = MMC->HRS06;
    hrs6 &= ~HRS6_EMMC_TUNE_VALUE_MASK;
    hrs6 |= (uint32_t)((value << SHIFT_8BIT) | HRS6_EMMC_TUNE_REQUEST);
    MMC->HRS06 = hrs6;

    do
    {
        read_data = MMC->HRS06;
    }while ((read_data & HRS6_EMMC_TUNE_REQUEST) != MMC_CLEAR);

}
/******************************************************************************/
static uint8_t calc_longest_valid_delay_chain_val(const uint8_t* pattern_ok)
{
    /* looking for longest valid delay chain value (the best tuning value) */
    uint8_t pos = 0u;
    uint8_t length = 0u;
    uint8_t curr_length = 0u;
    uint8_t i;
    for (i = 0u; i < 40u; i++)
    {
        if (pattern_ok[i] == 1u)
        {
            curr_length++;
            if (curr_length > length)
            {
                pos = i - length;
                length++;
            }
        }
        else 
        {
            curr_length = 0u;
        }
    }
    pos += (length / 2u);

    return (pos);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
