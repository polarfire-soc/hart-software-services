/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PoalrFire SoC Microprocessor Subsystem PDMA bare metal driver implementation.
 *
 * SVN $Revision$
 * SVN $Date$
 */

#include "drivers/mss_pdma/mss_pdma.h"
#include "mss_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MACRO to set the correct channel memory offset for the memory mapped
 * configuration register.
 */
#define MSS_PDMA_REG_OFFSET(x)  \
                    (uint64_t)(PDMA_REG_BASE + (PDMA_CHL_REG_OFFSET * (x)))

/* Default is maximum transaction size for both write_size and read_size. */
uint8_t g_channel_nextcfg_wsize[MSS_PDMA_lAST_CHANNEL] = { 0x0Fu, 0x0Fu, 0x0Fu, 0x0Fu };
uint8_t g_channel_nextcfg_rsize[MSS_PDMA_lAST_CHANNEL] = { 0x0Fu, 0x0Fu, 0x0Fu, 0x0Fu };

/*-------------------------------------------------------------------------*//**
 * MSS_PDMA_init()
 * See pse_pdma.h for description of this function.
 */
void
MSS_PDMA_init
(
    void
)
{
    ;
}

/*-------------------------------------------------------------------------*//**
 * MSS_PDMA_setup_transfer()
 * See pse_pdma.h for description of this function.
 */
mss_pdma_error_id_t
MSS_PDMA_setup_transfer
(
    mss_pdma_channel_id_t channel_id,
    mss_pdma_channel_config_t *channel_config
)
{
    if (channel_id > MSS_PDMA_CHANNEL_3)
    {
        return MSS_PDMA_ERROR_INVALID_CHANNEL_ID;
    }

    /* Set the register structure pointer for the PDMA channel. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET(channel_id);

    /* Basic House Keeping, return if errors exist. */
    if (channel_config->src_addr == 0u)
    {
        return MSS_PDMA_ERROR_INVALID_SRC_ADDR;
    }

    if (channel_config->dest_addr == 0u)
    {
        return MSS_PDMA_ERROR_INVALID_DEST_ADDR;
    }

    /* If a run transaction is in progress, return error.
     * Channel can only be claimed when run is low */
    if (pdmareg->control_reg & MASK_PDMA_CONTROL_RUN)
    {
        return MSS_PDMA_ERROR_TRANSACTION_IN_PROGRESS;
    }

    /* Set or clear the interrupts for the transfer. */
    if (channel_config->enable_done_int)
    {
        pdmareg->control_reg |= ((uint32_t)MASK_PDMA_ENABLE_DONE_INT);
    }
    else
    {
        pdmareg->control_reg &= ~((uint32_t)MASK_PDMA_ENABLE_DONE_INT);
    }

    if (channel_config->enable_err_int)
    {
        pdmareg->control_reg |= ((uint32_t)MASK_PDMA_ENABLE_ERR_INT);
    }
    else
    {
        pdmareg->control_reg &= ~((uint32_t)MASK_PDMA_ENABLE_ERR_INT);
    }

    /* clear Next registers. */
    pdmareg->control_reg |= (uint32_t)MASK_CLAIM_PDMA_CHANNEL;

    /* Setup the source and destination addresses.*/
    pdmareg->next_destination = channel_config->dest_addr;
    pdmareg->next_source      = channel_config->src_addr;

    /* Set the transfer size. */
    pdmareg->next_bytes       = channel_config->num_bytes;

    /* Setup repeat and force order requirements. */
    if (channel_config->repeat)
    {
        pdmareg->next_config |= MASK_REPEAT_TRANSCTION;
    }
    else
    {
        pdmareg->next_config &= ~((uint32_t)MASK_REPEAT_TRANSCTION);
    }

    if (channel_config->force_order)
    {
        pdmareg->next_config |= ((uint32_t)MASK_FORCE_ORDERING);
    }
    else
    {
        pdmareg->next_config &= ~((uint32_t)MASK_FORCE_ORDERING);
    }

    /* PDMA transaction size.. set to maximum. */
    pdmareg->next_config |=
            (g_channel_nextcfg_wsize[channel_id] << SHIFT_CH_CONFIG_WSIZE);
    pdmareg->next_config |=
            (g_channel_nextcfg_rsize[channel_id] << SHIFT_CH_CONFIG_RSIZE);

    return MSS_PDMA_OK;
}

