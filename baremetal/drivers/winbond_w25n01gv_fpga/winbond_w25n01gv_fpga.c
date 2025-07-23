/* winbond_w25n01gv_apb.c
 * Bare metal driver for the Winbond W25N01GV NAND flash memory using APB QSPI interface
 * on PolarFire SoC FPGA.
 * Preserves logic of original winbond_w25n01gv.c driver with optimized register access.
 * Copyright 2023-2025 Microchip FPGA Embedded Systems Solutions.
 * SPDX-License-Identifier: MIT
 */

#include "w25n01gv_apb.h"
#include <assert.h>
#include <string.h>

// External timer function (assumed provided by HSS)
extern void DEL_10us(int timer);

// Register access macros
#define REG_WRITE(addr, val) (*(volatile uint32_t *)(addr) = (val))
#define REG_READ(addr) (*(volatile uint32_t *)(addr))

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

#define MSS_QSPI_CMDBYTES_LIMIT                 (511 - 3)
#define MAX_SPIN_COUNT                          0x01000000

#define ARRAY_SIZE(x)                           (sizeof(x)/sizeof(x[0]))

// QSPI configuration
static struct {
    w25_io_format io_format;
} g_qspi_config = { W25_NORMAL };

// Internal: Transfer block via QSPI
static void qspi_transfer_block(uint8_t num_addr_bytes, const void *tx_buf, uint32_t tx_len,
                                const void *rx_buf, uint32_t rx_len, uint8_t num_dummy_cycles)
{
    uint8_t *tx_data = (uint8_t *)tx_buf;
    uint8_t *rx_data = (uint8_t *)rx_buf;
    uint32_t ctrl1 = QSPI_CTRL1_ENABLE | QSPI_CTRL1_nRESET | (1 << QSPI_CTRL1_S_DIV);

    // Set CE
    REG_WRITE(QSPI_BASE_ADDR + QSPI_WR_CTRL1, ctrl1 | QSPI_CTRL1_CE);

    // Send command and address
    if (tx_data && (num_addr_bytes + 1 <= tx_len + 1)) {
        ctrl1 |= QSPI_CTRL1_START_OP | ((uint32_t)(num_addr_bytes + 1) << QSPI_CTRL1_S_QT);
        REG_WRITE(QSPI_BASE_ADDR + QSPI_WR_CTRL1, ctrl1);
        for (uint8_t i = 0; i <= num_addr_bytes; i++) {
            while (REG_READ(QSPI_BASE_ADDR + QSPI_RD_STATUS) & QSPI_STATUS_FT_FULL) {
                DEL_10us(TMR_MQSPI);
            }
            REG_WRITE(QSPI_BASE_ADDR + QSPI_WR_DATA, (uint32_t)tx_data[i] << 24);
        }
        while (!(REG_READ(QSPI_BASE_ADDR + QSPI_RD_STATUS) & QSPI_STATUS_IDLE)) {
            DEL_10us(TMR_MQSPI);
        }
    }

    // Send additional TX data
    if (tx_data && tx_len > num_addr_bytes) {
        uint32_t data_len = tx_len - num_addr_bytes;
        ctrl1 = QSPI_CTRL1_ENABLE | QSPI_CTRL1_CE | QSPI_CTRL1_nRESET | (1 << QSPI_CTRL1_S_DIV) |
                QSPI_CTRL1_START_OP | ((uint32_t)data_len << QSPI_CTRL1_S_QT);
        REG_WRITE(QSPI_BASE_ADDR + QSPI_WR_CTRL1, ctrl1);
        for (uint32_t i = num_addr_bytes; i < tx_len; i++) {
            while (REG_READ(QSPI_BASE_ADDR + QSPI_RD_STATUS) & QSPI_STATUS_FT_FULL) {
                DEL_10us(TMR_MQSPI);
            }
            REG_WRITE(QSPI_BASE_ADDR + QSPI_WR_DATA, (uint32_t)tx_data[i] << 24);
        }
        while (!(REG_READ(QSPI_BASE_ADDR + QSPI_RD_STATUS) & QSPI_STATUS_IDLE)) {
            DEL_10us(TMR_MQSPI);
        }
    }

    // Send dummy cycles
    if (num_dummy_cycles) {
        ctrl1 = QSPI_CTRL1_ENABLE | QSPI_CTRL1_CE | QSPI_CTRL1_nRESET | (1 << QSPI_CTRL1_S_DIV) |
                QSPI_CTRL1_START_OP | ((uint32_t)num_dummy_cycles / 8 << QSPI_CTRL1_S_QT);
        REG_WRITE(QSPI_BASE_ADDR + QSPI_WR_CTRL1, ctrl1);
        for (uint8_t i = 0; i < num_dummy_cycles / 8; i++) {
            while (REG_READ(QSPI_BASE_ADDR + QSPI_RD_STATUS) & QSPI_STATUS_FT_FULL) {
                DEL_10us(TMR_MQSPI);
            }
            REG_WRITE(QSPI_BASE_ADDR + QSPI_WR_DATA, 0);
        }
        while (!(REG_READ(QSPI_BASE_ADDR + QSPI_RD_STATUS) & QSPI_STATUS_IDLE)) {
            DEL_10us(TMR_MQSPI);
        }
    }

    // Receive data
    if (rx_data && rx_len) {
        ctrl1 = QSPI_CTRL1_ENABLE | QSPI_CTRL1_CE | QSPI_CTRL1_nRESET | (1 << QSPI_CTRL1_S_DIV) |
                QSPI_CTRL1_START_OP | ((uint32_t)rx_len << QSPI_CTRL1_S_QR);
        REG_WRITE(QSPI_BASE_ADDR + QSPI_WR_CTRL1, ctrl1);
        for (uint32_t i = 0; i < rx_len; i++) {
            while (REG_READ(QSPI_BASE_ADDR + QSPI_RD_STATUS) & QSPI_STATUS_FR_EMPTY) {
                DEL_10us(TMR_MQSPI);
            }
            rx_data[i] = (REG_READ(QSPI_BASE_ADDR + QSPI_RD_DATA) >> 24) & 0xFF;
        }
        while (!(REG_READ(QSPI_BASE_ADDR + QSPI_RD_STATUS) & QSPI_STATUS_IDLE)) {
            DEL_10us(TMR_MQSPI);
        }
    }

    // Clear CE
    REG_WRITE(QSPI_BASE_ADDR + QSPI_WR_CTRL1, QSPI_CTRL1_ENABLE | QSPI_CTRL1_nRESET | (1 << QSPI_CTRL1_S_DIV));
}

