/*******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

/*******************************************************************************
 * @file mss_ddr_debug.h
 * @author Microchip-FPGA Embedded Systems Solutions
 * @brief mss_ddr_debug related defines
 *
 */

/*=========================================================================*//**
  @page DDR setup and monitoring
  ==============================================================================
  @section intro_sec Introduction
  ==============================================================================
  DDR debug helper functions

  ==============================================================================
  @section Items located in the north west corner
  ==============================================================================
  -

  ==============================================================================
  @section Overview of DDR related hardware
  ==============================================================================

 *//*=========================================================================*/

#include <stddef.h>
#include <stdint.h>


#ifndef __MSS_DDR_DEBUG_H_
#define __MSS_DDR_DEBUG_H_ 1

#ifdef DEBUG_DDR_INIT

#include "drivers/mss_mmuart/mss_uart.h"



#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************//**
 The ddr_read_write_fn function is used to write/read test patterns to the DDR

  @return
    This function returns 0 if successful, number of errors if not.

  Example:
  @code

    if (ddr_read_write_fn() != 0U)
    {
        .. warn the user, increment error count , wait for watchdog reset
    }

  @endcode
 */
uint32_t
ddr_read_write_fn
(
uint64_t* DDR_word_ptr,
uint32_t no_access,
uint32_t pattern
);

/***************************************************************************//**
  The uprint32() function is used to print to the designated debug port

  Example:
  @code

  (void)uprint32(g_debug_uart, "\n\r DDR_TRAINING_FAIL: ", error);

  @endcode
 */
void
uprint32
(
mss_uart_instance_t * uart,
const char* msg,
uint32_t d
);

/***************************************************************************//**
  The setup_ddr_debug_port() function is used to setup a serial port dedicated
  to printing information on the DDR start-up.

  @return
    This function returns 0 if successful

  Example:
  @code

    if (ddr_setup() != 0U)
    {
        .. warn the user, increment error count , wait for watchdog reset
    }

  @endcode
 */
uint32_t
setup_ddr_debug_port
(
mss_uart_instance_t * uart
);


void
sweep_status
(
mss_uart_instance_t *g_mss_uart_debug_pt
);

#endif


#ifdef __cplusplus
}
#endif

#endif /* __MSS_DDRC_H_ */