/***************************************************************************//**
 * See pse_pdma.h for description of this function.
 */
mss_pdma_error_id_t
MSS_PDMA_set_transction_size
(
    mss_pdma_channel_id_t channel_id,
    uint8_t write_size,
    uint8_t read_size
)
{
    if (channel_id > MSS_PDMA_CHANNEL_3)
    {
        return MSS_PDMA_ERROR_INVALID_CHANNEL_ID;
    }

    if (write_size > 0x0Fu)
    {
        return MSS_PDMA_ERROR_INVALID_NEXTCFG_WSIZE;
    }

    if (read_size > 0x0Fu)
    {
        return MSS_PDMA_ERROR_INVALID_NEXTCFG_RSIZE;
    }

    g_channel_nextcfg_wsize[channel_id] = write_size;
    g_channel_nextcfg_rsize[channel_id] = read_size;

    return MSS_PDMA_OK;
}

/***************************************************************************//**
 * See pse_pdma.h for description of this function.
 */
mss_pdma_error_id_t
MSS_PDMA_start_transfer
(
    mss_pdma_channel_id_t channel_id
)
{
    if (channel_id > MSS_PDMA_CHANNEL_3)
    {
        return MSS_PDMA_ERROR_INVALID_CHANNEL_ID;
    }

    /* Set the register structure pointer for the PDMA channel. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET(channel_id);

    /* If a run transaction is in progress, return error.
     * Channel can only be claimed when run is low */
    pdmareg->control_reg |= ((uint32_t)MASK_PDMA_CONTROL_RUN);

    return MSS_PDMA_OK;
}

/***************************************************************************//**
 * See pse_pdma.h for description of this function.
 */
