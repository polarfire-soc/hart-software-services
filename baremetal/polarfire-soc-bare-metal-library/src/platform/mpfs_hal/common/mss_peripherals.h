/*******************************************************************************
 * Copyright 2019 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * @file mss_peripherals.h
 * @author Microchip FPGA Embedded Systems Solutions
 * @brief PolarFire SoC MSS fumnctions related to MSS peripherals.
 *
 */

#ifndef MSS_PERIPHERALS_H
#define MSS_PERIPHERALS_H

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#if !defined (LIBERO_SETTING_CONTEXT_A_EN)
#define LIBERO_SETTING_CONTEXT_A_EN         0x00000000UL
#endif
#if !defined (LIBERO_SETTING_CONTEXT_B_EN)
#define LIBERO_SETTING_CONTEXT_B_EN         0x00000000UL
#endif
#if !defined (LIBERO_SETTING_CONTEXT_A_EN_FIC)
#define LIBERO_SETTING_CONTEXT_A_EN_FIC     0x0000000FUL
#endif
#if !defined (LIBERO_SETTING_CONTEXT_B_EN_FIC)
#define LIBERO_SETTING_CONTEXT_B_EN_FIC     0x0000000FUL
#endif

/***************************************************************************//**

 */
typedef enum PERIPH_RESET_STATE_
{

    PERIPHERAL_ON                       = 0x00,        /*!< 0 RST and clk ON  */
    PERIPHERAL_OFF                      = 0x01,        /*!< 1 RST and clk OFF */
} PERIPH_RESET_STATE;

#define CONTEXT_EN_INDEX                 0x00U
#define CONTEXT_EN_INDEX_FIC             0x01U
#define SUBBLK_CLOCK_NA_MASK             0x00U

typedef enum mss_peripherals_ {
    MSS_PERIPH_MMUART0      = 0U,
    MSS_PERIPH_MMUART1      = 1U,
    MSS_PERIPH_MMUART2      = 2U,
    MSS_PERIPH_MMUART3      = 3U,
    MSS_PERIPH_MMUART4      = 4U,
    MSS_PERIPH_WDOG0        = 5U,
    MSS_PERIPH_WDOG1        = 6U,
    MSS_PERIPH_WDOG2        = 7U,
    MSS_PERIPH_WDOG3        = 8U,
    MSS_PERIPH_WDOG4        = 9U,
    MSS_PERIPH_SPI0         = 10U,
    MSS_PERIPH_SPI1         = 11U,
    MSS_PERIPH_I2C0         = 12U,
    MSS_PERIPH_I2C1         = 13U,
    MSS_PERIPH_CAN0         = 14U,
    MSS_PERIPH_CAN1         = 15U,
    MSS_PERIPH_MAC0         = 16U,
    MSS_PERIPH_MAC1         = 17U,
    MSS_PERIPH_TIMER        = 18U,
    MSS_PERIPH_GPIO0        = 19U,
    MSS_PERIPH_GPIO1        = 20U,
    MSS_PERIPH_GPIO2        = 21U,
    MSS_PERIPH_RTC          = 22U,
    MSS_PERIPH_M2FINT       = 23U,
    MSS_PERIPH_CRYPTO       = 24U,
    MSS_PERIPH_USB          = 25U,
    MSS_PERIPH_QSPIXIP      = 26U,
    MSS_PERIPH_ATHENA       = 27U,
    MSS_PERIPH_TRACE        = 28U,
    MSS_PERIPH_MAILBOX_SC   = 29U,
    MSS_PERIPH_EMMC         = 30U,
    MSS_PERIPH_CFM          = 31U,
    MSS_PERIPH_FIC0         = 32U,
    MSS_PERIPH_FIC1         = 33U,
    MSS_PERIPH_FIC2         = 34U,
    MSS_PERIPH_FIC3         = 35U,
    MSS_PERIPH_INVALID      = 255U
} mss_peripherals;

#ifndef LIBERO_SETTING_TURN_OFF_RAM_IF_NOT_USED
#define LIBERO_SETTING_TURN_OFF_RAM_IF_NOT_USED
#endif

#ifndef LIBERO_SETTING_CONFIGURED_PERIPHERALS
#define LIBERO_SETTING_CONFIGURED_PERIPHERALS 0xFFFFFFFF
#endif

