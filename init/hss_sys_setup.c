/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file  System Setup Functions
 * \brief Functions for setting up the PMP, MPU, PLIC, IOMUX, PAD I/O, L2 Cache. Clocks
 *
 * These functions are used by the boot service, but they can be replaced by 
 * auto-generated code from an external tool, e.g. Libero.
 *
 * What needs to be implemented are the following functions:
 *     bool HSS_Setup_PAD_IO(void);
 *     bool HSS_Setup_PLIC(void);
 *     bool HSS_Setup_MPU(void);
 *     bool HSS_Setup_L2Cache(void);
 *     bool HSS_Setup_Clocks(void);
 *     bool HSS_Setup_PMP(void);
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#define read_csr csr_read
#define set_csr  csr_write

#include <assert.h>
#include <string.h>

#include "hss_clock.h"
#include "hss_state_machine.h"
#include "hss_sys_setup.h"

#include "csr_helper.h"

#ifdef CONFIG_PLATFORM_MPFS
#  define RISCV_CSR_ENCODING_H
#  define RISCV_ENCODING_H
#  include "mss_sysreg.h"
#  include "mss_plic.h"
#  include "mss_l2_cache.h"
#  include "nwc/mss_io_config.h"
#  include "system_startup.h"
#endif
#include "hss_memcpy_via_pdma.h"

#include "mpfs_reg_map.h"

/******************************************************************************/



/****************************************************************************/

//
// Setup Functions
//

/*!
 * \brief PAD I/O Configuration
 *
 * There is no dynamic PVT calibration required for the MSSIOs. Instead, pcode and ncode values
 * is read by MPFS HSS Embedded Software from system registers, which are initialized by G5C at
 * boot time (reading from factory data).
 *
 * The pcode and ncode values to be applied to each bank depend on the user selection of voltage
 * threshold standard (specified in MSS configurator) to be applied to the bank.
 *
 * MPFS HSS Embedded Software performs the setting of the pcode and ncode values to the MSSIOs,
 * to avoid requiring dedicated hardware decode them from the system registers.
 * The E51 will need to setup per-bank and per-IO configurations for MSSIO.
 *
 * Per bank configuration includes weak pullup/pulldown, drive strength etc. configured as per
 * user requirement (specified in MSS configurator).
 *
 * Per I/O configuration includes mux configuration.
 */
bool HSS_Setup_PAD_IO(void)
{
#ifdef CONFIG_PLATFORM_MPFS
    (void)mssio_setup();
#endif

    return true;
}

/*!
 * \brief PLIC Setup and Configuration
 *
 * The RISC-V Platform-Level Interrupt Controller (PLIC) allows 
 * broadcasting of interrupts to one  or more Harts within a Core Complex.
 *
 * The operation of the PLIC is suited to SMP operation: An interrupt 
 * source occurs to PLIC and then any available processor can conceivable 
 * claim it unless its configuration is locked down.
 *
 * This obviously doesn't work well if the processors are in AMP 
 * configuration and we are trying to * maintain hardware separation.
 * Therefore, PolarFire SoC can secure PLIC configuration through the
 * use of RISC-V PMP registers.
 *
 * Furthermore, MPRs allow assignment of each of 160 interrupts going to 
 * the PLIC to one of two hardware contexts. This assignment can be 
 * configured by E51 at boot time and is then locked until subsequent 
 * reset.
 */
bool HSS_Setup_PLIC(void)
{
#ifdef CONFIG_PLATFORM_MPFS
    uint32_t inc;

    /* default all priorities so effectively disabled */
    for(inc = 0U; inc < PLIC_NUM_SOURCES; ++inc)
    {   
        /* priority must be greater than threshold to be enabled, so setting to
         * 7 disables */
        PLIC->SOURCE_PRIORITY[inc]  = 0U; 
    }   

    for(inc = 0U; inc < NUM_CLAIM_REGS; ++inc)
    {   
        PLIC->TARGET[inc].PRIORITY_THRESHOLD  = 7U; 
    }   

    /* and clear all the enables */
    for(inc = 0UL; inc < PLIC_SET_UP_REGISTERS; inc++)
    {   
        PLIC->HART0_MMODE_ENA[inc] = 0U; 
        PLIC->HART1_MMODE_ENA[inc] = 0U; 
        PLIC->HART1_SMODE_ENA[inc] = 0U; 
        PLIC->HART2_MMODE_ENA[inc] = 0U; 
        PLIC->HART2_SMODE_ENA[inc] = 0U; 
        PLIC->HART3_MMODE_ENA[inc] = 0U; 
        PLIC->HART3_SMODE_ENA[inc] = 0U; 
        PLIC->HART4_MMODE_ENA[inc] = 0U; 
        PLIC->HART4_SMODE_ENA[inc] = 0U; 
    }   

    /* clear any pending interrupts- in case already there */
    PLIC_ClearPendingIRQ();
#endif

    return true;
}

