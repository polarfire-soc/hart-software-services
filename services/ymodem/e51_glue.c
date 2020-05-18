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
 *\file E51 Glue for ymodem
 *\brief E51 Glue for ymodem
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

#include "config/hardware/hw_platform.h"
#include "config/hardware/clocks/hw_mss_clks.h"
#undef ROUNDUP
#undef ROUNDDOWN
//#include "mss_hal.h"
#include "mss_util.h"
#include "drivers/mss_uart/mss_uart.h"
#include "uart_helper.h"
#include "ymodem.h"
#include "drivers/mss_envm/mss_envm.h"
#include "drivers/mss_sys_services/mss_sys_services.h"
#include "mss_sysreg.h"
//#include "baremetal/drivers/micron_mt25q/micron_mt25q.h"
#include "baremetal/drivers/winbond_w25n01gv/winbond_w25n01gv.h"


int ee_vsprintf(char *buf, const char *fmt, va_list args);


//
// Local prototypes
//
static int l_sprintf(char *buf, const char * const fmt, ...);
static void l_print_result(uint8_t result, const char *msg);


//
//

static int l_sprintf(char *buf, const char * const fmt, ...)
{
    int result;
    va_list args;
    va_start(args, fmt);
    result = ee_vsprintf(buf, fmt, args);
    va_end(args);
    return result;
}

mss_uart_instance_t *g_uart = &g_mss_uart0_lo;

static void l_print_result(uint8_t result, const char *msg)
{
    static char buffer[256];

    static const char *errorTable[] = {
        "Success" CRLF,
        "Verify Error" CRLF,
        "Bounds Check Error" CRLF,
        "Page Latch Load Error" CRLF,
        "Startup Failed Previously Error" CRLF,
        "Sector Outside Bounds Error" CRLF,
    };

    mHSS_DEBUG_PRINTF_EX("%s returned - ", msg);

    if (result < mSPAN_OF(errorTable)) {
        mHSS_DEBUG_PRINTF_EX(errorTable[result]);
    } else {
        l_sprintf(buffer, (const char * const)"Unknown Error (%u)" CRLF,
                (unsigned int) result);
        mHSS_DEBUG_PRINTF_EX(buffer);
    }
}

//static uint8_t l_envm_params[256];
static void l_e51_qspi_init(void)
{
    static bool initialized = false;

    if (!initialized) {
        MSS_SYS_select_service_mode(MSS_SYS_SERVICE_POLLING_MODE, NULL);

        MSS_QSPI_init();
        MSS_QSPI_enable();
        Flash_init(MSS_QSPI_NORMAL);
        initialized = true;
    }
}

void e51_ymodem_loop(void)
{
    uint8_t rx_byte;
    bool done = false;

    uint32_t received = 0u;
    extern uint64_t __ddr_start;
    uint8_t *pBuffer = (uint8_t *)&__ddr_start;
    uint32_t g_rx_size = 1024 * 1024 * 10u;

    while (!done) {
        static const char menuText[] = CRLF "QSPI Utility" CRLF \
            " 1. QSPI Erase Bulk -- erase all sectors" CRLF \
            " 2. YMODEM Receive -- receive application file" CRLF \
            " 3. QSPI Write -- write application file to the Device" CRLF \
            " 4. Quit -- quit QSPI Utility " CRLF CRLF \
            " Press a number (1--4):" CRLF;

        mHSS_PUTS(menuText);

        if (uart_getchar(&rx_byte, -1, false)) {
            switch (rx_byte) {
            case '1':
                mHSS_PUTS(CRLF "Erasing all of QSPI" CRLF );
                l_e51_qspi_init();
                l_print_result(0, "Flash_init()");
                Flash_erase();
                l_print_result(0, "Flash_erase()");
                break;

            case '2':
                mHSS_PUTS(CRLF "Attempting to receive .bin file using YMODEM (CTRL-C to cancel)" 
                    CRLF);
                received = ymodem_receive(pBuffer, g_rx_size);
                if (received == 0) {
                    mHSS_PUTS(CRLF "YMODEM failed to receive file successfully" CRLF CRLF);
                }
                break;

            case '3':
                mHSS_PUTS(CRLF "Attempting to flash received data" CRLF);
                l_e51_qspi_init();
                Flash_program((uint8_t *)pBuffer, 0, received);
                break;

            case '4':
                done = true;
                break;

            default:
                break;
            }
        }
    }
}
