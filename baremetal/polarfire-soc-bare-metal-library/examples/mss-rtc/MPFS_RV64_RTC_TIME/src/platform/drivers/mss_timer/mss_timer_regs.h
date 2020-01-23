/*******************************************************************************
 * (c) Copyright 2018 Microsemi SoC Products Group. All rights reserved.
 *
 * PolarFire microcontroller subsystem (MSS) timer register definition.
 *
 * SVN $Revision: 9987 $
 * SVN $Date: 2018-05-06 11:18:09 +0530 (Sun, 06 May 2018) $
 */

#ifndef __PSE_TIMER_REG_H_
#define __PSE_TIMER_REG_H_

#define __I  const volatile
#define __IO volatile

#ifdef __cplusplus
extern "C" {
#endif
/*----------------------------------------------------------------------------*/
/*---------------------------------- Timer -----------------------------------*/
/*----------------------------------------------------------------------------*/
typedef struct
{
    __I  uint32_t TIM1_VAL;
    __IO uint32_t TIM1_LOADVAL;
    __IO uint32_t TIM1_BGLOADVAL;
    __IO uint32_t TIM1_CTRL;
    __IO uint32_t TIM1_RIS;
    __I  uint32_t TIM1_MIS;

    __I  uint32_t TIM2_VAL;
    __IO uint32_t TIM2_LOADVAL;
    __IO uint32_t TIM2_BGLOADVAL;
    __IO uint32_t TIM2_CTRL;
    __IO uint32_t TIM2_RIS;
    __I  uint32_t TIM2_MIS;

    __I  uint32_t TIM64_VAL_U;
    __I  uint32_t TIM64_VAL_L;
    __IO uint32_t TIM64_LOADVAL_U;
    __IO uint32_t TIM64_LOADVAL_L;
    __IO uint32_t TIM64_BGLOADVAL_U;
    __IO uint32_t TIM64_BGLOADVAL_L;
    __IO uint32_t TIM64_CTRL;
    __IO uint32_t TIM64_RIS;
    __I  uint32_t TIM64_MIS;
    __IO uint32_t TIM64_MODE;
} TIMER_TypeDef;

/******************************************************************************/
/*                         Peripheral memory map                              */
/******************************************************************************/
#define TIMER_BASE         0x20125000

/******************************************************************************/
/*                         Peripheral declaration                             */
/******************************************************************************/
#define TIMER              ((volatile TIMER_TypeDef *) TIMER_BASE)

#define SYSREG_TIMER_SOFTRESET_MASK         ( (uint32_t)0x01u << 6u )

/*lint -restore */
#ifdef __cplusplus
}
#endif

#endif  /* __PSE_TIMER_REG_H_ */
