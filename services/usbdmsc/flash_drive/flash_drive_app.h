/***************************************************************************//**
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * USB MSC Class Storage Device example application to demonstrate the
 * PolarFire MSS USB operations in device mode.
 *
 * Header for flash_drive_app.h
 *
 */

#ifndef FLASH_DRIVE_APP_H_
#define FLASH_DRIVE_APP_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
  Exported functions from this file
*/

/***************************************************************************//**
  @brief FLASH_DRIVE_init()


  @param
    This function does not take any parameters.
  @return
    This function returns true if initialization succeeded, else false

  Example:
  @code
  @endcode
*/
bool FLASH_DRIVE_init(void);

/***************************************************************************//**
  @brief FLASH_DRIVE_init()


  @param
    This function does not take any parameters.
  @return
    This function returns a non-zero value when it has received GET_CAPACITY
    command from the host. This is taken as indication that the a working USB
    host connection is now established

  Example:
  @code
  @endcode
*/
uint32_t FLASH_DRIVE_is_host_connected(void);

void FLASH_DRIVE_dump_xfer_status(void);

#ifdef __cplusplus
}
#endif

#endif /* FLASH_DRIVE_APP_H_*/
