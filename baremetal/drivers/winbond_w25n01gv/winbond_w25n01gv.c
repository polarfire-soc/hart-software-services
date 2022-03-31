/***************************************************************************//**
 * Copyright 2019-2022 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Bare metal driver for the Winbond w25n01gv NAND flash memory.
 * This driver uses the MPFS MSS QSPI driver interface.
 */
#include "mpfs_hal/mss_hal.h"
#include "drivers/winbond_w25n01gv/winbond_w25n01gv.h"
#include "drivers/mss/mss_mmuart/mss_uart.h"

#include <assert.h> //assert
#include <string.h> //memcpy

/* The following constant must be defined if you want to use the interrupt mode
 * transfers provided by the MSS QSPI driver. Comment this out to use the
 * polling mode transfers.
 */
/* #define USE_QSPI_INTERRUPT      1u */

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_PAGES_PER_BLOCK                     64u
#define NUM_BLOCKS_PER_DIE                      1024u
#define PAGE_LENGTH                             2048u
#define BLOCK_LENGTH                            (PAGE_LENGTH * NUM_PAGES_PER_BLOCK)
#define DIE_SIZE                                (BLOCK_LENGTH * NUM_BLOCKS_PER_DIE)
#define NUM_LUTS                                20u

#define STATUS_REG_1                            0xA0u
#define STATUS_REG_2                            0xB0u
#define STATUS_REG_3                            0xC0u

#define STATUS_REG_2_OTP_L                      (1u << 7)
#define STATUS_REG_2_OTP_E                      (1u << 6)
#define STATUS_REG_2_SR1_L                      (1u << 5)
#define STATUS_REG_2_ECC_E                      (1u << 4)
#define STATUS_REG_2_BUF                        (1u << 3)

#define STATUS_REG_3_LUTF                       (1u << 6)
#define STATUS_REG_3_ECC1                       (1u << 5)
#define STATUS_REG_3_ECC0                       (1u << 4)
#define STATUS_REG_3_PFAIL                      (1u << 3)
#define STATUS_REG_3_EFAIL                      (1u << 2)
#define STATUS_REG_3_WEL                        (1u << 1)
#define STATUS_REG_3_BUSY                       (1u << 0)

#define DEVICE_RESET_OPCODE                     0xFFu
#define READ_ID_OPCODE                          0x9Fu

#define READ_STATUS_REG_OPCODE                  0x05u
#define WRITE_STATUS_REG_OPCODE                 0x01u
#define WRITE_ENABLE_OPCODE                     0x06u

#define PAGE_DATA_READ_OPCODE                   0x13u
#define READ_DATA_OPCODE                        0x03u
#define FAST_READ_OPCODE                        0x0Bu

#define BLOCK_ERASE_OPCODE                      0xD8u

#define LOAD_PROGRAM_DATA_OPCODE                0x02u
#define RANDOM_LOAD_PROGRAM_DATA_OPCODE         0x84u
#define PROGRAM_EXECUTE_OPCODE                  0x10u

#define READ_BBM_LUT_OPCODE                     0xA5u
#define BAD_BLOCK_MANAGEMENT_OPCODE             0xA1u

#define FAST_READ_DUAL_OUTPUT_OPCODE            0x3Bu
#define FAST_READ_DUAL_OUTPUT_ADDR4_OPCODE      0x3Cu
#define FAST_READ_DUAL_IO_OPCODE                0xBBu
#define FAST_READ_DUAL_IO_ADDR4_OPCODE          0xBCu

#define FAST_READ_QUAD_O_OPCODE                 0x6Bu
#define FAST_READ_QUAD_O_ADDR4_OPCODE           0x6Cu
#define FAST_READ_QUAD_IO_OPCODE                0xEBu
#define FAST_READ_QUAD_IO_ADDR4_OPCODE          0xECu

// The MSS QSPI peripheral has a CMDBYTES field that is limited to
// 511 bytes.
//
// We currently use it to transmit the command phase plus the TX
// phase of a TX transaction... So our payload is limited to
// 511 - 1 byte opcode - 2 byte address
//
// See the CoreQSPI Handbook for details
//
#define MSS_QSPI_CMDBYTES_LIMIT                   (511 - 3)

