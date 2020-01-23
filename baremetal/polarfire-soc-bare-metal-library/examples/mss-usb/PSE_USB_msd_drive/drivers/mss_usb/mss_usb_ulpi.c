/*******************************************************************************
 * (c) Copyright 2018 Microsemi SoC Products Group.  All rights reserved.
 * 
 * G5SoC Microcontroller Subsystem (MSS) USB controller ULPI register access.
 *
 * SVN $Revision: 10280 $
 * SVN $Date: 2018-08-01 16:36:37 +0530 (Wed, 01 Aug 2018) $
 */

#include "mss_usb_ulpi.h"
#include "mss_usb_core_regs.h"

#define ULPI_CTRL_RW_REQ_MASK       0x01u
#define ULPI_CTRL_COMPLETE_MASK     0x02u
#define ULPI_CTRL_RDN_WR_MASK       0x04u

uint8_t ulpi_read(uint8_t address)
{
    uint8_t ulpi_reg_value;
    uint8_t ulpi_ctrl;
    uint8_t ulpi_complete;
    
    /* Set address of register to read. */
    USB->ULPI_ADDR_REG = address;
    
    /* Set ULPIRdnWr bit of ULPIRegControl to 1. */
    ulpi_ctrl = ULPI_CTRL_RDN_WR_MASK | ULPI_CTRL_RW_REQ_MASK;
    USB->ULPI_CTRL_REG = ulpi_ctrl;
    
    /* Poll for complete. */
    do
    {
        ulpi_ctrl = USB->ULPI_CTRL_REG;
        ulpi_complete = ulpi_ctrl & ULPI_CTRL_COMPLETE_MASK;
    } while(0u == ulpi_complete);
    
    /* Read returned data. */
    ulpi_reg_value = USB->ULPI_DATA_REG;
    
    /* Clear the complete bit. */
    ulpi_ctrl = USB->ULPI_CTRL_REG;
    ulpi_ctrl &= ULPI_CTRL_COMPLETE_MASK;
    USB->ULPI_CTRL_REG = ulpi_ctrl;
    
    return ulpi_reg_value;
}

void ulpi_write(uint8_t address, uint8_t value)
{
    uint8_t ulpi_ctrl;
    uint8_t ulpi_complete;
    
    /* Set address of register to write. */
    USB->ULPI_ADDR_REG = address;
    
    /* write data to ULPI data register. */
    USB->ULPI_DATA_REG = value;
    
    /* Initiate register access with ULPIRdnWr bit set to 0. */
    ulpi_ctrl = ULPI_CTRL_RW_REQ_MASK;
    USB->ULPI_CTRL_REG = ulpi_ctrl;
    
    /* Poll for complete. */
    do
    {
        ulpi_ctrl = USB->ULPI_CTRL_REG;
        ulpi_complete = ulpi_ctrl & ULPI_CTRL_COMPLETE_MASK;
    } while(0u == ulpi_complete);
    
    /* Clear the complete bit. */
    ulpi_ctrl = USB->ULPI_CTRL_REG;
    ulpi_ctrl &= ULPI_CTRL_COMPLETE_MASK;
    USB->ULPI_CTRL_REG = ulpi_ctrl;
}

