/* -------------------------------------------------------------------
   $Rev: 1293 $ $Date: 2017-07-27 14:09:14 -0400 (Thu, 27 Jul 2017) $
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
      C header file for CAL hash functions.
   ------------------------------------------------------------------- */

#ifndef HASH_H
#define HASH_H

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
extern const SATHASHSIZE uiHashWordLen[SATHASHTYPE_LAST];


/* -------- -------- ---------- */
/* External Function Prototypes */
/* -------- -------- ---------- */

#ifndef HASH_C
#ifdef __cplusplus
extern "C" {
#endif

/* Published API Functions */
/* --------- --- --------- */
extern SATR CALHash(SATHASHTYPE eHashType, const void *pMsg,
  SATUINT32_t uiMsgLen, void *pHash);

extern SATR CALHashDMA(SATHASHTYPE eHashType, const void *pMsg,
  SATUINT32_t uiMsgLen, void *pHash, SATUINT32_t uiDMAChConfig);

extern SATR CALHashCtx(const SATRESHANDLE hResource,
  const SATRESCONTEXTPTR pContext, const void *pMsg, SATUINT32_t uiMsgLen, 
  void *pHash, const SATBOOL bFinal);

extern SATR CALHashCtxIni(const SATRESCONTEXTPTR pContext,
  const SATHASHTYPE eHashType);

extern SATR CALHashIni(SATHASHTYPE eHashType, SATUINT32_t uiMsgLen);

extern SATR CALHashWrite(const void *pBuffer, SATUINT32_t uiBufLen);

extern SATR CALHashRead(void *pHash);

extern SATR CALKeyTree(SATBOOL bPathSizeSel, const SATUINT32_t* puiKey,
  SATUINT8_t uiOpType, const SATUINT32_t* puiPath, SATUINT32_t* puiKeyOut);


/* Unpublished Function Prototypes */
/* ----------- -------- ---------- */
extern SATUINT32_t iGetHashLen(SATHASHTYPE eHashType);

extern SATUINT32_t iGetBlockLen(SATHASHTYPE eHashType);

extern SATR CALHashCtxLoad(const SATRESHANDLE hResource,
  SATRESCONTEXTPTR const pContext, SATUINT32_t uiRunLen);

extern SATUINT32_t iGetRunLen(SATRESCONTEXTPTR const pContext);

extern SATR CALHashTypeIni(void);


/* -------- ------ --------- */
/* External Global Variables */
/* -------- ------ --------- */

#ifdef __cplusplus
}
#endif
#endif
#endif