static mss_qspi_config_t g_qspi_config= { 0 };

#ifdef USE_QSPI_INTERRUPT
static volatile uint8_t g_rx_complete = 0u;
static volatile uint8_t g_tx_complete = 0u;
#endif

#define ARRAY_SIZE(x)		(sizeof(x)/sizeof(x[0]))

/*******************************************************************************
 * Local functions
 */
static uint8_t read_page(uint8_t* p_rx_buf, uint32_t page, uint16_t column, uint32_t read_len);
static void send_write_enable_command(void);
static void disable_write_protect(void);
static bool is_bad_block(uint16_t block_index);
static uint8_t erase_block(uint16_t addr);
static uint8_t program_page(uint8_t const * const p_tx_buf, uint32_t page, const uint32_t wr_len);
static uint8_t mark_block_as_bad(uint32_t block_index);
static void read_statusreg(uint8_t status_reg_address, uint8_t* rd_buf);
static void write_statusreg(uint8_t address, uint8_t value);
static void wait_if_busy(void);
static void wait_for_wel(void);
static uint32_t wait_if_busy_or_wel(void);

#ifdef USE_QSPI_INTERRUPT
void transfer_status_handler(uint32_t status)
{
    if (STTS_RDONE_MASK == (STTS_RDONE_MASK & status)) {
        g_rx_complete = 1;
    } else if (STTS_TDONE_MASK == (STTS_TDONE_MASK & status)) {
        g_tx_complete = 1;
    }
}

static void wait_for_tx_complete(void)
{
    while (0u == g_tx_complete) {
        ;
    }
    g_tx_complete = 0u;
}

static void wait_for_rx_complete(void)
{
    while (0u == g_rx_complete) {
        ;
    }
    g_rx_complete = 0u;
}
#endif

void Flash_init(mss_qspi_io_format io_format)
{
    volatile uint8_t transmit_buffer[2];
    volatile uint8_t receive_buffer[3];
    uint8_t status_reg2_value;

    MSS_QSPI_init();

    g_qspi_config.clk_div =  MSS_QSPI_CLK_DIV_8;
    g_qspi_config.sample = MSS_QSPI_SAMPLE_POSAGE_SPICLK;
    g_qspi_config.spi_mode = MSS_QSPI_MODE3;
    g_qspi_config.xip = MSS_QSPI_DISABLE;
    g_qspi_config.io_format = MSS_QSPI_NORMAL;
    MSS_QSPI_configure(&g_qspi_config);

    /* store the format for later use in READ and Write APIs */
    switch (io_format) {
    case MSS_QSPI_QUAD_FULL:
        __attribute__((fallthrough)); // deliberate fallthrough
    case MSS_QSPI_QUAD_EX_RO:
        __attribute__((fallthrough)); // deliberate fallthrough
    case MSS_QSPI_QUAD_EX_RW:
        g_qspi_config.io_format = MSS_QSPI_QUAD_EX_RO;
        break;

    case MSS_QSPI_DUAL_FULL:
        __attribute__((fallthrough)); // deliberate fallthrough
    case MSS_QSPI_DUAL_EX_RO:
        __attribute__((fallthrough)); // deliberate fallthrough
    case MSS_QSPI_DUAL_EX_RW:
        g_qspi_config.io_format = MSS_QSPI_DUAL_EX_RO;
        break;

    default:
        g_qspi_config.io_format = MSS_QSPI_NORMAL;
        break;
    }

    /* Reset Flash memory */
    transmit_buffer[0] = DEVICE_RESET_OPCODE;
    MSS_QSPI_polled_transfer_block(0u, (const void * const)transmit_buffer, 0u,
        (const void * const)receive_buffer, 0u, 0u);

    // Ensure BUF=1 and ECC-E=1 for page-based ECC
    read_statusreg(STATUS_REG_2, (uint8_t *)&status_reg2_value);
    status_reg2_value |= (STATUS_REG_2_ECC_E | STATUS_REG_2_BUF);
    write_statusreg(STATUS_REG_2, status_reg2_value);
}

