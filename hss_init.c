/*******************************************************************************
 * Copyright 2017-2020 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * 
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS Software Initalization
 * \brief Full System Initialization
 */

#include "config.h"
#include "hss_types.h"
#include <assert.h>

#include "hss_state_machine.h"
#include "ssmb_ipi.h"
#include "hss_debug.h"
#include "hss_registry.h"

#include "hss_atomic.h"
#include "hss_init.h"
#include "hss_version.h"
#ifdef CONFIG_TINYCLI
#    include "hss_tinycli.h"
#endif

#include "csr_helper.h"

#ifdef CONFIG_SERVICE_BOOT
#  include "hss_boot_service.h"
#endif

#ifdef CONFIG_OPENSBI
#  include "sbi/riscv_asm.h"
#endif

#include "hss_sys_setup.h"
#include "mpfs_reg_map.h"

bool HSS_UARTInit(void);
bool HSS_DDRInit(void);

#ifdef CONFIG_USE_PDMA
bool HSS_PDMAInit(void);
#endif

#ifdef CONFIG_USE_LOGO
bool HSS_LogoInit(void);
#endif

/**
 * \brief Main Initialization Function
 *
 * All other initialization routines to be chained off this...
 */

/****************************************************************************/

#include <string.h>

#define E51_DTIM_START   0x01000000u
#define E51_DTIM_END     0x01001FFFu

#define E51_ITIM_START   0x01800000u
#define E51_ITIM_END     0x01801FFFu

#define U54_1_ITIM_START 0x01808000u
#define U54_1_ITIM_END   0x0180EFFFu

#define U54_2_ITIM_START 0x01810000u
#define U54_2_ITIM_END   0x01816FFFu

#define U54_3_ITIM_START 0x01818000u
#define U54_3_ITIM_END   0x0181EFFFu

#define U54_4_ITIM_START 0x01820000u
#define U54_4_ITIM_END   0x01826FFFu

#define L2LIM_START      0x08000000u
#define L2LIM_END        0x09FFFFFFu

#include "mss_sysreg.h"
void HSS_LowlevelInit(void)
{
    //  Initializing Clocks and IO
    HSS_Setup_Clocks();
    HSS_Setup_PAD_IO();
}


/* Init memories.. */
void HSS_Init_TIMs(void)
{
    mHSS_DEBUG_PRINTF(" Initializing TIMs" CRLF);
    memset((void*)E51_DTIM_START, 0, E51_DTIM_END - E51_DTIM_START);       /* 8KiB */
    mHSS_DEBUG_PRINTF(" ... E51_DTIM (%lu bytes)" CRLF, E51_DTIM_END - E51_DTIM_START);

    memset((void*)U54_1_ITIM_START, 0, U54_1_ITIM_END - U54_1_ITIM_START); /* 28KiB */
    mHSS_DEBUG_PRINTF(" ... U54_1_ITIM (%lu bytes)" CRLF, U54_1_ITIM_END - U54_1_ITIM_START);

    memset((void*)U54_2_ITIM_START, 0, U54_2_ITIM_END - U54_2_ITIM_START); /* 28KiB */
    mHSS_DEBUG_PRINTF(" ... U54_2_ITIM (%lu bytes)" CRLF, U54_2_ITIM_END - U54_2_ITIM_START);

    memset((void*)U54_3_ITIM_START, 0, U54_3_ITIM_END - U54_3_ITIM_START); /* 28KiB */
    mHSS_DEBUG_PRINTF(" ... U54_3_ITIM (%lu bytes)" CRLF, U54_3_ITIM_END - U54_3_ITIM_START);

    memset((void*)U54_4_ITIM_START, 0, U54_4_ITIM_END - U54_4_ITIM_START); /* 28KiB */
    mHSS_DEBUG_PRINTF(" ... U54_4_ITIM (%lu bytes)" CRLF, U54_4_ITIM_END - U54_4_ITIM_START);

    //memset((void*)L2LIM_START, 0, L2LIM_END - L2LIM_START);                /* 32MiB */

    mb();
    mb_i();
}

void HSS_Init_Setup_RWDATA_And_BSS(void)
{
    //UART not setup at this point
    //mHSS_DEBUG_PRINTF("Setting up RW Data and BSS sections" CRLF);

    // Copy RWDATA
    {
        extern uint32_t _rwdata_load;
        extern uint32_t _rwdata_exec_start;
        extern uint32_t _rwdata_exec_end;
        uint32_t *pSrc = &_rwdata_load, *pDst = &_rwdata_exec_start;
        size_t len = (size_t)(&_rwdata_exec_end - &_rwdata_exec_start);

        assert(len);

        while (len--) {
            *pDst++ = *pSrc++;
        }
    }

    // Setup BSS
    {
        extern char _bss_start;
        extern char _bss_end;
        size_t len = (size_t)(&_bss_end - &_bss_start);

        assert(len);

        memset(&_bss_start, 0, len);
    }

    // wake sleeping HARTs
    mHSS_WriteRegU32(CLINT, MSIP_U54_1, 1u);
    mHSS_WriteRegU32(CLINT, MSIP_U54_2, 1u);
    mHSS_WriteRegU32(CLINT, MSIP_U54_3, 1u);
    mHSS_WriteRegU32(CLINT, MSIP_U54_4, 1u);
}

/****************************************************************************/

void HSS_Init(void)
{
    HSS_LowlevelInit();
    HSS_UARTInit(); // initialize this manually to enable debug output
    //HSS_Init_TIMs();
#ifdef CONFIG_USE_LOGO
    HSS_LogoInit();
#endif

    mHSS_DEBUG_STATUS_TEXT;

    mHSS_DEBUG_PRINTF("PolarFireSOC Hart Software Services (HSS) - Version %d.%d.%d" CRLF, 
        HSS_VERSION_MAJOR, HSS_VERSION_MINOR, HSS_VERSION_PATCH);
    mHSS_DEBUG_PRINTF("(c) Copyright 2017-2020 Microchip Corporation." CRLF CRLF);
    char misa_strBuf[26];
#ifdef CONFIG_OPENSBI
    misa_string(misa_strBuf, sizeof(misa_strBuf));
#else
    misa_strBuf[0] = '?';
    misa_strBuf[1] = '\0';
#endif
    mHSS_DEBUG_PRINTF("Vendor: %u, Arch: %u, MISA: %s" CRLF CRLF, CSR_GetVendorID(), CSR_GetArchID(), misa_strBuf);

    mHSS_DEBUG_NORMAL_TEXT;

    HSS_DDRInit(); // we may need DDR early
#ifdef CONFIG_TINYCLI
    HSS_TinyCLI_Parser();
#endif

    size_t i = 0u;

    for (i = 0u; i < spanOfInitFunctions; i++) {
        assert(initFunctions[i].handler);

        // mHSS_DEBUG_PRINTF("\tRunning %d of %d: %s()" CRLF, i, spanOfInitFunctions, initFunctions[i].pName);
        bool result = (initFunctions[i].handler)();

        if (!result) {
            mHSS_DEBUG_PRINTF("\t%s() returned %d" CRLF, initFunctions[i].pName, result);

            // TODO: What should we do on a failed init???
        }
    }

    mHSS_DEBUG_PRINTF("Init finished..." CRLF CRLF);
}
