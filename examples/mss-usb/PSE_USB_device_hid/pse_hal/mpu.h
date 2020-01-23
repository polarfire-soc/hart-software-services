/*
 * mpu.h
 *
 *  Created on: Aug 23, 2017
 *      Author: Ken.Irving
 */

#ifndef MPU_H
#define MPU_H

#include <stdint.h>

typedef struct {

	struct {
		volatile uint64_t  	pmp   : 38;
		volatile uint64_t  	rsrvd : 18;
		volatile uint64_t  	mode  : 8;
	} CFG[16];

	volatile uint64_t 	STATUS;
	uint64_t 	gap[15];
} mpu_t;

#define MPU_MODE_READ		(1u << 0u)
#define MPU_MODE_WRITE		(1u << 1u)
#define MPU_MODE_EXEC		(1u << 2u)
#define MPU_MODE_NAPOT		(3u << 3u)

#define MPU ((mpu_t*) 0x20005000)



#define MPU_ONES(bits) ((1u << (bits))-1)

static __inline__ uint64_t MPU_CFG(uint64_t base, unsigned bits)
{
	return ((base & ~MPU_ONES(bits)) | MPU_ONES(bits-1)) >> 3u;
}


#endif