/*!
 * \brief MPU Setup and Configuration
 *
 * The E51 ensures that hardware separate is ensured before the U54 code starts running.
 * To do this, it needs to partition memory and peripheral access, based on configuration
 * information provided at build time.
 *
 * The MSS MPUs are memory mapped and can be setup once by the E51. They are then locked
 * until subsequent system reboot. The MPRs allow assignment of peripheral resources to
 * one of two separate hardware contexts.
 */
bool HSS_Setup_MPU(void)
{
#ifdef CONFIG_PLATFORM_MPFS
#if 0
    static const struct HSS_MPU_Config {
        uint64_t fic0;
        uint64_t fic1;
        uint64_t fic2;
        uint64_t crypto;
        uint64_t ethernet0;
        uint64_t ethernet1;
        uint64_t usb;
        uint64_t mmc;
        uint64_t scb;
        uint64_t seg0;
        uint64_t seg1;
    } hss_MPU_Config = {
        0x0u, // fic0
        0x0u, // fic1
        0x0u, // fic2
        0x0u, // crypto
        0x0u, // ethernet0
        0x0u, // ethernet1
        0x0u, // usb
        0x0u, // mmc
        0x0u, // scb
        0x0u, // seg0
        0x0u, // seg1
    };

    mHSS_WriteRegU64(MPU, FIC0,		hss_MPU_Config.fic0);
    mHSS_WriteRegU64(MPU, FIC1,		hss_MPU_Config.fic1);
    mHSS_WriteRegU64(MPU, FIC2,		hss_MPU_Config.fic2);
    mHSS_WriteRegU64(MPU, CRYPTO,	hss_MPU_Config.crypto);
    mHSS_WriteRegU64(MPU, ETHERNET0,	hss_MPU_Config.ethernet0);
    mHSS_WriteRegU64(MPU, ETHERNET1,	hss_MPU_Config.ethernet1);
    mHSS_WriteRegU64(MPU, USB,		hss_MPU_Config.usb);
    mHSS_WriteRegU64(MPU, MMC,		hss_MPU_Config.mmc);
    mHSS_WriteRegU64(MPU, SCB,		hss_MPU_Config.scb);
    mHSS_WriteRegU64(MPU, SEG0,		hss_MPU_Config.seg0);
    mHSS_WriteRegU64(MPU, SEG1,		hss_MPU_Config.seg1);
#endif
#endif

    return true;
}

/*!
 * \brief PMP Setup and Configuration
 *
 * The E51 ensures that hardware separate is ensured before the U54 code starts running.
 * To do this, it needs to partition memory and peripheral access, based on configuration
 * information provided at build time.
 *
 * In order to setup RISC-V PMPs, the E51 instructs the U54s to  run code routines through
 * setting their reset vectors and temporarily bringing them out of WFI. This is because
 * the U54-specific PMP registers are CSRs and thus are only available locally on the
 * individual U54 and not memory mapped.
 *
 * The PMPs will be setup in M-Mode on the U54s and locked so that their configuration
 * cannot subsequently be changed without reboot to prevent accidental or malicious
 * modification through software defect.
 */
