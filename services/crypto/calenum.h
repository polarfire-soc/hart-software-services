/* -------------------------------------------------------------------
   $Rev: 1566 $ $Date: 2018-09-14 11:04:30 -0400 (Fri, 14 Sep 2018) $
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

     Most CAL types and constants are defined with the "SAT" prefix to
     reduce the chance of name space collisions with other users.
   ------------------------------------------------------------------- */

#ifndef CALENUM_H
#define CALENUM_H

/* -------- */
/* Includes */
/* -------- */

/* CAL base types. */
#include "caltypes.h"


/* ------- */
/* Defines */
/* ------- */
/* NULL definitions. */
#define SAT_NULL 0

/* Boolean definitions. */
#define SAT_TRUE ((SATBOOL)1)
#define SAT_FALSE ((SATBOOL)0)


/* Major cipher key/SSP type. */
/* ----- ------ ------- ----- */
#define SATSSPTYPE_NULL       (SATSSPTYPE)0
#define SATSSPTYPE_SYMKEY     (SATSSPTYPE)1
#define SATSSPTYPE_ASYMKEY    (SATSSPTYPE)2
/* Special marker for end of list. */
#define SATSSPTYPE_LAST       (SATSSPTYPE)3


/* Asymmetric Ciphers */
/* ---------- ------- */
#define SATASYMTYPE_NULL           (SATASYMTYPE)0U
#define SATASYMTYPE_DSA_SIGN       (SATASYMTYPE)1U
#define SATASYMTYPE_DSA_VERIFY     (SATASYMTYPE)2U
#define SATASYMTYPE_RSA_ENCRYPT    (SATASYMTYPE)3U
#define SATASYMTYPE_RSA_DECRYPT    (SATASYMTYPE)4U
#define SATASYMTYPE_DH             (SATASYMTYPE)5U
#define SATASYMTYPE_ECDSA_SIGN     (SATASYMTYPE)6U
#define SATASYMTYPE_ECDSA_VERIFY   (SATASYMTYPE)7U
#define SATASYMTYPE_ECDH           (SATASYMTYPE)8U
#define SATASYMTYPE_RSA_SIGN       (SATASYMTYPE)9U
#define SATASYMTYPE_RSA_VERIFY     (SATASYMTYPE)10U
/* Special marker for end of list. */
#define SATASYMTYPE_LAST           (SATASYMTYPE)11U


/* Encoding Types */
/* -------- ----- */
#define SATRSAENCTYPE_NULL       (SATRSAENCTYPE)0U
#define SATRSAENCTYPE_PKCS       (SATRSAENCTYPE)1U
#define SATRSAENCTYPE_ANSI       (SATRSAENCTYPE)2U
#define SATRSAENCTYPE_PSS        (SATRSAENCTYPE)3U
/* Special marker for end of list. */
#define SATRSAENCTYPE_LAST       (SATRSAENCTYPE)4U


/* Symmetric Ciphers */
/* --------- ------- */

/* Cipher Type. */
#define SATSYMTYPE_NULL            (SATSYMTYPE)0U
#define SATSYMTYPE_AES128          (SATSYMTYPE)1U
#define SATSYMTYPE_AES192          (SATSYMTYPE)2U
#define SATSYMTYPE_AES256          (SATSYMTYPE)3U
#define SATSYMTYPE_AESKS128        (SATSYMTYPE)4U
#define SATSYMTYPE_AESKS192        (SATSYMTYPE)5U
#define SATSYMTYPE_AESKS256        (SATSYMTYPE)6U
/* Special marker for end of list. */
#define SATSYMTYPE_LAST            (SATSYMTYPE)7U

/* Names for common cipher key lengths, in bits. */
#define SATSYMKEYSIZE_AES128       (SATSYMKEYSIZE)128U
#define SATSYMKEYSIZE_AES192       (SATSYMKEYSIZE)192U
#define SATSYMKEYSIZE_AES256       (SATSYMKEYSIZE)256U
#define SATSYMKEYSIZE_AESKS128     (SATSYMKEYSIZE)256U
#define SATSYMKEYSIZE_AESKS192     (SATSYMKEYSIZE)384U
#define SATSYMKEYSIZE_AESKS256     (SATSYMKEYSIZE)512U

