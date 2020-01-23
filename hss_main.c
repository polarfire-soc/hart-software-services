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
 *\file Main Entrypoint
 *\brief Main Entrypoint
 */

#include "config.h"
#include "hss_types.h"

#include <assert.h>

#include "hss_state_machine.h"

#include "hss_debug.h"
#include "ssmb_ipi.h"

#include "hss_init.h"

#ifndef CONFIG_OPENSBI
#  include "machine/encoding.h"
#  include "machine/bits.h"
#endif

#ifdef CONFIG_SERVICE_WDOG
#  include "wdog_service.h"
#endif

#include "csr_helper.h"

#include <string.h>

/******************************************************************************************************/

void hss_main(void)
{
    HSS_Init();

#ifdef CONFIG_SERVICE_WDOG
    HSS_Wdog_MonitorHart(HSS_HART_ALL);
#endif

    while (1) {
        RunStateMachines();
    }
}

int main(int argc, char **argv)
{
    (void)argc; // unused
    (void)argv; // unused

    HSS_Init_Setup_RWDATA_And_BSS();

#ifdef CONFIG_SUPERLOOP_IN_U_MODE
    CSR_Init();

    if (CSR_GetHartId() != 0) {
        while(1);
    }

    // set MSTATUS.MPP to Supervisor mode, and set MSTATUS.MPIE to 1
    uint32_t mstatus_val = read_csr(mstatus);

    // immediate_arg stores the desired privilege mode to return to...
    // typically PRV_S
    mstatus_val = INSERT_FIELD(mstatus_val, MSTATUS_MPP, PRV_U);
    mstatus_val = INSERT_FIELD(mstatus_val, MSTATUS_MPIE, 1);
    write_csr(mstatus, mstatus_val);

    // set MEPC to function address
    write_csr(mepc, (void *)hss_main);

    // execute MRET, 
    // causing MIE <= MPIE, new priv mode <= PRV_U, MPIE <= 1, MPP <= U
    asm("mret");
#else
    if (CSR_GetHartId() != 0) {
        while(1);
    }
    hss_main();
#endif

    // will never be reached
    return 0;
}
