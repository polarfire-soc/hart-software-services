
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
#include <sbi/riscv_atomic.h>
#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_const.h>
#include <sbi/sbi_ipi.h>
#include <sbi/sbi_init.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_hsm.h>
#include <sbi/sbi_hartmask.h>
#include <sbi/sbi_domain.h>
#include <sbi/sbi_math.h>
#include <sbi/sbi_system.h>
#include <sbi/sbi_timer.h>
#include <sbi_utils/fdt/fdt_fixup.h>
#include <sbi_utils/ipi/aclint_mswi.h>
#include <sbi_utils/irqchip/plic.h>
#include <sbi_utils/timer/aclint_mtimer.h>

#include "opensbi_service.h"
#include "opensbi_ecall.h"

#include "mpfs_reg_map.h"

#include "reboot_service.h"
#include "clocks/hw_mss_clks.h"    // LIBERO_SETTING_MSS_RTC_TOGGLE_CLK

#define MPFS_HART_COUNT            5
#define MPFS_HART_STACK_SIZE       8192

#define MPFS_CLINT_ADDR            0x2000000

#define MPFS_PLIC_ADDR             0xc000000
#define MPFS_PLIC_NUM_SOURCES      186
#define MPFS_PLIC_NUM_PRIORITIES   7

#define MPFS_ACLINT_MTIMER_FREQ    LIBERO_SETTING_MSS_RTC_TOGGLE_CLK
#define MPFS_ACLINT_MTIMER_ADDR    (0x02004000)

/**
 * PolarFire SoC has 5 HARTs but HART ID 0 doesn't have S mode. enable only
 * HARTs 1 to 4.
 */
#ifndef MPFS_ENABLED_HART_MASK
#  define MPFS_ENABLED_HART_MASK    (1 << 1 | 1 << 2 | 1 << 3 | 1 << 4)
#endif

#define MPFS_HARITD_DISABLED            ~(MPFS_ENABLED_HART_MASK)

static struct plic_data plicInfo = {
    .addr = MPFS_PLIC_ADDR,
    .num_src = MPFS_PLIC_NUM_SOURCES
};

static struct aclint_mswi_data mswi = {
    .addr = MPFS_CLINT_ADDR,
    .size = ACLINT_MSWI_SIZE,
    .first_hartid = 0,
    .hart_count = MPFS_HART_COUNT,
};

static struct aclint_mtimer_data mtimer = {
    .mtime_freq = MPFS_ACLINT_MTIMER_FREQ,
    .mtime_addr = MPFS_ACLINT_MTIMER_ADDR + ACLINT_DEFAULT_MTIME_OFFSET,
    .mtime_size = ACLINT_DEFAULT_MTIME_SIZE,
    .mtimecmp_addr = MPFS_ACLINT_MTIMER_ADDR + ACLINT_DEFAULT_MTIMECMP_OFFSET,
    .mtimecmp_size = ACLINT_DEFAULT_MTIMECMP_SIZE,
    .first_hartid = 0,
    .hart_count = MPFS_HART_COUNT,
    .has_64bit_mmio = TRUE
};

static struct {
    char name[64];
    u64 next_addr;
    u64 next_arg1;
    struct sbi_hartmask hartMask;
    u32 next_mode;
    int owner_hartid;
    int boot_pending;
    int reset_type;
    int reset_reason;
    bool allow_cold_reboot;
    bool allow_warm_reboot;
} hart_ledger[MAX_NUM_HARTS] = { { { 0, }, } };

extern unsigned long STACK_SIZE_PER_HART;

static void mpfs_modify_dt(void *fdt)
{
    fdt_cpu_fixup(fdt);
    fdt_fixups(fdt);
    fdt_reserved_memory_nomap_fixup(fdt);
}

static void __attribute__((__noreturn__)) mpfs_system_reset(u32 reset_type, u32 reset_reason)
{
    (void)reset_type;
    (void)reset_reason;

    const u32 hartid = current_hartid();
    struct sbi_scratch * const scratch = sbi_hartid_to_scratch(hartid);

    hart_ledger[hartid].boot_pending = 1;
    hart_ledger[hartid].reset_reason = reset_reason;
    hart_ledger[hartid].reset_type = reset_type;

    /* re-enable IPIs */
    csr_write(CSR_MSTATUS, MIP_MSIP);
    csr_write(CSR_MIE, MIP_MSIP);

    sbi_exit(scratch);

    __builtin_unreachable(); // never reached
}

