/*******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 * 
 * PSE microcontroller subsystem GPIO bare metal driver implementation.
 *
 * This driver is based on SmartFusion2 MSS GPIO driver v2.1.102
 *
 */
#include "mss_gpio.h"

#include "hal/hal_assert.h"
#include "mpfs_hal/mss_plic.h"
#include "mpfs_hal/mss_sysreg.h"

#if !defined(TARGET_ALOE)
#ifdef __cplusplus
extern "C" {
#endif 

/*-------------------------------------------------------------------------*//**
 * Defines.
 */
#define GPIO_INT_ENABLE_MASK                ((uint32_t)0x00000008uL)
#define OUTPUT_BUFFER_ENABLE_MASK           0x00000004u

/*Each GPIO block can have maximum 32 GPIOs*/
#define NB_OF_GPIO_GPIO0                    ((uint32_t)14)
#define NB_OF_GPIO_GPIO1                    ((uint32_t)24)
#define NB_OF_GPIO_GPIO2                    ((uint32_t)32)


/*This number includes the non-direct gpio interrupts as well*/
#define NB_OF_GPIO_INTR                     ((uint32_t)41)


/*-------------------------------------------------------------------------*//**
 * Lookup table of GPIO interrupt number indexed on GPIO ID.
 * The GPIO interrupts are multiplexed. Total GPIO interrupts are 41.
 * 41 = (14 from GPIO0 + 24 from GPIO1 + 3 non direct interrupts)
 * GPIO2 interrupts are not available by default. Setting the corresponding bit in
 * GPIO_INTERRUPT_FAB_CR(32:0) will enable GPIO2(32:0) corresponding interrupt on PLIC.
 *
 * PLINT    GPIO_INTERRUPT_FAB_CR
                0               1
    0       GPIO0 bit 0     GPIO2 bit 0
    1       GPIO0 bit 1     GPIO2 bit 1
    �           �
    12      GPIO0 bit 12    GPIO2 bit 12
    13      GPIO0 bit 13    GPIO2 bit 13
    14      GPIO1 bit 0     GPIO2 bit 14
    15      GPIO1 bit 1     GPIO2 bit 15
    �       �   �
    30      GPIO1 bit 16    GPIO2 bit 30
    31      GPIO1 bit 17    GPIO2 bit 31
    32          GPIO1 bit 18
    33          GPIO1 bit 19
    34          GPIO1 bit 20
    35          GPIO1 bit 21
    36          GPIO1 bit 22
    37          GPIO1 bit 23
    38  Or of all GPIO0 interrupts who do not have a direct connection above enabled
    39  Or of all GPIO1 interrupts who do not have a direct connection above enabled
    40  Or of all GPIO2 interrupts who do not have a direct connection above enabled
 *
 */
static const PLIC_IRQn_Type g_gpio_irqn_lut[NB_OF_GPIO_INTR] =
{
    GPIO0_BIT0_or_GPIO2_BIT0_PLIC_0,
    GPIO0_BIT1_or_GPIO2_BIT1_PLIC_1,
    GPIO0_BIT2_or_GPIO2_BIT2_PLIC_2,
    GPIO0_BIT3_or_GPIO2_BIT3_PLIC_3,
    GPIO0_BIT4_or_GPIO2_BIT4_PLIC_4,
    GPIO0_BIT5_or_GPIO2_BIT5_PLIC_5,
    GPIO0_BIT6_or_GPIO2_BIT6_PLIC_6,
    GPIO0_BIT7_or_GPIO2_BIT7_PLIC_7,
    GPIO0_BIT8_or_GPIO2_BIT8_PLIC_8,
    GPIO0_BIT9_or_GPIO2_BIT9_PLIC_9,
    GPIO0_BIT10_or_GPIO2_BIT10_PLIC_10,
    GPIO0_BIT11_or_GPIO2_BIT11_PLIC_11,
    GPIO0_BIT12_or_GPIO2_BIT12_PLIC_12,

    GPIO0_BIT13_or_GPIO2_BIT13_PLIC_13,
    GPIO1_BIT0_or_GPIO2_BIT14_PLIC_14,
    GPIO1_BIT1_or_GPIO2_BIT15_PLIC_15,
    GPIO1_BIT2_or_GPIO2_BIT16_PLIC_16,
    GPIO1_BIT3_or_GPIO2_BIT17_PLIC_17,
    GPIO1_BIT4_or_GPIO2_BIT18_PLIC_18,
    GPIO1_BIT5_or_GPIO2_BIT19_PLIC_19,
    GPIO1_BIT6_or_GPIO2_BIT20_PLIC_20,
    GPIO1_BIT7_or_GPIO2_BIT21_PLIC_21,
    GPIO1_BIT8_or_GPIO2_BIT22_PLIC_22,
    GPIO1_BIT9_or_GPIO2_BIT23_PLIC_23,
    GPIO1_BIT10_or_GPIO2_BIT24_PLIC_24,
    GPIO1_BIT11_or_GPIO2_BIT25_PLIC_25,
    GPIO1_BIT12_or_GPIO2_BIT26_PLIC_26,
    GPIO1_BIT13_or_GPIO2_BIT27_PLIC_27,

    GPIO1_BIT14_or_GPIO2_BIT28_PLIC_28,
    GPIO1_BIT15_or_GPIO2_BIT29_PLIC_29,
    GPIO1_BIT16_or_GPIO2_BIT30_PLIC_30,
    GPIO1_BIT17_or_GPIO2_BIT31_PLIC_31,

    GPIO1_BIT18_PLIC_32,
    GPIO1_BIT19_PLIC_33,
    GPIO1_BIT20_PLIC_34,
    GPIO1_BIT21_PLIC_35,
    GPIO1_BIT22_PLIC_36,
    GPIO1_BIT23_PLIC_37,

    GPIO0_NON_DIRECT_PLIC,
    GPIO1_NON_DIRECT_PLIC,
    GPIO2_NON_DIRECT_PLIC
};

/*Local functions*/
static uint8_t gpio_number_validate(GPIO_TypeDef * gpio, mss_gpio_id_t gpio_idx);

/*-------------------------------------------------------------------------*//**
 * MSS_GPIO_init
 * See "mss_gpio.h" for details of how to use this function.
 */
void MSS_GPIO_init( GPIO_TypeDef * gpio )
{
    uint32_t inc;
    
    /* reset appropriate MSS GPIO hardware block*/
    if((GPIO0_LO == gpio) || (GPIO0_HI == gpio))
    {
        SYSREG->SOFT_RESET_CR |= ( (1u << 20u));
    }
    else if((GPIO1_LO == gpio) || (GPIO1_HI == gpio))
    {
        SYSREG->SOFT_RESET_CR |= ( (1u << 21u));
    }
    else if((GPIO2_LO == gpio) || (GPIO2_HI == gpio))
    {
        SYSREG->SOFT_RESET_CR |= ( (1u << 22u));
    }
    else
        HAL_ASSERT(0);

    /* Clear any previously pended MSS GPIO interrupts*/

    /* Take appropriate MSS GPIO hardware out of reset. */
    if((GPIO0_LO == gpio) || (GPIO0_HI == gpio))
    {
        SYSREG->SOFT_RESET_CR &= ~( (1u << 20u) );
    }
    else if((GPIO1_LO == gpio) || (GPIO1_HI == gpio))

    {
        SYSREG->SOFT_RESET_CR &= ~( (1u << 21u) );
    }
    else if((GPIO2_LO == gpio) || (GPIO2_HI == gpio))
    {
        SYSREG->SOFT_RESET_CR &= ~( (1u << 22u) );
    }
}

/*-------------------------------------------------------------------------*//**
 * MSS_GPIO_config
 * See "mss_gpio.h" for details of how to use this function.
 */
void MSS_GPIO_config
(
    GPIO_TypeDef * gpio,
    mss_gpio_id_t port_id,
    uint32_t config
)
{
    if(0 == gpio_number_validate(gpio, port_id))
        gpio->GPIO_CFG[port_id] = config;
}

/*-------------------------------------------------------------------------*//**
 * MSS_GPIO_set_output
 * See "mss_gpio.h" for details of how to use this function.
 */
void MSS_GPIO_set_output
(
    GPIO_TypeDef * gpio,
    mss_gpio_id_t port_id,
    uint8_t value
)
{
    uint32_t gpio_setting;
    
    if(0 == gpio_number_validate(gpio, port_id))
    {
        /* Setting the bit in GPIO_SET_BITS (offset 0xA4) sets the corresponding
         * output port.
         * Setting the bit in GPIO_CLR_BITS (offset 0xA0) clears the corresponding
         * output port.*/

        if(value)
            gpio->GPIO_SET_BITS = ((uint32_t)0x01u << port_id);
        else
            gpio->GPIO_CLR_BITS = ((uint32_t)0x01u << port_id);
    }
}

/*-------------------------------------------------------------------------*//**
 * MSS_GPIO_drive_inout
 * See "mss_gpio.h" for details of how to use this function.
 */
void MSS_GPIO_drive_inout
(
    GPIO_TypeDef * gpio,
    mss_gpio_id_t port_id,
    mss_gpio_inout_state_t inout_state
)
{
    uint32_t outputs_state;
    uint32_t config;
    
    if(0 == gpio_number_validate(gpio, port_id))
    {
        switch(inout_state)
        {
            case MSS_GPIO_DRIVE_HIGH:
                /* Set output high */
                gpio->GPIO_SET_BITS = (uint32_t)1 << port_id;

                /* Enable output buffer */
                config = gpio->GPIO_CFG[port_id];
                config |= OUTPUT_BUFFER_ENABLE_MASK;
                gpio->GPIO_CFG[port_id] = config;
            break;

            case MSS_GPIO_DRIVE_LOW:
                /* Set output low */
                gpio->GPIO_CLR_BITS = (uint32_t)1 << port_id;
                /* Enable output buffer */
                config = gpio->GPIO_CFG[port_id];
                config |= OUTPUT_BUFFER_ENABLE_MASK;
                gpio->GPIO_CFG[port_id] = config;
            break;

            case MSS_GPIO_HIGH_Z:
                /* Disable output buffer */
                config = gpio->GPIO_CFG[port_id];
                config &= ~OUTPUT_BUFFER_ENABLE_MASK;
                gpio->GPIO_CFG[port_id] = config;
            break;

            default:
                HAL_ASSERT(0);
            break;
        }
    }
}

/*-------------------------------------------------------------------------*//**
 * MSS_GPIO_enable_irq
 * See "mss_gpio.h" for details of how to use this function.
 */
void MSS_GPIO_enable_irq
(
    GPIO_TypeDef * gpio,
    mss_gpio_id_t port_id
)
{
    uint32_t cfg_value;

    if(0 == gpio_number_validate(gpio, port_id))
    {
        cfg_value = gpio->GPIO_CFG[(uint8_t)port_id];
        gpio->GPIO_CFG[(uint8_t)port_id] = (cfg_value | GPIO_INT_ENABLE_MASK);

        if((GPIO0_LO == gpio) || (GPIO0_HI == gpio))
        {
            if((0x01UL << port_id) & SYSREG->GPIO_INTERRUPT_FAB_CR)
                PLIC_EnableIRQ(GPIO0_NON_DIRECT_PLIC);
            else
                PLIC_EnableIRQ(g_gpio_irqn_lut[port_id]);
        }
        else if((GPIO1_LO == gpio) || (GPIO1_HI == gpio))
        {
            if((0x01u << (port_id+14)) & SYSREG->GPIO_INTERRUPT_FAB_CR)
                PLIC_EnableIRQ(GPIO1_NON_DIRECT_PLIC);
            else
                PLIC_EnableIRQ(g_gpio_irqn_lut[port_id+14]);
        }
        else if((GPIO2_LO == gpio) || (GPIO2_HI == gpio))
        {
            if((0x01u << port_id) & SYSREG->GPIO_INTERRUPT_FAB_CR)
                PLIC_EnableIRQ(g_gpio_irqn_lut[port_id]);
            else
                PLIC_EnableIRQ(GPIO2_NON_DIRECT_PLIC);
        }
        else
            HAL_ASSERT(0);
    }
}

/*-------------------------------------------------------------------------*//**
 * MSS_GPIO_disable_irq
 * See "mss_gpio.h" for details of how to use this function.
 */

void MSS_GPIO_disable_irq
(
    GPIO_TypeDef * gpio,
    mss_gpio_id_t port_id
)
{
    uint32_t cfg_value;
    
    if(0 == gpio_number_validate(gpio, port_id))
    {
        cfg_value = gpio->GPIO_CFG[(uint8_t)port_id];
        gpio->GPIO_CFG[(uint8_t)port_id] = (cfg_value & ~GPIO_INT_ENABLE_MASK);

        if((GPIO0_LO == gpio) || (GPIO0_HI == gpio))
        {
            if((0x01UL << port_id) & SYSREG->GPIO_INTERRUPT_FAB_CR)
                PLIC_DisableIRQ(GPIO0_NON_DIRECT_PLIC);
            else
                PLIC_DisableIRQ(g_gpio_irqn_lut[port_id]);
        }
        else if((GPIO1_LO == gpio) || (GPIO1_HI == gpio))
        {
            if((0x01u << (port_id+14)) & SYSREG->GPIO_INTERRUPT_FAB_CR)
                PLIC_DisableIRQ(GPIO1_NON_DIRECT_PLIC);
            else
                PLIC_DisableIRQ(g_gpio_irqn_lut[port_id+14]);
        }
        else if((GPIO2_LO == gpio) || (GPIO2_HI == gpio))
        {
            if((0x01u << port_id) & SYSREG->GPIO_INTERRUPT_FAB_CR)
                PLIC_DisableIRQ(g_gpio_irqn_lut[port_id]);
            else
                PLIC_DisableIRQ(GPIO2_NON_DIRECT_PLIC);
        }
        else
            HAL_ASSERT(0);
    }
}

/*-------------------------------------------------------------------------*//**
 * MSS_GPIO_clear_irq
 * See "mss_gpio.h" for details of how to use this function.
 */
void MSS_GPIO_clear_irq
(
    GPIO_TypeDef * gpio,
    mss_gpio_id_t port_id
)
{
    if(0 == gpio_number_validate(gpio, port_id))
    {
        gpio->GPIO_IRQ = ((uint32_t)1) << port_id;
        __asm("fence.i");
    }
}

static uint8_t gpio_number_validate(GPIO_TypeDef * gpio, mss_gpio_id_t gpio_idx)
{
    if(((GPIO0_LO == gpio) || (GPIO0_HI == gpio)) && (gpio_idx >= NB_OF_GPIO_GPIO0))
    {
        HAL_ASSERT(0);
        return 1;
    }
    if(((GPIO1_LO == gpio) || (GPIO1_HI == gpio)) && (gpio_idx >= NB_OF_GPIO_GPIO1))
    {
        HAL_ASSERT(0);
        return 1;
    }
    if(((GPIO2_LO == gpio) || (GPIO2_HI == gpio)) && (gpio_idx >= NB_OF_GPIO_GPIO2))
    {
        HAL_ASSERT(0);
        return 1;
    }

    return 0;
}

#ifdef __cplusplus
}
#endif
#endif /* !defined(TARGET_ALOE) */
