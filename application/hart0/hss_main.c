/******************************************************************************************
 *
 * MPFS HSS Embedded Software
 *
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
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
#include "hss_registry.h"

#include "wdog_service.h"

#include "csr_helper.h"

#include <string.h>

/******************************************************************************************/

void hss_main(void);


void hss_main(void)
{
    if (IS_ENABLED(CONFIG_SERVICE_WDOG)) {
        HSS_Wdog_MonitorHart(HSS_HART_ALL);
    }

    while (true) {
        RunStateMachines(spanOfPGlobalStateMachines, pGlobalStateMachines);
    }
}

int main(int argc, char **argv)
{
    (void)argc; // unused
    (void)argv; // unused

    HSS_Init();

    if (current_hartid() != 0) {
        sbi_hart_hang();
    }

    hss_main();

    // will never be reached
    __builtin_unreachable();

    return 0;
}