static int mpfs_system_reset_check(u32 reset_type, u32 reset_reason)
{
    int result;

    switch (reset_type) {
    default:
        result = 0;
        break;

    case SBI_SRST_RESET_TYPE_SHUTDOWN:
        __attribute__((fallthrough)); // deliberate fallthrough
    case SBI_SRST_RESET_TYPE_COLD_REBOOT:
        __attribute__((fallthrough)); // deliberate fallthrough
    case SBI_SRST_RESET_TYPE_WARM_REBOOT:
        result = 1;
        break;
    }

    return result;
}


static struct sbi_system_reset_device mpfs_reset = {
    .name = "mpfs_reset",
    .system_reset_check = mpfs_system_reset_check,
    .system_reset = mpfs_system_reset,
};

static int mpfs_early_init(bool cold_boot)
{
    if (cold_boot) {
        sbi_system_reset_add_device(&mpfs_reset);
    }

    return 0;
}

static int mpfs_final_init(bool cold_boot)
{
    if (!cold_boot) {
        return 0;
    }

    void *fdt = sbi_scratch_thishart_arg1_ptr();
    if (fdt) {
        mpfs_modify_dt(fdt);
    }

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

static struct sbi_console_device mpfs_console = {
    .name = "mmuart",
    .console_putc = mpfs_console_putc,
    .console_getc = mpfs_console_getc,
};

static int mpfs_console_init(void)
{
    console_initialized = true;
    sbi_console_set_device(&mpfs_console);
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
        // instead of calling plic_warm_irqchip_init ...
        //
        //     rc = plic_warm_irqchip_init(&plicInfo,
        //         (hartid) ? (2 * hartid - 1) : 0, (hartid) ? (2 * hartid) : -1);
        //
        // we'll do it ourselves to customize behavior..
        //const int m_cntx_id =  (hartid) ? (2 * hartid - 1) : 0;
        const int s_cntx_id =  (hartid) ? (2 * hartid) : -1;
        struct plic_data * const plic = &plicInfo;
        size_t i, ie_words;

        if (!plic) {
                return SBI_EINVAL;
        } else {
            ie_words = plic->num_src / 32 + 1;

            // By default, disable all IRQs for M-mode of target HART
            // need to reconsider this with IHC...
            //
            //if (m_cntx_id > -1) {
            //    for (i = 0; i < ie_words; i++) {
            //        plic_set_ie(plic, m_cntx_id, i, 0);
            //    }
            //}

            /* By default, disable all IRQs for S-mode of target HART */
            if (s_cntx_id > -1) {
                for (i = 0; i < ie_words; i++) {
                    plic_set_ie(plic, s_cntx_id, i, 0);
                }
            }

            // By default, disable M-mode threshold
            // need to reconsider these with IHC...
            //
            //if (m_cntx_id > -1) {
            //    plic_set_thresh(plic, m_cntx_id, 0x7);
            //}

            /* By default, disable S-mode threshold */
            if (s_cntx_id > -1) {
                plic_set_thresh(plic, s_cntx_id, 0x7);
            }
        }
    }

    return rc;
}

static int mpfs_ipi_init(bool cold_boot)
{
    int result = 0;

    if (cold_boot) {
        result = aclint_mswi_cold_init(&mswi);
    }

    if (!result) {
        result = aclint_mswi_warm_init();
    }

    return result;
}

static int mpfs_timer_init(bool cold_boot)
{
    int result = 0;

    if (cold_boot) {
        result = aclint_mtimer_cold_init(&mtimer, NULL);
    }

    if (!result) {
        aclint_mtimer_warm_init();
    }

    return result;

}

