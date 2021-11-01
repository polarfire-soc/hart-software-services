/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
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

//#include <ctype.h> // tolower()
#include <string.h> // strcasecmp(), strtok(), strtok_r()
#include <strings.h>
#include <stdlib.h>

#include "hss_boot_init.h"
#include "gpt.h"
#include "hss_init.h"
#include "hss_state_machine.h"

#include "tinycli_service.h"
#include "tinycli_hexdump.h"

#include "hss_memtest.h"
#include "hss_progress.h"
#include "hss_version.h"
#include "hss_crc32.h"
#include "uart_helper.h"
#include "ddr_service.h"
#include "csr_helper.h"
#include "wdog_service.h"
#include "hss_perfctr.h"

#include "hss_registry.h"
#include "assert.h"

#if IS_ENABLED(CONFIG_SERVICE_USBDMSC) && IS_ENABLED(CONFIG_SERVICE_MMC)
#    include "usbdmsc_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_SCRUB)
#    include "scrub_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_BEU)
#    include "beu_service.h"
#endif

#define mMAX_NUM_TOKENS 40
static size_t argc_tokenCount = 0u;
static char *argv_tokenArray[mMAX_NUM_TOKENS];
static bool quitFlag = false;

#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_MONITOR)
#  define mNUM_MONITORS 10
struct tinycli_monitor
{
    bool active;
    bool allocated;
    HSSTicks_t time;
    size_t interval_sec;
    uintptr_t startAddr;
    size_t count;
} monitors[mNUM_MONITORS];
#endif

enum CmdId {
    CMD_YMODEM,
    CMD_QUIT,
    CMD_BOOT,
    CMD_RESET,
    CMD_HELP,
    CMD_VERSION,
    CMD_UPTIME,
    CMD_DEBUG,
#if IS_ENABLED(CONFIG_MEMTEST)
    CMD_MEMTEST,
#endif
#if IS_ENABLED(CONFIG_SERVICE_QSPI) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_PAYLOAD) || IS_ENABLED(CONFIG_SERVICE_SPI))
    CMD_QSPI,
#endif
#if IS_ENABLED(CONFIG_SERVICE_MMC) && (IS_ENABLED(CONFIG_SERVICE_QSPI) || IS_ENABLED(CONFIG_SERVICE_PAYLOAD) || IS_ENABLED(CONFIG_SERVICE_SPI))
    CMD_EMMC,
    CMD_MMC,
#endif
#if IS_ENABLED(CONFIG_SERVICE_PAYLOAD) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_QSPI) || IS_ENABLED(CONFIG_SERVICE_SPI))
    CMD_PAYLOAD,
#endif
#if IS_ENABLED(CONFIG_SERVICE_SPI) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_QSPI) || IS_ENABLED(CONFIG_SERVICE_PAYLOAD))
    CMD_SPI,
#endif
#if IS_ENABLED(CONFIG_SERVICE_USBDMSC) && IS_ENABLED(CONFIG_SERVICE_MMC)
    CMD_USBDMSC,
#endif
#if IS_ENABLED(CONFIG_SERVICE_SCRUB)
    CMD_SCRUB,
#endif
    CMD_INVALID
};

struct tinycli_key {
    const int tokenId;
    const char * const name;
    const char * const helpString;
};

const struct tinycli_key cmdKeys[] = {
#if IS_ENABLED(CONFIG_SERVICE_YMODEM)
    { CMD_YMODEM,  "YMODEM",  "Run YMODEM utility to download an image to DDR." },
#endif
    { CMD_QUIT,    "QUIT",    "Quit TinyCLI and return to regular boot process." },
    { CMD_BOOT,    "BOOT",    "Quit TinyCLI and return to regular boot process." },
    { CMD_RESET,   "RESET",   "Reset the E51." },
    { CMD_HELP,    "HELP",    "Display command summary / command help information." },
    { CMD_VERSION, "VERSION", "Display system version information." },
    { CMD_UPTIME,  "UPTIME",  "Display uptime information." },
    { CMD_DEBUG,   "DEBUG",   "Display debug information." },
#if IS_ENABLED(CONFIG_MEMTEST)
    { CMD_MEMTEST, "MEMTEST", "Full DDR memory test." },
#endif
#if IS_ENABLED(CONFIG_SERVICE_QSPI) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_PAYLOAD))
    { CMD_QSPI,    "QSPI",    "Select boot via QSPI." },
#endif
#if IS_ENABLED(CONFIG_SERVICE_MMC) && (IS_ENABLED(CONFIG_SERVICE_QSPI) || IS_ENABLED(CONFIG_SERVICE_PAYLOAD))
    { CMD_EMMC,    "EMMC",    "Select boot via MMC." },
    { CMD_MMC,     "MMC",     "Select boot via MMC." },
#endif
#if IS_ENABLED(CONFIG_SERVICE_PAYLOAD) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_QSPI))
    { CMD_PAYLOAD, "PAYLOAD", "Select boot via payload." },
#endif
#if IS_ENABLED(CONFIG_SERVICE_USBDMSC) && IS_ENABLED(CONFIG_SERVICE_MMC)
    { CMD_USBDMSC, "USBDMSC", "Export eMMC as USBD Mass Storage Class." },
#endif
#if IS_ENABLED(CONFIG_SERVICE_SCRUB)
    { CMD_SCRUB, "SCRUB", "Dump Scrub service stats." },
#endif
};

