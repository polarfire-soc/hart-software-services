/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
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

#ifdef CONFIG_SERVICE_IPI_POLL
#  include "ipi_poll_service.h"
#endif

#ifdef CONFIG_SERVICE_BOOT
# include "hss_boot_service.h"
#endif

#ifdef CONFIG_SERVICE_UART
# include "uart_service.h"
#endif

#ifdef CONFIG_SERVICE_SGDMA
# include "sgdma_service.h"
#endif

#ifdef CONFIG_SERVICE_GOTO
# include "goto_service.h"
#endif

#ifdef CONFIG_SERVICE_OPENSBI
# include "opensbi_service.h"
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
    uint32_t immediate_arg, void *p_extended_buffer_in_ddr)
{
    (void)transaction_id;
    (void)source;
    (void)immediate_arg;
    (void)p_extended_buffer_in_ddr;

    mHSS_DEBUG_PRINTF("%s() called -- ignoring" CRLF);
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
#ifdef CONFIG_SERVICE_BOOT
    { IPI_MSG_BOOT_REQUEST, 		HSS_Boot_IPIHandler },
    { IPI_MSG_PMP_SETUP, 		HSS_Boot_PMPSetupHandler },
#else
    { IPI_MSG_BOOT_REQUEST, 		HSS_Null_IPIHandler },
    { IPI_MSG_PMP_SETUP, 		HSS_Null_IPIHandler },
#endif
#ifdef CONFIG_SERVICE_SPI
    { IPI_MSG_SPI_XFER, 		HSS_Null_IPIHandler },
#else
    { IPI_MSG_SPI_XFER, 		HSS_Null_IPIHandler },
#endif
#ifdef CONFIG_SERVICE_NET
    { IPI_MSG_NET_RXPOLL, 		HSS_Null_IPIHandler },
    { IPI_MSG_NET_TX, 			HSS_Null_IPIHandler },
#else
    { IPI_MSG_NET_RXPOLL, 		HSS_Null_IPIHandler },
    { IPI_MSG_NET_TX, 			HSS_Null_IPIHandler },
#endif
#ifdef CONFIG_SERVICE_SGDMA
    { IPI_MSG_SCATTERGATHER_DMA, 	HSS_SGDMA_IPIHandler },
#else
    { IPI_MSG_SCATTERGATHER_DMA, 	HSS_Null_IPIHandler },
#endif
#ifdef CONFIG_SERVICE_WDOG
    { IPI_MSG_WDOG_INIT, 		HSS_Null_IPIHandler },
#else
    { IPI_MSG_WDOG_INIT, 		HSS_Null_IPIHandler },
#endif
    { IPI_MSG_GPIO_SET, 		HSS_Null_IPIHandler },
#ifdef CONFIG_SERVICE_UART
    { IPI_MSG_UART_TX,                  HSS_UartTx_IPIHandler },
    { IPI_MSG_UART_POLL_RX,             HSS_UartPollRx_IPIHandler },
#else
    { IPI_MSG_UART_TX,                  HSS_Null_IPIHandler },
    { IPI_MSG_UART_POLL_RX,             HSS_Null_IPIHandler },
#endif
#ifdef CONFIG_SERVICE_POWERMODE
    { IPI_MSG_POWERMODE,                HSS_Null_IPIHandler },
#else
    { IPI_MSG_POWERMODE,                HSS_Null_IPIHandler },
#endif
#ifdef CONFIG_SERVICE_FLASHFREEZE
    { IPI_MSG_FLASHFREEZE,              HSS_Null_IPIHandler },
#else
    { IPI_MSG_FLASHFREEZE,              HSS_Null_IPIHandler },
#endif
    { IPI_MSG_ACK_PENDING, 	        IPI_ACK_IPIHandler },
    { IPI_MSG_ACK_COMPLETE, 		IPI_ACK_IPIHandler },
    { IPI_MSG_HALT, 			HSS_Null_IPIHandler },
    { IPI_MSG_CONTINUE, 		HSS_Null_IPIHandler },
#ifdef CONFIG_SERVICE_GOTO
    { IPI_MSG_GOTO, 			HSS_GOTO_IPIHandler },
#endif
#ifdef CONFIG_SERVICE_OPENSBI
    { IPI_MSG_OPENSBI_INIT, 	  	HSS_OpenSBI_IPIHandler },
#else
    { IPI_MSG_OPENSBI_INIT, 	  	HSS_Null_IPIHandler },
#endif
};
const size_t spanOfIpiRegistry = mSPAN_OF(ipiRegistry);

/******************************************************************************************************/

