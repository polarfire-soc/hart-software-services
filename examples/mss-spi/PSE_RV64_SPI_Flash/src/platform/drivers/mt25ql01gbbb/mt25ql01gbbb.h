/**************************************************************************//**
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * 
 *  Micron MT25QL01GBBB SPI flash driver API.
 *
 * SVN $Revision:$
 * SVN $Date:$
 */

#ifndef __MT25QL01G_SPI_FLASH_H_
#define __MT25QL01G_SPI_FLASH_H_

#include <stdint.h>
#include <stdlib.h>
#include "drivers/mss_spi/mss_spi.h"

/***************************************************************************//**
 */
void FLASH_init
(
    mss_spi_instance_t *this_spi,
    mss_spi_oveflow_handler_t mss_spi_reset
);

/***************************************************************************//**
 */
void FLASH_read_device_id
(
    mss_spi_instance_t *this_spi,
    uint8_t * manufacturer_id,
    uint8_t * device_id
);

/***************************************************************************//**
 */
void FLASH_read
(
    mss_spi_instance_t *this_spi,
    uint32_t address,
    uint8_t * rx_buffer,
    size_t size_in_bytes
);

/***************************************************************************//**
 */
void FLASH_global_unprotect
(
    mss_spi_instance_t *this_spi
);

/***************************************************************************//**
 */
void FLASH_chip_erase
(
    mss_spi_instance_t *this_spi
);

/***************************************************************************//**
 */
void FLASH_erase_64k_block
(
    mss_spi_instance_t *this_spi,
    uint32_t address
);

/***************************************************************************//**
 */
void FLASH_erase_4k_block
(
    mss_spi_instance_t *this_spi,
    uint32_t address
);

/***************************************************************************//**
 */
uint8_t FLASH_get_status
(
    mss_spi_instance_t *this_spi
);

/***************************************************************************//**
 */
void FLASH_program
(
    mss_spi_instance_t *this_spi,
    uint32_t address,
    uint8_t * write_buffer,
    size_t size_in_bytes
);

#endif
