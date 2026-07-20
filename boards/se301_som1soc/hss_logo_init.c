/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
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

#include "hss_init.h"
#include "hss_debug.h"

//
// A variety of colored pixels are needed - red, white, black
// define these as ASCII characters if color output is not enabled
//
#if IS_ENABLED(CONFIG_COLOR_OUTPUT)
#  if IS_ENABLED(CONFIG_LOGO_INVERT_COLORS)
  const char B0_str[] ="\033[48;5;188m ";
  const char W0_str[] ="\033[0m ";
  const char r1_str[] ="\033[48;5;217m ";
  const char r2_str[] ="\033[48;5;210m ";
  const char r3_str[] ="\033[48;5;203m ";
  const char r4_str[] ="\033[48;5;196m ";
  const char b1_str[] ="\033[48;5;188m ";
  const char b2_str[] ="\033[48;5;145m ";
  const char b3_str[] ="\033[48;5;102m ";
  const char b4_str[] ="\033[48;5;59m ";
#  else
  const char B0_str[] = "\033[48;5;188m ";
  const char W0_str[] = "\033[48;5;188m ";
  const char r1_str[] ="\033[48;5;217m ";
  const char r2_str[] ="\033[48;5;210m ";
  const char r3_str[] ="\033[48;5;203m ";
  const char r4_str[] ="\033[48;5;196m ";
  const char b1_str[] ="\033[48;5;16m ";
  const char b2_str[] ="\033[48;5;59m ";
  const char b3_str[] ="\033[48;5;102m ";
  const char b4_str[] ="\033[48;5;145m ";
#  endif
  const char RST_str[] ="\033[0m";
#else
  const char B0_str[] =" ";
  const char W0_str[] =" ";
  const char r1_str[] =".";
  const char r2_str[] ="-";
  const char r3_str[] ="x";
  const char r4_str[] ="X";
  const char b1_str[] =":";
  const char b2_str[] ="o";
  const char b3_str[] ="0";
  const char b4_str[] ="O";
  const char RST_str[] ="";
#endif

enum Color {
    B0 = 0,
    W0,
    r1,
    r2,
    r3,
    r4,
    b1,
    b2,
    b3,
    b4,
    RST,
    CRLF_token,
};

const char* tokenStringTable[] = {
    B0_str,
    W0_str,
    r1_str,
    r2_str,
    r3_str,
    r4_str,
    b1_str,
    b2_str,
    b3_str,
    b4_str,
    RST_str,
    "\n",
};


