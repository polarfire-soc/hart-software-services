/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HSS Embedded Software
 *
 */

/*!
 * \file MPFS HSS Registered Tables
 * \brief Contains all registered tables.
 *
 * Items such as IPI handlers and state machines are statically registered
 * at compile time by design in the  MPFS HSS Embedded Software.
 * These tables are defined in this file.
 */

#include "config.h"
#include "hss_types.h"

#include "ssmb_ipi.h"

#if IS_ENABLED(CONFIG_SERVICE_IPI_POLL)
#  include "ipi_poll_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_BOOT)
# include "hss_boot_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_UART)
# include "uart_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_SPI)
# include "spi_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_SCRUB)
# include "scrub_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_SGDMA)
# include "sgdma_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_DDR)
# include "ddr_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_GOTO)
# include "goto_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_OPENSBI)
# include "opensbi_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_TINYCLI)
#  include "tinycli_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_USBDMSC)
#  include "usbdmsc_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_POWERMODE)
#  include "powermode_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_FLASHFREEZE)
#  include "flashfreeze_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_CRYPTO)
#  include "crypto_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_WDOG)
#  include "wdog_service.h"
#endif

#if IS_ENABLED(CONFIG_SERVICE_BEU)
#  include "beu_service.h"
#endif

#include "hss_debug.h"
#include "hss_registry.h"

#include <assert.h>

/*!
 * \brief Empty IPI handler
 *
 * Default empty handler for incoming IPI requests
 */
static enum IPIStatusCode HSS_Null_IPIHandler(TxId_t transaction_id, enum HSSHartId source,
    uint32_t immediate_arg, void *p_extended_buffer_in_ddr, void *p_ancilliary_buffer_in_ddr)
{
    (void)transaction_id;
    (void)source;
    (void)immediate_arg;
    (void)p_extended_buffer_in_ddr;
    (void)p_ancilliary_buffer_in_ddr;

    mHSS_DEBUG_PRINTF(LOG_NORMAL, "%s() called -- ignoring" CRLF);
    return IPI_SUCCESS;
}

/******************************************************************************************************/

/*!
 * \brief IPI Registration Table
 *
 * The following structure is used to connect in new IPIs...
 *
 * \warning It must end with a 'NULL' sentinel for the last handler function pointer to indicate end
 */

const struct IPI_Handler ipiRegistry[] = {
    { IPI_MSG_NO_MESSAGE, 		HSS_Null_IPIHandler },
#if IS_ENABLED(CONFIG_SERVICE_BOOT)
    { IPI_MSG_BOOT_REQUEST, 		HSS_Boot_IPIHandler },
    { IPI_MSG_PMP_SETUP, 		HSS_Boot_PMPSetupHandler },
#else
    { IPI_MSG_BOOT_REQUEST, 		HSS_Null_IPIHandler },
    { IPI_MSG_PMP_SETUP, 		HSS_Null_IPIHandler },
#endif
#if IS_ENABLED(CONFIG_SERVICE_SPI)
    { IPI_MSG_SPI_XFER, 		HSS_Null_IPIHandler },
#else
    { IPI_MSG_SPI_XFER, 		HSS_Null_IPIHandler },
#endif
#if IS_ENABLED(CONFIG_SERVICE_NET)
    { IPI_MSG_NET_RXPOLL, 		HSS_Null_IPIHandler },
    { IPI_MSG_NET_TX, 			HSS_Null_IPIHandler },
#else
    { IPI_MSG_NET_RXPOLL, 		HSS_Null_IPIHandler },
    { IPI_MSG_NET_TX, 			HSS_Null_IPIHandler },
#endif
#if IS_ENABLED(CONFIG_SERVICE_SGDMA)
    { IPI_MSG_SCATTERGATHER_DMA, 	HSS_SGDMA_IPIHandler },
#else
    { IPI_MSG_SCATTERGATHER_DMA, 	HSS_Null_IPIHandler },
#endif
#if IS_ENABLED(CONFIG_SERVICE_WDOG)
    { IPI_MSG_WDOG_INIT, 		HSS_Null_IPIHandler },
#else
    { IPI_MSG_WDOG_INIT, 		HSS_Null_IPIHandler },
#endif
    { IPI_MSG_GPIO_SET, 		HSS_Null_IPIHandler },
#if IS_ENABLED(CONFIG_SERVICE_UART)
    { IPI_MSG_UART_TX,                  HSS_UartTx_IPIHandler },
    { IPI_MSG_UART_POLL_RX,             HSS_UartPollRx_IPIHandler },
#else
    { IPI_MSG_UART_TX,                  HSS_Null_IPIHandler },
    { IPI_MSG_UART_POLL_RX,             HSS_Null_IPIHandler },
#endif
#if IS_ENABLED(CONFIG_SERVICE_POWERMODE)
    { IPI_MSG_POWERMODE,                HSS_Null_IPIHandler },
#else
    { IPI_MSG_POWERMODE,                HSS_Null_IPIHandler },
#endif
#if IS_ENABLED(CONFIG_SERVICE_FLASHFREEZE)
    { IPI_MSG_FLASHFREEZE,              HSS_Null_IPIHandler },
#else
    { IPI_MSG_FLASHFREEZE,              HSS_Null_IPIHandler },
#endif
    { IPI_MSG_ACK_PENDING, 	        IPI_ACK_IPIHandler },
    { IPI_MSG_ACK_COMPLETE, 		IPI_ACK_IPIHandler },
    { IPI_MSG_HALT, 			HSS_Null_IPIHandler },
    { IPI_MSG_CONTINUE, 		HSS_Null_IPIHandler },
#if IS_ENABLED(CONFIG_SERVICE_GOTO)
    { IPI_MSG_GOTO, 			HSS_GOTO_IPIHandler },
#else
    { IPI_MSG_GOTO, 			HSS_Null_IPIHandler },
#endif
#if IS_ENABLED(CONFIG_SERVICE_OPENSBI)
    { IPI_MSG_OPENSBI_INIT, 	  	HSS_OpenSBI_IPIHandler },
#else
    { IPI_MSG_OPENSBI_INIT, 	  	HSS_Null_IPIHandler },
#endif
};
const size_t spanOfIpiRegistry = ARRAY_SIZE(ipiRegistry);

