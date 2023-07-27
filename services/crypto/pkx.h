/* -------------------------------------------------------------------
   $Rev: 1298 $ $Date: 2017-08-04 13:04:19 -0400 (Fri, 04 Aug 2017) $
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

#ifndef PKX_H
#define PKX_H

/* -------- */
/* Includes */
/* -------- */
#include "caltypes.h"
#include "calenum.h"


/* ------- */
/* Defines */
/* ------- */

/* X5200 Addressing */
#define X52BER ((volatile SATUINT32_t *) (PKX0_BASE + 0x00000000u))
#define X52MMR ((volatile SATUINT32_t *) (PKX0_BASE + 0x00001000u))
#define X52TSR ((volatile SATUINT32_t *) (PKX0_BASE + 0x00002000u))
#define X52FPR ((volatile SATUINT32_t *) (PKX0_BASE + 0x00003000u))
#define X52LIR ((volatile SATUINT32_t *) (PKX0_BASE + 0x00004000u))
#define X52CSR ((volatile SATUINT32_t *) (PKX0_BASE + 0x00007F80u))
#define X52CSRMERRS  ((volatile SATUINT32_t *) (PKX0_BASE + 0x00007F8Cu))
#define X52CSRMERRT0 ((volatile SATUINT32_t *) (PKX0_BASE + 0x00007F94u))

#if SAT_LITTLE_ENDIAN
#define X52BER_ENDIAN ((volatile SATUINT32_t *) (PKX0_BASE + 0x00008000u))
#define X52MMR_ENDIAN ((volatile SATUINT32_t *) (PKX0_BASE + 0x00009000u))
#define X52TSR_ENDIAN ((volatile SATUINT32_t *) (PKX0_BASE + 0x0000A000u))
#define X52FPR_ENDIAN ((volatile SATUINT32_t *) (PKX0_BASE + 0x0000B000u))
#define X52DMACONFIG_ENDIAN 0x8
#else
#define X52BER_ENDIAN ((volatile SATUINT32_t *) (PKX0_BASE + 0x00000000u))
#define X52MMR_ENDIAN ((volatile SATUINT32_t *) (PKX0_BASE + 0x00001000u))
#define X52TSR_ENDIAN ((volatile SATUINT32_t *) (PKX0_BASE + 0x00002000u))
#define X52FPR_ENDIAN ((volatile SATUINT32_t *) (PKX0_BASE + 0x00003000u))
#define X52DMACONFIG_ENDIAN 0x0
#endif

/* X5200 Macros */
#define X52GO(x) (0x10 | ((x)<<8))      

/* Counter Measures */
#define RANDLEN 4

/* X5200 CSRMAIN bit field masks. */
#define X52CSRMAINRST        1
#define X52CSRMAINCCMPLT     2
#define X52CSRMAINCMPLT      4
#define X52CSRMAINBUSY       8
#define X52CSRMAINGO         0x10
#define X52CSRMAINPURGE      0x20
#define X52CSRMAINECDIS      0x40
#define X52CSRMAINALARM      0x80
#define X52CSRMAINLIRA       ((0xFFF) << 8)
#define X52CSRMERRSSEC       ((0x2) << 24)
#define X52CSRMERRSA         0x1FFF

/* Address pointers for ROM'd P-curve moduli */
#define P192_MOD (&uiROMMods[0])
#define P224_MOD (&uiROMMods[1])
#define P256_MOD (&uiROMMods[2])
#define P384_MOD (&uiROMMods[3])
#define P521_MOD (&uiROMMods[4])

/* X5200 Addressing Flags */
#define X52BYTEREVERSE_FLAG 1
#define X52WORDREVERSE_FLAG 2
#define X52BYTEREVERSE 0x00002000
#define X52WORDREVERSE 0x00004000
#define X52ADDRESSRANGE 0x2000

