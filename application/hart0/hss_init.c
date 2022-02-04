/*******************************************************************************
 * Copyright 2017-2021 Microchip FPGA Embedded Systems Solutions.
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
#if IS_ENABLED(CONFIG_SERVICE_TINYCLI)
#  include "tinycli_service.h"
#endif

#include "csr_helper.h"

#if IS_ENABLED(CONFIG_SERVICE_BOOT)
#  include "hss_boot_service.h"
#endif

#if IS_ENABLED(CONFIG_OPENSBI)
#  include "sbi/riscv_asm.h"
#  include "sbi/sbi_version.h"
#endif

#include "hss_sys_setup.h"
#include "mpfs_reg_map.h"

#include "hss_memcpy_via_pdma.h"
#include "mpfs_hal_version.h"
#include "miv_ihc_version.h"
#include "mss_sys_services.h"

/**
 * \brief Main Initialization Function
 *
 * All other initialization routines to be chained off this...
 */

/****************************************************************************/

#include <string.h>

#define mMEM_SIZE(REGION)      (REGION##_END - REGION##_START + 1u)
extern const uint64_t __dtim_start,    __dtim_end;
extern const uint64_t __l2lim_start;
extern const uint64_t __ddr_start,        __ddr_end;
extern const uint64_t _hss_start;

#define E51_DTIM_START         (&__dtim_start)
#define E51_DTIM_END           (&__dtim_end)

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
// can't access DDR_END without getting an error:
//     R_RISCV_PCREL_HI20 against symbol `__ddr_end'
// solution is to use assembler instead as the symbol is constant at link time
//
//  #define DDR_END                (&__ddr_end)
asm(".align 3\n"
    "hss_init_ddr_end: .quad (__ddr_end)\n");

extern const uint64_t hss_init_ddr_end;
#define DDR_END                (&hss_init_ddr_end)

#if IS_ENABLED(CONFIG_PLATFORM_MPFS)
#  include "mss_sysreg.h"
#endif

#define CHUNK_SIZE             0x2000u

bool HSS_ZeroDDR(void)
{
#if IS_ENABLED(CONFIG_INITIALIZE_MEMORIES)
    uint64_t volatile *pDWord = (uint64_t volatile *)DDR_START;

    while (pDWord < (uint64_t volatile const * const)DDR_END) {
        *pDWord = 0llu;
        pDWord++;
    }
#endif

    __sync_synchronize();

    return true;
}

/* Init memories.. */
#include "system_startup.h"
bool HSS_ZeroTIMs(void)
{
#if IS_ENABLED(CONFIG_INITIALIZE_MEMORIES)
    memset((void*)E51_DTIM_START, 0, E51_DTIM_END - E51_DTIM_START);       /* 8KiB */
    memset((void*)U54_1_ITIM_START, 0, U54_1_ITIM_END - U54_1_ITIM_START); /* 28KiB */
    memset((void*)U54_2_ITIM_START, 0, U54_2_ITIM_END - U54_2_ITIM_START); /* 28KiB */
    memset((void*)U54_3_ITIM_START, 0, U54_3_ITIM_END - U54_3_ITIM_START); /* 28KiB */
    memset((void*)U54_4_ITIM_START, 0, U54_4_ITIM_END - U54_4_ITIM_START); /* 28KiB */
#endif

    __sync_synchronize();

    return true;
}

bool HSS_Init_RWDATA_BSS(void)
{
    //UART not setup at this point
    //mHSS_DEBUG_PRINTF("Setting up RW Data and BSS sections" CRLF);

#if IS_ENABLED(CONFIG_PLATFORM_MPFS)
    init_memory();
#endif

    return true;
}


#if IS_ENABLED(CONFIG_CC_USE_GNU_BUILD_ID)
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

    mHSS_FANCY_PUTS(LOG_STATUS, "Build ID: ");

    for (int i = 0; i < gnu_build_id.descsz; ++i) {
        mHSS_PRINTF("%02x", pBuildId[i]);
    }

    mHSS_PRINTF(CRLF);
}
#endif

#if IS_ENABLED(CONFIG_DISPLAY_TOOL_VERSIONS)
#include "tool_versions.h"
void HSS_PrintToolVersions(void);
void HSS_PrintToolVersions(void)
{
    mHSS_FANCY_PUTS(LOG_STATUS, "Built with the following tools:" CRLF);

    mHSS_PUTS(" - " CC_VERSION_STRING CRLF);
    mHSS_PUTS(" - " LD_VERSION_STRING CRLF CRLF);
}
#endif

bool HSS_E51_Banner(void)
{
#ifndef VENDOR_STRING
#    define VENDOR_STRING ""
#endif
    mHSS_FANCY_PRINTF(LOG_STATUS,
        "PolarFire(R) SoC Hart Software Services (HSS) - version %d.%d.%d" VENDOR_STRING CRLF
        "MPFS HAL version %d.%d.%d"
#if IS_ENABLED(CONFIG_USE_IHC)
	" / Mi-V IHC version %d.%d.%d"
#endif
        "(c) Copyright 2017-2021 Microchip FPGA Embedded Systems Solutions." CRLF CRLF,
        HSS_VERSION_MAJOR, HSS_VERSION_MINOR, HSS_VERSION_PATCH,
        MPFS_HAL_VERSION_MAJOR, MPFS_HAL_VERSION_MINOR, MPFS_HAL_VERSION_PATCH,
#if IS_ENABLED(CONFIG_USE_IHC)
        MIV_IHC_VERSION_MAJOR, MIV_IHC_VERSION_MINOR, MIV_IHC_VERSION_PATCH
#endif
    )

    mHSS_FANCY_PRINTF(LOG_STATUS, "incorporating OpenSBI - version %d.%d" CRLF
        "(c) Copyright 2019-2021 Western Digital Corporation." CRLF CRLF,
        OPENSBI_VERSION_MAJOR, OPENSBI_VERSION_MINOR);

#if IS_ENABLED(CONFIG_CC_USE_GNU_BUILD_ID)
    HSS_PrintBuildId();
#endif

#if IS_ENABLED(CONFIG_DISPLAY_TOOL_VERSIONS)
    HSS_PrintToolVersions();
#endif

    if (&_hss_start == &__l2lim_start) {
        mHSS_FANCY_PRINTF(LOG_WARN, "NOTICE: Running from L2LIM" CRLF CRLF);
    }

    return true;
}

/****************************************************************************/

void HSS_Init(void)
{
    RunInitFunctions(spanOfGlobalInitFunctions, globalInitFunctions);
    HSS_Boot_RestartCore(HSS_HART_ALL);
#if IS_ENABLED(CONFIG_UART_SURRENDER)
#    if IS_ENABLED(CONFIG_SERVICE_TINYCLI)
    HSS_TinyCLI_SurrenderUART();
#    endif
    void uart_surrender(void);
    uart_surrender();
#endif
}
