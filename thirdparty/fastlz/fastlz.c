/*  
  FastLZ - lightning-fast lossless compression library

  Copyright (C) 2007 Ariya Hidayat (ariya@kde.org)
  Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
  Copyright (C) 2005 Ariya Hidayat (ariya@kde.org)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "hss_debug.h"

#if !defined(FASTLZ_DECOMPRESSOR)

/*
 * Always check for bound when decompressing.
 * Generally it is best to leave it defined.
 */
#define FASTLZ_SAFE

/*
 * Give hints to the compiler for branch prediction optimization.
 */
#if defined(__GNUC__) && (__GNUC__ > 2)
#define FASTLZ_EXPECT_CONDITIONAL(c)    (__builtin_expect((c), 1))
#define FASTLZ_UNEXPECT_CONDITIONAL(c)  (__builtin_expect((c), 0))
#else
#define FASTLZ_EXPECT_CONDITIONAL(c)    (c)
#define FASTLZ_UNEXPECT_CONDITIONAL(c)  (c)
#endif

/*
 * Use inlined functions for supported systems.
 */
#if defined(__GNUC__) || defined(__DMC__) || defined(__POCC__) || defined(__WATCOMC__) || defined(__SUNPRO_C)
#define FASTLZ_INLINE inline
#elif defined(__BORLANDC__) || defined(_MSC_VER) || defined(__LCC__)
#define FASTLZ_INLINE __inline
#else 
#define FASTLZ_INLINE
#endif

/*
 * Prevent accessing more than 8-bit at once, except on x86 architectures.
 */
#if !defined(FASTLZ_STRICT_ALIGN)
#define FASTLZ_STRICT_ALIGN
#if defined(__i386__) || defined(__386)  /* GNU C, Sun Studio */
#undef FASTLZ_STRICT_ALIGN
#elif defined(__i486__) || defined(__i586__) || defined(__i686__) /* GNU C */
#undef FASTLZ_STRICT_ALIGN
#elif defined(_M_IX86) /* Intel, MSVC */
#undef FASTLZ_STRICT_ALIGN
#elif defined(__386)
#undef FASTLZ_STRICT_ALIGN
#elif defined(_X86_) /* MinGW */
#undef FASTLZ_STRICT_ALIGN
#elif defined(__I86__) /* Digital Mars */
#undef FASTLZ_STRICT_ALIGN
#endif
#endif

/*
 * FIXME: use preprocessor magic to set this on different platforms!
 */
typedef unsigned char  flzuint8;
typedef unsigned short flzuint16;
typedef unsigned int   flzuint32;

/* prototypes */
int fastlz_compress(const void* input, int length, void* output);
int fastlz_compress_level(int level, const void* input, int length, void* output);
int fastlz_decompress(const void* input, int length, void* output, int maxout, void (*progressCallback)(unsigned int, unsigned int));

#define MAX_COPY       32
#define MAX_LEN       264  /* 256 + 8 */
#define MAX_DISTANCE 8192

#if !defined(FASTLZ_STRICT_ALIGN)
#define FASTLZ_READU16(p) *((const flzuint16*)(p)) 
#else
#define FASTLZ_READU16(p) ((p)[0] | (p)[1]<<8)
#endif

#define HASH_LOG  13
#define HASH_SIZE (1<< HASH_LOG)
#define HASH_MASK  (HASH_SIZE-1)
#define HASH_FUNCTION(v,p) { v = FASTLZ_READU16(p); v ^= FASTLZ_READU16(p+1)^(v>>(16-HASH_LOG));v &= HASH_MASK; }

#undef FASTLZ_LEVEL
#define FASTLZ_LEVEL 1

#undef FASTLZ_DECOMPRESSOR
#define FASTLZ_DECOMPRESSOR fastlz1_decompress
static FASTLZ_INLINE int FASTLZ_DECOMPRESSOR(const void* input, int length, void* output, int maxout, void (*progressCallback)(unsigned int, unsigned int));
#include "fastlz.c"

#undef FASTLZ_LEVEL
#define FASTLZ_LEVEL 2

#undef MAX_DISTANCE
#define MAX_DISTANCE 8191
#define MAX_FARDISTANCE (65535+MAX_DISTANCE-1)

#undef FASTLZ_DECOMPRESSOR
#define FASTLZ_DECOMPRESSOR fastlz2_decompress
static FASTLZ_INLINE int FASTLZ_DECOMPRESSOR(const void* input, int length, void* output, int maxout, void (*progressCallback)(unsigned int, unsigned int));
#include "fastlz.c"

