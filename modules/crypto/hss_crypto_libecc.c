/******************************************************************************************
 *
 * MPFS HSS Embedded Software
 *
 * Copyright 2021 Microchip FPGA Embedded Systems Solutions.
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
 *\file Image Signing Crypto
 *\brief Image Signing Crypto
 */

#include "config.h"
#include "hss_types.h"
#undef CONFIG_CC_HAS_INTTYPES
#include "hss_debug.h"

#include "hss_crypto.h"

#include <string.h>
#include <assert.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"

#include "libsig.h"
#pragma GCC diagnostic pop

#define ECDSA_P384_SIG_LEN ((384u/8)*2)
bool HSS_Crypto_Verify_ECDSA_P384(const size_t siglen, uint8_t sigBuffer[siglen],
    const size_t dataBufSize, uint8_t dataBuf[dataBufSize])
{
    bool result = false;

    assert(siglen == ECDSA_P384_SIG_LEN);

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
    // From the perspective of libecc, these are Affine keys.
#    define X509_ASN1_DER_KEY_OFFSET (24)
#    include "x509-ec-secp384r1-public.h"

    const char x509_asn1_ec_der_p384_root[] = {
      0x30, 0x76, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
      0x01, 0x06, 0x05, 0x2b, 0x81, 0x04, 0x00, 0x22, 0x03, 0x62, 0x00, 0x04,
    };

    if (strncmp(x509_asn1_ec_der_p384_root, SECP384R1_ECDSA_public_key, ARRAY_SIZE(x509_asn1_ec_der_p384_root))) {
        mHSS_DEBUG_PRINTF(LOG_ERROR, "invalid signing certificate type" CRLF);
        result = false;
    } else {
        uint8_t const * const aDataBuf = 0u;
        const uint16_t aDataBufSize = 0u;
        uint8_t const curve_name[] = "SECP384R1";
        const ec_str_params *p_str_params = ec_get_curve_params_by_name(&curve_name[0], ARRAY_SIZE(curve_name));

        ec_pub_key pub_key;
        ec_params params;
        uint8_t u8siglen;

        import_params(&params, p_str_params);

        int retval = ec_get_sig_len(&params, ECDSA, SHA384, (uint8_t*)&u8siglen);

        if (retval) {
            mHSS_DEBUG_PRINTF(LOG_ERROR, "ec_get_sig_len returned %d" CRLF, retval);
            result = false;
        } else {
            uint8_t crv_name_len = ARRAY_SIZE(curve_name);

            retval = ec_check_curve_type_and_name(SECP384R1, params.curve_name, crv_name_len);

            if (retval) {
                mHSS_DEBUG_PRINTF(LOG_ERROR, "ec_check_curve_type_and_name returned %d" CRLF, retval);
                result = false;
            } else {
                retval = ec_pub_key_import_from_aff_buf(&pub_key, &params,
                    (const uint8_t *)&SECP384R1_ECDSA_public_key[X509_ASN1_DER_KEY_OFFSET],
                    ARRAY_SIZE(SECP384R1_ECDSA_public_key) - X509_ASN1_DER_KEY_OFFSET, ECDSA);

                if (retval) {
                    mHSS_DEBUG_PRINTF(LOG_ERROR, "ec_pub_key_import_from_aff_buf returned %d" CRLF, retval);
                    result = false;
                } else {
                    int libecc_result = ec_verify(sigBuffer, u8siglen, &pub_key, dataBuf, dataBufSize,
                        ECDSA, SHA384, aDataBuf, aDataBufSize);

                    result = (libecc_result == 0) ? true : false;
                }
            }
        }
    }

    return result;
}
