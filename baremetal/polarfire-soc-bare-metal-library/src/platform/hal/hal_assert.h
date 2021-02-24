/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */
#ifndef ASSERT_HEADER
#define ASSERT_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * ASSERT() implementation.
 ******************************************************************************/
/* Disable assertions if we do not recognize the compiler. */
#if defined ( __GNUC__ )
#if defined(NDEBUG)
#define ASSERT(CHECK)
#else
#define ASSERT(CHECK)\
    do { \
        if (!(CHECK)) \
        { \
            __asm volatile ("ebreak"); \
        }\
    } while(0);
#endif /* NDEBUG check */
#endif /* compiler check */

#if defined(NDEBUG)
/***************************************************************************//**
 * ASSERT() is defined out when the NDEBUG symbol is used.
 ******************************************************************************/
#define ASSERT(CHECK)

#else
/***************************************************************************//**
 * Default behaviour for ASSERT() macro:
 *------------------------------------------------------------------------------
  The behaviour is toolchain specific and project setting specific.
 ******************************************************************************/
#define ASSERT(CHECK)     ASSERT(CHECK);

#endif  /* NDEBUG */

#ifdef __cplusplus
}
#endif

#endif  /* ASSERT_HEADER */

