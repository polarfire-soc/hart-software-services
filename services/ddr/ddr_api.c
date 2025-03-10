/*******************************************************************************
 * Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file DDR related utilities
 * \brief DDR utilities
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"
#include "hss_state_machine.h"
#include "hss_trigger.h"

#include "ssmb_ipi.h"
#include "hss_init.h"
#include "hss_memtest.h"
#include "ddr_service.h"
#if IS_ENABLED(CONFIG_SERVICE_GPIO_UI)
#  include "gpio_ui_service.h"
#endif

#include "csr_helper.h"
#include <assert.h>

extern const uint64_t __ddr_start;
extern const uint64_t __ddr_end;
extern const uint64_t __ddrhi_startAddr;
extern const uint64_t __ddrhi_end;

/////////////////

//
// We only want to specify the DDR location and size in one place - the linker script
//
// ddr_end is too high for the RV64 toolchain, sadly.  We get an error message concerning
// "relocation truncated to fit: R_RISCV_PCREL_HI20 against symbol `__ddr_end' defined in
//  .text.init section" as it is above 4GiB away from eNVM.
//
// See https://github.com/riscv/riscv-gnu-toolchain/issues/103 for background.
//
// So we can't easily do ...
//
//     extern uint64_t __ddr_end;
//     const uintptr_t ddr_size = (size_t)((char *)&__ddr_end - (char *)&__ddr_start);
//
// However, we can workaround by using the GNU assembler to store the DDR size into a 64-bit memory
// location and use this size in our C code
//


asm("	.globl __ddr_size\n"
    "	.type __ddr_size, @object\n"
    "	.globl __ddrhi_size\n"
    "	.type __ddrhi_size, @object\n"
    "	.globl __ddrhi_size\n"
    "	.type __ddrhi_size, @object\n"
    "	.globl __ddrhi_startAddr\n"
    "	.type __ddrhi_startAddr, @object\n"
    "	.align 3\n"
    "__ddr_size: .quad (__ddr_end-__ddr_start)\n"
    "__ddrhi_startAddr: .quad(__ddrhi_start)\n"
    "__ddrhi_size: .quad (__ddrhi_end-__ddrhi_start)\n"
);
extern const size_t __ddr_size;
extern const size_t __ddrhi_size;

size_t HSS_DDR_GetSize(void)
{
    return __ddr_size;
}

size_t HSS_DDRHi_GetSize(void)
{
    return __ddrhi_size;
}

uintptr_t HSS_DDR_GetStart(void)
{
    return (uintptr_t)&__ddr_start;
}

uintptr_t HSS_DDRHi_GetStart(void)
{
    return (uintptr_t)__ddrhi_startAddr;
}

void HSS_DDR_Train(void)
{
#if IS_ENABLED(CONFIG_SERVICE_GPIO_UI)
    HSS_GPIO_UI_ReportDDRInitStart();
#endif
    IPI_Send(HSS_HART_U54_1, IPI_MSG_DDR_TRAIN, 0u, 0u, NULL, NULL);
}

enum IPIStatusCode HSS_DDR_Train_IPIHandler(TxId_t transaction_id, enum HSSHartId source,
    uint32_t immediate_arg, void *p_extended_buffer_in_ddr, void *p_ancilliary_buffer_in_ddr)
{
    (void)transaction_id;
    (void)source;
    (void)immediate_arg;
    (void)p_extended_buffer_in_ddr;
    (void)p_ancilliary_buffer_in_ddr;

    HSS_DDRInit();
    HSS_ZeroDDR();
#if IS_ENABLED(CONFIG_MEMTEST)
    HSS_MemTestDDRFast();
#endif
    HSS_Trigger_Notify(EVENT_DDR_TRAINED);
#if IS_ENABLED(CONFIG_SERVICE_GPIO_UI)
    HSS_GPIO_UI_ReportDDRInitEnd();
#endif

    return IPI_IDLE;
}

bool HSS_DDR_IsAddrInDDR(uintptr_t addr)
{
    bool result = (addr >= HSS_DDR_GetStart())
        && (addr <= (HSS_DDR_GetStart() + HSS_DDR_GetSize()));

    result |= (addr >= HSS_DDRHi_GetStart())
        && (addr <= (HSS_DDRHi_GetStart() + HSS_DDRHi_GetSize()));

    return result;
}
