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
     C header file defining CAL policy for endianess, HW/SW, etc.
   ------------------------------------------------------------------- */

#ifndef CALPOLICY_H
#define CALPOLICY_H

#define INC_STDINT_H
#define CALCONFIGH "config_user.h"
/* -------- */
/* Includes */
/* -------- */

/* The following include provides a custom configuration header file when
   CALCONFIGH is defined
*/
#ifdef CALCONFIGH 
#  include CALCONFIGH
#else
#  error "CALCONFIGH not defined. CAL requires a custom configuration header \
defined by CALCONFIGH. Review CAL README."
#endif

/* ------- */
/* Defines */
/* ------- */

/* Context switching */
#ifndef MAXRESHANDLES
#  define MAXRESHANDLES 1
#endif

/* Little Endian (default) / Big Endian */
#ifndef SAT_LITTLE_ENDIAN
#  define SAT_LITTLE_ENDIAN 1
#endif

/* PK SW Point Validate Checking */
#ifndef PKSWCHKVALPT
#  define PKSWCHKVALPT 1
#endif

/* DMA */
#ifndef USE_X52EXEC_DMA
#  define USE_X52EXEC_DMA 0
#endif

/* SHA */
#define MAXHASHLEN 512
#define MAXHMACKEYLEN 512

/* RNG */
#define NRBGSIMNUMRO 16
#define ENTROPYMEMBLOCKS 7
#define BUFMEMBLOCKS 3
#ifndef RNXBLKLEN
#define RNXBLKLEN 32
#endif
#ifndef RNXBLKOUTLEN
#define RNXBLKOUTLEN 4
#endif

#ifndef USENRBGSW
#  define USENRBGSW 0
#endif

/* PK */
#ifndef PKX0_BASE
#  define PKX0_BASE 0xE0000000u
#endif

#ifdef USEPKSW
#  ifndef MAXMODSIZE
#    define MAXMODSIZE 8192
#  endif
#  ifndef PKSWBUFSIZE
#    define PKSWBUFSIZE 15*(MAXMODSIZE/32)
#  endif
#else
#  define USEPKSW 0
#endif

/* Set default values for X52 configuration defines. */
#ifndef X52_CFG_OPT
#  define X52_CFG_OPT 0
#endif
#ifndef X52_LIR_LEN
#  define X52_LIR_LEN 0x800
#endif
#ifndef X52_BER_LEN
#  define X52_BER_LEN 0x400
#endif
#ifndef X52_MMR_LEN
#  define X52_MMR_LEN 0x400
#endif
#ifndef X52_TSR_LEN
#  define X52_TSR_LEN 0x400
#endif
#ifndef X52_FPR_LEN
#  define X52_FPR_LEN 0x400
#endif
#if X52_LIR_ROM_LEN>0 && X52_LIR_LEN>X52_LIR_ROM_LEN
#  define PKX_OFFSET 2048
#else
#  define PKX_OFFSET 0
#endif

/* X52 Configuration Options */
#define AESPKX        (X52_CFG_OPT&0x00000001u)
#define AESPKXGCM     (X52_CFG_OPT&0x00000008u)
#define AESPKXFASTKEY (X52_CFG_OPT&0x01000000u)
#define SHAPKXOPT1    (X52_CFG_OPT&0x00000020u)
#define SHAPKXOPT224  (X52_CFG_OPT&0x00000040u)
#define SHAPKXOPT256  (X52_CFG_OPT&0x00000080u)
#define SHAPKXOPT384  (X52_CFG_OPT&0x00000100u)
#define SHAPKXOPT512  (X52_CFG_OPT&0x00000200u)
#define SHAPKXOPT5124 (X52_CFG_OPT&0x00400000u)
#define SHAPKXOPT5126 (X52_CFG_OPT&0x00800000u)

/* Define the maximum number of return values that may be handled using 
   CAL*TrfRes() function(s).
*/
#define CAL_MAXTRFS 4

/* Volatile pointer operations */
/* These access macros are designed so that they may be redefined by a 
   user compiling CAL. 
*/
#ifndef CALREAD32
#  define CALREAD32(ptr) ( *(ptr) )
#endif
#ifndef CALWRITE32
#  define CALWRITE32(ptr, val) ( *(ptr)=val )
#endif
#ifndef CALPOLL32
#  define CALPOLL32(ptr, val, mask) while ((*(ptr) & (mask)) != (val));
#endif


/* ---- ----------- */
/* Type Definitions */
/* ---- ----------- */

#ifndef CALPOLICY_C
#ifdef __cplusplus
extern "C" {
#endif

/* -------- -------- ---------- */
/* External Function Prototypes */
/* -------- -------- ---------- */

/* -------- ------ --------- */
/* External Global Variables */
/* -------- ------ --------- */

/* NOTE: this header file does not have an associated C file. */

#ifdef __cplusplus
}
#endif
#endif
#endif

