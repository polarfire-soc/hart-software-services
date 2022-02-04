#ifndef HSS_TYPES_H
#define HSS_TYPES_H

/*******************************************************************************
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
 *
 * Hart Software Services - HSS Types
 *
 */

/**
 * \file MPFS HSS Embedded Software - Types
 * \brief MPFS HSS Embedded Software - Types
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>     // ptrdiff_t
#ifndef CONFIG_OPENSBI
#  ifndef __ssize_t_defined
#    define __ssize_t_defined
       typedef long			ssize_t;
#  endif

#  include <sys/types.h>  // for size_t
#  include <stdbool.h> // for bool, true, false
#  include <stdint.h>
#else
#  ifdef __packed
#    undef __packed
#  endif
#  include "sbi/sbi_types.h"
#  define true TRUE
#  define false FALSE
#endif

/**
 * \brief HSS Cores Enumeration
 */
enum HSSHartId {
    HSS_HART_E51 = 0,
    HSS_HART_U54_1,
    HSS_HART_U54_2,
    HSS_HART_U54_3,
    HSS_HART_U54_4,

    HSS_HART_NUM_PEERS,
    HSS_HART_ALL = HSS_HART_NUM_PEERS
};
#define MAX_NUM_HARTS ((unsigned)HSS_HART_NUM_PEERS)
#define mHSS_BITMASK_ALL_U54 (0x1Eu)

#define BOOT_FLAG_ANCILLIARY_DATA (0x80u)
#define BOOT_FLAG_SKIP_OPENSBI    (0x40u)

typedef union HSSHartBitmask {
    unsigned int uint;
    struct {
       unsigned int e51:1;
       unsigned int u54_1:1;
       unsigned int u54_2:1;
       unsigned int u54_3:1;
       unsigned int u54_4:1;
    } s;
} HSSHartBitmask_t;



/**
 * \brief Chunk Descriptor Structure for boot image
 *
 * Describes where to copy and how much...
 *
 */
#pragma pack(8)
struct HSS_BootChunkDesc {
    enum HSSHartId owner;
    uintptr_t loadAddr;
    uintptr_t execAddr;
    size_t size;
    uint32_t crc32;
};


/**
 *  * \brief Descriptor for U54 Boot Zero-Initialized Chunk
 *   */
#pragma pack(8)
struct HSS_BootZIChunkDesc {
    enum HSSHartId owner;
    void *execAddr;
    size_t size;
};

/**
 * \brief Boot Image Structure
 *
 * \warning The chunk table *must* be terminated with a size of 0 sentinel!
 */
#define BOOT_IMAGE_MAX_NAME_LEN (256)

#pragma pack(8)
struct HSS_BootImage {
    uint32_t magic;
    uint32_t version;
    size_t headerLength;
    uint32_t headerCrc;
    size_t chunkTableOffset;
    size_t ziChunkTableOffset;
    struct {
        uintptr_t entryPoint;
        uint8_t privMode;
        uint8_t flags;
        size_t numChunks;
        size_t firstChunk;
        size_t lastChunk;
        char name[BOOT_IMAGE_MAX_NAME_LEN];
    } hart[MAX_NUM_HARTS-1]; // E51 is not counted, only U54s
    char set_name[BOOT_IMAGE_MAX_NAME_LEN];
    size_t bootImageLength;
    uint8_t hash[32];
    uint8_t ecdsaSig[32];
};


/**
 * \brief Compressed Image Structure
 *
 */
#pragma pack(8)
struct HSS_CompressedImage {
    uint32_t magic;
    uint32_t version;
    size_t headerLength;
    uint32_t headerCrc;
    uint32_t compressedCrc;
    uint32_t originalCrc;
    size_t compressedImageLen;
    size_t originalImageLen;
    uint8_t hash[32];
    uint8_t ecdsaSig[32];
};

#define mHSS_COMPRESSED_VERSION_DEFLATE  1u

/*
 * We'll use the same trick as used by Linux for its Kconfig options...
 *
 * With variable arguments to our macro, if our config option is defined, it will cause
 * the insertion of "0, " as a prefix the arguments to ___IS_ENABLED(CONFIG_), which will
 * then cause __IS_ENABLED(CONFIG_) itself to resolve to 1, otherwise 0
 *
 */
#define _ARG_SHUFFLE_RIGHT_IF_1                 0,
#define IS_ENABLED(cfg)                         _IS_ENABLED(cfg)
#define _IS_ENABLED(val)                        __IS_ENABLED(_ARG_SHUFFLE_RIGHT_IF_##val)
#define __IS_ENABLED(shuffle_or_blank)          ___IS_ENABLED(shuffle_or_blank 1, 0)
#define ___IS_ENABLED(ignored, desiredVal, ...) desiredVal

#define ARRAY_SIZE(x)		(sizeof(x)/sizeof(x[0]))

#define mHSS_BOOT_MAGIC		(0xB007C0DEu)
#define mHSS_COMPRESSED_MAGIC	(0xC08B8355u)

#ifndef CONFIG_OPENSBI
#  define MIN(A,B)		((A) < (B) ? A : B)
#  define likely(x)		__builtin_expect((x), 1)
#  define unlikely(x)		__builtin_expect((x), 0)
#endif

#ifdef __cplusplus
}
#endif

#endif
