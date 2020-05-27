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
#include "ddr_service.h"

#include <string.h>
#include <sys/types.h>

#include "config/hardware/hw_platform.h"
#include "config/hardware/clocks/hw_mss_clks.h"
#undef ROUNDUP
#undef ROUNDDOWN
#include "mss_util.h"
#include "drivers/mss_uart/mss_uart.h"
#include "uart_helper.h"
#include "ymodem.h"
#include "drivers/mss_sys_services/mss_sys_services.h"
#include "mss_sysreg.h"

#ifdef CONFIG_SERVICE_QSPI
#  include "qspi_service.h"
//#  include "baremetal/drivers/micron_mt25q/micron_mt25q.h"
#  include "baremetal/drivers/winbond_w25n01gv/winbond_w25n01gv.h"
#endif

#ifdef CONFIG_SERVICE_EMMC
#  include "emmc_service.h"
#  include "mss_mmc.h"
#endif


//
// Local prototypes
//
static void printResult_(uint8_t result, const char *msg);
#ifdef CONFIG_SERVICE_QSPI
static bool e51_qspi_init(void);
static bool e51_qspi_program(uint8_t *pBuffer, size_t wrAddr, size_t receivedCount);
static bool e51_qspi_erase(void);
#endif

#ifdef CONFIG_SERVICE_EMMC
static bool e51_emmc_init(void);
static bool e51_emmc_program(uint8_t *pBuffer, size_t wrAddr, size_t receivedCount);
static bool e51_emmc_erase(void);
#endif

static void printResult_(uint8_t result, const char *msg)
{
    static char buffer[256];

    static const char *errorTable[] = {
        "Failure" CRLF,
        "Success" CRLF,
    };

    mHSS_PRINTF("%s returned - ", msg);

    if (result < mSPAN_OF(errorTable)) {
        mHSS_PRINTF(errorTable[result]);
    } else {
        mHSS_PRINTF(buffer, (const char * const)"Unknown Error (%u)" CRLF,
                (unsigned int) result);
    }
}

#ifdef CONFIG_SERVICE_QSPI
static bool e51_qspi_init(void)
{
    static bool initialized = false;
    bool result = false;

    if (!initialized) {
        result = HSS_QSPIInit();

        Flash_init(MSS_QSPI_NORMAL);
        initialized = true;
    }
    return result;
}

static bool e51_qspi_program(uint8_t *pBuffer, size_t wrAddr, size_t receivedCount)
{
    bool result = HSS_QSPI_WriteBlock(wrAddr, pBuffer, receivedCount);
    return result;
}

static bool e51_qspi_erase(void)
{
    Flash_erase();
    return true;
}
#endif

#ifdef CONFIG_SERVICE_EMMC
static bool e51_emmc_init(void)
{
    //static bool initialized = false;
    bool result = false;

    //if (!initialized) {
        SYSREG->SUBBLK_CLOCK_CR |= (uint32_t)SUBBLK_CLOCK_CR_MMC_MASK;
        SYSREG->SOFT_RESET_CR   |= (uint32_t)SOFT_RESET_CR_MMC_MASK;
        SYSREG->SOFT_RESET_CR   &= ~(uint32_t)SOFT_RESET_CR_MMC_MASK;

        //mHSS_PRINTF("SYSREG->SUBBLK_CLOCK_CR is 0x%x (MMC == 0x%x)" CRLF, SYSREG->SUBBLK_CLOCK_CR, SYSREG->SUBBLK_CLOCK_CR & SUBBLK_CLOCK_CR_MMC_MASK);
        //mHSS_PRINTF("SYSREG->SOFT_RESET_CR   is 0x%x (MMC == 0x%x)" CRLF, SYSREG->SOFT_RESET_CR, SYSREG->SOFT_RESET_CR & SOFT_RESET_CR_MMC_MASK);

        result = HSS_EMMCInit();
    //    initialized = true;
    //}
    return result;
}

