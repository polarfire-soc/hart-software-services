/*******************************************************************************
 * Copyright 2019 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * @file mss_beu_def.h
 * @author Microchip FPGA Embedded Systems Solutions
 * @brief Bus Error Unit (BEU) fixed defines
 *
 */

#ifndef MSS_BEU_DEF_H
#define MSS_BEU_DEF_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum BEU_event_cause {
    BEU_EVENT_NO_ERROR                 = 0,
    BEU_EVENT_RESEVERD1                = 1,
    BEU_EVENT_ITIM_CORRECTABLE         = 2,
    BEU_EVENT_ITIM_UNCORRECTABLE       = 3,
    BEU_EVENT_RESERVED2                = 4,
    BEU_EVENT_TILELINK_BUS_ERROR       = 5,
    BEU_EVENT_DATA_CACHE_CORRECTABLE   = 6,
    BEU_EVENT_DATA_CACHE_UNCORRECTABLE = 7,
    MAX_BEU_CAUSES                     = BEU_EVENT_DATA_CACHE_UNCORRECTABLE + 1
};

typedef struct BEU_Type_
{
    volatile uint64_t CAUSE;     /*!< Cause of event, BEU_event_cause{} */
    volatile uint64_t VALUE;     /*!< Value of address where issue occurred */
    volatile uint64_t ENABLE;    /*!< Enable mask */
    volatile uint64_t PLIC_INT;  /*!< PLIC bit enables */
    volatile uint64_t ACCRUED;   /*!< events since this was last cleared */
    volatile uint64_t LOCAL_INT; /*!< Local int enables */
    volatile uint64_t reserved2[((0x1000U/8U) - 0x6U)];
} BEU_Type;

typedef struct BEU_Types_
{
    volatile BEU_Type regs[5];
} BEU_Types;

#define        MSS_BUS_ERROR_UNIT_H0                0x01700000UL
#define        MSS_BUS_ERROR_UNIT_H1                0x01701000UL
#define        MSS_BUS_ERROR_UNIT_H2                0x01702000UL
#define        MSS_BUS_ERROR_UNIT_H3                0x01703000UL
#define        MSS_BUS_ERROR_UNIT_H4                0x01704000UL

#define BEU    ((BEU_Types *)MSS_BUS_ERROR_UNIT_H0)

#ifdef __cplusplus
}
#endif

#endif /*MSS_SEG_H*/
