
/******************************************************************************************
 *
 * MPFS HSS Embedded Software
 *
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Originally based on code from OpenSBI, which is:
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 */

#include "config.h"
#include "hss_types.h"

#include <assert.h>

#include <sbi/sbi_types.h>
#define false FALSE
#define true TRUE

#include <libfdt.h>
#include <sbi/riscv_asm.h>
#include <sbi/riscv_io.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_const.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_hartmask.h>
#include <sbi/sbi_domain.h>
#include <sbi/sbi_math.h>
#include <sbi_utils/fdt/fdt_fixup.h>
#include <sbi_utils/irqchip/plic.h>
#include <sbi_utils/serial/uart8250.h>
#include <sbi_utils/sys/clint.h>

#include "opensbi_service.h"
#include "opensbi_ecall.h"


#define MPFS_HART_COUNT            5
#define MPFS_HART_STACK_SIZE       8192

#define MPFS_SYS_CLK               1000000000

#define MPFS_CLINT_ADDR            0x2000000

#define MPFS_PLIC_ADDR             0xc000000
#define MPFS_PLIC_NUM_SOURCES      186
#define MPFS_PLIC_NUM_PRIORITIES   7

#define MPFS_UART0_ADDR            0x10010000
#define MPFS_UART1_ADDR            0x10011000
#define MPFS_UART_BAUDRATE         115200

/**
 * PolarFire SoC has 5 HARTs but HART ID 0 doesn't have S mode. enable only
 * HARTs 1 to 4.
 */
#ifndef MPFS_ENABLED_HART_MASK
#  define MPFS_ENABLED_HART_MASK    (1 << 1 | 1 << 2 | 1 << 3 | 1 << 4)
#endif

#define MPFS_HARITD_DISABLED            ~(MPFS_ENABLED_HART_MASK)

struct plic_data plicInfo = {
    .addr = MPFS_PLIC_ADDR,
    .num_src = MPFS_PLIC_NUM_SOURCES
};

struct clint_data clintInfo = {
    .addr = MPFS_CLINT_ADDR,
    .first_hartid = 0,
    .hart_count = MPFS_HART_COUNT,
    .has_64bit_mmio = TRUE
};

extern unsigned long STACK_SIZE_PER_HART;

static void mpfs_modify_dt(void *fdt)
{
    fdt_cpu_fixup(fdt);

    fdt_fixups(fdt);

    //fdt_reserved_memory_nomap_fixup(fdt); // not needed for PolarFire SoC
}

static int mpfs_final_init(bool cold_boot)
{
    void *fdt;

    if (!cold_boot) {
        return 0;
    }

    fdt = sbi_scratch_thishart_arg1_ptr();
    mpfs_modify_dt(fdt);

    return 0;
}

static bool console_initialized = false;

#if IS_ENABLED(CONFIG_UART_SURRENDER)
static bool uart_surrendered_flag = false;

void uart_surrender(void);
void uart_surrender(void)
{
    uart_surrendered_flag = true;
}
#endif

