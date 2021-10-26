/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS HAL Embedded Software
 *
 */

 /*========================================================================*//**
  @mainpage Configuration for the MiV-IhC driver

    @section intro_sec Introduction
    Used to configure the driver

    @section

*//*==========================================================================*/


#ifndef MIV_IHC_CONFIG_H_
#define MIV_IHC_CONFIG_H_

#include "miv_ihc_add_mapping.h"

/*------------------------------------------------------------------------------
 * define the monitor hart (HSS hart) used in our system
 */
#define HSS_HART_MASK               HART0_MASK
#define HSS_HART_ID                 HART0_ID

/*------------------------------------------------------------------------------
 * HSS_REMOTE_HARTS_MASK
 * This is used to define the harts the HSS is communicating with
 */
#define HSS_REMOTE_HARTS_MASK 		(HART1_MASK | HART2_MASK |HART3_MASK | HART4_MASK)

/*------------------------------------------------------------------------------
 * Contex A and B hart ID's used in this system - user defined
 */
#define CONTEXTA_HARTID         0x01U
#define CONTEXTB_HARTID         0x04U

/*------------------------------------------------------------------------------
 * Define which harts are connected via comms channels to a particular hart
 * user defined
 */
#define IHCIA_H0_REMOTE_HARTS	(HSS_REMOTE_HARTS_MASK) /* connected to all harts */
#define IHCIA_H1_REMOTE_HARTS	(HSS_HART_MASK | HART4_MASK) /* HSS and Context B connected */
#define IHCIA_H2_REMOTE_HARTS	(HSS_HART_MASK)
#define IHCIA_H3_REMOTE_HARTS	(HSS_HART_MASK)
#define IHCIA_H4_REMOTE_HARTS	(HSS_HART_MASK | HART1_MASK) /* HSS and Context A connected */

/*------------------------------------------------------------------------------
 * interrupts enabled in this system design for a particular hart
 * User defined
 */
#define IHCIA_H0_REMOTE_HARTS_INTS    HSS_HART_DEFAULT_INT_EN  /* connected to all harts */
#define IHCIA_H1_REMOTE_HARTS_INTS    (HSS_HART_MP_INT_EN | HSS_HART_ACK_INT_EN | HART4_MP_INT_EN | HART4_ACK_INT_EN) /* HSS and Context B connected */
#define IHCIA_H2_REMOTE_HARTS_INTS    (HSS_HART_MP_INT_EN | HSS_HART_ACK_INT_EN)
#define IHCIA_H3_REMOTE_HARTS_INTS    (HSS_HART_MP_INT_EN | HSS_HART_ACK_INT_EN)
#define IHCIA_H4_REMOTE_HARTS_INTS    (HSS_HART_MP_INT_EN | HSS_HART_ACK_INT_EN | HART1_MP_INT_EN | HART1_ACK_INT_EN) /* HSS and Context A connected */

#endif /* MIV_IHC_CONFIG_H_ */

