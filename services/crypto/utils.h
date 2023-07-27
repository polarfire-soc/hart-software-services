/* -------------------------------------------------------------------
   $Rev: 1300 $ $Date: 2017-08-07 11:36:02 -0400 (Mon, 07 Aug 2017) $
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
      C header file for CAL utility functions.
   ------------------------------------------------------------------- */

#ifndef UTILS_H
#define UTILS_H

/* -------- */
/* Includes */
/* -------- */
#include "caltypes.h"
#include "calenum.h"


/* ------- */
/* Defines */
/* ------- */

/* ---- ----------- */
/* Type Definitions */
/* ---- ----------- */

/* -------- -------- ---------- */
/* External Function Prototypes */
/* -------- -------- ---------- */

#ifndef UTILS_C
#ifdef __cplusplus
extern "C" {
#endif

/* Published API Functions */
/* --------- --- --------- */

/* Unpublished Function Prototypes */
/* ----------- -------- ---------- */
extern void CALMemCopy(void *pDst, const void *pSrc, SATUINT32_t uiNum8);

extern void CALMemClear( void * pLoc, SATUINT32_t uiNum );

extern void CALMemClear32( SATUINT32_t* puiLoc, SATUINT32_t uiLen32 );

extern SATBOOL CALMemCmp(const void *pA, const void *pB, SATUINT32_t uiNum);

extern void CALVol32MemLoad(volatile SATUINT32_t * puiDst,  const void * pSrc, 
  SATUINT32_t uiNum32);

extern void CALVol32MemRead(void * pDst,  volatile SATUINT32_t * puiSrc,
  SATUINT32_t uiNum32);

extern void CALByteReverse(SATUINT32_t* puiArray, SATINT32_t iNumberBytes);

extern void CALByteReverseWord(SATUINT32_t* puiArray, SATINT32_t iNumberWords);

extern void CALWordReverse(SATUINT32_t *puiArray, SATINT32_t iNumberWords);

extern void vAppendNonzero(void *puiArray, SATUINT32_t uiLen, 
  SATUINT32_t uiBlockSize);

extern void vAppendZero(void *puiArray, SATUINT32_t uiLen, 
  SATUINT32_t uiBlockSize);

/* -------- ------ --------- */
/* External Global Variables */
/* -------- ------ --------- */

#ifdef __cplusplus
}
#endif
#endif
#endif

