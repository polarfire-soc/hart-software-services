/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
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
        result++;
        s++;
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
        cDest++; cSrc++;
    }

    return (dest);
#else
    return sbi_memcpy(dest, src, n);
#endif
}

__attribute__((weak)) void *memset(void *dest, int c, size_t n)
{
#ifndef CONFIG_OPENSBI
    char *cDest = (char*)dest;

    while (n--) {
        *cDest = (char)c;
        cDest++;
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
        s++;
        result++;
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

__attribute__((weak)) int strcasecmp(const char *s1, const char *s2)
{
    int result = 0;

    while ((*s1 != 0) && (*s2 != 0)) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);

        //if (c1 == c2) {
        //    s1++;
        //    s2++;
        //    continue;
        //} else 
        if (c1 < c2) {
            result = -1;
            break;
        } else if (c1 > c2) {
            result = 1;
            break;
        }

        s1++;
        s2++;
    }

    if (result == 0) {
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
                pDelim++;
            }
            *saveptr = *saveptr + 1;
        }
    }
    
    return result;
}