// RLE Microchip Logo, built up from our color pixel primitives above...
// RLE shrinks the size of this
const struct __attribute__((packed))  {
    uint8_t const count;
    enum Color const tokenIndex;
} rleLogoElements[] = {
    { 4, W0 }, { 1, r1 }, { 1, r3 }, { 7, r4 }, { 62, W0 }, { 1, RST }, { 1, CRLF_token },

    { 3, W0 }, { 1, r2 }, { 9, r4 }, { 1, r3 }, { 61, W0 }, { 1, RST }, { 1, CRLF_token },

    { 2, W0 }, { 1, r3 }, { 2, r4 }, { 1, r3 }, { 1, r2 }, { 5, r4 }, { 1, r2 },
        { 1, r4 }, { 1, r1 }, { 7, W0 }, { 1, b3 }, { 5, W0 }, { 1, b3 }, { 46, W0 },
        { 1, RST }, { 1, CRLF_token },

    { 1, W0 }, { 1, r1 }, { 3, r4 }, { 2, B0 }, { 1, r2 }, { 3, r4 }, { 1, r1 },
        { 1, B0 }, { 1, r1 }, { 1, r4 }, { 6, W0 }, { 1, b4 }, { 1, b1 }, { 1, b2 },
        { 3, W0 }, { 1, b3 }, { 1, b1 }, { 1, b4 }, { 45, W0 }, { 1, RST },
        { 1, CRLF_token },

    { 1, W0 }, { 3, r4 }, { 1, r1 }, { 3, B0 }, { 2, r4 }, { 1, r2 }, { 3, B0 },
        { 2, r3 }, { 5, W0 }, { 1, b4 }, { 2, b1 }, { 3, W0 }, { 2, b1 }, { 1, b4 },
        { 1, W0 }, { 1, b4 }, { 2, W0 }, { 1, b4 }, { 3, b3 }, { 1, b4 }, { 1, W0 },
        { 5, b4 }, { 2, W0 }, { 1, b4 }, { 3, b3 }, { 1, b4 }, { 2, W0 }, { 1, b4 },
        { 3, b3 }, { 1, b4 }, { 1, W0 }, { 1, b4 }, { 3, W0 }, { 1, b4 }, { 2, W0 },
        { 1, b4 }, { 1, W0 }, { 5, b4 }, { 1, W0 }, { 1, RST }, { 1, CRLF_token },

    { 1, r1 }, { 3, r4 }, { 4, B0 }, { 1, r1 }, { 1, r4 }, { 5, B0 }, { 1, r4 },
        { 1, r1 }, { 4, W0 }, { 1, b3 }, { 2, b1 }, { 1, b4 }, { 1, W0 }, { 1, b4 },
        { 2, b1 }, { 1, b3 }, { 1, W0 }, { 1, b1 }, { 1, b4 }, { 1, W0 }, { 5, b1 },
        { 1, W0 }, { 5, b1 }, { 1, b3 }, { 1, W0 }, { 5, b1 }, { 1, b4 }, { 1, W0 },
        { 5, b1 }, { 1, W0 }, { 1, b1 }, { 3, W0 }, { 1, b2 }, { 1, b4 }, { 1, W0 },
        { 1, b1 }, { 1, W0 }, { 5, b1 }, { 1, b3 }, { 1, RST }, { 1, CRLF_token },

    { 1, r2 }, { 2, r4 }, { 1, r3 }, { 1, r2 }, { 4, B0 }, { 1, r3 }, { 1, r2 },
        { 4, B0 }, { 1, r2 }, { 1, r4 }, { 4, W0 }, { 1, b3 }, { 3, b2 }, { 1, W0 },
        { 2, b2 }, { 1, b3 }, { 1, b2 }, { 1, W0 }, { 1, b1 }, { 2, b4 }, { 1, b1 },
        { 5, W0 }, { 1, b1 }, { 1, b4 }, { 2, W0 }, { 1, b3 }, { 1, b2 }, { 1, b4 },
        { 1, b1 }, { 3, W0 }, { 1, b2 }, { 1, b3 }, { 1, b4 }, { 1, b1 }, { 5, W0 },
        { 1, b1 }, { 1, b4 }, { 2, W0 }, { 1, b2 }, { 1, b4 }, { 1, W0 }, { 1, b1 },
        { 1, W0 }, { 1, b1 }, { 1, b4 }, { 2, W0 }, { 1, b3 }, { 1, b2 }, { 1, RST },
        { 1, CRLF_token },

    { 1, r3 }, { 1, r4 }, { 1, r1 }, { 1, B0 }, { 1, r4 }, { 5, B0 }, { 1, r4 },
        { 5, B0 }, { 1, r4 }, { 1, r2 }, { 3, W0 }, { 1, b2 }, { 1, b3 }, { 1, W0 },
        { 1, b1 }, { 1, b3 }, { 1, b1 }, { 1, b4 }, { 1, b3 }, { 1, b2 }, { 1, W0 },
        { 1, b1 }, { 2, b4 }, { 1, b2 }, { 5, W0 }, { 1, b1 }, { 1, b3 }, { 2, W0 },
        { 2, b3 }, { 1, b4 }, { 1, b1 }, { 3, W0 }, { 1, b2 }, { 1, b3 }, { 1, b4 },
        { 1, b2 }, { 5, W0 }, { 5, b1 }, { 1, b4 }, { 1, W0 }, { 1, b1 }, { 1, W0 },
        { 1, b1 }, { 1, b4 }, { 2, W0 }, { 1, b3 }, { 1, b2 }, { 1, RST },
        { 1, CRLF_token },

    { 1, r3 }, { 1, r2 }, { 2, B0 }, { 1, r2 }, { 1, r3 }, { 4, B0 }, { 1, r1 },
        { 1, r3 }, { 4, B0 }, { 1, r1 }, { 1, r4 }, { 3, W0 }, { 1, b1 }, { 1, b3 },
        { 1, W0 }, { 3, b1 }, { 1, W0 }, { 1, b4 }, { 1, b1 }, { 1, W0 }, { 1, b1 },
        { 2, b4 }, { 1, b2 }, { 5, W0 }, { 5, b1 }, { 1, W0 }, { 1, b4 }, { 1, b1 },
        { 3, W0 }, { 1, b2 }, { 1, b3 }, { 1, b4 }, { 1, b2 }, { 5, W0 }, { 1, b1 },
        { 1, b3 }, { 2, b4 }, { 1, b2 }, { 1, b4 }, { 1, W0 }, { 1, b1 }, { 1, W0 },
        { 5, b1 }, { 1, b3 }, { 1, RST }, { 1, CRLF_token },

    { 1, r1 }, { 4, B0 }, { 1, r4 }, { 1, r1 }, { 4, B0 }, { 1, r3 }, { 1, r2 },
        { 4, B0 }, { 1, r3 }, { 3, W0 }, { 1, b1 }, { 1, b4 }, { 1, W0 }, { 1, b3 },
        { 1, b1 }, { 1, b3 }, { 1, W0 }, { 1, b4 }, { 1, b1 }, { 1, W0 }, { 1, b1 },
        { 2, b4 }, { 1, b1 }, { 4, b4 }, { 1, W0 }, { 1, b1 }, { 1, b3 }, { 2, W0 },
        { 1, b2 }, { 1, b3 }, { 1, W0 }, { 1, b1 }, { 1, b3 }, { 2, b4 }, { 1, b1 },
        { 1, b3 }, { 1, b4 }, { 1, b1 }, { 4, b4 }, { 1, W0 }, { 1, b1 }, { 3, W0 },
        { 1, b2 }, { 1, b4 }, { 1, W0 }, { 1, b1 }, { 1, W0 }, { 1, b1 }, { 1, b3 },
        { 2, b4 }, { 2, W0 }, { 1, RST }, { 1, CRLF_token },

    { 5, B0 }, { 1, r3 }, { 1, r4 }, { 4, B0 }, { 1, r1 }, { 1, r4 }, { 5, B0 }, { 3, W0 },
        { 1, b1 }, { 1, b4 }, { 2, W0 }, { 1, b3 }, { 3, W0 }, { 1, b1 }, { 1, W0 },
        { 1, b1 }, { 1, b4 }, { 1, W0 }, { 5, b1 }, { 1, W0 }, { 1, b1 }, { 1, b4 },
        { 2, W0 }, { 2, b3 }, { 1, W0 }, { 1, b2 }, { 4, b1 }, { 2, W0 }, { 5, b1 },
        { 1, W0 }, { 1, b1 }, { 3, W0 }, { 1, b2 }, { 1, b4 }, { 1, W0 }, { 1, b1 },
        { 1, W0 }, { 1, b1 }, { 5, W0 }, { 1, RST }, { 1, CRLF_token },

    { 1, W0 }, { 3, B0 }, { 1, r2 }, { 2, r4 }, { 1, r2 }, { 3, B0 }, { 1, r3 }, { 1, r4 },
        { 1, r3 }, {2, B0 }, {1, B0}, { 58, W0 }, { 1, RST }, { 1, CRLF_token },

    { 1, W0 }, {1, r1}, { 2, B0 }, { 4, r4 }, { 2, B0 }, { 1, r1 }, { 3, r4 }, { 1, r1 },
        {1, B0 }, {1, r1}, { 58, W0 }, { 1, RST }, { 1, CRLF_token },

    { 2, W0 }, {1, r2},  { 5, r4 }, { 1, r3 }, { 1, r1 }, { 5, r4 }, {1, r3}, { 59, W0 },
        { 1, RST }, { 1, CRLF_token },

    { 3, W0 }, { 1, r2 }, { 11, r4 }, { 60, W0 }, { 1, RST }, { 1, CRLF_token },

    { 4, W0 }, { 1, r1 }, { 1, r3 }, { 7, r4 }, { 1, r3 }, { 61, W0 }, { 1, RST }, {1, CRLF_token}
};

bool HSS_LogoInit(void)
{
    mHSS_PUTS("\n");
    int i;

    // decode and output our RLE Logo
    for (i = 0; i < ARRAY_SIZE(rleLogoElements); i++) {
        uint8_t j;

        for (j = 0u; j < rleLogoElements[i].count; j++) {
            mHSS_PUTS(tokenStringTable[rleLogoElements[i].tokenIndex]);
        }
    }

    mHSS_PUTS("\n"
        "-------------------------\n"
        "-- Custom Board Design --\n"
        "-- PolarFire SoC FPGA  --\n"
        "-------------------------\n"
        "\n");

    return true;
}
