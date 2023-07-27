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
     C header file for for CAL PK.
   ------------------------------------------------------------------- */

#ifndef PK_H
#define PK_H

/* -------- */
/* Includes */
/* -------- */
#include "calpolicy.h"
#include "caltypes.h"


/* -------- -------- ---------- */
/* External Function Prototypes */
/* -------- -------- ---------- */

#ifndef PK_C
#ifdef __cplusplus
extern "C" {
#endif

/* Published API Functions */
/* --------- --- --------- */
extern SATR CALPKTrfRes(SATBOOL bBlock);

extern SATR CALDSASign(const SATUINT32_t* puiHash, const SATUINT32_t* puiG,
  const SATUINT32_t* puiK, const SATUINT32_t* puiX, const SATUINT32_t* puiP,
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiQ, const SATUINT32_t* puiQMu,
  SATUINT32_t uiN, SATUINT32_t uiL, SATUINT32_t* puiSigR, SATUINT32_t* puiSigS);
  
extern SATR CALDSASignCM(const SATUINT32_t* puiHash, const SATUINT32_t* puiG,
  const SATUINT32_t* puiK, const SATUINT32_t* puiX, const SATUINT32_t* puiP,
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiQ, const SATUINT32_t* puiQMu,
  SATUINT32_t uiN, SATUINT32_t uiL, SATUINT32_t* puiSigR, SATUINT32_t* puiSigS);
  
extern SATR CALDSASignHash(const SATUINT32_t* puiMsg, SATHASHTYPE eHashType, 
  SATUINT32_t uiMsgLen, const SATUINT32_t* puiG, const SATUINT32_t* puiK, 
  const SATUINT32_t* puiX, const SATUINT32_t* puiP, const SATUINT32_t* puiPMu, 
  const SATUINT32_t* puiQ, const SATUINT32_t* puiQMu, SATUINT32_t uiN, 
  SATUINT32_t uiL, SATUINT32_t* puiSigR, SATUINT32_t* puiSigS,
  SATBOOL bDMA, SATUINT32_t uiDMAChConfig);

extern SATR CALDSASignHashCM(const SATUINT32_t* puiMsg, SATHASHTYPE eHashType, 
  SATUINT32_t uiMsgLen, const SATUINT32_t* puiG, const SATUINT32_t* puiK, 
  const SATUINT32_t* puiX, const SATUINT32_t* puiP, const SATUINT32_t* puiPMu, 
  const SATUINT32_t* puiQ, const SATUINT32_t* puiQMu, SATUINT32_t uiN, 
  SATUINT32_t uiL, SATUINT32_t* puiSigR, SATUINT32_t* puiSigS,
  SATBOOL bDMA, SATUINT32_t uiDMAChConfig);
  
extern SATR CALDSAVerify(const SATUINT32_t* puiHash, const SATUINT32_t* puiG,
  const SATUINT32_t* puiY, const SATUINT32_t* puiSigR, const SATUINT32_t* puiSigS,
  const SATUINT32_t* puiP, const SATUINT32_t* puiPMu, const SATUINT32_t* puiQ,
  const SATUINT32_t* puiQMu, SATUINT32_t uiN, SATUINT32_t uiL);
  
extern SATR CALDSAVerifyHash(const SATUINT32_t* puiMsg, SATHASHTYPE eHashType, 
  SATUINT32_t uiMsgLen, const SATUINT32_t* puiG, const SATUINT32_t* puiY, 
  const SATUINT32_t* puiR, const SATUINT32_t* puiS, const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiQ, const SATUINT32_t* puiQMu, 
  SATUINT32_t uiN, SATUINT32_t uiL, SATBOOL bDMA, SATUINT32_t uiDMAChConfig);
  
extern SATR CALECDHC(const SATUINT32_t* puiS, const SATUINT32_t* puiWx, 
  const SATUINT32_t* puiWy, const SATUINT32_t* puiB, const SATUINT32_t* puiMod,
  const SATUINT32_t* puiMu, const SATUINT32_t* puiK, const SATUINT32_t* puiR,
  const SATUINT32_t* puiRMu, SATUINT32_t uiLen, SATUINT32_t uiPtCompress,
  SATUINT32_t* puiZ);  
  
extern SATR CALECDSASign(const SATUINT32_t* puiHash, const SATUINT32_t* puiGx,
  const SATUINT32_t* puiGy, const SATUINT32_t* puiK, const SATUINT32_t* puiD,
  const SATUINT32_t* puiB, const SATUINT32_t* puiP, const SATUINT32_t* puiPMu,
  const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, SATUINT32_t uiLen,
  SATUINT32_t* puiSigR, SATUINT32_t* puiSigS);

extern SATR CALECDSASignTwist(const SATUINT32_t* puiHash, 
  const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, const SATUINT32_t* puiK, 
  const SATUINT32_t* puiD, const SATUINT32_t* puiB, const SATUINT32_t* puiZ, 
  const SATUINT32_t* puiP, const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, 
  const SATUINT32_t* puiNMu, SATUINT32_t uiLen, SATUINT32_t* puiSigR, 
  SATUINT32_t* puiSigS);  

extern SATR CALECDSASignCM(const SATUINT32_t* puiHash, const SATUINT32_t* puiGx,
  const SATUINT32_t* puiGy, const SATUINT32_t* puiK, const SATUINT32_t* puiD,
  const SATUINT32_t* puiB, const SATUINT32_t* puiP, const SATUINT32_t* puiPMu,
  const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, SATUINT32_t uiLen,
  SATUINT32_t* puiSigR, SATUINT32_t* puiSigS);    

extern SATR CALECDSASignTwistCM(const SATUINT32_t* puiHash, 
  const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, const SATUINT32_t* puiK, 
  const SATUINT32_t* puiD, const SATUINT32_t* puiB, const SATUINT32_t* puiZ, 
  const SATUINT32_t* puiP, const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, 
  const SATUINT32_t* puiNMu, SATUINT32_t uiLen, SATUINT32_t* puiSigR, 
  SATUINT32_t* puiSigS);  

extern SATR CALECDSASignHash(const SATUINT32_t* puiMsg, SATHASHTYPE eHashType, 
  SATUINT32_t uiMsgLen, const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, 
  const SATUINT32_t* puiK, const SATUINT32_t* puiD, const SATUINT32_t* puiB, 
  const SATUINT32_t* puiP, const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, 
  const SATUINT32_t* puiNMu, SATUINT32_t uiLen, SATUINT32_t* puiSigR, 
  SATUINT32_t* puiSigS, SATBOOL bDMA, SATUINT32_t uiDMAChConfig);

extern SATR CALECDSASignHashCM(const SATUINT32_t* puiMsg, SATHASHTYPE eHashType, 
  SATUINT32_t uiMsgLen, const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, 
  const SATUINT32_t* puiK, const SATUINT32_t* puiD, const SATUINT32_t* puiB, 
  const SATUINT32_t* puiP, const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, 
  const SATUINT32_t* puiNMu, SATUINT32_t uiLen, SATUINT32_t* puiSigR, 
  SATUINT32_t* puiSigS, SATBOOL bDMA, SATUINT32_t uiDMAChConfig);

extern SATR CALECDSASignTwistHash(const SATUINT32_t* puiMsg, 
  SATHASHTYPE eHashType, SATUINT32_t uiMsgLen, const SATUINT32_t* puiGx, 
  const SATUINT32_t* puiGy, const SATUINT32_t* puiK, const SATUINT32_t* puiD, 
  const SATUINT32_t* puiB, const SATUINT32_t* puiZ, const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, const SATUINT32_t* puiNMu,
  SATUINT32_t uiLen, SATUINT32_t* puiSigR, SATUINT32_t* puiSigS, SATBOOL bDMA, 
  SATUINT32_t uiDMAChConfig);

extern SATR CALECDSAVerify(const SATUINT32_t* puiHash, const SATUINT32_t* puiGx,
  const SATUINT32_t* puiGy, const SATUINT32_t* puiQx, const SATUINT32_t* puiQy,
  const SATUINT32_t* puiSigR, const SATUINT32_t* puiSigS,const 
  SATUINT32_t* puiB, const SATUINT32_t* puiP, const SATUINT32_t* puiPMu, 
  const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, SATUINT32_t uiLen,
  SATUINT32_t uiPtCompress);

extern SATR CALECDSAVerifyTwist(const SATUINT32_t* puiHash, const SATUINT32_t* puiGx, 
  const SATUINT32_t* puiGy, const SATUINT32_t* puiQx, const SATUINT32_t* puiQy,
  const SATUINT32_t* puiSigR, const SATUINT32_t* puiSigS,
  const SATUINT32_t* puiB, const SATUINT32_t* puiZ, const SATUINT32_t* puiP,
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, const SATUINT32_t* puiNMu,
  SATUINT32_t uiLen, SATUINT32_t uiPtCompress);

extern SATR CALECDSAVerifyHash(const SATUINT32_t* puiMsg, SATHASHTYPE eHashType,
  SATUINT32_t uiMsgLen, const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, 
  const SATUINT32_t* puiQx, const SATUINT32_t* puiQy, 
  const SATUINT32_t* puiSigR, const SATUINT32_t* puiSigS,
  const SATUINT32_t* puiB, const SATUINT32_t* puiP, const SATUINT32_t* puiPMu, 
  const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, 
  SATUINT32_t uiLen, SATUINT32_t uiPtCompress, SATBOOL bDMA, 
  SATUINT32_t uiDMAChConfig);

extern SATR CALECDSAVerifyTwistHash(const SATUINT32_t* puiMsg, 
  SATHASHTYPE eHashType, SATUINT32_t uiMsgLen, const SATUINT32_t* puiGx, 
  const SATUINT32_t* puiGy, const SATUINT32_t* puiQx, const SATUINT32_t* puiQy, 
  const SATUINT32_t* puiSigR, const SATUINT32_t* puiSigS, 
  const SATUINT32_t* puiB, const SATUINT32_t* puiZ, const SATUINT32_t* puiP,  
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, const SATUINT32_t* puiNMu,
  SATUINT32_t uiLen, SATUINT32_t uiPtCompress, SATBOOL bDMA, 
  SATUINT32_t uiDMAChConfig);

extern SATR CALECMult(const SATUINT32_t* puiMul, const SATUINT32_t* puiPx,
  const SATUINT32_t* puiPy, const SATUINT32_t* puiB, const SATUINT32_t* puiMod,
  const SATUINT32_t* puiMu, SATUINT32_t uiLen, SATUINT32_t uiPtCompress,
  SATUINT32_t* puiRx, SATUINT32_t* puiRy);
  
extern SATR CALECMultCM(const SATUINT32_t* puiMul, const SATUINT32_t* puiPx,
  const SATUINT32_t* puiPy, const SATUINT32_t* puiB, const SATUINT32_t* puiMod,
  const SATUINT32_t* puiMu, const SATUINT32_t* puiN, SATUINT32_t uiLen, 
  SATUINT32_t uiPtCompress, SATUINT32_t* puiRx, SATUINT32_t* puiRy);

extern SATR CALECMultTwist(const SATUINT32_t* puiMul, const SATUINT32_t* puiPx,
  const SATUINT32_t* puiPy, const SATUINT32_t* puiB, const SATUINT32_t* puiZ, 
  const SATUINT32_t* puiMod, const SATUINT32_t* puiMu, SATUINT32_t uiLen, 
  SATUINT32_t* puiRx, SATUINT32_t* puiRy);

extern SATR CALECMultTwistCM(const SATUINT32_t* puiMul,
  const SATUINT32_t* puiPx, const SATUINT32_t* puiPy, const SATUINT32_t* puiB,
  const SATUINT32_t* puiZ, const SATUINT32_t* puiMod,const SATUINT32_t* puiMu,
  const SATUINT32_t* puiN, SATUINT32_t uiLen, SATUINT32_t* puiRx, 
  SATUINT32_t* puiRy);
  
extern SATR CALECMultAdd(const SATUINT32_t* puiMul, const SATUINT32_t* puiPx,
  const SATUINT32_t* puiPy, const SATUINT32_t* puiQx, const SATUINT32_t* puiQy,
  const SATUINT32_t* puiB, const SATUINT32_t* puiMod, const SATUINT32_t* puiMu,
  SATUINT32_t uiLen, SATUINT32_t uiPtPCompress, SATUINT32_t uiPtQCompress,
  SATUINT32_t* puiRx, SATUINT32_t* puiRy);
  
extern SATR CALECPtValidate(const SATUINT32_t* puiPx, const SATUINT32_t* puiPy,
  const SATUINT32_t* puiB, const SATUINT32_t* puiMod, const SATUINT32_t* puiMu, 
  SATUINT32_t uiLen);

extern SATR CALECKeyPairGen(const SATUINT32_t* puiC, const SATUINT32_t* puiPx,
  const SATUINT32_t* puiPy, const SATUINT32_t* puiMod, const SATUINT32_t* puiMu, 
  const SATUINT32_t* puiNM1, const SATUINT32_t* puiNM1Mu, const SATUINT32_t* puiB, 
  SATUINT32_t* puiD, SATUINT32_t* puiQx, SATUINT32_t* puiQy, SATUINT32_t uiLen);

extern SATR CALExpo(const SATUINT32_t* puiBase, const SATUINT32_t* puiExpo, 
  const SATUINT32_t* puiMod, const SATUINT32_t* puiMu, SATUINT32_t uiExpLen, 
  SATUINT32_t uiModLen, SATUINT32_t* puiResult);
  
extern SATR CALExpoCM(const SATUINT32_t* puiBase, const SATUINT32_t* puiExpo, 
  const SATUINT32_t* puiMod, const SATUINT32_t* puiMu, SATUINT32_t uiExpLen, 
  SATUINT32_t uiModLen, SATUINT32_t* puiResult);
  
extern SATR CALMMult(const SATUINT32_t* puiA, const SATUINT32_t* puiB,
  const SATUINT32_t* puiMod, const SATUINT32_t* puiMu, SATUINT32_t uiModLen,
  SATUINT32_t* puiResult);
  
extern SATR CALMMultAdd(const SATUINT32_t* puiA, const SATUINT32_t* puiB,
  const SATUINT32_t* puiC, const SATUINT32_t* puiMod, const SATUINT32_t* puiMu, 
  SATUINT32_t uiModLen, SATUINT32_t* puiResult);
  
extern SATR CALModRed(const SATUINT32_t* puiA, const SATUINT32_t* puiMod,
  const SATUINT32_t* puiMu, SATUINT32_t uiALen, SATUINT32_t uiModLen, 
  SATUINT32_t* puiResult);
  
extern SATR CALPreCompute(const SATUINT32_t* puiMod, SATUINT32_t* puiMu, 
  SATUINT32_t uiModLen);
  
extern SATR CALRSACRT(const SATUINT32_t* puiCipher, const SATUINT32_t* puiQInv, 
  const SATUINT32_t* puiDP, const SATUINT32_t* puiDQ,const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiQ, const SATUINT32_t* puiQMu,
  SATUINT32_t uiLen, SATUINT32_t* puiPlain);
  
extern SATR CALRSACRTCM(const SATUINT32_t* puiCipher,
  const SATUINT32_t* puiQInv, const SATUINT32_t* puiDP,
  const SATUINT32_t* puiDQ, const SATUINT32_t * puiE, const SATUINT32_t* puiP,
  const SATUINT32_t* puiQ, const SATUINT32_t * puiN, 
  const SATUINT32_t * puiNMu, SATUINT32_t uiLen, SATUINT32_t uiELen, 
  SATUINT32_t* puiPlain);
  
extern SATR CALRSACRTSign(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, 
  const SATUINT32_t* puiHash, const SATUINT32_t* puiQInv, 
  const SATUINT32_t* puiDP, const SATUINT32_t* puiDQ, const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiQ, const SATUINT32_t* puiQMu,
  const SATUINT32_t* puiN, SATUINT32_t uiLen, SATUINT32_t* puiSig);

extern SATR CALRSACRTSignCM(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, 
  const SATUINT32_t* puiHash, const SATUINT32_t* puiE, const SATUINT32_t uiELen,
  const SATUINT32_t* puiQInv, const SATUINT32_t* puiDP, 
  const SATUINT32_t* puiDQ, const SATUINT32_t* puiP, const SATUINT32_t* puiQ,
  const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, SATUINT32_t uiLen,
  SATUINT32_t* puiSig);

extern SATR CALRSACRTSignHash(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, 
  const SATUINT32_t* puiMsg, const SATUINT32_t* puiQInv, 
  const SATUINT32_t* puiDP, const SATUINT32_t* puiDQ,const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiQ, const SATUINT32_t* puiQMu,
  const SATUINT32_t* puiN, SATUINT32_t uiMsgLen, SATUINT32_t uiModLen, 
  SATUINT32_t* puiSig, SATBOOL bDMA, SATUINT32_t uiDMAChConfig);

extern SATR CALRSACRTSignHashCM(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, 
  const SATUINT32_t* puiMsg, const SATUINT32_t* puiE, const SATUINT32_t uiELen,
  const SATUINT32_t* puiQInv, const SATUINT32_t* puiDP, 
  const SATUINT32_t* puiDQ, const SATUINT32_t* puiP, const SATUINT32_t* puiQ,
  const SATUINT32_t* puiN,  const SATUINT32_t* puiNMu, SATUINT32_t uiMsgLen,
  SATUINT32_t uiModLen, SATUINT32_t* puiSig, SATBOOL bDMA, SATUINT32_t uiDMAChConfig);
  
extern SATR CALRSASign(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, 
  const SATUINT32_t* puiHash, const SATUINT32_t* puiD, const SATUINT32_t* puiN, 
  const SATUINT32_t* puiNMu, SATUINT32_t uiModLen, SATUINT32_t* puiSig);
  
extern SATR CALRSASignHash(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, 
  const SATUINT32_t* puiMsg, const SATUINT32_t* puiD, const SATUINT32_t* puiN, 
  const SATUINT32_t* puiNMu, SATUINT32_t uiMsgLen, SATUINT32_t uiModLen, 
  SATUINT32_t* puiSig, SATBOOL bDMA, SATUINT32_t uiDMAChConfig);  
  
extern SATR CALRSAVerify(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType,
  const SATUINT32_t* puiHash, const SATUINT32_t* puiE, SATUINT32_t uiExpLen, 
  const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, SATUINT32_t uiModLen, 
  const SATUINT32_t* puiSig);
  
extern SATR CALRSAVerifyHash(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, 
  const SATUINT32_t* puiMsg, const SATUINT32_t* puiE, SATUINT32_t uiExpLen,
  const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, SATUINT32_t uiMsgLen, 
  SATUINT32_t uiModLen, const SATUINT32_t* puiSig, SATBOOL bDMA, 
  SATUINT32_t uiDMAChConfig);

extern SATR CALPurge52(SATBOOL bVerify);
 

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
