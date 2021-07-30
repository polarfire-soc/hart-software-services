#ifndef ATHENA_CAL
#define ATHENA_CAL

/*******************************************************************************
 * Copyright 2019-2021 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 *
 * Hart Software Services - Athena CAL
 *
 */


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    SATASYMKEYTYPE_NULL,
    SATASYMKEYTYPE_RSA_PUBLIC,
    SATASYMKEYTYPE_RSA_PRIVATE,
    SATASYMKEYTYPE_EC_PUBLIC,
    SATASYMKEYTYPE_EC_PRIVATE
} SATASYMKEYTYPE;

typedef bool SATBOOL;

typedef enum {
    SATRSAENCTYPE_NULL,
    SATRSAENCTYPE_PKCS,
    SATRSAENCTYPE_ANSI,
    SATRSAENCTYPE_PSS
} SATRSAENCTYPE;

typedef enum {
    SATHASHTYPE_SHA1,
    SATHASHTYPE_SHA224,
    SATHASHTYPE_SHA256,
    SATHASHTYPE_SHA384,
    SATHASHTYPE_SHA512,
    SATHASHTYPE_SHA512_224,
    SATHASHTYPE_SHA512_256,
} SATHASHTYPE;

typedef enum {
    SATMACTYPE_NULL,
    SATMACTYPE_SHA1,
    SATMACTYPE_SHA224,
    SATMACTYPE_SHA256,
    SATMACTYPE_SHA384,
    SATMACTYPE_SHA512,
    SATMACTYPE_SHA512_224,
    SATMACTYPE_SHA512_256,
    SATMACTYPE_AESCMAC128,
    SATMACTYPE_AESCMAC192,
    SATMACTYPE_AESCMAC256,
    SATMACTYPE_AESGMAC
} SATMACTYPE;

typedef enum {
    SATR_BADCONTEXT,
    SATR_BADHANDLE,
    SATR_BADHASHLEN,
    SATR_BADHASHTYPE,
    SATR_BADLEN,
    SATR_BADMACLEN,
    SATR_BADMACTYPE,
    SATR_BADMODE,
    SATR_BADPARAM,
    SATR_BADTYPE,
    SATR_BUSY,
    SATR_DCMPARMB,
    SATR_DCMPARMP,
    SATR_DCMPARMX,
    SATR_FAIL,
    SATR_FNP,
    SATR_KEYSFULL,
    SATR_LSB0PAD,
    SATR_MSBICV1,
    SATR_MSBICV2,
    SATR_PADLEN,
    SATR_PAF,
    SATR_PARITYFLUSH,
    SATR_ROFATAL,
    SATR_SIGNFAIL,
    SATR_SATRESCONTEXT,
    SATR_SATRESHANDLE,
    SATR_SATSYMMODE,
    SATR_SIGNPARMD,
    SATR_SIGNPARMK,
    SATR_SUCCESS,
    SATR_VALIDATEFAIL,
    SATR_VALPARMB,
    SATR_VALPARMX,
    SATR_VALPARMY,
    SATR_VERIFYFAIL,
    SATR_VERPARMR,
    SATR_VERPARMS,
} SATR;

typedef void* SATRESCONTEXTPTR;

typedef enum {
    SATRES_DEFAULT,
    SATRES_CALSW
} SATRESHANDLE;

typedef enum {
    SATSYMMODE_ECB,
    SATSYMMODE_CBC,
    SATSYMMODE_CFB,
    SATSYMMODE_OFB,
    SATSYMMODE_CTR,
    SATSYMMODE_GCM,
    SATSYMMODE_GHASH,
} SATSYMMODE;

typedef enum {
    SATSYMTYPE_AES128,
    SATSYMTYPE_AES192,
    SATSYMTYPE_AES256,
    SATSYMTYPE_AESKS128,
    SATSYMTYPE_AESKS192,
    SATSYMTYPE_AESKS256,
} SATSYMTYPE;

typedef enum {
    SATSYMKEYSIZE_AES128,
    SATSYMKEYSIZE_AES256,
} SATSYMKEYSIZE;

