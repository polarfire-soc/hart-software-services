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
#include "mss_pll.h"

#include <assert.h>

#include <sbi/sbi_types.h>

#include <libfdt.h>
#include <sbi/riscv_atomic.h>
#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#include <sbi/riscv_io.h>
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
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_timer.h>
#include <sbi_utils/fdt/fdt_fixup.h>
#include <sbi_utils/ipi/aclint_mswi.h>
#include <sbi/sbi_irqchip.h>
#include <sbi_utils/irqchip/plic.h>
#include <sbi_utils/timer/aclint_mtimer.h>

#include "opensbi_service.h"
#include "opensbi_ecall.h"

#include "mpfs_reg_map.h"

#include "reboot_service.h"
#include "hss_boot_service.h"
#include "clocks/hw_mss_clks.h"    // LIBERO_SETTING_MSS_RTC_TOGGLE_CLK
#include "memory_map/hw_apb_split.h"
#include "hss_trigger.h"
#include <u54_state.h>
#include "hss_clock.h"
#include "mss_sysreg.h"

#define MPFS_HART_STACK_SIZE       8192

#define MPFS_CLINT_ADDR            0x2000000

#define MPFS_PLIC_ADDR             0xc000000
#define MPFS_PLIC_NUM_SOURCES      186
#define MPFS_PLIC_NUM_PRIORITIES   7

/* PLIC memory map offsets — mirrors the file-scoped defines in plic.c */
#define PLIC_CONTEXT_BASE          0x200000
#define PLIC_CONTEXT_STRIDE        0x1000

#define MPFS_ACLINT_MTIMER_FREQ    LIBERO_SETTING_MSS_RTC_TOGGLE_CLK
#define MPFS_ACLINT_MTIMER_ADDR    (0x02004000)

#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_CLOCK_SCALING)
#define MPFS_MMUART0_LO_BASE       0x20000000UL
#define MPFS_MMUART0_HI_BASE       0x28000000UL
#if (LIBERO_SETTING_APBBUS_CR & 0x1UL)
#  define MPFS_MMUART0_BASE        MPFS_MMUART0_HI_BASE
#else
#  define MPFS_MMUART0_BASE        MPFS_MMUART0_LO_BASE
#endif

#define MPFS_MMUART_DLR_OFFSET     0x00UL
#define MPFS_MMUART_DMR_OFFSET     0x04UL
#define MPFS_MMUART_LCR_OFFSET     0x0cUL
#define MPFS_MMUART_LSR_OFFSET     0x14UL
#define MPFS_MMUART_MM0_OFFSET     0x30UL
#define MPFS_MMUART_DFR_OFFSET     0x3cUL

#define MPFS_MMUART_LCR_DLAB       (1u << 7u)
#define MPFS_MMUART_LSR_TEMT       (1u << 6u)
#define MPFS_MMUART_MM0_EFBR       (1u << 7u)
#define MPFS_MMUART_DFR_MASK       0x3fU

#define MPFS_MMUART0_SUSPEND_CLOCK_DIVISOR 2UL

#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_SCB_CLOCK_SWITCH)
#define MPFS_SCB_CLOCK_HZ          80000000UL
#define MPFS_IOSCB_PLL_MSS_PLL_CTRL ((volatile void *)0x3e001004UL)
#define MPFS_IOSCB_MSS_MUX_MSSCLKMUX ((volatile void *)0x3e00200cUL)
#define MPFS_MSSCLKMUX_SEL_MASK    0x3UL
#define MPFS_MSS_PLL_CTRL_STATUS_MASK \
    (PLL_CTRL_LOCK_BIT | (1UL << 28U) | (1UL << 29U) | (1UL << 31U))
#define MPFS_MSS_PLL_LOCK_TIMEOUT  1000000UL
#define MPFS_CLOCK_MUX_SETTLE_LOOPS 100U
#endif
#endif

#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_DDR_POWER_SAVE)
#define MPFS_DDR_LOW_POWER_OPTIONS 0x1fU
/* MC_BASE2.CFG_DRAM_CLK_DISABLE_IN_SELF_REFRESH is at DDRC offset 0x4310. */
#define MPFS_DDRC_CFG_DRAM_CLK_DISABLE_SR_OFFSET 0x4310U
#endif

/**
 * PolarFire SoC has 5 HARTs but HART ID 0 doesn't have S mode. enable only
 * HARTs 1 to 4.
 */
#ifndef MPFS_ENABLED_HART_MASK
#  define MPFS_ENABLED_HART_MASK    (1 << 1 | 1 << 2 | 1 << 3 | 1 << 4)
#endif

/*
 * Static PLIC data with context_map storage.
 *
 * In v1.8, plic_data uses a flexible array member context_map[][2] that must
 * be sized for the number of harts.  We use a wrapper struct so the flexible
 * array extends into _ctx[] which provides the actual storage.
 *
 * MPFS PLIC context layout (9 contexts total):
 *   Hart 0 (E51):   M-context=0, no S-mode
 *   Hart 1 (U54_1): M-context=1, S-context=2
 *   Hart 2 (U54_2): M-context=3, S-context=4
 *   Hart 3 (U54_3): M-context=5, S-context=6
 *   Hart 4 (U54_4): M-context=7, S-context=8
 */
