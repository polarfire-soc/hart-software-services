/******************************************************************************************
 *
 * MPFS HSS Embedded Software
 *
 * Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
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
#include "drivers/mss/mss_mmuart/mss_uart.h"
#include "uart_helper.h"
#include "ymodem.h"
#include "drivers/mss/mss_sys_services/mss_sys_services.h"
#include "mss_sysreg.h"

#if IS_ENABLED(CONFIG_SERVICE_QSPI)
#  include "qspi_service.h"
#  if IS_ENABLED(CONFIG_SERVICE_QSPI_WINBOND_W25N01GV)
#    include "winbond_w25n01gv.h"
#  endif
#  if IS_ENABLED(CONFIG_SERVICE_QSPI_MICRON_MQ25T)
#    include "micron_mt25q.h"
#  endif
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
static bool hss_loader_qspi_erase(void);
#endif

#if IS_ENABLED(CONFIG_SERVICE_MMC)
static bool hss_loader_mmc_init(void);
static bool hss_loader_mmc_program(uint8_t *pBuffer, size_t wrAddr, size_t receivedCount);
#endif

#if IS_ENABLED(CONFIG_SERVICE_BOOT_SNVM)
static bool hss_loader_snvm_program(uint8_t *pBuffer, size_t receivedCount);
static bool hss_loader_snvm_verify(uint8_t *pBuffer, size_t receivedCount);
#endif

#if IS_ENABLED(CONFIG_SERVICE_QSPI)
static bool hss_loader_qspi_init(void)
{
    static bool initialized = false;
    bool result = false;

    if (!initialized) {
        result = HSS_QSPIInit();
        initialized = true;
    }
    return result;
}

static bool hss_loader_qspi_program(uint8_t *pBuffer, size_t wrAddr, size_t receivedCount)
{
    bool result = HSS_QSPI_WriteBlock(wrAddr, pBuffer, receivedCount);
    return result;
}

static bool hss_loader_qspi_erase(void)
{
    HSS_QSPI_FlashChipErase();
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

#if IS_ENABLED(CONFIG_SERVICE_BOOT_SNVM)
#define SNVM_PAGE_SIZE_NON_AUTH  252u
#define SNVM_MAX_PAGES           221u

static bool hss_loader_snvm_program(uint8_t *pBuffer, size_t receivedCount)
{
    uint8_t startPage = (uint8_t)CONFIG_SERVICE_BOOT_SNVM_START_PAGE;
    size_t pageCount = (receivedCount + SNVM_PAGE_SIZE_NON_AUTH - 1u) / SNVM_PAGE_SIZE_NON_AUTH;
    size_t maxPages = (size_t)CONFIG_SERVICE_BOOT_SNVM_PAGE_COUNT;
    uint16_t status;

    if (pageCount > maxPages) {
        mHSS_PRINTF("Error: data requires %lu pages but only %lu configured\n",
            pageCount, maxPages);
        return false;
    }

    if ((startPage + pageCount) > SNVM_MAX_PAGES) {
        mHSS_PRINTF("Error: pages %u-%lu exceed sNVM limit (%u)\n",
            startPage, startPage + pageCount - 1u, SNVM_MAX_PAGES);
        return false;
    }

    MSS_SYS_select_service_mode(MSS_SYS_SERVICE_POLLING_MODE, NULL);

    mHSS_PRINTF("Writing %lu bytes to sNVM pages %u-%lu ...\n",
        receivedCount, startPage, startPage + pageCount - 1u);

    uint8_t *pSrc = pBuffer;
    size_t remaining = receivedCount;

    for (size_t i = 0u; i < pageCount; i++) {
        uint8_t moduleIdx = startPage + (uint8_t)i;
        uint8_t pageData[SNVM_PAGE_SIZE_NON_AUTH];
        size_t chunkSize = (remaining >= SNVM_PAGE_SIZE_NON_AUTH) ?
            SNVM_PAGE_SIZE_NON_AUTH : remaining;

        /* Pad last partial page with 0xFF */
        memset(pageData, 0xFF, SNVM_PAGE_SIZE_NON_AUTH);
        memcpy(pageData, pSrc, chunkSize);

        status = MSS_SYS_secure_nvm_write(
            MSS_SYS_SNVM_NON_AUTHEN_TEXT_REQUEST_CMD,
            moduleIdx,
            pageData,
            NULL,   /* p_user_key: NULL for non-authenticated */
            0u      /* mb_offset */
        );

        if (status != MSS_SYS_SUCCESS) {
            mHSS_PRINTF("\nError: sNVM write page %u failed (status=%u)\n",
                moduleIdx, status);
            return false;
        }

        pSrc += chunkSize;
        remaining -= chunkSize;

        /* Progress indicator every 10 pages */
        if ((i % 10u) == 0u) {
            mHSS_PRINTF("  page %lu/%lu\n", i + 1u, pageCount);
        }
    }

    mHSS_PRINTF("sNVM write complete: %lu pages written\n", pageCount);
    return true;
}

