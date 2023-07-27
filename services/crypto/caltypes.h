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
     This file defines common types in the CAL. 
     
     Since support for the C99 stdint.h integer types is not universal, 
   these are defined herein and may require customization from compiler 
   to compiler, or use of the stdint.h header, if present (recommended). 

     C99 supports 64-bit types; however, support in older compilers is 
   spotty. For those that do not support it, the macro NO64BITINT may be 
   defined by the user to prevent defintion of 64-bit types. This is 
   generally safe with CAL-PK and CAL-SYM; however, this is incompatible
   with CAL-SW. 

     Most CAL types and constants are defined with the "SAT" prefix to 
   reduce the chance of name space collisions with other users. 
   ------------------------------------------------------------------- */

#ifndef CALTYPES_H
#define CALTYPES_H

/* -------- */
/* Includes */
/* -------- */
#include "calpolicy.h"


/* ------- */
/* Defines */
/* ------- */

/* ---- ----------- */
/* Type Definitions */
/* ---- ----------- */

/* Base Types */
/* ---- ----- */

/* The user may compile using stdint.h instead of the definitions below
   by defining the macro INC_STDINT_H. */
#ifdef INC_STDINT_H
#include <stdint.h>
#endif

/* Integer type abstraction layer. */
#ifndef INC_STDINT_H

typedef unsigned char         uint8_t;
typedef signed char           int8_t;

typedef unsigned short        uint16_t;
typedef signed short          int16_t;

typedef unsigned int          uint32_t;
typedef int                   int32_t;

/* stdint.h is a C99 feature, and C99 supports 64-bit ints, so this is 
   immune to the macro used to disable 64-bit int support. 
*/
typedef unsigned long long    uint64_t;
typedef long long             int64_t;

typedef long                  uintptr_t;

#endif

/* Boolean type. */
typedef uint8_t               SATBOOL;

/* Unsigned integer type */
typedef uint8_t               SATUINT8_t;
typedef uint16_t              SATUINT16_t;
typedef uint32_t              SATUINT32_t;
#ifdef INC_STDINT_H
typedef uint64_t              SATUINT64_t;
#else
#ifndef NO64BITINT
typedef uint64_t              SATUINT64_t;
#endif
#endif

/* Integer type */
typedef int8_t               SATINT8_t;
typedef int16_t              SATINT16_t;
typedef int32_t              SATINT32_t;
#ifdef INC_STDINT_H
typedef int64_t              SATINT64_t;
#else
#ifndef NO64BITINT
typedef int64_t              SATINT64_t;
#endif
#endif

typedef uintptr_t            SATUINTPTR_t;

/* Major cipher key/SSP type. */
/* ----- ------ ------- ----- */
typedef uint8_t               SATSSPTYPE;
typedef SATSSPTYPE *          SATSSPTYPEPTR;


/* Asymmetric Ciphers */
/* ---------- ------- */

/* Cipher type. */
typedef uint8_t               SATASYMTYPE;
typedef SATASYMTYPE *         SATASYMTYPEPTR;

/* Cipher size type. */
typedef uint16_t              SATASYMSIZE;
typedef SATASYMSIZE *         SATASYMSIZEPTR;

/* Cipher encoding */
typedef uint8_t               SATRSAENCTYPE;
typedef SATRSAENCTYPE *       SATRSAENCTYPEPTR;

/* DSA public/private key. */
typedef struct {
  uint16_t                    ui16PLen;  /* Length of modulus p. */
  uint16_t                    ui16QLen;  /* Length of prime divisor q. */
  uint32_t                    *pui32P;   /* Prime modulus p. */
  uint32_t                    *pui32Q;   /* Prime divisor q. */
  uint32_t                    *pui32G;   /* Generator g, length==p. */
  uint32_t                    *pui32X;   /* Private key x, length==p. */
  uint32_t                    *pui32Y;   /* Private key y, length==q. */
} SATASYMDSADOMAIN;
typedef SATASYMDSADOMAIN *    SATASYMDSADOMAINPTR;


/* Symmetric Ciphers */
/* --------- ------- */

/* Cipher type. */
typedef uint8_t               SATSYMTYPE;
typedef SATSYMTYPE *          SATSYMTYPEPTR;

/* Cipher key size type (in bits). */
typedef uint16_t              SATSYMKEYSIZE;
typedef SATSYMKEYSIZE *       SATSYMKEYSIZEPTR;

/* Cipher mode type. */
typedef uint8_t               SATSYMMODE;
typedef SATSYMMODE *          SATSYMMODEPTR;
 