static void tinyCLI_CmdHandler_(int tokenId);
static bool tinyCLI_NameToKeyIndex_(struct tinycli_key const * const keys, size_t numKeys,
    char const * const pToken, size_t *pIndex);
static bool tinyCLI_CmdIdToCommandIndex_(enum CmdId tokenId, size_t * pIndex);
static void tinyCLI_PrintVersion_(void);
static void tinyCLI_PrintUptime_(void);
static void tinyCLI_PrintHelp_(void);
static void tinyCLI_Debug_(void);
static void tinyCLI_Reset_(void);
#if IS_ENABLED(CONFIG_SERVICE_MMC)
static void tinyCLI_Boot_List_(void);
static void tinyCLI_Boot_Select_(void);
#endif
static bool tinyCLI_Boot_(void);
#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_MONITOR)
static void tinyCLI_Monitor_(void);
#endif
static void tinyCLI_CRC32_(void);
static void tinyCLI_HexDump_(void);
#if IS_ENABLED(CONFIG_MEMTEST)
static void tinyCLI_MemTest_(void);
#endif

struct tinycli_command {
    const enum CmdId tokenId;
    bool warnIfPostInit;
    void (* const handler)(int);
};

static struct tinycli_command commands[] = {
#if IS_ENABLED(CONFIG_SERVICE_YMODEM)
    { CMD_YMODEM,  true,  tinyCLI_CmdHandler_ },
#endif
    { CMD_QUIT,    true,  tinyCLI_CmdHandler_ },
    { CMD_BOOT,    true,  tinyCLI_CmdHandler_ },
    { CMD_RESET,   true,  tinyCLI_CmdHandler_ },
    { CMD_HELP,    false, tinyCLI_CmdHandler_ },
    { CMD_VERSION, false, tinyCLI_CmdHandler_ },
    { CMD_UPTIME,  false, tinyCLI_CmdHandler_ },
    { CMD_DEBUG,   false, tinyCLI_CmdHandler_ },
#if IS_ENABLED(CONFIG_MEMTEST)
    { CMD_MEMTEST, true,  tinyCLI_CmdHandler_ },
#endif
#if IS_ENABLED(CONFIG_SERVICE_QSPI) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_PAYLOAD) || IS_ENABLED(CONFIG_SERVICE_SPI))
    { CMD_QSPI,    true,  tinyCLI_CmdHandler_ },
#endif
#if IS_ENABLED(CONFIG_SERVICE_MMC) && (IS_ENABLED(CONFIG_SERVICE_QSPI) || IS_ENABLED(CONFIG_SERVICE_PAYLOAD) || IS_ENABLED(CONFIG_SERVICE_SPI))
    { CMD_EMMC,    true,  tinyCLI_CmdHandler_ },
    { CMD_MMC,     true,  tinyCLI_CmdHandler_ },
#endif
#if IS_ENABLED(CONFIG_SERVICE_PAYLOAD) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_QSPI) || IS_ENABLED(CONFIG_SERVICE_SPI))
    { CMD_PAYLOAD, true,  tinyCLI_CmdHandler_ },
#endif
#if IS_ENABLED(CONFIG_SERVICE_SPI) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_QSPI) || IS_ENABLED(CONFIG_SERVICE_PAYLOAD))
    { CMD_SPI,     true,  tinyCLI_CmdHandler_ },
#endif
#if IS_ENABLED(CONFIG_SERVICE_USBDMSC) && IS_ENABLED(CONFIG_SERVICE_MMC)
    { CMD_USBDMSC, true,  tinyCLI_CmdHandler_ },
#endif
#if IS_ENABLED(CONFIG_SERVICE_SCRUB)
    { CMD_SCRUB,   false, tinyCLI_CmdHandler_ },
#endif
};

