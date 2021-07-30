/***************************************************************************//**
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC(PSE) microcontroller subsystem SPI bare metal software driver
 * implementation.
 *
 * SVN $Revision$
 * SVN $Date$
 */

#include "mss_spi.h"
#include "mss_assert.h"
#include "mss_sysreg.h"
#include "mss_ints.h"
#include "mss_plic.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Mask of transfer protocol and SPO, SPH bits within control register.
 */
#define PROTOCOL_MODE_MASK  ((uint32_t) 0x0300000Cu)

/***************************************************************************//**
 * Mask of the frame count bits within the SPI control register.
 */
#define TXRXDFCOUNT_MASK    ((uint32_t) 0x00FFFF00u)
#define TXRXDFCOUNT_SHIFT   ((uint32_t) 8u)
#define BYTESUPPER_MASK     ((uint32_t) 0xFFFF0000u)
/***************************************************************************//**
 * SPI hardware FIFO depth.
 */
#define RX_FIFO_SIZE    4u
#define BIG_FIFO_SIZE   32u

/***************************************************************************//**
 * CONTROL register bit masks
 */
#define CTRL_ENABLE_MASK    0x00000001u
#define CTRL_MASTER_MASK    0x00000002u

/***************************************************************************//**
  Registers bit masks
 */
/* CONTROL register. */
#define MASTER_MODE_MASK        0x00000002u
#define CTRL_RX_IRQ_EN_MASK     0x00000010u
#define CTRL_TX_IRQ_EN_MASK     0x00000020u
#define CTRL_OVFLOW_IRQ_EN_MASK 0x00000040u
#define CTRL_URUN_IRQ_EN_MASK   0x00000080u
#define CTRL_REG_RESET_MASK     0x80000000u
#define BIGFIFO_MASK            0x20000000u
#define CTRL_CLKMODE_MASK       0x10000000u
#define SPS_MASK                0x04000000u

/* CONTROL2 register */
#define C2_ENABLE_CMD_IRQ_MASK     0x00000010u
#define C2_ENABLE_SSEND_IRQ_MASK   0x00000020u

/* STATUS register */
#define TX_DONE_MASK            0x00000001u
#define RX_DATA_READY_MASK      0x00000002u
#define RX_OVERFLOW_MASK        0x00000004u
#define RX_FIFO_EMPTY_MASK      0x00000040u
#define TX_FIFO_FULL_MASK       0x00000100u
#define TX_FIFO_EMPTY_MASK      0x00000400u

/* MIS register. */
#define TXDONE_IRQ_MASK         0x00000001u
#define RXDONE_IRQ_MASK         0x00000002u
#define RXOVFLOW_IRQ_MASK       0x00000004u
#define TXURUN_IRQ_MASK         0x00000008u
#define CMD_IRQ_MASK            0x00000010u
#define SSEND_IRQ_MASK          0x00000020u

/* COMMAND register */
#define AUTOFILL_MASK           0x00000001u
#define TX_FIFO_RESET_MASK      0x00000008u
#define RX_FIFO_RESET_MASK      0x00000004u

/***************************************************************************//**
 *
 */
#define RX_IRQ_THRESHOLD    (BIG_FIFO_SIZE / 2u)

/***************************************************************************//**
  Marker used to detect that the configuration has not been selected for a
  specific slave when operating as a master.
 */
#define NOT_CONFIGURED  0xFFFFFFFFu

/***************************************************************************//**
  Maximum frame length
 */
#define MAX_FRAME_LENGTH        32u

/***************************************************************************//**
 * SPI instance data structures for SPI0 and SPI1. A pointer to these data
 * structures must be used as first parameter to any of the SPI driver functions
 * to identify the SPI hardware block that will perform the requested operation.
 */
mss_spi_instance_t g_mss_spi0_lo;
mss_spi_instance_t g_mss_spi1_lo;

mss_spi_instance_t g_mss_spi0_hi;
mss_spi_instance_t g_mss_spi1_hi;

/***************************************************************************//*
 *  This peripheral tracks if the SPI peripheral is located on S5 or S6 on AXI
 *  switch. This will be used to determine which SPI instance is to be passed
 *  to SPI interrupt handler. Value 0 = S5(SPI_LO)  value 1 = S6(SPI_HI).
 *  Bit positions
 *  0 -> SPI0
 *  1 -> SPI1
 */
static uint8_t g_spi_axi_pos = 0u;

/***************************************************************************//**
  local functions
 */
static void recover_from_rx_overflow(mss_spi_instance_t * this_spi);
static void fill_slave_tx_fifo(mss_spi_instance_t * this_spi);
static void read_slave_rx_fifo(mss_spi_instance_t * this_spi);
static void mss_spi_isr(mss_spi_instance_t * this_spi);

/***************************************************************************//**
 * MSS_SPI_init()
 * See "mss_spi.h" for details of how to use this function.
 */
void MSS_SPI_init
(
    mss_spi_instance_t * this_spi
)
{
    uint16_t slave;

    ASSERT((this_spi == &g_mss_spi0_lo) || (this_spi == &g_mss_spi1_lo));

    /*
     * Initialize SPI driver instance data. Relies on the majority
     * of data requiring 0 for initial state so we just need to fill
     * with 0s and finish off with a small number of non zero values.
     */
    /* Shut down interrupts from the MSS SPI while we do this */
    PLIC_DisableIRQ( this_spi->irqn );

    memset(this_spi, 0, sizeof(mss_spi_instance_t));

    this_spi->cmd_done = 1u;

    for (slave = 0u; slave < (uint16_t)MSS_SPI_MAX_NB_OF_SLAVES; slave++)
    {
        this_spi->slaves_cfg[slave].ctrl_reg = NOT_CONFIGURED;
    }

    if (&g_mss_spi0_lo == this_spi)
    {
        this_spi->hw_reg = MSS_SPI0_LO_BASE;
        this_spi->irqn = SPI0_PLIC;
        g_spi_axi_pos &= ~0x01u;
    }
    else if (&g_mss_spi1_lo == this_spi)
    {
        this_spi->hw_reg = MSS_SPI1_LO_BASE;
        this_spi->irqn = SPI1_PLIC;
        g_spi_axi_pos &= ~0x02u;
    }
    else if (&g_mss_spi0_hi == this_spi)
    {
        this_spi->hw_reg = MSS_SPI0_HI_BASE;
        this_spi->irqn = SPI0_PLIC;
        g_spi_axi_pos |= 0x01u;
    }
    else if (&g_mss_spi1_hi == this_spi)
    {
        this_spi->hw_reg = MSS_SPI1_HI_BASE;
        this_spi->irqn = SPI1_PLIC;
        g_spi_axi_pos |= 0x02u;
    }
    else
    {
        ASSERT(0);
    }

    /* De-assert reset bit. */
    this_spi->hw_reg->CONTROL &= ~CTRL_REG_RESET_MASK;
}

