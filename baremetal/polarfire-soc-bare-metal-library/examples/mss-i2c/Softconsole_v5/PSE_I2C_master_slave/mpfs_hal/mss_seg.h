/***************************************************************************
 * (c) Copyright 2018 Microsemi-PRO Embedded Systems Solutions. All rights reserved.
 *
 * @file seg.h
 * @author Microsemi-PRO Embedded Systems Solutions
 * @brief segmentation block defines
 *
 * These blocks allow the DDR memory to be allocated to cached, non-cached regions and trace
 * depending on the amount of DDR memory physically connected. Conceptually an address offset
 * is added/subtracted from the DDR address provided by the Core Complex to point at a base
 * address in the DDR memory.
 *
 * The AXI bus simply passes through the segmentation block, and the address is modified.
 *
 * There are two segmentation blocks, they are grouped into the same address ranges as the
 * MPU blocks. Each one has seven 32-segmentation registers, but only two in SEG0 and five
 * in SEG1 are actually implemented.
 *
 * SVN $Revision: 10515 $
 * SVN $Date: 2018-11-08 18:00:02 +0000 (Thu, 08 Nov 2018) $
 */

#ifndef SEG_H
#define SEG_H


#include <stdint.h>
#include "mss_sysreg.h"

typedef union{                                                         /*!< CFG_REGDIMM register definition*/
    __IO  uint32_t                       SEG_REG;
    struct
    {
        volatile int32_t    offset : 15;    /* This sets offset that is added to address bits [37:24] to convert the Corecomplex address to DDR address. (twos complement) */
        volatile int32_t    rsrvd  : 16;    /* Reserved */
        volatile int32_t    locked : 31;    /* When set to ‘1’ the configuration cannot be changed until a reset occurs. */
    } bitfield;
} DDR_SEG_REG;

typedef struct {

    DDR_SEG_REG reg[7];
} seg_t;

#define SEG0 ((seg_t*) 0x20005d00)
#define SEG1 ((seg_t*) 0x20005e00)

#endif

