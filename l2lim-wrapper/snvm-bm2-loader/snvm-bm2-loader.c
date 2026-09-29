/*******************************************************************************
 * Copyright 2019-2026 Microchip FPGA Embedded Systems Solutions.
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
 *
 * sNVM Boot Mode 2 Loader - main entry point
 *
 * Called by the E51 hart after the CRT has zeroed BSS.  Performs the
 * following steps in order:
 *
 *   1. FEC-decode the SECDED 72/64 encoded l2lim-wrapper image from eNVM
 *      (0x20220100) to L2LIM (0x08000000), correcting single-bit errors.
 *      Halts on uncorrectable (double-bit) errors.
 *
 *   2. Validate the SHA-256 digest of the decoded LIM region against the
 *      build-time-embedded expected value.  Halts on mismatch.
 *
 *   3. Release U54 application harts (mhartid 1..4) by writing 1 to their
 *      CLINT MSIP registers.  Each U54 has already set mtvec = 0x08000000
 *      and is waiting in WFI. The software interrupt trap will send them
 *      directly to l2lim-wrapper _start.
 *
 *   4. Jump the E51 to L2LIM entry (0x08000000 - l2lim-wrapper _start).
 *
 *
 * Memory constants come from the linker script. Digest and size constants
 * come from the generated header build/snvm-bm2-loader/lim_sha256_digest.h
 * which is produced as a build step after l2lim-wrapper.bin is linked.
 */

#include "config.h"
#include <stdint.h>
#include <stddef.h>

#include "snvm-bm2-loader.h"
#include "fec_secded7264_bare.h"
#include "snvm-bm2-loader_sha256.h"
#include "lim_sha256_digest.h"

/* SYSREG->CLOCK_CONFIG_CR lets us read the live APB/AHB divider so we can
 * compute the real boot-time PCLK feeding the UART before the PLL runs. */
#include "mpfs_hal/common/mss_sysreg.h"

/* Hardware base addresses */
#define L2LIM_BASE   UINT32_C(0x08000000)
#define ENVM_BASE    UINT32_C(0x20220100)
#define CLINT_BASE   UINT32_C(0x02000000)

uint64_t __sc_load; // dummy
uint64_t __sc_start; // dummy
uint64_t __sc_end; // dummy

#define MSS_UART_115200_BAUD	115200u
#define MSS_UART_DATA_8_BITS	((uint8_t) 0x03)
#define MSS_UART_NO_PARITY	((uint8_t) 0x00)
#define MSS_UART_ONE_STOP_BIT	((uint8_t) 0x00)

/*
 * Real APB/AHB (PCLK) frequency feeding the UART during this loader.
 *
 * The sNVM BM2 loader runs on the E51 immediately after the system controller
 * releases it. At that point BOTH the MSS PLL and the master clock divider are
 * still at their reset values:
 *   - the MSS is fed from the ~80 MHz standby/SCB clock (see mss_pll.c
 *     "On startup, MSS supplied with 80MHz SCB clock"), and
 *   - SYSREG->CLOCK_CONFIG_CR is at its reset value 0x3F, i.e. the APB/AHB
 *     divider field [5:4] = 0b11 => /8 (see mss_pll.c CLOCK_CONFIG_CR notes).
 * The Libero divider value (0x24 => APB /4) and the 150 MHz PLL are only
 * programmed later by mss_nwc_init()->mss_pll_config(), which runs in
 * l2lim-wrapper after this loader hands over. HSS_Setup_Clocks() does NOT
 * touch CLOCK_CONFIG_CR, so during this loader the real PCLK is ~80/8 = 10 MHz.
 *
 * We pass this real PCLK to MSS_UART_init_ex() so the MMUART driver programs
 * the correct baud divisor and 115200 lands on the wire. We read the live APB
 * divider from CLOCK_CONFIG_CR[5:4] rather than assuming a fixed value, so this
 * stays correct regardless of the reset divider state.
 */
#define BM2_MSS_STANDBY_CLK	80000000u

static uint32_t bm2_boot_pclk(void)
{
    /* APB/AHB divider code lives in CLOCK_CONFIG_CR[5:4]: 0=>/1 1=>/2 2=>/4 3=>/8 */
    uint32_t div_code = (SYSREG->CLOCK_CONFIG_CR >> 4u) & 0x3u;

    return BM2_MSS_STANDBY_CLK >> div_code;
}


uint8_t mss_config_clk_rst(int peripheral, uint8_t hart, int req_state); // local stub
uint8_t mss_config_clk_rst(int peripheral, uint8_t hart, int req_state)
{
    (void)peripheral;
    (void)hart;
    (void)req_state;
    return 0u;
}

extern bool HSS_Setup_Clocks(void);
extern void *HSS_UART_GetInstance(int hartid);
extern void MSS_UART_init_ex(void* this_uart, uint32_t baud_rate,
                             uint8_t line_config, uint64_t pclk_freq);
extern void MSS_UART_polled_tx_string(void* ptr, const uint8_t *p_string);
extern void config_l2_cache(void);
extern void flush_early_caching(unsigned long num_ways);

/* Provided by snvm-bm2-loader.ld */
extern uint8_t __l2lim_start[];
extern uint8_t __l2lim_end[];

/* One way is enabled as cache at reset - see NUM_CACHEWAYS_AT_RESET in mss_entry.S */
#define NUM_CACHEWAYS_AT_RESET  1UL