/***************************************************************************//**
 * MSS_SPI_configure_slave_mode()
 * See "mss_spi.h" for details of how to use this function.
 */
void MSS_SPI_configure_slave_mode
(
    mss_spi_instance_t * this_spi,
    mss_spi_protocol_mode_t protocol_mode,
    uint8_t frame_bit_length,
    mss_spi_oveflow_handler_t recieve_buffer_overflow_handler
)
{
    ASSERT((this_spi == &g_mss_spi0) || (this_spi == &g_mss_spi1));
    ASSERT(frame_bit_length <= MAX_FRAME_LENGTH);

    /* Shut down interrupts from the MSS SPI while we do this */
    PLIC_DisableIRQ(this_spi->irqn);

    /*
     * Registering MSS SPI overflow handler to the SPI instance
     * */
    this_spi->buffer_overflow_handler = recieve_buffer_overflow_handler;

    /* Don't yet know what slave transfer mode will be used */
    this_spi->slave_xfer_mode = MSS_SPI_SLAVE_XFER_NONE;

    /* Set the mode. */
    this_spi->hw_reg->CONTROL &= ~(uint32_t)CTRL_MASTER_MASK;

    /* Set the protocol mode */
    this_spi->hw_reg->CONTROL &= ~(uint32_t)CTRL_ENABLE_MASK;
    this_spi->hw_reg->CONTROL = (this_spi->hw_reg->CONTROL & ~PROTOCOL_MODE_MASK)
                                | (uint32_t)protocol_mode | BIGFIFO_MASK;

    /* Set number of data frames to 1 by default */
    this_spi->hw_reg->FRAMESUP = 0u;
    this_spi->hw_reg->CONTROL = (this_spi->hw_reg->CONTROL & ~TXRXDFCOUNT_MASK)
                                | ((uint32_t)1 << TXRXDFCOUNT_SHIFT);

    this_spi->hw_reg->FRAMESIZE = frame_bit_length;
    this_spi->hw_reg->CONTROL |= CTRL_ENABLE_MASK;

    /* Re-enable interrupts */
    PLIC_EnableIRQ(this_spi->irqn);
}

/***************************************************************************//**
 * MSS_SPI_configure_master_mode()
 * See "mss_spi.h" for details of how to use this function.
 */
void MSS_SPI_configure_master_mode
(
    mss_spi_instance_t *    this_spi,
    mss_spi_slave_t         slave,
    mss_spi_protocol_mode_t protocol_mode,
    uint32_t                clk_div,
    uint8_t                 frame_bit_length,
    mss_spi_oveflow_handler_t recieve_buffer_overflow_handler
)
{
    ASSERT((this_spi == &g_mss_spi0) || (this_spi == &g_mss_spi1));
    ASSERT(slave < MSS_SPI_MAX_NB_OF_SLAVES);
    ASSERT(frame_bit_length <= MAX_FRAME_LENGTH);

    /* Check that the requested clock divider is within range and even number */
    ASSERT(clk_div >= 2u);
    ASSERT(clk_div <= 512u);
    ASSERT(0u == (clk_div & 0x00000001U));

    /* Shut down interrupts from the MSS SPI while we do this */
    PLIC_DisableIRQ( this_spi->irqn );

    /*
     * Registering MSS SPI overflow handler to the SPI instance
     * */
    this_spi->buffer_overflow_handler = recieve_buffer_overflow_handler;

    /* Reset slave transfer mode to unknown to wipe slate clean */
    this_spi->slave_xfer_mode = MSS_SPI_SLAVE_XFER_NONE;

    /* Set the mode. */
    this_spi->hw_reg->CONTROL &= ~(uint32_t)CTRL_ENABLE_MASK;
    this_spi->hw_reg->CONTROL |= CTRL_MASTER_MASK;
    this_spi->hw_reg->CONTROL |= CTRL_ENABLE_MASK;

    /*
     * Keep track of the required register configuration for this slave. These
     * values will be used by the MSS_SPI_set_slave_select() function to
     * configure the master to match the slave being selected.
     */
    if (slave < MSS_SPI_MAX_NB_OF_SLAVES)
    {
        uint32_t clk_gen;
        /*
        * Setting the SPS bit ensures the slave select remains asserted even
        * if we don't keep the TX FIFO filled in block mode. We only do it for
        * Motorola modes and if you need the slave selected deselected between
        * frames in modes 0 or 2 then remove SPS_MASK from below.
        */
        if ((MSS_SPI_MODE0 == protocol_mode) ||
           (MSS_SPI_MODE1 == protocol_mode) ||
           (MSS_SPI_MODE2 == protocol_mode) ||
           (MSS_SPI_MODE3 == protocol_mode))
        {
            this_spi->slaves_cfg[slave].ctrl_reg = MASTER_MODE_MASK | SPS_MASK |
                                                   BIGFIFO_MASK | CTRL_CLKMODE_MASK |
                                                   (uint32_t)protocol_mode |
                                                   ((uint32_t)1 << TXRXDFCOUNT_SHIFT);
        }
        else
        {
            this_spi->slaves_cfg[slave].ctrl_reg = MASTER_MODE_MASK |
                                                   BIGFIFO_MASK | CTRL_CLKMODE_MASK |
                                                   (uint32_t)protocol_mode |
                                                   ((uint32_t)1 << TXRXDFCOUNT_SHIFT);
        }
        this_spi->slaves_cfg[slave].txrxdf_size_reg = frame_bit_length;

        clk_gen = (clk_div / 2u) - (uint32_t)1u;
        this_spi->slaves_cfg[slave].clk_gen = (uint8_t)clk_gen;
    }

    /* Re enable interrupts */
    PLIC_EnableIRQ( this_spi->irqn );
}

/***************************************************************************//**
 * MSS_SPI_set_slave_select()
 * See "mss_spi.h" for details of how to use this function.
 */
void MSS_SPI_set_slave_select
(
    mss_spi_instance_t * this_spi,
    mss_spi_slave_t slave
)
{
    uint32_t rx_overflow;
    ASSERT((this_spi == &g_mss_spi0) || (this_spi == &g_mss_spi1));

    /* This function is only intended to be used with an SPI master. */
    ASSERT((this_spi->hw_reg->CONTROL & CTRL_MASTER_MASK)
               == CTRL_MASTER_MASK);

    ASSERT(this_spi->slaves_cfg[slave].ctrl_reg != NOT_CONFIGURED);

    /* Shut down interrupts from the MSS SPI while we do this */
    PLIC_DisableIRQ( this_spi->irqn );

    /* Recover from receive overflow. */
    rx_overflow = this_spi->hw_reg->STATUS & RX_OVERFLOW_MASK;
    if (rx_overflow > 0U)
    {
         recover_from_rx_overflow(this_spi);
    }

    /* Set the clock rate. */
    this_spi->hw_reg->CONTROL &= ~(uint32_t)CTRL_ENABLE_MASK;
    this_spi->hw_reg->CONTROL = this_spi->slaves_cfg[slave].ctrl_reg;
    this_spi->hw_reg->CLK_GEN = (uint32_t)(this_spi->slaves_cfg[slave].clk_gen);
    this_spi->hw_reg->FRAMESIZE =
            (uint32_t)(this_spi->slaves_cfg[slave].txrxdf_size_reg);

    this_spi->hw_reg->CONTROL |= CTRL_ENABLE_MASK;

    /* Set slave select */
    this_spi->hw_reg->SLAVE_SELECT |= ((uint32_t)1 << (uint32_t)slave);

    /* Re enable interrupts */
    PLIC_EnableIRQ( this_spi->irqn);
}

