/* -------------------------------------------------------------------
   $Rev: 1434 $ $Date: 2017-10-20 16:46:16 -0400 (Fri, 20 Oct 2017) $
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
     C header file for PKX-5200 Library.
   ------------------------------------------------------------------- */

#ifndef PKXLIB_H
#define PKXLIB_H

#include "caltypes.h"
#include "calpolicy.h"

#define PKX_JMP_NONE                 (0xFFFFu + PKX_OFFSET)

/* jump table entry points: starting PC value */
#define PKX_JMP_RECIP_PRECOMPUTE     (0x0000 + PKX_OFFSET)
#define PKX_JMP_MOD_EXP              (0x0002 + PKX_OFFSET)
#define PKX_JMP_RSA_CRT              (0x0004 + PKX_OFFSET)
#define PKX_JMP_EC_PTMUL             (0x0006 + PKX_OFFSET)
#define PKX_JMP_EC_DSA_SIGN          (0x0008 + PKX_OFFSET)
#define PKX_JMP_EC_DSA_VERIFY        (0x000A + PKX_OFFSET)
#define PKX_JMP_DSA_SIGN             (0x000C + PKX_OFFSET)
#define PKX_JMP_DSA_VERIFY           (0x000E + PKX_OFFSET)
#define PKX_JMP_MOD_MULT             (0x0010 + PKX_OFFSET)
#define PKX_JMP_MOD_RED              (0x0012 + PKX_OFFSET)
#define PKX_JMP_EC_PTDECOMP          (0x0014 + PKX_OFFSET)
#define PKX_JMP_EC_DHC               (0x0016 + PKX_OFFSET)
#define PKX_JMP_MOD_MULT_ADD         (0x0018 + PKX_OFFSET)
#define PKX_JMP_EC_PTMUL_ADD         (0x001A + PKX_OFFSET)
#define PKX_JMP_EC_PTVALIDATE        (0x001C + PKX_OFFSET)
#define PKX_JMP_F5200_SHA            (0x001E + PKX_OFFSET)
#define PKX_JMP_F5200_AES            (0x0020 + PKX_OFFSET)
#define PKX_JMP_F5200_AESK           (0x0022 + PKX_OFFSET)
#define PKX_JMP_F5200_GCM            (0x0024 + PKX_OFFSET)
#define PKX_JMP_F5200_GHA            (0x0026 + PKX_OFFSET)
#define PKX_JMP_F5200_AESKW          (0x0028 + PKX_OFFSET)
#define PKX_JMP_F5200_AESKWP         (0x002A + PKX_OFFSET)
#define PKX_JMP_RNG_INSTANTIATE      (0x002C + PKX_OFFSET)
#define PKX_JMP_RNG_RESEED           (0x002E + PKX_OFFSET)
#define PKX_JMP_RNG_GENERATE         (0x0030 + PKX_OFFSET)
#define PKX_JMP_RNG_UNINSTANTIATE    (0x0032 + PKX_OFFSET)
#define PKX_JMP_RNG_GETENTROPY       (0x0034 + PKX_OFFSET)
#define PKX_JMP_F5200_SHA_HMAC       (0x0036 + PKX_OFFSET)
#define PKX_JMP_F5200_AES_DMA        (0x0038 + PKX_OFFSET)
#define PKX_JMP_RNG_CTRLSTATUS       (0x003A + PKX_OFFSET)
#define PKX_JMP_F5200_SHA_DMA        (0x003C + PKX_OFFSET)
#define PKX_JMP_SHX_KEYTREE          (0x003E + PKX_OFFSET)
#define PKX_JMP_PKX_DSA_DMA          (0x0040 + PKX_OFFSET)
#define PKX_JMP_PKX_RSACRT_SIGN      (0x0042 + PKX_OFFSET)
#define PKX_JMP_PKX_RSA_SIGN         (0x0044 + PKX_OFFSET)
#define PKX_JMP_PKX_RSA_VERIFY       (0x0046 + PKX_OFFSET)
#define PKX_JMP_PKX_EC_DSA_DMA       (0x0048 + PKX_OFFSET)
#define PKX_JMP_F5200_AES_KEYROLL    (0x004A + PKX_OFFSET)
#define PKX_JMP_EXPM                 (0x004C + PKX_OFFSET)
#define PKX_JMP_RSACRTM              (0x004E + PKX_OFFSET)
#define PKX_JMP_EC_PTMULM            (0x0050 + PKX_OFFSET)
#define PKX_JMP_DSA_SIGN_M           (0x0052 + PKX_OFFSET)
#define PKX_JMP_EC_DSA_SIGN_M        (0x0054 + PKX_OFFSET)
#define PKX_JMP_EC_KEYPAIRGEN        (0x0056 + PKX_OFFSET)
#define PKX_JMP_RSACRTCM_SIGN        (0x0058 + PKX_OFFSET)
#define PKX_JMP_F5200_GCM_NEW        (0x005A + PKX_OFFSET)
/* PKX PKRev 2180 */
/* PKX SHARev 2160 */
/* PKX AESRev 2135 */
/* Hex Checksum: 0xd0d79866 */

extern const SATUINT32_t uiPKX_Flags;
extern const SATUINT32_t uiPKX_BERWords;
extern const SATUINT32_t uiPKX_LIRWords;
extern const SATUINT32_t uiPKX_Rev;
extern const SATUINT32_t uiPKX_LibSize;
extern const SATUINT32_t uiPKX_LibChksum;
extern const SATUINT32_t uiPKX_Lib[];
extern SATBOOL bMPF300TS_ES;

#endif
