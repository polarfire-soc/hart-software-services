/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC MSS eMMC SD Interface Level Driver.
 *
 * This eMMC/SD Interface driver provides functions for transferring
 * configuration and programming commands to the eMMC/SD device. Functions
 * contained within the eMMC/SD interface driver are accessed through the
 * mss_mmc_if.h header file.
 *
 * SVN $Revision: 12579 $
 * SVN $Date: 2019-12-04 16:41:30 +0530 (Wed, 04 Dec 2019) $
 */
#include "mss_mmc_if.h"
#include "mss_mmc_regs.h"
#include "mss_mmc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MMC_CLEAR                       0u
#define MMC_SET                         1u
#define SHIFT_16BIT                     16u
#define DELAY_COUNT                     0xFFFFu

/***************************************************************************//**
 * Local Function Prototypes
 */
static cif_response_t response_1_parser(void);
static uint32_t process_request_checkresptype(uint8_t responsetype);
static cif_response_t cq_execute_task(uint8_t task_id);
/***************************************************************************//**
 * cif_send_cmd()
 * See ".h" for details of how to use this function.
 */
cif_response_t cif_send_cmd
(
    uint32_t cmd_arg,
    uint32_t cmd_type,
    uint8_t resp_type
)
{
    uint32_t trans_status_isr;
    cif_response_t ret_status = TRANSFER_IF_FAIL;

   /* clear all status interrupts except:
    * current limit error, card interrupt, card removal, card insertion */
    MMC->SRS12 = ~(SRS12_CURRENT_LIMIT_ERROR
                            | SRS12_CARD_INTERRUPT
                            | SRS12_CARD_REMOVAL
                            | SRS12_CARD_INSERTION);

    /* Transfer the Command to the MMC device */
    send_mmc_cmd(cmd_arg, cmd_type, resp_type, CHECK_IF_CMD_SENT_POLL);

    /* No responses for CMD 0,4,15 */
    if ((MMC_CMD_0_GO_IDLE_STATE != cmd_type) && (MMC_CMD_4_SET_DSR != cmd_type)
                                && (MMC_CMD_15_GOTO_INACTIVE_STATE != cmd_type))
    {
        trans_status_isr = MMC->SRS12;

        if (SRS12_COMMAND_COMPLETE == (trans_status_isr & SRS12_COMMAND_COMPLETE))
        {
            /* If the response is an R1/B response */
            if ((MSS_MMC_RESPONSE_R1 == resp_type) || (MSS_MMC_RESPONSE_R1B == resp_type))
            {
                ret_status = response_1_parser();
            }
            else
            {
                ret_status = TRANSFER_IF_SUCCESS;
            }
        }
        else if (SRS12_ERROR_INTERRUPT == (SRS12_ERROR_INTERRUPT & trans_status_isr))
        {
            ret_status = TRANSFER_IF_FAIL;
        }
        else
        {
            ret_status = TRANSFER_IF_FAIL;
        }
    }
    else
    {
        ret_status = TRANSFER_IF_SUCCESS;
    }
    /* clear flags for the next time */
    MMC->SRS12 = ~(SRS12_CURRENT_LIMIT_ERROR
                            | SRS12_CARD_INTERRUPT
                            | SRS12_CARD_REMOVAL
                            | SRS12_CARD_INSERTION);

    return(ret_status);
}

/***************************************************************************//**
 * The send_mmc_cmd() function transfers the eMMC/SD command and argument to the
 * eMMC/SD device and waits until the core indicates that the command has been
 * transferred successfully.
 */
void send_mmc_cmd
(
    uint32_t cmd_arg,
    uint32_t cmd_type,
    uint8_t resp_type,
    cmd_response_check_options cmd_option
)
{
    uint32_t command_information;
    uint32_t srs9, trans_status_isr;

    /* check if command line is not busy */
    do
    {
        srs9 = MMC->SRS09;
    }while ((srs9 & SRS9_CMD_INHIBIT_CMD) != NO_CMD_INHIBIT);

    command_information = process_request_checkresptype(resp_type);

    MMC->SRS02 = cmd_arg;
    MMC->SRS03 = (uint32_t)((cmd_type << CMD_SHIFT) | command_information);

    switch (cmd_option)
    {
        /* only need to wait around if expecting no response */
        case CHECK_IF_CMD_SENT_POLL:
            do
            {
                trans_status_isr = MMC->SRS12;
            }while (((SRS12_COMMAND_COMPLETE | SRS12_ERROR_INTERRUPT) & trans_status_isr) == MMC_CLEAR);
            break;
        case CHECK_IF_CMD_SENT_INT:
            break;
        case CHECK_IF_CMD_SENT_NO:    /* use when expecting a response */
            /* No check- will be checked when response received */
            break;
        default:
        /* nothing */
            break;
    }
}

