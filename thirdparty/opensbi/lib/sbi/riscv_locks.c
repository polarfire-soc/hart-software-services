/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/sbi_hart.h>
#include <sbi/riscv_barrier.h>
#include <sbi/riscv_locks.h>

int spin_lock_check(spinlock_t *lock)
{
	return (lock->lock == __RISCV_SPIN_UNLOCKED) ? 0 : 1;
}

int spin_trylock(spinlock_t *lock)
{
	const int hartid = sbi_current_hartid();
	int tmp = hartid, busy;

	__asm__ __volatile__(
		"	amoswap.w %0, %2, %1\n" RISCV_ACQUIRE_BARRIER
		: "=r"(busy), "+A"(lock->lock)
		: "r"(tmp)
		: "memory");

	return (!busy || (busy == hartid));
}

void spin_lock(spinlock_t *lock)
{
	const int hartid = sbi_current_hartid();
	while (1) {
		if (lock->lock == hartid)
			break;

		if (spin_lock_check(lock))
			continue;

		if (spin_trylock(lock))
			break;
	}
}

void spin_unlock(spinlock_t *lock)
{
	__smp_store_release(&lock->lock, __RISCV_SPIN_UNLOCKED);
}
