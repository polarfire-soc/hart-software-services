/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 \file  strlen
 \brief Local trivial implementation of strlen
*/


#include "config.h"

#include <string.h>

/*!
 * \brief Calculate length of string
 *
 * Returns number of bytes in a string, excluding the null terminator.
 */
size_t strlen (const char *string)
{
    size_t result = 0;

    while (*string) {
        string++;
        result++;
    }

    return result;
}