bool HSS_Setup_PMP(void)
{
#ifdef CONFIG_PLATFORM_MPFS
#if 0
    static const struct HSS_PMP_Config {
        uint64_t pmpcfg0;
        uint64_t pmpcfg2;

        // 63:54 WIRI, 53:0 WARL corresponding to bits 55:2 of the address 
        void *pmpaddr0; 
        void *pmpaddr1;
        void *pmpaddr2;
        void *pmpaddr3;
        void *pmpaddr4;
        void *pmpaddr5;
        void *pmpaddr6;
        void *pmpaddr7;
        void *pmpaddr8;
        void *pmpaddr9;
        void *pmpaddr10;
        void *pmpaddr11;
        void *pmpaddr12;
        void *pmpaddr13;
        void *pmpaddr14;
        void *pmpaddr15;
    } hss_PMP_Config[4] = {
        // PMP Configuration for U54_1
        { 0x0u, 0x0u,  // pmpcfg*
          NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
          NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, }, // pmpaddr*

        // PMP Configuration for U54_2
        { 0x0u, 0x0u,  //pmpcfg*
          NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
          NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, }, // pmpaddr*

        // PMP Configuration for U54_3
        { 0x0u, 0x0u,  //pmpcfg*
          NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
          NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, }, // pmpaddr*
    
        // PMP Configuration for U54_4
        { 0x0u, 0x0u,  //pmpcfg*
          NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
          NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, }, // pmpaddr*
    };

    const enum HSSHartId myHartId = current_hartid();
    //mHSS_DEBUG_PRINTF("myHartId is %d\n", myHartId);
    assert(myHartId <= mSPAN_OF(hss_PMP_Config));

    const struct HSS_PMP_Config *pConfig = &hss_PMP_Config[myHartId]; 

    mHSS_CSR_WRITE(CSR_PMPCFG0,   pConfig->pmpcfg0);
    mHSS_CSR_WRITE(CSR_PMPCFG2,   pConfig->pmpcfg2);
    mHSS_CSR_WRITE(CSR_PMPADDR0,  pConfig->pmpaddr0);
    mHSS_CSR_WRITE(CSR_PMPADDR1,  pConfig->pmpaddr1);
    mHSS_CSR_WRITE(CSR_PMPADDR2,  pConfig->pmpaddr2);
    mHSS_CSR_WRITE(CSR_PMPADDR3,  pConfig->pmpaddr3);
    mHSS_CSR_WRITE(CSR_PMPADDR4,  pConfig->pmpaddr4);
    mHSS_CSR_WRITE(CSR_PMPADDR5,  pConfig->pmpaddr5);
    mHSS_CSR_WRITE(CSR_PMPADDR6,  pConfig->pmpaddr6);
    mHSS_CSR_WRITE(CSR_PMPADDR7,  pConfig->pmpaddr7);
    mHSS_CSR_WRITE(CSR_PMPADDR8,  pConfig->pmpaddr8);
    mHSS_CSR_WRITE(CSR_PMPADDR9,  pConfig->pmpaddr9);
    mHSS_CSR_WRITE(CSR_PMPADDR10, pConfig->pmpaddr10);
    mHSS_CSR_WRITE(CSR_PMPADDR11, pConfig->pmpaddr11);
    mHSS_CSR_WRITE(CSR_PMPADDR12, pConfig->pmpaddr12);
    mHSS_CSR_WRITE(CSR_PMPADDR13, pConfig->pmpaddr13);
    mHSS_CSR_WRITE(CSR_PMPADDR14, pConfig->pmpaddr14);
    mHSS_CSR_WRITE(CSR_PMPADDR15, pConfig->pmpaddr15);
#endif
#endif

    return true;
}

/*!
 * \brief L2Cache and Scratchpad Setup
 *
 * The E51 is responsible for configuring the cache controller, including locking of ways and
 * configuring between L2 Cache and Scratchpad for real-time AMP needs, based on configuration
 * information provided as a Libero output.
 */
bool HSS_Setup_L2Cache(void)
{
#ifdef CONFIG_PLATFORM_MPFS
    config_l2_cache();
#endif

    return true;
}

/*!
 * \brief Clock Configuration
 *
 * Setup clock enables and soft resets
 *
 */
bool HSS_Setup_Clocks(void)
{
#ifdef CONFIG_PLATFORM_MPFS
    static const uint32_t hss_subblk_clock_Config = 0xFFFFFFFFu;
    const uint32_t hss_soft_reset_Config = SYSREG->SOFT_RESET_CR & 
        ~( SOFT_RESET_CR_ENVM_MASK |
           SOFT_RESET_CR_MAC0_MASK |
           SOFT_RESET_CR_MAC1_MASK |
           SOFT_RESET_CR_TIMER_MASK |
           SOFT_RESET_CR_MMUART0_MASK |
           SOFT_RESET_CR_MMUART1_MASK |
           SOFT_RESET_CR_MMUART2_MASK |
           SOFT_RESET_CR_MMUART3_MASK |
           SOFT_RESET_CR_MMUART4_MASK |
           SOFT_RESET_CR_RTC_MASK |
           SOFT_RESET_CR_QSPI_MASK |
           SOFT_RESET_CR_GPIO2_MASK |
           //SOFT_RESET_CR_DDRC_MASK |
           //SOFT_RESET_CR_ATHENA_MASK |
           SOFT_RESET_CR_SGMII_MASK );

    SYSREG->SOFT_RESET_CR = 0xFFFFFFFFu;
    SYSREG->SUBBLK_CLOCK_CR = hss_subblk_clock_Config;
    SYSREG->SOFT_RESET_CR = hss_soft_reset_Config;
#endif

    return true;
}

/*!
 * \brief Bus Error Unit
 *
 * Setup bus error unit
 */
bool HSS_Setup_BusErrorUnit(void)
{
#ifdef CONFIG_PLATFORM_MPFS
    (void)init_bus_error_unit();
#endif

    return true;
}