/***************************************************************************//**
 * MSS_SPI_clear_slave_select()
 * See "mss_spi.h" for details of how to use this function.
 */
void MSS_SPI_clear_slave_select
(
    mss_spi_instance_t * this_spi,
    mss_spi_slave_t slave
)
{
    uint32_t rx_overflow;
    ASSERT((this_spi == &g_mss_spi0) || (this_spi == &g_mss_spi1));

    /* This function is only intended to be used with an SPI master. */
    ASSERT((this_spi->hw_reg->CONTROL & CTRL_MASTER_MASK)
               == CTRL_MASTER_MASK);

    /* Shut down interrupts from the MSS SPI while we do this */
    PLIC_DisableIRQ( this_spi->irqn );

    /* Recover from receive overflow. */
    rx_overflow = this_spi->hw_reg->STATUS & RX_OVERFLOW_MASK;
    if (rx_overflow > 0U)
    {
         recover_from_rx_overflow(this_spi);
    }
    this_spi->hw_reg->SLAVE_SELECT &= ~((uint32_t)1 << (uint32_t)slave);

    /* Re enable interrupts */
    PLIC_EnableIRQ( this_spi->irqn );
}

/***************************************************************************//**
 * MSS_SPI_transfer_frame()
 * See "mss_spi.h" for details of how to use this function.
 */
uint32_t MSS_SPI_transfer_frame
(
    mss_spi_instance_t * this_spi,
    uint32_t tx_bits
)
{
    uint32_t rx_ready;
    uint32_t tx_done;
    ASSERT((this_spi == &g_mss_spi0) || (this_spi == &g_mss_spi1));

    /* This function is only intended to be used with an SPI master. */
    ASSERT((this_spi->hw_reg->CONTROL & CTRL_MASTER_MASK)
              == CTRL_MASTER_MASK);

    /* Ensure single frame transfer selected so interrupts work correctly */
    this_spi->hw_reg->FRAMESUP = 0u;
    this_spi->hw_reg->CONTROL = (this_spi->hw_reg->CONTROL & ~TXRXDFCOUNT_MASK)
                                | ((uint32_t)1u << TXRXDFCOUNT_SHIFT);

    /* Flush the Tx and Rx FIFOs. */
    this_spi->hw_reg->COMMAND |= ((uint32_t)TX_FIFO_RESET_MASK |
                                  (uint32_t)RX_FIFO_RESET_MASK);

    /* Send frame. */
    this_spi->hw_reg->TX_DATA = tx_bits;

    /* Wait for frame Tx to complete. */
    tx_done = this_spi->hw_reg->STATUS & TX_DONE_MASK;
    while (0u == tx_done)
    {
        tx_done = this_spi->hw_reg->STATUS & TX_DONE_MASK;
    }

    /* Read received frame. */
    /* Wait for Rx complete. */
    rx_ready = this_spi->hw_reg->STATUS & RX_DATA_READY_MASK;
    while (0u == rx_ready)
    {
        rx_ready = this_spi->hw_reg->STATUS & RX_DATA_READY_MASK;
    }

    /* Return Rx data. */
    return( this_spi->hw_reg->RX_DATA );
}

/***************************************************************************//**
 * MSS_SPI_transfer_block()
 * See "mss_spi.h" for details of how to use this function.
 */
void MSS_SPI_transfer_block
(
    mss_spi_instance_t * this_spi,
    const uint8_t cmd_buffer[],
    uint32_t cmd_byte_size,
    uint8_t rd_buffer[],
    uint32_t rd_byte_size
)
{
    uint32_t transfer_idx = 0u;
    uint32_t tx_idx;
    uint32_t rx_idx;
    uint32_t frame_count;
    volatile uint32_t rx_raw;
    uint32_t transit = 0u;
    uint32_t tx_fifo_full;
    uint32_t rx_overflow;
    uint32_t rx_fifo_empty;

    uint32_t transfer_size;     /* Total number of bytes transfered. */

    ASSERT((this_spi == &g_mss_spi0) || (this_spi == &g_mss_spi1));

    /* This function is only intended to be used with an SPI master. */
    ASSERT((this_spi->hw_reg->CONTROL & CTRL_MASTER_MASK)
                == CTRL_MASTER_MASK);

    /* Compute number of bytes to transfer. */
    transfer_size = cmd_byte_size + rd_byte_size;

    /* Adjust to 1 byte transfer to cater for DMA transfers. */
    if (0u == transfer_size)
    {
        frame_count = 1u;
    }
    else
    {
        frame_count = transfer_size;
    }

    /* Flush the Tx and Rx FIFOs. */
    this_spi->hw_reg->COMMAND |= ((uint32_t)TX_FIFO_RESET_MASK |
                                  (uint32_t)RX_FIFO_RESET_MASK);

    /* Recover from receive overflow. */
    rx_overflow = this_spi->hw_reg->STATUS & RX_OVERFLOW_MASK;
    if (rx_overflow > 0U)
    {
         recover_from_rx_overflow(this_spi);
    }

    /* Set frame size to 8 bits and the frame count to the transfer size. */
    this_spi->hw_reg->CONTROL &= ~(uint32_t)CTRL_ENABLE_MASK;
    this_spi->hw_reg->FRAMESUP = frame_count & BYTESUPPER_MASK;
    this_spi->hw_reg->CONTROL = (this_spi->hw_reg->CONTROL & ~TXRXDFCOUNT_MASK) |
                                ((frame_count << TXRXDFCOUNT_SHIFT)
                                 & TXRXDFCOUNT_MASK);

    this_spi->hw_reg->FRAMESIZE = MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE;
    this_spi->hw_reg->CONTROL |= CTRL_ENABLE_MASK;

    /* Flush the receive FIFO. */
    rx_fifo_empty = this_spi->hw_reg->STATUS & RX_FIFO_EMPTY_MASK;
    while (0u == rx_fifo_empty)
    {
        rx_raw = this_spi->hw_reg->RX_DATA;
        rx_fifo_empty = this_spi->hw_reg->STATUS & RX_FIFO_EMPTY_MASK;
    }
    tx_idx = 0u;
    rx_idx = 0u;
    if (tx_idx < cmd_byte_size)
    {
        this_spi->hw_reg->TX_DATA = cmd_buffer[tx_idx];
        ++tx_idx;
        ++transit;
    }
    else
    {
        if (tx_idx < transfer_size)
        {
            this_spi->hw_reg->TX_DATA = 0x00u;
            ++tx_idx;
            ++transit;
        }
    }

    /* Perform the remainder of the transfer by sending a byte every time a byte
     * has been received. This should ensure that no Rx overflow can happen in
     * case of an interrupt occurs during this function. */
    while (transfer_idx < transfer_size)
    {
        rx_fifo_empty = this_spi->hw_reg->STATUS & RX_FIFO_EMPTY_MASK;
        if (0u == rx_fifo_empty)
        {
            /* Process received byte. */
            rx_raw = this_spi->hw_reg->RX_DATA;
            if (transfer_idx >= cmd_byte_size)
            {
                if (rx_idx < rd_byte_size)
                {
                    rd_buffer[rx_idx] = (uint8_t)rx_raw;
                }
                ++rx_idx;
            }
            ++transfer_idx;
            --transit;
        }
        tx_fifo_full = this_spi->hw_reg->STATUS & TX_FIFO_FULL_MASK;
        if (0u == tx_fifo_full)
        {
            if (transit < RX_FIFO_SIZE)
            {
                /* Send another byte. */
                if (tx_idx < cmd_byte_size)
                {
                    this_spi->hw_reg->TX_DATA = cmd_buffer[tx_idx];
                    ++tx_idx;
                    ++transit;
                }
                else
                {
                    if (tx_idx < transfer_size)
                    {
                        this_spi->hw_reg->TX_DATA = 0x00u;
                        ++tx_idx;
                        ++transit;
                    }
                }
            }
        }
    }
}