static struct {
    struct plic_data plic;
    s16 _ctx[MPFS_HART_COUNT][2];
} _plicInfo = {
    .plic.addr = MPFS_PLIC_ADDR,
    .plic.size = 0x4000000,
    .plic.num_src = MPFS_PLIC_NUM_SOURCES,
    .plic.flags = PLIC_FLAG_NO_PRIORITY_INIT,
    ._ctx = {
        { 0, -1},   /* Hart 0 (E51): M=0, no S-mode */
        { 1,  2},   /* Hart 1 (U54_1) */
        { 3,  4},   /* Hart 2 (U54_2) */
        { 5,  6},   /* Hart 3 (U54_3) */
        { 7,  8},   /* Hart 4 (U54_4) */
    },
};
#define plicInfo (_plicInfo.plic)

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
    .has_64bit_mmio = true
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
    bool has_stopped;
#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_DDR_POWER_SAVE)
    bool parked;
#endif
} hart_ledger[MAX_NUM_HARTS] = { { { 0, }, } };

static size_t num_sbi_domains = 0u;

/* plic_set_ie() and plic_set_thresh() have their static qualifier commented
 * out in plic.c to allow external use; declare them here at file scope. */
extern void plic_set_ie(const struct plic_data *plic, u32 cntxid, u32 word_index, u32 val);
extern void plic_set_thresh(const struct plic_data *plic, u32 cntxid, u32 val);

#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_CLOCK_SCALING)
static uint8_t mpfs_mmuart0_read8(unsigned long offset)
{
    return *(volatile uint8_t *)(MPFS_MMUART0_BASE + offset);
}

static void mpfs_mmuart0_write8(unsigned long offset, uint8_t value)
{
    *(volatile uint8_t *)(MPFS_MMUART0_BASE + offset) = value;
}

static void mpfs_mmuart0_wait_tx_empty(void)
{
    while ((mpfs_mmuart0_read8(MPFS_MMUART_LSR_OFFSET) & MPFS_MMUART_LSR_TEMT) == 0u) {
        ;
    }
}

static struct {
    uint8_t dfr;
    uint8_t dlr;
    uint8_t dmr;
    uint8_t lcr;
    uint8_t mm0;
} saved_mmuart0_baudrate;

static void mpfs_mmuart0_save_baudrate(void)
{
    saved_mmuart0_baudrate.lcr = mpfs_mmuart0_read8(MPFS_MMUART_LCR_OFFSET);
    saved_mmuart0_baudrate.mm0 = mpfs_mmuart0_read8(MPFS_MMUART_MM0_OFFSET);
    saved_mmuart0_baudrate.dfr = mpfs_mmuart0_read8(MPFS_MMUART_DFR_OFFSET);

    mpfs_mmuart0_write8(MPFS_MMUART_LCR_OFFSET,
        saved_mmuart0_baudrate.lcr | MPFS_MMUART_LCR_DLAB);
    saved_mmuart0_baudrate.dmr = mpfs_mmuart0_read8(MPFS_MMUART_DMR_OFFSET);
    saved_mmuart0_baudrate.dlr = mpfs_mmuart0_read8(MPFS_MMUART_DLR_OFFSET);
    mpfs_mmuart0_write8(MPFS_MMUART_LCR_OFFSET, saved_mmuart0_baudrate.lcr);
}

static void mpfs_mmuart0_restore_baudrate(void)
{
    uint8_t lcr = mpfs_mmuart0_read8(MPFS_MMUART_LCR_OFFSET);

    mpfs_mmuart0_write8(MPFS_MMUART_LCR_OFFSET, lcr | MPFS_MMUART_LCR_DLAB);
    mpfs_mmuart0_write8(MPFS_MMUART_DMR_OFFSET, saved_mmuart0_baudrate.dmr);
    mpfs_mmuart0_write8(MPFS_MMUART_DLR_OFFSET, saved_mmuart0_baudrate.dlr);
    mpfs_mmuart0_write8(MPFS_MMUART_LCR_OFFSET, saved_mmuart0_baudrate.lcr);

    mpfs_mmuart0_write8(MPFS_MMUART_DFR_OFFSET, saved_mmuart0_baudrate.dfr);
    mpfs_mmuart0_write8(MPFS_MMUART_MM0_OFFSET, saved_mmuart0_baudrate.mm0);
}

