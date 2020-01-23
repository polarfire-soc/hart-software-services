#ifndef _RISCV_ATOMIC_H_
#define _RISCV_ATOMIC_H_

#include "encoding.h"

#define mb() asm volatile ("fence" ::: "memory")
#define atomic_set(ptr, val) (*(volatile typeof(*(ptr)) *)(ptr) = val)
#define atomic_read(ptr) (*(volatile typeof(*(ptr)) *)(ptr))

#ifdef __riscv_atomic
# define atomic_swap(ptr, swp) __sync_lock_test_and_set(ptr, swp)
# define atomic_or(ptr, inc) __sync_fetch_and_or(ptr, inc)
#else
#define atomic_binop(ptr, inc, op) ({ \
  long flags = disable_irqsave(); \
  typeof(*(ptr)) res = atomic_read(ptr); \
  atomic_set(ptr, op); \
  enable_irqrestore(flags); \
  res; })
#define atomic_or(ptr, inc) atomic_binop(ptr, inc, res | (inc))
#define atomic_swap(ptr, swp) atomic_binop(ptr, swp, (swp))
#endif

#endif