/***************************************************************************//**
 * MSS_SPI_set_frame_rx_handler()
 * See "mss_spi.h" for details of how to use this function.
 */
void MSS_SPI_set_frame_rx_handler
(
    mss_spi_instance_t * this_spi,
    mss_spi_frame_rx_handler_t rx_handler
)
{
    uint32_t tx_fifo_empty;
    ASSERT((this_spi == &g_mss_spi0) || (this_spi == &g_mss_spi1));

    /* This function is only intended to be used with an SPI slave. */
    ASSERT((this_spi->hw_reg->CONTROL & CTRL_MASTER_MASK)
                != CTRL_MASTER_MASK);

    /* Shut down interrupts from the MSS SPI while we do this */
    PLIC_DisableIRQ( this_spi->irqn );

    /* Disable block Rx handlers as they are mutually exclusive. */
    this_spi->block_rx_handler = 0u;
    this_spi->cmd_handler = 0u;

    /* Keep a copy of the pointer to the rx handler function. */
    this_spi->frame_rx_handler = rx_handler;

    /* Make sure correct mode is selected */
    this_spi->slave_xfer_mode = MSS_SPI_SLAVE_XFER_FRAME;

    /* Automatically fill the TX FIFO with zeroes if no slave tx frame set.*/
    tx_fifo_empty = this_spi->hw_reg->STATUS & TX_FIFO_EMPTY_MASK;
    if (tx_fifo_empty > 0U)
    {
        this_spi->hw_reg->COMMAND |= AUTOFILL_MASK;
    }

    /* Ensure single frame transfer selected so interrupts work correctly */
    this_spi->hw_reg->FRAMESUP = 0u;
    this_spi->hw_reg->CONTROL = (this_spi->hw_reg->CONTROL & ~TXRXDFCOUNT_MASK)
                                | ((uint32_t)1u << TXRXDFCOUNT_SHIFT);

    /* Disable block specific interrupts */
    this_spi->hw_reg->CONTROL2 &= ~(uint32_t)C2_ENABLE_CMD_IRQ_MASK;
    this_spi->hw_reg->CONTROL2 &= ~(uint32_t)C2_ENABLE_SSEND_IRQ_MASK;

    /* Clear down interrupts to avoid stale interrupts triggering when we
     * enable them below */
    this_spi->hw_reg->INT_CLEAR = ((uint32_t)TXURUN_IRQ_MASK |
                                   (uint32_t)RXOVFLOW_IRQ_MASK |
                                   (uint32_t)RXDONE_IRQ_MASK);

    /*
     * Enable TX underrun and RX overflow interrupts to improve error
     * recovery and the Rx interrupt.
     */
    this_spi->hw_reg->CONTROL |= ((uint32_t)CTRL_URUN_IRQ_EN_MASK |
                                  (uint32_t)CTRL_OVFLOW_IRQ_EN_MASK |
                                  (uint32_t)CTRL_RX_IRQ_EN_MASK);

    PLIC_EnableIRQ(this_spi->irqn);
}

/***************************************************************************//**
 * MSS_SPI_set_slave_tx_frame()
 * See "mss_spi.h" for details of how to use this function.
 */
void MSS_SPI_set_slave_tx_frame
(
    mss_spi_instance_t * this_spi,
    uint32_t frame_value
)
{
    ASSERT((this_spi == &g_mss_spi0) || (this_spi == &g_mss_spi1));
    /* This function is only intended to be used with an SPI slave. */
    ASSERT((this_spi->hw_reg->CONTROL & CTRL_MASTER_MASK)
                != CTRL_MASTER_MASK);

    /* Shut down interrupts from the MSS SPI while we do this */
    PLIC_DisableIRQ(this_spi->irqn);

    /* Make sure correct mode is selected */
    this_spi->slave_xfer_mode = MSS_SPI_SLAVE_XFER_FRAME;

    /* Disable block Rx handlers as they are mutually exclusive. */
    this_spi->block_rx_handler = 0u;
    this_spi->cmd_handler = 0u;

    /* Disable slave block tx buffer as it is mutually exclusive with frame
     * level handling. */
    this_spi->slave_tx_buffer = 0u;
    this_spi->slave_tx_size = 0u;
    this_spi->slave_tx_idx = 0u;

    /* Keep a copy of the slave tx frame value. */
    this_spi->slave_tx_frame = frame_value;

    /* Disable automatic fill of the TX FIFO with zeroes.*/
    this_spi->hw_reg->COMMAND &= ~(uint32_t)AUTOFILL_MASK;
    this_spi->hw_reg->COMMAND |= TX_FIFO_RESET_MASK;
    /*
     * Ensure single frame transfer selected so interrupts work correctly
     *
     * IMPORTANT: Note this must be done before writing to the TX_DATA register
     * as it seems that doing these in the opposite order causes the receive
     * and transmit interrupts to be disabled.
     */
    this_spi->hw_reg->FRAMESUP = 0u;
    this_spi->hw_reg->CONTROL = (this_spi->hw_reg->CONTROL & ~TXRXDFCOUNT_MASK)
                                | ((uint32_t)1u << TXRXDFCOUNT_SHIFT);

    /* Load frame into Tx data register. */
    this_spi->hw_reg->TX_DATA = this_spi->slave_tx_frame;

   /* Disable block specific interrupts */
    this_spi->hw_reg->CONTROL2 &= ~(uint32_t)C2_ENABLE_CMD_IRQ_MASK;
    this_spi->hw_reg->CONTROL2 &= ~(uint32_t)C2_ENABLE_SSEND_IRQ_MASK;

    /* Clear down interrupts to avoid stale interrupts triggering when we
     * enable them below
     */
    this_spi->hw_reg->INT_CLEAR = ((uint32_t)TXURUN_IRQ_MASK |
                                   (uint32_t)RXOVFLOW_IRQ_MASK |
                                   (uint32_t)RXDONE_IRQ_MASK);
    /*
     * Enable Tx Done interrupt in order to reload the slave Tx frame after each
     * time it has been sent.
     *
     * Enable TX underrun and RX overflow interrupts to improve error
     * recovery.
     */
    this_spi->hw_reg->CONTROL |= ((uint32_t)CTRL_URUN_IRQ_EN_MASK |
                                  (uint32_t)CTRL_OVFLOW_IRQ_EN_MASK |
                                  (uint32_t)CTRL_RX_IRQ_EN_MASK);

    PLIC_EnableIRQ(this_spi->irqn);
}

