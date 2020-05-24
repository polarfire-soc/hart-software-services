/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
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

#include <ctype.h> // tolower()
#include <string.h> // strcasecmp(), strtok(), strtok_r()
#include <strings.h>

#include "hss_debug.h"
#include "hss_init.h"
#include "hss_tinycli.h"
#include "hss_memtest.h"
#include "hss_progress.h"
#include "hss_version.h"
#include "uart_helper.h"
#include "mpfs_reg_map.h"

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
#ifdef CONFIG_MEMTEST
    CMD_MEMTEST
#endif
};

static void   HSS_TinyCLI_CmdHandler(int cmdIndex);
static bool   HSS_TinyCLI_GetCmdIndex(char *pCmdToken, size_t *index);
static void   HSS_TinyCLI_PrintVersion(void);
#ifdef CONFIG_MEMTEST
static void   HSS_TinyCLI_MemTest(void);
#endif
static void   HSS_TinyCLI_PrintHelp(void);
static bool   HSS_TinyCLI_Getline(char **pBuffer, size_t *pBufLen);
static size_t HSS_TinyCLI_ParseIntoTokens(char *buffer);
static void   HSS_TinyCLI_Execute(void);


static struct {
    enum CmdIndex cmdIndex;
    void (* const handler)(int);
    const char * const name;
    const char * const helpString;
} commands[] = {
    { CMD_YMODEM, HSS_TinyCLI_CmdHandler, "YMODEM",  "Run YMODEM utility to update image in QSPI." },
    { CMD_QUIT,   HSS_TinyCLI_CmdHandler, "QUIT",    "Quit TinyCLI and return to regular boot process." },
    { CMD_BOOT,   HSS_TinyCLI_CmdHandler, "BOOT",    "Quit TinyCLI and return to regular boot process." },
    { CMD_RESET,  HSS_TinyCLI_CmdHandler, "RESET",   "Reset the E51." },
    { CMD_HELP,   HSS_TinyCLI_CmdHandler, "HELP",    "Display command summary / command help information." },
    { CMD_VERSION,HSS_TinyCLI_CmdHandler, "VERSION", "Display system version information." },
#ifdef CONFIG_MEMTEST
    { CMD_MEMTEST,HSS_TinyCLI_CmdHandler, "MEMTEST", "Full DDR memory test." },
#endif
};


/***********************************************************************/

static bool HSS_TinyCLI_GetCmdIndex(char *pCmdToken, size_t *index)
{
    bool result = false;
    size_t i;

    for (i = 0u; i < mSPAN_OF(commands); i++) {
        if (strcasecmp(commands[i].name, pCmdToken) == 0) {
            result = true;
            *index = i;
            break;
        }
    }

    return result;
}

static void HSS_TinyCLI_PrintVersion(void)
{
    (void)HSS_E51_Banner();
}

#ifdef CONFIG_MEMTEST
static void HSS_TinyCLI_MemTest(void)
{
    bool status = HSS_MemTestDDRFull();

    if (!status) {
        mHSS_FANCY_PRINTF(LOG_ERROR, "Failed!" CRLF);
    } else {
        mHSS_FANCY_PRINTF(LOG_STATUS, "Passed!" CRLF);
    }
}
#endif    

static void HSS_TinyCLI_PrintHelp(void)
{
    if (numTokens > 1) {
        int index;

        for (index = 1; index < numTokens; index++) {
            size_t i = 0u;
            //mHSS_DEBUG_PRINTF(LOG_NORMAL, "Argument: %s" CRLF, tokenArray[index]);
            if (HSS_TinyCLI_GetCmdIndex(tokenArray[index], &i)) {
                mHSS_FANCY_PRINTF(LOG_NORMAL, "%s: %s" CRLF, commands[i].name, commands[i].helpString);
            }
        }
    } else {
        mHSS_PUTS("Supported Commands:" CRLF "\t");

        for (size_t i = 0u; i < mSPAN_OF(commands); i++) {
            mHSS_PUTS(commands[i].name); 
            mHSS_PUTC(' ');
        }
        mHSS_PUTS("" CRLF);
    }
}

static void HSS_TinyCLI_CmdHandler(int cmdIndex)
{
#ifdef CONFIG_SERVICE_YMODEM
    void e51_ymodem_loop(void);
#endif
    void _start(void);

    size_t index;
    switch (cmdIndex) {
    case CMD_HELP:
        HSS_TinyCLI_PrintHelp();
        break;

    case CMD_VERSION:
        HSS_TinyCLI_PrintVersion();
        break;

    case CMD_YMODEM:
#ifdef CONFIG_SERVICE_YMODEM
        e51_ymodem_loop();
#endif
        break;

    case CMD_RESET:
        _start();
        break;

    case CMD_QUIT:
        quitFlag = true;
        break;
        
    case CMD_BOOT:
        quitFlag = true;
        break;

#ifdef CONFIG_MEMTEST
    case CMD_MEMTEST:
        HSS_TinyCLI_MemTest();
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

static bool HSS_TinyCLI_Getline(char **pBuffer, size_t *pBufLen)
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

static size_t HSS_TinyCLI_ParseIntoTokens(char *buffer)
{
    size_t i = 0u;
    static char *strtok_string = NULL;

    char *strtok_r(char *str, const char *delim, char **saveptr);

    char *pToken = strtok_r(buffer, "\n ", &strtok_string);
    while (pToken != NULL) {
        tokenArray[i] = pToken;
        i++;
        pToken = strtok_r(NULL, "\n ", &strtok_string);
    }

    return i;
}

static void HSS_TinyCLI_Execute(void)
{
    size_t i = 0u;
    bool matchFoundFlag = HSS_TinyCLI_GetCmdIndex(tokenArray[0], &i);

    if (matchFoundFlag) {
        commands[i].handler(i);
    } else {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Unknown command >>%s<<." CRLF CRLF, tokenArray[0]);
    }
}

bool HSS_TinyCLI_Parser(void)
{
    bool keyPressedFlag = false;
    uint8_t rcv_buf;

    keyPressedFlag = HSS_ShowTimeout("Press a key to enter CLI, ESC to skip" CRLF,
        CONFIG_TINYCLI_TIMEOUT, &rcv_buf);

    if (!keyPressedFlag) {
        mHSS_FANCY_PUTS(LOG_NORMAL, "CLI check timeout" CRLF);
    } else {
        static char *pBuffer = NULL;
        static size_t bufLen = 0u;

        mHSS_FANCY_PUTS(LOG_NORMAL, "Type HELP for list of commands" CRLF);

        while (!quitFlag) {
            mHSS_FANCY_PUTS(LOG_NORMAL, ">> ");
            bool result = HSS_TinyCLI_Getline(&pBuffer, &bufLen);

            if (result && (pBuffer != NULL)) {
                numTokens = HSS_TinyCLI_ParseIntoTokens(pBuffer);

                if (numTokens) {
                   HSS_TinyCLI_Execute();
                }
            }
        }
    }

    return true;
}