#define CAN0_RAM_OFF_IF_NOT_CONFIGURED() if((LIBERO_SETTING_CONFIGURED_PERIPHERALS\
        & (1U<<15U))==0U) (SYSREG->RAM_SHUTDOWN_CR |= (1U <<0U))
#define CAN1_RAM_OFF_IF_NOT_CONFIGURED() if((LIBERO_SETTING_CONFIGURED_PERIPHERALS\
        & (1U<<16U))==0U) (SYSREG->RAM_SHUTDOWN_CR |= (1U <<1U))
#define USB_RAM_OFF_IF_NOT_CONFIGURED() if((LIBERO_SETTING_CONFIGURED_PERIPHERALS\
        & (1U<<2U))==0U) (SYSREG->RAM_SHUTDOWN_CR |= (1U <<2U))
#define MAC0_RAM_OFF_IF_NOT_CONFIGURED() if((LIBERO_SETTING_CONFIGURED_PERIPHERALS\
        & (1U<<3U))==0U) (SYSREG->RAM_SHUTDOWN_CR |= (1U<<3U))
#define MAC1_RAM_OFF_IF_NOT_CONFIGURED() if((LIBERO_SETTING_CONFIGURED_PERIPHERALS\
        & (1U<<4U))==0U) (SYSREG->RAM_SHUTDOWN_CR |= (1U<<4U))
#define MMC_RAM_OFF_IF_NOT_CONFIGURED() if((LIBERO_SETTING_CONFIGURED_PERIPHERALS\
        & (3U<<0U))==0U) (SYSREG->RAM_SHUTDOWN_CR |= (1U<<5U))
#define DDR_RAM_OFF_IF_NOT_CONFIGURED() if((LIBERO_SETTING_DDRPHY_MODE &\
            DDRPHY_MODE_MASK) == DDR_OFF_MODE)\
                (SYSREG->RAM_SHUTDOWN_CR |= (1U<<7U))

/***************************************************************************//**
  This function is used to turn on or off a peripheral. If contexts have been
  configured, these will be checked to see if peripheral should be controlled
  from a particular context.

  @param peripheral
    See enum mss_peripherals for list of peripherals

  @param hart
    Origin hart of this request

  @req_state
    Turn peripheral on or off:
        - PERIPHERAL_ON
        - PERIPHERAL_OFF
  Example:
  @code
    uint8_t err_status;
    err_status = mss_config_clk_rst(MSS_PERIPH_MMUART0, (uint8_t) origin_hart_ID, PERIPHERAL_ON);

    if(0U != err_status)
    {
       print_uart0("\n\r Context not allowed to access UART0 from hart:%d\n\nr", origin_hart_ID);
    }
  @endcode
 */
uint8_t mss_config_clk_rst(mss_peripherals peripheral, uint8_t hart, PERIPH_RESET_STATE req_state);

/***************************************************************************//**
  This function is used to turn on the fabric enable

  Example:
  @code
    (void)mss_config_clk_rst(MSS_PERIPH_FIC3, (uint8_t)MPFS_HAL_FIRST_HART, PERIPHERAL_ON);
    mss_enable_fabric();
  @endcode
 */
void mss_enable_fabric(void);

/***************************************************************************//**
  This function is used to turn on the fabric enable

  Example:
  @code
    mss_disable_fabric();
  @endcode
 */
void mss_disable_fabric(void);

/***************************************************************************//**
  This function is used to set the apb_bus_cr register value

  @param reg_value value of the register you want to set.
  This value is available from the MSS configurator
  LIBERO_SETTING_APBBUS_CR

  Example:
  @code
    mss_set_apb_bus_cr(LIBERO_SETTING_APBBUS_CR);
  @endcode
 */
void mss_set_apb_bus_cr(uint32_t reg_value);

/***************************************************************************//**
  This function is used to get the apb_bus_cr register value

  @return Return the apb_bus_cr reg value

  Example:
  @code
    uint32_t cr_reg;
    cr_reg = mss_get_apb_bus_cr();
  @endcode
 */
uint32_t mss_get_apb_bus_cr(void);

/***************************************************************************//**
  This function is used to turn off RAM associated with peripherals that are
  marked as unused in the MSS Configurator.

  Example:
  @code
    mss_turn_off_unused_ram_clks();
  @endcode
 */
void mss_turn_off_unused_ram_clks(void);


#ifdef __cplusplus
}
#endif


#endif /* MSS_PERIPHERALS_H */