/***************************************************************************//**
 * The response_1_parser() returns the contents of the Card Status Register.
 * This function checks that none of the error fields are set within the CSR
 * and the status of the READY_FOR_DATA flag (Bit 8).
 */
static cif_response_t response_1_parser(void)
{
    cif_response_t ret_status = TRANSFER_IF_FAIL;
    uint32_t response;

    response = MMC->SRS04;
    if (MMC_CLEAR == (CARD_STATUS_ALL_ERRORS_MASK & response)) /* no error */
    {
        if ((CARD_STATUS_READY_FOR_DATA & response) != MMC_CLEAR)
        {
            ret_status = TRANSFER_IF_SUCCESS;
        }
        else
        {
            ret_status = DEVICE_BUSY;
        }
    }
    return(ret_status);
}

/*****************************************************************************/
static uint32_t process_request_checkresptype(uint8_t responsetype)
{
    uint32_t command_information;

    /* check response type */
    switch (responsetype) {
    default:
    case MSS_MMC_RESPONSE_NO_RESP:
        command_information = (uint32_t)SRS3_NO_RESPONSE;
        break;
    case MSS_MMC_RESPONSE_R2:
        command_information = (uint32_t)(SRS3_RESP_LENGTH_136
                                         | SRS3_CRC_CHECK_EN);
        break;
    case MSS_MMC_RESPONSE_R3:
    case MSS_MMC_RESPONSE_R4:
        command_information = (uint32_t)SRS3_RESP_LENGTH_48;
        break;
    case MSS_MMC_RESPONSE_R1:
    case MSS_MMC_RESPONSE_R5:
    case MSS_MMC_RESPONSE_R6:
    case MSS_MMC_RESPONSE_R7:
        command_information = (uint32_t)(SRS3_RESP_LENGTH_48
                                         | SRS3_CRC_CHECK_EN
                                         | SRS3_INDEX_CHECK_EN);
        break;
    case MSS_MMC_RESPONSE_R1B:
    case MSS_MMC_RESPONSE_R5B:
        command_information = (uint32_t)(SRS3_RESP_LENGTH_48B
                                         | SRS3_CRC_CHECK_EN
                                         | SRS3_INDEX_CHECK_EN);

        break;
    }

    return (command_information);
}

