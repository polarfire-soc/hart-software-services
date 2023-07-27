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
      C header file for SHA function hardware implementation for CAL.
   ------------------------------------------------------------------- */

#ifndef SHAF5200_H
#define SHAF5200_H

/* -------- */
/* Includes */
/* -------- */
#include "calpolicy.h"
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
#ifndef SHAF5200_C
#ifdef __cplusplus
extern "C" {
#endif

/* Published API Functions */
/* --------- --- --------- */

/* Unpublished Function Prototypes */
/* ----------- -------- ---------- */
extern SATR shapkxctx(SATRESCONTEXTPTR const pContext, const void *pBuffer, 
  SATUINT32_t uiBufLen,void *pHash, const SATBOOL bFinal);

extern SATR shapkxhmacctx(SATRESCONTEXTPTR const pContext, const void *pBuffer,
  SATUINT32_t uiBufLen, void *pHash, const SATUINT8_t uiFlag);

extern SATR shapkxctxload(SATRESCONTEXTPTR const pContext);

extern void shapkxctxunload(SATRESCONTEXTPTR const pContext);

extern SATR shapkxini(SATHASHTYPE eHashType, SATUINT32_t uiMsgLen);

extern SATR shapkxhmacini(SATHASHTYPE eHashType, SATUINT32_t uiMsgLen, 
  const void *pKey, SATUINT32_t uiKeyLen);

extern SATR shapkxwrite(const void *pBuffer, SATUINT32_t uiBufLen);

extern SATR shapkxhmacwrite(const void *pBuffer, SATUINT32_t uiBufLen);

extern SATR shapkxread(void *pHash, SATUINT32_t uiRevFlag);

extern SATR shapkxkeytree(SATBOOL bPathSizeSel, const SATUINT32_t* puiKey, 
  SATUINT8_t uiOpType, const SATUINT32_t* puiPath, SATUINT32_t* puiKeyOut);

extern SATR shapkxdma(SATHASHTYPE eHashType, SATUINT32_t uiMsgLen,  
  const void *pExtSrc, const void *pExtDest, SATUINT32_t uiDMAChConfig);
  
extern SATR shapkxhmacdma(SATMACTYPE eMACType, const SATUINT32_t *pKey, 
  SATUINT32_t uiKeyLen, const void *pMsg, SATUINT32_t uiMsgLen, 
  const void *pMAC, SATUINT32_t uiDMAChConfig);

extern SATR shapkxhashtypeini(void);


/* -------- ------ --------- */
/* External Global Variables */
/* -------- ------ --------- */

#ifdef __cplusplus
}
#endif
#endif
#endif

