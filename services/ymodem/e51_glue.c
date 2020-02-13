#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

#include "hw_platform.h"
#include "mss_hal.h"
#include "mss_util.h"
#include "mss_uart.h"
#include "uart_helper.h"
#include "ymodem.h"
#include "mss_envm.h"
#include "mss_sysreg.h"
#include "baremetal/drivers/micron_mt25q/micron_mt25q.h"

mss_sysreg_t*   SYSREG = (mss_sysreg_t*)BASE32_ADDR_MSS_SYSREG; // TODO: find a better place for this

int ee_vsprintf(char *buf, const char *fmt, va_list args);

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

void print_result(uint8_t result, const char *msg)
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


void print_failure_at(uint32_t * address, const char *msg, uint32_t expected, uint32_t found)
{
    static char buffer[256];

    l_sprintf(buffer, (const char * const)"%s - failure at 0x%p, expected 0x%08X, got 0x%08X" CRLF, 
        msg, address, expected, found);
    mHSS_DEBUG_PRINTF_EX(buffer);
}


void print_page_program_failure(uint8_t result, uint8_t sector, uint8_t page)
{
    static char buffer[256];

    print_result(result, "envm_program_page()");
    l_sprintf(buffer, (const char * const)"Failed at sector %u, page %u" CRLF, 
        (unsigned int) sector, (unsigned int) page);
    mHSS_DEBUG_PRINTF_EX(buffer);
}


void print_page_program_verify_failure(uint8_t * address, const char *msg, uint8_t expected, 
    uint8_t found)
{
    static char buffer[256];

    l_sprintf(buffer, (const char * const)"%s - failure at 0x%p, expected 0x%02X, got 0x%02X" CRLF, 
        msg, address, expected, found);
    mHSS_DEBUG_PRINTF_EX(buffer);
}

static void l_e51_envm_init(void)
{
    static bool initialized = false;

    if (!initialized) {
        volatile uint8_t resultTemp = envm_init();
        print_result(resultTemp, "envm_init()");

        envm_set_clock(MSS_COREPLEX_CPU_CLK);
        print_result(0, "envm_set_clock()");

        MSS_QSPI_init();
        MSS_QSPI_enable();
        Flash_init(MSS_QSPI_NORMAL);
        print_result(0, "Flash_init()");
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
    uint32_t g_rx_size = 1024 * 128;

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
                l_e51_envm_init();
                Flash_die_erase();
                print_result(0, CRLF " QSPI FLASH_chip_erase()" CRLF);
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
                l_e51_envm_init();
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
