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

#include "hss_crypto.h"

#include <assert.h>

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

bool HSS_Crypto_SHA384(const size_t hashBufSize, uint8_t hashBuffer[hashBufSize],
    const size_t dataBufSize, uint8_t dataBuf[dataBufSize])
{
    bool result = false;

#define SHA384_DIGEST_SIZE  48
    assert(hashBufSize = SHA384_DIGEST_SIZE);
    SATR retval = CALHash(SATHASHTYPE_SHA384, dataBuf, dataBufSize, hashBuffer);

    if (retval == SATR_SUCCESS) {
        result = true;
    }

    return result;
}

bool HSS_Crypto_SHA2(size_t hashBufSize, uint8_t hashBuffer[hashBufSize],
    size_t dataBufSize, uint8_t dataBuf[dataBufSize])
{
    bool result;

    crypto_init_();
#if IS_ENABLED(CONFIG_CRYPTO_SHA512_256)
    result =  HSS_Crypto_SHA512_256(hashBufSize, hashBuffer, dataBufSize, dataBuf);
#else
    result =  HSS_Crypto_SHA384(hashBufSize, hashBuffer, dataBufSize, dataBuf);
#endif

    return result;
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

uint32_t pubKeyX[12];
uint32_t pubKeyY[12];
uint32_t *P384_MOD = 0u; //TBD
uint32_t *sigR = 0u, *sigS = 0u; //TBD

bool HSS_Crypto_Verify_ECDSA_P384(const size_t siglen, uint8_t sigBuffer[siglen], const size_t dataBufSize, uint8_t dataBuf[dataBufSize])
{
    bool result = false;
    SATR retval;

    retval = CALECDSAVerify((const uint32_t *)dataBuf, P384_Gx, P384_Gy, pubKeyX, pubKeyY, sigR, sigS,
        P384_b, P384_MOD, SAT_NULL, P384_n, P384_npc, dataBufSize, SAT_FALSE);

    if (retval == SATR_SUCCESS) {
        retval = CALPKTrfRes(SAT_TRUE);
        if (retval == SATR_SUCCESS) {
            result = true;
        }
    }

    return result;
}