/******************************************************************************************************/

/*!
 * \brief State Machine Registration Table
 *
 * The following structure is used to connect in new state machines.
 *
 * \warning It must end with a 'NULL' sentinel to indicate end
 */

struct StateMachine /*@null@*/ * const pGlobalStateMachines[] = {
#if IS_ENABLED(CONFIG_SERVICE_IPI_POLL)
    &ipi_poll_service,
#endif
#if IS_ENABLED(CONFIG_SERVICE_BOOT)
    &boot_service1,
    &boot_service2,
    &boot_service3,
    &boot_service4,
#endif
#if IS_ENABLED(CONFIG_SERVICE_SPI)
    &spi_service,
#endif
#if IS_ENABLED(CONFIG_SERVICE_UART)
    &uart_service,
#endif
#if IS_ENABLED(CONFIG_SERVICE_WDOG)
    &wdog_service,
#endif
#if IS_ENABLED(CONFIG_SERVICE_SGDMA)
    &sgdma_service,
#endif
#if IS_ENABLED(CONFIG_SERVICE_POWERMODE)
    &powermode_service,
#endif
#if IS_ENABLED(CONFIG_SERVICE_FLASHFREEZE)
    &flashfreeze_service,
#endif
#if IS_ENABLED(CONFIG_SERVICE_CRYPTO)
    &crypto_service,
#endif
#if IS_ENABLED(CONFIG_SERVICE_DDR)
    &ddr_service,
#endif
#if IS_ENABLED(CONFIG_SERVICE_OPENSBI)
    &opensbi_service,
#endif
#if IS_ENABLED(CONFIG_SERVICE_TINYCLI_REGISTER)
    &tinycli_service,
#endif
#if IS_ENABLED(CONFIG_SERVICE_USBDMSC)
    &usbdmsc_service,
#endif
#if IS_ENABLED(CONFIG_SERVICE_SCRUB)
    &scrub_service,
#endif
#if IS_ENABLED(CONFIG_SERVICE_BEU)
    &beu_service,
#endif
};
const size_t spanOfPGlobalStateMachines = ARRAY_SIZE(pGlobalStateMachines);

/******************************************************************************************************/
/*!
 * \brief Init Function Registration Table
 *
 * The following structure is used to connect in new init functions.
 */

#include "hss_init.h"
#include "hss_boot_init.h"
#include "hss_boot_pmp.h"
#include "hss_sys_setup.h"
#include "hss_board_init.h"
#include "device_serial_number.h"
#if IS_ENABLED(CONFIG_MEMTEST)
#  include "hss_memtest.h"
#endif
#if !IS_ENABLED(CONFIG_TINYCLI)
#  include "tinycli_service.h"
#endif

const struct InitFunction /*@null@*/ globalInitFunctions[] = {
    // Name                            FunctionPointer                Halt   Restart
    { "HSS_Setup_L2Cache",             HSS_Setup_L2Cache,             false, false },
    { "HSS_Init_RWDATA_BSS",           HSS_Init_RWDATA_BSS,           false, false },
    { "HSS_BoardInit",                 HSS_BoardInit,                 false, false },
    { "HSS_UARTInit",                  HSS_UARTInit,                  false, false },
    { "HSS_OpenSBIInit",               HSS_OpenSBIInit,               false, false },
#if IS_ENABLED(CONFIG_USE_LOGO)
    { "HSS_LogoInit",                  HSS_LogoInit,                  false, false },
#endif
    { "HSS_E51_Banner",                HSS_E51_Banner,                false, false },
    { "Device_Serial_Number_Init",     Device_Serial_Number_Init,     false, false },
    { "HSS_DDRPrintSegConfig",         HSS_DDRPrintSegConfig,         false, false },
    { "HSS_DDRPrintL2CacheWaysConfig", HSS_DDRPrintL2CacheWaysConfig, false, false },
#if IS_ENABLED(CONFIG_MEMTEST)
    { "HSS_MemTestDDRFast",            HSS_MemTestDDRFast,            false, false },
#endif
    { "HSS_BoardLateInit",             HSS_BoardLateInit,             false, false },
#if IS_ENABLED(CONFIG_SERVICE_MMC)
    { "HSS_MMCInit",                   HSS_MMCInit,                   false, false },
#endif
#if IS_ENABLED(CONFIG_SERVICE_QSPI)
    { "HSS_QSPIInit",                  HSS_QSPIInit,                  false, false },
#endif
#if IS_ENABLED(CONFIG_SERVICE_TINYCLI)
    { "HSS_TinyCLI_Parser",            HSS_TinyCLI_Parser,            false, false },
#endif
#if IS_ENABLED(CONFIG_USE_IHC)
    { "HSS_IHCInit",                   HSS_IHCInit,                   false, false },
#endif
    { "IPI_QueuesInit",                IPI_QueuesInit,                false, false },
#if IS_ENABLED(CONFIG_SERVICE_BOOT)
    { "HSS_PMP_Init",                  HSS_PMP_Init,                  false, false },
    { "HSS_BootInit",                  HSS_BootInit,                  false, true },
#endif
};
const size_t spanOfGlobalInitFunctions = ARRAY_SIZE(globalInitFunctions);


/******************************************************************************************************/