/***************************************************************************//**
 * MSS_SPI_set_slave_block_buffers()
 * See "mss_spi.h" for details of how to use this function.
 */
void MSS_SPI_set_slave_block_buffers
(
    mss_spi_instance_t * this_spi,
    const uint8_t * tx_buffer,
    uint32_t tx_buff_size,
    uint8_t * rx_buffer,
    uint32_t rx_buff_size,
    mss_spi_block_rx_handler_t block_rx_handler
)
{
    uint32_t frame_count;
    uint32_t done = 0u;
    ASSERT((this_spi == &g_mss_spi0) || (this_spi == &g_mss_spi1));

    /* This function is only intended to be used with an SPI slave. */
    ASSERT((this_spi->hw_reg->CONTROL & CTRL_MASTER_MASK) != CTRL_MASTER_MASK);

    /* Shut down interrupts from the MSS SPI while we do this */
    PLIC_DisableIRQ(this_spi->irqn);

    /* Make sure correct mode is selected */
    this_spi->slave_xfer_mode = MSS_SPI_SLAVE_XFER_BLOCK;

    /* Set cmd_done correctly to ensure 0 padding works. */
    if ((mss_spi_block_rx_handler_t)0 == this_spi->cmd_handler)
    {
        this_spi->cmd_done = 1u;
    }
    else
    {
        this_spi->cmd_done = 0u;
    }

    /* Disable Rx frame handler as it is mutually exclusive with block rx handler. */
    this_spi->frame_rx_handler = 0u;

    /* Keep a copy of the pointer to the block rx handler function. */
    this_spi->block_rx_handler = block_rx_handler;

    this_spi->slave_rx_buffer = rx_buffer;
    this_spi->slave_rx_size = rx_buff_size;
    this_spi->slave_rx_idx = 0u;

    /* Initialise the transmit state data. */
    this_spi->slave_tx_buffer = tx_buffer;
    this_spi->slave_tx_size = tx_buff_size;
    this_spi->slave_tx_idx = 0u;

    /* Flush the Tx and Rx FIFOs.
     * Please note this does not have any effect on A2F200.
     */
    this_spi->hw_reg->COMMAND |= ((uint32_t)TX_FIFO_RESET_MASK
                                | (uint32_t)RX_FIFO_RESET_MASK);

    /* Recover from receive overflow if needs be */
    if (0u != (this_spi->hw_reg->STATUS & RX_OVERFLOW_MASK))
    {
         recover_from_rx_overflow(this_spi);
    }

    /* Flush Rx FIFO in case we are executing on A2F200. */
    while (0u == (this_spi->hw_reg->STATUS & RX_FIFO_EMPTY_MASK))
    {
        volatile uint32_t dummy;
        dummy = this_spi->hw_reg->RX_DATA;
    }

    /* Use the frame counter to control how often receive interrupts are
     * generated */
    frame_count = RX_IRQ_THRESHOLD;

    this_spi->hw_reg->CONTROL &= ~(uint32_t)CTRL_ENABLE_MASK;
    this_spi->hw_reg->FRAMESUP = frame_count & BYTESUPPER_MASK;
    this_spi->hw_reg->CONTROL = (this_spi->hw_reg->CONTROL & ~TXRXDFCOUNT_MASK)
                                | (frame_count << TXRXDFCOUNT_SHIFT);
    this_spi->hw_reg->FRAMESIZE = MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE;
    this_spi->hw_reg->CONTROL |= CTRL_ENABLE_MASK;

    /* Load the transmit FIFO. */
    while ((0u == (this_spi->hw_reg->STATUS & TX_FIFO_FULL_MASK))
            && (0u == done))
    {
        if (this_spi->slave_tx_idx < this_spi->slave_tx_size)
        {
            this_spi->hw_reg->TX_DATA =
                (uint32_t)(this_spi->slave_tx_buffer[this_spi->slave_tx_idx]);
        }
        else if (0u != this_spi->cmd_done)
        {
            /* Fill with 0s if no need to insert command response */
            this_spi->hw_reg->TX_DATA = 0x00u;
        }
        else
        {
            /* Exit loop early as command response needs to be inserted next */
            done = 1u;
        }
        ++this_spi->slave_tx_idx;
    }

    if (tx_buff_size > 0u)
    {
        /* Clear and enable TX interrupt. Also disable autofill */
        this_spi->hw_reg->COMMAND &= ~(uint32_t)AUTOFILL_MASK;
        this_spi->hw_reg->INT_CLEAR = TXDONE_IRQ_MASK;
        this_spi->hw_reg->CONTROL |= CTRL_TX_IRQ_EN_MASK;
    }
    else
    {
        this_spi->hw_reg->COMMAND |= AUTOFILL_MASK;
    }
    /* Ensure command interrupt disabled if no handler */
    if ((mss_spi_block_rx_handler_t)0 == this_spi->cmd_handler)
    {
        this_spi->hw_reg->CONTROL2 &= ~(uint32_t)C2_ENABLE_CMD_IRQ_MASK;
    }
    /*
     * Enable slave select release interrupt. The SSEND interrupt is used to
     * complete reading of the receive FIFO and prepare the transmit FIFO for
     * the next transaction.
     * Make sure to clear any pending send ints otherwise we will trigger
     * an immediate interrupt.
     */
    this_spi->hw_reg->INT_CLEAR = SSEND_IRQ_MASK;
    this_spi->hw_reg->CONTROL2 |= C2_ENABLE_SSEND_IRQ_MASK;

    /* Clear down interrupts to avoid stale interrupts triggering when we
     * enable them below */
    this_spi->hw_reg->INT_CLEAR = ((uint32_t)TXURUN_IRQ_MASK |
                                   (uint32_t)RXOVFLOW_IRQ_MASK |
                                   (uint32_t)RXDONE_IRQ_MASK);
    /*
     * Enable TX underrun and RX overflow interrupts to improve error
     * recovery.
     */
    this_spi->hw_reg->CONTROL |= ((uint32_t)CTRL_URUN_IRQ_EN_MASK |
                                  (uint32_t)CTRL_OVFLOW_IRQ_EN_MASK |
                                  (uint32_t)CTRL_RX_IRQ_EN_MASK);

    PLIC_EnableIRQ(this_spi->irqn);
}

