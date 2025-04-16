/*
 * Copyright 2019-2025 Microchip FPGA Embedded Systems Solutions.
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
#include "ddr_service.h"
#include "csr_helper.h"
#include "reboot_service.h"
#include "wdog_service.h"
#include "hss_perfctr.h"
#include "profiling.h"
#include "hss_trigger.h"
#include "u54_state.h"

#include "hss_registry.h"
#include "assert.h"

#include "clocks/hw_mss_clks.h" // LIBERO_SETTING_MSS_RTC_TOGGLE_CLK

#if IS_ENABLED(CONFIG_SERVICE_USBDMSC) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_QSPI))
#    include "usbdmsc_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_QSPI)
#    include "qspi_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_SCRUB)
#    include "scrub_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_BEU)
#    include "beu_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_HEALTHMON)
#    include "healthmon_service.h"
#endif

#if IS_ENABLED(CONFIG_CRYPTO_SIGNING)
#    include "hss_boot_secure.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_ENABLE_PREBOOT_TIMEOUT)
#  include "hss_clock.h"
#endif

#define mMAX_NUM_TOKENS 40
static size_t argc_tokenCount = 0u;
static char *argv_tokenArray[mMAX_NUM_TOKENS];
static size_t tokenId;

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

struct tinycli_cmd {
    const int tokenId;
    const char * const name;
    const char * const helpString;
    void (* const handler)(void);
};

struct tinycli_toplevel_cmd_safe {
    const int tokenId;
    bool warnIfPostInit;
};

static bool tinyCLI_NameToCmdIndex_(struct tinycli_cmd const * const pCmds, size_t numCmds,
    char const * const pToken, size_t *pIndex);
static void tinyCLI_PrintVersion_(void);
static void tinyCLI_PrintUptime_(void);
static void tinyCLI_PrintHelp_(void);
static void tinyCLI_Debug_(void);
static void tinyCLI_Reset_(void);
#if IS_ENABLED(CONFIG_SERVICE_BOOT)
#if IS_ENABLED(CONFIG_SERVICE_MMC)
static void tinyCLI_Boot_List_(void);
static void tinyCLI_Boot_Select_(void);
#endif
static void tinyCLI_Boot_Info_(void);
static void tinyCLI_Boot_(void);
#endif
#if IS_ENABLED(CONFIG_SERVICE_QSPI)
static void tinyCLI_QSPI_Scan_(void);
static void tinyCLI_QSPI_Erase_(void);
#endif
static void tinyCLI_QSPI_(void);
#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_MONITOR)
static void tinyCLI_MonitorHelp_(void)
static void tinyCLI_MonitorCreate_(void);
static void tinyCLI_MonitorDestroy_(void);
static void tinyCLI_MonitorEnable_(void);
static void tinyCLI_MonitorDisable_(void);
static void tinyCLI_MonitorList_(void);
static void tinyCLI_Monitor_(void);
#endif
static void tinyCLI_CRC32_(void);
static void tinyCLI_HexDump_(void);
#if IS_ENABLED(CONFIG_MEMTEST)
static void tinyCLI_MemTest_(void);
#endif
static void tinyCLI_UnsupportedBootMechanism_(char const * const pName);
#if IS_ENABLED(CONFIG_SERVICE_YMODEM)
static void tinyCLI_YModem_(void);
#endif
#if IS_ENABLED(CONFIG_SERVICE_SCRUB)
static void tinyCLI_Scrub_(void);
#endif
static void output_duration_(char const * const description, const uint32_t val, bool continuation);
static void tinyCLI_DumpStateMachines_(void);
static void tinyCLI_IPIDumpStats_(void);
static void tinyCLI_EMMC_(void);
static void tinyCLI_MMC_(void);
static void tinyCLI_SDCARD_(void);
static void tinyCLI_Payload_(void);
static void tinyCLI_SPI_(void);
#if IS_ENABLED(CONFIG_SERVICE_USBDMSC) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_QSPI))
static void tinyCLI_USBDMSC_(void);
#endif
#if IS_ENABLED(CONFIG_SERVICE_BEU)
static void tinyCLI_BEU_(void);
static void tinyCLI_ECC_(void);
#endif
#if IS_ENABLED(CONFIG_SERVICE_HEALTHMON)
static void tinyCLI_HEALTHMON_(void);
#endif
#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
static void tinyCLI_PerfCtrs_(void);
#endif
#if IS_ENABLED(CONFIG_DEBUG_PROFILING_SUPPORT)
static void tinyCLI_ProfileCtrs_(void);
#endif
static void tinyCLI_OpenSBI_(void);
static void tinyCLI_Seg_(void);
static void tinyCLI_L2Cache_(void);
static bool dispatch_command_(struct tinycli_cmd const * const pCmds, size_t arraySize, uint8_t level);
static void display_help_(struct tinycli_cmd const * const pCmds, size_t arraySize, uint8_t level);

enum CmdId {
    CMD_YMODEM,
    CMD_BOOT,
    CMD_RESET,
    CMD_HELP,
    CMD_VERSION,
    CMD_UPTIME,
    CMD_DEBUG,
    CMD_MEMTEST,
    CMD_QSPI,
    CMD_EMMC,
    CMD_SDCARD,
    CMD_MMC,
    CMD_PAYLOAD,
    CMD_SPI,
    CMD_USBDMSC,
    CMD_SCRUB,
    CMD_ECC,
    CMD_INVALID,

    CMD_DBG_BEU,
    CMD_DBG_HEALTHMON,
    CMD_DBG_SM,
    CMD_DBG_IPI,
    CMD_DBG_CRC32,
    CMD_DBG_HEXDUMP,
    CMD_DBG_MONITOR,
    CMD_DBG_OPENSBI,
    CMD_DBG_SEG,
    CMD_DBG_L2CACHE,
    CMD_DBG_PERFCTR,
    CMD_DBG_WDOG,

    CMD_DBG_MONITOR_CREATE,
    CMD_DBG_MONITOR_DESTROY,
    CMD_DBG_MONITOR_ENABLE,
    CMD_DBG_MONITOR_DISABLE,
    CMD_DBG_MONITOR_LIST,

    CMD_BOOT_INFO,
    CMD_BOOT_LIST,
    CMD_BOOT_SELECT,

    CMD_QSPI_ERASE,
    CMD_QSPI_SCAN,
};

#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_MONITOR)
static const struct tinycli_cmd monitorCmds[] = {
    { CMD_DBG_MONITOR_CREATE,  "CREATE",  "<interval> 0x<start_addr> 0x<length>", tinyCLI_MonitorCreate_ },
    { CMD_DBG_MONITOR_DESTROY, "DESTROY", "<index> ", tinyCLI_MonitorDestroy_ },
    { CMD_DBG_MONITOR_ENABLE,  "ENABLE",  "<index> ", tinyCLI_MonitorEnable_ },
    { CMD_DBG_MONITOR_DISABLE, "DISABLE", "<index> ", tinyCLI_MonitorDisable_ },
    { CMD_DBG_MONITOR_LIST,    "LIST",    "", tinyCLI_MonitorList_ },
};
#endif

static const struct tinycli_cmd debugCmds[] = {
#if IS_ENABLED(CONFIG_SERVICE_BEU)
    { CMD_DBG_BEU,      "BEU",     "debug Bus Error Unit monitor", tinyCLI_BEU_ },
#endif
#if IS_ENABLED(CONFIG_SERVICE_HEALTHMON)
    { CMD_DBG_HEALTHMON, "HEALTHMON", "debug health monitor", tinyCLI_HEALTHMON_ },
#endif
    { CMD_DBG_SM,       "SM",      "debug state machines", tinyCLI_DumpStateMachines_ },
    { CMD_DBG_IPI,      "IPI",     "debug HSS IPI Queues", tinyCLI_IPIDumpStats_ },
    { CMD_DBG_CRC32,    "CRC32",   "calculate CRC32 over memory region", tinyCLI_CRC32_ },
    { CMD_DBG_HEXDUMP,  "HEXDUMP", "display memory as hex dump", tinyCLI_HexDump_ },
#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_MONITOR)
    { CMD_DBG_MONITOR,  "MONITOR", "monitor memory locations periodically", tinyCLI_Monitor_ },
#endif
    { CMD_DBG_OPENSBI,  "OPENSBI", "debug OpenSBI state", tinyCLI_OpenSBI_ },
    { CMD_DBG_SEG,      "SEG",     "display seg registers", tinyCLI_Seg_ },
    { CMD_DBG_L2CACHE,  "L2CACHE", "display l2cache settings", tinyCLI_L2Cache_ },
#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
    { CMD_DBG_PERFCTR , "PERFCTR", "display perf counters", tinyCLI_PerfCtrs_ },
#endif
#if IS_ENABLED(CONFIG_DEBUG_PROFILING_SUPPORT)
    { CMD_DBG_PERFCTR , "PROFILE", "display profiling counters", tinyCLI_ProfileCtrs_ },
#endif
#if IS_ENABLED(CONFIG_SERVICE_WDOG)
    { CMD_DBG_WDOG ,    "WDOG",    "display watchdog statistics", HSS_Wdog_DumpStats },
#endif
};

#if IS_ENABLED(CONFIG_SERVICE_BOOT)
static const struct tinycli_cmd bootCmds[] = {
    { CMD_BOOT_INFO,   "INFO",     "display info about currently registered boot image", tinyCLI_Boot_Info_ },
#if IS_ENABLED(CONFIG_SERVICE_MMC)
    { CMD_BOOT_LIST,   "LIST",     "list boot partitions", tinyCLI_Boot_List_ },
    { CMD_BOOT_SELECT, "SELECT",   "select active boot partition", tinyCLI_Boot_Select_ },
#endif
};
#endif

#if IS_ENABLED(CONFIG_SERVICE_QSPI)
static const struct tinycli_cmd qspiCmds[] = {
    { CMD_QSPI_ERASE,   "ERASE",     "ERASE QSPI Flash", tinyCLI_QSPI_Erase_ },
    { CMD_QSPI_SCAN,    "SCAN",      "Scan QSPI Flash for bad blocks", tinyCLI_QSPI_Scan_ },
};
#endif

static const struct tinycli_cmd toplevelCmds[] = {
#if IS_ENABLED(CONFIG_SERVICE_YMODEM)
    { CMD_YMODEM,  "YMODEM",  "Run YMODEM utility to download an image to DDR.", tinyCLI_YModem_ },
#endif
#if IS_ENABLED(CONFIG_SERVICE_BOOT)
    { CMD_BOOT,    "BOOT",    "Quit TinyCLI and return to regular boot process.", tinyCLI_Boot_ },
#endif
    { CMD_RESET,   "RESET",   "Reset PolarFire SoC.", tinyCLI_Reset_ },
    { CMD_HELP,    "HELP",    "Display command summary / command help information.", tinyCLI_PrintHelp_ },
    { CMD_VERSION, "VERSION", "Display system version information.", tinyCLI_PrintVersion_ },
    { CMD_UPTIME,  "UPTIME",  "Display uptime information.", tinyCLI_PrintUptime_ },
    { CMD_DEBUG,   "DEBUG",   "Display debug information.", tinyCLI_Debug_ },
#if IS_ENABLED(CONFIG_MEMTEST)
    { CMD_MEMTEST, "MEMTEST", "Full DDR memory test.", tinyCLI_MemTest_ },
#endif
    { CMD_QSPI,    "QSPI",    "Select boot via QSPI.", tinyCLI_QSPI_ },
    { CMD_EMMC,    "EMMC",    "Select boot via eMMC.", tinyCLI_EMMC_ },
    { CMD_MMC,     "MMC",     "Select boot via SDCARD/eMMC.", tinyCLI_MMC_ },
    { CMD_SDCARD,  "SDCARD",  "Select boot via SDCARD.", tinyCLI_SDCARD_ },
    { CMD_PAYLOAD, "PAYLOAD", "Select boot via payload.", tinyCLI_Payload_ },
    { CMD_SPI,     "SPI",     "Select boot via SPI.", tinyCLI_SPI_ },
#if IS_ENABLED(CONFIG_SERVICE_USBDMSC) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_QSPI))
    { CMD_USBDMSC, "USBDMSC", "Export eMMC as USBD Mass Storage Class.", tinyCLI_USBDMSC_ },
#endif
#if IS_ENABLED(CONFIG_SERVICE_SCRUB)
    { CMD_SCRUB,   "SCRUB",   "Dump Scrub service stats.", tinyCLI_Scrub_ },
#endif
#if IS_ENABLED(CONFIG_SERVICE_BEU)
    { CMD_ECC,     "ECC",   "Dump ECC stats.", tinyCLI_ECC_ },
#endif
};

static struct tinycli_toplevel_cmd_safe toplevelCmdsSafeAfterBootFlags[] = {
#if IS_ENABLED(CONFIG_SERVICE_YMODEM)
    { CMD_YMODEM,  true },
#endif
#if IS_ENABLED(CONFIG_SERVICE_BOOT)
    { CMD_BOOT,    false },
#endif
    { CMD_RESET,   true },
    { CMD_HELP,    false },
    { CMD_VERSION, false },
    { CMD_UPTIME,  false },
    { CMD_DEBUG,   false },
#if IS_ENABLED(CONFIG_MEMTEST)
    { CMD_MEMTEST, true },
#endif
    { CMD_QSPI,    true },
    { CMD_EMMC,    true },
    { CMD_MMC,     true },
    { CMD_SDCARD,  true },
    { CMD_PAYLOAD, true },
    { CMD_SPI,     true },
#if IS_ENABLED(CONFIG_SERVICE_USBDMSC) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_QSPI))
    { CMD_USBDMSC, true },
#endif
#if IS_ENABLED(CONFIG_SERVICE_SCRUB)
    { CMD_SCRUB,   false },
#endif
#if IS_ENABLED(CONFIG_SERVICE_BEU)
    { CMD_ECC,     false },
#endif
};

_Static_assert(ARRAY_SIZE(toplevelCmds) == ARRAY_SIZE(toplevelCmdsSafeAfterBootFlags), "arrays toplevelCmds and topLevelCmdSafeAfterBootFlags need to be same dimension");


/***********************************************************************/