static bool postInit = false;


/***********************************************************************/

static bool tinyCLI_NameToKeyIndex_(struct tinycli_key const * const keys, size_t numKeys,
    char const * const pToken, size_t *pIndex)
{
    bool result = false;
    size_t i;

    assert(keys);
    assert(pToken);
    assert(pIndex);

    for (i = 0u; i < numKeys; i++) {	// check for full match
        if (strncasecmp(keys[i].name, pToken, strlen(keys[i].name)) == 0) {
            result = true;
            *pIndex = i;
            break;
        }
    }

    if (!result) {			// if no match found, check for partial match
        size_t count = 0u;
        for (i = 0u; i < numKeys; i++) {
            if (strncasecmp(keys[i].name, pToken, strlen(pToken)) == 0) {
                *pIndex = i;
                count++;
            }
        }

        if (count == 1u) {
            result = true;		// multiple matches => ambiguity
        }
    }

    return result;
}

static bool tinyCLI_CmdIdToCommandIndex_(enum CmdId tokenId, size_t * pIndex)
{
    bool result = false;
    size_t i;

    for (i = 0; i < ARRAY_SIZE(commands); i++) {
        if (commands[i].tokenId == tokenId) {
            result = true;
            *pIndex = i;
            break;
        }
    }

    return result;
}

static unsigned long int tinyCLI_strtoul_wrapper_(const char *__restrict nptr)
{
    int base = 10;

    int tolower(int __c); // normally in ctypes.h, but we don't have a _ctype_ defined

    if ((nptr[0] == '0') && (tolower((int)nptr[1]) == 'x')) {
        base = 16;
    }

    return strtoul(nptr, NULL, base);
}

static void tinyCLI_PrintUptime_(void)
{
    HSSTicks_t timeVal = CSR_GetTime();

    timeVal /= 1000000lu;

    uint32_t days = timeVal / (24lu * 3600lu);
    timeVal = timeVal - (days * (24 * 3600lu));

    uint32_t hours = timeVal / 3600lu;
    timeVal = timeVal - (hours * 3600lu);

    uint32_t mins = timeVal / 60lu;
    uint32_t secs = timeVal - (mins * 60lu);

    mHSS_FANCY_PRINTF(LOG_STATUS, "Uptime is ");
    if (days) {
        mHSS_FANCY_PRINTF_EX("%lu day%s, ", days, days == 1lu ? "":"s");
    }

    if (hours) {
        mHSS_FANCY_PRINTF_EX("%lu hour%s, ", hours, hours == 1lu ? "":"s");
    }

    if (mins) {
        mHSS_FANCY_PRINTF_EX("%lu minute%s, ", mins, mins == 1lu ? "":"s");
    }

    mHSS_FANCY_PRINTF_EX("%lu second%s", secs, secs == 1lu ? "":"s");
}

static void tinyCLI_PrintVersion_(void)
{
    (void)HSS_E51_Banner();
}

#if IS_ENABLED(CONFIG_MEMTEST)
static void tinyCLI_MemTest_(void)
{
    bool status = false;

    if (argc_tokenCount > 1u) {
        size_t count = 256u;
        const uintptr_t startAddr = tinyCLI_strtoul_wrapper_(argv_tokenArray[1]);

        if (argc_tokenCount > 2u) {
            count = tinyCLI_strtoul_wrapper_(argv_tokenArray[2]);
        }

        status = HSS_MemTestDDR_Ex((uint64_t *)startAddr, count);
    } else {
        status = HSS_MemTestDDRFull();
    }

    if (!status) {
        mHSS_FANCY_PRINTF(LOG_ERROR, "Failed!" CRLF);
    } else {
        mHSS_FANCY_PRINTF(LOG_STATUS, "Passed!" CRLF);
    }
}
#endif

