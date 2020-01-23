/*******************************************************************************
 * (c) Copyright 2008-2018 Microsemi SoC Products Group. All rights reserved.
 * 
 * SVN $Revision: 10514 $
 * SVN $Date: 2018-11-06 13:49:15 +0000 (Tue, 06 Nov 2018) $
 */
#ifndef HAL_ASSERT_HEADER
#define HAL_ASSERT_HEADER

#define NDEBUG 1

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

#endif  /* HAL_ASSERT_HEADER */