void Flash_readid(uint8_t* buf)
{
    const uint8_t command_buf[1] __attribute__ ((aligned (4))) = {READ_ID_OPCODE};

#ifdef USE_QSPI_INTERRUPT
    MSS_QSPI_irq_transfer_block(0u, (const void * const)command_buf, 0u,
        (const void *const)buf, 3u, 8u);
    wait_for_rx_complete();
#else
    MSS_QSPI_polled_transfer_block(0u, (const void * const)command_buf, 0u,
        (const void * const)buf, 3u, 8u);
#endif
}

uint8_t Flash_read(uint8_t* pDst, uint32_t srcAddr, uint32_t len)
{
    /* We only deal with reads from page boundaries for now */
    assert((srcAddr % PAGE_LENGTH) == 0);

    uint8_t result = 0u;
    uint32_t page_index;
    uint8_t * target_buf = pDst;
    int32_t remaining_length = (int32_t)len;

    page_index = srcAddr / PAGE_LENGTH;

    while (remaining_length > 0) {
        uint32_t length;

        if (remaining_length > PAGE_LENGTH) {
            length = PAGE_LENGTH;
        } else {
            length = remaining_length;
        }


        result = read_page(target_buf, page_index, 0u, length);

	if (result) {
            break;
        }

        remaining_length = remaining_length - length;
        page_index++;
        target_buf = target_buf + length;
    }

    return result;
}

uint8_t Flash_erase(void)
{
    uint16_t block_index;
    uint8_t status = 0xFFu;

    disable_write_protect();

    for (block_index = 0u; block_index < NUM_BLOCKS_PER_DIE; block_index++) {
        status = erase_block(block_index);
    }

    return(status);
}

uint8_t Flash_erase_block(uint16_t block_index)
{
    uint8_t status = 0xFFu;

    disable_write_protect();

    status = erase_block(block_index);
    //if (status & STATUS_REG_3_EFAIL) {
    //    mark_block_as_bad(block_index);
    //}

    return(status);
}

uint8_t Flash_program(uint8_t* buf, uint32_t addr, uint32_t len)
{
    int32_t remaining_length = (int32_t)len;
    uint32_t target_page_offset = addr / PAGE_LENGTH;
    uint8_t status = 0xFFu;

    disable_write_protect();

    while (remaining_length > 0u) {
        uint32_t length;

        if (remaining_length >= PAGE_LENGTH) {
            length = PAGE_LENGTH;
        } else {
            length = remaining_length;
        }

        status = program_page(buf, target_page_offset, length);

        //if (status & STATUS_REG_3_PFAIL) {
        //    mark_block_as_bad(target_page_offset / NUM_PAGES_PER_BLOCK);
        //}
        (void)mark_block_as_bad;

        remaining_length -= length;
        target_page_offset += 1u;
        buf += length;
    }

    return status;
}

uint8_t Flash_add_entry_to_bb_lut(uint16_t lba, uint16_t pba)
{
    uint8_t result = 0u;
    uint8_t status;

    read_statusreg(STATUS_REG_3, (uint8_t *)&status);

    if ((status & STATUS_REG_3_LUTF) == 0) {
        uint8_t command_buf[5] __attribute__ ((aligned (4))) = {
            BAD_BLOCK_MANAGEMENT_OPCODE, (lba >> 8) & 0xFF, lba & 0xFF, (pba >> 8) & 0xFF, lba & 0xFF
        };

        send_write_enable_command();
        MSS_QSPI_polled_transfer_block(0u, (const void * const)command_buf, 4u,
            (const void * const)0, 0u, 0u);
        status = wait_if_busy_or_wel();
        (void)status;

        result = 1u;
    }

    return result;
}

void Flash_read_status_regs(uint8_t * buf)
{
    read_statusreg(STATUS_REG_1, buf);
    read_statusreg(STATUS_REG_2, buf + 1);
    read_statusreg(STATUS_REG_3, buf + 2);
}

