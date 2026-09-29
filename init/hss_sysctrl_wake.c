/*******************************************************************************
 * Copyright 2019-2026 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file HSS System Controller Wake stub
 * \brief Default (no-op) implementation of HSS_SysCtrl_Wake().
 *
 * Boards that need to assert a GPIO wake signal to the System Controller
 * should provide a strong definition of this function in their board-specific
 * code (e.g. boards/<board>/hss_board_init.c).
 * The strong definition will silently replace this weak stub at link time.
 */

#include "config.h"
#include "hss_types.h"
#include "hss_init.h"

__attribute__((weak)) bool HSS_SysCtrl_Wake(void)
{
    return true;
}