static bool e51_emmc_program(uint8_t *pBuffer, size_t wrAddr, size_t receivedCount)
{
    bool result = HSS_EMMC_WriteBlock(wrAddr, pBuffer, receivedCount);
    return result;
}

static bool e51_emmc_erase(void)
{
    // dummy
    return true;
}
#endif

void e51_ymodem_loop(void)
{
    uint8_t rx_byte;
    bool done = false;
    bool result = false;

    uint32_t receivedCount = 0u;
    extern uint64_t __ddr_start;
    uint8_t *pBuffer = (uint8_t *)&__ddr_start;
    uint32_t g_rx_size = HSS_DDR_GetSize();

    while (!done) {
        static const char menuText[] = CRLF
#ifdef CONFIG_SERVICE_QSPI
            "QSPI"
#endif
#if defined(CONFIG_SERVICE_QSPI) && defined(CONFIG_SERVICE_EMMC)
           "/"
#endif
#ifdef CONFIG_SERVICE_EMMC
           "EMMC"
#endif
           " Utility" CRLF
#ifdef CONFIG_SERVICE_QSPI
            " 1. QSPI Erase Bulk -- erase all sectors" CRLF
#endif
#ifdef CONFIG_SERVICE_EMMC
            " 2. EMMC Erase Bulk -- erase all sectors" CRLF
#endif
            " 3. YMODEM Receive -- receive application file" CRLF
#ifdef CONFIG_SERVICE_QSPI
            " 4. QSPI Write -- write application file to the Device" CRLF
#endif
#ifdef CONFIG_SERVICE_EMMC
            " 5. EMMC Write -- write application file to the Device" CRLF
#endif
            " 6. Quit -- quit QSPI Utility " CRLF CRLF
            " Select a number:" CRLF;

        mHSS_PUTS(menuText);

        if (uart_getchar(&rx_byte, -1, false)) {
            switch (rx_byte) {
#ifdef CONFIG_SERVICE_QSPI
            case '1':
                mHSS_PUTS(CRLF "Erasing all of QSPI" CRLF );
                result = e51_qspi_init();
                printResult_(result, "e51_qspi_init()");
                result = e51_qspi_erase();
                printResult_(result, "e51_qspi_erase()");
                break;
#endif

#ifdef CONFIG_SERVICE_EMMC
            case '2':
                mHSS_PUTS(CRLF "Erasing all of EMMC" CRLF );
                result = e51_emmc_init();
                printResult_(result, "e51_emmc_init()");
                result = e51_emmc_erase();
                printResult_(result, "e51_emmc_erase()");
                break;
#endif
            case '3':
                mHSS_PUTS(CRLF "Attempting to receive .bin file using YMODEM (CTRL-C to cancel)"
                    CRLF);
                receivedCount = ymodem_receive(pBuffer, g_rx_size);
                if (receivedCount == 0) {
                    mHSS_PUTS(CRLF "YMODEM failed to receive file successfully" CRLF CRLF);
                }
                break;

#ifdef CONFIG_SERVICE_QSPI
            case '4':
                mHSS_PRINTF(CRLF "Attempting to flash received data (%u bytes)" CRLF, receivedCount);
                result = e51_qspi_init();
                printResult_(result, "e51_emmc_init()");
                result = e51_qspi_program((uint8_t *)pBuffer, 0, receivedCount);
                printResult_(result, "e51_qspi_program()");
                break;
#endif

#ifdef CONFIG_SERVICE_EMMC
            case '5':
                if (receivedCount ==0) { receivedCount = 1024; } //EMDALO
                mHSS_PRINTF(CRLF "Attempting to flash received data (%u bytes)" CRLF, receivedCount);
                result = e51_emmc_init();
                printResult_(result, "e51_emmc_init()");
                result = e51_emmc_program((uint8_t *)pBuffer, 0, receivedCount);
                printResult_(result, "e51_emmc_program()");
                break;
#endif

            case '6':
                done = true;
                break;

            default: // ignore
                break;
	    }
        }
    }
}