typedef uint8_t SATUINT8_t;
typedef uint16_t SATUINT16_t;
typedef uint32_t SATUINT32_t;
typedef uint64_t SATUINT64_t;

typedef void * DRBGCTXPTR;


// General Functions
SATR CALIni(void);
SATR CALPurge52(SATBOOL bVerify);
SATR CALPKTrfRes(SATBOOL bBlock);
SATR CALPreCompute(const uint32_t *puiMod, uint32_t *puiMu, uint32_t uiModLen);

// Conventional Public Key Crypto Functions
SATR CALExpo(const uint32_t *puiBase, const uint32_t *puiExpo, const uint32_t *puiMod, const uint32_t *puiMu, uint32_t uiExpLen, uint32_t uiModLen, uint32_t *puiResult);
SATR CALModRed(const uint32_t *puiA, const uint32_t *puiMod, const uint32_t *puiMu, uint32_t uiALen, uint32_t uiModLen, uint32_t *puiResult);
SATR CALMMult(const uint32_t *puiA, const uint32_t *puiB, const uint32_t *puiMod, const uint32_t *puiMu, uint32_t uiModLen, uint32_t *puiResult);
SATR CALMMultAdd(const uint32_t *puiA, const uint32_t *puiB, const uint32_t *puiC, const uint32_t *puiMod, const uint32_t *puiMu, uint32_t uiModLen, uint32_t *puiResult);
SATR CALRSACRT(const uint32_t *puiCipher, const uint32_t *puiQInv, const uint32_t *puiDP, const uint32_t *puiDQ, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiQ, const uint32_t *puiQMu, uint32_t uiLen, uint32_t *puiPlain);
SATR CALRSACRTSign(SATRSAENCTYPE eRSAEncod, SATHASHTYPE eHashType, const uint32_t *puiHash, const uint32_t *puiQInv, const uint32_t *puiDP, const uint32_t *puiDQ, const uint32_t *puiP,
    const uint32_t *puiPMu, const uint32_t *puiQ, const uint32_t *puiQMu, uint32_t uiLen, uint32_t *puiPlain);
SATR CALRSACRTSignHash(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, const uint32_t *puiMsg, const uint32_t *puiQInv, const uint32_t *puiDP, const uint32_t *puiDQ, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiQ, const uint32_t *puiQMu, uint32_t uiModLen, uint32_t uiMsgLen, uint32_t *puiSig, SATBOOL bDMA, uint32_t uiDMAChConfig);
SATR CALRSASign(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, const uint32_t *puiHash, const uint32_t *puiD, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiModLen, uint32_t *puiSig);
SATR  CALRSASignVerify(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, const uint32_t *puiHash, const uint32_t *puiE, const uint32_t uiExpLen, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiModLen, uint32_t *puiSig);

SATR CALDSASignHash(const uint32_t *puiMsg, SATHASHTYPE eHashType, uint32_t uiMsgLen, const uint32_t *puiG, const uint32_t *puiK, const uint32_t *puiX, const uint32_t *puiP, const uint32_t *puiQ, const uint32_t *puiQMu, uint32_t uiN, uint32_t uiL, uint32_t *puiSigR, uint32_t *puiSigS, SATBOOL bDMA, uint32_t uiDMAChConfig);
SATR CALDSAVerifyHash(const uint32_t *puiMsg, SATHASHTYPE eHashType, uint32_t uiMsgLen, const uint32_t *puiG, const uint32_t *puiY, const uint32_t *puiSigR, const uint32_t *puiSigS, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiQ, const uint32_t *puiQMu, uint32_t uiN, uint32_t uiL, SATBOOL bDMA, uint32_t uiDMAChConfig);

