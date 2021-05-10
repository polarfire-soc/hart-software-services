/*******************************************************************************
 * Copyright 2019-2021 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/**
 * \file Tiny CLI parser
 * \brief Tiny CLI parser
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#include <ctype.h> // tolower()
#include <string.h> // strcasecmp(), strtok(), strtok_r()
#include <strings.h>

#include "hss_boot_init.h"
#include "hss_init.h"
#include "hss_state_machine.h"
#include "tinycli_service.h"
#include "hss_memtest.h"
#include "hss_progress.h"
#include "hss_version.h"
#include "uart_helper.h"

#if defined(CONFIG_SERVICE_USBDMSC) && defined(CONFIG_SERVICE_MMC)
#    include "usbdmsc_service.h"
#endif

//#include "mpfs_reg_map.h"

static size_t numTokens = 0u;
#define mMAX_NUM_TOKENS 40
static char *tokenArray[mMAX_NUM_TOKENS];
static bool quitFlag = false;

enum CmdIndex {
    CMD_YMODEM,
    CMD_QUIT,
    CMD_BOOT,
    CMD_RESET,
    CMD_HELP,
    CMD_VERSION,
#if IS_ENABLED(CONFIG_MEMTEST)
    CMD_MEMTEST,
#endif
#if defined(CONFIG_SERVICE_QSPI) && (defined(CONFIG_SERVICE_MMC) || defined(CONFIG_SERVICE_PAYLOAD) || defined(CONFIG_SERVICE_SPI))
    CMD_QSPI,
#endif
#if defined(CONFIG_SERVICE_MMC) && (defined(CONFIG_SERVICE_QSPI) || defined(CONFIG_SERVICE_PAYLOAD) || defined(CONFIG_SERVICE_SPI))
    CMD_EMMC,
    CMD_MMC,
#endif
#if defined(CONFIG_SERVICE_PAYLOAD) && (defined(CONFIG_SERVICE_MMC) || defined(CONFIG_SERVICE_QSPI) || defined(CONFIG_SERVICE_SPI))
    CMD_PAYLOAD,
#endif
#if defined(CONFIG_SERVICE_SPI) && (defined(CONFIG_SERVICE_MMC) || defined(CONFIG_SERVICE_QSPI) || defined(CONFIG_SERVICE_PAYLOAD))
    CMD_SPI,
#endif
#if defined(CONFIG_SERVICE_USBDMSC) && defined(CONFIG_SERVICE_MMC)
    CMD_USBDMSC,
#endif
};

static void   tinyCLI_CmdHandler_(int cmdIndex);
static bool   tinyCLI_GetCmdIndex_(char *pCmdToken, size_t *index);
static void   tinyCLI_PrintVersion_(void);
static void   tinyCLI_PrintHelp_(void);
#if IS_ENABLED(CONFIG_MEMTEST)
static void   tinyCLI_MemTest_(void);
#endif


static struct {
    enum CmdIndex cmdIndex;
    void (* const handler)(int);
    const char * const name;
    const char * const helpString;
} commands[] = {
#if IS_ENABLED(CONFIG_SERVICE_YMODEM)
    { CMD_YMODEM,  tinyCLI_CmdHandler_, "YMODEM",  "Run YMODEM utility to download an image to DDR." },
#endif
    { CMD_QUIT,    tinyCLI_CmdHandler_, "QUIT",    "Quit TinyCLI and return to regular boot process." },
    { CMD_BOOT,    tinyCLI_CmdHandler_, "BOOT",    "Quit TinyCLI and return to regular boot process." },
    { CMD_RESET,   tinyCLI_CmdHandler_, "RESET",   "Reset the E51." },
    { CMD_HELP,    tinyCLI_CmdHandler_, "HELP",    "Display command summary / command help information." },
    { CMD_VERSION, tinyCLI_CmdHandler_, "VERSION", "Display system version information." },
#if IS_ENABLED(CONFIG_MEMTEST)
    { CMD_MEMTEST, tinyCLI_CmdHandler_, "MEMTEST", "Full DDR memory test." },
#endif
#if defined(CONFIG_SERVICE_QSPI) && (defined(CONFIG_SERVICE_MMC) || defined(CONFIG_SERVICE_PAYLOAD) || defined(CONFIG_SERVICE_SPI))
    { CMD_QSPI,    tinyCLI_CmdHandler_, "QSPI",    "Select boot via QSPI." },
#endif
#if defined(CONFIG_SERVICE_MMC) && (defined(CONFIG_SERVICE_QSPI) || defined(CONFIG_SERVICE_PAYLOAD) || defined(CONFIG_SERVICE_SPI))
    { CMD_EMMC,    tinyCLI_CmdHandler_, "EMMC",    "Select boot via MMC." },
    { CMD_MMC,     tinyCLI_CmdHandler_, "MMC",     "Select boot via MMC." },
#endif
#if defined(CONFIG_SERVICE_PAYLOAD) && (defined(CONFIG_SERVICE_MMC) || defined(CONFIG_SERVICE_QSPI) || defined(CONFIG_SERVICE_SPI))
    { CMD_PAYLOAD, tinyCLI_CmdHandler_, "PAYLOAD", "Select boot via payload." },
#endif
#if defined(CONFIG_SERVICE_SPI) && (defined(CONFIG_SERVICE_MMC) || defined(CONFIG_SERVICE_QSPI) || defined(CONFIG_SERVICE_PAYLOAD))
    { CMD_SPI,     tinyCLI_CmdHandler_, "SPI",     "Select boot via SPI Flash." },
#endif
#if defined(CONFIG_SERVICE_USBDMSC) && defined(CONFIG_SERVICE_MMC)
    { CMD_USBDMSC,  tinyCLI_CmdHandler_, "USBDMSC",  "Export eMMC as USBD Mass Storage Class." },
#endif
};


/***********************************************************************/

