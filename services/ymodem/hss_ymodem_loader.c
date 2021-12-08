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
 *\file E51 Glue for ymodem
 *\brief E51 Glue for ymodem
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"
#include "ddr_service.h"

#include <string.h>
#include <sys/types.h>

#include "fpga_design_config/fpga_design_config.h"
#include "fpga_design_config/clocks/hw_mss_clks.h"
#undef ROUNDUP
#undef ROUNDDOWN
#include "mss_util.h"
#include "drivers/mss_uart/mss_uart.h"
#include "uart_helper.h"
#include "ymodem.h"
#include "drivers/mss_sys_services/mss_sys_services.h"
#include "mss_sysreg.h"

#if IS_ENABLED(CONFIG_SERVICE_QSPI)
#  include "qspi_service.h"
#if IS_ENABLED(CONFIG_MT15Q)
#  include "baremetal/drivers/micron_mt25q/micron_mt25q.h"
#else
#  include "baremetal/drivers/winbond_w25n01gv/winbond_w25n01gv.h"
#endif
#endif

#if IS_ENABLED(CONFIG_SERVICE_MMC)
#  include "mmc_service.h"
#  include "mss_mmc.h"
#endif


//
// Local prototypes
//
#if IS_ENABLED(CONFIG_SERVICE_QSPI)
static bool hss_loader_qspi_init(void);
static bool hss_loader_qspi_program(uint8_t *pBuffer, size_t wrAddr, size_t receivedCount);
bool hss_loader_qspi_erase(void);
#endif

#if IS_ENABLED(CONFIG_SERVICE_MMC)
static bool hss_loader_mmc_init(void);
static bool hss_loader_mmc_program(uint8_t *pBuffer, size_t wrAddr, size_t receivedCount);
#endif

#if IS_ENABLED(CONFIG_SERVICE_QSPI)
static bool hss_loader_qspi_init(void)
{
    static bool initialized = false;
    bool result = true;

    if (!initialized) {
        result = HSS_QSPIInit();

        Flash_init(MSS_QSPI_NORMAL);
        initialized = true;
    }
    return result;
}

static bool hss_loader_qspi_program(uint8_t *pBuffer, size_t wrAddr, size_t receivedCount)
{
    bool result = HSS_QSPI_WriteBlock(wrAddr, pBuffer, receivedCount);
    return result;
}

bool hss_loader_qspi_erase(void)
{
    Flash_erase();
    return true;
}
#endif

#if IS_ENABLED(CONFIG_SERVICE_MMC)
bool hss_loader_mmc_init(void)
{
    bool result = false;

    SYSREG->SUBBLK_CLOCK_CR |= (uint32_t)SUBBLK_CLOCK_CR_MMC_MASK;
    SYSREG->SOFT_RESET_CR   |= (uint32_t)SOFT_RESET_CR_MMC_MASK;
    SYSREG->SOFT_RESET_CR   &= ~(uint32_t)SOFT_RESET_CR_MMC_MASK;

    result = HSS_MMCInit();

    return result;
}

bool hss_loader_mmc_program(uint8_t *pBuffer, size_t wrAddr, size_t receivedCount)
{
    bool result = HSS_MMC_WriteBlock(wrAddr, pBuffer, receivedCount);
    return result;
}
#endif

#define U_BOOT_OFFSET 0x400000

