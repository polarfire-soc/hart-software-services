/*******************************************************************************
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 * 
 * Register bit offsets and masks defintions for PSE GPIO.
 * This driver is based on SmartFusion2 MSS GPIO driver v2.1.102
 * 
 * SVN $Revision: 10222 $
 * SVN $Date: 2018-06-29 14:32:17 +0530 (Fri, 29 Jun 2018) $
 */
#ifndef MSS_GPIO_REGS_H_
#define MSS_GPIO_REGS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/*----------------------------------- GPIO -----------------------------------*/
/*----------------------------------------------------------------------------*/
typedef struct
{
    volatile uint32_t GPIO_CFG[32];
    volatile uint32_t GPIO_IRQ;
    volatile const uint32_t GPIO_IN;
    volatile uint32_t GPIO_OUT;
    volatile uint32_t GPIO_CFG_ALL;
    volatile uint32_t GPIO_CFG_BYTE[4];
//    volatile uint32_t GPIO_CFG_BYTE1;
//    volatile uint32_t GPIO_CFG_BYTE2;
//    volatile uint32_t GPIO_CFG_BYTE3;
    volatile uint32_t GPIO_CLR_BITS;
    volatile uint32_t GPIO_SET_BITS;
} GPIO_TypeDef;




#ifdef __cplusplus                       
}
#endif

#endif /* MSS_UART_REGS_H_ */