// Local functions
static uint8_t read_page(uint8_t* p_rx_buf, uint32_t page, uint16_t column, uint32_t read_len);
static void send_write_enable_command(void);
static void disable_write_protect(void);
static bool is_bad_block(uint16_t block_index);
static uint8_t erase_block(uint16_t addr);
static uint8_t program_page(uint8_t const * const p_tx_buf, uint32_t page, uint32_t wr_len);
static uint8_t mark_block_as_bad(uint32_t block_index);
static void read_statusreg(uint8_t status_reg_address, uint8_t* rd_buf);
static void write_statusreg(uint8_t address, uint8_t value);
static void wait_for_flash(void);
static void wait_for_wel(void);
static uint32_t wait_if(void);

void Flash_init(w25_io_format io_format)
{
    volatile uint8_t transmit_buffer[2];
    uint8_t status_reg2_value;

    // Initialize QSPI
    REG_WRITE(QSPI_BASE_ADDR + QSPI_WR_CTRL1, QSPI_CTRL1_ENABLE | QSPI_CTRL1_nRESET | (1));

    g_qspi_config.io_format = W25_NORMAL;
    switch (io_format) {
        case W25_QUAD_EX_RO:
            g_qspi_config.io_format = W25_QUAD_EX_RO;
            break;
        case W25_DUAL_EX_RO:
            g_qspi_config.io_format = W25_DUAL_EX_RO;
            break;
        default:
            g_qspi_config.io_format = W25_NORMAL;
            break;
    }

    // Reset Flash memory
    transmit_buffer[0] = DEVICE_RESET_OPCODE;
    qspi_transfer_block(0u, transmit_buffer, 0u, NULL, 0u, 0u);

    // Ensure BUF=1 and ECC-E=1 for page-based ECC
    read_statusreg(STATUS_REG_2, &status_reg2_value);
    status_reg2_value |= (STATUS_REG_2_ECC_E | STATUS_REG_2_BUF);
    write_statusreg(STATUS_REG_2, status_reg2_value);
}

