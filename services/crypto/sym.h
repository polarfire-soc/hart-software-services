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
     C header file for Symmetric Key Cryptography in CAL.
   ------------------------------------------------------------------- */

#ifndef SYM_H
#define SYM_H

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

#ifndef SYM_C
#ifdef __cplusplus
extern "C" {
#endif

/* Published API Functions */
/* --------- --- --------- */
extern SATR CALSymTrfRes(SATBOOL bBlock);

extern SATR CALSymEncrypt(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, 
  SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc,
  void *pDest, SATUINT32_t uiLen);
  
extern SATR CALSymDecrypt(SATSYMTYPE eSymType, const SATUINT32_t *puiKey,
  SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc,
  void *pDest, SATUINT32_t uiLen);

extern SATR CALSymEncryptDMA(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, 
  SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc,
  void *pDest, SATUINT32_t uiLen, SATUINT32_t uiDMAChConfig);
  
extern SATR CALSymDecryptDMA(SATSYMTYPE eSymType, const SATUINT32_t *puiKey,
  SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc,
  void *pDest, SATUINT32_t uiLen, SATUINT32_t uiDMAChConfig);

extern SATR CALSymEncryptKR(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, 
  SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc,
  void *pDest, SATUINT32_t uiLen, SATUINT32_t uiKRF);
  
extern SATR CALSymDecryptKR(SATSYMTYPE eSymType, const SATUINT32_t *puiKey,
  SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc,
  void *pDest, SATUINT32_t uiLen, SATUINT32_t uiKRF);

extern SATR CALSymEncryptKRDMA(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, 
  SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc,
  void *pDest, SATUINT32_t uiLen, SATUINT32_t uiKRF, SATUINT32_t uiDMAChConfig);
  
extern SATR CALSymDecryptKRDMA(SATSYMTYPE eSymType, const SATUINT32_t *puiKey,
  SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc,
  void *pDest, SATUINT32_t uiLen, SATUINT32_t uiKRF, SATUINT32_t uiDMAChConfig);
  
extern SATR CALSymEncAuth(SATSYMTYPE eSymType, const SATUINT32_t *puiKey,
  SATSYMMODE eMode, void *pIV, const void *pSrc, void *pDest,
  SATUINT32_t uiEncLen, void *pAuth, SATUINT32_t uiAuthLen, void *pMAC,
  SATUINT32_t uiMACLen);
  
extern SATR CALSymDecVerify(SATSYMTYPE eSymType, const SATUINT32_t *puiKey,
  SATSYMMODE eMode, void *pIV, const void *pSrc, void *pDest,
  SATUINT32_t uiEncLen, void *pAuth,  SATUINT32_t uiAuthLen, void *pMAC,
  SATUINT32_t uiMACLen);

extern SATR CALSymKw(SATSYMTYPE eSymType, const SATUINT32_t *puiKEK, 
  const SATUINT32_t *puiInKey, SATUINT32_t *puiOutKey, SATUINT32_t uiLen,
  SATBOOL bWrap);

extern SATR CALSymKwp(SATSYMTYPE eSymType, const SATUINT32_t *puiKEK,
  const SATUINT32_t *puiInKey, SATUINT32_t *puiOutKey, SATUINT32_t uiLen,
  SATBOOL bWrap);

extern SATR CALSymEncAuthDMA(SATSYMTYPE eSymType, const SATUINT32_t *puiKey,
  SATSYMMODE eMode, void *pIV, const void *pSrc, void *pDest,
  SATUINT32_t uiEncLen, void *pAuth, SATUINT32_t uiAuthLen, void *pMAC,
  SATUINT32_t uiMACLen, SATUINT32_t uiDMAChConfig);
  
extern SATR CALSymDecVerifyDMA(SATSYMTYPE eSymType, const SATUINT32_t *puiKey,
  SATSYMMODE eMode, void *pIV, const void *pSrc, void *pDest,
  SATUINT32_t uiEncLen, void *pAuth,  SATUINT32_t uiAuthLen, void *pMAC,
  SATUINT32_t uiMACLen, SATUINT32_t uiDMAChConfig);
  

/* Unpublished Function Prototypes */
/* ----------- -------- ---------- */

/* -------- ------ --------- */
/* External Global Variables */
/* -------- ------ --------- */

#ifdef __cplusplus
}
#endif
#endif
#endif