__attribute__((noreturn)) void bm2_main(void)
{
    const uint8_t *p_envm = (const uint8_t *)ENVM_BASE;
    uint8_t       *p_lim  = (uint8_t *)L2LIM_BASE;
    unsigned int   num_errors = 0u;

    (void)HSS_Setup_Clocks();
    /* Init the UART against the real (pre-PLL) boot PCLK so the MMUART driver
     * programs the correct divisor and a true 115200 lands on the wire. See
     * bm2_boot_pclk() above. */
    MSS_UART_init_ex(HSS_UART_GetInstance(0), MSS_UART_115200_BAUD, MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT, bm2_boot_pclk());

    MSS_UART_polled_tx_string(HSS_UART_GetInstance(0), (const uint8_t *)"\033[2J\033[H"
#if defined(CONFIG_COLOR_OUTPUT)
        "\033[0m"
#endif
    );

    /*
     * Do NOT program the L2 way split here - leave WAY_ENABLE at its reset
     * value and let l2lim-wrapper do it, which it already does during its own
     * start-up.
     *
     * config_l2_cache() writes WAY_ENABLE, which shrinks the usable L2LIM
     * window from its reset size (1920 KiB, one cache way) to whatever the
     * design specifies (WAY_ENABLE=0xB => 512 KiB on this board).  Doing that
     * before anything has fetched from L2LIM leaves the window writable and
     * readable as data - the FEC decode below still lands and its SHA-256
     * check below still passes - but instruction fetch from 0x08000000 then
     * takes an access fault (mcause=1) on every hart, so the handover at the
     * end of this function never arrives.
     *
     * Measured on RTPFS460ZT: entering L2LIM with WAY_ENABLE still at its
     * reset value works - the wrapper runs, calls config_l2_cache() itself
     * from inside L2LIM, and goes on to decompress the HSS to L2-Scratchpad.
     * Shrinking the window from code executing in LIM is fine, shrinking it
     * before entering LIM is not.  Note WAY_ENABLE is write-once-increment,
     * so this cannot be undone here once set.
     */
    flush_early_caching(NUM_CACHEWAYS_AT_RESET);

    /*
     * ECC-initialise L2LIM before the FEC decode populates it.
     *
     * L2LIM is ECC-protected on this device and comes out of reset with no
     * valid ECC.  The decode below writes the wrapper image with sub-granule
     * stores, which read-modify-write granules whose ECC was never
     * established.  The data bits then read back correctly - the SHA-256
     * check below passes and a debugger sees the right image - but an
     * instruction fetch landing on such a granule takes an access fault
     * (mcause=1), which is why the wrapper died fetching config_32_copy.
     * Writing the whole window first with aligned 64-bit stores establishes
     * valid ECC for every granule.
     *
     * Only the final (post-config_l2_cache) window is initialised. The
     * wrapper shrinks LIM to __l2lim_end once it starts, so anything above
     * that is about to disappear anyway.  The loader runs entirely from DTIM
     * (see snvm-bm2-loader.ld), so clearing LIM cannot disturb its own stack.
     */
    MSS_UART_polled_tx_string(HSS_UART_GetInstance(0),
       (const uint8_t *) "snvm-bm2-loader: ECC-initialising L2LIM ...  ");
    {
        volatile uint64_t *p_lim64 = (volatile uint64_t *)__l2lim_start;
        const size_t       n_words = ((size_t)(__l2lim_end - __l2lim_start))
                                         / sizeof(uint64_t);

        for (size_t i = 0u; i < n_words; i++) {
            p_lim64[i] = 0u;
        }
    }
    __asm__ volatile ("fence" ::: "memory");
    MSS_UART_polled_tx_string(HSS_UART_GetInstance(0), (const uint8_t *)"Done\r\n");

    MSS_UART_polled_tx_string(HSS_UART_GetInstance(0),
       (const uint8_t *) "snvm-bm2-loader: decoding eNVM (72/64b SECDEC) to L2LIM ...  ");

    /* FEC decode eNVM -> L2LIM */
    (void)fec_secded7264_decode(LIM_FEC_ENCODED_SIZE, p_envm, p_lim, &num_errors);

    if ((num_errors != 0u)  /* Uncorrectable FEC errors => halt */
          || (!bm2_validate_sha256(p_lim, LIM_DECODED_SIZE,
                                   lim_expected_sha256))) { /* Digest mismatch => halt */
        MSS_UART_polled_tx_string(HSS_UART_GetInstance(0), (const uint8_t *)"FAIL\r\n");
        for (;;) {
            __asm__ volatile ("ebreak");
        }
    }

    MSS_UART_polled_tx_string(HSS_UART_GetInstance(0), (const uint8_t *)"Passed\r\n");

    /* Release U54 harts via CLINT MSIP[1..4] */
    __asm__ volatile ("fence" ::: "memory");

    volatile uint32_t *clint_msip = (volatile uint32_t *)CLINT_BASE;
    clint_msip[1] = 1u;
    clint_msip[2] = 1u;
    clint_msip[3] = 1u;
    clint_msip[4] = 1u;

    /* => Jump E51 to L2LIM entry */
    __asm__ volatile ("fence.i" ::: "memory");

    MSS_UART_polled_tx_string(HSS_UART_GetInstance(0),
       (const uint8_t *)"snvm-bm2-loader: handover from DTIM to L2LIM ... ");

    void (*lim_entry)(void) = (void (*)(void))L2LIM_BASE;
    lim_entry();

    /* Unreachable */
    MSS_UART_polled_tx_string(HSS_UART_GetInstance(0), (const uint8_t *)"FAIL\r\n");
    for (;;) {}
    __builtin_unreachable();
}
