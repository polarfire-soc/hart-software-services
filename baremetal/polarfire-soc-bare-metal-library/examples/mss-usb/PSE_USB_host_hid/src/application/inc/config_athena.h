/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * PolarFire SoC MSS User Crypto additional register and SCB register 
 * definition.
 *
 * SVN $Revision: 12212 $
 * SVN $Date: 2019-09-03 15:18:58 +0530 (Tue, 03 Sep 2019) $
 */
#ifndef CONFIG_ATHENA_H
#define CONFIG_ATHENA_H

#ifdef POLARFIRE_SOC_MSS_USER_CRYPTO

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

#endif
#endif