static void tinyCLI_PrintHelp_(void)
{
    if (argc_tokenCount > 1u) {
        for (size_t i = 1u; i < argc_tokenCount; ++i)  {
            size_t index;

            if (tinyCLI_NameToKeyIndex_(cmdKeys, ARRAY_SIZE(cmdKeys), argv_tokenArray[i], &index)) {
                mHSS_FANCY_PRINTF(LOG_NORMAL, "%s: %s" CRLF, cmdKeys[index].name, cmdKeys[index].helpString);
            }
        }
    } else {
        mHSS_PUTS("Supported Commands:" CRLF "\t");

        for (size_t i = 0u; i < ARRAY_SIZE(cmdKeys); i++) {
            size_t index = CMD_INVALID;

            if (tinyCLI_CmdIdToCommandIndex_(cmdKeys[i].tokenId, &index) &&
                (commands[index].warnIfPostInit)) {
                HSS_Debug_Highlight(HSS_DEBUG_LOG_WARN);
            }
            mHSS_PUTS(cmdKeys[i].name);
            if ((index != CMD_INVALID) && (commands[index].warnIfPostInit)) {
                HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
            }
            mHSS_PUTC(' ');
        }
        mHSS_PUTS("" CRLF);
    }
}

static void tinyCLI_Reset_(void)
{
#if IS_ENABLED(CONFIG_SERVICE_WDOG)
    HSS_Wdog_Reboot(HSS_HART_E51);
    HSS_Wdog_Reboot(HSS_HART_U54_1);
    HSS_Wdog_Reboot(HSS_HART_U54_2);
    HSS_Wdog_Reboot(HSS_HART_U54_3);
    HSS_Wdog_Reboot(HSS_HART_U54_4);
#endif
}

static void tinyCLI_Seg_(void)
{
    (void)HSS_DDRPrintSegConfig();
}

static void tinyCLI_L2Cache_(void)
{
    (void)HSS_DDRPrintL2CacheWaysConfig();
    (void)HSS_DDRPrintL2CacheWayMasks();
}

#if IS_ENABLED(CONFIG_SERVICE_BEU)
static void tinyCLI_BEU_(void)
{
    HSS_BEU_DumpStats();
}
#endif

#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
static void tinyCLI_PerfCtrs_(void)
{
    HSS_PerfCtr_DumpAll();
}
#endif

static void tinyCLI_Debug_(void)
{
    bool usageError = false;
    enum DebugKey {
#if IS_ENABLED(CONFIG_SERVICE_BEU)
        DBG_BEU,
#endif
        DBG_SM,
        DBG_IPI,
        DBG_CRC32,
        DBG_HEXDUMP,
#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_MONITOR)
        DBG_MONITOR,
#endif
        DBG_SEG,
        DBG_L2CACHE,
#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
        DBG_PERFCTR,
#endif
        DBG_WDOG,
    };

    const struct tinycli_key debugKeys[] = {
#if IS_ENABLED(CONFIG_SERVICE_BEU)
        { DBG_BEU,      "BEU",     "debug Bus Error Unit monitor" },
#endif
        { DBG_SM,       "SM",      "debug state machines" },
        { DBG_IPI,      "IPI",     "debug HSS IPI Queues" },
        { DBG_CRC32,    "CRC32",   "calculate CRC32 over memory region" },
        { DBG_HEXDUMP,  "HEXDUMP", "display memory as hex dump" },
#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_MONITOR)
        { DBG_MONITOR,  "MONITOR", "monitor memory locations periodically" },
#endif
        { DBG_SEG,      "SEG",     "display seg registers" },
        { DBG_L2CACHE,  "L2CACHE", "display l2cache settings" },
#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
        { DBG_PERFCTR , "PERFCTR", "display perf counters" },
#endif
        { DBG_WDOG ,    "WDOG",    "display watchdog statistics" },
    };

    size_t keyIndex;
    if ((argc_tokenCount > 1u)
        && (tinyCLI_NameToKeyIndex_(debugKeys, ARRAY_SIZE(debugKeys), argv_tokenArray[1], &keyIndex))) {
        switch (keyIndex) {
#if IS_ENABLED(CONFIG_SERVICE_BEU)
        case DBG_BEU:
            tinyCLI_BEU_();
            break;
#endif

        case DBG_SM:
            DumpStateMachineStats();
            break;

        case DBG_IPI:
            IPI_DebugDumpStats();
            break;

        case DBG_CRC32:
            tinyCLI_CRC32_();
            break;

#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_MONITOR)
        case DBG_MONITOR:
            tinyCLI_Monitor_();
            break;
#endif

        case DBG_HEXDUMP:
            tinyCLI_HexDump_();
            break;

        case DBG_SEG:
            tinyCLI_Seg_();
            break;

        case DBG_L2CACHE:
            tinyCLI_L2Cache_();
            break;

#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
        case DBG_PERFCTR:
            tinyCLI_PerfCtrs_();
            break;
#endif

        case DBG_WDOG:
#if IS_ENABLED(CONFIG_SERVICE_WDOG)
            HSS_Wdog_DumpStats();
#endif
            break;

        default:
            usageError = true;
            break;
        }
    } else {
        usageError = true;
    }

    if (usageError) {
        mHSS_PUTS("Supported options:" CRLF);

        for (size_t i = 0u; i < ARRAY_SIZE(debugKeys); i++) {
            mHSS_PRINTF("\t%s - %s" CRLF, debugKeys[i].name, debugKeys[i].helpString);
        }
    }
}

