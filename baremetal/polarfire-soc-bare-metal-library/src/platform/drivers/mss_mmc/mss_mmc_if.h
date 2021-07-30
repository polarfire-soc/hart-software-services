/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC MSS eMMC SD Interface Level Header File.
 *
 * This eMMC SD Interface header file provides access to functions which are
 * used to configure and program the eMMC/SD device to allow data transfers
 * to be performed with the eMMC/SD Host.
 *
 * SVN $Revision: 12579 $
 * SVN $Date: 2019-12-04 16:41:30 +0530 (Wed, 04 Dec 2019) $
 */

#ifndef __MSS_MMC_IF_H
#define __MSS_MMC_IF_H

#ifdef __cplusplus
extern "C"
#endif

#include "hal/cpu_types.h"

/***************************************************************************//**
 * Macro Definitions
 */

#define CMD_SHIFT               24u
#define NO_CMD_INHIBIT          0u

/***************************************************************************//**
  The cif_response type is used to specify the status of the eMMC/SD command
  transfer to the eMMC/SD device. A value of this type is returned by the
  cif_send_cmd() function.
 */
typedef enum
{
    TRANSFER_IF_FAIL = 0u,
    TRANSFER_IF_SUCCESS = 1u,
    DEVICE_BUSY = 2u,
} cif_response_t;


typedef enum
{
    CHECK_IF_CMD_SENT_POLL      = 0u,
    CHECK_IF_CMD_SENT_INT       = 1u,
    CHECK_IF_CMD_SENT_NO        = 2u
} cmd_response_check_options;


/***************************************************************************//**

  The cif_send_cmd() function handles the interface level command and response
  data for communicating with the eMMC/SD device. This function issues
  configuration and control commands to the device, waits on the status register
  to update indicating that there was a response received (were expected) and
  parses the response to determine the successfulness of the transfer.

  @param cmd_arg
  The cmd_arg parameter specifies the eMMC/SD argument to be passed to the
  eMMC/SD device.

  @param cmd_type
  The cmd_type parameter specifies the eMMC/SD Command type to be passed to the
  eMMC/SD  device.

  @param resp_type
  The resp_type parameter specifies the eMMC/SD response type to be received from
  eMMC/SD device.

  @return
  This function returns a value of type cif_response_t representing the
  successfulness of the transfer. If this return value indicates that the
  eMMC/SD device is busy, subsequent actions must be taken to ensure that a
  command is not issued until the device returns to idle.

  Example:
  @code
    #define MMC_DW_CSD        0x03B70300u
    #define MMC_CMD_SWITCH    6u
    cif_response_t response_status;

    response_status = cif_send_cmd(MMC_DW_CSD, MMC_CMD_SWITCH, MMC_RESPONSE_R1B);

    while(DEVICE_BUSY == response_status)
    {
        response_status = cif_send_cmd(RCA_VALUE,
                                       MMC_CMD_13_SEND_STATUS,
                                       MMC_RESPONSE_R1);
    }
  @endcode
 */
cif_response_t cif_send_cmd
(
    uint32_t cmd_arg,
    uint32_t cmd_type,
    uint8_t resp_type
);

/***************************************************************************//**

  The send_mmc_cmd() function handles the interface level command and response
  data for communicating with the eMMC/SD device. This function issues
  configuration and control commands to the device, waits on the status register
  to update indicating that there was a response received (were expected) and
  parses the response to determine the successfulness of the transfer.

  @param cmd_arg
  The cmd_arg parameter specifies the eMMC/SD argument to be passed to the
  eMMC/SD  device.

  @param cmd_type
  The cmd_type parameter specifies the eMMC/SD Command type to be passed to the
  eMMC/SD device.

  @param resp_type
  The resp_type parameter specifies the eMMC/SD response type to be received from
  eMMC/SD device.

  @param cmd_option
  The cmd_option parameter specifies if the function checks if eMMC/SD has sent
  the command or not before returning. There is no need to check if you are
  expecting a response, just check for the response.

  @return
  This function returns a value of type cif_response_t representing the
  successfulness of the transfer. If this return value indicates that the
  eMMC/SD device is busy, subsequent actions must be taken to ensure that a
  command is not issued until the device returns to idle.

  Example:
  @code

    send_mmc_cmd(RCA_VALUE, MMC_CMD_13_SEND_STATUS, MMC_RESPONSE_R1, CHECK_IF_CMD_SENT_NO);
  @endcode
 */
void send_mmc_cmd
(
    uint32_t cmd_arg,
    uint32_t cmd_type,
    uint8_t resp_type,
    cmd_response_check_options cmd_option
);
/******************************************************************************/
cif_response_t cif_send_cq_direct_command
(
        uint8_t *desc_base_addr,
        uint32_t cmd_arg,
        uint32_t cmd_type,
        uint8_t resp_type,
        uint8_t task_id
);
/******************************************************************************/


#ifdef __cplusplus
}
#endif

#endif  /* __MSS_MMC_IF_H */
