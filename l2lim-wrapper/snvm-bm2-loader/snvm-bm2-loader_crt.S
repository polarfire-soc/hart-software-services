/*
 * Copyright 2019-2026 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * sNVM Boot Mode 2 Loader - CRT startup
 *
 * PolarFire SoC BM2: the system controller copies the authenticated sNVM
 * payload to the 8 KiB DTIM and releases all five harts simultaneously to
 * the DTIM entry point (_start).
 *
 * Hart dispatch:
 *
 *   E51  (mhartid == 0) - falls through to .e51_work:
 *       zeros BSS, calls bm2_main(),
 *       bm2_main() FEC-decodes eNVM -> L2LIM,
 *       validates the SHA256 digest,
 *       fires CLINT MSIP[1..4], then jumps to L2LIM entry.
 *
 *   U54s (mhartid 1..4) - take the .u54_wait path:
 *       set mtvec = 0x08000000 (L2LIM entry, direct mode),
 *       enable the machine-software-interrupt (MSIP),
 *       enable global MIE, then WFI.
 *       When the E51 writes CLINT MSIP[1..4], the software-interrupt trap fires
 *           and each U54 jumps directly to l2lim-wrapper _start.
 */

#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>

    .section .entry, "ax", @progbits
    .attribute unaligned_access, 0
    .attribute stack_align, 16
    .align  3
    .globl  _start

_start:
    /* Install a minimal early-fault trap handler in DTIM */
    la      t0, .early_trap
    csrw    CSR_MTVEC, t0

    /* Determine which hart we are */
    csrr    a0, CSR_MHARTID

    /*
     * Set up per-hart stack:
     *
     *   base = ALIGN(__stack_bottom, 64)
     *   sp   = base + (hartid + 1) * STACK_SIZE_PER_HART - 8
     *
     * The linker script aligns .stack to 64 bytes, but we re-align
     * defensively in case the symbol is not already aligned.
     */
    la      s0, __stack_bottom
    addi    s0, s0, 63
    andi    s0, s0, -64                 /* s0 = 64-byte-aligned base */
    la      s1, STACK_SIZE_PER_HART    /* load value 512 via abs-symbol trick */
    addi    t0, a0, 1                   /* t0 = hartid + 1 */
    mul     t0, t0, s1                  /* t0 = (hartid+1) * 512 */
    add     sp, s0, t0                  /* sp = base + offset */
    addi    sp, sp, -8                  /* ABI-required 8-byte backoff */

    /* Dispatch: E51 does the real work; U54s arm for LIM and wait */
    beqz    a0, .e51_work

.u54_wait:
    /*
     * Set mtvec to L2LIM entry (l2lim-wrapper _start) in direct mode.
     * 0x08000000 is naturally 4-byte aligned, so MODE field bits[1:0] = 0.
     */
    li      t0, 0x08000000
    csrw    CSR_MTVEC, t0

    /* Enable machine software interrupt only */
    csrw    CSR_MIE, zero
    li      t0, MIP_MSIP                /* bit 3 - MSIP enable in mie */
    csrw    CSR_MIE, t0

    /* Enable global machine-mode interrupts */
    li      t0, MSTATUS_MIE
    csrs    CSR_MSTATUS, t0

    /* Wait for software interrupt from E51 */
.u54_spin:
    wfi
    j       .u54_spin

.e51_work:
    /* Zero BSS (8 bytes at a time; __bss_start and __bss_end are 16-aligned) */
    la      a0, __bss_start
    la      a1, __bss_end
.zero_bss:
    beq     a0, a1, .bss_done
    sd      zero, 0(a0)
    addi    a0, a0, 8
    j       .zero_bss
.bss_done:

    call    bm2_main

    /* bm2_main() should never return.  Halt if it does. */
.e51_halt:
    j       .e51_halt

    .align  3
.early_trap:
    j       .early_trap

/*
 * void flush_early_caching(unsigned long num_ways)
 *
 * Empty the cache ways that are live before config_l2_cache() reassigns way
 * roles.  At reset only one way is enabled as cache (NUM_CACHEWAYS_AT_RESET);
 * anything cacheable this loader has already touched - notably its eNVM reads -
 * can be sitting in it.
 *
 * a0 = number of ways to flush (NUM_CACHEWAYS_AT_RESET == 1)
 */
    .equ    L2_ZERO_DEVICE_ADDR, 0x0A000000
    .equ    L2_CCACHE_ADDR,      0x02010000
    .equ    FLUSH64_OFFSET,      0x200

    .align  3
    .globl  flush_early_caching
    .type   flush_early_caching, @function
flush_early_caching:
    slli    a0, a0, 17                  /* a0 = num_ways * (4 * 512 * 64) */
    li      a5, L2_ZERO_DEVICE_ADDR
    add     a0, a0, a5                  /* a0 = end address */
    beq     a0, a5, .flush_early_exit

.preload_cache_with_known_addrs:
    ld      a3, 0(a5)
    addi    a5, a5, 64
    bne     a5, a0, .preload_cache_with_known_addrs

    li      a5, L2_ZERO_DEVICE_ADDR
    li      a3, L2_CCACHE_ADDR
.flush_known_addrs_from_cache:
    sd      a5, FLUSH64_OFFSET(a3)
    addi    a5, a5, 64
    bne     a5, a0, .flush_known_addrs_from_cache

.flush_early_exit:
    ret
