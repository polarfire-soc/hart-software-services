/******************************************************************************************
 *
 * MPFS HSS Embedded Software
 *
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
 */

/*\!
 *\file Decompression Wrapper
 *\brief Decompression Wrapper
 */

#include "config.h"
#include "hss_types.h"
#include "hss_debug.h"

#include "hss_crypto.h"

#include <assert.h>
#include <string.h>

#include "calini.h"
#include "calenum.h"
#include "hash.h"

static void crypto_init_(void)
{
    static bool initialized = false;

    if (!initialized) {
        SATR retval = CALIni();
        assert(retval == SATR_SUCCESS);
        initialized = true;
    }
}

// Required constants
const SATUINT32_t P384_Gx[] = {

    0x72760ab7, 0x3a545e38, 0xbf55296c, 0x5502f25d,
    0x82542a38, 0x59f741e0, 0x8ba79b98, 0x6e1d3b62,
    0xf320ad74, 0x8eb1c71e, 0xbe8b0537, 0xaa87ca22
};

const SATUINT32_t P384_Gy[] = {

    0x90ea0e5f, 0x7a431d7c, 0x1d7e819d, 0x0a60b1ce,
    0xb5f0b8c0, 0xe9da3113, 0x289a147c, 0xf8f41dbd,
    0x9292dc29, 0x5d9e98bf, 0x96262c6f, 0x3617de4a
};

const SATUINT32_t P384_b[] = {

    0xd3ec2aef, 0x2a85c8ed, 0x8a2ed19d, 0xc656398d,
    0x5013875a, 0x0314088f, 0xfe814112, 0x181d9c6e,
    0xe3f82d19, 0x988e056b, 0xe23ee7e4, 0xb3312fa7
};


const SATUINT32_t P384_n[] = {

    0xccc52973, 0xecec196a, 0x48b0a77a, 0x581a0db2,
    0xf4372ddf, 0xc7634d81, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};


const SATUINT32_t P384_npc[] = {

    0x333ad68d, 0x1313e695, 0xb74f5885, 0xa7e5f24d,
    0x0bc8d220, 0x389cb27e, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000001
};

bool HSS_Crypto_Verify_ECDSA_P384(const size_t siglen, uint8_t sigBuffer[siglen], const size_t dataBufSize, uint8_t dataBuf[dataBufSize])
{
    bool result = false;
    SATR retval;

    crypto_init_();

    //
    // X5.09 ASN.1 DER keys are of the format
    //
    // offset/len: description
    //
    // bytes 30 76 =>
    //      0/118: SEQUENCE {
    //
    // bytes 30 10 06 07 2A 86 48 CE 3D 02 01 =>
    //      2/ 16:   SEQUENCE {
    //
    // bytes 06 05 2B 81 04 00 22 =>
    //      4/  7:     OBJECT IDENTIFIER ecPublicKey (1 2 840 10045 2 1)
    //     13/  5:     OBJECT IDENTIFIER secp384r1 (1 3 132 0 34)
    //           :     }
    //
    // 03 bytes 02 00 04 xxxx =>
    //     20/ 98:   BIT STRING
    //           :     04  (this means the key is uncompressed)
    //     24/ 96:     xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx
    //           :     xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx
    //           :     xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx
    //           :     xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx
    //           :     xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx
    //           :     xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx
    //           :   }
    //
#    include "x509-ec-secp384r1-public.h"

    const char x509_asn1_ec_der_p384_root[] = {
      0x30, 0x76, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
      0x01, 0x06, 0x05, 0x2b, 0x81, 0x04, 0x00, 0x22, 0x03, 0x62, 0x00, 0x04,
    };

    if (strncmp(x509_asn1_ec_der_p384_root, SECP384R1_ECDSA_public_key, ARRAY_SIZE(x509_asn1_ec_der_p384_root))) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "invalid signing certificate type" CRLF);
        result = false;
    } else {
#define SHA384_DIGEST_SIZE  48
#define P384_MOD 0 //TODO REMOVE
        uint8_t hashBuffer[SHA384_DIGEST_SIZE];
        retval = CALHash(SATHASHTYPE_SHA384, dataBuf, dataBufSize, hashBuffer);

        if (retval == SATR_SUCCESS) {
            retval = CALPKTrfRes(SAT_TRUE);

            uint32_t u32SigBuffer[96/sizeof(uint32_t)];
            memcpy(u32SigBuffer, sigBuffer, 96);

            uint32_t u32PubKeyBuffer[96/sizeof(uint32_t)];
            memcpy(u32PubKeyBuffer, &(x509_asn1_ec_der_p384_root[24]), 96);

            if (retval == SATR_SUCCESS) {
                // signature is composed of (r, s)
                uint32_t *sigR = &(u32SigBuffer[0]), *sigS = &(u32SigBuffer[96/(sizeof(uint32_t)*2)]);

                // public key, after 24-byte header, is composed of (x, y)
                uint32_t *pubKeyX = &(u32PubKeyBuffer[0]), *pubKeyY = &(u32PubKeyBuffer[96/(sizeof(uint32_t)*2)]);

                retval = CALECDSAVerify((const uint32_t *)hashBuffer, P384_Gx, P384_Gy, pubKeyX, pubKeyY,
                    sigR, sigS, P384_b, P384_MOD, SAT_NULL, P384_n, P384_npc, SHA384_DIGEST_SIZE, SAT_FALSE);

                if (retval == SATR_SUCCESS) {
                    retval = CALPKTrfRes(SAT_TRUE);
                    if (retval == SATR_SUCCESS) {
                        result = true;
                    }
                }
            }
        }
    }

    return result;
}