/* Cipher key object. */
/* Other fields are only valid when sstCipher!=SATSYMTYPE_NULL. */
typedef struct {
  SATSYMTYPE                  sstCipher;
  SATSYMKEYSIZE               ssksKeyLen;
  uint32_t                    *pui32Key;
} SATSYMKEY;
typedef SATSYMKEY *           SATSYMKEYPTR;


/* Hashes */
/* ------ */

/* Hash type. */
typedef uint8_t               SATHASHTYPE;
typedef SATHASHTYPE *         SATHASHTYPEPTR;

/* Hash size type (in bits). */
typedef uint16_t              SATHASHSIZE;
typedef SATHASHSIZE *         SATHASHSIZEPTR;


/* Context switching. */
/* ----- ------ ------- ----- */

typedef uint32_t              SATRESHANDLE;
typedef SATRESHANDLE *        SATRESHANDLEPTR;

typedef struct {
  SATHASHTYPE       sshashtype;
  uint32_t          uiRunLen;
  uint32_t          uiHash[MAXHASHLEN/32]; /* holds intermed hash */
} SHACTX;

typedef struct {
  SATHASHTYPE       sshashtype;
  uint32_t          uiRunLen;
  uint32_t          uiKeyLen;
  uint32_t          uiHash[MAXHASHLEN/32];   /* holds intermed hash */
  uint32_t          uiKey[MAXHMACKEYLEN/32]; /* holds intermed key */
} SHAHMACCTX;

typedef struct {
  uint8_t uiContextType;
  union{ 
    SHACTX ctxSHA;
    SHAHMACCTX ctxMAC;
  }CTXUNION;     
} SATRESCONTEXT;
typedef SATRESCONTEXT *       SATRESCONTEXTPTR;


/* Message Authentication Codes */
/* ------- -------------- ----- */

/* MAC type. */
typedef uint8_t               SATMACTYPE;
typedef SATMACTYPE *          SATMACTYPEPTR;
typedef uint8_t               SATMACTYPEFLAG;


/* Random Number Generator */
/* ------ ------ --------- */
typedef union {
  uint32_t u32[4];
  uint8_t u8[16];
} uint128_t;

typedef struct {  
  uint128_t ui128V;
  uint128_t ui128K[2];  
  uint32_t uiReseedCnt;
  uint32_t uiReseedLim;
  uint32_t uiEntropyFactor;
  SATSYMKEYSIZE eStrength;
  SATBOOL bTesting;

} DRBGCTX;

typedef DRBGCTX *  DRBGCTXPTR;


/* Function Return Code */
/* -------- ------ ---- */
typedef uint16_t              SATR ;
typedef SATR *                SATRPTR ;


/* Transfer Results */
/* -------- ------- */
typedef struct {
  SATUINT32_t uiLen;
  volatile SATUINT32_t* vpuiSrc;
  void* pDest;
} SATDATATRF;

typedef struct {
  SATUINT32_t uiResType;
  SATUINT32_t uiNumDataTrf;
  SATDATATRF dtrfArray[CAL_MAXTRFS];
} SATRESULT;


/* EC Ultra Structs */
/* -- ----- ------- */
typedef uint32_t         SATECTYPE;

typedef struct {
  SATUINT32_t       uiLen;
  SATUINT32_t*      puiX;
  SATUINT32_t*      puiY;
} SATECPOINT;

typedef struct {
  SATUINT32_t       uiLen;
  SATUINT32_t*      puiMod;
  SATUINT32_t*      puiMontPrecompute;
  SATUINT32_t*      puiRSqd;
} SATECMONTMOD;

typedef struct {
  SATUINT32_t       uiCurveSize;
  SATECTYPE         eCurveType;
  SATECPOINT*       pBasePoint;
  SATECMONTMOD*     pModP;
  SATECMONTMOD*     pModN;
  SATUINT32_t*      puiA;
  SATUINT32_t*      puiB;
} SATECCURVE;

typedef struct {
  SATUINT32_t       uiLen;
  SATUINT32_t*      puiSigR;
  SATUINT32_t*      puiSigS;
  SATUINT32_t*      puiSigX;
  SATUINT32_t*      puiSigY;
} SATECDSASIG;

typedef struct { 
  SATHASHSIZE       sHashLen;
  SATUINT32_t*      puiHash;
} SATHASH;


/* -------- -------- ---------- */
/* External Function Prototypes */
/* -------- -------- ---------- */
#ifndef CALTYPES_C
#ifdef __cplusplus
extern "C" {
#endif

/* NOTE: this header file does not have an associated C file. */

/* -------- ------ --------- */
/* External Global Variables */
/* -------- ------ --------- */

#ifdef __cplusplus
}
#endif
#endif
#endif