/******************************************************************************/
cif_response_t cif_send_cq_direct_command
(
    uint8_t *desc_base_addr,
    uint32_t cmd_arg,
    uint32_t cmd_type,
    uint8_t resp_type,
    uint8_t task_id
)
{

    uint32_t *dcmdTaskDesc;
    uint32_t flags;
    uint32_t desc_offset;
    uint32_t reg;
    uint32_t cmd_response;
    cif_response_t ret_status = TRANSFER_IF_FAIL;

    /* Enable direct command */
    reg = MMC->CQRS02;
    reg |= (uint32_t)CQRS02_DIRECT_CMD_ENABLE;
    MMC->CQRS02 = reg;

    desc_offset = CQ_HOST_NUMBER_OF_TASKS * task_id;

    dcmdTaskDesc = (uint32_t *)(desc_base_addr + desc_offset);

    /* first prepare general task flags */
    flags = (uint32_t)(CQ_DESC_VALID |  CQ_DESC_END | CQ_DESC_ACT_TASK | CQ_DESC_INT);

    /* now prepare direct command specific flags */
    flags |= ((cmd_type & 0x3FU) << SHIFT_16BIT);

    switch (resp_type)
    {
    case MSS_MMC_RESPONSE_NO_RESP:
        flags |= (uint32_t)CQ_DESC_DCMD_RESP_TYPE_NO_RESP;
        break;
    case MSS_MMC_RESPONSE_R1:
    case MSS_MMC_RESPONSE_R4:
    case MSS_MMC_RESPONSE_R5:
        flags |= (uint32_t)CQ_DESC_DCMD_RESP_TYPE_R1_R4_R5;
        break;
    case MSS_MMC_RESPONSE_R1B:
        flags |= (uint32_t)CQ_DESC_DCMD_RESP_TYPE_R1B;
        break;
    default:
        /* nothing */
        break;
    }

    flags |= (uint32_t)CQ_DESC_DCMD_CMD_TIMING;

    dcmdTaskDesc[0] = flags;
    dcmdTaskDesc[1] = cmd_arg << SHIFT_16BIT;
    dcmdTaskDesc[2] = MMC_CLEAR;
    dcmdTaskDesc[3] = MMC_CLEAR;

    dcmdTaskDesc[4] = (uint32_t)(CQ_DESC_VALID |  CQ_DESC_END | CQ_DESC_ACT_NOP);
    dcmdTaskDesc[5] = MMC_CLEAR;
    dcmdTaskDesc[6] = MMC_CLEAR;
    dcmdTaskDesc[7] = MMC_CLEAR;

    ret_status = cq_execute_task(task_id);

    cmd_response = MMC->CQRS18;

    reg = MMC->CQRS02;
    reg &= ~(uint32_t)CQRS02_DIRECT_CMD_ENABLE;
    MMC->CQRS02 = reg;

    if (TRANSFER_IF_SUCCESS == ret_status)
    {
        if ((CARD_STATUS_ALL_ERRORS_MASK & cmd_response) == MMC_CLEAR) /* no error */
        {
            if ((CARD_STATUS_READY_FOR_DATA & cmd_response) != MMC_CLEAR)
            {
                ret_status = TRANSFER_IF_SUCCESS;
            }
            else
            {
                ret_status = DEVICE_BUSY;
            }
        }
    }
    return ret_status;
}

/******************************************************************************/
static cif_response_t cq_execute_task(uint8_t task_id)
{

    cif_response_t ret_status = TRANSFER_IF_FAIL;
    uint32_t reg;
    uint32_t trans_status_isr;
    uint32_t cmd_response;
    uint32_t value = DELAY_COUNT;

    /* Set doorbell to start processing descriptors by controller */
    reg = MMC_SET << task_id;
    MMC->CQRS10 = reg;

    while (value--);

    do
    {
        trans_status_isr = MMC->SRS12;
    }while (((SRS12_ERROR_INTERRUPT | SRS12_CMD_QUEUING_INT) & trans_status_isr) == MMC_CLEAR);

    if ((trans_status_isr & (SRS12_ERROR_INTERRUPT | SRS12_CMD_QUEUING_INT)) != MMC_CLEAR)
    {
        trans_status_isr = MMC->SRS12;
        MMC->SRS12 = trans_status_isr;

        if ((trans_status_isr & SRS12_ERROR_INTERRUPT) != MMC_CLEAR)
        {
            ret_status = TRANSFER_IF_FAIL;
        }
        if ((trans_status_isr & SRS12_CMD_QUEUING_INT) != MMC_CLEAR)
        {
            reg = MMC->CQRS04;
            MMC->CQRS04 = reg;

            if ((reg & CQRS04_RESP_ERR_INT) != MMC_CLEAR)
            {
                ret_status = TRANSFER_IF_FAIL;
            }

            if ((reg & CQRS04_TASK_COMPLETE_INT) != MMC_CLEAR)
            {
                reg = MMC->CQRS11;
                /* clear all caught notifications */
                MMC->CQRS11 = reg;
                if (task_id == CQ_DCMD_TASK_ID)
                {
                    if ((reg & (MMC_SET << task_id)) != MMC_CLEAR)
                    {
                        cmd_response = MMC->CQRS18;
                        ret_status = TRANSFER_IF_SUCCESS;
                    }
                }
                else
                {
                    ret_status = TRANSFER_IF_SUCCESS;
                }
            }
        }
    }
    else
    {
        ret_status = TRANSFER_IF_FAIL;
    }

    (void) cmd_response; // unused, so referencing to avoid compiler warning
    return ret_status;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
