/**************************************************************************//**
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * 
 *  Micron MT25QL01GBBB SPI Flash driver implementation.
 *
 * SVN $Revision:$
 * SVN $Date:$
 */

#include "mt25ql01gbbb.h"

#define DEVICE_ID_READ          0x9F
#define READ_CMD                0x03
#define WRITE_ENABLE_CMD        0x06
#define WRITE_DISABLE_CMD       0x04
#define PROGRAM_PAGE_CMD        0x02
#define WRITE_STATUS1_OPCODE    0x01
#define CHIP_ERASE_OPCODE       0xC4
#define ERASE_4K_BLOCK_OPCODE   0x20
#define ERASE_32K_BLOCK_OPCODE  0x52
#define ERASE_64K_BLOCK_OPCODE  0xD8
#define READ_STATUS             0x05
#define PROGRAM_RESUME_CMD      0xD0
#define READ_SECTOR_PROTECT     0x3C

#define READY_BIT_MASK          0x01
#define PROTECT_SECTOR_OPCODE     0x36
#define UNPROTECT_SECTOR_OPCODE 0x39

#define DONT_CARE               0x00u

#define NB_BYTES_PER_PAGE       256

#define BLOCK_ALIGN_MASK_4K      0xFFFFF000
#define BLOCK_ALIGN_MASK_32K     0xFFFF8000
#define BLOCK_ALIGN_MASK_64K     0xFFFF0000

/*******************************************************************************
 * Local functions
 */
static void wait_ready(mss_spi_instance_t *);
static uint8_t wait_ready_erase(mss_spi_instance_t *);

/*******************************************************************************
 */
void FLASH_init
(
    mss_spi_instance_t *this_spi,
    mss_spi_oveflow_handler_t mss_spi_reset
)
{
 /*--------------------------------------------------------------------------
     * Configure SPI.
     */
    MSS_SPI_init(this_spi);
    
    MSS_SPI_configure_master_mode
        (
            this_spi,
            MSS_SPI_SLAVE_1,
            MSS_SPI_MODE3,
            4,
            MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE,
            mss_spi_reset
        );

}

/*******************************************************************************
 */
void FLASH_read_device_id
(
    mss_spi_instance_t *this_spi,
    uint8_t * manufacturer_id,
    uint8_t * device_id
)
{
    uint8_t read_device_id_cmd = DEVICE_ID_READ;
    uint8_t read_buffer[3];
    
    MSS_SPI_set_slave_select(this_spi, MSS_SPI_SLAVE_1);
    MSS_SPI_transfer_block(this_spi, &read_device_id_cmd, 1, read_buffer,
                           sizeof(read_buffer));

    MSS_SPI_clear_slave_select(this_spi, MSS_SPI_SLAVE_1);

    *manufacturer_id = read_buffer[0];
    *device_id = read_buffer[1];

}

/*******************************************************************************
 */
void FLASH_read
(
    mss_spi_instance_t *this_spi,
    uint32_t address,
    uint8_t * rx_buffer,
    size_t size_in_bytes
)
{
    uint8_t cmd_buffer[6];
    
    cmd_buffer[0] = READ_CMD;
    cmd_buffer[1] = (uint8_t)((address >> 16) & 0xFF);
    cmd_buffer[2] = (uint8_t)((address >> 8) & 0xFF);;
    cmd_buffer[3] = (uint8_t)(address & 0xFF);
    cmd_buffer[4] = DONT_CARE;
    cmd_buffer[5] = DONT_CARE;
    
    MSS_SPI_set_slave_select(this_spi, MSS_SPI_SLAVE_1);
    wait_ready_erase(this_spi);
    wait_ready(this_spi);
    MSS_SPI_transfer_block(this_spi, cmd_buffer, 4, rx_buffer, size_in_bytes);
    wait_ready(this_spi);
    MSS_SPI_clear_slave_select(this_spi, MSS_SPI_SLAVE_1);
    
}

/*******************************************************************************
 */
void FLASH_global_unprotect
(
    mss_spi_instance_t *this_spi
)
{
    uint8_t cmd_buffer[2];
    /* Send Write Enable command */
    cmd_buffer[0] = WRITE_ENABLE_CMD;

    MSS_SPI_set_slave_select(this_spi, MSS_SPI_SLAVE_1);
    wait_ready(this_spi);
    MSS_SPI_transfer_block(this_spi, cmd_buffer, 1, 0, 0);
    
    /* Send Chip Erase command */
    cmd_buffer[0] = WRITE_STATUS1_OPCODE;
    cmd_buffer[1] = 0;
    
    wait_ready(this_spi);
    MSS_SPI_transfer_block(this_spi, cmd_buffer, 2, 0, 0);
    wait_ready(this_spi);
    MSS_SPI_clear_slave_select(this_spi, MSS_SPI_SLAVE_1);
}

