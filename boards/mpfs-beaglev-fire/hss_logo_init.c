/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
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

bool HSS_LogoInit(void)
{
    mHSS_PUTS("\n"
        "---------------------------------\n"
        "--        BeagleV-Fire         --\n"
        "---------------------------------\n"
        "\n");

    return true;
}

