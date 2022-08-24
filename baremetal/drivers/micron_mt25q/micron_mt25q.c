/***************************************************************************//**
 * (c) Copyright 2018 Microsemi SoC Products Group.  All rights reserved.
 *
 * Driver for MICRON_MT25Q QSPI flash memory on Xilinx Emulation board.
 * This dirver uses the PSE MSS QSPI driver interface.
 *
 *
 * SVN $Revision:  $
 * SVN $Date:  $
 */

#include "drivers/mss_qspi/mss_qspi.h"
#include "micron_mt25q.h"
#include "mss_plic.h"
#include "mss_assert.h"
#include "drivers/mss_uart/mss_uart.h"

#ifdef __cplusplus
extern "C" {
#endif


uint32_t status_reg = 0;
uint8_t flag_status_reg = 0;

mss_qspi_config_t beforexip_qspi_config={0};
mss_qspi_config_t qspi_config={0};
mss_qspi_config_t qspi_config_read={0};

static uint8_t g_flash_io_format = 0;
volatile uint8_t xip_en = 0xF3;
volatile uint8_t xip_dis = 0x0;
volatile uint8_t volatile_config_reg = 0u;
static volatile uint8_t g_enh_v_val = 0x0;
static volatile uint16_t g_nh_cfg_val = 0x0;

/*Make sure that the erase operation is complete. i.e. wait for  write enable bit to go 0*/
static void wait_for_wel(void)
{
    status_reg = 0;
    do {
        Flash_read_statusreg((uint8_t*)&status_reg);
    } while(0x02 & status_reg);
}

/*Make sure that the erase operation is complete. i.e. wait for  write enable bit to go 0*/
static void wait_for_wip(void)
{
    status_reg = 0;
    do {
        Flash_read_statusreg((uint8_t*)&status_reg);
    } while(0x01 & status_reg);
}


mss_qspi_io_format Flash_probe_io_format(void)
{
    qspi_config.io_format = MSS_QSPI_QUAD_FULL;
    MSS_QSPI_configure(&qspi_config);
    Flash_readid((uint8_t*)&g_enh_v_val);
    if (0xFF != g_enh_v_val) {
        return MSS_QSPI_QUAD_FULL;
    }

    qspi_config.io_format = MSS_QSPI_DUAL_FULL;
    MSS_QSPI_configure(&qspi_config);
    Flash_readid((uint8_t*)&g_enh_v_val);

    if (0xFF != g_enh_v_val) {
        return MSS_QSPI_DUAL_FULL;
    }

    qspi_config.io_format = MSS_QSPI_NORMAL;
    MSS_QSPI_configure(&qspi_config);

    Flash_readid((uint8_t*)&g_enh_v_val);
    if (0xFF != g_enh_v_val) {
        Flash_read_nvcfgreg((uint8_t*)&g_nh_cfg_val);
        return MSS_QSPI_NORMAL;
    }

    return MSS_QSPI_NORMAL; // ??
}

void Flash_force_normal_mode(void)
{
    qspi_config.io_format = MSS_QSPI_NORMAL;
    MSS_QSPI_configure(&qspi_config);

    Flash_readid((uint8_t*)&g_enh_v_val);
    if (0xFF != g_enh_v_val) {
        Flash_read_nvcfgreg((uint8_t*)&g_nh_cfg_val);
        g_flash_io_format = MSS_QSPI_NORMAL;
        return;
    }

    qspi_config.io_format = MSS_QSPI_DUAL_FULL;
    MSS_QSPI_configure(&qspi_config);
    Flash_readid((uint8_t*)&g_enh_v_val);

    if (0xFF != g_enh_v_val) {
        g_flash_io_format = MSS_QSPI_DUAL_FULL;
        goto push_normal;
    }

    qspi_config.io_format = MSS_QSPI_QUAD_FULL;
    MSS_QSPI_configure(&qspi_config);
    Flash_readid((uint8_t*)&g_enh_v_val);
    if (0xFF != g_enh_v_val) {
        g_flash_io_format = MSS_QSPI_QUAD_FULL;
        goto push_normal;
    }

push_normal:
    {
        qspi_config.io_format = g_flash_io_format;
        MSS_QSPI_configure(&qspi_config);

        Flash_read_enh_v_confreg((uint8_t*)&g_enh_v_val);
        g_enh_v_val = 0xFF;
        Flash_write_enh_v_confreg((uint8_t*)&g_enh_v_val);  //disable quad and DUAL mode

        /*we disabled quad and DUAL mode of the FLASH in previous step.
         * Now bring the QSPI controller to normal mode*/
        qspi_config.io_format = MSS_QSPI_NORMAL;
        MSS_QSPI_configure(&qspi_config);

        do {
            Flash_readid((uint8_t*)&g_enh_v_val);
        } while(0xFF == g_enh_v_val);

        Flash_read_nvcfgreg((uint8_t*)&g_nh_cfg_val);
    }
}

void Flash_init(mss_qspi_io_format io_format)
{
    qspi_config.clk_div =  MSS_QSPI_CLK_DIV_2;
    qspi_config.sample = MSS_QSPI_SAMPLE_POSAGE_SPICLK;
    qspi_config.spi_mode = MSS_QSPI_MODE3;
    qspi_config.xip = MSS_QSPI_DISABLE;

    Flash_read_enh_v_confreg((uint8_t*)&g_enh_v_val);

    if (io_format == MSS_QSPI_QUAD_FULL) {
        g_enh_v_val |= 0x40; // set the dual mode bit
        g_enh_v_val &= ~0x80; //clear the quad mode bit
    } else if (io_format == MSS_QSPI_DUAL_FULL) {
        g_enh_v_val |= 0x80;  //set the quad mode bit
        g_enh_v_val &= ~0x40; //clear the dual mode but
    } else {
        g_enh_v_val |= 0xC0; //normal
    }

    Flash_write_enh_v_confreg((uint8_t*)&g_enh_v_val);

    qspi_config.io_format = io_format;
    MSS_QSPI_configure(&qspi_config);
    Flash_read_enh_v_confreg((uint8_t*)&g_enh_v_val);

    uint32_t maxRetryCount = 1u<<31;
    do {
        Flash_readid((uint8_t*)&g_enh_v_val);

        // we don't want to get stuck forever here, so break out after a significant period of time
        maxRetryCount--; if (!maxRetryCount) { break; }
    } while(0xFF == g_enh_v_val);

    g_flash_io_format = io_format; //store the value for future reference.
}

void Flash_enter_xip(void) {
    uint8_t command_buf[5] __attribute__ ((aligned (4))) = {MICRON_WRITE_ENABLE};
    uint8_t temp =0;

    /*Write enable command must be executed before writing to volatile configuration register*/
    MSS_QSPI_polled_transfer_block(0, command_buf, 0, (uint8_t*)0, 0, 0);

    command_buf[0] = MICRON_WR_V_CONFIG_REG;
    command_buf[1] = xip_en;
    command_buf[2] = 0x00;
    command_buf[3] = 0x00;


    /*Enable XIP by writing to volatile configuration register*/
    MSS_QSPI_polled_transfer_block(0, command_buf, 1, (uint8_t*)0, 0, 0);

    wait_for_wel();
    wait_for_wip();

    /*Drive XIP confirmation using FAST read and keeping DQ0 to 0 during idle cycle*/
    command_buf[0] = MICRON_FAST_READ;
    command_buf[1] = 0x00;
    command_buf[2] = 0x00;
    command_buf[3] = 0x00;

    MSS_QSPI_polled_transfer_block(3, command_buf, 1, &temp, 1, 8);
    MSS_QSPI_get_config(&beforexip_qspi_config);

    beforexip_qspi_config.xip = MSS_QSPI_ENABLE;

    MSS_QSPI_configure(&beforexip_qspi_config);
}

void Flash_exit_xip
(
    void
)
{
    uint8_t command_buf[5] __attribute__ ((aligned (4))) = {MICRON_FAST_READ};

    beforexip_qspi_config.xip = MSS_QSPI_DISABLE;
    MSS_QSPI_configure(&beforexip_qspi_config);

    /*Drive XIP confirmation bit using FAST read and keeping DQ0 to 1 during idle cycle
     * this will exit the XIP*/

    command_buf[0] = MICRON_FAST_READ;
    command_buf[1] = 0x00;
    command_buf[2] = 0x00;
    command_buf[3] = 0xFF;

    MSS_QSPI_polled_transfer_block(3, command_buf, 0, (uint8_t*)&xip_dis, 1, 8);
}

void Flash_read_statusreg
(
    uint8_t* rd_buf
)
{
    uint8_t command_buf[1] __attribute__ ((aligned (4))) = {MICRON_READ_STATUS_REG};
    /*This command works for all modes. No Dummy cycles*/
    MSS_QSPI_polled_transfer_block(0, command_buf, 0, rd_buf, 1,0);
}

void Flash_read_nvcfgreg
(
    uint8_t* rd_buf
)
{
    uint8_t command_buf[1] __attribute__ ((aligned (4))) = {MICRON_READ_NV_CONFIG_REG};

    /*This command works for all modes. No Dummy cycles*/
    MSS_QSPI_polled_transfer_block(0, command_buf, 0, rd_buf, 2,0);
}

void Flash_read_enh_v_confreg
(
    uint8_t* rd_buf
)
{
    uint8_t command_buf[1] __attribute__ ((aligned (4))) = {MICRON_READ_ENH_V_CONFIG_REG};

    /*This command works for all modes. No Dummy cycles*/
    MSS_QSPI_polled_transfer_block(0, command_buf, 0, rd_buf, 1,0);
}


void Flash_write_enh_v_confreg
(
    uint8_t* enh_v_val
)
{
    uint8_t command_buf[2] __attribute__ ((aligned (4))) = {MICRON_WRITE_ENABLE};
    /*execute Write enable command again for writing the data*/
    MSS_QSPI_polled_transfer_block(0, command_buf, 0, (uint8_t*)0, 0, 0);

    command_buf[0] =  MICRON_WR_ENH_V_CONFIG_REG;
    command_buf[1] =  *enh_v_val;

    /*This command works for all modes. No Dummy cycles*/
    MSS_QSPI_polled_transfer_block(0, command_buf, 1, (uint8_t*)0, 0, 0);
}

void Flash_read_flagstatusreg
(
    uint8_t* rd_buf
)
{
    uint8_t command_buf[1] __attribute__ ((aligned (4))) = {MICRON_READ_FLAG_STATUS_REG};

    /*This command works for all modes. No Dummy cycles*/
    MSS_QSPI_polled_transfer_block(0, command_buf, 0, rd_buf, 1,0);
}

void Flash_enter_normal_mode
(
    void
)
{
    uint8_t enh_v_val = 0x0;

    Flash_read_enh_v_confreg(&enh_v_val);
    enh_v_val |= 0xC0; //quad
    Flash_write_enh_v_confreg(&enh_v_val);
}


void Flash_readid
(
    uint8_t* rd_buf
)
{
    uint8_t command_buf[1] __attribute__ ((aligned (4))) = {MICRON_MIO_READ_ID_OPCODE};

    /*This command works for all modes. No Dummy cycles.
     * MICRON_READ_ID_OPCODE works only for Normal mode*/
    MSS_QSPI_polled_transfer_block(0, command_buf, 0, rd_buf, 1,0);
}

void Flash_read(uint8_t* buf, uint32_t read_addr, uint32_t read_len)
{
    uint8_t command_buf[4] __attribute__ ((aligned (4))) = {MICRON_FAST_READ};
    command_buf[1] = (read_addr >> 16) & 0xFF;
    command_buf[2] = (read_addr >> 8) & 0xFF;
    command_buf[3] = read_addr & 0xFF;

    if(MSS_QSPI_NORMAL == g_flash_io_format) {
        MSS_QSPI_polled_transfer_block(3, command_buf, 0, buf, read_len,8);
    } else {
        if(g_flash_io_format & 0x01) //quad
            MSS_QSPI_polled_transfer_block(3, command_buf, 0, buf, read_len,10);
        else
            MSS_QSPI_polled_transfer_block(3, command_buf, 0, buf, read_len, 8);
    }
}

uint8_t Flash_program(uint8_t* buf, uint32_t wr_addr, uint32_t wr_len)
{
    uint8_t command_buf[512] __attribute__ ((aligned (4))) = {0};
    /*All commands in this function work in all modes....*/

    wait_for_wel();
    wait_for_wip();

    /*execute Write enable command again for writing the data*/
    command_buf[0] = MICRON_WRITE_ENABLE;
    MSS_QSPI_polled_transfer_block(0, command_buf, 0, (uint8_t*)0, 0,0);

    /*This command works for all modes. No Dummy cycles*/
    /*now program the sector. This will set the desired bits to 0.*/

    command_buf[0] = MICRON_PAGE_PROGRAM;
    command_buf[1] = (wr_addr >> 16) & 0xFF;
    command_buf[2] = (wr_addr >> 8) & 0xFF;
    command_buf[3] = wr_addr & 0xFF;

    for(uint16_t idx=0; idx< wr_len;idx++)
        command_buf[4 + idx] = *(uint8_t*)(buf+idx);

    MSS_QSPI_polled_transfer_block(3, command_buf, wr_len, (uint8_t*)0, 0,0);

    Flash_read_flagstatusreg(&flag_status_reg);

    wait_for_wel();
    wait_for_wip();

    return flag_status_reg;
}

/*Micron sector size is = 64k (65536 bytes).
 * addr parameter value should be any address  within the sector that needs to be erased */
void Flash_sector_erase(uint32_t addr)
{
    uint8_t command_buf[5] __attribute__ ((aligned (4))) = {MICRON_WRITE_ENABLE};

    /*This command works for all modes. No Dummy cycles*/
    /*Write enable command must be executed before erase*/
    MSS_QSPI_polled_transfer_block(0, command_buf, 0, (uint8_t*)0, 0,0);

    /*This command works for all modes. No Dummy cycles*/
    /*Erase the sector. This will write 1 to all bits*/
    command_buf[0] = MICRON_DIE_ERASE;
    command_buf[1] = (addr >> 16) & 0xFF;
    command_buf[2] = (addr >> 8) & 0xFF;
    command_buf[3] = addr & 0xFF;


    MSS_QSPI_polled_transfer_block(3, command_buf, 0, (uint8_t*)0, 0,0);
}

void Flash_die_erase(void)
{
    uint8_t command_buf[5] __attribute__ ((aligned (4))) = {MICRON_WRITE_ENABLE};
    /*Both Write enable and Die erase can work in all modes*/

    /*Write enable command must be executed before erase
     * WRITE ENABLE 06h 1-0-0 2-0-0 4-0-0 0 no dummy cycles.
     * */
    MSS_QSPI_polled_transfer_block(0, command_buf, 0, (uint8_t*)0, 0,0);

    /*Erase the die. This will write 1 to all bits
     * DIE ERASE C4h 1-1-0 2-2-0 4-4-0 no dummy cycles
     * */
    command_buf[0] = MICRON_DIE_ERASE;
    command_buf[1] = 0x00;    //memory address MSB
    command_buf[2] = 0x00;
    command_buf[3] = 0x03;

    MSS_QSPI_polled_transfer_block(3, command_buf, 0, (uint8_t*)0, 0,0);
}

#if 0 // UNUSED
static void Flash_reset(void);
static void Flash_reset(void)
{
    uint8_t command_buf[1] __attribute__ ((aligned (4))) = {MICRON_RESET_ENABLE};

    MSS_QSPI_serialize(8); //exit quad/dual
    MSS_QSPI_serialize(16); //exit DTR

    MSS_QSPI_polled_transfer_block(0, command_buf, 0, (uint8_t*)0, 0,0);
    for(volatile int i=0; i< 100000; i++);
    command_buf[0] = MICRON_RESET_MEMORY;
    MSS_QSPI_polled_transfer_block(0, command_buf, 0, (uint8_t*)0, 0,0);

    wait_for_wel();
    wait_for_wip();
}
#endif

#ifdef __cplusplus
}
#endif
