/*******************************************************************************
 * (c) Copyright 2018 Microsemi SoC Products Group.  All rights reserved.
 * 
 * G5SoC Microcontroller Subsystem (MSS) USB controller ULPI register access.
 *
 * SVN $Revision: 10280 $
 * SVN $Date: 2018-08-01 16:36:37 +0530 (Wed, 01 Aug 2018) $
 */

#ifndef USB_ULPI_READ_WRITE_H
#define USB_ULPI_READ_WRITE_H

#include <stdint.h>

uint8_t ulpi_read(uint8_t address);

void ulpi_write(uint8_t address, uint8_t value);

#endif  /* USB_ULPI_READ_WRITE_H */