static void mpfs_final_exit(void)
{
    /* re-enable IPIs */
    csr_write(CSR_MSTATUS, MIP_MSIP);
    csr_write(CSR_MIE, MIP_MSIP);
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

__extension__ static u32 mpfs_hart_index2id[MPFS_HART_COUNT] = {
    [0] = -1,
    [1] = 1,
    [2] = 2,
    [3] = 3,
    [4] = 4,
};

void mpfs_domains_register_hart(int hartid, int boot_hartid)
{
    hart_ledger[hartid].owner_hartid = boot_hartid;
    hart_ledger[hartid].boot_pending = 1;

    hart_ledger[hartid].reset_reason = 0;
    hart_ledger[hartid].reset_type = 0;
}

void mpfs_domains_deregister_hart(int hartid)
{
    hart_ledger[hartid].owner_hartid = 0;
    hart_ledger[hartid].boot_pending = 0;

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
    assert((hartid >= 0) & (hartid < ARRAY_SIZE(hart_ledger)));

    if (hartid < ARRAY_SIZE(hart_ledger)) {
        hart_ledger[hartid].boot_pending = 0;
    }
}

bool mpfs_are_harts_in_same_domain(int hartid1, int hartid2)
{
    bool result = false;

    assert((hartid1 >= 0) & (hartid1 < ARRAY_SIZE(hart_ledger)));
    assert((hartid2 >= 0) & (hartid2 < ARRAY_SIZE(hart_ledger)));

    result = (hart_ledger[hartid1].owner_hartid == hart_ledger[hartid2].owner_hartid);

    return result;
}

bool mpfs_is_cold_reboot_allowed(int hartid)
{
    assert((hartid >= 0) & (hartid < ARRAY_SIZE(hart_ledger)));
    return hart_ledger[hartid].allow_cold_reboot;
}

bool mpfs_is_warm_reboot_allowed(int hartid)
{
    assert((hartid >= 0) & (hartid < ARRAY_SIZE(hart_ledger)));
    return hart_ledger[hartid].allow_warm_reboot;
}

bool mpfs_is_last_hart_ready(void)
{
    bool result;

    int outstanding = 0;
    for (int hartid = 0; hartid < ARRAY_SIZE(hart_ledger); hartid++) {
        outstanding += hart_ledger[hartid].boot_pending;
    }

    result =  (outstanding == 0);
    return result;
}

void mpfs_domains_register_boot_hart(char *pName, u32 hartMask, int boot_hartid, u32 privMode,
     void * entryPoint, void * pArg1, bool allow_cold_reboot, bool allow_warm_reboot)
{
    assert(hart_ledger[boot_hartid].owner_hartid == boot_hartid);

    memcpy(hart_ledger[boot_hartid].name, pName, ARRAY_SIZE(hart_ledger[boot_hartid].name) - 1);
    hart_ledger[boot_hartid].next_addr = (u64)entryPoint;
    hart_ledger[boot_hartid].next_arg1 = (u64)pArg1;
    hart_ledger[boot_hartid].hartMask.bits[0] = hartMask;
    hart_ledger[boot_hartid].next_mode = privMode;
    hart_ledger[boot_hartid].allow_cold_reboot = allow_cold_reboot;
    hart_ledger[boot_hartid].allow_warm_reboot = allow_warm_reboot;
}

static struct sbi_domain dom_table[MAX_NUM_HARTS] = { 0 };
static int mpfs_domains_init(void)
{
    // register all AMP domains
    int result = SBI_EINVAL;
    for (int hartid = 1; hartid < ARRAY_SIZE(hart_ledger); hartid++) {
        const int boot_hartid = hart_ledger[hartid].owner_hartid;

        if (boot_hartid) {
            struct sbi_domain * const pDom = &dom_table[boot_hartid];

            if (!pDom->index) { // { pDom->boot_hartid != boot_hartid) {
                pDom->boot_hartid = boot_hartid;

                memcpy(pDom->name, hart_ledger[boot_hartid].name, ARRAY_SIZE(dom_table[0].name)-1);

                struct sbi_hartmask * const pMask = &(hart_ledger[boot_hartid].hartMask);
                struct sbi_scratch * const pScratch = sbi_scratch_thishart_ptr();

                pDom->regions = mpfs_domains_root_regions();
                pDom->regions[0].order = log2roundup(pScratch->fw_size);
                pDom->regions[0].base = pScratch->fw_start & ~((1UL << pDom->regions[0].order) - 1UL);
                pDom->regions[0].flags = 0u;

                pDom->next_arg1 = hart_ledger[boot_hartid].next_arg1;
                pDom->next_addr = hart_ledger[boot_hartid].next_addr;
                pDom->next_mode = hart_ledger[boot_hartid].next_mode;
                pDom->system_reset_allowed = TRUE;
                pDom->possible_harts = pMask;

                result = sbi_domain_register(pDom, pMask);
                if (result) {
                    sbi_printf("%s(): sbi_domain_register() failed for %s\n", __func__, pDom->name);
                    break;
                }
            }
        } else {
           //sbi_printf("%s(): boot_hart_id not set for u54_%d\n", __func__, hartid);
        }
    }

    return result;
}

static int mpfs_hart_start(u32 hartid, ulong saddr)
{
    (void)hartid;
    (void)saddr;

    return 0;
}

static int mpfs_hart_stop(void)
{
    const u32 hartid = current_hartid();
    /* re-enable IPIs */
    csr_write(CSR_MSTATUS, MIP_MSIP);
    csr_write(CSR_MIE, MIP_MSIP);

    if (hart_ledger[hartid].owner_hartid == hartid) {
        switch (hart_ledger[hartid].reset_reason) {
        case SBI_SRST_RESET_REASON_SYSFAIL:
            mHSS_DEBUG_PRINTF(LOG_ERROR, "u54_%d reported SYSTEM FAILURE\n", hartid);
            break;

        case SBI_SRST_RESET_REASON_NONE:
            __attribute__((fallthrough)); // deliberate fallthrough
        default:
            break;
        }

        switch(hart_ledger[hartid].reset_type) {
        case SBI_SRST_RESET_TYPE_SHUTDOWN:
            break;

#if IS_ENABLED(CONFIG_ALLOW_COLDREBOOT)
        case SBI_SRST_RESET_TYPE_COLD_REBOOT:
            if (IS_ENABLED(CONFIG_ALLOW_COLDREBOOT_ALWAYS) || hart_ledger[hartid].allow_cold_reboot) {
#  if IS_ENABLED(CONFIG_SERVICE_REBOOT)
                HSS_reboot_cold(HSS_HART_ALL);
#endif
            } else {
                mHSS_DEBUG_PRINTF(LOG_ERROR, "u54_%d not permitted to cold reboot\n", hartid);
            }
            __attribute__((fallthrough)); // deliberate fallthrough
#endif
        case SBI_SRST_RESET_TYPE_WARM_REBOOT:
            __attribute__((fallthrough)); // deliberate fallthrough
        default:
            HSS_OpenSBI_Reboot();
            break;
        }
    }

    asm("j _start");

    // never reached
    __builtin_unreachable();
}

static atomic_t coldboot_lottery = ATOMIC_INITIALIZER(0);

bool mpfs_is_first_boot(void);
bool mpfs_is_first_boot(void)
{
    return (atomic_xchg(&coldboot_lottery, 1) == 0);
}

const struct sbi_hsm_device mpfs_hsm = {
    .name = "mpfs_hsm",
    .hart_start = mpfs_hart_start,
    .hart_stop = mpfs_hart_stop,
    .hart_suspend = NULL
};

const struct sbi_platform_operations platform_ops = {
    .early_init = mpfs_early_init,
    .final_init = mpfs_final_init,
    .early_exit = NULL,
    .final_exit = mpfs_final_exit,

    .misa_check_extension = NULL,
    .misa_get_xlen = NULL,

    .console_init = mpfs_console_init,

    .ipi_init = mpfs_ipi_init,
    .irqchip_init = mpfs_irqchip_init,
    .irqchip_exit = NULL,

    .get_tlbr_flush_limit = mpfs_get_tlbr_flush_limit,

    .timer_init = mpfs_timer_init,
    .timer_exit = NULL,

    .domains_init = mpfs_domains_init,

    .vendor_ext_check = HSS_SBI_Vendor_Ext_Check,
    .vendor_ext_provider = HSS_SBI_ECALL_Handler
};

const struct sbi_platform platform = {
    .opensbi_version = OPENSBI_VERSION,
    .platform_version = SBI_PLATFORM_VERSION(0x0, 0x2),
    .name = "Microchip PolarFire(R) SoC",
    .features = SBI_PLATFORM_DEFAULT_FEATURES, // already have PMPs setup
    .hart_count = MPFS_HART_COUNT,
    .hart_index2id = mpfs_hart_index2id,
    .hart_stack_size = SBI_PLATFORM_DEFAULT_HART_STACK_SIZE,
    .platform_ops_addr = (unsigned long)&platform_ops,
    .firmware_context = 0
};