uint8_t Flash_read_bb_lut(w25_bb_lut_entry_t* lut_ptr)
{
    uint8_t num_bb = 0u;
    uint8_t buf[4 * NUM_LUTS] = { 0u };
    uint8_t command_buf[2] __attribute__ ((aligned (2))) = { READ_BBM_LUT_OPCODE, 0u };

    /*
     * Read Bad block LUT
     * 8 dummy cycles
     * 20 LUT entries of 4 bytes each
     */
    MSS_QSPI_polled_transfer_block(0u, (const void * const)command_buf, 0u,
        (const void * const)buf, 80u, 8u);

    for (int lut_idx = 0u; lut_idx < NUM_LUTS; lut_idx++) {
        const int buf_idx = lut_idx * 4u;

        lut_ptr[lut_idx].enable = ((buf[buf_idx] & 0x80u)? 1u : 0u);
        lut_ptr[lut_idx].invalid = ((buf[buf_idx] & 0x40u)? 1u : 0u);
        lut_ptr[lut_idx].lba = (((uint16_t)(buf[buf_idx] & 0x3u) << 8u) | buf[buf_idx+1]);
        lut_ptr[lut_idx].pba = (((uint16_t)buf[buf_idx+2] << 8u) | buf[buf_idx+3]);

        if ((1u == lut_ptr[lut_idx].enable) && (0u == lut_ptr[lut_idx].invalid)) {
            num_bb++;
        }
    }

    return num_bb;
}

uint32_t Flash_scan_for_bad_blocks(uint16_t* buf)
{
    uint32_t bad_count;
    bad_count = 0u;

    for (uint16_t block_num = 0u; block_num < NUM_BLOCKS_PER_DIE; block_num++) {
        if (is_bad_block(block_num)) {
            buf[bad_count++] = block_num;
        }
    }

    return(bad_count);
}

/*******************************************************************************
 * Local functions
 */

static void send_write_enable_command(void)
{
    uint8_t command_buf[1] __attribute__ ((aligned (4))) = { WRITE_ENABLE_OPCODE };

    MSS_QSPI_polled_transfer_block(0u, (const void * const)command_buf, 0u,
        (const void * const)0, 0u, 0u);
    wait_for_wel();
}

static void disable_write_protect(void)
{
    write_statusreg(STATUS_REG_1, 0x0u);
}

static bool is_bad_block(uint16_t block_index)
{
    bool result = false;

    uint8_t receive_buf[2];

    // read the OOB
    read_page(receive_buf, block_index * NUM_PAGES_PER_BLOCK, PAGE_LENGTH, ARRAY_SIZE(receive_buf));
    if ((receive_buf[0] != 0xFFu) || (receive_buf[1] != 0xFFu)) {
         result = true;
    }

    return result;
}

static uint8_t erase_block(uint16_t block_index)
{
    uint8_t status;

    if (is_bad_block(block_index)) {
        // never erase bad blocks, as we'll lose bad block marker information...
        status = STATUS_REG_3_EFAIL;
    } else {
        //
        // weirdly, the W25N01GV block erase (D8h) actually uses a PAGE ADDRESS
        // and not a block address
        //
        uint8_t command_buf[4] __attribute__ ((aligned (4))) = {
            BLOCK_ERASE_OPCODE,
            0u, // dummy for 8 clocks
            ((block_index * NUM_PAGES_PER_BLOCK) >> 8u) & 0xFF,
            (block_index * NUM_PAGES_PER_BLOCK) & 0xFF
        };

        read_statusreg(STATUS_REG_1, &status);
        assert(0 == status); // protection must be disabled!

        wait_if_busy();
        send_write_enable_command();
        MSS_QSPI_polled_transfer_block(0u, (const void * const)command_buf, 3u,
            (const void * const)0, 0u, 0u);

        status = wait_if_busy_or_wel();

        status = (STATUS_REG_3_EFAIL & status);
    }

    return status;
}