#if IS_ENABLED(CONFIG_SERVICE_MMC)
static void tinyCLI_Boot_List_(void)
{
    HSS_GPT_t gpt;

    if (IS_ENABLED(CONFIG_SERVICE_MMC) && IS_ENABLED(CONFIG_SERVICE_BOOT_MMC_USE_GPT)) {
        GPT_RegisterReadBlockFunction(HSS_MMC_ReadBlock);

        GPT_Init(&gpt);
        bool result = GPT_ReadHeader(&gpt);

        if (result) {
            size_t srcIndex = 0u;
            HSS_GPT_PartitionEntry_t const * pGptPartitionEntry;

            result = GPT_FindBootSectorIndex(&gpt, &srcIndex, &pGptPartitionEntry);

            do {
                mHSS_DEBUG_PRINTF(LOG_NORMAL, "Boot Partition found at index %lu" CRLF, srcIndex);

                if (!result) {
                    mHSS_DEBUG_PRINTF(LOG_ERROR, "GPT_FindBootSectorIndex() failed" CRLF);
                } else {
                    struct HSS_BootImage localBootImage;
                    size_t srcLBAOffset;

                    result = GPT_PartitionIdToLBAOffset(&gpt, srcIndex, &srcLBAOffset);
                    if (!result) {
                        //mHSS_DEBUG_PRINTF(LOG_ERROR, "GPT_PartitionIdToLBAOffset() failed" CRLF);
                    } else {
                        result = HSS_MMC_ReadBlock(&localBootImage, srcLBAOffset * GPT_LBA_SIZE,
                            sizeof(struct HSS_BootImage));
                    }
                    if (!result) {
                        //mHSS_DEBUG_PRINTF(LOG_ERROR, "HSS_MMC_ReadBlock() failed" CRLF);
                    } else {
                        if ((localBootImage.magic == mHSS_BOOT_MAGIC)
                            || (localBootImage.magic == mHSS_COMPRESSED_MAGIC)) {
                            mHSS_DEBUG_PRINTF(LOG_NORMAL, ">>%s<<" CRLF, localBootImage.set_name);
                        } else {
                            //mHSS_DEBUG_PRINTF(LOG_ERROR, "magic failed" CRLF);
                        }
                    }
                }

                srcIndex++;
                result = GPT_FindBootSectorIndex(&gpt, &srcIndex, &pGptPartitionEntry);
            } while (result);
        }
    }
}

static void tinyCLI_Boot_Select_(void)
{
    HSS_GPT_t gpt;

    if (argc_tokenCount > 2u) {
        size_t index = tinyCLI_strtoul_wrapper_(argv_tokenArray[2]);
        GPT_SetBootPartitionIndex(&gpt, index);
    } else {
        mHSS_PUTS("Usage:" CRLF
            "\tboot select <partition_index>" CRLF
            CRLF);
    }
}
#endif

static bool tinyCLI_Boot_(void)
{
    bool result = false;
    bool usageError = false;
    enum BootgKey {
        BOOT_LIST,
        BOOT_SELECT
    };
    const struct tinycli_key bootKeys[] = {
        { BOOT_LIST,   "LIST",     "list boot partitions" },
        { BOOT_SELECT, "SELECT",   "select active boot partition" },
    };

    size_t keyIndex;
    if (argc_tokenCount > 1u) {
        if (tinyCLI_NameToKeyIndex_(bootKeys, ARRAY_SIZE(bootKeys), argv_tokenArray[1], &keyIndex)) {
            switch (keyIndex) {
#if IS_ENABLED(CONFIG_SERVICE_MMC)
            case BOOT_LIST:
                tinyCLI_Boot_List_();
                break;

            case BOOT_SELECT:
                tinyCLI_Boot_Select_();
                break;
#endif

            default:
                usageError = true;
                break;
            }
        } else {
            usageError = true;
        }
    } else {
        result = true; // boot on its own
    }

    if (usageError) {
        mHSS_PUTS("Supported options:" CRLF);

        for (size_t i = 0u; i < ARRAY_SIZE(bootKeys); i++) {
            mHSS_PRINTF("\t%s - %s" CRLF, bootKeys[i].name, bootKeys[i].helpString);
        }
    }

    return result;
}