int fastlz_decompress(const void* input, int length, void* output, int maxout, void (*progressCallback)(unsigned int, unsigned int))
{
  /* magic identifier for compression level */
  int level = ((*(const flzuint8*)input) >> 5) + 1;

  if(level == 1)
    return fastlz1_decompress(input, length, output, maxout, progressCallback);
  if(level == 2)
    return fastlz2_decompress(input, length, output, maxout, progressCallback);

  /* unknown level, trigger error */
  return 0;
}

#else /* !defined(FASTLZ_DECOMPRESSOR) */

static FASTLZ_INLINE int FASTLZ_DECOMPRESSOR(const void* input, int length, void* output, int maxout, void (*progressCallback)(unsigned int, unsigned int))
{
  const flzuint8* ip = (const flzuint8*) input;
  const flzuint8* ip_limit  = ip + length;
  flzuint8* op = (flzuint8*) output;
  flzuint8* op_limit = op + maxout;
  flzuint32 ctrl = (*ip++) & 31;
  int loop = 1;

  do
  {
    const flzuint8* ref = op;
    flzuint32 len = ctrl >> 5;
    flzuint32 ofs = (ctrl & 31) << 8;

    if(ctrl >= 32)
    {
#if FASTLZ_LEVEL==2
      flzuint8 code;
#endif
      len--;
      ref -= ofs;
      if (len == 7-1)
#if FASTLZ_LEVEL==1
        len += *ip++;
      ref -= *ip++;
#else
        do
        {
          code = *ip++;
          len += code;
        } while (code==255);
      code = *ip++;
      ref -= code;

      /* match from 16-bit distance */
      if(FASTLZ_UNEXPECT_CONDITIONAL(code==255))
      if(FASTLZ_EXPECT_CONDITIONAL(ofs==(31 << 8)))
      {
        ofs = (*ip++) << 8;
        ofs += *ip++;
        ref = op - ofs - MAX_DISTANCE;
      }
#endif
      
#ifdef FASTLZ_SAFE
      if (FASTLZ_UNEXPECT_CONDITIONAL(op + len + 3 > op_limit)) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "op + len + 3 [%p] > op_limit [%p]" CRLF, op + len + 3, op_limit);
        return 0;
      }

      if (FASTLZ_UNEXPECT_CONDITIONAL(ref-1 < (flzuint8 *)output)) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "ref-1 [%p] < output [%p]" CRLF, output);
        return 0;
      }
#endif

      if(FASTLZ_EXPECT_CONDITIONAL(ip < ip_limit))
        ctrl = *ip++;
      else
        loop = 0;

      if(ref == op)
      {
        /* optimize copy for a run */
        flzuint8 b = ref[-1];
        *op++ = b;
        *op++ = b;
        *op++ = b;
        for(; len; --len)
          *op++ = b;
      }
      else
      {
#if !defined(FASTLZ_STRICT_ALIGN)
        const flzuint16* p;
        flzuint16* q;
#endif
        /* copy from reference */
        ref--;
        *op++ = *ref++;
        *op++ = *ref++;
        *op++ = *ref++;

#if !defined(FASTLZ_STRICT_ALIGN)
        /* copy a byte, so that now it's word aligned */
        if(len & 1)
        {
          *op++ = *ref++;
          len--;
        }

        /* copy 16-bit at once */
        q = (flzuint16*) op;
        op += len;
        p = (const flzuint16*) ref;
        for(len>>=1; len > 4; len-=4)
        {
          *q++ = *p++;
          *q++ = *p++;
          *q++ = *p++;
          *q++ = *p++;
        }
        for(; len; --len)
          *q++ = *p++;
#else
        for(; len; --len)
          *op++ = *ref++;
#endif
      }
    }
    else
    {
      ctrl++;
#ifdef FASTLZ_SAFE
      if (FASTLZ_UNEXPECT_CONDITIONAL(op + ctrl > op_limit)) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "op + ctrl [%p] > op_limit [%p]" CRLF, op + ctrl, op_limit);
        return 0;
      }
      if (FASTLZ_UNEXPECT_CONDITIONAL(ip + ctrl > ip_limit)) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "ip + ctrl [%p] > ip_limit [%p]" CRLF, ip + ctrl, ip_limit);
        return 0;
      }
#endif

      *op++ = *ip++;
      for(--ctrl; ctrl; ctrl--)
        *op++ = *ip++;

      loop = FASTLZ_EXPECT_CONDITIONAL(ip < ip_limit);
      if(loop)
        ctrl = *ip++;
    }
    if (progressCallback) { progressCallback(length, (int)(ip_limit - ip)); } // progress callback
  }
  while(FASTLZ_EXPECT_CONDITIONAL(loop));

  return op - (flzuint8*)output;
}

#endif /* !defined(FASTLZ_DECOMPRESSOR) */
