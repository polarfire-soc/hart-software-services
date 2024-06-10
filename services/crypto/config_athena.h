/*******************************************************************************
 * Copyright 2019-2023 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC MSS User Crypto additional register and SCB register
 * definition.
 *
 */
#ifndef CONFIG_ATHENA_H
#define CONFIG_ATHENA_H

#include <stdint.h>

#define BASE32_ADDR_ATHENAREG ((volatile SATUINT32_t *) (0x20127000u))
#define BASE32_ADDR_G5USCRYPTOTOPREG ((volatile SATUINT32_t *) (0xA7010000u))

typedef struct _athenareg
{
    /* Crypto Block controls */
    uint32_t ATHENA_CR;

    /* Seed For Athena stall logic */
    uint32_t ATHENA_STALL_CR;

    /* Sets the upper 6-bits of the 38 bit Master */
    uint32_t ATHENA_UPPER_ADDRESS;

} athenareg_t;

typedef struct _g5uscryptotopreg
{
    /* System Reset Register */
    uint32_t SOFT_RESET;
    /* DLL control register 0 */
    uint32_t DLL_CTRL0;
    /* DLL control register 1 */
    uint32_t DLL_CTRL1;
    /* DLL status register 0 */
    uint32_t DLL_STAT0;
    /* DLL status register 1 */
    uint32_t DLL_STAT1;
    /* DLL status register 2 */
    uint32_t DLL_STAT2;
    /* Enables test modes on the DLL */
    uint32_t DLL_TEST;
    /* User Accessible Register for controlling Athena core via SCB bus */
    uint32_t CONTROL_USER;
    /* Registers only accessible to G5-Control for it to override the the user
     * and gain control of the Athena core. G5C control does not have access
     * to the internals of the Athena core, but can initiate purge cycles.
     */
    uint32_t CONTROL_G5C;
    /* Athena Status  */
    uint32_t STATUS;
    /* Enables Interrupt events to Fabric */
    uint32_t INTERRUPT_ENABLE;
    /* Allows RAM access speed to be tuned for different operating voltages. */
    uint32_t MARGIN;
} g5uscryptotopreg_t;

#define ATHENAREG ((athenareg_t*) BASE32_ADDR_ATHENAREG)
#define G5USCRYPTOTOPREG ((g5uscryptotopreg_t*)BASE32_ADDR_G5USCRYPTOTOPREG)

#define SYSREG_ATHENACR_RESET       (1U << 0U)
#define SYSREG_ATHENACR_PURGE       (1U << 1U)
#define SYSREG_ATHENACR_GO          (1U << 2U)
#define SYSREG_ATHENACR_RINGOSCON   (1U << 3U)
#define SYSREG_ATHENACR_COMPLETE    (1U << 8U)
#define SYSREG_ATHENACR_ALARM       (1U << 9U)
#define SYSREG_ATHENACR_BUSERROR    (1U << 10U)
#define SYSREG_SOFTRESET_ENVM       (1U << 0U)
#define SYSREG_SOFTRESET_TIMER      (1U << 4U)
#define SYSREG_SOFTRESET_MMUART0    (1U << 5U)
#define SYSREG_SOFTRESET_DDRC       (1U << 23U)
#define SYSREG_SOFTRESET_FIC3       (1U << 27U)
#define SYSREG_SOFTRESET_ATHENA     (1U << 28U)

#endif