#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_MONITOR)
static void tinyCLI_Monitor_(void)
{
    bool usageError = false;
    enum MonitorKey {
        MONITOR_CREATE,
        MONITOR_DESTROY,
        MONITOR_ENABLE,
        MONITOR_DISABLE,
        MONITOR_LIST,
    };
    const struct tinycli_key monitorKeys[] = {
        { MONITOR_CREATE,  "CREATE",  "<interval> 0x<start_addr> 0x<length>" },
        { MONITOR_DESTROY, "DESTROY", "<index> " },
        { MONITOR_ENABLE,  "ENABLE",  "<index> " },
        { MONITOR_DISABLE, "DISABLE", "<index> " },
        { MONITOR_LIST,    "LIST",    "" },
    };

    size_t keyIndex;
    if ((argc_tokenCount > 2)
        && (tinyCLI_NameToKeyIndex_(monitorKeys, ARRAY_SIZE(monitorKeys), argv_tokenArray[2], &keyIndex))) {
        switch (keyIndex) {
        case MONITOR_CREATE:
            if (argc_tokenCount > 5u) {
                size_t index;
                for (index = 0u; index < ARRAY_SIZE(monitors); index++) {
                    if (!monitors[index].allocated) {
                        break;
                    }
                }

               if (index < ARRAY_SIZE(monitors)) {
                    monitors[index].allocated = true;
                    monitors[index].active = false;
                    monitors[index].interval_sec = tinyCLI_strtoul_wrapper_(argv_tokenArray[3]);
                    monitors[index].startAddr = tinyCLI_strtoul_wrapper_(argv_tokenArray[4]);
                    monitors[index].count = tinyCLI_strtoul_wrapper_(argv_tokenArray[5]);
                    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Allocated monitor index %lu" CRLF, index);
                } else {
                    mHSS_DEBUG_PRINTF(LOG_ERROR, "All monitors are allocated" CRLF);
                }
            } else {
                usageError = true;
            }
            break;

        case MONITOR_DESTROY:
            if (argc_tokenCount > 3u) {
                size_t index = tinyCLI_strtoul_wrapper_(argv_tokenArray[3]);

                if (index < ARRAY_SIZE(monitors)) {
                    if (monitors[index].allocated) {
                        monitors[index].allocated = false;
                        monitors[index].active = false;
                        monitors[index].interval_sec = 0u;
                        monitors[index].startAddr = 0u;
                        monitors[index].count = 0u;
                        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Destroyed monitor index %lu" CRLF, index);
                    } else {
                        mHSS_DEBUG_PRINTF(LOG_ERROR, "Monitor index %lu not allocated" CRLF, index);
                    }
                } else {
                    usageError = true;
                }
            } else {
                usageError = true;
            }
            break;

        case MONITOR_ENABLE:
            if (argc_tokenCount > 3u) {
                size_t index = tinyCLI_strtoul_wrapper_(argv_tokenArray[3]);

                if (index < ARRAY_SIZE(monitors)) {
                    if (monitors[index].allocated) {
                        monitors[index].active = monitors[index].allocated;
                        monitors[index].time = HSS_GetTime();
                        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Enabled monitor index %lu" CRLF, index);
                    } else {
                        mHSS_DEBUG_PRINTF(LOG_ERROR, "Monitor index %lu not allocated" CRLF, index);
                    }
                } else {
                    usageError = true;
                }
            } else {
                usageError = true;
            }
            break;

        case MONITOR_DISABLE:
            if (argc_tokenCount > 3u) {
                size_t index = tinyCLI_strtoul_wrapper_(argv_tokenArray[3]);

                if (index < ARRAY_SIZE(monitors)) {
                    if (monitors[index].allocated) {
                        monitors[index].active = false;
                        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Disabled monitor index %lu" CRLF, index);
                    } else {
                        mHSS_DEBUG_PRINTF(LOG_ERROR, "Monitor index %lu not allocated" CRLF, index);
                    }
                } else {
                    usageError = true;
                }
            } else {
                usageError = true;
            }
            break;

        case MONITOR_LIST:
            mHSS_PUTS(" Index Active Allocated Interval       Start_Addr    Count" CRLF
                      "===========================================================" CRLF);

            for (size_t index = 0; index < ARRAY_SIZE(monitors);  ++index) {
                mHSS_PRINTF(" % 5lu % 6d % 9d % 8lu %16x %8x" CRLF,
                    index,
                    monitors[index].active,
                    monitors[index].allocated,
                    monitors[index].interval_sec,
                    monitors[index].startAddr,
                    monitors[index].count);
            }
            break;

        default:
            usageError = true;
            break;
        }
    } else {
        usageError = true;
    }

    if (usageError) {
        mHSS_PUTS("Usage:" CRLF);

        for (size_t i = 0u; i < ARRAY_SIZE(monitorKeys); i++) {
            mHSS_PRINTF("\tDEBUG MONITOR %s - %s" CRLF,
                monitorKeys[i].name, monitorKeys[i].helpString);
        }
    }
}
#endif