static bool tinyCLI_NameToCmdIndex_(struct tinycli_cmd const * const pCmds, size_t numCmds,
    char const * const pToken, size_t *pIndex)
{
    bool result = false;
    size_t i;

    assert(pCmds);
    assert(pToken);
    assert(pIndex);

    for (i = 0u; i < numCmds; i++) {	// check for full match
        if (strncasecmp(pCmds[i].name, pToken, strlen(pCmds[i].name)) == 0) {
            result = true;
            *pIndex = i;
            break;
        }
    }

    if (!result) {			// if no match found, check for partial match
        size_t count = 0u;
        for (i = 0u; i < numCmds; i++) {
            if (strncasecmp(pCmds[i].name, pToken, strlen(pToken)) == 0) {
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

static unsigned long int tinyCLI_strtoul_wrapper_(const char *__restrict nptr)
{
    int base = 10;

    int tolower(int __c); // normally in ctypes.h, but we don't have a _ctype_ defined

    if ((nptr[0] == '0') && (tolower((int)nptr[1]) == 'x')) {
        base = 16;
    }

    return strtoul(nptr, NULL, base);
}

static void output_duration_(char const * const description, const uint32_t val, bool continuation)
{
    assert(description);
    if (val) {
        mHSS_FANCY_PRINTF_EX("%lu %s%s%s ", val, description, val == 1lu ? "":"s", continuation ? ",":"\n");
    }
}

static void tinyCLI_PrintUptime_(void)
{
    HSSTicks_t timeVal = CSR_GetTime();

    timeVal /= LIBERO_SETTING_MSS_RTC_TOGGLE_CLK;

    uint32_t days = timeVal / (24lu * 3600lu);
    timeVal = timeVal - (days * (24lu * 3600lu));

    uint32_t hours = timeVal / 3600lu;
    timeVal = timeVal - (hours * 3600lu);

    uint32_t mins = timeVal / 60lu;
    uint32_t secs = timeVal - (mins * 60lu);

    mHSS_FANCY_PRINTF(LOG_STATUS, "Uptime is ");
    output_duration_("day", days, true);
    output_duration_("hour", hours, true);
    output_duration_("minute", mins, true);
    output_duration_("second", secs, false);
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
        mHSS_FANCY_PRINTF(LOG_ERROR, "Failed!\n");
    } else {
        mHSS_FANCY_PRINTF(LOG_STATUS, "Passed!\n");
    }
}
#endif

static void tinyCLI_PrintHelp_(void)
{
    bool handled = false;

    if (argc_tokenCount > 1u) {
        for (size_t i = 1u; i < argc_tokenCount; ++i)  {
            size_t index;

            if (tinyCLI_NameToCmdIndex_(toplevelCmds, ARRAY_SIZE(toplevelCmds), argv_tokenArray[i], &index)) {
                mHSS_FANCY_PRINTF(LOG_NORMAL, "%s: %s\n", toplevelCmds[index].name, toplevelCmds[index].helpString);
                handled = true;
            }
        }
    }

    if (!handled) {
        display_help_(toplevelCmds, ARRAY_SIZE(toplevelCmds), 0u);
    }
}

static void tinyCLI_Reset_(void)
{
#if IS_ENABLED(CONFIG_SERVICE_REBOOT)
    HSS_reboot_cold(HSS_HART_ALL);
#endif
}

static void tinyCLI_Seg_(void)
{
    (void)HSS_DDRPrintSegConfig();
}

static void tinyCLI_OpenSBI_(void)
{
    extern void sbi_domain_dump_all(char const * const suffix);
    sbi_domain_dump_all("      ");

    HSS_U54_DumpStates();
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

#if IS_ENABLED(CONFIG_SERVICE_HEALTHMON)
static void tinyCLI_HEALTHMON_(void)
{
    HSS_Health_DumpStats();
}
#endif

#if IS_ENABLED(CONFIG_DEBUG_PERF_CTRS)
static void tinyCLI_PerfCtrs_(void)
{
    HSS_PerfCtr_DumpAll();
}
#endif

#if IS_ENABLED(CONFIG_DEBUG_PROFILING_SUPPORT)
static void tinyCLI_ProfileCtrs_(void)
{
    HSS_Profile_DumpAll();
}
#endif

static void tinyCLI_DumpStateMachines_(void)
{
    DumpStateMachineStats();
}

static void tinyCLI_IPIDumpStats_(void)
{
    IPI_DebugDumpStats();
}

static void tinyCLI_Debug_(void)
{
    if (!dispatch_command_(debugCmds, ARRAY_SIZE(debugCmds), 1u)) {
        display_help_(debugCmds, ARRAY_SIZE(debugCmds), 1u);
    }

}

extern struct HSS_Storage *HSS_BootGetActiveStorage(void);
#if IS_ENABLED(CONFIG_SERVICE_BOOT)

extern struct HSS_BootImage *pBootImage;
static void tinyCLI_Boot_Info_(void)
{
    if ((pBootImage->magic == mHSS_BOOT_MAGIC) || (pBootImage->magic == mHSS_COMPRESSED_MAGIC)) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Set Name: %s\n", pBootImage->set_name);
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Length:   %" PRIu64 " bytes\n", pBootImage->bootImageLength);

#if IS_ENABLED(CONFIG_CRYPTO_SIGNING)
        mHSS_DEBUG_PRINTF(LOG_ERROR, "Boot Image %s code signing\n",
            HSS_Boot_Secure_CheckCodeSigning(pBootImage) ? "passed" : "failed");
#endif
    } else {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Valid boot image not registered\n");
    }
}

#if IS_ENABLED(CONFIG_SERVICE_MMC)
static void tinyCLI_Boot_List_(void)
{
    HSS_GPT_t gpt;

    if (IS_ENABLED(CONFIG_SERVICE_MMC) && IS_ENABLED(CONFIG_SERVICE_BOOT_MMC_USE_GPT)) {
        struct HSS_Storage *pStorage = HSS_BootGetActiveStorage();
        assert(pStorage);

        uint32_t blockSize, eraseSize, blockCount;
        pStorage->getInfo(&blockSize, &eraseSize, &blockCount);

        gpt.lbaSize = blockSize;
        GPT_Init(&gpt, pStorage);
        bool result = GPT_ReadHeader(&gpt);

        if (result) {
            size_t srcIndex = 0u;
            HSS_GPT_PartitionEntry_t const * pGptPartitionEntry;

            result = GPT_FindBootSectorIndex(&gpt, &srcIndex, &pGptPartitionEntry);

            do {
                mHSS_DEBUG_PRINTF(LOG_NORMAL, "Boot Partition found at index %lu\n", srcIndex);

                if (!result) {
                    mHSS_DEBUG_PRINTF(LOG_ERROR, "GPT_FindBootSectorIndex() failed\n");
                } else {
                    struct HSS_BootImage localBootImage;
                    size_t srcLBAOffset;

                    result = GPT_PartitionIdToLBAOffset(&gpt, srcIndex, &srcLBAOffset);
                    if (!result) {
                        //mHSS_DEBUG_PRINTF(LOG_ERROR, "GPT_PartitionIdToLBAOffset() failed\n");
                    } else {
                        result = HSS_MMC_ReadBlock(&localBootImage, srcLBAOffset * gpt.lbaSize,
                            sizeof(struct HSS_BootImage));
                    }
                    if (!result) {
                        //mHSS_DEBUG_PRINTF(LOG_ERROR, "HSS_MMC_ReadBlock() failed\n");
                    } else {
                        if ((localBootImage.magic == mHSS_BOOT_MAGIC)
                            || (localBootImage.magic == mHSS_COMPRESSED_MAGIC)) {
                            mHSS_DEBUG_PRINTF(LOG_NORMAL, ">>%s<<\n", localBootImage.set_name);
                        } else {
                            //mHSS_DEBUG_PRINTF(LOG_ERROR, "magic failed\n");
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
        mHSS_PUTS("Usage:\n"
            "\tboot select <partition_index>\n"
            "\n");
    }
}
#endif

static void tinyCLI_Boot_(void)
{
    if (!dispatch_command_(bootCmds, ARRAY_SIZE(bootCmds), 1u)) {
        if (HSS_BootInit()) { HSS_BootHarts(); } // attempt boot
    }
}

#endif

#if IS_ENABLED(CONFIG_SERVICE_QSPI)
extern uint32_t Flash_scan_for_bad_blocks(uint16_t* buf);
extern void Flash_add_entry_to_bb_lut(uint16_t lba, uint16_t pba);

static void tinyCLI_QSPI_Erase_(void)
{
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "Erasing QSPI Flash\n");
    HSS_QSPIInit();
    HSS_QSPI_FlashChipErase();
}

static void tinyCLI_QSPI_Scan_(void)
{
    HSS_QSPIInit();
    HSS_QSPI_BadBlocksInfo();
}
#endif

static void tinyCLI_QSPI_(void)
{
#if IS_ENABLED(CONFIG_SERVICE_QSPI)

    if (argc_tokenCount > 1u) {
        if (!dispatch_command_(qspiCmds, ARRAY_SIZE(qspiCmds), 1u)) {
            display_help_(qspiCmds, ARRAY_SIZE(qspiCmds), 1u);
        }
    } else {
        HSS_BootSelectQSPI(); // qspi on its own
    }
#else
    tinyCLI_UnsupportedBootMechanism_("QSPI");
#endif
}

#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_MONITOR)
static void tinyCLI_MonitorCreate_(void)
{
    bool usageError = false;

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
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Allocated monitor index %lu\n", index);
        } else {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "All monitors are allocated\n");
        }
    } else {
        usageError = true;
    }

    if (usageError) {
        display_help_(monitorCmds, ARRAY_SIZE(monitorCmds), 2u);
    }
}

static void tinyCLI_MonitorDestroy_(void)
{
    bool usageError = false;

    if (argc_tokenCount > 3u) {
        size_t index = tinyCLI_strtoul_wrapper_(argv_tokenArray[3]);

        if (index < ARRAY_SIZE(monitors)) {
            if (monitors[index].allocated) {
                monitors[index].allocated = false;
                monitors[index].active = false;
                monitors[index].interval_sec = 0u;
                monitors[index].startAddr = 0u;
                monitors[index].count = 0u;
                mHSS_DEBUG_PRINTF(LOG_NORMAL, "Destroyed monitor index %lu\n", index);
            } else {
                mHSS_DEBUG_PRINTF(LOG_ERROR, "Monitor index %lu not allocated\n", index);
            }
        } else {
            usageError = true;
        }
    } else {
        usageError = true;
    }

    if (usageError) {
        display_help_(monitorCmds, ARRAY_SIZE(monitorCmds), 2u);
    }
}

static void tinyCLI_MonitorEnable_(void)
{
    bool usageError = false;

    if (argc_tokenCount > 3u) {
        size_t index = tinyCLI_strtoul_wrapper_(argv_tokenArray[3]);

        if (index < ARRAY_SIZE(monitors)) {
            if (monitors[index].allocated) {
                monitors[index].active = monitors[index].allocated;
                monitors[index].time = HSS_GetTime();
                mHSS_DEBUG_PRINTF(LOG_NORMAL, "Enabled monitor index %lu\n", index);
            } else {
                mHSS_DEBUG_PRINTF(LOG_ERROR, "Monitor index %lu not allocated\n", index);
            }
        } else {
            usageError = true;
        }
    } else {
        usageError = true;
    }

    if (usageError) {
        display_help_(monitorCmds, ARRAY_SIZE(monitorCmds), 2u);
    }
}

static void tinyCLI_MonitorDisable_(void)
{
    bool usageError = false;

    if (argc_tokenCount > 3u) {
        size_t index = tinyCLI_strtoul_wrapper_(argv_tokenArray[3]);

        if (index < ARRAY_SIZE(monitors)) {
            if (monitors[index].allocated) {
                monitors[index].active = false;
                mHSS_DEBUG_PRINTF(LOG_NORMAL, "Disabled monitor index %lu\n", index);
            } else {
                mHSS_DEBUG_PRINTF(LOG_ERROR, "Monitor index %lu not allocated\n", index);
            }
        } else {
            usageError = true;
        }
    } else {
        usageError = true;
    }

    if (usageError) {
        tinyCLI_MonitorHelp_();
        display_help_(monitorCmds, ARRAY_SIZE(monitorCmds), 2u);
    }
}

static void tinyCLI_MonitorList_(void)
{
    mHSS_PUTS(" Index Active Allocated Interval       Start_Addr    Count\n"
              "===========================================================\n");

    for (size_t index = 0; index < ARRAY_SIZE(monitors);  ++index) {
        mHSS_PRINTF(" % 5lu % 6d % 9d % 8lu %16x %8x\n",
            index,
            monitors[index].active,
            monitors[index].allocated,
            monitors[index].interval_sec,
            monitors[index].startAddr,
            monitors[index].count);
    }
    break;
}

static void tinyCLI_Monitor_(void)
{
    if (!dispatch_command_(monitorCmds, ARRAY_SIZE(monitorCmds), 2u)) {
        display_help_(monitorCmds, ARRAY_SIZE(monitorCmds), 2u);
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
        mHSS_PRINTF("CRC32: 0x%x\n", result);
    } else {
        mHSS_PUTS("Usage:\n"
            "\tcrc32 0x<start_addr> 0x<length>\n"
            "\n");
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
        mHSS_PUTS("Usage:\n"
            "\thexdump 0x<start_addr> 0x<length>\n"
            "\n");
    }
}

static void tinyCLI_UnsupportedBootMechanism_(char const * const pName)
{
    mHSS_PUTS(pName);
    mHSS_PUTS(" not supported in this build of the HSS.\n"
              "Supported boot mechanisms:\n");
    HSS_BootListStorageProviders();
}

#if IS_ENABLED(CONFIG_SERVICE_YMODEM)
static void tinyCLI_YModem_(void)
{
    void hss_loader_ymodem_loop(void);
    hss_loader_ymodem_loop();
}
#endif

#if IS_ENABLED(CONFIG_SERVICE_SCRUB)
static void tinyCLI_Scrub_(void)
{
    scrub_dump_stats();
}
#endif

static void tinyCLI_EMMC_(void)
{
#if IS_ENABLED(CONFIG_SERVICE_MMC)
    HSS_BootSelectEMMC();
#else
    tinyCLI_UnsupportedBootMechanism_("eMMC");
#endif
}

static void tinyCLI_MMC_(void)
{
#if IS_ENABLED(CONFIG_SERVICE_MMC)
    HSS_BootSelectMMC();
#else
    tinyCLI_UnsupportedBootMechanism_("eMMC/SDCard");
#endif
}

static void tinyCLI_SDCARD_(void)
{
#if IS_ENABLED(CONFIG_SERVICE_MMC)
    HSS_BootSelectSDCARD();
#else
    tinyCLI_UnsupportedBootMechanism_("SDCard");
#endif
}

static void tinyCLI_Payload_(void)
{
#if IS_ENABLED(CONFIG_SERVICE_BOOT_USE_PAYLOAD)
    HSS_BootSelectPayload();
#else
    tinyCLI_UnsupportedBootMechanism_("Payload");
#endif
}

static void tinyCLI_SPI_(void)
{
#if defined(CONFIG_SERVICE_SPI)
    HSS_BootSelectSPI();
#else
    tinyCLI_UnsupportedBootMechanism_("SPI");
#endif
}

#if IS_ENABLED(CONFIG_SERVICE_USBDMSC) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_QSPI))
static void tinyCLI_USBDMSC_(void)
{
    //USBDMSC_Init();
    USBDMSC_Activate();
    HSS_TinyCLI_WaitForUSBMSCDDone();
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
    if (!dispatch_command_(toplevelCmds, ARRAY_SIZE(toplevelCmds), 0u)) {
        mHSS_DEBUG_PRINTF(LOG_NORMAL, "Unknown command >>%s<< (%lu tokens)\n", argv_tokenArray[0], argc_tokenCount);
        for (int index = 1u; index < argc_tokenCount; index++) {
            mHSS_DEBUG_PRINTF(LOG_NORMAL, "Argument: %s\n", argv_tokenArray[index]);
        }
        mHSS_PUTS("\n");
    }
}

static void display_help_(struct tinycli_cmd const * const pCmds, size_t arraySize, uint8_t level)
{
    assert(pCmds);

    mHSS_PRINTF("Supported %scommands:\n", level ? "sub":"");

    for (size_t i = 0u; i < arraySize; i++) {
        if ((level == 0u) && (toplevelCmdsSafeAfterBootFlags[i].warnIfPostInit)) {
            HSS_Debug_Highlight(HSS_DEBUG_LOG_WARN);
        }
        mHSS_PUTS(pCmds[i].name);
        HSS_Debug_Highlight(HSS_DEBUG_LOG_NORMAL);
        mHSS_PUTC(' ');
        //mHSS_PRINTF(" - %s\n", pCmds[i].helpString);
    }

    mHSS_PUTS("\n");
}

static bool dispatch_command_(struct tinycli_cmd const * const pCmds, size_t arraySize, uint8_t level)
{
    bool handled = false;
    size_t cmdIndex;

    assert(pCmds);

    if (argc_tokenCount > level) {
        bool matchFoundFlag = tinyCLI_NameToCmdIndex_(pCmds, arraySize, argv_tokenArray[level], &cmdIndex);

        if (matchFoundFlag) {
            if (level == 0u) { // toplevel
                if (toplevelCmdsSafeAfterBootFlags[cmdIndex].warnIfPostInit && HSS_Trigger_IsNotified(EVENT_POST_BOOT)) {
                    mHSS_DEBUG_PRINTF(LOG_WARN,
                        "Command %s may cause problems post boot.\n"
                        "Please type it again if you definitely want to execute it"
                        "\n\n", pCmds[cmdIndex].name);
                    matchFoundFlag = false;
                    toplevelCmdsSafeAfterBootFlags[cmdIndex].warnIfPostInit = false; // disarming warning
                } else {
                    tokenId = pCmds[cmdIndex].tokenId;
                }
            }

            if (pCmds[cmdIndex].handler) {
                pCmds[cmdIndex].handler();
                handled = true;
            }
        }
    }

    return handled;
}

bool HSS_TinyCLI_Parser(void)
{
    bool keyPressedFlag = false;
    uint8_t rcv_buf;

    if (HSS_Trigger_IsNotified(EVENT_USBDMSC_REQUESTED)) {
        keyPressedFlag = true;
    } else {
        keyPressedFlag = HSS_ShowTimeout("parser: Press a key to enter CLI, ESC to skip\n",
            CONFIG_SERVICE_TINYCLI_TIMEOUT, &rcv_buf);
    }

    if (!keyPressedFlag) {
        mHSS_FANCY_PUTS(LOG_NORMAL, "CLI boot interrupt timeout\n");
    } else {
        mHSS_FANCY_PUTS(LOG_NORMAL, "Type HELP for list of commands\n");

#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_ENABLE_PREBOOT_TIMEOUT)
        HSSTicks_t readlineIdleTime = HSS_GetTime();
#endif

        {
            RunStateMachine(&tinycli_service);
#if IS_ENABLED(CONFIG_SERVICE_USBDMSC) && (IS_ENABLED(CONFIG_SERVICE_MMC) || IS_ENABLED(CONFIG_SERVICE_QSPI))
            RunStateMachine(&usbdmsc_service);
#endif

#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_ENABLE_PREBOOT_TIMEOUT)
#  define PREBOOT_IDLE_TIMEOUT (ONE_SEC * CONFIG_SERVICE_TINYCLI_PREBOOT_TIMEOUT)
            if (HSS_Timer_IsElapsed(readlineIdleTime, PREBOOT_IDLE_TIMEOUT)) {
                mHSS_DEBUG_PRINTF(LOG_ERROR, "***** Timeout on Pre-Boot TinyCLI *****\n");
                HSS_SpinDelay_Secs(5u);
                tinyCLI_Reset_();
            }
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

#if IS_ENABLED(CONFIG_SERVICE_BEU)
static void tinyCLI_ECC_(void)
{
    // addresses of L2 cache controller
    #define CACHE_CONTROLLER_BASE_ADDR 0x2010000
    #define CACHE_CONTROLLER_ECCInjectError_OFFSET 0x40
    #define CACHE_CONTROLLER_ECCDirFixCount_OFFSET 0x108
    #define CACHE_CONTROLLER_ECCDirFailCount_OFFSET 0x128
    #define CACHE_CONTROLLER_ECCDataFixCount_OFFSET 0x148
    #define CACHE_CONTROLLER_ECCDataFailCount_OFFSET 0x168

    uint32_t data = 0;

    HSS_BEU_DumpStats();

    // directory correctable errors
    data = mHSS_ReadRegU32(CACHE_CONTROLLER, ECCDirFixCount);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "% 45s:  %" PRIu64 "\n", "L2 ECCDirFixCount", data);

    // directory uncorrectable errors
    data = mHSS_ReadRegU32(CACHE_CONTROLLER, ECCDirFailCount);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "% 45s:  %" PRIu64 "\n", "L2 ECCDirFailCuont", data);

    // data correctable errors
    data = mHSS_ReadRegU32(CACHE_CONTROLLER, ECCDataFixCount);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "% 45s:  %" PRIu64 "\n", "L2 ECCDataFixCount", data);

    // data uncorrectable errors
    data = mHSS_ReadRegU32(CACHE_CONTROLLER, ECCDataFailCount);
    mHSS_DEBUG_PRINTF(LOG_NORMAL, "% 45s:  %" PRIu64 "\n", "L2 ECCDataFailCount", data);
}
#endif
