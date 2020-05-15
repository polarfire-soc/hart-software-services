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
#  include "hss_tinycli.h"
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

#include "hss_memcpy_via_pdma.h"

/**
 * \brief Main Initialization Function
 *
 * All other initialization routines to be chained off this...
 */

/****************************************************************************/

#include <string.h>

#define mMEM_SIZE(REGION)      (REGION##_END - REGION##_START + 1u)
#if 1
#define E51_DTIM_START         0x01000000u
#define E51_DTIM_END           0x01001FFFu

#define E51_ITIM_START         0x01800000u
#define E51_ITIM_END           0x01801FFFu

#define U54_1_ITIM_START       0x01808000u
#define U54_1_ITIM_END         0x0180EFFFu

#define U54_2_ITIM_START       0x01810000
#define U54_2_ITIM_END         0x01816FFFu

#define U54_3_ITIM_START       0x01818000u
#define U54_3_ITIM_END         0x0181EFFFu

#define U54_4_ITIM_START       0x01820000u
#define U54_4_ITIM_END         0x01826FFFu

#define L2LIM_START            0x08000000u
#define L2LIM_END              0x09FFFFFFu

#define L2_ZERO_DEVICE_START   0x0A000000u
#define L2_ZERO_DEVICE_END     0x0BFFFFFFu

#define DDR_START              0x801F0000u
#define DDR_END                0x100000000llu
#else
extern uint64_t __e51dtim_start,    __e51dtim_end;
extern uint64_t __e51itim_start,    __e51itim_end;
extern uint64_t __u54_1_itim_start, __u54_1_itim_end;
extern uint64_t __u54_2_itim_start, __u54_2_itim_end;
extern uint64_t __u54_3_itim_start, __u54_3_itim_end;
extern uint64_t __u54_4_itim_start, __u54_4_itim_end;
extern uint64_t __l2lim_start,      __l2lim_end;
extern uint64_t __ddr_start,        __ddr_end;

#define E51_DTIM_START         (&__e51dtim_start)         
#define E51_DTIM_END           (&__e51dtim_end)

#define E51_ITIM_START         (&__e51itim_start)         
#define E51_ITIM_END           (&__e51itim_end)

#define U54_1_ITIM_START       (&__u54_1_itim_start)         
#define U54_1_ITIM_END         (&__u54_1_itim_end)

#define U54_2_ITIM_START       (&__u54_2_itim_start)         
#define U54_2_ITIM_END         (&__u54_2_itim_end)

#define U54_3_ITIM_START       (&__u54_3_itim_start)         
#define U54_3_ITIM_END         (&__u54_3_itim_end)

#define U54_4_ITIM_START       (&__u54_4_itim_start)         
#define U54_4_ITIM_END         (&__u54_4_itim_end)

#define L2LIM_START            (&__l2lim_start)         
#define L2LIM_END              (&__l2lim_end)

#define L2_ZERO_DEVICE_START   0x0A000000u
#define L2_ZERO_DEVICE_END     0x0BFFFFFFu

#define DDR_START              (&__ddr_start)
#define DDR_END                (&__ddr_end)
#endif

#ifdef CONFIG_PLATFORM_MPFS
#  include "mss_sysreg.h"
#endif

#define CHUNK_SIZE             0x2000u

bool HSS_ZeroDDR(void)
{
#ifdef CONFIG_INITIALIZE_MEMORIES
#  ifdef CONFIG_USE_ZERO_DEVICE
    uint8_t *pChar = (uint8_t *)DDR_START;

    while (pChar < (uint8_t *)DDR_END) {
        memcpy_via_pdma(pChar, (void *)L2_ZERO_DEVICE_START, CHUNK_SIZE); 
        pChar += CHUNK_SIZE; 
    }
#  else
    uint64_t volatile *pDWord = (uint64_t volatile *)DDR_START;

    while (pDWord < (uint64_t volatile *)DDR_END) {
        *pDWord = 0llu;
        pDWord++;
    }
#  endif
#endif

    return true;
}

