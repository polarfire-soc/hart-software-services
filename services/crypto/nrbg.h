/* -------------------------------------------------------------------
   $Rev: 1292 $ $Date: 2017-07-26 15:12:11 -0400 (Wed, 26 Jul 2017) $
   ------------------------------------------------------------------- */
/* ----------- MERCURY SYSTEMS INC IP PROTECTION HEADER ----------------
* (c) Mercury Systems, Inc. 2020. All rights reserved.
* Mercury Proprietary Information
* 
*
* This file, the information contained herein, and its documentation are 
* proprietary to Mercury Systems, Inc. This file, the information contained 
* herein, and its documentation may only be used, duplicated,  or disclosed 
* in accordance with the terms of a written license agreement between the 
* receiving party and Mercury or the receiving party and an authorized 
* licensee of Mercury.
*    
* Each copy of this file shall include all copyrights, trademarks, service 
* marks, and proprietary rights notices, if any.
* 
* ------------ MERCURY SYSTEMS INC IP PROTECTION HEADER --------------*/

/* -------------------------------------------------------------------
   Description:
      C header file for NRBG functions for CAL.
   ------------------------------------------------------------------- */

#ifndef NRBG_H
#define NRBG_H

/* -------- */
/* Includes */
/* -------- */
#include "calpolicy.h"
#include "caltypes.h"


/* ------- */
/* Defines */
/* ------- */

/* ---- ----------- */
/* Type Definitions */
/* ---- ----------- */

/* -------- -------- ---------- */
/* External Function Prototypes */
/* -------- -------- ---------- */
#ifndef NRBG_C
#ifdef __cplusplus
extern "C" {
#endif

/* Published API Functions */
/* --------- --- --------- */
extern SATR CALNRBGSetTestEntropy(SATUINT32_t * puiEntropy, 
  SATUINT32_t uiEntLen32);

extern SATR CALNRBGGetEntropy(SATUINT32_t * puiEntropy, SATUINT32_t uiEntLen32, 
  SATBOOL bTesting);

extern SATR CALNRBGConfig(SATUINT32_t uiWriteEn, SATUINT32_t uiCSR, 
  SATUINT32_t uiCntLim, SATUINT32_t uiVoTimer, SATUINT32_t uiFMsk, 
  SATUINT32_t* puiStatus); 

extern SATUINT32_t CALNRBGHealthStatus(void);


/* Unpublished Function Prototypes */
/* ----------- -------- ---------- */
extern SATUINT32_t CALNRBGGetTestEntLen(void);

extern SATR nrbgpkxgetentropy(SATUINT32_t * puiEntropy, 
  SATUINT32_t uiEntLen32);


/* -------- ------ --------- */
/* External Global Variables */
/* -------- ------ --------- */

#ifdef __cplusplus
}
#endif
#endif
#endif