/* Cipher Mode. */
#define SATSYMMODE_NULL            (SATSYMMODE)0U
#define SATSYMMODE_ECB             (SATSYMMODE)1U
#define SATSYMMODE_CBC             (SATSYMMODE)2U
#define SATSYMMODE_CFB             (SATSYMMODE)3U
#define SATSYMMODE_OFB             (SATSYMMODE)4U
#define SATSYMMODE_CTR             (SATSYMMODE)5U
#define SATSYMMODE_GCM             (SATSYMMODE)6U
#define SATSYMMODE_GHASH           (SATSYMMODE)8U
/* Special marker for end of list. */
#define SATSYMMODE_LAST            (SATSYMMODE)9U


/* Hashes */
/* ------ */
#define SATHASHTYPE_NULL           (SATHASHTYPE)0U
#define SATHASHTYPE_SHA1           (SATHASHTYPE)1U
#define SATHASHTYPE_SHA224         (SATHASHTYPE)2U
#define SATHASHTYPE_SHA256         (SATHASHTYPE)3U
#define SATHASHTYPE_SHA384         (SATHASHTYPE)4U
#define SATHASHTYPE_SHA512         (SATHASHTYPE)5U
#define SATHASHTYPE_SHA512_224     (SATHASHTYPE)6U
#define SATHASHTYPE_SHA512_256     (SATHASHTYPE)7U
/* Special marker for end of list. */
#define SATHASHTYPE_LAST           (SATHASHTYPE)8U

/* Hash sizes defined in bits */
#define SATHASHSIZE_NULL           (SATHASHSIZE)0U
#define SATHASHSIZE_SHA1           (SATHASHSIZE)160U
#define SATHASHSIZE_SHA224         (SATHASHSIZE)224U
#define SATHASHSIZE_SHA256         (SATHASHSIZE)256U
#define SATHASHSIZE_SHA384         (SATHASHSIZE)384U
#define SATHASHSIZE_SHA512         (SATHASHSIZE)512U
#define SATHASHSIZE_SHA512_224     (SATHASHSIZE)224U
#define SATHASHSIZE_SHA512_256     (SATHASHSIZE)256U

#define SATHASHSIZE_HASH160        (SATHASHSIZE)160U
#define SATHASHSIZE_HASH192        (SATHASHSIZE)192U
#define SATHASHSIZE_HASH224        (SATHASHSIZE)224U
#define SATHASHSIZE_HASH256        (SATHASHSIZE)256U
#define SATHASHSIZE_HASH320        (SATHASHSIZE)320U
#define SATHASHSIZE_HASH384        (SATHASHSIZE)384U
#define SATHASHSIZE_HASH512        (SATHASHSIZE)512U
#define SATHASHSIZE_HASH521        (SATHASHSIZE)521U


/* Message Authentication Codes */
/* ------- -------------- ----- */

/* Message Authentication Types */
#define SATMACTYPE_NULL            (SATMACTYPE)0U
#define SATMACTYPE_SHA1            (SATMACTYPE)1U
#define SATMACTYPE_SHA224          (SATMACTYPE)2U
#define SATMACTYPE_SHA256          (SATMACTYPE)3U
#define SATMACTYPE_SHA384          (SATMACTYPE)4U
#define SATMACTYPE_SHA512          (SATMACTYPE)5U
#define SATMACTYPE_SHA512_224      (SATMACTYPE)6U
#define SATMACTYPE_SHA512_256      (SATMACTYPE)7U
#define SATMACTYPE_AESCMAC128      (SATMACTYPE)10U
#define SATMACTYPE_AESCMAC192      (SATMACTYPE)11U
#define SATMACTYPE_AESCMAC256      (SATMACTYPE)12U
#define SATMACTYPE_AESGMAC         (SATMACTYPE)13U
/* Special marker for end of list. */
#define SATMACTYPE_LAST            (SATMACTYPE)14U


/* Message Authentication Flags */
#define SATMACFLAG_OP             (SATMACTYPEFLAG)0U
#define SATMACFLAG_FIRSTPASS      (SATMACTYPEFLAG)1U
#define SATMACFLAG_FINALPASS      (SATMACTYPEFLAG)2U
#define SATMACFLAG_IKEYFINAL      (SATMACTYPEFLAG)4U
#define SATMACFLAG_OKEYFINAL      (SATMACTYPEFLAG)8U


/* Non-deterministic Random Bit Generator */
/* ------- -------------- ----- */