/***************************************************************************//**
 * MSS_SPI_set_cmd_handler()
 * See "mss_spi.h" for details of how to use this function.
 */
void MSS_SPI_set_cmd_handler
(
    mss_spi_instance_t * this_spi,
    mss_spi_block_rx_handler_t cmd_handler,
    uint32_t cmd_size
)
{
    /* Shut down interrupts from the MSS SPI while we do this */
    PLIC_DisableIRQ(this_spi->irqn);

    /* Make sure response state is cleared down */
    this_spi->resp_tx_buffer   = 0u;
    this_spi->resp_buff_size   = 0u;
    this_spi->resp_buff_tx_idx = 0u;

    if ((mss_spi_block_rx_handler_t)0 == cmd_handler)
    {
        /*
         * Set this flag so zero padding is enabled
         */
        this_spi->cmd_done = 1u;

        /*
         * Ensure command interrupt disabled if no handler
         * and handler pointer is wiped clean.
         */
        this_spi->cmd_handler = 0u;
        this_spi->hw_reg->CMDSIZE = 0u;
        this_spi->hw_reg->CONTROL2 &= ~(uint32_t)C2_ENABLE_CMD_IRQ_MASK;
    }
    else
    {
        /*
         * Clear this flag so zero padding is disabled until command response
         * has been taken care of.
         */
        this_spi->cmd_done = 0u;
        this_spi->cmd_handler = cmd_handler;
        this_spi->hw_reg->CMDSIZE = cmd_size;

        /* Flush the Tx FIFO. Please note this does not have any effect on
         * A2F200. */
        this_spi->hw_reg->COMMAND |= ((uint32_t)TX_FIFO_RESET_MASK |
                (uint32_t)RX_FIFO_RESET_MASK);

        /*
         * Reload TX FIFO as MSS_SPI_set_slave_block_buffers() may have zero
         * filled the FIFO if command handler was not in place when it was
         * called and so the first frame sent could be wrong.
         */
        this_spi->slave_tx_idx    = 0u;
        fill_slave_tx_fifo(this_spi);

        /*
         * Make sure to clear any pending command ints otherwise we will trigger
         * an immediate interrupt.
         */
        this_spi->hw_reg->INT_CLEAR = CMD_IRQ_MASK;
        this_spi->hw_reg->CONTROL2 |= C2_ENABLE_CMD_IRQ_MASK;
    }
    PLIC_EnableIRQ(this_spi->irqn); /* Safe to allow interrupts again */
}

/***************************************************************************//**
 * MSS_SPI_set_cmd_response()
 * See "mss_spi.h" for details of how to use this function.
 */
void MSS_SPI_set_cmd_response
(
    mss_spi_instance_t * this_spi,
    const uint8_t * resp_tx_buffer,
    uint32_t resp_buff_size
)
{
    this_spi->resp_tx_buffer = resp_tx_buffer;
    this_spi->resp_buff_size = resp_buff_size;
    this_spi->resp_buff_tx_idx = 0u;

    /* Note that we have provided response and start getting it into the FIFO */
    this_spi->cmd_done = 1u;
    fill_slave_tx_fifo(this_spi);
}

/***************************************************************************//**
 * MSS_SPI_enable()
 * See "mss_spi.h" for details of how to use this function.
 */
void MSS_SPI_enable
(
    mss_spi_instance_t * this_spi
)
{
    /* Shut down interrupts from the MSS SPI while we do this */
    PLIC_DisableIRQ(this_spi->irqn);

    this_spi->hw_reg->CONTROL |= CTRL_ENABLE_MASK;

    /* Re enable interrupts */
    PLIC_EnableIRQ(this_spi->irqn);
}

/***************************************************************************//**
 * MSS_SPI_disable()
 * See "mss_spi.h" for details of how to use this function.
 */
void MSS_SPI_disable
(
    mss_spi_instance_t * this_spi
)
{
    /* Shut down interrupts from the MSS SPI while we do this */
    PLIC_DisableIRQ(this_spi->irqn);

    this_spi->hw_reg->CONTROL &= ~(uint32_t)CTRL_ENABLE_MASK;

    /* Reenable interrupts */
    PLIC_EnableIRQ(this_spi->irqn);
}

/***************************************************************************//**
 * MSS_SPI_tx_done()
 * See "mss_spi.h" for details of how to use this function.
 */
uint32_t MSS_SPI_tx_done
(
    mss_spi_instance_t * this_spi
)
{
    uint32_t tx_done;

    tx_done = this_spi->hw_reg->STATUS & TX_DONE_MASK;

    return tx_done;
}

/***************************************************************************//**
 * Fill the transmit FIFO(used for slave block transfers).
 */
static void fill_slave_tx_fifo
(
    mss_spi_instance_t * this_spi
)
{
    uint32_t guard = 0u;

    while ((0u == (this_spi->hw_reg->STATUS & TX_FIFO_FULL_MASK)) &&
          (this_spi->slave_tx_idx < this_spi->slave_tx_size))
    {
        /* Sending from primary slave transmit buffer */
        this_spi->hw_reg->TX_DATA =
                (uint32_t)(this_spi->slave_tx_buffer[this_spi->slave_tx_idx]);
        ++this_spi->slave_tx_idx;
    }

    if (this_spi->slave_tx_idx >= this_spi->slave_tx_size)
    {
        while ((0u == (this_spi->hw_reg->STATUS & TX_FIFO_FULL_MASK)) &&
              (this_spi->resp_buff_tx_idx < this_spi->resp_buff_size))
        {
            /* Sending from command response buffer */
            this_spi->hw_reg->TX_DATA =
            (uint32_t)(this_spi->resp_tx_buffer[this_spi->resp_buff_tx_idx]);
            ++this_spi->resp_buff_tx_idx;
        }
    }

    if ((0u != this_spi->cmd_done) &&
       (this_spi->slave_tx_idx >= this_spi->slave_tx_size) &&
       (this_spi->resp_buff_tx_idx >= this_spi->resp_buff_size))
    {
        while ((0u == (this_spi->hw_reg->STATUS & TX_FIFO_FULL_MASK)) &&
              (guard < BIG_FIFO_SIZE))
        {
            /* Nothing left so pad with 0s for consistency */
            this_spi->hw_reg->TX_DATA = 0x00u;
            /*
             * We use the guard count to cover the unlikely event that we are
             * never seeing the TX FIFO full because the data is being pulled
             * out as fast as we can stuff it in. In this event we never spend
             * more than a full FIFOs worth of time spinning here.
             */
            guard++;
        }
    }
}

/***************************************************************************//**
 *
 */
