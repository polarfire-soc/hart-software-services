/*******************************************************************************
 * Copyright 2019-2022 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS Embedded Software
 *
 */

#include "config.h"
#include "hss_types.h"
#include "opensbi_service.h"

#if !IS_ENABLED(CONFIG_OPENSBI)
#  error OPENSBI needed for this module
#endif

#include <sbi/sbi_ecall.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_trap.h>
#include <sbi/sbi_version.h>
#include <sbi/riscv_asm.h>
#include <sbi/riscv_barrier.h>
#include <calini.h>
#include <sym.h>
#include <pk.h>
#include <pkx.h>
#include <config_athena.h>
#include "mss_peripherals.h"
#include "opensbi_ecall.h"
#include "opensbi_crypto_ecall.h"

#define AES_DIR_DECRYPT 0x00
#define AES_DIR_ENCRYPT 0x01

#define AES_MODE_ECB 	0x0100
#define AES_MODE_CBC 	0x0200
#define AES_MODE_CFB 	0x0300
#define AES_MODE_OFB 	0x0400
#define AES_MODE_CTR 	0x0500
#define AES_MODE_GCM 	0x0600
#define AES_MODE_GHASH 	0x0800

#define AES_TYPE_128 	0x010000
#define AES_TYPE_192 	0x020000
#define AES_TYPE_256 	0x030000

#define CRYPTO_SERVICE_AES  	0x01
#define CRYPTO_SERVICE_RSA  	0x02
#define CRYPTO_SERVICE_DSA  	0x03
#define CRYPTO_SERVICE_ECDSA  	0x04
#define CRYPTO_SERVICE_HASH  	0x05
#define CRYPTO_SERVICE_MAC  	0x06
#define CRYPTO_SERVICE_DRBG  	0x07

static int crypto_aes_operation(const struct sbi_trap_regs *regs)
{
    int result = 0;
    uint32_t * key;
    uint32_t * src;
    uint32_t * dst;
    uint32_t * iv;
    uint32_t len;
    uint32_t flags;
    uint8_t aes_key_type;
    uint8_t aes_mode;
    uint8_t status;
    struct mc_crypto_aes_op *aes_addr;

    aes_addr = (struct mc_crypto_aes_op *) regs->a1;
    flags = (uint32_t) regs->a2;

    src = (uint32_t *) aes_addr->src_addr;
    dst = (uint32_t *)  aes_addr->dst_addr;
    key = (uint32_t *) aes_addr->key_addr;
    iv = (uint32_t *)  aes_addr->iv_addr;
    len = (uint32_t)  aes_addr->length;

    /* AES Key Type - 128 or 192 or 256-bit */
    aes_key_type = (flags >> 16) & 0xff;
    /* AES Modes - ECB, CBC, CFB, OFB, CTR or GCM */
    aes_mode = (flags >> 8) & 0xff;

    if (flags & AES_DIR_ENCRYPT) {
        status = CALSymEncrypt(aes_key_type, key, aes_mode, iv,
                               aes_mode != SATSYMMODE_ECB? SAT_TRUE: SAT_FALSE,
                               src, dst, len);
    } else {
        status = CALSymDecrypt(aes_key_type, key, aes_mode, iv,
                               aes_mode != SATSYMMODE_ECB? SAT_TRUE: SAT_FALSE,
                               src, dst, len);
    }

    if (SATR_SUCCESS == status) {
        status = CALPKTrfRes(SAT_TRUE);
        if (SATR_SUCCESS == status)
            result = SBI_OK;
        else
            result = SBI_ERR_FAILED;
    } else {
        result = SBI_ERR_FAILED;
    }

    return result;
}

int sbi_ecall_crypto_handler(unsigned long extid, unsigned long funcid,
    const struct sbi_trap_regs *regs, unsigned long *out_val, struct sbi_trap_info *out_trap)
{
    int result = 0;
    uint32_t crypto_service;

    if (funcid == SBI_EXT_CRYPTO_INIT) {
        (void)mss_config_clk_rst(MSS_PERIPH_CRYPTO, (uint8_t) 0, PERIPHERAL_ON);
        (void)mss_config_clk_rst(MSS_PERIPH_ATHENA, (uint8_t) 0, PERIPHERAL_ON);
        /* Initialize the Athena core. To enable user crypto bring out off reset
         *  and enable RINGOSC on. */
        ATHENAREG->ATHENA_CR = SYSREG_ATHENACR_RESET | SYSREG_ATHENACR_RINGOSCON;
        ATHENAREG->ATHENA_CR = SYSREG_ATHENACR_RINGOSCON;
        /* Initializes the Athena Processor. */
        CALIni();
        result = SBI_OK;
    } else if (funcid == SBI_EXT_CRYPTO_SERVICE){

        crypto_service = (uint32_t) regs->a0;

        switch (crypto_service) {
            case CRYPTO_SERVICE_AES:
                result = crypto_aes_operation(regs);
                break;
            default:
                result = SBI_ENOTSUPP;
                break;
        }
    } else {
        result = SBI_ENOTSUPP;
    }

    if (result >= 0) {
        *out_val = result;
        result = 0;
    }

    return result;
}