// Elliptic Curve Crypto Functions
SATR CALECMult(const uint32_t *puiMul, const uint32_t *puiPx, const uint32_t *puiPy, const uint32_t *puiB, const uint32_t *puiMod, const uint32_t *puiMu, uint32_t uiLen, uint32_t uiPtCompress, uint32_t *puiRx, uint32_t *puiRy);
SATR CALECMultTwist(const uint32_t *puiMul, const uint32_t *puiPx, const uint32_t *puiPy, const uint32_t *puiB, const uint32_t *puiZ, const uint32_t *puiMod, const uint32_t *puiMu, const uint32_t *puiN, uint32_t uiLen, uint32_t *puiRx, uint32_t *puiRy);
SATR CALECMultAdd(const uint32_t *puiMul, const uint32_t *puiPx, const uint32_t *puiPy, const uint32_t *puiQx, const uint32_t *puiQy, const uint32_t *puiB, const uint32_t *puiMod, const uint32_t *puiMu, uint32_t uiLen, uint32_t uiPtPCompress, uint32_t uiPtQCompress, uint32_t *puiRx, uint32_t *puiRy);
SATR CALECPtValidate(const uint32_t *puiPx, const uint32_t *puiPy, const uint32_t *puiB, const uint32_t *puiMod, const uint32_t *puiMu, uint32_t uiLen);
SATR CALECKeyPairGen(const uint32_t *puiC, const uint32_t *puiPx, const uint32_t *puiPy, const uint32_t *puiMod, const uint32_t *puiMu, const uint32_t *puiNM1, const uint32_t *puiNM1Mu, const uint32_t *puiB, uint32_t *puiD, uint32_t *puiQx, uint32_t *puiQy, uint32_t uiLen);
SATR CALECDHC(const uint32_t *puiS, const uint32_t *puiWx, const uint32_t *puiWy, const uint32_t *puiB, const uint32_t *puiMod, const uint32_t *puiMu, const uint32_t *puiK, const uint32_t *puiR, const uint32_t *puiRMu, uint32_t uiLen, uint32_t uiPtCompress, uint32_t* puiZ);
SATR CALECDSASign(const uint32_t *puiHash, const uint32_t *puiGx, const uint32_t *puiGy, const uint32_t *puiK, const uint32_t *puiD, const uint32_t *puiB, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiLen, uint32_t *puiSigR, uint32_t *puiSigS);
SATR CALECDSAVerify(const uint32_t *puiHash, const uint32_t *puiGx, const uint32_t *puiGy, const uint32_t *puiQx, const uint32_t *puiQy, const uint32_t *puiSigR, const uint32_t *puiSigS, const uint32_t *puiB, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiLen, uint32_t uiPtCompress);
SATR CALECDSASignHash(const uint32_t *puiMsg, SATHASHTYPE eHashType, uint32_t uiMsgLen, const uint32_t *puiGx, const uint32_t *puiGy, const uint32_t *puiK, const uint32_t *puiD, const uint32_t *puiB, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiLen, const uint32_t *puiSigR, const uint32_t *puiSigS, SATBOOL bDMA, uint32_t uiDMAChConfig);
SATR CALECDSAVerifyHash(const uint32_t *puiMsg, SATHASHTYPE eHashType, uint32_t uiMsgLen, const uint32_t *puiGx, const uint32_t *puiGy, const uint32_t *puiQx, const uint32_t *puiQy, const uint32_t *puiSigR, const uint32_t *puiSigS, const uint32_t *puiB, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiLen, uint32_t uiPtCompress, SATBOOL bDMA, uint32_t uiDMAChConfig);
SATR CALECDSASignTwist(const uint32_t *puiHash, const uint32_t *puiGx, const uint32_t *puiGy, const uint32_t *puiK, const uint32_t *puiD, const uint32_t *puiB, const uint32_t *puiZ, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiLen, uint32_t *puiSigR, uint32_t *puiSigS);
SATR CALECDSAVerifyTwist(const uint32_t *puiHash, const uint32_t *puiGx, const uint32_t *puiGy, const uint32_t *puiQx, const uint32_t *puiQy, const uint32_t *puiSigR, const uint32_t *puiSigS, const uint32_t *puiB, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiLen, uint32_t uiPtCompress);
SATR CALECDSASignTwistHash(const uint32_t *puiMsg, SATHASHTYPE eHashType, uint32_t uiMsgLen, const uint32_t *puiGx, const uint32_t *puiGy, const uint32_t *puiK, const uint32_t *puiD, const uint32_t *puiB, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiLen, uint32_t *puiSigR, uint32_t *puiSigS, SATBOOL bDMA, uint32_t uiDMAChConfig);
SATR CALECDSAVerifyTwistHash(const uint32_t *puiMsg, SATHASHTYPE eHashType, uint32_t uiMsgLen, const uint32_t *puiGx, const uint32_t *puiGy, const uint32_t *puiQx, const uint32_t *puiQy, const uint32_t *puiSigR, const uint32_t *puiSigS, const uint32_t *puiB, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiLen, uint32_t uiPtCompress, SATBOOL bDMA, uint32_t uiDMAChConfig);