static void read_slave_rx_fifo
(
    mss_spi_instance_t * this_spi
)
{
    volatile uint32_t rx_frame;

    if (MSS_SPI_SLAVE_XFER_FRAME == this_spi->slave_xfer_mode)
    {
        while (0u == (this_spi->hw_reg->STATUS & RX_FIFO_EMPTY_MASK))
        {
            /* Single frame handling mode. */
            rx_frame = this_spi->hw_reg->RX_DATA;
            if ((mss_spi_frame_rx_handler_t)0 != this_spi->frame_rx_handler)
            {
                this_spi->frame_rx_handler( rx_frame );
            }
        }
    }
    else if (MSS_SPI_SLAVE_XFER_BLOCK == this_spi->slave_xfer_mode)
    {
        /* Block handling mode. */
        /* Something needs to be read from FIFO */
        while (0u == (this_spi->hw_reg->STATUS & RX_FIFO_EMPTY_MASK))
        {
            rx_frame = this_spi->hw_reg->RX_DATA;
            if (this_spi->slave_rx_idx < this_spi->slave_rx_size)
            {
                this_spi->slave_rx_buffer[this_spi->slave_rx_idx] =
                                         (uint8_t)rx_frame;
            }
            ++this_spi->slave_rx_idx;
        }
    }
    else
    {
        /* Should not happen... Just purge FIFO */
        while (0u == (this_spi->hw_reg->STATUS & RX_FIFO_EMPTY_MASK))
        {
            rx_frame = this_spi->hw_reg->RX_DATA;
        }
    }
}

/***************************************************************************//**
 * SPI interrupt service routine.
 */
static void mss_spi_isr
(
    mss_spi_instance_t * this_spi
)
{
    volatile uint32_t rx_frame;
    volatile  uint32_t *this_mis = &this_spi->hw_reg->MIS;

    ASSERT((this_spi == &g_mss_spi0) || (this_spi == &g_mss_spi1));

    if (0u != (*this_mis & RXDONE_IRQ_MASK))
    {
        if (MSS_SPI_SLAVE_XFER_FRAME == this_spi->slave_xfer_mode)
        {
            /* Single frame handling mode. */
            while (0u == (this_spi->hw_reg->STATUS & RX_FIFO_EMPTY_MASK))
            {
                rx_frame = this_spi->hw_reg->RX_DATA;
                if ((mss_spi_frame_rx_handler_t)0 != this_spi->frame_rx_handler)
                {
                    this_spi->frame_rx_handler(rx_frame);
                }
            }
        }
        else if (MSS_SPI_SLAVE_XFER_BLOCK == this_spi->slave_xfer_mode)
        {
            /* Block handling mode.
               Something needs to be read from FIFO
            */
            while (0u == (this_spi->hw_reg->STATUS & RX_FIFO_EMPTY_MASK))
            {
                /* Read from FIFO irrespective */
                rx_frame = this_spi->hw_reg->RX_DATA;
                /* Write to array if required */
                if (this_spi->slave_rx_idx < this_spi->slave_rx_size)
                {
                    this_spi->slave_rx_buffer[this_spi->slave_rx_idx] =
                                             (uint8_t)rx_frame;
                }
                ++this_spi->slave_rx_idx;
            }
        }
        else
        {
            /* No slave handling in place so just purge FIFO */
            while (0u == (this_spi->hw_reg->STATUS & RX_FIFO_EMPTY_MASK))
            {
                rx_frame = this_spi->hw_reg->RX_DATA;
            }
        }
        this_spi->hw_reg->INT_CLEAR = RXDONE_IRQ_MASK;
    }

   /* Handle transmit. */
    if (0u != (*this_mis & TXDONE_IRQ_MASK))
    {
        if (MSS_SPI_SLAVE_XFER_FRAME == this_spi->slave_xfer_mode)
        {
           /* Reload slave tx frame into Tx data register. */
            this_spi->hw_reg->TX_DATA = this_spi->slave_tx_frame;
        }
        else /* Must be block mode so load FIFO to the max */
        {
            fill_slave_tx_fifo (this_spi);
        }

        this_spi->hw_reg->INT_CLEAR = TXDONE_IRQ_MASK;
    }
    /* Handle command interrupt. */
    if (0u != (*this_mis & CMD_IRQ_MASK))
    {
        read_slave_rx_fifo (this_spi);
        /*
         * Call the command handler if one exists.
         */
        if ((mss_spi_block_rx_handler_t)0 != this_spi->cmd_handler)
        {
            (*this_spi->cmd_handler)(this_spi->slave_rx_buffer,
                                     this_spi->slave_rx_idx);

            fill_slave_tx_fifo(this_spi);
        }

        /* Set cmd_done to indicate it is now safe to 0 fill TX FIFO */
        this_spi->cmd_done = 1u;

        /* Disable command interrupt until slave select becomes de-asserted to
         * avoid retriggering. */
        this_spi->hw_reg->CONTROL2 &= ~(uint32_t)C2_ENABLE_CMD_IRQ_MASK;
        this_spi->hw_reg->INT_CLEAR = CMD_IRQ_MASK;
    }
    if (0u != (*this_mis & RXOVFLOW_IRQ_MASK))
    {
        /*
         * Receive overflow, not a lot we can do for this. Reset the receive
         *  FIFO, clear the interrupt and hope it doesn't happen again...
         */
        this_spi->hw_reg->COMMAND |= RX_FIFO_RESET_MASK;
        recover_from_rx_overflow(this_spi);

        this_spi->hw_reg->INT_CLEAR = RXOVFLOW_IRQ_MASK;
    }

    /*
     * Transmit underrun, try and recover by reapplying the current
     * slave TX FIFO data setup (if there is one).
     * In block mode this will probably not be very successful as we will
     * be out of synch with the master but the reset on SSEND will hopefully
     * take care of that for the next transfer.
     */
    if (0u != (*this_mis & TXURUN_IRQ_MASK))
    {
        this_spi->hw_reg->COMMAND |= TX_FIFO_RESET_MASK;
        if ( MSS_SPI_SLAVE_XFER_FRAME == this_spi->slave_xfer_mode )
        {
            this_spi->hw_reg->FRAMESUP = 0u;
            this_spi->hw_reg->CONTROL = (this_spi->hw_reg->CONTROL
                                         & ~TXRXDFCOUNT_MASK)
                                         | ((uint32_t)1u << TXRXDFCOUNT_SHIFT);

            /* Reload slave tx frame into Tx data register. */
            this_spi->hw_reg->TX_DATA = this_spi->slave_tx_frame;
        }
        else if (MSS_SPI_SLAVE_XFER_BLOCK == this_spi->slave_xfer_mode)
        {
            /* Block mode so reload FIFO to the max */
            this_spi->slave_tx_idx = 0u;
            fill_slave_tx_fifo(this_spi);
        }
        else
        {
            /* Not frame or block mode? Can't do anything here... */
        }
        this_spi->hw_reg->INT_CLEAR = TXURUN_IRQ_MASK;
    }
    /*
     * Handle slave select becoming de-asserted. Only enables if
     * we are operating in block mode, in frame mode we do everything
     * in the receive and transmit interrupt handlers.
     */
    if (0u != (*this_mis & SSEND_IRQ_MASK))
    {
        uint32_t rx_size;

        read_slave_rx_fifo(this_spi);
        rx_size = this_spi->slave_rx_idx;

        /*
         * Re-enable command interrupt if required and clear all the response
         * buffer state in readiness for next response. This must be done
         * before reloading the TX FIFO.
         */
        if (0 != this_spi->cmd_handler)
        {
            this_spi->cmd_done = 0u;
            this_spi->resp_tx_buffer = 0u;
            this_spi->resp_buff_size = 0u;
            this_spi->resp_buff_tx_idx = 0u;
            this_spi->hw_reg->INT_CLEAR = CMD_IRQ_MASK;
            this_spi->hw_reg->CONTROL2 |= C2_ENABLE_CMD_IRQ_MASK;
        }

        /*
         * Reset the transmit index to 0 to restart transmit at the start of the
         * transmit buffer in the next transaction. This also requires flushing
         * the Tx FIFO and refilling it with the start of Tx data buffer.
         */
        this_spi->slave_tx_idx = 0u;
        this_spi->hw_reg->COMMAND |= ((uint32_t)TX_FIFO_RESET_MASK |
                                      (uint32_t)RX_FIFO_RESET_MASK);

        fill_slave_tx_fifo(this_spi);

        /* Prepare to receive next packet. */
        this_spi->slave_rx_idx = 0u;

        /*
         * Call the receive handler if one exists.
         */
        if ((mss_spi_block_rx_handler_t)0 != this_spi->block_rx_handler)
        {
            (*this_spi->block_rx_handler)(this_spi->slave_rx_buffer, rx_size);
        }
        this_spi->hw_reg->INT_CLEAR = SSEND_IRQ_MASK;
    }
}
/************************************************************************//***
  This is the local function to recover from rx overflow. This function performs
  set of operations which includes storing the current state of MSS SPI
  registers, reset the SPI core, restore the registers.
  Reseting the SPI core is performed by call back function which user must
  implement. Driver sends information about the SPI instance in use and the
  handler function resets particular SPI core.

  Note: while configuring the SPI in master mode, user must provide the overflow
        handler function as one of the arguments.

  @param this_spi
    The this_spi parameter is a pointer to an mss_spi_instance_t structure
    identifying the MSS SPI hardware block to be configured. There are two such
    data structures, g_mss_spi0 and g_mss_spi1, associated with MSS SPI 0 and
    MSS SPI 1 respectively. This parameter must point to either the g_mss_spi0
    or g_mss_spi1 global data structure defined within the SPI driver.
 */