static bool hss_loader_snvm_verify(uint8_t *pBuffer, size_t receivedCount)
{
    uint8_t startPage = (uint8_t)CONFIG_SERVICE_BOOT_SNVM_START_PAGE;
    size_t pageCount = (receivedCount + SNVM_PAGE_SIZE_NON_AUTH - 1u) / SNVM_PAGE_SIZE_NON_AUTH;
    uint8_t readData[SNVM_PAGE_SIZE_NON_AUTH];
    uint8_t admin[4];
    uint16_t status;

    MSS_SYS_select_service_mode(MSS_SYS_SERVICE_POLLING_MODE, NULL);

    mHSS_PRINTF("Verifying %lu sNVM pages ...\n", pageCount);

    uint8_t *pExpected = pBuffer;
    size_t remaining = receivedCount;

    for (size_t i = 0u; i < pageCount; i++) {
        uint8_t moduleIdx = startPage + (uint8_t)i;
        size_t chunkSize = (remaining >= SNVM_PAGE_SIZE_NON_AUTH) ?
            SNVM_PAGE_SIZE_NON_AUTH : remaining;

        status = MSS_SYS_secure_nvm_read(
            moduleIdx,
            NULL,   /* p_user_key */
            admin,
            readData,
            SNVM_PAGE_SIZE_NON_AUTH,
            0u      /* mb_offset */
        );

        if (status != MSS_SYS_SUCCESS) {
            mHSS_PRINTF("\nError: sNVM read page %u failed (status=%u)\n",
                moduleIdx, status);
            return false;
        }

        if (memcmp(readData, pExpected, chunkSize) != 0) {
            mHSS_PRINTF("\nError: sNVM verify failed at page %u\n", moduleIdx);
            return false;
        }

        pExpected += chunkSize;
        remaining -= chunkSize;
    }

    mHSS_PRINTF("sNVM verify OK: %lu pages match\n", pageCount);
    return true;
}
#endif

