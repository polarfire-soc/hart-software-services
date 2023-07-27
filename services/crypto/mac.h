/* -------------------------------------------------------------------
   $Rev: 1296 $ $Date: 2017-08-01 13:54:48 -0400 (Tue, 01 Aug 2017) $
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
      C header file for CAL MAC functions.
   ------------------------------------------------------------------- */

#ifndef MAC_H
#define MAC_H

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
#ifndef MAC_C
#ifdef __cplusplus
extern "C" {
#endif

/* Published API Functions */
/* --------- --- --------- */
extern SATR CALMAC(SATMACTYPE eMACType, const SATUINT32_t *pKey, SATUINT32_t uiKeyLen,
  const void *pMsg, SATUINT32_t uiMsgLen, void *pMAC);

extern SATR CALMACDMA(SATMACTYPE eMACType, const SATUINT32_t *pKey, 
  SATUINT32_t uiKeyLen, const void *pMsg, SATUINT32_t uiMsgLen, 
  const void *pMAC, SATUINT32_t uiDMAChConfig);

extern SATR CALMACIni(SATMACTYPE eMACType, const SATUINT32_t *pKey, 
  SATUINT32_t uiKeyLen, SATUINT32_t uiMsgLen);

extern SATR CALMACCtx(const SATRESHANDLE hResource, 
  const SATRESCONTEXTPTR pContext, const void *pMsg, SATUINT32_t uiMsgLen,
  void *pHash, const SATUINT8_t uiFlag);

extern SATR CALMACCtxIni(const SATRESCONTEXTPTR pContext, const SATHASHTYPE eHashType,
  const SATUINT32_t *pKey, SATUINT32_t uiKeyLen);

extern SATR CALMACWrite(const void *pBuffer, SATUINT32_t uiBufLen);

extern SATR CALMACRead(void *pMAC);


/* Unpublished Function Prototypes */
/* ----------- -------- ---------- */
extern SATUINT32_t iGetMACKeyLen(SATMACTYPE eMACType);

extern SATR CALMACCtxLoad(const SATRESHANDLE hResource, 
  SATRESCONTEXTPTR const pContext, SATUINT32_t uiRunLen);

extern SATUINT32_t iGetMACRunLen(SATRESCONTEXTPTR const pContext);


/* -------- ------ --------- */
/* External Global Variables */
/* -------- ------ --------- */

#ifdef __cplusplus
}
#endif
#endif
#endif