static void tinyCLI_CRC32_(void)
{
    if (argc_tokenCount > 2u) {
        size_t count = 256u;
        const uintptr_t startAddr = tinyCLI_strtoul_wrapper_(argv_tokenArray[2]);

        if (argc_tokenCount > 3u) {
            count = tinyCLI_strtoul_wrapper_(argv_tokenArray[3]);
        }

        uint32_t result = CRC32_calculate((const uint8_t *)startAddr, count);
        mHSS_PRINTF("CRC32: 0x%x" CRLF, result);
    } else {
        mHSS_PUTS("Usage:" CRLF
            "\tcrc32 0x<start_addr> 0x<length>" CRLF
            CRLF);
    }
}

static void tinyCLI_HexDump_(void)
{
    static size_t hexdump_count = 256u;
    static uintptr_t hexdump_startAddr = 0u;


    if (argc_tokenCount == 2u) {
        // repeating hexdump command with no args => keep going with
        // same count
        if (hexdump_startAddr == 0u) {
            hexdump_startAddr = (const uintptr_t)HSS_DDR_GetStart() - hexdump_count;
        };
        hexdump_startAddr = hexdump_startAddr + hexdump_count;

        HSS_TinyCLI_HexDump((uint8_t *)hexdump_startAddr, hexdump_count);
    } else if (argc_tokenCount > 2u) {
        hexdump_startAddr = tinyCLI_strtoul_wrapper_(argv_tokenArray[2]);

        if (argc_tokenCount > 3u) {
            hexdump_count = tinyCLI_strtoul_wrapper_(argv_tokenArray[3]);
        }

        HSS_TinyCLI_HexDump((uint8_t *)hexdump_startAddr, hexdump_count);
    } else {
        mHSS_PUTS("Usage:" CRLF
            "\thexdump 0x<start_addr> 0x<length>" CRLF
            CRLF);
    }
}

static void tinyCLI_CmdHandler_(int tokenId)
{
#if IS_ENABLED(CONFIG_SERVICE_YMODEM)
    void hss_loader_ymodem_loop(void);
#endif
    void _start(void);

    size_t index;
    switch (tokenId) {
    case CMD_HELP:
        tinyCLI_PrintHelp_();
        break;

    case CMD_VERSION:
        tinyCLI_PrintVersion_();
        break;

    case CMD_UPTIME:
        tinyCLI_PrintUptime_();
        break;

    case CMD_DEBUG:
        tinyCLI_Debug_();
        break;

#if IS_ENABLED(CONFIG_SERVICE_YMODEM)
    case CMD_YMODEM:
        hss_loader_ymodem_loop();
        break;
#endif

    case CMD_RESET:
        tinyCLI_Reset_();
        break;

    case CMD_QUIT:
        quitFlag = true;
        break;

    case CMD_BOOT:
        quitFlag = tinyCLI_Boot_();
        break;

#if IS_ENABLED(CONFIG_MEMTEST)
    case CMD_MEMTEST:
        tinyCLI_MemTest_();
        break;
#endif

#if IS_ENABLED(CONFIG_SERVICE_QSPI) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_PAYLOAD) || IS_ENABLED(CONFIG_SERVICE_SPI))
    case CMD_QSPI:
        HSS_BootSelectQSPI();
        break;
#endif

#if IS_ENABLED(CONFIG_SERVICE_MMC) && (IS_ENABLED(CONFIG_SERVICE_QSPI) || IS_ENABLED(CONFIG_SERVICE_PAYLOAD) || IS_ENABLED(CONFIG_SERVICE_SPI))
    case CMD_MMC:
        HSS_BootSelectMMC();
        break;
#endif

#if IS_ENABLED(CONFIG_SERVICE_PAYLOAD) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_QSPI) || IS_ENABLED(CONFIG_SERVICE_SPI))
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
            HSS_TinyCLI_WaitForUSBMSCDDone();
        }
        break;
#endif

#if IS_ENABLED(CONFIG_SERVICE_SCRUB)
    case CMD_SCRUB:
	scrub_dump_stats();
        break;
#endif

    default:
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Unknown command %d (%lu tokens)" CRLF, tokenId, argc_tokenCount);
        for (index = 1u; index < argc_tokenCount; index++) {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Argument: %s" CRLF, argv_tokenArray[index]);
        }
        break;
    }

    mHSS_PUTS("" CRLF);
}