void Flash_readid(uint8_t* buf)
{
    const uint8_t command_buf[1] __attribute__ ((aligned (4))) = {READ_ID_OPCODE};
    qspi_transfer_block(0u, command_buf, 0u, buf, 3u, 8u);
}

uint8_t Flash_read(uint8_t* pDst, uint32_t srcAddr, uint32_t len)
{
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

        remaining_length -= length;
        page_index++;
        target_buf += length;
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
    return status;
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

    read_statusreg(STATUS_REG_3, &status);

    if ((status & STATUS_REG_3_LUTF) == 0) {
        uint8_t command_buf[5] __attribute__ ((aligned (4))) = {
            BAD_BLOCK_MANAGEMENT_OPCODE, (lba >> 8) & 0xFF, lba & 0xFF, (pba >> 8) & 0xFF, lba & 0xFF
        };

        send_write_enable_command();
        qspi_transfer_block(0u, command_buf, 4u, NULL, 0u, 0u);
        status = wait_if();
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

    qspi_transfer_block(0u, command_buf, 0u, buf, 80u, 8u);

    for (uint32_t lut_idx = 0u; lut_idx < NUM_LUTS; lut_idx++) {
        const uint32_t buf_idx = lut_idx * 4u;

        lut_ptr[lut_idx].enable = ((buf[buf_idx] & 0x80u) ? 1u : 0u);
        lut_ptr[lut_idx].invalid = ((buf[buf_idx] & 0x40u) ? 1u : 0u);
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
    uint32_t bad_count = 0u;

    for (uint16_t block_num = 0u; block_num < NUM_BLOCKS_PER_DIE; block_num++) {
        if (is_bad_block(block_num)) {
            buf[bad_count++] = block_num;
        }
    }

    return bad_count;
}

void Flash_flush(void)
{
    // No-op for compatibility
}

static void send_write_enable_command(void)
{
    uint8_t command_buf[1] __attribute__ ((aligned (4))) = { WRITE_ENABLE_OPCODE };
    qspi_transfer_block(0u, command_buf, 0u, NULL, 0u, 0u);
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
        status = STATUS_REG_3_EFAIL;
    } else {
        uint8_t command_buf[4] __attribute__ ((aligned (4))) = {
            BLOCK_ERASE_OPCODE,
            0u,
            ((block_index * NUM_PAGES_PER_BLOCK) >> 8u) & 0xFF,
            (block_index * NUM_PAGES_PER_BLOCK) & 0xFF
        };

        read_statusreg(STATUS_REG_1, &status);
        assert(0 == status);

        wait_for_flash();
        send_write_enable_command();
        qspi_transfer_block(0u, command_buf, 3u, NULL, 0u, 0u);

        status = wait_if();

        status = (STATUS_REG_3_EFAIL & status);
    }

    return status;
}

static uint8_t program_page(uint8_t const * const p_tx_buf, uint32_t page, uint32_t wr_len)
{
    uint8_t command_buf[PAGE_LENGTH + 3] __attribute__ ((aligned (4))) = { 0u };
    uint16_t column = 0u;
    uint8_t status = 0xFFu;

    assert(wr_len <= PAGE_LENGTH);

    int32_t remaining_length = (int32_t)wr_len;

    while (remaining_length > 0u) {
        uint32_t subpage_length;

        if (remaining_length > MSS_QSPI_CMDBYTES_LIMIT) {
            subpage_length = MSS_QSPI_CMDBYTES_LIMIT;
        } else {
            subpage_length = remaining_length;
        }

        if (remaining_length == wr_len) {
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
        qspi_transfer_block(2u, command_buf, subpage_length, NULL, 0u, 0u);

        assert(remaining_length >= subpage_length);
        remaining_length -= subpage_length;

        column += subpage_length;
    }

    command_buf[0] = PROGRAM_EXECUTE_OPCODE;
    command_buf[1] = 0u;
    command_buf[2] = (page >> 8) & 0xFF;
    command_buf[3] = page & 0xFF;

    send_write_enable_command();
    qspi_transfer_block(0u, command_buf, 3u, NULL, 0u, 0u);

    status = wait_if();

    return ((STATUS_REG_3_ECC1 | STATUS_REG_3_PFAIL) & status);
}

static uint8_t mark_block_as_bad(uint32_t block_index)
{
    uint8_t command_buf[PAGE_LENGTH + 3] __attribute__ ((aligned (4))) = { 0u };
    const uint16_t column = PAGE_LENGTH;
    uint8_t status = 0xFFu;
    const uint32_t page = block_index * NUM_PAGES_PER_BLOCK;

    command_buf[0] = RANDOM_LOAD_PROGRAM_DATA_OPCODE;
    command_buf[1] = (column >> 8) & 0xFF;
    command_buf[2] = column & 0xFF;
    command_buf[3] = 0x00u;
    command_buf[4] = 0x00u;

    send_write_enable_command();
    qspi_transfer_block(2u, command_buf, 2u, NULL, 0u, 0u);

    command_buf[0] = PROGRAM_EXECUTE_OPCODE;
    command_buf[1] = 0u;
    command_buf[2] = (page >> 8) & 0xFF;
    command_buf[3] = page & 0xFF;

    send_write_enable_command();
    qspi_transfer_block(0u, command_buf, 3u, NULL, 0u, 0u);

    status = wait_if();

    return (STATUS_REG_3_PFAIL & status);
}

static void wait_for_flash(void)
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

static uint32_t wait_if(void)
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

    qspi_transfer_block(0u, command_buf, 2u, NULL, 0u, 0u);
}

static void read_statusreg(uint8_t status_reg_address, uint8_t* pRd_buf)
{
    const uint8_t command_buf[2] __attribute__ ((aligned (4))) = {
        READ_STATUS_REG_OPCODE, status_reg_address
    };

    qspi_transfer_block(1u, command_buf, 0u, pRd_buf, 1u, 0u);
}

static uint8_t read_page(uint8_t* p_rx_buf, uint32_t page, uint16_t column, uint32_t read_len)
{
    uint8_t result = 0u;
    uint32_t length = read_len;
    uint8_t num_tx_bytes = 0u;
    uint8_t num_addr_bytes = 0u;
    uint8_t command_buf[4] __attribute__ ((aligned (4))) = { 0u };
    uint8_t num_dummy_cycles = 0u;

    wait_for_flash();

    command_buf[0] = PAGE_DATA_READ_OPCODE;
    command_buf[1] = 0u;
    command_buf[2] = (page >> 8u) & 0xFF;
    command_buf[3] = page & 0xFF;

    qspi_transfer_block(0u, command_buf, 3u, NULL, 0u, 0u);
    wait_for_flash();

    command_buf[1] = (column >> 8u) & 0xFF;
    command_buf[2] = column & 0xFF;
    num_addr_bytes = 2u;
    num_tx_bytes = 0u;

    switch (g_qspi_config.io_format) {
        case W25_QUAD_EX_RO:
            command_buf[0] = FAST_READ_QUAD_IO_OPCODE;
            num_dummy_cycles = 4u;
            break;
        case W25_DUAL_EX_RO:
            command_buf[0] = FAST_READ_DUAL_OUTPUT_OPCODE;
            num_dummy_cycles = 8u;
            break;
        default:
            command_buf[0] = FAST_READ_OPCODE;
            num_dummy_cycles = 8u;
            break;
    }

    if (read_len > PAGE_LENGTH) {
        length = PAGE_LENGTH;
    }

    w25_io_format temp = g_qspi_config.io_format;
    g_qspi_config.io_format = W25_NORMAL;

    qspi_transfer_block(num_addr_bytes, command_buf, num_tx_bytes, p_rx_buf, length, num_dummy_cycles);

    g_qspi_config.io_format = temp;

    read_statusreg(STATUS_REG_3, (uint8_t *)&result);
    return result;
}