static void mpfs_mmuart0_scale_baudrate(uint64_t old_pclk_hz,
    uint64_t new_pclk_hz)
{
    const uint32_t old_divisor =
        ((uint32_t)saved_mmuart0_baudrate.dmr << 8u)
        | saved_mmuart0_baudrate.dlr;
    const uint8_t lcr = mpfs_mmuart0_read8(MPFS_MMUART_LCR_OFFSET);
    uint64_t divisor_by_64;
    uint64_t new_divisor_by_64;
    uint32_t new_divisor;
    uint32_t fractional_divisor;

    if (old_pclk_hz == 0u || new_pclk_hz == 0u || old_divisor == 0u) {
        return;
    }

    divisor_by_64 = (uint64_t)old_divisor * 64u;
    if ((saved_mmuart0_baudrate.mm0 & MPFS_MMUART_MM0_EFBR) != 0u) {
        divisor_by_64 += saved_mmuart0_baudrate.dfr
            & MPFS_MMUART_DFR_MASK;
    }

    new_divisor_by_64 = (divisor_by_64 * new_pclk_hz
        + (old_pclk_hz / 2u)) / old_pclk_hz;
    new_divisor = (uint32_t)(new_divisor_by_64 / 64u);
    fractional_divisor = (uint32_t)(new_divisor_by_64 % 64u);

    if (new_divisor == 0u || new_divisor > 0xffffu) {
        return;
    }

    mpfs_mmuart0_write8(MPFS_MMUART_LCR_OFFSET, lcr | MPFS_MMUART_LCR_DLAB);
    mpfs_mmuart0_write8(MPFS_MMUART_DMR_OFFSET,
        (uint8_t)(new_divisor >> 8u));
    mpfs_mmuart0_write8(MPFS_MMUART_DLR_OFFSET, (uint8_t)new_divisor);
    mpfs_mmuart0_write8(MPFS_MMUART_LCR_OFFSET, lcr);

    if (new_divisor > 1u) {
        mpfs_mmuart0_write8(MPFS_MMUART_MM0_OFFSET,
            mpfs_mmuart0_read8(MPFS_MMUART_MM0_OFFSET) | MPFS_MMUART_MM0_EFBR);
        mpfs_mmuart0_write8(MPFS_MMUART_DFR_OFFSET,
            (uint8_t)fractional_divisor);
    } else {
        mpfs_mmuart0_write8(MPFS_MMUART_MM0_OFFSET,
            mpfs_mmuart0_read8(MPFS_MMUART_MM0_OFFSET) & (uint8_t)~MPFS_MMUART_MM0_EFBR);
    }
}

#  if !IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_SCB_CLOCK_SWITCH)
static void mpfs_mmuart0_set_suspend_baudrate(void)
{
    mpfs_mmuart0_scale_baudrate(LIBERO_SETTING_MSS_APB_AHB_CLK,
        LIBERO_SETTING_MSS_APB_AHB_CLK / MPFS_MMUART0_SUSPEND_CLOCK_DIVISOR);
}
#  endif

#  if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_SCB_CLOCK_SWITCH)
static uint32_t saved_mssclkmux;
static uint32_t saved_mss_pll_ctrl;
static bool scb_clock_state_saved = false;

static void mpfs_clock_mux_settle(void)
{
    volatile uint32_t delay;

    for (delay = 0u; delay < MPFS_CLOCK_MUX_SETTLE_LOOPS; delay++) {
        ;
    }
}

static void mpfs_mmuart0_set_scb_baudrate(void)
{
    mpfs_mmuart0_scale_baudrate(LIBERO_SETTING_MSS_APB_AHB_CLK,
        MPFS_SCB_CLOCK_HZ / 8UL);
}

static uint32_t mpfs_mss_pll_ctrl_for_write(uint32_t pll_ctrl)
{
    return pll_ctrl & ~MPFS_MSS_PLL_CTRL_STATUS_MASK;
}

static void mpfs_suspend_scb_clock(void)
{
    if (scb_clock_state_saved) {
        return;
    }

    saved_mssclkmux = readl(MPFS_IOSCB_MSS_MUX_MSSCLKMUX);
    saved_mss_pll_ctrl = readl(MPFS_IOSCB_PLL_MSS_PLL_CTRL);
    scb_clock_state_saved = true;

    mss_freq_scaling(MSS_CLK_SCALING_LOW);
    writel(saved_mssclkmux & ~MPFS_MSSCLKMUX_SEL_MASK,
        MPFS_IOSCB_MSS_MUX_MSSCLKMUX);
    mb();
    mpfs_clock_mux_settle();

    writel(mpfs_mss_pll_ctrl_for_write(saved_mss_pll_ctrl &
        ~PLL_CTRL_REG_DIVQ0_EN_MASK), MPFS_IOSCB_PLL_MSS_PLL_CTRL);
    mb();
}

static bool mpfs_mss_pll_is_locked(void)
{
    uint32_t timeout;

    for (timeout = 0u; timeout < MPFS_MSS_PLL_LOCK_TIMEOUT; timeout++) {
        if ((readl(MPFS_IOSCB_PLL_MSS_PLL_CTRL) &
            PLL_CTRL_LOCK_BIT) != 0u) {
            return true;
        }
    }

    return false;
}

static void mpfs_resume_scb_clock(void)
{
    if (!scb_clock_state_saved) {
        return;
    }

    writel(mpfs_mss_pll_ctrl_for_write(saved_mss_pll_ctrl),
        MPFS_IOSCB_PLL_MSS_PLL_CTRL);
    mb();
    if (!mpfs_mss_pll_is_locked()) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "MSS PLL failed to lock\n");
        assert(0);
        while (1) {
            wfi();
        }
    }

    writel(saved_mssclkmux, MPFS_IOSCB_MSS_MUX_MSSCLKMUX);
    mb();
    mpfs_clock_mux_settle();
    mss_freq_scaling(MSS_CLK_SCALING_NORMAL);

    scb_clock_state_saved = false;
}
#  endif
#endif

