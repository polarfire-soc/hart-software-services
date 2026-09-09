/*******************************************************************************
 * Copyright 2026 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * MPFS Embedded Software
 *
 */

#include <sbi/sbi_ecall.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_trap.h>

/* Previously the vendor ID was to be a number allocated by RISC-V International,
 * but this duplicates the work of JEDEC in maintaining a manufacturer ID standard.
 * SBI ECALLs in the VENDOR region are by MVENDORID as per section 3.1.2 of
 * RISC-V Privileged Architectures spec (2.2-draft or later).
 *
 * Microchip Technology JEDEC ID: Bank 1, 0x29 => MVENDORID = 0x029 */
#define MPFS_SBI_EXT_VENDOR_ID  (SBI_EXT_VENDOR_START | 0x029UL)

static int sbi_ecall_vendor_handler(unsigned long extid, unsigned long funcid,
				    struct sbi_trap_regs *regs,
				    struct sbi_ecall_return *out)
{
	return sbi_platform_vendor_ext_provider(sbi_platform_thishart_ptr(),
						funcid, regs, out);
}

struct sbi_ecall_extension ecall_vendor;

static int sbi_ecall_vendor_register_extensions(void)
{
	if (!sbi_platform_vendor_ext_check(sbi_platform_thishart_ptr()))
		return 0;

	ecall_vendor.extid_start = MPFS_SBI_EXT_VENDOR_ID;
	ecall_vendor.extid_end   = MPFS_SBI_EXT_VENDOR_ID;

	return sbi_ecall_register_extension(&ecall_vendor);
}

struct sbi_ecall_extension ecall_vendor = {
	.name			= "vendor",
	.extid_start		= SBI_EXT_VENDOR_START,
	.extid_end		= SBI_EXT_VENDOR_END,
	.register_extensions	= sbi_ecall_vendor_register_extensions,
	.handle			= sbi_ecall_vendor_handler,
};
