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
#include <sbi/riscv_atomic.h>
#include <sbi/riscv_barrier.h>
#include <calini.h>
#include <sym.h>
#include <pk.h>
#include <pkx.h>
#include <config_athena.h>
#include "mss_peripherals.h"
#include "opensbi_ecall.h"
#include "opensbi_crypto_ecall.h"

#define AES_DIR_DECRYPT		0x00
#define AES_DIR_ENCRYPT		0x01

#define AES_MODE_ECB		0x0100
#define AES_MODE_CBC		0x0200
#define AES_MODE_CFB		0x0300
#define AES_MODE_OFB		0x0400
#define AES_MODE_CTR		0x0500
#define AES_MODE_GCM		0x0600
#define AES_MODE_GHASH		0x0800

#define AES_TYPE_128		0x010000
#define AES_TYPE_192		0x020000
#define AES_TYPE_256		0x030000

#define MASK_8BIT		0xFF

#define DISABLED		0
#define ENABLED			1

static bool crypto_initialized = false;
static atomic_t crypto_in_progress = ATOMIC_INITIALIZER(0);

static int crypto_aes_operation(const struct sbi_trap_regs *regs)
{
	uint8_t aes_key_type;
	uint8_t aes_algo_mode;
	uint8_t status;
	uint8_t load_iv;
	uint32_t flags;
	struct mchp_crypto_aes_req *aes;

	aes = (struct mchp_crypto_aes_req *)regs->a1;
	flags = (uint32_t)regs->a2;

	/* AES Key Type - 128 or 192 or 256-bit */
	aes_key_type = (flags >> 16) & MASK_8BIT;
	/* AES Modes - ECB, CBC, CFB, OFB, CTR or GCM */
	aes_algo_mode = (flags >> 8) & MASK_8BIT;
	/* No IV for AES ECB mode */
	load_iv = aes_algo_mode != SATSYMMODE_ECB? SAT_TRUE: SAT_FALSE;

	if (flags & AES_DIR_ENCRYPT) {
		status = CALSymEncryptDMA(aes_key_type, (uint32_t *)aes->key,
					  aes_algo_mode, (void *)aes->iv, load_iv,
					  (void *)aes->src, (void *)aes->dst,
					  aes->size, X52CCR_DEFAULT);
	} else {
		status = CALSymDecryptDMA(aes_key_type, (uint32_t *)aes->key,
					  aes_algo_mode, (void *)aes->iv, load_iv,
					  (void *)aes->src, (void *)aes->dst,
					  aes->size, X52CCR_DEFAULT);
	}

	if (SATR_SUCCESS != status)
		return SBI_EFAIL;

	status = CALPKTrfRes(SAT_TRUE);
	if (SATR_SUCCESS != status)
		return SBI_EFAIL;

	return SBI_OK;
}

int sbi_ecall_crypto_handler(unsigned long extid,
			     unsigned long funcid,
			     const struct sbi_trap_regs *regs,
			     unsigned long *out_val,
			     struct sbi_trap_info *out_trap)
{
	int result = SBI_EFAIL;
	uint32_t crypto_service;
	uint8_t crypto_clk_enable;
	struct mchp_crypto_info *crypto;

	switch (funcid) {
	case SBI_EXT_CRYPTO_INIT:
		crypto_clk_enable = (uint8_t)regs->a0;
		switch (crypto_clk_enable) {
		case ENABLED:
			if (!crypto_initialized) {
				(void) mss_config_clk_rst(MSS_PERIPH_CRYPTO,
							  (uint8_t) 0, PERIPHERAL_ON);
				(void) mss_config_clk_rst(MSS_PERIPH_ATHENA,
							  (uint8_t) 0, PERIPHERAL_ON);
				/*
				 * Initialize the Athena core. To enable user
				 * crypto bring out off reset and enable RINGOSC on.
				 */
				ATHENAREG->ATHENA_CR = SYSREG_ATHENACR_RESET |
						       SYSREG_ATHENACR_RINGOSCON;
				ATHENAREG->ATHENA_CR = SYSREG_ATHENACR_RINGOSCON;

				/* Initializes the Athena Processor. */
				CALIni();
				crypto_initialized = true;
			}

			result = SBI_OK;
			break;
		case DISABLED:
			if (crypto_initialized) {
				(void) mss_config_clk_rst(MSS_PERIPH_CRYPTO,
							  (uint8_t) 0, PERIPHERAL_OFF);
				(void) mss_config_clk_rst(MSS_PERIPH_ATHENA,
							  (uint8_t) 0, PERIPHERAL_OFF);
				crypto_initialized = false;

				result = SBI_OK;
			} else {
				result = SBI_EALREADY_STOPPED;
			}

			break;
		default:
			break;
		}

		break;
	case SBI_EXT_CRYPTO_SERVICES_PROBE:
		if (crypto_initialized) {
			crypto = (struct mchp_crypto_info *)regs->a0;

			crypto->services = CRYPTO_SERVICE_AES;
			crypto->cipher_algo = CRYPTO_ALG_AES_ECB |
					      CRYPTO_ALG_AES_CBC |
					      CRYPTO_ALG_AES_OFB |
					      CRYPTO_ALG_AES_CFB |
					      CRYPTO_ALG_AES_CTR;
			crypto->aead_algo = 0;
			crypto->hash_algo = 0;
			crypto->mac_algo = 0;
			crypto->akcipher_algo = 0;
			crypto->nrbg_algo = 0;

			result = SBI_OK;
		}

		break;
	case SBI_EXT_CRYPTO_SERVICES:
		if (crypto_initialized) {
			if (!atomic_xchg(&crypto_in_progress, 1)) {
				crypto_service = (uint32_t)regs->a0;
				switch (crypto_service) {
				case CRYPTO_SERVICE_AES:
					result = crypto_aes_operation(regs);
					break;
				default:
					break;
				}

				atomic_write(&crypto_in_progress, 0);
			} else {
				result = SBI_ERR_ALREADY_STARTED;
			}
		}

		break;
	default:
		break;
	}

	return result;
}
