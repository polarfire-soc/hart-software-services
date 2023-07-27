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
      C header file for F5200 DRBG function hardware implementations
   in CAL.
   ------------------------------------------------------------------- */

#ifndef DRBGF5200_H
#define DRBGF5200_H

/* -------- */
/* Includes */
/* -------- */
#include "calpolicy.h"
#include "caltypes.h"
#include "calenum.h"


/* ------- */
/* Defines */
/* ------- */
#define RNXEFACTOR   (X52BER+2)  
#define RNXIRLEN     (X52BER+2)
#define RNXKEYORD    (X52BER+3)  
#define RNXRESPRED   (X52BER+4)  
#define RNXTESTSEL   (X52BER+5)  
#define RNXBADDIN    (X52BER+6)  
#define RNXPPSTR     (X52BER+7)  
#define RNXPCTX      (X52BER+8)
#define RNXPRENT     (X52BER+13)
#define RNXITMP3     (X52BER+20)
#define RNXPCTX2     (X52BER+22)
/* These use same locs as tb generator, but can change */
#define RNXRDATA     (X52BER_ENDIAN+0x24) 
#define RNXAIDATAOFF 0x0062
#define RNXAIDATA    (X52BER_ENDIAN+RNXAIDATAOFF)
#define RNXCTXOFF    0x0094
#define RNXCTX       (X52FPR+RNXCTXOFF)
#define RNXCTXV      (X52FPR_ENDIAN+RNXCTXOFF+6)
#define RNXCTXVMMR   (X52MMR_ENDIAN+RNXCTXOFF+6)
#define RNXTESTENT   (X52TSR_ENDIAN+8)

#define RNXCTXWORDS   18
#define RNXBLENLOC    (RNXCTX+RNXCTXWORDS)
#define RNXBOUTLENLOC (RNXCTX+RNXCTXWORDS+1)

#define RNXMAXTESTENT32 512
#define CALDRBGMAXADDINLEN X52_BER_LEN-RNXAIDATAOFF-4
#define CALDRBGMAXPSNONCELEN X52_BER_LEN-RNXAIDATAOFF-4


/* ---- ----------- */
/* Type Definitions */
/* ---- ----------- */

/* -------- -------- ---------- */
/* External Function Prototypes */
/* -------- -------- ---------- */
#ifndef DRBGF5200_C
#ifdef __cplusplus
extern "C" {
#endif

/* Published API Functions */
/* --------- --- --------- */

/* Unpublished Function Prototypes */
/* ----------- -------- ---------- */
extern SATR drbgf5200Ini(void);

extern SATR drbgf5200TrfRes(SATBOOL bBlock);

extern SATR drbgf5200instantiate(const SATUINT32_t *puiNonce,
  SATUINT32_t uiNonceLen, const SATUINT32_t *puiPzStr, SATUINT32_t uiPzStrLen, 
  SATSYMKEYSIZE eStrength, SATUINT32_t uiEntropyFactor, 
  SATUINT32_t uiReseedLim, SATBOOL bTesting);

extern SATR drbgf5200reseed(const SATUINT32_t *puiAddIn, 
  SATUINT32_t uiAddInLen);
  
extern SATR drbgf5200generate(const SATUINT32_t *puiAddIn,
  SATUINT32_t uiAddInLen, SATBOOL bPredResist, SATUINT32_t uiReqLen,
  SATUINT32_t *puiOut);

extern SATR drbgf5200uninstantiate(void);

extern SATR drbgf5200getctx(DRBGCTXPTR drbgCtxExt);

extern SATR drbgf5200loadctx(DRBGCTXPTR drbgCtxExt);


/* -------- ------ --------- */
/* External Global Variables */
/* -------- ------ --------- */

#ifdef __cplusplus
}
#endif
#endif
#endif