void hss_loader_ymodem_loop(void);
void hss_loader_ymodem_loop(void)
{
    uint8_t rx_byte;
    bool done = false;
    bool result = false;
    int offset = 0;

    uint32_t receivedCount = 0u;
    uint8_t *pBuffer = 0x10000000 + (uint8_t *)HSS_DDR_GetStart();
    uint32_t g_rx_size = HSS_DDR_GetSize();

    while (!done) {
        static const char menuText[] = CRLF
#if IS_ENABLED(CONFIG_SERVICE_QSPI)
            "QSPI"
#endif
#if IS_ENABLED(CONFIG_SERVICE_QSPI) && IS_ENABLED(CONFIG_SERVICE_MMC)
           "/"
#endif
#if IS_ENABLED(CONFIG_SERVICE_MMC)
           "MMC"
#endif
           " Utility" CRLF
#if IS_ENABLED(CONFIG_SERVICE_QSPI)
            " 1. QSPI Erase Bulk -- erase all sectors" CRLF
#endif
#if IS_ENABLED(CONFIG_SERVICE_MMC)
            " 2. MMC Init -- initialize MMC driver" CRLF
#endif
            " 3. YMODEM Receive -- receive application file" CRLF
#if IS_ENABLED(CONFIG_SERVICE_QSPI)
            " 4. QSPI Write -- write application file to the Device" CRLF
#endif
#if IS_ENABLED(CONFIG_SERVICE_MMC)
            " 5. MMC Write -- write application file to the Device" CRLF
#endif
            " 6. Quit -- quit QSPI Utility" CRLF CRLF
            " Select a number:" CRLF;

        mHSS_PRINTF(CRLF "hss_loader_ymodem_loop pBuffer %p, g_rx_size = %u offset %d" CRLF, pBuffer, g_rx_size, offset);
        mHSS_PUTS(menuText);

        if (uart_getchar(&rx_byte, -1, false)) {
            switch (rx_byte) {
#if IS_ENABLED(CONFIG_SERVICE_QSPI)
            case '1':
                mHSS_PUTS(CRLF "Initializing QSPI ... ");
                result = hss_loader_qspi_init();

                if (result) {
                    mHSS_PUTS(" Success" CRLF);

                    mHSS_PUTS(CRLF "Erasing all of QSPI ... ");
#ifdef OUT
                    result = hss_loader_qspi_erase();

                    if (result) {
                        mHSS_PUTS(" Success" CRLF);
                    }
#else
                    mHSS_PUTS(" is disabled " CRLF);
#endif
                }

                if (!result) {
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                    mHSS_PUTS(" FAILED" CRLF);
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                }
                break;
#endif

#if IS_ENABLED(CONFIG_SERVICE_MMC)
            case '2':
                mHSS_PUTS(CRLF "Initializing MMC ... ");
                result = hss_loader_mmc_init();

                if (result) {
                    mHSS_PUTS(" Success" CRLF);
                }

                if (!result) {
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                    mHSS_PUTS(" FAILED" CRLF);
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                }
                break;
#endif
            case '3':
                mHSS_PUTS(CRLF "Attempting to receive .bin file using YMODEM (CTRL-C to cancel)"
                    CRLF);
                receivedCount = ymodem_receive(pBuffer, g_rx_size);
                if (receivedCount == 0) {
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                    mHSS_PUTS(CRLF "YMODEM failed to receive file successfully" CRLF CRLF);
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                }
                break;

#if IS_ENABLED(CONFIG_SERVICE_QSPI)
            case '4':
                mHSS_PRINTF(CRLF "Attempting to flash received data (%u bytes)" CRLF, receivedCount);
                mHSS_PUTS(CRLF "Initializing QSPI ... ");
                result = hss_loader_qspi_init();

                if (result) {
                    mHSS_PUTS(" Success" CRLF);

                    mHSS_PUTS(CRLF "Programming QSPI ... ");
                    result = hss_loader_qspi_program((uint8_t *)pBuffer, U_BOOT_OFFSET, receivedCount);

                    if (result) {
                        mHSS_PUTS(" Success" CRLF);
                    }
                }

                if (!result) {
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                    mHSS_PUTS(" FAILED" CRLF);
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                }
                break;
#endif

#if IS_ENABLED(CONFIG_SERVICE_MMC)
            case '5':
                mHSS_PRINTF(CRLF "Attempting to flash received data (%u bytes)" CRLF, receivedCount);
                mHSS_PUTS(CRLF "Initializing MMC ... ");
                result = hss_loader_mmc_init();

                if (result) {
                    mHSS_PUTS(" Success" CRLF);

                    mHSS_PUTS(CRLF "Programming MMC ... ");
                    result = hss_loader_mmc_program((uint8_t *)pBuffer, 0u, receivedCount);

                    if (result) {
                        mHSS_PUTS(" Success" CRLF);
                    }
                }

                if (!result) {
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                    mHSS_PUTS(" FAILED" CRLF);
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                }
                break;
#endif

            case '6':
                done = true;
                break;

            case 'r':
                mHSS_PUTS(CRLF "QSPI READ ... " CRLF CRLF);
                mHSS_PUTS(CRLF "Initializing QSPI ... " CRLF);
                result = hss_loader_qspi_init();

                if (result) {
                    mHSS_PUTS(" Success" CRLF);

                    mHSS_PUTS(CRLF "Reading all of QSPI ... ");
                    result = 1;
                    memset(pBuffer, 0x0, 0x200);
                    mHSS_PRINTF(CRLF "Before %02x %02x %02x" CRLF, pBuffer[0], pBuffer[1], pBuffer[2]);
                    Flash_read((uint8_t *)pBuffer, U_BOOT_OFFSET + offset, 256);
                    offset += 256;

                    if (result) {
                        mHSS_PUTS(" Success" CRLF);
                        mHSS_PRINTF(CRLF "%02x %02x %02x" CRLF, pBuffer[0], pBuffer[1], pBuffer[2]);
                    }
                }

                if (!result) {
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                    mHSS_PUTS(" FAILED" CRLF);
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                }
                break;
            case 'w':
                mHSS_PUTS(CRLF "QSPI WRITE ... " CRLF CRLF);
                mHSS_PUTS(CRLF "Initializing QSPI ... ");
                result = hss_loader_qspi_init();

                if (result) {
                    mHSS_PUTS(" Success" CRLF);

                    mHSS_PUTS(CRLF "Writeing to QSPI ... ");
                    memset(pBuffer, 0xe, 0x200);
                    mHSS_PRINTF(CRLF "%02x %02x" CRLF, pBuffer[0], pBuffer[1]);
                    result = hss_loader_qspi_program((uint8_t *)pBuffer, U_BOOT_OFFSET, 0x100);
                    if (result) {
                        mHSS_PUTS(" Success" CRLF);
                    }
                }

                if (!result) {
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                    mHSS_PUTS(" FAILED" CRLF);
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                }
                break;
            case 'e':
                mHSS_PUTS(CRLF "QSPI ERASE ... " CRLF CRLF);
                mHSS_PUTS(CRLF "Initializing QSPI ... ");
                result = hss_loader_qspi_init();

                if (result) {
                    mHSS_PUTS(" Success" CRLF);

                    mHSS_PUTS(CRLF "Erasing all of QSPI ... ");
#if IS_ENABLED(CONFIG_MT15Q)
                    Flash_sector_erase(U_BOOT_OFFSET);
#endif
                    result = 1; //hss_loader_qspi_erase();

                    if (result) {
                        mHSS_PUTS(" Success" CRLF);
                    }
                }

                if (!result) {
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                    mHSS_PUTS(" FAILED" CRLF);
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                }
                break;
            case 'd':
                {
                    uint32_t *pBuffer32 = (uint32_t *)(pBuffer + offset);
                    int i;
                    mHSS_PUTS(CRLF "Dump memory ... " CRLF CRLF);
                    for ( i = 0; i < 0x100 /4; i = i + 4) {
                        mHSS_PRINTF(CRLF "%08x: %08x %08x %08x %08x" CRLF, &pBuffer32[i], pBuffer32[i],
                        pBuffer32[i + 1], pBuffer32[i + 2], pBuffer32[i + 3]);
                    }
                    offset += 256;
                }
            default: // ignore
                break;
	    }
        }
    }
}