static void mpfs_modify_dt(void *fdt)
{
    fdt_cpu_fixup(fdt);
    fdt_fixups(fdt);
    fdt_reserved_memory_fixup(fdt);
}

static void __attribute__((__noreturn__)) mpfs_system_reset(u32 reset_type, u32 reset_reason)
{
    const u32 hartid = current_hartid();
    struct sbi_scratch * const scratch = sbi_hartid_to_scratch(hartid);

    hart_ledger[hartid].boot_pending = 1;
    hart_ledger[hartid].reset_reason = reset_reason;
    hart_ledger[hartid].reset_type = reset_type;

    /* re-enable IPIs */
    csr_set(CSR_MSTATUS, MSTATUS_MIE);
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

static int mpfs_ipi_cold_init(void);
static struct sbi_system_suspend_device mpfs_suspend;

static int mpfs_early_init(bool cold_boot)
{
    if (cold_boot) {
        sbi_system_reset_add_device(&mpfs_reset);
        sbi_system_suspend_set_device(&mpfs_suspend);
        mpfs_console_init();
        mpfs_ipi_cold_init();
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

void mpfs_uart_surrender(void)
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
    if (uart_getchar(&rcvBuf, NO_BLOCK, false)) {
        result = rcvBuf;
    }

    return result;
}

static struct sbi_console_device mpfs_console = {
    .name = "mmuart",
    .console_putc = mpfs_console_putc,
    .console_getc = mpfs_console_getc,
};

void mpfs_console_init(void)
{
    console_initialized = true;
    sbi_console_set_device(&mpfs_console);
}

/*
 * Simple PLIC init for E51 — zeros all source priorities.
 * No OpenSBI framework dependency (no scratch, heap, or domain needed).
 */
bool HSS_PLIC_Init(void);
bool HSS_PLIC_Init(void)
{
    volatile char *base = (volatile char *)plicInfo.addr;

    for (int src = 1; src <= plicInfo.num_src; src++)
        writel(0, base + 4 * src);

    return true;
}




/* PLIC register layout constants */
#define MPFS_PLIC_ENABLE_BASE     0x2000
#define MPFS_PLIC_ENABLE_STRIDE   0x80
#define MPFS_PLIC_CONTEXT_BASE    0x200000
#define MPFS_PLIC_CONTEXT_STRIDE  0x1000

/*
 * Custom PLIC warm init for MPFS.
 *
 * Only touches S-mode context — leaves M-mode context alone to avoid
 * interference with IHC which uses M-mode external interrupts.
 */
static int mpfs_plic_warm_init(struct sbi_irqchip_device *dev)
{
    (void)dev;
    const struct plic_data *plic = &plicInfo;
    u32 hartindex = current_hartindex();
    s16 s_cntx_id = plic->context_map[hartindex][PLIC_S_CONTEXT];

    if (s_cntx_id < 0)
        return 0;

    u32 ie_words = plic->num_src / 32 + 1;
    volatile char *base = (volatile char *)plic->addr;

    /* Disable all S-mode IRQ enables for this hart */
    for (u32 i = 0; i < ie_words; i++) {
        writel(0, base + MPFS_PLIC_ENABLE_BASE +
               MPFS_PLIC_ENABLE_STRIDE * s_cntx_id + 4 * i);
    }

    /* Set S-mode priority threshold to max (effectively disables) */
    writel(0x7, base + MPFS_PLIC_CONTEXT_BASE +
           MPFS_PLIC_CONTEXT_STRIDE * s_cntx_id);

    return 0;
}

static int mpfs_irqchip_init(void)
{
#if 0
    /* Original v1.8 approach — calls plic_cold_irqchip_init which may
     * clobber M-mode PLIC state that IHC depends on. Disabled to test
     * whether reverting to old behaviour fixes the mcause=0 trap. */
    int rc;

    rc = plic_cold_irqchip_init(&plicInfo);
    if (rc)
        return rc;

    /* Override with custom warm_init that only touches S-mode context,
     * leaving M-mode alone for IHC.
     */
    plicInfo.irqchip.warm_init = mpfs_plic_warm_init;
    return 0;
#else
    /*
     * Old approach: global PLIC priority zeroing is done once from
     * HSS_PLIC_Init() during E51 early init.  We do NOT call
     * plic_cold_irqchip_init() here to avoid AMP interference.
     * Only register a custom irqchip device whose warm_init touches
     * S-mode context exclusively.
     */
    int rc;

    rc = sbi_domain_root_add_memrange(plicInfo.addr, plicInfo.size, BIT(20),
                    (SBI_DOMAIN_MEMREGION_MMIO |
                     SBI_DOMAIN_MEMREGION_SHARED_SURW_MRW));
    if (rc)
        return rc;

    plicInfo.irqchip.warm_init = mpfs_plic_warm_init;
    sbi_irqchip_add_device(&plicInfo.irqchip);

    return 0;
#endif
}

static int mpfs_ipi_cold_init(void)
{
    return aclint_mswi_cold_init(&mswi);
}

static int mpfs_timer_init(void)
{
    return aclint_mtimer_cold_init(&mtimer, NULL);
}

static void mpfs_final_exit(void)
{
    /* re-enable IPIs */
    csr_set(CSR_MSTATUS, MSTATUS_MIE);
    csr_write(CSR_MIE, MIP_MSIP);
}


#define MPFS_TLB_RANGE_FLUSH_LIMIT 0u
static u64 mpfs_get_tlbr_flush_limit(void)
{
    return MPFS_TLB_RANGE_FLUSH_LIMIT;
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

size_t mpfs_domains_get_count(void)
{
    return num_sbi_domains;
}

void mpfs_domains_register_hart(int hartid, int boot_hartid)
{
    hart_ledger[hartid].owner_hartid = boot_hartid;
    hart_ledger[hartid].boot_pending = 1;

    hart_ledger[hartid].reset_reason = 0;
    hart_ledger[hartid].reset_type = 0;
    __atomic_store_n(&hart_ledger[hartid].has_stopped, false, __ATOMIC_RELAXED);
#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_DDR_POWER_SAVE)
    __atomic_store_n(&hart_ledger[hartid].parked, false, __ATOMIC_RELAXED);
#endif
}

void mpfs_domains_deregister_hart(int hartid)
{
    hart_ledger[hartid].owner_hartid = 0;
    hart_ledger[hartid].boot_pending = 0;

    assert((hartid > 0) && (hartid < ARRAY_SIZE(mpfs_hart_index2id)));
    mpfs_hart_index2id[hartid] = -1;
}

bool mpfs_is_hart_using_opensbi(int hartid)
{
    bool result = true;

    assert((hartid > 0) && (hartid < ARRAY_SIZE(mpfs_hart_index2id)));

    if (mpfs_hart_index2id[hartid] == -1) {
        result = false;
    }

    return result;
}

void mpfs_mark_hart_as_booted(int hartid)
{
    assert((hartid >= 0) && (hartid < ARRAY_SIZE(hart_ledger)));

    if (hartid < ARRAY_SIZE(hart_ledger)) {
        hart_ledger[hartid].boot_pending = 0;
    }
}

bool mpfs_are_harts_in_same_domain(int hartid1, int hartid2)
{
    bool result = false;

    assert((hartid1 >= 0) && (hartid1 < ARRAY_SIZE(hart_ledger)));
    assert((hartid2 >= 0) && (hartid2 < ARRAY_SIZE(hart_ledger)));

    result = (hart_ledger[hartid1].owner_hartid == hart_ledger[hartid2].owner_hartid);

    return result;
}

bool mpfs_is_cold_reboot_allowed(int hartid)
{
    assert((hartid >= 0) && (hartid < ARRAY_SIZE(hart_ledger)));
    return hart_ledger[hartid].allow_cold_reboot;
}

bool mpfs_is_warm_reboot_allowed(int hartid)
{
    assert((hartid >= 0) && (hartid < ARRAY_SIZE(hart_ledger)));
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

            if (!pDom->index) { // not yet registered for this boot hart
                pDom->boot_hartid = boot_hartid;

                memcpy(pDom->name, hart_ledger[boot_hartid].name, ARRAY_SIZE(dom_table[0].name)-1);

                struct sbi_hartmask * const pMask = &(hart_ledger[boot_hartid].hartMask);
                struct sbi_scratch * const pScratch = sbi_scratch_thishart_ptr();

                pDom->regions = mpfs_domains_root_regions();
                sbi_domain_memregion_init(pScratch->fw_start, pScratch->fw_size, 0u, &(pDom->regions[0]));
                pDom->fw_region_inited = true;

                pDom->next_arg1 = hart_ledger[boot_hartid].next_arg1;
                pDom->next_addr = hart_ledger[boot_hartid].next_addr;
                pDom->next_mode = hart_ledger[boot_hartid].next_mode;
                pDom->system_reset_allowed = true;
                pDom->system_suspend_allowed = true;
                pDom->possible_harts = pMask;

                result = sbi_domain_register(pDom, pMask);
                if (result) {
                    sbi_printf("%s(): sbi_domain_register() failed for %s\n", __func__, pDom->name);
                    break;
                } else {
                    num_sbi_domains++;
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
    (void)saddr;

#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_DDR_POWER_SAVE)
    __atomic_store_n(&hart_ledger[hartid].parked, false, __ATOMIC_RELEASE);
#endif

    if (hart_ledger[hartid].owner_hartid != hartid &&
            __atomic_load_n(&hart_ledger[hartid].has_stopped,
                __ATOMIC_ACQUIRE)) {
        /*
         * Secondary hart took j _start on a previous HART_STOP and is now
         * sitting in HSS's IPI loop.  Ask E51 to send IPI_MSG_GOTO to it.
         *
         * sbi_hsm_hart_start() already moved state STOPPED -> START_PENDING
         * before calling us.  Complete the transition (START_PENDING ->
         * STARTED) and release the start ticket so future HART_START calls
         * can acquire it.  The hart will not go through OpenSBI's warmboot
         * path so sbi_hsm_hart_start_finish() is never called on the target.
         */
        __atomic_store_n(&hart_ledger[hartid].has_stopped, false,
            __ATOMIC_RELEASE);
        struct sbi_scratch *rscratch = sbi_hartid_to_scratch(hartid);
        sbi_hsm_hart_start_complete_for(rscratch);

        return HSS_Boot_SendResumeGOTO((enum HSSHartId)hartid,
            rscratch->next_addr, rscratch->next_arg1) ? SBI_OK : SBI_ERR_FAILED;
    }

    /*
     * Hart is in sbi_hsm_hart_wait() WFI loop (either the boot hart
     * parked by Linux, or a secondary hart on its first Linux start).
     * Wake it with a raw software IPI; OpenSBI init_warm_startup() will
     * call sbi_hsm_hart_start_finish() and jump to next_addr.
     */
    return sbi_ipi_raw_send(sbi_hartid_to_hartindex(hartid), true);
}

static int mpfs_hart_stop(void)
{
    const u32 hartid = current_hartid();
    struct sbi_scratch *scratch = sbi_scratch_thishart_ptr();
    void (*jump_warmboot)(void) = (void (*)(void))scratch->warmboot_addr;

    /* re-enable IPIs */
    csr_set(CSR_MSTATUS, MSTATUS_MIE);
    csr_write(CSR_MIE, MIP_MSIP);

    if (hart_ledger[hartid].owner_hartid == hartid && hart_ledger[hartid].boot_pending) {
        /*
         * Reached via mpfs_system_reset() (SBI_EXT_SRST path): boot_pending
         * is set only from mpfs_system_reset().  Re-enter HSS so the hart
         * is back in the SSMB loop and available to the E51 for the next
         * boot.  On PolarFire SoC there is no actual hardware power-off, so
         * shutdown and reboot are treated identically here.
         */
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
#  endif
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

        /* Re-enter HSS: hart rejoins the SSMB loop, ready for E51 */
        asm("j _start");
        __builtin_unreachable();
    }

    if (hart_ledger[hartid].owner_hartid != hartid) {
        /*
         * Secondary hart stopped via plain SBI_EXT_HSM_HART_STOP.
         * Re-enter HSS so E51 can coordinate its relaunch - either as
         * part of a system reboot (E51 will send IPI_MSG_GOTO to the
         * payload) or after system suspend resume (mpfs_hart_start() will
         * ask E51 to send IPI_MSG_GOTO with the Linux resume address).
         *
         * Using j _start here instead of jump_warmboot() is necessary
         * because E51 communicates via SSMB, not via raw software IPIs,
         * so the hart must be in HSS IPI processing loop to receive it.
         */
#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_DDR_POWER_SAVE)
        __atomic_store_n(&hart_ledger[hartid].parked, false, __ATOMIC_RELEASE);
#endif
        __atomic_store_n(&hart_ledger[hartid].has_stopped, true,
            __ATOMIC_RELEASE);
        asm("j _start");
        __builtin_unreachable();
    }

    /*
     * Boot hart, plain SBI_EXT_HSM_HART_STOP (parked by Linux):
     * use the warmboot path so it blocks in sbi_hsm_hart_wait().
     * mpfs_hart_start() wakes it with sbi_ipi_raw_send().
     */
    jump_warmboot();
    __builtin_unreachable();
}

static atomic_t coldboot_lottery = ATOMIC_INITIALIZER(0);

bool mpfs_is_first_boot(void);
bool mpfs_is_first_boot(void)
{
    return (atomic_xchg(&coldboot_lottery, 1) == 0);
}

#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_DDR_POWER_SAVE)
void mpfs_mark_hart_parked(void);
void mpfs_mark_hart_parked(void)
{
    const u32 hartid = current_hartid();

    if (hartid == HSS_HART_E51 || hartid >= ARRAY_SIZE(hart_ledger)) {
        return;
    }

    __atomic_store_n(&hart_ledger[hartid].parked, true, __ATOMIC_RELEASE);
}

bool mpfs_is_hart_parked(u32 hartid)
{
    assert(hartid < ARRAY_SIZE(hart_ledger));

    return __atomic_load_n(&hart_ledger[hartid].parked, __ATOMIC_ACQUIRE);
}
#endif

static uint32_t suspended_hartid = 0u;

void mpfs_set_suspended_hartid(uint32_t hartid)
{
    __atomic_store_n(&suspended_hartid, hartid, __ATOMIC_RELEASE);
}

u32 mpfs_get_suspended_hartid(void)
{
    return __atomic_load_n(&suspended_hartid, __ATOMIC_ACQUIRE);
}

extern void mpfs_hal_turn_ddr_selfrefresh_on(void);
extern void mpfs_hal_turn_ddr_selfrefresh_off(void);
#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_DDR_POWER_SAVE)
extern void mpfs_hal_ddr_logic_power_state(uint32_t, uint32_t);
extern void flush_l2_cache(uint32_t);
#endif

#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_DDR_POWER_SAVE)
static uint32_t mpfs_ddr_sr_read32(uint32_t offset)
{
    return *(volatile uint32_t *)((uintptr_t)MSS_DDRC_BASE_ADDR + offset);
}

static void mpfs_ddr_sr_write32(uint32_t offset, uint32_t value)
{
    *(volatile uint32_t *)((uintptr_t)MSS_DDRC_BASE_ADDR + offset) = value;
}

static uint32_t saved_ddr_dram_clk_disable_sr;

static void mpfs_ddr_enable_clk_disable_in_sr(void)
{
    saved_ddr_dram_clk_disable_sr =
        mpfs_ddr_sr_read32(MPFS_DDRC_CFG_DRAM_CLK_DISABLE_SR_OFFSET);
    mpfs_ddr_sr_write32(MPFS_DDRC_CFG_DRAM_CLK_DISABLE_SR_OFFSET, 1U);
    mb();
}

static void mpfs_ddr_restore_clk_disable_in_sr(void)
{
    mpfs_ddr_sr_write32(MPFS_DDRC_CFG_DRAM_CLK_DISABLE_SR_OFFSET,
        saved_ddr_dram_clk_disable_sr);
    mb();
}

static uint32_t saved_ddrc_clock_cr;
static bool ddrc_clock_cr_saved = false;

static void mpfs_suspend_ddrc_clock(void)
{
    if (ddrc_clock_cr_saved) {
        return;
    }

    saved_ddrc_clock_cr = SYSREG->SUBBLK_CLOCK_CR;
    ddrc_clock_cr_saved = true;

    SYSREG->SUBBLK_CLOCK_CR =
        saved_ddrc_clock_cr & ~SUBBLK_CLOCK_CR_DDRC_MASK;
    mb();
}

static void mpfs_resume_ddrc_clock(void)
{
    if (!ddrc_clock_cr_saved) {
        return;
    }

    SYSREG->SUBBLK_CLOCK_CR = saved_ddrc_clock_cr;
    mb();

    ddrc_clock_cr_saved = false;
}
#endif

void mpfs_system_suspend(void)
{
    if (!IS_ENABLED(CONFIG_SKIP_DDR)) {
        volatile uint32_t * const self_refresh_status_reg =
            (volatile uint32_t *)(MSS_DDRC_BASE_ADDR + MSS_DDRC_SELF_REFRESH_STATUS_OFFSET);
#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_DDR_POWER_SAVE)
        flush_l2_cache((uint32_t)1U);
        mpfs_ddr_enable_clk_disable_in_sr();
#endif

        mpfs_hal_turn_ddr_selfrefresh_on();
        while ((*self_refresh_status_reg & MSS_DDRC_SELF_REFRESH_ACK_BIT) == 0u) {
            ; // poll INIT_SELF_REFRESH_STATUS bit until DDRC ACKs entry
        }
#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_DDR_POWER_SAVE)
        mpfs_hal_ddr_logic_power_state(0, MPFS_DDR_LOW_POWER_OPTIONS);
        mpfs_suspend_ddrc_clock();
#endif
        mHSS_DEBUG_PRINTF(LOG_WARN, "%s: self_refresh active\n", __func__);
    }
#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_CLOCK_SCALING)
    mpfs_mmuart0_wait_tx_empty();
    mpfs_mmuart0_save_baudrate();
#  if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_SCB_CLOCK_SWITCH)
    mpfs_suspend_scb_clock();
    mpfs_mmuart0_set_scb_baudrate();
#  else
    mss_freq_scaling(MSS_CLK_SCALING_LOW);
    mpfs_mmuart0_set_suspend_baudrate();
#  endif
#endif
}

void mpfs_system_resume(void)
{
    if (!IS_ENABLED(CONFIG_SKIP_DDR)) {
        volatile uint32_t * const self_refresh_status_reg =
            (volatile uint32_t *)(MSS_DDRC_BASE_ADDR + MSS_DDRC_SELF_REFRESH_STATUS_OFFSET);
#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_DDR_POWER_SAVE)
        mpfs_resume_ddrc_clock();
        mpfs_hal_ddr_logic_power_state(1, MPFS_DDR_LOW_POWER_OPTIONS);
#endif
        mpfs_hal_turn_ddr_selfrefresh_off();
        while ((*self_refresh_status_reg & MSS_DDRC_SELF_REFRESH_ACK_BIT) != 0u) {
            ;
        }
#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_DDR_POWER_SAVE)
        mpfs_ddr_restore_clk_disable_in_sr();
        flush_l2_cache((uint32_t)1U);
#endif
        mHSS_DEBUG_PRINTF(LOG_WARN, "%s: self_refresh inactive\n", __func__);
    }
#if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_CLOCK_SCALING)
    mpfs_mmuart0_wait_tx_empty();
#  if IS_ENABLED(CONFIG_SERVICE_OPENSBI_SUSPEND_SCB_CLOCK_SWITCH)
    mpfs_resume_scb_clock();
#  else
    mss_freq_scaling(MSS_CLK_SCALING_NORMAL);
#  endif
    mpfs_mmuart0_restore_baudrate();
#endif
}

/*****************************************************************************
 * System Suspend Device (SUSP extension)
 *
 * Implements retentive suspend: DDR in self-refresh, boot hart WFIs
 * until E51's TinyCLI RESUME command fires the trigger + MSIP.
 */

static int mpfs_system_suspend_check(u32 sleep_type)
{
    if (sleep_type == SBI_SUSP_SLEEP_TYPE_SUSPEND)
        return SBI_OK;
    return SBI_ERR_NOT_SUPPORTED;
}

static int mpfs_do_system_suspend(u32 sleep_type, unsigned long mmode_resume_addr)
{
    (void)sleep_type;
    (void)mmode_resume_addr;

    mpfs_set_suspended_hartid(current_hartid());

    if (1 == mpfs_domains_get_count()) {
        /* Wait for all secondary harts to reach Idle before DDR self-refresh */
        const struct sbi_domain *dom = sbi_domain_thishart_ptr();
        unsigned long i;
        sbi_hartmask_for_each_hartindex(i, dom->possible_harts) {
            unsigned int hid = sbi_hartindex_to_hartid(i);
            if (hid == current_hartid())
                continue;
            while (HSS_U54_GetState_Ex((int)hid) != HSS_State_Idle) {
                ;
            }
        }

        mpfs_system_suspend();

        mHSS_DEBUG_PRINTF_EX(
            "__        __    _ _   _                 __\n"
            "\\ \\      / /_ _(_) |_(_)_ __   __ _    / _| ___  _ __\n"
            " \\ \\ /\\ / / _` | | __| | '_ \\ / _` |  | |_ / _ \\| '__|\n"
            "  \\ V  V / (_| | | |_| | | | | (_| |  |  _| (_) | |\n"
            "   \\_/\\_/ \\__,_|_|\\__|_|_| |_|\\__, |  |_|  \\___/|_|\n"
            "                              |___/\n"
            "                                           _                   _\n"
            " _ __ ___  ___ _   _ _ __ ___   ___    ___(_) __ _ _ __   __ _| |\n"
            "| '__/ _ \\/ __| | | | '_ ` _ \\ / _ \\  / __| |/ _` | '_ \\ / _` | |\n"
            "| | |  __/\\__ \\ |_| | | | | | |  __/  \\__ \\ | (_| | | | | (_| | |\n"
            "|_|  \\___||___/\\__,_|_| |_| |_|\\___|  |___/_|\\__, |_| |_|\\__,_|_|\n"
            "                                            |___/\n"
            "\n");

        HSS_Trigger_Clear(EVENT_SYSTEM_SUSPEND_RESUME);
        while (!HSS_Trigger_IsNotified(EVENT_SYSTEM_SUSPEND_RESUME)) {
            wfi();
        }

        /* Clear our own MSIP so it doesn't fire as a spurious M-mode
         * interrupt when sbi_hart_switch_mode() re-enables interrupts. */
        volatile uint32_t * const msip =
            (volatile uint32_t *)((uintptr_t)CLINT_BASE_ADDR + 4u * current_hartid());
        *msip = (uint32_t)0u;

        mpfs_system_resume();

        HSS_Trigger_Clear(EVENT_SYSTEM_SUSPEND_RESUME);
    }
    /* else: AMP — can't put DDR in self-refresh; come straight back out */

    return SBI_OK;
}

static struct sbi_system_suspend_device mpfs_suspend = {
    .name = "mpfs_suspend",
    .system_suspend_check = mpfs_system_suspend_check,
    .system_suspend = mpfs_do_system_suspend,
};

const struct sbi_hsm_device mpfs_hsm = {
    .name = "mpfs_hsm",
    .hart_start = mpfs_hart_start,
    .hart_stop = mpfs_hart_stop,
    .hart_suspend = NULL
};

static bool mpfs_cold_boot_allowed(u32 hartid)
{
    (void)hartid;
    return mpfs_is_last_hart_ready();
}

static bool mpfs_single_fw_region(void)
{
    return true;
}

const struct sbi_platform_operations platform_ops = {
    .cold_boot_allowed = mpfs_cold_boot_allowed,
    .single_fw_region = mpfs_single_fw_region,

    .early_init = mpfs_early_init,
    .final_init = mpfs_final_init,
    .early_exit = NULL,
    .final_exit = mpfs_final_exit,

    .misa_check_extension = NULL,
    .misa_get_xlen = NULL,

    .irqchip_init = mpfs_irqchip_init,

    .get_tlbr_flush_limit = mpfs_get_tlbr_flush_limit,

    .timer_init = mpfs_timer_init,

    .domains_init = mpfs_domains_init,

    .vendor_ext_provider = HSS_SBI_ECALL_Handler
};

const struct sbi_platform platform = {
    .opensbi_version = OPENSBI_VERSION,
    .platform_version = SBI_PLATFORM_VERSION(0x0, 0x3),
    .name = "Microchip PolarFire(R) SoC",
    .features = SBI_PLATFORM_DEFAULT_FEATURES,
    .hart_count = MPFS_HART_COUNT,
    .hart_stack_size = SBI_PLATFORM_DEFAULT_HART_STACK_SIZE,
    .heap_size = SBI_PLATFORM_DEFAULT_HEAP_SIZE(MPFS_HART_COUNT),
    .platform_ops_addr = (unsigned long)&platform_ops,
    .firmware_context = 0,
    .hart_index2id = mpfs_hart_index2id,
    .cbom_block_size = 0
};