uint32_t
MSS_PDMA_get_active_transfer_type
(
    mss_pdma_channel_id_t channel_id
)
{
    if (channel_id > MSS_PDMA_CHANNEL_3)
    {
        return 0u;
    }

    /* Set the register structure pointer for the PDMA channel. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET(channel_id);

    return pdmareg->exec_config;
}

/***************************************************************************//**
 * See pse_pdma.h for description of this function.
 */
uint64_t
MSS_PDMA_get_number_bytes_remaining
(
    mss_pdma_channel_id_t channel_id
)
{
    if (channel_id > MSS_PDMA_CHANNEL_3)
    {
        return 0u;
    }

    /* Set the register structure pointer for the PDMA channel. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET(channel_id);

    return pdmareg->exec_bytes;
}

/***************************************************************************//**
 * See pse_pdma.h for description of this function.
 */
uint64_t
MSS_PDMA_get_destination_current_addr
(
    mss_pdma_channel_id_t channel_id
)
{
    if (channel_id > MSS_PDMA_CHANNEL_3)
    {
        return 0u;
    }

    /* Set the register structure pointer for the PDMA channel. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET(channel_id);

    return pdmareg->exec_destination;
}

/***************************************************************************//**
 * See pse_pdma.h for description of this function.
 */
uint64_t
MSS_PDMA_get_source_current_addr
(
    mss_pdma_channel_id_t channel_id
)
{
    if (channel_id > MSS_PDMA_CHANNEL_3)
    {
        return 0u;
    }

    /* Set the register structure pointer for the PDMA channel. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET(channel_id);

    return pdmareg->exec_source;
}

/***************************************************************************//**
 * See pse_pdma.h for description of this function.
 */
uint8_t
MSS_PDMA_get_transfer_complete_status
(
    mss_pdma_channel_id_t channel_id
)
{
    if (channel_id > MSS_PDMA_CHANNEL_3)
    {
        return 0u;
    }

    /* Set the register structure pointer for the PDMA channel. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET(channel_id);

    if (pdmareg->control_reg & MASK_PDMA_TRANSFER_DONE)
    {
        return 1u;
    }
    else
    {
        return 0u;
    }
}

/***************************************************************************//**
 * See pse_pdma.h for description of this function.
 */
uint8_t
MSS_PDMA_get_transfer_error_status
(
    mss_pdma_channel_id_t channel_id
)
{
    if (channel_id > MSS_PDMA_CHANNEL_3)
    {
        return 0u;
    }
    /* Set the register structure pointer for the PDMA channel. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET(channel_id);

    if (pdmareg->control_reg & MASK_PDMA_TRANSFER_ERROR)
    {
        return 1u;
    }
    else
    {
        return 0u;
    }
}

/***************************************************************************//**
 * See pse_pdma.h for description of this function.
 */
uint8_t
MSS_PDMA_clear_transfer_complete_status
(
    mss_pdma_channel_id_t channel_id
)
{
    uint8_t intStatus = 0u;

    if (channel_id > MSS_PDMA_CHANNEL_3)
    {
        return 0u;
    }

    /* Set the register structure pointer for the PDMA channel. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET(channel_id);

    if (pdmareg->control_reg & MASK_PDMA_TRANSFER_DONE)
    {
        intStatus = 1u;
        pdmareg->control_reg &= ~((uint32_t)MASK_PDMA_TRANSFER_DONE);
    }

    return intStatus;
}

/***************************************************************************//**
 * See pse_pdma.h for description of this function.
 */
uint8_t
MSS_PDMA_clear_transfer_error_status
(
    mss_pdma_channel_id_t channel_id
)
{
    uint8_t intStatus = 0u;

    if (channel_id > MSS_PDMA_CHANNEL_3)
    {
        return 0u;
    }

    /* Set the register structure pointer for the PDMA channel. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET(channel_id);

    if (pdmareg->control_reg & MASK_PDMA_TRANSFER_ERROR)
    {
        intStatus = 1u;
        pdmareg->control_reg &= ~((uint32_t)MASK_PDMA_TRANSFER_ERROR);
    }

    return intStatus;
}

/***************************************************************************//**
 * Each DMA channel has two interrupts, one for transfer complete
 * and the other for the transfer error.
 */
uint8_t
dma_ch0_DONE_IRQHandler
(
    void
)
{
    /* Clear the interrupt enable bit. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET
                                                (MSS_PDMA_CHANNEL_0);
    pdmareg->control_reg &= ~((uint32_t)MASK_PDMA_ENABLE_DONE_INT);

    return 0u;
}

uint8_t
dma_ch0_ERR_IRQHandler
(
    void
)
{
    /* Clear the interrupt enable bit. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET
                                                (MSS_PDMA_CHANNEL_0);
    pdmareg->control_reg &= ~((uint32_t)MASK_PDMA_ENABLE_ERR_INT);

    return 0u;
}

uint8_t
dma_ch1_DONE_IRQHandler
(
    void
)
{
    /* Clear the interrupt enable bit. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET
                                                (MSS_PDMA_CHANNEL_1);
    pdmareg->control_reg &= ~((uint32_t)MASK_PDMA_ENABLE_DONE_INT);

    return 0u;
}

uint8_t
dma_ch1_ERR_IRQHandler
(
    void
)
{
    /* Clear the interrupt enable bit. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET
                                                (MSS_PDMA_CHANNEL_1);
    pdmareg->control_reg &= ~((uint32_t)MASK_PDMA_ENABLE_ERR_INT);

    return 0u;
}


uint8_t
dma_ch2_DONE_IRQHandler
(
    void
)
{
    /* Clear the interrupt enable bit. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET
                                                (MSS_PDMA_CHANNEL_2);
    pdmareg->control_reg &= ~((uint32_t)MASK_PDMA_ENABLE_DONE_INT);

    return 0u;
}

uint8_t
dma_ch2_ERR_IRQHandler
(
    void
)
{
    /* Clear the interrupt enable bit. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET
                                                (MSS_PDMA_CHANNEL_2);
    pdmareg->control_reg &= ~((uint32_t)MASK_PDMA_ENABLE_ERR_INT);

    return 0u;
}

uint8_t
dma_ch3_DONE_IRQHandler
(
    void
)
{
    /* Clear the interrupt enable bit. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET
                                                (MSS_PDMA_CHANNEL_3);
    pdmareg->control_reg &= ~((uint32_t)MASK_PDMA_ENABLE_DONE_INT);

    return 0u;
}

uint8_t
dma_ch3_ERR_IRQHandler
(
    void
)
{
    /* Clear the interrupt enable bit. */
    volatile mss_pdma_t *pdmareg = (mss_pdma_t *)MSS_PDMA_REG_OFFSET
                                                (MSS_PDMA_CHANNEL_3);
    pdmareg->control_reg &= ~((uint32_t)MASK_PDMA_ENABLE_ERR_INT);

    return 0u;
}

#ifdef __cplusplus
}
#endif