static void mpfs_console_putc(char ch)
{
    if (console_initialized) {
        u32 hartid = current_hartid();

#if IS_ENABLED(CONFIG_UART_SURRENDER)
        if (hartid || !uart_surrendered_flag) {
#else
        {
#endif
            int uart_putc(int hartid, const char ch); //TBD
            uart_putc(hartid, ch);
        }
    }
}

#define NO_BLOCK 0
#define GETC_EOF -1

static int mpfs_console_getc(void)
{
    int result = GETC_EOF;
    bool uart_getchar(uint8_t *pbuf, int32_t timeout_sec, bool do_sec_tick);

    uint8_t rcvBuf;
    if (uart_getchar(&rcvBuf, NO_BLOCK, FALSE)) {
        result = rcvBuf;
    }

    return result;
}

static int mpfs_console_init(void)
{
    console_initialized = true;
    return 0;
}


static int mpfs_irqchip_init(bool cold_boot)
{
    int rc = 0;
    u32 hartid = current_hartid();

    if (cold_boot) {
        rc = plic_cold_irqchip_init(&plicInfo);
    }

    if (!rc) {
        rc = plic_warm_irqchip_init(&plicInfo,
            (hartid) ? (2 * hartid - 1) : 0, (hartid) ? (2 * hartid) : -1);
    }

    return rc;
}

static int mpfs_ipi_init(bool cold_boot)
{
    int rc = 0;

    if (cold_boot) {
        rc = clint_cold_ipi_init(&clintInfo);
    }

    if (!rc) {
        rc = clint_warm_ipi_init();
    }

    return rc;
}

static int mpfs_timer_init(bool cold_boot)
{
    int rc = 0;

    if (cold_boot) {
        rc = clint_cold_timer_init(&clintInfo, NULL);
    }

    if (!rc) {
        clint_warm_timer_init();
    }

    return rc;

}

static void mpfs_system_down(u32 reset_type, u32 reset_reason)
{
    /* For now nothing to do, we'll instead rely on
     * mpfs_final_exit() kicking off the restart... */
    (void)reset_type;
    (void)reset_reason;

    /* re-enable IPIs */
    csr_write(CSR_MSTATUS, MIP_MSIP);
    csr_write(CSR_MIE, MIP_MSIP);

    HSS_OpenSBI_Reboot();

    while (1);
}

static void mpfs_final_exit(void)
{
    /* re-enable IPIs */
    csr_write(CSR_MSTATUS, MIP_MSIP);
    csr_write(CSR_MIE, MIP_MSIP);
    mpfs_system_down(0, 0);
}


#define MPFS_TLB_RANGE_FLUSH_LIMIT 0u
static u64 mpfs_get_tlbr_flush_limit(void)
{
    return MPFS_TLB_RANGE_FLUSH_LIMIT;
}

// don't allow OpenSBI to play with PMPs
int sbi_hart_pmp_configure(struct sbi_scratch *pScratch)
{
    (void)pScratch;
    return 0;
}

static struct sbi_domain_memregion mpfs_memregion[3] = {
    { .order = 0, .base = 0u, .flags = 0u },
    { .order = __riscv_xlen, .base = 0u, .flags =
        (SBI_DOMAIN_MEMREGION_READABLE | SBI_DOMAIN_MEMREGION_WRITEABLE | SBI_DOMAIN_MEMREGION_EXECUTABLE) },
    { .order = 0u, .base = 0u, .flags = 0u }
};

static struct sbi_domain_memregion * mpfs_domains_root_regions(void)
{
    return mpfs_memregion;
}

static struct {
    char name[64];
    u64 next_addr;
    u64 next_arg1;
    struct sbi_hartmask hartMask;
    u32 next_mode;
    int owner_hartid;
    int boot_pending;
} hart_table[MAX_NUM_HARTS] = { 0 };

u32 mpfs_hart_index2id[MPFS_HART_COUNT] = {
    [0] = -1,
    [1] = 1,
    [2] = 2,
    [3] = 3,
    [4] = 4,
};

void mpfs_domains_register_hart(int hartid, int boot_hartid)
{
    hart_table[hartid].owner_hartid = boot_hartid;
    hart_table[hartid].boot_pending = 1;
}

void mpfs_domains_deregister_hart(int hartid)
{
    hart_table[hartid].owner_hartid = 0;
    hart_table[hartid].boot_pending = 0;

    assert((hartid > 0) & (hartid < ARRAY_SIZE(mpfs_hart_index2id)));
    mpfs_hart_index2id[hartid] = -1;
}

bool mpfs_is_hart_using_opensbi(int hartid)
{
    bool result = true;

    assert((hartid > 0) & (hartid < ARRAY_SIZE(mpfs_hart_index2id)));

    if (mpfs_hart_index2id[hartid] == -1) {
        result = false;
    }

    return result;
}

void mpfs_mark_hart_as_booted(int hartid)
{
    assert(hartid < ARRAY_SIZE(hart_table));
    assert((hartid >= 0) & (hartid < ARRAY_SIZE(hart_table)));

    if (hartid < ARRAY_SIZE(hart_table)) {
        hart_table[hartid].boot_pending = 0;
    }
}

bool mpfs_is_last_hart_booting(void)
{
    int outstanding = 0;
    for (int hartid = 0; hartid < ARRAY_SIZE(hart_table); hartid++) {
        outstanding += hart_table[hartid].boot_pending;
    }

    if (outstanding > 1) {
        return false;
    } else {
        return true;
    }
}

void mpfs_domains_register_boot_hart(char *pName, u32 hartMask, int boot_hartid, u32 privMode,
     void * entryPoint, void * pArg1)
{
    assert(hart_table[boot_hartid].owner_hartid == boot_hartid);

    memcpy(hart_table[boot_hartid].name, pName, ARRAY_SIZE(hart_table[boot_hartid].name) - 1);
    hart_table[boot_hartid].next_addr = (u64)entryPoint;
    hart_table[boot_hartid].next_arg1 = (u64)pArg1;
    hart_table[boot_hartid].hartMask.bits[0] = hartMask;
    hart_table[boot_hartid].next_mode = privMode;
}

static struct sbi_domain dom_table[MAX_NUM_HARTS] = { 0 };
static int mpfs_domains_init(void)
{
    // register all AMP domains
    int result = SBI_EINVAL;
    for (int hartid = 1; hartid < ARRAY_SIZE(hart_table); hartid++) {
        const int boot_hartid = hart_table[hartid].owner_hartid;

        if (boot_hartid) {
            struct sbi_domain * const pDom = &dom_table[boot_hartid];

            if (!pDom->index) { // { pDom->boot_hartid != boot_hartid) {
                pDom->boot_hartid = boot_hartid;

                memcpy(pDom->name, hart_table[boot_hartid].name, ARRAY_SIZE(dom_table[0].name)-1);

                struct sbi_hartmask * const pMask = &(hart_table[boot_hartid].hartMask);
                struct sbi_scratch * const pScratch = sbi_scratch_thishart_ptr();

                pDom->regions = mpfs_domains_root_regions();
                pDom->regions[0].order = log2roundup(pScratch->fw_size);
                pDom->regions[0].base = pScratch->fw_start & ~((1UL << pDom->regions[0].order) - 1UL);
                pDom->regions[0].flags = 0u;

                pDom->next_arg1 = hart_table[boot_hartid].next_arg1;
                pDom->next_addr = hart_table[boot_hartid].next_addr;
                pDom->next_mode = hart_table[boot_hartid].next_mode;
                pDom->system_reset_allowed = TRUE;
                pDom->possible_harts = pMask;

                result = sbi_domain_register(pDom, pMask);
                if (result) {
                    sbi_printf("%s(): sbi_domain_register() failed for %s\n", __func__, pDom->name);
                    break;
                }
            }
        } else {
           //sbi_printf("%s(): boot_hart_id not set for hart %d\n", __func__, hartid);
        }
    }

    return result;
}


const struct sbi_platform_operations platform_ops = {
    .early_init = NULL,
    .final_init = mpfs_final_init,
    .early_exit = NULL,
    .final_exit = mpfs_final_exit,

    .misa_check_extension = NULL,
    .misa_get_xlen = NULL,

    .console_putc = mpfs_console_putc,
    .console_getc = mpfs_console_getc,
    .console_init = mpfs_console_init,

    .irqchip_init = mpfs_irqchip_init,
    .irqchip_exit = NULL,

    .ipi_send = clint_ipi_send,
    .ipi_clear = clint_ipi_clear,
    .ipi_init = mpfs_ipi_init,
    .ipi_exit = NULL,

    .get_tlbr_flush_limit = mpfs_get_tlbr_flush_limit,

    .timer_value = clint_timer_value,
    .timer_event_start = clint_timer_event_start,
    .timer_event_stop = clint_timer_event_stop,
    .timer_init = mpfs_timer_init,
    .timer_exit = NULL,

    .system_reset = mpfs_system_down,

    //.domains_root_regions = mpfs_domains_root_regions,
    .domains_init = mpfs_domains_init,

    .vendor_ext_check = HSS_SBI_Vendor_Ext_Check,
    .vendor_ext_provider = HSS_SBI_ECALL_Handler
};

const struct sbi_platform platform = {
    .opensbi_version = OPENSBI_VERSION,
    .platform_version = SBI_PLATFORM_VERSION(0x0, 0x01),
    .name = "Microchip PolarFire(R) SoC",
    .features = SBI_PLATFORM_DEFAULT_FEATURES, // already have PMPs setup
    .hart_count = MPFS_HART_COUNT,
    .hart_index2id = mpfs_hart_index2id,
    .hart_stack_size = SBI_PLATFORM_DEFAULT_HART_STACK_SIZE,
    .platform_ops_addr = (unsigned long)&platform_ops,
    .firmware_context = 0
};
