  .text
  .align 3
  .globl g5soc_init_mutex
  .globl g5soc_take_mutex
  .globl g5soc_release_mutex

g5soc_init_mutex:
  sw x0,(a0)
  ret

g5soc_take_mutex:
  li t0, 1
take_loop:
  amoswap.w t1,t0,(a0)
  bnez t1, take_loop
  ret

g5soc_release_mutex:
  amoswap.w x0,x0,(a0)
  ret

