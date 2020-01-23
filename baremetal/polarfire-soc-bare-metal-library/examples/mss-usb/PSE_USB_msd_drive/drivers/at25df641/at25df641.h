/***************************************************************************//**
 * (c) Copyright 2009 Actel Corporation.  All rights reserved.
 * 
 *  Atmel AT25DF641 SPI flash driver API.
 *
 * SVN $Revision:$
 * SVN $Date:$
 */

#ifndef __AT25DF641_SPI_FLASH_H_
#define __AT25DF641_SPI_FLASH_H_

#include <stdint.h>
#include <stdlib.h>

#define NO_OF_512_BLOCKS 127
//#define NO_OF_512_BLOCKS 1000
//#define NO_OF_512_BLOCKS 2000
void FLASH_init( void );

void FLASH_read_device_id
(
    uint8_t * manufacturer_id,
    uint8_t * device_id
);

void FLASH_read
(
    uint32_t address,
    uint8_t * rx_buffer,
    size_t size_in_bytes
);

void FLASH_global_unprotect( void );

void FLASH_chip_erase( void );

void FLASH_erase_4k_block
(
    uint32_t address
);

uint8_t FLASH_get_status( void );

void FLASH_program
(
    uint32_t address,
    uint8_t * write_buffer,
    size_t size_in_bytes
);

void flash_image_update(void);
#endif
