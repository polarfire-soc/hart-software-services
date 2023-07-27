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
      C header file for CAL context management functions.
   ------------------------------------------------------------------- */

#ifndef CALCONTEXT_H
#define CALCONTEXT_H

/* -------- */
/* Includes */
/* -------- */
#include "calpolicy.h"
#include "caltypes.h"


/* ------- */
/* Defines */
/* ------- */

/* Function resource handle. */
/* ----- ------ ------- ---- */
#define  SATRES_DEFAULT       (SATRESHANDLE)0U
#define  SATRES_CALSW         (SATRESHANDLE)1U


/* ---- ----------- */
/* Type Definitions */
/* ---- ----------- */
typedef  struct{
  SATUINT32_t uiBase;
  SATRESCONTEXTPTR pContext;
 }SATRESHANDLESTRUCT;
 

/* -------- -------- ---------- */
/* External Function Prototypes */
/* -------- -------- ---------- */
#ifndef CALCONTEXT_C
#ifdef __cplusplus
extern "C" {
#endif

/* Published API Functions */
/* --------- --- --------- */
extern SATRESCONTEXTPTR CALContextCurrent(const SATRESHANDLE hResource);

extern SATR CALContextLoad(const SATRESHANDLE hResource, 
  const SATRESCONTEXTPTR pContext);

extern SATR CALContextRemove(const SATRESHANDLE hResource);

extern SATR CALContextUnload(const SATRESHANDLE hResource);


/* Unpublished Function Prototypes */
/* ----------- -------- ---------- */
extern void init_reshandles(void);


/* -------- ------ --------- */
/* External Global Variables */
/* -------- ------ --------- */

#ifdef __cplusplus
}
#endif
#endif
#endif