// Functions with SCA counter measures
SATR CALExpoCM(const uint32_t *puiBase, const uint32_t *puiExpo, const uint32_t *puiMod, const uint32_t *puiMu, uint32_t uiExpLen, uint32_t uiModLen, uint32_t *puiResult);
SATR CALRSACRTCM(const uint32_t *puiCipher, const uint32_t *puiQInv, const uint32_t *puiDP, const uint32_t *puiDQ, const uint32_t *puiE, const uint32_t *puiP, const uint32_t *puiQ, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiLen, uint32_t uiELen, uint32_t *puiPlain);
SATR CALRSACRTSignCM(SATRSAENCTYPE eRSAEncod, SATHASHTYPE eHashType, const uint32_t *puiHash, const uint32_t *puiE, uint32_t uiELen, const uint32_t *puiQInv, const uint32_t *puiDP, const uint32_t *puiDQ, const uint32_t *puiP, const uint32_t *puiQ, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiLen, uint32_t *puiSig);
SATR CALRSACRTSignHashCM(SATRSAENCTYPE eRsaEncod, SATHASHTYPE eHashType, const uint32_t *puiMsg, const uint32_t *puiE, uint32_t uiELen, const uint32_t *puiQInv, const uint32_t *puiDP, const uint32_t *puiDQ, const uint32_t *puiP, const uint32_t *puiQ, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiMsgLen, uint32_t uiModLen, const uint32_t *puiSig, SATBOOL bDMA, uint32_t uiDMAChConfig);
SATR CALDSASignCM(const uint32_t *puiHash, const uint32_t *puiG, const uint32_t *puiK, const uint32_t *puiX, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiQ, const uint32_t *puiQMu, uint32_t uiN, uint32_t uiL, const uint32_t *puiSigR, const uint32_t *puiSigS);
SATR CALDSASignHashCM(const uint32_t *puiMsg, SATHASHTYPE eHashType, uint32_t uiMsgLen, const uint32_t *puiG, const uint32_t *puiK, const uint32_t *puiX, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiQ, const uint32_t *puiQMu, uint32_t uiN, uint32_t uiL, uint32_t *puiSigR, uint32_t *puiSigS, SATBOOL bDMA, uint32_t uiDMAChConfig);
SATR CALECMultCM(const uint32_t *puiMul, const uint32_t *puiPx, const uint32_t *puiPy, const uint32_t *puiB, const uint32_t *puiMod, const uint32_t *puiMu, const uint32_t *puiN, uint32_t uiLen, uint32_t uiPtCompress, uint32_t *puiRx, uint32_t *puiRy);
SATR CALECMultTwistCM(const uint32_t *puiMul, const uint32_t *puiPx, const uint32_t *puiPy, const uint32_t *puiB, const uint32_t *puiZ, const uint32_t *puiMod, const uint32_t *puiMu, const uint32_t *puiN, uint32_t uiLen, uint32_t *puiRx, uint32_t *puiRy);
SATR CALECDSASignCM(const uint32_t *puiHash, const uint32_t *puiGx, const uint32_t *puiGy, const uint32_t *puiK, const uint32_t *puiD, const uint32_t *puiB, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiLen, uint32_t *puiSigR, uint32_t *puiSigS);
SATR CALECDSASignHashCM(const uint32_t *puiMsg, SATHASHTYPE eHashType, uint32_t uiMsgLen, const uint32_t *puiGx, const uint32_t *puiGy, const uint32_t *puiK, const uint32_t *puiD, const uint32_t *puiB, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiLen, uint32_t *puiSigR, uint32_t *puiSigS, SATBOOL bDMA, uint32_t uiDMAChConfig);
SATR CALECDSASignTwistCM(const uint32_t *puiHash, const uint32_t *puiGx, const uint32_t *puiGy, const uint32_t *puiK, const uint32_t *puiD, const uint32_t *puiB, const uint32_t *puiZ, const uint32_t *puiP, const uint32_t *puiPMu, const uint32_t *puiN, const uint32_t *puiNMu, uint32_t uiLen, uint32_t *puiSigR, uint32_t *puiSigS);