/*******************************************************************************
 */
void FLASH_chip_erase
(
    mss_spi_instance_t *this_spi
)
{
    uint8_t cmd_buffer;
    /* Send Write Enable command */
    cmd_buffer = WRITE_ENABLE_CMD;

    MSS_SPI_set_slave_select(this_spi, MSS_SPI_SLAVE_1);
    wait_ready(this_spi);
    MSS_SPI_transfer_block(this_spi, &cmd_buffer, 1, 0, 0);
    
    /* Send Chip Erase command */
    cmd_buffer = CHIP_ERASE_OPCODE;
    
    wait_ready(this_spi);
    MSS_SPI_transfer_block(this_spi, &cmd_buffer, 1, 0, 0);
    wait_ready(this_spi);
    MSS_SPI_clear_slave_select(this_spi, MSS_SPI_SLAVE_1);
}

/*******************************************************************************
 */
void FLASH_erase_4k_block
(
    mss_spi_instance_t *this_spi,
    uint32_t address
)
{
    uint8_t cmd_buffer[4];

    address &= BLOCK_ALIGN_MASK_4K;

    /* Send Write Enable command */
    cmd_buffer[0] = WRITE_ENABLE_CMD;

    MSS_SPI_set_slave_select(this_spi, MSS_SPI_SLAVE_1);
    wait_ready(this_spi);
    MSS_SPI_transfer_block(this_spi, cmd_buffer, 1, 0, 0);
    
    /* Send Chip Erase command */
    cmd_buffer[0] = ERASE_4K_BLOCK_OPCODE;
    cmd_buffer[1] = (address >> 16) & 0xFF;
    cmd_buffer[2] = (address >> 8) & 0xFF;
    cmd_buffer[3] = address & 0xFF;
    
    wait_ready(this_spi);
    wait_ready_erase(this_spi);
    MSS_SPI_transfer_block(this_spi, cmd_buffer, sizeof(cmd_buffer), 0, 0);
    wait_ready(this_spi);
    wait_ready_erase(this_spi);
    MSS_SPI_clear_slave_select(this_spi, MSS_SPI_SLAVE_1);
}

/*******************************************************************************
 */
void FLASH_erase_64k_block
(
    mss_spi_instance_t *this_spi,
    uint32_t address
)
{
    uint8_t cmd_buffer[4];

    address &= BLOCK_ALIGN_MASK_64K;

    /* Send Write Enable command */
    cmd_buffer[0] = WRITE_ENABLE_CMD;

    MSS_SPI_set_slave_select(this_spi, MSS_SPI_SLAVE_1);
    wait_ready(this_spi);
    MSS_SPI_transfer_block(this_spi, cmd_buffer, 1, 0, 0);

    /* Send Chip Erase command */
    cmd_buffer[0] = ERASE_64K_BLOCK_OPCODE;
    cmd_buffer[1] = (address >> 16) & 0xFF;
    cmd_buffer[2] = (address >> 8) & 0xFF;
    cmd_buffer[3] = address & 0xFF;

    wait_ready(this_spi);
    MSS_SPI_transfer_block(this_spi, cmd_buffer, sizeof(cmd_buffer), 0, 0);
    wait_ready(this_spi);
    MSS_SPI_clear_slave_select(this_spi, MSS_SPI_SLAVE_1);
}

/*******************************************************************************
 */
void write_cmd_data
(
    mss_spi_instance_t * this_spi,
    const uint8_t * cmd_buffer,
    uint16_t cmd_byte_size,
    uint8_t * data_buffer,
    uint16_t data_byte_size
)
{
    uint8_t tx_buffer[516];
    uint16_t transfer_size;
    uint16_t idx = 0;
    
    transfer_size = cmd_byte_size + data_byte_size;
    
    for(idx = 0; idx < cmd_byte_size; ++idx)
    {
        tx_buffer[idx] = cmd_buffer[idx];
    }

    for(idx = 0; idx < data_byte_size; ++idx)
    {
        tx_buffer[cmd_byte_size + idx] = data_buffer[idx];
    }

    MSS_SPI_transfer_block(this_spi, tx_buffer, transfer_size, 0, 0);
    
}
        
