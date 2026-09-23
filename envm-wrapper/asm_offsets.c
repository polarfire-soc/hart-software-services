/*******************************************************************************
 * Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * envm-wrapper/asm_offsets.c
 *
 * Pseudo file to generate build/envm-wrapper/asm_offsets.h via the build system.
 * See envm-wrapper/Makefile for the build rule.
 *
 * Derives struct field offsets and magic constants directly from hss_types.h,
 * providing a *Single Source of Truth* for the defines used in envm-wrapper_crt.S.
 *
 * The build rule compiles this file with -S, greps the "^->" markers from the
 * assembly output, and converts them into #define statements.
 */

#include <stddef.h>
#include <assert.h>

#include "mpfs_hal/mss_hal.h"
#include "mpfs_hal/common/mss_l2_cache.h"

#include "include/hss_types.h"

/*
 * Emit a symbol->value mapping that the build rule can extract.
 * Each DEFINE() emits one "->sym value sym" line in the -S output.
 */
#define DEFINE(sym, val) \
    __asm__ volatile("\n->" #sym " %0 " #sym : : "i" ((long)(val)))

/*
 * The assembly implementation of config_l2_cache() in envm-wrapper_crt.S replaces
 * the weak C version in mss_l2_cache.c, and with it the static_assert()s that
 * version carried. Repeat them here, where they are still compile-time checked.
 *
 * Note that the C version's runtime ASSERT() on the size of the l2lim region is
 * deliberately *not* repeated: in the eNVM wrapper, __l2lim_end is
 * ORIGIN(l2lim) + LENGTH(l2lim) (a fixed 512KB) rather than the end of the
 * sections actually placed there, so the check compares a constant against the
 * configured LIM size and fires spuriously for any large scratchpad
 * configuration. The assembly version needs no LIM at all.
 */
static_assert(LIBERO_SETTING_WAY_ENABLE < 16U, "Too many ways");
static_assert(LIBERO_SETTING_WAY_ENABLE >= LIBERO_SETTING_NUM_SCRATCH_PAD_WAYS,
    "Scratchpad Missing");

void gen_offsets(void);
void gen_offsets(void)
{
    DEFINE(mHSS_COMPRESSED_VERSION_FASTLZ,              mHSS_COMPRESSED_VERSION_FASTLZ);
    DEFINE(mHSS_COMPRESSED_VERSION_DEFLATE,             mHSS_COMPRESSED_VERSION_DEFLATE);

    DEFINE(HSS_CompressedImage_magic_OFFSET,            offsetof(struct HSS_CompressedImage, magic));
    DEFINE(HSS_CompressedImage_version_OFFSET,          offsetof(struct HSS_CompressedImage, version));
    DEFINE(HSS_CompressedImage_headerLength_OFFSET,     offsetof(struct HSS_CompressedImage, headerLength));
    DEFINE(HSS_CompressedImage_headerCrc_OFFSET,        offsetof(struct HSS_CompressedImage, headerCrc));
    DEFINE(HSS_CompressedImage_compressedCrc_OFFSET,    offsetof(struct HSS_CompressedImage, compressedCrc));
    DEFINE(HSS_CompressedImage_originalCrc_OFFSET,      offsetof(struct HSS_CompressedImage, originalCrc));
    DEFINE(HSS_CompressedImage_compressedImageLen_OFFSET, offsetof(struct HSS_CompressedImage, compressedImageLen));
    DEFINE(HSS_CompressedImage_originalImageLen_OFFSET, offsetof(struct HSS_CompressedImage, originalImageLen));

    /*
     * Cache controller register offsets and geometry, for the stack-free
     * config_l2_cache() in envm-wrapper_crt.S.
     */
    DEFINE(CACHE_CTRL_ADDR,                            CACHE_CTRL_BASE);
    DEFINE(CACHE_CTRL_WAY_ENABLE_OFFSET,               offsetof(CACHE_CTRL_typedef, WAY_ENABLE));

    /*
     * The way-mask block starts at 0x800, beyond the reach of a 12-bit signed
     * load/store immediate, so the assembly biases a base register by
     * WAY_MASK_BLOCK_OFFSET and addresses each mask relative to that.
     */
    DEFINE(CACHE_CTRL_WAY_MASK_BLOCK_OFFSET,           offsetof(CACHE_CTRL_typedef, WAY_MASK_DMA));
#define WAY_MASK_REL(field) \
    (offsetof(CACHE_CTRL_typedef, field) - offsetof(CACHE_CTRL_typedef, WAY_MASK_DMA))
    DEFINE(WAY_MASK_DMA_REL,                           WAY_MASK_REL(WAY_MASK_DMA));
    DEFINE(WAY_MASK_AXI4_PORT_0_REL,                   WAY_MASK_REL(WAY_MASK_AXI4_SLAVE_PORT_0));
    DEFINE(WAY_MASK_AXI4_PORT_1_REL,                   WAY_MASK_REL(WAY_MASK_AXI4_SLAVE_PORT_1));
    DEFINE(WAY_MASK_AXI4_PORT_2_REL,                   WAY_MASK_REL(WAY_MASK_AXI4_SLAVE_PORT_2));
    DEFINE(WAY_MASK_AXI4_PORT_3_REL,                   WAY_MASK_REL(WAY_MASK_AXI4_SLAVE_PORT_3));
    DEFINE(WAY_MASK_E51_DCACHE_REL,                    WAY_MASK_REL(WAY_MASK_E51_DCACHE));
    DEFINE(WAY_MASK_E51_ICACHE_REL,                    WAY_MASK_REL(WAY_MASK_E51_ICACHE));
    DEFINE(WAY_MASK_U54_1_DCACHE_REL,                  WAY_MASK_REL(WAY_MASK_U54_1_DCACHE));
    DEFINE(WAY_MASK_U54_1_ICACHE_REL,                  WAY_MASK_REL(WAY_MASK_U54_1_ICACHE));
    DEFINE(WAY_MASK_U54_2_DCACHE_REL,                  WAY_MASK_REL(WAY_MASK_U54_2_DCACHE));
    DEFINE(WAY_MASK_U54_2_ICACHE_REL,                  WAY_MASK_REL(WAY_MASK_U54_2_ICACHE));
    DEFINE(WAY_MASK_U54_3_DCACHE_REL,                  WAY_MASK_REL(WAY_MASK_U54_3_DCACHE));
    DEFINE(WAY_MASK_U54_3_ICACHE_REL,                  WAY_MASK_REL(WAY_MASK_U54_3_ICACHE));
    DEFINE(WAY_MASK_U54_4_DCACHE_REL,                  WAY_MASK_REL(WAY_MASK_U54_4_DCACHE));
    DEFINE(WAY_MASK_U54_4_ICACHE_REL,                  WAY_MASK_REL(WAY_MASK_U54_4_ICACHE));
#undef WAY_MASK_REL

    DEFINE(SYSREG_ADDR,                                BASE32_ADDR_MSS_SYSREG);
    DEFINE(SYSREG_L2_SHUTDOWN_CR_OFFSET,               offsetof(mss_sysreg_t, L2_SHUTDOWN_CR));

    DEFINE(L2_ZERO_DEVICE_BOTTOM,                      ZERO_DEVICE_BOTTOM);
    DEFINE(L2_CACHE_BLOCK_BYTE_LENGTH,                 CACHE_BLOCK_BYTE_LENGTH);
    DEFINE(L2_BLOCKS_PER_WAY,                          WAY_BYTE_LENGTH / CACHE_BLOCK_BYTE_LENGTH);

    /*
     * INIT_MARKER is a 64-bit value, so it is emitted as two 32-bit halves and
     * reassembled in registers (no single RISC-V immediate can hold it).
     */
    DEFINE(L2_INIT_MARKER_HI,                          (INIT_MARKER >> 32) & 0xFFFFFFFFU);
    DEFINE(L2_INIT_MARKER_LO,                          INIT_MARKER & 0xFFFFFFFFU);

    /*
     * Libero-derived cache configuration.
     */
    DEFINE(CFG_WAY_ENABLE,                             LIBERO_SETTING_WAY_ENABLE);
    DEFINE(CFG_L2_SHUTDOWN_CR,                         LIBERO_SETTING_L2_SHUTDOWN_CR);
    DEFINE(CFG_NUM_SCRATCH_PAD_WAYS,                   LIBERO_SETTING_NUM_SCRATCH_PAD_WAYS);

    /*
     * Seed for the scratchpad way walk, matching the C original's
     * 0x1U << ((WAY_ENABLE + 1) - NUM_SCRATCH_PAD_WAYS).
     */
    DEFINE(CFG_SCRATCHPAD_SEED_WAY,
        1U << ((LIBERO_SETTING_WAY_ENABLE + 1U) - LIBERO_SETTING_NUM_SCRATCH_PAD_WAYS));

    DEFINE(CFG_WAY_MASK_DMA,                           LIBERO_SETTING_WAY_MASK_DMA);
    DEFINE(CFG_WAY_MASK_AXI4_PORT_0,                   LIBERO_SETTING_WAY_MASK_AXI4_PORT_0);
    DEFINE(CFG_WAY_MASK_AXI4_PORT_1,                   LIBERO_SETTING_WAY_MASK_AXI4_PORT_1);
    DEFINE(CFG_WAY_MASK_AXI4_PORT_2,                   LIBERO_SETTING_WAY_MASK_AXI4_PORT_2);
    DEFINE(CFG_WAY_MASK_AXI4_PORT_3,                   LIBERO_SETTING_WAY_MASK_AXI4_PORT_3);
    DEFINE(CFG_WAY_MASK_E51_DCACHE,                    LIBERO_SETTING_WAY_MASK_E51_DCACHE);
    DEFINE(CFG_WAY_MASK_E51_ICACHE,                    LIBERO_SETTING_WAY_MASK_E51_ICACHE);
    DEFINE(CFG_WAY_MASK_U54_1_DCACHE,                  LIBERO_SETTING_WAY_MASK_U54_1_DCACHE);
    DEFINE(CFG_WAY_MASK_U54_1_ICACHE,                  LIBERO_SETTING_WAY_MASK_U54_1_ICACHE);
    DEFINE(CFG_WAY_MASK_U54_2_DCACHE,                  LIBERO_SETTING_WAY_MASK_U54_2_DCACHE);
    DEFINE(CFG_WAY_MASK_U54_2_ICACHE,                  LIBERO_SETTING_WAY_MASK_U54_2_ICACHE);
    DEFINE(CFG_WAY_MASK_U54_3_DCACHE,                  LIBERO_SETTING_WAY_MASK_U54_3_DCACHE);
    DEFINE(CFG_WAY_MASK_U54_3_ICACHE,                  LIBERO_SETTING_WAY_MASK_U54_3_ICACHE);
    DEFINE(CFG_WAY_MASK_U54_4_DCACHE,                  LIBERO_SETTING_WAY_MASK_U54_4_DCACHE);
    DEFINE(CFG_WAY_MASK_U54_4_ICACHE,                  LIBERO_SETTING_WAY_MASK_U54_4_ICACHE);
}
