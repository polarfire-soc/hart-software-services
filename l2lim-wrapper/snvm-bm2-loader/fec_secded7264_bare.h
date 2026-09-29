/*******************************************************************************
 * Copyright 2019-2026 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Freestanding SECDED 72/64 FEC decoder - decode-only interface.
 * See fec_secded7264_bare.c for full attribution.
 */

#ifndef FEC_SECDED7264_BARE_H_
#define FEC_SECDED7264_BARE_H_

#include <stdint.h>

/*
 * fec_secded7264_decode
 *
 * Decode a block of SECDED 72/64 encoded bytes.
 *
 *   _enc_msg_len  - number of encoded input bytes
 *   _msg_enc      - encoded input buffer  (9 bytes encode 8 data bytes)
 *   _msg_dec      - decoded output buffer (must hold _enc_msg_len * 8/9 bytes)
 *   _num_errors   - on return: number of uncorrectable symbols (may be NULL)
 *
 * Returns the number of decoded bytes written to _msg_dec.
 *
 * Uncorrectable (double-bit) errors are counted in *_num_errors. The
 * corresponding decoded bytes are best-effort and should not be trusted.
 * Single-bit errors are silently corrected.
 */
unsigned int fec_secded7264_decode(unsigned int _enc_msg_len,
                                   const unsigned char *_msg_enc,
                                   unsigned char *_msg_dec,
                                   unsigned int *_num_errors);

#endif /* FEC_SECDED7264_BARE_H_ */