#if !IS_ENABLED(CONFIG_SERVICE_TINYCLI_REGISTER)
static bool tinyCLI_Getline_(char **pBuffer, size_t *pBufLen);
static bool tinyCLI_Getline_(char **pBuffer, size_t *pBufLen)
{
    bool result = false;
    ssize_t status = 0;

    status = uart_getline(pBuffer, pBufLen);

    if (status < 0) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Problem reading input" CRLF);
    } else {
        result = true;
    }

    return result;
}
#endif

size_t HSS_TinyCLI_ParseIntoTokens(char *pBuffer)
{
    size_t i = 0u;
    static char *strtok_string = NULL;

    char *pToken = strtok_r(pBuffer, "\n ", &strtok_string);
    while ((pToken != NULL) && (i < mMAX_NUM_TOKENS))  {
        argv_tokenArray[i] = pToken;
        i++;
        pToken = strtok_r(NULL, "\n ", &strtok_string);
    }

    argc_tokenCount = i;
    return i;
}

void HSS_TinyCLI_Execute(void)
{
    size_t keyIndex, cmdIndex;
    bool matchFoundFlag =
        tinyCLI_NameToKeyIndex_(cmdKeys, ARRAY_SIZE(cmdKeys), argv_tokenArray[0], &keyIndex)
        && tinyCLI_CmdIdToCommandIndex_(cmdKeys[keyIndex].tokenId, &cmdIndex);

    if (matchFoundFlag) {
        if (commands[cmdIndex].warnIfPostInit && postInit) {
            mHSS_DEBUG_PRINTF(LOG_WARN,
                "Command %s may cause problems post boot." CRLF
                "Please type it again if you definitely want to execute it"
                CRLF CRLF, cmdKeys[keyIndex].name);
            commands[cmdIndex].warnIfPostInit = false;
        } else {
            commands[cmdIndex].handler(commands[cmdIndex].tokenId);
        }
    } else {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Unknown command >>%s<<." CRLF CRLF, argv_tokenArray[0]);
    }
}

bool HSS_TinyCLI_IndicatePostInit(void)
{
    postInit = true;
    return postInit;
}

bool HSS_TinyCLI_Parser(void)
{
    bool keyPressedFlag = false;
    uint8_t rcv_buf;

    keyPressedFlag = HSS_ShowTimeout("Press a key to enter CLI, ESC to skip" CRLF,
        CONFIG_SERVICE_TINYCLI_TIMEOUT, &rcv_buf);

    if (!keyPressedFlag) {
        mHSS_FANCY_PUTS(LOG_NORMAL, "CLI boot interrupt timeout" CRLF);
    } else {
        mHSS_FANCY_PUTS(LOG_NORMAL, "Type HELP for list of commands" CRLF);
        while (!quitFlag) {
#if !IS_ENABLED(CONFIG_SERVICE_TINYCLI_REGISTER)
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
#  if IS_ENABLED(CONFIG_SERVICE_USBDMSC) && IS_ENABLED(CONFIG_SERVICE_MMC)
            RunStateMachine(&usbdmsc_service);
#  endif
#endif
        }
    }

    return true;
}

#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_MONITOR)
void HSS_TinyCLI_RunMonitors(void)
{
    size_t i;

    for (i = 0u; i < ARRAY_SIZE(monitors); ++i) {
        if (monitors[i].active
            && HSS_Timer_IsElapsed(monitors[i].time, monitors[i].interval_sec * ONE_SEC)) {
            mHSS_DEBUG_PRINTF(LOG_STATUS,": ");
            HSS_TinyCLI_HexDump((uint8_t *)monitors[i].startAddr, monitors[i].count);
            monitors[0].time = HSS_GetTime();
       }
    }
}
#endif