static uint8_t program_page(uint8_t const * const p_tx_buf, uint32_t page, const uint32_t wr_len)
{
    uint8_t command_buf[PAGE_LENGTH + 3] __attribute__ ((aligned (4))) = { 0u };
    uint16_t column = 0u;
    uint8_t status = 0xFFu;

    ASSERT(wr_len <= PAGE_LENGTH);

    int32_t remaining_length = (int32_t)wr_len;

    while (remaining_length > 0u) {
        uint32_t subpage_length;

        if (remaining_length > MSS_QSPI_CMDBYTES_LIMIT) {
            subpage_length = MSS_QSPI_CMDBYTES_LIMIT;
        } else {
            subpage_length = remaining_length;
        }

        if (remaining_length == wr_len) {
                // first time around, reset unused data bytes in data buffer to FFh
                command_buf[0] = LOAD_PROGRAM_DATA_OPCODE;
        } else {
                command_buf[0] = RANDOM_LOAD_PROGRAM_DATA_OPCODE;
	}

        command_buf[1] = (column >> 8) & 0xFF;
        command_buf[2] = column & 0xFF;

        for (uint16_t idx = 0u; idx < subpage_length; idx++) {
            command_buf[3 + idx] = p_tx_buf[column + idx];
        }

        send_write_enable_command();
        MSS_QSPI_polled_transfer_block(2u, (const void * const)command_buf, subpage_length,
            (const void * const)0, 0u, 0u);

        assert(remaining_length >= subpage_length);
        remaining_length -= subpage_length;

        column += subpage_length;
    }

    command_buf[0] = PROGRAM_EXECUTE_OPCODE;
    command_buf[1] = 0u;
    command_buf[2] = (page >> 8) & 0xFF;
    command_buf[3] = page & 0xFF;

    send_write_enable_command();
    MSS_QSPI_polled_transfer_block(0u, (const void * const)command_buf, 3u,
        (const void * const)0, 0u, 0u);

    status = wait_if_busy_or_wel();

    // ECC-1 indicates 2-bit error that cannot be corrected...
    return ((STATUS_REG_3_ECC1 | STATUS_REG_3_PFAIL) & status);
}

static uint8_t mark_block_as_bad(uint32_t block_index)
{
    uint8_t command_buf[PAGE_LENGTH + 3] __attribute__ ((aligned (4))) = { 0u };
    const uint16_t column = PAGE_LENGTH;
    uint8_t status = 0xFFu;
    const uint32_t page = block_index * NUM_PAGES_PER_BLOCK;

    // A "Bad Block Marker" is a non-FFh data byte stored at Byte 0 of Page 0
    // for each bad block. An additional marker is also stored in the first two
    // bytes of the 64-Byte spare area.
    //
    // We'll mark the block as bad by writing to the first two bytes of the
    // spare area...

    // opcode
    command_buf[0] = RANDOM_LOAD_PROGRAM_DATA_OPCODE;
    // 2-bytes address
    command_buf[1] = (column >> 8) & 0xFF;
    command_buf[2] = column & 0xFF;
    // 2-bytes data
    command_buf[3] = 0x00u;
    command_buf[4] = 0x00u;

    send_write_enable_command();
    MSS_QSPI_polled_transfer_block(2u, (const void * const)command_buf, 2u,
        (const void * const)0, 0u, 0u);

    // opcode
    command_buf[0] = PROGRAM_EXECUTE_OPCODE;
    // 0-bytes address
    // 3-bytes data
    command_buf[1] = 0u;
    command_buf[2] = (page >> 8) & 0xFF;
    command_buf[3] = page & 0xFF;

    send_write_enable_command();
    MSS_QSPI_polled_transfer_block(0u, (const void * const)command_buf, 3u,
        (const void * const)0, 0u, 0u);

    status = wait_if_busy_or_wel();

    return (STATUS_REG_3_PFAIL & status);
}

/*
 * Make sure that the program/erase operation is complete. i.e. wait for
 * busy bit to go 0.
 */

// this define is to prevent the code getting stuck forever spinning on a bit
// it is arbitrarily large, but big enough that it won't cause timeouts in
// normal operation
#define MAX_SPIN_COUNT 0x01000000

static void wait_if_busy(void)
{
    uint32_t status_reg = 0u;
    uint32_t max_count = MAX_SPIN_COUNT;

    do {
        read_statusreg(STATUS_REG_3, (uint8_t*)&status_reg);
        max_count--;
    } while (max_count && (STATUS_REG_3_BUSY & status_reg));
}

static void wait_for_wel(void)
{
    uint32_t status_reg = 0u;
    uint32_t max_count = MAX_SPIN_COUNT;

    do {
        read_statusreg(STATUS_REG_3, (uint8_t*)&status_reg);
        max_count--;
    } while (max_count && !(STATUS_REG_3_WEL & status_reg));
}

