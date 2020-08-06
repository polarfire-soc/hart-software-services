/*******************************************************************************
 * Copyright 2019-2020 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */
#ifndef MSS_ASSERT_H
#define MSS_ASSERT_H

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

#ifdef __cplusplus
}
#endif

#endif  /* MSS_ASSERT_H */