/* Init memories.. */
bool HSS_ZeroTIMs(void)
{
#ifdef CONFIG_INITIALIZE_MEMORIES
#  ifdef CONFIG_USE_ZERO_DEVICE
    memcpy_via_pdma((void *)E51_DTIM_START,   (void *)L2_ZERO_DEVICE_START, mMEM_SIZE(E51_DTIM));
    memcpy_via_pdma((void *)E51_ITIM_START,   (void *)L2_ZERO_DEVICE_START, mMEM_SIZE(E51_ITIM));
    memcpy_via_pdma((void *)U54_1_ITIM_START, (void *)L2_ZERO_DEVICE_START, mMEM_SIZE(U54_1_ITIM));
    memcpy_via_pdma((void *)U54_2_ITIM_START, (void *)L2_ZERO_DEVICE_START, mMEM_SIZE(U54_2_ITIM));
    memcpy_via_pdma((void *)U54_3_ITIM_START, (void *)L2_ZERO_DEVICE_START, mMEM_SIZE(U54_3_ITIM));
    memcpy_via_pdma((void *)U54_4_ITIM_START, (void *)L2_ZERO_DEVICE_START, mMEM_SIZE(U54_4_ITIM));
#  else
    memset((void*)E51_DTIM_START, 0, E51_DTIM_END - E51_DTIM_START);       /* 8KiB */
    memset((void*)U54_1_ITIM_START, 0, U54_1_ITIM_END - U54_1_ITIM_START); /* 28KiB */
    memset((void*)U54_2_ITIM_START, 0, U54_2_ITIM_END - U54_2_ITIM_START); /* 28KiB */
    memset((void*)U54_3_ITIM_START, 0, U54_3_ITIM_END - U54_3_ITIM_START); /* 28KiB */
    memset((void*)U54_4_ITIM_START, 0, U54_4_ITIM_END - U54_4_ITIM_START); /* 28KiB */
#  endif
#endif

    mb();
    mb_i();

    return true;
}

bool HSS_Init_Setup_RWDATA_And_BSS(void)
{
    //UART not setup at this point
    //mHSS_DEBUG_PRINTF("Setting up RW Data and BSS sections" CRLF);

#ifdef CONFIG_PLATFORM_MPFS
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
#endif

    return true;
}

#ifdef CONFIG_USE_GNU_BUILD_ID
void HSS_PrintBuildId(void);
void HSS_PrintBuildId(void)
{
    extern const struct {
        uint32_t namesz;
        uint32_t descsz;
        uint32_t type;
        uint8_t data[];
    } gnu_build_id;

    const size_t padding = sizeof(uint32_t) - 1u;
    const size_t offset = (gnu_build_id.namesz + padding) & ~padding;
    const uint8_t *pBuildId = (const uint8_t *)&(gnu_build_id.data[offset]);

    mHSS_FANCY_PRINTF("Build ID: ");

    for (int i = 0; i < gnu_build_id.descsz; ++i) {
        mHSS_FANCY_PRINTF_EX("%02x", pBuildId[i]);
    }

    mHSS_FANCY_PRINTF_EX(CRLF);
}
#endif

bool HSS_E51_Banner(void)
{
    mHSS_FANCY_STATUS_TEXT;

    mHSS_FANCY_PRINTF("PolarFire SoC Hart Software Services (HSS) - Version %d.%d.%d" CRLF, 
        HSS_VERSION_MAJOR, HSS_VERSION_MINOR, HSS_VERSION_PATCH);
    mHSS_FANCY_PRINTF("(c) Copyright 2017-2020 Microchip Corporation." CRLF CRLF);

#ifdef CONFIG_USE_GNU_BUILD_ID
    HSS_PrintBuildId();
#endif

    mHSS_FANCY_NORMAL_TEXT;

    return true;
}

/****************************************************************************/

void HSS_Init(void)
{
    RunInitFunctions(spanOfGlobalInitFunctions, globalInitFunctions);
}