static bool tinyCLI_GetCmdIndex_(char *pCmdToken, size_t *index)
{
    bool result = false;
    size_t i;

    for (i = 0u; i < ARRAY_SIZE(commands); i++) {
        if (strcasecmp(commands[i].name, pCmdToken) == 0) {
            result = true;
            *index = i;
            break;
        }
    }

    return result;
}

static void tinyCLI_PrintVersion_(void)
{
    (void)HSS_E51_Banner();
}

#if IS_ENABLED(CONFIG_MEMTEST)
static void tinyCLI_MemTest_(void)
{
    bool status = HSS_MemTestDDRFull();

    if (!status) {
        mHSS_FANCY_PRINTF(LOG_ERROR, "Failed!" CRLF);
    } else {
        mHSS_FANCY_PRINTF(LOG_STATUS, "Passed!" CRLF);
    }
}
#endif

static void tinyCLI_PrintHelp_(void)
{
    if (numTokens > 1) {
        int index;

        for (index = 1; index < numTokens; index++) {
            size_t i = 0u;
            //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Argument: %s" CRLF, tokenArray[index]);
            if (tinyCLI_GetCmdIndex_(tokenArray[index], &i)) {
                mHSS_FANCY_PRINTF(LOG_NORMAL, "%s: %s" CRLF, commands[i].name, commands[i].helpString);
            }
        }
    } else {
        mHSS_PUTS("Supported Commands:" CRLF "\t");

        for (size_t i = 0u; i < ARRAY_SIZE(commands); i++) {
            mHSS_PUTS(commands[i].name);
            mHSS_PUTC(' ');
        }
        mHSS_PUTS("" CRLF);
    }
}