/*!
 * \brief State Machine Registration Table
 *
 * The following structure is used to connect in new state machines.
 *
 * \warning It must end with a 'NULL' sentinel to indicate end
 */
#ifdef CONFIG_SERVICE_IPI_POLL
extern struct StateMachine ipi_poll_service;
#endif
#ifdef CONFIG_SERVICE_BOOT
extern struct StateMachine boot_service1;
extern struct StateMachine boot_service2;
extern struct StateMachine boot_service3;
extern struct StateMachine boot_service4;
#endif
#ifdef CONFIG_SERVICE_SPI
extern struct StateMachine spi_service;
#endif
#ifdef CONFIG_SERVICE_UART
extern struct StateMachine uart_service;
#endif
#ifdef CONFIG_SERVICE_WDOG
extern struct StateMachine wdog_service;
#endif
#ifdef CONFIG_SERVICE_SGDMA
extern struct StateMachine sgdma_service;
#endif
#ifdef CONFIG_SERVICE_POWERMODE
extern struct StateMachine powermode_service;
#endif
#ifdef CONFIG_SERVICE_FLASHFREEZE
extern struct StateMachine flashfreeze_service;
#endif
#ifdef CONFIG_SERVICE_CRYPTO
extern struct StateMachine crypto_service;
#endif
extern struct StateMachine ddr_service;
#ifdef CONFIG_SERVICE_OPENSBI
extern struct StateMachine opensbi_service;
#endif

struct StateMachine /*@null@*/ * const pGlobalStateMachines[] = {
#ifdef CONFIG_SERVICE_IPI_POLL
    &ipi_poll_service,
#endif
#ifdef CONFIG_SERVICE_BOOT
    &boot_service1,
    &boot_service2,
    &boot_service3,
    &boot_service4,
#endif
#ifdef CONFIG_SERVICE_SPI
    &spi_service,
#endif
#ifdef CONFIG_SERVICE_UART
    &uart_service,
#endif
#ifdef CONFIG_SERVICE_WDOG
    &wdog_service,
#endif
#ifdef CONFIG_SERVICE_SGDMA
    &sgdma_service,
#endif
#ifdef CONFIG_SERVICE_POWERMODE
    &powermode_service,
#endif
#ifdef CONFIG_SERVICE_FLASHFREEZE
    &flashfreeze_service,
#endif
#ifdef CONFIG_SERVICE_CRYPTO
    &crypto_service,
#endif
    &ddr_service,
#ifdef CONFIG_SERVICE_OPENSBI
    &opensbi_service,
#endif
};
const size_t spanOfPGlobalStateMachines = mSPAN_OF(pGlobalStateMachines);

/******************************************************************************************************/
/*!
 * \brief Init Function Registration Table
 *
 * The following structure is used to connect in new init functions.
 */

#include "hss_init.h"
#include "hss_tinycli.h"
#include "hss_boot_pmp.h"
#include "hss_sys_setup.h"
#include "hss_board_init.h"
#ifdef CONFIG_MEMTEST
#  include "hss_memtest.h"
#endif

const struct InitFunction /*@null@*/ globalInitFunctions[] = {
    // Name                  FunctionPointer     Halt   Restart
    { "HSS_BoardInit",       HSS_BoardInit,      false, false },
    { "HSS_UARTInit",        HSS_UARTInit,       false, false },
#ifdef CONFIG_OPENSBI
    { "HSS_OpenSBIInit",     HSS_OpenSBIInit,    false, false },
#endif
#ifdef CONFIG_USE_LOGO
    { "HSS_LogoInit",        HSS_LogoInit,       false, false },
#endif
    { "HSS_E51_Banner",      HSS_E51_Banner,     false, false },
#ifdef CONFIG_MEMTEST
    { "HSS_MemTestDDRFast",  HSS_MemTestDDRFast, false, false },
#endif
#ifdef CONFIG_SERVICE_EMMC
    { "HSS_EMMCInit",        HSS_EMMCInit,       false, false },
#endif
#ifdef CONFIG_SERVICE_QSPI
    { "HSS_QSPIInit",        HSS_QSPIInit,       false, false },
#endif
#ifdef CONFIG_TINYCLI
    { "HSS_TinyCLI_Parser",  HSS_TinyCLI_Parser, false, false },
#endif
    { "IPI_QueuesInit",      IPI_QueuesInit,     false, false },
#ifdef CONFIG_SERVICE_BOOT
    { "HSS_PMP_Init",        HSS_PMP_Init,       false, false },
    { "HSS_BootInit",        HSS_BootInit,       false, false },
#endif
};
const size_t spanOfGlobalInitFunctions = mSPAN_OF(globalInitFunctions);


/******************************************************************************************************/