/* X5200 DMA channel configuration constants. */
#define X52CCR_DEFAULT       0  /* BSIZE=auto, ESWP=none, PROT=user, INC=inc */
#define X52CCR_BSIZEAUTO     0  /* BSIZE=auto */
#define X52CCR_BSIZEBYTE  0x10  /* BSIZE=byte */
#define X52CCR_BSIZEHWORD 0x20  /* BSIZE=half word */
#define X52CCR_BSIZEWORD  0x30  /* BSIZE=word */
#define X52CCR_ESWPNONE      0  /* ESWP=none */
#define X52CCR_ESWPWORD    0x8  /* ESWP=swap bytes in word [0123]->[3210] */
#define X52CCR_PROTUSER      0  /* PROT=user */
#define X52CCR_PROTPRIV    0x2  /* PROT=priv */
#define X52CCR_INCADDR       0  /* INC=inc */
#define X52CCR_NOINCADDR   0x1  /* INC=non-inc */

/* -------- -------- ---------- */
/* External Function Prototypes */
/* -------- -------- ---------- */
#ifndef PKX_C
#ifdef __cplusplus
extern "C" {
#endif

/* Published API Functions */
/* --------- --- --------- */

/* Unpublished Function Prototypes */
/* ----------- -------- ---------- */
extern void CALPKXIni(void);

extern void CALPKMem32Load(volatile SATUINT32_t * puiDst,  
  const SATUINT32_t * puiSrc, SATUINT32_t uiNum );

extern SATR CALPKXTrfRes(SATBOOL bBlock);

extern SATR CALPKXPreCompute(const SATUINT32_t* puiMod, SATUINT32_t* puiMu, 
  SATUINT32_t uiModLen);

extern SATR CALPKXExpo(const SATUINT32_t* puiBase, const SATUINT32_t* puiExpo, 
  const SATUINT32_t* puiMod, const SATUINT32_t* puiMu, SATUINT32_t uiExpLen, 
  SATUINT32_t uiModLen, SATUINT32_t* puiResult);

extern SATR CALPKXExpoCM(const SATUINT32_t* puiBase,
  const SATUINT32_t* puiExpo, const SATUINT32_t* puiMod,
  const SATUINT32_t* puiMu, SATUINT32_t uiExpLen, SATUINT32_t uiModLen,
  SATUINT32_t* puiResult);
  
extern SATR CALPKXDSASign(const SATUINT32_t* puiHash, const SATUINT32_t* puiG,
  const SATUINT32_t* puiK, const SATUINT32_t* puiX, const SATUINT32_t* puiP,
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiQ, const SATUINT32_t* puiQMu,
  SATUINT32_t uiN, SATUINT32_t uiL, SATUINT32_t* puiSigR, SATUINT32_t* puiSigS);
  
extern SATR CALPKXDSASignCM(const SATUINT32_t* puiHash, const SATUINT32_t* puiG,
  const SATUINT32_t* puiK, const SATUINT32_t* puiX, const SATUINT32_t* puiP,
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiQ, const SATUINT32_t* puiQMu,
  SATUINT32_t uiN, SATUINT32_t uiL, SATUINT32_t* puiSigR, SATUINT32_t* puiSigS);
  
extern SATR CALPKXDSASHDMA(const SATUINT32_t* puiExtInput, 
  SATHASHTYPE eHashType, SATUINT32_t uiMsgLen, const SATUINT32_t* puiG, 
  const SATUINT32_t* puiK, const SATUINT32_t* puiX, const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiQ, const SATUINT32_t* puiQMu, 
  SATUINT32_t uiN, SATUINT32_t uiL, SATUINT32_t* puiSigR, SATUINT32_t* puiSigS, 
  SATUINT32_t uiDMAChConfig, const SATBOOL bCM);
  
extern SATR CALPKXDSAVerify(const SATUINT32_t *puiHash, 
  const SATUINT32_t *puiG, const SATUINT32_t *puiY, 
  const SATUINT32_t *puiSigR, const SATUINT32_t *puiSigS, 
  const SATUINT32_t *puiP, const SATUINT32_t *puiPMu, 
  const SATUINT32_t *puiQ, const SATUINT32_t *puiQMu, 
  SATUINT32_t uiN, SATUINT32_t uiL);
  
extern SATR CALPKXDSAVHDMA(const SATUINT32_t *puiExtInput, SATHASHTYPE eHashType, 
  SATUINT32_t uiMsgLen, const SATUINT32_t *puiG, const SATUINT32_t *puiY, 
  const SATUINT32_t *puiSigR, const SATUINT32_t *puiSigS, 
  const SATUINT32_t *puiP, const SATUINT32_t *puiPMu, const SATUINT32_t *puiQ, 
  const SATUINT32_t *puiQMu, SATUINT32_t uiN, SATUINT32_t uiL, 
  SATUINT32_t uiDMAChConfig);
  
extern SATR CALPKXModRed(const SATUINT32_t* puiA, const SATUINT32_t* puiMod,
  const SATUINT32_t* puiMu, SATUINT32_t uiALen, SATUINT32_t uiModLen, 
  SATUINT32_t* puiResult);
  
extern SATR CALPKXMMult(const SATUINT32_t* puiA, const SATUINT32_t* puiB,
  const SATUINT32_t* puiMod, const SATUINT32_t* puiMu, SATUINT32_t uiModLen,
  SATUINT32_t* puiResult);

extern SATR CALPKXMMultAdd(const SATUINT32_t* puiA, const SATUINT32_t* puiB,
  const SATUINT32_t* puiC, const SATUINT32_t* puiMod, const SATUINT32_t* puiMu, 
  SATUINT32_t uiModLen, SATUINT32_t* puiResult);
  
extern SATR CALPKXECMult(const SATUINT32_t* puiMul, const SATUINT32_t* puiPx,
  const SATUINT32_t* puiPy, const SATUINT32_t* puiB, const SATUINT32_t* puiMod,
  const SATUINT32_t* puiMu, SATUINT32_t uiLen, SATUINT32_t uiPtCompress,
  SATUINT32_t* puiRx, SATUINT32_t* puiRy);

extern SATR CALPKXECMultTwist(const SATUINT32_t* puiMul, 
  const SATUINT32_t* puiPx,const SATUINT32_t* puiPy, const SATUINT32_t* puiB, 
  const SATUINT32_t* puiZ, const SATUINT32_t* puiMod, const SATUINT32_t* puiMu, 
  SATUINT32_t uiLen, SATUINT32_t* puiRx, SATUINT32_t* puiRy);

extern SATR CALPKXECMultCM(const SATUINT32_t* puiMul, const SATUINT32_t* puiPx,
  const SATUINT32_t* puiPy, const SATUINT32_t* puiB, const SATUINT32_t* puiMod,
  const SATUINT32_t* puiMu, const SATUINT32_t * puiN, SATUINT32_t uiLen, 
  SATUINT32_t uiPtCompress, SATUINT32_t* puiRx, SATUINT32_t* puiRy);

extern SATR CALPKXECMultTwistCM(const SATUINT32_t* puiMul,
  const SATUINT32_t* puiPx, const SATUINT32_t* puiPy, const SATUINT32_t* puiB,
  const SATUINT32_t* puiZ, const SATUINT32_t* puiMod, const SATUINT32_t* puiMu,
  const SATUINT32_t * puiN, SATUINT32_t uiLen, SATUINT32_t* puiRx, 
  SATUINT32_t* puiRy);

extern SATR CALPKXECMultAdd(const SATUINT32_t* puiMul, const SATUINT32_t* puiPx,
  const SATUINT32_t* puiPy, const SATUINT32_t* puiQx, const SATUINT32_t* puiQy, 
  const SATUINT32_t* puiB, const SATUINT32_t* puiMod, const SATUINT32_t* puiMu,
  SATUINT32_t uiLen, SATUINT32_t uiPtPCompress, SATUINT32_t uiPtQCompress,
  SATUINT32_t* puiRx, SATUINT32_t* puiRy);
  
extern SATR CALPKXECDSASign(const SATUINT32_t* puiHash,
  const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, const SATUINT32_t* puiK,
  const SATUINT32_t* puiD, const SATUINT32_t* puiB, const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, 
  SATUINT32_t uiLen, SATUINT32_t* puiSigR, SATUINT32_t* puiSigS);

extern SATR CALPKXECDSASignTwist(const SATUINT32_t* puiHash, 
  const SATUINT32_t* puiGx,const SATUINT32_t* puiGy, const SATUINT32_t* puiK, 
  const SATUINT32_t* puiD, const SATUINT32_t* puiB, const SATUINT32_t* puiZ,
  const SATUINT32_t* puiP, const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, 
  const SATUINT32_t* puiNMu, SATUINT32_t uiLen, SATUINT32_t* puiSigR, 
  SATUINT32_t* puiSigS);

extern SATR CALPKXECDSASignCM(const SATUINT32_t* puiHash, 
  const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, const SATUINT32_t* puiK, 
  const SATUINT32_t* puiD, const SATUINT32_t* puiB, const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiN,
  const SATUINT32_t* puiNMu, SATUINT32_t uiLen, SATUINT32_t* puiSigR,
  SATUINT32_t* puiSigS);

extern SATR CALPKXECDSASignTwistCM(const SATUINT32_t* puiHash, 
  const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, const SATUINT32_t* puiK, 
  const SATUINT32_t* puiD, const SATUINT32_t* puiB, const SATUINT32_t* puiZ, 
  const SATUINT32_t* puiP, const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, 
  const SATUINT32_t* puiNMu, SATUINT32_t uiLen, SATUINT32_t* puiSigR, 
  SATUINT32_t* puiSigS);

extern SATR CALPKXECDSASH(const SATUINT32_t* puiHash, const SATHASHTYPE eHashType,
  const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, const SATUINT32_t* puiK, 
  const SATUINT32_t* puiD, const SATUINT32_t* puiB, const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, 
  const SATUINT32_t uiLen, SATUINT32_t* puiSigR, SATUINT32_t* puiSigS);

extern SATR CALPKXECDSASHCM(const SATUINT32_t* puiHash, const SATHASHTYPE eHashType,
  const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, const SATUINT32_t* puiK, 
  const SATUINT32_t* puiD, const SATUINT32_t* puiB, const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, 
  const SATUINT32_t uiLen, SATUINT32_t* puiSigR, SATUINT32_t* puiSigS);

extern SATR CALPKXECDSASHDMA(const SATUINT32_t* puiExtInput, 
  SATHASHTYPE eHashType, SATUINT32_t uiMsgLen, const SATUINT32_t* puiGx,
  const SATUINT32_t* puiGy, const SATUINT32_t* puiK, const SATUINT32_t* puiD,
  const SATUINT32_t* puiB, const SATUINT32_t* puiP, const SATUINT32_t* puiPMu,
  const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, SATUINT32_t uiLen,
  SATUINT32_t* puiSigR, SATUINT32_t* puiSigS, SATUINT32_t uiDMAChConfig,
  const SATBOOL bCM);

extern SATR CALPKXECDSASTwistH(const SATUINT32_t* puiHash, 
  const SATHASHTYPE eHashType, const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, 
  const SATUINT32_t* puiK, const SATUINT32_t* puiD, const SATUINT32_t* puiB, 
  const SATUINT32_t* puiZ, const SATUINT32_t* puiP, const SATUINT32_t* puiPMu, 
  const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, const SATUINT32_t uiLen, 
  SATUINT32_t* puiSigR, SATUINT32_t* puiSigS);

extern SATR CALPKXECDSASTwistHDMA(const SATUINT32_t* puiExtInput, 
  SATHASHTYPE eHashType, SATUINT32_t uiMsgLen, const SATUINT32_t* puiGx,
  const SATUINT32_t* puiGy, const SATUINT32_t* puiK, const SATUINT32_t* puiD,
  const SATUINT32_t* puiB, const SATUINT32_t* puiZ, const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, const SATUINT32_t* puiNMu,
  SATUINT32_t uiLen, SATUINT32_t* puiSigR, SATUINT32_t* puiSigS, 
  SATUINT32_t uiDMAChConfig);

extern SATR CALPKXECDSAVerify(const SATUINT32_t* puiHash,
  const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, const SATUINT32_t* puiQx,
  const SATUINT32_t* puiQy, const SATUINT32_t* puiSigR,
  const SATUINT32_t* puiSigS, const SATUINT32_t* puiB, const SATUINT32_t* puiP,
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, const SATUINT32_t* puiNMu,
  SATUINT32_t uiLen, SATUINT32_t uiPtCompress);

extern SATR CALPKXECDSAVerifyTwist(const SATUINT32_t* puiHash, 
  const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, const SATUINT32_t* puiQx, 
  const SATUINT32_t* puiQy, const SATUINT32_t* puiSigR, 
  const SATUINT32_t* puiSigS, const SATUINT32_t* puiB, const SATUINT32_t* puiZ,
  const SATUINT32_t* puiP, const SATUINT32_t* puiPMu, const SATUINT32_t* puiN,
  const SATUINT32_t* puiNMu, SATUINT32_t uiLen, SATUINT32_t uiPtCompress);

extern SATR CALPKXECDSAVH(const SATUINT32_t* puiHash, const SATHASHTYPE eHashType, 
  const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, const SATUINT32_t* puiQx, 
  const SATUINT32_t* puiQy, const SATUINT32_t* puiSigR, 
  const SATUINT32_t* puiSigS, const SATUINT32_t* puiB,const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, 
  const SATUINT32_t uiLen, const SATUINT32_t uiPtCompress);

extern SATR CALPKXECDSAVHDMA(const SATUINT32_t* puiExtInput, 
  SATHASHTYPE eHashType, SATUINT32_t uiMsgLen, const SATUINT32_t* puiGx, 
  const SATUINT32_t* puiGy, const SATUINT32_t* puiQx, const SATUINT32_t* puiQy,
  const SATUINT32_t* puiSigR, const SATUINT32_t* puiSigS, 
  const SATUINT32_t* puiB,const SATUINT32_t* puiP, const SATUINT32_t* puiPMu,
  const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, SATUINT32_t uiLen,
  SATUINT32_t uiPtCompress, SATUINT32_t uiDMAChConfig);  

extern SATR CALPKXECDSAVTwistH(const SATUINT32_t* puiHash, const SATHASHTYPE eHashType, 
  const SATUINT32_t* puiGx, const SATUINT32_t* puiGy, const SATUINT32_t* puiQx, 
  const SATUINT32_t* puiQy, const SATUINT32_t* puiSigR, 
  const SATUINT32_t* puiSigS, const SATUINT32_t* puiB, const SATUINT32_t* puiZ,
  const SATUINT32_t* puiP, const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, 
  const SATUINT32_t* puiNMu, const SATUINT32_t uiLen, SATUINT32_t uiPtCompress);

extern SATR CALPKXECDSAVTwistHDMA(const SATUINT32_t* puiExtInput, 
  SATHASHTYPE eHashType, SATUINT32_t uiMsgLen, const SATUINT32_t* puiGx, 
  const SATUINT32_t* puiGy, const SATUINT32_t* puiQx, const SATUINT32_t* puiQy,
  const SATUINT32_t* puiSigR, const SATUINT32_t* puiSigS, 
  const SATUINT32_t* puiB, const SATUINT32_t* puiZ, const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiN, 
  const SATUINT32_t* puiNMu, SATUINT32_t uiLen, SATUINT32_t uiPtCompress, 
  SATUINT32_t uiDMAChConfig);

extern SATR CALPKXECDHC(const SATUINT32_t* puiS, const SATUINT32_t* puiWx, 
  const SATUINT32_t* puiWy, const SATUINT32_t* puiB, const SATUINT32_t* puiMod,
  const SATUINT32_t* puiMu, const SATUINT32_t* puiK, const SATUINT32_t* puiR,
  const SATUINT32_t* puiRMu, SATUINT32_t uiLen, SATUINT32_t uiPtCompress,
  SATUINT32_t* puiZ);
  
extern SATR CALPKXRSACRT(const SATUINT32_t* puiCipher, 
  const SATUINT32_t* puiQInv, const SATUINT32_t* puiDP,
  const SATUINT32_t* puiDQ, const SATUINT32_t* puiP, const SATUINT32_t* puiPMu,
  const SATUINT32_t* puiQ, const SATUINT32_t* puiQMu, SATUINT32_t uiLen,
  SATUINT32_t* puiPlain);

extern SATR CALPKXRSACRTCM(const SATUINT32_t* puiCipher,
  const SATUINT32_t* puiQInv, const SATUINT32_t* puiDP,
  const SATUINT32_t* puiDQ, const SATUINT32_t* puiE,
  const SATUINT32_t* puiP, const SATUINT32_t* puiQ, const SATUINT32_t* puiN, 
  const SATUINT32_t* puiNMu, const SATUINT32_t uiLen, const SATUINT32_t uiELen, 
  SATUINT32_t* puiPlain);

extern SATR CALPKXRSACRTSign(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, 
  const SATUINT32_t* puiCipher, const SATUINT32_t* puiQInv, 
  const SATUINT32_t* puiDP, const SATUINT32_t* puiDQ, const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiQ, 
  const SATUINT32_t* puiQMu, const SATUINT32_t* puiN, SATUINT32_t uiLen,
  SATUINT32_t* puiS);
  
extern SATR CALPKXRSACRTSignCM(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, 
  const SATUINT32_t* puiHash, const SATUINT32_t* puiE, const SATUINT32_t uiELen,
  const SATUINT32_t* puiQInv, const SATUINT32_t* puiDP, const SATUINT32_t* puiDQ, 
  const SATUINT32_t* puiP, const SATUINT32_t* puiQ, const SATUINT32_t* puiN, 
  const SATUINT32_t* puiNMu, SATUINT32_t uiLen, SATUINT32_t* puiSig);

extern SATR CALPKXRSACRTSHDMA(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, 
  const SATUINT32_t* puiExtInput, const SATUINT32_t* puiQInv, 
  const SATUINT32_t* puiDP, const SATUINT32_t* puiDQ, const SATUINT32_t* puiP, 
  const SATUINT32_t* puiPMu, const SATUINT32_t* puiQ, const SATUINT32_t* puiQMu, 
  const SATUINT32_t* puiN, SATUINT32_t uiMsgLen, SATUINT32_t uiModLen, 
  SATUINT32_t* puiSig, SATUINT32_t uiDMAChConfig);

extern SATR CALPKXRSACRTSHDMACM(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, 
  const SATUINT32_t* puiExtInput, const SATUINT32_t* puiE, const SATUINT32_t uiELen,
  const SATUINT32_t* puiQInv, const SATUINT32_t* puiDP, const SATUINT32_t* puiDQ, 
  const SATUINT32_t* puiP, const SATUINT32_t* puiQ, const SATUINT32_t* puiN, 
  const SATUINT32_t* puiNMu, SATUINT32_t uiMsgLen, SATUINT32_t uiLen, 
  SATUINT32_t* puiSig, SATUINT32_t uiDMAChConfig);
  
extern SATR CALPKXRSASign(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType,
  const SATUINT32_t* puiHash, const SATUINT32_t* puiD, const SATUINT32_t* puiN, 
  const SATUINT32_t* puiNMu, SATUINT32_t uiModLen, SATUINT32_t* puiSig);
  
extern SATR CALPKXRSASHDMA(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType,
  const SATUINT32_t* puiExtInput, const SATUINT32_t* puiD, 
  const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, SATUINT32_t uiMsgLen, 
  SATUINT32_t uiModLen, SATUINT32_t* puiSig, SATUINT32_t uiDMAChConfig);  
  
extern SATR CALPKXRSAVerify(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, 
  const SATUINT32_t* puiHash, const SATUINT32_t* puiE, SATUINT32_t uiExpLen, 
  const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, SATUINT32_t uiModLen, 
  const SATUINT32_t* puiS);
  
extern SATR CALPKXRSAVHDMA(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, 
  const SATUINT32_t* puiExtInput, const SATUINT32_t* puiE,SATUINT32_t uiExpLen,
  const SATUINT32_t* puiN, const SATUINT32_t* puiNMu, SATUINT32_t uiMsgLen, 
  SATUINT32_t uiModLen, const SATUINT32_t* puiS, SATUINT32_t uiDMAChConfig);
  
extern SATR CALPKXECPtValidate(const SATUINT32_t* puiPx,
  const SATUINT32_t* puiPy, const SATUINT32_t* puiB, const SATUINT32_t* puiMod,
  const SATUINT32_t* puiMu, SATUINT32_t uiLen); 

extern SATR CALPKXECKeyPairGen(const SATUINT32_t* puiC, const SATUINT32_t* puiPx,
  const SATUINT32_t* puiPy, const SATUINT32_t* puiMod, const SATUINT32_t* puiMu, 
  const SATUINT32_t* puiNM1, const SATUINT32_t* puiNM1Mu, const SATUINT32_t* puiB, 
  SATUINT32_t* puiD, SATUINT32_t* puiQx, SATUINT32_t* puiQy, SATUINT32_t uiLen);

extern SATR CALPKXPurge52(SATBOOL bVerify);

/* -------- ------ --------- */
/* External Global Variables */
/* -------- ------ --------- */
extern SATRESULT SATResults;

/* -------- ------ --------- */
/* External Global Constants */
/* -------- ------ --------- */
extern const SATUINT32_t uiROMMods[];

#ifdef __cplusplus
}
#endif
#endif
#endif
