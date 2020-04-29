
/* Define three pools with sizes 256, 512, and 1512 bytes */
LWIP_MALLOC_MEMPOOL_START
LWIP_MALLOC_MEMPOOL(20, 256)
LWIP_MALLOC_MEMPOOL(10, 512)
LWIP_MALLOC_MEMPOOL(5, 1512)
LWIP_MALLOC_MEMPOOL(5, 1540)
LWIP_MALLOC_MEMPOOL_END 

/* My sys_arch uses memory pools to allocate mbox and sems */
//LWIP_MEMPOOL(SYS_MBOX, 22, sizeof(struct sys_mbox_struct), "SYS_MBOX")
//LWIP_MEMPOOL(SYS_SEM, 12, sizeof(struct sys_sem_struct), "SYS_SEM")
