/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * This file contains trivial C standard library routines for the
 * Hart Software Services, to allow it link without external dependencies.
 *
 */


#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#include <assert.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
//#include <ctype.h>  // including ctype.h breaks the build currently
#include <string.h>
#include <sbi_string.h>

#include "csr_helper.h"

#define MAX_TEXT_LENGTH   256

//#ifndef CONFIG_OPENSBI
__attribute__((weak)) size_t strnlen(const char *s, size_t count)
{
    size_t result = 0;
    while (count && *s) {
        ++result;
        ++s;
    }

    return result;
}

__attribute__((weak)) void *memcpy(void * restrict dest, const void * restrict src, size_t n)
{
    // no overlaps allowed!!
    char *cDest = (char*)dest;
    char *cSrc = (char*)src;

    // no overlaps allowed!!
    {
        if (cDest > cSrc) {
            assert((cSrc + n -1) < cDest);
        } else {
            assert((cDest + n -1) < cSrc);
        }
    }

#ifndef CONFIG_OPENSBI
    while (n--) {
        *cDest = *cSrc;
        ++cDest;
        ++cSrc;
    }

    return (dest);
#else
    return sbi_memcpy(dest, src, n);
#endif
}

__attribute__((weak)) int memcmp(const void * restrict s1, const void * restrict s2, size_t n)
{
#ifndef CONFIG_OPENSBI
    int result;

    const unsigned char *temp1 = s1;
    const unsigned char *temp2 = s2;

    while (n > 0 && (*temp1 == *temp2)) {
        temp1++;
        temp2++;
        n--;
    }

    if (n > 0) {
        result = *temp1 - *temp2;
    } else {
    	result = 0;
    }

    return result;
#else
    return sbi_memcmp(s1, s2, n);
#endif
}


__attribute__((weak)) void *memset(void *dest, int c, size_t n)
{
#ifndef CONFIG_OPENSBI
    char *cDest = (char*)dest;

    while (n--) {
        *cDest = (char)c;
        ++cDest
    }

    return (dest);
#else
    return sbi_memset(dest, c, n);
#endif
}

__attribute__((weak)) size_t strlen (const char *s)
{
#ifndef CONFIG_OPENSBI
    size_t result = 0;

    while (*s) {
        ++s
        ++result
    }

    return result;
#else
    return sbi_strlen(s);
#endif
}
//#endif

/***********************************************************************/

int tolower(int __c); // ctypes.h
__attribute__((weak)) int tolower(int __c)
{
    if ((__c > ('A'-1)) && (__c < ('Z'+1))) {
        __c -= ('A'-'a');
    }
    return __c;
}

__attribute__((weak)) int strncasecmp(const char *s1, const char *s2, size_t n)
{
    int result = 0;

    while ((*s1 != 0) && (*s2 != 0) && n) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);

        //if (c1 == c2) {
        //    ++s1
        //    ++s2
        //    continue;
        //} else
        if (c1 < c2) {
            result = -1;
            break;
        } else if (c1 > c2) {
            result = 1;
            break;
        }

        ++s1;
        ++s2;
        --n;
    }

    if (n && !result) {
        if ((*s1) && !(*s2)) { result = 1; }
        if ((*s2) && !(*s1)) { result = -1; }
    }

    return result;
}

__attribute__((weak)) char *strtok_r(char *str, const char *delim, char **saveptr)
{
    char *result = NULL;

    if (*saveptr == NULL) {
        *saveptr = str;
    }

    if (**saveptr == 0) {
        *saveptr = NULL;
        result = NULL;
    } else {
        result = *saveptr;
    }

    if (result != NULL) {
        while (**saveptr != 0) {
            const char *pDelim = delim;
            while (*pDelim != 0) {
                if (**saveptr == *pDelim) {
                    **saveptr = 0;
                    *saveptr = *saveptr + 1;

                    return result;
                }
                ++pDelim;
            }
            *saveptr = *saveptr + 1;
        }
    }

    return result;
}


__attribute__((weak)) int strncmp(const char *s1, const char *s2, size_t n)
{
    int result = 0;

    assert(s1);
    assert(s2);

    for (size_t i = 0u; i < n; i++) {
        if ((*s1 == '\0') || (*s2 == '\0')) {
            break;
        } else if (*s1 < *s2) {
            result = -1;
            break;
        } else if (*s1 > *s2) {
            result = 1;
            break;
        } else /* if (*s1 == *s2) */ {
            result = 0;
        }

        ++s1;
        ++s2;
    }

    return result;
}


__attribute__((weak)) int64_t __bswapdi2(int64_t a);
__attribute__((weak)) int64_t __bswapdi2(int64_t a)
{
    int64_t result;

    result =
          ((a & 0xFF) << 56)
        | ((a & 0xFF00) << 40)
        | ((a & 0xFF0000) << 24)
        | ((a & 0xFF000000) << 8)
        | ((a & 0xFF00000000) >> 8)
        | ((a & 0xFF0000000000) >> 24)
        | ((a & 0xFF000000000000) >> 40)
        | ((a & 0xFF00000000000000) >> 56);

    return result;
}

__attribute__((weak)) uint64_t strtoul(const char * restrict nptr, char ** restrict endptr, int base)
{
    size_t count = 0;
    uint64_t result = 0u;
    bool done = false;

    assert(nptr);

    do {
        const char digit = *nptr;
        switch (digit) {
        case '-':
            continue;

        case '0'...'9':
            result *= base;
            result += digit - '0';
            break;

        case 'a'...'f':
            result *= base;
            result += 10u + digit - 'a';
            break;

        case 'A'...'F':
            result *= base;
            result += 10u + digit - 'A';
            break;

        case 'x':
            if ((count != 1) || (base != 16)) {
                done = true;
            }
            break;

        default:
            done = true;
            break;
        }

        ++count;
        ++nptr;
    } while (!done);

    if (endptr) {
        *endptr = (char *)nptr;
    }

    return result;
}