static void recover_from_rx_overflow
(
    mss_spi_instance_t * this_spi
)
{
    uint32_t control_reg;
    uint32_t clk_gen;
    uint32_t frame_size;
    uint32_t control2;
    uint32_t packet_size;
    uint32_t cmd_size;
    uint32_t slave_select;

    /*
     * Read current SPI hardware block configuration.
     */
    control_reg = this_spi->hw_reg->CONTROL;
    clk_gen = this_spi->hw_reg->CLK_GEN;
    frame_size = this_spi->hw_reg->FRAMESIZE;
    control2 = this_spi->hw_reg->CONTROL2;
    packet_size = this_spi->hw_reg->PKTSIZE;
    cmd_size = this_spi->hw_reg->CMDSIZE;
    slave_select = this_spi->hw_reg->SLAVE_SELECT;

    /*
     * Reset the SPI hardware block.
     */
    if (this_spi == &g_mss_spi0_lo)
    {
        this_spi->hw_reg = MSS_SPI0_LO_BASE;
        this_spi->irqn = SPI0_PLIC;

        /* reset SPI0 */
        this_spi->buffer_overflow_handler(0U);
        this_spi->hw_reg->CONTROL &= ~CTRL_REG_RESET_MASK;
    }
    else if (this_spi == &g_mss_spi1_lo)
    {
        this_spi->hw_reg = MSS_SPI1_LO_BASE;
        this_spi->irqn = SPI1_PLIC;

        /* reset SPI1 */
        this_spi->buffer_overflow_handler(1u);
        this_spi->hw_reg->CONTROL &= ~CTRL_REG_RESET_MASK;
    }
    else if (this_spi == &g_mss_spi0_hi)
    {
        this_spi->hw_reg = MSS_SPI0_HI_BASE;
        this_spi->irqn = SPI0_PLIC;

        /* reset SPI0 */
        this_spi->buffer_overflow_handler(0u);
        this_spi->hw_reg->CONTROL &= ~CTRL_REG_RESET_MASK;
    }
    else if (this_spi == &g_mss_spi1_hi)
    {
        this_spi->hw_reg = MSS_SPI1_HI_BASE;
        this_spi->irqn = SPI1_PLIC;

        /* reset SPI1 */
        this_spi->buffer_overflow_handler(1u);
        this_spi->hw_reg->CONTROL &= ~CTRL_REG_RESET_MASK;
    }
    else
    {
        ASSERT(0);
    }

    /*
     * Restore SPI hardware block configuration.
     */
    control_reg &= ~(uint32_t)CTRL_ENABLE_MASK;
    this_spi->hw_reg->CONTROL = control_reg;
    this_spi->hw_reg->CLK_GEN = clk_gen;
    this_spi->hw_reg->FRAMESIZE = frame_size;
    this_spi->hw_reg->CONTROL |= CTRL_ENABLE_MASK;
    this_spi->hw_reg->CONTROL2 = control2;
    this_spi->hw_reg->PKTSIZE = packet_size;
    this_spi->hw_reg->CMDSIZE = cmd_size;
    this_spi->hw_reg->SLAVE_SELECT = slave_select;
}

/***************************************************************************//**
 * SPI0 interrupt service routine.
 * Please note that the name of this ISR is defined as part of the PSE HAL
 * start-up code.
 */
uint8_t spi0_plic_IRQHandler(void)
{
    if (g_spi_axi_pos & 0x01u)
    {
        mss_spi_isr(&g_mss_spi0_hi);
    }
    else
    {
        mss_spi_isr(&g_mss_spi0_lo);
    }

    return (uint8_t)EXT_IRQ_KEEP_ENABLED;
}

/***************************************************************************//**
 * SPI1 interrupt service routine.
 * Please note that the name of this ISR is defined as part of the PSE HAL
 * startup code.
 */
uint8_t spi1_plic_IRQHandler(void)
{
    if (g_spi_axi_pos & 0x02u)
    {
        mss_spi_isr(&g_mss_spi1_hi);
    }
    else
    {
        mss_spi_isr(&g_mss_spi1_lo);
    }

    return (uint8_t)EXT_IRQ_KEEP_ENABLED;
}

#ifdef __cplusplus
}
#endif

