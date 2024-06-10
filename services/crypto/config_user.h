/* -------------------------------------------------------------------
   $Rev: 727 $ $Date: 2017-10-20 16:50:53 -0400 (Fri, 20 Oct 2017) $
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

   User configuration file to include/exclude CAL components.
   ------------------------------------------------------------------- */

#ifndef CALCONFIG_F5200_H
#define CALCONFIG_F5200_H

#include "x52cfg_user.h"
#include<stdint.h>

#define SAT_LITTLE_ENDIAN 1
#define PKX0_BASE (0x22000000)
#define USEPKX 1
#define USEAESPKX 1
#define USESHAPKX 1
#define USEDRBGPKX 1
#define USENRBGPKX 1
#define USECALCTX 1

#endif