void hss_loader_ymodem_loop(void);
void hss_loader_ymodem_loop(void)
{
    uint8_t rx_byte;
    bool done = false;

    uint32_t receivedCount = 0u;
#if IS_ENABLED(CONFIG_SERVICE_BOOT_SNVM) && IS_ENABLED(CONFIG_SKIP_DDR)
    /* No DDR available: use L2-LIM staging area for YMODEM receive buffer */
    uint8_t *pBuffer = (uint8_t *)(CONFIG_SERVICE_BOOT_SNVM_STAGING_ADDR);
    uint32_t g_rx_size = (uint32_t)CONFIG_SERVICE_BOOT_SNVM_MAX_SIZE;
#else
    uint8_t *pBuffer = (uint8_t *)HSS_DDR_GetStart();
    uint32_t g_rx_size = HSS_DDR_GetSize();
#endif

    while (!done) {
#if IS_ENABLED(CONFIG_SERVICE_QSPI) || IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_BOOT_SNVM)
        bool result = false;
#endif
        static const char menuText[] = "\n"
#if IS_ENABLED(CONFIG_SERVICE_QSPI)
            "QSPI"
#endif
#if IS_ENABLED(CONFIG_SERVICE_QSPI) && IS_ENABLED(CONFIG_SERVICE_MMC)
           "/"
#endif
#if IS_ENABLED(CONFIG_SERVICE_MMC)
           "MMC"
#endif
#if IS_ENABLED(CONFIG_SERVICE_BOOT_SNVM)
           "/sNVM"
#endif
           " Utility\n"
#if IS_ENABLED(CONFIG_SERVICE_QSPI)
            " 1. QSPI Erase Bulk -- erase all sectors\n"
#endif
#if IS_ENABLED(CONFIG_SERVICE_MMC)
            " 2. MMC Init -- initialize MMC driver\n"
#endif
            " 3. YMODEM Receive -- receive application file\n"
#if IS_ENABLED(CONFIG_SERVICE_QSPI)
            " 4. QSPI Write -- write application file to the Device\n"
#endif
#if IS_ENABLED(CONFIG_SERVICE_MMC)
            " 5. MMC Write -- write application file to the Device\n"
#endif
            " 6. Quit -- quit Utility\n"
#if IS_ENABLED(CONFIG_SERVICE_BOOT_SNVM)
            " 7. sNVM Write -- write received file to sNVM pages\n"
            " 8. sNVM Verify -- verify sNVM contents against received file\n"
#endif
            "\n Select a number:\n";

        mHSS_PUTS(menuText);

        if (uart_getchar(&rx_byte, -1, false)) {
            switch (rx_byte) {
#if IS_ENABLED(CONFIG_SERVICE_QSPI)
            case '1':
                mHSS_PUTS("\nInitializing QSPI ... ");
                result = hss_loader_qspi_init();

                if (result) {
                    mHSS_PUTS(" Success\n");

                    mHSS_PUTS("\nErasing all of QSPI ... ");
                    result = hss_loader_qspi_erase();

                    if (result) {
                        mHSS_PUTS(" Success\n");
                    }
                }

                if (!result) {
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                    mHSS_PUTS(" FAILED\n");
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                }
                break;
#endif

#if IS_ENABLED(CONFIG_SERVICE_MMC)
            case '2':
                mHSS_PUTS("\nInitializing MMC ... ");
                result = hss_loader_mmc_init();

                if (result) {
                    mHSS_PUTS(" Success\n");
                }

                if (!result) {
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                    mHSS_PUTS(" FAILED\n");
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                }
                break;
#endif
            case '3':
                mHSS_PUTS("\nAttempting to receive .bin file using YMODEM (CTRL-C to cancel)"
                    "\n");
                receivedCount = ymodem_receive(pBuffer, g_rx_size);
                if (receivedCount == 0) {
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                    mHSS_PUTS("\nYMODEM failed to receive file successfully\n\n");
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                }
                break;

#if IS_ENABLED(CONFIG_SERVICE_QSPI)
            case '4':
                mHSS_PRINTF("\nAttempting to flash received data (%u bytes)\n", receivedCount);
                mHSS_PUTS("\nInitializing QSPI ... ");
                result = hss_loader_qspi_init();

                if (result) {
                    mHSS_PUTS(" Success\n");

                    mHSS_PUTS("\nProgramming QSPI ... ");
                    result = hss_loader_qspi_program((uint8_t *)pBuffer, 0u, receivedCount);

                    if (result) {
                        mHSS_PUTS(" Success\n");
                    }
                }

                if (!result) {
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                    mHSS_PUTS(" FAILED\n");
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                }
                break;
#endif

#if IS_ENABLED(CONFIG_SERVICE_MMC)
            case '5':
                mHSS_PRINTF("\nAttempting to flash received data (%u bytes)\n", receivedCount);
                mHSS_PUTS("\nInitializing MMC ... ");
                result = hss_loader_mmc_init();

                if (result) {
                    mHSS_PUTS(" Success\n");

                    mHSS_PUTS("\nProgramming MMC ... ");
                    result = hss_loader_mmc_program((uint8_t *)pBuffer, 0u, receivedCount);

                    if (result) {
                        mHSS_PUTS(" Success\n");
                    }
                }

                if (!result) {
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                    mHSS_PUTS(" FAILED\n");
                    HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                }
                break;
#endif

            case '6':
                done = true;
                break;

#if IS_ENABLED(CONFIG_SERVICE_BOOT_SNVM)
            case '7':
                if (receivedCount == 0u) {
                    mHSS_PUTS("\nNo data received. Use option 3 (YMODEM Receive) first.\n");
                } else {
                    mHSS_PRINTF("\nWriting %u bytes to sNVM ...\n", receivedCount);
                    result = hss_loader_snvm_program(pBuffer, receivedCount);

                    if (!result) {
                        HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                        mHSS_PUTS(" sNVM Write FAILED\n");
                        HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                    }
                }
                break;

            case '8':
                if (receivedCount == 0u) {
                    mHSS_PUTS("\nNo data received. Use option 3 (YMODEM Receive) first.\n");
                } else {
                    mHSS_PRINTF("\nVerifying %u bytes against sNVM ...\n", receivedCount);
                    result = hss_loader_snvm_verify(pBuffer, receivedCount);

                    if (!result) {
                        HSS_Debug_Highlight(HSS_DEBUG_LOG_ERROR);
                        mHSS_PUTS(" sNVM Verify FAILED\n");
                        HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
                    }
                }
                break;
#endif

            default: // ignore
                break;
	    }
        }
    }
}
