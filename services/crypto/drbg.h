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
      C header file for DRBG functions for CAL
   ------------------------------------------------------------------- */

#ifndef DRBG_H
#define DRBG_H

/* -------- */
/* Includes */
/* -------- */
#include "calpolicy.h"
#include "caltypes.h"


/* ------- */
/* Defines */
/* ------- */
#define CALDRBGENTROPYFACTOR 2

/* ---- ----------- */
/* Type Definitions */
/* ---- ----------- */

/* -------- -------- ---------- */
/* External Function Prototypes */
/* -------- -------- ---------- */
#ifndef DRBG_C
#ifdef __cplusplus
extern "C" {
#endif

/* Published API Functions */
/* --------- --- --------- */
extern SATR CALDRBGInstantiate(const SATUINT32_t *puiNonce,
  SATUINT32_t uiNonceLen, const SATUINT32_t *puiPzStr,
  SATUINT32_t uiPzStrLen,  SATSYMKEYSIZE eStrength, 
  SATUINT32_t uiEntropyFactor, SATUINT32_t uiReseedLim, SATBOOL bTesting);

extern SATR CALDRBGReseed(const SATUINT32_t *puiAddIn, SATUINT32_t uiAddInLen);

extern SATR CALDRBGGenerate(const SATUINT32_t *puiAddIn, SATUINT32_t uiAddInLen,
  SATBOOL bPredResist, SATUINT32_t *puiOut, SATUINT32_t uiOutBlocks);

extern SATR CALDRBGUninstantiate(void);

extern SATR CALDRBGGetCtx(DRBGCTXPTR drbgCtxExt);

extern SATR CALDRBGLoadCtx(DRBGCTXPTR drbgCtxExt);

extern SATR CALDRBGGetbInst(void);


/* Unpublished Function Prototypes */
/* ----------- -------- ---------- */

extern SATR CALDrbgTrfRes(SATBOOL bBlock);

extern void CALDrbgIni(void);


/* -------- ------ --------- */
/* External Global Variables */
/* -------- ------ --------- */


#ifdef __cplusplus
}
#endif
#endif
#endif