static void tinyCLI_CmdHandler_(int cmdIndex)
{
#if IS_ENABLED(CONFIG_SERVICE_YMODEM)
    void hss_loader_ymodem_loop(void);
#endif
    void _start(void);

    size_t index;
    switch (cmdIndex) {
    case CMD_HELP:
        tinyCLI_PrintHelp_();
        break;

    case CMD_VERSION:
        tinyCLI_PrintVersion_();
        break;

#if IS_ENABLED(CONFIG_SERVICE_YMODEM)
    case CMD_YMODEM:
        hss_loader_ymodem_loop();
        break;
#endif

    case CMD_RESET:
        _start();
        break;

    case CMD_QUIT:
        quitFlag = true;
        break;

    case CMD_BOOT:
        quitFlag = true;
        break;

#if IS_ENABLED(CONFIG_MEMTEST)
    case CMD_MEMTEST:
        tinyCLI_MemTest_();
        break;
#endif

#if defined(CONFIG_SERVICE_QSPI) && (defined(CONFIG_SERVICE_MMC) || defined(CONFIG_SERVICE_PAYLOAD) || defined(CONFIG_SERVICE_SPI))
    case CMD_QSPI:
        HSS_BootSelectQSPI();
        break;
#endif

#if defined(CONFIG_SERVICE_MMC) && (defined(CONFIG_SERVICE_QSPI) || defined(CONFIG_SERVICE_PAYLOAD) || defined(CONFIG_SERVICE_SPI))
    case CMD_MMC:
        HSS_BootSelectMMC();
        break;
#endif

#if defined(CONFIG_SERVICE_PAYLOAD) && (defined(CONFIG_SERVICE_MMC) || defined(CONFIG_SERVICE_QSPI) || defined(CONFIG_SERVICE_SPI))
    case CMD_PAYLOAD:
        HSS_BootSelectPayload();
        break;
#endif

#if defined(CONFIG_SERVICE_SPI) && (defined(CONFIG_SERVICE_MMC) || defined(CONFIG_SERVICE_QSPI) || defined(CONFIG_SERVICE_PAYLOAD))
    case CMD_SPI:
        HSS_BootSelectSPI();
        break;
#endif

#if defined(CONFIG_SERVICE_USBDMSC) && defined(CONFIG_SERVICE_MMC)
    case CMD_USBDMSC:
        {
            USBDMSC_Init();
            USBDMSC_Start();
            void tinycli_wait_for_usbmscd_done(void);
            tinycli_wait_for_usbmscd_done();
        }
        break;
#endif

    default:
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Unknown command %d (%lu tokens)" CRLF, cmdIndex, numTokens);
        for (index = 1; index < numTokens; index++) {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Argument: %s" CRLF, tokenArray[index]);
        }
        break;
    }

    mHSS_PUTS("" CRLF);
}

#if !defined(CONFIG_SERVICE_TINYCLI_REGISTER)
static bool tinyCLI_Getline_(char **pBuffer, size_t *pBufLen);
static bool tinyCLI_Getline_(char **pBuffer, size_t *pBufLen)
{
    bool result = false;
    ssize_t status = 0;

    status = uart_getline(pBuffer, pBufLen);

    if (status < 0) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Problem reading input" CRLF);
        // result = false;
    } else {
        result = true;
    }

    return result;
}
#endif

size_t HSS_TinyCLI_ParseIntoTokens(char *buffer)
{
    size_t i = 0u;
    static char *strtok_string = NULL;

    char *pToken = strtok_r(buffer, "\n ", &strtok_string);
    while ((pToken != NULL) && (i < mMAX_NUM_TOKENS))  {
        tokenArray[i] = pToken;
        i++;
        pToken = strtok_r(NULL, "\n ", &strtok_string);
    }

    numTokens = i;
    return i;
}

void HSS_TinyCLI_Execute(void)
{
    size_t i = 0u;
    bool matchFoundFlag = tinyCLI_GetCmdIndex_(tokenArray[0], &i);

    if (matchFoundFlag) {
        commands[i].handler(commands[i].cmdIndex);
    } else {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Unknown command >>%s<<." CRLF CRLF, tokenArray[0]);
    }
}

bool HSS_TinyCLI_Parser(void)
{
    bool keyPressedFlag = false;
    uint8_t rcv_buf;

    keyPressedFlag = HSS_ShowTimeout("Press a key to enter CLI, ESC to skip" CRLF,
        CONFIG_SERVICE_TINYCLI_TIMEOUT, &rcv_buf);

    if (!keyPressedFlag) {
        mHSS_FANCY_PUTS(LOG_NORMAL, "CLI check timeout" CRLF);
    } else {
        mHSS_FANCY_PUTS(LOG_NORMAL, "Type HELP for list of commands" CRLF);
        while (!quitFlag) {
#if !defined(CONFIG_SERVICE_TINYCLI_REGISTER)
            static char *pBuffer = NULL;
            static size_t bufLen = 0u;

            mHSS_FANCY_PUTS(LOG_NORMAL, ">> ");
            bool result = tinyCLI_Getline_(&pBuffer, &bufLen);

            if (result && (pBuffer != NULL)) {
                if (HSS_TinyCLI_ParseIntoTokens(pBuffer)) {
                   HSS_TinyCLI_Execute();
                }
            }
#else
            RunStateMachine(&tinycli_service);
#  if defined(CONFIG_SERVICE_USBDMSC) && defined(CONFIG_SERVICE_MMC)
            RunStateMachine(&usbdmsc_service);
#  endif
#endif
        }
    }

    return true;
}
