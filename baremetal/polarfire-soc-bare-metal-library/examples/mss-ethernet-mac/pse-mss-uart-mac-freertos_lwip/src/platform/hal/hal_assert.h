/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */
/*******************************************************************************
 * 
 * SVN $Revision: 11865 $
 * SVN $Date: 2019-07-29 15:28:05 +0100 (Mon, 29 Jul 2019) $
 */
#ifndef HAL_ASSERT_HEADER
#define HAL_ASSERT_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#if defined(NDEBUG)
/***************************************************************************//**
 * HAL_ASSERT() is defined out when the NDEBUG symbol is used.
 ******************************************************************************/
#define HAL_ASSERT(CHECK)

#else
/***************************************************************************//**
 * Default behaviour for HAL_ASSERT() macro:
 *------------------------------------------------------------------------------
  The behaviour is toolchain specific and project setting specific.
 ******************************************************************************/
#define HAL_ASSERT(CHECK)     ASSERT(CHECK);

#endif  /* NDEBUG */

#ifdef __cplusplus
}
#endif

#endif  /* HAL_ASSERT_HEADER */