static uint32_t wait_if_busy_or_wel(void)
{
    uint32_t status_reg = 0u;
    uint32_t max_count = MAX_SPIN_COUNT;

    do {
        read_statusreg(STATUS_REG_3, (uint8_t*)&status_reg);
        max_count--;
    } while (max_count && ((STATUS_REG_3_BUSY & status_reg) || (STATUS_REG_3_WEL & status_reg)));

    return status_reg;
}

static void write_statusreg(uint8_t address, uint8_t value)
{
    const uint8_t command_buf[3] __attribute__ ((aligned (4))) = {
        WRITE_STATUS_REG_OPCODE, address, value
    };

    MSS_QSPI_polled_transfer_block(0u, (const void * const)command_buf, 2u,
        (const void * const)0, 0u, 0u);
}

static void read_statusreg(uint8_t status_reg_address, uint8_t* pRd_buf)
{
    const uint8_t command_buf[2] __attribute__ ((aligned (4))) = {
        READ_STATUS_REG_OPCODE, status_reg_address
    };

    MSS_QSPI_polled_transfer_block(1u, (const void * const)command_buf, 0u,
        (const void * const)pRd_buf, 1u, 0u);
}

static uint8_t read_page(uint8_t* p_rx_buf, uint32_t page, uint16_t column, uint32_t read_len)
{
    uint8_t result = 0u;
    uint32_t length = read_len;
    uint8_t num_tx_bytes = 0u;
    uint8_t num_addr_bytes = 0u;
    uint8_t command_buf[4] __attribute__ ((aligned (4))) = { 0u };
    uint8_t num_dummy_cycles = 0u;

    wait_if_busy();

    command_buf[0] = PAGE_DATA_READ_OPCODE;
    command_buf[1] = 0u;
    command_buf[2] = (page >> 8u) & 0xFF;
    command_buf[3] = page & 0xFF;

    MSS_QSPI_polled_transfer_block(0u, (const void * const)command_buf, 3u,
        (const void * const)0, 0u, 0u);
    wait_if_busy();

    command_buf[1] = (column >> 8u) & 0xFF;
    command_buf[2] = column & 0xFF;
    num_addr_bytes = 2u;
    num_tx_bytes = 0u;

    switch (g_qspi_config.io_format) {
    case MSS_QSPI_QUAD_FULL:
        __attribute__((fallthrough)); // deliberate fallthrough
    case MSS_QSPI_QUAD_EX_RO:
        __attribute__((fallthrough)); // deliberate fallthrough
    case MSS_QSPI_QUAD_EX_RW:
        command_buf[0] = FAST_READ_QUAD_IO_OPCODE;
        num_dummy_cycles = 4u;
        break;

    case MSS_QSPI_DUAL_FULL:
        __attribute__((fallthrough)); // deliberate fallthrough
    case MSS_QSPI_DUAL_EX_RO:
        __attribute__((fallthrough)); // deliberate fallthrough
    case MSS_QSPI_DUAL_EX_RW:
        command_buf[0] = FAST_READ_DUAL_OUTPUT_OPCODE;
        num_dummy_cycles = 8u;
        break;

    default:
        command_buf[0] = FAST_READ_OPCODE; //READ_DATA_OPCODE;
        num_dummy_cycles = 8u;
        break;
    }

    if (read_len > PAGE_LENGTH) {
        length = PAGE_LENGTH;
    }

    mss_qspi_io_format temp = g_qspi_config.io_format;
    MSS_QSPI_configure(&g_qspi_config);

    MSS_QSPI_polled_transfer_block(num_addr_bytes, (const void * const)command_buf, num_tx_bytes,
        (const void * const)p_rx_buf, length, num_dummy_cycles);

    g_qspi_config.io_format = MSS_QSPI_NORMAL;
    MSS_QSPI_configure(&g_qspi_config);
    g_qspi_config.io_format = temp;

    read_statusreg(STATUS_REG_3, (uint8_t *)&result);
    result &= (STATUS_REG_3_ECC1 & STATUS_REG_3_ECC0);

    return result;
}

#ifdef __cplusplus
}
#endif