// CAL Synmmetric Crypto Functions
SATR CALSymTrfRes(SATBOOL bBlock);
SATR CALSymEncrypt(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc, void *pDest, SATUINT32_t uiLen);
SATR CALSymDecrypt(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc, void *pDest, SATUINT32_t uiLen);
SATR CALSymEncryptDMA(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc, void *pDest, SATUINT32_t uiLen, SATUINT32_t uiDMAChConfig);
SATR CALSymDecryptDMA(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc, void *pDest, SATUINT32_t uiLen, SATUINT32_t uiDMAChConfig);

// Combined Encryption-Authentication Functions
SATR CALSymEncAuth(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, SATSYMMODE eMode, void *pIV, const void *pSrc, void *pDest, SATUINT32_t uiEncLen, void *pAuth, SATUINT32_t uiAuthLen, void *pMAC, SATUINT32_t uiMACLen);
SATR CALSymDecVerify(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, SATSYMMODE eMode, void *pIV, const void *pSrc, void *pDest, SATUINT32_t uiEncLen, void *pAuth, SATUINT32_t uiAuthLen, void *pMAC, SATUINT32_t uiMACLen);
SATR CALSymEncAuthDMA(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, SATSYMMODE eMode, void *pIV, const void *pSrc, void *pDest, SATUINT32_t uiEncLen, void *pAuth, SATUINT32_t uiAuthLen, void *pMAC, SATUINT32_t uiDMAChConfig);
SATR CALSymDecVerifyDMA(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, SATSYMMODE eMode, void *pIV, const void *pSrc, void *pDest, SATUINT32_t uiEncLen, void *pAuth, SATUINT32_t uiAuthLen, void *pMAC, SATUINT32_t uiDMAChConfig);
SATR CALSymEncryptKR(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc, void *pDest, SATUINT32_t uiLen, SATUINT32_t uiKRF) ;
SATR CALSymDecryptKR(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc, void *pDest, SATUINT32_t uiLen, SATUINT32_t uiKRF);
SATR CALSymEncryptKRDMA(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc, void *pDest, SATUINT32_t uiLen, SATUINT32_t uiKRF, SATUINT32_t uiDMAChConfig);
SATR CALSymDecryptKRDMA(SATSYMTYPE eSymType, const SATUINT32_t *puiKey, SATSYMMODE eMode, void *pIV, SATBOOL bLoadIV, const void *pSrc, void *pDest, SATUINT32_t uiLen, SATUINT32_t uiKRF, SATUINT32_t uiDMAChConfig);

// Hashes
SATR CALHash(SATHASHTYPE eHashType, const void *pMsg, SATUINT32_t uiMsgLen, void *pHash);
SATR CALHashDMA(SATHASHTYPE eHashType, const void *pMsg, SATUINT32_t uiMsgLen, void *pHash, SATUINT32_t uiDMAChConfig);

// Multiple Call Hash Functions
SATR CALHashIni(SATHASHTYPE eHashType, SATUINT32_t uiMsgLen);
SATR CALHashWrite(const void *pBuffer, SATUINT32_t uiBufLen);
SATR CALHashRead(void *pHash);

// Hash Functions with Context Switching
SATR CALHashCtx(const SATRESHANDLE hResource, SATRESCONTEXTPTR const pContext, const void *pMsg, SATUINT32_t uiMsgLen, void *pHash, const SATBOOL bFinal);
SATR CALHashCtxIni(SATRESCONTEXTPTR const pContext, const SATHASHTYPE eHashType);
SATR CALMAC(SATMACTYPE eMACType, const SATUINT32_t *puiKey, SATUINT32_t uiKeyLen, const void *pMsg, SATUINT32_t uiMsgLen, void *pMAC);
SATR CALMACDMA(SATMACTYPE eMACType, const SATUINT32_t *puiKey, SATUINT32_t uiKeyLen, const void *pMsg, SATUINT32_t uiMsgLen, void *pMAC, SATUINT32_t uiDMAChConfig);

