/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * 
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS Software Initalization
 * \brief Full System Initialization
 */

#include "config.h"
#include "hss_types.h"

#include "hss_debug.h"
#include "hss_logo_init.h"

//
// A variety of colored pixels are needed - red, white, black
// define these as ASCII characters if color output is not enabled
//
#ifdef CONFIG_COLOR_OUTPUT
  // white
  const char W0[] = "\033[48;5;188m ";
  // red
  const char r1[] ="\033[48;5;217m ";
  const char r2[] ="\033[48;5;210m ";
  const char r3[] ="\033[48;5;203m ";
  const char r4[] ="\033[48;5;196m ";
  // black
  const char b1[] ="\033[48;5;16m ";
  const char b2[] ="\033[48;5;59m ";
  const char b3[] ="\033[48;5;102m ";
  const char b4[] ="\033[48;5;145m ";
  // EOL reset 
  const char RST[] ="\033[0m";
#else
  // white
  const char W0[] =" ";
  // red
  const char r1[] =".";
  const char r2[] ="-";
  const char r3[] ="x";
  const char r4[] ="X";
  // black
  const char b1[] =":";
  const char b2[] ="o";
  const char b3[] ="0";
  const char b4[] ="O";
  // EOL reset
  const char RST[] ="";
#endif


// RLE Microchip Logo, built up from our color pixel primitives above...
// RLE shrinks the size of this
const struct {
    uint8_t const count;
    char const * const token;
} rleLogoElements[] = {
    { 4, W0 }, { 1, r1 }, { 1, r3 }, { 7, r4 }, { 62, W0 }, { 1, RST }, { 1, CRLF },
    { 3, W0 }, { 1, r2 }, { 9, r4 }, { 1, r3 }, { 61, W0 }, { 1, RST }, { 1, CRLF },
    { 2, W0 }, { 1, r3 }, { 2, r4 }, { 1, r3 }, { 1, r2 }, { 5, r4 }, { 1, r2 }, 
        { 1, r4 }, { 1, r1 }, { 7, W0 }, { 1, b3 }, { 5, W0 }, { 1, b3 }, { 46, W0 }, 
        { 1, RST }, { 1, CRLF },
    { 1, W0 }, { 1, r1 }, { 3, r4 }, { 2, W0 }, { 1, r2 }, { 3, r4 }, { 1, r1 }, 
        { 1, W0 }, { 1, r1 }, { 1, r4 }, { 6, W0 }, { 1, b4 }, { 1, b1 }, { 1, b2 }, 
        { 3, W0 }, { 1, b3 }, { 1, b1 }, { 1, b4 }, { 45, W0 }, { 1, RST }, 
        { 1, CRLF },
    { 1, W0 }, { 3, r4 }, { 1, r1 }, { 3, W0 }, { 2, r4 }, { 1, r2 }, { 3, W0 }, 
        { 2, r3 }, { 5, W0 }, { 1, b4 }, { 2, b1 }, { 3, W0 }, { 2, b1 }, { 1, b4 }, 
        { 1, W0 }, { 1, b4 }, { 2, W0 }, { 1, b4 }, { 3, b3 }, { 1, b4 }, { 1, W0 }, 
        { 5, b4 }, { 2, W0 }, { 1, b4 }, { 3, b3 }, { 1, b4 }, { 2, W0 }, { 1, b4 }, 
        { 3, b3 }, { 1, b4 }, { 1, W0 }, { 1, b4 }, { 3, W0 }, { 1, b4 }, { 2, W0 }, 
        { 1, b4 }, { 1, W0 }, { 5, b4 }, { 1, W0 }, { 1, RST }, { 1, CRLF },
    { 1, r1 }, { 3, r4 }, { 4, W0 }, { 1, r1 }, { 1, r4 }, { 5, W0 }, { 1, r4 }, 
        { 1, r1 }, { 4, W0 }, { 1, b3 }, { 2, b1 }, { 1, b4 }, { 1, W0 }, { 1, b4 }, 
        { 2, b1 }, { 1, b3 }, { 1, W0 }, { 1, b1 }, { 1, b4 }, { 1, W0 }, { 5, b1 }, 
        { 1, W0 }, { 5, b1 }, { 1, b3 }, { 1, W0 }, { 5, b1 }, { 1, b4 }, { 1, W0 }, 
        { 5, b1 }, { 1, W0 }, { 1, b1 }, { 3, W0 }, { 1, b2 }, { 1, b4 }, { 1, W0 }, 
        { 1, b1 }, { 1, W0 }, { 5, b1 }, { 1, b3 }, { 1, RST }, { 1, CRLF },
    { 1, r2 }, { 2, r4 }, { 1, r3 }, { 1, r2 }, { 4, W0 }, { 1, r3 }, { 1, r2 }, 
        { 4, W0 }, { 1, r2 }, { 1, r4 }, { 4, W0 }, { 1, b3 }, { 3, b2 }, { 1, W0 }, 
        { 2, b2 }, { 1, b3 }, { 1, b2 }, { 1, W0 }, { 1, b1 }, { 2, b4 }, { 1, b1 }, 
        { 5, W0 }, { 1, b1 }, { 1, b4 }, { 2, W0 }, { 1, b3 }, { 1, b2 }, { 1, b4 }, 
        { 1, b1 }, { 3, W0 }, { 1, b2 }, { 1, b3 }, { 1, b4 }, { 1, b1 }, { 5, W0 }, 
        { 1, b1 }, { 1, b4 }, { 2, W0 }, { 1, b2 }, { 1, b4 }, { 1, W0 }, { 1, b1 }, 
        { 1, W0 }, { 1, b1 }, { 1, b4 }, { 2, W0 }, { 1, b3 }, { 1, b2 }, { 1, RST }, 
        { 1, CRLF },
    { 1, r3 }, { 1, r4 }, { 1, r1 }, { 1, W0 }, { 1, r4 }, { 5, W0 }, { 1, r4 }, 
        { 5, W0 }, { 1, r4 }, { 1, r2 }, { 3, W0 }, { 1, b2 }, { 1, b3 }, { 1, W0 }, 
        { 1, b1 }, { 1, b3 }, { 1, b1 }, { 1, b4 }, { 1, b3 }, { 1, b2 }, { 1, W0 }, 
        { 1, b1 }, { 2, b4 }, { 1, b2 }, { 5, W0 }, { 1, b1 }, { 1, b3 }, { 2, W0 }, 
        { 2, b3 }, { 1, b4 }, { 1, b1 }, { 3, W0 }, { 1, b2 }, { 1, b3 }, { 1, b4 }, 
        { 1, b2 }, { 5, W0 }, { 5, b1 }, { 1, b4 }, { 1, W0 }, { 1, b1 }, { 1, W0 }, 
        { 1, b1 }, { 1, b4 }, { 2, W0 }, { 1, b3 }, { 1, b2 }, { 1, RST }, 
        { 1, CRLF },
    { 1, r3 }, { 1, r2 }, { 2, W0 }, { 1, r2 }, { 1, r3 }, { 4, W0 }, { 1, r1 }, 
        { 1, r3 }, { 4, W0 }, { 1, r1 }, { 1, r4 }, { 3, W0 }, { 1, b1 }, { 1, b3 }, 
        { 1, W0 }, { 3, b1 }, { 1, W0 }, { 1, b4 }, { 1, b1 }, { 1, W0 }, { 1, b1 }, 
        { 2, b4 }, { 1, b2 }, { 5, W0 }, { 5, b1 }, { 1, W0 }, { 1, b4 }, { 1, b1 }, 
        { 3, W0 }, { 1, b2 }, { 1, b3 }, { 1, b4 }, { 1, b2 }, { 5, W0 }, { 1, b1 }, 
        { 1, b3 }, { 2, b4 }, { 1, b2 }, { 1, b4 }, { 1, W0 }, { 1, b1 }, { 1, W0 }, 
        { 5, b1 }, { 1, b3 }, { 1, RST }, { 1, CRLF },
    { 1, r1 }, { 4, W0 }, { 1, r4 }, { 1, r1 }, { 4, W0 }, { 1, r3 }, { 1, r2 }, 
        { 4, W0 }, { 1, r3 }, { 3, W0 }, { 1, b1 }, { 1, b4 }, { 1, W0 }, { 1, b3 }, 
        { 1, b1 }, { 1, b3 }, { 1, W0 }, { 1, b4 }, { 1, b1 }, { 1, W0 }, { 1, b1 }, 
        { 2, b4 }, { 1, b1 }, { 4, b4 }, { 1, W0 }, { 1, b1 }, { 1, b3 }, { 2, W0 }, 
        { 1, b2 }, { 1, b3 }, { 1, W0 }, { 1, b1 }, { 1, b3 }, { 2, b4 }, { 1, b1 }, 
        { 1, b3 }, { 1, b4 }, { 1, b1 }, { 4, b4 }, { 1, W0 }, { 1, b1 }, { 3, W0 }, 
        { 1, b2 }, { 1, b4 }, { 1, W0 }, { 1, b1 }, { 1, W0 }, { 1, b1 }, { 1, b3 }, 
        { 2, b4 }, { 2, W0 }, { 1, RST }, { 1, CRLF },
    { 5, W0 }, { 1, r3 }, { 1, r4 }, { 4, W0 }, { 1, r1 }, { 1, r4 }, { 8, W0 }, 
        { 1, b1 }, { 1, b4 }, { 2, W0 }, { 1, b3 }, { 3, W0 }, { 1, b1 }, { 1, W0 }, 
        { 1, b1 }, { 1, b4 }, { 1, W0 }, { 5, b1 }, { 1, W0 }, { 1, b1 }, { 1, b4 }, 
        { 2, W0 }, { 2, b3 }, { 1, W0 }, { 1, b2 }, { 4, b1 }, { 2, W0 }, { 5, b1 }, 
        { 1, W0 }, { 1, b1 }, { 3, W0 }, { 1, b2 }, { 1, b4 }, { 1, W0 }, { 1, b1 }, 
        { 1, W0 }, { 1, b1 }, { 5, W0 }, { 1, RST }, { 1, CRLF },
    { 4, W0 }, { 1, r2 }, { 2, r4 }, { 1, r2 }, { 3, W0 }, { 1, r3 }, { 1, r4 }, 
        { 1, r3 }, { 61, W0 }, { 1, RST }, { 1, CRLF },
    { 4, W0 }, { 4, r4 }, { 2, W0 }, { 1, r1 }, { 3, r4 }, { 1, r1 }, { 60, W0 }, 
        { 1, RST }, { 1, CRLF },
    { 3, W0 }, { 5, r4 }, { 1, r3 }, { 1, r1 }, { 5, r4 }, { 60, W0 }, { 1, RST }, 
        { 1, CRLF },
    { 3, W0 }, { 1, r2 }, { 11, r4 }, { 60, W0 }, { 1, RST }, { 1, CRLF },
    { 4, W0 }, { 1, r1 }, { 1, r3 }, { 7, r4 }, { 1, r3 }, { 61, W0 }, { 1, RST }, 
        {1, CRLF}
};

bool HSS_LogoInit(void)
{
    mHSS_PUTS(CRLF);
    int i;

    // decode and output our RLE Logo
    for (i = 0; i < mSPAN_OF(rleLogoElements); i++) {
        uint8_t j;

        for (j = 0u; j < rleLogoElements[i].count; j++) {
            mHSS_PUTS(rleLogoElements[i].token);
        }
    }
    return true;
}