/*******************************************************************************
 */
void FLASH_program
(
    mss_spi_instance_t *this_spi,
    uint32_t address,
    uint8_t * write_buffer,
    size_t size_in_bytes
)
{
    uint8_t cmd_buffer[4];
    
    uint32_t in_buffer_idx;
    uint32_t nb_bytes_to_write;
    uint32_t target_addr;

    MSS_SPI_set_slave_select(this_spi, MSS_SPI_SLAVE_1);
    
    /* Send Write Enable command */
    cmd_buffer[0] = WRITE_ENABLE_CMD;
    wait_ready(this_spi);
    MSS_SPI_transfer_block(this_spi, cmd_buffer, 1, 0, 0);
    
    /* Unprotect sector */
    cmd_buffer[0] = UNPROTECT_SECTOR_OPCODE;
    cmd_buffer[1] = (address >> 16) & 0xFF;
    cmd_buffer[2] = (address >> 8) & 0xFF;
    cmd_buffer[3] = address & 0xFF;
    wait_ready(this_spi);
    MSS_SPI_transfer_block(this_spi, cmd_buffer, sizeof(cmd_buffer), 0, 0);
    wait_ready_erase(this_spi);

    /* Send Write Enable command */
    cmd_buffer[0] = WRITE_ENABLE_CMD;
    wait_ready(this_spi);
    MSS_SPI_transfer_block(this_spi, cmd_buffer, 1, 0, 0);
    
    /**/
    in_buffer_idx = 0;
    nb_bytes_to_write = size_in_bytes;
    target_addr = address;
    
    while (in_buffer_idx < size_in_bytes)
    {
        wait_ready_erase(this_spi);
        uint32_t size_left;

        /* adjust max possible size to page boundary. */
        nb_bytes_to_write = 0x100 - (target_addr & 0xFF);
        size_left = size_in_bytes - in_buffer_idx;

        if (size_left < nb_bytes_to_write)
        {
            nb_bytes_to_write = size_left;
        }
        
        wait_ready(this_spi);
        
        /* Send Write Enable command */
        cmd_buffer[0] = WRITE_ENABLE_CMD;
        MSS_SPI_transfer_block(this_spi, cmd_buffer, 1, 0, 0);
            
        /* Program page */
        wait_ready(this_spi);
    
        cmd_buffer[0] = PROGRAM_PAGE_CMD;
        cmd_buffer[1] = (target_addr >> 16) & 0xFF;
        cmd_buffer[2] = (target_addr >> 8) & 0xFF;
        cmd_buffer[3] = target_addr & 0xFF;
        
        write_cmd_data
          (
            this_spi,
            cmd_buffer,
            sizeof(cmd_buffer),
            &write_buffer[in_buffer_idx],
            nb_bytes_to_write
          );
        
        target_addr += nb_bytes_to_write;
        in_buffer_idx += nb_bytes_to_write;
    }
    
    /* Send Write Disable command. */
    cmd_buffer[0] = WRITE_DISABLE_CMD;

    wait_ready(this_spi);

    MSS_SPI_transfer_block(this_spi, cmd_buffer, 1, 0, 0);
    MSS_SPI_clear_slave_select(this_spi, MSS_SPI_SLAVE_1);
}

/*******************************************************************************
 */
uint8_t FLASH_get_status
(
    mss_spi_instance_t *this_spi
)
{
    uint8_t status;
    uint8_t command = READ_STATUS;
    
    MSS_SPI_transfer_block(this_spi, &command, sizeof(uint8_t), &status,
                           sizeof(status));

    return status;
}

/*******************************************************************************
 */
static void wait_ready
(
    mss_spi_instance_t *this_spi
)
{
    uint8_t ready_bit;
    uint8_t command = READ_STATUS;
    
    do {
        MSS_SPI_transfer_block(this_spi, &command, sizeof(command),
                               &ready_bit, sizeof(ready_bit));
        ready_bit = ready_bit & READY_BIT_MASK;
    } while (ready_bit == 1);
}

/*******************************************************************************
 */
static uint8_t wait_ready_erase
(
    mss_spi_instance_t *this_spi
)
{
    uint32_t count = 0;
    uint8_t ready_bit;
    uint8_t command = 0x70 ; /* FLAG_READ_STATUS; */
    do {
        MSS_SPI_transfer_block(this_spi, &command, sizeof(command),
                               &ready_bit, sizeof(ready_bit));
        count++;
    } while ((ready_bit & 0x80) == 0);

    return (ready_bit);
}