/* NRBG register write enables */
#define SATNRBGCONFIG_NONE            0x0
#define SATNRBGCONFIG_RNG_CSR         0x1
#define SATNRBGCONFIG_RNG_CNTLIM      0x2
#define SATNRBGCONFIG_RNG_VOTIMER     0X4
#define SATNRBGCONFIG_RNG_FMSK        0X8

/* RNG_CSR access defines */
#define SATNRBGCONFIG_CSR_RODIS       0x0
#define SATNRBGCONFIG_CSR_ROEN        0x1
#define SATNRBGCONFIG_CSR_ROFATAL     0x2
#define SATNRBGCONFIG_CSR_ROFATALCLR  0X4

/* RNG_FMSK mask values */
#define SATNRBGCONFIG_FMSK_ROOSCF     0xFF
#define SATNRBGCONFIG_FMSK_MONOBITF   0x10000
#define SATNRBGCONFIG_FMSK_POKERF     0x20000
#define SATNRBGCONFIG_FMSK_RUNSF      0x40000
#define SATNRBGCONFIG_FMSK_LRUNSF     0x80000
#define SATNRBGCONFIG_FMSK_F1401      0xF0000
#define SATNRBGCONFIG_FMSK_REPCNTF    0x100000
#define SATNRBGCONFIG_FMSK_APROPF     0x200000
#define SATNRBGCONFIG_FMSK_SP800      0x300000

/* RNG_ROHEALTH mask values */
#define SATNRBGCONFIG_HLTH_ROOSCF     0x3FC0
#define SATNRBGCONFIG_HLTH_APROPF     0x20
#define SATNRBGCONFIG_HLTH_REPCNTF    0x10
#define SATNRBGCONFIG_HLTH_LRUNSF     0x8
#define SATNRBGCONFIG_HLTH_RUNSF      0x4
#define SATNRBGCONFIG_HLTH_POKERF     0x2
#define SATNRBGCONFIG_HLTH_MONOBITF   0x1


/* Return Codes */
/* ------ ----- */
#define SATR_SUCCESS               (SATR)0U
#define SATR_FAIL                  (SATR)1U
#define SATR_BADPARAM              (SATR)2U
#define SATR_VERIFYFAIL            (SATR)3U
#define SATR_KEYSFULL              (SATR)4U
#define SATR_BUSY                  (SATR)5U
#define SATR_ROFATAL               (SATR)6U
#define SATR_PARITYFLUSH           (SATR)7U
#define SATR_SIGNFAIL              (SATR)8U
#define SATR_VALIDATEFAIL          (SATR)9U
#define SATR_PAF                   (SATR)10U
#define SATR_VALPARMX              (SATR)11U
#define SATR_VALPARMY              (SATR)12U
#define SATR_VALPARMB              (SATR)13U
#define SATR_DCMPPARMX             (SATR)14U
#define SATR_DCMPPARMB             (SATR)15U
#define SATR_DCMPPARMP             (SATR)16U
#define SATR_SIGNPARMD             (SATR)17U
#define SATR_SIGNPARMK             (SATR)18U
#define SATR_VERPARMR              (SATR)19U
#define SATR_VERPARMS              (SATR)20U
#define SATR_MSBICV1               (SATR)21U
#define SATR_MSBICV2               (SATR)22U
#define SATR_PADLEN                (SATR)23U
#define SATR_LSB0PAD               (SATR)24U
#define SATR_BADLEN                (SATR)25U
#define SATR_BADHASHTYPE           (SATR)26U
#define SATR_BADTYPE               (SATR)27U
#define SATR_BADMODE               (SATR)28U
#define SATR_BADCONTEXT            (SATR)29U
#define SATR_BADHASHLEN            (SATR)30U
#define SATR_BADMACTYPE            (SATR)31U
#define SATR_BADMACLEN             (SATR)32U
#define SATR_BADHANDLE             (SATR)33U
#define SATR_FNP                   (SATR)34U
#define SATR_HFAULT                (SATR)35U
#define SATR_NOPEND                (SATR)36U
#define SATR_BADRSAENC             (SATR)37U
#define SATR_BADMOD                (SATR)38U
/* Special marker for end of list. */
#define SATR_LAST                  (SATR)39U


/* ---- ----------- */
/* Type Definitions */
/* ---- ----------- */

/* See caltypes.h for type definitions associated with defines above. */

/* -------- -------- ---------- */
/* External Function Prototypes */
/* -------- -------- ---------- */
#ifndef CALENUM_C
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
