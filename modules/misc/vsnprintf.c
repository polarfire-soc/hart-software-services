/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file Local vsnprintf
 * \brief Local implementation of vsnprintf
 */

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>

/*!
 * \brief prints at most size characters to buffer pointed to by str,
 * using a va_list (variable arguments list)
 */

enum e_FormatFlags
{
    FORMAT_FLAG_LEFT,
    FORMAT_FLAG_RIGHT_JUSTIFY,
    FORMAT_FLAG_SPACES,
    FORMAT_FLAG_SPECIAL,
    FORMAT_FLAG_ZERO_PAD
};

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    char *pBuf;
    int formatFlags = 0;
    unsigned int field_width = 0u;

    for (pBuf = str; *format && size; format++) {
        if (*format != '%') {
            *pBuf = *format;
            pBuf++;
            size--;
            continue;
        }

NEXT_CHARACTER:
        switch (*format) {
        case '-':
            formatFlags |= (int)FORMAT_FLAG_LEFT;
            goto NEXT_CHARACTER;
        case '+':
            formatFlags |= (int)FORMAT_FLAG_RIGHT_JUSTIFY;
            goto NEXT_CHARACTER;
        case ' ':
            formatFlags |= (int)FORMAT_FLAG_SPACES;
            goto NEXT_CHARACTER;
        case '#':
            formatFlags |= (int)FORMAT_FLAG_SPECIAL;
            goto NEXT_CHARACTER;
        case '0':
            formatFlags |= (int)FORMAT_FLAG_ZERO_PAD;
            goto NEXT_CHARACTER;
        case '1'...'9':
            field_width = 1u;
            break;
        case 'd':
            // unimplemented
            break;
        case 'u':
            // unimplemented
            break;
        case 's':
            {
                char *strArg = va_arg(ap, char*);
                if (!strArg) { strArg = (char *)"(null)"; }
                //puts(strArg);
            }
            break;
        case 'c':
            //putc(va_arg(ap, int), stdout);
            break;
        default:
            break;
        }
    }

    (void)field_width;
    return 0;
}
