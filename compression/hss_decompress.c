/******************************************************************************************
 * 
 * MPFS HSS Embedded Software
 *
 * Copyright 2019 Microchip Corporation.
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
 */

/*\!
 *\file Decompression Wrapper
 *\brief Decompression Wrapper
 */

#include "config.h"
#include "hss_types.h"
#include "fastlz.h"
#include "hss_crc32.h"
#include "hss_debug.h"

#include "hss_decompress.h"
#include "hss_progress.h"

#include <assert.h>

int HSS_Decompress(const void* pInputBuffer, void* pOutputBuffer)
{
    int result = 0;
    struct HSS_CompressedImage compressedImageHdr = *(struct HSS_CompressedImage *)pInputBuffer;

    if (compressedImageHdr.magic != HSS_COMPRESSED_MAGIC) {
        mHSS_DEBUG_PRINTF("Compressed Image is missing magic value (%08X vs %08X)" CRLF, 
            compressedImageHdr.magic, HSS_COMPRESSED_MAGIC);
    } else {
        mHSS_DEBUG_PRINTF("Compressed Image Length is %lu" CRLF, compressedImageHdr.compressedImageLen);

        uint32_t originalCrc = compressedImageHdr.headerCrc;

        compressedImageHdr.headerCrc = 0;
        uint32_t compressedCrc = CRC32_calculate((const uint8_t *)&compressedImageHdr, 
            sizeof(struct HSS_CompressedImage)); 

        if (originalCrc != compressedCrc) {
            mHSS_DEBUG_PRINTF("Compressed Image failed CRC check" CRLF);
        } else {
            const uint8_t *pByteOffset = pInputBuffer;
            pByteOffset += sizeof(struct HSS_CompressedImage);

            mHSS_DEBUG_PRINTF("Decompressing from 0x%p to 0x%p" CRLF, pByteOffset, pOutputBuffer);

            result = fastlz_decompress(
                (const void *)pByteOffset, (int)compressedImageHdr.compressedImageLen, 
                (void *)pOutputBuffer, (int)(compressedImageHdr.originalImageLen), HSS_ShowProgress);
        }
    } 

    return result;
}
