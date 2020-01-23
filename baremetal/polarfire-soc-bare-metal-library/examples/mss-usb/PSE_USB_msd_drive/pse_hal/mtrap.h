#ifndef _RISCV_MTRAP_H_
#define _RISCV_MTRAP_H_

#include "bits.h"
#include "encoding.h"

#ifndef __ASSEMBLER__
#define read_const_csr(reg) ({ unsigned long __tmp; \
  asm ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })
#endif

#define IPI_SOFT          0x01
#define IPI_FENCE_I       0x02
#define IPI_SFENCE_VMA    0x04

#define MACHINE_STACK_SIZE  (RISCV_PGSIZE)    /* this is 4k for HLS and 4k for the stack*/
#define MENTRY_HLS_OFFSET   (INTEGER_CONTEXT_SIZE + SOFT_FLOAT_CONTEXT_SIZE)
#define MENTRY_FRAME_SIZE   (MENTRY_HLS_OFFSET + HLS_SIZE)
#define MENTRY_IPI_OFFSET   (MENTRY_HLS_OFFSET)
#define MENTRY_IPI_PENDING_OFFSET (MENTRY_HLS_OFFSET + REGBYTES)

#ifdef __riscv_flen
# define SOFT_FLOAT_CONTEXT_SIZE (0)
#else
# define SOFT_FLOAT_CONTEXT_SIZE (8 * 32)
#endif

#define HLS_SIZE          (64)
#define INTEGER_CONTEXT_SIZE (32 * REGBYTES)

#ifndef __ASSEMBLER__
typedef struct {
  volatile uint32_t *  ipi;
  volatile int         mipi_pending;
  volatile uint64_t *  timecmp;
  volatile uint32_t *  plic_m_thresh;
  volatile uintptr_t * plic_m_ie;
  volatile uint32_t *  plic_s_thresh;
  volatile uintptr_t * plic_s_ie;
} hls_t;

/* This code relies on the stack being allocated on a 4K boundary */
/* also can  not be bigger than 4k */
#define MACHINE_STACK_TOP() ({ \
  register uintptr_t sp asm ("sp"); \
  (void *)((sp + RISCV_PGSIZE) & -RISCV_PGSIZE); })

// hart-local storage
#define HLS() ((hls_t*)(MACHINE_STACK_TOP() - HLS_SIZE))
#define OTHER_HLS(id) ((hls_t*)((void *)HLS() + RISCV_PGSIZE * ((id) - read_const_csr(mhartid))))

#endif


#endif