// Multiple Call MAC Functions

SATR CALMACIni(SATMACTYPE eMACType, const SATUINT32_t *puiKey, SATUINT32_t uiKeyLen, SATUINT32_t uiMsgLen);
SATR CALMACWrite(const void *pBuffer, SATUINT32_t uiBufLen);
SATR CALMACRead(const SATUINT32_t *puiKey, SATUINT32_t uiKeyLen, void *pMAC);

// MAC Functions with Context Switching
SATR CALMACCtx(const SATRESHANDLE hResource, SATRESCONTEXTPTR const pContext, const void *pMsg, SATUINT32_t uiMsgLen, void *pHash, const SATBOOL bFinal);
SATR CALMACCtxIni(SATRESCONTEXTPTR const pContext, SATHASHTYPE eHashType, const SATUINT32_t *puiKey, SATUINT32_t uiKey);

// Random Number Generation
SATR CALNRBGSetTestEntropy(SATUINT32_t *puiEntropy, SATUINT32_t uiEntLen32);
SATR CALNRBGAddTestEntropy(SATUINT32_t *puiEntropy, SATUINT32_t uiEntLen32);
SATR CALNRBGGetEntropy(SATUINT32_t *puiEntropy, SATUINT32_t uiEntLen32, SATBOOL bTesting);
SATR CALNRBGConfig(SATUINT32_t uiWriteEn, SATUINT32_t uiCSR, SATUINT32_t uiCntLim, SATUINT32_t uiVoTimer, SATUINT32_t uiFMsk, SATUINT32_t *puiStatus);
SATUINT32_t CALNRBGHealthStatus(void);

// Deterministic Random Bit Generation (SP800-90A)
SATR CALDRBGInstantiate(const SATUINT32_t *puiNonce, SATUINT32_t uiNonceLen, const SATUINT32_t *puiPzStr, SATUINT32_t uiPzStrLen, SATSYMKEYSIZE eStrength, SATUINT32_t uiEntropyFactor, SATUINT32_t uiReseedLim, SATBOOL bTesting);
SATR CALDRBGReseed(const SATUINT32_t *puiAddIn, SATUINT32_t uiAddInLen);
SATR CALDRBGGenerate(const SATUINT32_t *puiAddIn, SATUINT32_t uiAddInLen, SATBOOL bPredResist, SATUINT32_t *puiOut, SATUINT32_t uiOutBlocks);
SATR CALDRBGUninstantiate(void);
SATR CALDRBGGetCtx(DRBGCTXPTR drbgCtxExt);
SATR CALDRBGLoadCtx(DRBGCTXPTR drbgCtxExt);

// Key Wrap and Unwrap
SATR CALSymKw(SATSYMTYPE eSymType, const SATUINT32_t *puiKEK, const SATUINT32_t *puiInKey, SATUINT32_t *puiOutKey, SATUINT32_t uiLen, SATBOOL bWrap);
SATR CALSymKwp(SATSYMTYPE eSymType, const SATUINT32_t *puiKEK, const SATUINT32_t *puiInKey, SATUINT32_t *puiOutKey, SATUINT32_t uiLen, SATBOOL bWrap);

// Context Management
SATRESCONTEXTPTR  CALContextCurrent(const SATRESHANDLE hResource);
SATR CALContextLoad(const SATRESHANDLE hResource, SATRESCONTEXTPTR const pContext);
SATR CALContextRemove(const SATRESHANDLE hResource);
SATR CALContextUnload(const SATRESHANDLE hResource);

// Key Derivation Functions
SATR CALKeyTree(SATBOOL bPathSizeSel, const SATUINT32_t *puiKey, const uint8_t uiOpType, const SATUINT32_t *puiPath, const SATUINT32_t *puiKeyOut);

#ifdef __cplusplus
}
#endif


#endif
