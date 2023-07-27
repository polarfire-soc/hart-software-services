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
      C header file for F5200 AES function hardware implementation for 
   CAL.
   ------------------------------------------------------------------- */

#ifndef AESF5200_H
#define AESF5200_H

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
#ifndef AESF5200_C
#ifdef __cplusplus
extern "C" {
#endif

/* Published API Functions */
/* --------- --- --------- */

/* Unpublished Function Prototypes */
/* ----------- -------- ---------- */
extern SATR aesf5200aes (SATSYMTYPE eSymType, SATSYMMODE eMode, 
  void *pIV, SATBOOL bLoadIV, const void *pSrc, void *pDest, SATUINT32_t uiLen, 
  SATBOOL bDecrypt);

extern SATR aesf5200aesk (SATSYMTYPE eSymType, const SATUINT32_t *puiKey);

extern SATR aesf5200gcm (SATSYMTYPE eSymType, SATSYMMODE eMode, void *pIV,
  const void *pSrc, void *pDest, SATUINT32_t uiEncLen,
  const void *pAuth, SATUINT32_t uiAuthLen, void *pMAC, SATUINT32_t uiMACLen, 
  SATBOOL bDecrypt);

extern SATR aesf5200gcmdma(SATSYMTYPE eSymType, SATSYMMODE eMode, void *pIV,
    const void *pSrc, void *pDest, SATUINT32_t uiEncLen, const void *pAuth, 
    SATUINT32_t uiAuthLen, void *pMAC, SATUINT32_t uiMACLen, SATBOOL bDecrypt, 
    SATUINT32_t uiDMAChConfig);

extern SATR aesf5200kw(SATSYMTYPE eSymType, const SATUINT32_t *puiKEK, 
  const SATUINT32_t *puiInKey, SATUINT32_t *puiOutKey, SATUINT32_t uiLen,
  SATBOOL bWrap);

extern SATR aesf5200kwp(SATSYMTYPE eSymType, const SATUINT32_t *puiKEK,
  const SATUINT32_t *puiInKey, SATUINT32_t *puiOutKey, SATUINT32_t uiLen,
  SATBOOL bWrap);
  
extern SATR aesf5200kr(SATSYMTYPE eSymType, SATSYMMODE eMode,
  const SATUINT32_t *puiKey, void *pIV, SATBOOL bLoadIV, const void *pSrc,
  void *pDest, SATUINT32_t uiLen, SATUINT32_t uiKrf, SATBOOL bDecrypt);
  
extern SATR aesf5200dma(SATSYMTYPE eSymType, SATSYMMODE eMode, void *pIV,
  SATBOOL bLoadIV, const void *pExtSrc, void *pExtDest, SATUINT32_t uiLen, 
  SATUINT32_t uiDMAChConfig, SATBOOL bDecrypt);
  
extern SATR aesf5200krdma(SATSYMTYPE eSymType, SATSYMMODE eMode,
  const SATUINT32_t *puiKey, void *pIV, SATBOOL bLoadIV, const void *pExtSrc,
  void *pExtDest, SATUINT32_t uiLen, SATUINT32_t uiKrf, 
  SATUINT32_t uiDMAChConfig, SATBOOL bDecrypt);
  

/* -------- ------ --------- */
/* External Global Variables */
/* -------- ------ --------- */

#ifdef __cplusplus
}
#endif
#endif
#endif
