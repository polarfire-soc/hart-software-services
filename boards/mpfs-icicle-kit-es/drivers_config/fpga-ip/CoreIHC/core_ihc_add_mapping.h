/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

 /*========================================================================*//**
  @mainpage Configuration for the Core IHC driver

    @section intro_sec Introduction
    Used to configure the driver

    @section

*//*==========================================================================*/


#ifndef CORE_IHC_ADD_MAPPING_H_
#define CORE_IHC_ADD_MAPPING_H_

#ifndef COMMON_AHB_BASE_ADD
#define COMMON_AHB_BASE_ADD     0x50000000UL
#endif
#ifndef IHC_HO_BASE_OFFSET
#define IHC_HO_BASE_OFFSET      0x00000000UL
#endif
#ifndef IHC_H1_BASE_OFFSET
#define IHC_H1_BASE_OFFSET      0x00000500UL
#endif
#ifndef IHC_H2_BASE_OFFSET
#define IHC_H2_BASE_OFFSET      0x00000A00UL
#endif
#ifndef IHC_H3_BASE_OFFSET
#define IHC_H3_BASE_OFFSET      0x00000F00UL
#endif
#ifndef IHC_H4_BASE_OFFSET
#define IHC_H4_BASE_OFFSET      0x00001400UL
#endif

/************** My Hart 0 ************/

#ifndef IHC_LOCAL_H0_REMOTE_H1
#define IHC_LOCAL_H0_REMOTE_H1      0x50000000
#endif

#ifndef IHC_LOCAL_H0_REMOTE_H2
#define IHC_LOCAL_H0_REMOTE_H2      0x50000100
#endif

#ifndef IHC_LOCAL_H0_REMOTE_H3
#define IHC_LOCAL_H0_REMOTE_H3      0x50000200
#endif

#ifndef IHC_LOCAL_H0_REMOTE_H4
#define IHC_LOCAL_H0_REMOTE_H4      0x50000300
#endif

#ifndef IHCA_LOCAL_H0
#define IHCA_LOCAL_H0               0x50000400
#endif

/************** My Hart 1 ************/

#ifndef IHC_LOCAL_H1_REMOTE_H0
#define IHC_LOCAL_H1_REMOTE_H0      0x50000500
#endif

#ifndef IHC_LOCAL_H1_REMOTE_H2
#define IHC_LOCAL_H1_REMOTE_H2      0x50000600
#endif

#ifndef IHC_LOCAL_H1_REMOTE_H3
#define IHC_LOCAL_H1_REMOTE_H3      0x50000700
#endif

#ifndef IHC_LOCAL_H1_REMOTE_H4
#define IHC_LOCAL_H1_REMOTE_H4      0x50000800
#endif

#ifndef IHCA_LOCAL_H1
#define IHCA_LOCAL_H1               0x50000900
#endif

/************** My Hart 2 ************/

#ifndef IHC_LOCAL_H2_REMOTE_H0
#define IHC_LOCAL_H2_REMOTE_H0      0x50000A00
#endif

#ifndef IHC_LOCAL_H2_REMOTE_H1
#define IHC_LOCAL_H2_REMOTE_H1      0x50000B00
#endif

#ifndef IHC_LOCAL_H2_REMOTE_H3
#define IHC_LOCAL_H2_REMOTE_H3      0x50000C00
#endif

#ifndef IHC_LOCAL_H2_REMOTE_H4
#define IHC_LOCAL_H2_REMOTE_H4      0x50000D00
#endif

#ifndef IHCA_LOCAL_H2
#define IHCA_LOCAL_H2               0x50000E00
#endif

/************** My Hart 3 ************/

#ifndef IHC_LOCAL_H3_REMOTE_H0
#define IHC_LOCAL_H3_REMOTE_H0      0x50000F00
#endif

#ifndef IHC_LOCAL_H3_REMOTE_H1
#define IHC_LOCAL_H3_REMOTE_H1      0x50001000
#endif

#ifndef IHC_LOCAL_H3_REMOTE_H2
#define IHC_LOCAL_H3_REMOTE_H2      0x50001100
#endif

#ifndef IHC_LOCAL_H3_REMOTE_H4
#define IHC_LOCAL_H3_REMOTE_H4      0x50001200
#endif

#ifndef IHCA_LOCAL_H3
#define IHCA_LOCAL_H3               0x50001300
#endif

/************** My Hart 4 ************/

#ifndef IHC_LOCAL_H4_REMOTE_H0
#define IHC_LOCAL_H4_REMOTE_H0      0x50001400
#endif

#ifndef IHC_LOCAL_H4_REMOTE_H1
#define IHC_LOCAL_H4_REMOTE_H1      0x50001500
#endif

#ifndef IHC_LOCAL_H4_REMOTE_H2
#define IHC_LOCAL_H4_REMOTE_H2      0x50001600
#endif

#ifndef IHC_LOCAL_H4_REMOTE_H3
#define IHC_LOCAL_H4_REMOTE_H3      0x50001700
#endif

#ifndef IHCA_LOCAL_H4
#define IHCA_LOCAL_H4               0x50001800
#endif

#endif /* CORE_IHC_ADD_MAPPING_H_ */

