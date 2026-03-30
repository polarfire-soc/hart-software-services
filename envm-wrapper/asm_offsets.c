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
#include "include/hss_types.h"

/*
 * Emit a symbol->value mapping that the build rule can extract.
 * Each DEFINE() emits one "->sym value sym" line in the -S output.
 */
#define DEFINE(sym, val) \
    __asm__ volatile("\n->" #sym " %0 " #sym : : "i" ((long)(val)))

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
}
