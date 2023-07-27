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
/* X5200 configuration */
/* $Date: 2015-07-23 14:30:19 -0400 (Thu, 23 Jul 2015) $ $Rev: 2093 $ */
/* -------------------------------------------------------------------
   Options:
   * LIR ROM size : 4096
   * LIR RAM size : 2048
   * BER size     : 1024
   * MMR size     : 1024
   * TSR size     : 1024
   * FPR size     : 1024
   * Single error correct, dual error detect (SECDED) memory parity
   * DMA enabled
   * FPGA multipliers disabled
   * AES enabled
     - Fast key schedule generation
     - GCM/GHASH enabled
     - AES countermeasures enabled
   * RNG enabled
   * External PRNG disabled
   * SHA enabled
     - Fast SHA enabled
     - SHA-1 enabled
     - SHA-224 enabled
     - SHA-256 enabled
     - SHA-384 enabled
     - SHA-512 enabled
     - SHA-512/224 enabled
     - SHA-512/256 enabled
     - SHA countermeasures enabled
   * P-curves populated in FCR:
     - P-192 populated
     - P-224 populated
     - P-256 populated
     - P-384 populated
     - P-521 populated
   * Data memory scrambling enabled

   ------------------------------------------------------------------- */

#ifndef X52CFG_H
#define X52CFG_H

#define X52_CFG_MODEL   0xf5200
#define X52_CFG_DATE    0x15072720
#define X52_CFG_REV     0x0000082e
#define X52_CFG_OPT     0x0fd87ff9
#define X52_CFG_OPT2    0xc0000000
#define X52_LIR_LEN     0x1800
#define X52_LIR_ROM_LEN 0x1000
#define X52_LIR_RAM_LEN 0x0800
#define X52_BER_LEN     0x400
#define X52_MMR_LEN     0x400
#define X52_TSR_LEN     0x400
#define X52_FPR_LEN     0x400

#endif
